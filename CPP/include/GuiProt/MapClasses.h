/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef MAPCLASSES_H
#define MAPCLASSES_H
#include "arch.h"
#include "GuiProt/Serializable.h"
#include "MapEnums.h"
#include "Buffer.h"
namespace isab {
   using namespace MapEnums;
   class Buffer;

   ///Abstract superclass to all the boundingbox classes. Each of
   ///BoundingBox's subclasses describe to the server how to calculate
   ///the bounding box of the map image. 
   class BoundingBox: public Serializable {
      ///No assignment semantics for BoundingBoxes.
      const BoundingBox& operator=(const BoundingBox& rhs);
   protected:
      ///Copy constructor. Does nothing since BoundingBox has no members.
      ///Protected to ensure proper use.
      ///@param bb the boundingbox to copy.
      BoundingBox(const BoundingBox&)
      {
      }
      ///Default constructor.
      BoundingBox()
      {
      }

   public:
      ///Enum enumerating the different kinds of bounding box
      ///classes. This is used in serialization.
      enum BoxType{
         invalidBox  = 0,///invalid.
         boxBox      = 1,///See <code>BoxBox</code>.
         vectorBox   = 2,///See <code>VectorBox</code>.
         diameterBox = 3,///See <code>DiameterBox</code>.
         routeBox    = 4,///See <code>RouteBox</code>.
      };
      ///Factory function that creates an instance of one of
      ///BoundingBox's subclasses by deserializing the contents of the
      ///argument buffer.
      ///@param buf pointer to the buffer holding the serialized version of 
      ///           a BoundingBox subclass.
      ///@return an instance of a BoundingBox subclass.
      static BoundingBox* deserialize(Buffer* buf);
      ///Returns the corresponding enum value of the BoundingBox instance.
      ///@return the proper enum value from <code>enum BoxType</code>.
      virtual enum BoxType getBoxType() const = 0;
      ///Copies a BoundingBox without slicing.
      ///@return a copy of the BoundinBox.
      virtual BoundingBox* clone() const = 0;
      ///Tests whether a bounding box contains a certain coordinate.
      ///Certain types of bounding boxes can't determine whether a
      ///coordinate is containd by the box, those boxes shoulf use the
      ///default implementation which returns false.
      ///@param lat the latitude to test.
      ///@param lon the longitude to test.
      ///@return true if the parameter coordinate is contained in 
      ///        the bounding box, false otherwise.
      virtual bool Contains(int32 lat, int32 lon) const;
   };

   ///This type of bounding box tells the server exactly what
   ///coordinates should define the map image. The top right and
   ///bottom left corners define a rectangular shape that should be
   ///included in the map image.
   class BoxBox : public BoundingBox {
      ///The superclass BoundingBox needs access to the private constructor.
      friend class BoundingBox;
      ///Construct a BoxBox instance by decoding it's serialized form.
      BoxBox(Buffer* buf);
   public:
      ///Construct a Boxox instance from two coordinate pairs.
      ///All latitudes and longitudes are specified in WGS-84 radians * 1E8.
      ///@param topLat    the upper bounding latitude.
      ///@param leftLon   the left bounding longitude.
      ///@param bottomLat the lower bounding latitude.
      ///@param rightLon  the right bounding longitude.
      BoxBox(int32 topLat, int32 leftLon, int32 bottomLat, int32 rightLon);
      ///Serializes this object into the parameter buffer.
      ///@param buf the buffer to serialize into.
      virtual void serialize(Buffer* buf) const;
      ///@return the upper bounding latitude.
      int32 getTopLat() const;
      ///@return the left bounding longitude.
      int32 getLeftLon() const;
      ///@return the lower bounding latitude.
      int32 getBottomLat() const;
      ///@return the right bounding longitude.
      int32 getRightLon() const;
      ///@return <code>BoundingBox::boxBox</code>
      virtual enum BoxType getBoxType() const;
      ///@return a copy of this object.
      virtual BoundingBox* clone() const;
      ///Tests whether a bounding box contains a certain coordinate.
      ///@param lat the latitude to test.
      ///@param lon the longitude to test.
      ///@return true if the parameter coordinate is contained in 
      ///        the bounding box, false otherwise.
      virtual bool Contains(int32 lat, int32 lon) const;
   private:
      ///The upper bounding latitude.
      int32 m_topLat;
      ///The left bounding longitude.
      int32 m_leftLon;
      ///The lower bounding latitude.
      int32 m_bottomLat;
      ///The right bounding longitude.
      int32 m_rightLon;
      ///This class's BoxType enum value.
      static const enum BoxType boxType;
   };

