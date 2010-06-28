/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef GUI_CONTROL_H
#define GUI_CONTROL_H


#define PENDING_REQUESTS 5

#ifdef USE_THINCLIENT
/* #include "Vector.h" */
/* #include "DataBuffer.h" */
#endif

#include <deque>

namespace isab{
#ifdef USE_THINCLIENT
   using namespace Packet_Enums;
#endif

   class Favorite;

   namespace UiCtrlInternal {
      struct GpsData;
   }

   typedef struct general_param {
      uint16   gui_param_id;
      uint16   param_type;
      enum ParameterEnums::ParamIds   nav2_param_id;
   } general_param_t;

   /**
    * Controls the gui. Used by UiCtrl.
    *
    */
   class GuiCtrl
   {
   public:

      /**
       * Constructor.
       *
       * @param uiCtrl Pointer to the top module object UiCtrl.
       *               Used for communicating with the rest of
       *               Nav2.
       *
       * @param navServComProvPubl Public provider for communicating
       *               with the server.
       * @param useIAPProxy If to use proxy setting of IAP.
       * @param hardIapProxyHost The hardcoded IAP proxy host, used if 
       *                         useIAPProxy and not NULL.
       * @param hardIapProxyPort The hardcoded IAP proxy port, see 
       *                         hardIapProxyHost.
       */
      GuiCtrl( class UiCtrl* uiCtrl,
               class NavServerComProviderPublic* navServComProvPubl,
               class NavTaskProviderPublic* navTaskProvPubl,
               class SerialProviderPublic* serialProvPubl,
               class DestinationsProviderPublic* destProvPubl,
               class ParameterProviderPublic* paramProvPubl,
               enum ParameterEnums::ParamIds serverListParam,
               bool useIAPProxy,
               const char* hardIapProxyHost,
               uint16 hardIapProxyPort,
               const char* userClipString);

      /**
       * Destructor.
       */
      virtual ~GuiCtrl();

   
      /**
       * Protocol version used for communication with the GUI.
       */
      enum {
         m_guiProtocolVersion = 8,
      };

      /**
       * Used when serializing GUI messages to a Buffer.
       *
       * Determines the size of data buffers needed for 
       * serializing messages sent to the GUI. 
       */
      enum {
         m_normalGuiPacketSize = 256,
      };

      /**
       * Used when serializing GUI messages to a DataBuffer.
       *
       * Determines the size of data buffers needed for 
       * serializing messages sent to the GUI. 
       */
      enum {
         m_maxGuiPacketSize = 1024 * 15,
      };

      enum FSIrequestType {
         FSIIndexRange = 0,
         FSISingleItemId = 1,
         FSISaveAsFav = 2,
      };

      static const char m_serverNamesToken;


      void setPosition(int32 lat, int32 lon, uint8 heading, int speed);

      /**
       * Takes the appropriate action for responding 
       * to a message from the GUI.
       *
       * @param guiMessage The messag to respond to. This message
       *                   has been created using the gui protocol
       *                   from Thin Client. (protocol version 0).
       */
#ifdef USE_THINCLIENT
      void messageReceived(const class NavCtrlMMIMessage* guiMessage);
#endif

      /**
       * Takes the appropriate action for responding 
       * to a message from the GUI.
       *
       * @param guiMessage  The messag to respond to.
       */
      void messageReceived(const class GuiProtMess* guiMessage);

      /**
       * Methods used for handling locally cached parameters.
       */
         /**
          * Set the number of search matches to be returned.
          */
         void setMaxNbrSearchMatches(int32 nbrOfSearchMatches);

         /**
          * Set top regionlist downloaded from server or read
          * from parameter file.
          */
         void setTopRegionList(class TopRegionList* topRegionList);

         /**
          * Set server address list.
          */
         void setServerHostAndPortStrings(const char* serverHostAndPortStrings);

         /**
          * Influences which units are used in distance calculations.
          */
         void setDistanceMode(enum DistancePrintingPolicy::DistanceMode distMode);

         /*
          * If true then check for new versions of app on startup
          */
         void setCheckForUpdates(uint8 aValue);

         /**
          * Send iap2 to GUI as integer.
          */
         void setSelectedAccessPointId2(int32 selectedAccessPointId2);          

