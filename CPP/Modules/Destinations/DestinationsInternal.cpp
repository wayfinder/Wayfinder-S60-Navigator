/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* #include "Buffer.h" */
#include "MsgBufferEnums.h"
#include "Module.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "PacketEnums.h"
#include "CtrlHub.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/HelperEnums.h"
#include "GuiProt/AdditionalInfo.h"
#include "NavPacket.h"
#include "NavServerCom.h"

#include "Destinations.h"
#include "DestinationsInternal.h"

#include <arch.h>
#include <stdlib.h>
#include "Serial.h"
#include "RequestList.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "NavTask.h"
#include "MD5Class.h"

#include "Log.h"
#include "Matches.h"
/* #include "GuiProt/GuiProtEnums.h" */
/* #include "ErrorModule.h" */
#include "Buffer.h"
#include "MsgBuffer.h"
#include "TraceMacros.h"
#include <algorithm>
#include "LogMacros.h"
#include "crc32.h"

using namespace std;

namespace isab{

#define DESTINATIONS_FAVORITE_VERSION        2

   
Destinations::Destinations( bool using_fav_ver_2, const char* imei ) :
   Module( "Destinations" ),
   m_paramProvider(NULL),
   m_nscProvider(NULL),
   m_destConsumer(NULL),
   m_shutDown(false),
   m_syncRequestOut(0), m_syncRequestIn(0),
   m_using_fav_ver_2( using_fav_ver_2 ),
   m_favoritesCRC( NULL )
{
   m_favorites = new vector< Favorite* >();
   m_sidefavorites = new vector< Favorite* >();
   m_specials = new vector< GuiProtEnums::AdditionalInfoType >();
   m_removedFavoriteIds = new vector< uint32 >();
   m_destConsumer = new DestinationsConsumerPublic(this);
   m_paramProvider = new ParameterProviderPublic(this);
   m_nscProvider = new NavServerComProviderPublic(this);
   uint32 imeiCrc = crc32::calcCrc32((uint8*)imei, strlen(imei));
   m_imei = new char[9]; // Eight chars and room for ending zero
   sprintf(m_imei, "%08x", imeiCrc);
//    m_imei = strdup_new(imei);
   bool sendError = false;
   m_nextFavoriteId = getNextFavoriteId(*m_favorites, sendError);
   
   // Initing parameters.
   m_sortOrder = GuiProtEnums::invalidSortingType;

   m_favoritesCRC = NULL;
}

Destinations::~Destinations()
{
   delete m_destConsumer;
   delete m_paramProvider;
   delete m_nscProvider;

   if (m_favorites != NULL){
      for (uint32 i = 0; i < m_favorites->size(); i++){
         delete (*m_favorites)[i];
      }
      delete m_favorites;
   }
   for ( uint32 i = 0 ; i < m_sidefavorites->size() ; ++i ) {
      delete (*m_sidefavorites)[ i ];
   }
   delete m_sidefavorites;
   delete m_specials;

   delete m_removedFavoriteIds;
   delete[] m_imei;
}

MsgBuffer* Destinations::dispatch(MsgBuffer* buf)
{
   if(buf) buf = m_destProvDecoder.dispatch ( buf, this );
   if(buf) buf = m_nscDecoder.dispatch    ( buf, this );
   if(buf) buf = m_hubDecoder.dispatch    ( buf, this );
   if(buf) buf = m_paramDecoder.dispatch  ( buf, this );
   if(buf) buf = Module::dispatch         ( buf );
   return buf;
}

void 
Destinations::decodedShutdownNow( int16 upperTimeout )
{
   m_shutDown = true;
   Module::decodedShutdownNow( upperTimeout );
}

void Destinations::decodedStartupComplete()
{
   Module::decodedStartupComplete();


   // List wanted parameters here. Try to keep them in order. They
   // will be processed to join any muticast groups and fetched
   // from the parameter module.
   ParameterEnums::ParamIds params[] = {  
      ParameterEnums::Dest_StoredFavorites,
      ParameterEnums::Dest_StoredFavoritesVer2,
      ParameterEnums::Dest_FavoritesSortingOrder,
      ParameterEnums::Dest_DeletedFavorites,
      ParameterEnums::Dest_SupportedSpecialFavorites,
      ParameterEnums::Dest_StoredFavoritesVer3,
      ParameterEnums::Dest_favoritesCRC,
      ParameterEnums::NSC_favoritesCRC,
      ParameterEnums::Dest_StoredFavoritesVer4,
   };
   // maybe we want to join all parameters, just in case? 
   uint16 mcGroup = 0;
   for(unsigned a = 0; a < sizeof(params)/sizeof(*params); ++a){
      if(ParameterProviderPublic::paramIdIsMulticast(params[a])){
         uint16 g = ParameterProviderPublic::paramIdToMulticastGroup(params[a]);
         if(g != mcGroup){
            mcGroup = g;
            m_rawRootPublic->manageMulticast(JOIN, mcGroup);
        }
      }
         m_paramProvider->getParam(params[a]);
   }
}

void 
Destinations::decodedSolicitedError( const ErrorObj& err, 
                                     uint32 src, uint32 dst )
{
   Nav2Error::ErrorNbr forwardedError = err.getErr();
   DBG("decodedSolicitedError(errorNbr = %#x, src = %#x, dst = %#x)",
       err.getErr(), (uint)src, (uint)dst);
   switch ( err.getErr() ) {
   case Nav2Error::NSC_CANCELED_BY_REQUEST:
      forwardedError = Nav2Error::DEST_SYNC_FAILED;
      break;
   case Nav2Error::NSC_NO_GPS:
   case Nav2Error::NSC_OUT_OF_SERVERS:
   case Nav2Error::NSC_EXPIRED_USER:
   case Nav2Error::NSC_AUTHORIZATION_FAILED:
   case Nav2Error::NSC_OUT_OF_THIS_WORLD:
   case Nav2Error::NSC_OPERATION_NOT_SUPPORTED:
   case Nav2Error::NSC_BEYOND_SALVATION:
   case Nav2Error::NSC_NO_USERNAME:
   case Nav2Error::NSC_SERVER_COMM_TIMEOUT_CONNECTING:
   case Nav2Error::NSC_SERVER_COMM_TIMEOUT_CONNECTED:
   case Nav2Error::NSC_SERVER_COMM_TIMEOUT_DISCONNECTING:
   case Nav2Error::NSC_SERVER_COMM_TIMEOUT_CLEAR:
   case Nav2Error::NSC_SERVER_COMM_TIMEOUT_WAITING_FOR_USER:
   case Nav2Error::NSC_SERVER_NOT_OK:
   case Nav2Error::NSC_SERVER_REQUEST_TIMEOUT:
   case Nav2Error::NSC_SERVER_OUTSIDE_MAP:
   case Nav2Error::NSC_SERVER_NOT_FOUND:
   case Nav2Error::NSC_TRANSPORT_FAILED:
   case Nav2Error::NSC_SERVER_CONNECTION_BROKEN:
   case Nav2Error::NSC_SERVER_UNREACHABLE:
   case Nav2Error::NSC_SERVER_NOT_RESPONDING:
   case Nav2Error::NSC_SERVER_PROTOCOL_ERROR:
   case Nav2Error::NSC_NO_TRANSACTIONS:
   case Nav2Error::NSC_NO_LICENSE_ID:
   case Nav2Error::NSC_SERVER_UNAUTHORIZED_MAP:
   case Nav2Error::NSC_TCP_INTERNAL_ERROR:
   case Nav2Error::NSC_PARAM_REQ_NOT_FIRST:
   case Nav2Error::NSC_REQUEST_ALLOC_FAILED:
   case Nav2Error::NSC_UNAUTH_OTHER_HAS_LICENSE :
   case Nav2Error::NSC_OLD_LICENSE_NOT_IN_ACCOUNT :
   case Nav2Error::NSC_OLD_LICENSE_IN_MANY_ACCOUNTS :
   case Nav2Error::NSC_NEW_LICENSE_IN_MANY_ACCOUNTS :
   case Nav2Error::NSC_OLD_LICENSE_IN_OTHER_ACCOUNT :
   case Nav2Error::NSC_NO_NETWORK_AVAILABLE:
   case Nav2Error::NSC_FLIGHT_MODE:
   case Nav2Error::NSC_NO_GPS_WARN:
   case Nav2Error::NSC_NO_GPS_ERR:
   case Nav2Error::NSC_CANCELED_BY_SHUTDOWN:
   case Nav2Error::NSC_FAKE_CONNECT_TIMEOUT:
   case Nav2Error::NSC_UPGRADE_MUST_CHOOSE_REGION:
      // let forwardedError be errorNbr.
      break;
   case Nav2Error::NSC_SERVER_NEW_VERSION:
   case Nav2Error::NO_ERRORS:
   case Nav2Error::MAX_USER_ERROR:
   case Nav2Error::PANIC_ABORT:
   case Nav2Error::DUMMY_ERROR:
   case Nav2Error::DEST_SYNC_ALREADY_IN_PROGRESS:
   case Nav2Error::DEST_SYNC_FAILED:
   case Nav2Error::DEST_REMOVE_DEST_MISSING_DEST_ID:
   case Nav2Error::DEST_TO_LONG_STRING_IN_DEST:
   case Nav2Error::DEST_DEST_INFO_MISSING_DEST_ID:
   case Nav2Error::DEST_INVALID_FAVORTE_ID_IN_LIST:
   case Nav2Error::NAVTASK_ROUTE_INVALID:
   case Nav2Error::NAVTASK_NSC_OUT_OF_SYNC:
   case Nav2Error::NAVTASK_INTERNAL_ERROR:
   case Nav2Error::NAVTASK_FAR_AWAY:
   case Nav2Error::NAVTASK_CONFUSED:
   case Nav2Error::NAVTASK_ALREADY_DOWNLOADING_ROUTE:
   case Nav2Error::UC_REQUEST_TIMED_OUT:
   case Nav2Error::UC_CONFUSED:
   case Nav2Error::UC_ASKED_ROUTE_FROM_WRONG_MODULE:
   case Nav2Error::UC_INVALID_GUI_REQUEST:
   case Nav2Error::UC_NO_ROUTE:
   case Nav2Error::UC_INVALID_PARAM:
   case Nav2Error::UC_UNKNOWN_PARAM:
   case Nav2Error::UC_UNKNOWN_SEARCH_ID:
   case Nav2Error::INVALID_ERROR_NBR:
   case Nav2Error::NSC_SERVER_NO_ROUTE_FOUND:
   case Nav2Error::NSC_SERVER_ROUTE_TOO_LONG:
   case Nav2Error::NSC_SERVER_BAD_ORIGIN:      
   case Nav2Error::NSC_SERVER_BAD_DESTINATION:
   case Nav2Error::NSC_SERVER_NO_HOTDEST:
   case Nav2Error::GUIPROT_FAILED_GET_TOP_REGION_LIST:     
   case Nav2Error::GUIPROT_FAILED_GET_CALL_CENTER_NUMBERS: //Deprecated
   case Nav2Error::GUIPROT_FAILED_GET_SIMPLE_PARAMETER:    
   case Nav2Error::GUIPROT_FAILED_SET_CALL_CENTER_NUMBERS: //Deprecated
   case Nav2Error::GUIPROT_FAILED_SET_SIMPLE_PARAMETER:    
   case Nav2Error::GUIPROT_FAILED_GET_FAVORITES:           
   case Nav2Error::GUIPROT_FAILED_GET_FAVORITES_ALL_DATA:  
   case Nav2Error::GUIPROT_FAILED_SORT_FAVORITES:          
   case Nav2Error::GUIPROT_FAILED_SYNC_FAVORITES:          
   case Nav2Error::GUIPROT_FAILED_GET_FAVORITE_INFO:       
   case Nav2Error::GUIPROT_FAILED_ADD_FAVORITE:            
   case Nav2Error::GUIPROT_FAILED_ADD_FAVORITE_FROM_SEARCH:
   case Nav2Error::GUIPROT_FAILED_REMOVE_FAVORITE:         
   case Nav2Error::GUIPROT_FAILED_CHANGE_FAVORITE:         
   case Nav2Error::GUIPROT_FAILED_ROUTE_TO_FAVORITE:       
   case Nav2Error::GUIPROT_FAILED_ROUTE_TO_HOT_DEST:       
   case Nav2Error::GUIPROT_FAILED_DISCONNECT_GPS:          
   case Nav2Error::GUIPROT_FAILED_CONNECT_GPS:             
   case Nav2Error::NAVTASK_NO_ROUTE:
   case Nav2Error::PARAM_NO_OPEN_FILE:
   case Nav2Error::PARAM_NO_READ_LINE:
   case Nav2Error::PARAM_CORRUPT_BINARYBLOCK:
   case Nav2Error::PARAM_CORRUPT_STRING:
   case Nav2Error::PARAM_CORRUPT_SYNC:
   case Nav2Error::PARAM_CORRUPT_TYPE:
   case Nav2Error::PARAM_TRYING_BACKUP:
   case Nav2Error::PARAM_TRYING_ORIGINAL:
   case Nav2Error::PARAM_NO_VALID_PARAM_FILE:
   case Nav2Error::PARAM_NO_SPACE_ON_DEVICE:
   case Nav2Error::PARAM_NO_SPACE_ON_DEVICE_2:
   case Nav2Error::NSC_NO_ROUTE_RIGHTS:
   case Nav2Error::NSC_TCP_INTERNAL_ERROR2:
      return;
   } // switch

   // Return error on sync request.
   DBG("dst: %#x %c= m_syncRequestOut: %#x", unsigned(dst),
       dst == m_syncRequestOut ? '=' : '!' , unsigned(m_syncRequestOut));
   if(dst == m_syncRequestOut){
      ErrorObj error( err );
      error.setErr( forwardedError );
      m_destConsumer->solicitedError( error, m_syncRequestIn );
      m_syncRequestIn = m_syncRequestOut = 0;
   }

}
          
void Destinations::decodedGetFavorites(uint16 startIdx, uint16 endIdx, 
                                       uint32 src)
{
   DBG("Destinations::decodedGetFavorites startIdx: %d, endIdx: %d", startIdx, endIdx);
   if (endIdx == MAX_UINT16){
      // All favorites to the end of the list are asked for.
      endIdx = m_favorites->size();
   }
   std::vector<GuiFavorite*> guiFavorites;
   uint32 i = MAX_UINT32;
   for (i = startIdx; i < endIdx; i++){
      guiFavorites.push_back( (*m_favorites)[i]->getGuiFavorite());
   }
   DBG("decodedGetFavorites - sending");
   m_destConsumer->getFavoritesReply(guiFavorites, src);
   DBG("decodedGetFavorites - sending done");
   
   for ( i = 0; i < guiFavorites.size(); i++){
      delete guiFavorites[i];
   }
   DBG("decodedGetFavorites - end");
} // decodedGetFavorites
          
void Destinations::decodedGetFavoritesAllData(uint16 startIdx, 
                                              uint16 endIdx, 
                                              uint32 src)
{
   DBG("Destinations::decodedGetFavoritesAllData. startIdx: %d, endIdx: %d", startIdx, endIdx);
   std::vector<Favorite*> *favsToReturn = new vector<Favorite*>;
      
   if (endIdx == MAX_UINT16){
      endIdx = m_favorites->size();
   }
   for ( uint32 i = startIdx ; i < endIdx ; ++i ) {
      favsToReturn->push_back( (*m_favorites)[i] );
   }
   m_destConsumer->getFavoritesAllDataReply(*favsToReturn, src);
   delete favsToReturn;
} // decodedGetFavoritesAllData
          
void Destinations::decodedGetFavoriteInfo(uint32 favoriteId, 
                                       uint32 src)
{
   DBG("GetFavoriteInfo for favorite with id: %d.", (uint)favoriteId);
   bool found = false;
   uint32 i = 0;
   while ( (!found) && (m_favorites != NULL) && (i < m_favorites->size()) ){
      found = ( (*m_favorites)[i]->getID() == favoriteId);

      if (!found){
         i++;
      }
   }
   if (found){
      m_destConsumer->getFavoriteInfoReply( (*m_favorites)[i], src);
   }
   else{
      WARN("GetFavoriteInfo. Favorite id does not exist. ID = %u", (uint)favoriteId);
      m_destConsumer->solicitedError(Nav2Error::DEST_DEST_INFO_MISSING_DEST_ID, src);
   }
} // decodedGetFavoriteInfo

Nav2Error::ErrorNbr
Destinations::sortFavorites(GuiProtEnums::SortingType sortingOrder)
{
   Nav2Error::ErrorNbr result = Nav2Error::NO_ERRORS;
   DBG("Destinations::sortFavorites");
   
   sort( m_favorites->begin(), m_favorites->end(),
         isab::FavoriteCmp(sortingOrder));

   return result;
} // sortFavorites

void Destinations::saveFavorites(vector<Favorite*>& favorites)
{
   DBG("Destinations::saveFavorites");
   
   Buffer buf;
   uint16 nbrOfFavorites = favorites.size();
#if defined (NAV2_USE_UTF8)
   buf.writeNextUnaligned16bit(DESTINATIONS_FAVORITE_VERSION);
#endif
   buf.writeNextUnaligned16bit(nbrOfFavorites);
   for (uint i = 0; i < nbrOfFavorites; i++){
      favorites[i]->serialize( &buf, 
#ifdef NAV2_USE_UTF8
                               4
#else
                               1
#endif
                               );
   }
#if defined (NAV2_USE_UTF8)
   m_paramProvider->setParam( ParameterEnums::Dest_StoredFavoritesVer4,
                              buf.accessRawData(), buf.getLength() );
#else
   m_paramProvider->setParam(ParameterEnums::Dest_StoredFavorites,
         buf.accessRawData(), buf.getLength());
#endif

   if(m_removedFavoriteIds->empty()){
      /* No deleted ids, empty parameter*/
      DBG("Clearing DeletedFavorites parameter");
      m_paramProvider->clearParam(ParameterEnums::Dest_DeletedFavorites);
   } else {
      /* Got deleted ids. Save them. */
      DBG("Saving deleted ids.");
      Buffer dbuf;
      uint16 numDeleted = m_removedFavoriteIds->size();
      dbuf.writeNextUnaligned16bit(numDeleted);
      for (uint j = 0; j < numDeleted; j++){
         dbuf.writeNextUnaligned32bit((*m_removedFavoriteIds)[j]);
      }
      m_paramProvider->setParam(ParameterEnums::Dest_DeletedFavorites,
            dbuf.accessRawData(), dbuf.getLength());
   }
   // Remove param 003d8005 (Dest_StoredFavoritesVer3) since all favorites
   // will be added in 003d8007 (Dest_StoredFavoritesVer4) and we don't want
   // any duplicates.
   m_paramProvider->clearParam(ParameterEnums::Dest_StoredFavoritesVer3);
   DBG("Saved favorites");
}

uint32 Destinations::getNextFavoriteId(vector<Favorite*>& favorites, bool sendError)
{
   uint32 result = LOWEST_CLIENT_FAV_ID;

   for (uint32 i = 0; i < favorites.size(); i++){
      uint32 currFavID = favorites[i]->getID();
      if(currFavID >= result){
         result = currFavID + 1;
         
         // Error check.
         if (currFavID == MAX_UINT32){
            WARN("Favorite with id MAX_UINT32 when looking for next favorite id.");
            if (sendError){
               unsolicitedError(Nav2Error::DEST_INVALID_FAVORTE_ID_IN_LIST);
            }
            else{
               DBG("Not sending DEST_INVALID_FAVORTE_ID_IN_LIST error.");
            }
         }
      }
   }

   DBG("Next favorite id = %u", (uint)result);
   return result;
}

void Destinations::decodedSortFavorites(GuiProtEnums::SortingType sortingOrder,
                                        uint32 src)
{
   DBG("SortFavorites. sortingOrder: %d", sortingOrder);
   WARN("SortFavorites. Only alphabetical order on name implemented. "
           "This sorting order will be used.");

   
   m_sortOrder = sortingOrder;
   Nav2Error::ErrorNbr error = sortFavorites(m_sortOrder);
   
   if (error == Nav2Error::NO_ERRORS){
      m_destConsumer->destRequestOk(src);
   }
   else {
      m_destConsumer->solicitedError(error, src);
   }
} // decodedSortFavorites

void Destinations::decodedSyncFavorites( uint32 src)
{
   DBG("decodedSyncFavorites(%#010"PRIx32")",src); 
   if(m_syncRequestOut == 0){
      std::vector<Favorite*> syncFav;
      std::vector<Favorite*> addFav;
      NParamBlock params;
      // Nosync false
      params.addParam( NParam( 4800, uint32( 0 ) ) );
      std::vector<Favorite*>::iterator q;
      for( q = m_favorites->begin() ; q != m_favorites->end() ; ++q ) {
         DBG( "syncing: id: %#010"PRIx32" name: %s", (*q)->getID(),
              (*q)->getName() );
         if ( (*q)->isSynced()){
            syncFav.push_back( *q );
         } else {
            addFav.push_back( *q );
         }
      }
      for( q = m_sidefavorites->begin() ; q != m_sidefavorites->end() ; 
           ++q ) 
      {
         syncFav.push_back( *q );
      }
      if ( syncFav.size() > 0 ) {
         NParam& paddFavIDs = params.addParam( NParam( 4801 ) );
         for( q = syncFav.begin() ; q != syncFav.end() ; ++q ) {
            paddFavIDs.addUint32( (*q)->getID() );
         }
      }
      if ( addFav.size() > 0 ) {
         NParam& paddFavs = params.addParam( NParam( 4803 ) );
         for( q = addFav.begin() ; q != addFav.end() ; ++q ) {
            paddFavs.addUint32( 0 ); // ID
            paddFavs.addInt32( (*q)->getLat() );
            paddFavs.addInt32( (*q)->getLon() );
            paddFavs.addString( (*q)->getName() );
            paddFavs.addString( (*q)->getShortName() );
            paddFavs.addString( (*q)->getDescription() );
            paddFavs.addString( (*q)->getCategory() );
            paddFavs.addString( (*q)->getMapIconName() );
            if ( m_using_fav_ver_2 ) {
               const Favorite::InfoVect& infos = (*q)->getInfos();
#define MIN_that_can(a, b)  (((a) < (b)) ? (a) : (b))
               uint16 size = MIN_that_can( infos.size(), size_t(MAX_UINT16) );
               paddFavs.addUint16( size );
               for ( uint32 i = 0 ; i < size ; ++i ) {
                  paddFavs.addUint16( infos[ i ]->getType() );
                  paddFavs.addString( infos[ i ]->getKey() );
                  paddFavs.addString( infos[ i ]->getValue() );
               }
            }
         } // End for all add favorites
      }

      if ( m_removedFavoriteIds->size() > 0 ) {
         NParam& pdelFavIDs = params.addParam( NParam( 4802 ) );
         for(unsigned j = 0; j < m_removedFavoriteIds->size(); ++j){
            DBG("syncing: remove %#010"PRIx32, (*m_removedFavoriteIds)[j]);
            pdelFavIDs.addUint32( (*m_removedFavoriteIds)[ j ] );
         }
      }

      uint16 reqVer = 1;
      if ( m_using_fav_ver_2 ) {
         reqVer = 3; // Three is two with favoritesCRC
      }
      m_syncRequestOut = m_nscProvider->sendRequest( 
         navRequestType::NAV_FAV_REQ, params, reqVer );

      m_syncRequestIn = src;
      DBG("Sending sync request to server reqid: 0x%08"PRIx32, 
          m_syncRequestOut);
      DBG("decodedSyncFavorites. m_syncRequestIn = 0x%08x", (uint)m_syncRequestIn);
   } else {
      m_destConsumer->solicitedError(Nav2Error::DEST_SYNC_ALREADY_IN_PROGRESS, 
                                     src);

      DBG("Sync %#010"PRIx32" already in progress.", m_syncRequestOut);
   }
} // decodedSyncFavorites

void
Destinations::decodedSyncDestinationsReply(std::vector<Favorite*>& favs, 
                                           std::vector<uint32>& removed, 
                                           uint32 /*src*/, uint32 /*dst*/)
{
   DBG("Destinations::decodedSyncDestinationsReply, %u favs, %u removed",
       favs.size(), removed.size());
   //TRACE_DBG( "decodedSyncDestinationsReply" );
   favIt q;
   typedef std::vector<uint32>::iterator u32It;
   //need the vector of removed id's to be sorted.
   std::sort(removed.begin(), removed.end());
   //need the vector of new favorites to be sorted.
   std::sort(favs.begin(), favs.end(), FavoriteCmp(m_sortOrder));
   
   //a temporary vector for the old favorites before we merge sort them all.
   std::vector<Favorite*> tmp;
   tmp.reserve(m_favorites->size()+favs.size());
   //search the old favorites vectror for favorites to delete
   for ( q = m_favorites->begin() ; q != m_favorites->end() ; ++q ) {
      if(!(*q)->isSynced()){
         DBG("deleting local fav %#x", (uint)(*q)->getID());
         delete *q; //This fav shall be removed. A valid copy is
         *q = NULL; //provided by the server in the favs vector.
      } else if(!removed.empty()){
         DBG("Removed is not empty.");
         //the removed vector contains ids of favorites in m_favorites
         //that shall be removed. We do a binary search in the removed
         //vector to see if the favorite *q shall be removed.
         // Also removed used later to remove ids from m_removedFavoriteIds
         // equal_rane is O( 2*log( n ) ), binary_search is O ( log ( n ) )
         if ( std::binary_search( removed.begin(), removed.end(), 
                                  (*q)->getID() ) )
         {
            DBG("Favorite %#x deleted", (uint)(*q)->getID());
            // q should be deleted.
            delete *q;
            *q = NULL;
         }
      }
      if(*q){
         DBG("Keeping fav %#x", (uint)(*q)->getID());
         //*q shall be kept. Move it to tmp. Also check for maxid.
         tmp.push_back(*q);
         *q = NULL;
      }
   }
   for ( q = m_sidefavorites->begin() ; q != m_sidefavorites->end() ; ++q )
   {
      if ( std::binary_search( removed.begin(), removed.end(), 
                               (*q)->getID() ) )
      {
         DBG("Special Favorite %#x deleted", (uint)(*q)->getID());
         // q should be deleted.
         delete *q;
      } else {
         // Out of sorted order! But moved aside later so it is ok.
         tmp.push_back( *q );
      }
      *q = NULL;
   }
   m_sidefavorites->clear();

   int size = tmp.size() + favs.size();
   m_favorites->reserve(size);
   m_favorites->resize(size, NULL);
   m_sidefavorites->reserve( size );
   //merge sort the vectors tmp and favs into m_favorites.
   std::merge(tmp.begin(), tmp.end(), favs.begin(), favs.end(), 
              m_favorites->begin(), FavoriteCmp(m_sortOrder));

//    for ( q = m_favorites->begin() ; q != m_favorites->end() ; ++q ) {
//       TRACE_FUNC1( (*q)->getName() );
//    }

   // Move special favorites aside.
   moveSpecialsAside( m_favorites, m_sidefavorites );

   std::sort(m_removedFavoriteIds->begin(), m_removedFavoriteIds->end());
   DBG("Remove from locally removed vector. local: %u, server: %u",
       m_removedFavoriteIds->size(), removed.size());
   u32It end = m_removedFavoriteIds->end();
   for(u32It w = removed.begin(); w != removed.end(); ++w){
      DBG("removing id %#x from removed list", (uint)*w);
      end = remove_if(m_removedFavoriteIds->begin(), end, 
                      bind1st(equal_to<uint32>(), *w));
   }
   m_removedFavoriteIds->erase(end, m_removedFavoriteIds->end());
   if(!m_removedFavoriteIds->empty()){
      WARN("Still deleted favs on list after sync.");
   }

   m_nextFavoriteId = getNextFavoriteId(*m_favorites);

   //notify uictrl.
   DBG("Sending dest ok to UiCtrl. m_syncRequestIn = 0x%08x", (uint)m_syncRequestIn);
   m_destConsumer->destRequestOk(m_syncRequestIn);
   favoritesChanged();

   //bookkeeping.
   m_syncRequestIn = m_syncRequestOut = 0;
   
}


void 
Destinations::decodedReply( 
   navRequestType::RequestType type, NParamBlock& params, 
   uint8 requestVer, uint8 /*statusCode*/, const char* /*statusMessage*/,
   uint32 src, uint32 dst )
{
   DBG("decodedNavReply dst = 0x%08"PRIx32, dst);

   switch ( type ) {
      case navRequestType::NAV_FAV_REPLY : {
         // NGP favorite data
         std::vector<Favorite*> favs;
         std::vector<uint32> removed;
         
         if ( params.getParam( 4900 ) ) {
            vector< const NParam* > pas;
            params.getAllParams( 4900, pas );
            Buffer buff( 4096 );
            uint32 lmID = MAX_UINT32;
            for ( uint32 i = 0 ; i < pas.size() ; ++i ) {
               uint32 pos = 0;
               while ( pos < pas[ i ]->getLength() ) {
                  lmID = MAX_UINT32;
                  buff.clear();
                  buff.writeNext8bit( 1 ); // synced
                  // Id
                  buff.writeNextUnaligned32bit( pas[ i ]->getUint32( pos ) );
                  pos += 4;
                  uint32 lat = pas[ i ]->getUint32( pos );
                  buff.writeNextUnaligned32bit( lat ); 
                  pos += 4;
                  uint32 lon = pas[ i ]->getUint32( pos );
                  buff.writeNextUnaligned32bit( lon ); 
                  pos += 4;
                  // Name
                  const char* name = pas[ i ]->getString( pos );
                  buff.writeNextCharString( name ); 
                  //TRACE_FUNC1( pas[ i ]->getString( pos ) );
                  pos += strlen( pas[ i ]->getString( pos ) ) + 1;
                  // Short name
                  buff.writeNextCharString( pas[ i ]->getString( pos ) ); 
                  pos += strlen( pas[ i ]->getString( pos ) ) + 1;
                  // Description
                  const char* desc = pas[ i ]->getString( pos );
                  buff.writeNextCharString( desc ); 
                  pos += strlen( pas[ i ]->getString( pos ) ) + 1;
                  // Category
                  buff.writeNextCharString( pas[ i ]->getString( pos ) ); 
                  pos += strlen( pas[ i ]->getString( pos ) ) + 1;
                  // MapIcon
                  buff.writeNextCharString( pas[ i ]->getString( pos ) ); 
                  pos += strlen( pas[ i ]->getString( pos ) ) + 1;
                  if ( requestVer > 1 ) {
                     uint16 nbrInfos = pas[ i ]->getUint16( pos );
                     // Nbr infos
                     pos += 2;
                     buff.writeNextUnaligned32bit( uint32( nbrInfos ) );
                     for ( uint32 j = 0 ; j < nbrInfos ; ++j ) {
                        // Type
                        buff.writeNextUnaligned32bit( 
                           uint32( pas[ i ]->getUint16( pos ) ) );
                        pos += 2;
                        // Key
                        const char* key = pas[ i ]->getString( pos );
                        buff.writeNextCharString( key );
                        pos += strlen( pas[ i ]->getString( pos ) ) + 1;
                        // Value
                        const char* value = pas[ i ]->getString( pos );
                        buff.writeNextCharString( value );
                        pos += strlen( pas[ i ]->getString( pos ) ) + 1;
                        if (strcmp(key, "lmid") == 0 &&
                            strncmp(m_imei, value, strlen(m_imei)) == 0) {
                           // Found a lmid key, check the value
                           // Found a matching imei, check the lmID
                           sscanf(value + 8, "%x", &lmID);
                        }
                     }
                  } else {
                     buff.writeNextUnaligned32bit( 0 );
                  }
                  // For all favorites that hasn't been added yet calc the md5sum and
                  // set dummy values for lmID and lmsID
                  buff.writeNextUnaligned32bit(lmID);
                  buff.writeNextCharString("");
                  uint8* md5sum = Favorite::calcMd5Sum(name, desc, lat, lon);
                  buff.writeNextByteArray(md5sum, Favorite::MAX_SIZE_MD5SUM);
                  delete[] md5sum;

                  // Make Favorite
                  favs.push_back( new Favorite( &buff, 0, 4 ) );
               } // while more in param
            } // For all 4900 params
         } // End if 4900 param

         if ( params.getParam( 4901 ) ) {
            const NParam* pd = params.getParam( 4901 );
            for ( uint16 i = 0 ; i < pd->getLength() / 4 ; ++i ) {
               removed.push_back( pd->getUint32Array( i ) );
            }
         }

         // Favorites CRC
         if ( params.getParam( 4903 ) ) {
            const char* crcStr = params.getParam( 4903 )->getString();
            delete [] m_favoritesCRC;
            m_favoritesCRC = strdup_new( crcStr );
            setDestFavoritesCRC( crcStr );
            // Also set server CRC as this is from server.
            m_paramProvider->setParam( ParameterEnums::NSC_favoritesCRC,
                                       &crcStr );
         }
         
         decodedSyncDestinationsReply( favs, removed, src, dst );
      } break;
            
      default:
         /* Ignore unknown packets. */
         break;
   } // End switch type
}


      
void Destinations::decodedAddFavorite( Favorite* favorite, 
                                          uint32 src)
{
   Nav2Error::ErrorNbr error = addFavorite(favorite);
   
   
   if (error == Nav2Error::NO_ERRORS){
      m_destConsumer->destRequestOk(src);
      setDestFavoritesCRC( "" ); // Not in sync with server
      favoritesChanged();
   }
   else {
      m_destConsumer->solicitedError(error, src);
   }

   
} // decodedAddFavorite
         
void Destinations::decodedRemoveFavorite( uint32 favoriteId, 
                                          uint32 src)
{
   DBG("RemoveFavorite. favoriteId: %u", (uint)favoriteId);
   Nav2Error::ErrorNbr error = removeFavorite(favoriteId);
   if (error == Nav2Error::NO_ERRORS){
      m_destConsumer->destRequestOk(src);
      setDestFavoritesCRC( "" ); // Not in sync with server
      favoritesChanged();
   }
   else{
      m_destConsumer->solicitedError(error, src);
   }
} // decodedRemoveFavorite

void Destinations::decodedChangeFavorite( Favorite* favorite, 
                                          uint32 src)
{
   DBG("ChangeFavorite. favorite->getID(): %u", (uint)favorite->getID());
   
   Nav2Error::ErrorNbr error = removeFavorite(favorite->getID());
   if (error == Nav2Error::NO_ERRORS){
      error = addFavorite(favorite);
      if (error == Nav2Error::NO_ERRORS){
         m_destConsumer->destRequestOk(src);
         setDestFavoritesCRC( "" ); // Not in sync with server
         favoritesChanged();
      }
      else{
         m_destConsumer->solicitedError(error, src);
      }
   }
   else{
      m_destConsumer->solicitedError(error, src);
   }
   
} // decodedChangeFavorite

Nav2Error::ErrorNbr Destinations::addFavorite(Favorite* favorite)
{
   Nav2Error::ErrorNbr result = Nav2Error::NO_ERRORS;

   if( !checkFavoriteStringLengths(favorite) ){
      result = Nav2Error::DEST_TO_LONG_STRING_IN_DEST;
   }
   else{

      DBG("Adding favortie with id: m_nextFavoriteId: %u", (uint)m_nextFavoriteId);
      favorite->setID(m_nextFavoriteId++);
      m_favorites->push_back(favorite);
      result = sortFavorites(m_sortOrder);
   }

   return result;
} // addFavorite
 
Nav2Error::ErrorNbr Destinations::removeFavorite(uint32 favoriteId)
{
   Nav2Error::ErrorNbr result = Nav2Error::NO_ERRORS;
   
   bool found = false;
   vector<Favorite*>::iterator it = m_favorites->begin();
   while ( (!found) && (it < m_favorites->end() ) ){
      found = ( (*it)->getID() == favoriteId );
      if (!found){
         ++it;
      }
   }
   if (found){
      delete (*it);
      (*it) = NULL;
      m_favorites->erase(it);
      m_removedFavoriteIds->push_back(favoriteId);
   } else {
      WARN("Could not find the favorite to remove/change.");
      result = Nav2Error::DEST_REMOVE_DEST_MISSING_DEST_ID;
   }

   return result;
} // removeFavorite

void Destinations::favoritesChanged(bool writeToParam)
{
   MsgBuffer *buf = new MsgBuffer(DestMessages | MsgBufferEnums::ADDR_MULTICAST_UP,
                                  this->getRequestId(), 
                                  MsgBufferEnums::FAVORITES_CHANGED_MSG, 
                                  64 );
   m_queue->insert(buf);

   // Only save if the favorites are unchanged for 1 minute.
   if (writeToParam) {
      favVect favs;
      int size = m_favorites->size() + m_sidefavorites->size();
      favs.resize( size, NULL );
      std::merge( m_favorites->begin(), m_favorites->end(), 
                  m_sidefavorites->begin(), m_sidefavorites->end(), 
                  favs.begin(), FavoriteCmp( m_sortOrder ) );
      saveFavorites( favs );
   }
}

void 
Destinations::decodedBinaryUploadReply( uint32 /*src*/, uint32 /*dst*/ )
{
}
void 
Destinations::decodedBinaryDownload( const uint8* /*data*/, 
                                    size_t /*length*/, 
                                    uint32 /*src*/, uint32 /*dst*/ )
{
}
void 
Destinations::decodedWhereAmIReply( const char* /*country*/, 
                                    const char* /*municipal*/, 
                                    const char* /*city*/, 
                                    const char* /*district*/, 
                                    const char* /*streetname*/, 
                                    int32 /*lat*/, int32 /*lon*/, 
                                    uint32 /*src*/, uint32 /*dst*/ )
{
}

void 
Destinations::decodedRouteReply( uint32 /*src*/, uint32 /*dst*/ )
{
}

void 
Destinations::decodedProgressMessage(NavServerComEnums::ComStatus /*status*/, 
                                     GuiProtEnums::ServerActionType /*type*/,
                                     uint32 /*done*/, uint32 /*of*/,
                                     uint32 /*src*/, uint32 /*dst*/ )
{
}

void 
Destinations::decodedParamNoValue(uint32 paramId,
                                  uint32 /*src*/,
                                  uint32 /*dst*/)  
{
   // Default values for different paramters
   GuiProtEnums::SortingType defaultSortOrder = 
      GuiProtEnums::newSort;

   switch(paramId){
   case ParameterEnums::Dest_DeletedFavorites:
      {
         DBG("No deleted favorite ids were loaded from the parameter file.");
      } break;
   case ParameterEnums::Dest_StoredFavorites:
      {
         DBG("No deprecated favorites were loaded from the parameter file.");
      } break;
   case ParameterEnums::Dest_StoredFavoritesVer2:
      {
         DBG("No favorites were loaded from the parameter file.");
      } break;
   case ParameterEnums::Dest_FavoritesSortingOrder:
      {
         m_sortOrder = defaultSortOrder;
         int32 tmpSortOrder = m_sortOrder;
         m_paramProvider->setParam(ParameterEnums::ParamIds(paramId), &tmpSortOrder );
         DBG("Using default sorting order for favorites. "
             "sorting type = %u", 
             static_cast<uint16>(m_sortOrder));         
      } break;
      case ParameterEnums::Dest_SupportedSpecialFavorites: {
         DBG("No supported special favorites were loaded.");
      } break;

      case ParameterEnums::Dest_favoritesCRC: {
         setDestFavoritesCRC( "" );
      } break;
   default:
      WARN("Received unknown %s parameter 0x%x", "no value", (uint)paramId);
   } // switch
}

void 
Destinations::decodedParamValue(uint32 paramId,
                               const int32 * data,
                               int32 numEntries,
                               uint32 /*src*/,
                               uint32 /*dst*/)
{
   switch(paramId){
      case ParameterEnums::Dest_FavoritesSortingOrder:
         {
            m_sortOrder = static_cast<GuiProtEnums::SortingType>(data[0]);
            DBG("Got sorting order for favorites. "
                "sorting type = %u", 
                static_cast<uint16>(m_sortOrder));         
            sortFavorites(m_sortOrder);
         } break;
      case ParameterEnums::Dest_SupportedSpecialFavorites: {
         for ( int32 n = 0 ; n < numEntries ; ++n ) {
            m_specials->push_back( GuiProtEnums::AdditionalInfoType( 
                                      data[ n ] ) );
         }
      } break;
#ifdef __VC32__

   case 0: // fall through. avoids compiler warning.

#endif

   default:
      WARN("Received unknown %s parameter 0x%x", "int32", (uint)paramId);
   }
}
void 
Destinations::decodedParamValue(uint32 paramId,
                               const float * /*data*/,
                               int32 /*numEntries*/,
                               uint32 /*src*/,
                               uint32 /*dst*/)
{
   switch(paramId){
#ifdef __VC32__

   case 0: // fall through. avoids compiler warning.

#endif

   default:
      WARN("Received unknown %s parameter 0x%x", "float", (uint)paramId);
   }
}
void 
Destinations::decodedParamValue(uint32 paramId,
                               const char * const * data,
                               int32 /*numEntries*/,
                               uint32 /*src*/,
                               uint32 /*dst*/)
{
   switch ( paramId ) {
      case ParameterEnums::Dest_favoritesCRC: {
         //TRACE_DBG( "Dest %s", *data );
         setFavoritesCRC( *data );
      } break;
      case ParameterEnums::NSC_favoritesCRC: {
         //TRACE_DBG( "Got %s have %s", *data, getFavoritesCRC() );
         if ( getFavoritesCRC() != NULL && 
              (getFavoritesCRC()[ 0 ] == '\0'||
               strcmp( getFavoritesCRC(), *data ) != 0) )
         {
            // Sync!
            decodedSyncFavorites( 0/*src*/ );
         }
      } break;

      default:
         WARN("Received unknown %s parameter 0x%x", "string", (uint)paramId);
   }
}
void Destinations::decodedParamValue (uint32 paramId, const uint8 *data, 
                                      int size, uint32 /*src*/, uint32 /*dst*/)
{
   // This is ok because we release the data before
   // deleting the buffer.
   uint8* notConstData = const_cast<uint8*>(data);
   Buffer* buf = new Buffer(notConstData, size, size);
   notConstData = NULL;

   switch(paramId){
   case ParameterEnums::Dest_DeletedFavorites:
      {
         DBG("got deleted favorite ids");
         uint16 nbrOfFavorites = buf->readNextUnaligned16bit();
         uint16 i = 0;
         for (i = 0; i < nbrOfFavorites; i++){
            m_removedFavoriteIds->push_back(buf->readNextUnaligned32bit());
         }
      } break;
   case ParameterEnums::Dest_StoredFavorites:
      {
         /* Deprecated favorite version. */
         DBG("Detected deprecated favorites version");
         uint16 nbrOfFavorites = buf->readNextUnaligned16bit();
         uint16 i = 0;
         for (i = 0; i < nbrOfFavorites; i++){
            m_favorites->push_back( new Favorite( buf, 1, 1 ) );
         }

         sortFavorites(m_sortOrder);
         m_nextFavoriteId = getNextFavoriteId(*m_favorites);
         favoritesChanged(false);

#if defined (NAV2_USE_UTF8)
         saveFavorites(*m_favorites);
         m_paramProvider->clearParam(ParameterEnums::Dest_StoredFavorites);   
#endif

      } break;
   case ParameterEnums::Dest_StoredFavoritesVer2:
   case ParameterEnums::Dest_StoredFavoritesVer3:
   case ParameterEnums::Dest_StoredFavoritesVer4:
      {
#if defined (NAV2_USE_UTF8)
         uint16 srvversion = buf->readNextUnaligned16bit();
         if ( srvversion != DESTINATIONS_FAVORITE_VERSION ) {
            /* Non compatible version for favorites. */
            DBG("Non compatible version for favorites");
            break;
         }
         int version = 0;
         switch (paramId) {
         case ParameterEnums::Dest_StoredFavoritesVer2:
            version = 2;
            break;
         case ParameterEnums::Dest_StoredFavoritesVer3:
            version = 3;
            break;
         case ParameterEnums::Dest_StoredFavoritesVer4:
            version = 4;
            break;
         }
         uint16 nbrOfFavorites = buf->readNextUnaligned16bit();
         uint16 i = 0;
         for ( i = 0 ; i < nbrOfFavorites ; ++i ) {
            m_favorites->push_back( new Favorite( buf, 0, version ) );
            // Get the newly added favorite and check if md5sum is set,
            // if not calc the md5sum by sending NULL as parameter to 
            // Favorite::setMd5sum
            Favorite* fav = m_favorites->back();
            if (!fav->isMd5sumSet()) {
               fav->setMd5sum(NULL);
            }
         }

         sortFavorites( m_sortOrder );
         moveSpecialsAside( m_favorites, m_sidefavorites );
         m_nextFavoriteId = getNextFavoriteId( *m_favorites );
         favoritesChanged( version >= 4 );
#else
         /* Downgrading is not supported with migrating favorites. */
         m_paramProvider->clearParam( ParameterEnums::ParamIds( paramId ) );
#endif
      } break;
   default:
      WARN("Received unknown %s parameter 0x%x", "BLOB", (uint)paramId);
   }

   buf->releaseData();
   delete buf;
   buf = NULL;
}
  
