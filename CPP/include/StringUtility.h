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

class StringUtility {
   public:

      /**
       *  URLEncodes the zero-terminated inbuf into outbuf,
       *  which will also be zero-terminated. Each input char
       *  can result in three output chars.
       *  @param outbuf The result. Should be 3*strlen(inbuf)+1 long.
       *  @param inbuf  The buffer to encode.
       *  @return The length of the written string.
       *
       *  NOTE: This method is used in both MC2 and Nav2.
       *        Please change both if bugs are found.
       */
      static int URLEncode(char* outbuf,
                           const char* inbuf);

      /**
       *  URLEncodes the inbuf into outbuf. Outbuf will not be 
       *  zero terminated.
       *  @param outbuf   The result. Should be 3*inlength long.
       *  @param inbuf    The buffer to encode.
       *  @param inlength The length of the inbuffer.
       *  @return Number of bytes written to outbuf.
       *
       *  NOTE: This method is used in both MC2 and Nav2.
       *        Please change both if bugs are found.
       */
      static int URLEncode(char* outbuf,
                           const char* inbuf,
                           int inlength);

      /**
       * URLDecodes the inbuf onto outbuf. Outbuf will not be 
       * zero terminated.
       *  @param outbuf   The result. Should be inlength long.
       *  @param inbuf    The buffer to decode.
       *  @return Number of bytes written to outbuf.
       *
       *  NOTE: This method is used in both MC2 and Nav2.
       *        Please change both if bugs are found.
       */
      static int URLDecode(byte* outbuf,
                           const char* inbuf);

      /**
       *  Replaces strings in strings. A new string or NULL is returned,
       *  the original string is not deleted, the caller has to delete
       *  the new string when done.
       *  @param source      The original string
       *  @param searchFor   The string to replace
       *  @param replaceWith The string to replace with. Can be NULL or just           *                     "".
       *  @param maxReplace  The maximum number of strings to replace, 0 means
       *                     replace all ocurrences (also default).
       *  @return Pointer to a new string with all replacements done, NULL if no
       *          replacements were made;
       *
       *  NOTE: This method is used in both MC2 and Nav2.                              *        Please change both if bugs are found.
       */
      static char* replaceString(const char* source,
                                 const char* searchFor,
                                 const char* replaceWith,
                                 int maxReplace = 0 );

      /**
       *
       */
      static char* newStrDup(const char* s);

};
