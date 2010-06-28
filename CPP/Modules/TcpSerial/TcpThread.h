/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include "arch.h"
#include "Thread.h"
#include "TCPSocket.h"
#include "Module.h"

namespace isab {

   /** A wrapper for the TCPSocket, handling reconnects. Has its own 
       Thread of execution. */
   class TcpThread : public Thread
   {
      /** Set if the thread should reconnect when the connection is broken. */
      bool m_reconnect;

      /** Signals if the thread should exit. */
      bool m_stop;

      /** A pointer to the TcpSerial module this TcpThread serves. */
      class TcpSerial* m_owner;

      /** The hostname to connect to. If NULL, listen for connections 
          instead. */
      char* m_hostname;

      /** The remote port to connect to or the local port to listen at, 
          depending on context.*/
      int m_port;

      /** The Socket used for reading and writeing. */
      class TCPSocket* m_socket;

      /** The socket used for listening for connections. */
      class TCPSocket* m_listener;

      /** A mutex used to protect m_socket and m_lock. */
      Monitor m_mutex;

      /** The Selector object allows us to use a timeout on the reads.*/
      class Selector* m_selector;

      /** Parameter used to specify connection behaviour.*/
      int32 m_connParam;

   public:
      /***/
      enum DisconnectReason {
         OnOrder,
         ByPeer,
         NoLookup,
         NetFail,
         NoResponse,
         InternalError,
         TransportFailed,
      };
   private:
      /***/
      enum DisconnectReason m_reason;
      /** A loop handling connections and reconnections. */
      void reconnect();
      
      /** Tests the m_stop vaiable. This function is protected by the mutex. 
       * @return the current value of the m_stop variable. 
       */
      bool stop();

      /**
       * 
       */
      int connectSocket();

   public:
      /** The constructor. Creates a listning or connecting socket 
       * depending on the arguments.
       * @param name     the name of this Thread. 
       * @param owner    pointer to this TcpThread's owning TcpSerial module.
       * @param port     the remote port to connect to or the local port 
       *                 to listen at depending on the hostname argument.
       * @param hostname the hostname of the remote host to connect to.
       *                 If NULL, this TcpThread will listen for connections 
       *                 to the port specified in the port argument instead.
       */
      TcpThread(const char* name, int32 connectionParameter, TcpSerial* owner, 
                int port, const char* hostname = NULL, int reconnect = 0);
      /* FIXME - XXX - FIXME !!!!!!!!!!!!!!!!!!!!!  reconnect should
       * default to true. This hack is done to prevent GuiTcp from
       * reconnecting during shutdown. Dring a reconnect of an
       * listening socket TcpThread is stuck in the accept call for
       * many seconds. This is only a quick workaround. */
      
      /** The destructor. Stops the thread and closes all sockets. */
      virtual ~TcpThread();

      /** Gets the current state of the read/write socket.
       * @return the current status of m_socket.
       */
      TCPSocket::TCPSocketState getState();

      /** Tests, protcted by mutex, whether m_socket != NULL and is connected.
       */
      bool socketConnected();


      /** Tells if this Tcpthread autamatically reconnects when losing
       * connection. 
       * @return true if this thread automatically reconnects.
       */
      bool isReconnecting() const;

      /** The run method. */
      virtual void run();
      
      /** Stops this thread. */
      virtual void terminate();
      
      /** Writes data to the socket. 
       * @param data   a byte array at least <code>length</code> bytes long.
       * @param length the number of bytes to write.
       * @return the number of bytes written. 
       */
      ssize_t write(const uint8 *data, ssize_t length);

      /** Returns the hostname.
       * @return the hostname.
       */
      const char* getHostname() const;

      /** Returns the port number.
       * @return the port number.
       */
      int getPort() const;

      enum Module::ConnectionNotifyReason getDisconnectReason() const;
      void setDisconnectReason(enum DisconnectReason reason);
      
      void setConnectionParam(int32 newVal);
   };

   //=============================================================
   //======= inlines for TcpThread ===============================
   
   inline const char* TcpThread::getHostname() const
   {
      return m_hostname;
   }

   inline int TcpThread::getPort() const
   {
      return m_port;
   }

   inline bool TcpThread::isReconnecting() const
   {
      return m_reconnect;
   }

   inline enum Module::ConnectionNotifyReason
   TcpThread::getDisconnectReason() const
   {
      switch(m_reason){
      case TcpThread::OnOrder:         return  Module::REQUESTED;
      case TcpThread::ByPeer:          return  Module::BY_PEER;        
      case TcpThread::NoLookup:        return  Module::LOOKUP_FAILED;
      case TcpThread::TransportFailed: return  Module::TRANSPORT_FAILED; 
      case TcpThread::NetFail:         return  Module::CHANNEL_FAILED; 
      case TcpThread::NoResponse:      return  Module::NO_RESPONSE;    
      case TcpThread::InternalError:   return  Module::INTERNAL_ERROR; 
      }
      return Module::UNSPECIFIED;
   }

   inline void TcpThread::setDisconnectReason(enum DisconnectReason reason){
      m_reason = reason;
   }

   inline void TcpThread::setConnectionParam(int32 newVal)
   {
      m_mutex.lock();
      m_connParam = newVal;
      m_mutex.unlock();
   }

}

#endif
