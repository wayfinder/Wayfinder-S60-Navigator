/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GTK_TCP_CONNECTIONHANDLER_H
#define GTK_TCP_CONNECTIONHANDLER_H

#include "HttpClientConnection.h"

//#include <gtk--/main.h>
//#include <gtk--/widget.h>
#include <gtk/gtk.h>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class GtkTCPConnectionData {
public:
   GtkTCPConnectionData() {
      m_fd = -1;
      m_readBuf = 0;
      m_readLength = 1;
   }
   
   /**
    *   The event source id which signals if there are bytes to read.
    */
   guint m_inSignal;
   
   /**
    *   The event source id which signals if there are bytes to write.
    */
   guint m_outSignal;
   
   /// The file descriptor
   int m_fd;

   /// The current read buffer
   byte* m_readBuf;
   /// The current read length
   int m_readLength;
   /// The sockaddr_in
   struct sockaddr_in m_sin;
};

class GtkTCPConnectionHandler : public TCPConnectionHandler {
public:

   /**
    *   Sets some variables.
    */
   GtkTCPConnectionHandler();
   virtual ~GtkTCPConnectionHandler() {}

   /**
    *   Tells the underlying layer to connect to the
    *   supplied host and port. When connected, connected
    *   should be called.
    *   Should call ccon->connectDone when done.
    *   Max one connect per ClientConnection is allowed simultaneously.
    */
   void connect(const char* host, unsigned int port,
                TCPClientConnection* ccon);

   /**
    *   Tells the underlying layer to disconnect the
    *   current connection.
    */
   void disconnect(TCPClientConnection* ccon);

   /**
    *   Tells the underlying layer to write the supplied
    *   bytes. The underlying layer must keep the order
    *   of the write requests.
    *   Should call ccon->writeDone when done.
    *   Max one write per ClientConnection is allowed.
    *   Not allowed if not connected.
    */
   void write(const byte* bytes, int length,
              TCPClientConnection* ccon);
      
   /**
    *   Tells the underlying layer to read up to length bytes from the
    *   socket. Max <code>length</code> bytes.
    *   Should call the ccon->readDone when done.
    *   Max one read per ClientConnection is allowed.
    *   Not allowed if not connected.
    */
   void read(int length,
             TCPClientConnection* ccon);

   /**
    *   Removes the conn from the internal storage.
    */
   void remove(TCPClientConnection* conn);

   /**
    *   Requests a timeout after nbrMillis milliseconds.
    */
   int requestTimer(uint32 nbrMillis, TCPClientConnection* conn);
   
   /**
    *   Cancels the timer with id timerID.
    */
   void cancelTimer(int timerID);
   

protected:

   /**
    *   Connects or disconnects a read slot to the connection.
    */
   void handleReadSlot(TCPClientConnection* conn, bool connect);
   
   /**
    *   Called by GTK when there is something to read on the fd.
    */
   void readCallBack(TCPClientConnection* conn);
   
   /**
    *   Connects or disconnects a write slot to the connection.
    */
   void handleWriteSlot(TCPClientConnection* conn, bool connect);
   
   /**
    *   Called by GTK when there is something to write on the fd.
    */
   void writeCallBack(TCPClientConnection* conn);

   /**
    *   Called when a timeout has expired.
    */
   int on_timeout(int timerID, TCPClientConnection* conn);

   typedef struct CallbackInfo {
      int timer_id;
      GtkTCPConnectionHandler *handler;
      TCPClientConnection *conn;
   };

private:
   /*
   static gboolean gdkReadCallback( GIOChannel* channel, 
                                    GIOCondition condition, gpointer data );
*/
   static void gdkReadCallback( gpointer data,
                                gint source,
                                GdkInputCondition condition );
   static void gdkWriteCallback( gpointer data,
                                 gint source,
                                 GdkInputCondition condition );
   /*
   static gboolean gdkWriteCallback( GIOChannel* channel, 
                                     GIOCondition condition, gpointer data );
   */
   static gboolean timeoutCallback(CallbackInfo *info);
   /// Read buffer
   byte* m_readBuf;

   /// Size of readbuf
   int m_readBufSize;


   int innerConnect(const char* host, unsigned int port,
                     TCPClientConnection* con);
   
   /// Type of storage
   typedef map<TCPClientConnection*, GtkTCPConnectionData> connStorage_t;
   /// Map containing the current connections.
   connStorage_t m_connections;

   /// Type of storage for timers.
   typedef map<int, int> timerStorage_t;

   /// Storage of timers
   timerStorage_t m_timers;

   /// ID of the next timer
   int m_nextTimerID;
   
};

#endif
