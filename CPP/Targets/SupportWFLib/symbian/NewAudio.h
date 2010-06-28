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

#ifndef NEWAUDIO_H
#define NEWAUDIO_H

#include <e32base.h>
#include <badesca.h>

#include "NewAudioServerTypes.h"

struct NewAudioThreadData;

class MNewAudioObserver 
{
   public:
      // TInt aErr          one of the system errors
      // TInt aSoundLength  time from the start of the clips 
      //                    until the timing marker expressed 
      //                    in ms
      virtual void PrepareSoundCompleted(TInt ms, TInt aErr) = 0;

      // TInt aErr          one of the system errors
      virtual void PlaySoundCompleted(TInt aErr) = 0;

      virtual void ServerDied() = 0;

};

class CNewAudioOwner;

// Use this calss to connect to a preexisting server.
// This is probably not intended, use CNewAudio instead.
class CNewAudioNoServer : protected RSessionBase, protected CActive
{
   public:

      /// Returns the special string for the timing marker sound
      static const TDesC& getTimingMarker();

      /// Returns the special string for the end marker.
      static const TDesC& getEndMarker();

      static CNewAudioNoServer* NewLC(MNewAudioObserver & aObs);
      virtual ~CNewAudioNoServer();
      void ConstructL();

      /// Loads the sounds from disk. Calls PrepareSoundCompleted when done
      void PrepareSoundL( CDesCArray& clips );
      /// Plays the previously loaded sounds. Calls PlaySoundCompleted after.
      void PlaySound();
      /// Stops the current sound
      void StopSound();
      /// Sets volume in percent of max volume.
      void SetVolume( int volPercent );
      /// Disconnects the server. Cannot be used after that.
      void Disconnect();
      
   protected:
      CNewAudioNoServer(MNewAudioObserver & aObs);
    

   protected:   // From CActive
      void DoCancel();
      void RunL();
      TInt RunError(TInt aError);

   protected:
      MNewAudioObserver & iObs;
      NewAudioServerTypes::TRequestTypes iOutstandingRequest;
      TInt iSoundLength;              // Filled in by the server on return
      /// Passed to the server. Points to iSoundLength
      TPtr8 iSoundLengthPtr;          
      HBufC * iMsgBuffer;
      /// Passed to the server, Points to contents of iMsgBuffer
      TPtr iMsgBufferPtr;
      /// Passed to the server. Contains the length of iMsgBuffer
      TInt iMsgBufferLength;
};


class CNewAudio : public CNewAudioNoServer
{
   public:
      static CNewAudio* NewLC(MNewAudioObserver & aObs);
      static CNewAudio* NewL(MNewAudioObserver & aObs);
      virtual ~CNewAudio();
      void ConstructL();

   protected:
      CNewAudio(MNewAudioObserver & aObs);

   protected:
      // Owns the server thread
      CNewAudioOwner * iServerOwner;
};



#endif /* NEWAUDIO_H */

