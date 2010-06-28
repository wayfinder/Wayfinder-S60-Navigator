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

#include "Buffer.h"
#include "MsgBuffer.h"

#include "GuiProt/ServerEnums.h"
#include "Destinations.h"

// Logging macros
#include "LogMacros.h"

using namespace std;

/* using namespace isab::DestinationsEnums; */
namespace isab {


MsgBuffer* 
DestinationsProviderDecoder::dispatch(MsgBuffer *buf,
                                      DestinationsProviderInterface *m )
{
   uint32 src = buf->getSource();

   switch (buf->getMsgType()) {
   case MsgBufferEnums::GET_FAVORITES_REQ:
      {
         const uint16 startIndex = buf->readNextUnaligned16bit();
         const uint16 endIndex   = buf->readNextUnaligned16bit();
         m->decodedGetFavorites(startIndex, endIndex, src);

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::GET_FAVORITES_ALL_DATA_REQ:
      {
         const uint16 startIndex = buf->readNextUnaligned16bit();
         const uint16 endIndex   = buf->readNextUnaligned16bit();
         m->decodedGetFavoritesAllData(startIndex, endIndex, src);
         
         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::GET_FAVORITE_INFO_REQ:
      {
         const uint32 favoriteId = buf->readNextUnaligned32bit();
         m->decodedGetFavoriteInfo(favoriteId, src);

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::SORT_FAVORITES_REQ:
      {  
         GuiProtEnums::SortingType sortOrder = 
            GuiProtEnums::SortingType(buf->readNextUnaligned16bit());
         m->decodedSortFavorites( sortOrder, src );

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::SYNC_FAVORITES_REQ:
      {
         m->decodedSyncFavorites(src);

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::ADD_FAVORITE_REQ:
      {
         Favorite* favorite = new Favorite(buf);
         m->decodedAddFavorite(favorite, src);
         // Do not delete the favorite, it is saved 
         // by the dest module.

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::REMOVE_FAVORITE_REQ:
      {
         uint32 favoriteId = buf->readNextUnaligned32bit();
         m->decodedRemoveFavorite(favoriteId, src);

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::CHANGE_FAVORITE_REQ:
      {
         Favorite* favorite = new Favorite(buf);
         m->decodedChangeFavorite(favorite, src);
         // Do not delete the favorite, it is saved 
         // by the dest module.

         delete buf;
         return NULL;
      }
   default:
      return buf;
   }

} //DestinationsProviderDecoder::dispatch

MsgBuffer* 
DestinationsConsumerDecoder::dispatch(MsgBuffer *buf,
                                      DestinationsConsumerInterface *m )
{
   uint32 src = buf->getSource();
   uint32 dst = buf->getDestination();

   switch (buf->getMsgType()) {
   case MsgBufferEnums::GET_FAVORITES_REPLY:
      {
         uint32 nbrOfFavorites = buf->readNextUnaligned16bit();
         vector<GuiFavorite*> guiFavorites;
         uint32 i = MAX_UINT32;
         for (i = 0; i < nbrOfFavorites; i++){
            guiFavorites.push_back(new GuiFavorite(buf));
         }
         m->decodedGetFavoritesReply(guiFavorites, src, dst);

         // Clean
         for (i = 0; i < guiFavorites.size(); i++){
            delete guiFavorites[i];
         }
         guiFavorites.clear();

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::GET_FAVORITES_ALL_DATA_REPLY:
      {
         uint32 nbrOfFavorites = buf->readNextUnaligned16bit();
         vector<Favorite*> favorites;
         uint32 i = MAX_UINT32;
         for (i = 0; i < nbrOfFavorites; i++){
            favorites.push_back(new Favorite(buf));
         }
         m->decodedGetFavoritesAllDataReply(favorites, src, dst);

         // Clean
         for (i = 0; i < favorites.size(); i++){
            delete favorites[i];
         }
         favorites.clear();

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::GET_FAVORITE_INFO_REPLY:
      {
         Favorite* favorite = new Favorite(buf);
         m->decodedGetFavoriteInfoReply(favorite, src, dst);

         // Clean
         delete favorite;
         favorite = NULL;

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::FAVORITES_CHANGED_MSG:
      {
         m->decodedFavoritesChanged(src, dst);

         delete buf;
         return NULL;
      } break;
   case MsgBufferEnums::DEST_REQUEST_OK_REPLY:
      {
         m->decodedDestRequestOk(src, dst);

         delete buf;
         return NULL;
      } break;
   }

   return buf;
} //DestinationsProviderDecoder::dispatch



uint32 DestinationsProviderPublic::getFavorites(uint16 startIdx, uint16 endIdx, uint32 dst) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 
   MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::GET_FAVORITES_REQ, 
                                  64);
   buf->writeNextUnaligned16bit(startIdx);
   buf->writeNextUnaligned16bit(endIdx);
/*    DBG("getFavorites pre insert buf"); */
   m_queue->insert(buf);
/*    DBG("getFavorites post insert buf"); */

   return src;
} // DestinationsProviderPublic::getFavorites



uint32 
DestinationsProviderPublic::getFavoritesAllData(uint16 startIdx, uint16 endIdx,
                                                uint32 dst) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 
   MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::GET_FAVORITES_ALL_DATA_REQ, 
                                  64);
   buf->writeNextUnaligned16bit(startIdx);
   buf->writeNextUnaligned16bit(endIdx);
   m_queue->insert(buf);

   return src;
} // DestinationsProviderPublic::getFavoritesAllData

uint32 
DestinationsProviderPublic::getFavoriteInfo(uint32 favouriteId, 
                                            uint32 dst) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 
   MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::GET_FAVORITE_INFO_REQ, 
                                  64);
   buf->writeNextUnaligned32bit(favouriteId);
   m_queue->insert(buf);

   return src;
} // DestinationsProviderPublic::getFavoriteInfo

uint32 
DestinationsProviderPublic::sortFavorites(GuiProtEnums::SortingType sortingOrder, 
                                          uint32 dst) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 
   MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::SORT_FAVORITES_REQ, 
                                  64);
   buf->writeNextUnaligned16bit(sortingOrder);
   m_queue->insert(buf);

   return src;
}

uint32 
DestinationsProviderPublic::syncFavorites( uint32 dst ) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::SYNC_FAVORITES_REQ, 
                                  64);
   m_queue->insert(buf);

   return src;
}


uint32 
DestinationsProviderPublic::addFavorite( Favorite* favorite, 
                                         uint32 dst) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 
   MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::ADD_FAVORITE_REQ, 
                                  64 + favorite->getSize());
   favorite->serialize(buf);
   m_queue->insert(buf);

   return src;
}


