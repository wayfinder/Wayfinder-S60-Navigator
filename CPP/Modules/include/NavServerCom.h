/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULE_NAVSERVERCOM_H
#define MODULE_NAVSERVERCOM_H
#include <vector>
#include "NavRequestType.h"

namespace isab {

   class Favorite;

   class NavServerComTimerCallBack {
   public:
      virtual void timerExpired(uint16 id) = 0;
   };

   class NavServerComReplyInterface {
   public:
      virtual void sendError( const ErrorObj& err, uint32 src ) const = 0;
      virtual uint16 setTimer(int32 timeout, 
                              class NavServerComTimerCallBack* callback) = 0;
      virtual void getCoords(int32& lat, int32&lon) const = 0;
      virtual class NavServerComConsumerPublic& NavServerComConsumerPublicRef() = 0;
      virtual class NavTaskProviderPublic& NavTaskProviderPublicRef() = 0;
      virtual class ParameterProviderPublic& ParameterProviderPublicRef() = 0;
   };


   //for backward compability
   /** A module to convert function calls into messages that are 
    * sent to another module. That other module must implement
    * NavTaskConsumerInterface. */
   class NavServerComConsumerPublic : public ModulePublic {
   public:
      /** Constructor.
       * @param m the Module this object will use to send it's messages.
       */
      NavServerComConsumerPublic(Module *m) : 
         ModulePublic(m) { };

      /** Answers a WhereAmI request.
       * @param country    the name of the country of the queried position.
       * @param municipal  the name of the municipal.
       * @param city       the name of the city.
       * @param district   the name of the district.
       * @param streetname the streetname.
       * @param lat        the latitude (rad * 1e8) that was queried.
       * @param lon        the longitude (rad * 1e8) that was queried.
       * @param dst        the request ID of the WhereAmI request.
       * @return the request ID of the reply.
       */
      virtual uint32 whereAmIReply(const char* country, 
                                   const char* municipal, const char* city, 
                                   const char* district, 
                                   const char* streetname, 
                                   int32 lat, int32 lon, 
                                   uint32 dst) const;

      /**
       * Answers a message request.
       * @param length     The length of the data.
       * @param data       The data.
       * @param dst        the request ID of the WhereAmI request.
       * @return the request ID of the reply.
       */
      virtual uint32 messageReply(int length,
                                  const char* data,
                                  uint32 dst) const;

      /** Answers a search request.
       * @param data the search reply data as i came from the server.
       * @param len  the size of the data argument.
       * @param dst  the request ID of the search request.
       * @return the request ID of the reply.
       */
      virtual uint32 searchReply(const uint8* data, int len, 
                                 uint32 dst) const;

      virtual uint32 searchInfoReply(const uint8* data, int len, 
                                     uint32 dst) const;
      virtual uint32 licenseKeyReply(bool keyOk, bool phoneOk, bool regionOk,
                                     bool nameOk, bool emailOk,
                                     enum GuiProtEnums::WayfinderType wfType,
                                     uint32 dst) const;

      /** Answers a route request. The actual data must be requested 
       * from NavTask.
       * @param dst the request ID of the route request.
       * @return the request ID of the reply.
       */
      virtual uint32 routeReply(uint32 dst) const;

      /** Answers a map request.
       * @param topLeftLat      the latitude of the top left corner of the 
       *                        image.
       * @param topLeftLon      the latitude of the top left corner of the 
       *                        image.
       * @param bottomRightLat  the latitude of the bottom right corner of 
       *                        the image.
       * @param bottomRightLon  the latitude of the bottom right corner of 
       *                        the image.
       * @param imageType       enum detailing the image format.
       * @param imageBufferSize the size of the image buffer.
       * @param imageBuffer     the image buffer data. 
       * @param dst             the request id of the map request.
       * @return the request ID of the reply.
       */
      virtual uint32 mapReply(int len, const uint8* data, uint32 dst) const;


      /** Answers a vector map request.
       * @param size the size of the <code>data</code> array.
       * @param pointer to the data array.
       * @param dst the request id of the vector map request.
       * @return the request ID of the reply.
       */ 
      virtual uint32 vectorMapReply(uint32 size, const uint8* data, 
                                    uint32 dst) const;

