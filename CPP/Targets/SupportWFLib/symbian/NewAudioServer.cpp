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

#include <e32def.h>
#include <e32std.h>
#include <algorithm>
#include <mda/common/audio.h>

#include "NewAudioServerTypes.h"
#include "NewAudioServer.h"
#include "NewAudioOggConverter.h"
#include "NewAudioMdaConverter.h"
#include "nav2util.h"

#include "config.h"

using namespace std;

// #ifndef USE_TRACE
//  #define USE_TRACE
// #endif

#undef USE_TRACE

#ifdef USE_TRACE
#include "TraceMacros.h"
#endif

/**************************************
 * CNewAudioServer
 **************************************/
namespace {
   _LIT(KSemicolon,   ";" );
}

const TDesC&
CNewAudioServer::getTimingMarker()
{
   return NewAudioServerTypes::KSoundTiming;
}


const TDesC&
CNewAudioServer::getEndMarker()
{
   return NewAudioServerTypes::KSoundEnd;
}



CNewAudioServer * CNewAudioServer::NewLC()
{
   CNewAudioServer *ptr = new (ELeave) CNewAudioServer();
   CleanupStack::PushL(ptr);
   ptr->ConstructL();
   return ptr;
}

CNewAudioServer::CNewAudioServer()
 #if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   : CServer2(EPriorityLow, ESharableSessions),
 #else
   : CServer(EPriorityLow, ESharableSessions),
 #endif
     iPlayerControl(NULL)
{
}

void CNewAudioServer::ConstructL()
{
   iPlayerControl = CPlayerControl::NewLC();
   CleanupStack::Pop( iPlayerControl );
}

CNewAudioServer::~CNewAudioServer()
{
   delete iPlayerControl;
}

#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
CSession2* CNewAudioServer::NewSessionL(const TVersion& /*aVersion*/, const RMessage2& /*aMessage*/) const
{
   // Ignore the version for now.
   CNewAudioSession * sess = new (ELeave) CNewAudioSession(*iPlayerControl);
   return sess;
}
#else
CSharableSession* CNewAudioServer::NewSessionL(const TVersion& /*aVersion*/) const
{
   // Ignore the version for now.
   CNewAudioSession * sess = new (ELeave) CNewAudioSession(*iPlayerControl);
   return sess;
}
#endif

void CNewAudioServer::StopServerL()
{
   iPlayerControl->StopServerL();
}

/**************************************
 * CNewAudioSession
 **************************************/

CNewAudioServer::
CNewAudioSession::CNewAudioSession(CPlayerControl & aPlayerControl)
   : iPlayerControl(aPlayerControl)
{
}

CNewAudioServer::CNewAudioSession::~CNewAudioSession()
{
}

#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
void CNewAudioServer::CNewAudioSession::ServiceL(const RMessage2& aMessage)
{
   iPlayerControl.HandleRequest(aMessage);
}
#else
void CNewAudioServer::CNewAudioSession::ServiceL(const RMessage& aMessage)
{
   iPlayerControl.HandleRequest(aMessage);
}
#endif




/**************************************
 * CPlayerControl
 **************************************/

CNewAudioServer::CPlayerControl * CNewAudioServer::CPlayerControl::NewLC()
{   
   CPlayerControl * ptr = new(ELeave) CPlayerControl();
   CleanupStack::PushL( ptr );
   ptr->ConstructL();   
   return ptr;
}

CNewAudioServer::CPlayerControl::CPlayerControl()
{
   m_requestActive = false;
   iStopping = false;
   m_audioPlayer = NULL;
   m_audioConverter = NULL;      
}

void CNewAudioServer::CPlayerControl::ConstructL()
{
#define USE_OGG
#ifdef USE_OGG
   NewAudioConverter* audioConverter = NewAudioOggConverter::NewLC( *this );
#else
   NewAudioConverter* audioConverter = NewAudioMdaConverter::NewLC( *this );
#endif
   NewAudioPlayer* audioPlayer = new (ELeave) NewAudioPlayer(*this);
   CleanupStack::PushL( audioPlayer );
   audioPlayer->ConstructL();

   // Set our variables. Nothing can leave here.
   m_audioPlayer    = audioPlayer;
   m_audioConverter = audioConverter;

   // And pop.
   CleanupStack::Pop( m_audioPlayer );
   CleanupStack::Pop( m_audioConverter );
}

