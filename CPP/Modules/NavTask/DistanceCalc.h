/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef DISTANCECALC_H
#define DISTANCECALC_H

#include "Penalties.h"
#include <arch.h>
#include <math.h>
namespace isab{
   class DistanceCalc{
   public:
      /**
       * @param angleToSegment        byte where the compass 
       *                              heading from the point P to the closest 
       *                              point on the line segment will be 
       *                              written.
       * @param segmentAngle          byte where the compass 
       *                              heading of the line segment will be 
       *                              written. Heading is from point 1 to 
       *                              point 2.
       * @param realDistanceToSegment 2-byte field where the
       *                              distance to the line segment whithout 
       *                              penalties will be written.
       * @param distanceLeftOnSegment 2-byte field where the 
       *                              distance between point 2 and the point 
       *                              on the line segment closest to point P 
       *                              will be written.
       * @param segmentLength         2-byte field where the length
       *                              of the line segment will be written.
       */
      class DistanceResult {
         public:
            uint8  angleToSegment;
            uint8  segmentAngle;
            uint16 realDistanceToSegment;
            uint16 distanceLeftOnSegment;
            uint16 segmentLength;
      };

      /** Calculates the distance between the two points (x1,y1) and (x2,y2).
       * @param x1 the X-coordinate of the first point.
       * @param y1 the Y-coordinate of the first point.
       * @param x2 the X-coordinate of the second point.
       * @param y2 the Y-coordinate of the second point.
       * @return the distance between the two points. 
       */
      static int32 distance(int16 x1, int16 y1, int16 x2, int16 y2)
      {
         double x = x1 - x2;
         double y = y1 - y2;
         int32 ret = int32(sqrt(x * x + y * y));
         return ret;
      }

      /** Calculates the distance form a point to a line segment,
       * and also return the coordinates for the closest point.
       * A penalty is applied to the distance in some cases.
       *
       * The caluculation is done like this:
       *
       * Call the closest point to P on the line through 1-2 point
       * L. The distance from 1 to L along the line (positive being
       * towards 2) is given by v1L = (v12 * v1P) / ( |v12| ). If this
       * number is less than 0 or greater than |v12| the corresponding 
       * endpoint is closest, otherwise L is closest.
       *
       * @param params                a pointer to a Penalties object 
       *                              containing the penaties to apply.
       * @param carAngle              the compass heading of the car.
       * @param x1                    x-coordinate of the first line endpoint.
       * @param y1                    y-coordinate of the first line endpoint.
       * @param x2                    x-coordinate of the second line endpoint.
       * @param y2                    y-coordinate of the second line endpoint.
       * @param xP                    x-coordinate of the point P.
       * @param yP                    y-coordinate of the point P.
       * @param result                An object that is filled in with 
       *                              additional data on the segment.
       * @return the distance from point P to the line segment, with 
       *         penalties applied.
       */
      static uint16 distancePointToSegment(const Penalties* params,
                                           uint8 carAngle,
                                           int16 x1, int16 y1,
                                           int16 x2, int16 y2,
                                           int16 xP, int16 yP,
                                           DistanceResult &result);
   private:

   };

}
#endif
