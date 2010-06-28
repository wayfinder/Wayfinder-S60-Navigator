/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WINTCPCONNECTIONHANDLER_H
#define WINTCPCONNECTIONHANDLER_H

#include "HttpClientConnection.h"
#include <windows.h>
#include <winsock.h>
#include <string>

/* Windows Implementation of TCPConnectionHandler */
/* Uses WinSock 1.1 */

/* Simple Socket class - synch DNS, asynch connection, disconnection and I/O */
class ClientSocket
{
   /** data **/
   private:
      /* the socket */
      SOCKET m_socket;
      /* the parent window */
      HWND m_parentWindow;
      /* the host and port */
      std::string m_host;
      int32 m_port;

   public:
      /* return codes for the read\write (IO) methods */
      enum 
      {
         IO_OK = -100,
         IO_ERROR = -101,
         IO_CLOSED = -102,
         IO_TIMEDOUT = -103
      };

   /** methods **/
   private:
      /* private constructor */
      ClientSocket();
      /* second-phase constructor */
      bool construct(HWND parentWin);

   public:
      /* allocator - async messages are sent to the parent window */
      static ClientSocket* allocate(HWND parentWin);

      /* destructor */
      ~ClientSocket();

      /* connects to the specified host and port */
      int connect(const char* hostName, int32 port);

      /* reconnects the socket to the previously specified host:port */
      int reconnect();

      /* disconnects the socket */
      int disconnect();

      /* shuts down all operations on the socket, but the socket remains connected */
      int shutdown();

      /* writes the specified bytes to the socket */
      int write(const byte* dataBuf, int32 len);

      /* reads bytes from the socket and returns 
         the data in the specified buffer. Returns 
         the actual number of bytes read in bytesRead */
      int read(byte* dataBuf, int32 maxReadLen, int32& bytesRead);
};

class WinTCPConnectionHandler : public TCPConnectionHandler
{
   /*** data ***/
   private:

      // hack to get the codes passed thru : FIXME
      /**
      *   Enum describing the status of the different calls.
      */
      enum status_t {
         /// All is done.
         TCPOK                =  0,
         /// Some error occured.
         TCPERROR             = -1,
         /// A timout occured
         TCPTIMEOUT           = -2,
         /// An operation of the same kind has been requested already.
         TCPALREADY_REQUESTED = -3,
         /// The connection was closed
         TCPCLOSED            = -4,
      };


      /* application instance */
      HINSTANCE m_appInstance;
      /* the window which handles the socket messages */
      HWND m_hwnd;
      /* the socket handle for connection */
      ClientSocket* m_conn;

      /* the current listener being called */
      TCPClientConnection* m_listener;

      /* the number of bytes to be written in the current write request */
      /* used during the async callback to check if another write is required */
      int m_bytesToWrite;
      /* the write buffer */
      byte* m_writeBuffer;

      /* number of bytes to read and the buffer to read them into */
      int m_bytesToRead;
      /* the read buffer */
      byte* m_readBuffer;

      /* the currently active object */
      static WinTCPConnectionHandler* m_curObject;

   /*** methods ***/
   private:
      /* private constructor */
      WinTCPConnectionHandler();

      /* second-phase constructor */
      bool construct(HINSTANCE hInst);

      /* sets the current handler */
      static void setCurObject(WinTCPConnectionHandler* obj)
      {
         m_curObject = obj;
         return;
      }

      /* prepare for an asynchronous operation */
      void prepareForAsync(TCPClientConnection* conn)
      {
         /* set the current object to ourselves */
         WinTCPConnectionHandler::setCurObject(this);
         /* set the listener */
         m_listener = conn;
         return;
      }

      /* check if the class name already exists */
      bool classExists(LPCTSTR className);

   public:

      /* Allocator.
       * hInst : the parent application's instance.
       */
      static WinTCPConnectionHandler* allocate(HINSTANCE hInst);

      /* destructor */
      ~WinTCPConnectionHandler();

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
       *   Tells the underlying layer to start receiving reads
       *   with up to maxLength bytes from the 
       *   socket. Max <code>length</code> bytes.
       *   Should call the ccon->readDone when done.
       *   Max one read per ClientConnection is allowed.
       *   Not allowed if not connected.
       */
      void read(int maxLength,
                TCPClientConnection* ccon);

      /**
       *   Removes the connection from the internal list of
       *   connections in the TCPConnectionHandler.
       */
      void remove(TCPClientConnection* conn);

      /**
       *   Requests a timeout for the connection conn.
       *   After the timeout, the connectionhandler should call
       *   timerExpired on the connection.
       *   @param nbrMillis The number of milliseconds to wait before
       *                    sending the timeout.
       *   @param conn      The connection that should have the timeout.
       *   @return ID of the timeout.
       */
      int requestTimer(uint32 nbrMillis,
                       TCPClientConnection* conn);

      /**
       *   Cancels the timer with id timerID.
       */
      void cancelTimer(int timerID);

      /*
       * The Window Message Processing method.
       */
      static LRESULT CALLBACK SocketWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
