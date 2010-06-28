/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef UTF8UTIL_H
#define UTF8UTIL_H

class UTF8Util {
public:
   /**
    *    Writes utf-8 encoded string into res from str ISO encoding.
    *    @param str is the iso encoded string
    *    @param outlen The length of the encoded string excluding zero
    *                  termination. Optional.
    *    @param inlen  The length of the input string. Optional.
    *    @return New char* encoded in UTF8. Will be twice as long
    *            as the original.
    */
   static char* isoToUtf8L( const char* str,
                           int* outlen = NULL,
                           int* inlen  = NULL );

   /**
    *  Converts a unicode character to a utf-8 string.
    *  The utf-8 string must be at least 7 characters 
    *  long.
    *  @param unicode The unicode character.
    *  @param utf8    Destination string.
    *  @return Number of bytes added to <code>utf8</code> excl zero term.
    */   
   static int unicodeToUtf8(uint32 unicode, char* utf8);

   /**
    *
    * Append the src string to dest, reallocating dest if
    * src does not fit in left bytes.
    */
   static void appendUtf8(char **dest, uint32 *left, char *src);

   /**
    *   Converts a string that can be utf-8 or latin-1 into
    *   utf-8. Does this by assuming that all invalid combinations
    *   are latin-1.
    */
   static char* cleanUtf8( const char* instr );

   static int strCollateUtf8( const char* str1, const char* str2);

};

#endif
