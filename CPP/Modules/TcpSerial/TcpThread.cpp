/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TcpThread.h"
#include "TcpSerial.h"
#include "Selector.h"
#include "nav2util.h"
#include "DNSCache.h"
// set the pointer to log object to use in LogMacros.h
//Yes, this is a very weird macro, but ensures that we get a return
//value from the log macros.
#define LOGPOINTER (!m_owner ? 0: m_owner->m_log)
#include "LogMacros.h" 

//The LOCK and UNLOCK macros are used to debug mutex operation. The
//macros will always perform the lock or unlock operation, but when DEBUG_LOCKS
//is defined and NO_LOG_OUTPUT is not defined every LOCK
//or UNLOCK operation will also print to the log.

//LOCK/UNLOCK macros always return the lock/unlock return values.

//The LOGLOCK and LOGUNLOCK are used to lock the mutex before printing
//to the log object. If NO_LOG_OUTPUT is defined they are no-ops.
#if defined DEBUG_LOCKS && !defined NO_LOG_OUTPUT
# define LOCK(mutex)   (DBG(#mutex ".lock()"),   int res = mutex.lock(),   DBG(#mutex ".---taken"), res)
# define UNLOCK(mutex) (DBG(#mutex ".unlock()"), int res = mutex.unlock(), DBG(#mutex ".---released"), res)
# define LOGLOCK(mutex) LOCK(mutex)
# define LOGUNLOCK(mutex) UNLOCK(mutex)
#else
# define LOCK(mutex)   mutex.lock()
# define UNLOCK(mutex) mutex.unlock()
# define LOGLOCK(mutex) mutex.lock()
# define LOGUNLOCK(mutex) mutex.unlock()
#endif

#ifndef NO_LOG_OUTPUT
# undef LOGLOCK
# undef LOGUNLOCK
# define LOGLOCK(mutex) if(0){LOCK(mutex);}
# define LOGUNLOCK(mutex) if(0){UNLOCK(mutex);}
#endif

namespace isab {

   TcpThread::TcpThread(const char* name, int32 connectionParameter,
                        TcpSerial* owner, int port, const char* hostname, 
                        int reconnect) : 
      Thread(name), m_reconnect(reconnect), m_stop(false), m_owner(owner), 
      m_hostname(NULL), m_port(port), m_socket(NULL), m_listener(NULL), 
      m_mutex(), m_selector( NULL ), m_connParam(connectionParameter), 
      m_reason(OnOrder)
   {
      DBG("TcpThread() threadname = \"%s\"", name);
      if(hostname){
         m_hostname = strdup_new(hostname);
      }
      DBG("TcpThread: Reason set to OnOrder");
   }

   TcpThread::~TcpThread()
   {
      DBG("~TcpThread");
      terminate();
      DBG("~TcpThread: terminate has been called.");
      while(isAlive()){
         DBG("~TcpThread: Still alive. Joining...");
         int joinres = join(500);
         joinres = joinres;
         DBG("~TcpThread: Join result: %d", joinres);
      }

      LOCK(m_mutex);
      delete[] m_hostname;
#ifndef __SYMBIAN32__
      delete m_selector;
#endif
      UNLOCK(m_mutex); 
      DBG("~TcpThread: At the end of my rope. Bye bye!");
   }

   TCPSocket::TCPSocketState TcpThread::getState()
   {
      TCPSocket::TCPSocketState ret = TCPSocket::UNKNOWN;
      
      LOCK(m_mutex); 
      if(m_socket){
         ret = m_socket->getState();
      }      
      UNLOCK(m_mutex); 
      return ret;
   }

   ssize_t TcpThread::write(const uint8 *data, ssize_t length)
   {
      ssize_t retval = 0;
      LOCK(m_mutex); 
      if(m_socket != NULL && m_socket->getState() == TCPSocket::CONNECTED){
         retval = m_socket->write(data, length);
      }
      UNLOCK(m_mutex); 
      return retval;
   }

   void TcpThread::terminate()
   {
      DBG("TcpThread: This is terminate");
      if(isAlive()){
         LOCK(m_mutex); 
         if(m_socket){
            m_socket->cancelAll();
         }
         m_reconnect = false;
         m_stop = true;
         if ( m_selector != NULL ) {
            m_selector->terminate();
         }
         DBG("No reconnect, stop set, selector terminated");
         UNLOCK(m_mutex); 
         if(m_reason != NoResponse){
            m_reason = OnOrder;
            DBG("TcpThread: Reason set to OnOrder, it wasn't NoResponse");
         }
      } else {
         DBG("Bah! Thread wasn't even alive!");
         LOCK(m_mutex);
         m_stop = true;
         UNLOCK(m_mutex);
      }
   }

   // five seconds in microseconds.
#define ACCEPT_TIMEOUT (5 * 1000 * 1000)
   // five seconds in milliseconds
#define CONNECT_WAIT (5 * 1000)

   bool TcpThread::socketConnected(){
      LOCK(m_mutex);
      bool ret = m_socket && m_socket->getState() == TCPSocket::CONNECTED;
      UNLOCK(m_mutex);
      return ret;
   }

