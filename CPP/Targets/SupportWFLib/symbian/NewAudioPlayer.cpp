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

#include "config.h" // From maplib really.

#include "NewAudioPlayer.h"
#include "NewAudioServerTypes.h"

// FIXME: Put the needed stuff in another file.
#include "NewAudioServer.h"

#if defined NAV2_CLIENT_UIQ
   // Neede to find out the phone type.
   #include <hal.h> 
   #include <hal_data.h>
   #include <unistd.h>
#endif

// #ifndef USE_TRACE
//  #define USE_TRACE
// #endif

#undef USE_TRACE

#include "TraceMacros.h"

// Trust play complete if the phone calls MaoscPlayComplete after playing.
#define TRUST_PLAY_COMPLETE

#if defined NAV2_CLIENT_UIQ
# undef TRUST_PLAY_COMPLETE
#endif

// MaoscPlayComplete is not called for S80/S90
#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES60_V3
# undef TRUST_PLAY_COMPLETE
#endif

// In the emulator, MaoscPlayComplete is called after playing.
#if defined NAV2_CLIENT_SERIES60_V2 && !defined(__WINS__)
# undef TRUST_PLAY_COMPLETE
#endif

/// Class that is used to delete AudioPlayers in an active object.
class PlayerDeleter : public CActive
{

public:

   PlayerDeleter() : CActive(EPriorityStandard){
      CActiveScheduler::Add( this );
   }

   /// Schedule deletition of the player.
   void addPlayerToDelete( CMdaAudioOutputStream* player ) {
      m_playersToDelete.push_back( player );
      if ( ! IsActive() ) {
         class TRequestStatus* status = &iStatus; 
         iStatus = KRequestPending;
         SetActive();
         User::RequestComplete(status, KErrNone);
      }
   }

   ~PlayerDeleter() 
   {
//       if (!m_playersToDelete.empty()) {
//          TRACE_FUNC();
//       }
      for ( uint32 i = 0; i < m_playersToDelete.size(); ++i ) {
         delete m_playersToDelete[ i ];
      }
      if ( IsActive() ) {
         Cancel();
      }
   }
private:
   void RunL() {
      // Delete all players.
      // TRACE_FUNC();
      for ( uint32 i = 0; i < m_playersToDelete.size(); ++i ) {
         // TRACE_FUNC();
         delete m_playersToDelete[ i ];
      }
      m_playersToDelete.clear();
   }

   void DoCancel()
   {
   }

   std::vector<CMdaAudioOutputStream*> m_playersToDelete;
};


class NewAudioPlayerTimer : public CTimer {
public:
   static NewAudioPlayerTimer* NewL(NewAudioPlayer* player) {
      NewAudioPlayerTimer* tmp = new (ELeave) NewAudioPlayerTimer(player);
      tmp->ConstructL();
      return tmp;
   }

   /// Call stop when the timer expires.
   void delayedStop() {
      After( 1 );
   }

   void RunL() {
      // Can only do stop for now.
      m_player->stop();
   }
   
   ~NewAudioPlayerTimer() {
      if ( IsActive() ) {
         Cancel();
      }
   }
   
protected:
   NewAudioPlayerTimer( NewAudioPlayer* player)
         : CTimer( CActive::EPriorityStandard ) {
      m_player = player;
   }
   
   void ConstructL() {
      CTimer::ConstructL();
      CActiveScheduler::Add(this);
   }

private:
   /// The player to call stop on.
   NewAudioPlayer* m_player;
   
};

