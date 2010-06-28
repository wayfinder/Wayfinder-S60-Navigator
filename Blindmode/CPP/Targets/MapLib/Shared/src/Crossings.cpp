/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"
#include "Crossings.h"
#include "TileMapHandler.h"
#include "GfxUtility.h"
#include "GfxConstants.h"
#include "CrossingItem.h"

void
CrossingsMap::detectIntersection(  TileMapHandler* mapHandler,
                                   RoadSegment& rs1,
                                   const MC2SimpleString& name1,
                                   RoadSegment& rs2,
                                   const MC2SimpleString& name2)
{
   int32 intersectX = 0;
   int32 intersectY = 0;

   bool didIntersect =
            GfxUtility::getIntersection( rs1.startPosition().getX(),
                                    rs1.startPosition().getY(),
                                    rs1.endPosition().getX(),
                                    rs1.endPosition().getY(),
                                    rs2.startPosition().getX(),
                                    rs2.startPosition().getY(),
                                    rs2.endPosition().getX(),
                                    rs2.endPosition().getY(),
                                    intersectX, intersectY);
   if(!didIntersect)
      return;

   MC2Coordinate intersectionCoord;
   mapHandler->inverseTranformUsingCosLat(intersectionCoord,
                                          MC2Point(intersectX, intersectY));

   MC2Coordinate inner1;
   MC2Coordinate inner2;
   MC2Coordinate outer1;
   MC2Coordinate outer2;

   mapHandler->inverseTranformUsingCosLat(inner1, rs1.startPosition());
   mapHandler->inverseTranformUsingCosLat(inner2, rs1.endPosition());
   mapHandler->inverseTranformUsingCosLat(outer1, rs2.startPosition());
   mapHandler->inverseTranformUsingCosLat(outer2, rs2.endPosition());

   reportIntersection(name1, inner1, intersectionCoord);
   reportIntersection(name1, inner2, intersectionCoord);
   reportIntersection(name2, outer1, intersectionCoord);
   reportIntersection(name2, outer2, intersectionCoord);
}



void
CrossingsMap::getAllCrossings(TileMapHandler* tm,
                              MC2Coordinate position,
                              int cutoff,
                              vicVec_t& retVector)
{
   float cosLat = tm->getCosLat();

   for(CrossingsMap::iterator ci = begin(); ci!=end(); ci++) {
      MC2Coordinate intersectionCoord = ci->first;

      TileMapCoord tmPosition(position.lat, position.lon);
      TileMapCoord tmIntersection(intersectionCoord.lat,
                                  intersectionCoord.lon);

      int64 distance = tm->sqDistanceCoordToLine(tmPosition,
                                                 tmIntersection,
                                                 tmIntersection,
                                                 cosLat);

      float curDistMeter =
         sqrt( float( distance ) ) * GfxConstants::MC2SCALE_TO_METER;

      int curDist_meter = static_cast<int64>( curDistMeter + 0.5f );

      if( curDist_meter > cutoff )
         continue;

      MC2Direction direction = 
         GfxUtility::getDirection(  position.lat,
                                    position.lon,
                                    intersectionCoord.lat,
                                    intersectionCoord.lon);
      
      const Crossing& crossing = ci->second;
      
      CrossingItem* crossItem =
         new CrossingItem( curDist_meter,
                           "",
                           intersectionCoord,
                           direction,
                           crossing);

      retVector.push_back(crossItem);
   }
}


void
CrossingsMap::reportIntersection(MC2SimpleString name,
                                 MC2Coordinate position,
                                 MC2Coordinate intersection)
{
   if(position==intersection)
      return;

   CrossingsMap::iterator cmi = this->find(intersection);
   
   MC2Direction direction = 
      GfxUtility::getDirection(intersection.lat,
                               intersection.lon,
                               position.lat,
                               position.lon);

   IntersectingRoad potential;
   potential.name = name;
   potential.direction = direction;
   
   if(cmi==end()) {
      /* First IntersectingRoad, no need to check */
      Crossing c;
      c.push_back( potential );
      insert( std::make_pair(intersection, c) );
   }  else {
      Crossing& c = cmi->second;
      
      if( gateFunction(c, potential) ) {
         c.push_back( potential );         
      }
      
      std::sort(c.begin(), c.end());
   }
}


bool
CrossingsMap::gateFunction(const Crossing& crossing,
                           const IntersectingRoad& potential) const
{
   for(Crossing::const_iterator ci = crossing.begin();
       ci!=crossing.end();
       ci++)
   {
      if(!criteriaHolds(potential, *ci)) {
         return false;
      }
   }
   
   return true;
}

bool
CrossingsMap::criteriaHolds( const IntersectingRoad& lhs,
                             const IntersectingRoad& rhs) const
{
   return
      lhs.name!=rhs.name ||
      fabs(lhs.direction.GetAngle() - rhs.direction.GetAngle()) > 1.0f;
}

void
CrossingItem::printStreets() {
   for(  std::vector<IntersectingRoad>::iterator street
            = m_crossing.begin();
         street != m_crossing.end();
         street++)
   {
      mc2dbg << street->name << endl;
   }
}

bool
operator<(const IntersectingRoad& lhs, const IntersectingRoad& rhs) {
   return lhs.direction.GetAngle() < rhs.direction.GetAngle();
}

IntersectingRoadComparator::IntersectingRoadComparator(int heading)
{
   m_heading = heading;
}

IntersectingRoadComparator::result_type
IntersectingRoadComparator::operator()(const IntersectingRoad& lhs,
                                       const IntersectingRoad& rhs) const
{
   int lhsAngle = lhs.direction.GetAngle() - m_heading;
   if(lhsAngle<0)
      lhsAngle+=360;
   
   int rhsAngle = rhs.direction.GetAngle() - m_heading;
   if(rhsAngle<0)
      rhsAngle+=360;
   
   return lhsAngle < rhsAngle;
}