      void receivedGeneralParam(uint32 paramId, const uint8 * data, 
                                int32 numEntries, uint16 reqId);
      void receivedGeneralParam(uint32 paramId, const int32 * data, 
                                int32 numEntries, uint16 reqId);
      void receivedGeneralParam(uint32 paramId, const float * data, 
                                int32 numEntries, uint16 reqId);
      void receivedGeneralParam(uint32 paramId, const char * const * data, 
                                int32 numEntries, uint16 reqId);
      void receivedGeneralParamUnset(uint32 paramId, uint16 reqId);

      //@}

      /**
       * Methods for handling messages to the GUI.
       */
      //@{

         void sendPendingMessagesToGui();

         /**
          * Called when a search reply is received.
          * 
          * @param nOverviwMatches The number of overview matches, may 
          *                        not be more than 255.
          * @param omVector The overview matches.
          * @param nMatches The number of search matches, may 
          *                 not be more than 255.
          * @param mVector  The search matches.
          */
         void handleSearchReply( uint32 nOverviewMatches, 
                                 const class OverviewMatch* const* omVector,
                                 uint32 nMatches, 
                                 const class Match* const* mVector);
         void handleSearchReply( uint32 nAreas, 
                                 const class SearchArea* const* sa,
                                 uint32 nItens, 
                                 const FullSearchItem* const* fsi
                                 ,uint16 begindex, uint16 total,
                                 GuiProtEnums::MessageType type, uint16 id
                                 );
      void handleAdditionalInfo(uint16 num, const char*const* id,
                                const unsigned* index, unsigned numInfo,
                                const class AdditionalInfo* const* info,
                                class RequestListItem* req);

      void handleLicenseReply(bool keyOk, bool phoneOk,
                              bool regionOk, bool nameOk, bool emailOk,
                              enum GuiProtEnums::WayfinderType type,
                              uint16 reqId);
      void handleCellConfirm(uint32 size, const uint8* data, uint16 reqId);

      void handleMultiVectorMapReply(uint32 size,
            const uint8* data,
            uint16 reqId);
      void handleForceFeedMultiVectorMapReply(
            uint32 size, const uint8* data );

      void handleTunnelDataReply(uint32 size, const uint8* data,
            uint32 ad_size, const uint8* ad_data, uint16 reqId);
      void handleTunnelDataRequest(const DataGuiMess* guiMessage);

      void handleNGPTunnelDataReply(uint32 ad_size, const uint8* ad_data,
            uint16 packet_id, uint16 reqId);
      void handleNGPTunnelDataRequest(const DataGuiMess* guiMessage);

      void handleLatestNews(const uint8 *data,
                            unsigned length,
                            uint32 checksum);
      /**
       * Send Satellite info to GUI.
       */
      void handleSatelliteInfo(class GpsSatInfoHolder *satInfo); 
      /**
       * Pass through file operation messages from NavTask to Gui.
       */
      void handleFileOperation(class GuiFileOperation* op);
      /**
       * Send route info messages on to GUI.
       */
      void handleRouteInfo(const class RouteInfo& r); 
      /**
       * Send route list to GUI.
       */
      void handleRouteList(RouteList &rl, uint16 reqId); 

      void handleRouteDownloaded(int64 routeid, int32 tLat, int32 lLon, 
                                 int32 bLat, int32 rLon, int32 oLat, 
                                 int32 oLon, int32 dLat, int32 dLon, 
                                 uint16 reqId);

      void handleUnsolicitedRouteDownloaded(int64 routeid,
                                            int32 tLat, int32 lLon, 
                                            int32 bLat, int32 rLon,
                                            int32 oLat, int32 oLon, 
                                            int32 dLat, int32 dLon);


         /**
          * 
          */
         void sendGpsData(struct UiCtrlInternal::GpsData *gpsData);

         
         /**
          * Favorite messages.
          */
         //@{
            void handleGetFavoritesReply(std::vector<class GuiFavorite*>& guiFavorites,
                                         uint16 reqId);
            void handleGetFavoritesAllDataReply(std::vector<class Favorite*>& favorites,
                                                uint16 reqId);
            
