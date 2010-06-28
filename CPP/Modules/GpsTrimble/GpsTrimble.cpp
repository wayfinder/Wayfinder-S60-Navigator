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
#include "Log.h"
#include "GpsTrimble.h"

#include "Buffer.h"
#include "MsgBuffer.h"

#include "LogMacros.h"
#undef DBG
#define DBG DROP

namespace isab {

   GpsTrimble::GpsTrimble(SerialProviderPublic *p)
      : Module("GpsTrimblethread"), m_provider(p)
   {
      SerialConsumerPublic *consumer;

      DBG("GpsTrimble::GpsTrimble\n");
      m_slowGpsTimerId = m_queue->defineTimer();
      m_trimbleParser = new TrimbleParser(p, m_log);
      consumer=new SerialConsumerPublic(m_queue);
      p->setOwnerModule(this);
      p->connect(consumer);
      newLowerModule(p);
   }

   GpsTrimble::~GpsTrimble()
   {
      delete m_trimbleParser;
   }

   void GpsTrimble::decodedConnectionCtrl(enum ConnectionCtrl command, 
                                       const char *method, uint32 src)
   {
      m_provider->connectionCtrl(command, method, src);
   }

   void GpsTrimble::decodedStartupComplete()
   {
      Module::decodedStartupComplete();
      m_trimbleParser->connection_ready();
   }

   MsgBuffer * GpsTrimble::dispatch(MsgBuffer *buf)
   {
      DBG("GpsTrimble::dispatch : %i\n", buf->getMsgType());
      buf=m_consumerDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=m_providerDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=Module::dispatch(buf);
      return buf;
   }

   void GpsTrimble::decodedExpiredTimer(uint16 timerid)
   {
      if (timerid == m_slowGpsTimerId) {
         // Timer expired -> the gps was too slow to respond. Label the receiver
         // as ????
         return;
      }
      Module::decodedExpiredTimer(timerid);
   }

   void GpsTrimble::decodedReceiveData(int length, const uint8 *data, uint32 /*src*/)
   {
      DBG("GpsTrimble::decodedReceiveData(%i, xxxxx)\n", length);
      m_trimbleParser->receiveData(data, length);
   }

   void GpsTrimble::decodedResetReceiver(uint32 /*src*/)
   {
   }

   GpsProviderPublic * GpsTrimble::newPublicGps()
   {  
      DBG("newPublicGps()\n");
      return new GpsProviderPublic(m_queue);
   }

   GpsConsumerPublic * GpsTrimble::rootPublic()
   {  
      return reinterpret_cast<GpsConsumerPublic *>(m_rawRootPublic);
   }

   int GpsTrimble::highlevelConnectTo()
   {
      m_trimbleParser->setConsumer(rootPublic());
      return 0;
   }


} /* namespace isab */

