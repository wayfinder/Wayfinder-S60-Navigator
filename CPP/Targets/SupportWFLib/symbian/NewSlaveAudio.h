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


#ifndef NEW_SLAVEAUDIO_H
#define NEW_SLAVEAUDIO_H

// INCLUDES
#if !(defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
#include <saclient.h>
#elif defined NAV2_CLIENT_SERIES60_V3
#include <centralrepository.h>
#endif
#include <e32std.h>
#include <f32file.h>

#include "SlaveAudio.h"
#include "NewAudio.h"

class MSlaveAudioListener;

/**
 *  CNewSlaveAudio  container control class.
 *  Uses the NewAudio to prepare and play the clips.
 */
class CNewSlaveAudio : public CSlaveAudio,
                       public MNewAudioObserver
{
public: // Constructors and destructor

   /**
    *   First phase constructor.
    *   @param listener          Object that should receive updates
    *                            when sounds are prepared, played or
    *                            an error occurs.
    *   @param fileServerSession File server sesssion to use.
    */
   CNewSlaveAudio( MSlaveAudioListener& listener,
                   RFs& fileServerSession );

   /**
    *   Second stage constructor.
    *   @param resourcePath Path where the sounds are stored.
    */
   void ConstructL( const TDesC& resourcePath );

   /**
    *   First and second phase constructors in one.
    */
   static CNewSlaveAudio* NewL( MSlaveAudioListener& listener,
                                RFs& fileServerSession,
                                const TDesC& resourcePath );
   /**
    *   First and second phase constructors in one.
    *   Leaves the constructed object on the CleanupStack.
    */
   static CNewSlaveAudio* NewLC( MSlaveAudioListener& listener,
                                 RFs& fileServerSession,
                                 const TDesC& resourcePath );
   
   /**
    * Destructor.
    */
   virtual ~CNewSlaveAudio();

   // - MNewAudioObserver
   
   /**
    *  @param aErr          one of the system errors
    *  @param aSoundLength  time from the start of the clips 
    *                       until the timing marker expressed
    *                       in ms
    */
   void PrepareSoundCompleted(TInt ms, TInt aErr);

   /// @param aErr          one of the system errors
   void PlaySoundCompleted(TInt aErr);

   void ServerDied();

   // - End MNewAudioObserver
   
public:

   // From SlaveAudio

   void PrepareSound( TInt aNumberSounds, const TInt* aSounds );

   void Play();
   
   void Stop();

   void SetVolume( TInt aVolume );

   void SetMute( TBool aMute );

   TBool IsMute();

   TInt32 GetDuration();

   
private:

   // - Functions
   
   /// Prepares sounds. Can leave.
   void PrepareSoundL( TInt aNumberSounds, const TInt* aSounds );

   /// Translate a sound number into a filename.
   TBool GetSound( TInt aSound, TDes &aFileName );

   /// Get a filename for a sound
   const char* getFileName( int aSound );

   // - Variables
   
   /// Total duration up to the end or timing sound
   long m_totalDurationTenths;
   
   /// Sound listener to send updates to when sounds have been prepped etc.
   MSlaveAudioListener& m_soundListener;
   
   /// Path where the resources are
   HBufC* m_resPath;
   
   /// File server session needed to check the presence of the files.
   RFs& m_fileServerSession;
   
   /// Plays and prepares the sounds for real.
   CNewAudio* m_newAudio;
   
   /// True if the player is muted
   TBool m_mute;

   /// True if no sound has been prepared
   TBool m_noSoundPrepared;

#if !(defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
   // To know if the phone is in silent mode or not.
   RSystemAgent m_sysAgent;
#elif defined NAV2_CLIENT_SERIES60_V3
   // To know if the phone is in silent mode or not.
   CRepository* m_repository;
#endif

};

#endif

// End of File
