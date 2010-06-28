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

#ifndef OGGDEBOCDER_HG
#define OGGDEBOCDER_HG

#include <e32def.h>

// SEP_HEAP is defined in os_types.h which is indir included by ivorbisfile
#include "ivorbisfile.h"
#include <f32file.h>

// For size_t etc.
#include <stdlib.h>

class ClipBuf;

/**
 *   Class that will be able to decode ogg files.
 */
class OggDecoder : public CBase {
public:
   /**
    *   Doesn't do much.
    */
   OggDecoder();

   /**
    *   Symbian second phase constructor.
    */
   void ConstructL();

   /**
    *   Dr. Destructo
    */
   ~OggDecoder();
      
   /**
    *   Converts the specified file to PCM or leaves.
    */
   void ConvertL( const TDesC& fileName );

   /// Returns number of channels
   int numChannels() const;
   /// Return sample rate in Hz
   int sampleRate() const;
   /// Return the duration in unknown unit
   int duration() const;
   
   /**
    *   Puts the converted data into the buffer buf.
    */
   void putDataL( ClipBuf& buf );
   
   /**
    *   Closes the file etc.
    */
   void Close();

private:

   /// Downsamples the buffer by half.
   char* downSample( char* oldBuf, int& written );
   
   /// Resets ogg vorbis
   void ResetL();
   
   /// Callback for libvorbisfile
   size_t read_func(void* ptr, size_t size, size_t nitems);
   /// Callback for libvorbisfile
   int seek_func(ogg_int64_t offset, int whence);
   /// Callback for libvorbisfile
   int close_func();
   /// Callback for libvorbisfile
   long tell_func();
   
   /// Callback for libvorbisfile
   static size_t ovf_read_func(void* ptr, size_t size,
                                      size_t nitems, void* datasource);
   
   /// Callback for libvorbisfile
   static int ovf_seek_func(void* datasource, ogg_int64_t offset,
                                   int whence);

   /// Callback for libvorbisfile
   static int ovf_close_func(void* datasource);

   /// Callback for libvorbisfile
   static long ovf_tell_func(void* datasource);

   /// File server session
   RFs m_fileSession;
   /// File to read from
   RFile m_file;
   /// True if the file is opened.
   int m_fileOpened;
   /// Ogg vorbis file object (struct)
   OggVorbis_File m_oggFile;

   /// Callbacks to give to the ogg lib.
   static const ov_callbacks KOggVorbisCallbacks;

   /// Something.
   TInt m_oggSection;

#ifdef SEP_HEAP
   RHeap* iDefaultHeap;
   RHeap* iVorbisHeap;
   RChunk iVorbisChunk;
#endif
   
};

#endif // OGGDEBOCDER_HG
