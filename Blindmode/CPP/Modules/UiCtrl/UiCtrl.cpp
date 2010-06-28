/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MsgBufferEnums.h"
#include "Module.h"
#include "RouteEnums.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "NavTask.h"

#include "GuiProt/ServerEnums.h"
#include "NavPacket.h"
#include "CtrlHub.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "NavServerCom.h"

#include "Serial.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "DestinationsEnums.h"
#include "Destinations.h"
#include "ErrorModule.h"
#include "CtrlHub.h"
#include "UiCtrl.h"

#include "RequestList.h"
#include "UiCtrlInternal.h"

#include "TopRegion.h"
#include "Nav2Error.h"
#include "ErrorModule.h"
#include "RouteInfo.h"

#include "nav2util.h"

/* #include "Packet.h" */

#ifdef USE_THINCLIENT
/* #include "ThinClient/include/NavCtrlGuiProt.h" */
#endif

#include "GuiProt/GuiProtMess.h"
/* #include "GuiProt/GuiProtRouteMess.h" */
/* #include "GuiProt/GuiProtSearchMess.h" */
/* #include "GuiProt/GuiProtMapMess.h" */
/* #include "GuiProt/GuiProtFavoriteMess.h" */
#include "GuiProt/MapClasses.h"
#include "GuiProt/GuiParameterEnums.h"

#include "DistancePrintingPolicy.h"
#include "GuiCtrl.h"

#include "AudioCtrlLanguage.h"
#include "AudioCtrl.h"
// Logging macros
#include "LogMacros.h"
#include "TraceMacros.h"

#define DEFAULT_NUMBER_OF_SEARCH_MATCHES  25

using namespace std;

#ifdef USE_THINCLIENT
using namespace Packet_Enums;
#endif

namespace isab{
   typedef pair<GuiProtMess*, uint32> pairdef;

   UiCtrl::UiCtrl(SerialProviderPublic* spp,
                  Nav2Error::Nav2ErrorTable *errorTable,
                  AudioCtrlLanguage *audioSyntax,
                  const char* serialNumber,
                  bool httpProto,
                  bool useIAPProxy,
                  const char* hardIapProxyHost,
                  uint16 hardIapProxyPort,
                  const char* userClipString) : 
      Module( "UiCtrl" ), 
      m_provider( spp ), 
      m_navSeqID( 0 ), 
      m_pdaSeqID( 0 ), 
      m_guiPositionUpdateTimeOut( DS_DEFAULT_TIMEOUT ), 
      m_gotPda( false ), 
      m_subscribing( false ), 
      m_noPda( LOST_PDA_MAX_TIMEOUTS ), 
      m_dataPhoneNo(NULL), 
      m_voicePhoneNo(NULL),
      m_userID(NULL), 
      m_passwd(NULL),
      m_serverHostname(NULL), 
      m_shutdown( false ),
      m_connectedToGui( false ),
      m_pending( NULL ),
      m_lastGpsData( NULL ), 
      m_lastRouteList(NULL),
      m_previousStatus( NavServerComEnums::invalid ),
      m_previousMessage( NavServerComEnums::NAV_SERVER_INVALID ),
      m_errorTable( errorTable )
   {
      INFO( "UiCtrl(%p)", spp );

      m_pending = new RequestList( MAX_PENDING );
      m_lastGpsData = new UiCtrlInternal::GpsData();
      
      if ( serialNumber ) {
         m_serialNumber = strdup_new(serialNumber);
      } else {
         m_serialNumber = strdup_new("default");
      }
      m_waitingMessages = new deque<pair<GuiProtMess*, uint32>* >();
#ifdef USE_THINCLIENT
      m_waitingNavCtrlMessages = new deque<pair<NavCtrlMMIMessage*, uint32>* >;
#endif

      spp->setOwnerModule( this );
      SerialConsumerPublic* scp = new SerialConsumerPublic( m_queue );
      spp->connect( scp );
      newLowerModule( spp );

      m_serverProvider  = new NavServerComProviderPublic( this );
      m_navTaskProvider = new NavTaskProviderPublic( this );
      m_paramProvider   = new ParameterProviderPublic( this );
      m_destProvider    = new DestinationsProviderPublic( this );  

      if ( httpProto ) {
         m_serverListParam = 
            ParameterEnums::NSC_HttpServerHostname;
      } else {
         m_serverListParam = ParameterEnums::NSC_ServerHostname;
      }

      m_guiCtrl = new GuiCtrl(this, 
                              m_serverProvider, 
                              m_navTaskProvider,
                              m_provider, 
                              m_destProvider, 
                              m_paramProvider,
                              m_serverListParam,
                              useIAPProxy,
                              hardIapProxyHost,
                              hardIapProxyPort,
                              userClipString);
      m_audioCtrl = new AudioCtrl(this, 
                              m_provider, 
                              m_paramProvider,
                              audioSyntax);

      m_parseBuffer = new Buffer(32); //totally arbitrary length.
 
      // Variables used for debug prints.
      m_printedPos = false;
      m_uiCtrlParams = NULL;
      m_uiCtrlParamsSize = 0;
      
      m_allParamsReceived = false;
   }

   UiCtrl::~UiCtrl()
   {
      if (m_lastRouteList) {
         delete m_lastRouteList;
      }
      delete[] m_serialNumber;
      delete[] m_dataPhoneNo;
      delete[] m_voicePhoneNo;
      delete[] m_userID;
      delete[] m_passwd;
      delete[] m_serverHostname;
      delete m_serverProvider; 
      delete m_paramProvider;
      delete m_guiCtrl;
      delete m_parseBuffer;
      delete m_destProvider;
      delete[] m_uiCtrlParams;
      delete m_waitingMessages;
#ifdef USE_THINCLIENT
      delete m_waitingNavCtrlMessages;
#endif
      delete m_audioCtrl;
      delete m_pending;
      delete m_lastGpsData;
   }
   
   MsgBuffer* UiCtrl::dispatch(MsgBuffer* buf)
   {
      if(buf) buf = m_serialDecoder.dispatch ( buf, this );
      if(buf) buf = m_nscDecoder.dispatch    ( buf, this );
      if(buf) buf = m_hubDecoder.dispatch    ( buf, this );
      if(buf) buf = m_navTaskDecoder.dispatch( buf, this );
      if(buf) buf = m_paramDecoder.dispatch  ( buf, this );
      if(buf) buf = m_destDecoder.dispatch   ( buf, this );
      if(buf) buf = m_errorDecoder.dispatch  ( buf, this );
      if(buf) buf = Module::dispatch         ( buf );
      return buf;
   }

   void UiCtrl::decodedShutdownNow( int16 upperTimeout )
   {
      m_shutdown = true;
      m_queue->removeTimer( m_guiPositionUpdateTimerId );
      Module::decodedShutdownNow( upperTimeout );
   }

   void UiCtrl::treeIsShutdown()
   {
      delete m_navTaskProvider;
      Module::treeIsShutdown();
   }

   void UiCtrl::decodedConnectionNotify(enum ConnectionNotify state, 
                                        enum ConnectionNotifyReason reason,
                                        uint32 src)
   {
      src = src; //currently unused.
      reason = reason; //currently unused.
      switch(state){
      case CONNECTING:
         INFO("CONNECTING to GUI");
         break;
      case CONNECTED:
         INFO("CONNECTED to GUI");
         m_connectedToGui = true;
         
         // No position has been sent while disconnected.
         m_guiCtrl->sendGpsData(m_lastGpsData);
         
         // Make sure all messages that should have been sent are sent.
         m_guiCtrl->sendPendingMessagesToGui();

         // Start the process of sending position updates regularly.
         m_queue->setTimer(m_guiPositionUpdateTimerId, 
                              m_guiPositionUpdateTimeOut);

         INFO("CONNECTED to GUI");
         break;
      case DISCONNECTING:
      case CLEAR:
         // Sending dicsonnected events even when reporting CLEAR is
         // necessary for the timeout mechanism.
         m_connectedToGui = false;
         INFO("DISCONNECTING from GUI");
         break;
      case WAITING_FOR_USER:
         ERR("WAITING_FOR_USER on GUI connection");
         break;
      }
   }
   
   void UiCtrl::decodedExpiredTimer(uint16 timerid)
   {
      DBG("decodedExpiredTimer(%"PRIu16")", timerid);
      if( timerid == m_guiPositionUpdateTimerId ) {
         if ( !m_shutdown && connectedToGui() ) {

            // It is time to send the GUI a position update.
            m_guiCtrl->sendGpsData(m_lastGpsData);
            m_queue->setTimer(m_guiPositionUpdateTimerId, 
                              m_guiPositionUpdateTimeOut);

            //MN: Debug print.
            DBG("Nbr of pending requests. outstanding:%u, remaining:%u.",
                m_pending->outstanding(), m_pending->remaining() );

            // Check if any message not dealt with has timed out.
            checkWaitingMessagesTimeOut();
         }
         DBG("end of expiredtimer");
         return;
      }
      DBG("Unknown timer will be handled by Module....");
      Module::decodedExpiredTimer(timerid);
   }