   ///This type of bounding box asks the server to calculate a
   ///bounding box so that the map image contains a suitably large
   ///area given the users speed, heading, and position. The goal is
   ///to create a map image that can be used to display the users
   ///position for a long time while still offer enough details to be
   ///useful.
   ///The user may request a real-world-width in meters of the map
   ///image, but the server may ignore it.
   class VectorBox : public BoundingBox {
      ///The superclass BoundingBox needs access to the private constructor.
      friend class BoundingBox;
      ///Construct a VectorBox instance by decoding it's serialized form.
      VectorBox(Buffer* buf);
   public:
      ///Construct a VectorBox instance by defining the members.
      ///Latitudes and longitude are specified in WGS-84 radiand * 1E8.
      ///@param lat user latitude.
      ///@param lon user longitude.
      ///@param speed user speed in m/s * 32.
      ///@param heading user heading as 256 parts of a circle.
      VectorBox(int32 lat, int32 lon, uint16 speed, uint8 heading, 
                uint32 width);
      ///Serializes this object into the parameter buffer.
      ///@param buf the buffer to serialize into.      
      virtual void serialize(Buffer* buf) const;
      ///@return the users latitude.
      int32 getLat() const;
      ///@return the users longitude.
      int32 getLon() const;
      ///@return the users speed in m/s * 32.
      uint16 getSpeed() const;
      ///@return the users heading as 256 parts of a circle.
      uint8 getHdg() const;
      ///@return the specified real world width.
      uint32 getRealWorldWidth() const;
      ///@return <code>BoundingBox::boxBox</code>
      virtual enum BoxType getBoxType() const;
      ///@return a copy of this object.
      virtual BoundingBox* clone() const;
   private:
      ///The users latitude.
      int32 m_lat;
      ///The users longitude.
      int32 m_lon;
      ///The users speed in m/s * 32.
      uint16 m_speed;
      ///The users heading as 256 parts of a circle.
      uint8 m_hdg;
      ///The specified real world width.
      uint32 m_realWorldWidth;
      ///This class's BoxType enum value.
      static const enum BoxType boxType;
   };

   ///This type of bounding box instructs the server to calculate a
   ///bounding box that can hold a circle with a certain center and
   ///diameter.
   class DiameterBox : public BoundingBox {
      ///The superclass BoundingBox needs access to the private constructor.
      friend class BoundingBox;
      ///Constructs a DiameterBox from a serialized representation
      ///@param buf the buffer containing the serialized DiameterBox.
      DiameterBox(Buffer* buf);
   public:
      ///Constructor with all the member variable explicitly defined.
      ///Latitude and longitude are expressed in WGS-84 radians * 1E8.
      ///@param lat the center latitude of the circle.
      ///@param lon the center longitude of the circle.
      ///@param diameter the circlke diameter in meters.
      DiameterBox(int32 lat, int32 lon, uint32 diameter);
      ///Serializes this object into the parameter buffer.
      ///@param buf the buffer to serialize into.      
      virtual void serialize(Buffer* buf) const;
      int32 getLat() const;
      int32 getLon() const;
      uint32 getDiameter() const;
      virtual enum BoxType getBoxType() const;
      virtual BoundingBox* clone() const;
      ///Tests whether a bounding box contains a certain coordinate.
      ///@param lat the latitude to test.
      ///@param lon the longitude to test.
      ///@return true if the parameter coordinate is contained in 
      ///        the bounding box, false otherwise.
      virtual bool Contains(int32 lat, int32 lon) const;
   private:
      int32 m_lat;
      int32 m_lon;
      uint32 m_diameter;
      static const enum BoxType boxType;
   };

   class RouteBox : public BoundingBox {
      ///The superclass BoundingBox needs access to the private constructor.
      friend class BoundingBox;
      ///Construct a RouteBox instance by decoding it's serialized form.
      RouteBox(Buffer* buf);
   public:
      RouteBox(int64 routeid);
      virtual void serialize(Buffer* buf) const;
      int64 getRouteid() const;
      virtual enum BoxType getBoxType() const;
      virtual BoundingBox* clone() const;
   private:
      int64 m_routeid;
      static const enum BoxType boxType;
   };

   class MapItem : public Serializable {
   public:
      enum MapItemType getType() const;
      static MapItem* deserialize(Buffer* buf);
      virtual void serialize(Buffer* buf) const;
      virtual MapItem* clone() const = 0;
   protected:
      MapItem(enum MapItemType type = InvalidMapItemType);
      MapItem(const MapItem& mi);
   private:
      const MapItem& operator=(const MapItem& rhs);
      enum MapItemType m_type;
   };

   class PositionItem : public MapItem {
   public:
      PositionItem(enum MapItemType type, int32 lat, int32 lon);
      PositionItem(Buffer* buf);
      virtual void serialize(Buffer* buf) const;
      int32 getLat() const;
      int32 getLon() const;
      virtual MapItem* clone() const ;
   private:
      int32 m_lat;
      int32 m_lon;
   };

   class RouteItem : public MapItem {
   public:
      RouteItem(int64 routeid);
      RouteItem(Buffer* buf);
      virtual void serialize(Buffer* buf) const;
      int64 getRouteid() const;
      virtual MapItem* clone() const ;
   private:
      int64 m_routeid;
   };

   class ExtraMapInfo : public Serializable {
   public:
      ExtraMapInfo(enum MapInfoType type, uint32 payload);
      ExtraMapInfo(Buffer* buf);
      virtual void serialize(Buffer* buf) const;
      enum MapInfoType getType() const;
      uint32 getPayload() const;
   private:
      enum MapInfoType m_type;
      uint32 m_payload;
   };

   /*
   class XXX : public YYY {
   public:
      XXX();
      XXX(Buffer* buf);
      virtual void serialize(Buffer* buf) const;
   private:
   };
   */
}
#endif
