/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef PENALTIES_H
#define PENALTIES_H
#include <arch.h>
namespace isab{
   /** Class used to carry penalties to the functions using them. */
   typedef struct Penalties {
   public:
      /** Maximum penalty for angle error (given at 180 degrees) */
      int16 anglePenalty;
      /** If earlier than this and the segment is longer than 
          smallest_seg_with_early then apply early_in_seg_penalty */
      uint16 earlyInSegmentLimit;      
      /** See early_in_seg_limit */
      int16 earlyInSegmentPenalty;
      /** See early_in_seg_limit */
      uint16 smallestSegmentWithEarly;
      /** Constructor. */
/*       inline Penalties(int16 angle, uint16 earlyLimit, int16 earlyPenalty,  */
/*                 uint16 smallestEarly); */
   } Penalties;

   //===================================================
   //====== inlines for Penalties ======================
/*    inline Penalties::Penalties(int16 angle,  */
/*                         uint16 earlyLimit,  */
/*                         int16 earlyPenalty, */
/*                         uint16 smallestEarly) : */
/*       anglePenalty(angle),  */
/*       earlyInSegmentLimit(earlyLimit),  */
/*       earlyInSegmentPenalty(earlyPenalty), */
/*       smallestSegmentWithEarly(smallestEarly) */
/*    { */
/*    } */
}
#endif
