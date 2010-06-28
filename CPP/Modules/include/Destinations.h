/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef DESTINATIONS_MODULE_H
#define DESTINATIONS_MODULE_H

#include "Module.h"
#include "Buffer.h"
#include "GuiProt/Favorite.h"
#include "GuiProt/GuiProtMess.h"
#include "DestinationsEnums.h"
#include "Nav2Error.h"


namespace isab {
/*    using namespace DestinationsEnums; */



/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * DestinationsConsumerInterface. */
class DestinationsConsumerPublic : public ModulePublic {
   public:
      DestinationsConsumerPublic(Module *m) : 
         ModulePublic(m) { };

      /**
       * @param dst The address of the module to receive the message and the 
       *            Nav2 message number from the request this message answers
       *            to.
       */
      //@{
         uint32 getFavoritesReply( std::vector<GuiFavorite*>& guiFavorites, uint32 dst ) const;
         uint32 getFavoritesAllDataReply( std::vector<Favorite*>& favorites, uint32 dst ) const;
         uint32 getFavoriteInfoReply( Favorite* favorite, uint32 dst ) const;
         uint32 destRequestOk(uint32 dst) const;
      //@}
};

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * DestinationsProviderInterface. */
class DestinationsProviderPublic : public ModulePublic {
   public:
      DestinationsProviderPublic(Module* m) : 
         ModulePublic(m)
      { 
/*          m_log = new Log("DestProvPub"); */
/*          m_log->debug("Started DestProvPub!"); */
         setDefaultDestination(Module::addrFromId(Module::DestinationsModuleId));
      };

/*       virtual ~DestinationsProviderPublic() { delete m_log; } */
/*       Log *m_log; */


      uint32 getFavorites(uint16 startIdx = 0, uint16 endIdx = MAX_UINT16, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      
      uint32 getFavoritesAllData(uint16 startIdx = 0 , uint16 endIdx = MAX_UINT16,
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      
      uint32 getFavoriteInfo(uint32 favouriteId, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      
      uint32 sortFavorites(GuiProtEnums::SortingType sortingOrder, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      
      uint32 syncFavorites(   uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;

      
      uint32 addFavorite( Favorite* favorite, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      
      uint32 removeFavorite( uint32 favoriteId, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
      
      uint32 changeFavorite( Favorite* favorite, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT) const;
};




/* Next are the interfaces. These should be implemented by the 
 * module that wishes to receive the corresponding messages. 
 * The classes below are pure virtual and correspond in function
 * to "interfaces" in java. */

/** An interface that must be implemented to receive messages in a
 * Destinations provider-consumer relationship. This is implemented 
 * in the _upper_ module.
 */
class DestinationsConsumerInterface {
   public:
      virtual void decodedGetFavoritesReply( std::vector<GuiFavorite*>& guiFavorites, 
                                             uint32 src, uint32 dst ) = 0;
      virtual void decodedGetFavoritesAllDataReply( std::vector<Favorite*>& favorites, 
                                             uint32 src, uint32 dst  ) = 0;
      virtual void decodedGetFavoriteInfoReply( Favorite* favorite, 
                                             uint32 src, uint32 dst  ) = 0;
      virtual void decodedFavoritesChanged(  uint32 src, uint32 dst  ) = 0;

      /**
       * This message ansvers all requests that does not hav specific 
       * replies, such as sortFavorites, syncFavorites, addFavorite,
       * removeFavorite and changeFavorite.
       *
       * The purpouse of sending this message is that the requesting
       * module can remove the request from its pending list.
       */
      virtual void decodedDestRequestOk (uint32 src, uint32 dst) = 0;
};

/** An interface that must be implemented to receive messages in a
 * Destinations provider-consumer relationship. This is implemented 
 * in the _lower_ module.
 */
class DestinationsProviderInterface {
   public:

      virtual void decodedGetFavorites(uint16 startIdx, uint16 endIdx, 
                                       uint32 src) = 0;
      virtual void decodedGetFavoritesAllData(uint16 startIdx, uint16 endIdx, 
                                       uint32 src) = 0;
      virtual void decodedGetFavoriteInfo(uint32 favouriteId, uint32 src) = 0;

      virtual void decodedSortFavorites(GuiProtEnums::SortingType sortingOrder, 
                                       uint32 src) = 0;
      virtual void decodedSyncFavorites( uint32 src) = 0;
      
      virtual void decodedAddFavorite( Favorite* favorite, 
                                       uint32 src) = 0;      
      virtual void decodedRemoveFavorite( uint32 favoriteId, 
                                       uint32 src) = 0;
      virtual void decodedChangeFavorite( Favorite* favorite, 
                                       uint32 src) = 0;
};

/** An object of this class is owned by each module that implements 
 * the DestinationsConsumerInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in DestinationsConsumerInterface.
 */
class DestinationsConsumerDecoder {
   public:
/*       DestinationsConsumerDecoder() { m_log = new Log("DestConsDec"); m_log->debug("Started DestConsDec!"); } */
/*       ~DestinationsConsumerDecoder() { delete m_log; } */
/*       Log *m_log; */
      class MsgBuffer * dispatch(class MsgBuffer *buf, DestinationsConsumerInterface *m );
};

/** An object of this class is owned by each module that implements 
 * the DestinationsProviderInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in DestinationsProviderInterface.
 *
 * @return the buffer if further processing may be done or NULL if 
 *         the buffer was processed and either passed on or 
 *         destroyed and thus no further processing should be done.
 */
class DestinationsProviderDecoder {
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf, DestinationsProviderInterface *m );
};
































} // namespace isab.

#endif // DESTINATIONS_INTERNAL_H

