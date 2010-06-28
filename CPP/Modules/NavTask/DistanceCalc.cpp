/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DistanceCalc.h"
#include "FastTrig.h"
#include "ArchFastMath.h"

namespace isab {

   /**
    * Calculate the angle penalty
    *
    * Currently linear in the angle error
    * For very small segments (less than 5 meters long) angle errors 
    * under 56 degrees (arbitrarily chosen to be larger than 45 
    * degrees) are free. Such short segments have little precision
    * anyway.
    *
    * @param segmentLength the length of the segment in meters.
    * @param segmentAngle  the angle of the segment. 256 degrees on the 
    *                      compass, 0 is north and angel increases clockwise.
    *                      Angle is calculated in the direction the route is
    *                      going.
    * @param carAngle      the compass heading the car is travelling in. 
    * @param param         a Penalties object holding penalty parameters.
    * @return the penality.
    */
   static uint16 calc_penalty(uint16 segmentLength, uint8 segmentAngle, 
                              uint8 carAngle, const Penalties* param)
   {
      int8 heading_diff;

      heading_diff = (int8)segmentAngle - (int8)carAngle;
      if (heading_diff == MIN_INT8) heading_diff = MAX_INT8;
      if (heading_diff < 0) heading_diff = -heading_diff;
      if (segmentLength < 5 && heading_diff <= 40)
         return 0;
      return (param->anglePenalty * uint8(heading_diff)) / 128;
   }

   /**
    * If the closest point on a line segment is further away than 
    * this we do not add any penalties. It will all be off track
    * anyway. Doing it this way avoids overflows that otherwise may
    * occur when compiled with SUPPORT_FULL_RANGE.  */
#define FARTHEREST_WITH_PENALTIES ((uint16)500)


   /*+*******************************************************************
    * Calculate the distance from a point to a line segment,
    * and also return the coordinates for the closest point. 
    * A penalty is applied to the distance in some cases.
    * 
    * int16 x1  x-coordinate of the first line endpoint
    * int16 y1  y-coordinate of the first line endpoint
    * int16 x2  x-coordinate of the second line endpoint
    * int16 y2  y-coordinate of the second line endpoint
    * int16 xP  x-coordinate of the point P
    * int16 yP  y-coordinate of the point P
    *
    * Call the closest point to P on the line through 1-2 point
    * L. The distance from 1 to L along the line (positive being
    * towards 2) is given by v1L = (v12 * v1P) / ( |v12| ). If this
    * number is less than 0 or greater than |v12| the corresponding 
    * endpoint is closest, otherwise L is closest.
    *
    * WARNING: Unless compiled with SUPPORT_FULL_RANGE this 
    * function is not guaranteed to work unless the coordinates
    * are -16384<x<16383 (same for y). With the define the input
    * is prescaled if needed. If compiled with SUPPORT_FULL_RANGE
    * distances above 65535 will be returned as 65535
    *
    *******************************************************************+*/
   
