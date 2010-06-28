/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUIPROTROUTEMESS_H
#define GUIPROTROUTEMESS_H
#include "GuiProt/GuiProtMess.h"
namespace isab{
   /**
    * Class that represents the ROUTE_MESSAGE. 
    * This message is supposed to go from the GUI to Nav2. It makes no
    * sense in the other direction.
    * When sent to Nav2 it will cause Nav2 to request a new route
    * description from the server.
    */
   class RouteMess : public GuiProtMess
   {
   public:
      /**
       * Construct the RouteMess object from a serialised state stored
       * in a Buffer.
       *
       * NOTE: The string properties are NOT copied from the Buffer,
       *       so the Buffers lifetime must be longer than the the
       *       RouteMess objects.
       *
       * @param buf The buffer containing the serialised message.
       */
      RouteMess(Buffer* buf);
      /**
       * Construct a RouteMess that requests a route from the current
       * position to another posistion specified by a coordinate.
       * The coordinate is expressed in WGS84 radians * 10^8
       *
       * NOTE: The new object does NOT take ownership of any string
       *       arguments. It doesn't make any copies either, so it is
       *       the callers responsibility to make sure the strings
       *       have longer lifetime then the RouteMess object.
       *
       * @param d_type Encodes what kind of position the destination is.
       * @param d_id If the d_type is PositionTypeSearch, this string
       *             should be the search item id. If it is
       *             PositionTypeFavorite it should be the favorite id
       *             as a string. All other cases NULL or the empty
       *             string.
       * @param d_lat The latitude of the destination.
       * @param d_lon The longitude of the destination.
       * @param destName The name of the destination. Will be returned
       *                 in any STARTED_NEW_ROUTE message.
       */
      RouteMess(GuiProtEnums::PositionType d_type, char *d_id,
            int32 d_lat, int32 d_lon,
            char *destName);
      /**
       * Construct a RouteMess that requests a route from one position
       * to another. Both positions are specified by coordinates.  The
       * coordinates is expressed in WGS84 radians * 10^8
       *
       * NOTE: The new object does NOT take ownership of any string
       *       arguments. It doesn't make any copies either, so it is
       *       the callers responsibility to make sure the strings
       *       have longer lifetime then the RouteMess object.
       *
       * @param o_type Encodes what kind of position the origin is.
       * @param d_id If the o_type is PositionTypeSearch, this string
       *             should be the search item id. If it is
       *             PositionTypeFavorite it should be the favorite id
       *             as a string. All other cases NULL or the empty
       *             string.
       * @param o_lat  The latitude of the origin.
       * @param o_lon  The longitude of the origin.
       * @param d_type Encodes what kind of position the destination is.
       * @param d_id If the d_type is PositionTypeSearch, this string
       *             should be the search item id. If it is
       *             PositionTypeFavorite it should be the favorite id
       *             as a string. All other cases NULL or the empty
       *             string.
       * @param d_lat  The latitude of the destination.
       * @param d_lon  The longitude of the destination.
       * @param destName The name of the destination. Will be returned
       *                 in any STARTED_NEW_ROUTE message.
       */
      RouteMess(GuiProtEnums::PositionType o_type, char *o_id,
            int32 o_lat, int32 o_lon,
            GuiProtEnums::PositionType d_type, char *d_id,
            int32 d_lat, int32 d_lon,
            char *destName);
      /**
       * Virtual destructor. The m_destName, m_oId, and m_dId members
       * are not deleted by the destructor. 
       */
      virtual ~RouteMess();
      /**
       * Does nothing. 
       */
      void deleteMembers();
      /**
       * Serialize this message into a Buffer.
       * @param buf The Buffer.
       */
      void serializeMessData(Buffer* buf) const;
      /**
       * @return The origin Id. 
       */
      const char* getoId() const;
      /**
       * @return The destination Id.
       */
      const char* getdId() const;
      /**
       * @return The destination name. 
       */
      const char* getDestinationName() const;
      /**
       * @return The origin type. 
       */
      GuiProtEnums::PositionType getoType() const;
      /**
       * @return The destination type. 
       */
      GuiProtEnums::PositionType getdType() const;
      /**
       * @return The origin latitude.
       */
      int32 getoLat() const;
      /**
       * @return The origin longitude..
       */
      int32 getoLon() const;
      /**
       * @return The destination latitude.
       */
      int32 getdLat() const;
      /**
       * @return The destination longitude.
       */
      int32 getdLon() const;
   private:
      /** The destination name. */
      const char* m_destName;
      /** The origin type. */
      GuiProtEnums::PositionType m_oType;
      /** The destination type. */
      GuiProtEnums::PositionType m_dType;
      /** The origin id. */
      const char* m_oId;
      /** The destination id. */
      const char* m_dId;
      /** The origin latitude. */
      int32 m_oLat;
      /** The origin longitude. */
      int32 m_oLon;
      /** The destination latitude. */
      int32 m_dLat;
      /** The destination longitude. */
      int32 m_dLon;
   };

