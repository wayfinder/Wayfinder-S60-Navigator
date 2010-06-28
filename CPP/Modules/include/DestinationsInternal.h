/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DESTINATIONS_INTERNAL_H
#define DESTINATIONS_INTERNAL_H

#include "NavRequestType.h"

/// Max size of m_pending
#define MAX_PENDING 10


namespace isab{


   using namespace Packet_Enums;
   // forward declaration, not pretty but necessary.
   class Packet;
   /**
    * Destinations.
    *
    */
   class Destinations : public Module, 
                        public CtrlHubAttachedInterface,
                        public DestinationsProviderInterface,
                        public NavServerComConsumerInterface,
                        public ParameterConsumerInterface
   {
      public:

        // Macro for making a reply packet type from a request packet type
#define REPLY(i) PacketType(i | 0x80)      
        // Macro for checking if a packet type is a request.
#define NAV_DISPLAY_SERIAL_IS_REQUEST(i)  (!(i&0x80))

        /**
         * The server does not use ID:s from MAX_INT32 to
         * MAX_UINT32.
         */
#define LOWEST_CLIENT_FAV_ID (uint32(MAX_INT32) + 1);

      /**
       * Constructor. The imei will be the imei crc32 converted to hex.
       */
      Destinations( bool using_fav_ver_2, const char* imei = NULL );


      virtual ~Destinations();


      /**
       * Module functions
       */
      //@{
         /**
          * Called when shutdown starts.
          */
         virtual void decodedShutdownNow(int16 upperTimeout);

         /**
          * Called when startup is done and module is ready.
          */
         virtual void decodedStartupComplete();

         /**
          * All error messages that causes request to other modules in 
          * Nav2 to fail, ends up here.
          *
          * @param errorNbr The error number of the error causing the 
          *                 request to fail.
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
       * Destinations module methods.
       */
      //@{

         virtual void decodedGetFavorites(uint16 startIdx, uint16 endIdx, 
                                          uint32 src);

         virtual void decodedGetFavoritesAllData(uint16 startIdx, uint16 endIdx, 
                                          uint32 src);

         virtual void decodedGetFavoriteInfo(uint32 favoriteId, 
                                          uint32 src);

         virtual void decodedSortFavorites(GuiProtEnums::SortingType sortingOrder, 
                                       uint32 src);

         virtual void decodedSyncFavorites( uint32 src);
      
         virtual void decodedAddFavorite( Favorite* favorite, 
                                          uint32 src); 
     
         virtual void decodedRemoveFavorite( uint32 favoriteId, 
                                          uint32 src);

         virtual void decodedChangeFavorite( Favorite* favorite, 
                                          uint32 src);

         /**
          * Called when sync destination reply has been decoded.
          */
         void decodedSyncDestinationsReply( std::vector<Favorite*>& favs, 
                                            std::vector<uint32>& removed, 
                                            uint32 src, uint32 dst );
      //@}

      /**
       * NavServerCom functions
       */
      //@{
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

      virtual void decodedSearchReply(unsigned /*nAreas*/, 
                                      const SearchArea* const* /*sa*/,
                                      unsigned /*nItems*/, 
                                      const FullSearchItem* const* /*fsi*/,
                                      uint16 /*begindex*/, uint16 /*total*/,
                                      uint32 /*src*/, uint32 /*dst*/)
      { /* This function intentionally left blank. */}
      virtual void decodedSearchInfoReply(uint16 /*num*/, const char* const* /*id*/,
                                          const unsigned* /*index*/,
                                          unsigned /*numInfo*/,
                                          const AdditionalInfo*const* /*info*/,
                                          uint32 /*src*/, uint32 /*dst*/)
      { /* This function intentionally left blank. */}

      virtual void decodedLicenseReply(bool /*keyOk*/, bool /*phoneOk*/, 
                                       bool /*regionOk*/, bool /*nameOk*/, 
                                       bool /*emailOk*/,
                                       GuiProtEnums::WayfinderType /*type*/,
                                       uint32 /*src*/, uint32 /*dst*/)
      { /* This function intentionally left blank. */}

      virtual void decodedCellConfirm(uint32 /*size*/, const uint8* /*data*/,
                                      uint32 /*dst*/)
      { /* This function intentionally left blank. */}

         /**
          * Called when a route reply is received.
          *
          * @param src The id of the module?
          * @param dst The id of the request that this is a reply for?
          */
         virtual void decodedRouteReply( uint32 src, uint32 dst );

      virtual void decodedMapReply(const class BoxBox& /*bb*/,
            uint32 /*realWorldWidth*/,
            uint32 /*realWorldHeight*/,
            uint16 /*imageWidth*/,
            uint16 /*imageHeight*/,
            enum MapEnums::ImageFormat /*imageType*/, 
            uint32 /*imageBufferSize*/, 
            const uint8* /*imageBuffer*/,
            uint32 /*dst*/)
      {
      }
      virtual void decodedVectorMapReply(const char* /*request*/, uint32 /*size*/,
                                         const uint8* /*data*/, uint32 /*dst*/)
      {
         //This function intentionally left blank
      }
      virtual void decodedMulitVectorMapReply(uint32 /*size*/,
                                              const uint8* /*data*/, uint32 /*dst*/)
      {
         //This function intentionally left blank
      }


      virtual void decodedForceFeedMuliVectorMapReply(
         uint32 /*size*/, const uint8* /*data*/, uint32 /*dst*/ )
      {
         //This function intentionally left blank
      }


      virtual void decodedMessageReply(const uint8* /*data*/, unsigned /*length*/,
                                       uint32 /*src*/, uint32 /*dst*/)
      {}

      virtual void decodedLatestNews(uint32 /*checksum*/, const uint8* /*data*/, 
                                     unsigned /*length*/, uint32 /*src*/, 
                                     uint32 /*dst*/)
      {}
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
      virtual void decodedProgressMessage(NavServerComEnums::ComStatus status, 
                                          GuiProtEnums::ServerActionType type,
                                          uint32 done, uint32 of,
                                          uint32 src, uint32 dst );

      virtual void decodedParamSyncReply(GuiProtEnums::WayfinderType /*wft*/, 
                                         uint32 /*src*/, uint32 /*dst*/)
      { /* this function intentionally left blank */}

      virtual void decodedReply( 
         navRequestType::RequestType type, NParamBlock& params, 
         uint8 requestVer, uint8 statusCode, const char* statusMessage,
         uint32 src, uint32 dst );
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
      virtual void decodedParamValue (uint32 paramId, const uint8 *data, 
                                      int size, uint32 src, uint32 dst);
      //@}
       


   protected:
      /**
       * Called by hub when message is sent to this module?
       *
       * @param buf The request Msg?
       * @return MsgBuffer with the reply?
       */
      virtual MsgBuffer* dispatch( MsgBuffer* buf );


      
         /**
          * Helper methods used for manipulating the favorite
          * list.
          */
         //@{
            Nav2Error::ErrorNbr addFavorite(Favorite* favorite);
            Nav2Error::ErrorNbr removeFavorite(uint32 favoriteId);

            /**
             * Used for sorting the favorites.
             *
             * @param sortingOrder The sorting order to use.
             * @return Returns false if an error occurs.
             */
            Nav2Error::ErrorNbr sortFavorites(GuiProtEnums::SortingType soringOrder);
            
            /**
             * This method should be called every time the favorite list 
             * changes so it can be multicasted.
             */
            void favoritesChanged(bool writeToParam = true); 
         //@}




      /**
       * Sends the favorites to the paramter module for saving.
       *
       * @param favorites The favorites to save. Usually the 
       *                  member variable m_favorites.
       */
      void saveFavorites(std::vector<Favorite*>& favorites);


      /**
       * Returns the highest currently used favorite id. This is 
       * one of the ID:s reserved by the client.
       *
       * The lowest possible value to be returned is 
       * LOWEST_CLIENT_FAV_ID.
       *
       * @param favorites The favorites vector to look in.
       * @param sendError If an error occurs and this variable
       *                  is set to true, an error is sent.
       *
       * @return The highest currently used favorite id
       */
      uint32 getNextFavoriteId(std::vector<Favorite*>& favorites, bool sendError = true);

      /**
       * Used for setting for example parameter values.
       */
      void setCharValue(char*& dst, const char* src);

      /**
       * Checks whether any of the strings in the favorite
       * is to long to be stored in mc2.
       *
       * @return Returns false if any of the strings are
       *         to long, otherwise true.
       */
      bool checkFavoriteStringLengths(Favorite* favorite);

      typedef std::vector< Favorite* > favVect;
      typedef std::vector< Favorite* >::iterator favIt;

      /**
       * Move the unsupported special favorites aside.
       */
      void moveSpecialsAside( favVect* favorites, favVect* sidefavorites );

      /**
       * Set favoritesCRC.
       */
      void setFavoritesCRC( const char* s );

      /**
       * Get favoritesCRC.
       */
      const char* getFavoritesCRC() const;

      /**
       * Set Dest_favoritesCRC parameter.
       */
      void setDestFavoritesCRC( const char* s );

   private: 

      /**
       * Stores the favorites in this modlue.
       */
      favVect* m_favorites;

      /**
       * Stores the special favorites that is not used by the GUI.
       */
      favVect* m_sidefavorites;

      typedef std::vector< GuiProtEnums::AdditionalInfoType > specialsVect;
      /**
       * Stores the special favorites that is not used by the GUI.
       */
      specialsVect* m_specials;

      /**
       * The currently used sorting order.
       */
      GuiProtEnums::SortingType m_sortOrder;

      /**
       * Ids of favorites that has been removed since 
       * last sync.
       */
      std::vector< uint32 > *m_removedFavoriteIds;

      /**
       * Next id to use for new favorites.
       */
      uint32 m_nextFavoriteId;


      // communicate with the parameter module.
      ParameterProviderPublic * m_paramProvider;

      /// The interface to nav server com
      NavServerComProviderPublic* m_nscProvider;


      DestinationsConsumerPublic* m_destConsumer;


      NavServerComConsumerDecoder  m_nscDecoder;     // XXX: not set!
      CtrlHubAttachedDecoder       m_hubDecoder;     // XXX: not set!
      ParameterConsumerDecoder     m_paramDecoder;   // XXX: not set!
      DestinationsProviderDecoder  m_destProvDecoder;// XXX: not set!

      /// If module is shuting down.
      bool m_shutDown;

      /// Oustanding requests.
/*       class RequestList *m_pending; */


      /**

       * When this one times out it is time to save the 

       * favorites.

       */

      uint16 m_saveFavoritesTimer;

      /** If this field is equal to zero, no sync request is outstanding.
       * Otherwise this field contain the request id of a sync request 
       * sent by the Destinations Modukle to the NavServerCom module.*/
      uint32 m_syncRequestOut;
      /** This field holds the request id of the sync request received by 
          the Destinations module.
       */
      uint32 m_syncRequestIn;

      /**
       * If gui uses favorites version 2.
       */
      bool m_using_fav_ver_2;

      /**
       * The stored favorites CRC.
       */
      char* m_favoritesCRC;

      /**
       * The imei string, crc32 and converted to hex.
       */
      char* m_imei;
   };

   inline CtrlHubAttachedPublic *Destinations::newPublicCtrlHub()
   {
      return new CtrlHubAttachedPublic(m_queue);
   }

} // namespace isab.
#endif // DESTINATIONS_INTERNAL_H


