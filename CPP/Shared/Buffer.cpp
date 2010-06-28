/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "arch.h"
#include "Buffer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace isab {

// - NonOwningBuffer

NonOwningBuffer::NonOwningBuffer( const uint8* bufferData, size_t len ) :
      Buffer( const_cast<uint8*>( bufferData ), len, len ) 
{
   // This is the difference from an ordinary buffer, currently.
   m_deleteData = false;
}
   
// - Buffer
   
Buffer::Buffer(size_t length) :
      m_dataLength(length), m_readPos(0), m_writePos(0), 
      m_validBytes(0), m_resize(true), m_deleteData(true)
{
   align32bit(m_dataLength);
   if (m_dataLength != 0) {
      m_data = new uint8[m_dataLength];
      if(m_data == NULL){
         m_dataLength = 0;
      }
   } else {
      m_data = NULL;
   }
}
   
Buffer::Buffer(uint8* bufferData, size_t len, size_t valid) :
      m_data(bufferData), m_dataLength(len), m_readPos(0), 
      m_writePos(valid), m_validBytes(valid), m_resize(false),
      m_deleteData( true )
{
}

Buffer::Buffer(const Buffer &b) :
      m_data(new uint8[b.m_dataLength]),  m_dataLength(b.m_dataLength), 
      m_readPos(b.m_readPos), m_writePos(b.m_writePos), 
      m_validBytes(b.m_validBytes), m_resize(b.m_resize),
      m_deleteData( true )
{
   memcpy(m_data, b.m_data, m_dataLength);
}

Buffer::~Buffer() {
   if ( m_deleteData ) {
      delete[] m_data;
   }
   m_data = NULL;
}

uint8*
Buffer::releaseData()
{
   uint8* data = m_data;
   m_data = NULL;
   m_dataLength = 0;
   m_readPos = 0;
   m_writePos = 0;
   m_validBytes = 0;   
   m_resize = true;
   // If resize is true then m_delete data must be true too.
   m_deleteData = true;
   return data;
}

void Buffer::checkIntegrity() const
{
//    if (m_readPos > m_validBytes) {
//       Assert(false);
//    }
//    if (m_writePos > m_dataLength) {
//       Assert(false);
//    }
}

bool Buffer::enlargeData(size_t amount)
{
   // Enlarge the data by another 25%. 
   // Blergh, I wanted to write MAX( amount, m_dataLength/4 ) but the
   // compiler wouldn't let me.
   amount = (amount > (m_dataLength/4)) ? amount : (m_dataLength/4);
   if(m_resize){
      size_t newLength = m_dataLength + amount;
      uint8* tmp = new uint8[newLength];

      if (m_data) {
         memcpy(tmp, m_data, m_writePos);
         if ( m_deleteData ) {
            delete[] m_data;
         }
      }
      m_dataLength = newLength;
      m_data = tmp;
      m_deleteData = true;
   }
   checkIntegrity();
   return m_resize;
}

void Buffer::clear()
{
   m_writePos = 0;
   m_readPos  = 0;
   m_validBytes = 0;
   checkIntegrity();
}

size_t Buffer::setReadPos(size_t newPos)
{  
   size_t old = m_readPos;
   if (newPos > m_validBytes) {
      newPos = m_validBytes;
   }
   m_readPos = newPos;
   checkIntegrity();
   return old;
}

size_t Buffer::setWritePos(size_t newPos)
{  
   size_t old = m_writePos;
   if (newPos > m_validBytes) {
      newPos = m_validBytes;
   }
   m_writePos = newPos;
   checkIntegrity();
   return old;
}

int Buffer::writeNextCharString(const char* data)
{
   if(data == NULL) data = "";
   int len = strlen(data) + 1;
   bool ok = writeNextByteArray(reinterpret_cast<const uint8*>(data), len);
   checkIntegrity();
   return ok ? len : 0;
}

int Buffer::nprintf(size_t size, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   int n = this->vnprintf(size, format, args);
   va_end(args);
   checkIntegrity();
   return n;
}

