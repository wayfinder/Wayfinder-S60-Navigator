/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "nav2util.h"

#include <stdlib.h>

namespace isab{
   char* replaceString(char*& dst, const char* src)
   {
      int dstlen = dst ? strlen(dst) : 0;
      int srclen = strlen(src);
      if(dstlen < srclen){
         delete[] dst;
         dst = NULL;
      }
      if(!dst){
         dst = new char[srclen + 1];
      }
      return strcpy(dst, src);
   }

   char* strsep(char**stringp, const char* delim)
   {
      char* token = *stringp;
      if(*stringp){
         size_t tokenLen = strcspn(*stringp, delim);
         *stringp += tokenLen;
         if(**stringp != '\0'){
            //found a delim
            **stringp = '\0';
            *stringp += 1;
         } else {
            //no delim
            *stringp = NULL;
         }
      }
      return token;
   }
   int char2unicode(int n, uint16* dst, const char* src)
   {
      int i = 0;
      while((i++ < n) && (0 != (*dst++ = *src++))) { 
      }
      if(n == i){
         dst[n] = 0;
      }
      return i;
   }

   bool parseHost(char* arg, char** host, uint16* port)
   {
      bool ret = arg && host && port; //no arg is null
      if(ret){
         // find the ':' character in the string.
         char* colon = strchr(arg, ':');
         if(!colon || colon == arg){ //no ':' found, or ':' first in string.
            colon = arg; //':' first in arg is an error, which will show later.
         } else {
            *colon++ = '\0'; //erase ':' and separate strings.
         }
         // read the port number
         char* endptr = NULL;
         unsigned long lport = strtoul(colon, &endptr, 10);
         //check that a conversion was made, and that it is a valid port number
         ret = (endptr != colon) && (lport > 1) && (lport < MAX_UINT16);
         if(ret){
            //Parsed OK, write return values.
            *port = 0x0ffff & lport;
            if(colon != arg){
               *host = arg;
            }
         } else if(colon != arg){
            //the arg couldn't be parsed, restore arg.
            *--colon = '\0'; // XXX: Shouldn't this then be ':'?
         }
      }
      return ret;
   }

   bool parseVersion(uint32 target[3], const char* versionString)
   {
      char* end = NULL;
      for(int i = 0; i < 3; ++i){
         target[i] = strtoul(versionString, &end, 10);
         if(end && *end == '.'){
            versionString = end + 1;
         } else if(i < 2) {
            return false;
         }
      }
      return true;
   }

   char* strdup_new(const char* src)
   {
      char* result = NULL;
      if(src){
         if((result = new char[strlen(src) + 1])){
            strcpy(result, src);
         }
      }
      return result;
   }


#if defined(_MSC_VER) && !defined(__SYMBIAN32__)
   int dupChar2Tchar(const char* original, tchar*& duplicate){
      if(original){
         int len = strlen(original);
         const char* tmp = original;
         int cnt = 0;
         while(NULL != (tmp = strchr(tmp, '\n'))){
            ++cnt;
         }
         duplicate = new tchar[len + 1 + cnt];
         tmp = original;
         tchar* ttmp = duplicate;
         while(*tmp){
            if(*tmp != '\n'){
               *ttmp++ = *tmp++;
            } else {
               *ttmp++ = '\r';
               *ttmp++ = *tmp++;
            }
         }
         *ttmp++ = '\0';
         return ttmp - duplicate;
      }
      return -1;
   }
#endif

}