   /**
    * Determines what parameters the UiCtrl
    * module asks from the Parameter module.
    *
    * All parameters marked false must have arrived from the
    * Parameters module before uictrl will answer any GUI requests.
    *
    * Never make this vector have a length
    * less than 2.
    */
   const UiCtrlInternal::UiCtrlParams uiCtrlParams[] = 
   {  
      { ParameterEnums::UC_GPSAutoConnect,               true },
      { ParameterEnums::NSC_UserAndPasswd,               true },
      { ParameterEnums::UC_VectorMapSettings,            true },
      { ParameterEnums::UC_VectorMapCoordinates,         true },
      { ParameterEnums::SelectedAccessPointIdReal,       false },
/*       { ParameterEnums::SelectedAccessPointId,           false }, */
      { ParameterEnums::WayfinderType,                   false },
#if defined ( NAV2_USE_UTF8 )
      { ParameterEnums::TopRegionList,                   false  }, 
#else
      { ParameterEnums::TopRegionListLegacy,             false  }, 
#endif
//      { ParameterEnums::NSC_RequestedRouteTime,          true },
      { ParameterEnums::UC_GuiMode,                      true },
      { ParameterEnums::UC_MaxNumberOfGuiSearchResults,  true },
      { ParameterEnums::UC_SoundVolume,                  true },
      { ParameterEnums::UC_UseMainSpeaker,               true },
      { ParameterEnums::UC_AutoReroute,                  true },
      { ParameterEnums::UC_StoreSMSDestInMyDest,         true },
      { ParameterEnums::UC_AutomaticRouteOnSMSDest,      true },
      { ParameterEnums::UC_KeepSMSDestInInbox,           true },
      { ParameterEnums::UC_BacklightStrategy,            true },
      { ParameterEnums::NSC_ServerHostname,              true },
      { ParameterEnums::NSC_TransportationType,          true },
      { ParameterEnums::UC_DistanceMode,                 true },
      { ParameterEnums::NSC_RouteCostType,               true  },
      { ParameterEnums::NSC_RouteTollRoads,              true  },
      { ParameterEnums::NSC_RouteHighways,               true  },
      { ParameterEnums::UC_WebUser,                      false },
      { ParameterEnums::SelectedAccessPointId2,          true },
      { ParameterEnums::UC_WebPasswd,                    true },
      { ParameterEnums::UC_MuteTurnSounds,               true },
      { ParameterEnums::Language,                        true  },
      { ParameterEnums::UC_GUISearchStrings,             true },
      { ParameterEnums::UC_GUISearchCountryBlob,         true },
      { ParameterEnums::NSC_ExpireVector,                true },
      { ParameterEnums::CategoryNames,                   true },
      { ParameterEnums::CategoryIds,                     true },
      { ParameterEnums::UC_GUIPositionSelectDataBlob,    true },
      { ParameterEnums::UC_GUIMyDestCurrentSelectedIndex,true },
      { ParameterEnums::UC_GUILastKnownRouteEndPoints,   true },
      { ParameterEnums::UC_GUILastKnownRouteId,          true },
      { ParameterEnums::UC_FavoriteShow,                 true },
      { ParameterEnums::UC_PoiCategories,                true },
      { ParameterEnums::UC_AutoTracking,                 true },
      { ParameterEnums::TR_trackLevel,                   true },
      { ParameterEnums::TR_trackPIN,                     true },
      { ParameterEnums::UC_MapLayerSettings,             true },
      { ParameterEnums::NSC_userRights,                  true },
      { ParameterEnums::UC_LinkLayerKeepAlive,           true },
      { ParameterEnums::UC_UserTermsAccepted,            true },
      { ParameterEnums::UC_LatestShownNewsChecksum,      true },
      { ParameterEnums::NSC_LatestNewsChecksum,          true },
      { ParameterEnums::UC_TurnSoundsLevel,              true },
      { ParameterEnums::NT_UserTrafficUpdatePeriod,      true },
      { ParameterEnums::UC_DirectionType,                true },
      { ParameterEnums::UC_FeedOutputFormat,             true },
      { ParameterEnums::UC_FeedWaitPeriod,               true },
      { ParameterEnums::UC_LockedNavWaitPeriod,          true },
      { ParameterEnums::UC_GpsMinWalkingSpeed,           true },
      { ParameterEnums::UC_VicinityListUpdatePeriod,     true }
   };
   

   void UiCtrl::decodedStartupComplete()
   {
      Module::decodedStartupComplete();
      

      m_guiPositionUpdateTimerId = 
         m_queue->defineTimer( m_guiPositionUpdateTimeOut );


      m_rawRootPublic->manageMulticast(JOIN, Module::NavTaskMessages);
      m_rawRootPublic->manageMulticast(JOIN, Module::NSCProgressMessages);
      m_rawRootPublic->manageMulticast(JOIN, Module::DestMessages);
      m_rawRootPublic->manageMulticast(JOIN, Module::ErrorMessages);
      m_rawRootPublic->manageMulticast(JOIN, Module::LatestNews);

      // Copy the UiCtrl parameter vector to a member.      
      
      // Difference in address between first and second element.
      uint32 elementSize = (uint8*)&uiCtrlParams[1] - 
                           (uint8*)&uiCtrlParams[0];
      m_uiCtrlParamsSize = 
         sizeof(uiCtrlParams) / elementSize;
      m_uiCtrlParams = 
         new UiCtrlInternal::UiCtrlParams[m_uiCtrlParamsSize];
      for (uint32 i = 0; i<m_uiCtrlParamsSize; i++){
         m_uiCtrlParams[i] = uiCtrlParams[i];
      }

      // maybe we want to join all parameters, just in case? 
      uint16 mcGroup = 0;
      for(unsigned a = 0; a < m_uiCtrlParamsSize; ++a){
         if ( m_serverListParam == ParameterEnums::NSC_HttpServerHostname ) 
         {
            if ( m_uiCtrlParams[a].paramId == ParameterEnums::NSC_ServerHostname ) {
               // Use http servers
               m_uiCtrlParams[a].paramId = ParameterEnums::NSC_HttpServerHostname;
            }
         }
         if(ParameterProviderPublic::paramIdIsMulticast(m_uiCtrlParams[a].paramId)){
            uint16 g = ParameterProviderPublic::paramIdToMulticastGroup(m_uiCtrlParams[a].paramId);
            if(g != mcGroup){
               mcGroup = g;
               m_rawRootPublic->manageMulticast(JOIN, mcGroup);
           }
         }
         m_paramProvider->getParam(m_uiCtrlParams[a].paramId);
      }


     /* Test Destinations module.
	  //m_destProvider->getFavorites();
     m_destProvider->getFavorites();
     Favorite* fav = new Favorite( 3, 4, "Edmonds field", 
                   "newFavShortName", "newFavDescription", "newFavCategory", "newFavMapIconName");
      
	  m_destProvider->addFavorite(fav);
	  m_destProvider->addFavorite(fav);
	  m_destProvider->addFavorite(fav);
	  m_destProvider->addFavorite(fav);

	  m_destProvider->getFavorites();
	  m_destProvider->removeFavorite( uint32(MAX_INT32)+1);
     delete fav;
     //m_destProvider->getFavoritesAllData();
	  */
   }


   void UiCtrl::decodedUnsolicitedError( const ErrorObj& err,
                                         uint32 src )
   {
      if ( err.getErr() == Nav2Error::PANIC_ABORT ){
         WARN("Panic abort received by UiCtrl.");
         GenericGuiMess* ggm = new GenericGuiMess(GuiProtEnums::PANIC_ABORT);
         m_guiCtrl->sendMessageToGui(ggm, 0);
         delete ggm;
      } else if ( err.getErr() == Nav2Error::NO_ERRORS){
         WARN("UiCtrl received unsolicited error NO_ERRORS. Not sending this to GUI.");
      } else {
         DBG("Unsolicited error errorNbr: 0x%08x, "
             "source module address: 0x%08"PRIx32, err.getErr(), src);
         const char* errorStr = m_errorTable->getErrorString( 
            err.getErr() );
         if ( err.getErrorMessage()[ 0 ] != '\0' ) {
            errorStr = err.getErrorMessage();
         }
         if ( errorStr != NULL ) {
            ErrorMess* errMsg = 
               new ErrorMess( err.getErr(), errorStr );
            m_guiCtrl->sendMessageToGui(errMsg,0);
            DBG("Unsolicited error sent to GUI. errorNbr=0x%08x, string=%s",
                 err.getErr(), errorStr );
            errMsg->deleteMembers();
            delete errMsg;
         }
      }
   }

   void UiCtrl::decodedSolicitedError( const ErrorObj& err, 
                                       uint32 src, uint32 dst )
   {
      // Dummy errors should not be present in the final
      // product.
      if ( err.getErr() == Nav2Error::DUMMY_ERROR ) {
         WARN("Dummy error received. This one should have been "
                 "replaced by a real error number.");
         DBG("Not sending dummy error to the GUI.");
         return;
      }

      DBG( "Solicited error. errorNbr: 0x%08x", err.getErr() );
      RequestListItem* req = m_pending->dequeue( dst );
      if (req != NULL){
         // Associate the error with a request.
         uint16 guiSeqId = req->reqID;
         uint16 requestMessageType = req->request;
         m_guiCtrl->handleRequestFailed( err,
                                         guiSeqId,
                                         requestMessageType );
      } else {
         // This error cannot be associated with a request 
         // from the GUI.
      }




      // Different source modules
      const char* moduleName = "Unknown module(because of nonfix address "
                               "or missing entry in switch in UiCtrl::"
                               "decodedSolicitedError).";
      uint32 moduleId = ( (src >> 24) & 0xff );
      switch (moduleId){
      case Module::CtrlHubModuleId:
         {
            moduleName = "CtrlHubModule";
         } break;
      case Module::NavTaskModuleId:
         {
            moduleName = "NavTaskModule";
         } break;
      case Module::ParameterModuleId:
         {
            moduleName = "ParameterModule";
         } break;
      case Module::NavServerComModuleId:
         {
            moduleName = "NavServerComModule";
         } break;
      case Module::DisplaySerialModuleId:
         {
            moduleName = "DisplaySerialModule";
         } break;
      case Module::BtManagerModuleId:
         {
            moduleName = "BtManagerModule";
         } break;
      case Module::DestinationsModuleId:
         {
            moduleName = "DestinationsModule";
         } break;
      }// switch

      INFO( "Got error from %s "
            "with Nav2Error::ErrorNbr 0x%08"PRIx32".", 
            moduleName, (int32)err.getErr() );

      if (req != NULL){
         m_pending->deallocate(req);
      }
   }


   void UiCtrl::decodedSyncDestinationsReply(std::vector<Favorite*>& /*favs*/, 
                                             std::vector<uint32>& /*removed*/,
                                             uint32 /*src*/, uint32 /*dst*/)
   {
      // This is where the result of a destination sync comes in.
      // It is not taken care of here, but in the destinations module.
   }


   void UiCtrl::decodedBinaryUploadReply( uint32 /*src*/, uint32 dst )
   {
      RequestListItem* req = m_pending->dequeue(dst);

      // Nothing important to do here.
      
      m_pending->deallocate(req);
   }

