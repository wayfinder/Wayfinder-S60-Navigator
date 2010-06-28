/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "config.h"
#include "PositionInterpolator.h"
#include "GfxUtility.h"
#include <algorithm>
#include "Utility.h"
#include "GfxConstants.h"
#include <math.h>
#include "PositionInterpolatorMath.h"

inline float64 getCosLat( int32 lat )
{
   static const float64 mc2scaletoradians = 1.0 / (11930464.7111*
                                                  180.0/3.14159265358979323846);
   return cos( mc2scaletoradians * lat );
}

PositionInterpolator::PositionInterpolator()
{
   m_convertFromMC2 = true;
   m_angleLookAheadTimeMillis = 700;

   m_debugVals.curConfig = 0;
   m_debugVals.deltaMeters = -1.0;
   m_debugVals.deltaCompareAngle = 0.0;
   m_debugVals.calcSpeedKMPH = 0.0;
   m_debugVals.correctSpeedKMPH = 0.0;
   m_startTimeMillis = MAX_INT32;
   m_catchUp.allowedCatchUpFactor = 0;
   m_debugVals.disableInterpolation = false;
   m_numReturnedPositions = 0;
   
   setConfiguration();
}

PositionInterpolator::~PositionInterpolator()
{
}

PositionInterpolator::NodeIterPair
PositionInterpolator::getCurrentNodeIterPair( float64 timeSec ) const
{
   NodeVec::const_iterator it = m_data.begin();
   NodeVec::const_iterator end = m_data.end();

   /**
    *   We traverse the data set to find the pair of nodes that
    *   timeSec lies between.
    */
   while( it != end && timeSec >= it->nodeTimeSec ) {
      it++;
   }

   if( it == end ) {
      /** Then our time is later than that of all the nodes,
       *  meaning that we do not have a path to interpolate
       *  on. So we return two iterators both pointing to
       *  the last element.
       */

      NodeIterPair ret;
      ret.first = end - 1;
      ret.second = end -1;
      // return std::make_pair( end - 1, end - 1 );
      return ret;
   } else { 
      /**
       *  We have now found the two nodes that timeSec lies between.
       */
      return std::make_pair( it - 1, it );
   }
}

float64
PositionInterpolator::getTraversedDistanceAlongPathMeters(
   unsigned int timeMillis ) const
{
   MC2Coordinate coord = getPosition( timeMillis );
   
   return getTraversedDistanceAlongPathMeters( timeMillis,
                                               coord );
}

float64
PositionInterpolator:: getTraversedDistanceAlongPathMeters(
   unsigned int timeMillis,
   MC2Coordinate lastPositionCoord  ) const
{
   NodeIterPair curNodePair = getCurrentNodeIterPair( timeMillis / 1000.0 );
   
   /**
    *  Start by calculating the length between the last node
    *  and the last coordinate.
    */ 
   
   float64 totalDistanceMeters = PositionInterpolatorMath::
      getDistanceMeters( curNodePair.first->coord,
                         lastPositionCoord,
                         m_convertFromMC2 );
   
   /**
    *   Three cases for the calculations.
    *   
    *   (A)------------(B)
    *                     \
    *                      \
    *                       \
    *                       (C)---x------(D)
    *
    */
   
   if( curNodePair.first == m_data.begin() ) {
      /**
       *  At the beginning of the set, i.e. between A and B.
       *  We have no preceding node and can return the distance
       *  as the distance between A and the coordinate requested.
       */ 

      return totalDistanceMeters;
   } else if ( curNodePair.second == curNodePair.first ) {
      /**
       *  At the end of the set, i.e. past D. Both nodes are
       *  then set to point at D. Subtract the first node to
       *  C and then sum length backwards.
       */ 

      curNodePair.first--;
      
      if( curNodePair.first == m_data.begin() ) {
         return totalDistanceMeters;
      }
   } else {

      /**
       *  Between two nodes, i.e. C and D. Subtract both
       *  iterators to point at B and C and then sum the
       *  length backwards.
       */
      
      curNodePair.first--;
      curNodePair.second--;      
   }
   
   /**
    *   Continually sum the path length backwards.
    */ 
   while( true ) {
      totalDistanceMeters += PositionInterpolatorMath::
         getDistanceMeters( curNodePair.first->coord,
                            curNodePair.second->coord,
                            m_convertFromMC2 );

      /**
       *  If we've reached the end, we are done. Otherwise,
       *  step pairwise backwards again.
       */
      
      if( curNodePair.first == m_data.begin() ) {
         break;
      } else {
         curNodePair.first--;
         curNodePair.second--;
      }
   } 
   
   return totalDistanceMeters;
}

