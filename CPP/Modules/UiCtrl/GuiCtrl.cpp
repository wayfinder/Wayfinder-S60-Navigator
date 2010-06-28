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
#include "ParameterEnums.h"
#include "Parameter.h"

#include "Serial.h"
#include "RequestList.h"

#include "Nav2Error.h"

#include "PacketEnums.h"
#include "RouteEnums.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "NavTask.h"

#include "NavPacket.h"
#include "GuiProt/ServerEnums.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "NavServerCom.h"

#include "GuiProt/HelperEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtRouteMess.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtMapMess.h"
#include "GuiProt/GuiProtFileMess.h"
#include "GuiProt/Favorite.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiRequestFailed.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/SearchRegion.h"
#include "GuiProt/SearchArea.h"
#include "GuiProt/SearchItem.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/FullSearchItem.h"
#include "GuiProt/SearchPrintingPolicyFactory.h"
#include "GuiProt/MapClasses.h"
#include "GuiProt/GuiParameterEnums.h"

#include "Destinations.h"

#include "DistancePrintingPolicy.h"

#include "GuiCtrl.h"

#include "ErrorModule.h"
#include "CtrlHub.h"
#include "UiCtrl.h"

#include "nav2util.h"  

#include "Constants.h"

#include <functional>
#include <deque>
#include <algorithm>
#include <ctype.h>
#include "RoutePositionData.h"

#include "ArchFastMath.h"

#include "TimeUtils.h"
#include <time.h>

/// If defined, no data is sent to the GUI.
//  #define NO_DATA_TO_GUI

/* #include "StringUtility.h" */
#include "LogMacros.h"
#include "UiCtrlInternal.h"
#include "TrackPIN.h"

#include "NavServerComProtoVer.h"
#include "NavRequestType.h"

namespace {

   using namespace std;

   using namespace isab;

   uint32 crowsFlight(int32 lat1, int32 lon1, int32 lat2, int32 lon2)
   {
      if ((lat1 == 0 && lon1 == 0) ||
          (lat1 == MAX_INT32 && lon1 == MAX_INT32) ||
          (lat2 == 0 && lon2 == 0) ||
          (lat2 == MAX_INT32 && lon2 == MAX_INT32)
         ) {
         /* No good coordinates. */
         return MAX_UINT32;
      }
      const double C = 1e8f; 
      double d1 = lat1 / C;
      double d2 = lat2 / C;
      double cd1 = cos(d1);
      double cd2 = cos(d2);
      double sd1 = sin(d1);
      double sd2 = sin(d2);
      double ldiff = (lon1 - lon2) / C;
      double cldiff = cos(ldiff);
      double ca = cd1 * cd2 * cldiff + sd1 * sd2;
      double distance = Constants::earthRadius * acos(ca);
      return uint32(distance);
   }

   class UpdateDistance{
      const int32 m_lat;
      const int32 m_lon;
   public:
      UpdateDistance(int32 lat, int32 lon) : m_lat(lat), m_lon(lon) {}
      typedef void result_type;
      typedef FullSearchItem* argument_type;
      result_type operator()(argument_type arg)
      {
         arg->setDistance(crowsFlight(m_lat, m_lon, 
                                      arg->getLat(), arg->getLon()));
      }
   };
}