   void UiCtrl::decodedBinaryDownload( const uint8* data, 
                                              size_t length, 
                                              uint32 src, uint32 dst )
   {
      DBG("decodedBinaryDownload(%p, %u, %0#"PRIx32", %0#"PRIx32")", data, length, src, dst);
      DBG("Not sending BLOB to GUI.");
/* Not sending BLOB to GUI.
      //DBGDUMP("decodedBinaryDownload", data, length);
      Packet* binaryBlob = new Packet(DOWNLOAD_DATA, m_navSeqID++, true);
      binaryBlob->writeNextUnaligned16bit(0);
      binaryBlob->writeNextByteArray(data, length);
      binaryBlob->closeMessage();
      m_provider->sendData(binaryBlob);
      */
   }

   void UiCtrl::decodedWhereAmIReply( const char* /*country*/, 
                                      const char* municipal, 
                                      const char* city, 
                                      const char* district, 
                                      const char* streetname, 
                                      int32 /*lat*/, int32 /*lon*/, 
                                      uint32 /*src*/, uint32 dst )
   {
      RequestListItem* req = m_pending->dequeue(dst);
      if(req){
         DBG("Where am I reply.");
		   DBG("Not sending where am I reply to GUI");

         const char* emptyStr = "";
         if (!municipal){
            municipal = emptyStr;
         }
         if (!city){
            city = emptyStr;
         }
         if (!municipal){
            municipal = emptyStr;
         }
         if (!district){
            district = emptyStr;
         }
         if (!streetname){
            streetname = emptyStr;
         }
         DBG("WHERE_AM_I_REPLY. Municipal: %s.\n"
             "                  City;      %s.\n"
             "                  District:  %s.\n"
             "                  Street:    %s.\n",
             municipal,
             city,
             district,
             streetname);
         
         m_pending->deallocate(req);
      } 
      else {
         WARN("Never sent WhereAmI request with id 0x%0"PRIx32". Discarding.", 
              dst );
      }
   }


   void UiCtrl::decodedSearchReply(unsigned nAreas, 
                                   const SearchArea* const* sa,
                                   unsigned nItems, 
                                   const FullSearchItem*const* fsi,
                                   uint16 begindex, uint16 total,
                                   uint32 src, uint32 dst)
   {
      RequestListItem* req = m_pending->dequeue(dst);
      DBG("decodedSearchReply: %u areas, %u matches, src: 0x%08"PRIx32", "
          "dst: 0x%08"PRIx32")", nAreas, nItems, src, dst);
      if(req != NULL){
         m_guiCtrl->handleSearchReply(nAreas, sa, nItems, fsi
                                      ,begindex, total, 
                                      GuiProtEnums::MessageType(req->request), 
                                      req->reqID
                                      );
         m_pending->deallocate(req);
      } else {
         WARN("Never sent search request with id 0x%0"PRIx32". Discarding.", 
              dst);
      }
   }

   void UiCtrl::decodedSearchInfoReply(uint16 num, const char* const* id,
                                       const unsigned* index,
                                       unsigned numInfo,
                                       const AdditionalInfo*const* info,
                                       uint32 /*src*/, uint32 dst)
   {
      RequestListItem* req = m_pending->dequeue(dst);
      DBG("decodedSearchInfoReply: %u AdditionalInfo", num);
      if(req != NULL){
         //XXX turn off spinning LOGO.
         decodedProgressMessage(NavServerComEnums::done, 
                                GuiProtEnums::InvalidActionType, 0, 0, 0, 0);
         //XXX this way of doing it is not pretty.
         m_guiCtrl->handleAdditionalInfo(num, id, index, numInfo, info, req);
         m_pending->deallocate(req);
      } else {
         WARN("Never sent searchInfo request with id %#"PRIx32". Discarding", 
              dst);
      }
   }

   void UiCtrl::decodedLicenseReply(bool keyOk, bool phoneOk, bool regionOk,
                                    bool nameOk, bool emailOk, 
                                    GuiProtEnums::WayfinderType type, 
                                    uint32 /*src*/, uint32 dst)
   {
      RequestListItem* req = m_pending->dequeue(dst);
      DBG("decodedLicenseReply: %ckeyOk, %cphoneOk, %cregionOk, "
          "%cnameOk, %cemailOk", keyOk?' ':'!', phoneOk?' ':'!', 
          regionOk?' ':'!', nameOk?' ':'!', emailOk?' ':'!');
      if(req != NULL){
         m_guiCtrl->handleLicenseReply(keyOk, phoneOk, regionOk, nameOk, 
                                       emailOk, type, req->reqID);
         m_pending->deallocate(req);
      } else {
         WARN("Never sent license key with reqid %#"PRIx32". Discarding", dst);
      }
   }

   void UiCtrl::decodedMulitVectorMapReply(uint32 size,
                                           const uint8* data, uint32 dst)
   {
      RequestListItem* req = m_pending->dequeue(dst);
      DBG("decodedMultiVectorMapReply(%"PRIu32", %p, %0#10"PRIx32")", 
          size, data, dst);
      if(req != NULL){
         DBG("Matches request with id %u", req->reqID);
         m_guiCtrl->handleMultiVectorMapReply(size, data, req->reqID);
         m_pending->deallocate(req);
      } else {
         WARN("The cat peed on my maps with request id %#"PRIx32"!", dst);
      }
   }


   void 
   UiCtrl::decodedForceFeedMuliVectorMapReply(
      uint32 size, const uint8* data, uint32 /*dst*/ )
   {
      DBG("decodedForceFeedMultiVectorMapReply( %"PRIu32", %p )", size, data);
      m_guiCtrl->handleForceFeedMultiVectorMapReply( size, data );
   }


   void UiCtrl::decodedCellConfirm(uint32 size, const uint8* data, uint32 dst)
   {
      RequestListItem* req = m_pending->dequeue(dst);
      DBG("decodedCellConfirm(%"PRIu32", %p, %0#10"PRIx32")", size, data, dst);
      if(req != NULL){
         m_guiCtrl->handleCellConfirm(size, data, req->reqID);
         m_pending->deallocate(req);
      } else {
         WARN("The dog ate my cell report with request id %#"PRIx32"!", dst);
      }
   }      
      
   void UiCtrl::decodedMapReply(const BoxBox& bb, uint32 realWorldWidth,
                                uint32 realWorldHeight, uint16 imageWidth,
                                uint16 imageHeight,
                                enum ImageFormat imageType, 
                                uint32 imageBufferSize, 
                                const uint8* imageBuffer, uint32 dst)
   {
      DBG("decodedMapReply([%"PRId32", %"PRId32"], [%"PRId32", %"PRId32"], "
          "%u, %"PRIu32", %p, %0#10"PRIx32")",
          bb.getTopLat(), bb.getLeftLon(), bb.getBottomLat(), 
          bb.getRightLon(), imageType, imageBufferSize, imageBuffer, dst);
      RequestListItem* req = m_pending->dequeue(dst);
      if(req){
         m_guiCtrl->handleMapReply(bb, imageWidth, imageHeight, 
                                   realWorldWidth, realWorldHeight, 
                                   imageType, 
                                   imageBufferSize, imageBuffer, req->reqID);
         m_pending->deallocate(req);
      } else {
         ERR("Never sent a map request with id %0#10"PRIx32, dst);
      }
   }

   void UiCtrl::decodedMessageReply(const uint8* data, unsigned length,
                                    uint32 src, uint32 dst)
   {
      DBG("decodedMessageReply(%p, %u, %#"PRIx32", %#"PRIx32")",
          data, length, src, dst);
      DBGDUMP("decodedMessageReply", data, length);
      RequestListItem* req = m_pending->dequeue(dst);
      if(req){
         m_guiCtrl->handleMessageSent(data, length, req->reqID);
         m_pending->deallocate(req);
      } else {
         ERR("Never sent message request widt id %#"PRIx32".", dst);
      }
   }

   void UiCtrl::decodedNTRouteReply(int64 routeid, uint32 src, uint32 dst )
   {
      RequestListItem* req = m_pending->dequeue( dst );
      
      DBG("decodedNTRouteReply(0x%08"PRIx32"%08"PRIx32", 0x%08"PRIx32", "
          "0x%08"PRIx32")", HIGH(routeid), LOW(routeid), src, dst);

      if(req != NULL){
         // This message indicates to the GUI that the route
         // requested has been downloaded but not started to be followed.
         // (That is decodedInvalidateRoute()'s responsibility.
         // As a side effect, the message below with routeid set to 0
         // will stop navigation in the GUI.
         // XXX Future fix: Don't use handleRouteDownloaded() to
         // report both that route is downloaded and that route is
         // available.
         m_guiCtrl->handleRouteDownloaded(0, MAX_INT32, MAX_INT32, 
                                          MAX_INT32, MAX_INT32, 
                                          0,0,0,0, req->reqID);
         m_pending->deallocate(req);
      } else {
         WARN("Never sent route req. with id %#02"PRIx32". Discarding.", dst);
      }
   }

   void UiCtrl::decodedRouteReply( uint32 /*src*/, uint32 dst )
   {
      // At the present this method should not get called
      // because all route requests should go through NavTask.

      RequestListItem* req = m_pending->dequeue( dst );
      if(req != NULL){
         WARN("Asked for route from NSC!!");
         m_guiCtrl->handleRequestFailed(Nav2Error::UC_ASKED_ROUTE_FROM_WRONG_MODULE,
                                        0, // This is a ThinClient GUI prot message.
                                        req->request);
         m_pending->deallocate(req); // this doesn't happen but if it did it should probably work as the other requests.
      }
      else {
         WARN("Never sent route req. with id %#02"PRIx32". Discarding.", dst);
      }
   }

