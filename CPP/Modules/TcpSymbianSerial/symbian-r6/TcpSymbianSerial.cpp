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
#include "ParameterEnums.h"
#include "Parameter.h"
#include "TcpSymbianSerial.h"
#include <ctype.h>

#include "LogMacros.h"
#include "nav2util.h"

#include "ThreadOwner.h"
#include "TcpAdmin.h"


#define SYMBIAN_DELAY_DISCONNECT
#undef SYMBIAN_DELAY_DISCONNECT
#define SYMBIAN_DELAY_AMOUNT (1000*10)
namespace {
   inline bool delayDisconnect()
   {
#ifdef SYMBIAN_DELAY_DISCONNECT
      return true;
#else
      return false;
#endif
   }

}

#ifdef __WINS__
# define IFWINS(a,b) (a)
#else
# define IFWINS(a,b) (b)
#endif

_LIT( KTcpSymbianThreadName, "TcpSymbianThread");

namespace isab {

   TcpSymbianSerial::TcpSymbianSerial(const char* name, 
                                      int considerWLAN) :
      Module(name ? name : "TcpSymbianSerial"),         
      m_threadHolder(NULL),                             
      m_state(CLEAR),                                   
      m_lastSent(ConnectionNotify(-1)),                 
      m_keepConnectionTimeout(0),                       
      m_connectionParam(IFWINS(-2,-1)),                 
      m_resetThread(false),
      m_tcpThreadPanicked(false),
      m_considerWLAN(considerWLAN)
   {
      DBG("TcpSymbianSerial waiting for orders");
      m_paramProvider = new ParameterProviderPublic(this);
      m_connectionTimer = m_queue->defineTimer();
   }

   TcpSymbianSerial::~TcpSymbianSerial()
   {
      delete m_paramProvider;
   }

   void TcpSymbianSerial::decodedStartupComplete()
   {
      Module::decodedStartupComplete();
      // Parameters we need and will subscribe to.
      enum ParameterEnums::ParamIds params[] = 
         { ParameterEnums::SelectedAccessPointId2 };
      //sort the list to facilitate joining
      std::sort(params, params + sizeof(params)/sizeof(*params));
      //for loop to get parameters and join parameter multicast groups
      uint16 last = 0;
      for(unsigned j = 0; j < sizeof(params)/sizeof(*params); ++j){
         uint16 group = ParameterProviderPublic::paramIdToMulticastGroup(params[j]);
         m_paramProvider->getParam(params[j]);
         if(group != last){
            m_rawRootPublic->manageMulticast(JOIN, group);
         }
         last = group;
      }
      //subscribe to navtask messages.
      m_rawRootPublic->manageMulticast(JOIN, Module::NavTaskMessages);
      DBG("Start thread.");
      resetThread();
   }

   void TcpSymbianSerial::decodedShutdownPrepare(int16 upperTimeout)
   {
      DBG("decodedShutdownPrepare, close thread.");
      resetThread(false);
      Module::decodedShutdownPrepare(upperTimeout);
   }

   void TcpSymbianSerial::resetThread(bool restart)
   {
      DBG("resetThread(%d)", int(restart));
      delete m_threadHolder;
      m_threadHolder = NULL;
      m_tcpAdmin = NULL;
      if(restart){
         class CTcpAdminFactory* factory = 
            CTcpAdminFactory::New(this, KTcpSymbianThreadName, 
                                  m_log->getLogMaster());
         if(factory){         
            DBG("New factory, trying to start thread.");
            m_threadHolder = ::CThreadOwner::New(factory);
            DBG("CThreadOwner::New done");
            m_tcpAdmin = factory->CreatedObject();
            m_tcpAdmin->ConsiderWLAN();
            delete factory;
         } 
         if(!m_threadHolder){
            WARN("No new thread created.");
            tcpThreadPanic(KErrNoMemory);
            //startup has failed miserably.
         }
      }
   }

   class SerialProviderPublic* TcpSymbianSerial::newPublicSerial()
   {
      DBG("newPublicSerial()\n");
      class SerialProviderPublic* spp = new SerialProviderPublic(m_queue);
      return spp;
   }
   
   class MsgBuffer* TcpSymbianSerial::dispatch(class MsgBuffer *buf)
   {
      //DBG("TcpSymbianSerial::dispatch : %i\n",  buf->getMsgType());
      if(!m_shutdownPrepared){
         if(m_resetThread){
            DBG("RESETTHREAD");
            m_resetThread = false;
            resetThread();
         }
         buf = m_providerDecoder.dispatch(buf, this);
         if(buf) buf = m_paramDecoder.dispatch(buf, this);
      }
      if(buf) buf = Module::dispatch(buf);
      return buf;
   }