            /**
             * @param favorite   Favorite object keeping the information on the 
             *                   favorite asked for.
             * @param guiMsgType The type of GUI request that initiated the 
             *                   request for favorite info from the Destinations
             *                   module.
             * @param guiSeqId   The sequence id got from the gui for the request
             *                   resulting in this reply.
             */
            void handleGetFavoriteInfoReply(class Favorite* favorite, 
                    enum GuiProtEnums::MessageType guiMsgType,
                    uint16 guiSeqId);
            void handleFavoritesChanged();
         //@}

            /**
             * Notifies the GUI that a request failed.
             *
             * @param errorNbr The error causing the request to fail.
             * @param failedRequestSeqId The sequence id got from the 
             *                 GUI of the request that failed.
             *     
             *             NB! If this one is 0, it is assumed that the 
             *                 request used the old ThinClient GUI prot.
             *
             * @param failedRequestMessageNbr The message type in the 
             *                 GUI protocol. Eighter 
             *                 GuiProtEnums::MessageType or 
             *                 ThinClientGuiProtEnums::PacketNumber_t
             *                 depending on the value of failedRequestSeqId.
             */
            void handleRequestFailed( const ErrorObj& err,
                                      uint16 failedRequestSeqId,
                                      uint16 failedRequestMessageType );

         /**
          * Help methods for methods hadling messages to the GUI
          */
         //@{
            
            /**
             * Sends a message to the GUI.
             *
             * @param message The message to send to the GUI. 
             *                It is not deleted.
             */
#ifdef USE_THINCLIENT
            void sendMessageToGui(const class NavCtrlMMIMessage* message);
#endif
            /** Sends a message to the GUI. 
             * @param message a pointer to the message to send. The
             *                caller retains ownership of the message
             *                object.
             * @param requestId if the message is a reply to a message
             *                  received from the GUI, this parameter
             *                  should be the id of the request
             *                  message. If this is an unsolicited *
             *                  message, let requestId have it's
             *                  default value 0.
             * @return the id of the sent message. This may be the
             *         same as the requestId parameter.
             */
            uint16 sendMessageToGui(class GuiProtMess* message,
                  uint16 requestId);

            void sendParameterChangedToGui(const class GuiProtEnums::ParameterTypeHolder& paramType);

         //@}

      //@}

   protected:

      /**
       * Methods for handling messages from the GUI.
       */
      //@{
      // Older thinclient messages.
#ifdef USE_THINCLIENT
      void handleSearchRequest(const class SearchRequestMessage* searchMessage);
      void handleCreateRouteRequest(const class CreateRouteMessage* routeMessage);
      void startRoute(const class VanillaMatch* destination);

      /**
       * Saves the data of the destination in history.
       *
       * @param The destination to save history data for.
       */
      void saveHistory(const class VanillaMatch* destination);
      /**
       * Handles rotute request, which uses a destination stored in GuiCtrl.
       *
       * @param packetType Used for determining what destination type to
       *                   to use when requesting the route from the 
       *                   server.
       */
      void handleRouteRequest(const class RouteMessage* routeMessage,
                              NavCtrlGuiProt::PacketNumber_t packetType );
#endif
         /**
          * Handles search request.
          */
      void handleSearchRequest(const class SearchMess* searchMess);
      void handleGetfilteredRouteList(const class GetFilteredRouteListMess* mess);
         /**
          * Handles route request, which uses raw lat, lon as destination.
          */
      void handleRouteToPositionRequest(const class RouteToPositionMess* routeMess);
      void handleRoute(const class RouteMess* mess);
      void handleRouteToSearchItemRequest(const class GenericGuiMess* routeMess);
      void handleInvalidateRoute(const class GenericGuiMess* mess);
      void handleRouteToPositionCancel(const class GenericGuiMess* mess);
      void handleFileOpMessage(const class GuiProtFileMess* mess);
      void handleReroute(const class GenericGuiMess* mess);
      void handleRequestCrossingSound(const class GenericGuiMess* mess);
      void handleSearchAreas(const class GenericGuiMess* areaMess);
      void handleSearchItems(const class GenericGuiMess* itemMess);
      void handleFullSearchData(const class GenericGuiMess* fullItemReq);
      void handleFullSearchDataFromItemId(const class GenericGuiMess* fullItemReq);
      void handleMapRequest(const class GetMapMess* mapMess);
      void handleMessageRequest(const class SendMessageMess* guiMessage);
      void handleLicenseKey(const class LicenseKeyMess* guiMessage);
      void handleCellReport(const class DataGuiMess* guiMessage);
      void handleVectorMapRequest(const class GenericGuiMess* guiMessage);
      void handleMultiVectorMapRequest(const class DataGuiMess* guiMessage);
      void handleChangeUIN(const class GenericGuiMess* guiMessage);

