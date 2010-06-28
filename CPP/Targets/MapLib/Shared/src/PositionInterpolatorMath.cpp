/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "PositionInterpolatorMath.h"
#include "GfxConstants.h"
#include <math.h>
#include "Utility.h"

inline float64 getCosLat( int32 lat )
{
   static const float64 mc2scaletoradians = 1.0 / (11930464.7111*
                                                   180.0/3.14159265358979323846);
   return cos( mc2scaletoradians * lat );
}

float64
PositionInterpolatorMath::getClosestAngleOffset( float64 angleA,
                                                 float64 angleB )
{
   float64 distCCW = 0.0;
   float64 distCW = 0.0;
   
   if( angleA < angleB ) {
      distCW = angleB - angleA;
      distCCW = 360.0 - angleB + angleA;
   } else if ( angleA > angleB ) {
      distCW = 360.0 - angleA + angleB;
      distCCW = angleA - angleB;
   }

   if( distCW < distCCW ) {
      return distCW;
   } else {
      return -distCCW;
   }    
}

float64
PositionInterpolatorMath::getDistanceMeters( const MC2Coordinate& coord1,
                                             const MC2Coordinate& coord2,
                                             bool convertFromMC2 )
{
   /**
    *   This returns the length of the path between the two nodes
    *   in nodes in meters.
    */

   float64 vecXMeters = coord2.lon - coord1.lon;
   float64 vecYMeters = coord2.lat - coord1.lat;

   /**
    *   Please note the somewhat awkward average calculation, this
    *   to avoid overflow.
    */ 
   float64 cosLat = getCosLat( coord1.lat / 2 + coord2.lat / 2 );
   
   if( convertFromMC2 ) {
      vecXMeters = vecXMeters * GfxConstants::MC2SCALE_TO_METER * cosLat;
      vecYMeters = vecYMeters * GfxConstants::MC2SCALE_TO_METER;
   }

   float64 lengthMeters = sqrt( vecXMeters * vecXMeters +
                                vecYMeters * vecYMeters );

   return lengthMeters;
}

float64
PositionInterpolatorMath::getNormalizedPosition( const MC2Coordinate& start,
                                                 const MC2Coordinate& end,
                                                 float64 traversedDistanceMeters,
                                                 bool convertFromMC2 )
{
   float64 totalDistanceMeters =
      PositionInterpolatorMath::getDistanceMeters( start,
                                                   end,
                                                   convertFromMC2 );

   if ( totalDistanceMeters == 0 ) {
      return 0.0;
   }

   float64 normalizedDistance = traversedDistanceMeters / totalDistanceMeters;

   return normalizedDistance;
}

float64
PositionInterpolatorMath::getInterpolatedAngleDegrees(
   const MC2Coordinate& start,
   const MC2Coordinate& end,
   float64 startAngleDegrees,
   float64 endAngleDegrees,
   float64 traversedDistanceMeters,
   bool convertFromMC2 )
{
   /**
    *   First, we determine a normalized distance. This value
    *   ranges between 0 and 1, and when the value is 0, the
    *   position will be that of the first node. If the value
    *   is 1, the position will be that of the second.
    */

   float64 normalizedDistance = getNormalizedPosition( start,
                                                       end,
                                                       traversedDistanceMeters,
                                                       convertFromMC2 );

      
   
   /**
    *   Then we find the optimal offset, in reality a one-dimensional vector,
    *   that we wish to interpolate over in order to get to the desired location.
    */ 

   float64 closestAngleDegrees = getClosestAngleOffset( startAngleDegrees,
                                                        endAngleDegrees );

   float64 interpolatedAngleDegrees = startAngleDegrees +
      normalizedDistance * closestAngleDegrees;
   
   if( interpolatedAngleDegrees > 360.0 ) {
      interpolatedAngleDegrees -= 360.0; 
   } else if( interpolatedAngleDegrees < 0.0 ) {
      interpolatedAngleDegrees += 360.0; 
   }
   
   return interpolatedAngleDegrees;
}

