/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef DISPLAY_SERIAL_H
#define DISPLAY_SERIAL_H


#include "arch.h"
#include "Module.h"
#include "Serial.h"
#include "Buffer.h"
#include "RequestList.h"
#include "NavTask.h"
#include "CtrlHub.h"
#include "Log.h"
#define I_AM_DISPLAY_SERIAL_AND_IN_DIRE_NEED_OF_OUTDATED_ENUMS
#include "NavServerCom.h"
#include "Matches.h"
#include "PacketEnums.h"
#include "Parameter.h"

/** Timeout for sending data messages. in milliseconds.*/
#define DS_BUSY_TIMEOUT 1000
#define DS_IDLE_TIMEOUT 1000
#define DS_DEFAULT_TIMEOUT 1000
#define LOST_PDA_MAX_TIMEOUTS 5

#define NAV_VERSION_STRING_1  "Navigator"
#define MAX_VERSION_STRING   40

/// Max size of m_pending
#define MAX_PENDING 10

// Size of phonenumber, XXX: Parametermodule
#define MAX_PHONE_NO_SIZE  20

using namespace isab::NavServerComEnums;
namespace isab{

   typedef struct packetPersistentData {
      Buffer *decodingBuffer;
      int decodingState;
   } packetPersistentData_t;
   typedef packetPersistentData_t *packetPersistentData_p;