   /**
    * This class represents a ROUTE_TO_POSITION message. 
    * It is supposed to be sent from the GUI to Nav2. It doesn't make
    * any sense in the other direction.
    * When sent to Nave it will request a route from the current
    * position to another position spacified by a coordinate. The
    * coordinate is specified as WGS84 radians * 10^8.
    */
   class RouteToPositionMess : public GuiProtMess
   {
   public:
      /**
       * Construct a RouteToPositionMess object from a serialized
       * representation stored in a Buffer.
       *
       * NOTE: The destination property is NOT copied from the Buffer,
       *        so the Buffers lifetime must be longer than the the
       *        RouteToPositionMess objects.
       *
       * @param buf The Buffer. 
       */
      RouteToPositionMess(Buffer* buf);
      /**
       * Constructor.
       * 
       * NOTE: The destination property is NOT copied and not taken
       *       ownership of. The lifetime of the destination string
       *       must extend past the lifetime of the
       *       RouteToPositionMess object.
       *
       * @param destination The name of the destination. Will be returned
       *                    in any STARTED_NEW_ROUTE message.
       * @param lat The destination latitude.
       * @param lon The destination longitude.
       */
      RouteToPositionMess(const char* destination, int32 lat, int32 lon);
      /**
       * Virtual destructor. 
       * Does nothing. 
       */
      virtual ~RouteToPositionMess();
      /**
       * Does nothing. 
       */
      virtual void deleteMembers();
      /**
       * Serialize this message into a Buffer.
       * @param buf The Buffer.
       */      
      virtual void serializeMessData(Buffer* buf) const;
      /**
       * @return The destination name. 
       */
      const char* getDestinationName() const;
      /**
       * @return The destination latitude. 
       */
      int32 getLat() const;
      /**
       * @return The destination longitude.
       */
      int32 getLon() const;
   private:
      /** The destination name. */
      const char* m_destinationName;
      /** The destination latitude. */
      int32 m_lat;
      /** The destination longitude. */
      int32 m_lon;
   };
   
   /**
    * This class represents a STARTED_NEW_ROUTE message.
    * The message is sent from Nav2 to the GUI when a new route has
    * been downloaded. Sending the messsage in the other direction
    * makes no sense.
    */
   class StartedNewRouteMess : public GuiProtMess
   {
   public:
      /**
       * Create a StartedNewRouteMess from a serialized representation
       * stored in a Buffer.
       * 
       * NOTE: The destination property is NOT copied from the Buffer,
       *        so the Buffers lifetime must be longer than the the
       *        RouteToPositionMess objects.
       */
      StartedNewRouteMess(Buffer* buf);
      /**
       * Constructor. 
       * 
       * NOTE: The destination property is NOT copied and not taken
       *       ownership of. The lifetime of the destination string
       *       must extend past the lifetime of the
       *       RouteToPositionMess object.
       *
       * @param routeid     The id of the route. 
       * @param topLat      The top edge of a bounding box that 
       *                    contains the entire route.
       * @param leftLon     The left edge of a bounding box that 
       *                    contains the entire route.
       * @param bottomLat   The bottom edge of a bounding box that 
       *                    contains the entire route.
       * @param rightLon    The right edge of a bounding box that 
       *                    contains the entire route.
       * @param oLat        The latitude of the routes origin.
       * @param oLon        The longitude of the routes origin. 
       * @param dLat        The latitude of the routes destination.  
       * @param dLon        The longitude of the routes destination. 
       * @param destination The destination name. 
       */
      StartedNewRouteMess(int64 routeid, 
                          int32 topLat, int32 leftLon,
                          int32 bottomLat, int32 rightLon, 
                          int32 oLat, int32 oLon, 
                          int32 dLat, int32 dLon, 
                          const char* destination);
      /**
       * Virtual destructor. 
       * Does nothing. 
       */
      virtual ~StartedNewRouteMess();
      /**
       * Does nothing. 
       */
      virtual void deleteMembers();
      /**
       * Serialize this message into a Buffer.
       * @param buf The Buffer.
       */
      virtual void serializeMessData(Buffer* buf) const;
      /** @return The route ID. */
      int64 getRouteId() const;
      /** @return The bounding box's top edge. */
      int32 getTopLat() const;
      /** @return The bounding box's left edge. */
      int32 getLeftLon() const;
      /** @return The bounding box's bottom edge. */
      int32 getBottomLat() const;
      /** @return The bounding box's right edge. */
      int32 getRightLon() const;
      /** @return The latitude of the origin. */
      int32 getoLat() const;
      /** @return The longitude of the origin. */
      int32 getoLon() const;
      /** @return The latitude of the destination. */
      int32 getdLat() const;
      /** @return The longitude of the destination. */
      int32 getdLon() const;
      /** @return The destination name. */
      const char* getDestination() const;
   private:
      /** The route ID. */
      int64 m_routeid;
      /** The bounding box's top edge. */
      int32 m_tLat;
      /** The bounding box's left edge. */
      int32 m_lLon;
      /** The bounding box's bottom edge. */
      int32 m_bLat;
      /** The bounding box's right edge. */
      int32 m_rLon;
      /** The latitude of the origin. */
      int32 m_oLat;
      /** The longitude of the origin. */
      int32 m_oLon;
      /** The latitude of the destination. */
      int32 m_dLat;
      /** The longitude of the destination. */
      int32 m_dLon;
      /** The destination name. */
      const char* m_destination;
   };

}
#endif