   void TcpSymbianSerial::decodedExpiredTimer(uint16 timerID)
   {
      DBG("decodedExpiredTimer(%u)", timerID);
#ifdef SYMBIAN_DELAY_DISCONNECT
      if(timerID == m_connectionTimer){
         DBG("ConnectionTimer. Time for FAKETIMEOUT");
         decodedConnectionCtrl(DISCONNECT, "FAKETIMEOUT", 0);
         return;
      }
#endif

      Module::decodedExpiredTimer(timerID);
   }

   void TcpSymbianSerial::decodedConnectionCtrl(enum ConnectionCtrl ctrl, 
                                                const char *method, uint32 src)
   {
      if (!rootPublic() || m_shutdownPrepared || m_tcpThreadPanicked) {
         WARN("Won't handle connectioncontrol.");
         return;
      }
      switch(ctrl){
      case CONNECT:
         DBG("CONNECT %s", method);
         handleConnect(method, src);
         break;
      case DISCONNECT:
         DBG("DISCONNECT %s", method);
         handleDisconnect(method, src);
         break;
      case QUERY:
         DBG("QUERY %s", method);
         {
            enum ConnectionNotifyReason reason = UNSPECIFIED;
            //no lastsent check here. This is after all an explicit request.
            rootPublic()->connectionNotify(m_state, reason, src);
         }
         break;
      default:
         ERR("ConnectionCtrl: Unknown ctrl value: %d", ctrl);
      }
      DBG("end of decodedConnectionCtrl");
   }


   void TcpSymbianSerial::handleConnect(const char* method, uint32 src)
   {
      char* tmp = strdup_new(method);
      char* host = NULL;
      uint16 port = 0;
#ifdef SYMBIAN_DELAY_DISCONNECT
      if(m_queue->isTimerSet(m_connectionTimer)){
         DBG("stopping timer %u", m_connectionTimer);
         m_queue->stopTimer(m_connectionTimer);
      }
#endif
      if(!parseHost(tmp, &host, &port)){
         WARN("parsing of %s failed.", method);
         sendConnectionNotify(DISCONNECTING, METHOD_ERROR, src);
         // this look suspicious
         //rootPublic()->connectionNotify(CLEAR, UNSPECIFIED, src);
      } else {
         DBG("Host parsed as %s:%u", host, port);
         if(m_tcpAdmin){
            if(! m_tcpAdmin->IsConnected()){
               DBG("Not already connected.");
               m_tcpAdmin->ConnectionControl(CONNECT, method);
            } else {
               DBG("Probably fake-disconnected.");
               //just a faked disconnect before. 
               sendConnectionNotify(CONNECTING, REQUESTED, src);
               sendConnectionNotify(CONNECTED, REQUESTED, src);
            }
         } else {
            DBG("No TcpAdmin, DISCONNECT INTERNAL_ERROR");
            sendConnectionNotify(DISCONNECTING, INTERNAL_ERROR, src);
         }
      }
      delete[] tmp;
   }

   void TcpSymbianSerial::handleDisconnect(const char* method, uint32 src)
   {
      m_keepConnectionTimeout = 0;
      if(method && isdigit(*method)){
         uint32 timeoutTime = 0;
         char* endp = NULL;
         timeoutTime = strtoul(method, &endp, 10);
         if(endp != method){
            m_keepConnectionTimeout = timeoutTime;
         }
      }
      DBG("KeepConnectionTimeout parsed to %"PRIu32, m_keepConnectionTimeout);
      if(method && delayDisconnect() && !strequ(method, "FAKETIMEOUT")){
         DBG("Fake Disconnect!");
         sendConnectionNotify(DISCONNECTING, REQUESTED, src);
         sendConnectionNotify(CLEAR, UNSPECIFIED, src);
         m_queue->setTimer(m_connectionTimer, SYMBIAN_DELAY_AMOUNT);
      } else {
         DBG("Real disconnect");
         if(m_tcpAdmin){
            m_tcpAdmin->ConnectionControl(DISCONNECT, method);
         }
      }
      DBG("DISCONNECTION COMPLETE");
   }

#define CNSTRING(ctrl)                                                    \
      ((ctrl == CONNECTING) ? "CONNECTING" :                              \
       ((ctrl == CONNECTED) ? "CONNECTED" :                               \
        ((ctrl == DISCONNECTING) ? "DISCONNECTING" :                      \
         ((ctrl == CLEAR) ? "CLEAR" :                                     \
          ((ctrl == WAITING_FOR_USER ? "WAITING_FOR_USER" : "?????"))))))

   void TcpSymbianSerial::connectionNotify(enum ConnectionNotify ctrl,
                                           enum ConnectionNotifyReason reason)
   {
      if(reason == INTERNAL_ERROR){
         m_resetThread = true;
      }
      DBG("connectionNotify: %s", CNSTRING(ctrl));
      m_state = ctrl;
      DBG("Sending connection notify upwards");
      sendConnectionNotify(m_state, reason, 
                           uint32(MsgBufferEnums::ADDR_LINK_LOCAL));
   }