MC2Coordinate
PositionInterpolatorMath::getInterpolatedPosition(
   const MC2Coordinate& start,
   const MC2Coordinate& end,
   float64 traversedDistanceMeters,
   bool convertFromMC2 )
{
   /**
    *   First, we determine a normalized distance. This value
    *   ranges between 0 and 1, and when the value is 0, the
    *   position will be that of the first node. If the value
    *   is 1, the position will be that of the second.
    */

   float64 normalizedDistance = getNormalizedPosition( start,
                                                       end,
                                                       traversedDistanceMeters,
                                                       convertFromMC2 );
   
   /**
    *   Then, we take this normalized distance and apply to the
    *   vector between the two nodes. Please note that we do not
    *   need to convert this distance metric to meters, since the
    *   normalized distance is a relative measure.
    *
    *   Let u denote the normalized distance. If u is equal to
    *   say 0.33, we will need to obtain the vector which has a
    *   length 0.33 * the total length of the vector, going
    *   in the same direction.
    *
    *                      * ( Node B, u = 1.0 )
    *                     /  
    *                    /
    *                   /
    *                  /
    *                 /
    *                x ( u = 0.33 )
    *               /
    *              /
    *             /
    *            *  ( Node A, u = 0.0 )
    *
    *   In the graph above, the desired vector lies between 0.0 and 0.33.
    *   This vector is obtained by multiplying the vector AB (B - A)
    *   with 0.33.
    *   
    */
   float64 vecX = end.lon - start.lon;
   float64 vecY = end.lat - start.lat;

   float64 finalX = vecX * normalizedDistance + start.lon;
   float64 finalY = vecY * normalizedDistance + start.lat;

   /**
    *   Note the odd ordering, this is because lon is x and lat is y,
    *   and MC2Coordinate takes the arguments in lat-lon order.
    */

   return MC2Coordinate( static_cast<int>( finalY + 0.5 ),
                         static_cast<int>( finalX + 0.5 ) );
}

float64
PositionInterpolatorMath::getTraversedDistanceMeters(
   float64 curTimeSec,
   float64 startTimeSec,
   float64 endTimeSec,
   float64 startVelocityMPS,
   float64 endVelocityMPS )
{
   /**
    *   This is the time it takes to traverse the path
    *   from the previous node to the next node. The position
    *   will lie somewhere on this path.
    */

   float64 timeDiffSec = endTimeSec - startTimeSec;
   
   if( timeDiffSec == 0.0 ) {
      return 0.0;
   }
   
   /**
    *   This is the difference in velocity between the previous
    *   node and the next node. This determines if we should
    *   accelerate, decelerate or remain at the same speed while
    *   traversing the path.
    */

   float64 velocityDiffMPS = endVelocityMPS - startVelocityMPS;
   
   /**
    *   To correctly determine the distance traversed on the path
    *   between the previous node and the next node, we need to
    *   integrate the linear velocity function over the time axis.
    *
    *   To understand this, imagine a scenario in which we accelerate
    *   from 10 meters per second to 20 meters per second. This means that
    *   we will travel faster in the second half of the path. Another way
    *   to look at it is that the first half of the distance will not be
    *   covered in half the time (in this case it will take longer).
    *
    *   o  o
    *   | (velocity in meters per second)
    *   |
    *   |
    *   |                                         /-
    *   |                                       /-
    *   |                                     /-
    *   |                                   /-
    *   |                                 /-
    *   |                               /-
    *   |                             /-
    *   |                           /-
    *   |                         /-
    *   |                       /-   (k is the slope of
    *   |                     /       the curve)-
    *   |                   /-
    *   |                 /-
    *   |               /-
    *   |             /-
    *   |           /-
    *   |         /-
    *   |       /-
    *   |     /-
    *   |   /-
    *   | /-
    *   +---------------------------------------------------------
    *                                             (time in seconds)
    *
    *
    *   v = f(t) = kt + m      where k is the slope, m is the "y-intercept",
    *
    *   These parameters are calculated below.
    */

   float64 k = velocityDiffMPS / timeDiffSec;
   float64 m = startVelocityMPS;
   float64 t = curTimeSec - startTimeSec;
   
   /*
    *   To determine the distance traversed, we need to integrate
    *   over this function from the start time (which is the time
    *   at the start of the path from the previous node to the next).p
    *
    *   distance = F(t) = k*(t^2/2) + mt
    */

   float64 distanceMeters = k * ( t * t / 2.0 ) + m * t;

   return distanceMeters;
}


