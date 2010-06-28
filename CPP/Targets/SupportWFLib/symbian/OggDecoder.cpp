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

#include "OggDecoder.h"

#ifdef SEP_HEAP

// Separate heap for Vorbis. SEP_HEAP can be defined in os_types.h
// in the tremor library.
#  define SEP_HEAP_ASSERT(x) MC2_ASSERT(x)
#  define VORBIS_TRAP(err, c) do { User::SwitchHeap(iVorbisHeap); \
                                   User::Check(); \
                                   TRAP(err, c); \
                                   User::SwitchHeap(iDefaultHeap); \
                                   User::Check(); \
                              } while(0)

#  define VORBIS_TRAPD(err, c) TInt err; VORBIS_TRAP(err, c);
#  define SET_V_HEAP() do { User::SwitchHeap( iVorbisHeap ); } while(0)
#  define SET_S_HEAP() do { User::SwitchHeap( iDefaultHeap ); } while(0)
#  define SET_V_HEAP_STATIC(x) do { \
          User::SwitchHeap( static_cast<OggDecoder*>(x)->iVorbisHeap ); } \
          while(0)
#  define SET_S_HEAP_STATIC(x) do { \
          User::SwitchHeap( static_cast<OggDecoder*>(x)->iDefaultHeap ); } \
          while(0)
#else

// No separate heap.

#  define SEP_HEAP_ASSERT(x)
#  define VORBIS_TRAP(err, c) do { err= KErrNone; c; } while (0)
#  define VORBIS_TRAPD(err, c) TInt err = KErrNone; VORBIS_TRAP(err,c);
#  define SET_V_HEAP() 
#  define SET_S_HEAP() 
#  define SET_V_HEAP_STATIC(x) 
#  define SET_S_HEAP_STATIC(x) 
#endif


#include "NewAudioServerTypes.h"

#include <errno.h>

OggDecoder::OggDecoder()
{
#ifdef SEP_HEAP
   iVorbisHeap = NULL;
   iDefaultHeap = &User::Heap();
#endif
   m_fileOpened = false;
   memset( &m_oggFile, 0, sizeof( m_oggFile ) );
}

OggDecoder::~OggDecoder()
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   TRAPD( what_to_do, ResetL() );
   m_fileSession.Close();
#ifdef SEP_HEAP
   // How do I get rid of the heaps?
   if ( iVorbisHeap ) {
      //iVorbisHeap->FreeAll();
      //iVorbisHeap->Close(); // Crash
      iVorbisChunk.Close();
   }
   
   // The RHeap could be allocated inside the chunk?
   //delete iVorbisHeap;
#endif
}

void
OggDecoder::ConstructL()
{
   User::LeaveIfError( m_fileSession.Connect() );
#ifdef SEP_HEAP
#  if 0
//#  ifdef NAV2_CLIENT_SERIES60_V1
   // Create a local heap
   iVorbisHeap = User::ChunkHeap( NULL, 0x1000, 0x20000 );
   User::LeaveIfNull( iVorbisHeap );
#  else
   User::LeaveIfError(
      iVorbisChunk.CreateLocal(0x4000, 0x20000, EOwnerThread));
   iVorbisHeap = User::ChunkHeap(iVorbisChunk, 0x4000);
#  endif
#endif    
   ResetL();
}

void
OggDecoder::ResetL()
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   Close();
   VORBIS_TRAPD(err, ov_clear(&m_oggFile));
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   User::LeaveIfError(err);
}

void
OggDecoder::Close()
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   if ( m_fileOpened ) {
      m_file.Close();
      m_fileOpened = false;
   }   
}

