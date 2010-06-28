/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CROSSINGS_H
#define CROSSINGS_H

#include <algorithm>
#include <functional>

#include "MC2SimpleString.h"
#include "MC2BoundingBox.h"
#include "MC2Point.h"
#include <map>
#include <vector>
#include "VicinityItem.h"
#include "PixelBox.h"
#include "Road.h"
#include "Crossings.h"
#include "MC2Direction.h"

class TileMapHandler;
class CrossingsMap;

typedef struct IntersectingRoad {
   MC2SimpleString   name;
   MC2Direction      direction;
}  IntersectingRoad;

bool operator<(const IntersectingRoad& lhs,
               const IntersectingRoad& rhs);

typedef std::vector<IntersectingRoad> Crossing;

class IntersectingRoadComparator :
   public std::binary_function<const IntersectingRoad&,
                               const IntersectingRoad&,
                               bool>
{
public:
   IntersectingRoadComparator(int heading);
   
   result_type operator()(const IntersectingRoad& lhs,
                          const IntersectingRoad& rhs) const;
private:
   int m_heading;
};


class CrossingsMap : public std::map<MC2Coordinate, Crossing>
{
public:
   /**
   *  Detects intersection between two road segments. If found,
   *  it will be added to crossCollection.
   *
   */
   void detectIntersection(TileMapHandler* mapHandler,
                           RoadSegment& rs1,
                           const MC2SimpleString& name1,
                           RoadSegment& rs2,
                           const MC2SimpleString& name2);

   void getAllCrossings(   TileMapHandler* tm,
                           MC2Coordinate position,
                           int cutoff,
                           vicVec_t& retVector);
private:
   void reportIntersection(MC2SimpleString roadName,
                           MC2Coordinate position,
                           MC2Coordinate intersection);

   bool criteriaHolds( const IntersectingRoad& lhs,
                       const IntersectingRoad& rhs) const;
   
   bool gateFunction( const Crossing& crossing,
                      const IntersectingRoad& potential) const;
};

bool operator<(const IntersectingRoad& lhs, const IntersectingRoad& rhs);


typedef std::vector<Road> RoadCollection;

#endif

