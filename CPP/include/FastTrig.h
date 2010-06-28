/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

/* Fast trignometry functions.
 * These are usually integer functions that are faster than their 
 * more exact floating point counterparts. 
 */


#ifndef SHARED_FastTrig_H
#define SHARED_FastTrig_H

namespace isab {

   class FastTrig {

      public:

         /**
          * Calculate the angle of a vector based on the distances 
          * dx,dy along the axes.
          * 
          * @param dx
          *           The vector component along the secondary (normally
          *           horizontal or east-west) axis. Valid range 
          *           -32767..32767. -32768 is NOT ALLOWED. (Handled now
          *           but may be dropped later)
          *
          * @param dy
          *           The vector component along the primary (normally
          *           vertical or north-south) axis. Valid range
          *           -32767..32767. -32768 is NOT ALLOWED. (Handled now
          *           but may be dropped later)
          *
          * @return is a int8 assuming 256 units on the complete circle.
          *
          * NOTE! The angle 0 is along the *secondary* axis, i.e. north and
          * increases towards the primary axis, i.e. east. 0=north (positive 
          * y axis), 64=east (positive x axis), 128=south (negative y axis), 
          * 192=west (negative x axis).
          *
          * If dx=dy=0 then 0 (north) is returned.
          *
          * The routine reflects in the two axes and diagonal and then
          * does a table lookup for the interval 0..32 (0..pi/4) and
          * then undoes the effects of the reflections.
          */
         static uint8 angleTheta2int(int16 dx, int16 dy) 
#if !defined(_MSC_VER) && !defined(__MWERKS__)
            __attribute__ ((const))
#endif
            ;

         /**
          * Calculate the angle of a vector based on the distances 
          * dx,dy along the axes.
          * 
          * @param dx
          *           The vector component along the secondary (normally
          *           horizontal or east-west) axis.
          *
          * @param dy
          *           The vector component along the primary (normally
          *           vertical or north-south) axis.
          *
          * @return is a int8 assuming 256 units on the complete circle.
          *
          * NOTE! The angle 0 is along the *secondary* axis, i.e. north and
          * increases towards the primary axis, i.e. east. 0=north (positive 
          * y axis), 64=east (positive x axis), 128=south (negative y axis), 
          * 192=west (negative x axis).
          *
          * If dx=dy=0 then 0 (north) is returned.
          *
          * See angleTheta2int() for more details.
          */
         static uint8 angleTheta2float(float dx, float dy)
#if !defined(_MSC_VER) && !defined(__MWERKS__)
            __attribute__ ((const))
#endif
            ;

         /*
          * @param j nbr of 256-parts clockwise from the North-direction
          */
         static int32 cos256(int16 j)
#if !defined(_MSC_VER) && !defined(__MWERKS__)
            __attribute__ ((const))
#endif
            ;

         /*
          * @param j nbr of 256-parts clockwise from the North-direction
          */
         static int32 sin256(int16 j)
#if !defined(_MSC_VER) && !defined(__MWERKS__)
            __attribute__ ((const))
#endif
            ;

         /*
          * @param j nbr of 256-parts clockwise from the North-direction
          * @return sec(j) == 1/cos(j)
          */
         static int32 sec256(int16 j)
#if !defined(_MSC_VER) && !defined(__MWERKS__)
            __attribute__ ((const))
#endif
            ;


      private:
         static const uint16 shared_trig_sin_table[];
         static const uint16 shared_trig_sec_table[];
         static const uint8 angletable[];

   };

} /* namespace isab */

#endif /* SHARED_FastTrig_H */

