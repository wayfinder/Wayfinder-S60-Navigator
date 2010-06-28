/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* The public interface to the NavTask. This is the algorithm that 
 * actually follows the position of the car.
 */


#ifndef MODULE_NAVTASK_H
#define MODULE_NAVTASK_H


namespace isab {
   class RouteInfo;
   class RouteList;
   //   using namespace RouteEnums;
/** 
 * A class that converts function calls into messages that are sent to
 * a module expecting messages from NavTask. That other module must
 * implement NavTaskConsumerInterface. 
 */
class NavTaskConsumerPublic : public ModulePublic {
   public:
   /**
    * Constructor.
    * @param m The owning module.
    */
      NavTaskConsumerPublic(Module *m) : 
         ModulePublic(m) { };

   /**
    * Sends a NT_GPS_SATELLITE_INFO message.
    * @param satInfo The satellite info to send. 
    * @param dst The destination address.
    * @return The request id and return address.
    */
      virtual uint32 satelliteInfo(class GpsSatInfoHolder *satInfo,
                                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
   /**
    * Sends a NAV_TASK_FILE_OPERATION message. 
    * @param op The file operation.
    * @param dst The destination address or originating request id. 
    * @return The request id and return address.
    */
      virtual uint32 sendFileOperation(class GuiFileOperation *op, 
                                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
   /**
    * Sends a POSITION_STATE message.
    * @param p The position state.
    * @param dst The destination address or originating request id. 
    * @return The request id and return address.
    */
      virtual uint32 positionState(const struct PositionState &p,
                                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
   /**
    * Sends a NEW_ROUTE_COORD message.
    * @param lat The latitude of the route coordinate.
    * @param lon The longitude of the route coordinate.
    * @param dst The destination address or originating request id. 
    * @return The request id and return address.
    */
      virtual uint32 sendRouteCoord(int32 lat, int32 lon, uint8 dir,
                                    uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

   /**
    * Sends an INVALIDATE_ROUTE message. This message is sent to
    * inform all listeners that the current route is being discarded.
    *
    * If newRouteAvailable is false, all other arguments should be
    * ignored by the recipient.
    *
    * @param newRouteAvailable Set to true if a new route is replacing
    *                          the invalidated one.
    * @param routeid           The route ID of the new route.
    * @param tLat              The top latitide of the new route. 
    * @param lLon              The leftmost longitude of the new route.
    * @param bLat              The bottom latitide of the new route.
    * @param rLon              The rightmost longitude of the new route.
    * @param oLat              The latitude of the routes origin.
    * @param oLon              The longitude of the routes origin.
    * @param dLat              The latitude of the routes destination.
    * @param dLon              The longitude of the routes destination.
    * @param dst The destination address or originating request id. 
    * @return The request id and return address.
    */
      virtual uint32 invalidateRoute(bool newRouteAvailable, int64 routeid,
                                   int32 tLat, int32 lLon,
                                   int32 bLat, int32 rLon,
                                   int32 oLat, int32 oLon,
                                   int32 dLat, int32 dLon,
                                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
   /**
    * Sends a ROUTE_INFO message, containing the information needed to
    * show the user the next two turns.
    * @param r The RouteInfo to send. 
    * @param dst The destination address or originating request id. 
    * @return The request id and return address.
    */
      virtual uint32 routeInfo(const RouteInfo &r,
                                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
   /**
    * Sends a ROUTE_LIST message, containing the information needed to
    * show the user the route description.
    * @param rl The RouteInfo to send. 
    * @param dst The destination address or originating request id. 
    * @return The request id and return address.
    */
      virtual uint32 routeList(const RouteList &rl,
                                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      /** Answers a route request. The actual data must be requested 
       * from NavTask.            
       * @param dst the request ID of the route request.
       * @return the request ID of the reply.
       */
      virtual uint32 ntRouteReply(int64 routeid, 
                                  uint32 dst) const;


};

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * NavTaskProviderInterface. */
class NavTaskProviderPublic : public ModulePublic {
   public:
      NavTaskProviderPublic(Module *m) : 
         ModulePublic(m) { 
            setDefaultDestination(Module::addrFromId(Module::NavTaskModuleId));
         };

      enum CommandType { 
         Nop = 0, 
         AbandonRoute, 
         ForgetWheelsize, 
         SendStatus, 
         Reroute,
         RouteToPositionCancel,
      };

      virtual uint32 command(enum CommandType command,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 fileOperation(class GuiFileOperation* op,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 newRoute(bool routeOrigFromGPS,
            int32 origLat,
            int32 origLon, 
            int32 destLat,
            int32 destLon, 
            int packetLength,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 routeChunk(bool failedRoute,
            int chunkLength, 
            const uint8 *chunkData,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

      virtual uint32 keepRoute(int64 routeid, 
                               uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

      /** Requests a route from one position to another. 
       * @param fromLat the latitude of the FROM position.  
       * @param fromLon the longitude of the FROM position.
       * @param toLat   the latitude of the TO position.
       * @param toLon   the longitude of the TO position.
       * @param dst     the address of the receiving module. Defaults
       *                to ADDR_DEFAULT.
       * @return the request ID of this request.
       */
      virtual uint32 ntRouteToGps(int32 toLat, int32 toLon,
                                int32 fromLat = MAX_INT32, 
                                int32 fromLon = MAX_INT32, 
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

      virtual uint32 getFilteredRouteList(int16 startWpt, int16 numWpts,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

};




/* Next are the interfaces. These should be implemented by the 
 * module that wishes to receive the corresponding messages. 
 * The classes below are pure virtual and correspond in function
 * to "interfaces" in java. */

/** An interface that must be implemented to receive messages in a
 * NavTask provider-consumer relationship. This is implemented 
 * in the _upper_ module.
 */
class NavTaskConsumerInterface {
   public:
      virtual void decodedGpsSatelliteInfo(
            class GpsSatInfoHolder* satInfo,
            uint32 src) = 0;
      virtual void decodedFileOperation(
            class GuiFileOperation* op,
            uint32 src) = 0;
      virtual void decodedPositionState(
            const struct PositionState &p, 
            uint32 src) = 0;
      virtual void setRouteCoordinate( int32 lat, int32 lon, int8 dir ) = 0;
      virtual void decodedRouteInfo(
            const RouteInfo &r,
            uint32 src) = 0;
      virtual void decodedInvalidateRoute(bool newRouteAvailable,
                                          int64 routeid,
                                          int32 tLat, int32 lLon, 
                                          int32 bLat, int32 rLon,
                                          int32 oLat, int32 oLon,
                                          int32 dLat, int32 dLon,
                                          uint32 src) = 0;
      /** Receives replies to route requests. Signals that a route has
       * been downloaded to NavTask.
       * @param src the sender of the route reply.
       * @param dst the request id of the route request.
       */
      virtual void decodedNTRouteReply(int64 routeid,
                                       
                                       uint32 src, uint32 dst) = 0;
      virtual void decodedRouteList(class Buffer *buf,
            uint32 src, uint32 dst) = 0;
};

/** An interface that must be implemented to receive messages in a
 * NavTask provider-consumer relationship. This is implemented 
 * in the _lower_ module.
 */
class NavTaskProviderInterface {
   public:
      virtual void decodedCommand(enum NavTaskProviderPublic::CommandType command,
                             uint32 src) = 0;
      virtual void decodedFileOperation(class GuiFileOperation* op,
                             uint32 src) = 0;
      virtual void decodedNewRoute(bool routeOrigFromGPS,
                                   int32 origLat, int32 origLon,
                                   int32 destLat, int32 destLon, 
                                   int packetLength,
                                   uint32 src) = 0;
      virtual void decodedRouteChunk(bool failedRoute, int chunkLength, 
                                     const uint8 *chunkData,
                                     uint32 src) = 0;

      virtual void decodedKeepRoute(int64 routeid, uint32 src, uint32 dst) = 0;

      /** Receives route requests.
       * @param toLat   latitude to route to.
       * @param toLon   longitude to route to.
       * @param fromLat latitude to route from.
       * @param fromLon longitude to route from.
       * @param heading current heading.
       * @param src  the address of the original requester.
       */
      virtual void decodedNTRouteToGps(int32 toLat, int32 toLon,
                                     int32 fromLat, int32 fromLon,
                                     uint32 src) = 0;

      virtual void decodedGetFilteredRouteList(int16 startWpt, int16 numWpts,
                                   uint32 src) = 0;


};

/** An object of this class is owned by each module that implements 
 * the NavTaskConsumerInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in NavTaskConsumerInterface.
 */
class NavTaskConsumerDecoder {
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf, NavTaskConsumerInterface *m );
};

/** An object of this class is owned by each module that implements 
 * the NavTaskProviderInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in NavTaskProviderInterface.
 *
 * @return the buffer if further processing may be done or NULL if 
 *         the buffer was processed and either passed on or 
 *         destroyed and thus no further processing should be done.
 */
class NavTaskProviderDecoder {
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf, NavTaskProviderInterface *m );
};

} /* namespace isab */

#endif /* MODULE_NavTask_H */


