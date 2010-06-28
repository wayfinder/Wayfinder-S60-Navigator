/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MODULE_TestShell_H
#define MODULE_TestShell_H

#include "Module.h"
#include <stdarg.h>
#include "NavRequestType.h"


using namespace isab::NavServerComEnums;

namespace isab {
   /**
    * Class that can be used to test certain operations in Nav2.  It
    * uses a serial interface to the rest of the world, connects
    * directly to the Ctrlhub, and can talk to any other known module.
    */
   class TestShell : public Module, 
                     public CtrlHubAttachedInterface, 
                     public SerialConsumerInterface,
                     public ParameterConsumerInterface,
                     public NavServerComConsumerInterface,
                     public SerialProviderInterface,
                     public DestinationsConsumerInterface,
                     public NavTaskConsumerInterface,
                     public ErrorProviderInterface
   {
   public:
      /** @name Constructors and destructor. */
      //@{
      /**
       * Constructor. 
       * @param p The SerialProviderPublic object TestShell uses to
       *          communicate with the outside world.
       * @param errorTable TestShell keeps its own pointer to the
       *        errorTable so that it can translate error messages to
       *        human readable form.
       * @param regressionTest If set, the output from TestShell takes
       *                       a certain welldefined form. 
       *                       Defaults to false. 
       */
      TestShell(class SerialProviderPublic *p,
                class Nav2Error::Nav2ErrorTable *errorTable,
                bool regressionTest = false );
      //@}

      /** @name From Module. */
      //@{
      virtual void decodedStartupComplete();
      virtual void decodedSolicitedError( const ErrorObj& err, 
                                          uint32 src, uint32 dst );
      virtual void decodedConnectionNotify(enum ConnectionNotify, 
                                           enum ConnectionNotifyReason, 
                                           uint32 src);
      //@}

      /** @name From ErrorProviderInterface. */
      //@{
      virtual void decodedUnsolicitedError( const ErrorObj& err, 
                                            uint32 src );
      //@}

      /**
       * Creates a new CtrlHubAttachedPublic object used to connect this
       * module to the CtrlHub.
       * @return a new CtrlHubAttachedPublic object connected to the queue.
       */
      class CtrlHubAttachedPublic * newPublicCtrlHub();

      /** @name From DestinationsConsumerInterface. */
      //@{
      virtual void decodedGetFavoritesReply(std::vector<class GuiFavorite*>& guiFavorites, 
                                            uint32 src, uint32 dst );
      virtual void decodedGetFavoritesAllDataReply(std::vector<class Favorite*>& favorites,
                                             uint32 src, uint32 dst  ) ;
      virtual void decodedGetFavoriteInfoReply(class Favorite* favorite, 
                                               uint32 src, uint32 dst);
      virtual void decodedFavoritesChanged(uint32 src, uint32 dst);

      virtual void decodedDestRequestOk(uint32 src, uint32 dst); 
      //@}

      /** @name From SerialConsumerInterface. */
      //@{
      virtual void decodedReceiveData(int length, const uint8 *inData, 
                                      uint32 src);
      //@}

      /** @name From ParameterConsumerInterface.*/
      //@{
      virtual void decodedParamNoValue(uint32 paramId, uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 paramId, const int32 * data,
                                     int32 numEntries, uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 paramId, const float * data,
                                     int32 numEntries, uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 paramId, const char * const * data,
                                     int32 numEntries, uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 paramId, const uint8* data, 
                                     int size, uint32 src, uint32 dst);
      //@}

      /** @name From NavTaskConsumerInterface. */
      //@{
      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedPositionState(const struct PositionState &/*p*/, 
                                        uint32 /*src*/)
      {/* Not needed*/};
      
      virtual void setRouteCoordinate( int32 /*lat*/, int32 /*lon*/, int8 /*dir*/ )
      {}

      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedGpsSatelliteInfo(class GpsSatInfoHolder* /*v*/,
                                           uint32 /*src*/)
      {}
      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedFileOperation(class GuiFileOperation* /*op*/,
                                        uint32 /*src*/)
      {}
      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedRouteInfo(const class RouteInfo &/*r*/,
                                    uint32 /*src*/)
      {/* Not needed*/};
      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedRouteList(class Buffer* /*buf*/,
                                    uint32 /*src*/, uint32 /*dst*/)
      {/* Not needed*/};
      /**Unused. included only to comply with certain interfaces.*/
      virtual void decodedInvalidateRoute(bool newRouteAvailable,
                                          int64 routeid,
                                          int32 tLat, int32 lLon, 
                                          int32 bLat, int32 rLon,
                                          int32 oLat, int32 oLon, 
                                          int32 dLat, int32 dLon,
                                          uint32 src);
      //@}

      /** @name From SerialProviderInterface. */
      //@{
      virtual void decodedSendData(int length, const uint8 *data, uint32 src);
      //@}

      /** @name From NavServerComConsumerInterface. */
      //@{
      virtual void decodedWhereAmIReply(const char* country, 
                                        const char* municipal, 
                                        const char* city, const char* district,
                                        const char* streetname,
                                        int32 lat, int32 lon, uint32 src,
                                        uint32 dst);
      virtual void decodedSyncDestinationsReply(std::vector<class Favorite*>& favs,
                                                std::vector<uint32>& removed,
                                                uint32 src, uint32 dst);
      virtual void decodedSearchReply(unsigned nAreas, 
                                      const class SearchArea* const*sa,
                                      unsigned nItems, 
                                      const class FullSearchItem* const*fsi,
                                      uint16 begindex, uint16 total,
                                      uint32 src, uint32 dst);

      virtual void decodedSearchInfoReply(uint16 num, const char* const* id,
                                          const unsigned* index,
                                          unsigned numInfo,
                                          const class AdditionalInfo*const* info,
                                          uint32 src, uint32 dst);

      virtual void decodedLicenseReply(bool, bool, bool, bool, bool, 
                                       enum GuiProtEnums::WayfinderType, 
                                       uint32, uint32);

      virtual void decodedRouteReply(uint32 src, uint32 dst);
      virtual void decodedMapReply(const class BoxBox& bb, uint32 realWorldWidth,
                           uint32 realWorldHeight, uint16 imageWidth,
                           uint16 imageHeight,
                           enum MapEnums::ImageFormat imageType, 
                           uint32 imageBufferSize, 
                           const uint8* imageBuffer, uint32 dst);
      virtual void decodedVectorMapReply(const char* request, uint32 size,
                                         const uint8* data, uint32 dst);
      virtual void decodedMulitVectorMapReply(uint32 size,
                                              const uint8* data, uint32 dst);

      virtual void decodedForceFeedMuliVectorMapReply(
         uint32 size, const uint8* data, uint32 dst );


      virtual void decodedNTRouteReply(int64 routeid, uint32 src, uint32 dst);
      virtual void decodedBinaryUploadReply(uint32 src, uint32 dst);
      virtual void decodedBinaryDownload(const uint8* data, size_t length, 
                                         uint32 src, uint32 dst);
      virtual void decodedProgressMessage(ComStatus status, 
                                          GuiProtEnums::ServerActionType type,
                                          uint32 done, uint32 of,
                                          uint32 src, uint32 dst);

      virtual void decodedMessageReply(const uint8* /*data*/, unsigned /*length*/,
                                       uint32 /*src*/, uint32 /*dst*/)
      {}

      virtual void decodedLatestNews(uint32 checksum, const uint8* data, 
                                     unsigned length, uint32 src, 
                                     uint32 dst);
      virtual void decodedParamSyncReply(enum GuiProtEnums::WayfinderType /*wft*/, 
                                         uint32 /*src*/, uint32 /*dst*/)
      { /* this function intentionally left blank */}
      virtual void decodedCellConfirm(uint32 /*size*/, const uint8* /*data*/,
                                      uint32 /*dst*/)
      { /* This function intentionally left blank. */}
      virtual void decodedReply( 
         enum navRequestType::RequestType /*type*/, class NParamBlock& /*params*/, 
         uint8 /*requestVer*/, uint8 /*statusCode*/, const char* /*statusMessage*/,
         uint32 /*src*/, uint32 /*dst*/ )
      { /* this function intentionally left blank */}

      class SerialProviderPublic * newPublicSerial();

   protected:
      virtual class MsgBuffer * dispatch(class MsgBuffer *buf);

      /** Decoder for CtrlHub-messages */
      CtrlHubAttachedDecoder m_ctrlHubAttachedDecoder;
      /** Decoder for Serial-messages */
      SerialConsumerDecoder m_serialConsumerDecoder;
      /** Decoder for Parameter-messages */
      ParameterConsumerDecoder m_parameterConsumerDecoder;
      /** Decoder for incoming serial messages.*/
      SerialProviderDecoder m_serialProviderDecoder;
      /** Decoder for NavTask messages. */
      NavTaskConsumerDecoder m_navTaskDecoder;
      /***/
      class NavTaskProviderPublic * m_navTaskProvider;
      /** Encoder for Parameter-messages */
      class ParameterProviderPublic * m_parameterProvider;
      /** Encoder for Serial-messages */
      class SerialProviderPublic * m_serialProvider;
      /** Encoder for Destinations-messages */
      class DestinationsProviderPublic * m_dstProvider;
      /** Decoder for Destinations-messages*/
      DestinationsConsumerDecoder m_dstDecoder;
      /** Just a cast to the correct type, really. */
      class CtrlHubPublic * rootPublic();

      class NavServerComProviderPublic * m_nscProvider;
      NavServerComConsumerDecoder m_nscDecoder;
      ErrorProviderDecoder m_errorDecoder;
/*       Route m_route; */

      /**
       * Write Point as string to outputbuffer.
       * Used by regression test.
       * @param self The point that should have written itself, were this OO code.
       * @param txt Reference to output buffer.
       */
      void Point__printAsString(class Point& self, class OutputBuffer &txt);
      
      virtual void treeIsShutdown();

      void outputToTcp(class OutputBuffer& txt);
   private:
      uint32 m_syncrequestID;
      bool m_regressionTest;
      /// If true, routes will be output without trackpoint data.
      bool m_shortRouteOutput;
      class Nav2Error::Nav2ErrorTable * m_errorTable;

      int m_mapnum;
   };

} /* namespace isab */

#endif /* MODULE_TestShell_H */
