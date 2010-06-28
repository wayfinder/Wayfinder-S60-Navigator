/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UI_CONTROL_H
#define UI_CONTROL_H

#include <deque>
#include <utility>
#include "NavRequestType.h"

class NavCtrlMMIMessage; // forward declaration.

//MN: Do we really need these ones?
/** Timeout for sending data messages. in milliseconds.*/
#define DS_BUSY_TIMEOUT 1000
#define DS_IDLE_TIMEOUT 1000

/** The value of this timeout influences both
 *  how often positions are sent to the GUI
 *  and for how long the GUI requests not taken 
 *  care of directly are stored.
 */
#define DS_DEFAULT_TIMEOUT 1000

#define LOST_PDA_MAX_TIMEOUTS 5


/// Max size of m_pending
#define MAX_PENDING 10

namespace isab{

   class GuiCtrl; // forward declaration.
   class AudioCtrl; // forward declaration.
   class AudioCtrlLanguage; // forward declaration.

   namespace UiCtrlInternal {
      struct GpsData;
      struct UiCtrlParams;
   }

   /**
    * Controls the user interface.
    *
    */
   class UiCtrl : public Module, 
                  public SerialConsumerInterface, 
                  public NavTaskConsumerInterface, 
                  public CtrlHubAttachedInterface,
                  public NavServerComConsumerInterface,
                  public ParameterConsumerInterface,
                  public DestinationsConsumerInterface,
                  public ErrorProviderInterface
   {
      friend class GuiCtrl;
      friend class AudioCtrl;
      
      public:
       
      /**
       * Constructor that takes an SerialProviderPublic.
       *
       * @param spp The SerialProviderPublic.
       * @param httpProto If talking HTTP to server.
       * @param useIAPProxy If to use proxy setting of IAP.
       * @param hardIapProxyHost The hardcoded IAP proxy host, used if 
       *                         useIAPProxy and not NULL.
       * @param hardIapProxyPort The hardcoded IAP proxy port, see 
       *                         hardIapProxyHost.
       * @param userClipString   transform the web username by clipping
       *                         the string from the beginning.
       */
      UiCtrl(SerialProviderPublic* spp, 
            Nav2Error::Nav2ErrorTable * errorTable,
            AudioCtrlLanguage * audioSyntax,
            const char *serialNumber,
             bool httpProto = false,
             bool useIAPProxy = false,
             const char* hardIapProxyHost = NULL,
             uint16 hardIapProxyPort = 0,
             const char* userClipString = NULL);

      /**
       * Destructor.
       */
      virtual ~UiCtrl();

      /**
       * Tells whether the GUI has connected.
       */
      bool connectedToGui();

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
          * Called when the status of the connection to the GUI
          * is changed.
          */
         virtual void decodedConnectionNotify(enum ConnectionNotify, 
                                              enum ConnectionNotifyReason,
                                              uint32 src);

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
          * All error messages that causes request to other modules in 
          * Nav2 to fail, ends up here.
          *
          * @param err       The error causing the request to fail.
          * @param src      Address and message id from the module sending
          *                 the error.
          * @param dst      Address and message id of the request that 
          *                 failed.
          */
         virtual void decodedSolicitedError( const ErrorObj& err, 
                                             uint32 src, uint32 dst );

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
       * Error provider functions.
       */
      //@{      
         /**
          * This method is called when one of the other modules 
          * sends an unsolicited error.
          *
          * @param err      The error sent by the module.
          * @param src      The address of the sending module.
          */
         virtual void decodedUnsolicitedError( const ErrorObj& err, 
                                               uint32 src );
      //@}

      /**
       * NavServerCom functions
       */
      //@{
         /**
          * Called when sync destination reply has been decoded.
          * XXX: Implement.
          */
         virtual void decodedSyncDestinationsReply(std::vector<Favorite*>& fvs,
                                                   std::vector<uint32>& rmd,
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
                                      const SearchArea* const* a,
                                      unsigned nItems, 
                                      const FullSearchItem*const* fsi,
                                      uint16 begindex, uint16 total,
                                      uint32 src, uint32 dst);
      virtual void decodedSearchInfoReply(uint16 num, const char* const* id,
                                          const unsigned* index,
                                          unsigned numInfo,
                                          const AdditionalInfo*const* info,
                                          uint32 src, uint32 dst);

      virtual void decodedMessageReply(const uint8* data, unsigned length,
                                       uint32 src, uint32 dst);

      virtual void decodedLicenseReply(bool keyOk, bool phoneOk, bool regionOk,
                                       bool nameOk, bool emailOk, 
                                       GuiProtEnums::WayfinderType type,
                                       uint32 src, uint32 dst);


         /**
          * Called when a route reply is received.
          *
          * @param src The id of the module?
          * @param dst The id of the request that this is a reply for?
          * DEPRECATED!
          */
         virtual void decodedRouteReply( uint32 src, uint32 dst );

      virtual void decodedMapReply(const BoxBox& bb, uint32 realWorldWidth,
                           uint32 realWorldHeight, uint16 imageWidth,
                           uint16 imageHeight,
                           enum MapEnums::ImageFormat imageType, 
                           uint32 imageBufferSize, 
                           const uint8* imageBuffer, uint32 dst);
      virtual void decodedVectorMapReply(const char* /*request*/, uint32 /*size*/,
                                         const uint8* /*data*/, uint32 /*dst*/)
      {
         //This function temporarily left blank
      }

      virtual void decodedMulitVectorMapReply(uint32 size,
                                              const uint8* data, uint32 dst);


      virtual void decodedForceFeedMuliVectorMapReply(
         uint32 size, const uint8* data, uint32 dst );


      virtual void decodedCellConfirm(uint32 size, const uint8* data,
                                      uint32 dst);


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
      virtual void decodedProgressMessage( NavServerComEnums::ComStatus status, 
                                           GuiProtEnums::ServerActionType type,
                                           uint32 done, uint32 of,
                                           uint32 src, uint32 dst );

      virtual void decodedLatestNews(uint32 checksum, const uint8* data, 
                                     unsigned length, uint32 src, 
                                     uint32 dst);
      virtual void decodedParamSyncReply(GuiProtEnums::WayfinderType wft, 
                                         uint32 src, uint32 dst);

      virtual void decodedReply( 
         navRequestType::RequestType type, NParamBlock& params, 
         uint8 requestVer, uint8 statusCode, const char* statusMessage,
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
            const struct PositionState& p, uint32 src );

         /**
          * Set the nearest route coord.
          *
          * @param lat The latitude of the route coordinate.
          * @param lon The longitude of the route coordinate.
          */  
         virtual void setRouteCoordinate( int32 lat, int32 lon, int8 dir );

         /**
          * Called when satellite info is available.
          *
          */
         virtual void decodedGpsSatelliteInfo(class GpsSatInfoHolder *v,
               uint32 src);

         /**
          * Called when NavTask wishes to send a File operation
          * message to the GUI.
          */
         virtual void decodedFileOperation(class GuiFileOperation *op,
               uint32 src);

         /**
          * Called when route info is received from NavTask.
          * 
          * @param r The RouteInfo with data.
          * @param src The id of the module?
          */
         virtual void decodedRouteInfo( 
            const RouteInfo& r, 
            uint32 src );


         /**
          * Called when a route list is received from NavTask.
          * 
          * @param buf A buffer containing the route list. Can be expanded by
          *            creating a new RouteList(buf)
          * @param src The id of the module?
          */
         virtual void decodedRouteList( 
            Buffer *buf,
            uint32 src, uint32 dst );


         /**
          * Called when a route reply is received. (From NavTask)
          *
          * @param src The id of the module?
          * @param dst The id of the request that this is a reply for?
          */
         virtual void decodedNTRouteReply(int64 routeid, uint32 src, 
                                          uint32 dst );

         /**
          * Called whenever a new route has been downloaded, and 
          * we're following that route instead.
          */
         virtual void decodedInvalidateRoute(bool newRouteAvailable,
                                             int64 routeid,
                                             int32 tLat, int32 lLon, 
                                             int32 bLat, int32 rLon,
                                             int32 oLat, int32 oLon, 
                                             int32 dLat, int32 dLon,
                                             uint32 src);
      //@}

      /** 
       * ParameterConsumer functions.
       */
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
   virtual void decodedParamValue(uint32 paramId, 
                                  const uint8* data, 
                                  int size,
                                  uint32 src,
                                  uint32 dst);
      //@}

      
      /**
       * Destinations modlule methods.
       */
      //@{  
      virtual void decodedGetFavoritesReply( std::vector<GuiFavorite*>& guiFavorites, 
                                             uint32 src, uint32 dst );
      virtual void decodedGetFavoritesAllDataReply( std::vector<Favorite*>& favorites, 
                                             uint32 src, uint32 dst  );
      virtual void decodedGetFavoriteInfoReply( Favorite* favorite, 
                                             uint32 src, uint32 dst  );
      virtual void decodedFavoritesChanged(  uint32 src, uint32 dst  );

      virtual void decodedDestRequestOk(  uint32 src, uint32 dst );

      //@}

      enum calledGPSConnectionMode_t {
         ConnectToGPSCalled,
         DisconnectToGPSCalled,
         nothingCalled
      };

      calledGPSConnectionMode_t getcalledGPSConnectionMode() const {
         return m_calledGPSConnectionMode;
      }

      void setcalledGPSConnectionMode( calledGPSConnectionMode_t m ) {
         m_calledGPSConnectionMode = m;
      }

   protected:
      /**
       * Constants.
       */
      //@{

      /**
       * This value multiplicated with m_guiPositionUpdateTimeOut
       * is the time before non handled Gui messages are timed out
       * in milliseconds.
       */
      enum { GUI_MESS_TIMEOUT = 30 };

      //@}


      // XXX move these to parameter module.
      const char* getVersionString();
      uint32 getSerialNo();
      const char* getSoftwareVersion();

      /**
       * Called by hub when message is sent to this module?
       *
       * @param buf The request Msg?
       * @return MsgBuffer with the reply?
       */
      virtual class MsgBuffer* dispatch(class MsgBuffer* buf );


      /**
       * Internal functions.
       */
      //@{   

         /**
          * Checks if all params have been received from the 
          * parameter module.
          *
          * If this method returns true for the first time, 
          * it calls handleWaitingMessages before returning.
          *
          * @param print If this parameter is set to true,
          *              an not all parameter have been received,
          *              the method prints an info message
          *              telling what parameter(s) that has not
          *              been received yet.
          */
         bool allParamsReceived(bool print = false);

         /**
          * Call this method to indicate that a parameter has
          * been received from parameter module. It will _NOT_ notify
          * the GUI.
          */
         void paramReceivedNotSimpleParam(enum ParameterEnums::ParamIds paramId);

         /**
          * Call this method to indicate that a parameter has
          * been received from parameter module. It will notify
          * the GUI.
          */
         void paramReceived(enum ParameterEnums::ParamIds paramId,
                            const class GuiProtEnums::ParameterTypeHolder& guiParamId);

         /**
          * Goes through the waiting messages queue and sends them
          * one at a time ot m_guiCtrl->messageReceived.
          */
         void handleWaitingMessages();

         /**
          * Initializes the IAP parameter to IMEI:-1
          */
         void SendDefaultIAP();
         /**
          * Throws away to old Gui messages and responds to the
          * Gui with a request failed message.
          */
         void checkWaitingMessagesTimeOut();

         /** 
          * Sends an ExtendedError message to the NavClient. 
          * 
          * @param errorCode   A value from the NavErrorType enum.
          * @param errorID     The ID of the request that caused an error.
          * @param errorString an error message.
          */
         void sendExtendedError( uint16 errorCode, uint16 errorID,
                                 const char* errorMessage = NULL );
      //@}
         






   private:
      // communicate with the serial provider.
      SerialProviderPublic* m_provider;         
      // communicate with navtask
      NavTaskProviderPublic * m_navTaskProvider;
      // communicate with the parameter module.
      ParameterProviderPublic * m_paramProvider;       
      /// The interface to nav server com
      NavServerComProviderPublic* m_serverProvider;
      /// Interface to the destinations module.
      DestinationsProviderPublic* m_destProvider;  

      // decoders.
      SerialConsumerDecoder        m_serialDecoder;  // XXX: not set!
      NavTaskConsumerDecoder       m_navTaskDecoder; // XXX: not set
      NavServerComConsumerDecoder  m_nscDecoder;     // XXX: not set!
      CtrlHubAttachedDecoder       m_hubDecoder;     // XXX: not set!
      ParameterConsumerDecoder     m_paramDecoder;   // XXX: not set!
      DestinationsConsumerDecoder  m_destDecoder;
      ErrorProviderDecoder         m_errorDecoder;
      
      /**
       * This helper object is used for handling messages sent to and
       * from the GUI. It also cashes data used by the GUI.
       *
       * It has the roll NavCtrl had in the PDA.
       */
      GuiCtrl* m_guiCtrl;

      /**
       * This helper object is used for handling the audio
       * messages and streaming them correctly.
       */
      AudioCtrl* m_audioCtrl;

      /// Sequence id to send next to display.
      uint16 m_navSeqID;
      /// Expected id from display next.
      uint16 m_pdaSeqID;

      /**
       * The current timeout time for the timer.
       *
       * The value of this timer also influences
       * for how long the GUI requests not taken 
       * care of directly are stored.
       */
      uint16 m_guiPositionUpdateTimeOut;
      
      /// The id of the timer
      uint16 m_guiPositionUpdateTimerId;





//MN: Remove these when communication goes directly to GUI.
//@{
      /// If contact with display
      bool m_gotPda;

      /// If display is subscribing, to route info.
      bool m_subscribing;

      /// Counter for waiting for reply from display.
      uint16 m_noPda;
//@}


      /// Parameters cached here:
      //@{
         char* m_dataPhoneNo;
         char* m_voicePhoneNo;
         char* m_userID;
         char* m_passwd;
         char* m_serverHostname;
      //@}

         
      /**
       * Is set to true when all parameters hav been received
       * from the parameter module.
       */
      bool m_allParamsReceived;

      /**
       * Stores messages until they can be dealt with.
       *
       * The uint32 member of the pair is counted down
       * until it reaches 0. Then it times out. Initially,
       * this member is set to GUI_MESS_TIMEOUT.
       */
      //@{
         std::deque<std::pair<GuiProtMess*, uint32>* >* m_waitingMessages;
         
#ifdef USE_THINCLIENT
         // Remove this one when we have stopped using 
         // the ThinClient GUI prot.
         std::deque<std::pair<NavCtrlMMIMessage*, uint32>* >* m_waitingNavCtrlMessages;
#endif
      //@}



      /** Buffer to store unparsed data in. */
      Buffer* m_parseBuffer;

      /**
       * Allocates src if needed and copies name to it.
       *
       * @param src The string to copy to.
       * @param name The string to copy.
       */
      void setCharValue(char*& src, const char* name);

      /// If module is shuting down.
      bool m_shutdown;

      bool m_connectedToGui;

      /**
       * Oustanding requests to Nav2.
       *
       * Table describes the use of request list in UiCtrl.
       *
       * Request | ThinClient GUI   | Nav2 GUI
       * Memeber | Protocol         | Protocol
       * --------------------------------------------
       * request | ThinClientGuiPr\ | GuiProtEnums::
       *         | otEnums::        | MessageType
       *         | PacketNumber_t   |
       *         |                  |
       * reqID   | ???????????????? | GUI sequence id.
       *         |                  |
       * src     | Nav2 message id  | Nav2 message id.
       *         |                  |
       * status  | 1 if NavTask     |
       *         | invalidate route |
       *         | or NavServerCom  |
       *         | route downloaded |
       *         | has been         |
       *         | received.        |
       *         | Otherwise 0.     |
       *
       * RequestList::dequeue(uint16 requestID) returns
       * the item with the right reqID member.
       *
       * RequestList::dequeue(uint32 requestID) returns
       * the item with the right src member.
       */
      class RequestList *m_pending;

      //MN: Is this one needed? Constructor?
      /// GPS position data.
      struct UiCtrlInternal::GpsData *m_lastGpsData;

      /** Temporary hack - FIXME
       * This is used to cached the last route list so that 
       * playSoundForCrossing() has access to the information. 
       */
      class RouteList *m_lastRouteList;

      /**
       * List with the parameters ids UiCtrl should 
       * ask from the paramter module.
       *
       * UiCtrlParams also has a member telling if 
       * a parameter has been received.
       */
      struct UiCtrlInternal::UiCtrlParams* m_uiCtrlParams;

      /**
       * Size of m_uiCtrlParams.
       */
      uint32 m_uiCtrlParamsSize;


      /**
       * Handle requests from other part of UiCtrl for
       * the sound for a specific crossing. 
       * See AudioCtrl.
       */
      void playSoundForCrossing(uint16 crossingNo);

      
      /**
       * Methods used for keeping track of requests to Nav2
       */
      //@{
         /**
          * NB! See commment on m_pending for how the request list is used. 
          *
          * Checks if a request already with the same GUI sequence id is 
          * pending then silently discards it.
          *
          * If not pending then try to allocate a new RequestListItem. If
          * that fails send error reply.
          *
          * @param guiSequenceID The id set in the request by the GUI.
          *
          * @return An allocated request, or NULL if a request with the
          *         guiSequenceID already exists or allocation failed for
          *         any reason.
          */
         class RequestListItem* checkForRequestAndCreate( uint32 guiMessageType, uint16 guiMessageId );

         /**
          * NB! See commment on m_pending for how the request list is used.
          *
          * Method used by ThinClient GUI protocol. 
          * XXX Remove it when the ThinClient protocol is no longer used!
          *
          * @return An allocated request or NULL if allocation
          *         did not succeed.
          */
         class RequestListItem* checkForRequestAndCreate();

         /**
          * NB! See commment on m_pending for how the request list is used.
          * 
          * Adds an request. If ID is MAX_UINT16 then an error is sent.
          * Used by messageReceived.
          *
          * @param nav2messageId The value returned by ModulePublic methods.
          *                      Also called src sometimes.
          *
          * @param req           The request to add. The src member of the 
          *                      request is set by this method.
          */
         void addRequest( uint32 nav2messageId, class RequestListItem* req );
      //@}

      
      /**
       * Debug methods and variables.
       */
      //@{
         /**
          * Keeps track of if a position has been printed.
          */
         bool m_printedPos;
         /**
          * Prints the data of a favorite.
          */
         void dumpFavorite(GuiFavorite* guiFav);
         void dumpFavorite(Favorite* fav);
      //@}

      /**
       * Variables used for status messages to the GUI.
       */
      //@{
         NavServerComEnums::ComStatus m_previousStatus;
         NavServerComEnums::MessageType m_previousMessage;
      //@}


      /// The type of Serverlist used. Nav or Http.
      enum ParameterEnums::ParamIds m_serverListParam;

      enum calledGPSConnectionMode_t m_calledGPSConnectionMode;

      public:
         Nav2Error::Nav2ErrorTable * m_errorTable;

         char *m_serialNumber;
         
   }; // UiCtrl

// ===================================================================
//                             Implementation of the inlined methods =

   
   inline CtrlHubAttachedPublic *UiCtrl::newPublicCtrlHub()
   {
      return new CtrlHubAttachedPublic(m_queue);
   }

   
   inline const char* UiCtrl::getVersionString()
   {
      return "Navigator";
   }
   inline uint32 UiCtrl::getSerialNo()
   {
      return 0xdeadbeef;
   }
   inline const char* UiCtrl::getSoftwareVersion()
   {
      return "2.5.0";
   }

} //namespace isab




#endif // UI_CONTROL_H

