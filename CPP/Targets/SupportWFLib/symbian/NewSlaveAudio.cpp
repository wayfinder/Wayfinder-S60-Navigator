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
#include "AudioClipsEnum.h"
#include "AudioClipTable.h"

#include "WFTextUtil.h"
#include "NewSlaveAudio.h"
#include "SlaveAudioListener.h"
#include "WFSymbianUtil.h"
#if defined NAV2_CLIENT_SERIES60_V3
#include <profileenginesdkcrkeys.h>
#endif

using namespace isab;

// #ifndef USE_TRACE
//  #define USE_TRACE
// #endif

#undef USE_TRACE

#ifdef USE_TRACE
#include "TraceMacros.h"
#endif

// Maybe the file ending should returned by NewAudio.
_LIT(KDefaultFileEnding, ".ogg");

CNewSlaveAudio::CNewSlaveAudio( MSlaveAudioListener& listener,
                                RFs& fileSession )
      : m_soundListener( listener ),
        m_fileServerSession(fileSession)
{
   m_totalDurationTenths = 0;
   m_newAudio = NULL;
   m_mute = EFalse;
   m_noSoundPrepared = ETrue;
}


void
CNewSlaveAudio::ConstructL( const TDesC& resourcePath )
{
   m_resPath = resourcePath.AllocLC();
   m_newAudio = CNewAudio::NewLC( *this );
   CleanupStack::Pop( m_newAudio );
   CleanupStack::Pop( m_resPath );
#if !(defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
   User::LeaveIfError( m_sysAgent.Connect() );
#elif defined NAV2_CLIENT_SERIES60_V3
   m_repository = CRepository::NewL( KCRUidProfileEngine );
#endif
}

CNewSlaveAudio*
CNewSlaveAudio::NewLC( MSlaveAudioListener& listener,
                       RFs& fileServerSession,
                       const TDesC& resourcePath )
{
   CNewSlaveAudio* tmp =
      new (ELeave)CNewSlaveAudio( listener, fileServerSession );
   CleanupStack::PushL( tmp );
   tmp->ConstructL( resourcePath );
   return tmp;
}

CNewSlaveAudio*
CNewSlaveAudio::NewL( MSlaveAudioListener& listener,
                      RFs& fileServerSession,
                      const TDesC& resourcePath )
{
   CNewSlaveAudio* tmp = CNewSlaveAudio::NewLC( listener, fileServerSession,
                                                resourcePath );
   CleanupStack::Pop( tmp );
   return tmp;
}


// Destructor
CNewSlaveAudio::~CNewSlaveAudio()
{
   delete m_resPath;
   delete m_newAudio;
#if !(defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
   m_sysAgent.Close();
#elif defined NAV2_CLIENT_SERIES60_V3
   delete m_repository;
#endif
}

//using namespace isab::RouteEnums;

void
CNewSlaveAudio::PrepareSoundCompleted( TInt ms, TInt aErr )
{
   // Called by NewAudio when the preps are completed.
   m_noSoundPrepared = EFalse;
   if ( aErr == KErrNone ) {
      // Should be in tenths of seconds I think
      m_totalDurationTenths = ms / 100;      
   } else {
      m_totalDurationTenths = 0;
   }
   // Don't know what to do if it leaves. It is rude to leave there.
   TRAPD( trapRes, m_soundListener.SoundReadyL( aErr, m_totalDurationTenths ) );
   if( trapRes != KErrNone ){
    #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "Trap error#: %i", trapRes);
      TRACE_FUNC1( errStr );
      delete errStr;
    #endif
   }
}

void
CNewSlaveAudio::PlaySoundCompleted( TInt aErr )
{
   // Called by NewAudio when preparations are completed.
   if( aErr != KErrNone ){
    #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "Trap error#: %i", aErr);
      TRACE_FUNC1( errStr );
      delete errStr;
    #endif
   }

   // Don't know what to do if it leaves. It is rude to leave there.
   TRAPD( trapRes, m_soundListener.SoundPlayedL( aErr ) );
   if( trapRes != KErrNone ){
    #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "Trap error#: %i", trapRes);
      TRACE_FUNC1( errStr );
      delete errStr;
    #endif
   }
}

void CNewSlaveAudio::PrepareSound( TInt aNumberSounds, const TInt* aSounds )
{
   // Don't don't do anything at all if we have been muted.
   m_noSoundPrepared = ETrue;
   if ( false && m_mute ) {
      m_noSoundPrepared = ETrue;
      m_totalDurationTenths = 0;
      TRAPD( trapRes, m_soundListener.SoundReadyL( KErrNone,
                                                   m_totalDurationTenths ) );
      if( trapRes != KErrNone ){
       #ifdef USE_TRACE
         char* errStr = new char[32];
         sprintf(errStr, "Trap error#: %i", trapRes);
         TRACE_FUNC1( errStr );
         delete errStr;
       #endif
      }
      return;
   }
   TRAPD( res, PrepareSoundL( aNumberSounds, aSounds ) );
   if ( res != KErrNone ) {
    #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "Trap error#: %i", res);
      TRACE_FUNC1( errStr );
      delete errStr;
    #endif
      PrepareSoundCompleted( 0, res );
   } else {
      // PrepareSoundCompleted will be called later.
   }
}

