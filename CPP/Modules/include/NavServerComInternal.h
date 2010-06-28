/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVSERVERCOMINTERNAL_H
#define NAVSERVERCOMINTERNAL_H

#include <deque>
#include <vector>

class CSettingInfo;

namespace isab{
   //Forward declaration of classes found in the NavServerCom directory.
   class ParameterBlock;
   class Server;
   class ServerParser;
   class Tracking;
   class HWID;

   using namespace NavServerComEnums;
   /** This class handles all server communication. It needs a lower
       serial layer that handles the transfer of serial data to and
       from the server. */
   class NavServerCom : public Module,
                        public SerialConsumerInterface, 
                        public CtrlHubAttachedInterface,
                        public NavServerComProviderInterface,
                        public ParameterConsumerInterface,
                        public NavTaskConsumerInterface
   {
   public:
      /** The constructor. 
       * @param spp pointer to the SerialProviderPublic object providing
       *            serial data from the NavigatorServer.  
       * @param serialNo IMEI or NES. If NULL, NavServerCom will try 
       *                 to use the GetIMEI function.
       * @param server the default server.
       * @param port the default server port.
       * @param majorRel Major version reported to server.
       * @param minorRel Minor version reported to server.
       * @param build    Build number reported to server. 
       * @param majorResource Major resource version reported to server.
       * @param minorResource Minor resource version reported to server.
       * @param buildResource Resource build number reported to server. 
       * @param clientType Clienttype reported to server.
       * @param clientOptions Client options reported to server.
       * @param httpProto If to talk using HTTP to server.
       * @param httpUserAgent The User-Agent string to use.
       * @param httpRequest The HTTP request string if null, 
       *                    "/nav" will be used. 
       * @param httpFixedHost If httpProto is true this string will be
       *                      used in the Host header of the http
       *                      request. If httpProto is true and
       *                      httpFixedHost is NULL, the real server
       *                      will be used in the Host header.
       * @param useTracking Must be true for tracking to occur.
       * @param maxnbroutstanding The maxumum number of server requests that
       *                          can be outstanding.
       * @param licenseKeyType The license key type. IMEI if NULL, otherwise "imei" or "imsi".
       */
      NavServerCom(SerialProviderPublic* spp, 
                   const char* serialNo,
                   const char* server = NULL, uint16 port = 0, 
                   uint32 majorRel = 0, uint32 minorRel = 0, uint32 build = 0, 
                   uint32 majorResource = 0, uint32 minorResource = 0, 
                   uint32 buildResource = 0, const char* clientType = NULL, 
                   const char* clientOptions = NULL, 
                   bool httpProto = false,
                   const char* httpUserAgent = NULL,
                   const char* httpRequest = NULL,
                   const char* httpFixedHost = NULL,
                   bool useTracking = false,
                   uint32 maxnbroutstanding = MAX_UINT32,
                   const char* licenseKeyType = NULL,
                   bool wayfinderIDStartUp = false,
                   std::vector<HWID*> *hardwareIDs = NULL );

      /** Destructor.*/
      virtual ~NavServerCom();

      /** Allocates a new CtrlHubAttachedPublic object connected to the
       * queue of this object.
       * @return a pointer to a new CtrlHubAttachedPublic object.
       */
      CtrlHubAttachedPublic * newPublicCtrlHub();   

      virtual void decodedStartupComplete();
      virtual void decodedShutdownPrepare(int16 upperTimeout);
      virtual void decodedShutdownNow(int16 upperTimeout);
      virtual void treeIsShutdown();

      virtual void decodedParamNoValue(uint32 param, 
                                     uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 param, const int32* data, 
                                     int32 numEntries, uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 param, const float* data, 
                                     int32 numEntries, uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 paramId, const uint8* data, 
                                     int size, uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 param, const char*const* data, 
                                     int32 numEntries, uint32 src, uint32 dst);

      /** This method is called from NavServerComDecoder::dispatch.
       * It clears out old parameter settings and set the new ones.
       * @param data   the parameter data. This data is never parsed but sent 
       *               as is to the server at the beginning of every session.
       *               NavServerCom depends on this block being terminated with
       *               the end parameter tag.
       * @param length the size of the data block.
       * @param src    the address of the sender of the parameter block.
       */
      virtual void decodedSetServerParams(const uint8* data, size_t length, 
                                          uint32 src);
      
      /** Tests if coordinates are valid for route and where-am-I
       * requests.  If the test fails, an error is sent to
       * <code>src</code>. The error is of type
       * <code>Nav2Error::NSC_OUT_OF_THIS_WORLD</code> if the supplied
       * arguments are invalid, and <code>NSC_NO_GPS</code> if both
       * <code>lat</code> and <code>lon</code> are equal to
       * <code>MAX_INT32</code> and no proper gps coordinates have
       * arrived yet.
       * @param lat the latitude in rad * 1e8.
       * @param lon the longitude in rad * 1e8.
       * @param src the address to send errors to.
       * @return true if the (lat, lon) is 'on the world' or if they both
       *         are MAX_INT32 and NavServerCom thinks the current position
       *         is 'on the world'.*/
      bool validCoords(int32 lat, int32 lon, uint32 src) const;

      /** Called by NavServerComDecoder::dispatch when a whereAmI request 
       * arrives from another module. 
       * @param lat the latitude of the postion to look up. Given in rad*1e8.
       * @param lon the longitude of the position to look up. Given in rad*1e8.
       * @param src the address of the original requester.
       */
      virtual void decodedWhereAmI(int32 lat, int32 lon, uint32 src);

      /** 
       * Called by NavServerComDecoder::dispatch when a search request 
       * arrives from another module. 
       * @param data      The request data already coded as a server request.
       * @param length    The size of the data block.
       * @param lat       The latitude to search from. 
       *                  (Redundant, but used for error checking.)
       * @param lon       The longitude to search from.
       *                  (Redundant, but used for error checking.)
       * @param country   The code for the country to search in. 
       *                  (Redundant, but used for error checking.)
       * @param city      The city string entered by the user.
       *                  (Redundant, but use for error checking.)
       * @param src       The address of the requester.
       * @param latOffset The offset into the data where the new latitude 
       *                  should be written. Defaults to -1, which 
       *                  means don't refresh.
       * @param lonOffset The offset into the data where the new longitude 
       *                  should be written. Defaults to -1, which 
       *                  means don't refresh.
       * @param hdgOffset The offset into the data where the new heading
       *                  should be written. Defaults to -1, which 
       *                  means don't refresh.
       */   
      virtual void decodedSearchRequest(const uint8* data, int length,
                                        int32 lat, int32 lon, uint8 hdg,
                                        uint32 country, const char* city,
                                        uint32 src, 
                                        int latOffset = -1, 
                                        int lonOffset = -1,
                                        int hdgOffset = -1);
      virtual void decodedSearchInfoRequest(int len, const uint8* data, 
                                            uint32 src);

      virtual void decodedVerifyLicenseKey(const char* key, const char* phone,
                                           const char* name, const char* email,
                                           const char* optional,
                                           uint32 region, uint32 src);

      virtual void decodedCellReport(const uint8* data, uint32 length, 
                                     uint32 src);

      /** 
       * Called by NavServerComDecoder::dispatch when a route request
       * arrives from another module. All latitude and lonitude
       * coordinates are specified in rad * 1e8.
       * @param toLat            Latitude to route to.
       * @param toLon            Longitude to route to.
       * @param fromLat          Latitude to route from.
       * @param fromLon          Longitude to route from.
       * @param heading          Current heading.
       * @param routeRequestorId The id of the Module that sent the 
       *                         original route request. Not
       *                         necesarily the same as
       *                         <code>src</code>.
       * @param oldRouteId       Contains the id of the old route if 
       *                         this is a reroute.
       * @param rerouteReason    Value from RerouteReson enum. Details
       *                         why a reroute was requested. Only
       *                         matters if <code>oldRouteId !=
       *                         0</code>.
       * @param src              the address of the requester.
       */   
      virtual void decodedRouteToGps(int32 toLat, int32 toLon, 
                                     int32 fromLat, int32 fromLon,
                                     uint16 heading, 
                                     uint32 routeRequestorId,
                                     int64 oldRouteId, uint8 rerouteReason,
                                     uint32 src);

      virtual void decodedMapRequest(int len, const uint8* data, uint32 src, 
                                     int latOffset = -1, int lonOffset = -1,
                                     int hdgOffset = -1, int speedOffset = -1);
      virtual void decodedVectorMapRequest(int len, const uint8* data, 
                                           uint32 src);
      virtual void decodedMultiVectorMapRequest(int len, const uint8* data, 
                                                uint32 src);

      /** Called by NavServerComDecoder::dispatch when a binary upload request 
       * arrives from another module. 
       * @param len  the size of the data block.
       * @param data the bainry block to send to the server.
       * @param src  the address of the original requester.
       */   
      virtual void decodedBinaryUpload(int len, const uint8 * data, 
                                       uint32 src);

      /** This method handles all connection notifications. 
       * Calls updateState with the proper event.
       * @param state the new connection state.
       * @param src   the sender of the message.
       */
      virtual void decodedConnectionNotify(enum ConnectionNotify, 
                                           enum ConnectionNotifyReason, 
                                           uint32 src);

      /** Sends a message to NavTask to tell it a new route is incoming.
       * @param req_id   The server id for this request.
       * @param packlen  The size of the route, whithout header.
       * @param ptui     The new server side Periodic Traffic Information
       *                 Interval.
       */
      void newRoute(uint8 req_id, int packlen, uint32 ptui);

      /** Send a chunk of the route to NavTask.
       * @param req_id the server id of the request resulting in this route.
       * @param chunk  a Buffer holding a chunk of route data
       * @param last   a boolean indicating if this chunk is the last one 
       *               received from the server. Default value: false.
       */
      void sendChunk(uint8 req_id, Buffer* chunk, bool last = false);

      /**
       * Signal to NavTask that it should keep its current route.
       * @param req_id The NavigatorServer request id of the reroute
       *               request.
       * @param routeId The id of the route that shall be kept. 
       *                For sanity check.
       * @param ptui The new server side Periodic Traffic Information
       *             Interval.
       */
      void keepRoute(uint8 req_id, int64 routeId, uint32 ptui);

      /** Called from ServerParser::decodeServerPacket. This function
       * sorts the reply as either unsolicited, solicited or faulty and passes
       * it on to the appropriate function.
       * This method calls updateState.
       * @param inBuf  a Buffer containing the reply data. The Buffers read
       *               position should be set to the first byte after the 
       *               status byte.
       * @param req_id the request id field from the reply header.
       * @param type   the type field from the reply header.
       * @param status The reply's status byte.
       * @param extendedError The Earth extended error code.
       * @param err Contains the extendedErrorString and statusMessage.
       * @param statusMessage The error text from server, if any.
       */
      void decodeBuffer( Buffer* inBuf, uint8 req_id, uint16 type, 
                         uint8 status, ErrorObj& err,
                         uint32 extendedError );


      /** Transmits a multicast progressMessage. 
       * @param status a value from the NavServerComConsumerPublic::ComStatus
       *               enum indicating the stage of communication.
       * @param type   a value from the MessageType enum indicating what 
       *               message is currently being sent or received. 
       * @param sent   bytes sent.
       * @param toSend total amount of bytes to send. 
       */
      void reportProgress(ComStatus status,
                          MessageType type = NAV_SERVER_INVALID, 
                          uint32 sent = 0, uint32 toSend = 0);

      void decodedExpiredTimer(uint16 timerID);

      void decodedCancelRequest(uint32 originalSrc, uint32 netmask, 
                                uint32 src);

      virtual void decodedRequestReflash(uint32 src);

      virtual void decodedPositionState(const struct PositionState &p, 
                                        uint32 src);

      /**
       * Set the nearest route coord.
       *
       * @param lat The latitude of the route coordinate.
       * @param lon The longitude of the route coordinate.
       */  
      virtual void setRouteCoordinate( int32 lat, int32 lon, int8 dir );

      virtual void decodedMessageRequest(const uint8* data, unsigned length,
                                         uint32 src);


      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedGpsSatelliteInfo(class GpsSatInfoHolder* /*v*/,
                                    uint32 /*src*/){}
      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedFileOperation(class GuiFileOperation* /*op*/,
                                    uint32 /*src*/){}
      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedRouteInfo(const RouteInfo &/*r*/,
                                    uint32 /*src*/){}
      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedRouteList(Buffer* /*buf*/,
                                    uint32 /*src*/, uint32 /*dst*/){}


      /**Unused. included only to comply with certain interfaces.*/
      virtual 
      void decodedInvalidateRoute(bool /*newRouteAvailable*/,
                                  int64 /*routeid*/,
                                  int32 /*tLat*/, int32 /*lLon*/, 
                                  int32 /*bLat*/, int32 /*rLon*/,
                                  int32 /*oLat*/, int32 /*oLon*/, 
                                  int32 /*dLat*/, int32 /*dLon*/,
                                  uint32 /*src*/){ /* Unused */}

      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedNTRouteReply(int64 /*routeid*/, uint32 /*src*/, uint32 /*dst*/){}

      void decodedParamSync(uint32 src);

      virtual void decodedSendRequest( const MsgBuffer* buf );

      /**
       * Turns an extended error code into an Nav2 error code.
       *
       * @param extendedError The code to convert.
       * @return The Nav2 error code.
       */
      enum Nav2Error::ErrorNbr extenedErrorToNav2Error( 
         uint32 extendedError ) const;

      /**
       * Force to disconnect. 
       * Typically used when a forced disconnect is necessary
       * (for instance after getting Connection: Close).
       */
      void forceDisconnect();

      /**
       *
       */
      class RequestListItem* addNrpRequest(class NavRequestPacket* nrpData);

   private:
      /**
       * Set the NT_ServerTrafficUpdatePeriod parameter, if the
       * parameter is not MAX_UINT32.
       * @param ptui The new Server set Periodic Traffic Update
       *             Interval.
       */
      void setServerPtui(uint32 ptui);
   
      /**
       * Check and move pending requests to be sent again.
       * 
       * @return True if reconnect is necessary, false otherwise.
       */
      bool checkAndSetupPendingRequestsToResend(); 

      /** States in the NavServerCom state machine.*/
      enum SessionStates {
         /** Invalid.*/
         invalidSessionState = 0,
         /** We cant start any sessions yet, we're waiting for parameters.*/
         noParams,
         /** No session is in progress. */
         noSession,
         /** Waiting for a connection message from the serial provider.*/
         waitingForConnect,
         /** Exchanging parameters with the server. */
         sendingParams,
         /** Sending requests to the server. */
         sendingRequests,
         /** Polling server. */
         serverPoll,
         /** Wainting for Disconnection message from the serial provider. */
         waitingForDisconnect,
         /** We lost the connection in mid-session. Recovery in progress. */
         interrupted,
         /** The connection has timed out. */
         timedOut,
      };

      /** Events that cause us to change state. */
      enum Event{
         /** Invalid. */
         invalidEvent = 0,
         /** All parameters have arrived.*/
         allParamsRead,
         /** Received a request from another module. */
         receivedRequest,
         /** Received a connection message from the serial provider. */
         connectionEstablished,
         /** Received a parameter reply message from the server. */
         parameterReply,
         /** A request has been completely sent.*/
         sentRequest,
         /** Received a reply from the server.*/ 
         receivedServerReply,
         /***/
         receivedServerError,
         /** Received a poll reply from the server. Still more data.*/
         pollReply,
         /** Received the final data from the server. */
         finalPollReply,
         /** Received a disconnection message from the serial provider. */
         disconnected,
         /** */
         parameterChange,
         /** */
         timeout,
         /// Redirect order was received from the navigator server.
         redirect,
         /// Request to force a disconnect
         requestDisconnect,
         /// The authorization failed.
         unauthorized,
         /// shutdown has been requested and will be effected.
         shutdown,
      };

      int writeVersionFile(const char* verInfo, const char *version_file_name);

      /** The message sorting function. 
       * @param buf the MsgBuffer containing the message.  */
      virtual MsgBuffer* dispatch(MsgBuffer* buf);

      /** Called from SerialConsumerDecoder::dispatch. 
       * @param len the size of the in byte array.
       * @param in  a byte array containing the received serial data.
       * @param src the address of the module sending the data.
       */
      virtual void decodedReceiveData(int length, const uint8 *data, 
                                      uint32 src);

      //forward declaration
      class RefreshLocation;
      /** Sets up request data for incoming requests and puts them in the 
       * to-be-sent-list. Then calls updateState to tell that a new request  
       * has arrived.
       * @param in_request the request type.
       * @param data       the request data.
       * @param refresh    a pointer to a RefreshLocation object that is to 
       *                   be stored with the request. May be NULL.
       * @param src        the sender of the request.
       * @return           a pointer to the RequestListItem inserted into 
       *                   m_requestList.
       */
      class RequestListItem* simpleRequest(uint16 in_request, Buffer* data, 
                                     RefreshLocation* refresh,
                                     uint32 src);

      /** Sets up request data for incoming requests and puts them in the 
       * to-be-sent-list. Then calls updateState to tell that a new request  
       * has arrived. 
       * This version of simpleRequest is just an alias for the
       * four-argument version of simpleRequest, with the refresh
       * argument set to NULL.
       * @param in_request the request type.
       * @param data       the request data.
       * @param src        the sender of the request.
       * @return           a pointer to the RequestListItem inserted into 
       *                   m_requestList.
       */
      class RequestListItem* simpleRequest(uint16 in_request, Buffer* data, 
                                     uint32 src)
      { 
         return simpleRequest(in_request, data, NULL, src);
      }

      /** Replies to the sender of the route request, indicating that the
       * route is avaliable at NavTask. Also removes the request data from 
       * the sent-list.
       * @param reqID the server id of this operation.
       */
      void routeReply(uint16 reqID);

      /** Sends an error message to a requester.
       * @param request the data from the sent-list.
       */
      void sendError( const ErrorObj& err, uint32 src = 0 ) const;

      /**
       * Helper function for decodeBuffer that handles error codes.
       * 
       * @param status The error code.
       * @param err Containing the extendedErrorString and statusMessage.
       * @param extendedError The extended error code.
       * @param src The address of the requesting module. 
       * @return The event that is the result of the error handling.       
       */
      enum Event handleErrorStatus( enum ReplyStatus status, 
                                    ErrorObj& err,
                                    uint32 extendedError,
                                    uint32 src );

      /**
       * Helper function for decodeBuffer that handles route replies
       * with special status set.
       * @param rrs The RouteReplyStatus, which is the request
       *            specific status of the route reply.
       * @param input The route reply data buffer. 
       * @param type The type of the request which reply we are
       *             handling. For now, always NAV_SERVER_ROUTE_REPLY
       * @param request The data regarding this request. 
       * @return The event that is the result of the route reply. 
       */
      enum Event 
      handleRouteReplyStatus(enum RouteReplyStatus rrs,
                             class Buffer& input, uint16 type, 
                             class RequestListItem* request);

      /** Handles server replies that have no request data filed in the 
       * sent-list. This may be because of error or because no is filed
       * for certain requests.
       * @param input  the Buffer containing the reply data. The Buffers 
       *               read position should be set to the first byte after 
       *               the status byte.
       * @param type   the type field from the reply header.
       * @param status the status byte from the reply.
       * @param err    Contains the extendedErrorString and statusMessage.
       * @param extendedError The Earth extended error code.
       * @return       the event to send to updateState. 
       */
      enum Event unsolicitedMessage(
         Buffer& input, uint16 type, uint8 status, ErrorObj& err,
         uint32 extendedError );

      /** Handle messages that have corresponding records in the sent-list.
       * @param input   a Buffer containing the reply data. The Buffers read 
       *                position should be set to the first byte after the 
       *                state byte.
       * @param type    the type field from the reply header.
       * @param request the request data for this reply.
       * @return        the event to send to updateState. Presently always
       *                Event::receivedServerReply.
       */
      enum Event solicitedMessage(Buffer& input, uint16 type,
                                  RequestListItem*& request);
      /** This method decodes the reply of a server poll. If the reply 
       * contained data it passes it on to the binaryDownload function.
       * @param buffer the Buffer containing the reply. The read position 
       *               should be set to the first byte AFTER the status byte.
       * @param status the status byte from the reply.
       * @return either Event::pollReply or Event::finalPollReply, depending
       *         on the status byte.
       */
      enum Event decodePollServerReply(Buffer& buffer,uint8 status);

      /** This method decodes the parameter block sent from the server
       * and distributes the parameters it contains to their final
       * destinations. 
       * @param buffer the Buffer containing the parameter block sent by the
       *               server.
       * @param status the status byte from the reply.
       * @param err Contains the extendedErrorString and statusMessage.
       * @param extendedError The extened error that can be used when
       *                      showing the error in Content Window.
       */
      enum Event decodeParameterReply( Buffer& buffer,uint8 status, 
                                       ErrorObj& err,
                                       uint32 extendedError );

      /** Decodes a search reply and sends a reply to the original requester.
       * @param buffer a Buffer containing the reply data. The Buffers 
       *               reading position should be set to the byte after 
       *               the status byte.
       * @param src    the address of the original requester.
       */
      void decodeSearchReply(Buffer& buffer, uint32 src);

      /** Decodes a WhereAmI-reply, and then encodes it in a new message to 
       * the requester.
       * @param buf a Buffer containing the reply data.
       * @param src the address of the requester, where to send the reply.
       */
      void decodeWhereAmIReply(Buffer& buf, uint32 src);

      /** Huh! shouldn't ever be needed. */
      void decodeGpsInitReply(Buffer& buf, uint32 src);

      /**
       * Decodes a message reply.
       * @param buf The buf.
       * @param src The src.
       */
      void decodeMessageReply(Buffer& buf, uint32 src);


      /** This method is called whenever one of the specified event in the 
       * enum Event happens. This method updates the state. 
       * The state shouldn't be updated anywhere else.
       * @param event an event specified in the Event enum.
       */
      void updateState(enum Event event);

      /** This method is called whenever the state changes. 
       * Depending on the new state appropriate action is taken.
       */
      void stateChangeAction(enum SessionStates fromState);

      /***/
      bool allNecessaryParamsSet();


      /**
       * decrypts stored IMEI.
       *
       * @param key The IMEI to decrypt.
       * @param keyLen The length of key.
       * @return A new decrypted IMEI. Is keyLen long.
       */
      uint8* decryptIMEI( const uint8* key, int keyLen ) const;


      /**
       * Ecrypts an IMEI for storage.
       *
       * @param key The IMEI to encrypt.
       * @param keyLen The length of key.
       * @return A new encrypted IMEI. Is keyLen long.
       */
      uint8* encryptIMEI( const uint8* key, int keyLen ) const;

      /**
       * This is part of a Work around for a race between timer and message
       * queue decode.
       *
       * The thing that could happen was that a timer expired and added it's
       * timer expired message to the message queue and while the message 
       * was still in the queue we set the timer again to a new timeout 
       * value. When the message then arrived we triggered the 
       * decodedExpiredTimer even though we acutally had set the timer 
       * again to a new timeout value. 
       *
       * In this function we check a local timestamp if the timer expired can 
       * actually be the correct one based on its timeout. 
       */
      bool reallyTriggerTimout(uint16 timerID);

      /**
       * This is part of a Work around for a race between timer and message
       * queue decode.
       *
       * The thing that could happen was that a timer expired and added it's
       * timer expired message to the message queue and while the message 
       * was still in the queue we set the timer again to a new timeout 
       * value. When the message then arrived we triggered the 
       * decodedExpiredTimer even though we acutally had set the timer 
       * again to a new timeout value. 
       *
       * This is a convenience function that should always be used to set 
       * the timeout timer since this one also makes sure to set the 
       * timestamp that is then used in the function above.
       */
      void setTimeoutTimer(uint32 timeout);

   private:
      /** A layer to put between the RequestListItem object and the
       * Buffer when the I-Am-Here fields in a request should be
       * updated just before it is sent to the server. */
      class RefreshLocation{
      public:
         /** The offset into the buffer where a new latitude value
             should be written. */
         int latOffset;
         /** The offset into the buffer where a new longitude value
             should be written. */
         int lonOffset;
         /** The offset into the Buffer where a new heading value
          * should be written. */
         int headingOffset;
         /** Simple constuctor that just copies the parameters into
          * the corresponding variables.  
          * @param buf a pointer to the Buffer the offsets pertain to.
          * @param lat the offset to the Latitude data.
          * @param lon the offset to the longitude data.
          */
         RefreshLocation(int lat, int lon, int heading = -1) : 
            latOffset(lat), lonOffset(lon), headingOffset(heading)
         {}

         static RefreshLocation* NewIf(int32 testLat, int32 testLon, 
                                       int lat, int lon, int heading = -1)
         {
            return NewIf(((testLat == MAX_INT32) && (testLon == MAX_INT32)),
                         lat, lon, heading);
         }
         static RefreshLocation* NewIf(bool refresh, 
                                       int lat, int lon, int heading = -1)
         {
            if(refresh){
               return new RefreshLocation(lat, lon, heading);
            }
            return NULL;
         }

      };

      class ServerRequestData : public RequestData {
         Buffer* m_requestBuffer;
         RefreshLocation* m_refresh;
      public:
         ///Adds a previously created RefreshLocation object to the
         ///request. The ServerRequestData object takes ownership of
         ///the RefreshLocation object and will destory it in time.
         void addRefresh(RefreshLocation* rl);
         ServerRequestData(Buffer* requestMsg);
         ~ServerRequestData();
         ///Creates a new RefreshLocation object to the request.
         void addRefresh(int latOffset, int lonOffset, int hOffset=-1);
         Buffer* getBuffer();
         int getLengthToSend();
         void refreshData(int32 lat, int32 lon, uint8 heading);
      };

      class NrpDataHolder : public RequestData {
         NavRequestPacket* m_nrpData;
      public:
         NrpDataHolder(NavRequestPacket* nrpRequest);
         ~NrpDataHolder();
         NavRequestPacket* getNrpData();
      };

      /**
       * A pointer to the public interface that is below us,
       * i.e. the server connection.
       */
      SerialProviderPublic* m_ser_provider;

      /**
       * This object is used to decode the serial data from
       * server (lower module).
       */
      SerialConsumerDecoder m_ser_consumer_decoder;

      /** A pointer to this objects consumer interface. This interface
          is mainly used to send data to this object, and maybe the
          pointer is unnecessary.*/
      SerialConsumerPublic* m_serialConsumer;

      NavTaskConsumerDecoder m_navTaskDecoder;

      /** Calculates the data to send in the current session*/
      uint32 calcDataToSend();

      /** Sequence number for requests to the Navigator Server.*/
      uint8 m_reqID;

      /** How much data was sent in this session. */
      uint32 m_sentData;

      /** List used to keep track of requests sent to server that we
          haven't received replies for yet. Silly really, as we should
          never have more than one outstanding request at the server.*/
      RequestList m_pendingList;

      /** List used to keep track of requests that haven't been sent
          to the server yet. */
      RequestList m_requestList;

      /** Variable that keeps track of the state of the connection to
          the server. */
      Module::ConnectionNotify m_connectState;

      /** Interface to the NavTask module. Usefull for sending
          downloaded routes and whatnot. */
      NavTaskProviderPublic* m_navTaskProvider;

      /** Interface to the Parameter Module. */
      ParameterProviderPublic* m_paramProvider;

      /** Interface to all our consumers. Used to answer any requests. */
      NavServerComConsumerPublic* m_nscConsumer;

      /** Decoder for all messages concerned with infrastructure matters. */
      CtrlHubAttachedDecoder m_ctrlHubAttachedDecoder;

      /** Decoder for messages directed to this Module.  */
      NavServerComProviderDecoder m_comDecoder;

      /** Decoder for parameter messages. */
      ParameterConsumerDecoder m_paramDecoder;

      /** Block of NavClient-to-NavServer parameters. Includes the end
       * parameter. */
      ParameterBlock* m_paramBlock;
      /// chache of webusername parameters. Used to avoid setting
      /// UC_WebUser too often
      char* m_webUser;

      /** Keeps the RequestListItem of a sync request untill the poll data 
       * is downloaded. This is necessary since NSC doesn't start polling 
       * until the RequestLists are empty.*/
      RequestListItem* m_syncRequest;

      ///Holds addresses of those who wish to know about the result of
      ///ParameterBlock exchanges. This is typically those who have
      ///requested ParameterSync.
      ///It seems a bit wasteful to use a deque for this purpose.
      std::deque<uint32> m_paramSyncs;

      /** Holds the current state for NavServerCom. */
      enum SessionStates m_sessionState;

      /** Pointer to a helper object that handles parsing and encoding
          of some server messages. */
      ServerParser* m_parser;

      /** Vector of known NavigatorServers.*/
      std::vector<Server*>* m_servers;
      int m_serverIdx; 

      /** The id of the timeout timer.*/
      uint16 m_timeoutTimerID;
      /** The timeout time. */
      uint32 m_timeoutTime;
      uint16 m_fakeConnectTimer;
      /** The id of the newly added misc timer, which is used on some places 
          instead of the timeoutTimer since we have imposed a new limit 
          on the timeout timer so it could not be re-used in some cases.*/
      uint16 m_miscTimerID;
      /** The timestamp when the timeoutTimer was last set. This is used 
          to prevent a race when the timer is triggered but decodedExpTimer 
          still has not happend due to the message in the queue has not 
          reached its function. With this timestamp we just return in 
          decodedExpiredTimer if the timestamp is too young. */
      uint32 m_timeoutTimerTimestamp;
      /** The poll data offset.
       * This is supposed to be resest each session.*/
      uint32 m_pollOffset;
    
      /** Different disconnection strategies. */
      enum DisconnectStrategy {
         /** Invalid value. */
         invalidStrategy       = 0,
         /** Disconnect as soon as the session is complete. */
         disconnectImmediately = 0x01,
         /** Wait the short timeout before shutting down the connection. */
         shortTimeout          = 0x02,
         /** Wait the long timeout before shutting down the connection. */
         longTimeout           = 0x03
      };
      /** Shifts used to find the disconnection strategy associated
       * with an event.*/
      enum DisconnectStrategyShift{
         /** When the last request sent was a binary upload.*/
         synchronizeShift = 0,
         /** When the last request sent was a search.*/
         searchShift      = 2,
         /** When the last request sent was a route request.*/
         routeShift       = 4
      };
      /** Timeout for cached connections where necessary. The long version.*/
      uint32 m_longConnectionTimeout;
      /** Timeout for cached connections where necessary. The short version.*/
      uint32 m_shortConnectionTimeout;
      /** Disconnection strategy for dialup connections and the like.
       * Consists of several Disconnection strategies shifted per
       * event and then ORed together. */
      DisconnectStrategy m_disconnectStrategy;
      /** Used to store the last request of the request types
       * affecting disconnection strategy, that was sent.*/
      MessageType m_lastRequest;

      int32 m_lastLat;
      int32 m_lastLon;
      uint8 m_lastHeading;
      /// m/s * 32
      uint16 m_lastSpeed;

      ///The last kind of ProgressMessage sent.
      ComStatus m_lastCs;
      ///The message type sent in the last progressmessage
      MessageType m_lastType;
      ///The sent field of the last sent progress message
      uint32 m_lastSent;
      ///The tosend field of the last sent progress message
      uint32 m_lastToSend;
      ///A vector of booleans indicating what progress messages to send.
      bool m_csLookUp[numberOfStatuses];

      /// Used to select route optimizing criteria in route
      /// requests. Should be one of the values in the RouteCostType
      /// enum.
      uint32 m_routeType;
      uint32 m_routeTollRoads;
      uint32 m_routeHighways;
      /// Used to select what kind of vehicle the route should be
      /// optimized for. Should be a value from the TransportationType
      /// enum.
      uint32 m_transportationType;

      std::vector<int32> m_expireVector;
      enum ConnectionNotifyReason m_disconnectReason;

      ///Random seed used in symbian
      int64 m_seed;


      /// The IMEI XOR bytes.
      const byte* m_imeiXor;


      /// The length of the IMEI XOR bytes.
      uint32 m_imeiXorLen;


      /// The address of the module requesting the latest route.
      uint32 m_lastRouteAddress;


      /// The type of Serverlist used. Nav or Http.
      ParameterEnums::ParamIds m_serverListParam;


      /// The type of ServerlistChecksum used. Nav or Http.
      ParameterEnums::ParamIds m_serverListChecksumParam;


      /// The proxy, if any.
      char* m_proxyServerAndPort;


      /// The Tracking parasite
      Tracking* m_tracking;

      class OfflineTester* m_offlineTester;

      /// The maxnbroutstanding.
      uint32 m_maxnbroutstanding;
   };