PositionInterpolator::NodePair
PositionInterpolator::getCurrentNodePair( float64 timeSec ) const
{
   NodeIterPair iterPair = getCurrentNodeIterPair( timeSec );
   return std::make_pair( *iterPair.first, *iterPair.second );
}

MC2Coordinate
PositionInterpolator::getPosition( unsigned int timeMillis ) const
{   

   /**
    *  If we have no data, then we cannot perform any calculations.
    */  
   if( m_data.empty() ) {
      return MC2Coordinate();
   } else if( m_data.size() == 1 ) {
      return m_data.back().coord;
   }
   
   /**
    *   Likewise, if we have data but the data is only valid in a
    *   time interval which our time does not lie within, we cannot
    *   perform any calculations.
    */ 
   
   float64 curTimeSec = timeMillis / 1000.0;
   
   if( curTimeSec < m_data.front().nodeTimeSec ) {
      return m_data.front().coord;
   } else if( curTimeSec > m_data.back().nodeTimeSec ) {
      return m_data.back().coord;
   }

   /**
    *   First, find the two nodes that the position will lie between.
    */
   
   NodePair nodes = getCurrentNodePair( curTimeSec );

   /**
    *   Then, find out how far along (in meters) the position will lie
    *   on the path between these nodes.
    */ 
   
   float64 traversedDistanceMeters = PositionInterpolatorMath::
      getTraversedDistanceMeters( curTimeSec,
                                  nodes.first.nodeTimeSec,
                                  nodes.second.nodeTimeSec,
                                  nodes.first.velocityMPS,
                                  nodes.second.velocityMPS );
   
   /**
    *   Finally, use this data to determine the final position.
    */
   return PositionInterpolatorMath::
      getInterpolatedPosition( nodes.first.coord,
                               nodes.second.coord,
                               traversedDistanceMeters,
                               m_convertFromMC2 );
}

void PositionInterpolator::disableMC2Conversion()
{
   m_convertFromMC2 = false;
}

void PositionInterpolator::prepareNewData( unsigned int startTimeMillis )
{
   m_debugVals.deltaCompareCoordinate = getPosition( startTimeMillis );
   m_debugVals.deltaCompareAngle = getDirectionDegrees( startTimeMillis );
   m_debugVals.calcSpeedKMPH = getVelocityMPS( startTimeMillis ) * 3.6;

   /**
    *   Setup the start position for the catch up phase
    */
   
   if( m_catchUp.allowedCatchUpFactor > 0 ) {
      m_catchUp.startPosition = peekInterpolatedPosition( startTimeMillis );
   }

   m_data.clear();
   
   m_dataPeriodMillis.updatePeriod( startTimeMillis );
   
   m_startTimeMillis = startTimeMillis;
}

void PositionInterpolator::addDataPoint( const MC2Coordinate& coord,
                                         int velocityCmPS )
{
   InterpolationNode node;
   node.coord = coord;
   node.velocityMPS = velocityCmPS / 100.0f;
   node.nodeTimeSec = -1.0;
   m_data.push_back( node );

}

