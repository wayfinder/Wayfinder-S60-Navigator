/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ROUTEPOSITIONDATA_H
#define ROUTEPOSITIONDATA_H

namespace isab {

class PositionObject {
public:
   PositionObject();
   PositionObject(uint8 type,
         int32 lat, int32 lon,
         const char *id, const char *name);
   ~PositionObject();

   uint8 getType() { return m_type; }
   int32 getLat() { return m_lat; }
   int32 getLon() { return m_lon; }
   const char *getName() { return m_name; }
   const char *getId() { return m_id; }

   void setType(uint8 type) { m_type = type; }
   void setLat(int32 lat) { m_lat=lat; }
   void setLon(int32 lon) { m_lon=lon; }
   void setName(const char *name);
   void setId(const char *id);

   void setPositionOk(uint8 ok) { m_position_set = ok; }
   uint8 positionOk() { return m_position_set; }

   uint8 m_position_set;
   uint8 m_type;
   int32 m_lat;
   int32 m_lon;
   char *m_id;
   char *m_name;
};

class RoutePositionData
{
public:
   RoutePositionData();
   RoutePositionData(const char *destName,
         int32 orig_lat, int32 orig_lon,
         int32 dest_lat, int32 dest_lon);
   ~RoutePositionData();

   int32 getoLat() { return m_origin->getLat(); }
   int32 getoLon() { return m_origin->getLon(); }
   int32 getdLat() { return m_dest->getLat(); }
   int32 getdLon() { return m_dest->getLon(); }
   const char *getDestinationName() { return m_dest->getName(); }

   void setoType(uint8 type) { m_origin->setType(type); }
   void setdType(uint8 type) { m_dest->setType(type); }

   void setoLat(int32 lat) { m_origin->setLat(lat); }
   void setoLon(int32 lon) { m_origin->setLon(lon); }
   void setdLat(int32 lat) { m_dest->setLat(lat); }
   void setdLon(int32 lon) { m_dest->setLon(lon); }

   void setoName(const char *name) { m_origin->setName(name); }
   void setdName(const char *name) { m_dest->setName(name); }
   void setoId(const char *id) { m_origin->setId(id); }
   void setdId(const char *id) { m_dest->setId(id); }

   void setoPositionOk(uint8 ok) { m_origin->setPositionOk(ok); }
   void setdPositionOk(uint8 ok) { m_dest->setPositionOk(ok); }

   uint8 getoPositionOk() { return m_origin->positionOk(); }
   uint8 getdPositionOk() { return m_dest->positionOk(); }

   uint16 m_req_type;
   uint16 m_req_id;
   PositionObject *m_origin;
   PositionObject *m_dest;
};

}
#endif /* ROUTEPOSITIONDATA_H */
