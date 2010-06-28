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
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtRouteMess.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtMapMess.h"

namespace isab{
   // RouteMess ///////////////////////////////
   RouteMess::RouteMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_oType = (GuiProtEnums::PositionType)buf->readNext8bit();
      m_oId = buf->getNextCharString();
      m_oLat = buf->readNextUnaligned32bit();
      m_oLon = buf->readNextUnaligned32bit();
      m_dType = (GuiProtEnums::PositionType)buf->readNext8bit();
      m_dId = buf->getNextCharString();
      m_dLat = buf->readNextUnaligned32bit();
      m_dLon = buf->readNextUnaligned32bit();
      m_destName = buf->getNextCharString();
   }

   RouteMess::RouteMess(GuiProtEnums::PositionType d_type, char *d_id,
         int32 d_lat, int32 d_lon,
         char *destName) :
      GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::ROUTE_MESSAGE)
   {
      m_oType = GuiProtEnums::PositionTypeCurrentPos;
      m_oId = "";
      m_oLat = MAX_INT32;
      m_oLon = MAX_INT32;
      m_dType = d_type;
      m_dId = d_id;
      m_dLat = d_lat;
      m_dLon = d_lon;
      m_destName = destName;
   }

   RouteMess::RouteMess(GuiProtEnums::PositionType o_type, char *o_id,
         int32 o_lat, int32 o_lon,
         GuiProtEnums::PositionType d_type, char *d_id,
         int32 d_lat, int32 d_lon,
         char *destName) :
      GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::ROUTE_MESSAGE)
   {
      m_oType = o_type;
      m_oId = o_id;
      m_oLat = o_lat;
      m_oLon = o_lon;
      m_dType = d_type;
      m_dId = d_id;
      m_dLat = d_lat;
      m_dLon = d_lon;
      m_destName = destName;
   }

   RouteMess::~RouteMess()
   {
   }

   void RouteMess::deleteMembers()
   {
   }

   void RouteMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNext8bit(m_oType);
      buf->writeNextCharString(m_oId);
      buf->writeNextUnaligned32bit(m_oLat);
      buf->writeNextUnaligned32bit(m_oLon);
      buf->writeNext8bit(m_dType);
      buf->writeNextCharString(m_dId);
      buf->writeNextUnaligned32bit(m_dLat);
      buf->writeNextUnaligned32bit(m_dLon);
      buf->writeNextCharString(m_destName);
   }

   const char* RouteMess::getoId() const
   {
      return m_oId;
   }
   const char* RouteMess::getdId() const
   {
      return m_dId;
   }
   const char* RouteMess::getDestinationName() const
   {
      return m_destName;
   }
   GuiProtEnums::PositionType RouteMess::getoType() const
   {
      return m_oType;
   }
   GuiProtEnums::PositionType RouteMess::getdType() const
   {
      return m_dType;
   }
   int32 RouteMess::getoLat() const
   {
      return m_oLat;
   }
   int32 RouteMess::getoLon() const
   {
      return m_oLon;
   }
   int32 RouteMess::getdLat() const
   {
      return m_dLat;
   }
   int32 RouteMess::getdLon() const
   {
      return m_dLon;
   }

   // RouteToPositionMess ///////////////////////////////
   RouteToPositionMess::RouteToPositionMess(Buffer* buf) :
      GuiProtMess(buf)
      {
         m_destinationName = buf->getNextCharString();
         m_lat = buf->readNextUnaligned32bit();
         m_lon = buf->readNextUnaligned32bit();
      }

   RouteToPositionMess::RouteToPositionMess(const char* destination, 
         int32 lat, int32 lon) :
      GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::ROUTE_TO_POSITION),
   m_destinationName(destination), m_lat(lat), m_lon(lon)
   {
   }

   RouteToPositionMess::~RouteToPositionMess()
   {
   }

   void RouteToPositionMess::deleteMembers()
   {
   }


   void RouteToPositionMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNextCharString(m_destinationName);
      buf->writeNextUnaligned32bit(m_lat);
      buf->writeNextUnaligned32bit(m_lon);
   }


   const char* RouteToPositionMess::getDestinationName() const
   {
      return m_destinationName;
   }


   int32 RouteToPositionMess::getLat() const
   {
      return m_lat;
   }

   int32 RouteToPositionMess::getLon() const
   {
      return m_lon;
   }
   // StartedNewRouteMess ///////////////////////////////
   StartedNewRouteMess::StartedNewRouteMess(Buffer* buf) :
      GuiProtMess(buf)
      {
         m_routeid = buf->readNextUnaligned64bit();
         m_tLat = buf->readNextUnaligned32bit();
         m_lLon = buf->readNextUnaligned32bit();
         m_bLat = buf->readNextUnaligned32bit();
         m_rLon = buf->readNextUnaligned32bit();
         m_oLat = buf->readNextUnaligned32bit();
         m_oLon = buf->readNextUnaligned32bit();
         m_dLat = buf->readNextUnaligned32bit();
         m_dLon = buf->readNextUnaligned32bit();
         m_destination = buf->getNextCharString();
      }

   StartedNewRouteMess::StartedNewRouteMess(int64 routeid,
                                            int32 topLat, int32 leftLon,
                                            int32 bottomLat, int32 rightLon, 
                                            int32 oLat, int32 oLon,
                                            int32 dLat, int32 dLon,
                                            const char* destination) :
      GuiProtMess(GuiProtEnums::type_and_data,GuiProtEnums::STARTED_NEW_ROUTE),
      m_routeid(routeid), m_tLat(topLat), m_lLon(leftLon), m_bLat(bottomLat), 
      m_rLon(rightLon), m_oLat(oLat), m_oLon(oLon), m_dLat(dLat), m_dLon(dLon),
      m_destination(destination)
   {
   }

   StartedNewRouteMess::~StartedNewRouteMess()
   {
   }

   void StartedNewRouteMess::deleteMembers()
   {
      m_destination = NULL;
   }


   void StartedNewRouteMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNextUnaligned64bit(m_routeid); //id
      buf->writeNextUnaligned32bit(m_tLat); //bounding box
      buf->writeNextUnaligned32bit(m_lLon);
      buf->writeNextUnaligned32bit(m_bLat);
      buf->writeNextUnaligned32bit(m_rLon);
      buf->writeNextUnaligned32bit(m_oLat); //start pos
      buf->writeNextUnaligned32bit(m_oLon);
      buf->writeNextUnaligned32bit(m_dLat); //dest pos
      buf->writeNextUnaligned32bit(m_dLon);
      buf->writeNextCharString(m_destination); //dest name
   }


   int64 StartedNewRouteMess::getRouteId() const
   {
      return m_routeid;
   }

   int32 StartedNewRouteMess::getTopLat() const
   {
      return m_tLat;
   }
   int32 StartedNewRouteMess::getLeftLon() const
   {
      return m_lLon;
   }
   int32 StartedNewRouteMess::getBottomLat() const
   {
      return m_bLat;
   }
   int32 StartedNewRouteMess::getRightLon() const
   {
      return m_rLon;
   }
   int32 StartedNewRouteMess::getoLat() const
   {
      return m_oLat;
   }
   int32 StartedNewRouteMess::getoLon() const
   {
      return m_oLon;
   }
   int32 StartedNewRouteMess::getdLat() const
   {
      return m_dLat;
   }
   int32 StartedNewRouteMess::getdLon() const
   {
      return m_dLon;
   }
   const char* StartedNewRouteMess::getDestination() const
   {
      return m_destination;
   }


}
