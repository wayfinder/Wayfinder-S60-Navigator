/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */


#ifndef WF_NEWS_DATA_H
#define WF_NEWS_DATA_H

#include "arch.h"

/**
 */
class WFNewsData
{
public:
   WFNewsData() :
      m_checksum(0),
      m_shownChecksum(0),
      m_checksumSet(0),
      m_shownChecksumSet(0),
      m_shown(0)
   {}

   ~WFNewsData();

   void setLatestShownNewsChecksum(const char* checksum);

   void setLatestNewsChecksum(const char* checksum);

   bool checksumMatch();

   void setShown() { m_shown = 1; }
   bool shown() { return m_shown; }

   const char* getChecksum() { return m_checksum; }
   const char* getShownChecksum() { return m_shownChecksum; }
private:
   char* m_checksum;
   char* m_shownChecksum;
   uint8 m_checksumSet;
   uint8 m_shownChecksumSet;
   uint8 m_shown;
};

#endif

