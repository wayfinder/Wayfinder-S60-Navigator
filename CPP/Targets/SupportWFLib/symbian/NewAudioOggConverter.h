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

#ifndef NEWAUSDFADIO_COGO_CONVERHER_H
#define NEWAUSDFADIO_COGO_CONVERHER_H

#include "NewAudioConverter.h"

class NewAudioOggConverter : public NewAudioConverter {
public:

   /// Creates new NewAudioOggConverter with supplied listener
   NewAudioOggConverter( NewAudioConverterListener& listener );
   
   /// Second stage construction
   void ConstructL( );

   /// As ConstructL, but also new:s the converter
   static NewAudioOggConverter* NewL( NewAudioConverterListener& listener );

   /// As NewL but also pushes the converter onto the CleanupStack
   static NewAudioOggConverter* NewLC( NewAudioConverterListener& listener );

   /// Converts the clips in the list from ogg to pcm
   void convertL( TClips& clips );

   /// Stops conversion as soon as possible. Calls the callback.
   void stop();
   
private:

   /// The listener to tell about when the conversion is done
   NewAudioConverterListener& m_listener;                      
};

#endif
