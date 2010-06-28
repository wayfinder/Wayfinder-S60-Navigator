/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYSTEMINDEPENDENTTYPES_H
#define SYSTEMINDEPENDENTTYPES_H

#include <string.h>

#define STR2(a) #a
#define STR(a) STR2(a)

///this macro creates a char array containing the string "[file:line]"
///where file is created by the __FILE__ macro and line by the
///__LINE__ macro. The macro is useful when debugging stack dumps.

class Suddare {
 public:
   Suddare( char* buf, int len ) {
      m_buf = buf;
      m_len = len;
   }
   
   ~Suddare() {
      memset( m_buf, 0xbb, m_len );
   }
 private:
   char* m_buf;
   int m_len;
};

/// Highest bit methods
#define REMOVE_UINT32_MSB(a) (uint32(a)&0x7fffffffu)
#define GET_UINT32_MSB(a) (uint32(a)&0x80000000u)
#define TOGGLE_UINT32_MSB(a) (uint32(a)^0x80000000u)
#define SET_UINT32_MSB(a) (uint32(a)|0x80000000u)



#endif // SYSTEMINDEPENDENTTYPES_H