void PositionInterpolator::finalizeNewData()
{
   m_numReturnedPositions = 0;
   
   if( m_data.empty() ) {
      m_debugVals.deltaMeters = -1.0;
      return;
   }

   m_debugVals.correctSpeedKMPH = ( m_data.front().velocityMPS ) * 3.6;

   
   if( m_debugVals.deltaCompareCoordinate.isValid() &&
       m_data.front().coord.isValid() )
   {
      /**
       *  Find the angle between the two points.
       */ 
      float64 angleDegrees =
         GfxUtility::getAngleFromNorthDegrees(
            m_debugVals.deltaCompareCoordinate.lat,
            m_debugVals.deltaCompareCoordinate.lon,
            m_data.front().coord.lat,
            m_data.front().coord.lon,
            getCosLat( m_data.front().coord.lat/2 +
                       m_debugVals.deltaCompareCoordinate.lat / 2 )
            );

      /**
       *  Get the closest offset (1-d vector) going from
       *  angleDegrees -> m_debugVals.deltaCompareAngle
       */ 

      float64 angleDiffDegrees =
         PositionInterpolatorMath::getClosestAngleOffset(
            angleDegrees, m_debugVals.deltaCompareAngle );

      /**
       *  Find the distance from the simulated position and the
       *  correct position.
       */
      
      m_debugVals.deltaMeters =
         PositionInterpolatorMath::getDistanceMeters(
            m_debugVals.deltaCompareCoordinate,
            m_data.front().coord,
            m_convertFromMC2 );

      
      /**
       *  If the simulated position was ahead of the correct position,
       *  negate the sign (since we're going backwards to reach it)
       */ 
      
      if( angleDiffDegrees > 90 || angleDiffDegrees < -90 ) {
         m_debugVals.deltaMeters = -m_debugVals.deltaMeters;
      }
   }
   
   float64 cumulativeTimeSec = m_startTimeMillis / 1000.0;

   /**
    *   Iterate over the path of nodes and determine the individual
    *   times of the nodes.
    */ 
   
   for( unsigned int i = 0; i < m_data.size() - 1; i++ ){
      InterpolationNode& n0 = m_data[ i ];
      InterpolationNode& n1 = m_data[ i + 1 ];

      /**
       *   Determine the distance between this node and the next.
       */
      
      float64 distanceMeters =
         PositionInterpolatorMath::getDistanceMeters( n0.coord,
                                                      n1.coord,
                                                      m_convertFromMC2 );

      /**
       *   Determine the average velocity of the path.
       */
      
      float64 averageVelocityMPS = ( n0.velocityMPS + n1.velocityMPS ) / 2.0;

      /**
       *   Determine how it will take to traverse the path between
       *   the nodes.
       */ 
      
      float64 segmentTimeSec = distanceMeters / averageVelocityMPS;

      m_data[ i ].nodeTimeSec = cumulativeTimeSec;

      cumulativeTimeSec += segmentTimeSec;

   }

   m_data.back().nodeTimeSec = cumulativeTimeSec;

   /**
    *   We only want the "Catch up" mechanism if the allowed time to catch
    *   up is larger than zero, and the new data set has a larger timestamp
    *   than the previous one. Also, the previous set must also have
    *   returned a coordinate from getPosition.
    */
   if( m_catchUp.allowedCatchUpFactor > 0 &&
       m_dataPeriodMillis.isValid() &&
       m_catchUp.startPosition.isValid() )
   {
      initializeCatchUp();
   }

}


bool PositionInterpolator::hasUsefulInterpolationData() const
{
   bool disable =
      m_debugVals.disableInterpolation && m_numReturnedPositions > 0;
   
   return
      !m_data.empty() && m_data.size() != 1 &&
      !disable;
}

bool PositionInterpolator::withinRange( unsigned int timeMillis ) const
{
   float64 curTimeSec = static_cast<float64>( timeMillis ) / 1000.0;

   return
      curTimeSec >= m_data.front().nodeTimeSec &&
      curTimeSec <= m_data.back().nodeTimeSec;
}

bool PositionInterpolator::insideCatchUpPhase( unsigned int timeMillis ) const
{
   return m_catchUp.periodMillis.withinPeriod( timeMillis );
}