   uint16 DistanceCalc::distancePointToSegment(const Penalties* params,
                                               uint8 carAngle,
                                               int16 x1, int16 y1,
                                               int16 x2, int16 y2,
                                               int16 xP, int16 yP,
                                               DistanceResult &r)
      /*
uint16 lib_point_distance_to_line_and_point_with_punish(
				       struct penaltyparams_s *penaltyparams,
				       uint8 car_angle,
		                       int16 x1,  int16 y1,
                                       int16 x2,  int16 y2,
				       int16 xP,  int16 yP,
				       uint8 *angle_to_seg,
				       uint8 *segment_angle,
				       uint16 *real_dist_to_seg,
				       uint16 *dist_left_along_seg,
				       uint16 *segment_length)
   */
   {
      int16 v12x, v12y;           /* Vector from 1 to 2 */
      int16 v1Px, v1Py;           /* Vector from 1 to P */
      int32 l_v12_sqr;            /* |v12|^2            */
      uint16 l_v12;               /* |v12|              */
      int32 scalarprod;           /* v12 * v1P          */
#ifdef SUPPORT_FULL_RANGE
      int32 v12xtmp, v12ytmp;
      int32 v1Pxtmp, v1Pytmp;
      int prescaled;
#endif

#ifdef SUPPORT_FULL_RANGE
      v12xtmp=(int32)x2-(int32)x1;
      v12ytmp=(int32)y2-(int32)y1;
      v1Pxtmp=(int32)xP-(int32)x1;
      v1Pytmp=(int32)yP-(int32)y1;
      if (v12xtmp>=32768 || v12xtmp<=-32768 ||
          v12ytmp>=32768 || v12ytmp<=-32768 ||
          v1Pxtmp>=32768 || v1Pxtmp<=-32768 ||
          v1Pytmp>=32768 || v1Pytmp<=-32768) 
         {
            v12x=v12xtmp>>1;
            v12y=v12ytmp>>1;
            v1Px=v1Pxtmp>>1;
            v1Py=v1Pytmp>>1;
            prescaled=1;
         } else {
            v12x=v12xtmp;
            v12y=v12ytmp;
            v1Px=v1Pxtmp;
            v1Py=v1Pytmp;
            prescaled=0;
         }
#else
      v12x = x2 - x1;
      v12y = y2 - y1;
      v1Px = xP - x1;
      v1Py = yP - y1;
#endif
      /* Test of v12==0 - use pythagoras in that case since
       * there is no line to test against */
      if ( v12x == 0 && v12y == 0 )
         {
            uint16 l_v1P;

#ifdef SUPPORT_FULL_RANGE
            /* Unlike the case with the scalar product below we know
             * that a prescaling must have been casued by v1P since
             * v12 is known to be the zero vector if we get here. 
             * Thus no extra checking of the size of v1P needs to be done. */
#endif
            /* V1Px,y may be negative, but the square is
             * certain to be positive and fit in 15*2=30
             * unsigned bits. Summing them yeilds at most 
             * 31 unsigned bits which is what the fast 
             * square root handles. */
            l_v1P = sqrt_int_fast(fast_sqr_int16_to_int32(v1Px)+
                             fast_sqr_int16_to_int32(v1Py));
#ifdef SUPPORT_FULL_RANGE
            if (prescaled) 
               {
                  if (l_v1P>=0x8000)
                     l_v1P=MAX_UINT16;
                  else
                     l_v1P=l_v1P<<1;
               }
#endif
            /* Distance along the line must be 0 since the line has 
             * no length. The closest point similarly must be both the 
             * points. Since we don't know about the angle of the segment
             * we always add the maximum angle penalty. This is ok since 
             * we really shouldn't be called with zero-length segments 
             * anyway. I.e. we add both the maximum angle- and 
             * the early-on-the-segment penalties */
            r.realDistanceToSegment = l_v1P;
            if (l_v1P < FARTHEREST_WITH_PENALTIES){
               /* Make sure the possibly negative penalty does not 
                  cause the unsigned l_v1P to become negative. */
               int16 penalty = params->anglePenalty + params->earlyInSegmentPenalty;
               if ((penalty > 0) || (l_v1P > -penalty)) {
                  l_v1P += penalty;
               }
            }

            if ( (v1Px != 0) || (v1Py != 0) ) {
               r.angleToSegment = FastTrig::angleTheta2int(-v1Px,-v1Py);
            } else {
               r.angleToSegment = 0;
            }
            r.distanceLeftOnSegment = 0;/* No segment to have a length along */
            r.segmentLength = 0;
            r.segmentAngle = 0;

            return l_v1P;
         }


      /* l_v12_sqr = |v12|^2. This is a 31 bit positive number. It
       * is known that l_v12_sqr > 0 since there is a test above 
       * that catches degenerate lines. */
      /* This is needed this early so we have that information for 
       * distanceLeftOnSegment */
      l_v12_sqr = fast_sqr_int16_to_int32(v12x) +
         fast_sqr_int16_to_int32(v12y);

      /* V12x,y may be negative, but the square is
       * certain to be positive and fit in 15*2=30
       * unsigned bits. Summing them yeilds at most 
       * 31 unsigned bits which is what the fast 
       * square root handles. */
      l_v12 = sqrt_int_fast(l_v12_sqr);
#ifdef SUPPORT_FULL_RANGE
      if (prescaled) 
         {
            if (l_v12>=0x8000)
               l_v12=MAX_UINT16;
            else
               l_v12=l_v12<<1;
         }
#endif
      r.segmentLength = l_v12;
      r.segmentAngle  = FastTrig::angleTheta2int(v12x,v12y);
  
      /* scalarprod= v12 * v1P. This is really a full signed 32-bit number
       * (15 bits + sign) * (15 bits + sign) = (30 bits + sign) 
       * (30 bits + sign) + (30 bits + sign) = (31 bits + sign) */
      scalarprod = fast_mul_int16_int16_to_int32(v12x,v1Px) +
         fast_mul_int16_int16_to_int32(v12y,v1Py);

      /* v12*v1P<=0 means we are outside the segment and
       * closest to point 1 - use pythagoras */
      if ( scalarprod<=0 ){
         uint16 l_v1P;
         
#ifdef SUPPORT_FULL_RANGE
         /* Prescaling may not be necessary anymore if P and 1 are close.
          * The original prescaling may have been caused by v12 alone. */
         if (prescaled && 
             v1Pxtmp<32768 && v1Pxtmp>-32768 &&
             v1Pytmp<32768 && v1Pytmp>-32768) 
            {
               v1Px=v1Pxtmp;
               v1Py=v1Pytmp;
               prescaled=0;
            }
#endif
         /* V1Px,y may be negative, but the square is
          * certain to be positive and fit in 15*2=30
          * unsigned bits. Summing them yeilds at most 
          * 31 unsigned bits which is what the fast 
          * square root handles. */
         l_v1P=sqrt_int_fast(fast_sqr_int16_to_int32(v1Px)+
                        fast_sqr_int16_to_int32(v1Py));
#ifdef SUPPORT_FULL_RANGE
         if (prescaled) 
            {
               if (l_v1P>=0x8000)
                  l_v1P=MAX_UINT16;
               else
                  l_v1P=l_v1P<<1;
            }
#endif
         
         /* Apply penalties, if close enough. Since we are "earlier" 
          * on the line than the first point we add the early penalty. */
         /* FIXME - v12x,y and v1Px,y may be prescaled differently. 
          * At the moment and with the current algorithm this is ok. Beware! */
         r.realDistanceToSegment = l_v1P;
         if (l_v1P < FARTHEREST_WITH_PENALTIES){
            /* Make sure the possibly negative penalty does not 
               cause the unsigned l_v1P to become negative. */
            int16 penalty = calc_penalty(r.segmentLength, r.segmentAngle, 
                                         carAngle, params) +
               params->earlyInSegmentPenalty;
            if ((penalty > 0) || (l_v1P > -penalty)) {
               l_v1P += penalty;
            }
         }
         
         if ( (v1Px != 0) || (v1Py != 0) ) {
            r.angleToSegment = FastTrig::angleTheta2int(-v1Px,-v1Py);
         } else {
            r.angleToSegment = r.segmentAngle;
         }
         r.distanceLeftOnSegment = l_v12;
         
         return l_v1P;
      }
  
      /* Are we past point 2? If so, use pythagoras on v2P */
      if (scalarprod > l_v12_sqr){
         uint16 l_v2P;
         int16 v2Px, v2Py;
#ifdef SUPPORT_FULL_RANGE
         int32 v2Pxtmp, v2Pytmp;

         v2Pxtmp=(int32)xP-(int32)x2;
         v2Pytmp=(int32)yP-(int32)y2;
         /* Prescaling may be necessary */
         if (v2Pxtmp>=32768 || v2Pxtmp<=-32768 ||
             v2Pytmp>=32768 || v2Pytmp<=-32768) 
            {
               v2Px=v2Pxtmp>>1;
               v2Py=v2Pytmp>>1;
               prescaled=1;
            } else {
               v2Px=v2Pxtmp;
               v2Py=v2Pytmp;
               prescaled=0;
            }
#else
         v2Px = xP - x2;
         v2Py = yP - y2;
#endif
         /* V1Px,y may be negative, but the square is
          * certain to be positive and fit in 15*2=30
          * unsigned bits. Summing them yeilds at most 
          * 31 unsigned bits which is what the fast 
          * square root handles. */
         l_v2P=sqrt_int_fast(fast_sqr_int16_to_int32(v2Px)+
                        fast_sqr_int16_to_int32(v2Py));
#ifdef SUPPORT_FULL_RANGE
         if (prescaled) 
            {
               if (l_v2P>=0x8000)
                  l_v2P=MAX_UINT16;
               else
                  l_v2P=l_v2P<<1;
            }
#endif
         /* Apply penalties, if close enough. Since we are "later" on the line
          * than the second point we dont add the early penalty. */
         /* FIXME - v12x,y and v2Px,y may be prescaled differently. At the moment
          * and with the current algorithm this is ok. Beware! */
         r.realDistanceToSegment=l_v2P;
         if (l_v2P < FARTHEREST_WITH_PENALTIES){
            l_v2P += calc_penalty(r.segmentLength, r.segmentAngle, carAngle, params);
         }
         
         if ( (v2Px != 0) || (v2Py != 0) ) {
            r.angleToSegment = FastTrig::angleTheta2int(-v2Px,-v2Py);
         } else {
            /* This is arguably incorrect, perhaps it should be the 
             * reversed direction? */
            r.angleToSegment = r.segmentAngle;
         }
         r.distanceLeftOnSegment=0;   /* Past end of segment */
         
         return l_v2P;
      }

      /* We must be on the line. Use the formula for minimum distance from
       * a point to a line.
       *   |vPL| = | (v12 x v1P) | / |v12|
       * At this point everything is scaled nicely, just do the math.
       * Crossprod is ok. The multiplication of two full int16 gives
       * a int31 (30 bits + sign). Sum two of them and you have 
       * a full int32 (31 bits + sign).
       * We know that |vPL| must fit in an uint16, so the division 
       * will be ok. Really. */
      {
         int32 crossprod;
         uint32 crossprod_pos;
         int8   pos_cross;
         uint16 l_vPL;
         uint32 l_v12_sqr_shifted;
         uint16 l_v1L;

         crossprod=fast_mul_int16_int16_to_int32(v12x,v1Py) -
            fast_mul_int16_int16_to_int32(v12y,v1Px);
         if (crossprod<0) {
            pos_cross=0;
            crossprod_pos= -crossprod;
         } else {
            pos_cross=1;
            crossprod_pos= crossprod;
         }

         /* Prescale the dividend and the divisor. Actually, scale the
          * divisor prior to taking the square root. This slows it down
          * somewhat, but thats the price for accuracy */
         l_v12_sqr_shifted=l_v12_sqr;
         while ( !(crossprod_pos     & 0x80000000) && 
                 !(l_v12_sqr_shifted & 0x60000000) )
            {
               crossprod_pos     <<= 1;
               l_v12_sqr_shifted <<= 2;
            }
    
         l_vPL=fast_div_uint32_uint16_to_uint16(crossprod_pos,
                                                sqrt_int_fast(l_v12_sqr_shifted));
#ifdef SUPPORT_FULL_RANGE
         if (prescaled) 
            {
               if (l_vPL>=0x8000)
                  l_vPL=MAX_UINT16;
               else
                  l_vPL=l_vPL<<1;
            }
#endif
    
         /* Calculate the distance along the segment */
         /* Prescale the dividend and the divisor. Actually, scale the
          * divisor prior to taking the square root. This slows it down
          * somewhat, but thats the price for accuracy */
         l_v12_sqr_shifted=l_v12_sqr;
         while ( !(scalarprod        & 0x80000000) && 
                 !(l_v12_sqr_shifted & 0x60000000) )
            {
               scalarprod        <<= 1;
               l_v12_sqr_shifted <<= 2;
            }
    

         /* Calculate the coordinates for the point L. Calculate by
          *   L=v1+ |v1L|*v12 * |v12|^-1        */
         l_v1L = fast_div_uint32_uint16_to_uint16(scalarprod,
                                                  sqrt_int_fast(l_v12_sqr_shifted));
         /* Calculate penalties */
         r.realDistanceToSegment = l_vPL;
         if (l_vPL < FARTHEREST_WITH_PENALTIES){
            l_vPL += calc_penalty(r.segmentLength, r.segmentAngle, 
                                  carAngle, params);
            
            /* Add the early-in-segment penalty if we are in fact early in 
             * a large enough regment. Very small segments are counted
             * as having no early part. */
            if (l_v1L < params->earlyInSegmentLimit){
               if (l_v12<params->smallestSegmentWithEarly){
                  l_vPL += params->earlyInSegmentPenalty;
               }
            }
         }
         
         if ( (v12x != 0) || (v12y != 0) ) {
            r.angleToSegment = FastTrig::angleTheta2int(v12x,v12y);
            if (pos_cross) 
               r.angleToSegment +=64;
            else
               r.angleToSegment -=64;
         } else {
            r.angleToSegment = r.segmentAngle;
         }
    
         r.distanceLeftOnSegment = l_v12 - l_v1L; 
         if (r.distanceLeftOnSegment > r.segmentLength) 
            r.distanceLeftOnSegment = r.segmentLength; /* Just in case... */

         return l_vPL;
      }

   }