uint32 
DestinationsProviderPublic::removeFavorite( uint32 favoriteId, 
                                            uint32 dst) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 
   MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::REMOVE_FAVORITE_REQ, 
                                  64);
   buf->writeNextUnaligned32bit(favoriteId);
   m_queue->insert(buf);

   return src;
}

uint32 
DestinationsProviderPublic::changeFavorite( Favorite* favorite, 
                                            uint32 dst) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 
   MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::CHANGE_FAVORITE_REQ, 
                                  64 + favorite->getSize());
   favorite->serialize(buf);
   m_queue->insert(buf);

   return src;
}


uint32
DestinationsConsumerPublic::getFavoritesReply( vector<GuiFavorite*>& guiFavorites, uint32 dst ) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 

   uint32 bufferSize = guiFavorites.size() * GuiFavorite::MAX_GUI_FAV_SIZE + 2;   
   MsgBuffer *buf = new MsgBuffer(dst, src, 
                                  MsgBufferEnums::GET_FAVORITES_REPLY,
                                  bufferSize);
   buf->writeNextUnaligned16bit(guiFavorites.size());
   for (uint16 i = 0; i < guiFavorites.size(); i++){
      guiFavorites[i]->serialize(buf);
   }
   m_queue->insert(buf);
   return src;
} // DestinationsConsumerPublic::getFavoritesReply


uint32 
DestinationsConsumerPublic::getFavoritesAllDataReply( vector<Favorite*>& favorites, uint32 dst ) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 

   uint32 bufferSize = 2;
   uint16 i = 0;
   for ( i = 0 ; i < favorites.size() ; ++i ) {
      bufferSize += favorites[ i ]->getSize();
   }
   MsgBuffer *buf = new MsgBuffer(dst, src, 
                                  MsgBufferEnums::GET_FAVORITES_ALL_DATA_REPLY,
                                  bufferSize);
   buf->writeNextUnaligned16bit(favorites.size());
   for ( i = 0 ; i < favorites.size() ; ++i ) {
      favorites[ i ]->serialize( buf );
   }
   m_queue->insert(buf);
   return src;
} // DestinationsConsumerPublic::getFavoritesAllDataReply

uint32 
DestinationsConsumerPublic::getFavoriteInfoReply( Favorite* favorite, uint32 dst ) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId(); 

   bool deleteFavorite = false;
   if (favorite == NULL){
      // The id of the favorite asked for does not
      // exist. Send an invalid favorite.

      Favorite* invalidFavorite = 
         new Favorite( uint32(Favorite::INVALID_FAV_ID), 
                       0, 0, "", "", "", "", "" );
      favorite = invalidFavorite;
      deleteFavorite = true;
   }

   uint32 bufferSize = favorite->getSize();   
   MsgBuffer *buf = new MsgBuffer(dst, src, 
                                  MsgBufferEnums::GET_FAVORITE_INFO_REPLY,
                                  bufferSize);
   favorite->serialize(buf);
   m_queue->insert(buf);



   // Clean up any allocated invalid favorites.
   if (deleteFavorite){
      delete favorite;
      favorite = NULL;
   }
   
   return src;
} // DestinationsConsumerPublic::getFavoriteInfoReply
 

uint32 
DestinationsConsumerPublic::destRequestOk(uint32 dst) const
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();

   MsgBuffer *buf = new MsgBuffer(dst, src, 
                                  MsgBufferEnums::DEST_REQUEST_OK_REPLY,
                                  64);

   m_queue->insert(buf);

   return src;
} // DestinationsConsumerPublic::destRequestOk



} // namespace isab.