   int TcpThread::connectSocket()
   {
      uint32 ip = 0;
      int retval = 1;
      if(m_owner->getDNSCache().lookupHost(m_hostname, ip)){
         DBG("Host %s found in cache: %"PRIuIPFORMAT, m_hostname, IPEXPAND(ip));
         retval = m_socket->connect(ip, m_port);
         if(retval != 1){
            DBG("connect failed, unCacheing host %s", m_hostname);
            m_owner->getDNSCache().removeHost(m_hostname);
         }
      } else {
         retval = m_socket->connect(m_hostname, m_port);
         if(retval == 1){
            const uint32 ip = m_socket->getIP();
            DBG("socket->connect ok with code %d, "
                "caching host %s ip %"PRIuIPFORMAT, 
                retval, m_hostname, IPEXPAND(ip));
            m_owner->getDNSCache().cacheHost(m_hostname, ip);
         }
      }
      return retval;
   }

   void TcpThread::reconnect()
   {
      int retries = 0; // number of retries for a non-reconnecting Tcpthread.
      while(!stop() && 
            !socketConnected()){
         if(m_hostname == NULL){ // Listen on port
            TCPSocket::TCPSocketState state = m_listener->getState();
            switch(state){
            case TCPSocket::UNKNOWN:
            case TCPSocket::CLOSED:
               WARN("TCPSocket::CLOSED - this code is no more.");
//               old stuff no longer needed - i think /petersv 2003-08-27
//               m_listener->create(m_connParam);
//               break;
            case TCPSocket::OPEN:
            case TCPSocket::BOUND:
               m_socket->setupConnection(m_connParam);
               m_listener->listen(m_port);
               break;
            case TCPSocket::LISTEN:
               LOCK(m_mutex); 
               m_socket = m_listener->accept(ACCEPT_TIMEOUT);
               UNLOCK(m_mutex); 
               yield();
               break;
            default:
               break;
            }
         } else {                // connect to host
            LOCK(m_mutex);
            TCPSocket::TCPSocketState state = m_socket->getState();
            UNLOCK(m_mutex);
            switch(state){
            case TCPSocket::UNKNOWN:
            case TCPSocket::CLOSED:
               WARN("TCPSocket::CLOSED - this code is no more!");
//               old stuff no longer needed - i think /petersv 2003-08-27
//               LOCK(m_mutex);
//               if (!m_socket->create(m_connParam)) {
//                  ERR("TcpThread: m_socket create returned false!!");
//               }
//               UNLOCK(m_mutex); 
//               break;
            case TCPSocket::OPEN:
               {
                  int connErr;
                  // FIXME - add error handling here /petersv 2003-09-01
                  m_owner->connectionNotify(Module::WAITING_FOR_USER);
                  LOCK(m_mutex);
                  if (TCPSocket::None != (connErr = m_socket->setupConnection(m_connParam))) {
                     if (!stop()) {
                        m_stop = true;
                        m_reconnect = false;
                        m_reason = TransportFailed;
                        DBG("TcpThread: Reason set to Netfail (setupConnection failed)");
                     }
                  }  
                  UNLOCK(m_mutex);
                  m_owner->connectionNotify(Module::CONNECTING);
                  LOCK(m_mutex);
                  if(!stop() && //the first two are probably duplicates
                     connErr == TCPSocket::None && 
                     1 != (connErr = connectSocket())){
                     DBG("TcpThread: connErr = %d", connErr);
                     m_socket->close();
                     delete m_socket;
                     m_socket = NULL;
                     DBG("TcpThread: Socket deleted");
                     if(!stop() && (m_reconnect || retries-- > 0)){
                        DBG("TcpThread: Retry %d", retries);
                        m_socket = new TCPSocket( 5 );
                        UNLOCK(m_mutex);
                        if (Thread::sleep(CONNECT_WAIT) != Thread::AWOKE) {
                           DBG("TcpThread: TcpThread failed to wait");
                        } else {
                           DBG("TcpThread: TcpThread ok wait");
                        }
                        LOCK(m_mutex);
                     } else if(!stop()){
                        DBG("TcpThread: dont retry");
                        //no need to actually call terminate. Just set
                        //the right flags here. We are inside the
                        //mutex protected region anyway.
                        m_stop = true;
                        m_reconnect = false;
                        switch(connErr){
                        case TCPSocket::None:
                           ERR("TcpThread: socket::connect failed with "
                               "error None? Impossible!");
                           break;
                        case TCPSocket::NoLookup:
                           m_reason = NoLookup;
                           ERR("TcpThread: DNS lookup of '%s' failed", 
                               getHostname());
                           break;
                        case TCPSocket::Timeout:
                           m_reason = NetFail;
                           DBG("TcpThread: Reason set to Timeout");
                           break;
                        case TCPSocket::Refused:
                           m_reason = NoResponse;
                           DBG("TcpThread: Reason set to NoResponse");
                           break;
                        case TCPSocket::Unable:
                           m_reason = InternalError;
                           DBG("TcpThread: Reason set to InternalError");
                        }
                     }
                     DBG("Abort! Abort! Abort!");
                  } else {
                     DBG("socket->connect ok with code %d", connErr);
                  }
                  UNLOCK(m_mutex);
               } 
               break;
            default:
               break;
            }
         }
      }
      DBG("Leaving reconnect");
   }

