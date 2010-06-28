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
#include "UTF8Util.h"
#include "TextIterator.h"

char*
UTF8Util::isoToUtf8L( const char* str, int* outlen,
                     int* inlen )
{
   int len = inlen ? *inlen : strlen(str);
   
   char* res = new char[ (len << 1) + 1 ];
   int inpos = 0;
   uint32 ch = 0;

   int outpos = 0;
   
   while( inpos < len ) {
      ch = uint8( str[ inpos ] );
      
      if ( (ch & 0x80) == 0 ) { // Single byte, ch < 0x80
         res[outpos++] = ch;
      } else { // Multibyte utf-8 char
         // Since input is ISO-8859-1 ch is always < 0x800
         // So only two bytes are needed
         res[outpos++] = char( ((ch >> 6) | 0xC0) );
         res[outpos++] = char( ((ch & 0x3F) | 0x80) );
      }
      ++inpos;
   }

   // Zero terminate
   res[outpos] = '\0';
   
   if ( outlen ) {
      *outlen = outpos;
   }
   return res;
}

int
UTF8Util::unicodeToUtf8(uint32 unicode,
                        char* utf8)
{
   int idx = 0;
   if( (unicode >= 0x00000000 ) && (unicode <= 0x0000007f) ) {
      utf8[idx++] = unicode;
   } else if( (unicode >= 0x00000080) && (unicode <= 0x000007ff) ) {
      // First char
      unsigned char firstChar = 192 + (unicode / 64);
      utf8[idx++] = ( (char ) firstChar);
      // Second char
      unsigned char secondChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char ) secondChar);
   } else if( (unicode >= 0x00000800) && (unicode <= 0x0000ffff) ) {
      // First char
      unsigned char firstChar = 224 + (unicode / 4096);
      utf8[idx++] = ( (char) firstChar);
      // Second char
      unsigned char secondChar = 128 + ((unicode / 64) % 64);
      utf8[idx++] = ( (char) secondChar);
      // Third char
      unsigned char thirdChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char) thirdChar);
   } else if( (unicode >= 0x00010000) && (unicode <= 0x001fffff) ) {
      // First char
      unsigned char firstChar = 240 + (unicode / 262144);
      utf8[idx++] = ( (char) firstChar);
      // Second char
      unsigned char secondChar = 128 + ((unicode / 4096) % 64);
      utf8[idx++] = ( (char) secondChar);
      // Third char
      unsigned char thirdChar = 128 + ((unicode / 64) % 64);
      utf8[idx++] = ( (char) thirdChar);
      // Fourth char
      unsigned char fourthChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char) fourthChar);
   } else if( (unicode >= 0x00200000) && (unicode <= 0x03ffffff) ) {
      // First char
      unsigned char firstChar = 248 + (unicode / 16777216);
      utf8[idx++] = ( (char) firstChar);
      // Second char
      unsigned char secondChar = 128 + ((unicode / 262144) % 64);
      utf8[idx++] = ( (char) secondChar);
      // Third char
      unsigned char thirdChar = 28 + ((unicode / 4096) % 64);
      utf8[idx++] = ( (char) thirdChar);
      // Fourth char
      unsigned char fourthChar = 128 + ((unicode / 64) % 64);
      utf8[idx++] = ( (char) fourthChar);
      // Fifth char
      unsigned char fifthChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char) fifthChar);
   } else if( (unicode >= 0x04000000) && (unicode <= 0x7fffffff) ) {
      // First char
      unsigned char firstChar = 252 + (unicode / 1073741824);
      utf8[idx++] = ( (char) firstChar);
      // Second char
      unsigned char secondChar = 128 + ((unicode / 16777216) % 64);
      utf8[idx++] = ( (char) secondChar);
      // Third char
      unsigned char thirdChar = 128 + ((unicode / 262144) % 64);
      utf8[idx++] = ( (char) thirdChar);
      // Fourth char
      unsigned char fourthChar = 128 + ((unicode / 4096) % 64);
      utf8[idx++] = ( (char) fourthChar);
      // Fifth char
      unsigned char fifthChar = 128 + ((unicode / 64) % 64);
      utf8[idx++] = ( (char) fifthChar);
      // Sixth char
      unsigned char sixthChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char) sixthChar);
   }

   // Zero terminate.
   utf8[idx] = '\0';
   return idx;
}