   //MN: Take a good look at this method when handling status messages.
   void UiCtrl::decodedProgressMessage(NavServerComEnums::ComStatus status, 
                                       GuiProtEnums::ServerActionType type, 
                                       uint32 /*done*/, uint32 /*of*/,
                                       uint32 /*dst*/, uint32 /*src*/)
   {
      GenericGuiMess turnOn(GuiProtEnums::PROGRESS_INDICATOR, 
                            uint8(true), uint8(type));
      GenericGuiMess turnOff(GuiProtEnums::PROGRESS_INDICATOR, 
                             uint8(false), uint8(type));
      GenericGuiMess* toSend = NULL;
      switch(status){
      case NavServerComEnums::connecting:
      case NavServerComEnums::connected:
         toSend = &turnOn;
         break;
      case NavServerComEnums::done:
         toSend = &turnOff;
      default:
         break;
      }
      if(toSend){
         m_guiCtrl->sendMessageToGui(toSend, 0);
      }
   }

   void UiCtrl::decodedLatestNews(uint32 checksum, const uint8* data, 
                                  unsigned length, uint32 src, uint32 /*dst*/)
   {
      DBG("Received a LatestNews image of %u bytes, checksum %#"PRIx32", "
          "from %#"PRIx32, length, checksum, src);
      m_guiCtrl->handleLatestNews(data, length, checksum);
   }

   void UiCtrl::sendExtendedError( uint16 /*errorCode*/, uint16 /*errorID*/,
                                          const char* /*errorString*/)
   {
      DBG("Not sending error to GUI.");
/* MN; Not sending extended error to GUI.
      Packet *extError = new Packet( EXTENDED_ERROR, 
                                     errorID, true );
      extError->writeNextUnaligned16bit( errorCode );
      if(errorString != NULL){
         extError->writeNextCharString(errorString);
      } else {
         extError->writeNextCharString("");
      }
      extError->closeMessage();
      m_provider->sendData( extError );

      const char* tmpErrorString = "";
      if (errorString != NULL){
         tmpErrorString = errorString;
      }
      DBG("Sends extended error. error Code: %d, errorID: %d, error string: %s,",
          errorCode,
          errorID,
          tmpErrorString);
      */
   }

   
   void UiCtrl::decodedPositionState( 
      const PositionState& p, uint32 /*src*/ )
   {
      if (!m_printedPos && ( (p.lat != MAX_INT32) || (p.lon != MAX_INT32) ) ){
         INFO("POSITION HAS ARRIVED.");
         DBG("Lat: %"PRId32", Lon: %"PRId32".", p.lat, p.lon);
         DBG("Not printing more positions.");
         m_printedPos = true;
      }

      m_lastGpsData->posQuality     = p.gpsQuality;
      if(p.gpsQuality != QualityMissing){
         m_lastGpsData->speedQuality   = 3; //XXX
         m_lastGpsData->headingQuality = 3; //XXX
         m_lastGpsData->lat            = p.lat;
         m_lastGpsData->lon            = p.lon;
         m_lastGpsData->heading        = p.heading;
         m_lastGpsData->speed          = p.speed;
         m_lastGpsData->alt            = p.alt;
      } else {
         DBG("GPS missing");
         m_lastGpsData->setMissing();
      }

      m_guiCtrl->setPosition(m_lastGpsData->lat, m_lastGpsData->lon, 
                             m_lastGpsData->heading, m_lastGpsData->speed);
   }

   void UiCtrl::decodedGpsSatelliteInfo(
         GpsSatInfoHolder *satInfo, uint32 /*src*/ )
   {
      m_guiCtrl->handleSatelliteInfo(satInfo);
   }

   void UiCtrl::decodedFileOperation(class GuiFileOperation* op, uint32 /*src*/ )
   {
      m_guiCtrl->handleFileOperation(op);
   }

   void UiCtrl::decodedRouteInfo(
         const RouteInfo& r, uint32 /*src*/ )
   {
      m_guiCtrl->handleRouteInfo(r);
      m_audioCtrl->handleRouteInfo(r);
   }


   void UiCtrl::decodedRouteList(
         Buffer *buf, uint32 /*src*/, uint32 dst )
   {
      RequestListItem* req = m_pending->dequeue(dst);
      if(req){
#if 0
         m_guiCtrl->handleRouteList(buf);
#else
         if (m_lastRouteList) {
            delete m_lastRouteList;
         }
         m_lastRouteList = new RouteList(buf);
         m_guiCtrl->handleRouteList(*m_lastRouteList, req->reqID);
#endif
         m_pending->deallocate(req);
      } else {
         ERR("Never sent a RouteList request with id %0#10"PRIx32, dst);
      }
   }


   void UiCtrl::decodedInvalidateRoute(bool newRouteAvailable, int64 routeid,
                                       int32 tLat, int32 lLon, 
                                       int32 bLat, int32 rLon,
                                       int32 oLat, int32 oLon, 
                                       int32 dLat, int32 dLon, uint32 src)
   {
      DBG("decodedInvalidateRoute. newRouteAvailable:%s src:0x%08"PRIx32" )", 
           newRouteAvailable ? "TRUE" : "FALSE", src);

      if (m_lastRouteList) {
         delete m_lastRouteList;
         m_lastRouteList = NULL;
      }
      if(newRouteAvailable){
         // this is the result of a GUI request.
         DBG("Request from GUI");

         // Route reply has been received. Fetch the route data from 
         // NavTask.
         DBG("Sending RouteId, bounding box and start and end.");

         m_guiCtrl->handleRouteDownloaded( routeid,
               tLat, lLon, bLat, rLon,
               oLat, oLon, dLat, dLon, 0);
      } else {
         m_guiCtrl->handleUnsolicitedRouteDownloaded(0, MAX_INT32, MAX_INT32, 
                                                     MAX_INT32, MAX_INT32, 
                                                     0, 0, 0, 0);
      }
   }
   
   void UiCtrl::decodedReceiveData(int length, const uint8* data, uint32 /*src*/ )
   {
      // We are not interested in serial data when shutting down.
      if (m_shutdown){
         return;
      }

      DBG("Message data from GUI(on serial):");
      DBGDUMP("Serial data:", data, length);
      //DBG("pre setup: wpos: %d rpos: %d", m_parseBuffer->getWritePos(), 
      //     m_parseBuffer->getReadPos());
      m_parseBuffer->setWritePos(m_parseBuffer->getLength());
      //DBG("post sewpos: wpos: %d rpos: %d", m_parseBuffer->getWritePos(), 
      //    m_parseBuffer->getReadPos());
      m_parseBuffer->writeNextByteArray(data, length); //add data to buffer
      //DBG("post writearray: wpos: %d rpos: %d", m_parseBuffer->getWritePos(), 
      //    m_parseBuffer->getReadPos());
      m_parseBuffer->setReadPos(0);
      //DBG("post setup: wpos: %d rpos: %d", m_parseBuffer->getWritePos(), 
      //    m_parseBuffer->getReadPos());
      while(m_parseBuffer->getLength() > 4){ // possible to read length
         uint32 messageLength = m_parseBuffer->readNextUnaligned32bit();
         int protocol = (messageLength >> 24) & 0x0ff; //8 bit prot. ver.
         messageLength = messageLength & 0x0ffffff;     //24 bit length
         DBG("Protocol version: %d", protocol);
         DBG("Packet length: %"PRIu32, messageLength);
         DBG("Data in buffer: %"PRIu32" bytes", m_parseBuffer->getLength());
         if(m_parseBuffer->getLength() >= messageLength){
            //able to read at least one message from buffer.
            
#ifdef USE_THINCLIENT
            if ( protocol == GuiProtMess::THIN_CLIENT_GUI_PROT_NBR ){
               // Gui protocol from Thin Client.

               //Old MMI-messages use another kind of buffer.
               const uint8* ttmp = m_parseBuffer->accessRawData(0);
               DataBuffer buf(messageLength);
               buf.writeNextByteArray(ttmp, messageLength);
               buf.setCurrentOffset(0);

               NavCtrlMMIMessage* message
                  = NavCtrlMMIMessage::createMessageFromBuf(&buf);
               
               DBG("Created NavCtrlMMIMessage of type %x (hex).", message->getPacketType());

               bool print = true;
               if (allParamsReceived(print)){
                  //send message to handler.
                  m_guiCtrl->messageReceived(message);
                  delete message; // done with message
               }
               else{
                  m_waitingNavCtrlMessages->push_back( new pair<NavCtrlMMIMessage*, uint32>(message, static_cast<uint32>(GUI_MESS_TIMEOUT) )   );
                  DBG("Could not handle message direct. "
                      "m_waitingNavCtrlMessages->size() = %u",
                      m_waitingNavCtrlMessages->size());
               }
               
               // keep left over data.
               m_parseBuffer->setReadPos(messageLength);
            } else
#endif
               if ( protocol == GuiProtMess::CURRENT_GUI_PROT_NBR){
               GuiProtMess* message = 
                  GuiProtMess::createMsgFromBuf(m_parseBuffer);
               DBG("Created GuiProtMess of type: %x (hex).", message->getMessageType());
               
               bool print = true;
               if (allParamsReceived(print)){
                  m_guiCtrl->messageReceived(message);
                  m_audioCtrl->messageReceived(message);
                  message->deleteMembers();
                  delete message;
               }
               else{
                  m_waitingMessages->push_back(new pair<GuiProtMess*, uint32>(message, static_cast<uint32>(GUI_MESS_TIMEOUT) ) );
                  DBG("Could not handle message direct. "
                      "m_waitingMessages->size() = %u",
                      m_waitingMessages->size());
               }
            }

            int rem = 0;
            if((rem = m_parseBuffer->remaining()) > 0){
               Buffer tmp(rem);
               tmp.writeNextByteArray(m_parseBuffer->accessRawData(), rem);
               m_parseBuffer->takeDataAndClear(tmp); //read and write pos at 0
               m_parseBuffer->setWritePos(m_parseBuffer->getLength());
            } else {
               m_parseBuffer->clear(); //no data left
            }
         } else {
            break;
         }
         //ready to either parse more data or to wait for more data.
      }
      DBG("UiCtrl::decodedReceiveData. "
          "Not enough data to parse another message.");
      //not enough data to parse another message.
   }