   void TcpThread::run()
   {
      DBG("TcpThread::run");
      // Setup sockets
      LOCK(m_mutex); 
#ifndef __SYMBIAN32__
      m_selector = new Selector();
#endif
      if ( m_hostname == NULL ) { // Listening socket. 
         m_listener = new TCPSocket( 5 );
      }
      UNLOCK(m_mutex); 

      const int bufSize = 1024;
      int numRead = 0;
      uint8 readData[bufSize];
      while (!stop()) {
         LOCK(m_mutex);
         if ( m_hostname != NULL) {
            m_socket = new TCPSocket( 5 );
            if (!m_socket) {
               DBG("TcpThread: Big dodo in TCPThread, no m_socket!");
               UNLOCK(m_mutex);
               return;
            }
         }
         UNLOCK(m_mutex);

         m_owner->connectionNotify(Module::CONNECTING);
         reconnect();
         if (!stop()) {
            m_owner->connectionNotify(Module::CONNECTED);
#ifndef __SYMBIAN32__
            LOCK(m_mutex);
            m_selector->addSelectable(m_socket, Selector::READING);
            UNLOCK(m_mutex);
#endif
         }
         while(!stop() && numRead >= 0) {
            /* This access of m_socket is not protected by the
               mutex since the call may block. Hopefully, this will
               not be a problem as nothing should change neither
               the pointer to socket or the sockets state while the
               Thread is live.*/
#ifndef __SYMBIAN32__
            Selectable* wakeOn = NULL;
            int res = m_selector->select(-1, wakeOn);
#else
            int res = 1; // One socket, timeout below
#endif

            if(res > 0){
               // wakeOn should always contain something in this situation.
#ifndef __SYMBIAN32__
               numRead = m_socket->read(readData, bufSize);
#else
               numRead = m_socket->read( readData, bufSize, 200000 );
#endif
               if(numRead > 0){
                  // FIXME - is this really safe in the event of
                  // shutdown???? Thread safe rootPublic????
                  if (m_owner->rootPublic()) {
                     DBG("TcpThread pushing %d read bytes", numRead);
                     //DEBUGDUMP("data:", readData, numRead);
                     m_owner->rootPublic()->receiveData(numRead, readData);
                  } else {
                     DBG("TcpThread ignoring %d read bytes", numRead);
                  }
               }
#ifdef __SYMBIAN32__
               else if(numRead == -2){
                  numRead = 0;
               } 
#endif
               else if(m_reason != NoResponse){
                  m_reason = ByPeer;
                  DBG("TcpThread: Reason set to ByPeer");
                  LOGLOCK(m_mutex);
                  DBG("TcpThread: Got error of type %"PRId32": numread: %d", 
                      m_socket->m_error, numRead);
                  LOGUNLOCK(m_mutex);
               } else {
                  DBG("socket closed reason %d", m_reason);
               }
            }
         }

         LOCK(m_mutex);//LOCK HERE - UNLOCK AT DIFFERENT EXECUTION BRANCHES
         if (m_socket){
            DBG("TcpThread: Left loop with error of type %"PRId32, 
                m_socket->m_error);
#ifndef __SYMBIAN32__
            m_selector->removeSelectable(m_socket, Selector::READING);
#endif
         }
         if (m_socket != NULL){
            m_socket->close();
            delete m_socket;
            m_socket = NULL;
         }
         numRead = 0;
         DBG("TcpThread::run: Error reading from socket");
         DBG("TcpThread: Reconnecting? %s", m_reconnect ? "Yes indeed!" : "No way!");
         if(!m_reconnect){
            UNLOCK(m_mutex); //lock clearly marked above.
            break;
         }
         UNLOCK(m_mutex); //lock clearly marked above.
         /* Wait a while before reconnecting. */
         if (Thread::sleep(CONNECT_WAIT) != Thread::AWOKE) {
            DBG("TcpThread failed to wait");
         } else {
            //DBG("TcpThread ok wait");
         }
      }
      LOCK(m_mutex); //lock clearly marked above.
      m_owner->connectionNotify(Module::DISCONNECTING);
      
      if ( m_socket ) {
         m_socket->close();
         delete m_socket;
         m_socket = NULL;
      }
      
      if ( m_listener ) {
         m_listener->close();
         delete m_listener;
         m_listener = NULL;
      }
      m_owner->connectionNotify(Module::CLEAR);
      UNLOCK(m_mutex);//lock clearly marked above
      DBG("TcpThread runs out.");
   }
   
   bool TcpThread::stop()
   {
      LOCK(m_mutex); 
      bool ret = m_stop;
      UNLOCK(m_mutex); 
      return ret;
   }
}