void CNewSlaveAudio::ServerDied()
{
 #ifdef USE_TRACE
   TRACE_FUNC();
 #endif
}

void CNewSlaveAudio::PrepareSoundL( TInt aNumberSounds, const TInt* aSounds )
{   
   // If this function leaves, PrepareSound will call the callback   
//#define TESTA_RAMSA
#ifdef TESTA_RAMSA
   static const TInt theSounds[] =
   { AudioClipsEnum::EnSoundTheFirst,
     AudioClipsEnum::EnSoundTheSecond,
     AudioClipsEnum::EnSoundTheThird,
     AudioClipsEnum::SoundTimingMarker, // Timing here
     AudioClipsEnum::EnSoundTheFourth,
     AudioClipsEnum::EnSoundTheFifth,
     AudioClipsEnum::EnSoundTheSixth,
     AudioClipsEnum::EnSoundTheSeventh,
     AudioClipsEnum::EnSoundTheEighth,
     AudioClipsEnum::SoundEnd,         
     AudioClipsEnum::EnSoundHere };    // Not played
   
   aNumberSounds = sizeof( theSounds ) / sizeof( theSounds[0] );
   aSounds = theSounds;
#endif
                  
   // Prepare the names of the sounds for the NewAudio
   {
      // Res will be true if at least one clip exists.
      bool res = false;
      CDesCArraySeg* clips = new (ELeave) CDesCArraySeg(aNumberSounds);
      CleanupStack::PushL( clips );
      for ( int i = 0; i < aNumberSounds; ++i ) {
         TFileName fileName;         
         if ( GetSound( aSounds[i], fileName ) ) {
            // FileName or special string found.
            res = true;
            // Seems like the contents of the fileName are copied.
            clips->AppendL( fileName );
         } else {
            // Do not add stuff that hasn't got a file name.
            //break;
         }
      }
      if ( res ) {
         m_totalDurationTenths = 0;
         m_newAudio->PrepareSoundL( *clips );
      } else {
         // It will not leave here.
         TRAPD( trapRes, m_soundListener.SoundReadyL( KErrNotFound, 0 ) );
         if( trapRes != KErrNone ){
          #ifdef USE_TRACE
            char* errStr = new char[32];
            sprintf(errStr, "Trap error#: %i", trapRes);
            TRACE_FUNC1( errStr );
            delete errStr;
          #endif
         }
      }
      CleanupStack::PopAndDestroy( clips );
   }
}


void CNewSlaveAudio::Play()
{
   TBool silent = EFalse;
#if !(defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3 || defined __WINS__ || defined __WINSCW__)
   const TUid KUidSilentMode = {0x100052DF};
   silent = m_sysAgent.GetState(KUidSilentMode) != 0;
#elif defined NAV2_CLIENT_SERIES60_V3
   TInt activeProfile = 0;
   m_repository->Get( KProEngActiveProfile, activeProfile );
   silent = activeProfile == 1;
#endif
   if ( m_mute || m_noSoundPrepared || silent ) {
      // Pretend that the sound has been played
    #ifdef USE_TRACE
      TRACE_FUNC1( "Muted" );
    #endif
      PlaySoundCompleted(KErrNone);
   } else {
    #ifdef USE_TRACE
      TRACE_FUNC1( "PlaySound" );
    #endif
      m_newAudio->PlaySound();
   }
}

void CNewSlaveAudio::Stop()
{
   m_newAudio->StopSound();
}

void CNewSlaveAudio::SetVolume( TInt aVolume )
{
   if ( aVolume == 99 ) {
      aVolume = 100;
   }
   m_newAudio->SetVolume( aVolume );
}

void CNewSlaveAudio::SetMute( TBool aMute )
{
   m_mute = aMute;
   if ( m_mute ) {
      m_newAudio->StopSound();
   }
}

TBool CNewSlaveAudio::IsMute()
{
   return m_mute;
}

TInt32 CNewSlaveAudio::GetDuration()
{
   return m_totalDurationTenths;
}

static const TDesC& appendChars( TDes& dest,
                                 const char* orig )
{
   while ( *orig != 0 ) {
      dest.Append( TChar( *orig++ ) );
   }
   return dest;
}

TBool
CNewSlaveAudio::GetSound( TInt aSound, TDes &outFileName )
{
   // Special sounds 
   if ( aSound == AudioClipsEnum::SoundTimingMarker ) {
      outFileName.Copy( CNewAudio::getTimingMarker() );
      return true; // Do not add path etc.
   } else if ( aSound == AudioClipsEnum::SoundEnd ) {
      outFileName.Copy( CNewAudio::getEndMarker() );
      return true; // Do not add path etc.
   }

   // Real clips
   if ( NULL == m_audioTable ) {
      return false;
   }
   const char* fileNameChar = m_audioTable->getFileName( aSound );
   if ( fileNameChar == NULL ) {
      return false;
   }
   
   outFileName.Copy( *m_resPath );
   appendChars( outFileName, fileNameChar );
   outFileName.Append( KDefaultFileEnding );
   
   TInt symbianError;
   if (!WFSymbianUtil::doesFileExist(m_fileServerSession,
                                     outFileName, symbianError)) {
      return EFalse;
   }
   
   return ETrue;
}


// End of File