   using namespace Packet_Enums;
   // forward declaration, not pretty but necessary.
   class Packet;
   /**
    * DisplaySerial is for serial communication from ... to gui (display).
    *
    */
   class DisplaySerial : public Module, 
                         public SerialConsumerInterface, 
                         public NavTaskConsumerInterface, 
                         public CtrlHubAttachedInterface,
                         public NavServerComConsumerInterface,
                         public ParameterConsumerInterface
   {
      public:
      /**
       * Module functions
       */
      //@{
         /**
          * Called when shutdown starts.
          */
         virtual void decodedShutdownNow(int16 upperTimeout);

         /**
          * Called when shutdown is complete and no further calls will
          * be made.
          */
         virtual void treeIsShutdown();

         /**
          * Called when timer has expired.
          *
          * @param timerid The id of the timer that has expired.
          */
         virtual void decodedExpiredTimer( uint16 timerid );

         /**
          * Called when startup is done and module is ready.
          */
         virtual void decodedStartupComplete();

         /** 
          * Creates a new CtrlHubAttachedPublic object used to connect 
          * this module to the CtrlHub.
          * 
          * @return A new CtrlHubAttachedPublic object connected to the
          *         queue.
          */
         inline CtrlHubAttachedPublic* newPublicCtrlHub();
      //@}

      /**
       * NavServerCom functions
       */
      //@{
         /**
          * Called when sync destination reply has been decoded.
          * XXX: Implement.
          */
      virtual void decodedSyncDestinationsReply(std::vector<Favorite*>& favs, 
                                                std::vector<uint32>& removed,
                                                uint32 src, uint32 dst);

         /**
          * Called when binary upload has been received by ...
          * XXX: Implement 
          *
          * @param src The id of the module?
          * @param dst The id of the request.
          */
         virtual void decodedBinaryUploadReply( uint32 src, uint32 dst );

         /**
          * Called when binary download has been received by ...
          * XXX: Implement 
          *
          * @param data The data downloaded, is valid for this method.
          * @param length The length of the data.
          * @param src The id of the module?
          * @param dst The id of the request.
          */
         virtual void decodedBinaryDownload( const uint8* data, 
                                             size_t length, 
                                             uint32 src, uint32 dst );

         /**
          * Called when a where an I reply has been received.
          * XXX: Implement 
          *
          * @param country String with the name of the country where you 
          *                are.
          * @param municipal String with the name of the municipal where 
          *                  you are.
          * @param city String with the name of the city where you are.
          * @param district String with the name of the district in the 
          *                 city where you are.
          * @param streetname String with the name of the street where you 
          *                   are.
          * @param lat The latitude of your current position.
          * @param lon The longitude of your current position.
          * @param src The id of the module?
          * @param dst The id of the request.
          */
         virtual void decodedWhereAmIReply( const char* country, 
                                            const char* municipal, 
                                            const char* city, 
                                            const char* district, 
                                            const char* streetname, 
                                            int32 lat, int32 lon, 
                                            uint32 src, uint32 dst );

      virtual void decodedSearchReply(unsigned nAreas, 
                                      const SearchArea* const *sa,
                                      unsigned nItems, 
                                      const FullSearchItem* const *fsi,
                                      uint16 begindex, uint16 total,
         uint32 src, uint32 dst){}

         /**
          * Called when a route reply is received.
          *
          * @param src The id of the module?
          * @param dst The id of the request that this is a reply for?
          */
         virtual void decodedRouteReply( uint32 src, uint32 dst );

      virtual void decodedParamSyncReply(GuiProtEnums::WayfinderType wft, 
                                         uint32 src, uint32 dst)
      { /* this function intentionally left blank */}

         /**
          * Called when a progres message is received.
          *
          * @param status The status of the communication.
          * @param type The type of message that the progress is for.
          * @param done The number of sent bytes?
          * @param of The total amount of bytes?
          * @param src The id of the module?
          * @param dst The id of the request that this is progress message
          *            for
          */
      virtual void decodedProgressMessage( ComStatus status, 
                                           GuiProtEnums::ServerActionType type,
                                           uint32 done, uint32 of,
                                           uint32 src, uint32 dst );
      //@}

      /**
       * Serial Consumer functions.
       */
      //@{         
         /**
          * Called when data is received.
          * 
          * @param length The length of data.
          * @param data The data received.
          * @param src The id of the module?
          */
         virtual void decodedReceiveData( int length, const uint8 *data, 
                                          uint32 src );
      //@}

      /**
       * NavTask functions.
       */
      //@{
         /**
          * Called when a position state has been received.
          * 
          * @param p The new PositionState.
          * @param src The id of the module?
          */
         virtual void decodedPositionState( 
            const NavTaskConsumerPublic::PositionState& p, uint32 src );

         virtual void setRouteCoordinate( int32 lat, int32 lon, int8 dir );

         virtual void decodedInvalidateRoute(bool newRouteAvailable,
               int64 routeid,
               int32 tLat, int32 lLon, int32 bLat, int32 rLon,
               int32 oLat, int32 oLon, int32 dLat, int32 dLon,
               uint32 src);
      //@}

      /** ParameterConsumer functions. */
      //@{
      virtual void decodedParamNoValue(uint32 paramId,
                                     uint32 src,
                                     uint32 dst);
      virtual void decodedParamValue(uint32 paramId,
                                     const int32 * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst);
      virtual void decodedParamValue(uint32 paramId,
                                     const float * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst);
      virtual void decodedParamValue(uint32 paramId,
                                     const char * const * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst);
      virtual void decodedParamValue(uint32 paramId, const uint8* data, 
                                     int size, uint32 src, uint32 dst);

      //@}
       

      /// The implemented nav serial protocol version.
#ifndef _MSC_VER
      static const int protocolVersion = 8;
#else
      enum { protocolVersion = 8 };
#endif

      /**
       * Constructor that takes an SerialProviderPublic.
       *
       * @param spp The SerialProviderPublic.
       */
      DisplaySerial( SerialProviderPublic* spp );

      virtual ~DisplaySerial();

      // Macro for making a reply packet type from a request packet type
#define REPLY(i) PacketType(i | 0x80)      
      // Macro for checking if a packet type is a request.
#define NAV_DISPLAY_SERIAL_IS_REQUEST(i)  (!(i&0x80))

   protected:
      /**
       * Called by hub when message is sent to this module?
       *
       * @param buf The request Msg?
       * @return MsgBuffer with the reply?
       */
      virtual MsgBuffer* dispatch( MsgBuffer* buf );

      // parameters XXX: To be moved to Parameter Module?
      const char* getVersionString();
      uint32 getSerialNo();
      const char* getSoftwareVersion();

      /**
       * Internal functions.
       */
      //@{   
         /** 
          * Sends a simple packet to any connected NavClient.
          * 
          * @param packetType A value from the enum PacketType. 
          * @param packetID   The id of the packet to ack.
          */
         void sendAck( Packet_Enums::PacketType packetType, 
                       uint16 packetID );

         /** 
          * Sends an ExtendedError message to the NavClient. 
          * 
          * @param errorCode   A value from the NavErrorType enum.
          * @param errorID     The ID of the request that caused an error.
          * @param errorString an error message.
          */
         void sendExtendedError( uint16 errorCode, uint16 errorID,
                                 const char* errorMessage = NULL );
         /** 
          * Sends the data stored in the m_atestNavData structure to the
          * NavClient.
          *
          * @param packetType A value from the PacketType enum. 
          * @param packetID   The ID of the packet to send.
          */
         void sendLatestNavData( Packet_Enums::PacketType packetType, 
                                 uint16 packetID ); 
      //@}

   private: 
      // communicate with the serial provider.
      SerialProviderPublic* m_provider;
      SerialConsumerPublic* m_consumer;
      // communicate with navtask
      NavTaskProviderPublic * m_navTaskProvider;
      // communicate with the parameter module.
      ParameterProviderPublic * m_paramProvider;

      // decoders.
      SerialConsumerDecoder        m_serialDecoder;  // XXX: not set!
      NavTaskConsumerDecoder       m_navTaskDecoder; // XXX: not set
      NavServerComConsumerDecoder  m_nscDecoder;     // XXX: not set!
      CtrlHubAttachedDecoder       m_hubDecoder;     // XXX: not set!
      ParameterConsumerDecoder     m_paramDecoder;   // XXX: not set!

      /// The current timeout time for the timer
      uint16 m_timeout;
      
      /// The id of the timer
      uint16 m_timerId;

      /// If contact with display
      bool m_gotPda;

      /// If display is subscribing, to route info.
      bool m_subscribing;

      /// Counter for waiting for reply from display.
      uint16 m_noPda;

      /// Sequence id to send next to display.
      uint16 m_navSeqID;

      /// Expected id from display next.
      uint16 m_pdaSeqID;

      /// GPS position data.
      struct NavData{ 
         uint8 posQuality;
         uint8 speedQuality;
         uint8 headingQuality;
         int32 lat, lon;
         int32 routelat, routelon;
         uint8 heading;
         uint8 routeheading
         float speed;
         NavData() : posQuality(0), speedQuality(0), headingQuality(0), 
            lat(0), lon(0), routelat(0), routelon(0),
            heading(0), routeheading(0), speed(0) {}
      } m_lastNavData;

      /// Oustanding requests.
      RequestList m_pending;

      // Parameters cached here:
      char* m_dataPhoneNo;
      char* m_voicePhoneNo;
      char* m_userID;
      char* m_passwd;
      uint32 m_navID;
      char* m_serverHostname;

      void setParam(Packet* packet);
      void getParam(Packet* packet);
      void setCharValue(char*& src, const char* name);

      /// The whole of current route.
      uint8* m_FIXME_fullRoute;

      /// The length of current route.
      size_t m_FIXME_fullRouteLength;

      /// The interface to nav server com
      NavServerComProviderPublic* m_serverProvider;

      Buffer* m_tunnelBuffer;
      uint16  m_tunnelPacketNumber;

      /**
       * This function is the input handler for messages sent
       * by the PDA(display) side to the navigator. These are always
       * small messages (type and 2 x uint32 etc.)
       * The function handles the request and sends it to nav ctrl
       * if so needed. When the request is simple and it can be
       * handled here, an ack packet is sent directly.
       * For more complicated requests, the routine saves the
       * request id together with the id from nav_ctrl.
       * This can later be used to match requests to replys. 
       *
       * @return The function always returns zero.
       */
      int messageReceived( Packet* packet );

      /**
       * Packet handling functions. For handling of packets in 
       * messageReceived that require more than a few lines to handle.
       */
      //@{
         /**
          * Handles a navigator type packet.
          *
          * @param p The Packet.
          */
         void handleNavigatorType( Packet* p );

         /** Handles Navigator Parameters sent from NavCtrl.
          * @param packet the packet.
          */
         void handleNavigatorParams(Packet* packet);

         /**
          * Handles a get current route packet.
          *
          * @param p The Packet.
          */
         void handleGetCurrentRoute( Packet* p );

         /**
          * Handles a search packet.
          *
          * @param p The Packet.
          */
         void handleSearch( Packet* p );

         /**
          * Handles a download route packet.
          *
          * @param p The Packet.
          */
         void handleDownLoadRoute( Packet* p );

         /** Handles subscribe and unsubscribe packages.
          * @param packet the Packet.
          */
         void handleSubscribe( Packet* packet);
         
         /**
          * Handle a server parameters packet.
          *
          * @param p The Packet.
          */
         void handleServerParameters( Packet* p );
      
         /** Handles a data chunk.
          * @param packet the packet containing the data chunk.
          */
         void handleDataChunk(Packet* packet);
      //@}


      /**
       * Checks if a request already is pending then silently discard the 
       * packet.
       * If not pending then try to allocate a new RequestListItem. If
       * that fails send error reply.
       * If allocated ok then return that.
       * Used by messageReceived.
       *
       * @param id The id of the packet. 
       */
      RequestListItem* checkForRequestAndCreate( uint16 id );

      /**
       * Adds an request. If ID is MAX_UINT16 then an error is sent.
       * Used by messageReceived.
       *
       * @param req The request to add.
       * @param id The id of the display request.
       */
      void addRequest( RequestListItem* req, uint16 id );

      /// If module is shuting down.
      bool m_shutdown;


      ComStatus m_previousStatus;

      MessageType m_previousMessage;

   private:

      packetPersistentData_t m_packetPersistentData;

   };


// ========================================================================
//                                  Implementation of the inlined methods =
   
