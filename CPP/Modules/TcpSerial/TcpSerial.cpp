/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TcpSerial.h"
#include <ctype.h>
#include "TcpThread.h"
#include "nav2util.h"
#include "DNSCache.h"

#include "LogMacros.h"


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

#define SLEEP_AT_CONNECT 1000

namespace isab {
   TcpSerial::TcpSerial(const char* name) :
      Module(name ? name : "TcpSerial"), m_tcpThread(NULL), 
      m_threadNum(0), m_state(CLEAR), m_keepConnectionTimeout(0),
#ifdef __WINS__
      m_connectionParam(-2),
#else
      m_connectionParam(-1),
#endif
      m_lastSent(ConnectionNotify(-1))
   {
      DBG("TcpSerial waiting for orders");
      m_paramProvider = new ParameterProviderPublic(this);
      m_connectionTimer = m_queue->defineTimer();
      m_dnsCache = new DNSCache();
   }


   TcpSerial::TcpSerial(const char *hostname, int port, const char* name) :
      Module(name ? name : "TcpSerial"), m_tcpThread(NULL), m_threadNum(0),
      m_state(CLEAR), 
      m_keepConnectionTimeout(0), 
#ifdef __WINS__
      m_connectionParam(-2),
#else
      m_connectionParam(-1),
#endif
      m_lastSent(ConnectionNotify(-1))
   {
      DBG("TcpSerial(\"%s\", %i)\n", hostname, port);
      m_paramProvider = new ParameterProviderPublic(this);
      m_connectionTimer = m_queue->defineTimer();
      m_dnsCache = new DNSCache();

      newThread(port, hostname, 1);
      DBG("TcpSerial end");
   }
   
   TcpSerial::TcpSerial(int port, const char* name) :
      Module(name ? name : "TcpSerial"), m_tcpThread(NULL), m_threadNum(0),
      m_state(CLEAR), m_keepConnectionTimeout(0), 
#ifdef __WINS__
      m_connectionParam(-2),
#else
      m_connectionParam(-1),
#endif
      m_lastSent(ConnectionNotify(-1))
   {
      DBG("TcpSerial(%i)\n", port);
      m_paramProvider = new ParameterProviderPublic(this);
      m_connectionTimer = m_queue->defineTimer();
      m_dnsCache = new DNSCache();

      newThread(port);
   }

   TcpSerial::~TcpSerial()
   {
      delete m_paramProvider;
      delete m_dnsCache;
   }

   void TcpSerial::decodedStartupComplete()
   {
      Module::decodedStartupComplete();
      // Parameters we need and will subscribe to.
      ParameterEnums::ParamIds params[] = {ParameterEnums::SelectedAccessPointId2};
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
      if(m_tcpThread){
         DBG("TcpSerial starting TcpThread");
         if (m_tcpThread->start() == Thread::GOOD_START) {
            DBG("TcpThread start ok.");
            THREADINFO(*m_tcpThread);
         } else {
            DBG("TcpThread start NOT ok");
         }
      }
   }

   void TcpSerial::decodedShutdownPrepare( int16 upperTimeout )
   {
      delete m_tcpThread;
      m_tcpThread = NULL;
      Module::decodedShutdownPrepare( upperTimeout );
   }

   void TcpSerial::treeIsShutdown()
   {
      delete m_tcpThread;
      m_tcpThread = NULL;
      Module::treeIsShutdown();
   }

   SerialProviderPublic * TcpSerial::newPublicSerial()
   {
      DBG("newPublicSerial()\n");
      SerialProviderPublic* spp = new SerialProviderPublic(m_queue);
      return spp;
   }
   
   MsgBuffer * TcpSerial::dispatch(MsgBuffer *buf)
   {
      //DBG("TcpSerial::dispatch : %i\n",  buf->getMsgType());
      buf = m_providerDecoder.dispatch(buf, this);
      if(buf) buf = m_paramDecoder.dispatch(buf, this);
      if(buf) buf = Module::dispatch(buf);
      return buf;
   }

   void TcpSerial::decodedExpiredTimer(uint16 timerID)
   {
      DBG("decodedExpiredTimer(%u)", timerID);
#ifdef SYMBIAN_DELAY_DISCONNECT
      if(timerID == m_connectionTimer){
         if(m_tcpThread != NULL){
            closeThread();
            DBG("deleted tcpThread.");
         }
         return;
      }
#endif

      Module::decodedExpiredTimer(timerID);
   }


   void TcpSerial::closeThread()
   {
      delete m_tcpThread;
      m_tcpThread = NULL;
   }