      /** Answers a multiple vector maps request.
       * @param length the lenagth of the data array.
       * @param data vector map data.
       * @param dst the request id of the multi vector map request.
       * @return the request id of the reply.
       */
      virtual uint32 multiVectorMapReply(uint32 length, const uint8* data,
                                         uint32 dst) const;


      /**
       * Unsolicited multiple vector maps reply.
       *
       * @param length the lenagth of the data array.
       * @param data vector map data.
       * @param dst the request id of the multi vector map request.
       * @return the request id of the reply.
       */
      virtual uint32 unsolMultiVectorMapReply(
         uint32 length, const uint8* data,
         uint32 dst ) const;


      virtual uint32 cellConfirm(uint32 length, const uint8* data,
                                 uint32 dst) const;
      
      /** Answers a request.
       * @param dst the request ID of the binaryUpload request.
       * @return the request ID of the reply.
       */
      virtual uint32 binaryUploadReply(uint32 dst) const;

      /** Sends the results of a server poll to DisplaySerial.  As
       * this data isn't associated with any particular request, we
       * use DisplaySerials address as the default destination. XXX
       * This should be fixed so we are not dependent on the
       * DisplaySerialModule receiveing the server poll answers.
       * @param data   the binary data.
       * @param length the size of the data parameter.
       * @param dst    the address of the receiver of poll data. 
       *               Defaults to DisplaySerial.
       * @return the request ID of the reply.
       */
      virtual uint32 binaryDownload(const uint8* data, size_t length,
                                    uint32 dst = 
                                    Module::DisplaySerialModuleId) const;

      /** Used to send Multicast messages reporting the status of the 
       * server connection.
       * @param status indicates progress in quantified steps.
       * @param done   parts done.
       * @param of     total number of parts to do. 
       * @param dst    the address of the receiver of this message. 
       *               Defaults to a multicast address.
       * @return       the source address this message will have. 
       */
      virtual uint32 
      progressMessage(isab::NavServerComEnums::ComStatus status, GuiProtEnums::ServerActionType type, 
                      uint32 done = 0, uint32 of = 0,
                      uint32 dst = MsgBufferEnums::ADDR_DEFAULT ) const;
      

      virtual uint32 messageReply(const uint8* data, unsigned len, 
                                  uint32 dst) const; 

