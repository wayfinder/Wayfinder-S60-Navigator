/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TEXTITERATOR_H
#define TEXTITERATOR_H

class isoTextIterator {
  public:

   /**
    *  Constructor for a isoTextIterator, which iterates a string which
    *  is encoded with iso-8859-1.
    */
   isoTextIterator(const char* isostr) {
      m_str = isostr;
      m_pos = 0;
      m_maxLen = MAX_UINT32;
   }
   
   /**
    *  Constructor for a isoTextIterator, which iterates a string which
    *  is encoded with iso-8859-1.
    */
   isoTextIterator(const char* isostr, int maxlen ) {
      m_str = isostr;
      m_pos = 0;
      m_maxLen = maxlen;
   }
   
   const isoTextIterator& operator++() {
      ++m_pos;
      return *this;
   }

   int nbrBytesForCurrentChar() const {
      return 1;
   }
   
   const uint32 operator*() {
      if ( m_pos < m_maxLen ) {
         return uint32(uint8(m_str[m_pos]));
      } else {
         return 0;
      }
   }
   
  private:
   // The string to be iterated
   const char* m_str;

   // The current position
   unsigned int m_pos;
   // Maximum lenght
   uint32 m_maxLen;   
};


class utf8TextIterator {
  public:

   /**
    *  Constructor for a utf8TextIterator, which iterates a string which
    *  is encoded with utf-8.
    */
   utf8TextIterator(const char* utf8str) {
      m_str    = utf8str;
      m_pos    = 0;
      m_maxLen = MAX_UINT32;
   }
   
   /**
    *  Constructor for a utf8TextIterator, which iterates a string which
    *  is encoded with utf-8.
    *  Will return 0 when the max length is reached.
    */
   utf8TextIterator(const char* utf8str, int maxlen ) {
      m_str    = utf8str;
      m_pos    = 0;
      m_maxLen = maxlen;
   }
   
   static int nbrBytesForUtf8Char(const char* str, int pos ) {
      unsigned char c = str[pos];
      uint32 nbrBytes = 1;
      if( c <= 127 )
         nbrBytes = 1;
      else if( (c >= 192) && (c <= 223) )
         nbrBytes = 2;
      else if( (c >= 224) && (c <= 239) )
         nbrBytes = 3;
      else if( (c >= 240) && (c <= 247) )
         nbrBytes = 4;
      else if( (c >= 248) && (c <= 251) )
         nbrBytes = 5;
      else if( (c >= 252) && (c <= 253) )
         nbrBytes = 6;
      return nbrBytes;
   }
   
   int nbrBytesForCurrentChar() const {
      return nbrBytesForUtf8Char(m_str, m_pos);
   }
   
   const utf8TextIterator& operator++() {
      int nbrBytes = nbrBytesForUtf8Char(m_str, m_pos);
      m_pos += nbrBytes;
      return *this;
   }
   
    const uint32 operator*() {
      uint32 nbrBytes = nbrBytesForUtf8Char(m_str, m_pos);

      if ( m_pos + nbrBytes > m_maxLen ) {
         // Return end of string.
         return 0;
      }
      
      uint32 unicode = 0;
      switch (nbrBytes) {
         case 1: {
            unsigned char firstChar = m_str[m_pos];
            unicode = uint32(firstChar);
            break;
         }
         case 2: {
            unsigned char firstChar = m_str[m_pos];
            unsigned char secondChar = m_str[m_pos+1];
            unicode = uint32( (firstChar-192)*64 + (secondChar-128) );
            break;
         }
         case 3: {
            unsigned char firstChar = m_str[m_pos];
            unsigned char secondChar = m_str[m_pos+1];
            unsigned char thirdChar = m_str[m_pos+2];
            unicode = uint32( (firstChar-224)*4096 +
                              (secondChar-128)*64 +
                              (thirdChar-128) );
            break;
         }
         case 4: {
            unsigned char firstChar = m_str[m_pos];
            unsigned char secondChar = m_str[m_pos+1];
            unsigned char thirdChar = m_str[m_pos+2];
            unsigned char fourthChar = m_str[m_pos+3];
            unicode = uint32( (firstChar-240)*262144 +
                              (secondChar-128)*4096 +
                              (thirdChar-128)*64 +
                              (fourthChar-128) );
            break;
         }
         case 5: {
            unsigned char firstChar = m_str[m_pos];
            unsigned char secondChar = m_str[m_pos+1];
            unsigned char thirdChar = m_str[m_pos+2];
            unsigned char fourthChar = m_str[m_pos+3];
            unsigned char fifthChar = m_str[m_pos+4];
            unicode = uint32( (firstChar-248)*16777216 +
                              (secondChar-128)*262144 +
                              (thirdChar-128)*4096 +
                              (fourthChar-128)*64 +
                              (fifthChar-128) );
            break;
         }
         case 6: {
            unsigned char firstChar = m_str[m_pos];
            unsigned char secondChar = m_str[m_pos+1];
            unsigned char thirdChar = m_str[m_pos+2];
            unsigned char fourthChar = m_str[m_pos+3];
            unsigned char fifthChar = m_str[m_pos+4];
            unsigned char sixthChar = m_str[m_pos+5];
            unicode = uint32( (firstChar-252)*1073741824 +
                              (secondChar-128)*16777216 +
                              (thirdChar-128)*262144 +
                              (fourthChar-128)*4096 +
                              (fifthChar-128)*64 +
                              (sixthChar-128) );
         }
         default:
            unicode = 0;
            break;
      }
      return unicode;
   }

  private:
   // The string to be iterated
   const char* m_str;

   // The current position
   unsigned int m_pos;
   
   // Maximum lenght
   uint32 m_maxLen;   
};

// Creates a utf8 or iso text iterator
#ifdef MC2_UTF8
typedef utf8TextIterator mc2TextIterator;
#else
typedef isoTextIterator mc2TextIterator;
#endif

#endif
  
