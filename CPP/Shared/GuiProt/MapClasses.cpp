/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "GuiProt/MapClasses.h"
#include <math.h>
#include "Constants.h"
#include <algorithm>

//defeat microsoft macros.
#undef max
#undef min


namespace isab{
   using namespace MapEnums;

   class BoundingBox* BoundingBox::deserialize(Buffer* buf)
   {
      enum BoxType type = BoxType(buf->readNextUnaligned16bit());
      switch(type){
         case boxBox:
            return new BoxBox(buf);
         case vectorBox:
            return new VectorBox(buf);
         case diameterBox:
            return new DiameterBox(buf);
         case routeBox:
            return new RouteBox(buf);
         case invalidBox:
            break;
      }
      buf->jumpReadPos(-2);
      return NULL;
   }

   const BoundingBox::BoxType BoxBox::boxType      = BoundingBox::boxBox;
   const BoundingBox::BoxType VectorBox::boxType   = BoundingBox::vectorBox;
   const BoundingBox::BoxType DiameterBox::boxType = BoundingBox::diameterBox;
   const BoundingBox::BoxType RouteBox::boxType    = BoundingBox::routeBox;


   BoxBox::BoxBox(int32 topLat, int32 leftLon, 
                  int32 bottomLat, int32 rightLon) :
      m_topLat(topLat), m_leftLon(leftLon), 
      m_bottomLat(bottomLat), m_rightLon(rightLon) 
   {}

   BoxBox::BoxBox(Buffer* buf) :
      m_topLat(MAX_INT32), m_leftLon(MAX_INT32), 
      m_bottomLat(MAX_INT32), m_rightLon(MAX_INT32) 
   {
      m_topLat    = buf->readNextUnaligned32bit();
      m_leftLon   = buf->readNextUnaligned32bit();
      m_bottomLat = buf->readNextUnaligned32bit();
      m_rightLon  = buf->readNextUnaligned32bit();
   }

   BoundingBox* BoxBox::clone() const
   {
      return new BoxBox(*this);
   }

   void BoxBox::serialize(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(boxType);
      buf->writeNextUnaligned32bit(m_topLat);
      buf->writeNextUnaligned32bit(m_leftLon);
      buf->writeNextUnaligned32bit(m_bottomLat);
      buf->writeNextUnaligned32bit(m_rightLon);
   }

   int32 BoxBox::getTopLat() const
   {
      return m_topLat;
   }

   int32 BoxBox::getLeftLon() const
   {
      return m_leftLon;
   }

   int32 BoxBox::getBottomLat() const
   {
      return m_bottomLat;
   }

   int32 BoxBox::getRightLon() const
   {
      return m_rightLon;
   }
   
   bool BoxBox::Contains(int32 lat, int32 lon) const
   {
      return lat > std::min(m_topLat, m_bottomLat) && 
             lat < std::max(m_topLat, m_bottomLat) &&
             lon > std::min(m_leftLon, m_rightLon) &&
             lon < std::max(m_leftLon, m_rightLon);
   }

   BoundingBox::BoxType BoxBox::getBoxType() const
   {
      return boxType;
   }

   bool BoundingBox::Contains(int32 /*lat*/, int32 /*lon*/) const
   { 
      return false;
   }

   VectorBox::VectorBox(int32 lat, int32 lon, uint16 speed, uint8 heading, 
                        uint32 realWorldWidth) :
      m_lat(lat), m_lon(lon),  m_speed(speed),  m_hdg(heading), 
      m_realWorldWidth(realWorldWidth)
   {}

   VectorBox::VectorBox(Buffer* buf) :
      m_lat(MAX_INT32), m_lon(MAX_INT32),  m_speed(0),  m_hdg(0), 
      m_realWorldWidth(0)
   {
      m_lat            = buf->readNextUnaligned32bit();
      m_lon            = buf->readNextUnaligned32bit();
      m_speed          = buf->readNextUnaligned16bit();
      m_hdg            = buf->readNext8bit();
      buf->jumpReadPos(1);
      m_realWorldWidth = buf->readNextUnaligned32bit();

   }

   BoundingBox* VectorBox::clone() const
   {
      return new VectorBox(*this);
   }

   void VectorBox::serialize(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(boxType);
      buf->writeNextUnaligned32bit(m_lat);
      buf->writeNextUnaligned32bit(m_lon);
      buf->writeNextUnaligned16bit(m_speed);
      buf->writeNext8bit(m_hdg);
      buf->writeNext8bit(0);
      buf->writeNextUnaligned32bit(m_realWorldWidth);
   }

   int32 VectorBox::getLat() const
   {
      return m_lat;
   }

   int32 VectorBox::getLon() const
   {
      return m_lon;
   }

   uint16 VectorBox::getSpeed() const
   {
      return m_speed;
   }

   uint8 VectorBox::getHdg() const
   {
      return m_hdg;
   }

   uint32 VectorBox::getRealWorldWidth() const
   {
      return m_realWorldWidth;
   }


   BoundingBox::BoxType VectorBox::getBoxType() const
   {
      return boxType;
   }

   DiameterBox::DiameterBox(int32 lat, int32 lon, uint32 diameter) :
      m_lat(lat), m_lon(lon), m_diameter(diameter) 
   {}

   DiameterBox::DiameterBox(Buffer* buf) :
      m_lat(MAX_INT32), m_lon(MAX_INT32), m_diameter(0) 
   {
      m_lat      = buf->readNextUnaligned32bit();
      m_lon      = buf->readNextUnaligned32bit();
      m_diameter = buf->readNextUnaligned32bit();
      buf->jumpReadPos(4);
   }