namespace isab{
   using namespace RouteEnums;
   using namespace MapEnums;

const general_param_t generalParamArray[] = {
   {  GuiProtEnums::paramCategoryNames,
      GuiProtEnums::paramTypeString,
      ParameterEnums::CategoryNames },
   {  GuiProtEnums::paramCategoryIds,
      GuiProtEnums::paramTypeString,
      ParameterEnums::CategoryIds },
   {  GuiProtEnums::paramCategoryIcons,
      GuiProtEnums::paramTypeString,
      ParameterEnums::CategoryIcons },
   {  GuiProtEnums::paramCategoryIntIds,
      GuiProtEnums::paramTypeBinary,
      ParameterEnums::CategoryIntIds },
   {  GuiProtEnums::paramSearchStrings,
      GuiProtEnums::paramTypeBinary,
      ParameterEnums::UC_GUISearchStrings },
/*    {  GuiProtEnums::paramSearchCountry, */
/*       GuiProtEnums::paramTypeBinary, */
/*       ParameterEnums::UC_GUISearchCountryBlob }, */
/*    {  GuiProtEnums::paramPositionSelectData, */
/*       GuiProtEnums::paramTypeBinary, */
/*       ParameterEnums::UC_GUIPositionSelectDataBlob }, */
/*    {  GuiProtEnums::paramMyDestIndex, */
/*       GuiProtEnums::paramTypeInt32, */
/*       ParameterEnums::UC_GUIMyDestCurrentSelectedIndex }, */
/*    {  GuiProtEnums::paramLastKnownRouteEndPoints, */
/*       GuiProtEnums::paramTypeInt32, */
/*       ParameterEnums::UC_GUILastKnownRouteEndPoints }, */
   {  GuiProtEnums::paramLastKnownRouteId,
      GuiProtEnums::paramTypeBinary,
      ParameterEnums::UC_GUILastKnownRouteId },
   {  GuiProtEnums::paramSelectedAccessPointId2,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::SelectedAccessPointId2 },
   {  GuiProtEnums::paramSelectedAccessPointId,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::SelectedAccessPointIdReal },
   {  GuiProtEnums::paramWayfinderType,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::WayfinderType },
   {  GuiProtEnums::paramLanguage,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::Language },
   {  GuiProtEnums::paramTollRoads,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::NSC_RouteTollRoads },
   {  GuiProtEnums::paramHighways,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::NSC_RouteHighways },
   {  GuiProtEnums::paramTimeDist,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::NSC_RouteCostType },
/*    {  GuiProtEnums::paramTimeLeft, */
/*       GuiProtEnums::paramTypeInt32, */
/*       ParameterEnums::NSC_ExpireVector }, */
/*    {  GuiProtEnums::paramGuiMode, */
/*       GuiProtEnums::paramTypeInt32, */
/*       ParameterEnums::UC_GuiMode }, */
   {  GuiProtEnums::paramFavoriteShow,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_FavoriteShow },
   {  GuiProtEnums::paramGPSAutoConnect,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_GPSAutoConnect },
   {  GuiProtEnums::paramVectorMapSettings,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_VectorMapSettings },
   {  GuiProtEnums::paramPoiCategories,
      GuiProtEnums::paramTypeBinary,
      ParameterEnums::UC_PoiCategories },
   {  GuiProtEnums::paramVectorMapCoordinates,
      GuiProtEnums::paramTypeBinary,
      ParameterEnums::UC_VectorMapCoordinates },
   {  GuiProtEnums::paramAutoTracking,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_AutoTracking },
   {  GuiProtEnums::paramPositionSymbol,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_PositionSymbolType },
/*    {  GuiProtEnums::paramLatestNewsChecksum, */
/*       GuiProtEnums::paramTypeInt32, */
/*       ParameterEnums::NSC_LatestNewsChecksum }, */
/*    {  GuiProtEnums::paramLatestShownNewsChecksum, */
/*       GuiProtEnums::paramTypeInt32, */
/*       ParameterEnums::UC_LatestShownNewsChecksum }, */
   {  GuiProtEnums::paramUserTermsAccepted,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_UserTermsAccepted },
   {  GuiProtEnums::paramLinkLayerKeepAlive,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_LinkLayerKeepAlive },
   {  GuiProtEnums::paramTrackingLevel,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::TR_trackLevel },
   {  GuiProtEnums::paramTrackingPIN,
      GuiProtEnums::paramTypeBinary,
      ParameterEnums::TR_trackPIN },
   {  GuiProtEnums::paramBtGpsAddressAndName,
      GuiProtEnums::paramTypeString,
      ParameterEnums::BtGpsAddressAndName },
   {  GuiProtEnums::paramMapLayerSettings,
      GuiProtEnums::paramTypeBinary,
      ParameterEnums::UC_MapLayerSettings },
   {  GuiProtEnums::userRights,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::NSC_userRights },
   {  GuiProtEnums::userTrafficUpdatePeriod,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::NT_UserTrafficUpdatePeriod }, 

   {  GuiProtEnums::paramHttpServerNameAndPort,
      GuiProtEnums::paramTypeString,
      ParameterEnums::NSC_HttpServerHostname},
   {  GuiProtEnums::paramServerNameAndPort,
      GuiProtEnums::paramTypeString,
      ParameterEnums::NSC_ServerHostname },
   {  GuiProtEnums::paramTransportationType,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::NSC_TransportationType },
   {  GuiProtEnums::paramUserAndPassword,
      GuiProtEnums::paramTypeString,
      ParameterEnums::NSC_UserAndPasswd },
   {  GuiProtEnums::paramAutomaticRouteOnSMSDest,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_AutomaticRouteOnSMSDest },
   {  GuiProtEnums::paramAutoReroute,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_AutoReroute },
   {  GuiProtEnums::paramBacklightStrategy,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_BacklightStrategy },
   {  GuiProtEnums::paramDistanceMode,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_DistanceMode },
   {  GuiProtEnums::paramKeepSMSDestInInbox,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_KeepSMSDestInInbox },
   {  GuiProtEnums::paramMaxNbrOfSearchResults,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_MaxNumberOfGuiSearchResults },
/*    {  GuiProtEnums::paramMuteTurnSound, */
/*       GuiProtEnums::paramTypeInt32, */
/*       ParameterEnums::UC_MuteTurnSounds }, */
   {  GuiProtEnums::paramSoundVolume,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_SoundVolume },
   {  GuiProtEnums::paramStoreSMSDestInMyDest,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_StoreSMSDestInMyDest },
   {  GuiProtEnums::paramTurnSoundsLevel,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_TurnSoundsLevel },
   {  GuiProtEnums::paramUseSpeaker,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_UseMainSpeaker },
   {  GuiProtEnums::paramWebPassword,
      GuiProtEnums::paramTypeString,
      ParameterEnums::UC_WebPasswd },
   {  GuiProtEnums::paramWebUsername,
      GuiProtEnums::paramTypeString,
      ParameterEnums::UC_WebUser },
   {  GuiProtEnums::paramTopRegionList,
      GuiProtEnums::paramTypeBinary,
      ParameterEnums::TopRegionList },
   {  GuiProtEnums::paramShowNewsServerString,
      GuiProtEnums::paramTypeString,
      ParameterEnums::NSC_latestNewsId },
   {  GuiProtEnums::paramShownNewsChecksum,
      GuiProtEnums::paramTypeString,
      ParameterEnums::UC_latestNewsId },
   {  GuiProtEnums::paramNeverShowUSDisclaimer,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_neverShowUSDisclaimer },
   {  GuiProtEnums::paramRegistrationSmsSent,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_registrationSmsSent },
   {  GuiProtEnums::paramMapACPSetting,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_MapACPSetting },
   {  GuiProtEnums::paramCheckForUpdates,
      GuiProtEnums::paramTypeInt32,
      ParameterEnums::UC_CheckForUpdates },
   {  GuiProtEnums::paramNewVersion,
      GuiProtEnums::paramTypeString,
      ParameterEnums::NSC_newVersion },
   {  GuiProtEnums::paramNewVersionUrl,
      GuiProtEnums::paramTypeString,
      ParameterEnums::NSC_newVersionUrl },

};


GuiCtrl::GuiCtrl(UiCtrl* uiCtrl,
                 NavServerComProviderPublic* navServComProvPubl,
                 NavTaskProviderPublic* navTaskProvPubl,
                 SerialProviderPublic* serialProvPubl,
                 DestinationsProviderPublic* destProvPubl, 
                 ParameterProviderPublic* paramProvPubl,
                 ParameterEnums::ParamIds serverListParam,
                 bool useIAPProxy,
                 const char* hardIapProxyHost,
                 uint16 hardIapProxyPort,
                 const char* userClipString) :
   m_uiCtrl(uiCtrl), 
   //provider public objects
   m_nscProvPubl(navServComProvPubl), m_ntProvPubl(navTaskProvPubl), 
   m_serialProvPubl(serialProvPubl), m_destProvPubl(destProvPubl), 
   m_paramProvider(paramProvPubl), 
   //state vars
   m_nextGuiMessToSendId(0), m_currRouteData(NULL), m_hasAdditionalInfo(false),
   m_numberOfFirstSearchItem(0), m_totalnumberOfSearchItems(0),
   m_serverListParam( serverListParam ),
   m_useIAPProxy( useIAPProxy ),
   m_hardIapProxyHost( hardIapProxyHost ),
   m_hardIapProxyPort( hardIapProxyPort ),
   //cached parameters.
   m_paramMaxNbrSearchMatches(0xff), //invalid
   m_distanceMode(DistancePrintingPolicy::ModeInvalid), //Invalid value.
   m_webUsername(NULL),  //invalid
   m_checkForUpdates(1),  // default is DO check for updates
   m_lastLat(MAX_INT32), //invalid
   m_lastLon(MAX_INT32), //invalid
   m_lastHeading(0),
   m_lastSpeed(0), 
   m_webUsernameClipString(userClipString),
   m_refinedSearch(navServComProvPubl) //basic setup
{
   m_log = new Log("GuiCtrl");
   // Cached parameters.
/*    m_topRegionList = new TopRegionList(); // Valid empty parameter list. */
   m_messagesToSendToGui = new deque<Buffer*>();

   DBG("useIAPProxy:      %s", useIAPProxy ? "yes" : "no");
   DBG("hardIapProxyHost: %s", hardIapProxyHost);
   DBG("hardIapProxyPort: %"PRIu16, hardIapProxyPort);
   DBG("userClipString:   %s", userClipString);
}

GuiCtrl::~GuiCtrl()
{
   delete m_log;
 
   delete m_currRouteData;

   delete[] m_webUsername;

   for (uint32 i = 0; i < m_messagesToSendToGui->size(); i++){
      delete (*m_messagesToSendToGui)[i];
   }
   delete m_messagesToSendToGui;

   std::for_each(m_searchMatches.begin(), m_searchMatches.end(), 
                 Delete<FullSearchItem*>());
   std::for_each(m_searchAreas.begin(), m_searchAreas.end(), 
                 Delete<SearchArea*>());
}

void 
GuiCtrl::messageReceived(const GuiProtMess* guiMessage)
{
   DBG("messageReceived");

   GuiProtEnums::MessageType messageType = guiMessage->getMessageType();
   DBG("messageReceived: id: %d, type: %d", guiMessage->getMessageID(), 
       messageType);
   switch (messageType){
   case GuiProtEnums::GET_GENERAL_PARAMETER:
      {
         DBG("Got GET_GENERAL_PARAMETER from GUI.");
         handleGetGeneralParameter(
            static_cast<const GeneralParameterMess*>(guiMessage));
      } break;
   case GuiProtEnums::CHANGE_UIN: 
      {
         DBG("Got CHANGE_UIN from GUI.");
         handleChangeUIN(
            static_cast<const GenericGuiMess*>(guiMessage));
      } break;
   case GuiProtEnums::SET_GENERAL_PARAMETER:
      {
         DBG("Got SET_GENERAL_PARAMETER from GUI.");
         handleSetGeneralParameter(
            static_cast<const GeneralParameterMess*>(guiMessage));
      } break;
   case GuiProtEnums::GET_FAVORITES:
      {  
         DBG("Got GET_FAVORITES from GUI."); 
         handleGetFavorites(
            static_cast<const GenericGuiMess*>(guiMessage));
      }break;
   case GuiProtEnums::GET_FAVORITES_ALL_DATA:
      {  
         DBG("Got GET_FAVORITES_ALL_DATA from GUI."); 
         handleGetFavoritesAllData(
            static_cast<const GenericGuiMess*>(guiMessage));         
      }break;
   case GuiProtEnums::SORT_FAVORITES:
      {  
         DBG("Got SORT_FAVORITES from GUI."); 
         handleSortFavorites(
            static_cast<const GenericGuiMess*>(guiMessage));         
      }break;
   case GuiProtEnums::SYNC_FAVORITES:
      {  
         DBG("Got SYNC_FAVORITES from GUI."); 
         handleSyncFavorites(
            static_cast<const GenericGuiMess*>(guiMessage));         
      }break;
   case GuiProtEnums::GET_FAVORITE_INFO:
      {  
         DBG("Got GET_FAVORITE_INFO from GUI."); 
         handleGetFavoriteInfo(
            static_cast<const GenericGuiMess*>(guiMessage));         
      }break;
   case GuiProtEnums::ADD_FAVORITE:
      {  
         DBG("Got ADD_FAVORITE from GUI."); 
         handleAddFavorite(
            static_cast<const AddFavoriteMess*>(guiMessage));           
      }break;
   case GuiProtEnums::ADD_FAVORITE_FROM_SEARCH:
      {  
         DBG("Got ADD_FAVORITE_FROM_SEARCH from GUI."); 
         handleAddFavoriteFromSearch(
            static_cast<const GenericGuiMess*>(guiMessage));         
      }break;
   case GuiProtEnums::REMOVE_FAVORITE:
      {  
         DBG("Got REMOVE_FAVORITE from GUI."); 
         handleRemoveFavorite(
            static_cast<const GenericGuiMess*>(guiMessage));         
      }break;
   case GuiProtEnums::CHANGE_FAVORITE:
      {  
         DBG("Got CHANGE_FAVORITE from GUI."); 
         handleChangeFavorite(
            static_cast<const ChangeFavoriteMess*>(guiMessage));   
      }break;
   case GuiProtEnums::CONNECT_GPS:
      {
         DBG("Got CONNECT_GPS from GUI.");
         handleGpsConnection(
               static_cast<const GenericGuiMess*>(guiMessage));
         m_uiCtrl->setcalledGPSConnectionMode( 
            UiCtrl::ConnectToGPSCalled );
      } break;
   case GuiProtEnums::DISCONNECT_GPS:
      {
         DBG("Got DISCONNECT_GPS from GUI.");
         handleGpsConnection(
               static_cast<const GenericGuiMess*>(guiMessage));
         m_uiCtrl->setcalledGPSConnectionMode( 
            UiCtrl::DisconnectToGPSCalled );
      } break;
   case GuiProtEnums::GET_MAP:
      DBG("Got GET_MAP from GUI");
      handleMapRequest(static_cast<const GetMapMess*>(guiMessage));
      break;
   case GuiProtEnums::ROUTE_TO_FAVORITE:
      {
         DBG("Got ROUTE_TO_FAVORITE from GUI.");
         handleRouteToFavorite(
            static_cast<const GenericGuiMess*>(guiMessage));
      } break;
   case GuiProtEnums::ROUTE_TO_POSITION:
      DBG("Got ROUTE_TO_POSITION fromGUI.");
      handleRouteToPositionRequest(static_cast<const RouteToPositionMess*>(guiMessage));
      break;
   case GuiProtEnums::ROUTE_TO_SEARCH_ITEM:
      DBG("Got ROUTE_TO_SEARCH_ITEM from GUI.");
      handleRouteToSearchItemRequest(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::ROUTE_MESSAGE:
      DBG("Got ROUTE_MESSAGE from GUI.");
      handleRoute(static_cast<const RouteMess*>(guiMessage));
      break;
   case GuiProtEnums::REROUTE:
      DBG("Got REROUTE from GUI.");
      handleReroute(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::REQUEST_CROSSING_SOUND:
      DBG("Got REQUEST_CROSSING_SOUND from GUI.");
      handleRequestCrossingSound(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::GET_FILTERED_ROUTE_LIST:
      DBG("Got GET_FILTERED_ROUTE_LIST from GUI.");
      handleGetfilteredRouteList(static_cast<const GetFilteredRouteListMess*>(guiMessage));
      break;
   case GuiProtEnums::SEARCH:
      DBG("Got SEARCH from GUI.");
      handleSearchRequest(static_cast<const SearchMess*>(guiMessage));
      break;
   case GuiProtEnums::GET_SEARCH_AREAS:
      DBG("Got GET_SEARCH_AREAS from GUI.");
      handleSearchAreas(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::GET_SEARCH_ITEMS:
      DBG("Got GET_SEARCH_ITEMS from GUI.");
      handleSearchItems(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::GET_FULL_SEARCH_DATA:
      DBG("Got GET_FULL_SEARCH_DATA from GUI");
      handleFullSearchData(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID:
      DBG("Got GET_FULL_SEARCH_DATA_FROM_ITEMID from GUI");
      handleFullSearchDataFromItemId(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::SEND_MESSAGE:
      DBG("Got SEND_MESSAGE from GUI");
      handleMessageRequest(static_cast<const SendMessageMess*>(guiMessage));
      break;
   case GuiProtEnums::GET_MORE_SEARCH_DATA:
      DBG("Got GET_MORE_SEARCH_DATA");
      handleGetMoreSearchData(static_cast<const GetMoreDataMess*>(guiMessage));
      break;
   case GuiProtEnums::REQUEST_LICENSE_UPGRADE:
      DBG("Got LICENSE_UPGRADE");
      handleLicenseKey(static_cast<const LicenseKeyMess*>(guiMessage));
      break;
   case GuiProtEnums::PARAMETERS_SYNC:
      DBG("Got PARAMETERS_SYNC");
      handleParameterSync(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::CELL_INFO_TO_SERVER:
      DBG("Got CELL_INFO_TO_SERVER");
      handleCellReport(static_cast<const DataGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::GET_VECTOR_MAP:
      DBG("Got GET_VECTOR_MAP");
      handleVectorMapRequest(static_cast<const GenericGuiMess*>(guiMessage));
      break;      
   case GuiProtEnums::GET_MULTI_VECTOR_MAP:
      DBG("Got GET_MULTI_VECTOR_MAP");
      handleMultiVectorMapRequest(static_cast<const DataGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::GUI_TO_NGP_DATA:
      DBG("Got GUI_TO_NGP_DATA");
      handleNGPTunnelDataRequest(static_cast<const DataGuiMess*>(guiMessage));
      break;      
   case GuiProtEnums::GUI_TUNNEL_DATA:
      DBG("Got GUI_TUNNEL_DATA");
      handleTunnelDataRequest(static_cast<const DataGuiMess*>(guiMessage));
      break;      
   case GuiProtEnums::INVALIDATE_ROUTE:
      DBG("Got INVALIDATE_ROUTE");
      handleInvalidateRoute(static_cast<const GenericGuiMess*>(guiMessage));
      break;
   case GuiProtEnums::ROUTE_TO_POSITION_CANCEL:
      DBG("Got ROUTE_TO_POSITION_CANCEL");
      handleRouteToPositionCancel(static_cast<const GenericGuiMess*>(guiMessage));
      break;
      // dealt with in AudioCtrl
   case GuiProtEnums::PREPARE_SOUNDS_REPLY:
   case GuiProtEnums::PLAY_SOUNDS_REPLY:
      WARN("Got message of type %#"PRIx32", should go to AudioCtrl.", 
           uint32(messageType));
      break;
   case GuiProtEnums::FILEOP_GUI_MESSAGE:
      handleFileOpMessage(static_cast<const GuiProtFileMess*>(guiMessage));
      break;
      /* Unused messages. */
   case GuiProtEnums::ROUTE_TO_HOT_DEST:
   case GuiProtEnums::GET_TOP_REGION_LIST:
      break;
     //Mesages going the other way.
   case GuiProtEnums::MESSAGETYPE_ERROR:
   case GuiProtEnums::FAVORITES_CHANGED:
   case GuiProtEnums::GET_FAVORITES_ALL_DATA_REPLY:
   case GuiProtEnums::GET_FAVORITES_REPLY:
   case GuiProtEnums::GET_FAVORITE_INFO_REPLY:
   case GuiProtEnums::GET_MAP_REPLY:
   case GuiProtEnums::GET_SEARCH_AREAS_REPLY:
   case GuiProtEnums::GET_SEARCH_ITEMS_REPLY:
   case GuiProtEnums::GET_TOP_REGION_LIST_REPLY:
   case GuiProtEnums::PANIC_ABORT:
   case GuiProtEnums::PARAMETER_CHANGED:
   case GuiProtEnums::PLAY_SOUNDS:
   case GuiProtEnums::PREPARE_SOUNDS:
   case GuiProtEnums::SOUND_FILE_LIST:
   case GuiProtEnums::LOAD_AUDIO_SYNTAX:
   case GuiProtEnums::PROGRESS_INDICATOR:
   case GuiProtEnums::REQUEST_FAILED:
   case GuiProtEnums::SEARCH_RESULT_CHANGED:
   case GuiProtEnums::GET_FULL_SEARCH_DATA_REPLY:
   case GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID_REPLY:
   case GuiProtEnums::STARTED_NEW_ROUTE:
   case GuiProtEnums::UPDATE_POSITION_INFO:
   case GuiProtEnums::UPDATE_ROUTE_INFO:
   case GuiProtEnums::ROUTE_LIST:
   case GuiProtEnums::LICENSE_UPGRADE_REPLY:
   case GuiProtEnums::SEND_MESSAGE_REPLY:
   case GuiProtEnums::PARAMETERS_SYNC_REPLY:
   case GuiProtEnums::SYNC_FAVORITES_REPLY:
   case GuiProtEnums::CELL_INFO_FROM_SERVER:
   case GuiProtEnums::GET_VECTOR_MAP_REPLY:
   case GuiProtEnums::GUI_TUNNEL_DATA_REPLY:
   case GuiProtEnums::GUI_TO_NGP_DATA_REPLY:
   case GuiProtEnums::GET_MULTI_VECTOR_MAP_REPLY:
   case GuiProtEnums::FORCEFEED_MULTI_VECTOR_MAP:
   case GuiProtEnums::FORCEFEED_MULTI_VECTOR_MAP_REPLY:
   case GuiProtEnums::SATELLITE_INFO:
      ERR("Got message of type %#"PRIx32", should be going the other way", 
          uint32(messageType));
      break;
   } // switch
   DBG("messageReceived end");

}

void 
GuiCtrl::setMaxNbrSearchMatches(int32 nbrOfSearchMatches)
{
   DBG("setMaxNbrSearchMatches");
   m_paramMaxNbrSearchMatches = nbrOfSearchMatches;
}

void 
GuiCtrl::setDistanceMode(DistancePrintingPolicy::DistanceMode distMode)
{
   m_distanceMode = distMode;
}

void 
GuiCtrl::setCheckForUpdates(uint8 aValue)
{
   m_checkForUpdates = aValue;
}


void 
GuiCtrl::sendPendingMessagesToGui()
{
   DBG("sendPendingMessagesToGui");
   while(!m_messagesToSendToGui->empty()){
      Buffer* message = m_messagesToSendToGui->front();
      // The message buffer is deleted in here.
      DBG("Sends pending message to gui.");
      m_serialProvPubl->sendData( message );
      m_messagesToSendToGui->pop_front();
   }
}

void GuiCtrl::handleSearchRequest(const SearchMess* searchMess)
{
   if ( m_paramMaxNbrSearchMatches == MAX_UINT8 ){
      // maxNbrSearchMatches param is not yet been received.
      ERR("maxNbrSearchMatches param has not yet been received");
      //MN: Notify the GUI with an error.
   }
   else{
      uint32 topRegion = searchMess->getTopRegion();
      const char* area = searchMess->getAreaQuery();
      const char* item = searchMess->getItemQuery();
      const char* id   = searchMess->getAreaID();
      int32 origLat = searchMess->getOrigLat();
      int32 origLon = searchMess->getOrigLon();
      DBG("Indata to search:\n areaId: '%s', Area: '%s', Item: '%s', "
          "countryID %"PRIu32, id, area, item, topRegion);
   
      // Check if a selection of the city was made from the listbox.
      SearchAreaContainer::iterator areaIt = m_searchAreas.end();
      if(searchMess->areaSelected()){
         areaIt = find_if(m_searchAreas.begin(), m_searchAreas.end(),
                          MatchString<SearchArea>(id, &SearchArea::getID));
         if(!(areaIt != m_searchAreas.end())){
            id = NULL;
         }
      }
      RequestListItem* req = 
         m_uiCtrl->checkForRequestAndCreate(searchMess->getMessageType(), 
                                            searchMess->getMessageID());
      uint32 nav2MessageId = 0;
      
      if ( req == NULL ) return;
      
      DBG( "Search request id %#0"PRIx32, req->src );
      if(areaIt != m_searchAreas.end()){
         nav2MessageId = m_refinedSearch.request(item, (*areaIt)->getID(), 
                                                 topRegion, origLat, origLon);
         DBG("refinedSearch('%s', '%s', %"PRIu32", %"PRId32", %"PRId32")", item,
             (*areaIt)->getID(), topRegion, origLat, origLon);
      } else {
         nav2MessageId = m_refinedSearch.firstSearch(item, area, topRegion,
                                                     origLat, origLon);
         DBG("originalSearch('%s', '%s', %u, %"PRIu32")",area, item, 0, topRegion);
      }

      m_uiCtrl->addRequest(nav2MessageId, req);

   }
}


void GuiCtrl::handleMapRequest(const GetMapMess* mapMess)
{
   RequestListItem* req = 
      m_uiCtrl->checkForRequestAndCreate(mapMess->getMessageType(), 
                                         mapMess->getMessageID());
   uint32 nav2MessageId = 0;
   
   if ( req == NULL ) return;
   DBG( "Search request id %#0"PRIx32, req->src );
   
   Buffer mapReq(128);
   mapMess->serialize(&mapReq);
   mapReq.setReadPos(8); //skip navui header.
//    DBGDUMP("Map request as sent to NSC", mapReq.accessRawData(),
//            mapReq.remaining());
   nav2MessageId = m_nscProvPubl->mapRequest(mapReq.accessRawData(), 
                                             mapReq.remaining());
   req->request = GuiProtEnums::GET_MAP;
   m_uiCtrl->addRequest(nav2MessageId, req);
}

void GuiCtrl::handleMapReply(const BoxBox& bb, uint16 imageWidth, 
                             uint16 imageHeight, uint32 realWorldWidth,
                             uint32 realWorldHeight, enum ImageFormat format,
                             unsigned datasize, const uint8* data, 
                             uint16 reqId)
{
   MapReplyMess mrm(bb.getTopLat(), bb.getLeftLon(), bb.getBottomLat(), 
                bb.getRightLon(), imageWidth, imageHeight, realWorldWidth,
                realWorldHeight, format, datasize, data);
   sendMessageToGui(&mrm, reqId);
}


void GuiCtrl::copyItems(uint32 nItems, const FullSearchItem* const* fsi,
                        uint16 begindex, uint16 total)
{
   DBG("Items changed! Deleting %u old areas.", m_searchMatches.size());
   m_hasAdditionalInfo = false;
   std::for_each(m_searchMatches.begin(), m_searchMatches.end(), 
                 Delete<FullSearchItem*>());
   DBG("Resizing item vector to %"PRIu32, nItems);
   m_searchMatches.resize(nItems, 0);
   DBG("Cloning %"PRIu32" FullSearchItems to vector", nItems);
   std::transform(fsi, fsi + nItems, m_searchMatches.begin(), 
                  Clone<FullSearchItem>());
   m_numberOfFirstSearchItem  = begindex;
   m_totalnumberOfSearchItems = total;
}

void GuiCtrl::sendItemsToGui(uint32 begindex, uint32 endex, uint16 reqId)
{
   DBG("sendItemsToGui");
   SearchItemContainer::iterator last, first = last = m_searchMatches.begin();
   std::advance(first, min(max(begindex - m_numberOfFirstSearchItem, 
                               uint32(0)),
                           uint32(m_searchMatches.size())));
   std::advance(last,  min(max(endex - m_numberOfFirstSearchItem, 
                               uint32(0)),
                           uint32(m_searchMatches.size())));
   std::for_each(first, last, UpdateDistance(m_lastLat, m_lastLon));
   std::vector<SearchItem*> si(std::distance(first, last));
   // We MUST copy the objects, or they will be serialized as
   // FullSearchItems, which is fatal.
   std::transform(first, last, si.begin(), Clone<SearchItem>());
   SearchItemReplyMess moreItems(&(si.front()), si.size(), 
                                 m_totalnumberOfSearchItems,
                                 m_numberOfFirstSearchItem);
   sendMessageToGui(&moreItems, reqId);
   moreItems.deleteMembers();
}

void GuiCtrl::handleSearchReply( uint32 nAreas, 
                                 const SearchArea* const* sa,
                                 uint32 nItems, 
                                 const FullSearchItem* const* fsi
                                 ,uint16 begindex, uint16 total,
                                 GuiProtEnums::MessageType type, uint16 id
                                 )
{
   DBG("handleSearchReply(%"PRIu32", (SearchArea**)%p, "
       "%"PRIu32", (FullSearchItem**)%p",
       nAreas, sa, nItems, fsi);
   if(type == GuiProtEnums::SEARCH){
      // Calculate if areas and or items changed. Areas are considered
      // changed if a) more than 1 area was returned, b) 1 area was
      // returned, and it is not one we already have. 
      bool areasChanged = nAreas > 1 || (m_searchAreas.empty() && nAreas != 0);
      if(!areasChanged && nAreas > 0){
         SearchAreaContainer::iterator result;
         result = std::find_if(m_searchAreas.begin(), m_searchAreas.end(), 
                               std::bind1st(PointerOperator<SearchArea, 
                                            std::equal_to<SearchArea> >(), 
                                            sa[0]));
         areasChanged = (result == m_searchAreas.end());
      }
      // items are considered changed if 1 or more item was returned.
      bool itemsChanged = nItems > 0;
      
      //delete old areas and copy the new ones.
      if(areasChanged){
         DBG("Areas changed! Deleting %u old areas.", m_searchAreas.size());
         std::for_each(m_searchAreas.begin(), m_searchAreas.end(), 
                       Delete<SearchArea*>());
         DBG("Resizing area vector to %"PRIu32, nAreas);
         m_searchAreas.resize(nAreas, (SearchArea*)NULL);
         DBG("Cloning %"PRIu32" SearchAreas to vector", nAreas);
         std::transform(sa, sa + nAreas, m_searchAreas.begin(), 
                        Clone<SearchArea>());
      }
      //delete old items and copy the new ones.
      if(itemsChanged){
         copyItems(nItems, fsi, begindex, total);
      }
      //send message to gui,
      GenericGuiMess change(GuiProtEnums::SEARCH_RESULT_CHANGED, 
                            areasChanged, itemsChanged);
      sendMessageToGui(&change, id);
   } else if(type == GuiProtEnums::GET_SEARCH_ITEMS){
      copyItems(nItems, fsi, begindex, total);
      sendItemsToGui(begindex, begindex + nItems, id);
   } else {
      handleRequestFailed(Nav2Error::UC_CONFUSED, type, id);
   }
}

void GuiCtrl::handleAdditionalInfo(uint16 num, const char*const* id,
                                   const unsigned* index, unsigned numInfo,
                                   const AdditionalInfo* const* info,
                                   RequestListItem* req)
{
   FSIrequest* freq = static_cast<FSIrequest*>(req->data);
   GenericGuiMess* mess = freq->message;

   if ( freq->type == FSIIndexRange || freq->type == FSISaveAsFav ) {
      //for each given id...
      for(int i = 0; i < num; ++i){
         SearchItemContainer::iterator item;
         //... find the matching item ...
         item = find_if(m_searchMatches.begin(), m_searchMatches.end(),
               MatchString<FullSearchItem>(id[i], 
                  &FullSearchItem::getID));
         if(item != m_searchMatches.end()){
            //the end index of the info vector.
            int end = (i+1) < num ? index[i+1] : numInfo;
            //... and add the info items.
#ifndef NO_LOG_OUTPUT
            DBG("-------------");
            DBG("SearchItem '%s' named '%s' has %u AIs.", (*item)->getID(), 
                  (*item)->getName(), (*item)->noAdditionalInfo());
            for(unsigned q = 0; q < (*item)->noAdditionalInfo(); ++q){
               DBG(" AI %u: '%s' : '%s'", q, (*item)->getInfo(q)->getKey(),
                     (*item)->getInfo(q)->getValue());
            }
#endif
            for(int j = index[i]; j < end; ++j){
               (**item).addInfo(*(info[j]));
            }
#ifndef NO_LOG_OUTPUT
            DBG("SearchItem '%s' named '%s' has %u AIs.", (*item)->getID(), 
                  (*item)->getName(), (*item)->noAdditionalInfo());
            for(unsigned w = 0; w < (*item)->noAdditionalInfo(); ++w){
               DBG(" AI %u: '%s' : '%s'", w, (*item)->getInfo(w)->getKey(),
                     (*item)->getInfo(w)->getValue());
            }
            DBG("-------------");
#endif
         }
      }
      m_hasAdditionalInfo = true;

      /* Index range sent from GUI. */
      /* Handle by emulating a new request for the same */
      /* range, which now is cached. */

      if ( freq->type == FSIIndexRange ) {
         handleFullSearchData( mess );
      } else {
         handleAddFavoriteFromSearch( mess );
      }
   } else if (freq->type == FSISingleItemId) {
      /* Return additional info in a general parameter packet. */
      FullSearchItem* fsi = new FullSearchItem(
            mess->getSecondString(), mess->getFirstString(),
            0, 0, 0,
            NULL, 0,
            MAX_INT32, MAX_INT32, 0,
            0,
            info, numInfo);

/*    (uint16 num, const char*const* id, */
/*    const unsigned* index, unsigned numInfo, */
/*    const AdditionalInfo* const* info, */
/*    RequestListItem* req) */

      FullSearchDataFromItemIdReplyMess fullInfo(&fsi, 1);
      sendMessageToGui(&fullInfo, mess->getMessageID());
      delete fsi;
   }
   delete freq;
   req->data = NULL;
   mess->deleteMembers();
   delete mess;
}

void
GuiCtrl::handleLatestNews(const uint8 *data,
                          unsigned length,
                          uint32 checksum)
{
   DBG("handleLatestNews");
   /* Fake a GeneralParameterMessage for image and one for checksum. */
   uint8 *d = (uint8*)data;
   DBG("Sending image length %d", length);
   GeneralParameterMess *mess = 
      new GeneralParameterMess(GuiProtEnums::paramLatestNewsImage, d, length);
   sendMessageToGui(mess, 0);
   delete mess;
}

uint16 
GuiCtrl::sendMessageToGui(GuiProtMess* message, uint16 requestId)
{
   DBG("sendMessageToGui");
#ifndef NO_DATA_TO_GUI
   Buffer* serializedMessage = new Buffer(m_normalGuiPacketSize);
   if(!(requestId & 0x01)){ //all even IDs come from Nav2
      requestId = (m_nextGuiMessToSendId += 2);
   }
   DBG("Sending %x message with reqid %u to GUI", 
       unsigned(message->getMessageType()), requestId);
   message->setMessageId(requestId);
   message->serialize(serializedMessage);


   if ( m_uiCtrl->connectedToGui() ){
      
//       DBGDUMP("Message data sent to GUI (prot 3):", 
//                 serializedMessage->accessRawData(),
//                 serializedMessage->getLength());
    
      // serializedMessage is deleted inside.

      m_serialProvPubl->sendData(serializedMessage);
   }
   else{
      WARN("Gui not connected, adding to waiting container.");
      m_messagesToSendToGui->push_back(serializedMessage);
   }
#else
   requestId = 0;
#endif
   return requestId;
}

void
GuiCtrl::sendParameterChangedToGui(const GuiProtEnums::ParameterTypeHolder& paramType)
{
   DBG("sendParameterChangedToGui");
   GenericGuiMess parameterChangedMess( GuiProtEnums::PARAMETER_CHANGED,
                                        static_cast<uint16>(paramType));
   sendMessageToGui(&parameterChangedMess, 0);
   DBG("Sent PARAMETER_CHANGED, paramType=0x08%x to GUI.", (uint16)paramType);
} // sendParameterChangedToGui

void GuiCtrl::handleRouteToPositionRequest(const RouteToPositionMess* routeMess)
{
   const char* coordDestName = routeMess->getDestinationName();
   if (coordDestName == NULL){
      coordDestName = "";
   }
   INFO("Routing to coordinate. Name used for goal = \"%s\"", coordDestName);

   delete m_currRouteData;
   m_currRouteData = new RoutePositionData(coordDestName,
                                           MAX_INT32, MAX_INT32,
                                           routeMess->getLat(), 
                                           routeMess->getLon());
   startRoute(m_currRouteData,
              routeMess->getMessageType(), 
              routeMess->getMessageID());
}

int
GuiCtrl::getSearchPosition(char *searchId, int32& lat, int32& lon)
{
   if (searchId == NULL){
      return -1;
   }

   SearchItemContainer::iterator res;
   res= find_if(m_searchMatches.begin(), m_searchMatches.end(), 
         MatchString<SearchItem>(searchId, &SearchItem::getID));
   if(res != m_searchMatches.end()){
      /* Found the item in the search match vector. */
      lat = (*res)->getLat();
      lon = (*res)->getLon();
   } else {
      /* Didn't find the item to route to in the */
      /* search match list. This means we cannot route */
      /* to it either. */
      ERR("Unknown item id!");
      return -1;
   }
   return 0;
}

void GuiCtrl::handleRoute(const RouteMess* routeMess)
{
   delete m_currRouteData;
   m_currRouteData = new RoutePositionData();
   RoutePositionData* rpd = m_currRouteData;
   rpd->setoType(routeMess->getoType());
   rpd->setdType(routeMess->getdType());
   rpd->setoLat(routeMess->getoLat());
   rpd->setoLon(routeMess->getoLon());
   rpd->setdLat(routeMess->getdLat());
   rpd->setdLon(routeMess->getdLon());
   rpd->setoId(routeMess->getoId());
   rpd->setdId(routeMess->getdId());

   if(routeMess->getdType() == GuiProtEnums::PositionTypeSearch){
      SearchItemContainer::iterator si =
         std::find_if(m_searchMatches.begin(), m_searchMatches.end(), 
                      MatchString<FullSearchItem>(routeMess->getdId(),
                                                  &SearchItem::getID));
      char* longDest = NULL;
      if(si != m_searchMatches.end()){
         FullSearchItem& fsi = **si;
         size_t len = strlen(fsi.getName()) + 1;
         const SearchRegion* sr = fsi.getRegionOfType(GuiProtEnums::address);
         if(sr){
            len += strlen(sr->getName()) + 2;
         }
         longDest = new char[len];
         strcpy(longDest, fsi.getName());
         if(sr){
            strcat(longDest, ", ");
            strcat(longDest, sr->getName());
         }

         rpd->setdLat(fsi.getLat());
         rpd->setdLon(fsi.getLon());
         rpd->setdPositionOk(true);
      } else {
         longDest = strdup_new(routeMess->getDestinationName());
         if(rpd->getdLat() != MAX_INT32 && rpd->getdLon() != MAX_INT32){
            rpd->setdPositionOk(true);
         }
      }
      rpd->setdName(longDest);
      delete[] longDest;

   } else {
      rpd->setdName(routeMess->getDestinationName());
   }

   if(routeMess->getoType() == GuiProtEnums::PositionTypeSearch){
      SearchItemContainer::iterator si =
         std::find_if(m_searchMatches.begin(), m_searchMatches.end(), 
                      MatchString<FullSearchItem>(routeMess->getoId(),
                                                  &SearchItem::getID));
      if(si != m_searchMatches.end()){
         rpd->setoLat((**si).getLat());
         rpd->setoLon((**si).getLon());
         rpd->setoPositionOk(true);
      } else if(rpd->getoLat() != MAX_INT32 && rpd->getoLon() != MAX_INT32){
         rpd->setoPositionOk(true);
      }
   }
   rpd->m_req_type = routeMess->getMessageType();
   rpd->m_req_id = routeMess->getMessageID();


   /* Set route data. */
   checkAndSendRouteData(rpd);
}

void
GuiCtrl::checkAndSetRouteData(RoutePositionData* rpd,
            const char *name, int32 lat, int32 lon)
{
   if (!rpd->getoPositionOk()) {
      /* Enter data in origin. */
      rpd->setoName(name);
      rpd->setoLat(lat);
      rpd->setoLon(lon);
      rpd->setoPositionOk(1);
   } else if (!rpd->getdPositionOk()) {
      /* Enter data in destination. */
      rpd->setdName(name);
      rpd->setdLat(lat);
      rpd->setdLon(lon);
      rpd->setdPositionOk(1);
   } else {
      /* This should never happen. */
      ERR("checkAndSetRouteData but route data was already set!");
   }
}

void GuiCtrl::resetFavoriteToPosition(RoutePositionData* rpd)
{
   if(!rpd->getoPositionOk()){
      rpd->setoType(GuiProtEnums::PositionTypePosition);
   } else if(!rpd->getdPositionOk()){
      rpd->setdType(GuiProtEnums::PositionTypePosition);
   } else {
      /* This should never happen. */
      ERR("resetFavoriteToPosition but route data was already set!");
   }      
   
}

void
GuiCtrl::checkAndSendRouteData(RoutePositionData* rpd)
{
   if (!rpd->getoPositionOk()) {
      /* Check origin first. */
      if (checkPositionData(rpd->m_origin, rpd->m_req_type, rpd->m_req_id)) {
         /* The position type needed additional sub-requests */
         /* i.e. PositionTypeFavorite. */
         /* The rest of the processing will be done when the */
         /* sub-request is answered. */
         return;
      }
      /* The data did not require sending sub-requests. */
   }
   if (!rpd->getdPositionOk()) {
      /* Check destination. */
      if (checkPositionData(rpd->m_dest, rpd->m_req_type, rpd->m_req_id)) {
         /* The position type needed additional sub-requests */
         /* i.e. PositionTypeFavorite. */
         /* The rest of the processing will be done when the */
         /* sub-request is answered. */
         return;
      }
      /* The data did not require sending sub-requests. */
   }
   if (rpd->getoPositionOk() && rpd->getdPositionOk()) {
      /* All data available, send request. */

      RequestListItem* req = 
         m_uiCtrl->checkForRequestAndCreate(rpd->m_req_type, rpd->m_req_id);
      if ( req == NULL ){
         WARN("GuiCtrl. Did not send start route. "
              "Something wrong with the requests.");
         handleRequestFailed(Nav2Error::UC_CONFUSED,
               rpd->m_req_type, rpd->m_req_id);
      } else{
         // For now decodedInvalidateRoute is depending on 
         // this variable being set like this.
         req->status = 0;

         uint32 nav2MessageId = m_ntProvPubl->ntRouteToGps(
               rpd->m_dest->m_lat, rpd->m_dest->m_lon,
               rpd->m_origin->m_lat, rpd->m_origin->m_lon);
         
         DBG( "Route request id %#0"PRIx32, nav2MessageId );
         DBG( "Org  = Lat: %"PRId32", Lon: %"PRId32,
               rpd->m_origin->m_lat, rpd->m_origin->m_lon);
         DBG( "Dest = Lat: %"PRId32", Lon: %"PRId32,
               rpd->m_dest->m_lat, rpd->m_dest->m_lon);

         m_uiCtrl->addRequest(nav2MessageId, req);
      }
      /* Remove the data used to create the route request. */
   } else {
      /* This means that the data was corrupted. */
      /* Report failure to user. */
      handleRequestFailed(Nav2Error::UC_CONFUSED, 
                          rpd->m_req_type,
                          rpd->m_req_id);
   }
}

uint32
GuiCtrl::favoriteIdFromString(char *str_id)
{
   uint32 retval = strtoul(str_id, NULL, 10);
   return retval;
}

int
GuiCtrl::checkPositionData(PositionObject *po, uint16 req_type, uint16 req_id)
{
   switch (po->getType()) {
      case GuiProtEnums::PositionTypeInvalid:
         WARN("handleRoute: Invalid position type!");
         break;
      case GuiProtEnums::PositionTypeSearch:
         /* Use search result to get origin position. */
         if (getSearchPosition(po->m_id, po->m_lat, po->m_lon) < 0) {
            /* Failed getting search position. */
            WARN("handleRoute: Failed getting search position!");
            handleRequestFailed(Nav2Error::UC_CONFUSED, req_type, req_id);
         } else {
            /* Ok search position. */
            po->setPositionOk(1);
         }
         break;
      case GuiProtEnums::PositionTypeFavorite:
         /* Use favorite position as origin. */
         {
            uint32 fav_id = favoriteIdFromString(po->m_id);
            DBG("checking PositionTypeFavorite. fav_id: %"PRIx32
                ", INVALID_FAV_ID: %x", fav_id, 
                unsigned(Favorite::INVALID_FAV_ID));
            if(fav_id != Favorite::INVALID_FAV_ID){
               RequestListItem* req =
                  m_uiCtrl->checkForRequestAndCreate( req_type, req_id );
               if (req){
                  // When routing to a favorite, first we ask the
                  // Destinations module for the favorite info. When
                  // this is received, a route is asked for in
                  // handleGetFavoriteInfoReply.

                  uint32 nav2MessageId = 
                     m_destProvPubl->getFavoriteInfo( fav_id );
                  m_uiCtrl->addRequest(nav2MessageId, req);
                  return 1;
               } else {
                  WARN("Something wrong with the request list.");
                  handleRequestFailed(Nav2Error::UC_CONFUSED, 
                                      req_type, req_id);
               }
            } else {
               po->setType(GuiProtEnums::PositionTypePosition);
               return checkPositionData(po, req_type, req_id);
            }
         }
         break;
      case GuiProtEnums::PositionTypePosition:
         /* Use the specified position as origin. */
         po->setPositionOk(1);
         break;
      case GuiProtEnums::PositionTypeHotDest:
         /* Contact server to get position. */
         /* This will only work when it's the destination. */
         /* Origin coordinate will be filled in by NavServerCom. */
         po->setLat(MAX_INT32);
         po->setLon(MAX_INT32);
         po->setPositionOk(1);
         break;
      case GuiProtEnums::PositionTypeCurrentPos:
         /* Use current GPS position. */
         /* This will probably not work when set as destination, */
         /* since this is the same as HotDest. */
         po->setLat(MAX_INT32);
         po->setLon(MAX_INT32);
         po->setPositionOk(1);
         break;
   }
   return 0;
}

void GuiCtrl::handleRouteToSearchItemRequest(const GenericGuiMess* routeMess)
{
   const char* id = routeMess->getFirstString();
   if(id != NULL){
      SearchItemContainer::iterator res;
      res= find_if(m_searchMatches.begin(), m_searchMatches.end(), 
                   MatchString<SearchItem>(id, &SearchItem::getID));
      if(res != m_searchMatches.end()){
         /* Found the item in the search match vector. */
         DBG("Got search item");
         delete m_currRouteData;
         const SearchRegion* sr = 
            (*res)->getRegionOfType(GuiProtEnums::address);
         size_t len = 
            strlen((*res)->getName()) + (sr ? strlen(sr->getName()) : 0);
         char* name = new char[len + 2 + 1];
         strcpy(name, (*res)->getName());
         if(sr){
            strcat(name, ", ");
            strcat(name, sr->getName());
         }
         m_currRouteData = new RoutePositionData(name, MAX_INT32, MAX_INT32,
                                                 (*res)->getLat(), 
                                                 (*res)->getLon());
         delete[] name;
         DBG("Created route data");
         startRoute(m_currRouteData, routeMess->getMessageType(),
                    routeMess->getMessageID());
         DBG("After startRoute");
      } else {
         /* Didn't find the item to route to in the */
         /* search match list. This means we cannot route */
         /* to it either. */
         handleRequestFailed(Nav2Error::UC_CONFUSED, 
                             routeMess->getMessageType(),
                             routeMess->getMessageID());
         ERR("NULL id! wooho!");
      }
   } else {
      handleRequestFailed(Nav2Error::UC_CONFUSED, 
                          routeMess->getMessageType(),
                          routeMess->getMessageID());
      ERR("NULL id! wooho!");
   }
}

void GuiCtrl::handleInvalidateRoute(const GenericGuiMess* /*mess*/)
{
   m_ntProvPubl->command(NavTaskProviderPublic::AbandonRoute);
}

void GuiCtrl::handleRouteToPositionCancel(const GenericGuiMess* /*mess*/)
{
   m_ntProvPubl->command(NavTaskProviderPublic::RouteToPositionCancel);
}

void
GuiCtrl::handleFileOpMessage(const GuiProtFileMess* mess)
{
   // We're not intrested in this message, just pass it on
   // to NavTask, it knows what to do with it.
   DBG("GuiProtFileMess forwarded to NavTask.");
   m_ntProvPubl->fileOperation(mess->getFileOperation());
}

void GuiCtrl::handleReroute(const GenericGuiMess* mess)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( mess->getMessageType(), 
                                          mess->getMessageID());
   if(req){
      uint32 nav2MessageId = 
         m_ntProvPubl->command(NavTaskProviderPublic::Reroute);
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           mess->getMessageID(),
                           mess->getMessageType() );
   }
}

void GuiCtrl::handleRequestCrossingSound(const GenericGuiMess* mess)
{
   m_uiCtrl->playSoundForCrossing(mess->getFirstUint16());
}

void GuiCtrl::handleGetfilteredRouteList(const GetFilteredRouteListMess* mess)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( mess->getMessageType(), 
                                          mess->getMessageID());
   if(req) {
      uint32 nav2MessageId = 
         m_ntProvPubl->getFilteredRouteList( mess->getStartWpt(),
               mess->getNumWpts() );
         DBG("Will get filtered routelist for %u to %u - 0x%"PRIx32,
               mess->getStartWpt(), mess->getNumWpts(), nav2MessageId );
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           mess->getMessageID(),
                           mess->getMessageType() );
   }
}

void GuiCtrl::startRoute(RoutePositionData *rpd, 
                         GuiProtEnums::MessageType messType,
                         uint16 messID)
{
   RequestListItem* req = 
      m_uiCtrl->checkForRequestAndCreate(messType, messID);
   if ( req == NULL ){
      WARN("GuiCtrl. Did not send start route. "
           "Something wrong with the requests.");
      
      // MN: This should result in a solicited error.
      //     Perhaps return it from the metod.
      handleRequestFailed(Nav2Error::UC_CONFUSED, messType, messID);
      return;
   } else{
      // For now decodedInvalidateRoute is depending on 
      // this variable being set like this.
      req->status = 0;
      
      int32 lat = rpd->getdLat();
      int32 lon = rpd->getdLon();
      uint32 nav2MessageId = m_ntProvPubl->ntRouteToGps( lat, lon );
      
      DBG( "Route request id %#0"PRIx32, nav2MessageId );
      DBG( "Dest = Lat: %"PRId32", Lon: %"PRId32, lat, lon);
      
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
}


void GuiCtrl::handleRouteDownloaded(int64 routeid, int32 tLat, int32 lLon, 
                                    int32 bLat, int32 rLon, 
                                    int32 oLat, int32 oLon, 
                                    int32 dLat, int32 dLon, uint16 reqId)
{
   const char* destName = "";
   if(m_currRouteData != NULL){
      destName = m_currRouteData->getDestinationName();
   }

/*    m_routeid = routeid; */
   StartedNewRouteMess rdm(routeid, tLat, lLon, bLat, rLon,
                           oLat, oLon, dLat, dLon, destName);
   sendMessageToGui(&rdm, reqId);
   DBG("Sent STARTED_NEW_MESSAGE with req-id %d to GUI", reqId);
}

void GuiCtrl::handleUnsolicitedRouteDownloaded(int64 routeid, 
                                               int32 tLat, int32 lLon, 
                                               int32 bLat, int32 rLon, 
                                               int32 oLat, int32 oLon, 
                                               int32 dLat, int32 dLon)
{
   handleRouteDownloaded(routeid, tLat, lLon, bLat, rLon, 
                         oLat, oLon, dLat, dLon, 0);
}

void
GuiCtrl::handleFileOperation(GuiFileOperation* op)
{
   GuiProtFileMess mess(op);
   sendMessageToGui(&mess, 0);
   /* Satinfo is deleted by our caller. */
}

void
GuiCtrl::handleSatelliteInfo(GpsSatInfoHolder* satInfo)
{
   SatelliteInfoMess mess(satInfo);
   sendMessageToGui(&mess, 0);
   /* Satinfo is deleted by our caller. */
} // handleSatelliteInfo

void
GuiCtrl::handleRouteInfo(const RouteInfo& r)
{
   DBG("handleRouteInfo");
#ifndef NO_LOG_OUTPUT
   //r.log(m_log);
#endif
   UpdateRouteInfoMess urim(&r);
   sendMessageToGui(&urim, 0);
} // handleRouteInfo


void
GuiCtrl::handleRouteList(RouteList &rl, uint16 reqId)
{
   RouteListMess rlm(&rl);
   sendMessageToGui(&rlm, reqId);
}

void 
GuiCtrl::sendGpsData(struct UiCtrlInternal::GpsData* gpsData){
   UpdatePositionMess upm(gpsData->lat, gpsData->lon,
                          gpsData->routelat, gpsData->routelon,
                          gpsData->posQuality,
                          gpsData->heading, gpsData->routeheading,
                          gpsData->headingQuality,
                          gpsData->speed, gpsData->speedQuality,
                          gpsData->alt,
                          gpsData->timeStampMillis,
                          gpsData->positionHints);
   sendMessageToGui(&upm, 0);
} // sendGpsData

void
GuiCtrl::setSelectedAccessPointId2(int32 selectedAccessPointId2)
{
   GeneralParameterMess *mess = new GeneralParameterMess(
         GuiProtEnums::paramSelectedAccessPointId2, selectedAccessPointId2);
   sendMessageToGui(mess, 0);
   /* Need to do this, above constructor will copy the data */
   mess->deleteMembers();
   delete mess;
}

void
GuiCtrl::receivedGeneralParamUnset(uint32 paramId, uint16 reqId)
{
   DBG("Got general parameter 0x%"PRIx32" unset!", paramId);
   const general_param_t *gp = generalNav2ParamLookup(paramId);
   
   if (!gp) {
      /* Unknown parameter. */
      return;
   }

   /* Send a "GET" to the GUI to let it know the parameter was unset. */
   GeneralParameterMess *mess = new GeneralParameterMess(gp->gui_param_id);
   sendMessageToGui(mess, reqId);
   /* No data to delete here. */
/*    mess->deleteMembers(); */
   delete mess;
}
void
GuiCtrl::receivedGeneralParam(uint32 paramId, const uint8 * data,
      int32 numEntries, uint16 reqId)
{
   DBG("Got general parameter uint8*!");
   const general_param_t *gp = generalNav2ParamLookup(paramId);
   if (!gp) {
      /* Unknown parameter. */
      return;
   }
   uint8 *dataCopy = const_cast<uint8*>(data);

   GeneralParameterMess *mess =
      new GeneralParameterMess(gp->gui_param_id,
            dataCopy, numEntries);
   sendMessageToGui(mess, reqId);
   /* Must not delete data, does not belong to us. */
   /* mess->deleteMembers(); */
   delete mess;
}
void
GuiCtrl::receivedGeneralParam(uint32 paramId, const int32 * data,
      int32 numEntries, uint16 reqId)
{
   DBG("Got general parameter int32!");
   //used when the incoming parameter needs to be tranformed, such as
   //the expire vector.
   const general_param_t *gp = generalNav2ParamLookup(paramId);
   if (!gp) {
      /* Unknown parameter. */
      return;
   }
   int32 *dataCopy = const_cast<int32*>(data);

   GeneralParameterMess *mess =
      new GeneralParameterMess(gp->gui_param_id,
            dataCopy, numEntries);
   sendMessageToGui(mess, reqId);
   /* Do not use deleteMembers, the data does not belong to the message. */
   /* mess->deleteMembers(); */
   delete mess;
}

void
GuiCtrl::receivedGeneralParam(uint32 paramId, const float * data,
      int32 numEntries, uint16 reqId)
{
   DBG("Got general parameter binary!");
   const general_param_t *gp = generalNav2ParamLookup(paramId);
   if (!gp) {
      /* Unknown parameter. */
      return;
   }
   float *dataCopy = const_cast<float *>(data);

   GeneralParameterMess *mess =
      new GeneralParameterMess(gp->gui_param_id,
            dataCopy, numEntries);
   sendMessageToGui(mess, reqId);
   /* Do not use deleteMembers, the data does not belong to the message. */
   /* mess->deleteMembers(); */
   delete mess;
}
void
GuiCtrl::receivedGeneralParam(uint32 paramId, const char * const * data,
      int32 numEntries, uint16 reqid)
{
   DBG("Got general parameter strings!");
   const general_param_t *gp = generalNav2ParamLookup(paramId);
   if (!gp) {
      /* Unknown parameter. */
      WARN("Parameter was unknown!");
      return;
   }
   DBG("Creating char* array of size %"PRId32, numEntries);
   char **dataCopy = new char*[numEntries];
   for (int32 i = 0; i < numEntries; i++) {
      DBG("Copying string %"PRId32" = \"%s\" (%d)", i, data[i], strlen(data[i])+1);
      dataCopy[i] = new char[strlen(data[i])+1];
      strcpy(dataCopy[i], data[i]);
   }

   GeneralParameterMess *mess =
      new GeneralParameterMess(gp->gui_param_id,
            dataCopy, numEntries);
   DBG("Got parameter %#"PRIx32" = %d", 
       uint32(gp->nav2_param_id), gp->gui_param_id);
   DBG("Sending with request id %d", reqid);
   sendMessageToGui(mess, reqid);
   mess->deleteMembers();
   delete mess;
}

const general_param_t *
GuiCtrl::generalNav2ParamLookup(uint32 paramId)
{
   DBG("generalNav2ParamLookup %"PRIx32, paramId);
   //yes, this sizeof calculation does the right thing. See the C++ standard.
   int32 numGeneralParams = 
      sizeof(generalParamArray) / sizeof(generalParamArray[0]);
   enum ParameterEnums::ParamIds eParamId = ParameterEnums::ParamIds(paramId);

   for (int32 i = 0; i < numGeneralParams; i++) {
      if (generalParamArray[i].nav2_param_id == eParamId) {
         return &generalParamArray[i];
      }
   }
   return NULL;
}

const general_param_t *
GuiCtrl::generalGuiParamLookup(uint16 paramId)
{
   DBG("generalGuiParamLookup %"PRIx16, paramId);
   //yes, this sizeof calculation does the right thing. See the C++ standard.
   int32 numGeneralParams = 
      sizeof(generalParamArray) / sizeof(generalParamArray[0]);
   for (int32 i = 0; i < numGeneralParams; i++) {
      if (generalParamArray[i].gui_param_id == paramId) {
         return &generalParamArray[i];
      }
   }
   return NULL;
}

void
GuiCtrl::handleGetGeneralParameter( const GeneralParameterMess* guiMessage)
{
   /* Allocate new request list item. */
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());

   DBG("Got handle get general parameter type %u!", 
       guiMessage->getMessageType());
   if (req){
      /* Request parameter from parameter module. */
      /* Get real parameter id from the table. */
      const general_param_t *gp =
         generalGuiParamLookup(guiMessage->getParamId());
      if (gp) {
         /* Found it. */
         uint32 nav2MessageId = m_paramProvider->getParam( gp->nav2_param_id );
         m_uiCtrl->addRequest(nav2MessageId, req);
      } else {
         /* Not in the general param list? */
         WARN("Param not found in general list!");
         handleRequestFailed( Nav2Error::UC_CONFUSED,
                              guiMessage->getMessageID(),
                              guiMessage->getMessageType() );
      }
   } else {
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
   /* The sending of the parameter data is handled when the reply */
   /* comes from the parameter module. */
} // handleGetGeneralParameter

void GuiCtrl::handleChangeUIN(const class GenericGuiMess* guiMessage)	
{
   const char* up[2] = {NULL, NULL};      //for setting Nav2 parameters
   up[0] = guiMessage->getFirstString();  // The UIN/UserLogin
   //   m_paramProvider->setParam(ParameterEnums::NSC_UserAndPasswd, up,2);

   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());

   if (req){
      uint32 nav2MessageId = 
         m_paramProvider->setParam(ParameterEnums::NSC_UserAndPasswd, up, 2);
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void 
GuiCtrl::handleSetGeneralParameter( const GeneralParameterMess* guiMessage)
{
   /* Send parameter change to parameter module. */
   DBG("Got handle set general parameter!");

   /* Get real parameter id. */
   const general_param_t *gp =
      generalGuiParamLookup(guiMessage->getParamId());
   if(! gp){ //Didn't find the parameter in the mapping. Just ignore it. 
      WARN("Couldn't find parameter %d, ignoring", int(guiMessage->getParamId()));
      return;
   }
   switch (gp->param_type) {
      case GuiProtEnums::paramTypeInt32:
         DBG("GUI set int32 parameter.");
         if (guiMessage->getParamId() ==
             GuiProtEnums::paramSelectedAccessPointId2) {
            // Setting proxy and port. 
            char* tmpStr = makeSelectedAccessPointId2String(
               guiMessage->getIntegerData()[0]);
            m_paramProvider->setParam( ParameterEnums::SelectedAccessPointId2,
                  &tmpStr, 1 );
            delete[] tmpStr;
         } else {
            m_paramProvider->setParam(gp->nav2_param_id, 
                                      guiMessage->getIntegerData(), 
                                      guiMessage->getNumEntries());
         }
      break;
      case GuiProtEnums::paramTypeFloat:
         DBG("GUI set float parameter.");
         m_paramProvider->setParam(gp->nav2_param_id, 
               guiMessage->getFloatData(),
               guiMessage->getNumEntries());
      break;
      case GuiProtEnums::paramTypeString:
         DBG("GUI set string parameter.");
         m_paramProvider->setParam(gp->nav2_param_id, 
                                   guiMessage->getStringData(),
                                   guiMessage->getNumEntries());
      break;
      case GuiProtEnums::paramTypeBinary:
         DBG("GUI set binary parameter %"PRId32" bytes",
             guiMessage->getNumEntries());
         m_paramProvider->setParam(gp->nav2_param_id, 
                                   (uint8 *)guiMessage->getBinaryData(),
                                   guiMessage->getNumEntries());
      break;
      default:
      WARN("GUI set unknown parameter.");
      /* Should not get here, this should have been a GET_GENERAL_PARAMETER. */
      break;
   }
   ///If this is not a multicast parameter, say that it has changed now.
   if(! ParameterProviderPublic::paramIdIsMulticast(gp->nav2_param_id)){
      sendParameterChangedToGui(GuiProtEnums::ParameterType(guiMessage->getParamId()));
   }
} // handleSetGeneralParameter.


void
GuiCtrl::handleGetFavorites( const GenericGuiMess* guiMessage)
{
   DBG("handleGetFavorites");
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());

   if (req){
      uint32 nav2MessageId = 
         m_destProvPubl->getFavorites( guiMessage->getFirstUint16(), 
                                       guiMessage->getSecondUint16() );
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
} // handleGetFavorites

void
GuiCtrl::handleGetFavoritesReply(vector<GuiFavorite*>& guiFavorites, 
                                 uint16 reqId)
{
   DBG("handleGetFavoritesReply");
   GetFavoritesReplyMess gfrm(&guiFavorites);
   sendMessageToGui(&gfrm, reqId);
}

void  
GuiCtrl::handleGetFavoritesAllData( const GenericGuiMess* guiMessage)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   
   if(req){
      uint32 nav2MessageId = 
         m_destProvPubl->getFavoritesAllData( guiMessage->getFirstUint16(), 
                                              guiMessage->getSecondUint16() );
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void 
GuiCtrl::handleGetFavoritesAllDataReply(vector<Favorite*>& favorites, 
                                        uint16 reqId)
{
   GetFavoritesAllDataReplyMess gfadrm(&favorites);
   sendMessageToGui(&gfadrm, reqId);
}

void  
GuiCtrl::handleSortFavorites( const GenericGuiMess* guiMessage)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if(req){   
      uint32 nav2MessageId = 
         m_destProvPubl->sortFavorites(
            GuiProtEnums::SortingType(guiMessage->getFirstUint16()) );
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void  
GuiCtrl::handleSyncFavorites( const GenericGuiMess* guiMessage)
{
/* #define EAT_SHOTGUN_PELLETS_YOU_HORRIBLE_MONSTER */
#ifdef EAT_SHOTGUN_PELLETS_YOU_HORRIBLE_MONSTER
   DBG("WILL DIE NOW!!!");
   *((volatile uint8*)(NULL)) = 1;
   *((volatile uint8*)(NULL)) = 0;
   int a = *(volatile int*)(NULL);
   a = a;
#endif
   DBG("handleSyncFavorites");

   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if(req){   
      uint32 nav2MessageId = m_destProvPubl->syncFavorites();
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void  
GuiCtrl::handleGetFavoriteInfo( const GenericGuiMess* guiMessage)
{
   DBG("GetFavoriteInfo. Favorite id: %"PRId32".", guiMessage->getFirstUint32());

   RequestListItem* req =
   m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                       guiMessage->getMessageID());

   if (req){
      uint32 nav2MessageId = 
         m_destProvPubl->getFavoriteInfo( guiMessage->getFirstUint32() );
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void 
GuiCtrl::handleGetFavoriteInfoReply(Favorite* favorite, 
                                    GuiProtEnums::MessageType guiMsgType,
                                    uint16 guiSeqId)
{
   if ( guiMsgType == GuiProtEnums::GET_FAVORITE_INFO){
      // This is an ansver to a get favorite info from GUI.

      GetFavoriteInfoReplyMess gfirm(favorite);
      sendMessageToGui(&gfirm, guiSeqId);
   } else if ( guiMsgType == GuiProtEnums::ROUTE_TO_FAVORITE){
      // This is a partial step in a route to favorite request
      // from the GUI.

      DBG("Routing to favorite lat: %"PRId32", lon %"PRId32".", 
          favorite->getLat(), favorite->getLon());
      //XXX factor out some of this code!
      delete m_currRouteData;
      m_currRouteData = new RoutePositionData(favorite->getName(),
            MAX_INT32, MAX_INT32, 
            favorite->getLat(), 
            favorite->getLon());
      RequestListItem* req = 
         m_uiCtrl->checkForRequestAndCreate(guiMsgType, guiSeqId); 
      if ( req == NULL ){
         WARN("GuiCtrl. Did not send start route. "
              "Something wrong with the requests.");
         
         // MN: This should result in a solicited error.
         //     Perhaps return it from the metod.
         handleRequestFailed(Nav2Error::UC_CONFUSED, guiMsgType, guiSeqId);
         return;
      } else{
         // For now decodedInvalidateRoute is depending on 
         // this variable being set like this.
         req->status = 0;
         
         int32 lat = m_currRouteData->getdLat();
         int32 lon = m_currRouteData->getdLon();
         uint32 nav2MessageId = m_ntProvPubl->ntRouteToGps( lat, lon );
         
         DBG( "Route request id %#0"PRIx32, nav2MessageId );
         DBG( "Dest = Lat: %"PRId32", Lon: %"PRId32, lat, lon);
         
         m_uiCtrl->addRequest(nav2MessageId, req);
      }
   } else if ( guiMsgType == GuiProtEnums::ROUTE_MESSAGE){
      /* Check and set data. */
      DBG("Got favorite reply, entering data for route");

      checkAndSetRouteData(m_currRouteData,
            favorite->getName(),
            favorite->getLat(),
            favorite->getLon());
      /* The below routine will send the route request for us. */
      checkAndSendRouteData(m_currRouteData);
   } else{
      WARN("GuiCtrl. Something wrong with the requests.");
      handleRequestFailed( Nav2Error::UC_CONFUSED, guiSeqId, guiMsgType );
   }
}


void  
GuiCtrl::handleAddFavorite( const AddFavoriteMess* guiMessage)
{
   RequestListItem* req =
   m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                       guiMessage->getMessageID());

   Favorite* favToAdd = guiMessage->getFavorite();
   
   if (req){
      uint32 nav2MessageId = 
         m_destProvPubl->addFavorite(favToAdd);
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void  
GuiCtrl::handleAddFavoriteFromSearch( const GenericGuiMess* guiMessage ) {
   if ( m_hasAdditionalInfo ) {

      RequestListItem* req =
         m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                             guiMessage->getMessageID() );

      if ( req ) {
         //new guimessage version
         const char* id = guiMessage->getFirstString();
         SearchItemContainer::iterator res = 
            find_if( m_searchMatches.begin(), m_searchMatches.end(),
                     MatchString<SearchItem>( id, &SearchItem::getID ) ); 
         if ( res != m_searchMatches.end() ) {
            //we dont want the distance in the favorite description, so
            //we replace it with the Scary Invalid Distance (TM)
            uint32 keepDistance = (*res)->getDistance();
            (*res)->setDistance(MAX_UINT32);
            char* concat = SearchPrintingPolicyFactory::parseSearchItem(
               *res, false, m_distanceMode, false );
            DBG( "Processed string: '%s'", concat );
            Favorite favToAdd( (*res)->getLat(), (*res)->getLon(), 
                               (*res)->getName(), "", 
                               concat/*description*/, "", "" );
            // Add fav infos... from AdditionalInfos
            uint32 i = 0;
            for ( i = 0 ; i < (*res)->noAdditionalInfo() ; ++i ) {
               favToAdd.addAdditionalInfo( 
                  new AdditionalInfo( *(*res)->getInfo( i ) ) );
            }
            if ( favToAdd.truncateStrings() ) {
               DBG("Truncated one or more strings when adding "
                   "a favorite from a search match.");
            }

            uint32 nav2MessageId = m_destProvPubl->addFavorite( &favToAdd );
            m_uiCtrl->addRequest( nav2MessageId, req );
            //restore distance
            (*res)->setDistance( keepDistance );
            delete [] concat;
         } else {
            WARN( "Could not add a favorite from search because "
                  "the given search match id could not be found." );
         
            handleRequestFailed( Nav2Error::UC_UNKNOWN_SEARCH_ID,
                                 guiMessage->getMessageID(),
                                 guiMessage->getMessageType() );
         }
      } else {
         WARN( "Something wrong with the request list." );
         handleRequestFailed( Nav2Error::UC_CONFUSED,
                              guiMessage->getMessageID(),
                              guiMessage->getMessageType() );
      }
   } else {
      getInfoForAllSearchMatches( guiMessage, FSISaveAsFav );
   }
}

void
GuiCtrl::getInfoForAllSearchMatches( const GenericGuiMess* mess,
                                     enum FSIrequestType reqType )
{
   // Get info first
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( mess->getMessageID(),
                                          mess->getMessageType() );
   if ( req ) {
      std::vector<const char*> ids( m_searchMatches.size(), NULL );
      std::transform( m_searchMatches.begin(), m_searchMatches.end(),
                      ids.begin(), 
                      const_mem_fun_t<const char*, 
                      FullSearchItem>( &FullSearchItem::getID ) );
      uint32 nav2MessageId =    
         m_nscProvPubl->requestSearchInfo( ids.size(), &(ids.front()) );
      GenericGuiMess* keep = new GenericGuiMess( mess );
      keep->setMessageId( mess->getMessageID() );
      req->data = new FSIrequest( keep, reqType );
      m_uiCtrl->addRequest( nav2MessageId, req );
   } else {
      WARN( "Something wrong with the request list." );
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           mess->getMessageID(),
                           mess->getMessageType() );
   }
}

void  
GuiCtrl::handleRemoveFavorite( const GenericGuiMess* guiMessage)
{
   RequestListItem* req =
   m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                       guiMessage->getMessageID());
   if (req){
      uint32 nav2MessageId = 
         m_destProvPubl->removeFavorite(guiMessage->getFirstUint32());
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void  
GuiCtrl::handleChangeFavorite( const class ChangeFavoriteMess* guiMessage)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   Favorite* favToChange = guiMessage->getFavorite();
   if (req){
      uint32 nav2MessageId =
         m_destProvPubl->changeFavorite(favToChange);
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void
GuiCtrl::handleRouteToFavorite ( const GenericGuiMess* guiMessage)
{
   RequestListItem* req =
   m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                       guiMessage->getMessageID());
   if (req){
      // When routing to a favorite, first we ask the Destinations
      // module for the favorite info. When this is received, 
      // a route is asked for in handleGetFavoriteInfoReply.

      uint32 nav2MessageId = 
         m_destProvPubl->getFavoriteInfo(guiMessage->getFirstUint32() );
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void
GuiCtrl::handleGpsConnection(const GenericGuiMess* guiMessage)
{
   // Do not save this request. We do not get a reply.
   Module::ConnectionCtrl action = Module::DISCONNECT;
   if ( guiMessage->getMessageType() == GuiProtEnums::CONNECT_GPS ){
      action = Module::CONNECT;
   }
   else if ( guiMessage->getMessageType() == GuiProtEnums::DISCONNECT_GPS ){
      action = Module::DISCONNECT;
   }
   else{
      WARN("Calling handleGpsConnection with wrong message type.");
      m_uiCtrl->unsolicitedError(Nav2Error::UC_CONFUSED);
   }

   // This only has effect if a GPS is connected. Otherwise it is stuck in the 
   // simulation.
   WARN("GPS connection handling not implemented. action = %d (bool)", action);
   //const char* method = ""; //We're not telling what to connect to.
   //DBG("Calling NT connectionCtrl. action = 0x%x", action);
   //m_ntProvPubl->connectionCtrl(action, method);

} // handleGpsConnection

void
GuiCtrl::handleFavoritesChanged ()
{
   // No request handling here.
   // All requests have separate replies.
   GenericGuiMess ggm(GuiProtEnums::FAVORITES_CHANGED);
   sendMessageToGui(&ggm, 0);
}

void 
GuiCtrl::handleRequestFailed( const ErrorObj& err,
                              uint16 failedRequestSeqId,
                              uint16 failedRequestMessageType )
{
   WARN("Request failed. errorNbr=0x%08x, guiSeqId=0x%08x, "
        "messageType=0x%08x", err.getErr(), failedRequestSeqId, 
        failedRequestMessageType);

   if((GuiProtEnums::MessageType(failedRequestMessageType) == 
       GuiProtEnums::ROUTE_MESSAGE) &&
      (!(m_currRouteData->getoPositionOk() && 
         m_currRouteData->getdPositionOk()))){
      resetFavoriteToPosition(m_currRouteData);
      /* The below routine will send the route request for us. */
      checkAndSendRouteData(m_currRouteData);
      return;
   }

   const char* errorString = m_uiCtrl->m_errorTable->getErrorString( 
      err.getErr() );
   if ( err.getErrorMessage()[ 0 ] != '\0' ) {
      errorString = err.getErrorMessage();
   }

   if ( errorString != NULL ) {
      if (failedRequestSeqId != 0){
         // This is a GuiProtMess message
         GuiProtEnums::MessageType messageNbr = 
            GuiProtEnums::MessageType(failedRequestMessageType);
         const char* failedRequestString = 
            GuiRequestFailed::getFailedRequestString(messageNbr, m_uiCtrl->m_errorTable);   
      
         RequestFailedMess rfm( err.getErr(), errorString, 
                                err.getExtendedErrorString(),
                                failedRequestSeqId,
                                messageNbr,
                                failedRequestString );
         sendMessageToGui(&rfm, failedRequestSeqId);
         rfm.deleteMembers();
      } else {
         ERR("Request failed for old ThinClient style request. BROKEN!");
      }
   }
   else{
      WARN( "Received error with no string. ErrorNbr: 0x%08x", 
            err.getErr() );
   }
} // handleRequestFailed

void GuiCtrl::handleSearchAreas(const GenericGuiMess* areaMess)
{
   //this way works as long as m_searchAreas is a
   //std::vector<SearchArea*>, and it's iterator a SearchArea**
   SearchAreaReplyMess mess(&m_searchAreas.front(), m_searchAreas.size());
   sendMessageToGui(&mess, areaMess->getMessageID());
}

void GuiCtrl::handleSearchItems(const GenericGuiMess* itemMess)
{
   uint16 requestIndex = itemMess->getFirstUint16();
   if(requestIndex >= m_totalnumberOfSearchItems){
      DBG("Bad index %u, failing with a confused.", requestIndex);
      handleRequestFailed(Nav2Error::UC_CONFUSED, itemMess->getMessageID(),
                          itemMess->getMessageType());
   } else if(requestIndex >= m_numberOfFirstSearchItem &&
             (requestIndex < 
              (m_numberOfFirstSearchItem + m_searchMatches.size()))){
      sendItemsToGui(requestIndex, requestIndex + m_paramMaxNbrSearchMatches, 
                     itemMess->getMessageID());
   } else {
      RequestListItem* req = 
         m_uiCtrl->checkForRequestAndCreate(itemMess->getMessageType(), 
                                            itemMess->getMessageID());
      if (req){
         uint32 nav2MessageId = m_refinedSearch.request(requestIndex);
         m_uiCtrl->addRequest(nav2MessageId, req);
      } else{
         WARN("Something wrong with the request list.");
         handleRequestFailed( Nav2Error::UC_CONFUSED, itemMess->getMessageID(),
                              itemMess->getMessageType() );
      }
   }      
}

void GuiCtrl::handleFullSearchDataFromItemId(const GenericGuiMess* fullItemReq)
{
   char *item_id = fullItemReq->getFirstString();
   char *name = fullItemReq->getSecondString();
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( fullItemReq->getMessageType(), 
                                          fullItemReq->getMessageID());
   if (req){
      std::vector<const char*> ids;
      ids.push_back(item_id);
      uint32 nav2MessageId =
         m_nscProvPubl->requestSearchInfo(ids.size(), &(ids.front()));
      GenericGuiMess* keep = new GenericGuiMess(fullItemReq->getMessageType(), 
                                                item_id,
                                                name);
      req->data = new FSIrequest(keep, FSISingleItemId);
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
}

void GuiCtrl::handleFullSearchData(const GenericGuiMess* fullItemReq)
{
   // the GUI protocol states that the range requested is [first,
   // last], but stl algorithms always works on the range [begin, end). 
   // A little tampering with the indexes is needed to guarantee
   // that they will work properly with the iterators below.
   int32 first = fullItemReq->getFirstUint16();
   int32 last  = fullItemReq->getSecondUint16() + 1; 
   last  = min(m_searchMatches.size(), (size_t)last);
   first = min(first, last);

   if(m_hasAdditionalInfo){
      const FullSearchItem*const* fsiStart = NULL;
      if(last - first > 0){
         fsiStart = &(m_searchMatches[first]); 
      }
      std::for_each(&m_searchMatches[first], &m_searchMatches[last],
                    UpdateDistance(m_lastLat, m_lastLon));
      FullSearchDataReplyMess fullInfo(fsiStart, last - first);
      sendMessageToGui(&fullInfo, fullItemReq->getMessageID());
   } else {
      getInfoForAllSearchMatches( fullItemReq, FSIIndexRange );
   }
}

void GuiCtrl::handleMessageRequest(const SendMessageMess* guiMessage)
{
   DBG("handleMessageRequest");
   const SendMessageMess& sms = *guiMessage;
   RequestListItem* req = 
      m_uiCtrl->checkForRequestAndCreate(sms.getMessageType(), 
                                         sms.getMessageID());
   if (req){
      uint32 nav2MessageId = m_nscProvPubl->messageRequest(sms.getMedia(), 
                                                           sms.getObject(), 
                                                           sms.getTo(), 
                                                           sms.getFrom(), 
                                                           sms.getSignature(), 
                                                           sms.getId());
      m_uiCtrl->addRequest(nav2MessageId, req);
   }
   else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED, sms.getMessageID(),
                           sms.getMessageType() );
   }
}

void GuiCtrl::handleMessageSent(const uint8* data, unsigned length, 
                                uint16 reqId)
{
   DBG("handleMessageSent");
   MessageSentMess msm(const_cast<uint8*>(data), length);
   sendMessageToGui(&msm, reqId);
}

void GuiCtrl::handleGetMoreSearchData(const GetMoreDataMess* guiMessage)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if (req){
      const char* id = m_searchMatches[guiMessage->getIndex()]->getID();
      const char* val = guiMessage->getValue();
      uint32 nav2MessageId = m_nscProvPubl->requestSearchInfo(0, &id, &val);
      GenericGuiMess* keep = new GenericGuiMess(guiMessage->getMessageType(), 
                                                guiMessage->getIndex(), 
                                                guiMessage->getIndex() + 1);
      keep->setMessageId(guiMessage->getMessageID());
      req->data = new FSIrequest(keep, FSIIndexRange);
      m_uiCtrl->addRequest(nav2MessageId, req);
   }else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED,
                           guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void GuiCtrl::handleLicenseKey(const LicenseKeyMess* guiMessage)
{
   DBG("handleLicenseKey");
   if(guiMessage->getKey() && guiMessage->getPhone()){
      DBG("Key '%s', phone: '%s'", guiMessage->getKey(), 
          guiMessage->getPhone());
      char* key = strdup_new(guiMessage->getKey());
         RequestListItem* req =
            m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                                guiMessage->getMessageID());
         if (req){
            uint32 nav2MessageId = 
               m_nscProvPubl->verifyLicenseKey(key, guiMessage->getPhone(), 
                                               guiMessage->getName(),
                                               guiMessage->getEmail(),
                                               guiMessage->getOptional(),
                                               guiMessage->getRegion());
            m_uiCtrl->addRequest(nav2MessageId, req);
         }else{
            WARN("Something wrong with the request list.");
            handleRequestFailed( Nav2Error::UC_CONFUSED,
                                 guiMessage->getMessageID(),
                                 guiMessage->getMessageType() );
         }
      DBG("Deleting key: %p", key);
      delete[] key;
   } else {
      GenericGuiMess ggm(GuiProtEnums::LICENSE_UPGRADE_REPLY, 
                         guiMessage->getKey() != NULL, 
                         guiMessage->getPhone() != NULL, true);
      sendMessageToGui(&ggm, guiMessage->getMessageID());  
   }
   
}

void GuiCtrl::handleParameterSync(const GenericGuiMess* guiMessage)
{
   DBG( "GuiCtrl::handleParameterSync" );
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if (req){
      uint32 nav2MessageId = m_nscProvPubl->paramSync();
      m_uiCtrl->addRequest(nav2MessageId, req);
   }else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED, guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void GuiCtrl::handleLicenseReply(bool keyOk, bool phoneOk, bool regionOk,
                                 bool nameOk, bool emailOk,
                                 enum GuiProtEnums::WayfinderType type, 
                                 uint16 reqId)
{
   class LicenseReplyMess lrm(keyOk, phoneOk, regionOk, nameOk, emailOk, type);
   sendMessageToGui(&lrm, reqId);
}

void GuiCtrl::handleVectorMapRequest(const GenericGuiMess* guiMessage)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if (req){
      uint32 nav2MessageId = 
         m_nscProvPubl->vectorMapRequest(guiMessage->getFirstString());
      m_uiCtrl->addRequest(nav2MessageId, req);
   }else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED, guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void GuiCtrl::handleMultiVectorMapRequest(const DataGuiMess* guiMessage)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if (req){
      uint32 nav2MessageId = 
         m_nscProvPubl->multiVectorMapRequest(guiMessage->getSize(),
                                              guiMessage->getData());
      m_uiCtrl->addRequest(nav2MessageId, req);
   }else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED, guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void
GuiCtrl::handleMultiVectorMapReply(uint32 size, const uint8* data, uint16 reqId)
{
   DataGuiMess cc(GuiProtEnums::GET_MULTI_VECTOR_MAP_REPLY, size, data);
   sendMessageToGui(&cc, reqId);
   cc.deleteMembers();
} 

void 
GuiCtrl::handleForceFeedMultiVectorMapReply( uint32 size, const uint8* data )
{
   DataGuiMess cc( GuiProtEnums::FORCEFEED_MULTI_VECTOR_MAP_REPLY,
                   size, data );
   sendMessageToGui( &cc, 0x1/*reqId*/ );
   cc.deleteMembers();
}

void
GuiCtrl::handleTunnelDataReply(uint32 size, const uint8* data,
                               uint32 ad_size, const uint8* ad_data,
                               uint16 reqId)
{
   DBG("handleTunnelDataReply");
   DataGuiMess cc(GuiProtEnums::GUI_TUNNEL_DATA_REPLY,
         size, data, ad_size, ad_data);
   sendMessageToGui(&cc, reqId);
   cc.deleteMembers();
}

void GuiCtrl::handleTunnelDataRequest(const DataGuiMess* guiMessage)
{
   DBG("handleTunnelDataRequest");
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if (req){

      NParamBlock params;
      NParam& p = params.addParam( NParam( 5400 ) );
      p.addByteArray(guiMessage->getAdditionalData(),
         guiMessage->getAdditionalSize());

      NParam& p2 = params.addParam( NParam( 5401 ) );
      p2.addByteArray(guiMessage->getData(), guiMessage->getSize());
      uint32 nav2MessageId = m_nscProvPubl->sendRequest( 
         navRequestType::NAV_TUNNEL_DATA_REQ, params );
      m_uiCtrl->addRequest(nav2MessageId, req);
   } else {
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED, guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void
GuiCtrl::handleNGPTunnelDataReply(uint32 ad_size, const uint8* ad_data,
      uint16 packet_id, uint16 reqId)
{
   DBG("handleNGPTunnelDataReply");

   Buffer *buf = new Buffer(16);
   buf->writeNextUnaligned16bit(packet_id);
   const uint8* data = buf->accessRawData();
   int32 size = 4;

   DataGuiMess cc(GuiProtEnums::GUI_TO_NGP_DATA_REPLY,
         size, data, ad_size, ad_data);
   sendMessageToGui(&cc, reqId);
   cc.deleteMembers();
   delete buf;
}

void
GuiCtrl::handleNGPTunnelDataRequest(const DataGuiMess* guiMessage)
{
   DBG("handleNGPTunnelDataRequest");
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if (req){
      Buffer *buf = new NonOwningBuffer(
            guiMessage->getData(), guiMessage->getSize());

      uint16 packet_id = buf->readNextUnaligned16bit();
      delete buf;

      NParamBlock params(guiMessage->getAdditionalData(),
            guiMessage->getAdditionalSize(), NSC_PROTO_VER);

      uint8 requestVer = guiMessage->getRequestVersion();
      uint32 nav2MessageId = m_nscProvPubl->sendRequest(
         (isab::navRequestType::RequestType)packet_id, params, requestVer);
      m_uiCtrl->addRequest(nav2MessageId, req);
   } else {
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED, guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void GuiCtrl::handleCellReport(const DataGuiMess* guiMessage)
{
   RequestListItem* req =
      m_uiCtrl->checkForRequestAndCreate( guiMessage->getMessageType(), 
                                          guiMessage->getMessageID());
   if (req){
      uint32 nav2MessageId = m_nscProvPubl->cellReport(guiMessage->getSize(),
                                                       guiMessage->getData());
      m_uiCtrl->addRequest(nav2MessageId, req);
   }else{
      WARN("Something wrong with the request list.");
      handleRequestFailed( Nav2Error::UC_CONFUSED, guiMessage->getMessageID(),
                           guiMessage->getMessageType() );
   }
}

void GuiCtrl::handleCellConfirm(uint32 size, const uint8* data, uint16 reqId)
{
   DataGuiMess cc(GuiProtEnums::CELL_INFO_FROM_SERVER, size, data);
   sendMessageToGui(&cc, reqId);
   cc.deleteMembers();
}


char*
GuiCtrl::makeSelectedAccessPointId2String( 
   int32 selectedAccessPointId2 ) const 
{
   DBG("makeSelectedAccessPointId2String");
   char* host = NULL;
   uint32 port = 0;
   bool haveProxy = getIAPProxy( host, port, selectedAccessPointId2 );
   char* tmpStr = new char[ 40 + (host ? strlen( host ) : 0) + 1 ];
   if ( haveProxy ) {
      sprintf( tmpStr, "%"PRIi32",%s:%"PRIi32"",
               selectedAccessPointId2, host, port );
   } else {
      sprintf( tmpStr, "%"PRIi32"", selectedAccessPointId2 );
   }
   delete [] host;
   return tmpStr;
}


GuiCtrl::RefinedSearchRequest::RefinedSearchRequest(const NSCPP* provider) :
   m_provider(provider), m_destination(NULL), m_areaId(NULL), m_area(NULL),
   m_countryCode(MAX_UINT32), m_lat(MAX_INT32), m_lon(MAX_INT32), m_hdg(0)
{}

GuiCtrl::RefinedSearchRequest::~RefinedSearchRequest()
{
   delete[] m_destination;
   delete[] m_areaId;
   delete[] m_area;
}

uint32 GuiCtrl::RefinedSearchRequest::request(uint16 startIndex) const
{
   if(m_areaId){
      return m_provider->refinedSearch(m_destination, m_areaId, startIndex,
                                       m_countryCode, m_lat, m_lon, m_hdg);
   } else {
      return m_provider->originalSearch(m_area, m_destination, startIndex, 
                                        m_countryCode, m_lat, m_lon, m_hdg);
   }
}

void GuiCtrl::RefinedSearchRequest::setData(const char* destination,
                      const char* areaId,
                      uint32 region,
                      int32 lat, int32 lon,
                      uint8 heading)
{
   delete[] m_destination;
   delete[] m_areaId;
   m_destination = strdup_new(destination);
   m_areaId = strdup_new(areaId);
   m_countryCode = region;
   m_lat = lat;
   m_lon = lon;
   m_hdg = heading;
}


uint32 GuiCtrl::RefinedSearchRequest::request(const char* destination, 
                                              const char* areaId, 
                                              uint32 region, 
                                              int32 lat, int32 lon, 
                                              uint8 heading)
{
   delete[] m_destination;
   delete[] m_areaId;
   m_destination = strdup_new(destination);
   m_areaId = strdup_new(areaId);
   m_countryCode = region;
   m_lat = lat;
   m_lon = lon;
   m_hdg = heading;
   return request(0);
}

uint32 GuiCtrl::RefinedSearchRequest::firstSearch(const char* destination, 
                                                  const char* areaString,
                   uint32 region, int32 lat, int32 lon,
                   uint8 heading)
{
   delete[] m_area;
   m_area = strdup_new(areaString);
   return request(destination, NULL, region, lat, lon, heading);
}

void GuiCtrl::RefinedSearchRequest::setAreaId(const char* id)
{
   delete[] m_areaId;
   m_areaId = strdup_new(id);
}

} // namespace isab


#ifdef __SYMBIAN32__
#include <commdb.h>
#include <utf.h>

char* TDesCToUtf8L( const TDesC& inbuf ) {
   HBufC8* temp;
   temp = HBufC8::NewLC( inbuf.Length()*6 );
   TPtr8 temp_ptr = temp->Des();
   TInt truncated = CnvUtfConverter::
      ConvertFromUnicodeToUtf8( temp_ptr, inbuf );
   if ( truncated != KErrNone ) {
      // This should never happen as 6 bytes per unicode
      // is all that is possible.
   }
   TInt len = temp->Length();
   char* tmpStr = new char[ len + 4 ];
   int i = 0;
   for ( i = 0 ; i < len ; ++i ) {
      tmpStr[i] = (*temp)[i];
   }
   tmpStr[ i ] = 0;
   CleanupStack::PopAndDestroy( temp );

   return tmpStr;
}
#endif


bool
isab::GuiCtrl::getIAPProxy( char*& host, uint32& port, uint32 iap ) const
{
   host = NULL;
   port = 0;
   bool found = false;
   DBG( "getIAPProxy get IAP proxy called" );
   int32 currIAPId = iap;
   DBG( "currIAPId: %"PRId32", m_serverListParam: %#x, m_useIAPProxy: %d, "
        "m_hardIapProxyHost: %s", currIAPId, m_serverListParam, m_useIAPProxy,
        m_hardIapProxyHost ? m_hardIapProxyHost : "(null)");
   if ( ( currIAPId >= 0 && 
          ( m_serverListParam == ParameterEnums::NSC_HttpServerHostname ) &&
          m_useIAPProxy ) ||
        ( currIAPId < 0 && m_useIAPProxy && ( m_hardIapProxyHost != NULL ) ) )
   {
      DBG( "getIAPProxy get IAP proxy starts, IAP %"PRId32, currIAPId );
      if ( m_hardIapProxyHost != NULL ) {
         host = strdup_new( m_hardIapProxyHost );
         port = m_hardIapProxyPort;
         DBG( "getIAPProxy get IAP proxy hardcoded %s:%"PRId32, host, port );
         return true;
      }
#ifdef __SYMBIAN32__

      // Set to a real value
# if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
      // Open database and don't create it if does not exist.
      CCommsDatabase* comdb = NULL;
      TRAPD(err, comdb = CCommsDatabase::NewL(EFalse));
      if (err != KErrNone) {
         // Could not open CommsDB return no proxy found.
         return EFalse;
      }
# else
      CCommsDatabase * comdb = CCommsDatabase::NewL(EDatabaseTypeUnspecified);
# endif
      CleanupStack::PushL( comdb );
      DBG( "getIAPProxy comdb created" );
      // First get the IAP
      CCommsDbTableView* iaptable = comdb->OpenViewMatchingUintLC(
         TPtrC( IAP ), TPtrC( COMMDB_ID ), currIAPId );
      DBG( "getIAPProxy iaptable created" );
      TInt iapres = iaptable->GotoFirstRecord();
      DBG( "getIAPProxy iaptable goto res %d", iapres );
      if ( iapres == KErrNone ) {
         DBG( "getIAPProxy iaptable have record" );
         HBufC* iap_name = iaptable->ReadLongTextLC( 
            TPtrC( COMMDB_NAME) );
         uint32 iap_service = 0;
         iaptable->ReadUintL( TPtrC( IAP_SERVICE ), iap_service );
         DBG( "iap_service %"PRIu32, iap_service );
         // The current IAP exists!
         HBufC* iap_type = iaptable->ReadLongTextLC( 
            TPtrC( IAP_SERVICE_TYPE ) );
         // Find Proxy for ISP (and same service type)
         CCommsDbTableView* proxytable = NULL;
         TRAPD(err, 
               proxytable = comdb->OpenViewMatchingUintLC(TPtrC(PROXIES), 
                                                          TPtrC(PROXY_ISP), 
                                                          iap_service);
               CleanupStack::Pop(proxytable));
         if (err != KErrNone) {
            // Cleanup and return no proxy found.
            delete proxytable;
            CleanupStack::PopAndDestroy(iap_type);
            CleanupStack::PopAndDestroy(iap_name);
            CleanupStack::PopAndDestroy(iaptable);
            CleanupStack::PopAndDestroy(comdb);
            return EFalse;
         }
         CleanupStack::PushL(proxytable);
         DBG( "getIAPProxy created proxytable" );
         TInt dretval= proxytable->GotoFirstRecord();
         while ( dretval == KErrNone && !found ) {
            DBG( "getIAPProxy proxy loop" );
            // Check if matching proxy service type
            HBufC* proxy_service_type = proxytable->ReadLongTextLC( 
               TPtrC( PROXY_SERVICE_TYPE ) );
            if ( proxy_service_type != NULL &&
                 proxy_service_type->CompareC( *iap_type ) == 0 ) 
            {
               DBG( "getIAPProxy proxy loop match" );
               // Match!
               // PROXY_USE_PROXY_SERVER 
               TBool proxy_use_proxy_server = 0;
               proxytable->ReadBoolL( TPtrC( PROXY_USE_PROXY_SERVER ), 
                                      proxy_use_proxy_server );
# ifdef NAV2_CLIENT_SERIES60_V2
               if ( proxy_use_proxy_server ) {
# endif
                  DBG( "getIAPProxy proxy loop use proxy" );
                  // PROXY_SERVER_NAME - Name of the proxy server
                  HBufC* proxy_server_name = proxytable->ReadLongTextLC( 
                     TPtrC( PROXY_SERVER_NAME ) );
                  if ( proxy_server_name ) {
                     DBG( "getIAPProxy proxy loop have proxy server" );
                     found = true;
                     // Convert to something we can use.
                     host = TDesCToUtf8L( proxy_server_name->Des() );
                     DBG( "getIAPProxy proxy loop have proxy server name "
                          "%s", host );
                     proxytable->ReadUintL( TPtrC( PROXY_PORT_NUMBER ), 
                                            port );
                     // Sanity on port
                     if ( port == 9201 ) {
                        // We don't talk wap
                        // XXX: Or no proxy at all?
                        port = 8080;
                     } else if ( port == 0 ) {
                        // Not valid => no proxy
                        found = false;
                        delete [] host;
                        host = NULL;
                     }
                  } // End if have proxy_server_name
                  CleanupStack::PopAndDestroy( proxy_server_name );
# ifdef NAV2_CLIENT_SERIES60_V2
               } // End if proxy_use_proxy_server is true
# endif
            } // End if service type matches
            CleanupStack::PopAndDestroy( proxy_service_type );

            // Next proxy
            dretval = proxytable->GotoNextRecord();
         } // End while all proxies

         // XXX: Perhaps "IAP_SERVICE_TYPE" table -> [GPRS|ISP]_IP_GATEWAY 
         // especially in s60v1
         CleanupStack::PopAndDestroy( proxytable );
         CleanupStack::PopAndDestroy( iap_type );
         CleanupStack::PopAndDestroy( iap_name );
      } // End if the current IAP is found (iapres == KErrNone)
      DBG( "getIAPProxy get IAP proxy after end if current IAP is found" );
      CleanupStack::PopAndDestroy( iaptable );
      CleanupStack::PopAndDestroy( comdb );
      DBG( "getIAPProxy get IAP proxy ends" );

#endif // __SYMBIAN32__
      
   } // End if selected IAP is a valid number and we want proxy 

   return found;
}