   //===============================================================
   //======= inlines for NavServerCom::ServerRequestData ===========
   inline 
   NavServerCom::ServerRequestData::ServerRequestData(Buffer* requestMsg) : 
      m_requestBuffer(requestMsg), m_refresh(NULL)
   {
   }
                                             
   inline NavServerCom::ServerRequestData::~ServerRequestData()
   {
      delete m_requestBuffer;
      delete m_refresh;
   }
                                             
   inline void NavServerCom::ServerRequestData::addRefresh(int latOffset,
                                                           int lonOffset,
                                                           int hOffset)
   {
      m_refresh = new RefreshLocation(latOffset, lonOffset, hOffset);
   }

   inline void NavServerCom::ServerRequestData::addRefresh(RefreshLocation* rl)
   {
      m_refresh = rl;
   }

   inline Buffer* NavServerCom::ServerRequestData::getBuffer()
   {
      return m_requestBuffer;
   }
   inline int NavServerCom::ServerRequestData::getLengthToSend()
   {
      return m_requestBuffer ? m_requestBuffer->getLength() : 0;
   }


   //===============================================================
   //======= inlines for NavServerCom::NrpDataHolder  ==============
   inline 
   NavServerCom::NrpDataHolder::NrpDataHolder(NavRequestPacket* nrpRequest) : 
      m_nrpData(nrpRequest)
   {
   }
                                             
   inline NavServerCom::NrpDataHolder::~NrpDataHolder()
   {
      delete m_nrpData;
   }

   inline NavRequestPacket* NavServerCom::NrpDataHolder::getNrpData()
   {
      return m_nrpData;
   }

}


#endif