void
CNewAudioServer::CPlayerControl::completeRequest( TInt aError )
{
   if( aError != KErrNone ){
    #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf( errStr, "Error#: %i", aError );
      TRACE_FUNC1( errStr );
      delete errStr;
    #endif
   }
   MC2_ASSERT ( m_requestActive );
   if ( m_requestActive ) {
      m_requestActive = false;
      iMessage.Complete( aError );
   }
}

void
CNewAudioServer::CPlayerControl::playCompleteL( TInt aError )
{
   if( aError == KErrNone ){
   }
   else if( aError == KErrCancel ){
    #ifdef USE_TRACE
      TRACE_FUNC1( "aError == KErrCancel" );
    #endif
   }
   else{
    #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf( errStr, "Error#: %i", aError );
      TRACE_FUNC1( errStr );
      delete errStr;
    #endif
   }
   completeRequest( aError );

   
   // The following is done inside the player for Nokia phones.
   // It should be done inside, really.
#if defined NAV2_CLIENT_UIQ //|| defined NAV2_CLIENT_UIQ3
   // XXX !!! THIS CRASHES THE AUDIOTHREAD ON S60 AND
   // CANNOT BE USED !!!
   // !!! THIS CRASHES THE AUDIOTHREAD ON UIQ phones as well AND
   // CANNOT BE USED !!! XXX
   // XXX The audio player should be stopped by the 
   // TRUST_PLAY_COMPLETE define instead. XXX
   //Need to call stop, or the player will keep the
   //radio muted in motorola car-kits.
   //m_audioPlayer->stop();
#endif
   
   if ( iStopping ) {
      delete m_audioPlayer;
      m_audioPlayer = NULL;
      delete m_audioConverter;
      m_audioConverter = NULL;
      CActiveScheduler::Current()->Stop();
   }
}

void
CNewAudioServer::CPlayerControl::conversionCompleteL( TInt aError,
                                                      long durationMS )
{
   // Add the silence that the player adds before the sound.
   if ( durationMS ) {
#if 1
      long audioPlayerSilence = m_audioPlayer->getDurationOfSilenceBefore();
#else
      long audioPlayerSilence = 0;
#endif
      StoreSoundLengthReply( durationMS + audioPlayerSilence );
                             
   } else {
      StoreSoundLengthReply( 0 );
   }
   completeRequest( aError );
   if ( iStopping ) {
      delete m_audioPlayer;
      m_audioPlayer = NULL;
      delete m_audioConverter;
      m_audioConverter = NULL;
      CActiveScheduler::Current()->Stop();
   }   
}

void
CNewAudioServer::CPlayerControl::StoreSoundLengthReply(TInt aSoundLength)
{
   TPtr8 msPtr( NULL, 0, 0 );
   msPtr.Set( (TUint8*) &aSoundLength,
              sizeof(aSoundLength), sizeof(aSoundLength) );
 #if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   iMessage.WriteL(2, msPtr);
 #else
   iMessage.WriteL(iMessage.Ptr2(), msPtr);
 #endif
}


CNewAudioServer::CPlayerControl::~CPlayerControl()
{
   // Check if there is audio being prepared or played
   // FIXME - check how?
   delete m_audioPlayer;
   delete m_audioConverter;
   
   // Check if there is an outstanding request from a
   // client. This test _should_ give identical results
   // as the one above.
   if ( m_requestActive ) {
      if (iMessage.Function() == NewAudioServerTypes::PrepareSoundRequest) {
         StoreSoundLengthReply(0);
      }
      completeRequest( KErrCancel );
   }
}

TClips::iterator
CNewAudioServer::CPlayerControl::
findClip( TClips::iterator begin,
          TClips::iterator end,
          const TDesC& filename )
{
   for( TClips::iterator it = begin;
        it != end;
        ++it ) {
      if ( *it != NULL ) {
         if ( (*it)->getFileName() == filename ) {
            return it;
         }
      }
   }
   return end;
}