   void TcpSerial::newThread(int port, const char* hostname, int reconnect)
   {
      char threadName[128] = {0};
      snprintf(threadName, sizeof(threadName) - 1, 
               "%d%s_TcpThread", m_threadNum++, getName());
      DBG("New TcpThread named '%s'", threadName);
# ifdef SLEEP_AT_CONNECT
      sleep(SLEEP_AT_CONNECT, 0);
# endif
      m_tcpThread = new TcpThread(threadName, m_connectionParam, this, port, 
                                  hostname, reconnect);
   }
   
   Thread::startStatus TcpSerial::startNewThread(int port, const char* hostname, 
                                                 int reconnect)
   {
      newThread(port, hostname, reconnect);
      enum startStatus status = startStatus(m_tcpThread->start());
      if(status == Thread::GOOD_START){
         THREADINFO(*m_tcpThread);
      }
      return status;
   }

   void TcpSerial::decodedConnectionCtrl(enum ConnectionCtrl ctrl, 
                                         const char *method, uint32 src)
   {
      if (!rootPublic() || m_shutdownPrepared) {
         return;
      }
      //sleep(1000,0);           // FIXME - temporary hack for strange hang in TcpThread (PageFault) /petersv 20030929
      switch(ctrl){
      case CONNECT:
         {
            DBG("CONNECT %s", method);
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
               if(m_tcpThread && !m_tcpThread->isAlive()){
                  closeThread();
               }
               if(!m_tcpThread){ //not connected before.
                  DBG("Not alredy connected");
                  startStatus start = startNewThread(port, host);
                  if(GOOD_START != start){
                     ERR("TcpThread didn't start. Errorcode %d", start);
                     closeThread();
                     ERR("TcpThread deleted");
                     sendConnectionNotify(DISCONNECTING, INTERNAL_ERROR,src);
                     DBG("Notified Consumer %p", rootPublic());
                  } else {
                     DBG("TcpThread started.");
                  }
                  DBG("Started or not, here i GO....");
               } else if(host != NULL){
                  DBG("already connected, new connection initiated");
                  const char* hostname = m_tcpThread->getHostname();
                  if(hostname == NULL || strcmp(host, hostname) != 0 || 
                     port != m_tcpThread->getPort()){
                     // disconnect current and connect to a new server.
                     DBG("disconnect %s:%d and connect to %s",
                         m_tcpThread->getHostname(), m_tcpThread->getPort(),
                         method);
                     closeThread();
                     startStatus start = startNewThread(port, host);
                     if(GOOD_START != start){
                        ERR("TcpThread didn't start. Errorcode %d", start);
                        closeThread();
                        sendConnectionNotify(DISCONNECTING, INTERNAL_ERROR, 
                                             src);
                     } else {
                        DBG("New tcpthread sterted.");
                     }
                  } else {
                     DBG("Already connected to host. Reuse and notify.");
                     sendConnectionNotify(CONNECTING, UNSPECIFIED, src);
                     if ( m_tcpThread->socketConnected() ) {
                        sendConnectionNotify( CONNECTED, UNSPECIFIED, 
                                              src );
                     }
                  }
               } else if(port != m_tcpThread->getPort()){
                  DBG("Alredy connected, listening connection initiated");
                  closeThread();
                  startNewThread(port);
               }
            }
            delete[] tmp;
         }
         break;
      case DISCONNECT:
        DBG("DISCONNECT %s", method);
        m_keepConnectionTimeout = 0;
        if(method && isdigit(*method)){
           uint32 timeoutTime = 0;
           char* endp = NULL;
           timeoutTime = strtoul(method, &endp, 10);
           if(endp != method){
              m_keepConnectionTimeout = timeoutTime;
           }
        } else if(method && strequ(method, "FAKETIMEOUT")){
           if(m_tcpThread){
              DBG("disconnectreason set to NoResponse");
              m_tcpThread->setDisconnectReason(TcpThread::NoResponse);
           }
        }
        DBG("KeepConnectionTimeout parsed to %"PRId32,m_keepConnectionTimeout);
        if(m_tcpThread){
           DBG("method: %p, delay: %s, *method: %s", method, 
               delayDisconnect() ? "true" : "false", method);
           if(method && delayDisconnect() && 
              !strequ(method, "FAKETIMEOUT")){
              //#ifdef SYMBIAN_DELAY_DISCONNECT
              DBG("Fake disconnect!");
              sendConnectionNotify(DISCONNECTING, REQUESTED, src);
              sendConnectionNotify(CLEAR, UNSPECIFIED, src);
              m_queue->setTimer(m_connectionTimer, SYMBIAN_DELAY_AMOUNT);
           } else {
              DBG("Real disconnect, deleting TcpThread");
              //#else
              closeThread();
              DBG("deleted tcpThread.");
           }
           //#endif
        } else {
           DBG("m_state = CLEAR");
           m_state = CLEAR;
        }
        DBG("DISCONNECTION COMPLETE");
        break;
      case QUERY:
         DBG("QUERY %s", method);
         {
            enum ConnectionNotifyReason reason = UNSPECIFIED;
            if(m_tcpThread){
               reason = m_tcpThread->getDisconnectReason();
            }
            //no lastsent check here. This is after all an explicit request.
            rootPublic()->connectionNotify(m_state, reason, src);
         }
         break;
      default:
         ERR("ConnectionCtrl: Unknown ctrl value: %d", ctrl);
      }
      DBG("end of decodedConnectionCtrl");
   }

