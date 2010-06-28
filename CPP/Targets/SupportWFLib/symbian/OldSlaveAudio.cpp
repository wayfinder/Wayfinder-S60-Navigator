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

// INCLUDE FILES
#include <f32file.h>
#include <e32std.h>

#include <arch.h>

#include "WFSymbianUtil.h"
#include "OldSlaveAudio.h"
#include "AudioClipsEnum.h"
#include "AudioClipTable.h"
#include "SlaveAudioListener.h"

using namespace isab;

#define LOGNEW(x,y)
#define LOGDEL(x)

COldSlaveAudio::COldSlaveAudio( MSlaveAudioListener& listener,
                          RFs& fileSession )
      : m_soundListener( listener ),
        m_fileServerSession(fileSession),
        iWavPlayer1(NULL),
        iWavPlayer2(NULL),
        iWavPlayer3(NULL),
        iWavPlayer4(NULL),
        iWavPlayer5(NULL),
        iWavPlayer6(NULL),
        iWavPlayer7(NULL),
        iWavPlayer8(NULL),
        iWavPlayer9(NULL),
        iWavPlayer10(NULL),
        iWavPlayer11(NULL), 
        iWavPlayer12(NULL),
        iWavPlayer13(NULL),
        iWavPlayer14(NULL),
        iWavPlayer15(NULL)
{
   m_resPath = NULL;
   iNbrBytes = 0;
   iVolPercent = 90;
   iMute = EFalse;
   iCurrentPlayer = NULL;
   iEndOfTurnPlayer = NULL;

   for(TInt i = 0; i < NUM_SOUND_SLOTS; ++i ) {
      iSound[i] = 0;
      iPreviousSound[i] = 0;
   }

   iTotalDuration = 0;
   iPlayInstruction = EFalse;
}

// ---------------------------------------------------------
// COldSlaveAudio::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void COldSlaveAudio::ConstructL( const TDesC& resourcePath )
{
   m_resPath = resourcePath.AllocL();
}

COldSlaveAudio*
COldSlaveAudio::NewLC( MSlaveAudioListener& listener,
                    RFs& fileServerSession,
                    const TDesC& resourcePath )
{
   COldSlaveAudio* tmp =
      new (ELeave)COldSlaveAudio( listener, fileServerSession );
   CleanupStack::PushL( tmp );
   tmp->ConstructL( resourcePath );
   return tmp;
}

COldSlaveAudio*
COldSlaveAudio::NewL( MSlaveAudioListener& listener,
                   RFs& fileServerSession,
                   const TDesC& resourcePath )
{
   COldSlaveAudio* tmp = COldSlaveAudio::NewLC( listener, fileServerSession,
                                                resourcePath );
   CleanupStack::Pop( tmp );
   return tmp;
}

// Destructor
COldSlaveAudio::~COldSlaveAudio()
{
   delete m_resPath;
   DeleteAllPlayers();
}


