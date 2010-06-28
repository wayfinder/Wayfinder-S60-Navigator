/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef BUFFER_H
#define BUFFER_H
#include "arch.h"
namespace isab{
   class Buffer{
   protected:
      /** The payload data of this buffer */
      uint8* m_data;

      /** Actual size of the data area. N.B. this is not the 
       * number of used bytes in the buffer. This is internal 
       * only. */
      size_t m_dataLength;

      /** Current read pointer */
      size_t m_readPos;

      /** Current write pointer. */
      size_t m_writePos;

      /** Number of valid bytes in the buffer. */
      size_t m_validBytes;

      /** Indicates if this Buffer is allowed to resize on it's own.*/
      bool m_resize;

      /** True if data should be deleted by destructor */
      bool m_deleteData;

      /** Increases the data area by amnount. This is not a cheap operation
       * and should be used with reason. 
       * @param amount how many bytes to enlarge the data area.
       * @return       true if the opration was successfull. 
       */
      bool enlargeData(size_t amount);

      const Buffer& operator=(const Buffer& buf);

   public:
      static inline void align32bit(size_t &a) {
         a=(a+3) & ~ size_t(0x03);
      }

      static inline void align16bit(size_t &a) {
         a=(a+1) & ~ size_t(0x01);
      }

      size_t alignReadPos16bit();
      size_t alignWritePos16bit();

      Buffer(size_t length = 4);

      /** Constructor using supplied memory as the buffer memory 
       * area. 
       * Note: Once a data area is given over to a Buffer, 
       *       it really shouldn't be touched in any way other 
       *       than through the Buffer. 
       * Note: If you don't want the Buffers destructor to free 
       *       the data area, call the releaseData function before
       *       the destructor is invoked.
       * @param bufferData a pointer to the memory area the Buffer 
       *                   will call its own.
       * @param len        the size of the bufferData.
       * @param valid      how many bytes in the bufferData that are 
       *                   already written. This value will also be 
       *                   used as the write position.
       */
      Buffer(uint8* bufferData, size_t len, size_t valid);

      /**
       *  Releases the Buffers memory area and resets the Buffer to 
       *  a cleared state. This method is used to make the Buffer 
       *  release it's data before the Buffer is destroyed. This is 
       *  usefull if the data area was allocated outside the buffer.
       *  This will turn a NonOwningBuffer into an ordinary buffer too.
       */
      uint8* releaseData();
      
      /**
       *   Copy constructor. Copies the data too.
       */
      Buffer(const Buffer &b);
      
      virtual ~Buffer();

      /** @return the number of valid data bytes in this buffer */
      uint32 getLength() const
      {
         return m_validBytes;
      }

      /** @return the number of remaing bytes to read in this buffer. */
      uint32 remaining() const
      {
         int32 rem = int32(m_validBytes) - int32(m_readPos);
         return rem >= 0 ? rem : 0;
      }

      /** Move the read pointer to the specified position. If a move is 
       * attempted outside the interval 0..m_validBytes-1 it will be 
       * truncated.
       * @param newPos the desired read position 
       */
      size_t setReadPos(size_t newPos);

      /** @return the current read pointer. This value can be passed to
       * setReadPos() later on.
       */
      size_t getReadPos() const
      {
         return m_readPos;
      }

      /** Move the write pointer to the specified position. If a move is 
       * attempted outside the interval 0..m_validBytes it will be 
       * truncated.
       * @param newPos the desired read position 
       */
      size_t setWritePos(size_t newPos);

      /** @return the current write pointer. This value can be passed to
       * setWritePos() later on.
       */
      size_t getWritePos() const
      {
         return m_writePos;
      }

      void setValidBytes(size_t validBytes)
      {
         m_validBytes = validBytes;
      }

      /** Clear tha buffer logically by setting m_validBytes = 0
       * and resets the read and write pointers to 0 as well.
       * Normally called before any write-functions. */
      void clear();

      /**
       * Write n bytes of data from an array to the end of the buffer
       * @param data a pointer to at least n bytes of valid data.
       * @param n    the number of bytes to read from data and write 
       *             to the buffer.
       * @return true if the array was written, false if it wasn't due to 
       *         lack of space.
       */
      bool writeNextByteArray(const uint8* data, size_t n);

      /** Writes a c-style string to the buffer.
       * @param data a c-style string, i.e. a zero-terminated char array.
       * @return the number of bytes written. 
       */
      int writeNextCharString(const char* data);

      /** Writes a c-style string to buffer using a printf style
       * format string.
       * @param size the maximum number of characters written, 
       *             including the terminating '\0'.
       * @param the format string. 
       * @param ... varags list.
       */
      int nprintf(size_t size, const char* format, ...) 
#if !defined(_MSC_VER) && !defined(__MWERKS__)
               __attribute__ ((format (printf, 3, 4)));
#endif
               ;

      int vnprintf(size_t size, const char* format, va_list args);

      /** 
       *    Write an int32 to the end of the buffer.
       *    @param data Data to write in buffer. 
       *    @return true if the data was succesfully written, false if 
       *            no data was written due to lack of space.
       */
      bool writeNext32bit(int32 data);   

