/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MATHUTILITY_H
#define MATHUTILITY_H
#include "config.h"
#include <math.h>

#ifdef __SYMBIAN32__
#include <e32math.h>
#endif

/**
  *   Static methods concerning mathematical operations.
  * 
  */
class MathUtility {

   public:
      /**
       *    Uses logarithm.
       *    @return The number of bits needed to represent 
       *             the unsigned value.
       */
      inline static int getNbrBits( uint32 value );

      /**
       *    Uses logarithm.
       *    @return The number of bits needed to represent 
       *             the signed value.
       */
      inline static int getNbrBitsSigned( int signedValue );

      /**
       *    Generic version that doesn't use any math.
       *    @return The number of bits needed to represent 
       *             the unsigned value.
       */
      inline static int getNbrBitsGeneric( uint32 x );
      
      /**
       *    Generic version that doesn't use any math.
       *    @return The number of bits needed to represent 
       *             the signed value.
       */
      inline static int getNbrBitsSignedGeneric( int x );

      /**
       *    Calculates the square root.
       */
      inline static double SQRT( const double& tal );
      inline static float SQRT( const float& tal );

      /**
       *    Fast inverse sqrt, originally from Quake3 source.
       *    Note that this method will return an approximation.
       *    Fast implementation only for i386 and Symbian,
       *    check if it works for other CPUs before enabling. 
       *
       *    @see http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
       *    
       */
      inline static float fastInvSqrt( float x );
};

// ---- Implementation of inlined methods ----

#ifdef MC2_SYSTEM
inline int
MathUtility::getNbrBits( uint32 value )
{
   if ( value == 0 ) {
      return 1;
   }

   return (int) ceil( ::log( value + 1 ) / ::log( 2 ) );
}

inline int
MathUtility::getNbrBitsSigned( int signedValue )
{
   if ( signedValue == 0 ) {
      return 1;
   }

   return (int)ceil( ::log( abs<int>( signedValue ) + 1 ) / ::log( 2 ) ) + 1;
}
#endif
   
inline int 
MathUtility::getNbrBitsGeneric( uint32 x )
{
   // Ugly fix. Pretend that one bit is needed for the value 0.
   if ( x == 0 ) {
      return 1;
   }
   int r = 0;
   while ( x != 0 ) {
      x >>= 1;
      ++r;
   }

   return r;
}

inline int 
MathUtility::getNbrBitsSignedGeneric( int x )
{
   // Ugly fix. Pretend that one bit is needed for the value 0.
   if ( x == 0 ) {
      return 1;
   }

   if ( x < 0 ) { 
      x = ~x + 1;
   }
   return getNbrBitsGeneric( uint32( x ) ) + 1;
}

inline double 
MathUtility::SQRT( const double& tal ) {
#ifdef __SYMBIAN32__
   TReal res;
   Math::Sqrt(res, tal);
   return res;
#else
   return sqrt(tal);
#endif
}

inline float 
MathUtility::SQRT( const float& tal ) {
#ifdef __SYMBIAN32__
   TReal res;
   Math::Sqrt(res, tal);
   return res;
#else
   return sqrtf(tal);
#endif
}


inline float 
MathUtility::fastInvSqrt( float x )
{
#if defined( __SYMBIAN32__ ) || defined( __i386__ )
   // Huueeaa?
   float xhalf = 0.5f*x;
   int i = *(int*)&x;
   i = 0x5f3759df - (i >> 1);
   x = *(float*)&i;
   x = x*(1.5 - xhalf*x*x);
   return x;
#else
   return 1 / SQRT( x ); 
#endif
}

#endif