#define CNSTRING(ctrl)                                                    \
      ((ctrl == CONNECTING) ? "CONNECTING" :                              \
       ((ctrl == CONNECTED) ? "CONNECTED" :                               \
        ((ctrl == DISCONNECTING) ? "DISCONNECTING" :                      \
         ((ctrl == CLEAR) ? "CLEAR" :                                     \
          ((ctrl == WAITING_FOR_USER ? "WAITING_FOR_USER" : "?????"))))))

   void TcpSerial::connectionNotify(enum ConnectionNotify ctrl)
   {

      DBG("connectionNotify: %s", CNSTRING(ctrl));
      m_state = ctrl;
      ConnectionNotifyReason reason = UNSPECIFIED;
      if(m_tcpThread){
         reason = m_tcpThread->getDisconnectReason();
      }
      DBG("Sending connection notify upwards");
      sendConnectionNotify(m_state, reason, 
                           uint32(MsgBufferEnums::ADDR_LINK_LOCAL));
   }

   void TcpSerial::decodedSendData(int length, const uint8 *data, uint32 /*src*/)
   {
      DBG("TcpSerial::decodedSendData(%i, %p)\n", length, data);
      if(m_tcpThread){
         int ret = 0;
         if(0 > (ret = m_tcpThread->write(data, ssize_t(length)))){
            ERR("No data sent. %d", ret);
         }
      } else {
         WARN("Cannot send data while disconnected!");
      }
   }

   SerialConsumerPublic * TcpSerial::rootPublic()
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
   void TcpSerial::Kill(int reason, Thread* originator)
   {
      if(m_tcpThread){
         m_tcpThread->Kill(reason, originator);
      }
      Module::Kill(reason, originator);
   }
#endif

   void TcpSerial::decodedParamNoValue(uint32 paramId, uint32 /*src*/, uint32 /*dst*/)
   {
      switch(paramId){
      case ParameterEnums::SelectedAccessPointId2:
#ifdef __WINS__
         m_connectionParam = -2;
#else
         m_connectionParam = -1;
#endif
         if(m_tcpThread){
            m_tcpThread->setConnectionParam(m_connectionParam);
         }
         break;
      default:
         break;
      }
   }

   void TcpSerial::decodedParamValue(uint32 paramId, const int32 * data,
                                     int32 /*numEntries*/, uint32 /*src*/, 
                                     uint32 /*dst*/)
   {
      switch(paramId){
      case ParameterEnums::SelectedAccessPointId2:
         m_connectionParam = *data;
         if(m_tcpThread){
            m_tcpThread->setConnectionParam(m_connectionParam);
         }
         break;
      default:
         break;
      }
   }


   void TcpSerial::decodedParamValue( uint32 param, const char*const* data,
                                      int32 /*numEntries*/, uint32 /*src*/, 
                                      uint32 /*dst*/)
   {
      switch ( param ) {
         case ParameterEnums::SelectedAccessPointId2: {
            char* tmpStr = NULL;
            int32 p = strtol( data[ 0 ], &tmpStr, 10 );
            if ( tmpStr != data[ 0 ] ) {
               m_connectionParam = p;
               if ( m_tcpThread != NULL ) {
                  m_tcpThread->setConnectionParam( m_connectionParam );
               }
            }
         }  break;
            
         default:
            break;
      }
   }


   void TcpSerial::sendConnectionNotify(enum ConnectionNotify msg, 
                                        enum ConnectionNotifyReason rsn, 
                                        uint32 dst)
   {
      if(rootPublic() && m_lastSent != msg && 
           !(m_lastSent == CLEAR && msg == DISCONNECTING)){
         DBG("Really sending %s upward", CNSTRING(msg));
         rootPublic()->connectionNotify(msg, rsn, dst);
         DBG("connext notify sent");
         m_lastSent = msg;
         DBG("m_lastSent set");
      }
      DBG("Leaving sendConnectionNotify");
   }

   class DNSCache& TcpSerial::getDNSCache()
   {
      return *m_dnsCache;
   }


} /* namespace isab */

