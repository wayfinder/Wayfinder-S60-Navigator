/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TMK_AUTO_MOVE_LIST_H
#define TMK_AUTO_MOVE_LIST_H

// -- Automovement stuff

#include <algorithm>
#include "GfxConstants.h"
#include "TMKTimingInfo.h"
#include "MapMovingInterface.h"
#include "MapDrawingInterface.h"
#include "MathUtility.h"

/**
 *   List that calculates movements for the TileMapKeyHandler
 *   when used in auto-move mode.
 */
class TMKAutoMoveList : TMKTimingInfo {
public:

   inline double LOG(const double& tal) {
#ifdef __SYMBIAN32__
      TReal res;
      Math::Ln( res, tal );
      return res;
#else
      return log(tal);
#endif
   }

   inline double EXP(const double& tal) {
#ifdef __SYMBIAN32__
      TReal res;
      Math::Exp(res, tal);
      return res;
#else
      return exp(tal);
#endif
   }

   TMKAutoMoveList(MapMovingInterface* tmh,
                   MapDrawingInterface* drawer ) {
      m_tileMapHandler = tmh;
      m_mapDrawingInterface = drawer;
      // Create vector of destinations
      // Malmö
//      m_dests.push_back (
//         TMKDestination( MC2Coordinate( 663428099, 154983536 ),
//                         1.0 ) );
//      m_dests.push_back (
//         TMKDestination( MC2Coordinate( 663428099, 154983536 ),
//                         20000 ) );
//      // Paris
//      m_dests.push_back(
//         TMKDestination( MC2Coordinate( 582925894, 27950373),
//                         1.0 ) );

//      // Around Malmö center.
//      m_dests.push_back (
//         TMKDestination( MC2Coordinate( 663374016, 155108865 ),
//                         6.9 ) );
//      m_dests.push_back (
//         TMKDestination( MC2Coordinate( 663374016, 155250555 ),
//                         6.9 ) );

      // Malmö SW
      m_dests.push_back (
         TMKDestination( MC2Coordinate( 663024828, 154093456 ),
                         6.9 ) );
      // Malmö NE
      m_dests.push_back (
         TMKDestination( MC2Coordinate( 663499060, 155658271 ),
                         6.9 ) );
      // Lund center.
      m_dests.push_back (
         TMKDestination( MC2Coordinate( 664616271, 157440644 ),
                         6.9 ) );
      // Helsingborg.
      m_dests.push_back (
         TMKDestination( MC2Coordinate( 668798623, 151386232 ),
                         6.9 ) );
         
      m_curStepDestIdx = 0;
      m_nbrRunsLeft = 0;
      createMoves();
//      m_tileMapHandler->setTMKTimingInfo( this );
   }

   static float64 mc2DistMeters( const MC2Coordinate& first,
                                 const MC2Coordinate& second ) {
      const int32 lat1 = first.lat;
      const int32 lat2 = second.lat;
      static const float64 conv_factor = M_PI / ((float64) 0x80000000 );
      const float64 cos_lat= cos( float64 ( (lat1 >> 1) + (lat2 >> 1) ) *
                                  conv_factor );
      
      const float64 delta_lat = first.lat - second.lat;
      const float64 delta_lat_sq = delta_lat * delta_lat;
      const float64 delta_lon = float64(second.lon - first.lon) * cos_lat;
      const float64 delta_lon_sq =  delta_lon * delta_lon;
      return ( (delta_lat_sq + delta_lon_sq) * 
               GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER);
   }

   void makeZoom( double fromZoom,
                  double toZoom,
                  const MC2Coordinate& coord ) {
      // I have a strong feeling that some of the things below are
      // the same.
      double fromLog = LOG(fromZoom);
      double logdiff = fromLog - LOG(toZoom);
      const double stepSize = LOG(0.5);
      int nbrSteps = abs(int(logdiff / stepSize));
      if ( nbrSteps == 0 ) {
         return;
      }
      double logDiffPerStep = -logdiff / nbrSteps;
      for ( int i = 0; i < nbrSteps; ++i ) {
         m_stepDests.push_back(
            TMKDestination( coord,
                            EXP(fromLog + i * logDiffPerStep ) ) );
      }
      // Also push back the final pos.
      m_stepDests.push_back( TMKDestination( coord, toZoom ) );
   }

