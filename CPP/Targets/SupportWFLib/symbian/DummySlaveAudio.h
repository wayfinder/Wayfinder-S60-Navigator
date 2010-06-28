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

#ifndef BLAAARGH_DUMMYSLAVEAUDIO_H
#define BLAAARGH_DUMMYSLAVEAUDIO_H

// I'm guessing here
#include <e32base.h>

#include "SlaveAudio.h"

class MSlaveAudioListener;
class RFs;

/**
 *    Operations in DummySlaveAudio used by WayFinderAppUI/WayfinderAppUi
 */
class CDummySlaveAudio : public CSlaveAudio {
public:

   /// NewL:s a dummy slaveaudio
   static CDummySlaveAudio* NewL( MSlaveAudioListener& listener,
                                  RFs& fileServerSession,
                                  const TDesC& resourcePath );

   /// Destructor
   ~CDummySlaveAudio();
   
   /// Prepares playing of the sounds. Calls SoundReadyL when done.
   void PrepareSound( TInt aNumberSounds, const TInt* aSounds );

   /// Starts playing the sounds
   void Play();

   /// Stops converting or playing.
   void Stop();
   
   /// Sets volume as soon as possible
   void SetVolume( TInt aVolume );   

   /// Sets mute on off
   void SetMute( TBool aMute );

   /// Returns true if mute.
   TBool IsMute();

   /// Returns the duration of the clips or upto SoundTimingMarker
   TInt32 GetDuration();
private:
   /// Constructor
   CDummySlaveAudio( MSlaveAudioListener& soundListener );
   
   /// Sound listener to send updates to when sounds have been prepped etc.
   MSlaveAudioListener& m_soundListener;

   /// True if mute
   bool m_mute;
};

#endif

// End of File
