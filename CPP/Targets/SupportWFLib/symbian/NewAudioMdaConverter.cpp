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

#include "NewAudioMdaConverter.h"

#if defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
#define UIQ_OR_SERIES_60_V1
#endif

NewAudioMdaConverter::
NewAudioMdaConverter( NewAudioConverterListener& listener ) :
      m_unpackBuffer( NULL, 0, 0 ),
      m_listener(listener )
{
   m_clips         = NULL;
   m_requestActive = false;
   m_cancelling    = false;
   m_converter     = NULL;
   m_unpackBuffer.Set( reinterpret_cast<TUint8*>(m_realUnpackBuf),
                       0, c_unPackBufSize );
   iNextClipLocation = NULL;
}

NewAudioMdaConverter::
~NewAudioMdaConverter()
{
   delete m_converter;
   delete iNextClipLocation;
   completeRequest( KErrCancel );
}

void
NewAudioMdaConverter::ConstructL()
{
   m_converter =
      CMdaAudioConvertUtility::NewL(*this,
                                    NULL,
                                    EMdaPriorityNormal,
                                    EMdaPriorityPreferenceTimeAndQuality);
}

void
NewAudioMdaConverter::completeRequest( int errCode )
{
   if ( m_requestActive ) {
      m_requestActive = false;
      m_listener.conversionCompleteL( errCode, getDurationMs() );
   }
}

NewAudioMdaConverter*
NewAudioMdaConverter::NewLC( NewAudioConverterListener& listener )
{
   NewAudioMdaConverter* tmp = new (ELeave) NewAudioMdaConverter(listener);
   tmp->ConstructL();
   CleanupStack::PushL( tmp );
   return tmp;
}

NewAudioMdaConverter*
NewAudioMdaConverter::NewL( NewAudioConverterListener& listener )
{
   NewAudioMdaConverter* tmp = NewAudioMdaConverter::NewLC(listener);
   CleanupStack::Pop( tmp );
   return tmp;
}

void
NewAudioMdaConverter::convertL( TClips& clips )
{
   resetDuration();
   m_clips = &clips;
   m_cancelling = false;
   m_requestActive = true;
   // Start from the top
   m_nextLoadClipIt = m_clips->begin();
   OpenNextSound();
}

void
NewAudioMdaConverter::stop()
{
   if ( m_requestActive ) {
      m_cancelling = true;
   } else {
      completeRequest( KErrCancel );
   }
}

void
NewAudioMdaConverter::OpenNextSound()
{
   while ( m_nextLoadClipIt != m_clips->end() &&
           (*m_nextLoadClipIt)->isConverted() ) {
      // Skip already converted clips.
      ++m_nextLoadClipIt;
   }

   TInt result = KErrNone;

   do {
      if ( m_nextLoadClipIt == m_clips->end() ||
           (*m_nextLoadClipIt)->getFileName() ==
           NewAudioServerTypes::KSoundEnd ) {
         // Set sound length to whole sound.
         setSoundLength( m_clips->begin(), m_nextLoadClipIt );
         // Play complete
         // iMessage is valid here, see the callers
         completeRequest(KErrNone);
         return;
      }
      if ( (*m_nextLoadClipIt)->getFileName() ==
           NewAudioServerTypes::KSoundTiming ) {
         // Loop through the other sounds and sum up the times.
         setSoundLength( m_clips->begin(), m_nextLoadClipIt );
         // Advance and recurse.
         ++m_nextLoadClipIt;
         OpenNextSound();
         return;
      }
      // Reset the buffer so that the data will be at the start of it.
      m_unpackBuffer.SetLength(0);

      // Make sure the new clip doesn't get the same pointer.
      // It doesn't work if I do not new it here. Don't know
      // if the converter recognizes the pointer or if the
      // copy constructor only works once.
      {
         TMdaDesClipLocation* oldClip = iNextClipLocation;
         iNextClipLocation = new (ELeave) TMdaDesClipLocation( m_unpackBuffer ); // FIXME note leaving function
         delete oldClip;
      }
      TMdaAudioDataSettings* ptr = NULL;
      
      // Just in case the converter was busy. Should not happen.
      m_converter->Close();
      // Convert from the file of the next clip to the unpack buffer.
      TRAP(result,
           m_converter->OpenL( (*m_nextLoadClipIt)->getFileName(), 
                              iNextClipLocation,
                              &iBufFormat,
                              &iBufCodec,
                              ptr ) );     

      // The result will come back in MoscoStateChangeEvent().
      if ( result == KErrPathNotFound ) {
         // Move forward to avoid looping.
         ++m_nextLoadClipIt;
      }
   } while (result == KErrPathNotFound);

   // Either loaded a sound or failed for a non-trivial reason.
   if (result != KErrNone && m_requestActive ) {
      completeRequest( result );
   }
}

void
NewAudioMdaConverter::MoscoStateChangeEvent( CBase* /*aObject*/, 
                                             TInt aPreviousState, 
                                             TInt aCurrentState, 
                                             TInt aErrorCode)
{
   if (! m_requestActive ) {
      // A state change not associated with any request. 
      // Ignore it.
      return;
   }
   if ( m_cancelling || ( aErrorCode != KErrNone ) ) {
      m_converter->Close();
      if ( m_cancelling ) {
         completeRequest(KErrCancel);
      } else {
         completeRequest(aErrorCode);
      }
      m_cancelling = false;
      return;
   }
   if ( (aCurrentState == CMdaAudioClipUtility::EPlaying) ||
        (aCurrentState == CMdaAudioClipUtility::ERecording) ) {
      // Probably just an Open->Converting transition.
      // Ignore it.
      return;
   }


   if ( (aPreviousState == CMdaAudioClipUtility::ENotReady) && 
        (aCurrentState == CMdaAudioClipUtility::EOpen) ) {
      // A sound file was just opened. 
      // Start converting the file
      TInt result = KErrNone;
#ifdef UIQ_OR_SERIES_60_V1
      // Old interface. (Version 6.1). Don't know how valid
      // the NAV2_CLIENT_UIQ is. Could be different for different
      // versions of UIQ.
      // Problem here is that PlayL leaves with KErrNotReady.
      TRAP( result, m_converter->PlayL() );
#else            
      TRAP( result, m_converter->ConvertL() );
      
#endif
      if ( result != KErrNone && m_requestActive ) {
         m_converter->Close();
         completeRequest( result );
      }
   } else if ( (aPreviousState == CMdaAudioClipUtility::EPlaying) && 
               (aCurrentState == CMdaAudioClipUtility::EOpen) ) {
      // Conversion completed
      // Get the size, create a buffer and copy the result
      (*m_nextLoadClipIt)->setAudioData( m_unpackBuffer,
                                         m_converter->Duration() );
      ++m_nextLoadClipIt;
      // Take the next sound and convert it.
      OpenNextSound();
   }    
}
