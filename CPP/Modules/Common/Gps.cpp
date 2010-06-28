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
#include "ModuleQueue.h"
#include "Gps.h"

#include "Buffer.h"
#include "MsgBuffer.h"
#include "GpsSatelliteInfo.h"

namespace isab {
   
uint32 GpsProviderPublic::resetReceiver(uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   MsgBuffer *buf=new MsgBuffer(dst, m_owner->getRequestId(), 
         MsgBufferEnums::GPS_RESET_RECEIVER,0);

//   cout << "GpsProviderPublic::resetReceiver " << endl;
   m_queue->insert(buf);
   return 0;
}

uint32 GpsProviderPublic::connect(GpsConsumerPublic *consumer)
{
//   cout << "GpsProviderPublic::connect" << endl;
   return lowlevelConnectTo(consumer);
}

uint32 GpsConsumerPublic::gpsPositionVelocity(
      const struct GpsPositionVelocityVector &v,
      uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::GPS_POSITION_VELOCITY, 4*7);

   buf->writeNextFloat(v.latitude);
   buf->writeNextFloat(v.longitude);
   buf->writeNextFloat(v.altitude);
   buf->writeNext8bit(v.heading);
   buf->writeNextFloat(v.speed_over_ground);
   buf->writeNextFloat(v.vertical_speed);
   buf->writeNext16bit(v.weekno_for_fix);
   buf->writeNextFloat(v.tow_for_fix);
   buf->writeNext32bit(v.fix_time.millis());  // BROKEN!!!! FIXME!!!!
   buf->writeNext16bit(v.constellation_no);
   buf->writeNextFloat(v.latency);
   // Use insertUnique since we want to replace any existing GPS_POSITION_VELOCITY
   // messages, in order to avoid letting the queue grow bigger and bigger
   // with old unprocessed gps positions.
   //
   // In an ideal world the queue would see that message was of such a type that only
   // one (the latest) was allowed in the queue.
   m_queue->insertUnique(buf);
   return src;
}

uint32 GpsConsumerPublic::gpsSatInfo(class GpsSatInfoHolder* satInfo,
      uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::GPS_SATELLITE_INFO,
         satInfo->size()*16+4);

   satInfo->writeToBuffer(buf);

   m_queue->insert(buf);

   return src;
}

uint32 GpsConsumerPublic::gpsStatus(isabTime t,
              enum GpsStatus sum_status,
              uint16 error_conditions,
              float pdop,
              uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::GPS_STATUS, 12);

   buf->writeNext32bit(t.millis());
   buf->writeNext16bit(sum_status);
   buf->writeNext16bit(error_conditions);
   buf->writeNextFloat(pdop);
   m_queue->insert(buf);
   return 0;
}

uint32 GpsConsumerPublic::gpsTime(isabTime /*t*/, 
            float /*tow*/,
            int /*weekno*/,
            float /*utc_offset*/,
            uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   src=src;
   return 0;
}



MsgBuffer * GpsConsumerDecoder::dispatch(MsgBuffer *buf, GpsConsumerInterface *m )
{
   int length;

//   cout << "GpsConsumerDecoder::dispatch : " << buf->getMsgType() << endl;
   length=buf->getLength();
   switch (buf->getMsgType()) {
      case MsgBufferEnums::GPS_SATELLITE_INFO:
         {
            GpsSatInfoHolder *satInfo = new GpsSatInfoHolder();

            satInfo->readFromBuffer(buf);

            m->decodedGpsSatelliteInfo(satInfo, buf->getSource());

            delete satInfo;
         }
         delete(buf);
         return NULL;
      case MsgBufferEnums::GPS_POSITION_VELOCITY:
         {
            struct GpsConsumerPublic::GpsPositionVelocityVector v;
            v.latitude          = buf->readNextFloat();
            v.longitude         = buf->readNextFloat();
            v.altitude          = buf->readNextFloat();
            v.heading           = buf->readNext8bit();
            v.speed_over_ground = buf->readNextFloat();
            v.vertical_speed    = buf->readNextFloat();
            v.weekno_for_fix    = buf->readNext16bit();
            v.tow_for_fix       = buf->readNextFloat();
            /*v.fix_time    =  */ buf->readNext32bit();  // BROKEN!!! FIXME!!! 
            v.constellation_no  = buf->readNext16bit();
            v.latency           = buf->readNextFloat();
            m->decodedGpsPositionVelocity(v, buf->getSource());
         }
         delete(buf);
         return NULL;
      case MsgBufferEnums::GPS_STATUS:
         {
            isabTime t;                       /* FIXME!!! - really read data here */
               buf->readNext32bit();
            enum GpsStatus sum_status = 
               (enum GpsStatus)(buf->readNext16bit());
            uint16 error_conditions   = buf->readNext16bit();
            float pdop                = buf->readNextFloat();
            m->decodedGpsStatus(t, sum_status, error_conditions, pdop, buf->getSource());
         }
         delete(buf);
         return NULL;
      case MsgBufferEnums::GPS_TIME:
         delete(buf);
         return NULL;
      default:
         return buf;
   }

}

MsgBuffer * GpsProviderDecoder::dispatch(MsgBuffer *buf, GpsProviderInterface *m )
{
   int length;

//   cout << "GpsProviderDecoder::dispatch : " << buf->getMsgType() << endl;
   length=buf->getLength();
   switch (buf->getMsgType()) {
      case MsgBufferEnums::GPS_RESET_RECEIVER:
         m->decodedResetReceiver(buf->getSource());
         delete(buf);
         return NULL;
      default:
         return buf;
   }

}


} /* namespace isab */
