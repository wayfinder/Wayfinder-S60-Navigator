/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _SYMBIANTCPCONNECTIONHANDLER_H_
#define _SYMBIANTCPCONNECTIONHANDLER_H_

#include "HttpClientConnection.h"
#include <e32base.h>
#include <es_sock.h>
#include <in_sock.h>
#include "LogFile.h"

#ifdef NAV2_CLIENT_SERIES60_V2
# define USE_RCONNECTION
#endif

class SymbianTCPConnectionHandler;

/**
 *  Symbian Socket Writer Class
 */
class SymbianSocketWriter : public CActive
{
   private:
      /**
       * Socket to write to.
       * Should already be opened and connected.
       */
      RSocket& m_writeSocket;

      /**
       * stores the pointer to the data until write completes
       */
      TPtrC8 m_dataPtr;

      /**
       * callback to use on completion of write
       */
      TCPClientConnection* m_writeCallback;

      /**
       * Stores the parent socket engine.
       * Used to completely close a connection.
       */
      SymbianTCPConnectionHandler* m_parent;

   protected:
      /**
       * handles the write when completed
       */
      void RunL();

      /**
       * Cancels all pending reads and stops the reading
       */
      void DoCancel();

   public:
      /**
       * Constructor
       */
      SymbianSocketWriter(RSocket& writeSock);

      /**
       * Destructor
       */
      ~SymbianSocketWriter();

      /**
       * sets the parent socket engine
       */
      void setParent(SymbianTCPConnectionHandler* parent);

      /**
       * Write specified bytes to the socket
       */
      void Write(TCPClientConnection* conn, 
                   const byte* data, int32 dataLen);
};



/**
 *  Symbian Socket Reader Class
 */
class SymbianSocketReader : public CActive
{
   private:
      /**
       * Socket to read from.
       * Should already be opened and connected.
       */
      RSocket& m_readSocket;

      /**
       * Internal buffer to store incoming data.
       */
      TBuf8<1024> m_buffer;
      
      /**
       * length of last packet recieved
       */
      TSockXfrLength m_prevPacketLen;

      /**
       * callback to use on completion of read
       */
      TCPClientConnection* m_readCallback;

      /**
       * Stores the parent socket engine.
       * Used to completely close a connection.
       */
      SymbianTCPConnectionHandler* m_parent;

   protected:
      /**
       * handles the read when completed
       */
      void RunL();

      /**
       * Cancels all pending reads and stops the reading
       */
      void DoCancel();

   public:
      /**
       * Constructor
       */
      SymbianSocketReader(RSocket& readSock);

      /**
       * Destructor
       */
      ~SymbianSocketReader();

      /**
       * sets the parent socket engine
       */
      void setParent(SymbianTCPConnectionHandler* parent);

      /**
       * reads bytes uptil maxLen from the socket
       */
      void Read(TCPClientConnection* conn, int32 maxLen);
};



/**
 *  Symbian TCP Connection Handler
 */
class SymbianTCPConnectionHandler : public TCPConnectionHandler, 
                                    public CActive
{
   public:
      /**
       * Possible states for the object
       */
      enum connStatus
      {
         ENotConnected,
         EConnecting,
         EConnected
      };

   private:
      /**
       * Socket Server 
       */
      RSocketServ m_socketServer;

#ifdef USE_RCONNECTION
      /**
       * Connection manager
       */
      RConnection m_connMgr;
#endif

      /**
       * for resolving names using DNS
       */
      RHostResolver m_nameResolver;

      /**
       * Socket used to connect
       */
      RSocket m_socket;

      /**
       * state of the connectionHandler
       */
      connStatus m_status;

      /**
       * stores the IP of the host. Used during Connect and 
       * must remain valid until RunL is invoked.
       */
      TInetAddr m_address;

      /**
       * stores the host port
       */
      TInt m_hostPort;

      /**
       * stores the current callback handler for the active object
       */
      TCPClientConnection* m_callback;

      /**
       * Socket writer
       */
      SymbianSocketWriter m_socketWriter;

      /**
       * Socket reader
       */
      SymbianSocketReader m_socketReader;

   protected:
      /**
       * handles the request when completed
       */
      void RunL();

      /**
       * Cancels all pending requests and stops the active object
       */
      void DoCancel();

   public:
      /**
       * Constructor
       */
      SymbianTCPConnectionHandler();
      /**
       * Destructor
       */
      ~SymbianTCPConnectionHandler();

      /**
       * Resets the state of the connection.
       */
      void resetStatus();

      /**
       * methods from TCPConnectionHandler
       */
      void connect(const char* host, unsigned int port,
                        TCPClientConnection* ccon);
      void disconnect(TCPClientConnection* ccon);
      void write(const byte* bytes, int length,
                      TCPClientConnection* ccon);
      void read(int maxLength,
               TCPClientConnection* ccon);
      void remove(TCPClientConnection* conn);
      int requestTimer(uint32 nbrMillis, 
                     TCPClientConnection* conn);
      void cancelTimer(int timerID);
};

#endif
