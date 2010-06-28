/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SymbianTCPConnectionHandler.h"
#include <eikenv.h>

/* various messages for debugging info */
_LIT(KMsgGenericMsgTitle, "TMap :");
_LIT(KMsgSocketConnected, "Connected!");
_LIT(KMsgSocketDisconnected, "Disconnected!");
_LIT(KMsgNameResolved, "IP Resolved!");
_LIT(KMsgReadFromSocket, "Packet Read!");
_LIT(KMsgWriteToSocket, "Packet Written!");

/* error messages */
_LIT(KMsgGenericErrorTitle, "TMap : Error :");
_LIT(KMsgDNSError, "Err : DNS Server Connection!");
_LIT(KMsgDNSNameError, "Err : DNS Name Resolution!");
_LIT(KMsgConnectError, "Err : TMap Server Connection!");
_LIT(KMsgSocketClosedRead, "Err : Read Socket Closed!");
_LIT(KMsgSocketClosedWrite, "Err : Write Socket Closed!");
_LIT(KMsgReadError, "Err : Socket Read!");
_LIT(KMsgWriteError, "Err : Socket Write!");

/* busy messages */
_LIT(KMsgBusyDNS, "Resolving IP...");
_LIT(KMsgBusyConn, "Connecting...");

/* macros to display above messages */
#define SHOWMSGWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericMsgTitle, x)
#define SHOWMSG(x)   CEikonEnv::Static()->InfoMsg(x)
#define SHOWERRWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericErrorTitle, x)
#define SHOWERR(x)   CEikonEnv::Static()->InfoMsg(x)
#define SHOWBUSY(x)  CEikonEnv::Static()->BusyMsgL(x)
#define CANCELBUSY() CEikonEnv::Static()->BusyMsgCancel()

/* debug : logfiles for socket times */
_LIT(KSocketReadLog, "c:\\system\\temp\\ASocketRead.log");
_LIT(KSocketWriteLog, "c:\\system\\temp\\ASocketWrite.log");
_LIT(KSocketConnectLog, "c:\\system\\temp\\ASocketConnect.log");

/* converts a c-string to a unicode Symbian descriptor */
static int char2TDes(TDes &dest, const char* src)
{
   dest.Zero();
   if (!src) {
      dest.PtrZ();
      return 0;
   }
   int length = strlen(src);
   int i = 0;

   // Crashes if we try to write more then maxlength in the buffer
   if( length >= dest.MaxLength() )
      length = dest.MaxLength()-1;

   while (i < length) {
      dest.Append((unsigned int)(src[i]));
      i++;
   }
   dest.PtrZ();
   return length;
}



/**
 * SymbianSocketWriter
 */

/**
 * Constructor
 */
SymbianSocketWriter::SymbianSocketWriter(RSocket& writeSock) :
      CActive(EPriorityNormal),
      m_writeSocket(writeSock),
      m_dataPtr(NULL,0),
      m_writeCallback(NULL),
      m_parent(NULL) {

   /* add this to the ActiveScheduler */
   CActiveScheduler::Add(this);

}

/**
 * Destructor
 */
SymbianSocketWriter::~SymbianSocketWriter() {

   if(IsActive()) {
      Cancel();
   }
   /* remove this object from the ActiveScheduler */
   Deque();
}


/**
 * sets the parent socket engine
 */
void SymbianSocketWriter::setParent(SymbianTCPConnectionHandler* parent) {
   m_parent = parent;
   return;
}


/**
 * Write specified bytes to the socket
 */
void SymbianSocketWriter::Write(TCPClientConnection* conn, 
                                const byte* data, int32 dataLen) {
   
   /* check if a write is already in progress */
   if(IsActive()) {
      /* return if one is still active */
      return;
   }

   /* set the callback */
   m_writeCallback = conn;

   /* create a TPtr8 out of the byte data to send to socket */
   m_dataPtr.Set(data, dataLen);

   /* start the asynchronous socket writing */
   m_writeSocket.Write(m_dataPtr, iStatus);

   /* set this object as active so it gets processed */
   SetActive();

   return;
}

/**
 * handles the write when completed
 */