   bool UiCtrl::allParamsReceived(bool print)
   {
      bool result = true;

      if(!m_allParamsReceived){
         uint32 i = 0;
         while ( (result) && (i < m_uiCtrlParamsSize) ){
            result = m_uiCtrlParams[i].received;
            i++;
         }
         m_allParamsReceived = result;

         if (m_allParamsReceived){
            INFO("All needed parameters have been received.");
            handleWaitingMessages();
         }
         else{
            if (print){

               // Print what parameter(s) is not handled correctly.
               INFO("Still waiting for parameters.");
               for(uint32 j = 0; j<m_uiCtrlParamsSize; j++){
                  if(!m_uiCtrlParams[j].received){
                     INFO("Not received param: 0x%x", m_uiCtrlParams[j].paramId);
                  }
               }
            }
         }
      }

      return result;
   }

   void UiCtrl::paramReceivedNotSimpleParam(ParameterEnums::ParamIds paramId)
   {
      bool print = true;
      if (!allParamsReceived()){
      
         bool found = false;
         uint32 i = 0;
         while ( (!found) && (i<m_uiCtrlParamsSize) ){
            found = (m_uiCtrlParams[i].paramId == paramId);

            if (!found){
               i++;
            }
         }
         if (found){
            m_uiCtrlParams[i].received = true;
            allParamsReceived(print);
         }      
         else{
            // This happens if we have forgot to add a parameter to
            // m_uiCtrlParams.
            WARN("Received parameter we did not ask for. ParamId = 0x%08x", paramId);
         }
      }
   }

   void UiCtrl::paramReceived(ParameterEnums::ParamIds paramId, 
                              const GuiProtEnums::ParameterTypeHolder& guiParamId)
   {
      paramReceivedNotSimpleParam(paramId);

      DBG("paramReceived 0x%x", paramId );

      // Only notify the GUI that parameters have been received
      m_guiCtrl->sendParameterChangedToGui(guiParamId);
   }

   void UiCtrl::checkWaitingMessagesTimeOut()
   {
      // GuiProtMess messages.
      {
         deque<pair<GuiProtMess*, uint32>* >::iterator it =
            m_waitingMessages->begin();
         while ( it != m_waitingMessages->end() ){

            if ( (*it)->second <= 0){
               pairdef* currentPair = *it;
               GuiProtMess* guiMess = currentPair->first;
               it = m_waitingMessages->erase(it);

               // Send request failed to GUI.
               uint16 guiSeqId = guiMess->getMessageID();
               uint16 requestMessageType = guiMess->getMessageType();
               m_guiCtrl->handleRequestFailed( Nav2Error::UC_REQUEST_TIMED_OUT,
                                               guiSeqId,
                                               requestMessageType );
               guiMess->deleteMembers();
               delete guiMess;
               delete currentPair;
            }
            else{
               (*it)->second--;
               it++;
            }
         }
      }

#ifdef USE_THINCLIENT
      // NavCtrlMMIMessage messages.
      {
         deque<pair<NavCtrlMMIMessage*, uint32>* >::iterator it =
            m_waitingNavCtrlMessages->begin();
         while ( it != m_waitingNavCtrlMessages->end() ){

            if ( (*it)->second <= 0){
               NavCtrlMMIMessage* guiMess = (*it)->first;
               it = m_waitingNavCtrlMessages->erase(it);

               // Send request failed to GUI.
               uint16 guiSeqId = 0;
               uint16 requestMessageType = guiMess->getPacketType();
               m_guiCtrl->handleRequestFailed( Nav2Error::UC_REQUEST_TIMED_OUT,
                                               guiSeqId,
                                               requestMessageType );
            }
            else{
               (*it)->second--;
               it++;
            }
         }
      }
#endif

   
   } // checkWaitingMessagesTimeOut

   void UiCtrl::handleWaitingMessages(){
      DBG("Handles waiting messages.");

      while(!m_waitingMessages->empty()){
         pairdef* currentPair = m_waitingMessages->front();
         GuiProtMess* message = currentPair->first;
         m_guiCtrl->messageReceived(message);
         m_audioCtrl->messageReceived(message);
         message->deleteMembers();
         delete message;
         m_waitingMessages->pop_front();
         delete currentPair;
      }

#ifdef USE_THINCLIENT
      while(!m_waitingNavCtrlMessages->empty()){
         NavCtrlMMIMessage* message = m_waitingNavCtrlMessages->front()->first;
         m_guiCtrl->messageReceived(message);
         delete message;
         m_waitingNavCtrlMessages->pop_front();
      }
#endif
   }

   void UiCtrl::decodedParamNoValue(uint32 paramId, uint32 src, uint32 dst)
   {
      DBG("Parameter unset 0x%"PRIx32, paramId);
      const enum ParameterEnums::ParamIds enumParamId = 
         ParameterEnums::ParamIds(paramId);
      switch(paramId){
      case ParameterEnums::NSC_ServerHostname:
         // Someone else sets default.
         DBG("Received empty NSC_ServerHostname parameter.");
         break;
      case ParameterEnums::NSC_HttpServerHostname:
         // Someone else sets default.
         DBG("Received empty NSC_HttpServerHostname parameter.");
         break;
      case ParameterEnums::UC_WebUser:
         {
            DBG("Received empty UC_WebUser parameter.");
            const char *defaultWebUser = "unknown";
            m_paramProvider->setParam(enumParamId, &defaultWebUser );
         } 
         break;
      case ParameterEnums::UC_WebPasswd:
         // Someone else sets default.
         DBG("Received empty UC_WebPasswd parameter.");
         break;
      case ParameterEnums::NSC_UserAndPasswd:
         // Someone else sets default.
         DBG("Received empty NSC_UserAndPasswd parameter.");
         break;
      case ParameterEnums::NSC_TransportationType:
         // Someone else sets default.
         DBG("Received empty NSC_TransportationType parameter.");
         break;
      case ParameterEnums::UC_DistanceMode:
         {
            DBG("Received empty UC_DistanceMode parameter.");
            const DistancePrintingPolicy::DistanceMode mode = 
               DistancePrintingPolicy::ModeInvalid;
            m_guiCtrl->setDistanceMode(mode);
            m_audioCtrl->setDistanceMode(mode);
            const int32 defaultDistanceMode = mode;
            m_paramProvider->setParam(enumParamId, &defaultDistanceMode );
         }
         break;
#if defined ( NAV2_USE_UTF8 )
      case ParameterEnums::TopRegionList:
#else
      case ParameterEnums::TopRegionListLegacy:
#endif
         // Someone else sets default.
         DBG("Received empty TopRegionList parameter.");
         break;
      case ParameterEnums::UC_MaxNumberOfGuiSearchResults:
         {
            DBG("Setting default value for UC_MaxNumberOfGuiSearchResults");
            const int32 defaultNbrOfSearchMatches = 
               DEFAULT_NUMBER_OF_SEARCH_MATCHES;
            m_guiCtrl->setMaxNbrSearchMatches( defaultNbrOfSearchMatches );
            m_paramProvider->setParam(enumParamId, &defaultNbrOfSearchMatches);
            //MN: maxNbrSearchMatches should be set by the GUI. ==> Default
            //    only used as fallback.
         } 
         break;
      case ParameterEnums::UC_SoundVolume:
         {
            DBG("Setting default value for UC_SoundVolume parameter.");
            const uint8 soundVolume = 70;
            const int32 defaultSoundVolume = soundVolume;
            m_guiCtrl->setDeviceSoundVolume(soundVolume);
            m_paramProvider->setParam(enumParamId, &defaultSoundVolume );
         }
         break;
      case ParameterEnums::UC_UseMainSpeaker:
         {
            DBG("Setting default value for UC_UseMainSpeaker parameter.");
            const int32 defaultUseMainSpeaker = 1; //bool true.
            m_guiCtrl->setDeviceUseSpeaker(bool(defaultUseMainSpeaker));
            m_paramProvider->setParam(enumParamId, &defaultUseMainSpeaker );
         }
         break;
      case ParameterEnums::UC_AutoReroute:
         {
            DBG("Setting default value for UC_AutoReroute parameter.");
            const int32 defaultAutoReroute = 1;    //true
            m_guiCtrl->setAutoReroute(bool(defaultAutoReroute));
            m_paramProvider->setParam(enumParamId, &defaultAutoReroute );
         } 
         break;
      case ParameterEnums::UC_CallCenterNumbers: //Deprecated
         // XXX Deprecated XXX
         break;
      case ParameterEnums::UC_StoreSMSDestInMyDest:
         {
            DBG("Setting default value for UC_StoreSMSDestInMyDest");
            const enum GuiProtEnums::YesNoAsk storeSMS = GuiProtEnums::ask;
            m_guiCtrl->setStoreSMSDestInMyDest(storeSMS);
            const int32 defaultStoreSMSDestInMyDest = storeSMS;
            m_paramProvider->setParam(enumParamId,
                                      &defaultStoreSMSDestInMyDest);
         } 
         break;
      case ParameterEnums::UC_AutomaticRouteOnSMSDest:
         {
            DBG("Setting default value for UC_AutomaticRouteOnSMSDest");
            const int32 defaultAutomaticRouteOnSMSDest = 1; //bool true;
            const bool autoReroute(defaultAutomaticRouteOnSMSDest);
            m_guiCtrl->setAutomaticRouteOnSMSDest(autoReroute);
            m_paramProvider->setParam(enumParamId, 
                                      &defaultAutomaticRouteOnSMSDest );
         } 
         break;
      case ParameterEnums::UC_KeepSMSDestInInbox:
         {
            DBG("Setting default value for UC_KeepSMSDestInInbox parameter.");
            const enum GuiProtEnums::YesNoAsk keepSMS = GuiProtEnums::ask;
            m_guiCtrl->setKeepSMSDestInInbox(keepSMS);
            const int32 defaultKeepSMSDestInInbox = keepSMS;
            m_paramProvider->setParam(enumParamId, &defaultKeepSMSDestInInbox);
         }
         break;
      case ParameterEnums::UC_BacklightStrategy:
         {
            DBG("Setting default value for UC_BacklightStrategy parameter.");
            const enum GuiProtEnums::BacklightStrategy strategy = 
               GuiProtEnums::backlight_always_on;
            m_guiCtrl->setBacklightStrategy(strategy);
            const int32 defaultBacklightStrategy = strategy;
            m_paramProvider->setParam(enumParamId, &defaultBacklightStrategy);
         } 
         break;

      case ParameterEnums::UC_DirectionType:
         {
            DBG("Setting default value for UC_DirectionType parameter.");
            const int32 defaultDirectionType = 0; //Default: clock based
            m_guiCtrl->setDirectionType(defaultDirectionType); 
            m_paramProvider->setParam(enumParamId, &defaultDirectionType);
         }
         break;
      case ParameterEnums::UC_FeedOutputFormat:
         {
            DBG("Setting default value for UC_FeedOutputFormat parameter.");
            const int32 defaultFeedFormat = 0; //Default: concise
            m_guiCtrl->setFeedOutputFormat(defaultFeedFormat); 
            m_paramProvider->setParam(enumParamId, &defaultFeedFormat);
         }
         break;
      case ParameterEnums::UC_FeedWaitPeriod:
         {
            const int32 defaultWaitPeriod = 10;
            m_guiCtrl->setVicinityFeedWaitingPeriod(defaultWaitPeriod); 
            m_paramProvider->setParam(enumParamId, &defaultWaitPeriod);
         } break;
      case ParameterEnums::UC_LockedNavWaitPeriod:
         {
            const int32 defaultWaitPeriod = 15;
            m_guiCtrl->setLockedNavWaitingPeriod(defaultWaitPeriod); 
            m_paramProvider->setParam(enumParamId, &defaultWaitPeriod);
         } break;
      case ParameterEnums::UC_GpsMinWalkingSpeed:
         {
            const int32 minSpeed = 3;
            m_guiCtrl->setGpsMinWalkingSpeed(minSpeed); 
            m_paramProvider->setParam(enumParamId, &minSpeed);
         } break;
     case ParameterEnums::UC_VicinityListUpdatePeriod:
         {
            const int32 defaultPeriod = 20;
            m_guiCtrl->setVicinityListUpdatePeriod( defaultPeriod );
            m_paramProvider->setParam(enumParamId, &defaultPeriod);
         } break;
      case ParameterEnums::UC_LinkLayerKeepAlive:
         {
            DBG("Setting default value for UC_LinkLayerKeepAlive.");
            const int32 defaultLinkLayer = 1; //true
            m_paramProvider->setParam(enumParamId, &defaultLinkLayer);
            decodedParamValue(enumParamId, &defaultLinkLayer, 1, src, dst);
         }
         break;
      case ParameterEnums::SelectedAccessPointIdReal:
         {
            /* No access point parameter. */
            /* Initialize to IMEI:-1 */
            char defaultIAP[16];
#ifdef __WINS__
            const int32 defaultIapId = -2; // Ask
#else
            const int32 defaultIapId = -1; // Ask
#endif
            sprintf(defaultIAP, "%"PRIi32, defaultIapId);

            const char * defaultSet[2];
            defaultSet[0] = m_serialNumber;
            defaultSet[1] = defaultIAP;
            DBG("Setting default value for UC_SelectedAccessPointIdReal.");
            m_guiCtrl->setSelectedAccessPointIdReal(defaultIapId);
            m_guiCtrl->sendSelectedAccessPointIdReal();
         } 
         break;
      case ParameterEnums::SelectedAccessPointId:
         /* Old compatibility parameter. */
         DBG("UC_SelectedAccessPointId - ignored");
         break;
      case ParameterEnums::SelectedAccessPointId2:
         {
            DBG("Setting default value for UC_SelectedAccessPointId2 param.");
            const int32 val = m_guiCtrl->getSelectedAccessPointId();
            m_guiCtrl->setSelectedAccessPointId2(val);
            // Change to string here
            char* tmpStr = m_guiCtrl->makeSelectedAccessPointId2String(val);
            m_paramProvider->setParam( enumParamId, &tmpStr );
            delete [] tmpStr;
         } 
         break;
      case ParameterEnums::UC_MuteTurnSounds:
         {
            DBG("Setting default value for UC_MuteTurnSounds param.");
            // Mute turn sounds on phone call.
            const int32 defaultMuteTurnSounds = 1; 
            m_guiCtrl->setMuteTurnSounds(defaultMuteTurnSounds);
            m_paramProvider->setParam(enumParamId, &defaultMuteTurnSounds );
         }
         break;
      case ParameterEnums::UC_TurnSoundsLevel:
         {
            DBG("Setting default value for UC_TurnSoundsLevel param.");
            // The default level is normal.
            //const int32 defaultTurnSoundsLevel = GuiProtEnums::turnsound_normal; 
            const int32 defaultTurnSoundsLevel = GuiProtEnums::turnsound_mute; 
            m_guiCtrl->setTurnSoundsLevel(defaultTurnSoundsLevel);
            m_paramProvider->setParam(enumParamId, &defaultTurnSoundsLevel );
            m_audioCtrl->setVerbosity(static_cast<GuiProtEnums::TurnSoundsLevel>(defaultTurnSoundsLevel));
         }
         break;
      case ParameterEnums::WayfinderType:
         {
            const int32 wftype = GuiProtEnums::Trial;
            DBG("Setting default value for WayfinderType param.");
            m_paramProvider->setParam(enumParamId, &wftype, 1);
         } 
         break;
      case ParameterEnums::TR_trackLevel: 
         {
            int32 defaultTrackingLevel = GuiProtEnums::tracking_level_none;
            m_paramProvider->setParam( enumParamId, &defaultTrackingLevel );
         } 
         break;
      default:
         m_guiCtrl->receivedGeneralParamUnset(paramId, 0);
         // This happens if there are parameters that we are not
         // interested in, in multicast groups we want some 
         // parameters from.
         DBG("Received unknown %s parameter 0x%"PRIx32, "unset", paramId);
      }
   }

