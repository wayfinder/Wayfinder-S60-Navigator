/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "arch.h"
#include <ctype.h>

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Matches.h"
#include "ErrorModule.h"
#include "NavPacket.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "GuiProt/ServerEnums.h"
#include "NavServerCom.h"

#include "Buffer.h"
#include "MsgBuffer.h"

#include "Matches.h"
#include "Nav2Error.h"

#include "GuiProt/SearchRegion.h"
#include "GuiProt/SearchArea.h"
/* #include "GuiProt/SearchItem.h" */
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/FullSearchItem.h"
#include "GuiProt/Favorite.h"
#include "GuiProt/MapClasses.h"
#include "TraceMacros.h"
#include "NavRequestType.h"

#ifndef NO_LOG_OUTPUT
# define NO_LOG_OUTPUT
#endif

#include "LogMacros.h"

namespace isab {
   using namespace NavServerComEnums;

   MsgBuffer* 
   NavServerComProviderDecoder::dispatch(MsgBuffer *buf,
                                         NavServerComProviderInterface *m )
   {
#ifndef NO_LOG_OUTPUT
      Log log("NSCPP::dispatch");
      Log* m_log = &log;
#endif
      int length;

      length=buf->getLength();
      switch (buf->getMsgType()) {
      case MsgBufferEnums::WHERE_AM_I_REQUEST:
         {
            int32 lat = buf->readNextUnaligned32bit();
            int32 lon = buf->readNextUnaligned32bit();
            m->decodedWhereAmI(lat, lon, buf->getSource());
            delete buf ;
            return NULL;
         }
      case MsgBufferEnums::SEARCH_REQUEST:
         {
            int origPos = buf->setReadPos(2); //jump startindex.
            //used to see if the input is valid.
            const char* city = buf->getNextCharString();
            //In case city is null, we check to see if we have a city-id
            const char* id = buf->getNextCharString();
            if(!city || !*city){
               city = id;
            }
            buf->getNextCharString(); //jump searchDestination
            uint32 country = buf->readNextUnaligned32bit();
            int latOffset  = buf->getReadPos(); // keep lat offset
            int32 lat      = buf->readNextUnaligned32bit();
            int lonOffset  = buf->getReadPos(); // keep lon offset
            int32 lon      = buf->readNextUnaligned32bit();
            int hdgOffset  = buf->getReadPos(); // keep hdg offset 
            uint8 hdg      = buf->readNext8bit();
            buf->setReadPos(origPos); // restore buffer read pos.
            if(!((lat == MAX_INT32) && (lon == MAX_INT32))){ // don't refresh
               latOffset = lonOffset = hdgOffset = -1; //invalid offsets
            }
            m->decodedSearchRequest(buf->accessRawData(0), buf->getLength(),
                                    lat, lon, hdg, country, city, 
                                    buf->getSource(), 
                                    latOffset, lonOffset, hdgOffset);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::ROUTE_REQUEST:
         {
            buf->setReadPos(0);
            int32  fromLat          = buf->readNextUnaligned32bit();
            int32  fromLon          = buf->readNextUnaligned32bit();
            int32  toLat            = buf->readNextUnaligned32bit();
            int32  toLon            = buf->readNextUnaligned32bit();
            uint16 hdng             = buf->readNextUnaligned16bit();
            int64  oldRouteId       = buf->readNextUnaligned64bit();
            uint32 routeRequestorId = buf->readNextUnaligned32bit();
            uint8  rerouteReason    = buf->readNext8bit();
            m->decodedRouteToGps(toLat, toLon, fromLat, fromLon, hdng, 
                                 routeRequestorId, oldRouteId, rerouteReason,
                                 buf->getSource());
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::MAP_REQUEST:
         {
            int lat = -1;
            int lon = -1;
            int hdg = -1;
            int spd = -1;
            //DBGDUMP("MapRequest", buf->accessRawData(0), buf->getLength());
            BoundingBox* bb = BoundingBox::deserialize(buf);
            switch(bb->getBoxType()){
            case BoundingBox::vectorBox:
               DBG("MAP_REQUEST: vectorBox, refresh heading and speed");
               hdg = 12;
               spd = 10;
            case BoundingBox::diameterBox:
               DBG("MAP_REQUEST: refresh lat and lon");
               lat = 2; lon = 6;
            default:
               break;
            }
            m->decodedMapRequest(buf->getLength(), buf->accessRawData(0),
                                 buf->getSource(), lat, lon, hdg, spd);
            delete bb;
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::VECTOR_MAP_REQUEST:
         m->decodedVectorMapRequest(buf->getLength(), buf->accessRawData(0), 
                                    buf->getSource());
         delete buf;
         return NULL;
      case MsgBufferEnums::MULTI_VECTOR_MAP_REQUEST:
         m->decodedMultiVectorMapRequest(buf->getLength(), 
                                         buf->accessRawData(0), 
                                         buf->getSource());
         delete buf;
         return NULL;
      case MsgBufferEnums::ADDITIONAL_INFO_REQ:
         m->decodedSearchInfoRequest(buf->getLength(), buf->accessRawData(0),
                                     buf->getSource());
         delete buf;
         return NULL;
      case MsgBufferEnums::LICENSE_KEY:
         {
            uint32 region = buf->readNextUnaligned32bit();
            const char* key   = buf->getNextCharString();
            const char* phone = buf->getNextCharString();
            const char* name  = buf->getNextCharString();
            const char* email = buf->getNextCharString();
            const char* optional = buf->getNextCharString();
            m->decodedVerifyLicenseKey(key, phone, name, email, optional,
                                       region, buf->getSource());
         }
         delete buf;
         return NULL;
      case MsgBufferEnums::BINARY_UPLOAD:
         {
            m->decodedBinaryUpload(buf->getLength(), buf->accessRawData(0), 
                                   buf->getSource());
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::SET_SERVER_PARAMS:
         {
            m->decodedSetServerParams(buf->accessRawData(0), 
                                      buf->getLength(), buf->getSource());
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::CANCEL_NSC_REQUEST:
         { 
            uint32 src = buf->getSource();
            uint32 cancelID = buf->readNextUnaligned32bit();
            uint32 netmask = buf->readNextUnaligned32bit();
            m->decodedCancelRequest(cancelID, netmask, src);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::REQUEST_REFLASH:
         m->decodedRequestReflash(buf->getSource());
         delete buf;
         return NULL;
      case MsgBufferEnums::MESSAGE_REQ:
         m->decodedMessageRequest(buf->accessRawData(0), buf->getLength(),
                                  buf->getSource());
         delete buf;
         return NULL;
      case MsgBufferEnums::PARAM_SYNC:
         m->decodedParamSync(buf->getSource());
         delete buf;
         return NULL;
      case MsgBufferEnums::CELL_REPORT:
         m->decodedCellReport(buf->accessRawData(0), buf->getLength(),
                              buf->getSource());
         delete buf;
         return NULL;
      case MsgBufferEnums::NAV_REQUEST : 
         m->decodedSendRequest( buf );
         delete buf;
         return NULL;
         break;
      default:
         return buf;
      }

   }


   MsgBuffer* 
   NavServerComConsumerDecoder::dispatch(MsgBuffer *buf, 
                                         NavServerComConsumerInterface *m )
   {
      uint32 src = buf->getSource();
      uint32 dst = buf->getDestination();
      int length;
      MsgBuffer& input = *buf;
      length=buf->getLength();
      switch (input.getMsgType()) {
      case MsgBufferEnums::WHERE_AM_I_REPLY:
         {
            const char *country    = input.getNextCharString();
            const char *municipal  = input.getNextCharString();
            const char *city       = input.getNextCharString();
            const char *district   = input.getNextCharString();
            const char *streetname = input.getNextCharString();
            int32 lat = input.readNextUnaligned32bit();
            int32 lon = input.readNextUnaligned32bit();
            m->decodedWhereAmIReply(country, municipal, city, district,
                                    streetname, lat, lon, src, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::SEARCH_REPLY:
         {
#if !defined(NO_LOG_OUTPUT) || defined(_MSC_VER) || defined(__MWERKS__)
            Log log("NSCCD:SEARCH_REPLY");
            Log* m_log = &log; //easier to use LogMacros.
            m_log = m_log;
#endif
            uint8 kind = input.readNext8bit();
            uint16 numMatch = input.readNextUnaligned16bit();
            uint16 start = input.readNextUnaligned16bit();
            uint16 total = input.readNextUnaligned16bit();
            uint16 numRegion = input.readNextUnaligned16bit();
            DBG("Search Reply contains %u %s and %u regions.", numMatch,
                kind == 0 ? "Matches" : "Areas", numRegion);
            DBG("StartIndex: %u, Total: %u", start, total);
            typedef std::vector<SearchRegion*> SRvector;
            typedef std::vector<FullSearchItem*> FSIvector;
            typedef std::vector<SearchArea*> SAvector;
            SRvector regions(numRegion);
            FSIvector matches(numMatch);
            SAvector areas(numMatch);
            for(int j = 0; j < numRegion; ++j){
               GuiProtEnums::RegionType rType = 
                  GuiProtEnums::RegionType(input.readNextUnaligned32bit());
               const char* id = input.getNextCharString();
               const char* name = input.getNextCharString();
               regions[j] = new SearchRegion(rType, id, name);
               DBG("Region[%d] : ('%s', '%s', %#"PRIx32")", 
                   j, regions[j]->getName(), 
                   regions[j]->getId(), regions[j]->getType());
            }
            if(kind == 0){ //Matches
               for(int i = 0; i < numMatch; ++i){
                  // extract info from buffer
                  const char* id = input.getNextCharString();
                  uint16 type = input.readNextUnaligned16bit();
                  uint16 subtype = input.readNextUnaligned16bit();
                  uint32 lat = input.readNextUnaligned32bit();
                  uint32 lon = input.readNextUnaligned32bit();
                  uint32 alt = input.readNextUnaligned32bit();
                  uint8 nbr = input.readNext8bit();
                  int nameLen = input.getReadPos();
                  char* name = strdup_new(input.getNextCharString());
                  nameLen = input.getReadPos() - nameLen;
                  SRvector sr;
                  sr.reserve(nbr+1);
                  sr.resize(nbr, NULL);
                  SearchRegion* addressRegion = NULL;
                  for(int k = 0; k < nbr; ++k){
                     sr[k] = regions[input.readNextUnaligned16bit()];
                     DBG("FSI[%d]-Region[%d]: ('%s', '%s', %#"PRIx32")", i, k,
                         sr[k]->getName(), sr[k]->getId(), sr[k]->getType());
                  }
#define SEPARATE_STREET_NAME
#ifdef SEPARATE_STREET_NAME
                  // separate name and address from the name string.
                  char* comma = strrchr(name, ',');
                  if(comma){
                     char* address = comma;
                     while(! isalnum(int(*++address))) {}
                     *comma = '\0';
                     addressRegion = new SearchRegion(GuiProtEnums::address, 
                                                      "BAD", address); 
                     sr.push_back(addressRegion);
                     DBG("FSI[%d]-Region[%d]: ('%s', '%s', %#"PRIx32")", i, 
                         sr.size() - 1, sr.back()->getName(), 
                         sr.back()->getId(), sr.back()->getType());
                  }
#endif
                  std::sort(sr.begin(), sr.end(), PointerOperator<SearchRegion,
                            std::less<SearchRegion> >());

                  //create FullSearchItem
                  matches[i] = new FullSearchItem(name, id, type, subtype, 0,
                                                  &sr.front(), sr.size(), lat, 
                                                  lon, alt, 0, NULL, 0);
                  DBG("FullSearchItem[%d] : ('%s', '%s', %#x, %#x, %"PRIu32", "
                      "(%u regions), %"PRIu32", %"PRIu32", %"PRIu32", "
                      "(%u infos), %#"PRIx32")", i, 
                      matches[i]->getName(), matches[i]->getID(), 
                      matches[i]->getType(), matches[i]->getSubType(), 
                      matches[i]->getDistance(), matches[i]->noRegions(), 
                      matches[i]->getLat(), matches[i]->getLon(), 
                      matches[i]->getAlt(), matches[i]->noAdditionalInfo(), 
                      matches[i]->getSearchModuleStatusFlags());
                  delete[] name;
                  delete addressRegion;
               }
               m->decodedSearchReply(0, NULL, matches.size(), &matches.front(),
                                     start, total, src, dst);
            } else { //areas
               for(int i = 0; i < numMatch; ++i){
                  const char* id = input.getNextCharString();
                  const char* name = input.getNextCharString();
                  uint8 nbr = input.readNext8bit();
                  uint32 type = input.readNextUnaligned32bit();
                  type = type;
                  SRvector sr(nbr);
                  for(int k = 0; k < nbr; ++k){
                     sr[k] = regions[input.readNextUnaligned16bit()];
                  }
                  areas[i] = new SearchArea(name, id);
                  DBG("SearchArea[%d] : ('%s', '%s')", i,
                      areas[i]->getName(), areas[i]->getID());
               }
               m->decodedSearchReply(numMatch, &areas.front(), 0, NULL, 
                                     start, total, src, dst);
            }
            std::for_each(areas.begin(), areas.end(), Delete<SearchArea*>());
            std::for_each(matches.begin(), matches.end(),
                          Delete<FullSearchItem*>());
            std::for_each(regions.begin(), regions.end(), 
                          Delete<SearchRegion*>());
            
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::ADDITIONAL_INFO_REPLY:
         {
#if !defined(NO_LOG_OUTPUT) || defined(_MSC_VER) || defined(__MWERKS__)
            Log log("AdditionalInfo");
            Log* m_log = &log;
#endif
            // DBGDUMP("AdditionalInfo data", buf->accessRawData(), 
            //         buf->remaining());
            uint16 num = buf->readNextUnaligned16bit();
            DBG("Number of ids: %d", num);
            std::vector<AdditionalInfo*> info; info.reserve(20);
            std::vector<const char*> id(num, NULL);
            std::vector<unsigned> index(num, 0);
            for(int i = 0; i < num; ++i){
               id[i] = buf->getNextCharString();
               index[i] = info.size();
               DBG("id '%s' starts writing AIs at index %u", id[i], index[i]);
               uint8 ais = buf->readNext8bit();
               DBG("id '%s' has %u AIs", id[i], ais);
               if(info.capacity() - info.size() <= ais){
                  info.reserve(info.capacity() + ais);
               }
               for(int j = 0; j < ais; ++j){
                  uint32 type = buf->readNextUnaligned32bit();
                  const char* key = buf->getNextCharString();
                  const char* value = buf->getNextCharString();
                  info.push_back(new AdditionalInfo(key, value, type));
                  DBG("info[%d] == (%s, %s, %#"PRIx32")", 
                      j, info.back()->getKey(), 
                      info.back()->getValue(), info.back()->getType());
               }
            }
            m->decodedSearchInfoReply(num, &(id.front()), &(index.front()),
                                      info.size(), &(info.front()), src, dst);
            std::for_each(info.begin(), info.end(), Delete<AdditionalInfo*>());
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::ROUTE_REPLY:
         {
            m->decodedRouteReply(src, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::MAP_REPLY:
         {
#ifndef NO_LOG_OUTPUT
            Log l("NSCCP::MAP_REPLY");
#endif
#ifndef NO_LOG_OUTPUT
            //l.debugDump("MapReply:", buf->accessRawData(), buf->remaining());
#endif
            int32 tlLat     = buf->readNextUnaligned32bit();
            int32 tlLon     = buf->readNextUnaligned32bit();
            int32 brLat     = buf->readNextUnaligned32bit();
            int32 brLon     = buf->readNextUnaligned32bit();
            uint16 iwidth   = buf->readNextUnaligned16bit();
            uint16 iheight  = buf->readNextUnaligned16bit();
            uint32 rwwidth  = buf->readNextUnaligned32bit();
            uint32 rwheight = buf->readNextUnaligned32bit();
            uint32 size     = buf->readNextUnaligned32bit();
            enum MapEnums::ImageFormat type = MapEnums::ImageFormat(buf->readNextUnaligned16bit());
#ifndef NO_LOG_OUTPUT
            l.debug("[%"PRId32",%"PRId32"][%"PRId32",%"PRId32"], "
                    "%"PRIu32" bytes, imagetype: %d, "
                    "meters: [%"PRIu32",%"PRIu32"]", 
                    tlLat, tlLon, brLat, brLon, size, type, 
                    rwwidth, rwheight);
#endif
            const uint8* buffer = buf->accessRawData(); 
            //l.debugDump("imagebuffer", buffer, size);
            m->decodedMapReply(BoxBox(tlLat, tlLon, brLat, brLon), 
                               rwwidth, rwheight, iwidth, iheight, 
                               type, size, buffer, buf->getDestination());
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::BINARY_DOWNLOAD:
         {
            size_t length = input.getLength() - input.getReadPos();
            const uint8* data = input.accessRawData();
            m->decodedBinaryDownload(data, length, src, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::BINARY_UPLOAD_REPLY:
         m->decodedBinaryUploadReply(src, dst);
         delete buf;
         return NULL;
      case MsgBufferEnums::PROGRESS_MESSAGE:
         {
            ComStatus status = ComStatus(buf->readNext8bit());
            GuiProtEnums::ServerActionType type = 
               GuiProtEnums::ServerActionType(buf->readNext8bit());
            uint32 sent   = buf->readNextUnaligned32bit();
            uint32 total  = buf->readNextUnaligned32bit();

            m->decodedProgressMessage(status, type, sent, total, src, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::MESSAGE_REPLY:
         {
         int length;
         length = buf->readNextUnaligned32bit();
         const uint8* data;
         data = NULL;
         if (length > 0) {
            data = buf->accessRawData(4); // do not include the length
         }
         m->decodedMessageReply(data, length, 
                                src, dst);
         delete buf;
         return NULL;
         }
      case MsgBufferEnums::LICENSE_KEY_REPLY:
         {
            bool key     = !!buf->readNext8bit();
            bool phone   = !!buf->readNext8bit();
            bool region  = !!buf->readNext8bit();
            bool nameOk  = !!buf->readNext8bit();   
            bool emailOk = !!buf->readNext8bit();
            GuiProtEnums::WayfinderType type = 
               GuiProtEnums::WayfinderType(buf->readNext8bit());
            m->decodedLicenseReply(key, phone, region, nameOk, emailOk, 
                                   type, src, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::LATEST_NEWS:
         {
            uint32 crc = buf->readNextUnaligned32bit();
            m->decodedLatestNews(crc, buf->accessRawData(), buf->remaining(),
                                 src, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::PARAM_SYNC_REPLY:
         {
            GuiProtEnums::WayfinderType wft = 
               GuiProtEnums::WayfinderType(buf->readNextUnaligned32bit());
            m->decodedParamSyncReply(wft, src, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::VECTOR_MAP_REPLY:
         {
            const char* reqid = buf->getNextCharString();
            uint32 size = buf->readNextUnaligned32bit();
            const uint8* data = buf->accessRawData();
            m->decodedVectorMapReply(reqid, size, data, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::MULTI_VECTOR_MAP_REPLY:
         {
            uint32 size = buf->readNextUnaligned32bit();
            const uint8* data = buf->accessRawData();
            m->decodedMulitVectorMapReply(size, data, dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::FORCEFEED_MULTI_VECTOR_MAP_REPLY:
         {
            uint32 size = buf->readNextUnaligned32bit();
            const uint8* data = buf->accessRawData();
            m->decodedForceFeedMuliVectorMapReply( size, data, dst );
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::CELL_CONFIRM:
         {
            uint32 size = buf->readNextUnaligned32bit();
            m->decodedCellConfirm(size, buf->accessRawData(), dst);
            delete buf;
            return NULL;
         }
      case MsgBufferEnums::NAV_REPLY:
         {
            //TRACE_DBG( "Pub got NAV_REPLY" );
            uint32 size = buf->getLength();
            const uint8* data = buf->accessRawData();
            NavReplyPacket r( data, size );

            m->decodedReply(
                  NavReplyPacket::getRequestType( data, size ),
                  r.getParamBlock(),
                  r.getReqVer(),
                  r.getStatusCode(),
                  r.getStatusMessage(),
                  src,
                  dst
                  );
            delete buf;
            return NULL;
         }
      default:
         {}
      }
      return buf;
   }


   typedef GuiProtEnums::ServerActionType ActionType;
   uint32 NavServerComConsumerPublic::progressMessage(ComStatus status, 
                                                      ActionType type,
                                                      uint32 sent, 
                                                      uint32 total,
                                                      uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = uint32(Module::NSCProgressMessages) | 
            uint32(MsgBufferEnums::ADDR_MULTICAST_UP) ;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::PROGRESS_MESSAGE,12);
      buf->writeNext8bit(status);
      buf->writeNext8bit(type);
      buf->writeNextUnaligned32bit(sent);
      buf->writeNextUnaligned32bit(total);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComConsumerPublic::messageReply(const uint8* data, 
                                                   unsigned len, 
                                                   uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::MESSAGE_REPLY, len);
      if(len > 0)
         buf->writeNextByteArray(data, len);
      m_queue->insert(buf);
      return src;
   } 

   uint32 NavServerComConsumerPublic::whereAmIReply(const char* country,
                                                    const char* municipal, 
                                                    const char* city,
                                                    const char* district, 
                                                    const char* streetname,
                                                    int32 lat, 
                                                    int32 lon, 
                                                    uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      char empty = '\0';
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::WHERE_AM_I_REPLY, 
                                     64);
      buf->writeNextCharString((country) ? country : &empty);
      buf->writeNextCharString((municipal) ? municipal : &empty);
      buf->writeNextCharString((city) ? city : &empty);
      buf->writeNextCharString((district) ? district : &empty);
      buf->writeNextCharString((streetname) ? streetname : &empty);
      buf->writeNextUnaligned32bit(lat);
      buf->writeNextUnaligned32bit(lon);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComConsumerPublic::messageReply(int length,
                                                   const char* data,
                                                   uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::MESSAGE_REPLY, 
                                     64+length);
      buf->writeNextUnaligned32bit(length);
      buf->writeNextByteArray((const uint8*)data, length);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComConsumerPublic::searchReply(const uint8* data, 
                                                  int len, 
                                                  uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::SEARCH_REPLY,
                                     len + 10);
      if(data){
         buf->writeNextByteArray(data, len);
      }
      m_queue->insert(buf);
      return src;
   }


   uint32 NavServerComConsumerPublic::searchInfoReply(const uint8* data,
                                                      int len,
                                                      uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, 
                                     MsgBufferEnums::ADDITIONAL_INFO_REPLY, 
                                     len + 10);
      if(data){
         buf->writeNextByteArray(data, len);
      }
      m_queue->insert(buf);
      return src;
   }

   uint32 
   NavServerComConsumerPublic::licenseKeyReply(bool keyOk, bool phoneOk,
                                               bool regionOk, bool nameOk,
                                               bool emailOk,
                                               GuiProtEnums::WayfinderType wfType,
                                               uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer* buf = new MsgBuffer(dst, src, MsgBufferEnums::LICENSE_KEY_REPLY,4);
      buf->writeNext8bit(keyOk);
      buf->writeNext8bit(phoneOk);
      buf->writeNext8bit(regionOk);
      buf->writeNext8bit(nameOk);
      buf->writeNext8bit(emailOk);      
      buf->writeNext8bit(wfType);
      m_queue->insert(buf);
      return src;
   }


   uint32 NavServerComConsumerPublic::routeReply(uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::ROUTE_REPLY,
                                     12);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComConsumerPublic::mapReply(int len, const uint8* data, 
                                               uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::MAP_REPLY, len + 10);
      if(data){
         buf->writeNextByteArray(data, len);
      }
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComConsumerPublic::vectorMapReply(uint32 size, 
                                                     const uint8* data, 
                                                     uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, 
                                     MsgBufferEnums::VECTOR_MAP_REPLY, size + 12);
      buf->writeNextByteArray(data, size);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComConsumerPublic::multiVectorMapReply(uint32 length,
                                                          const uint8* data,
                                                          uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, 
                                     MsgBufferEnums::MULTI_VECTOR_MAP_REPLY, 
                                     length + 12);
      buf->writeNextByteArray(data, length);
      m_queue->insert(buf);
      return src;
   }


   uint32 
   NavServerComConsumerPublic::unsolMultiVectorMapReply(
      uint32 length, const uint8* data, uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer( 
         dst, src, 
         MsgBufferEnums::FORCEFEED_MULTI_VECTOR_MAP_REPLY, 
         length + 12 );
      buf->writeNextByteArray(data, length);
      m_queue->insert(buf);
      return src;
   }


   uint32 NavServerComConsumerPublic::cellConfirm(uint32 length, 
                                                  const uint8* data,
                                                  uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::CELL_CONFIRM, 
                                     length + 12);
      buf->writeNextByteArray(data, length);
      m_queue->insert(buf);
      return src;      
   }
      
   uint32 NavServerComConsumerPublic::binaryUploadReply(uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst; //XXX
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src,
                                     MsgBufferEnums::BINARY_UPLOAD_REPLY, 12);
      m_queue->insert(buf);
      return src;
   }

   /**XXX  Queue? Isn't this going in the wrong direction? */
   uint32 NavServerComConsumerPublic::binaryDownload(const uint8* data, 
                                                     size_t length,
                                                     uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst; //XXX
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::BINARY_DOWNLOAD,
                                     length + 20);
      buf->writeNextByteArray(data, length);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComConsumerPublic::latestNews(uint32 checksum,
                                                 const uint8* data, 
                                                 unsigned size, 
                                                 uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = uint32(Module::LatestNews) | 
            uint32(MsgBufferEnums::ADDR_MULTICAST_UP) ;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::LATEST_NEWS,
                                     size + 20);
      buf->writeNextUnaligned32bit(checksum);
      buf->writeNextByteArray(data, size);
      m_queue->insert(buf);
      return src;
   }

   
   uint32 
   NavServerComConsumerPublic::paramSyncReply(GuiProtEnums::WayfinderType wft,
                                              uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::PARAM_SYNC_REPLY, 4);
      buf->writeNextUnaligned32bit(wft);
      m_queue->insert(buf);
      return src;
   }


   uint32
   NavServerComConsumerPublic::sendReply( const uint8* data, size_t length,
                                          uint32 dst ) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer( dst, src, MsgBufferEnums::NAV_REPLY,
                                      length + 20 );
      buf->writeNextByteArray( data, length );
      m_queue->insert( buf );
      return src;
   }


   uint32 NavServerComProviderPublic::setServerParams(const uint8* data, 
                                                      size_t length,
                                                      uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::SET_SERVER_PARAMS,
                                     length);
      buf->writeNextByteArray(data, length);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComProviderPublic::whereAmI(int32 lat, int32 lon, 
                                               uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();

      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::WHERE_AM_I_REQUEST,
                                     12);
      buf->writeNextUnaligned32bit(lat);
      buf->writeNextUnaligned32bit(lon);
      m_queue->insert(buf);
      return src;
   }
   uint32 
   NavServerComProviderPublic::searchRequest(const char* searchCity, 
                                             const char* searchDestination, 
                                             const char* areaId, 
                                             uint16 startIndex, uint32 lat, 
                                             uint32 lon, uint8 heading,
                                             uint32 country,
                                             uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::SEARCH_REQUEST, 30);
      buf->writeNextUnaligned16bit(startIndex);
      buf->writeNextCharString(searchCity);
      buf->writeNextCharString(areaId);
      buf->writeNextCharString(searchDestination);
      buf->writeNextUnaligned32bit(country);
      buf->writeNextUnaligned32bit(lat);
      buf->writeNextUnaligned32bit(lon);
      buf->writeNext8bit(heading);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComProviderPublic::requestSearchInfo(uint16 num, 
                                                        const char* const* ids,
                                                        const char*const* vals,
                                                        uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = 
         new MsgBuffer(dst, src, MsgBufferEnums::ADDITIONAL_INFO_REQ, 30);
      
      buf->writeNextUnaligned16bit(num);
      for(int i = 0; i < num; ++i){
         buf->writeNextCharString(ids[i]);
         if(vals){
            buf->writeNextCharString(vals[i]);
         }else{
            buf->writeNextCharString(NULL);
         }
      }

      m_queue->insert(buf);
      return src;

   }

   uint32 NavServerComProviderPublic::verifyLicenseKey(const char* key, 
                                                       const char* phone, 
                                                       const char* name,
                                                       const char* email,
                                                       const char* optional,
                                                       uint32 region, 
                                                       uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::LICENSE_KEY, 40);
      buf->writeNextUnaligned32bit(region);
      buf->writeNextCharString(key);
      buf->writeNextCharString(phone);
      buf->writeNextCharString(name);
      buf->writeNextCharString(email);
      buf->writeNextCharString(optional);
      m_queue->insert(buf);
      return src;
   }

   uint32 
   NavServerComProviderPublic::originalSearch(const char* searchCity, 
                                              const char* searchDestination,
                                              uint16 startIndex,
                                              uint32 countryCode, int32 lat, 
                                              int32 lon, uint8 heading, 
                                              uint32 dst) const
   {
      return searchRequest(searchCity, searchDestination, NULL, 
                           startIndex, lat, lon, heading, countryCode, dst);
   }

   uint32 NavServerComProviderPublic::refinedSearch(const char* destination, 
                                                    const char* areaId,
                                                    uint16 startIndex,
                                                    uint32 countryCode,
                                                    int32 lat, int32 lon,
                                                    uint8 heading,
                                                    uint32 dst) const
   {
      return searchRequest(NULL, destination, areaId, startIndex,
                           lat, lon, heading, countryCode, dst);
   }

   uint32 NavServerComProviderPublic::routeToGps(int32 toLat, int32 toLon, 
                                                 uint16 heading, 
                                                 int32 fromLat, int32 fromLon, 
                                                 uint32 routeRequestorId,
                                                 int64 oldRouteId,
                                                 uint8 rerouteReason,
                                                 uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::ROUTE_REQUEST, 20);
      buf->writeNextUnaligned32bit(fromLat);
      buf->writeNextUnaligned32bit(fromLon);
      buf->writeNextUnaligned32bit(toLat);
      buf->writeNextUnaligned32bit(toLon);
      buf->writeNextUnaligned16bit(heading);
      buf->writeNextUnaligned64bit(oldRouteId);
      buf->writeNextUnaligned32bit(routeRequestorId);
      buf->writeNext8bit(rerouteReason);
      m_queue->insert(buf);
      return src;
   }
   

   uint32 NavServerComProviderPublic::mapRequest(const class BoundingBox& bb,
                                                 uint16 imageWidth, 
                                                 uint16 imageHeight,
                                                 uint16 vbWidth,
                                                 uint16 vbHeight,
                                                 enum MapEnums::ImageFormat format,
                                                 uint16 noMapItems,
                                                 const MapItem*  items,
                                                 uint16 noExtras,
                                                 const ExtraMapInfo*info,
                                                 uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::MAP_REQUEST, 40);
      
      bb.serialize(buf);
      buf->writeNextUnaligned16bit(imageWidth);
      buf->writeNextUnaligned16bit(imageHeight);
      buf->writeNextUnaligned16bit(vbWidth);
      buf->writeNextUnaligned16bit(vbHeight);
      buf->writeNextUnaligned16bit(format);
      buf->writeNextUnaligned16bit(noMapItems);
      for(int i = 0; i < noMapItems; ++i){
         items[i].serialize(buf);
      }
      buf->writeNextUnaligned16bit(noExtras);
      for(int j = 0; j < noExtras; ++j){
         info[j].serialize(buf);
      }

      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComProviderPublic::mapRequest(const uint8* data, 
                                                 unsigned length, 
                                                 uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::MAP_REQUEST, length);

      buf->writeNextByteArray(data, length);

      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComProviderPublic::vectorMapRequest(const char* request,
                                                       uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::VECTOR_MAP_REQUEST, 
                                     strlen(request) + 12);

      buf->writeNextCharString(request);

      m_queue->insert(buf);
      return src;
   }      

   uint32 NavServerComProviderPublic::multiVectorMapRequest(uint32 startOffset,
                                                            uint32 maxBytes,
                                                            const uint8* requestStringData,
                                                            uint32 dataLength, 
                                                            uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, 
                                     MsgBufferEnums::MULTI_VECTOR_MAP_REQUEST, 
                                     dataLength + 16);
//      buf->writeNextUnaligned32bit(dataLength);
      buf->writeNextUnaligned32bit(startOffset);
      buf->writeNextUnaligned32bit(maxBytes);
      buf->writeNextByteArray(requestStringData, dataLength);

      m_queue->insert(buf);
      return src;
   }      

   uint32 NavServerComProviderPublic::multiVectorMapRequest(uint32 size,
                                                            const uint8* data,
                                                            uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, 
                                     MsgBufferEnums::MULTI_VECTOR_MAP_REQUEST, 
                                     size + 16);
      buf->writeNextByteArray(data, size);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComProviderPublic::cellReport(uint32 size, 
                                                 const uint8* data, 
                                                 uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::CELL_REPORT, size+8);
      buf->writeNextUnaligned32bit(size);
      buf->writeNextByteArray(data, size);

      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComProviderPublic::binaryUpload(int len, const uint8 * data,
                                                   uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::BINARY_UPLOAD,
                                     len + 12);
      buf->writeNextByteArray(data, len);
      m_queue->insert(buf);
      return src;
   }

   uint32 
   NavServerComProviderPublic::cancelRequest(uint32 messageSrc, 
                                             uint32 dst) const
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::CANCEL_NSC_REQUEST,
                                     16);
      buf->writeNextUnaligned32bit(messageSrc);
      buf->writeNextUnaligned32bit(m_owner->getNetmask());
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComProviderPublic::requestReflash(uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::REQUEST_REFLASH, 0);
      m_queue->insert(buf);
      return src;
   }

   uint32 
   NavServerComProviderPublic::messageRequest(GuiProtEnums::UserMessageType messType, 
                                              GuiProtEnums::ObjectType object, 
                                              const char* to, const char* from,
                                              const char* sig, const char* id,
                                              uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::MESSAGE_REQ, 64);
      buf->writeNext8bit(messType);
      buf->writeNext8bit(object);
      buf->writeNextCharString(id);
      buf->writeNextCharString(to);
      buf->writeNextCharString(from);
      buf->writeNextCharString(sig);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavServerComProviderPublic::paramSync(uint32 dst) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::PARAM_SYNC, 0);
      m_queue->insert(buf);
      return src;
   }



   uint32 NavServerComProviderPublic::sendRequest( 
      navRequestType::RequestType type,
      NParamBlock& params,
      uint8 requestVer,
      uint32 dst ) const
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      std::vector< byte > buff;
      params.writeParams( buff, NavPacket::getCurrentProtoVer() );
      MsgBuffer *buf = new MsgBuffer(
         dst, src, MsgBufferEnums::NAV_REQUEST, 5 + buff.size() );
      buf->writeNext8bit( NavPacket::getCurrentProtoVer() );
      buf->writeNextUnaligned32bit( type );
      buf->writeNext8bit( requestVer );
      buf->writeNextByteArray( &buff.front(), buff.size() );
      
      m_queue->insert( buf );
      return src;      
   }

}