void
OggDecoder::ConvertL( const TDesC& fileName )
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   ResetL();
   User::LeaveIfError( m_file.Open( m_fileSession, fileName, EFileRead ) );
   m_fileOpened = true;
   int err;
   // Switches heap and back again
   VORBIS_TRAPD(err2,
       err = ov_open_callbacks(this,
                               &m_oggFile,
                               NULL, 0, KOggVorbisCallbacks); );
   
   User::LeaveIfError(err2);

   // Now check the other error
   switch (err) {
      case OV_ENOTVORBIS:
      case OV_EVERSION:
      case OV_EBADHEADER:
         User::Leave(KErrCorrupt);
      case OV_EFAULT:
         VORBIS_TRAP(err, ov_clear(&m_oggFile));
         User::LeaveIfError(err);
         User::Leave(KErrGeneral);
   }
   m_oggSection = m_oggFile.current_serialno;

   if ( false ) {
      // Do tricks with comment to see if that helps.
      char **ptr = NULL;
      VORBIS_TRAP(err, ptr = ov_comment(&m_oggFile,-1)->user_comments);
      User::LeaveIfError( err );
      
      vorbis_info *vi;
      VORBIS_TRAP(err, vi=ov_info(&m_oggFile,-1));
      
      User::LeaveIfError( err );
      vi = vi;
      while(*ptr){
//         fprintf(stderr,"%s\n",*ptr);
         ++ptr;
      }
//        fprintf(stderr,
//                "\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
//        fprintf(stderr,"\nDecoded length: %ld samples\n",
//                (long)ov_pcm_total(&vf,-1));
//        fprintf(stderr,"Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
   }

   
}

int
OggDecoder::sampleRate() const
{
#ifdef NAV2_CLIENT_SERIES60_V1
   return m_oggFile.vi->rate / 2;
#else
   return m_oggFile.vi->rate;
#endif
}

int
OggDecoder::numChannels() const
{
   return m_oggFile.vi->channels;
}

int
OggDecoder::duration() const
{
   // this does nothing with the heap according to the example
   OggVorbis_File tmp = m_oggFile;

   int err;
   int32 tmptime = 0;
   VORBIS_TRAP( err, tmptime = int32(ov_time_total(&tmp, 0) ) );
   if ( err == KErrNone ) {
      return tmptime;
   } else {
      return 0;
   }
}

char*
OggDecoder::downSample( char* oldBuf, int& written )
{
   int16* newBuf = reinterpret_cast<int16*>(oldBuf);
   for ( int i = 0; i < ( written / 2 ); i += 2 ) {
      newBuf[i/2] = int16( (int32(newBuf[i]) + newBuf[i+1] ) / 2 );
   }
   written /= 2;
   oldBuf = reinterpret_cast<char*>( User::ReAllocL( oldBuf, written ) );
   return oldBuf;
}

void
OggDecoder::putDataL( ClipBuf& clip )
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   // FIXME: Is there a way to get the amount of needed memory
   // from the tremor lib before allocation?
   // Guess that we need at least aboot 10k
   int maxLen = 0;
   VORBIS_TRAPD( tmpErr, maxLen = ov_pcm_total( &m_oggFile, 0 ) );
   if ( maxLen == OV_EINVAL ) {
      // Guess 5 kb
      maxLen = 5*1024;
   } else {
      // Multiply by two since we "know" that it is 16-bit data.
      maxLen <<= 1;
      // Add two bytes so that we can see if the end of the
      // stream has been reached without reallocing.
      maxLen += 2;
   }
   // It can't be odd
   const bool maxLenWasOdd = int(maxLen) & 1;
   if ( maxLenWasOdd ) {      
      maxLen++;
   }
   
   char* bufc = reinterpret_cast<char*>(User::AllocL( maxLen ));   
   CleanupStack::PushL( bufc );
   
   TInt written = 0;
   bool eof = false;
   bool realloced = false;

   // Read until end-of-file. 
   while ( !eof  ) {

      if ( written == maxLen ) {
         // Realloc the buffer.
         int newLen = maxLen * 2;
         char* oldbufc = bufc;
         realloced = true;
         bufc = reinterpret_cast<char*>( User::ReAllocL( bufc, newLen ) );
         // oldbufc and bufc may be the same, maybe not.
         CleanupStack::Pop( oldbufc );
         CleanupStack::PushL( bufc );
         maxLen = newLen;
      }
      
      char* ptr = bufc + written;
      TInt res = 0;
      
      SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
      
      VORBIS_TRAPD(err, res=ov_read(&m_oggFile, ptr,
                                    maxLen - written, &m_oggSection));
      
      SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
      
      User::LeaveIfError(err);
      if (res<0)
      { // some sort of error with the stream - ignore and carry on
      }
      else if (res==0) 
      { // eof
         eof=ETrue;
      }
      else
      { // read res bytes of data
         written+=res;
         MC2_ASSERT( written <= maxLen );         
      }
   }

