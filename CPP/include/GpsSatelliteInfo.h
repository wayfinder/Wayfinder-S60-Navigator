/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/


#ifndef GPS_SATELLITE_INFO_H
#define GPS_SATELLITE_INFO_H

#include "Buffer.h"
#include <vector>


namespace isab{

class GpsSatInfo {
   public:
      GpsSatInfo(int32 id, int32 elevation, int32 azimuth, int32 snr) :
         m_id(id),
         m_elevation(elevation),
         m_azimuth(azimuth),
         m_snr(snr) {}

      int32 m_id;
      int32 m_elevation;
      int32 m_azimuth;
      int32 m_snr;
};
typedef std::vector<class GpsSatInfo *> GpsSatInfoVector;

class GpsSatInfoHolder {
   public:
/*    GpsSatInfoHolder(GpsSatInfoVector *satInfo) : m_satInfo(satInfo) {} */
   GpsSatInfoHolder() : m_satInfo(0) {}
   GpsSatInfoHolder(Buffer *buf);

   ~GpsSatInfoHolder();

   void readFromBuffer(Buffer *buf);
   void writeToBuffer(Buffer *buf);

   void addInfo(GpsSatInfo *item) { if (!m_satInfo) { m_satInfo = new GpsSatInfoVector(); } m_satInfo->push_back(item); }
   uint32 size() { if (m_satInfo) { return m_satInfo->size(); } return 0; }

   private:
   void deleteSatInfo();

   private:
   GpsSatInfoVector *m_satInfo;
};
}

#endif  /* GPS_SATELLITE_INFO_H */