   void UiCtrl::decodedParamValue(uint32 paramId, const int32 * data,
                                  int32 numEntries, uint32 /*src*/, 
                                  uint32 dst)
   {
      DBG("Parameter int32 0x%"PRIx32, paramId);

      switch(paramId){
      case ParameterEnums::NSC_TransportationType:
         {
            m_guiCtrl->setTransportationType(NavServerComEnums::VehicleType(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramTransportationType);
         } break;
      case ParameterEnums::UC_DistanceMode:
         {
            DistancePrintingPolicy::DistanceMode mode =
               DistancePrintingPolicy::DistanceMode(data[0]);
            m_guiCtrl->setDistanceMode(mode);
            m_audioCtrl->setDistanceMode(mode);
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramDistanceMode);
         } break;
      case ParameterEnums::UC_MaxNumberOfGuiSearchResults:
         {
            int32 max_num = static_cast<uint8>(data[0]);
            if (max_num != DEFAULT_NUMBER_OF_SEARCH_MATCHES) {
               // XXX number of max searches is not changeable by user.
               // XXX We therefore need to rewrite the value to allow
               // XXX old users to get the change.
               max_num = DEFAULT_NUMBER_OF_SEARCH_MATCHES;
               m_paramProvider->setParam(
                     ParameterEnums::ParamIds(paramId),
                     &max_num);
            }

            m_guiCtrl->setMaxNbrSearchMatches(max_num);
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramMaxNbrOfSearchResults);
         }
      case ParameterEnums::UC_SoundVolume:
         {
            m_guiCtrl->setDeviceSoundVolume(static_cast<uint8>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramSoundVolume);
         } break;
      case ParameterEnums::UC_UseMainSpeaker:
         {
            m_guiCtrl->setDeviceUseSpeaker(static_cast<bool>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramUseSpeaker);
         } break;
      case ParameterEnums::UC_AutoReroute:
         {
            m_guiCtrl->setAutoReroute(static_cast<bool>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramAutoReroute);
         } break;
      case ParameterEnums::UC_StoreSMSDestInMyDest:
         {
            m_guiCtrl->setStoreSMSDestInMyDest(static_cast<GuiProtEnums::YesNoAsk>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramStoreSMSDestInMyDest);
         } break;
      case ParameterEnums::UC_AutomaticRouteOnSMSDest:
         {
            m_guiCtrl->setAutomaticRouteOnSMSDest(static_cast<bool>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramAutomaticRouteOnSMSDest);
         } break;
      case ParameterEnums::UC_KeepSMSDestInInbox:
         {
            m_guiCtrl->setKeepSMSDestInInbox(static_cast<GuiProtEnums::YesNoAsk>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramKeepSMSDestInInbox);
         } break;
      case ParameterEnums::UC_BacklightStrategy:
         {
            m_guiCtrl->setBacklightStrategy(
                  static_cast<GuiProtEnums::BacklightStrategy>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId),
                  GuiProtEnums::paramBacklightStrategy);
         } break;
      case ParameterEnums::SelectedAccessPointId:
         {
            /* Just ignore this parameter. */
         } break;
      case ParameterEnums::SelectedAccessPointId2:
         {
            DBG( "Got int SelectedAccessPointId2 %"PRId32, data[0] );
            m_guiCtrl->setSelectedAccessPointId2(
                  static_cast<int32>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId),
                  GuiProtEnums::paramSelectedAccessPointId2);
         } break;
      case ParameterEnums::UC_MuteTurnSounds:
         {
            m_guiCtrl->setMuteTurnSounds(
                  static_cast<int32>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId),
                          GuiProtEnums::paramMuteTurnSound);
         } break;
      case ParameterEnums::UC_DirectionType:
         {
            m_guiCtrl->setDirectionType(static_cast<uint8>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId),
                          GuiProtEnums::userDirectionMode);
         } break;
      case ParameterEnums::UC_FeedOutputFormat:
         {
            m_guiCtrl->setFeedOutputFormat(static_cast<uint8>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId),
                          GuiProtEnums::userVicinityFeedMode);
         } break;

      case ParameterEnums::UC_FeedWaitPeriod:
         {
            m_guiCtrl->setVicinityFeedWaitingPeriod(static_cast<uint8>(data[0]));
            paramReceived( ParameterEnums::ParamIds(paramId),
                           GuiProtEnums::paramFeedWaitPeriod );

         } break;
      case ParameterEnums::UC_LockedNavWaitPeriod:
         {
            m_guiCtrl->setLockedNavWaitingPeriod(static_cast<uint8>(data[0]));
            paramReceived( ParameterEnums::ParamIds(paramId),
                           GuiProtEnums::paramLockedNavWaitPeriod );

         } break;
      case ParameterEnums::UC_GpsMinWalkingSpeed:
         {
            m_guiCtrl->setGpsMinWalkingSpeed(static_cast<uint8>(data[0]));
            paramReceived( ParameterEnums::ParamIds(paramId),
                           GuiProtEnums::paramGpsMinWalkingSpeed );

         } break;
     case ParameterEnums::UC_VicinityListUpdatePeriod:
         {
            m_guiCtrl->setVicinityListUpdatePeriod(static_cast<uint8>(data[0]));
            paramReceived( ParameterEnums::ParamIds(paramId),
                           GuiProtEnums::paramVicinityListUpdatePeriod );
         } break;
      case ParameterEnums::UC_TurnSoundsLevel:
         {
            m_guiCtrl->setTurnSoundsLevel(
                  static_cast<uint8>(data[0]));
            paramReceived(ParameterEnums::ParamIds(paramId),
                  GuiProtEnums::paramTurnSoundsLevel);
            m_audioCtrl->setVerbosity(static_cast<GuiProtEnums::TurnSoundsLevel>(data[0]));
         } break;