int Buffer::vnprintf(size_t size, const char* format, va_list args)
{
   int pos = getWritePos();
   if(!(m_dataLength - m_writePos < size) || enlargeData(size)){
      char* dst = reinterpret_cast<char*>(m_data + m_writePos);
      vsnprintf(dst, size, format, args);
      dst[size - 1] = '\0'; //No matter if truncated or not, this
                            //should be safe.
      int written = strlen(dst) + 1;
      m_writePos += written;
      if(m_validBytes < m_writePos){
         m_validBytes = m_writePos;
      }
   }
   checkIntegrity();
   return getWritePos() - pos;
}

   
bool Buffer::writeNextByteArray(const uint8* data, size_t n)
{
   size_t l = n;
   align32bit(l);
   if(!(m_dataLength - m_writePos < n) || 
      enlargeData(l + 4)) {

      // And copy.
      memcpy( m_data + m_writePos, data, n );
      m_writePos += n;
      
      if (m_writePos > m_validBytes) {
         m_validBytes = m_writePos;
      }
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}

bool Buffer::writeNext32bit(int32 data)
{
   align32bit(m_writePos);
   if(!(m_dataLength - m_writePos < 4) || enlargeData(8)){
      m_data[m_writePos++] = uint8((data >> (3 * 8)) & 0x0ff);
      m_data[m_writePos++] = uint8((data >> (2 * 8)) & 0x0ff);
      m_data[m_writePos++] = uint8((data >>      8 ) & 0x0ff);
      m_data[m_writePos++] = uint8((data           ) & 0x0ff);
      if (m_writePos > m_validBytes) {
         m_validBytes = m_writePos;
      }
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}  

bool Buffer::writeNext16bit(int16 data)
{
   align16bit(m_writePos);
   if(!(m_dataLength - m_writePos < 2) || enlargeData(4)){
      m_data[m_writePos++] = uint8((data >>      8 ) & 0x0ff);
      m_data[m_writePos++] = uint8((data           ) & 0x0ff);
      if (m_writePos > m_validBytes) {
         m_validBytes = m_writePos;
      }
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}  

bool Buffer::writeNextUnaligned32bit(int32 data)
{
   if(!(m_dataLength - m_writePos < 4) || enlargeData(8)){
      m_data[m_writePos++] = uint8((data >> (3 * 8)) & 0x0ff);
      m_data[m_writePos++] = uint8((data >> (2 * 8)) & 0x0ff);
      m_data[m_writePos++] = uint8((data >>      8 ) & 0x0ff);
      m_data[m_writePos++] = uint8((data           ) & 0x0ff);
      if (m_writePos > m_validBytes) {
         m_validBytes = m_writePos;
      }
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}  

bool Buffer::writeNextUnaligned64bit(int64 data)
{
   if(!(m_dataLength - m_writePos < 8) || enlargeData(16)){
      uint32 high = HIGH(data);
      uint32 low  =  LOW(data); 
      writeNextUnaligned32bit(high);
      writeNextUnaligned32bit(low);
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}  

int64 Buffer::readNextUnaligned64bit()
{
   if (m_validBytes - m_readPos < 8) {
//       Assert(false);
      return 0;
   }

   uint32 high = readNextUnaligned32bit();
   uint32 low = readNextUnaligned32bit();
   int64 data = MakeInt64(high, low);
   checkIntegrity();
   return data;
}

bool Buffer::writeNext64bit(int64 data)
{
   if(!(m_dataLength - m_writePos < 8) || enlargeData(16)){
      uint32 high = HIGH(data);
      uint32 low  = LOW(data); 
      writeNext32bit(high);
      writeNext32bit(low);
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}

int64 Buffer::readNext64bit()
{
   if (m_validBytes - m_readPos < 8) {
//       Assert(false);
      return 0;
   }

   uint32 high = readNext32bit();
   uint32 low = readNext32bit();
   int64 data = MakeInt64(high, low);
   checkIntegrity();
   return data;
}

bool Buffer::writeNextUnaligned16bit(int16 data)
{
   if(!(m_dataLength - m_writePos < 2) || enlargeData(4)){
      m_data[m_writePos++] = uint8((data >> 8 ) & 0x0ff);
      m_data[m_writePos++] = uint8((data      ) & 0x0ff);
      if (m_writePos > m_validBytes) {
         m_validBytes = m_writePos;
      }
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}  

bool Buffer::writeNext8bit(int8 data)
{
   if(!(m_dataLength - m_writePos < 1) || enlargeData(4)){
      m_data[m_writePos++] = (data);
      if (m_writePos > m_validBytes) {
         m_validBytes = m_writePos;
      }
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}  

bool Buffer::writeNextFloat(float data)
{
   align32bit(m_writePos);
   if(!(m_dataLength - m_writePos < 4) || enlargeData(8)){
      m_data[m_writePos++] = *(((uint8 *) &data)+3);
      m_data[m_writePos++] = *(((uint8 *) &data)+2);
      m_data[m_writePos++] = *(((uint8 *) &data)+1);
      m_data[m_writePos++] = *(((uint8 *) &data)+0);
      if (m_writePos > m_validBytes) {
         m_validBytes = m_writePos;
      }
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}  

bool Buffer::writeNextBuffer(const Buffer& data)
{
   uint8* src = data.m_data;
   size_t n = data.getLength();
   if(!(m_dataLength - m_writePos < n) || enlargeData((n + 3) & ~0x03)){
      uint8* dst = m_data + m_writePos;
      memcpy(dst, src, n);
      m_writePos += n;
      if(m_writePos > m_validBytes){
         m_validBytes = m_writePos;
      }
      checkIntegrity();
      return true;
   }
   checkIntegrity();
   return false;
}

int32 Buffer::readNext32bit()
{
   int32 data;

   align32bit(m_readPos);
   if (m_validBytes - m_readPos < 4) {
//       Assert(false);
      return 0;
   }
   data  = int32(m_data[m_readPos++] ) << (3*8);
   data |= int32(m_data[m_readPos++] ) << (2*8);
   data |= int32(m_data[m_readPos++] ) << 8;
   data |= int32(m_data[m_readPos++] );
   checkIntegrity();
   return data;
}

int16 Buffer::readNext16bit()
{
   int16 data;

   align16bit(m_readPos);
   if (m_validBytes - m_readPos < 2) {
      //      Assert(false);
      return 0;
   }
   data  = int16(int16(m_data[m_readPos++] ) << 8);
   data |= int16(m_data[m_readPos++] );
   checkIntegrity();
   return data;
}


int32 Buffer::readNextUnaligned32bit()
{
   int32 data;

   if (m_validBytes - m_readPos < 4) {
//       Assert(false);
      return 0;
   }
   data  = int32(m_data[m_readPos++] ) << (3*8);
   data |= int32(m_data[m_readPos++] ) << (2*8);
   data |= int32(m_data[m_readPos++] ) << 8;
   data |= int32(m_data[m_readPos++] );
   checkIntegrity();
   return data;
}

int16 Buffer::readNextUnaligned16bit()
{
   int16 data;

   if (m_validBytes - m_readPos < 2) {
//       Assert(false);
      return 0;
   }
   data  = int16(int(m_data[m_readPos++] ) << 8);
   data |= int16(m_data[m_readPos++] );
   checkIntegrity();
   return data;
}

int8 Buffer::readNext8bit()
{
   int8 data;

   if (m_validBytes - m_readPos < 1) {
//       Assert(false);
      return 0;
   }
   data  = (m_data[m_readPos++] );
   checkIntegrity();
   return data;
}

float Buffer::readNextFloat()
{
   float value;

   align32bit(m_readPos);
   if (m_validBytes - m_readPos < 4) {
      //      Assert(false);
      return 0;
   }

   *(((uint8 *) &value)+3) = m_data[m_readPos++];
   *(((uint8 *) &value)+2) = m_data[m_readPos++];
   *(((uint8 *) &value)+1) = m_data[m_readPos++];
   *(((uint8 *) &value)+0) = m_data[m_readPos++];
   checkIntegrity();
   return value;
}

int Buffer::readNextByteArray(uint8* data, int n)
{
   if( int32(m_validBytes) - int32(m_readPos) < n){
      n = int32(m_validBytes) - int32(m_readPos);
   }

   memcpy( data, m_data + m_readPos, n );
   m_readPos += n;
   
   checkIntegrity();
   return n;
}

int Buffer::readNextCharString(char* dst, size_t n)
{
   unsigned i;
   bool fullString = false;
   if(m_validBytes - m_readPos < n){
      n = m_validBytes - m_readPos;
   }
   for(i = 0; i < n; ++i){
      if(0 == (dst[i] = m_data[m_readPos + i])){
         fullString = true;
         break;
      }
   }
   m_readPos += i;
   if(! fullString){
      dst[n-1] = '\0';
      return -1;
   }
   checkIntegrity();
   return i;
}



char* Buffer::getNextCharStringAlloc()

{

   char* dst = NULL;
   if (m_data == NULL){
   }
   else{
      char* chString = 
         reinterpret_cast<char*>(&m_data[m_readPos]);
      size_t n = 
         //strnlen(chString, m_validBytes - m_readPos) + 1;
         strlen(chString) + 1;
   
      if (n > m_validBytes - m_readPos){
      }
      else{
         dst = new char[n];
         strncpy(dst, chString, n);
      }
      m_readPos += n;
   }
   checkIntegrity();
   return dst;
}

const char* Buffer::getNextCharString()
{
   void *nextZero = NULL;
   char* nextString = NULL;
   if(m_validBytes > m_readPos){
      nextZero = memchr(m_data + m_readPos, 0, m_validBytes - m_readPos);
      if(nextZero != NULL){
         nextString = reinterpret_cast<char*>(m_data + m_readPos);
         m_readPos = reinterpret_cast<uint8*>(nextZero) - m_data + 1;
      }
   }
   checkIntegrity();
   return nextString;
}

void Buffer::takeDataAndClear(Buffer &buf)
{
   if ( m_deleteData ) {
      delete[] m_data;
   }
   m_data = buf.m_data;
   m_deleteData = buf.m_deleteData;
   m_dataLength = buf.m_dataLength;
   m_validBytes = buf.m_validBytes;
   m_readPos = 0;
   m_writePos = 0;
   buf.m_data = NULL;
   buf.m_validBytes = 0;
   buf.m_dataLength = 0;
   buf.m_readPos = 0;
   buf.m_writePos = 0;
   checkIntegrity();
}

void Buffer::takeDataAndDelete(Buffer *buf)
{
   if ( m_deleteData ) {
      delete[] m_data;
   }
   m_data = buf->m_data;
   m_deleteData = buf->m_deleteData;
   m_dataLength = buf->m_dataLength;
   m_validBytes = buf->m_validBytes;
   m_readPos = 0;
   m_writePos = 0;
   buf->m_data = NULL;
   delete buf;
   checkIntegrity();
}

size_t Buffer::jumpReadPos(int offset)
{
   size_t ret = m_readPos;
   if(offset < 0){
      size_t tmp = abs(offset);
      if(tmp > m_readPos){
         m_readPos = 0;
      } else {
         m_readPos -= tmp;
      }
   } else if(offset > 0){
      size_t tmp = offset + m_readPos;
      if(tmp >= m_validBytes){
         m_readPos = m_validBytes;
      } else {
         m_readPos = tmp;
      }
   }
   checkIntegrity();
   return ret;
}

size_t Buffer::jumpReadPosTo(uint8 value)
{
   size_t ret = m_readPos;
   while (m_readPos < m_validBytes && m_data[m_readPos] != value) {
      ++m_readPos;
   }
   checkIntegrity();
   return ret;
}

size_t Buffer::jumpWritePos(int offset)
{
   size_t ret = m_writePos;
   if(offset < 0){
      size_t tmp = abs(offset);
      if(tmp > m_writePos){
         m_writePos = 0;
      } else {
         m_writePos -= tmp;
      }
   } else if(offset > 0){
      size_t tmp = offset + m_writePos;
      if(tmp >= m_validBytes){
         m_writePos = m_validBytes;
      } else {
         m_writePos = tmp;
      }
   }
   checkIntegrity();
   return ret;
}

size_t Buffer::alignReadPos16bit()
{
   size_t oldPos = m_readPos;
   align16bit(m_readPos);   
   return m_readPos - oldPos;
}

size_t Buffer::alignWritePos16bit()
{
   size_t oldPos = m_writePos;
   align16bit(m_writePos);   
   return m_writePos - oldPos;
}


} /* namespace isab */