   BoundingBox* DiameterBox::clone() const
   {
      return new DiameterBox(*this);
   }

   void DiameterBox::serialize(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(boxType);
      buf->writeNextUnaligned32bit(m_lat);
      buf->writeNextUnaligned32bit(m_lon);
      buf->writeNextUnaligned32bit(m_diameter);
      buf->writeNextUnaligned32bit(0);
   }

   int32 DiameterBox::getLat() const
   {
      return m_lat;
   }

   int32 DiameterBox::getLon() const
   {
      return m_lon;
   }

   uint32 DiameterBox::getDiameter() const
   {
      return m_diameter;
   }


   BoundingBox::BoxType DiameterBox::getBoxType() const
   {
      return boxType;
   }

   bool DiameterBox::Contains(int32 lat, int32 lon) const
   { 
      const double dlat = double(lat) / 1e8;
      const double dlon = double(lon) / 1e8;
      const double xrad = Constants::earthRadius * cos(double(m_lat) / 1e8);
      const double xcirc = xrad * 2 * Constants::pi;
      const double ycirc = Constants::earthCircumference;
      const double xfrac = 2*Constants::pi * m_diameter / xcirc;
      const double yfrac = 2*Constants::pi * m_diameter / ycirc;
      const bool xcont = fabs(dlat - (m_lat / 1e8)) < (yfrac/2);
      const bool ycont = fabs(dlon - (m_lon / 1e8)) < (xfrac/2);
      return ycont && xcont;
   }

   RouteBox::RouteBox(int64 routeid) :
      m_routeid(routeid) 
   {}

   RouteBox::RouteBox(Buffer* buf) :
      m_routeid(0) 
   {
      m_routeid = buf->readNextUnaligned64bit();
      buf->jumpReadPos(8);
   }

   BoundingBox* RouteBox::clone() const
   {
      return new RouteBox(*this);
   }

   void RouteBox::serialize(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(boxType);
      buf->writeNextUnaligned64bit(m_routeid);
      buf->writeNextUnaligned64bit(0);
   }

   int64 RouteBox::getRouteid() const
   {
      return m_routeid;
   }

   BoundingBox::BoxType RouteBox::getBoxType() const
   {
      return boxType;
   }

   MapItem::MapItem(enum MapEnums::MapItemType type) :
      m_type(type)
   {}

   MapItem::MapItem(const MapItem& mi) :
      m_type(mi.m_type)
   {}

   MapItem* MapItem::deserialize(Buffer* buf)
   {
      using namespace MapEnums;
      enum MapItemType type = MapItemType(buf->readNextUnaligned16bit());
      MapItem* ret = NULL;
      switch(type){
         case MapRouteItem: 
            ret = new RouteItem(buf);
            ret->m_type = type;
            break;
         case MapSearchItem:
         case MapDestinationItem:
         case MapUserPositionItem:
            ret = new PositionItem(buf);
            ret->m_type = type;
            break;
         case InvalidMapItemType:
            break;
      }
      if(!ret){
         buf->jumpReadPos(-2);
      }
      return ret; 
   }

   enum MapEnums::MapItemType MapItem::getType() const
   {
      return m_type;
   }

   void MapItem::serialize(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(m_type);
   }

   PositionItem::PositionItem(enum MapEnums::MapItemType type, 
                              int32 lat, int32 lon) :
      MapItem(type), m_lat(lat), m_lon(lon) 
   {}

   PositionItem::PositionItem(Buffer* buf) :
      m_lat(MAX_INT32), m_lon(MAX_INT32) 
   {
      m_lat = buf->readNextUnaligned32bit();
      m_lon = buf->readNextUnaligned32bit();
   }

   MapItem* PositionItem::clone() const
   {
      return new PositionItem(*this);
   }

   void PositionItem::serialize(Buffer* buf) const
   {
      MapItem::serialize(buf);
      buf->writeNextUnaligned32bit(m_lat);
      buf->writeNextUnaligned32bit(m_lon);
   }

   int32 PositionItem::getLat() const
   {
      return m_lat;
   }

   int32 PositionItem::getLon() const
   {
      return m_lon;
   }

   RouteItem::RouteItem(int64 routeid) :
      MapItem(MapEnums::MapRouteItem), m_routeid(routeid) 
   {}

   RouteItem::RouteItem(Buffer* buf) :
      m_routeid(0) 
   {
      m_routeid = buf->readNextUnaligned64bit();
   }

   MapItem* RouteItem::clone() const
   {
      return new RouteItem(*this);
   }

   void RouteItem::serialize(Buffer* buf) const
   {
      MapItem::serialize(buf);
      buf->writeNextUnaligned64bit(m_routeid);
   }

   int64 RouteItem::getRouteid() const
   {
      return m_routeid;
   }

   ExtraMapInfo::ExtraMapInfo(enum MapEnums::MapInfoType type, uint32 payload):
      m_type(type), m_payload(payload) 
   {}

   ExtraMapInfo::ExtraMapInfo(Buffer* buf) :
      m_type(MapEnums::InvalidMapInfoType), m_payload(0) 
   {
      m_type = MapEnums::MapInfoType(buf->readNextUnaligned16bit());
      m_payload = buf->readNextUnaligned32bit();
   }

   void ExtraMapInfo::serialize(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(m_type);
      buf->writeNextUnaligned32bit(m_payload);
   }

   enum MapEnums::MapInfoType ExtraMapInfo::getType() const
   {
      return m_type;
   }

   uint32 ExtraMapInfo::getPayload() const
   {
      return m_payload;
   }


}