      /** 
       *    Write an int16 to the end of the buffer.
       *    @param data Data to write in buffer. 
       * @return true if the operation succeeded.
       */
      bool writeNext16bit(int16 data);   

      /** Writes an int16 to the end of the buffer WITHOUT aligning
       * the data.
       * @param data the data to write to the buffer.
       * @return true if the operation suceeded.
       */
      bool writeNextUnaligned16bit(int16 data);

      /** Writes an int32 to the end of the buffer WITHOUT aligning
       * the data.
       * @param data the data to write to the buffer.
       * @return true if the operation suceeded.
       */
      bool writeNextUnaligned32bit(int32 data);

      /** 
       *    Write an int8 to the end of the buffer.
       *    @param data Data to write in buffer. 
       * @return true if the operation succeeded.
       */
      bool writeNext8bit(int8 data);   

      /** 
       *    Write a float to the end of the buffer.
       *    @param data Data to write in buffer. 
       * @return true if the operation succeeded.
       */
      bool writeNextFloat(float data);   
      
      /** Writes all the data from the Buffer supplied as an argument 
       * at the end of this Buffer.
       * @param data the Buffer that shall be copied.
       * @return true if the operation was successfull.
       */
      bool writeNextBuffer(const Buffer& data);

      int32 readNext32bit();
      int16 readNext16bit();
      int16 readNextUnaligned16bit();
      int32 readNextUnaligned32bit();
      int8  readNext8bit();
      float readNextFloat();
      

      bool writeNextUnaligned64bit(int64 data);
      int64 readNextUnaligned64bit();
      bool writeNext64bit(int64 data);
      int64 readNext64bit();
      /**
       * Read the next n bytes from the buffer and store 
       * them at the start of the data vector.
       * @param data a pointer to at least n bytes of space.
       * @param n    the number of bytes to read from the buffer and write 
       *             to the data vector.
       */
      int readNextByteArray(uint8* data, int n);

      /** Read the next 0-terminated c-type char string from 
       * the Buffer and store it at the specified place. If the string 
       * is longer than the destination area, it will be truncated. 
       * The written string is ALWAYS zero terminated and the zero byte
       * is included in the return value.
       * The read position will be set to the next unwritten byte, no
       * matter if the string was truncated or not.
       * @param dst pointer to a data area where the read string 
       *            will be written.
       * @param n   the size of the dst memory area.
       * @return the number of bytes written. -1 if the string was 
       *         truncated as a result of the dst memory area 
       *         being too small.
       */
      int readNextCharString(char* dst, size_t n);

      /** 
       * Read the next 0-terminated c-type char string from 
       * the Buffer, allocates a string of the right length and stores
       * the string in the allocated string. If no 0-termination is 
       * present after the current position in the Buffer, NULL is
       * returned.
       *
       * If NULL is returned the postiong in the Buffer is unaffected.
       *
       * @return An allocated string containing the next c-type string
       *         in the buffer, or NULL if no such string exists.
       */
      char* getNextCharStringAlloc();

      /** Returns a pointer to the next char string and moves the 
       * read position past it.
       * @return a pointer to a const char string.
       */
      const char* getNextCharString();

      void checkIntegrity() const;

      /** Take over the data from the buffer and delete it */
      void takeDataAndDelete(Buffer *buf);
      /** Take over the data from the buffer and leave the buffer without any data */
      void takeDataAndClear(Buffer &buf);
      /** Provides acces to the raw buffer data. Note that the data may 
       * not be changed by any outside function.
       * @param index the buffer index to start looking at.
       * @return a pointer to the data, starting at the specified index.
       *         NULL if there are no data or the index is out of bounds.
       */
      const uint8* accessRawData(unsigned index = unsigned(-1)) const
      {
         if(index == unsigned(-1)){
            index = m_readPos;
         }
         if(m_data && index < m_validBytes){
            return m_data + index;
         } else {
            return NULL;
         }
      }

      /** Moves the read position a signed offset from the current position.
       * @param offset the offset.*/
      size_t jumpReadPos(int offset);
      /** Jumps read pos forward to the next byte with the specified value.
       * @param the value to search for.
       * @return */
      size_t jumpReadPosTo(uint8 value);
      /** Moves the write position a signed offser from the current position.
       * @param offset the offset. */
      size_t jumpWritePos(int offset);

      void reserve(int size){
         size = (size - m_dataLength + 3) & ~3U;
         if (size < 0) {
            size = 0;
         } else {
            enlargeData(size);
         }
      }

      size_t capacity() const
      {
         return m_dataLength;
      }
  };

   /**
    *    Buffer that does not own its data.
    *
    *    WARNING!
    *
    *    The buffer is intended for local use e.g.
    *    in functions that want a temporary buffer to
    *    read messages from. Some functions, e.g. releaseData()
    *    might lead the one to beleive that the released data
    *    should be deleted etc. so don't send the buffer to
    *    another function.
    */
   class NonOwningBuffer : public Buffer {
   public:
      /**
       *   Constructor.
       *   @param bufferData Buffer to borrow for the buffer data.
       *   @param len        Length of valid data.
       */      
      NonOwningBuffer( const uint8* bufferData, size_t len );
      
   };


}

#endif