void
CNewAudioServer::
CPlayerControl::prepClips( const MDesCArray& filenames )
{
   // Create new vector of correct size to hold the new clips
   const int nbrNewClips = filenames.MdcaCount();
   TClips newClips( nbrNewClips );

   // Look for clips in old vector and swap them into the new one.
   {
      for ( int i = nbrNewClips-1; i >= 0; --i ) {
         TClips::iterator it = findClip( m_clips.begin(),
                                         m_clips.end(),
                                         filenames.MdcaPoint(i) );
         if ( it != m_clips.end() ) {
            // Found old clip like new clip -> swap them
            std::swap( *it, newClips[i] );
         }
      }
   }
   
   // We won't be using the old clips anymore, so delete them.
   for_each( m_clips.begin(),
             m_clips.end(),
             isab::Delete<TClips::value_type>() );
   m_clips.clear();

   // Look for duplicates and copy them. Else init empty.
   {
      int i = 0;
      for ( TClips::iterator it = newClips.begin();
            it != newClips.end();
            ++it, ++i ) {
         if ( *it != NULL ) {
            // Don't look for stuff already filled in.
            continue;
         }
         // Look from the next clip and forward.
         TClips::iterator startit = it;
         TClips::iterator findit = findClip( ++startit,
                                             newClips.end(),
                                             filenames.MdcaPoint(i) );
         if ( findit != newClips.end() ) {
            // Create copy of the old buffer.
            *it = new ClipBuf( **findit );
         } else {
            // Create empty ClipBuf to be loaded later.
            *it = new ClipBuf( filenames.MdcaPoint(i) );
         }
      }      
   }
   // Put in the new clips. Hope this compiles correctly for ARM.
   m_clips.swap( newClips );   
}

void
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
CNewAudioServer::CPlayerControl::decodeAndPrepClipsL(const RMessage2& aMessage)
#else
CNewAudioServer::CPlayerControl::decodeAndPrepClipsL(const RMessage& aMessage)
#endif
{
   // The clip file names are sent in separated by semicolons
   CPtrCArray * clips = new(ELeave) CPtrCArray(10);
   CleanupStack::PushL(clips);
   int msgLen = aMessage.Int0();
   HBufC * msgbuf = HBufC::NewLC(msgLen);
   TPtr tmp(NULL, 0);
   tmp.Set( msgbuf->Des() );
   // This can leave. Then we will have the old clips again.
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   aMessage.ReadL( 1, tmp );
#else
   aMessage.ReadL(aMessage.Ptr1(), tmp );
#endif
   
   TPtrC rest;
   rest.Set(*msgbuf);
   
   while (rest.Length() > 0) {
      TInt semiPos = rest.Find( KSemicolon );
      if (semiPos == KErrNotFound) {
         // No more semocolons - ignore the rest
         break;
      }
      // This is the clip name - push it on clips!!!
      TPtrC tmp;
      tmp.Set( rest.Left(semiPos) );
      if ( tmp == NewAudioServerTypes::KSoundEnd ) {
         // We have no use for the end sound and the sounds after that.
         break;
      }
      clips->AppendL( tmp );
      if ( (semiPos+1) >= rest.Length() ) {
         break;
      }
      rest.Set( rest.Mid(semiPos+1) );
   }
   prepClips( *clips );
   CleanupStack::PopAndDestroy(msgbuf);
   CleanupStack::PopAndDestroy(clips);
}


