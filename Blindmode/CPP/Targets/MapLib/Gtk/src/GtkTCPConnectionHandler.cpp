/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "config.h"

#include "GtkTCPConnectionHandler.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <errno.h>

#include <arpa/inet.h>

GtkTCPConnectionHandler::GtkTCPConnectionHandler()
{
   m_nextTimerID = 0;
   m_readBuf = NULL;
   m_readBufSize = 0;
   // FIXME: Add destructor
   signal(SIGPIPE, SIG_IGN);
}

int
GtkTCPConnectionHandler::innerConnect(const char* hostname, unsigned int port,
                                      TCPClientConnection* con)
{
   connStorage_t::iterator it = m_connections.find(con);
   if ( it == m_connections.end() ) {
      m_connections.insert(make_pair(con, GtkTCPConnectionData()));
      it = m_connections.find(con);
   }
   
   if ( it->second.m_fd < 0 ) {
      // Not connected
      int tcp_proto_nbr = getprotobyname("tcp")->p_proto;
      it->second.m_fd =  socket( PF_INET, SOCK_STREAM, tcp_proto_nbr );
      if ( it->second.m_fd < 0 ) {
         // Error
         mc2dbg << "[GTKTCP]: Error : could not open socket : "
                << strerror(errno) << endl;
         return -1;
      }
      static const unsigned int on = 1;
      if (setsockopt(it->second.m_fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&on,
                     sizeof(on)) < 0 ) {
         mc2log << warn
                << "[GTKTCP]: Failed to set SO_KEEPALIVE; "
                << strerror(errno) << endl;
      }
      static const int level = IPPROTO_TCP;
      // Disable Nagle. We will not type stuff.
      if (setsockopt(it->second.m_fd, level, TCP_NODELAY,
                     (char*)&on, sizeof(on)) < 0 ) {
         mc2log << warn << "[GTKTCP]: failed to set TCP_NODELAY; "
                << strerror(errno) << endl;
      }

      // Set non-blocking
      int res = fcntl(it->second.m_fd, F_SETFL, O_NONBLOCK);
      if ( res < 0 ) {
         mc2dbg << "[GTKTCP]: Error : could not set non-blocking : "
                << strerror(errno) << endl;
         return -1;
      }
      
      
      // Get hostname
      struct sockaddr_in& sin = it->second.m_sin;
      struct hostent *hostEntry;

      memset(&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      uint16 portnumber = port;
      mc2dbg << "[GtkTCPConnectionData]: port is " << portnumber << endl;
      sin.sin_port = htons((u_short)portnumber);
      
      if ((hostEntry = gethostbyname(hostname)) != 0) {
         memcpy(&sin.sin_addr, hostEntry->h_addr, hostEntry->h_length);
      } else {
         if ((sin.sin_addr.s_addr = inet_addr(hostname)) == INADDR_NONE) {
            mc2dbg << "[GTKTCP]: Could not lookup address: "
                   << hostname << ":" << strerror(errno)
                   << endl;
            return -1;
         }
      }
      // And connect
      // Activate writeslot
      handleWriteSlot(con, true);
      if ( ::connect(it->second.m_fd,
                   (struct sockaddr *)&sin, sizeof(sin)) < 0 ) {
         if ( errno == EAGAIN || errno == EINPROGRESS ) {
            return 1; // Wait for something to happen.
         } else {
            mc2dbg << "[GTKTCP]: Error -1" << strerror(errno)
                   << endl;
            return -1;
         }
      } else {
         return 2;
      }
   } else {
      mc2dbg << "[GTKTCP]: Already has fd" << endl;
      return 3;
   }
      
}

void
GtkTCPConnectionHandler::connect(const char* host, unsigned int port,
                                 TCPClientConnection* con)
{
   int res = innerConnect(host, port, con);
   if ( res > 0 ) {
      mc2dbg << "[GtkTCPConnectionHandler]: res is " << res << endl;
      
      if ( res == 2 ) {
         mc2dbg << "[GtkTCPConnectionHandler]: I pity the fool" << endl;
         con->connectDone(TCPClientConnection::OK);
      } else {
       
      }
   } else {
      mc2dbg << "[GtkTCPConnectionHandler]: connect failed" << endl;      
   }
}

void
GtkTCPConnectionHandler::readCallBack(TCPClientConnection* conn)
{
   
   mc2dbg << "[GTKTCP]: readCallBack called for fd"<<endl;

   connStorage_t::iterator it = m_connections.find(conn);
   // if no connection found return
   if ( it == m_connections.end() )
      return;


   if ( it->second.m_readLength ) {
      if ( m_readBuf == NULL || m_readBufSize < it->second.m_readLength ) {
         mc2dbg << "[GTKTCP]: Creating read buffer " << endl;
         delete [] m_readBuf;
         m_readBufSize = it->second.m_readLength;
         m_readBuf = new byte[m_readBufSize];
      }
      int readRes =
         ::read(it->second.m_fd,
                m_readBuf, it->second.m_readLength );
      // Don't use the buffer anymore.
      if ( readRes != 0 ) {
         conn->readDone(TCPClientConnection::OK, m_readBuf, readRes);
      } else {
         // Connection was closed
         mc2dbg << "[GTKTCP]: Read returned 0 disconnecting" << endl;
         remove(conn);
         conn->connectionClosed(TCPClientConnection::OK);
      }
   } else {
      mc2dbg << "[GtkTCPConnectionHandler]: Read called for "
             << "connection without buffer" << endl;
   }

}

void
GtkTCPConnectionHandler::
writeCallBack(TCPClientConnection* conn)
{
   mc2dbg << "[GTKTCP]: writeCallBack called" << endl;

   handleWriteSlot(conn, false);

   connStorage_t::iterator it = m_connections.find(conn);
   if ( ::connect(it->second.m_fd,
                  (struct sockaddr *)&(it->second.m_sin),
                  sizeof(it->second.m_sin)) < 0 ) {
      mc2dbg << "[GtkTCPConnectionHandler]: error when connecting "
             << strerror(errno) << endl;
      ::close(it->second.m_fd);
      it->second.m_fd = -1;
      conn->connectDone(TCPClientConnection::ERROR);
   } else {
      // Activate the read callback
      handleReadSlot(conn, true);
      conn->connectDone(TCPClientConnection::OK);
   }
}

typedef struct ConnData {
   GtkTCPConnectionHandler *handler;
   TCPClientConnection *client;
};

void
GtkTCPConnectionHandler::
gdkReadCallback( gpointer data,
                 gint source,
                 GdkInputCondition condition )
{
   ConnData *conndata = static_cast<ConnData*>( data );
   conndata->handler->readCallBack( conndata->client ); 
}

void
GtkTCPConnectionHandler::
gdkWriteCallback( gpointer data,
                  gint source,
                  GdkInputCondition condition ) 
{
   ConnData *conndata = static_cast<ConnData*>( data );
   conndata->handler->writeCallBack( conndata->client ); 
}

void destroyCallbackData(gpointer data) {
   delete static_cast<ConnData *>(data);
}

void
GtkTCPConnectionHandler::handleReadSlot(TCPClientConnection* conn,
                                        bool connect)
{
   connStorage_t::iterator it = m_connections.find(conn);
   if ( connect ) {
      // Disconnect.
      handleReadSlot(conn, false);
      
      // create connection data that we
      // pass to callback function
      ConnData *data = new ConnData;
      data->handler = this;
      data->client = conn;

      //
      // Note: gdk_input_add_full is deprecated
      // but currently the g_io_* functions are 
      // too ....painfull
      guint event_id = gdk_input_add_full(
               it->second.m_fd,
               (GdkInputCondition)(GDK_INPUT_READ | GDK_INPUT_EXCEPTION),
               GtkTCPConnectionHandler::gdkReadCallback,
               data,
               destroyCallbackData);

      it->second.m_inSignal = event_id;
   } else {
      gdk_input_remove( it->second.m_inSignal );
   }
}


void
GtkTCPConnectionHandler::handleWriteSlot(TCPClientConnection* conn,
                                         bool connect)
{
   connStorage_t::iterator it = m_connections.find(conn);
   if ( connect ) {
      // Disconnect...
      // note: old code here, not sure if we should disconnect or not...?
      // handleWriteSlot(conn, false);

      // create connection data that we
      // pass to callback function
      ConnData *data = new ConnData;
      data->handler = this;
      data->client = conn; 

      //
      // Note: gdk_input_add_full is deprecated
      // but currently the g_io_* functions are 
      // too ....painfull
      guint event_id = gdk_input_add_full( 
                it->second.m_fd,                                     
                GDK_INPUT_WRITE,
                GtkTCPConnectionHandler::gdkWriteCallback,
                data,
                destroyCallbackData);


      it->second.m_outSignal = event_id;
   } else {
      gdk_input_remove( it->second.m_outSignal );
   }
}

void
GtkTCPConnectionHandler::read(int length,
                              TCPClientConnection* conn)
{
   mc2dbg << "[GtkTCPConnectionHandler]: read" << endl;
   connStorage_t::iterator it = m_connections.find(conn);
   if ( it != m_connections.end() ) {
      mc2dbg << "[GtkTCPConnectionHandler]: read (2)" << endl;
      it->second.m_readLength = length;
#if 0
      if ( m_readBuf == NULL || m_readBufSize < it->second.m_readLength ) {
            mc2dbg << "[GTKTCP]: Creating read buffer " << endl;
            delete [] m_readBuf;
            m_readBufSize = it->second.m_readLength;
            m_readBuf = new byte[m_readBufSize];
      }
      int readRes;
      if ((readRes = ::read(it->second.m_fd, m_readBuf,
                 it->second.m_readLength)) >= 0 ) {         
         conn->readDone(TCPClientConnection::OK, readRes);
      } else {
         mc2dbg << "[GtkTCPConnectionHandler] readRes = " << readRes
                << " strerror = " << strerror(errno) << endl;
      }
#endif      
      //handleReadSlot(conn, true);
   }
}

void
GtkTCPConnectionHandler::disconnect(TCPClientConnection* conn)
{
   connStorage_t::iterator it = m_connections.find(conn);
   if ( it != m_connections.end() ) {
      // We should shut down nicely instead.
      ::close(it->second.m_fd);
      it->second.m_fd = -1;
   }
}

void
GtkTCPConnectionHandler::write(const byte* bytes, int length,
                               TCPClientConnection* conn)
{
   mc2dbg << "[GtkTCPConnectionHandler]: write" << endl;
   connStorage_t::iterator it = m_connections.find(conn);
   if ( it != m_connections.end() ) {
      // Will not work properly
      // Set blocking
      int res = fcntl(it->second.m_fd, F_SETFL, 0);
      if ( res < 0 ) {
         mc2dbg << "[GTKTCP]: Error : could not set blocking : "
                << strerror(errno) << endl;
      }
      do {
         res = ::write(it->second.m_fd, bytes, length);
         if ( res >= 0 ) {
            bytes  += res;
            length -= res;
         }
      } while (length && res >= 0);
      mc2dbg << "[Gtktcp]: " << length << " bytes left to send" << endl;
      // Set non-blocking again
      res = fcntl(it->second.m_fd, F_SETFL, O_NONBLOCK);
      if ( res < 0 ) {
         mc2dbg << "[GTKTCP]: Error : could not set nonblocking : "
                << strerror(errno) << endl;
      }
      mc2dbg << "[GTKTCP]: Calling writeDone" << endl;
      conn->writeDone(TCPClientConnection::OK);
   }
}

void
GtkTCPConnectionHandler::remove(TCPClientConnection* conn)
{
   connStorage_t::iterator it = m_connections.find(conn);
   if ( it != m_connections.end() ) {
      handleReadSlot(conn, false);
      handleWriteSlot(conn, false);
      m_connections.erase(conn);
      ::close(it->second.m_fd);
      it->second.m_fd = -1;         
   }
}

gboolean GtkTCPConnectionHandler::timeoutCallback(CallbackInfo *info) {

   uint32 timer_id = info->timer_id;
   TCPClientConnection *conn = info->conn;
   // make sure connection is still there
   if (info->handler->m_connections.find( conn ) != 
       info->handler->m_connections.end() ) {
      conn->timerExpired( timer_id );
      info->handler->m_timers.erase( timer_id );
   }

   delete info;

   return false;
}

int
GtkTCPConnectionHandler::requestTimer(uint32 nbrMillis,
                                      TCPClientConnection* conn)
{

   m_nextTimerID++;

   //!! TODO !!
   // if timer never expires or we destroy this object
   // the the callback info will still be allocated ...
   // note: check this some more

   CallbackInfo *info = new CallbackInfo;
   info->handler = this;
   info->timer_id = m_nextTimerID;
   info->conn = conn;
   m_timers[m_nextTimerID] = g_timeout_add( nbrMillis,
                  (GSourceFunc)GtkTCPConnectionHandler::timeoutCallback,
                  info );

   return m_nextTimerID;
}

void
GtkTCPConnectionHandler::cancelTimer(int timerID)
{
   timerStorage_t::iterator it = m_timers.find(timerID);
   if ( it != m_timers.end() ) {
      g_source_remove( m_timers[timerID] );
      m_timers.erase( timerID );
   }
}
