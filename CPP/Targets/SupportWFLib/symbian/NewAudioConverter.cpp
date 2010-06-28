/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "NewAudioConverter.h"

NewAudioConverter::~NewAudioConverter()
{
}

void
NewAudioConverter::setSoundLength( TClips::const_iterator begin,
                                   TClips::const_iterator end )
{
   // Do not set if already set. (I.e. ignore all but the first timing marker)
   if ( m_totalTimeMs < 0 ) {
      long timingSum = 0;
      for( TClips::const_iterator it = begin;
           it != end;
           ++it ) {
         // We return the length in milliseconds according to the
         // callback functions.
         TInt64 tmp = (*it)->m_time.Int64() / 1000;
         timingSum += LOW(tmp);
      }
      m_totalTimeMs = timingSum;
   }   
}

long
NewAudioConverter::getDurationMs() const
{
   if ( m_totalTimeMs < 0 ) {
      return 0;
   } else {
      return m_totalTimeMs;
   }
}

void
NewAudioConverter::resetDuration()
{
   m_totalTimeMs = -1;
}
