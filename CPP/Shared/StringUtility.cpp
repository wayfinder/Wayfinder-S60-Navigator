/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "StringUtility.h"

#include <string.h>
#include <stdio.h>

int
StringUtility::URLEncode(char* outbuf,
                         const char* inbuf)
{
   int len = URLEncode(outbuf, inbuf, strlen(inbuf));
   outbuf[len] = '\0';
   return len;
}

int
StringUtility::URLEncode(char* outbuf,
                         const char* inbuf,
                         int inlength)
{
   uint32 pos = 0;
   uint32 outpos = 0;
   char cstr[44];
   byte ch;

   while ( inlength -- ) {
      ch = inbuf[pos++];
      if ( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
           (ch >= '0' && ch <= '9') || (ch >= '-' && ch <= '.') ||
           ( ch == '_' )) {
         // Ordinary character
         // Append to outbuf
         outbuf[outpos++] = (char)ch;
      } else if ( ch == 32 ) { // ASCII 32 == ' '
         // Space - special
         outbuf[outpos++] = '+';
      } else {
         // Even more special.
         sprintf(cstr, "%%%02X", ch);
         int cstrlen = 3; // strlen(cstr); // Should be constant really.
         memcpy(&outbuf[outpos], cstr, cstrlen);
         outpos += cstrlen;
      }
   }   
   return outpos;
}


int 
StringUtility::URLDecode(byte* outbuf,
                         const char* inbuf)
{
   uint32 pos = 0;
   uint32 outPos = 0;
   uint32 result = 0;   

   while ( inbuf[pos] != '\0' ) {
      if ( inbuf[pos] == '%' ) {
         switch ( inbuf[pos + 1] ) {
            case 'A' : result = 10; break;
            case 'B' : result = 11; break;
            case 'C' : result = 12; break;
            case 'D' : result = 13; break;
            case 'E' : result = 14; break;
            case 'F' : result = 15; break;
            case 'a' : result = 10; break;
            case 'b' : result = 11; break;
            case 'c' : result = 12; break;
            case 'd' : result = 13; break;
            case 'e' : result = 14; break;
            case 'f' : result = 15; break;
            default: result = inbuf[pos + 1] - 48;
         }
         result = result*16;
         switch ( inbuf[pos + 2] ) { 
            case 'A' : result += 10; break;
            case 'B' : result += 11; break;
            case 'C' : result += 12; break;
            case 'D' : result += 13; break;
            case 'E' : result += 14; break;
            case 'F' : result += 15; break;
            case 'a' : result += 10; break;
            case 'b' : result += 11; break;
            case 'c' : result += 12; break;
            case 'd' : result += 13; break;
            case 'e' : result += 14; break;
            case 'f' : result += 15; break;
            default: result = result + inbuf[pos + 2] - 48;
         }
         outbuf[outPos++] = (char)result; // Works 0-255.
         // '%' and first number, last number handled normaly at end
         pos += 2;
      }
      else if ( inbuf[pos] == '+' ) {
         outbuf[outPos++] = ' ';
      } else {
         outbuf[outPos++] = inbuf[pos];
      }
      pos++;
   }

   return outPos;
}

#define MC2_ASSERT(A)

char* 
StringUtility::replaceString(const char* source,
                             const char* searchFor, 
                             const char* replaceWith,
                             int maxReplace)
{           
   MC2_ASSERT(source != NULL);                         
   MC2_ASSERT(searchFor != NULL);
   int ocurrences = 0;
   if(strstr(source, searchFor) != NULL) {
      // count the ocurrences
      char* s = (char*)source;
      while((s = strstr(s, searchFor)) != NULL) {
         ++ocurrences;
         ++s;
      }  
   
      // check maximum replace count
      if ((maxReplace > 0) && (ocurrences > maxReplace))
         ocurrences = maxReplace;
      
      // check replacement string                               
      const char* repl = "";                                          
      if (replaceWith != NULL)
         repl = replaceWith;

      // allocate new string
      uint32 newLength = strlen(source) + ocurrences * strlen(repl);
      char* newString = new char[newLength];
      newString[0] = '\0';

      // build the new string from source and repl, using ocurrences as the max
      char* found = (char*)source;
      char* last = (char*)source;
      int searchLen = strlen(searchFor);
      while ( (ocurrences > 0) && ((found = strstr(last, searchFor)) != NULL)) {
         strncat(newString, last, found - last);
         strcat(newString, repl);
         found += searchLen;
         last = found;
         --ocurrences;
      }
      // add the last part
      if (last < source + strlen(source))
         strcat(newString, last);

      return newString;
   } else {
      return NULL;
   }
}

char*
StringUtility::newStrDup(const char* s)
{
   char* theDupe = NULL;
   if (s) {
      theDupe = new char[strlen(s) + 1];
      strcpy( theDupe, s );
   }
   return theDupe;
}