   void TcpSymbianSerial::decodedSendData(int length, const uint8 *data, 
                                          uint32 /*src*/)
   {
      DBG("TcpSymbianSerial::decodedSendData(%i, %p)\n", length, data);
      if(m_tcpAdmin && !m_tcpThreadPanicked){
         m_tcpAdmin->SendData(data,length);
      } else {
         WARN("Cannot send data while disconnected!");
      }
   }



   class SerialConsumerPublic * TcpSymbianSerial::rootPublic()
   {
      //   // FIXME - use dynamic_cast here? Check null result then.
      //   // FIXME - inline this!
      //   return dynamic_cast<SerialConsumerPublic *>m_rawRootPublic;

      // This is actually safe since the connect message is only sent 
      // by a directly attached public class to the module. Since we
      // created the public interface (of type SerialProviderPublic)
      // we know that that is the class that created the connect message.
      // SerialProviderPublic in turn only accepts a 
      // SerialConsumerPublic as parameter. 
      return static_cast<SerialConsumerPublic *>(m_rawRootPublic);
   }


#ifdef __SYMBIAN32__
   void TcpSymbianSerial::Kill(int reason, class Thread* originator)
   {
      delete m_threadHolder;
      m_threadHolder = NULL;
      m_tcpAdmin = NULL;
      Module::Kill(reason, originator);
   }
#endif

   void TcpSymbianSerial::decodedParamNoValue(uint32 paramId, uint32 src, 
                                              uint32 dst)
   {
      switch(paramId){
      case ParameterEnums::SelectedAccessPointId2:
         {
            const char* tmp = "-1";
            decodedParamValue(paramId, &tmp, 1, src, dst);
         } 
         break;
      default:
         break;
      }
   }

   void TcpSymbianSerial::decodedParamValue(uint32 paramId,
                                            const int32* data,
                                            int32 /*numEntries*/,
                                            uint32 /*src*/,
                                            uint32 /*dst*/)
   {
      switch(paramId){
      case ParameterEnums::SelectedAccessPointId2:
         {
            m_connectionParam = data[0];
            DBG("Connectionparam set to %"PRId32, m_connectionParam);
            if ( m_tcpAdmin != NULL ) {
#ifndef __WINS__
               m_tcpAdmin->SetConnectionParam( m_connectionParam );
#else
               m_tcpAdmin->SetConnectionParam( -2 );                  
#endif
               DBG("Connection param set in TcpAdmin.");
            }
         }
         break;
      }
   }
   void TcpSymbianSerial::decodedParamValue(uint32 paramId,
                                            const char * const * data,
                                            int32 /*numEntries*/,
                                            uint32 /*src*/,
                                            uint32 /*dst*/)
   {
      switch(paramId){
      case ParameterEnums::SelectedAccessPointId2:
         {
            DBG("SelectedAccessPointId2 is '%s'", data[0]);
            char* tmpStr = NULL;
            int32 p = strtol( data[ 0 ], &tmpStr, 10 );
            if ( tmpStr != data[ 0 ] ) {
               m_connectionParam = p;
               DBG("Connectionparam set to %"PRId32, m_connectionParam);
               if ( m_tcpAdmin != NULL ) {
#ifndef __WINS__
                  m_tcpAdmin->SetConnectionParam( m_connectionParam );
#else
                  m_tcpAdmin->SetConnectionParam( -2 );                  
#endif
                  DBG("Connection param set in TcpAdmin.");
               }
            }
         }
         break;
      }
   }

   void TcpSymbianSerial::sendConnectionNotify(enum ConnectionNotify msg, 
                                        enum ConnectionNotifyReason rsn, 
                                        uint32 src)
   {
      if(rootPublic() && m_lastSent != msg && 
         !(m_lastSent == CLEAR && msg == DISCONNECTING)){
         DBG("Really sending %s upward", CNSTRING(msg));
         rootPublic()->connectionNotify(msg, rsn, src);
         DBG("connext notify sent");
         m_lastSent = msg;
         DBG("m_lastSent set");
      }
      DBG("Leaving sendConnectionNotify");
   }

   bool TcpSymbianSerial::receive(const uint8* data, int length)
   {
      DBG("receive(%p, %d)", data, length);
      bool ret = false;
      if(isAlive()){
         if(rootPublic()){
            uint32 src = rootPublic()->receiveData(length, data);
            src = src;
            DBG("Message sent with id: %0#10"PRIx32, src);
            ret = true;
         }
      }
      return ret;
   }

   void TcpSymbianSerial::tcpThreadPanic(int reason)
   {
      WARN("tcpThreadPanic(%d)", reason);
      m_tcpThreadPanicked = true;

      unsolicitedError(Nav2Error::PANIC_ABORT);
   }

} /* namespace isab */