static TMdaAudioDataSettings::TAudioCaps
convertSampleRate(int samplerate_hz)
{
   switch ( samplerate_hz ) {
     case 8000:
        return TMdaAudioDataSettings::ESampleRate8000Hz;
     case 11025:
        return TMdaAudioDataSettings::ESampleRate11025Hz;
     case 16000:
        return TMdaAudioDataSettings::ESampleRate16000Hz;
     case 22050:
        return TMdaAudioDataSettings::ESampleRate22050Hz;
     case 32000:
        return TMdaAudioDataSettings::ESampleRate32000Hz;
     case 44100:
        return TMdaAudioDataSettings::ESampleRate44100Hz;
     case 44800:
        return TMdaAudioDataSettings::ESampleRate48000Hz;
     default:
        // Since this is for the player really, return
        // a value that will make the sound sound strange.
        return TMdaAudioDataSettings::ESampleRate16000Hz;
   }
}

static int
convertSampleOrChannelsRateBack(TMdaAudioDataSettings::TAudioCaps caps)
{
   switch ( caps ) {
      case TMdaAudioDataSettings::ESampleRate8000Hz:
         return 8000;
      case TMdaAudioDataSettings::ESampleRate11025Hz:
         return 11025;
      case TMdaAudioDataSettings::ESampleRate16000Hz:
         return 16000;
      case TMdaAudioDataSettings::ESampleRate22050Hz:
         return 22050;
      case TMdaAudioDataSettings::ESampleRate32000Hz:
         return 32000;
      case TMdaAudioDataSettings::ESampleRate44100Hz:
         return 44100;
      case TMdaAudioDataSettings::ESampleRate48000Hz:
         return 48000;
      case TMdaAudioDataSettings::EChannelsMono:
         return 1;
      case TMdaAudioDataSettings::EChannelsStereo:
         return 2;
      default:
        return 16000;
   }
}

static TMdaAudioDataSettings::TAudioCaps
convertNumChannels(int num)
{
   if ( num < 2 ) {
      return TMdaAudioDataSettings::EChannelsMono;
   } else {
      return TMdaAudioDataSettings::EChannelsStereo;
   }
}

int
NewAudioPlayer::setAudioProperties( int numchannels,
                                    int samplerate )
{
   m_wantedNbrChannels = convertNumChannels( numchannels );
   m_wantedSampleRate  = convertSampleRate( samplerate );
   return true;
}

NewAudioPlayer::NewAudioPlayer( NewAudioPlayerListener& listener )
      : m_listener( listener )
{
   // Reset all variables here
   m_playerState = NOT_OPENED;
   m_player = NULL;
   m_playerToDelete = NULL;
   m_requestActive = false;
   m_clips = NULL;
   m_wantedNbrChannels   = TMdaAudioDataSettings::EChannelsMono;
   m_wantedSampleRate    = TMdaAudioDataSettings::ESampleRate16000Hz;
   m_wantedVolumePercent = 100;
   m_stopping = false;
   m_playerDeleter = NULL;
}

long
NewAudioPlayer::getDurationOfSilenceBefore() const
{
   int sampleRateHZ = convertSampleOrChannelsRateBack( m_wantedSampleRate );
   int nbrChannels  = convertSampleOrChannelsRateBack( m_wantedNbrChannels );
   // 16 bits etc.
   return c_nbrSilenceBefore * m_silence.Length() / sampleRateHZ / nbrChannels / 2;
}

void
NewAudioPlayer::ConstructL()
{
//    #ifdef USE_TRACE
//       TRACE_FUNC();
//    #endif

   iIsA925 = EFalse;
#ifdef NAV2_CLIENT_UIQ
   TInt machineUidValue;
   HAL::Get( HALData::EMachineUid, machineUidValue );
   if( machineUidValue == 0x101F6B26 ){
      iIsA925 = ETrue;
   }
#endif

   MC2_ASSERT( m_player == NULL );
   MC2_ASSERT( m_playerToDelete == NULL );

   m_playerDeleter = new PlayerDeleter;

   m_timer = NewAudioPlayerTimer::NewL( this );
   restartPlayer();

   m_silence.SetLength( m_silence.MaxLength() );
   m_silence.FillZ();
}

NewAudioPlayer::~NewAudioPlayer()
{
   delete m_timer;
   delete m_player;
   delete m_playerDeleter;
}