   /*+*******************************************************************
    * Calculate the distance between two points
    * 
    * int16 x1  x-coordinate of the first point
    * int16 y1  y-coordinate of the first point
    * int16 x2  x-coordinate of the second point
    * int16 y2  y-coordinate of the second point
    *
    * WARNING: Unless compiled with SUPPORT_FULL_RANGE this 
    * function is not guaranteed to work unless the coordinates
    * are -16384<x<16383 (same for y). With the define the input
    * is prescaled if needed. If compiled with SUPPORT_FULL_RANGE
    * distances above 65535 will be returned as 65535.
    *
    *******************************************************************+*/
   
   uint16
   lib_point_local_distance_fast(int16 x1, int16 y1, int16 x2, int16 y2)
   {
      int16 v12x, v12y;           /* Vector from 1 to 2 */
      uint16 l_v12;
#ifdef SUPPORT_FULL_RANGE
      int32 v12xtmp, v12ytmp;
      int prescaled;
#endif

#ifdef SUPPORT_FULL_RANGE
      v12xtmp=(int32)x2-(int32)x1;
      v12ytmp=(int32)y2-(int32)y1;
      if (v12xtmp>=32768 || v12xtmp<=-32768 ||
          v12ytmp>=32768 || v12ytmp<=-32768)
         {
            v12x=v12xtmp>>1;
            v12y=v12ytmp>>1;
            prescaled=1;
         } else {
            v12x=v12xtmp;
            v12y=v12ytmp;
            prescaled=0;
         }
#else
      v12x=x2-x1;
      v12y=y2-y1;
#endif

      /* V1Px,y may be negative, but the square is
       * certain to be positive and fit in 15*2=30
       * unsigned bits. Summing them yeilds at most 
       * 31 unsigned bits which is what the fast 
       * square root handles. */
      l_v12=sqrt_int_fast(fast_sqr_int16_to_int32(v12x)+
                     fast_sqr_int16_to_int32(v12y));
#ifdef SUPPORT_FULL_RANGE
      if (prescaled) 
         {
            if (l_v12>=0x8000)
               l_v12=MAX_UINT16;
            else
               l_v12=l_v12<<1;
         }
#endif
      return l_v12;
   }

}