void SymbianSocketWriter::RunL() {

   if(iStatus == KErrNone) {
      /* data got written correctly */
      m_writeCallback->writeDone(TCPClientConnection::OK);
      SHOWMSG(KMsgWriteToSocket);
   }
   else if(iStatus == KErrEof) {
      /* Error, socket has been closed */
      m_parent->remove(m_writeCallback);
      //m_writeCallback->connectionClosed(TCPClientConnection::CLOSED);
      SHOWERR(KMsgSocketClosedWrite);
   }
   else {
      /* Error, some kind of write error */
      //m_writeCallback->writeDone(TCPClientConnection::ERROR);
      m_parent->remove(m_writeCallback);
      SHOWERR(KMsgWriteError);
   }
   return;
}

/**
 * Cancels all pending reads and stops the reading
 */
void SymbianSocketWriter::DoCancel() {
   /* shutdown all write operations */
   //m_writeSocket.CancelWrite();
   if(m_writeCallback) {
      //m_dataPtr.Set(NULL,0);
      /* call the listener with Error */
      //m_writeCallback->connectionClosed(TCPClientConnection::OK);
   }

   return;
}


/**
 * SymbianSocketReader
 */

/**
 * Constructor
 */
SymbianSocketReader::SymbianSocketReader(RSocket& readSock) :
      CActive(EPriorityNormal),
      m_readSocket(readSock),
      m_prevPacketLen(0),
      m_readCallback(NULL),
      m_parent(NULL)
{
   /* add this to the ActiveScheduler */
   CActiveScheduler::Add(this);

}

/**
 * Destructor
 */
SymbianSocketReader::~SymbianSocketReader() {

   if(IsActive()) {
      Cancel();
   }
   /* remove this object from the ActiveScheduler */
   Deque();
}

/**
 * sets the parent socket engine
 */
void SymbianSocketReader::setParent(SymbianTCPConnectionHandler* parent) {
   m_parent = parent;
   return;
}

/**
 * reads bytes uptil maxLen from the socket
 */
void SymbianSocketReader::Read(TCPClientConnection* conn,
                               int32 /*maxLen*/) {

   /* check if a read is already in progress */
   if(IsActive()) {
      /* return if one is still active */
      return;
   }

   /* set the callback */
   m_readCallback = conn;

   /* start the asynchronous socket reading */
   m_readSocket.RecvOneOrMore(m_buffer, 0, iStatus, m_prevPacketLen);

   /* set this object as active so it gets processed */
   SetActive();

   return;
}

/**
 * handles the read when completed
 */
void SymbianSocketReader::RunL() {

   if(iStatus == KErrNone) {
      /* data got read correctly */
      m_readCallback->readDone(TCPClientConnection::OK, 
                               m_buffer.Ptr(),
                               m_prevPacketLen());
      SHOWMSG(KMsgReadFromSocket);
   }
   else if(iStatus == KErrEof) {
      /* Error, socket has been closed */
      m_parent->remove(m_readCallback);
      //m_readCallback->connectionClosed(TCPClientConnection::CLOSED);
      SHOWERR(KMsgSocketClosedRead);
   }
   else {      
      /* Error, some kind of read error */
      //m_readCallback->readDone(TCPClientConnection::ERROR, NULL, 0);
      //m_readCallback->connectionClosed(TCPClientConnection::CLOSED);
      m_parent->remove(m_readCallback);
      SHOWERR(KMsgReadError);
   }

   return;
}

/**
 * Cancels all pending reads and stops the reading
 */
void SymbianSocketReader::DoCancel() {

   /* shutdown all read operations */
   //m_readSocket.CancelRecv();
   if(m_readCallback) {
      /* call the listener with Error */
      //m_readCallback->connectionClosed(TCPClientConnection::OK);
   }
}



/**
 * SymbianTCPConnectionHandler
 */

/**
 * Constructor
 */
