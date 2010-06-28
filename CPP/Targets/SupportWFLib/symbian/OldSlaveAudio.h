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

#ifndef OLD_SLAVE_AUDIO_H
#define OLD_SLAVE_AUDIO_H

// INCLUDES
#include <e32std.h>
#include <mdaaudiosampleplayer.h>
#include "SlaveAudio.h"

// FORWARD DECLARATIONS
class MSlaveAudioListener;

// CONSTANTS
#define NUM_SOUND_SLOTS 15

// CLASS DECLARATION

/**
 *  COldSlaveAudio  container control class.
 *  
 */
class COldSlaveAudio : public CSlaveAudio,
                       public MMdaAudioPlayerCallback
{
public: // Constructors and destructor

   /**
    *   First phase constructor.
    *   @param listener          Object that should receive updates
    *                            when sounds are prepared, played or
    *                            an error occurs.
    *   @param fileServerSession File server sesssion to use.
    */
   COldSlaveAudio( MSlaveAudioListener& listener,
                   RFs& fileServerSession );

   /**
    *   Second stage constructor.
    *   @param resourcePath Path where the sounds are stored.
    */
   void ConstructL( const TDesC& resourcePath );

   /**
    *   First and second phase constructors in one.
    */
   static COldSlaveAudio* NewL( MSlaveAudioListener& listener,
                                RFs& fileServerSession,
                                const TDesC& resourcePath );
   /**
    *   First and second phase constructors in one.
    *   Leaves the constructed object on the CleanupStack.
    */
   static COldSlaveAudio* NewLC( MSlaveAudioListener& listener,
                                 RFs& fileServerSession,
                                 const TDesC& resourcePath );
   
   /// Destructor
   virtual ~COldSlaveAudio();

public: // New functions

   void PrepareSound( TInt aNumberSounds, const TInt* aSounds );

   void Play();
   
   void Stop();

   void SetVolume( TInt aVolume );

   void SetMute( TBool aMute );

   TBool IsMute();

   TInt32 GetDuration();
protected:

private: 

   CMdaAudioPlayerUtility** CheckPreviousPlayers( TInt aSound, TInt &aSwitchSound, TInt aNbrChecks );

   CMdaAudioPlayerUtility** CheckCurrentPlayers( TInt aSound, TInt aNbrChecks );

   TBool CheckPointer( CMdaAudioPlayerUtility* aPlayer, TInt aNumber );
   
   TBool LoadNextByte();

   CMdaAudioPlayerUtility* LoadSoundL( TInt aSound );

   TBool GetSound( TInt aSound, TDes &aFileName );

   void DeleteAllPlayers();
   void ResetToSaneState();

   MSlaveAudioListener& m_soundListener;
   HBufC* m_resPath;
   RFs& m_fileServerSession;

public: // Functions from base classes

   virtual void MapcInitComplete(TInt aError,
                                 const TTimeIntervalMicroSeconds& aDuration);

   virtual void MapcPlayComplete(TInt aError);
private:

   CMdaAudioPlayerUtility* iCurrentPlayer;

   CMdaAudioPlayerUtility* iEndOfTurnPlayer;
   CMdaAudioPlayerUtility* iWavPlayer1;
   CMdaAudioPlayerUtility* iWavPlayer2;
   CMdaAudioPlayerUtility* iWavPlayer3;
   CMdaAudioPlayerUtility* iWavPlayer4;
   CMdaAudioPlayerUtility* iWavPlayer5;
   CMdaAudioPlayerUtility* iWavPlayer6;
   CMdaAudioPlayerUtility* iWavPlayer7;
   CMdaAudioPlayerUtility* iWavPlayer8;
   CMdaAudioPlayerUtility* iWavPlayer9;
   CMdaAudioPlayerUtility* iWavPlayer10;
   CMdaAudioPlayerUtility* iWavPlayer11;
   CMdaAudioPlayerUtility* iWavPlayer12;
   CMdaAudioPlayerUtility* iWavPlayer13;
   CMdaAudioPlayerUtility* iWavPlayer14;
   CMdaAudioPlayerUtility* iWavPlayer15;

   TInt iVolPercent;

   TBool iMute;

   TInt iSound[NUM_SOUND_SLOTS];
   TInt iPreviousSound[NUM_SOUND_SLOTS];

   TInt iNbrBytes;

   TInt iNbrLoaded;

   TBool iPlayInstruction;

   TInt iTimeingPosition;

   TInt32 iTotalDuration;

   TBool iIsEOTurn;
};

#endif // OLD_SLAVE_AUDIO_H