void
NewAudioPlayer::setVolumePercent( int percent )
{
   m_wantedVolumePercent = percent;
   
   if ( m_player == NULL ||
        ( m_playerState != OPENED && m_playerState != PLAYING ) ) {
      return;
   }
   
   TInt playerMax = m_player->MaxVolume();
   
   if ( playerMax < 0 ) {
      return;
   }
   int wantedVolume = 1;
   if ( percent >= 100 ) {
      wantedVolume = playerMax;
   } else if ( percent <= 0 || playerMax == 0 ) {
      wantedVolume = 0;
   } else {
      wantedVolume = TInt(double(playerMax) * 0.01 * percent);
   }

   // Set it now if possible
   if ( m_playerState == OPENED || m_playerState == PLAYING ) {
      m_player->SetVolume( wantedVolume );
   }   
}

void
NewAudioPlayer::completeRequest( TInt aError )
{
   MC2_ASSERT( m_requestActive );

   // Playing should not be wanted anymore.
   if ( m_playerState == PLAYING_WANTED ) {
      m_playerState = OPENING;
   } else if ( m_playerState != NOT_OPENED ) {
      m_playerState = OPENED;
   }
   
   m_requestActive = false;
   m_stopping = false;
   m_clips = NULL;

#ifdef NAV2_CLIENT_UIQ3
   m_player->Stop();
#endif

   TRAPD( trapRes, m_listener.playCompleteL( aError ) );
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
NewAudioPlayer::MaoscOpenComplete(TInt aError)
{
   if ( aError != KErrNone ) {
      m_playerState = NOT_OPENED;
      completeRequest( aError );
      return;
   }

   // Make sure playing starts when the player is opened.
   const bool play = ( PLAYING_WANTED == m_playerState );
   m_playerState = OPENED;

   // Set volume
   setVolumePercent( m_wantedVolumePercent );

   if ( play && m_requestActive ) {
      playSounds();
   }
}

void
NewAudioPlayer::playSounds()
{
   MC2_ASSERT( m_clips != NULL );
   MC2_ASSERT( m_playerState == OPENED );

   // Delete any old player.
   delete m_playerToDelete;
   m_playerToDelete = NULL;

#ifndef NAV2_CLIENT_UIQ3
   setVolumePercent( m_wantedVolumePercent );
#endif
   
   m_nbrWrittenBuffers = 0;
   m_silenceToWrite = c_nbrSilenceToWrite;
   TInt trapRes = KErrNone;
   
   m_bufIterator = ClipBufIterator(*m_clips);
   if ( *m_bufIterator == NULL ) {
      // No convertible clips.
      completeRequest( KErrNone );
      return;
   }
   

   // Start with some silence
   for ( int i = 0; i < c_nbrSilenceBefore; ++i ) {
      TRAP( trapRes, m_player->WriteL( m_silence ) );
      if ( trapRes == KErrNone ) {
         m_nbrWrittenBuffers++;         
      }
      else{
       #ifdef USE_TRACE
         char* errStr = new char[32];
         sprintf(errStr, "Trap error#: %i", trapRes);
         TRACE_FUNC1( errStr );
         delete errStr;
       #endif
      }
   }
   
   // And write all of our ordinary clips. It is possible to write
   // only one here. If so, the rest will be written in MaoscBufferCopied
   {
      int i = 0;
      while ( *m_bufIterator ) {
         
         TRAP( trapRes, m_player->WriteL( **m_bufIterator ) );
         ++m_bufIterator;
         
         if ( trapRes == KErrNone ) {
            ++m_nbrWrittenBuffers;
         }
         else{
          #ifdef USE_TRACE
            char* errStr = new char[32];
            sprintf(errStr, "Trap error#: %i", trapRes);
            TRACE_FUNC1( errStr );
            delete errStr;
          #endif
         }
         // Limit the number of buffers written
         if ( 30 == i ) {
            break;
         }
         ++i;         
      }      
   }

   if ( *m_bufIterator == NULL ) {
      while ( m_silenceToWrite > 0 ) {
         --m_silenceToWrite;
         TRAPD( trapRes, m_player->WriteL( m_silence ) );
         if ( trapRes == KErrNone ) {
            ++m_nbrWrittenBuffers;
         }
         else{
          #ifdef USE_TRACE
            char* errStr = new char[32];
            sprintf(errStr, "Trap error#: %i", trapRes);
            TRACE_FUNC1( errStr );
            delete errStr;
          #endif
         }
      }
   }

   if ( m_nbrWrittenBuffers == 0 ) {
      // Nothing will be played.
   #ifdef USE_TRACE
      TRACE_FUNC1( "m_nbrWrittenBuffers == 0" );
   #endif
      completeRequest( trapRes );
   } else {
      m_playerState = PLAYING;
   #ifdef USE_TRACE
      TRACE_FUNC1( "m_playerState = PLAYING" );
   #endif
   }
}

void
NewAudioPlayer::playL( const TClips& clips )
{
   #ifdef USE_TRACE
      TRACE_FUNC();
   #endif
#ifdef RAISE_PRIO_ON_PLAY
   {
      class RThread thisThread; // Not pushed on the cleanup stack
      thisThread.SetPriority(EPriorityRealTime);
   }
#endif
   // Cancel stop-timer if it is running.
   if ( m_timer->IsActive() ) {
      m_timer->Cancel();
   }
   // Create new player if necessary.
   if ( m_player == NULL ) {
//      m_playerState = NOT_OPENED;
//      // Try again. Complete the request if it didn't work.
//      TRAPD( res, m_player = CMdaAudioOutputStream::NewL(*this) );
      TInt res = restartPlayer();
      if ( res != KErrNone ) {
         completeRequest( res );
         return;
      }      
   }
   /// These states etc. should not be possible
   MC2_ASSERT( m_clips == NULL );   
   MC2_ASSERT( m_requestActive == false );   
   MC2_ASSERT( m_playerState != PLAYING );
   MC2_ASSERT( m_playerState != PLAYING_WANTED );

   m_stopping = false;
   
   // Open the player here to be able to use the frequency settings.
   if ( m_playerState == NOT_OPENED ) {
      // Look in the clips and see if there is any samplerate etc.
      // It will be set forever after this.
      for ( TClips::const_iterator it = clips.begin();
            it != clips.end();
            ++it ) {
         if ( ! (*it)->isConverted() ) continue;
         int sampleRate = (*it)->sampleRate();
         int numChannels = (*it)->numChannels();
         if ( sampleRate != -1 &&
              numChannels != -1 ) {
            setAudioProperties( numChannels, sampleRate );
            break;
         }
      }
      iBufSet.Query();    // Initialize to "Unknown"
      iBufSet.iChannels   = m_wantedNbrChannels;
      iBufSet.iSampleRate = m_wantedSampleRate;
      iBufSet.iVolume     = iBufSet.iMaxVolume;

#ifdef NAV2_CLIENT_UIQ3
      // Cannot be done after the player is opened, it seems.
      m_player->SetAudioPropertiesL( m_wantedSampleRate,
                                     m_wantedNbrChannels );
      m_player->SetBalanceL( KMMFBalanceCenter );
      TFourCC fourCC = TFourCC(KMMFFourCCCodePCM16);
      m_player->SetDataTypeL( fourCC );
#endif

      m_playerState = OPENING;
      m_player->Open( &iBufSet );
   }
   
   m_clips = &clips;
   m_requestActive = true;
   
   switch ( m_playerState ) {
      case OPENED:
      case PLAYING:        // Not possible
         playSounds();
         break;
      case OPENING:
      case PLAYING_WANTED: // Not possible
         m_playerState = PLAYING_WANTED;
         break;
      case NOT_OPENED:
         m_player->Open(&iBufSet);
         m_playerState = PLAYING_WANTED;
         break;
   }
}

void
NewAudioPlayer::stop()
{
   #ifdef USE_TRACE
      TRACE_FUNC();
   #endif
   if ( m_stopping ) {
      MC2_ASSERT(m_requestActive);
      return;
   }
   if ( m_timer->IsActive() ) {
      m_timer->Cancel();
   }
   if ( m_player == NULL ) {
      completeRequest( KErrNone );
      return;
   }
   // Always stop the player, even if it isn't playing
   //m_requestActive = true;
   MC2_ASSERT(m_requestActive);

   if ( m_requestActive ) {
      m_stopping = true;
      m_player->Stop();
   } else {
      completeRequest( KErrNone );
   }
}

void
NewAudioPlayer::MaoscBufferCopied(TInt aError,
                                  const TDesC8& /*aBuffer*/)
{
   m_nbrWrittenBuffers--;
   if ( aError == KErrNone ) {
      if ( *m_bufIterator != NULL ) {
         TRAPD( trapRes, m_player->WriteL( **m_bufIterator ) );      
         if ( trapRes == KErrNone ) {
            ++m_bufIterator;
            ++m_nbrWrittenBuffers;
         }
         else{
#ifdef USE_TRACE
            char* errStr = new char[32];
            sprintf(errStr, "Trap error#: %i", trapRes);
            TRACE_FUNC1( errStr );
            delete errStr;
#endif
         }
      } else {
         while ( m_silenceToWrite > 0 ) {
            --m_silenceToWrite;
            TRAPD( trapRes, m_player->WriteL( m_silence ) );
            if ( trapRes == KErrNone ) {
               ++m_nbrWrittenBuffers;
            }
            else{
#ifdef USE_TRACE
               char* errStr = new char[32];
               sprintf(errStr, "Trap error#: %i", trapRes);
               TRACE_FUNC1( errStr );
               delete errStr;
#endif
            }
         }
      }
   }
   else if( aError == KErrAbort ){
#ifdef USE_TRACE
      TRACE_FUNC1( "aError == KErrAbort" );
#endif
   }
   else{
#ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "aError#: %i", aError);
      TRACE_FUNC1( errStr );
      delete errStr;
#endif
   }
   // This is what Stop will call according to the docu for 6.1 and 7.x
   // It is called when the buffer is not fully copied.
   // Seems like MaoscPlayComplete is only called when stopping in 7.x
   // The documentation says that MaoscBufferCopied is called for each
   // buffer sent in, even if stopping so I removed the check for error code
#ifndef TRUST_PLAY_COMPLETE
   if ( ( m_nbrWrittenBuffers == 0 ) && !m_stopping ) {
      m_playerState = OPENED;
# if defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_SERIES60_V3
      // Seems like you cannot stop these phones from within a callback.
      m_timer->delayedStop();
# else
      // Series 80/90
      stop();
# endif
      return;
   }
#else
   aError = aError; // To remove warnings
#endif

#if defined NAV2_CLIENT_UIQ
   //XXX This is done by delayedStop() instead! XXX
   /*
   if ( iIsA925 && ( m_nbrWrittenBuffers == 0 ) && !m_stopping ) {
      sleep(1);
      MaoscPlayComplete( KErrNone );
      return;
   }
   */
#endif
}

TInt
NewAudioPlayer::restartPlayer()
{
   #ifdef USE_TRACE
      TRACE_FUNC();
   #endif

   // Cancel the stop callback since it will stop the wrong player
   // otherwise (if it is active). 
   if (m_timer && m_timer->IsActive() ) {
      TRACE_FUNC();
      m_timer->Cancel();
   }

   // Delete the player in another active object's context, since we
   // can be in the m_player context now.
   m_playerDeleter->addPlayerToDelete( m_player );
   m_player = NULL;
   m_playerState = NOT_OPENED;
   TRAPD( res, m_player = CMdaAudioOutputStream::NewL(*this) );
   if ( res == KErrNone ) {
#if !defined NAV2_CLIENT_UIQ
      m_player->SetPriority( EMdaPriorityMin, 
                             EMdaPriorityPreferenceTimeAndQuality );
#endif
   }
   else{
    #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "Trap error#: %i", res);
      TRACE_FUNC1( errStr );
      delete errStr;
    #endif
   }
   return res;
}

void 
NewAudioPlayer::MaoscPlayComplete(TInt aError)
{
   // This is what Stop will call according to the docu for 7.x
   // Go back to the OPENED state.
   m_playerState = OPENED;
   if ( aError == KErrUnderflow ) {
      // Note: MMdaAudioOutputStreamCallback::MaoscPlayComplete()
      // may also be called if there is no more audio data to play.
      // In such circumstances the audio stream is automatically
      // closed and aError of the callback is set to KErrUnderFlow.
    #ifdef USE_TRACE
      TRACE_FUNC1( "aError == KErrUnderflow" );
    #endif
      aError = KErrNone;
#ifndef TRUST_PLAY_COMPLETE
      return;
#endif
   }
   if ( aError != KErrNone ) {
   
      if( aError == KRequestPending ){
         // The thread will die in UIQ
       #ifdef USE_TRACE
         TRACE_FUNC1( "aError == KRequestPending" );
       #endif
         return;
      }
 
      if( aError == 4761 ){
         // This is the error we get in the motorola phones,
         // that causes the thread to die.
       #ifdef USE_TRACE
         TRACE_FUNC1( "aError == 4761" );
       #endif
         return;
      }

      if( aError == KErrInUse ){
       #ifdef USE_TRACE
         TRACE_FUNC1( "aError == KErrInUse" );
       #endif
      }
      else if( aError == KErrCancel ){
       #ifdef USE_TRACE
         TRACE_FUNC1( "aError == KErrCancel" );
       #endif
         restartPlayer();
      }
      else if ( aError == KErrDied ){
       #ifdef USE_TRACE
         TRACE_FUNC1( "aError == KErrDied" );
       #endif
         restartPlayer();
      }
      else if( aError == KErrAbort ) {
       #ifdef USE_TRACE
         TRACE_FUNC1( "aError == KErrAbort" );
       #endif
         restartPlayer();
      }
      else if( aError == KErrServerTerminated ){
       #ifdef USE_TRACE
         TRACE_FUNC1( "aError == KErrServerTerminated" );
       #endif
         restartPlayer();
      }
      else if( aError == KErrAccessDenied ){
         // This seems to happen when S60V3 receives SMS.
         // Restarting the player helped. Crash occured otherwise.
#ifdef USE_TRACE
         TRACE_FUNC1( "aError == KErrAccessDenied" );
#endif
         restartPlayer();
      }
      else{
       #ifdef USE_TRACE
         char* errStr = new char[32];
         sprintf( errStr, "Error#: %i", aError );
         TRACE_FUNC1(errStr);
         delete errStr;
       #endif
      }
      completeRequest( aError );
      return;
   }
   
   // XXX is there any point in setting the volume here?
   //setVolumePercent( m_wantedVolumePercent );

#ifdef REOPEN_PLAYER_AFTER_EACH_SOUND   
   // On Series 60 v1, the frequency can be changed
   // (to 800Hz) in the player if it is not reopened
   // after each underflow.
   // Re-open player and complete request.
   if ( m_wantedSampleRate != TMdaAudioDataSettings::ESampleRate8000Hz ) {
      restartPlayer();
   }
#endif

   if ( m_nbrWrittenBuffers == 0 ) {
      completeRequest( aError );
   }
   // Make it possible to get the request completed with
   // MaoscBufferCopied. Don't know the order in which those
   // are called.
   m_stopping = false;
}

