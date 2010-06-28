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

#include "ModuleQueue.h"

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Quality.h"
#include "RouteEnums.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "NavTask.h"

#include "RouteEnums.h"
#include "RouteInfo.h"
#include "GpsSatelliteInfo.h"

#include "Buffer.h"
#include "MsgBuffer.h"

#include "GuiFileOperation.h"

namespace isab {
   using namespace RouteEnums;   

   MsgBuffer * NavTaskProviderDecoder::dispatch(MsgBuffer *buf, 
                                                NavTaskProviderInterface *m )
   {
      int length;

      length=buf->getLength();
      switch (buf->getMsgType()) {
         case MsgBufferEnums::NEW_ROUTE:
            {
               bool routeOrigFromGPS = buf->readNext8bit();
               int32 origLat=buf->readNext32bit();
               int32 origLon=buf->readNext32bit();
               int32 destLat=buf->readNext32bit();
               int32 destLon=buf->readNext32bit();
               int32 packetLength=buf->readNext32bit();
               m->decodedNewRoute(routeOrigFromGPS, origLat, origLon, destLat, destLon, packetLength, 
                                  buf->getSource() );
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::ROUTE_CHUNK:
            {
               bool failedRoute = buf->readNext32bit() != 0;
               size_t readPos = buf->getReadPos();
               int  chunkLength = buf->getLength() - readPos;
               const uint8 *chunkData = buf->accessRawData(readPos);
               if (! chunkData) {
//                  cerr << "RouteChunk:no data" << endl;
               } else {
                  m->decodedRouteChunk(failedRoute, chunkLength, chunkData, buf->getSource());
               }
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::KEEP_ROUTE:
            {
               int64 routeid = buf->readNext64bit();
               m->decodedKeepRoute(routeid, 
                                   buf->getSource(), buf->getDestination());
               return NULL;
            }
         case MsgBufferEnums::NAV_TASK_COMMAND:
            {
               enum NavTaskProviderPublic::CommandType cmd;
               cmd = NavTaskProviderPublic::CommandType(buf->readNext32bit());
               m->decodedCommand(cmd, buf->getSource());
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::NAV_TASK_FILE_OPERATION:
            {
               GuiFileOperation* op = GuiFileOperation::createFromBuffer(buf);
               m->decodedFileOperation(op, buf->getSource());
               delete(op);
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::NT_ROUTE_REQUEST:
            {
               buf->setReadPos(0);
               int32 fromLat   = buf->readNextUnaligned32bit();
               int32 fromLon   = buf->readNextUnaligned32bit();
               int32 toLat = buf->readNextUnaligned32bit();
               int32 toLon = buf->readNextUnaligned32bit();
               m->decodedNTRouteToGps(toLat, toLon, fromLat, fromLon,
                                    buf->getSource());
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::GET_FILTERED_ROUTE_LIST:
            {
               int16 startWpt = buf->readNextUnaligned16bit();
               int16 numWpts = buf->readNextUnaligned16bit();
               m->decodedGetFilteredRouteList(startWpt, numWpts,
                                    buf->getSource());
               delete buf;
               return NULL;
            }
         default:
            return buf;
      }

   }

   MsgBuffer * NavTaskConsumerDecoder::dispatch(MsgBuffer *buf, NavTaskConsumerInterface *m )
   {
      int length;

      length=buf->getLength();
      switch (buf->getMsgType()) {
         case MsgBufferEnums::NT_GPS_SATELLITE_INFO:
            {
               class GpsSatInfoHolder *satInfo = new class GpsSatInfoHolder();
               satInfo->readFromBuffer(buf);
               m->decodedGpsSatelliteInfo(satInfo, buf->getSource());
               delete satInfo;
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::NAV_TASK_FILE_OPERATION:
            {
               class GuiFileOperation *op = GuiFileOperation::createFromBuffer(buf);
               m->decodedFileOperation(op, buf->getSource());
               delete op;
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::POSITION_STATE:
            {
               struct PositionState p;
               p.lat = buf->readNext32bit();
               p.lon = buf->readNext32bit();
               p.alt = buf->readNext32bit();
               p.speed = buf->readNext16bit();
               p.heading = buf->readNext8bit();
               p.gpsQuality = (enum Quality)(buf->readNext8bit());
               /* p.time = ?????? */ buf->readNext32bit();
               p.timeStampMillis = buf->readNext32bit();
               uint32 numHints = buf->readNext32bit();
               for (unsigned int i = 0; i < numHints; i++) {
                  HintNode curHint;
                  curHint.lat = buf->readNext32bit();
                  curHint.lon = buf->readNext32bit();
                  curHint.velocityCmPS = buf->readNext32bit();
                  p.positionHints.push_back(curHint);
               }
               m->decodedPositionState(p, buf->getSource());
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::NEW_ROUTE_COORD:
            {
               int32 lat = buf->readNext32bit();
               int32 lon = buf->readNext32bit();
               uint8 dir = buf->readNext8bit();
               m->setRouteCoordinate(lat, lon, dir);
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::ROUTE_INFO:
            {
               RouteInfo r;
               r.readFromBuf(buf);
               m->decodedRouteInfo(r, buf->getSource());
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::ROUTE_LIST:
            {
               m->decodedRouteList(buf, buf->getSource(), buf->getDestination());
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::INVALIDATE_ROUTE:
            {
               bool newRouteAvailable = (buf->readNext8bit() != 0);
               int64 routeid = buf->readNextUnaligned64bit();
               int32 tLat = buf->readNext32bit();
               int32 lLon = buf->readNext32bit();
               int32 bLat = buf->readNext32bit();
               int32 rLon = buf->readNext32bit();
               int32 oLat = buf->readNext32bit();
               int32 oLon = buf->readNext32bit();
               int32 dLat = buf->readNext32bit();
               int32 dLon = buf->readNext32bit();
               m->decodedInvalidateRoute(newRouteAvailable, routeid,
                                         tLat, lLon, bLat, rLon,
                                         oLat, oLon, dLat, dLon,
                                         buf->getSource());
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::NT_ROUTE_REPLY:
            {  
               m->decodedNTRouteReply(buf->readNextUnaligned64bit(),
                                      buf->getSource(), buf->getDestination());
               delete buf;
               return NULL;
            }  
         default:
            return buf;
      }

   }

   uint32 NavTaskConsumerPublic::satelliteInfo(
         class GpsSatInfoHolder *satInfo,
         uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::NT_GPS_SATELLITE_INFO,
            satInfo->size()*16+4);

      satInfo->writeToBuffer(buf);

      m_queue->insert(buf);

      return src;
   }

   uint32 NavTaskConsumerPublic::sendFileOperation(
         class GuiFileOperation *op,
         uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src,
            MsgBufferEnums::NAV_TASK_FILE_OPERATION, 128);

      op->writeToBuffer(buf);

      m_queue->insert(buf);

      delete op;
      return src;
   }

   uint32 NavTaskConsumerPublic::positionState(const PositionState &p, uint32 dst) 
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::POSITION_STATE,16);
      buf->writeNext32bit(p.lat);
      buf->writeNext32bit(p.lon);
      buf->writeNext32bit(p.alt);
      buf->writeNext16bit(p.speed);
      buf->writeNext8bit (p.heading);
      buf->writeNext8bit (p.gpsQuality);
      buf->writeNext32bit(p.time.millis());
      buf->writeNext32bit(p.timeStampMillis);
      buf->writeNext32bit(p.positionHints.size());
      
      for(unsigned int i = 0; i < p.positionHints.size(); i++) {
         buf->writeNext32bit(p.positionHints[i].lat);
         buf->writeNext32bit(p.positionHints[i].lon);
         buf->writeNext32bit(p.positionHints[i].velocityCmPS);
      }
      
      m_queue->insert(buf);
      return src;
   }
   
   uint32 NavTaskConsumerPublic::sendRouteCoord(int32 lat, int32 lon, uint8 dir, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::NEW_ROUTE_COORD,8);
      buf->writeNext32bit(lat);
      buf->writeNext32bit(lon);
      buf->writeNext8bit(dir);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskConsumerPublic::routeInfo(const RouteInfo &r, uint32 dst) 
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::ROUTE_INFO,sizeof(RouteInfo)+8);
      r.writeToBuf(buf);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskConsumerPublic::routeList(const RouteList &rl, uint32 dst) 
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::ROUTE_LIST,20+8);
      rl.writeToBuf(buf);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskConsumerPublic::invalidateRoute(bool newRouteAvailable,
         int64 routeid,
         int32 tLat, int32 lLon, int32 bLat, int32 rLon,
         int32 oLat, int32 oLon, int32 dLat, int32 dLon,
         uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::INVALIDATE_ROUTE, 1);
      buf->writeNext8bit(newRouteAvailable);
      buf->writeNextUnaligned64bit(routeid);

      buf->writeNext32bit(tLat);
      buf->writeNext32bit(lLon);
      buf->writeNext32bit(bLat);
      buf->writeNext32bit(rLon);

      buf->writeNext32bit(oLat);
      buf->writeNext32bit(oLon);
      buf->writeNext32bit(dLat);
      buf->writeNext32bit(dLon);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskConsumerPublic::ntRouteReply(int64 routeid, uint32 dst) const
   {           
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId(); 
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::NT_ROUTE_REPLY,
                                     12);
      buf->writeNextUnaligned64bit(routeid);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskProviderPublic::command(enum CommandType command, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::NAV_TASK_COMMAND,4);
      buf->writeNext32bit(command);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskProviderPublic::fileOperation(class GuiFileOperation* op,
         uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::NAV_TASK_FILE_OPERATION, 128);
      op->writeToBuffer(buf);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskProviderPublic::newRoute(bool routeOrigFromGPS,
                                          int32 origLat, int32 origLon,
                                          int32 destLat, int32 destLon, 
                                          int packetLength, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::NEW_ROUTE,12);
      buf->writeNext8bit(routeOrigFromGPS);
      buf->writeNext32bit(origLat);
      buf->writeNext32bit(origLon);
      buf->writeNext32bit(destLat);
      buf->writeNext32bit(destLon);
      buf->writeNext32bit(packetLength);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskProviderPublic::routeChunk(bool failedRoute, 
         int chunkLength, const uint8 *chunkData, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::ROUTE_CHUNK,12);
      buf->writeNext32bit(failedRoute);
      buf->writeNextByteArray(chunkData, chunkLength);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskProviderPublic::keepRoute(int64 routeid, uint32 dst)
   {
      if(dst == MsgBufferEnums::ADDR_DEFAULT){
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      class MsgBuffer* buf = 
         new MsgBuffer(dst, src, MsgBufferEnums::KEEP_ROUTE, 16);
      buf->writeNext64bit(routeid);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskProviderPublic::ntRouteToGps(int32 toLat, int32 toLon, 
                                            int32 fromLat, int32 fromLon, 
                                            uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, MsgBufferEnums::NT_ROUTE_REQUEST, 20);
      buf->writeNextUnaligned32bit(fromLat);
      buf->writeNextUnaligned32bit(fromLon);
      buf->writeNextUnaligned32bit(toLat);
      buf->writeNextUnaligned32bit(toLon);
      m_queue->insert(buf);
      return src;
   }

   uint32 NavTaskProviderPublic::getFilteredRouteList(int16 startWpt, int16 numWpts, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::GET_FILTERED_ROUTE_LIST,4);
      buf->writeNextUnaligned16bit(startWpt);
      buf->writeNextUnaligned16bit(numWpts);
      m_queue->insert(buf);
      return src;
   }



} /* namespace isab */