   void Destinations::setCharValue(char*& dst, const char* src)
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

bool
Destinations::checkFavoriteStringLengths(Favorite* favorite)
{
   bool result = true;

   if ( strlen(favorite->getName()) <
        Favorite::MAX_SIZE_NAME ){
      if ( strlen(favorite->getShortName()) <
        Favorite::MAX_SIZE_SHORTNAME ){
         if ( strlen(favorite->getDescription()) <
           Favorite::MAX_SIZE_DESCRIPTION ){
            if ( strlen(favorite->getCategory()) <
              Favorite::MAX_SIZE_CATEGORY ){
               if ( strlen(favorite->getMapIconName()) <
                 Favorite::MAX_SIZE_MAPICONPATH ){
               }
               else{
                  result = false;
                  DBG("Map icon name of favorite is to long. "
                      "Map icon name = %s.",
                      favorite->getMapIconName());
               }
            }
            else{
               result = false;
               DBG("Category of favorite is to long. "
                   "Category = %s.",
                   favorite->getCategory());
            }
         }
         else{
            result = false;
            DBG("Description of favorite is to long. "
                "Description = %s.",
                favorite->getDescription());
         }
      }
      else{
         result = false;
         DBG("Short name of favorite is to long. "
             "Short name = %s.",
             favorite->getShortName());
      }
   }
   else{
      result = false;
      DBG("Name of favorite is to long. "
          "Name= %s.",
          favorite->getName());
   }
   
   return result;
} // checkFavoriteStringLengths


void
Destinations::moveSpecialsAside( favVect* favorites, favVect* sidefavorites ) {
   for ( favIt q = favorites->begin() ; q != favorites->end() ; /**/ ) {
      if ( (*q)->hasInfoType( GuiProtEnums::special_flag ) ) {
         bool supported = false;
         for ( specialsVect::const_iterator sit = m_specials->begin() ;
               sit != m_specials->end() ; ++sit )
         {
            if ( (*q)->hasInfoType( *sit ) ) {
               supported = true;
               break;
            }
         }
         if ( !supported ) {
            sidefavorites->push_back( *q );
            q = favorites->erase( q );
         } else {
            ++q;
         }
      } else {
         ++q;
      }
   }
}

void
Destinations::setFavoritesCRC( const char* s ) {
   delete [] m_favoritesCRC;
   m_favoritesCRC = strdup_new( s );
}

const char*
Destinations::getFavoritesCRC() const {
   return m_favoritesCRC;
}

void
Destinations::setDestFavoritesCRC( const char* s ) {
   m_paramProvider->setParam( ParameterEnums::Dest_favoritesCRC, &s );
}

} // End namespace isab
