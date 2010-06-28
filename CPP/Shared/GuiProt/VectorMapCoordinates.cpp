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
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/VectorMapCoordinates.h"
#include "GuiProt/GuiParameterEnums.h"
#include "Buffer.h"

namespace isab{
#define VM_COORD_VERSION 2

VectorMapCoordinates::VectorMapCoordinates(Buffer* buf)
   : m_scale(0), m_lat(0), m_lon(0), m_valid(false)
{
   if (buf->remaining() >= 4 && buf->readNext32bit() != VM_COORD_VERSION) {
      /* Wrong version. */
      return;
   }

   if (buf->remaining() >= 12) {
      m_scale = buf->readNext32bit();
      m_lon = buf->readNext32bit();
      m_lat = buf->readNext32bit();
      m_valid = true;
   }
   buf->releaseData();
   delete buf;
}

VectorMapCoordinates::~VectorMapCoordinates()
{
}

VectorMapCoordinates::VectorMapCoordinates(int32 scale, int32 lat, int32 lon)
{
   m_scale = scale;
   m_lat = lat;
   m_lon = lon;
   m_valid = true;
}

GeneralParameterMess*
VectorMapCoordinates::CreateGeneralParameter()
{
   if (!Valid()) {
      return NULL;
   }

   Buffer *buf = new Buffer(20);                                

   buf->writeNext32bit(VM_COORD_VERSION); /* Version. */
   buf->writeNext32bit(m_scale);
   buf->writeNext32bit(m_lon);
   buf->writeNext32bit(m_lat);

   const uint8 *rawdata = buf->accessRawData();
   int32 len = buf->getLength();

   GeneralParameterMess* gpm = new GeneralParameterMess(
         GuiProtEnums::paramVectorMapCoordinates,
         (uint8 *)rawdata, len);

   return gpm;
}

}
