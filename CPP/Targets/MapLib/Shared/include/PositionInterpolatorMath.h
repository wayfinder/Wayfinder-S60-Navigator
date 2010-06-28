/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _POSITIONINTERPOLATORMATH_H_
#define _POSITIONINTERPOLATORMATH_H_

#include "PositionInterpolator.h"

class PositionInterpolatorMath {
public:
   typedef PositionInterpolator::InterpolationNode InterpolationNode;
   typedef PositionInterpolator::NodePair NodePair;
   
   /**
    *   Returns the length of the path in meters between the nodes in
    *   the argument nodes.
    */

   static float64
   getDistanceMeters( const MC2Coordinate& coord1,
                      const MC2Coordinate& coord2,
                      bool convertFromMC2 );

   /**
    *  Find the closest way to reach angleB from angleA.
    *  The two options are: going clockwise and counter-
    *  clockwise.
    *
    *  Either way:
    *
    *  ( angleA + getClosestAngleOffset( angleA, angleB ) ) % 360
    *  == angleB 
    */ 
   
   static float64
   getClosestAngleOffset( float64 angleA,
                          float64 angleB );
   
   /**
    *   Returns the normalized position ( 0..1 ), assuming
    *   that we have travelled traversedDistanceMeters from
    *   start towards end.
    *   
    *   @param   start           The start coordinate of the path
    *   @param   end             The end coordinate of the path
    *   @param   convertFromMC2  If true, treat coordinates as MC2
    *                            and convert them to meters internally
    *
    *   @return                  Normalized position between 0 and 1.
    */ 
   
   static float64
   getNormalizedPosition( const MC2Coordinate& start,
                          const MC2Coordinate& end,
                          float64 traversedDistanceMeters,
                          bool convertFromMC2 );
   
   /**
    *   Returns the interpolated position, given how many meters have
    *   been traversed going from start towards end.
    *
    *   @param   start           The start coordinate of the path
    *   @param   end             The end coordinate of the path
    *   @param   convertFromMC2  If true, treat coordinates as MC2
    *                            and convert them to meters internally
    *
    *   @return  The interpolated position between start and end where the
    *            caller has traversed traversedDistanceMeters along the path.
    *            
    */ 
   
   static MC2Coordinate
   getInterpolatedPosition( const MC2Coordinate& start,
                            const MC2Coordinate& end,
                            float64 traversedDistanceMeters,
                            bool convertFromMC2 );

   /**
    *   Returns the interpolated angle, given how many meters have
    *   been traversed going from start towards end. Using this will
    *   cause the transition of angles to be dependant on speed and
    *   acceleration.
    *
    *   @param   start           The start coordinate of the path
    *   @param   end             The end coordinate of the path
    *   @param   startAngle      The angle from which the interpolation
    *                            should start from.
    *   @param   endAngle        The angle to which the interpolation should
    *                            move towards.
    *   @param   convertFromMC2  If true, treat coordinates as MC2
    *                            and convert them to meters internally
    *
    *   @return  The interpolated angle between start and end where the
    *            caller has traversed traversedDistanceMeters along the path.
    *            
    */ 
   
   static float64
   getInterpolatedAngleDegrees( const MC2Coordinate& start,
                                const MC2Coordinate& end,
                                float64 startAngleDegrees,
                                float64 endAngleDegrees,
                                float64 traversedDistanceMeters,
                                bool convertFromMC2 );
   
   /**
    *   Returns the traversed distance in meters betwen the nodes in
    *   the argument nodes, given that the time is curTimeSec.
    *   For this to work, curTimeSec must lie between the start times
    *   of the nodes.
    */ 
   
   static float64 getTraversedDistanceMeters( float64 curTimeSec,
                                              float64 startTimeSec,
                                              float64 endTimeSec,
                                              float64 startVelocityMPS,
                                              float64 endVelocityMPS );
   
};

#endif /* _POSITIONINTERPOLATORMATH_H_ */