   void makeLine( const MC2Coordinate& fromCoord,
                  const MC2Coordinate& toCoord,
                  const double& zoom ) {
      const float64 mc2dist = MathUtility::SQRT( mc2DistMeters( fromCoord,
                                                   toCoord ) );
      if ( mc2dist != 0 ) {
         const int32 latDiff = toCoord.lat - fromCoord.lat;
         const int32 lonDiff = toCoord.lon - fromCoord.lon;
         double pixelDist = mc2dist / zoom;
         const double pixelsPerStep = 10;
         int nbrSteps = int(pixelDist / pixelsPerStep);
         if ( nbrSteps == 0 ) {
            return;
         }
         const double latDiffPerStep = latDiff / nbrSteps;
         const double lonDiffPerStep = lonDiff / nbrSteps;
         for ( int i = 0; i < nbrSteps; ++i ) {
            m_stepDests.push_back(
               TMKDestination(
                  MC2Coordinate( int32(fromCoord.lat + i * latDiffPerStep),
                                 int32(fromCoord.lon + i * lonDiffPerStep)),
                  zoom ) );
         }
      }
      m_stepDests.push_back( TMKDestination( toCoord, zoom ) );
   }
   
   void createMoves() {
      const int nbrToDo = m_dests.size() - 1;
      for ( int i = 0; i < nbrToDo; ++i ) {
         const TMKDestination& from = m_dests[i];
         const TMKDestination& to   = m_dests[i+1];
         double maxZoom = MAX( to.m_scale, from.m_scale );
         
         if ( to.m_scale > from.m_scale ) {
            // Start by adding the zoom steps
            makeZoom( from.m_scale, to.m_scale, from.m_coord );
         }

         // Move in a line
         makeLine( from.m_coord, to.m_coord, maxZoom );
                  
         if ( to.m_scale < from.m_scale ) {
            // End by adding the zoom steps
            makeZoom( from.m_scale, to.m_scale, to.m_coord );
         }
      }

      // Make unique destinations
      {
         destVect_t tmpVec;
         // Stupid armi will crash if we don't clear the empty vector 
         // before swapping.
         tmpVec.clear();
         tmpVec.swap( m_stepDests );
         
         destVect_t::iterator last =
            std::unique( tmpVec.begin(), tmpVec.end() );
         // Insert the destinations in the vector to save
         for ( destVect_t::iterator it = tmpVec.begin();
               it != last;
               ++it ) {
            // Insert each destination 4 times.
            for( int i = 0; i < 2; ++i ) {
               m_stepDests.push_back( *it );
            }
         }
      }
      
#ifdef __unix__
      mc2log << "[TMK]: Dumping steps :" << endl;
      for ( vector<TMKDestination>::const_iterator it = m_stepDests.begin();
            it != m_stepDests.end();
            ++it ) {
         mc2log << *it << endl;
      }
      mc2log << "[TMK]: END steps :" << endl;
#endif
   }

   bool handleStep() {
      if ( m_curStepDestIdx >= m_stepDests.size() ) {
         m_curStepDestIdx = 0;
         m_nbrRunsDone++;
         m_avgTimePerRun = m_elapsedTime / m_nbrRunsDone;
         if ( --m_nbrRunsLeft <= 0 ) {
            // No more runs. Stop.
            return false;
         }
      }

      mc2log << "[TMKH]: Step is = " << m_curStepDestIdx << endl;
      if ( m_curStepDestIdx == 0 ||
           m_stepDests[ m_curStepDestIdx - 1 ] !=
           m_stepDests [ m_curStepDestIdx ] ) {
         // Avoid blinking
         m_tileMapHandler->setScale( m_stepDests[ m_curStepDestIdx ].m_scale );
         m_tileMapHandler->setCenter(m_stepDests[ m_curStepDestIdx ].m_coord );
      }
      uint32 startRepaint = TileMapUtil::currentTimeMillis();
      m_mapDrawingInterface->repaintNow();
      m_lastTime = ( TileMapUtil::currentTimeMillis() - startRepaint );
      m_elapsedTime += m_lastTime;
      ++m_curStepDestIdx;
      return true;
   }

   void addRun() {      
      m_nbrRunsLeft++;
   }
   
   void reset() {
      m_curStepDestIdx = 0;
      m_nbrRunsLeft = 1;
      const TMKDestination& curDest = m_stepDests[m_curStepDestIdx];
      m_tileMapHandler->setScale( curDest.m_scale );
      m_tileMapHandler->setCenter( curDest.m_coord );
      ++m_curStepDestIdx;
   }

   typedef vector<TMKDestination> destVect_t;
   destVect_t m_dests;
   destVect_t m_stepDests;
   MapMovingInterface* m_tileMapHandler;
   MapDrawingInterface* m_mapDrawingInterface;
   unsigned int m_curStepDestIdx;
};

// -- End this automovement stuff.

#endif
