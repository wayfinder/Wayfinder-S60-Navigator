/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TEXTCONV_H
#define TEXTCONV_H

/* 
 *  Class with static methods to convert text from one form to another.
 *  Currently, supports UTF8 to Unicode and back.
 *  Only for Win32 platform.
 */

#include "config.h"
#include <cstring>

class TextConverter
{
public:
   /* Converts from UTF8 to Unicode, returns number of characters in output 
      if successful; else a system-specific error code (negative) */
   static int UTF2Unicode(const char *utfStr, 
                          unsigned short *uniStr, 
                          int outLen)
   {
      /* set the output to zero */
      memset(uniStr, 0, outLen);

      /* convert the UTF8 text to Unicode */
      int uniLen = 0;
      uniLen = MultiByteToWideChar(CP_UTF8,
                                   0,
                                   utfStr,
                                   strlen(utfStr), // nbr bytes in str.
                                   uniStr,
                                   outLen);
      // Also null terminate.
      if ( uniLen < outLen ) {
         uniStr[ uniLen ] = 0;
      } else if ( outLen > 0 ) {
         uniStr[ outLen - 1 ] = 0;
      }

      return(uniLen);
   };


   /* Converts from UTF8 to Unicode, returns number of characters in output 
      if successful; else a system-specific error code (negative) */
   static int Unicode2UTF(const unsigned short *uniStr, 
                          char *utfStr, 
                          int outLen)
   {
      /* set the output to zero */
      memset(utfStr, 0, outLen);

      /* convert the Unicode text to UTF8 */
      int uniLen = 0;
      uniLen = WideCharToMultiByte(CP_OEMCP,
                                   0,
                                   uniStr,
                                   wcslen(uniStr),
                                   utfStr,
                                   outLen,
                                   NULL, NULL);

      return(uniLen);
   };
};

#endif
