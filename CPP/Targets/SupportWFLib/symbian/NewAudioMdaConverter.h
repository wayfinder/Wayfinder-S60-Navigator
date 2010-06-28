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

#ifndef NEWAUSDFADIO_MDA_COGO_CONVERHER_H
#define NEWAUSDFADIO_MDA_COGO_CONVERHER_H

#include <mdaaudiosampleeditor.h>
#include "NewAudioConverter.h"

class NewAudioMdaConverter : public NewAudioConverter,
                             public MMdaObjectStateChangeObserver {
   
public:

   /// Creates new NewAudioOggConverter with supplied listener
   NewAudioMdaConverter( NewAudioConverterListener& listener );

   /// Destructor - deletes the internal converter
   virtual ~NewAudioMdaConverter();
   
   /// Second stage construction
   void ConstructL( );

   /// As ConstructL, but also new:s the converter
   static NewAudioMdaConverter* NewL( NewAudioConverterListener& listener );

   /// As NewL but also pushes the converter onto the CleanupStack
   static NewAudioMdaConverter* NewLC( NewAudioConverterListener& listener );

   /// Converts the clips in the list from ogg to pcm
   void convertL( TClips& clips );

   /// Stops conversion as soon as possible. Calls the callback.
   void stop();

protected:
   // From MMdaObjectStateChangeObserver
   void MoscoStateChangeEvent(CBase* aObject,
                              TInt aPreviousState,
                              TInt aCurrentState,
                              TInt aErrorCode);
   
private:

   /// Opens the next sound or calls the completion function if done
   void OpenNextSound();

   void completeRequest( int errCode );

   /**
    *   Description of where the next clip should be stored.
    *   Seems like it has to be a pointer to work more than once.
    *   (Newed and deleted).
    */
   TMdaDesClipLocation* iNextClipLocation;

   enum {
      /// Size of unpack buffer. Room for the longest sound is enough.
      c_unPackBufSize = 10*16000*2 // 10 seconds at 16 kHz, 16 bit
   };
   
   /// Buffer to unpack the data in
   TUint32 m_realUnpackBuf[ (c_unPackBufSize + 3 ) >> 2 ];
   /// TPtr8 to that will point to the load buffer.
   TPtr8 m_unpackBuffer;

   
   /// The audio converter. Called for each clip in OpenNextSound
   CMdaAudioConvertUtility * m_converter;
   
   /// The listener to tell about when the conversion is done
   NewAudioConverterListener& m_listener;

   /// Out format of conversion
   TMdaRawAudioClipFormat iBufFormat;
   /// The decoder for the out format of the conversion.
   TMdaSL16RawAudioCodec  iBufCodec;   
   /// The next clip to be loaded
   TClips::iterator m_nextLoadClipIt;

   /// Current clips
   TClips* m_clips;
   
   /// True if a request is active
   int m_requestActive;
   /// True if stop has been called
   int m_cancelling;
   
};

#endif
