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

#ifndef NEWAUDIOSERVERTYPES_H
#define NEWAUDIOSERVERTYPES_H

#include "config.h"
// #include<e32std.h>
#include<vector>
// #include<badesca.h>

namespace NewAudioServerTypes {

   enum TRequestTypes {
      CancelAnyRequest,
      PrepareSoundRequest,
      PlaySoundRequest,
      VolumeRequest,
   };

   /// Special sound for timing
   _LIT(KSoundTiming, "XXXSoundTiming");
   /// Special sound for the last clip
   _LIT(KSoundEnd,    "XXXSoundEnd");
}
/// Callback class for the audio player
class NewAudioPlayerListener {
public:
   virtual void playCompleteL(TInt aErr) = 0;
};

/// Callback for the audio converter class.
class NewAudioConverterListener {
public:
   virtual void conversionCompleteL(TInt aErr, long durationMS ) = 0;
};


/**
 *   Class containing info about a clip and the converted clip.
 *   FIXME: Leave-safety.
 */
class ClipBuf {
public:
   
   void initEmpty( const TDesC& fileName ); 
   
   ClipBuf(const TDesC& fileName );
   
   /// Copy constructor
   ClipBuf(const ClipBuf& other);

   /// Sets number of channels and samplerate in Hz
   void setAudioProperties( int numchannels,
                            int samplerate ) ;

   void handoverAudioData( TDes8& audioData,
                           const TTimeIntervalMicroSeconds& timeMicro ) ;   
   void setAudioData(const TDesC8& audioData,
                     const TTimeIntervalMicroSeconds& timeMicro ) ;
   int sampleRate() const ;
   int numChannels() const ;
   
   bool isConverted() const;

   int getNbrBuffers() const;
   
   const TPtrC8& getBuffer(int i );
   
   const TDesC& getFileName() const;
   
   ~ClipBuf();

private:
   /// Pointer to the data.
   TPtrC8 m_des;
   /// Buffer where the data is stored
   TUint8* m_realBuf;
   /// File name of the clip
   HBufC* m_fileName;
public:
   /// Duration of the clip
   class TTimeIntervalMicroSeconds m_time;
   /// Pointers to different parts of the buffers
   class CPtrC8Array* m_array;
   /// Number of channels
   int m_numChannels;
   /// Sample rate in Hz
   int m_sampleRate;
};

typedef std::vector<ClipBuf*> TClips;

class ClipBufIterator {
public:

   ClipBufIterator();
   
   ClipBufIterator(const TClips& clips);

   ClipBufIterator& operator++();
   
   const TDesC8* operator*() const;
private:
   /// Empty vector to use when initializing empty. Remove later
   TClips m_emptyClips;
   const TClips* m_clips;
   TClips::const_iterator m_clipIt;
   int m_bufNbr;
};

#endif