float64
PositionInterpolator::getDirectionDegrees( unsigned int timeMillis ) const
{
   /**
    *   Look back and ahead to determine two positions that
    *   we will use to determine the current angle.
    */

   unsigned int lookupTimeA =
      timeMillis - m_angleLookAheadTimeMillis / 2;

   unsigned int lookupTimeB =
      timeMillis + m_angleLookAheadTimeMillis / 2;

   MC2Coordinate posA = getPosition( lookupTimeA );
   
   MC2Coordinate posB = getPosition( lookupTimeB );
      
   float64 cosLat = getCosLat( ( posA.lat + posB.lat ) / 2 );
   
   float64 resultAngle =
      GfxUtility::getAngleFromNorthDegrees( posA.lat, posA.lon,
                                            posB.lat, posB.lon,
                                            cosLat );
   return resultAngle;

}

float64
PositionInterpolator::getVelocityMPS( unsigned int timeMillis ) const
{
   if( m_data.empty() ) {
      return 0.0;
   } else if( m_data.size() == 1 ) {
      return m_data.back().velocityMPS;
   }
   
   float64 curTimeSec = timeMillis / 1000.0;
   
   if( curTimeSec < m_data.front().nodeTimeSec ) {
      return m_data.front().velocityMPS;
   } else if( curTimeSec > m_data.back().nodeTimeSec ) {
      return m_data.back().velocityMPS;
   }

   NodePair nodes = getCurrentNodePair( curTimeSec );
   if ( nodes.first.nodeTimeSec == nodes.second.nodeTimeSec ) {
      return nodes.second.velocityMPS;
   }
   
   float64 timeDiff = nodes.second.nodeTimeSec - nodes.first.nodeTimeSec;
   float64 speedDiff = nodes.second.velocityMPS - nodes.first.velocityMPS;
   float64 curRelativePosition =
      ( curTimeSec - nodes.first.nodeTimeSec ) / timeDiff;
   
   return nodes.first.velocityMPS + curRelativePosition * speedDiff;
}

void PositionInterpolator::setAllowedCatchUpFactor( float64 factor ) 
{
   m_catchUp.allowedCatchUpFactor = factor; 
}

float64 PositionInterpolator::getPositionDeltaMeters() const
{
   return m_debugVals.deltaMeters;
}

float64 PositionInterpolator::getPrevCorrectSpeedKMPH() const
{
   return m_debugVals.correctSpeedKMPH;
}

float64 PositionInterpolator::getPrevCalcSpeedKMPH() const
{
   return m_debugVals.calcSpeedKMPH;
}

std::vector<PositionInterpolator::InterpolationNode>
PositionInterpolator::getInterpolationVector() const
{
   return m_data;
}

InterpolatedPosition
PositionInterpolator::getInterpolatedPosition( unsigned int timeMillis )
{
   m_numReturnedPositions++;
   
   InterpolatedPosition ret;
   
   if( m_debugVals.disableInterpolation ||
       !insideCatchUpPhase( timeMillis ) )
   {
      ret = peekInterpolatedPosition( timeMillis );
   } else {
      ret = getCatchUpPosition( timeMillis );
   }
   
   return ret;
}

InterpolatedPosition
PositionInterpolator::peekInterpolatedPosition( unsigned int timeMillis ) const
{
   InterpolatedPosition ret;      
   ret.coord = getPosition( timeMillis );
   ret.velocityMPS = getVelocityMPS( timeMillis );
   ret.directionDegrees = getDirectionDegrees( timeMillis );
   return ret;
}