#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
void CNewAudioServer::CPlayerControl::HandleRequest( const RMessage2 & aMessage)
#else
void CNewAudioServer::CPlayerControl::HandleRequest( const RMessage & aMessage)
#endif
{
 #ifdef USE_TRACE
   TRACE_FUNC();
 #endif
   // Volume request is always OK
   if ( aMessage.Function() == NewAudioServerTypes::VolumeRequest ) {
    #ifdef USE_TRACE
      TRACE_FUNC1( "NewAudioServerTypes::VolumeRequest" );
    #endif
      int volume = aMessage.Int0();
      m_audioPlayer->setVolumePercent( volume );
      aMessage.Complete( KErrNone );
      return;
   }

   // Not a volume request.
   if ( m_requestActive ) {
      // Already busy, only service cancelation messages
      if (aMessage.Function() == NewAudioServerTypes::CancelAnyRequest) {
       #ifdef USE_TRACE
         TRACE_FUNC1( "NewAudioServerTypes::CancelAnyRequest" );
       #endif
         switch (iMessage.Function()) {
            case NewAudioServerTypes::PrepareSoundRequest:
               m_audioConverter->stop();
               break;
            case NewAudioServerTypes::PlaySoundRequest:
               // Should complete the message in MaoscBufferCopied
               // or MaoscPlayComplete
               m_audioPlayer->stop();
               break;
            case NewAudioServerTypes::CancelAnyRequest:
               // This should not happen. Ignore it.
               // There should never be a CancelAnyRequest in 
               // iMessage.
               break;
            default:
               completeRequest( KErrCancel );
               break;
         }
         aMessage.Complete(KErrCancel);
         return;
      }

      // Not a cancelation event, return busy
      aMessage.Complete(KErrServerBusy);
      return;
   } else {
      if (aMessage.Function() == NewAudioServerTypes::CancelAnyRequest) {
       #ifdef USE_TRACE
         TRACE_FUNC1( "NewAudioServerTypes::CancelAnyRequest" );
       #endif
         aMessage.Complete(KErrCancel);               
         return;
      }
   }

   // Not servicing a prior request, process the new request.
   iMessage = aMessage;
   m_requestActive = true;
   switch (iMessage.Function()) {
      case NewAudioServerTypes::CancelAnyRequest:
         // Already handled above
       #ifdef USE_TRACE
         TRACE_FUNC1( "NewAudioServerTypes::CancelAnyRequest" );
       #endif
         break;
      case NewAudioServerTypes::PrepareSoundRequest:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1( "NewAudioServerTypes::PrepareSoundRequest" );
          #endif
            // Prepare the clip vector. Read file names from other side
            TRAPD( decodeRes, decodeAndPrepClipsL( aMessage ) );
            if ( decodeRes != KErrNone ) {
             #ifdef USE_TRACE
               char* errStr = new char[32];
               sprintf(errStr, "Trap error#: %i", decodeRes);
               TRACE_FUNC1( errStr );
               delete errStr;
             #endif
               completeRequest( decodeRes );
               return;
            }
            // Start converting the sounds
            TRAPD( convertRes, m_audioConverter->convertL( m_clips ));
            if ( convertRes != KErrNone ) {
             #ifdef USE_TRACE
               char* errStr = new char[32];
               sprintf(errStr, "Trap error#: %i", convertRes);
               TRACE_FUNC1( errStr );
               delete errStr;
             #endif
               completeRequest( convertRes );
               return;
            }
            // If using the ogg-converter, the request is already
            // completed now. If using the mda-converter, it will
            // call the callback later.
         }
         break;
      case NewAudioServerTypes::PlaySoundRequest:
         // Play our clips.
       #ifdef USE_TRACE
         TRACE_FUNC1( "NewAudioServerTypes::PlaySoundRequest" );
       #endif
         TRAPD( playRes, m_audioPlayer->playL( m_clips ) );
         if ( playRes != KErrNone ) {
          #ifdef USE_TRACE
            char* errStr = new char[32];
            sprintf(errStr, "Trap error#: %i", playRes);
            TRACE_FUNC1( errStr );
            delete errStr;
          #endif
            completeRequest( playRes );
            return;
         }
         break;
      default:
       #ifdef USE_TRACE
         TRACE_FUNC1( "KErrNotSupported" );
       #endif
         completeRequest( KErrNotSupported );
         break;
   }
}

void CNewAudioServer::CPlayerControl::StopServerL()
{
   if (m_requestActive) {
      switch (iMessage.Function()) {
         case NewAudioServerTypes::PrepareSoundRequest:
            iStopping = true;
            StoreSoundLengthReply(0);
            m_audioConverter->stop();
            return;
         case NewAudioServerTypes::PlaySoundRequest:
            // Will complete the message in MaoscPlayComplete
            iStopping = true;
            m_audioPlayer->stop();
            return;
         case NewAudioServerTypes::CancelAnyRequest:
            // This should not happen. Ignore it.
            // There should never be a CancelAnyRequest in 
            // iMessage.
            return;
         default:
            CActiveScheduler::Current()->Stop();
            completeRequest( KErrCancel );
            return;
      }
   }
   
   // Stop the active scheduler
   CActiveScheduler::Current()->Stop();
}

