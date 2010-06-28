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

#ifndef NEWAUDICOMVERNERT_H
#define NEWAUDICOMVERNERT_H

#include "NewAudioServerTypes.h"

class NewAudioConverter : public CBase {
public:

   /**
    *   Initializes some variables.
    */
   NewAudioConverter() { m_totalTimeMs = 0; }

   /**
    *   Virtual destructor. Currently does nothing.
    */
   virtual ~NewAudioConverter();

   /**
    *   Convert the clips in <code>clips</code> to PCM format
    *   suitable for playing on the NewAudioPlayer.
    *   If this function does not leave, it should call
    *   NewAudioConverterListener::conversionCompleteL when done.
    */
   virtual void convertL( TClips& clips ) = 0;

   /**
    *   Stops the conversion as soon as possible.
    *   Calls NewAudioConverterListener::conversionCompleteL when stopped
    *   or the conversion is complete (if it can't be stopped)
    */
   virtual void stop() = 0;

   /**
    *   Calculates the sum of the clips from <code>begin</code> and upto
    *   but not including <code>end</code>.
    */
   void setSoundLength( TClips::const_iterator begin,
                        TClips::const_iterator end );

   /**
    *   Returns the total duration.
    */
   long getDurationMs() const;

   /**
    *   Resets the duration.
    */
   void resetDuration();
   
protected:
   /// Total time in milliseconds
   long m_totalTimeMs;
};

#endif // NEWAUDICOMVERNERT_H