InterpolatedPosition
PositionInterpolator::getCatchUpPosition( unsigned int timeMillis )
{
   InterpolatedPosition ret;
   
   float64 traversedDistanceMeters =
      PositionInterpolatorMath::getTraversedDistanceMeters(
         timeMillis / 1000.0,
         m_catchUp.periodMillis.start / 1000.0,
         m_catchUp.periodMillis.end / 1000.0,
         m_catchUp.startPosition.velocityMPS,
         m_catchUp.endPosition.velocityMPS );
   
   ret.coord =
      PositionInterpolatorMath::getInterpolatedPosition(
         m_catchUp.startPosition.coord,
         m_catchUp.endPosition.coord,
         traversedDistanceMeters,
         m_convertFromMC2 );
   
   ret.directionDegrees =
      PositionInterpolatorMath::getInterpolatedAngleDegrees(
         m_catchUp.startPosition.coord,
         m_catchUp.endPosition.coord,
         m_catchUp.startPosition.directionDegrees,
         m_catchUp.endPosition.directionDegrees,
         traversedDistanceMeters,
         m_convertFromMC2 );

   float normalizedTimeMillis =
      m_catchUp.periodMillis.normalizedTime( timeMillis );

   ret.velocityMPS =
      m_catchUp.startPosition.velocityMPS * ( 1.0 - normalizedTimeMillis ) +
      m_catchUp.endPosition.velocityMPS * normalizedTimeMillis;
   
   
   return ret;
}

void PositionInterpolator::initializeCatchUp()
{
   /** 
    *   Initialize the period that the catchup phase will work on.
    */ 
   unsigned int catchUpLengthMillis =
      static_cast<unsigned int> ( m_dataPeriodMillis.length() *
                                  m_catchUp.allowedCatchUpFactor );

   m_catchUp.periodMillis.updatePeriod( m_dataPeriodMillis.end,
                                        m_dataPeriodMillis.end +
                                        catchUpLengthMillis );
   
   /** 
    *   Initialize the end position of the catchup algorithm.
    */
   
   m_catchUp.endPosition =
      peekInterpolatedPosition( m_catchUp.periodMillis.end );

   /**
    *   Get the distance in meters between the start and end
    *   of the catchup algorithm.
    */
   
   float64 pathLengthMeters =
      PositionInterpolatorMath::getDistanceMeters(
         m_catchUp.startPosition.coord,
         m_catchUp.endPosition.coord,
         m_convertFromMC2 );
   
   float64 timeDiffSec = m_catchUp.periodMillis.length() / 1000.0;

   /**
    *  timeDiffSec can't be zero, so this division should be ok.
    */
   
   float64 avgSpeedMPS = pathLengthMeters / timeDiffSec;

   /**
    *  If we solve
    *
    *                 ( startPosition.velocityMPS + endPosition.velocityMPS )
    *  avgSpeedMPS = --------------------------------------------------------
    *                                       2.0
    *                                       
    *  for startPosition.velocityMPS, we get the following:
    */

   m_catchUp.startPosition.velocityMPS =
      2.0 * avgSpeedMPS - m_catchUp.endPosition.velocityMPS;
}

void PositionInterpolator::cycleConfigurationForward()
{
   m_debugVals.curConfig =
      ( m_debugVals.curConfig + 1 ) % NUM_CONFIGS;
   
   setConfiguration();
}

void PositionInterpolator::cycleConfigurationBackward()
{
   m_debugVals.curConfig--;
   
   if( m_debugVals.curConfig < 0 ) {
      m_debugVals.curConfig = NUM_CONFIGS - 1;
   }

   setConfiguration();
} 

void PositionInterpolator::setConfiguration()
{
   m_debugVals.disableInterpolation = false;
   
   switch( m_debugVals.curConfig ) {
   case CONFIG_A:
      m_catchUp.allowedCatchUpFactor = 0.25;
      break;
   case CONFIG_B:
      m_catchUp.allowedCatchUpFactor = 0.5;
      break;
   case CONFIG_C:
      m_catchUp.allowedCatchUpFactor = 0.9;
      break;
   case CONFIG_D:
      m_debugVals.disableInterpolation = true;
      break;
   }
}

int PositionInterpolator::getConfiguration() const
{
   return m_debugVals.curConfig;
}