void COldSlaveAudio::PrepareSound( TInt aNumberSounds, const TInt* aSounds )
{
   if( iCurrentPlayer != NULL )
      iCurrentPlayer->Stop();
   
   iCurrentPlayer = NULL;
   iNbrLoaded = 0;
   iTimeingPosition = 0;
   iTotalDuration = 0;
   iNbrBytes = 0;
   iIsEOTurn = EFalse;
   TBool done;
   TInt i;

   //why?
/*    TInt sound; */
/*    for( i=0; i < aNumberSounds; i++ ){ */
/*       sound = aSounds[i]; */
/*    } */
   //end why

   if( aNumberSounds > 0 && aSounds[0] == AudioClipsEnum::SoundNewCrossing ){
      iNbrBytes = 1;
      iIsEOTurn = ETrue;
      if( iEndOfTurnPlayer == NULL ){
         iEndOfTurnPlayer = LoadSoundL( AudioClipsEnum::SoundNewCrossing );
         iNbrLoaded++;
      }
      else{
         TTimeIntervalMicroSeconds microSeconds = iEndOfTurnPlayer->Duration();
         TInt64 duration = microSeconds.Int64()/100000;
         TInt32 longDuration = LOW(duration);
         m_soundListener.SoundReadyL( KErrNone, longDuration );
      }
   }
   else{
      done = EFalse;
      i = 0;
      while( !done ){
         if( i == aNumberSounds || aSounds[i] == AudioClipsEnum::SoundEnd ){
            done = ETrue;
         }
         else if( aSounds[i] == AudioClipsEnum::SoundTimingMarker ){
            iTimeingPosition = iNbrBytes;
         }
         else if( aSounds[i] > 0 ){
            iSound[iNbrBytes++] = aSounds[i];
         }
         i++;
      }

      CMdaAudioPlayerUtility** player = NULL;
      CMdaAudioPlayerUtility* dummyPlayer;

      //this loop unrolling is baaaad XXXX
      if( iNbrBytes >= 1 ){ //play one snippet or more
         if( iPreviousSound[0] != iSound[0] ){ //not the same as last
            //find the sound at any other place in previous list
            player = CheckPreviousPlayers( iSound[0], iPreviousSound[0], 1 );
            if( player == NULL && iWavPlayer1 != NULL ){ //not found
               //dont need old player 1
               if( CheckPointer( iWavPlayer1, 1 ) ){ 
                  //no more pointers to player 1
                  LOGDEL(iWavPlayer1);
                  delete iWavPlayer1;
               }
               //forget player 1, if not deleted there are other pointers.
               iWavPlayer1 = NULL; 
            }
            else if( player != NULL ){ //found the player
               //switch place with player 1
               dummyPlayer = iWavPlayer1;
               iWavPlayer1 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[0] = iSound[0]; //remember the sound for next time.
      }
      if( iNbrBytes >= 2 ){
         if( iPreviousSound[1] != iSound[1] ){
            player = CheckPreviousPlayers( iSound[1], iPreviousSound[1], 2 );
            if( player == NULL && iWavPlayer2 != NULL ){
               if( CheckPointer( iWavPlayer2, 2 ) ){
                  LOGDEL(iWavPlayer2);
                  delete iWavPlayer2;
               }
               iWavPlayer2 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer2;
               iWavPlayer2 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[1] = iSound[1];
      }
      if( iNbrBytes >= 3 ){
         if( iPreviousSound[2] != iSound[2] ){
            player = CheckPreviousPlayers( iSound[2], iPreviousSound[2], 3 );
            if( player == NULL && iWavPlayer3 != NULL ){
               if( CheckPointer( iWavPlayer3, 3 ) ){
                  LOGDEL(iWavPlayer3);
                  delete iWavPlayer3;
               }
               iWavPlayer3 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer3;
               iWavPlayer3 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[2] = iSound[2];
      }
      if( iNbrBytes >= 4 ){
         if( iPreviousSound[3] != iSound[3] ){
            player = CheckPreviousPlayers( iSound[3], iPreviousSound[3], 4 );
            if( player == NULL && iWavPlayer4 != NULL ){
               if( CheckPointer( iWavPlayer4, 4 ) ){
                  LOGDEL(iWavPlayer4);
                  delete iWavPlayer4;
               }
               iWavPlayer4 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer4;
               iWavPlayer4 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[3] = iSound[3];
      }
      if( iNbrBytes >= 5 ){
         if( iPreviousSound[4] != iSound[4] ){
            player = CheckPreviousPlayers( iSound[4], iPreviousSound[4], 5 );
            if( player == NULL && iWavPlayer5 != NULL ){
               if( CheckPointer( iWavPlayer5, 5 ) ){
                  LOGDEL(iWavPlayer5);
                  delete iWavPlayer5;
               }
               iWavPlayer5 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer5;
               iWavPlayer5 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[4] = iSound[4];
      }
      if( iNbrBytes >= 6 ){
         if( iPreviousSound[5] != iSound[5] ){
            player = CheckPreviousPlayers( iSound[5], iPreviousSound[5], 6 );
            if( player == NULL && iWavPlayer6 != NULL ){
               if( CheckPointer( iWavPlayer6, 6 ) ){
                  LOGDEL(iWavPlayer6);
                  delete iWavPlayer6;
               }
               iWavPlayer6 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer6;
               iWavPlayer6 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[5] = iSound[5];
      }
      if( iNbrBytes >= 7 ){
         if( iPreviousSound[6] != iSound[6] ){
            player = CheckPreviousPlayers( iSound[6], iPreviousSound[6], 7 );
            if( player == NULL && iWavPlayer7 != NULL ){
               if( CheckPointer( iWavPlayer7, 7 ) ){
                  LOGDEL(iWavPlayer7);
                  delete iWavPlayer7;
               }
               iWavPlayer7 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer7;
               iWavPlayer7 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[6] = iSound[6];
      }
      if( iNbrBytes >= 8 ){
         if( iPreviousSound[7] != iSound[7] ){
            player = CheckPreviousPlayers( iSound[7], iPreviousSound[7], 8 );
            if( player == NULL && iWavPlayer8 != NULL ){
               if( CheckPointer( iWavPlayer8, 8 ) ){
                  LOGDEL(iWavPlayer8);
                  delete iWavPlayer8;
               }
               iWavPlayer8 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer8;
               iWavPlayer8 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[7] = iSound[7];
      }
      if( iNbrBytes >= 9 ){
         if( iPreviousSound[8] != iSound[8] ){
            player = CheckPreviousPlayers( iSound[8], iPreviousSound[8], 9 );
            if( player == NULL && iWavPlayer9 != NULL ){
               if( CheckPointer( iWavPlayer9, 9 ) ){
                  LOGDEL(iWavPlayer9);
                  delete iWavPlayer9;
               }
               iWavPlayer9 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer9;
               iWavPlayer9 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[8] = iSound[8];
      }
      if( iNbrBytes >= 10 ){
         if( iPreviousSound[9] != iSound[9] ){
            player = CheckPreviousPlayers( iSound[9], iPreviousSound[9], 10 );
            if( player == NULL && iWavPlayer10 != NULL ){
               if( CheckPointer( iWavPlayer10, 10 ) ){
                  LOGDEL(iWavPlayer10);
                  delete iWavPlayer10;
               }
               iWavPlayer10 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer10;
               iWavPlayer10 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[9] = iSound[9];
      }
      if( iNbrBytes >= 11 ){
         if( iPreviousSound[10] != iSound[10] ){
            player = CheckPreviousPlayers( iSound[10], iPreviousSound[10], 11 );
            if( player == NULL && iWavPlayer11 != NULL ){
               if( CheckPointer( iWavPlayer11, 11 ) ){
                  LOGDEL(iWavPlayer11);
                  delete iWavPlayer11;
               }
               iWavPlayer11 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer11;
               iWavPlayer11 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[10] = iSound[10];
      }
      if( iNbrBytes >= 12 ){
         if( iPreviousSound[11] != iSound[11] ){
            player = CheckPreviousPlayers( iSound[11], iPreviousSound[11], 12 );
            if( player == NULL && iWavPlayer12 != NULL ){
               if( CheckPointer( iWavPlayer12, 12 ) ){
                  LOGDEL(iWavPlayer12);
                  delete iWavPlayer12;
               }
               iWavPlayer12 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer12;
               iWavPlayer12 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[11] = iSound[11];
      }
      if( iNbrBytes >= 13 ){
         if( iPreviousSound[12] != iSound[12] ){
            player = CheckPreviousPlayers( iSound[12], iPreviousSound[12], 13 );
            if( player == NULL && iWavPlayer13 != NULL ){
               if( CheckPointer( iWavPlayer13, 13 ) ){
                  LOGDEL(iWavPlayer13);
                  delete iWavPlayer13;
               }
               iWavPlayer13 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer13;
               iWavPlayer13 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[12] = iSound[12];
      }
      if( iNbrBytes >= 14 ){
         if( iPreviousSound[13] != iSound[13] ){
            player = CheckPreviousPlayers( iSound[13], iPreviousSound[13], 14 );
            if( player == NULL && iWavPlayer14 != NULL ){
               if( CheckPointer( iWavPlayer14, 14 ) ){
                  LOGDEL(iWavPlayer14);
                  delete iWavPlayer14;
               }
               iWavPlayer14 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer14;
               iWavPlayer14 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[13] = iSound[13];
      }
      if( iNbrBytes >= 15 ){
         if( iPreviousSound[14] != iSound[14] ){
            player = CheckPreviousPlayers( iSound[14], iPreviousSound[14], 15 );
            if( player == NULL && iWavPlayer15 != NULL ){
               if( CheckPointer( iWavPlayer15, 15 ) ){
                  LOGDEL(iWavPlayer15);
                  delete iWavPlayer15;
               }
               iWavPlayer15 = NULL;
            }
            else if( player != NULL ){
               dummyPlayer = iWavPlayer15;
               iWavPlayer15 = *player;
               *player = dummyPlayer;
               player = NULL;
            }
         }
         iPreviousSound[14] = iSound[14];
      }
      //all players already present have been found
      done = EFalse;
      while( !done ){
         done = LoadNextByte();
      }
   }
}


void COldSlaveAudio::Play()
{
   TTimeIntervalMicroSeconds small(10);
   iCurrentPlayer = NULL;

   if( iMute ){ // XXX mute is set here since the volume zero doesn't work
      iNbrBytes = 0;
      m_soundListener.SoundPlayedL( KErrNone );
   }
   else{
      if( iIsEOTurn ){
         iCurrentPlayer = iEndOfTurnPlayer;
      }
      else{
         switch( iNbrLoaded-iNbrBytes )
         {
         case 1:
            iCurrentPlayer = iWavPlayer1;
            break;
         case 2:
            iCurrentPlayer = iWavPlayer2;
            break;
         case 3:
            iCurrentPlayer = iWavPlayer3;
            break;
         case 4:
            iCurrentPlayer = iWavPlayer4;
            break;
         case 5:
            iCurrentPlayer = iWavPlayer5;
            break;
         case 6:
            iCurrentPlayer = iWavPlayer6;
            break;
         case 7:
            iCurrentPlayer = iWavPlayer7;
            break;
         case 8:
            iCurrentPlayer = iWavPlayer8;
            break;
         case 9:
            iCurrentPlayer = iWavPlayer9;
            break;
         case 10:
            iCurrentPlayer = iWavPlayer10;
            break;
         case 11:
            iCurrentPlayer = iWavPlayer11;
            break;
         case 12:
            iCurrentPlayer = iWavPlayer12;
            break;
         case 13:
            iCurrentPlayer = iWavPlayer13;
            break;
         case 14:
            iCurrentPlayer = iWavPlayer14;
            break;
         case 15:
            iCurrentPlayer = iWavPlayer15;
            break;
         }
      }
      if( iCurrentPlayer != NULL ){
         TInt volume = 0; 
         if( !iMute )
            volume = TInt(iCurrentPlayer->MaxVolume() * ( iVolPercent * 0.01 ));
         iCurrentPlayer->SetVolume( volume );
         iCurrentPlayer->SetVolumeRamp(small);
         iCurrentPlayer->Play();
      }
      else{
         iNbrBytes = 0;
         m_soundListener.SoundPlayedL( KErrNone );
      }
   }
}

void COldSlaveAudio::Stop()
{
   if( iCurrentPlayer != NULL )
      iCurrentPlayer->Stop();
}

void COldSlaveAudio::SetVolume( TInt aVolume )
{
   iVolPercent = aVolume;
}

void COldSlaveAudio::SetMute( TBool aMute )
{
   iMute = aMute;
}

TBool COldSlaveAudio::IsMute()
{
   return iMute;
}

TInt32 COldSlaveAudio::GetDuration()
{
   return iTotalDuration;
}

CMdaAudioPlayerUtility** COldSlaveAudio::CheckPreviousPlayers( TInt aSound, 
                                                            TInt &aSwitchSound,
                                                            TInt aNbrChecks )
{
   // Disabling sound cache only works in 6600.
#define USE_SOUND_CACHING
#ifdef USE_SOUND_CACHING
   if( aNbrChecks < 1 && iPreviousSound[0] == aSound ){
      iPreviousSound[0] = aSwitchSound;
      return &iWavPlayer1;
   }
   else if( aNbrChecks < 2 && iPreviousSound[1] == aSound ){
      iPreviousSound[1] = aSwitchSound;
      return &iWavPlayer2;
   }
   else if( aNbrChecks < 3 && iPreviousSound[2] == aSound ){
      iPreviousSound[2] = aSwitchSound;
      return &iWavPlayer3;
   }
   else if( aNbrChecks < 4 && iPreviousSound[3] == aSound ){
      iPreviousSound[3] = aSwitchSound;
      return &iWavPlayer4;
   }
   else if( aNbrChecks < 5 && iPreviousSound[4] == aSound ){
      iPreviousSound[4] = aSwitchSound;
      return &iWavPlayer5;
   }
   else if( aNbrChecks < 6 && iPreviousSound[5] == aSound ){
      iPreviousSound[5] = aSwitchSound;
      return &iWavPlayer6;
   }
   else if( aNbrChecks < 7 && iPreviousSound[6] == aSound ){
      iPreviousSound[6] = aSwitchSound;
      return &iWavPlayer7;
   }
   else if( aNbrChecks < 8 && iPreviousSound[7] == aSound ){
      iPreviousSound[7] = aSwitchSound;
      return &iWavPlayer8;
   }
   else if( aNbrChecks < 9 && iPreviousSound[8] == aSound ){
      iPreviousSound[8] = aSwitchSound;
      return &iWavPlayer9;
   }
   else if( aNbrChecks < 10 && iPreviousSound[9] == aSound ){
      iPreviousSound[9] = aSwitchSound;
      return &iWavPlayer10;
   }
   else if( aNbrChecks < 11 && iPreviousSound[10] == aSound ){
      iPreviousSound[10] = aSwitchSound;
      return &iWavPlayer11;
   }
   else if( aNbrChecks < 12 && iPreviousSound[11] == aSound ){
      iPreviousSound[11] = aSwitchSound;
      return &iWavPlayer12;
   }
   else if( aNbrChecks < 13 && iPreviousSound[12] == aSound ){
      iPreviousSound[12] = aSwitchSound;
      return &iWavPlayer13;
   }
   else if( aNbrChecks < 14 && iPreviousSound[13] == aSound ){
      iPreviousSound[13] = aSwitchSound;
      return &iWavPlayer14;
   }
   else if( aNbrChecks < 15 && iPreviousSound[14] == aSound ){
      iPreviousSound[14] = aSwitchSound;
      return &iWavPlayer15;
   }
#endif
   return NULL;
}


CMdaAudioPlayerUtility** COldSlaveAudio::CheckCurrentPlayers( TInt aSound, TInt aNbrChecks )
{
   TInt i = 0;
   TBool done = EFalse;
   while( !done ){
      if( i == aNbrChecks ){
         done = ETrue;
         i = -1;
      }
      else if( aSound == iSound[i] )
         done = ETrue;
      i++;
   }
   if( i == 1 ){
      return &iWavPlayer1;
   }
   else if( i == 2 ){
      return &iWavPlayer2;
   }
   else if( i == 3 ){
      return &iWavPlayer3;
   }
   else if( i == 4 ){
      return &iWavPlayer4;
   }
   else if( i == 5 ){
      return &iWavPlayer5;
   }
   else if( i == 6 ){
      return &iWavPlayer6;
   }
   else if( i == 7 ){
      return &iWavPlayer7;
   }
   else if( i == 8 ){
      return &iWavPlayer8;
   }
   else if( i == 9 ){
      return &iWavPlayer9;
   }
   else if( i == 10 ){
      return &iWavPlayer10;
   }
   else if( i == 11 ){
      return &iWavPlayer11;
   }
   else if( i == 12 ){
      return &iWavPlayer12;
   }
   else if( i == 13 ){
      return &iWavPlayer13;
   }
   else if( i == 14 ){
      return &iWavPlayer14;
   }
   else if( i == 15 ){
      return &iWavPlayer15;
   }
   return NULL;
}


TBool COldSlaveAudio::CheckPointer( CMdaAudioPlayerUtility* aPlayer, TInt aNum )
{
   if( aNum != 1 && aPlayer == iWavPlayer1 )
      return EFalse;
   if( aNum != 2 && aPlayer == iWavPlayer2 )
      return EFalse;
   if( aNum != 3 && aPlayer == iWavPlayer3 )
      return EFalse;
   if( aNum != 4 && aPlayer == iWavPlayer4 )
      return EFalse;
   if( aNum != 5 && aPlayer == iWavPlayer5 )
      return EFalse;
   if( aNum != 6 && aPlayer == iWavPlayer6 )
      return EFalse;
   if( aNum != 7 && aPlayer == iWavPlayer7 )
      return EFalse;
   if( aNum != 8 && aPlayer == iWavPlayer8 )
      return EFalse;
   if( aNum != 9 && aPlayer == iWavPlayer9 )
      return EFalse;
   if( aNum != 10 && aPlayer == iWavPlayer10 )
      return EFalse;
   if( aNum != 11 && aPlayer == iWavPlayer11 )
      return EFalse;
   if( aNum != 12 && aPlayer == iWavPlayer12 )
      return EFalse;
   if( aNum != 13 && aPlayer == iWavPlayer13 )
      return EFalse;
   if( aNum != 14 && aPlayer == iWavPlayer14 )
      return EFalse;
   if( aNum != 15 && aPlayer == iWavPlayer15 )
      return EFalse;
   return ETrue;;
}


TBool COldSlaveAudio::LoadNextByte()
{
   TBool loading = EFalse;

   iNbrLoaded++;

   if( iNbrLoaded > iNbrBytes ){
      // Complete instruction is in memory
      loading = ETrue;
      m_soundListener.SoundReadyL( KErrNone, iTotalDuration );
   }
   else{
      TTimeIntervalMicroSeconds microSeconds( TInt64(0) );
      CMdaAudioPlayerUtility** player = NULL;
      switch( iNbrLoaded )
      {
      case 1:
         if( iWavPlayer1 == NULL ){
            player = CheckCurrentPlayers( iSound[0], 0 );
            if( player != NULL ){
               iWavPlayer1 = *player;
               microSeconds = iWavPlayer1->Duration();
            }
            else{
               iWavPlayer1 = LoadSoundL( iSound[0] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer1->Duration();
         }
         break;
      case 2:
         if( iWavPlayer2 == NULL ){
            player = CheckCurrentPlayers( iSound[1], 1 );
            if( player != NULL ){
               iWavPlayer2 = *player;
               microSeconds = iWavPlayer2->Duration();
            }
            else{
               iWavPlayer2 = LoadSoundL( iSound[1] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer2->Duration();
         }
         break;
      case 3:
         if( iWavPlayer3 == NULL ){
            player = CheckCurrentPlayers( iSound[2], 2 );
            if( player != NULL ){
               iWavPlayer3 = *player;
               microSeconds = iWavPlayer3->Duration();
            }
            else{
               iWavPlayer3 = LoadSoundL( iSound[2] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer3->Duration();
         }
         break;
      case 4:
         if( iWavPlayer4 == NULL ){
            player = CheckCurrentPlayers( iSound[3], 3 );
            if( player != NULL ){
               iWavPlayer4 = *player;
               microSeconds = iWavPlayer4->Duration();
            }
            else{
               iWavPlayer4 = LoadSoundL( iSound[3] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer4->Duration();
         }
         break;
      case 5:
         if( iWavPlayer5 == NULL ){
            player = CheckCurrentPlayers( iSound[4], 4 );
            if( player != NULL ){
               iWavPlayer5 = *player;
               microSeconds = iWavPlayer5->Duration();
            }
            else{
               iWavPlayer5 = LoadSoundL( iSound[4] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer5->Duration();
         }
         break;
      case 6:
         if( iWavPlayer6 == NULL ){
            player = CheckCurrentPlayers( iSound[5], 5 );
            if( player != NULL ){
               iWavPlayer6 = *player;
               microSeconds = iWavPlayer6->Duration();
            }
            else{
               iWavPlayer6 = LoadSoundL( iSound[5] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer6->Duration();
         }
         break;
      case 7:
         if( iWavPlayer7 == NULL ){
            player = CheckCurrentPlayers( iSound[6], 6 );
            if( player != NULL ){
               iWavPlayer7 = *player;
               microSeconds = iWavPlayer7->Duration();
            }
            else{
               iWavPlayer7 = LoadSoundL( iSound[6] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer7->Duration();
         }
         break;
      case 8:
         if( iWavPlayer8 == NULL ){
            player = CheckCurrentPlayers( iSound[7], 7 );
            if( player != NULL ){
               iWavPlayer8 = *player;
               microSeconds = iWavPlayer8->Duration();
            }
            else{
               iWavPlayer8 = LoadSoundL( iSound[7] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer8->Duration();
         }
         break;
      case 9:
         if( iWavPlayer9 == NULL ){
            player = CheckCurrentPlayers( iSound[8], 8 );
            if( player != NULL ){
               iWavPlayer9 = *player;
               microSeconds = iWavPlayer9->Duration();
            }
            else{
               iWavPlayer9 = LoadSoundL( iSound[8] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer9->Duration();
         }
         break;
      case 10:
         if( iWavPlayer10 == NULL ){
            player = CheckCurrentPlayers( iSound[9], 9 );
            if( player != NULL ){
               iWavPlayer10 = *player;
               microSeconds = iWavPlayer10->Duration();
            }
            else{
               iWavPlayer10 = LoadSoundL( iSound[9] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer10->Duration();
         }
         break;
      case 11:
         if( iWavPlayer11 == NULL ){
            player = CheckCurrentPlayers( iSound[10], 10 );
            if( player != NULL ){
               iWavPlayer11 = *player;
               microSeconds = iWavPlayer11->Duration();
            }
            else{
               iWavPlayer11 = LoadSoundL( iSound[10] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer11->Duration();
         }
         break;
      case 12:
         if( iWavPlayer12 == NULL ){
            player = CheckCurrentPlayers( iSound[11], 11 );
            if( player != NULL ){
               iWavPlayer12 = *player;
               microSeconds = iWavPlayer12->Duration();
            }
            else{
               iWavPlayer12 = LoadSoundL( iSound[11] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer12->Duration();
         }
         break;
      case 13:
         if( iWavPlayer13 == NULL ){
            player = CheckCurrentPlayers( iSound[12], 12 );
            if( player != NULL ){
               iWavPlayer13 = *player;
               microSeconds = iWavPlayer13->Duration();
            }
            else{
               iWavPlayer13 = LoadSoundL( iSound[12] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer13->Duration();
         }
         break;
      case 14:
         if( iWavPlayer14 == NULL ){
            player = CheckCurrentPlayers( iSound[13], 13 );
            if( player != NULL ){
               iWavPlayer14 = *player;
               microSeconds = iWavPlayer14->Duration();
            }
            else{
               iWavPlayer14 = LoadSoundL( iSound[13] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer14->Duration();
         }
         break;
      case 15:
         if( iWavPlayer15 == NULL ){
            player = CheckCurrentPlayers( iSound[14], 14 );
            if( player != NULL ){
               iWavPlayer15 = *player;
               microSeconds = iWavPlayer15->Duration();
            }
            else{
               iWavPlayer15 = LoadSoundL( iSound[14] );
               loading = ETrue;
            }
         }
         else{
            microSeconds = iWavPlayer15->Duration();
         }
         break;
      default:
         m_soundListener.SoundReadyL( KErrNone, iTotalDuration );
         loading = ETrue;
         break;
      }
      if( !loading ){
         if( iTimeingPosition != 0 && iNbrLoaded <= iTimeingPosition ){
            // Save duration in 1/10 second.
            TInt64 duration = microSeconds.Int64()/100000;
            TInt32 longDuration = LOW(duration);
            iTotalDuration += longDuration;
         }
      }
   }
   return loading;
}

 
CMdaAudioPlayerUtility* COldSlaveAudio::LoadSoundL( TInt aSound )
{
   TFileName fileName;
   CMdaAudioPlayerUtility* newPlayer = NULL;
   if( GetSound( aSound, fileName ) ){

      TRAPD( err,
             newPlayer = CMdaAudioPlayerUtility::NewFilePlayerL( fileName, *this ) );
      if (err){
         ResetToSaneState();
         m_soundListener.SoundReadyL( KErrNotFound, 0 );
      } else {
         LOGNEW(newPlayer, CMdaAudioPlayerUtility);
      }
   }
   else{
      ResetToSaneState();
      m_soundListener.SoundReadyL( KErrNotFound, 0 );
   }
   if( newPlayer != NULL )
      return (CMdaAudioPlayerUtility*)newPlayer;
   else
      return NULL;
}

static const TDesC& appendChars( TDes& dest,
                                 const char* orig )
{
   while ( *orig != 0 ) {
      dest.Append( TChar( *orig++ ) );
   }
   return dest;
}

TBool COldSlaveAudio::GetSound( TInt aSound, TDes &outFileName )
{
   if ( NULL == m_audioTable ) {
      return false;
   }

   const char* fileNameChar = m_audioTable->getFileName( aSound );
   if ( fileNameChar == NULL ) {
      return false;
   }
   
   outFileName.Copy( *m_resPath );
   appendChars( outFileName, fileNameChar );
   appendChars( outFileName, ".wav" );
   
   TInt symbianError;
   if (!WFSymbianUtil::doesFileExist(m_fileServerSession,
                                     outFileName, symbianError) ) {
      return EFalse;
   }

/*    iAppUI->ShowConfirmDialog( outFileName ); */
   return ETrue;
}

void
COldSlaveAudio::ResetToSaneState()
{
   iCurrentPlayer = NULL;
   for (TInt i = 0; i < NUM_SOUND_SLOTS; i++) {
      iSound[i] = 0;
      iPreviousSound[i] = 0;
   }
   iNbrBytes = 0;
   iNbrLoaded = 0;
   iTotalDuration = 0;
   iTimeingPosition = 0;
   iIsEOTurn = EFalse;
   DeleteAllPlayers();
}

void
COldSlaveAudio::DeleteAllPlayers()
{
   LOGDEL(iEndOfTurnPlayer);
   delete iEndOfTurnPlayer;
   iEndOfTurnPlayer = NULL;
   if( iWavPlayer1 != NULL && CheckPointer( iWavPlayer1, 1 ) ){
      LOGDEL(iWavPlayer1);
      delete iWavPlayer1;
   }
   iWavPlayer1 = NULL;

   if( iWavPlayer2 != NULL && CheckPointer( iWavPlayer2, 2 ) ){
      LOGDEL(iWavPlayer2);
      delete iWavPlayer2;
   }
   iWavPlayer2 = NULL;
   if( iWavPlayer3 != NULL && CheckPointer( iWavPlayer3, 3 ) ){
      LOGDEL(iWavPlayer3);
      delete iWavPlayer3;
   }
   iWavPlayer3 = NULL;
   if( iWavPlayer4 != NULL && CheckPointer( iWavPlayer4, 4 ) ){
      LOGDEL(iWavPlayer4);
      delete iWavPlayer4;
   }
   iWavPlayer4 = NULL;
   if( iWavPlayer5 != NULL && CheckPointer( iWavPlayer5, 5 ) ){
      LOGDEL(iWavPlayer5);
      delete iWavPlayer5;
   }
   iWavPlayer5 = NULL;
   if( iWavPlayer6 != NULL && CheckPointer( iWavPlayer6, 6 ) ){
      LOGDEL(iWavPlayer6);
      delete iWavPlayer6;
   }
   iWavPlayer6 = NULL;
   if( iWavPlayer7 != NULL && CheckPointer( iWavPlayer7, 7 ) ){
      LOGDEL(iWavPlayer7);
      delete iWavPlayer7;
   }
   iWavPlayer7 = NULL;
   if( iWavPlayer8 != NULL && CheckPointer( iWavPlayer8, 8 ) ){
      LOGDEL(iWavPlayer8);
      delete iWavPlayer8;
   }
   iWavPlayer8 = NULL;
   if( iWavPlayer9 != NULL && CheckPointer( iWavPlayer9, 9 ) ){
      LOGDEL(iWavPlayer9);
      delete iWavPlayer9;
   }
   iWavPlayer9 = NULL; 
   if( iWavPlayer10 != NULL && CheckPointer( iWavPlayer10, 10 ) ){
      LOGDEL(iWavPlayer10);
      delete iWavPlayer10;
   }
   iWavPlayer10 = NULL; 
   if( iWavPlayer11 != NULL && CheckPointer( iWavPlayer11, 11 ) ){
      LOGDEL(iWavPlayer11);
      delete iWavPlayer11;
   }
   iWavPlayer11 = NULL; 
   if( iWavPlayer12 != NULL && CheckPointer( iWavPlayer12, 12 ) ){
      LOGDEL(iWavPlayer12);
      delete iWavPlayer12;
   }
   iWavPlayer12 = NULL; 
   if( iWavPlayer13 != NULL && CheckPointer( iWavPlayer13, 13 ) ){
      LOGDEL(iWavPlayer13);
      delete iWavPlayer13;
   }
   iWavPlayer13 = NULL; 
   if( iWavPlayer14 != NULL && CheckPointer( iWavPlayer14, 14 ) ){
      LOGDEL(iWavPlayer14);
      delete iWavPlayer14;
   }
   iWavPlayer14 = NULL; 
   if( iWavPlayer15 != NULL && CheckPointer( iWavPlayer15, 15 ) ){
      LOGDEL(iWavPlayer15);
      delete iWavPlayer15;
   }
   iWavPlayer15 = NULL; 
}
  
void COldSlaveAudio::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds& aDuration )
{
   if(aError == KErrNone){
      if( iTimeingPosition != 0 && iNbrLoaded <= iTimeingPosition ){
         // Save duration in 1/10 second.
         TInt64 duration = aDuration.Int64()/100000;
         TInt32 longDuration = LOW(duration);
         iTotalDuration += longDuration;
      }
      TBool done = EFalse;
      while( !done ){
         done = LoadNextByte();
      }
   }
   else{
      ResetToSaneState();
      m_soundListener.SoundReadyL( aError, iTotalDuration );
   }
}

void COldSlaveAudio::MapcPlayComplete(TInt aError)
{
   iCurrentPlayer = NULL;
   if(aError == KErrNone){
      iNbrBytes--;
      if( iNbrBytes <= 0 ){
         m_soundListener.SoundPlayedL( KErrNone );
         iNbrBytes = 0;
      }
      else{
         Play();
      }
   }
   else{
      ResetToSaneState();
      m_soundListener.SoundPlayedL( aError );
   }
}


// End of File