void
UTF8Util::appendUtf8(char **real_dest, uint32 *real_left, char *src)
{
   char *dest = *real_dest;
   uint32 left = *real_left;

   uint32 copy_length = strlen(src);

   if (left < (copy_length+1)) {
      /* Need to reallocate the string buffer. */
      int new_left = left+copy_length+16;
      char *tmp = new char[strlen(dest)+new_left];
      strcpy(tmp, dest);
      delete[] dest;
      dest = tmp;
      left = new_left;
   }

   /* Copy src to dest. */
   strcat(dest, src);
   left -= copy_length;

   *real_dest = dest;
   *real_left = left;
}

char *
UTF8Util::cleanUtf8( const char * instr ) 
{
   int count = 0;
   uint32 length = strlen(instr);
   /* Need at least this much if it's an UTF-8 string already. */
   char *res = new char[length+1];
   res[0] = 0; /* Nul terminate. */
   uint32 left = length; /* Number of chars left in buffer. */

   for ( uint32 pos = 0; pos < length; pos += count ) {
      count = utf8TextIterator::nbrBytesForUtf8Char( instr, pos );
      if ( pos + count > length ) {
         // Illegal. Assume latin-1
         char tmp[8];
         unicodeToUtf8( (byte)instr[pos], tmp );
         appendUtf8(&res, &left, tmp);
/*           res += tmp; */
         count = 1;
         continue;
      }
      
      // Check for valid utf-8 in other ways
      bool valid = true;
      switch ( count ) {
         case 1:
            // High bit must not be set. Ascii...
            valid = ( instr[pos] & 0x80 ) == 0;
            break;
         case 2:
            valid = ( ( instr[pos] & 0xe0 ) == 0xc0 ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 );
            break;
         case 3:
            valid = ( ( instr[pos] & 0xf0 ) == 0xe0 ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+2] & 0xc0 ) == 0x80 );
            break;
         case 4:
            valid = ( ( instr[pos] & 0xf8 ) == 0xf0 ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+2] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+3] & 0xc0 ) == 0x80 );
            break;
         case 5:
            valid = ( ( instr[pos] & 0xfc ) == 0xf8 ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+2] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+3] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+4] & 0xc0 ) == 0x80 );
            break;
         case 6:
            valid = ( ( instr[pos] & 0xfe ) == 0xfc ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+2] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+3] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+4] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+5] & 0xc0 ) == 0x80 );
            break;            
         default:
            valid = false;
      }

      if ( valid ) {
         char tmp[8];
         strncpy( tmp, &instr[pos], count );
         tmp[count] = 0; /* Nul terminate since we use strncpy. */
         appendUtf8(&res, &left, tmp);
/*          res += instr.substr( pos, count ); */
      } else {
         char tmp[8];
         unicodeToUtf8( (byte)instr[pos], tmp );
         appendUtf8(&res, &left, tmp);
/*          res += tmp; */
         count = 1;
      }
   }
   return res;
}

#ifdef __SYMBIAN32__
#include <e32std.h>
#endif

int
UTF8Util::strCollateUtf8( const char* str1, const char* str2)
{
   utf8TextIterator cs (str1);
   utf8TextIterator ct (str2);

   while ( 1 ) {
#ifdef __SYMBIAN32__
      // XXX This sucker does not seem to work with for example 
      // swedish characters even though the locale is Swedish. 
      // At least not when it comes to comparing favorites. XXX
      uint32 a = User::Collate(*cs);
      uint32 b = User::Collate(*ct);
#else
      uint32 a = *cs;
      uint32 b = *ct;
#endif
      if ( a < b ) {
         return -1;
      }
      if ( a > b ) {
         return 1;
      }
      if ( a == b && a == 0 ) {
         return 0;
      }
      ++cs;
      ++ct;
   }
}