      void checkAndSendRouteData(class RoutePositionData* rpd);
      uint32 favoriteIdFromString(char *str_id);
      int checkPositionData(class PositionObject *po, uint16 req_type, uint16 req_id);
      void checkAndSetRouteData(class RoutePositionData* rpd,
            const char *name, int32 lat, int32 lon);
      void resetFavoriteToPosition(class RoutePositionData* rpd);


      const general_param_t * generalGuiParamLookup(uint16 paramId);
      const general_param_t * generalNav2ParamLookup(uint32 paramId);

   public:
      void handleMessageSent(const uint8* data, unsigned length, uint16 reqId);
      void handleMapReply(const BoxBox& bb, uint16 imageWidth, 
                          uint16 imageHeight, uint32 realworldWidth,
                          uint32 realWorldHeight, enum ImageFormat format,
                          unsigned datasize, const uint8* data, uint16 reqId);

      /**
       * Constructs a new string for the SelectedAccessPointId2 parameter.
       *
       * @param selectedAccessPointId2 The SelectedAccessPointId2 value.
       * @return A new string, caller must delete.
       */
      char* makeSelectedAccessPointId2String( 
         int32 selectedAccessPointId2 ) const;

   protected:
         /**
          * Favorite messages.
          */
         //@{
           void handleGetTopRegionList( const class GenericGuiMess* guiMessage);
           void handleGetGeneralParameter( const class GeneralParameterMess* guiMessage);
           void handleSetGeneralParameter( const class GeneralParameterMess* guiMessage);
           void handleGetSimpleParameter( const class GenericGuiMess* guiMessage);
           void handleSetSimpleParameter( const class SimpleParameterMess* guiMessage);
           void handleGetFavorites( const class GenericGuiMess* guiMessage);
           void handleGetFavoritesAllData( const class GenericGuiMess* guiMessage);
           void handleSortFavorites( const class GenericGuiMess* guiMessage);
           void handleSyncFavorites( const class GenericGuiMess* guiMessage);
           void handleGetFavoriteInfo( const class GenericGuiMess* guiMessage);
           void handleAddFavorite( const class AddFavoriteMess* guiMessage);
           void handleAddFavoriteFromSearch( const class GenericGuiMess* guiMessage);
           void handleRemoveFavorite( const class GenericGuiMess* guiMessage);
           void handleChangeFavorite( const class ChangeFavoriteMess* guiMessage);
           void handleRouteToFavorite ( const class GenericGuiMess* guiMessage);
           void handleRouteToHotDest ( const class GenericGuiMess* guiMessage);
           void handleGpsConnection(const class GenericGuiMess* guiMessage);
           void handleParameterSync(const class GenericGuiMess* guiMessage);
         //@}

      void handleGetMoreSearchData(const class GetMoreDataMess* guiMessage);


         /**
          * Help methods to methods handling messages from GUI.
          */
         //@{
         
            /**
              * Requests a route using destination in the parameter.
              * @param The destination to request a route for.
              */
      void startRoute(class RoutePositionData *rpd, 
                      GuiProtEnums::MessageType messType,
                      uint16 messID);

      /**
       * Sends a request for full search info.
       * 
       * @param mess The gui message to send request for.
       * @param reqType The reason why.
       */
      void getInfoForAllSearchMatches( const GenericGuiMess* mess,
                                       enum FSIrequestType reqType );
         //@}
      /**
       * Get the coordinates for the search item and return
       * them in lat/lon.
       * @return > 0 on success
       */
      int getSearchPosition(char *searchId, int32& lat, int32& lon);
      //@}

