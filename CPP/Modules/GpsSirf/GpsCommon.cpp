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

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "Gps.h"
#include "GpsCommon.h"

#include "GpsParser.h"

#include "Buffer.h"
#include "MsgBuffer.h"

#include "LogMacros.h"

#undef DBG
#define DBG DROP


namespace isab {

   GpsCommon::GpsCommon(const char *name, SerialProviderPublic *p, 
                        int expectedDataInterval)
      : Module(name), m_provider(p), m_receiverMissing(true), 
        m_parser(NULL), m_expectedDataInterval(expectedDataInterval)
   {
      SerialConsumerPublic *consumer;

      m_slowGpsTimerId = m_queue->defineTimer();

      DBG("GpsCommon::GpsCommon\n");
      consumer=new SerialConsumerPublic(m_queue);
      p->setOwnerModule(this);
      p->connect(consumer);
      newLowerModule(p);
   }

   GpsCommon::~GpsCommon()
   {
      delete m_parser;
   }

   void GpsCommon::treeIsShutdownPrepared()
   {
      m_parser->setConsumer(NULL);
      Module::treeIsShutdownPrepared();
   }

   void GpsCommon::decodedConnectionCtrl(enum ConnectionCtrl command, 
                                       const char *method, uint32 /*src*/)
   {
      DBG("Calls connection control of m_provider");
      m_provider->connectionCtrl(command, method);
   }

   void GpsCommon::decodedStartupComplete()
   {
      Module::decodedStartupComplete();
      m_parser->connection_ready();
   }

   MsgBuffer * GpsCommon::dispatch(MsgBuffer *buf)
   {
      DBG("GpsCommon::dispatch : %i\n", buf->getMsgType());
      buf=m_consumerDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=m_providerDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=Module::dispatch(buf);
      return buf;
   }

   void GpsCommon::decodedExpiredTimer(uint16 timerid)
   {
      if (timerid == m_slowGpsTimerId) {
         DBG("Quality set to missing due to timeout");
         if (m_rawRootPublic) {
            m_receiverMissing = true;
            sendFakedGpsStatus();
         }
         return;
      }
      Module::decodedExpiredTimer(timerid);
   }

   void GpsCommon::decodedReceiveData(int length, const uint8 *data, uint32 /*src*/)
   {
      DBG("GpsCommon::decodedReceiveData(%i, xxxxx)\n", length);
      if (m_parser->receiveData(data, length) > 0) {
         // Got at least one valid packet, reset the timer
         m_queue->setTimer(m_slowGpsTimerId, m_expectedDataInterval * 3);
         DBG("Timer set");
         if (m_receiverMissing) {
            m_parser->sendStatus();
            m_receiverMissing = false;
         }
      }
      DBG("end of receiveData");
   }

   void GpsCommon::decodedResetReceiver(uint32 /*src*/)
   {
   }

   GpsProviderPublic * GpsCommon::newPublicGps()
   {  
      DBG("newPublicGps()\n");
      return new GpsProviderPublic(m_queue);
   }

   void GpsCommon::sendFakedGpsStatus()
   {
      isabTime now;

      DBG("Notify on missing receiver");
      if (rootPublic()) {
         rootPublic()->gpsStatus(now, os_gps_fatal, OS_GPS_ERROR_MISSING, 0);
      }
   }
   
   int GpsCommon::highlevelConnectTo()
   {
      m_parser->setConsumer(rootPublic());
      if (m_receiverMissing) {
         sendFakedGpsStatus();
      } else {
         m_parser->sendStatus();
      }
      return 0;
   }


} /* namespace isab */

