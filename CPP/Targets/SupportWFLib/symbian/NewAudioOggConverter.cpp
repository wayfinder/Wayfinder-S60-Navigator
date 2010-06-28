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

#include "NewAudioOggConverter.h"
#include "NewAudio.h"
#include "OggDecoder.h"

NewAudioOggConverter::
NewAudioOggConverter( NewAudioConverterListener& listener)
      : m_listener(listener) {
}

void
NewAudioOggConverter::ConstructL()
{
}

NewAudioOggConverter*
NewAudioOggConverter::NewLC( NewAudioConverterListener& listener )
{
   NewAudioOggConverter* tmp = new (ELeave)NewAudioOggConverter(listener);
   CleanupStack::PushL( tmp );
   tmp->ConstructL();
   return tmp;
}

NewAudioOggConverter*
NewAudioOggConverter::NewL( NewAudioConverterListener& listener )
{
   NewAudioOggConverter* tmp = NewAudioOggConverter::NewLC(listener);
   CleanupStack::Pop( tmp );
   return tmp;
}

void
NewAudioOggConverter::convertL( TClips& clips )
{
#ifdef LOWER_THREAD_PRIO_ON_CONVERT
   {
      class RThread thisThread; // Not pushed on the cleanup stack
      thisThread.SetPriority(EPriorityNormal);
   }
#endif
   resetDuration();
   for ( TClips::iterator it = clips.begin();
         it != clips.end();
         ++it ) {
      if ( (*it)->isConverted() ) {
         // Already converted.
         continue;
      }
      if ( (*it)->getFileName() != NewAudioServerTypes::KSoundEnd &&
           (*it)->getFileName() != NewAudioServerTypes::KSoundTiming ) {
         // Create new decoder
         OggDecoder* oggDec = new (ELeave) OggDecoder();
         CleanupStack::PushL( oggDec );
         oggDec->ConstructL();
         TRAPD( res, oggDec->ConvertL( (*it)->getFileName() ) );
         if ( res == KErrNone ) {
            oggDec->putDataL( **it );
            (*it)->setAudioProperties( oggDec->numChannels(),
                                       oggDec->sampleRate() );
         }         
         CleanupStack::PopAndDestroy( oggDec );
      } else {
         // Will only set the sound length if not already set.
         setSoundLength( clips.begin(), it );
      }
      if ( (*it)->getFileName() == NewAudioServerTypes::KSoundEnd ) {
         // Get out of ze lup.
         break;
      }
   }
   // Will only set the sound length if not already set.
   setSoundLength( clips.begin(), clips.end() );
   m_listener.conversionCompleteL( KErrNone, getDurationMs() );
}

void
NewAudioOggConverter::stop()
{
   // Call the callback with no sound length.
   m_listener.conversionCompleteL( KErrCancel, 0 );
}