SymbianTCPConnectionHandler::SymbianTCPConnectionHandler() :
      CActive(EPriorityNormal),
      m_status(ENotConnected),
      m_hostPort(0),
      m_callback(NULL),
      m_socketWriter(m_socket),
      m_socketReader(m_socket)
{

   /* try to connect to the Socket Server */
   if(m_socketServer.Connect() != KErrNone) {
      /* call the connectDone() with error */
      //ccon->connectDone(TCPClientConnection::ERROR);
      SHOWERR(KMsgConnectError);
   }
   
   /* try to connect the connection managet to the Socket Server */
#ifdef USE_RCONNECTION
   if(m_connMgr.Open(m_socketServer) != KErrNone) {
      /* call the connectDone() with error */
      SHOWERR(KMsgConnectError);
   }
#endif

   /* add this to the ActiveScheduler */
   CActiveScheduler::Add(this);

   /* set this as parents of reader/writer class */
   m_socketWriter.setParent(this);
   m_socketReader.setParent(this);

}

/**
 * Destructor
 */
SymbianTCPConnectionHandler::~SymbianTCPConnectionHandler() {
   if(IsActive()) {
      Cancel();
   }

#ifdef USE_RCONNECTION
   m_connMgr.Close();
#endif

   m_socketServer.Close();

   /* this object from the ActiveScheduler */
   Deque();
}

/**
 * Resets the state of the connection.
 */
void SymbianTCPConnectionHandler::resetStatus() {

   /* if the socket is connected, close all socket classes */
   if(m_status == EConnected) {
      m_socket.Close();
   }

   if ( m_socketWriter.IsActive() ) {
      m_socketWriter.Cancel();
   }
   if ( m_socketReader.IsActive() ) {
      m_socketReader.Cancel();
   }

#ifdef USE_RCONNECTION
   //This will force the connection to be dropped. Probably no right.
   //m_connMgr.Stop();
   //m_connMgr.Close();
#endif

   m_socketReader.setParent(this);
   
   /* set status to not connected */
   m_status = ENotConnected;

   return;
}

/**
 * methods from TCPConnectionHandler
 */