      virtual uint32 latestNews(uint32 checksum, const uint8* data, 
                                unsigned len, 
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      virtual 
      uint32 paramSyncReply(GuiProtEnums::WayfinderType wfType,
                            uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;


      virtual uint32 sendReply( 
         const uint8* data, size_t length,
         uint32 dst = MsgBufferEnums::ADDR_DEFAULT ) const;
   };

   /** A module to convert function calls into messages that are 
    * sent to another module. That other module must implement
    * NavTaskProviderInterface. */
   class NavServerComProviderPublic : public ModulePublic {
   public:
      /** Constructor.
       * @param m the module the created object will use to send 
       *          it's messages.
       */
      NavServerComProviderPublic(Module *m) : 
         ModulePublic(m) 
      {             
         setDefaultDestination(Module::addrFromId(Module::NavServerComModuleId));
      };

      /** Requests an address for a set of coordinates.
       * @param lat the latitude to look up. (rad * 1e8)
       * @param lon the longitude to look up. (rad * 1e8)
       * @param dst the address of the receiving module. 
       *            Defaults to ADDR_DEFAULT.
       * @return the requestID of this request.
       */
      virtual uint32 whereAmI(int32 lat = MAX_INT32, int32 lon = MAX_INT32,
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

   public:
      /** Request a new, fresh, search that is based on no earlier search data.
       * @param nbrOfHits   the maximum number of hits to return in the answer.
       * @param location    the name of the target location.
       * @param destination the name of the target destination.
       * @param countryCode a code that identifies the target country or state.
       *                    Default value MAX_UINT32, which represents the 
       *                    current country.
       * @param lat         the current latitude in rad * 1e8. Default value
       *                    MAX_INT32, which will be replaced with current 
       *                    coordinates just before the request is transmitted 
       *                    to the server.
       * @param lon         the current latitude in rad * 1e8. Default value
       *                    MAX_INT32, which will be replaced with current 
       *                    coordinates just before the request is transmitted 
       *                    to the server.
       * @param heading     the current heading in the 256 parts system.
       *                    Defaults to zero.
       * @param dst         the address of the destination module. Default 
       *                    value is the default location, which is the 
       *                    NavServerCom module.
       * @return the id of this request.
       */
      virtual uint32 originalSearch(const char* searchCity, 
                                    const char* searchDestination,
                                    uint16 startIndex = 0,
                                    uint32 countryCode = MAX_UINT32, 
                                    int32 lat = MAX_INT32, 
                                    int32 lon = MAX_INT32,
                                    uint8 heading = 0,
                                    uint32 dst = MsgBufferEnums::ADDR_DEFAULT)const;
      /** Requests a search where the location string is the result of an 
       * earlier search and we now some other details.
       * @param nbrOfHits   the maximum number of hits to return in the answer.
       * @param location    the name of the target location.
       * @param destination the name of the target destination.
       * @param itemID      the item id for location, returned by the previous
       *                    search.
       * @param mapID       the map id for location, returned by a previous 
       *                    search.
       * @param mask        the mask for the location, returned by a previous 
       *                    search.
       * @param countryCode a code that identifies the target country or state.
       *                    Default value MAX_UINT32, which represents the 
       *                    current country.
       * @param lat         the current latitude in rad * 1e8. Default value
       *                    MAX_INT32, which will be replaced with current 
       *                    coordinates just before the request is transmitted 
       *                    to the server.
       * @param lon         the current latitude in rad * 1e8. Default value
       *                    MAX_INT32, which will be replaced with current 
       *                    coordinates just before the request is transmitted 
       *                    to the server.
       * @param heading     the current heading in the 256 parts system.
       *                    Defaults to zero.
       * @param dst         the address of the destination module. Default 
       *                    value is the default location, which is the 
       *                    NavServerCom module.
       * @return the id of this request.
       */
      virtual uint32 refinedSearch(const char* destination, 
                                   const char* areaId,
                                   uint16 startIndex = 0,
                                   uint32 countryCode = MAX_INT32,
                                   int32 lat = MAX_INT32, 
                                   int32 lon = MAX_INT32,
                                   uint8 heading = 0,
                                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      /** Requests a route from one position to another. 
       * @param fromLat the latitude of the FROM position.  
       * @param fromLon the longitude of the FROM position.
       * @param heading the present heading at the FROM positon.
       * @param toLat   the latitude of the TO position.
       * @param toLon   the longitude of the TO position.
       * @param routeRequestorId The address of the module requesting route
       *                         to send unsolicited  extra route data to.
       * @param dst     the address of the receiving module. Defaults
       *                to ADDR_DEFAULT.
       * @return the request ID of this request.
       */
      virtual uint32 routeToGps(int32 toLat, int32 toLon, uint16 heading = -1, 
                                int32 fromLat = MAX_INT32, 
                                int32 fromLon = MAX_INT32, 
                                uint32 routeRequestorId = 0,
                                int64 oldRouteId = 0,
                                uint8 rerouteReason = NavServerComEnums::user_request,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      

      virtual uint32 mapRequest(const class BoundingBox& bb,
                                uint16 imageWidth, 
                                uint16 imageHeight,
                                uint16 vbWidth,
                                uint16 vbHeight,
                                enum MapEnums::ImageFormat format = MapEnums::GIF,
                                uint16 noMapItems = 0,
                                const class MapItem*  items = NULL,
                                uint16 noExtras = 0,
                                const class ExtraMapInfo*  info = NULL,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      virtual uint32 mapRequest(const uint8* data, unsigned  length,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      /** Requests a vector map chunk from the server.
       * @param request a string describing the request.
       * @param dst     the address of the receiving module. Defaults
       *                to ADDR_DEFAULT.
       * @return the request id of this request.
       */
      virtual uint32 vectorMapRequest(const char* request, 
                                      uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      /** Request multiple vector maps from the server. The reply will
       * be size bound.
       * @param startOffset magic number from vector map handler.
       * @param maxBytes max number of bytes in resply.
       * @param requestStringData vector map request strings from 
       *                          vector map handler. Handled as a binary 
       *                          data vector of length 
       *                          <code>dataLength</code>. 
       * @param dataLength the size of the binary data array 
       *                   <code>requestStringData</code>
       * @param dst     the address of the receiving module. Defaults
       *                to ADDR_DEFAULT.
       * @return the request id of this request. 
       */
      virtual uint32 multiVectorMapRequest(uint32 startOffset,
                                           uint32 maxBytes,
                                           const uint8* requestStringData,
                                           uint32 dataLength, 
                                           uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      /** Request multiple vector maps from the server. The reply will
       * be size bound.
       * @param size the size in bytes of the data block.
       * @param data the data block.
       * @param dst the address of the recieving module. 
       *            Defaults to ADDR_DEFAULT.
       * @return the request id of this request.
       */ 
      virtual 
      uint32 multiVectorMapRequest(uint32 size, const uint8* data,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      virtual uint32 cellReport(uint32 size, const uint8* data, 
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;


      /** Requests an upload of binary data to the server. 
       * @param len  the size of the data parameter.
       * @param data the data to upload.
       * @param dst  the address of the receiving module. 
       *             Defaults to ADDR_DEFAULT.
       * @return the request id for this request.
       */
      virtual uint32 binaryUpload(int len, const uint8 * data, 
                                  uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      /** Sends a block of parameter data to the NavServerCom module.
       * The data contains any number of NavClient-to-Server
       * parameters and is terminated with an end parameter. See the
       * Server Parameter Protocol specification fo rmore info on the
       * format of the parameter data.
       * @param data the parameter data.
       * @param len  the size of the parameter data.
       * @param dat  the address of the NavServerCom module. Defaults
       *             to ADDR_DEFAULT, which should do fine.
       * @return the request id of this request.
       */
      virtual uint32 setServerParams(const uint8* data, size_t len, 
                                     uint32 dst=MsgBufferEnums::ADDR_DEFAULT) const;

      /** Requests the cancelation of a request in the NavServerCom 
       * Request queue. Note that a message can only be canceled by 
       * the module that sent it.
       * @param messageSrc the requestId of the original request. Defaults to 
       *                   all-bits-one, which means that all requests will 
       *                   be canceled.
       * @param dst        the address of the receiver of the message. 
       *                   Defaults to the NavServerCom module.
       */
      virtual uint32 cancelRequest(uint32 messageSrc = MAX_INT32, 
                                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      /** Request a reflash from the server. What the effects of this would 
       * be on the NavPort is something to ponder
       * XXX XXX XXX XXX XXX
       * @return the request id.
       */
      virtual uint32 requestReflash(uint32 dst =MsgBufferEnums::ADDR_DEFAULT) const;

      virtual 
      uint32 requestSearchInfo(uint16 num, const char* const* ids, 
                               const char* const* values = NULL,
                               uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      virtual 
      uint32 messageRequest(GuiProtEnums::UserMessageType messType, 
                            GuiProtEnums::ObjectType object, const char* to, 
                            const char* from, const char* sig, const char* id,
                            uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      virtual 
      uint32 verifyLicenseKey(const char* key, const char* phone, 
                              const char* name, const char* email,
                              const char* optional,
                              uint32 region, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      virtual uint32 paramSync(uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;


      /**
       * Send a NGP request.
       *
       * @param type The type of request, serach, route etc..
       * @param params The data to send.
       * @param requestVer The version of request, search, route etc.,
       *                   default 1.
       * @param dst The destination module, default is default address.
       * @return The request id.
       */
      virtual uint32 sendRequest( 
         enum navRequestType::RequestType type,
         class NParamBlock& params,
         uint8 requestVer = 1,
         uint32 dst = MsgBufferEnums::ADDR_DEFAULT ) const;


   private:
      // for backwards compability
      friend class DisplaySerial;
      /** Packs the parameters into a SEARCH_REQUEST packet for NavServerCom.
       * For an explanation of the parameters, look in the NavigatorServer 
       * Protocol Specification.
       * @param city         the name of the city.
       * @param selectedCity the name of the selected city from a 
       *                     previous search.
       * @param destination  the name of the destination.
       * @param itemID       the item id of a previous search.
       * @param mapID        the mapID of a previous search.
       * @param maskOrOffset the maskOrOfsset of a previous search.
       * @param searchNbr    the maximum number of search hits.
       * @param lat          the present latitude.
       * @param lon          the present longitude.
       * @param dst          the address of the receiving module. 
       * @return the request ID of this request.
       */
      virtual uint32 searchRequest(const char* searchCity, 
                                   const char* searchDestination, 
                                   const char* areaId, 
                                   uint16 startIndex, 
                                   uint32 lat, 
                                   uint32 lon, 
                                   uint8 heading, 
                                   uint32 country, 
                                   uint32 dst) const;

   };

   /* Next are the interfaces. These should be implemented by the 
    * module that wishes to receive the corresponding messages. 
    * The classes below are pure virtual and correspond in function
    * to "interfaces" in java. */

   /** An interface that must be implemented to receive messages in a
    * NavTask provider-consumer relationship. This is implemented 
    * in the _upper_ module.
    */
   class NavServerComConsumerInterface {
   public:
      /** Receives replies that acknowledges that a binary upload went well.
       * @param src       the address of the module that sent this message.
       * @param dst       the request ID of the binary upload request.
       */
      virtual void decodedBinaryUploadReply(uint32 src, uint32 dst) = 0;

      /** Receives unrequested binary data that was downloadad from
       * the server as a result of server poll.
       * @param data   the binary data.
       * @param length the size of the data parameter.
       * @param src    the sender of the message.
       * @param dst    the address of the receiving module.
       */
      virtual void decodedBinaryDownload(const uint8* data, size_t length, 
                                         uint32 src, uint32 dst) = 0;


      /** Receives answers to WhereAmI requests.
       * @param country    the name of the country of the queried position.
       * @param municipal  the name of the municipal.
       * @param city       the name of the city.
       * @param district   the name of the district.
       * @param streetname the streetname.
       * @param lat        the latitude (rad * 1e8) that was queried.
       * @param lon        the longitude (rad * 1e8) that was queried.
       * @param src        the sender of this message.
       * @param dst        the request ID of the WhereAmI request.
       */
      virtual void decodedWhereAmIReply(const char* country, 
                                        const char* municipal, 
                                        const char* city, 
                                        const char* district, 
                                        const char* streetname, int32 lat,
                                        int32 lon, uint32 src, 
                                        uint32 dst) = 0;

      virtual void decodedSearchReply(unsigned nAreas, 
                                      const class SearchArea* const *sa,
                                      unsigned nItems, 
                                      const class FullSearchItem* const *fsi,
                                      uint16 begindex, uint16 total,
                                      uint32 src, uint32 dst) = 0;

      virtual void decodedSearchInfoReply(uint16 num, const char* const* id,
                                          const unsigned* index,
                                          unsigned numInfo,
                                          const class AdditionalInfo*const* info,
                                          uint32 src, uint32 dst) =0;

      /** Receives replies to route requests. Signals that a route has
       * been downloaded to NavTask.
       * @param src the sender of the route reply.
       * @param dst the request id of the route request.
       */
      virtual void decodedRouteReply(uint32 src, uint32 dst) = 0;


      virtual void decodedMapReply(const class BoxBox& bb, uint32 realWorldWidth,
                                   uint32 realWorldHeight, uint16 imageWidth,
                                   uint16 imageHeight,
                                   enum MapEnums::ImageFormat imageType, 
                                   uint32 imageBufferSize, 
                                   const uint8* imageBuffer, uint32 dst) = 0;
      /** Receives replies to vector map requests.
       * @param request a string containgin something.
       * @param size the size of the data array.
       * @param data binary data.
       * @param dst the request id.
       */
      virtual void decodedVectorMapReply(const char* request, uint32 size,
                                         const uint8* data, uint32 dst) = 0;

      virtual void decodedMulitVectorMapReply(uint32 size,
                                              const uint8* data, 
                                              uint32 dst) = 0;


      virtual void decodedForceFeedMuliVectorMapReply( uint32 size,
                                                       const uint8* data, 
                                                       uint32 dst ) = 0;


      virtual void decodedCellConfirm(uint32 size, const uint8* data,
                                      uint32 dst) = 0;

      /** Receives progress messages from NavServerCom. Since these
       * messages are multicasted, any module that expects to receive
       * progress messages must join the multicast channel
       * Module::NSCProgressMessages.
       * @param status indicates progress in quantified steps. 
       * @param done   parts done.
       * @param of     total number of parts to do. 
       * @param src    the sender of this message, probably NavServerCom.
       * @param dst    the address of the receiver, probaly the 
       *               multicast address.
       */
      virtual void decodedProgressMessage(isab::NavServerComEnums::ComStatus status, 
                                          GuiProtEnums::ServerActionType type,
                                          uint32 done, uint32 of,
                                          uint32 src, uint32 dst) = 0;
      virtual void decodedMessageReply(const uint8* data, unsigned length,
                                       uint32 src, uint32 dst) = 0;

      virtual void decodedLicenseReply(bool keyOk, bool phoneOk, 
                                       bool regionOk, bool nameOk, 
                                       bool emailOk,
                                       GuiProtEnums::WayfinderType type,
                                       uint32 src, uint32 dst) = 0;

      virtual void decodedLatestNews(uint32 checksum, const uint8* data, 
                                     unsigned length, uint32 src, 
                                     uint32 dst) = 0;

      virtual void decodedParamSyncReply(GuiProtEnums::WayfinderType wft, 
                                         uint32 src, uint32 dst) = 0;


      /**
       * Received a NGP reply.
       *
       * @param type The type of reply, serach, route etc..
       * @param params The reply data.
       * @param requestVer The version of reply, search, route etc..
       * @param statusCode The status of the reply.
       * @paramstatusMessage The status message as string.
       * @param src The origininating module.
       * @param dst The destination module.
       */
      virtual void decodedReply( 
         navRequestType::RequestType type, class NParamBlock& params, 
         uint8 requestVer, uint8 statusCode, const char* statusMessage,
         uint32 src, uint32 dst ) = 0;

   };

   /** An interface that must be implemented to receive messages in a
    * NavTask provider-consumer relationship. This is implemented 
    * in the _lower_ module.
    */
   class NavServerComProviderInterface {
   public:
      virtual ~NavServerComProviderInterface() = 0;

      /** Receives WhereAmI requests. If the value of either or both
       * of lat and lon is MAX_INT32, that is a request to fill in
       * the last known position just before transmission to the
       * server.
       * @param lat the latitude to look up. 
       * @param lon the longitude to look up.
       * @param src the request ID.
       */
      virtual void decodedWhereAmI(int32 lat, int32 lon, uint32 src) = 0;

      /** Receives search request.
       * @param length    the length of the data block.
       * @param data      the data, ready to send to the server.
       * @param lat
       * @param lon
       * @param hdg
       * @param country
       * @param city
       * @param src       the request ID.
       * @param latOffset the offset into the data where the new latitude 
       *                  should be written if refresh is set.
       * @param lonOffset the offset into the data where the new longitude 
       *                  should be written if refresh is set.
       * @param hdgOffset 
       */
      virtual void decodedSearchRequest(const uint8 *data, int length,
                                        int32 lat, int32 lon, uint8 hdg,
                                        uint32 country, const char* city, 
                                        uint32 src, int latOffset = -1, 
                                        int lonOffset = -1, 
                                        int hdgOffset = -1) = 0;


      /** Receives route requests.
       * @param toLat   latitude to route to.
       * @param toLon   longitude to route to.
       * @param fromLat latitude to route from.
       * @param fromLon longitude to route from.
       * @param heading current heading.
       * @param src  the address of the original requester.
       */   
      virtual void decodedRouteToGps(int32 toLat, int32 toLon, 
                                     int32 fromLat, int32 fromLon,
                                     uint16 heading, 
                                     uint32 routeRequestorId,
                                     int64 oldRouteId, 
                                     uint8 rerouteReason,
                                     uint32 src) = 0;

      /** Receives binary upload requests.
       * @param len the length of the data block.
       * @param data the data, ready to send to the server.
       * @param src the request ID.
       */
      virtual void decodedBinaryUpload(int len, const uint8 * data, 
                                       uint32 src) = 0;


      virtual void decodedSearchInfoRequest(int len, const uint8* data, 
                                            uint32 src) = 0;

      virtual void decodedVerifyLicenseKey(const char* key, const char* phone,
                                           const char* name, const char* email,
                                           const char* optional,
                                           uint32 region, uint32 src) = 0;

      virtual void decodedMapRequest(int len, const uint8* data, uint32 src, 
                                     int latOffset = -1, int lonOffset = -1, 
                                     int hdgOffset = -1, int spdOffset = -1)=0;

      virtual void decodedVectorMapRequest(int len, const uint8* data, 
                                           uint32 src) = 0;

      virtual void decodedMultiVectorMapRequest(int len, const uint8* data, 
                                                uint32 src) = 0;

      virtual void decodedCellReport(const uint8* data, uint32 size, 
                                     uint32 src) = 0;

      /** Receives server parameters from the NavClient.
       * @param data the Parameter data.
       * @param len  the size of the parameter data.
       * @param src  the request id of this request.
       */
      virtual void decodedSetServerParams(const uint8* data, size_t len, 
                                          uint32 src) = 0;
      /** Receives request to cancel one or more messages.
       * @param originalSrc the request ID of the request to cancel.
       *                    If all bits are set, all requests will be canceled.
       * @param netmask     the netmask of the sending module.
       * @param src         the request id of the cancel request.
       */
      virtual void decodedCancelRequest(uint32 originalSrc, uint32 netmask,
                                        uint32 src) = 0;

      /** Receives reflash requests.
       * @param sr the sender of the request.
       */
      virtual void decodedRequestReflash(uint32 src) = 0;


      virtual void decodedMessageRequest(const uint8* data, unsigned length,
                                         uint32 src) = 0;

      virtual void decodedParamSync(uint32 src) = 0;


      virtual void decodedSendRequest( const class MsgBuffer* buf ) = 0;
   };

   /** An object of this class is owned by each module that implements 
    * the NavTaskConsumerInterface. This object contains a dispatch()
    * function that does the actual decoding of an incoming buffer.
    * It then call the apropriate methods in NavTaskConsumerInterface.
    */
   class NavServerComConsumerDecoder {
   public:
      /** Checks if the message is sent from a
       * NavServerComProviderPublic, and if so sends it to the proper
       * function in the NavServerComConsumerInterface.  
       * @param buf the MessageBuffer containing the message.
       * @param m   the NavServerComConsumerInterface that will handle 
       *            the message if it is from a NavServerComProviderPublic.
       * @return NULL if the message was from a NavServerComProviderPublic, 
       *         the buffer otherwise.
       */
      MsgBuffer * dispatch(MsgBuffer *buf, NavServerComConsumerInterface *m );
   };

   /** An object of this class is owned by each module that implements 
    * the NavTaskProviderInterface. This object contains a dispatch()
    * function that does the actual decoding of an incoming buffer.
    * It then call the apropriate methods in NavTaskProviderInterface.
    */
   class NavServerComProviderDecoder {
   public:
      /** Checks if the message is sent from a
       * NavServerComConsumerPublic, and if so sends it to the proper
       * function in the NavServerComProviderInterface.  
       * @param buf the MessageBuffer containing the message.
       * @param m   the NavServerComProviderInterface that will handle 
       *            the message if it is from a NavServerComConsumerPublic.
       * @return NULL if the message was from a NavServerComConsumerPublic, 
       *         the buffer otherwise.
       */
      MsgBuffer * dispatch(MsgBuffer *buf, NavServerComProviderInterface *m );
   };


   //Definition of pure virtual destructor.
   inline NavServerComProviderInterface::~NavServerComProviderInterface()
   {
   }


} /* namespace isab */

#endif /* MODULE_NAVSERVERCOM_H */