      /// Copies an array of <code>FullSearchItems</code> into the
      /// local array of <code>FullSearchItems</code>. Handles
      /// updateing of all related variables
      /// (<code>m_hasAdditionalInfo</code>,
      /// <code>m_numberOfFirstSearchItem</code>,
      /// <code>m_totalnumberOfSearchItems</code>) and deleting of old
      /// <code>FullSearchItems</code>.
      /// @param nItems   number of received <code>FullSearchItems</code>
      /// @param fsi      a pointer to a vector of <code>nItems</code> 
      ///                 pointers to FullSearchItems to be copied.
      /// @param begindex the index of the first 
      ///                 <code>FullSearchItems</code> in the total 
      ///                 search result vector.
      /// @param total    the total number of search matches in the 
      ///                 total search result vector.
      void copyItems(uint32 nItems, const FullSearchItem* const* fsi,
                     uint16 begindex, uint16 total);
      /// Sends the <code>SearchItem</code> part of the
      /// <code>FullSearchItems</code> with indexes in the range
      /// [begindex, endex[ to the Gui in a
      /// <code>SearchItemsReplyMess</code> message.
      /// Note that the indexes are in the total search result vector.
      /// @param begindex the index of the first <code>SearchItem</code> 
      ///                 to send. 
      /// @param endex    the index + 1 of the last 
      ///                 <code>SearchItem</code> to send to the GUI.
      /// @param reqId    the request ID of the search request. If 
      ///                 there is no GUI search request connected with 
      ///                 this message, set to 0.
      void sendItemsToGui(uint32 begindex, uint32 endex, uint16 reqId);


      /**
       * Get the IAP proxy for the current iap.
       *
       * @param host Set to a new string with the proxy host name.
       * @param port Set to the proxy port number.
       * @return True if proxy set false if not.
       */
      bool getIAPProxy( char*& host, uint32& port, uint32 iap ) const;

   private:

      /**
       * Pointer to the top module object UiCtrl.
       * Used for communicating with the rest of
       * Nav2.
       *
       * Not owned by this object, so it should not be
       * deleted here.
       */
      class UiCtrl* m_uiCtrl;

      /**
       * Log object used for debug prints. Not necessarily 
       * used only by this object.
       *
       * Not owned by this object, so it should not be
       * deleted here.
       */
      Log* m_log;

      /**
       * Public provider for communicating with the server.
       *
       * Should not be deleted in this class.
       */
      NavServerComProviderPublic* m_nscProvPubl;


      /**
       * Public provider for communicating with NavTask.
       *
       * Should not be deleted in this class.
       */
      NavTaskProviderPublic* m_ntProvPubl;

      /**
       * Connects to the GUI.
       *
       * Should not be deleted in this class.
       */
      SerialProviderPublic* m_serialProvPubl;
      

      /**
       * Connects to the Destinations module.
       *
       * Should not be deleted in this class.
       */
      DestinationsProviderPublic* m_destProvPubl;
      
      /**
       * Connects to the Parameter module.
       *
       * Should not be deleted in this class.
       */
      ParameterProviderPublic* m_paramProvider;

      /**
       * Increased for each message sent to the GUI.
       */
/*       uint16 m_lastGuiMessReceivedId; //MN: Implement! */

      /**
       * The GUI message id to use when sending the 
       * next message to the GUI.
       */
      uint16 m_nextGuiMessToSendId; 

      /**
       * Queue used for storing messages to send to the 
       * GUI while the GUI still has not connected. These
       * messages are sent when the GUI connects.
       */
      std::deque< Buffer* >*       m_messagesToSendToGui;

/*       FullSearchItem* m_currDestItem; */
      class RoutePositionData* m_currRouteData;

      struct FSIrequest : public RequestData{
         FSIrequest(class GenericGuiMess* m, enum FSIrequestType type)
            : message(m), type(type) {}
         class GenericGuiMess* message;
         enum FSIrequestType type;
      };
      typedef std::vector<class SearchArea*>     SearchAreaContainer;
      typedef std::vector<class FullSearchItem*> SearchItemContainer;
      SearchAreaContainer m_searchAreas;
      SearchItemContainer m_searchMatches;
      bool m_hasAdditionalInfo;
      uint16 m_numberOfFirstSearchItem;
      uint16 m_totalnumberOfSearchItems;


