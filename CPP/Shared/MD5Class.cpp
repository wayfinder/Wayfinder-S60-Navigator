/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MD5Class.h"
#include <stdio.h>

isab::MD5::MD5() 
{
   Reset();
}

void isab::MD5::Reset()
{
   ::md5_init(&m_md5_state);
}

void isab::MD5::Append(const uint8* data, int size)
{
   ::md5_append(&m_md5_state, data, size);
}

void isab::MD5::Finish()
{
   ::md5_finish(&m_md5_state, m_raw_md5_checksum);
   for(int di = 0; di < 16; ++di){
      sprintf(m_hex_md5_checksum + di * 2, "%02x", m_raw_md5_checksum[di]);
   }
}

uint8* isab::MD5::GetRawMd5sum()
{
   if (!m_raw_md5_checksum) {
      return NULL;
   }
   uint8* raw = new uint8[16];
   memcpy(raw, m_raw_md5_checksum, 16);
   return raw;
}

uint8* isab::MD5::GetRawMd5sum(const uint8* data, int size)
{
   if (m_raw_md5_checksum || m_hex_md5_checksum) {
      Reset();
   }
   Append(data, size);
   Finish();
   return GetRawMd5sum();
}