   inline const char* DisplaySerial::getVersionString()
   {
      return "Navigator";
   }
   inline uint32 DisplaySerial::getSerialNo()
   {
      return 0xdeadbeef;
   }
   inline const char* DisplaySerial::getSoftwareVersion()
   {
      return "2.5.0";
   }

   inline CtrlHubAttachedPublic *DisplaySerial::newPublicCtrlHub()
   {
      return new CtrlHubAttachedPublic(m_queue);
   }

}
#define ENUMNAMES
#ifdef ENUMNAMES
//typedef isab::Packet_Enums DS;
#define PACKET_TYPE_STRING(pt)                                                \
((!(pt & 0x80)) ?                                                             \
 ((pt == ERRORE) ? "ERRORE" :                                             \
  ((pt == TEST_DATA) ? "TEST_DATA" :                                      \
   ((pt == PDA_TYPE) ? "PDA_TYPE" :                                       \
    ((pt == NAVIGATOR_TYPE) ? "NAVIGATOR_TYPE" :                          \
     ((pt == RESET_NAVIGATOR) ? "RESET_NAVIGATOR" :                       \
      ((pt == GET_DATA_PHONE_NO) ? "GET_DATA_PHONE_NO" :                  \
       ((pt == SET_DATA_PHONE_NO) ? "SET_DATA_PHONE_NO" :                 \
        ((pt == GET_VOICE_PHONE_NO) ? "GET_VOICE_PHONE_NO" :              \
         ((pt == SET_VOICE_PHONE_NO) ? "SET_VOICE_PHONE_NO" :             \
          ((pt == GET_USER_ID) ? "GET_USER_ID" :                          \
           ((pt == GET_PASSWORD) ? "GET_PASSWORD" :                       \
            ((pt == SET_USER_ID) ? "SET_USER_ID" :                        \
             ((pt == SET_PASSWORD) ? "SET_PASSWORD" :                     \
              ((pt == EXTENDED_ERROR) ? "EXTENDED_ERROR" :                \
               ((pt == NAVIGATOR_PARAMETERS) ? "NAVIGATOR_PARAMETERS" :   \
                ((pt == SERVER_PARAMETERS) ? "SERVER_PARAMETERS" :        \
                 ((pt == START_ROUTE) ? "START_ROUTE" :                   \
                  ((pt == STOP_ROUTE) ? "STOP_ROUTE" :                    \
                   ((pt == CONTINUE_ROUTE) ? "CONTINUE_ROUTE" :           \
                    ((pt == GET_CURRENT_ROUTE) ? "GET_CURRENT_ROUTE" :    \
                     ((pt == GET_DESTINATIONS) ? "GET_DESTINATIONS" :     \
                      ((pt == WHERE_AM_I) ? "WHERE_AM_I" :                \
                       ((pt == SYNC_DESTINATIONS) ? "SYNC_DESTINATIONS" : \
                        ((pt == DOWNLOAD_ROUTE) ? "DOWNLOAD_ROUTE" :      \
                         ((pt == DELETE_DEST) ? "DELETE_DEST" :           \
                          ((pt == ADD_DEST) ? "ADD_DEST" :                \
                           ((pt == SEARCH) ? "SEARCH" :                   \
                            ((pt == CALL_SERVER_FOR_ROUTE) ?              \
                             "CALL_SERVER_FOR_ROUTE" :                        \
                             ((pt == CALL_VOICE_FOR_ROUTE) ?              \
                              "CALL_VOICE_FOR_ROUTE" :                        \
                              ((pt == NEW_ROUTE) ? "NEW_ROUTE" :          \
                               ((pt == DOWNLOAD_DATA) ? "DOWNLOAD_DATA" : \
                                ((pt == START_TUNNEL) ? "START_TUNNEL" :  \
                                 ((pt == DATA_CHUNK) ? "DATA_CHUNK" :     \
                                  ((pt == SUBSCRIBE) ? "SUBSCRIBE" :      \
                                   ((pt == UNSUBSCRIBE) ? "UNSUBSCRIBE" : \
                                    ((pt == GET_INPUT_DATA) ?             \
                                     "GET_INPUT_DATA" :                       \
                                     ((pt == INPUT_DATA) ? "INPUT_DATA" : \
                                      ((pt == ROUTE_INFO) ? "ROUTE_INFO" :\
                                       ((pt == STATUS_PACKET) ?           \
                                        "STATUS_PACKET" :                     \
                                        "UNKNOWN")))                          \
                                       )))))))))))))))))))))))))))))))))))) : \
 ((pt == ERRORE_REPLY) ? "ERRORE_REPLY" :                                 \
  ((pt == TEST_DATA_REPLY) ? "TEST_DATA_REPLY" :                          \
   ((pt == PDA_TYPE_REPLY) ? "PDA_TYPE_REPLY" :                           \
    ((pt == NAVIGATOR_TYPE_REPLY) ? "NAVIGATOR_TYPE_REPLY" :              \
     ((pt == RESET_NAVIGATOR_REPLY) ? "RESET_NAVIGATOR_REPLY" :           \
      ((pt == GET_DATA_PHONE_NO_REPLY) ? "GET_DATA_PHONE_NO_REPLY" :      \
       ((pt == SET_DATA_PHONE_NO_REPLY) ? "SET_DATA_PHONE_NO_REPLY" :     \
        ((pt == GET_VOICE_PHONE_NO_REPLY) ? "GET_VOICE_PHONE_NO_REPLY" :  \
         ((pt == SET_VOICE_PHONE_NO_REPLY) ? "SET_VOICE_PHONE_NO_REPLY" : \
          ((pt == GET_USER_ID_REPLY) ? "GET_USER_ID_REPLY" :              \
           ((pt == GET_PASSWORD_REPLY) ? "GET_PASSWORD_REPLY" :           \
            ((pt == SET_USER_ID_REPLY) ? "SET_USER_ID_REPLY" :            \
             ((pt == SET_PASSWORD_REPLY) ? "SET_PASSWORD_REPLY" :         \
              ((pt == EXTENDED_ERROR_REPLY) ? "EXTENDED_ERROR_REPLY" :    \
               ((pt == NAVIGATOR_PARAMETERS_REPLY) ?                      \
                "NAVIGATOR_PARAMETERS_REPLY" :                                \
                ((pt == SERVER_PARAMETERS_REPLY) ?                        \
                 "SERVER_PARAMETERS_REPLY" :                                  \
                 ((pt == START_ROUTE_REPLY) ? "START_ROUTE_REPLY" :       \
                  ((pt == STOP_ROUTE_REPLY) ? "STOP_ROUTE_REPLY" :        \
                   ((pt == CONTINUE_ROUTE_REPLY) ?                        \
                    "CONTINUE_ROUTE_REPLY" :                                  \
                    ((pt == GET_CURRENT_ROUTE_REPLY) ?                    \
                     "GET_CURRENT_ROUTE_REPLY" :                              \
                     ((pt == GET_DESTINATIONS_REPLY) ?                    \
                      "GET_DESTINATIONS_REPLY" :                              \
                      ((pt == WHERE_AM_I_REPLY) ? "WHERE_AM_I_REPLY" :    \
                       ((pt == SYNC_DESTINATIONS_REPLY) ?                 \
                        "SYNC_DESTINATIONS_REPLY" :                           \
                        ((pt == DOWNLOAD_ROUTE_REPLY) ?                   \
                         "DOWNLOAD_ROUTE_REPLY" :                             \
                         ((pt == DELETE_DEST_REPLY) ?                     \
                          "DELETE_DEST_REPLY" :                               \
                          ((pt == ADD_DEST_REPLY) ? "ADD_DEST_REPLY" :    \
                           ((pt == SEARCH_REPLY) ? "SEARCH_REPLY" :     \
                            ((pt == CALL_SERVER_FOR_ROUTE_REPLY) ?        \
                             "CALL_SERVER_FOR_ROUTE_REPLY" :                  \
                             ((pt == CALL_VOICE_FOR_ROUTE_REPLY) ?        \
                              "CALL_VOICE_FOR_ROUTE_REPLY" :                  \
                              ((pt == NEW_ROUTE_REPLY) ?                  \
                               "NEW_ROUTE_REPLY" :                            \
                               ((pt == DOWNLOAD_DATA_REPLY) ?             \
                                "DOWNLOAD_DATA_REPLY" :                       \
                                ((pt == START_TUNNEL_REPLY) ?             \
                                 "START_TUNNEL_REPLY" :                       \
                                 ((pt == DATA_CHUNK_REPLY) ?              \
                                  "DATA_CHUNK_REPLY" :                        \
                                  ((pt == SUBSCRIBE_REPLY) ?              \
                                   "SUBSCRIBE_REPLY" :                        \
                                   ((pt == UNSUBSCRIBE_REPLY) ?           \
                                    "UNSUBSCRIBE_REPLY" :                     \
                                    ((pt == GET_INPUT_DATA_REPLY) ?       \
                                     "GET_INPUT_DATA_REPLY" :                 \
                                     ((pt == INPUT_DATA_REPLY) ?          \
                                      "INPUT_DATA_REPLY" :                    \
                                      "UNKNOWN"                               \
                                      ))))))))))))))))))))))))))))))))))))))
#else
#define PACKET_TYPE_STRING(pt) #pt
#endif
#endif // DISPLAY_SERIAL_H