      /// The type of Serverlist used. Nav or Http.
      enum ParameterEnums::ParamIds m_serverListParam;

      /**
       * If to use proxy setting of IAP.
       */
      bool m_useIAPProxy;


      /**
       * The hardcoded IAP proxy host, if any.
       */
      const char* m_hardIapProxyHost;


      /**
       * The hardcoded IAP proxy port, see also m_hardIapProxyHost.
       */
      uint16 m_hardIapProxyPort;


      /**
       * Cached parameters used by the GUI.
       */
      //@{

         /**
          * Max number of search matches returned from server
          * when searching.
          */
         uint8 m_paramMaxNbrSearchMatches;

         /**
          * Top region list used when searching.
          */
         TopRegionList* m_topRegionList;

         /**
          * Influences how distances are displayed.
          */
         DistancePrintingPolicy::DistanceMode m_distanceMode;

         char* m_webUsername;

         uint8 m_checkForUpdates;
      //@}
      
      //Cache last pos data here.
      int32 m_lastLat;
      int32 m_lastLon; 
      uint8 m_lastHeading;
      int m_lastSpeed;
      const char* m_webUsernameClipString;

      ///This isn't pretty...

      /// This class is used to remeber Search requests if we ever
      /// need to use them again.
      class RefinedSearchRequest{
      public:
         typedef NavServerComProviderPublic NSCPP;
         ///This pointer type can point to either originalSearch or
         ///refinedSearch, since they by coincidence has the same
         ///arguments. How about that!
// typedef uint32(NSCPP::*request)(const char *char1, const char *char2, 
//                                 uint16 startIndex, uint32 countryCode,
//                                 int32 lat, int32 lon, uint8 heading, 
//                                 uint32 dst = MsgBuffer::ADDR_DEFAULT);
         ///Constructor
         ///@param provider a reference to the pointer to the 
         ///                NavServerComProviderPublic to use.
         RefinedSearchRequest(const NSCPP* provider);
         ///Destructor. Deletes strings it's holding.
         ~RefinedSearchRequest();
         ///Performs a new search request with the same data as last. 
         /// @param startIndex the start index for the search data.
         /// @return the request id.
         uint32 request(uint16 startIndex) const;
         ///Performs a search request with new data and startindex 0.
         /// @param destination the Destination string. 
         ///                    Is duplicated for storage.
         /// @param areaId      the area id string. Is duplicated for storage.
         /// @param region      the region id. Is stored.
         /// @param lat         the lat to search nearby. Is stored.
         /// @param lon         the lon to search nearby. Is stored.
         /// @param heading     the heading to search in. Is stored.
         /// @return the request id.
         uint32 request(const char* destination, const char* areaId,
                        uint32 region, int32 lat, int32 lon, 
                        uint8 heading = 0);
         uint32 firstSearch(const char* destination, const char* areaString,
                            uint32 region, int32 lat, int32 lon,
                            uint8 heading = 0);
         void setData(const char* destination,
                      const char* areaId,
                      uint32 region,
                      int32 lat, int32 lon,
                      uint8 heading = 0);
         void setAreaId(const char* id);
      private:
         ///Used for nav2 requests.
         const NSCPP* m_provider;
         ///Destination string
         char* m_destination;
         /// area id
         char* m_areaId;
         /// area search string
         char* m_area;
         /// region id
         uint32 m_countryCode;
         ///latitude
         int32 m_lat;
         ///longitude
         int32 m_lon;
         ///heading
         uint8 m_hdg;
         RefinedSearchRequest(const RefinedSearchRequest& from);
         const RefinedSearchRequest& operator=(const RefinedSearchRequest& r);
      };

      class RefinedSearchRequest m_refinedSearch;
   }; // GuiCtrl

   inline void GuiCtrl::setPosition(int32 lat, int32 lon, 
                                    uint8 heading, int speed)
   {
      m_lastLat = lat;
      m_lastLon = lon; 
      m_lastHeading = heading;
      m_lastSpeed = speed;
   }

} //namespace isab




#endif // GUI_CONTROL_H
