/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <arch.h>
#include "GpsSatelliteInfo.h"

namespace isab {

GpsSatInfoHolder::GpsSatInfoHolder(Buffer *buf)
{
   m_satInfo = 0;
   readFromBuffer(buf);
}

GpsSatInfoHolder::~GpsSatInfoHolder()
{
   deleteSatInfo();
}

void
GpsSatInfoHolder::deleteSatInfo()
{
   if (m_satInfo) {
      GpsSatInfoVector::iterator it = m_satInfo->begin();
      while (it != m_satInfo->end()) {
         delete *it;
         it++;
      }
      m_satInfo->clear();
      delete m_satInfo;
      m_satInfo = 0;
   }
}

void
GpsSatInfoHolder::readFromBuffer(Buffer *buf)
{
   if (m_satInfo) {
      deleteSatInfo();
   }

   m_satInfo = new GpsSatInfoVector();
   GpsSatInfo *info;
   int32 numInfo = buf->readNextUnaligned32bit();
   for (int32 i = 0; i < numInfo; i++) {
      int32 id = buf->readNextUnaligned32bit();
      int32 el = buf->readNextUnaligned32bit();
      int32 az = buf->readNextUnaligned32bit();
      int32 sn = buf->readNextUnaligned32bit();
      info = new GpsSatInfo( id, el, az, sn);
      m_satInfo->push_back(info);
   }
}

void
GpsSatInfoHolder::writeToBuffer(Buffer *buf)
{
   if (m_satInfo) {
      buf->writeNextUnaligned32bit(m_satInfo->size());

      GpsSatInfoVector::iterator it = m_satInfo->begin();
      while (it != m_satInfo->end()) {
         GpsSatInfo *info = *it;
         buf->writeNextUnaligned32bit(info->m_id);
         buf->writeNextUnaligned32bit(info->m_elevation);
         buf->writeNextUnaligned32bit(info->m_azimuth);
         buf->writeNextUnaligned32bit(info->m_snr);
         it++;
      }
   } else {
      buf->writeNextUnaligned32bit(0);
   }
}

}

