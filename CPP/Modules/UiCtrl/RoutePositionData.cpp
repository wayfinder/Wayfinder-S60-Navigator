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
#include "RoutePositionData.h"
#include "GuiProt/ServerEnums.h"

namespace isab {

using namespace GuiProtEnums;

PositionObject::PositionObject()
{
   m_position_set = 0;
   m_type = PositionTypeInvalid;
   m_lat = MAX_INT32;
   m_lon = MAX_INT32;
   m_id = NULL;
   m_name = NULL;
}
PositionObject::PositionObject(uint8 /*type*/,
      int32 lat, int32 lon,
      const char *id, const char *name)
{
   m_position_set = 0;
   m_type = PositionTypePosition;
   m_lat = lat;
   m_lon = lon;
   m_id = NULL;
   m_name = NULL;
   if (id) {
      m_id = strdup_new(id);
   }
   if (name) {
      m_name = strdup_new(name);
   }
}

void
PositionObject::setName(const char *name)
{
   delete[] m_name;
   if (name) {
      m_name = strdup_new(name);
   } else {
      m_name = NULL;
   }
}

void
PositionObject::setId(const char *id)
{
   delete[] m_id;
   if (id) {
      m_id = strdup_new(id);
   } else {
      m_id = NULL;
   }
}

PositionObject::~PositionObject()
{
   delete[] m_id;
   delete[] m_name;
}

RoutePositionData::RoutePositionData()
{
   m_origin = new PositionObject();
   m_dest = new PositionObject();
}

RoutePositionData::RoutePositionData(const char *destName,
      int32 orig_lat, int32 orig_lon,
      int32 dest_lat, int32 dest_lon)
{
   m_origin = new PositionObject(PositionTypePosition,
         orig_lat, orig_lon,
         NULL, NULL);
   m_dest = new PositionObject(PositionTypePosition,
         dest_lat, dest_lon,
         NULL, destName);
}

RoutePositionData::~RoutePositionData()
{
   if (m_origin) {
      delete m_origin;
   }
   if (m_dest) {
      delete m_dest;
   }
}

}