void SymbianTCPConnectionHandler::connect(const char* host, unsigned int port,
                  TCPClientConnection* ccon) {

   if(IsActive()) {
      return;
   }
   
   /* 
      If already connected, tell the stupid HttpClientConnection.
   */
   if(m_status == EConnected) {
      ccon->connectDone(TCPClientConnection::OK);
   }
#ifdef USE_RCONNECTION
   if(m_connMgr.Open(m_socketServer) != KErrNone) {
      /* call the connectDone() with error */
      SHOWERR(KMsgConnectError);
   }
#endif

#ifdef USE_RCONNECTION
   /* try to connect the connection managet to the Socket Server */
   if(m_connMgr.Start() != KErrNone) {
      /* call the connectDone() with error */
      SHOWERR(KMsgConnectError);
   }
#endif


   TBuf<256> url;
   char2TDes(url, host);
   SHOWBUSY(KMsgBusyDNS);
   /* check if the specified address needs to be resolved to an IP */
   if(m_address.Input(url) != KErrNone) {
      /* the IP needs to be resolved */
#ifdef USE_RCONNECTION
      if(m_nameResolver.Open(m_socketServer, 
                             KAfInet,
                             KProtocolInetUdp,
                             m_connMgr) != KErrNone) {
#else
      if(m_nameResolver.Open(m_socketServer, 
                             KAfInet,
                             KProtocolInetUdp) != KErrNone) {
#endif
         /* couldn't start DNS service, return with error */
         CANCELBUSY();
         ccon->connectDone(TCPClientConnection::ERROR);
         SHOWERR(KMsgDNSError);
         return;
      }

      /* get the IP of the host synchronously */
      TNameEntry urlIp;
      TNameRecord urlIpRec;
      TRequestStatus resolveStatus;
      m_nameResolver.GetByName(url, urlIp, resolveStatus);
      User::WaitForRequest( resolveStatus );
      if (resolveStatus == KErrNone) {
      //if(m_nameResolver.GetByName(url, urlIp) == KErrNone) {
         /* get the IP in m_address */
         urlIpRec = urlIp();
         m_address = TInetAddr::Cast(urlIpRec.iAddr);
      }
      else {
         /* name could not be resolved! return with error */
         CANCELBUSY();
         m_nameResolver.Close();
         ccon->connectDone(TCPClientConnection::ERROR);
         SHOWERR(KMsgDNSNameError);
         return;
      }
   }

   /* all went well, IP is in m_address .. close DNS service */
   CANCELBUSY();
   m_nameResolver.Close();
   SHOWMSG(KMsgNameResolved);

   SHOWBUSY(KMsgBusyConn);
   /* try to connect to the host asynchronously */
#ifdef USE_RCONNECTION
   if(m_socket.Open(m_socketServer, KAfInet, 
                  KSockStream, KProtocolInetTcp,
                  m_connMgr) != KErrNone) {
#else
   if(m_socket.Open(m_socketServer, KAfInet, 
                  KSockStream, KProtocolInetTcp) != KErrNone) {
#endif
         /* socket could not be opened */
         CANCELBUSY();
#ifdef USE_RCONNECTION
         //m_connMgr.Close();
#endif
         ccon->connectDone(TCPClientConnection::ERROR);
         SHOWERR(KMsgConnectError);
         return;
   }
   
   /* set the address of the host and try to connect */
   m_hostPort = port;
   m_address.SetPort(m_hostPort);

   /* do the asynchronous connect and set status to connecting.. */
#if 0
   TRequestStatus connectStatus;
   m_socket.Connect(address, connectStatus);
   User::WaitForRequest(connectStatus);
   if (connectStatus == KErrNone) {
      m_status = EConnected;
      ccon->connectDone(TCPClientConnection::OK);
      SHOWMSG(KMsgSocketConnected);
   } else {
      m_hostPort = 0;
      m_socket.Close();
      m_status = ENotConnected;
      ccon->connectDone(TCPClientConnection::ERROR);
      SHOWERR(KMsgConnectError);
   }
#else
   m_socket.Connect(m_address, iStatus);
   m_callback = ccon;
   m_status = EConnecting;

   /* activate the object for the scheduler */
   SetActive();
#endif
   
   return;
}

void SymbianTCPConnectionHandler::disconnect(TCPClientConnection* ccon) {

   resetStatus();

   /* inform callback of success in closing */
   ccon->connectionClosed(TCPClientConnection::CLOSED);

   SHOWMSG(KMsgSocketDisconnected);

   return;
}

void SymbianTCPConnectionHandler::write(const byte* bytes, int length,
                           TCPClientConnection* ccon) {

   /* if not connected to server, return quietly */
   if(m_status != EConnected) {
      ccon->connectionClosed(TCPClientConnection::CLOSED);
      return;
   }

   /* issue the write request to the socket using the writer */
   /* it will call the appropriate callback */
   m_socketWriter.Write(ccon, bytes, length);

   return;
}

void SymbianTCPConnectionHandler::read(int maxLength,
                           TCPClientConnection* ccon) {

   /* if not connected to server, return quietly */
   if(m_status != EConnected) {
      ccon->connectionClosed(TCPClientConnection::CLOSED);
      return;
   }

   /* issue the read request to the socket using the reader */
   /* it will call the appropriate callback */
   m_socketReader.Read(ccon, maxLength);

   return;
}

void SymbianTCPConnectionHandler::remove(TCPClientConnection* conn) {

   /* just call disconnect for the connection */
   disconnect(conn);
   return;
}

int SymbianTCPConnectionHandler::requestTimer(uint32 /*nbrMillis*/, 
               TCPClientConnection* /*conn*/) {
   return(0);
}

void SymbianTCPConnectionHandler::cancelTimer(int /*timerID*/) {
   return;
}

/**
 * methods from CActive
 */
void SymbianTCPConnectionHandler::RunL() {

   switch(m_status)
   {
      case(EConnecting):
      {
         CANCELBUSY();
         if(iStatus == KErrNone) {
            /* socket is connected */
            m_status = EConnected;
            if ( m_callback ) {
               m_callback->connectDone(TCPClientConnection::OK);
            }
            //m_callback = NULL; // Why?
            SHOWMSG(KMsgSocketConnected);
         }
         else {
            /* error; socket cannot be connected */
            /* clear all info, close all socket stuff */
            m_hostPort = 0;
            m_socket.Close();
#ifdef USE_RCONNECTION
            //m_connMgr.Close();
#endif
            m_status = ENotConnected;
            if ( m_callback ) {
               m_callback->connectDone(TCPClientConnection::ERROR);
            }
            SHOWERR(KMsgConnectError);
         }
         break;
      }
      default: break;
   }
   return;
}

void SymbianTCPConnectionHandler::DoCancel() {

   resetStatus();

   return;
}
