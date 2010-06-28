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

#include "NewAudioServerTypes.h"
#include <badesca.h>

void ClipBuf::initEmpty( const TDesC& fileName ) 
{     
   // Copy the filename
   m_array = NULL;
   m_time = TTimeIntervalMicroSeconds(0);
   m_fileName = fileName.Alloc();
   m_des.Set( NULL, 0 );
   m_realBuf = NULL;
   m_numChannels = -1;
   m_sampleRate = -1;
}
   
ClipBuf::ClipBuf(const TDesC& fileName ) 
{
   initEmpty( fileName );
}
   
ClipBuf::ClipBuf(const class ClipBuf& other) 
{
   // Copy filename and set buffer to null.
   initEmpty( other.m_fileName->Des() );
   // Also copy the data.
   if ( other.m_des.Length() != 0 ) {
      // It was loaded.
      setAudioData( other.m_des, other.m_time );
   }
}

void ClipBuf::setAudioProperties( int numchannels,
                                  int samplerate ) 
{
   m_numChannels = numchannels;
   m_sampleRate  = samplerate;
}

void ClipBuf::handoverAudioData( TDes8& audioData,
                                 const class TTimeIntervalMicroSeconds& timeMicro )
 {
   delete [] m_realBuf;
   m_realBuf = NULL;
   delete m_array;
   m_array = NULL;
   if ( audioData.Length() ) {
      // Need an ordinary buffer, sice the HBufC8 does not
      // work for audio playing.
      // Blaarhhg. Need to const_cast it.
      m_realBuf = const_cast<TUint8*>( audioData.Ptr() );
      // But the API wants a TDes8
      m_des.Set( reinterpret_cast<TUint8*>(m_realBuf), audioData.Length());
      m_time = timeMicro;
#if 1
      // Try splitting it into smaller buffers
#ifdef NAV2_CLIENT_UIQ3
      const int dataPerBuf = 8192 / 2;
#else
      const int dataPerBuf = 4096 / 2;
#endif
      m_array =
         new CPtrC8Array( (audioData.Length()+dataPerBuf-1) / dataPerBuf );
      int curPos = 0;
      int dataLeft = audioData.Length();
      const TUint8* data = (TUint8*)m_realBuf;
      while ( dataLeft > 0 ) {
         int dataToAdd = MIN( dataPerBuf, dataLeft );
         TPtrC8 tmp;
         tmp.Set( data + curPos, dataToAdd );
         m_array->AppendL( tmp );
         dataLeft -= dataToAdd;
         curPos   += dataToAdd;
      }
#else
      m_array = new CPtrC8Array(1);
      m_array->AppendL( m_des );
#endif
   }      
}
   
void ClipBuf::setAudioData(const TDesC8& audioData,
                           const class TTimeIntervalMicroSeconds& timeMicro )
{
   // Delete these before allocing new data.
   delete [] m_realBuf;
   m_realBuf = NULL;
   delete [] m_array;
   m_array = NULL;
   TPtr8 ptr( NULL, 0, 0 );
   ptr.Set( NULL, 0, 0 );
   if ( audioData.Length() ) {
      TUint8* tmpBuf = new TUint8[ audioData.Length() ];
      ptr.Set( tmpBuf, audioData.Length(), audioData.Length() );
      ptr.Copy( audioData );
      //           Mem::Copy( tmpBuf, audioData.Ptr(),
      //                      audioData.Length() );
         
   }
   handoverAudioData( ptr, timeMicro );
}

int ClipBuf::sampleRate() const 
{ 
   return m_sampleRate;
}
int ClipBuf::numChannels() const 
{ 
   return m_numChannels;
}
   
bool ClipBuf::isConverted() const 
{ 
   return m_realBuf != NULL; 
}

int ClipBuf::getNbrBuffers() const 
{ 
   if ( isConverted() ) {
      return m_array->MdcaCount(); 
   } else { 
      return 0; 
   } 
}
   
const TPtrC8& ClipBuf::getBuffer(int i )
{ 
   return (*m_array)[i]; 
}
   
const TDesC& ClipBuf::getFileName() const 
{ 
   return *m_fileName;
}
   
ClipBuf::~ClipBuf() 
{ 
   delete [] m_realBuf; 
   delete m_fileName; 
   delete m_array;
}

ClipBufIterator::ClipBufIterator() : 
   m_clips(&m_emptyClips) 
{
   m_clipIt = m_clips->begin();      
   m_bufNbr = 0;
}
   
ClipBufIterator::ClipBufIterator(const TClips& clips) : 
   m_clips( &clips )
{      
   m_clipIt = m_clips->begin();
   while ( m_clipIt != m_clips->end() &&
           !(*m_clipIt)->isConverted() ) {
      ++m_clipIt;
   }
   m_bufNbr = 0;
}

class ClipBufIterator& ClipBufIterator::operator++() 
{      
   m_bufNbr++;
   if ( m_bufNbr >= (*m_clipIt)->getNbrBuffers() ) {
      if ( m_clipIt != m_clips->end() ) {
         ++m_clipIt;
      }
      // Skip the unconverted ones.
      while ( m_clipIt != m_clips->end() &&
              !(*m_clipIt)->isConverted() ) {
         ++m_clipIt;
      }
      m_bufNbr = 0;
   }
   return *this;
}
   
const TDesC8* ClipBufIterator::operator*() const 
{
   if ( m_clipIt != m_clips->end() &&
        m_bufNbr < (*m_clipIt)->getNbrBuffers() ) {
      return &((*m_clipIt)->getBuffer(m_bufNbr));
   } else {
      return NULL;
   }
}