#if 0
         /* This is now sent as a general parameter. */
      case ParameterEnums::Language:
         {
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramLanguage);
         } break;
#endif

      default:
         {
            RequestListItem* req = m_pending->dequeue(dst);
            paramReceivedNotSimpleParam(ParameterEnums::ParamIds(paramId));
            m_guiCtrl->receivedGeneralParam(paramId, data, numEntries, 
                                            req ? req->reqID : 0);
            if(req){
               m_pending->deallocate(req);
            }
         }
         // This happens if there are parameters that we are not
         // interested in, in multicast groups we want some 
         // parameters from.
         DBG("Received unknown %s parameter 0x%"PRIx32, "int32", paramId);
      }
   } // decodedParamValue(int32)

   void UiCtrl::decodedParamValue(uint32 paramId, const float * data,
                                         int32 numEntries, uint32 /*src*/,
                                         uint32 dst)
   {
      DBG("Parameter float 0x%"PRIx32, paramId);
      switch(paramId){
      case 0: //to avoid compiler warning about missing cases. Damn MS!
      default:
         {
            RequestListItem* req = m_pending->dequeue(dst);
            paramReceivedNotSimpleParam(ParameterEnums::ParamIds(paramId));
            m_guiCtrl->receivedGeneralParam(paramId, data, numEntries, 
                                            req ? req->reqID : 0);
            if(req){
               m_pending->deallocate(req);
            }
            // This happens if there are parameters that we are not
            // interested in, in multicast groups we want some 
            // parameters from.
            DBG("Received unknown %s parameter 0x%"PRIx32, "float", paramId);
         }
      } // switch
   } // decodedParamValue(float)

   void UiCtrl::decodedParamValue(uint32 paramId,
                                         const char * const * data, 
                                         int32 numEntries, uint32 /*src*/,
                                         uint32 dst)
   {
      DBG("Parameter char* 0x%"PRIx32, paramId);
      switch(paramId){
      case ParameterEnums::NSC_ServerHostname:
      case ParameterEnums::NSC_HttpServerHostname :
         {
            
            // Calculate string length.
            uint32 totalStringLength = 0;
            int32 i = 0;
            for (i = 0; i < numEntries; i++){
               totalStringLength += strlen(data[i]);
            }
            // Add space for semicolon.
            totalStringLength += (numEntries - 1) * 1;
            // Add space for terminating null byte.
            totalStringLength += 1;
            

            // Put the string together.
            char* serverHostAndPortStrings = 
               new char[totalStringLength];
            uint32 sizeLeft = totalStringLength;
            strcpy(serverHostAndPortStrings, "");
            for (i = 0; i < numEntries; i++){
               strncat(serverHostAndPortStrings, data[i], sizeLeft);
               DBG("serverPortString: %s", data[i]);
               sizeLeft -= (strlen(data[i]));
               if (i < (numEntries-1) ){
                  // There is at least one more string to add.
                  strncat(serverHostAndPortStrings, ";", sizeLeft);
                  sizeLeft -= (1);
               }
            }

            DBG("UiCtrl::decodedParamValue. serverHostAndPortStrings = %s", serverHostAndPortStrings);
            m_guiCtrl->setServerHostAndPortStrings(serverHostAndPortStrings);
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramServerNameAndPort);

            delete[] serverHostAndPortStrings;


         } break;
      case ParameterEnums::NSC_UserAndPasswd:
         {
            if (numEntries == 2){
               DBG("UiCtrl::decodedParamValue. user %s pw %s",
                   data[0], data[1]);

               m_guiCtrl->setUserNameAndPass(data[0], data[1]);
               m_guiCtrl->initWebData(data[0], data[1]);
               paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramUserName);
               paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramUsername);
               paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramPassword);
            }
            else{
               WARN("numEntries in user name and passwd param != 2. "
                    "nbrEntries=%"PRIu32, numEntries);
               unsolicitedError(Nav2Error::UC_CONFUSED);
            }
         } break;
      case ParameterEnums::UC_WebUser:
         {
            DBG("UiCtrl::decodedParamValue. webuser %s", data[0]);
            m_guiCtrl->setWebUsername(data[0]);
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramWebUsername);
         } break;
      case ParameterEnums::UC_WebPasswd:
         {
            DBG("UiCtrl::decodedParamValue. webpass %s", data[0]);
            m_guiCtrl->setWebPassword(data[0]);
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramWebPassword);
         } break;
      case ParameterEnums::UC_CallCenterNumbers: //Deprecated
         // XXX Deprecated XXX
         break;
      case ParameterEnums::SelectedAccessPointId2:
         {
            DBG( "Got SelectedAccessPointId2 %s", data[0] );
            int32 id = atoi( data[0] );
            /*Parameter problem test*/
            if ( id != -1 ) {
               // Retuned default value from Paramters no need to send
               m_guiCtrl->setSelectedAccessPointId2( id );
               paramReceived( ParameterEnums::ParamIds(paramId),
                              GuiProtEnums::paramSelectedAccessPointId2 );
            }
         } break;

      case ParameterEnums::SelectedAccessPointIdReal:
         {
            /* Received access point array. */
            /* Find the entry that corresponds to our serial number. */
            const char *serialNo = NULL;
            int32 iapNo = -1;
            for (int32 i = 0; i < numEntries ; i++) {
               if (strcmp(data[i], m_serialNumber) == 0) {
                  /* The same! */
                  serialNo = data[i];
                  if ((i+1) < numEntries &&
                        (((data[i+1][0] >= '0') && (data[i+1][0] <= '9')) ||
                          data[i+1][0] == '-')) {
                     iapNo = atoi(data[i+1]);
                  }
                  DBG("Setting value for UC_SelectedAccessPointIdReal");
                  m_guiCtrl->setSelectedAccessPointIdReal(iapNo, data, 
                                                          numEntries);
                  paramReceived(ParameterEnums::ParamIds(paramId),
                           GuiProtEnums::paramSelectedAccessPointId);
                  /* Send also as a General Parameter. */
                  RequestListItem* req = m_pending->dequeue(dst);
                  m_guiCtrl->receivedGeneralParam(ParameterEnums::SelectedAccessPointIdReal,
                                                  &iapNo, 1, 
                                                  req ? req->reqID : 0);
                  if(req){
                     m_pending->deallocate(req);
                  }
                  break;
               }
            }
            if (!serialNo) {
               /* Didn't find an access point for our hardware. */
               /* Initialize to IMEI:-1 */
               DBG("Setting default value for UC_SelectedAccessPointIdReal.");
               int32 defaultSelectedAccessPointId = -1; // Ask
               m_guiCtrl->setSelectedAccessPointIdReal(
                     static_cast<int32>(defaultSelectedAccessPointId),
                     data, numEntries);
               m_guiCtrl->setSelectedAccessPointIdReal(
                     static_cast<int32>(defaultSelectedAccessPointId));
               m_guiCtrl->sendSelectedAccessPointIdReal();
            }

         } break;
      default:
         {
            RequestListItem* req = m_pending->dequeue(dst);
            paramReceivedNotSimpleParam(ParameterEnums::ParamIds(paramId));
            m_guiCtrl->receivedGeneralParam(paramId, data, numEntries, 
                                            req ? req->reqID : 0);
            if(req){
               m_pending->deallocate(req);
            }
         }
         // This happens if there are parameters that we are not
         // interested in, in multicast groups we want some 
         // parameters from.
         DBG("Received unknown %s parameter 0x%"PRIx32, "string", paramId);
      }
   } // decodedParamValue(char*)


   void UiCtrl::decodedParamValue(uint32 paramId, const uint8* data, int size, 
                                  uint32 /*src*/, uint32 dst)
   {
      DBG("Parameter uint8* 0x%"PRIx32, paramId);
      switch(paramId){
#if defined ( NAV2_USE_UTF8 )
      case ParameterEnums::TopRegionList:
#else
      case ParameterEnums::TopRegionListLegacy:
#endif
         {
            //The const_cast isn't pretty, but the data isn't changed
            //by anything in this function, and the call to
            //Buffer::releaseData below releases the data so that it's
            //not deleted when the Buffer tmp is destroyed.
            // I have seen the NonOwningBuffer class that hides this ugly
            // thing...
            Buffer tmp(const_cast<uint8*>(data), size, size);
            tmp.setReadPos(0);
            m_guiCtrl->setTopRegionList(new TopRegionList(tmp));
            paramReceived(ParameterEnums::ParamIds(paramId), GuiProtEnums::paramTopRegionList);
            tmp.releaseData();
         }
         break;

      default:
         {
            RequestListItem* req = m_pending->dequeue(dst);
            paramReceivedNotSimpleParam(ParameterEnums::ParamIds(paramId));
            m_guiCtrl->receivedGeneralParam(paramId, data, size, 
                                            req ? req->reqID : 0);
            if(req){
               m_pending->deallocate(req);
            }
         }
         // This happens if there are parameters that we are not
         // interested in, in multicast groups we want some 
         // parameters from.
         DBG("Received unknown %s parameter 0x%"PRIx32, "binary block", 
                 paramId);
      }
   } // decodedParamValue(uint8*)


   void UiCtrl::setCharValue(char*& dst, const char* src)
   {
      size_t srclen = 0;
      if(src != NULL){
         srclen = strlen(src);
      }
      if(dst != NULL && (src == NULL || strlen(dst) < srclen)){
         delete[] dst;
         dst = NULL;
      }
      if(dst == NULL && src != NULL){
         dst = new char[srclen + 1];
      }
      if(src != NULL){
         strcpy(dst, src);
      }
   }

   // XXX:
   // Merge this method with checkForRequestAndCreate( uint16 id )
   // when it is no longer called directly from the methods 
   // handling ThinClient GUI messages.
   RequestListItem* 
   UiCtrl::checkForRequestAndCreate() {
      RequestListItem* req = m_pending->allocate();
      if ( req == NULL ) {
         // Oups! Too many outstanding packets!
         ERR( "UiCtrl::checkForRequestAndCreate. "
              "Memory problem when creating request.");
      }
      
      //MN: Debug print.
      DBG("CheckForRequestAndCreate old. outstanding:%u, remaining:%u.",
          m_pending->outstanding(), m_pending->remaining() );
      return req;
   } // checkForRequestAndCreate()

   RequestListItem* 
   UiCtrl::checkForRequestAndCreate( uint32 guiMessageType, uint16 guiMessageId ) {
      RequestListItem* result = NULL;
         
      // Check if we already have a request with this ID pending. 
      RequestListItem* req = m_pending->dequeue( guiMessageId );
      if ( req != NULL ) {
         // Hm... Resend request ?
         // We ignore this and discard the packet
         // on the assumption that it is a resend.

         DBG( "Got resent request: %i, ignoring", guiMessageId );
         m_pending->enqueue( req );
      }
      else{
         result = checkForRequestAndCreate();
         if(result != NULL){
            result->request = guiMessageType;
            result->reqID = guiMessageId; // Sequence id from GUI.
         }
         else{
            //sendExtendedError does nothing anyway
            // sendExtendedError( nav_out_other_error, guiMessageId );
         }
      }
            
      //MN: Debug print.
      DBG("CheckForRequestAndCreate new. outstanding:%u, ramaining:%u.",
          m_pending->outstanding(), m_pending->remaining() );
      return result;
   } // checkForRequestAndCreate( uint32 guiMessageType, uint16 guiMessageId )

   void 
   UiCtrl::addRequest(uint32 nav2messageId, RequestListItem* req)
   {
      req->src = nav2messageId;

      if ( req->reqID == MAX_UINT16 ) {
         WARN("UiCtrl::addRequest. requestId = MAX_UINT16");
         unsolicitedError(Nav2Error::UC_CONFUSED);

         if ( !m_pending->deallocate( req ) ) {
            ERR( "UiCtrl: Error deallocating req_elem." );
            unsolicitedError(Nav2Error::UC_CONFUSED);
         }
      } else {
         if( !m_pending->enqueue( req ) ) {
            ERR( "UiCtrl: Internal error, pending or req is zero..." );
            unsolicitedError(Nav2Error::UC_CONFUSED);
         }
      }

      //MN: Debug print.
      DBG("Added request. outstanding:%u, remaining:%u.",
          m_pending->outstanding(), m_pending->remaining() );
   } // addRequest

   
   void
   UiCtrl::dumpFavorite(GuiFavorite* guiFav){
      DBG("GuiFavorite: name: %s, ID: %"PRIu32,
          guiFav->getName(), guiFav->getID() );
   } // dumpFavorite

   void
   UiCtrl::dumpFavorite(Favorite* fav){
      DBG("GuiFavorite: name: %s, ID: %"PRIu32","
          "lat: %"PRId32", lon: %"PRId32","
          "shortName: %s, description: %s,"
          "category: %s, mapIconName: %s.", 
          fav->getName(), fav->getID(),
          fav->getLat(), fav->getLon(),
          fav->getShortName(), fav->getDescription(),
          fav->getCategory(), fav->getMapIconName());
   } // dumpFavorite

   void 
   UiCtrl::decodedGetFavoritesReply( vector<GuiFavorite*>& guiFavorites, 
                                             uint32 /*src*/, uint32 dst )
   {
      RequestListItem* req = m_pending->dequeue( dst );
      
      DBG("Got GetFavoritesReply from Destinations module.");
      if(req != NULL){ 
         m_guiCtrl->handleGetFavoritesReply(guiFavorites, req->reqID);

#if 0
         // Debug print.
         uint32 numFav = guiFavorites.size();
         for (uint32 i = 0; i < numFav; i++){
            dumpFavorite(guiFavorites[i]);
         }
#endif
         
         m_pending->deallocate(req);
      }
      else{
         DBG("Got get favorites reply we did not ask for. "
             "Do not handle it.");
      }
   } // decodedGetFavoritesReply

   void 
   UiCtrl::decodedGetFavoritesAllDataReply( vector<Favorite*>& favorites, 
                                             uint32 /*src*/, uint32 dst  )
   {
      RequestListItem* req = m_pending->dequeue( dst );
      
      DBG("Got GetFavoritesAllDataReply from Destinations module.");
      if(req != NULL){
         m_guiCtrl->handleGetFavoritesAllDataReply(favorites, req->reqID);

         // Debug print.
         for (uint32 i = 0; i < favorites.size(); i++){
            dumpFavorite(favorites[i]);
         }

         m_pending->deallocate(req);
      }
      else{
         DBG("Got get favories all data reply we did not ask for. "
             "Do not handle it.");
      }
   } // decodedGetFavoritesAllDataReply
   
   void 
   UiCtrl::decodedGetFavoriteInfoReply( Favorite* favorite, 
                                        uint32 /*src*/, uint32 dst  )
   {
      RequestListItem* req = m_pending->dequeue( dst );
      
      DBG("Got GetFavoritesInfoReply from Destinations module.");
      
      if (req != NULL){
         GuiProtEnums::MessageType guiMsgType = 
            GuiProtEnums::MessageType(req->request);
         uint16 guiSeqId = req->reqID;
         m_guiCtrl->handleGetFavoriteInfoReply(favorite, guiMsgType, guiSeqId);

         // Debug print.
         dumpFavorite(favorite);

         m_pending->deallocate(req);
      }
      else{
         DBG("Got get favorite info reply we did not ask for. "
             "Do not handle it.");
      }
   } // decodedGetFavoriteInfoReply

   void 
   UiCtrl::decodedFavoritesChanged(  uint32 /*src*/, uint32 /*dst*/  )
   {
      DBG("Got GetFavoritesChanged from Destinations module.");
      m_guiCtrl->handleFavoritesChanged();
   } // decodedFavoritesChanged


   void UiCtrl::decodedDestRequestOk( uint32 /*src*/, uint32 dst )
   {
      RequestListItem* req = m_pending->dequeue( dst );
      DBG("decodedDestRequestOk dst = 0x%08"PRIx32".", dst);

      if (req != NULL){
         DBG("Got request ok from Destinations module, "
             "dst = 0x%08"PRIx32", guiRequest = 0x%08x", 
             dst, req->request);

         m_pending->deallocate(req);
         GenericGuiMess sfr(GuiProtEnums::SYNC_FAVORITES_REPLY);
         m_guiCtrl->sendMessageToGui(&sfr, req->reqID);
      }
      else{
         WARN("Never sent destination module request with "
              "id = 0x%08"PRIx32", discarding.", dst);
      }
   } // decodedDestRequestOk 

   bool 
   UiCtrl::connectedToGui()
   {
      return m_connectedToGui;
   }

   /* Sent from another part of UiCtrl. Passed directly to AudioCtrl. */
   void UiCtrl::playSoundForCrossing(uint16 crossingNo)
   {
      m_audioCtrl->playSoundForCrossing(crossingNo);
   }

   void UiCtrl::decodedParamSyncReply(GuiProtEnums::WayfinderType wft, 
                                      uint32 src, uint32 dst)
   {
      RequestListItem* req = m_pending->dequeue(dst);
      DBG("decodedParamSyncReply(%d, %#"PRIx32" %#"PRIx32")", 
          int(wft), src, dst);
      if (req != NULL){
         m_pending->deallocate(req);
         GenericGuiMess psr(GuiProtEnums::PARAMETERS_SYNC_REPLY, uint32(wft));
         m_guiCtrl->sendMessageToGui(&psr, req->reqID);
      } else {
         WARN("Dont recognize request id %#"PRIx32", discarding", dst); 
      }
   }

   void UiCtrl::decodedReply( 
      NavPacket::RequestType type, NParamBlock& params, 
      uint8 /*requestVer*/, uint8 /*statusCode*/, const char* /*statusMessage*/,
      uint32 /*src*/, uint32 dst )
   {
      RequestListItem* req = m_pending->dequeue( dst );
      DBG("decodedNavReply dst = 0x%08"PRIx32, dst);

      if (req != NULL){
         DBG("Got ok NavReply, "
             "dst = 0x%08"PRIx32", guiRequest = 0x%08x", 
             dst, req->request);

         m_pending->deallocate(req);

         switch ( type ) {
            case NavPacket::NAV_TUNNEL_DATA_REPLY:
            {
               const NParam* addParam = params.getParam( 5500 );
               const NParam* dataParam = params.getParam( 5501 );

               const uint8* data = 0;
               uint32 size = 0;
               const uint8* ad_data = 0;
               uint32 ad_size = 0;

               if (dataParam) {
                  data = dataParam->getBuff();
                  size = dataParam->getLength();
               }
               if (addParam) {
                  ad_data = addParam->getBuff();
                  ad_size = addParam->getLength();
               }

               m_guiCtrl->handleTunnelDataReply(
                  size, data, ad_size, ad_data, req->reqID);
            }
            break;
            
            default:
               /* Ignore unknown packets. */
            break;
         }
      } else {
         WARN("Never sent NavRequest with id = 0x%08"PRIx32", discarding.", 
              dst);
      }
   }

} //namespace isab