#ifdef NAV2_CLIENT_SERIES60_V1
   bufc = downSample(bufc, written);
#endif
   {
      TPtr8 buf(NULL, 0, 0);
      buf.Set( reinterpret_cast<TUint8*>(bufc), written, written );
      if ( realloced ) {
         // This will copy the buffer to a buffer of the correct size.
         clip.setAudioData( buf, int64(duration())*1000 );
         CleanupStack::PopAndDestroy( bufc );
      } else {
         // This will not copy the buffer.
         clip.handoverAudioData( buf, int64(duration())*1000 );
         CleanupStack::Pop( bufc );
      }
   }
}


size_t
OggDecoder::read_func( void* ptr, size_t size, size_t nitems )
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   if ( size == 0 || nitems == 0 ) return 0;
   TPtr8 tptr(NULL, 0, 0);
   tptr.Set( reinterpret_cast<TUint8*>(ptr), 0, size*nitems );
   int res = m_file.Read( tptr, size*nitems );
   
   if ( res != KErrNone ) {
      // Cannot return neg value.
      // Set errno. Not possible
      errno = EIO;
      tptr.SetLength(0); // To be sure
   }
   // Seems like the length of tptr is 0 if eof.
   return tptr.Length() / size;
}

int
OggDecoder::seek_func( ogg_int64_t offset, int whence)
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   int offset32 = int32(offset);
   int seek_res = KErrNone;
   switch (whence) {
      case SEEK_SET:
         seek_res = m_file.Seek( ESeekStart, offset32 );
         break;
      case SEEK_CUR:
         seek_res = m_file.Seek( ESeekCurrent, offset32 );
         break;
      case SEEK_END:
         seek_res = m_file.Seek( ESeekEnd, offset32 );
         break;
   }
   if ( seek_res == KErrNone ) {
      return 0;
   } else {
      errno = EIO;
      return -1;
   }
}

int
OggDecoder::close_func()
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   return 0;
}

long
OggDecoder::tell_func()
{
   SEP_HEAP_ASSERT( &User::Heap() == iDefaultHeap );
   // Get the pos.
   int pos = 0;
   int seek_res = m_file.Seek( ESeekCurrent, pos );
   if ( seek_res == KErrNone ) {
      return pos;
   } else {
      errno = EIO;
      return -1;
   }
}



size_t
OggDecoder::ovf_read_func(void* ptr, size_t size, size_t nitems, void* file)
{
   // Switch to standard heap since we are called from ogg.
   SET_S_HEAP_STATIC(file);
   size_t res = static_cast<OggDecoder*>(file)->read_func(ptr, size, nitems);
   // Switch back
   SET_V_HEAP_STATIC(file);
   return res;
}

int
OggDecoder::ovf_seek_func(void* datasource, ogg_int64_t offset, int whence)
{
   // Switch to standard heap since we are called from ogg.
   SET_S_HEAP_STATIC(datasource);
   int res = static_cast<OggDecoder*>(datasource)->seek_func(offset, whence);
   // Switch back
   SET_V_HEAP_STATIC(datasource);
   return res;
}

int
OggDecoder::ovf_close_func(void* datasource)
{
   // Switch to standard heap since we are called from ogg.
   SET_S_HEAP_STATIC(datasource);
   int res = static_cast<OggDecoder*>(datasource)->close_func();
   // Switch back
   SET_V_HEAP_STATIC(datasource);
   return res;
}

long
OggDecoder::ovf_tell_func(void* datasource)
{
   // Switch to standard heap since we are called from ogg.
   SET_S_HEAP_STATIC(datasource);
   long res = static_cast<OggDecoder*>(datasource)->tell_func();
   // Switch back
   SET_V_HEAP_STATIC(datasource);
   return res;
}

const ov_callbacks
OggDecoder::KOggVorbisCallbacks = {
   ovf_read_func,
   ovf_seek_func,
   ovf_close_func,
   ovf_tell_func
};
