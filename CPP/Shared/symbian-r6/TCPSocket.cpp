/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

/*
 * Symbian TCP Socket implementation.
 */


#include "arch.h"
#include "Sockets.h"

#include <string.h>

#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
# include <commdbconnpref.h>
# if defined(NAV2_CLIENT_SERIES60_V2)
#  include <apengineconsts.h>
# endif
#elif NAV2_CLIENT_SERIES60_V1
#else
//# include <nifman.h>
#endif


#include "TCPSocket.h"
#include "GlobalData.h"

#define MAX_READ_START 128

namespace isab {
   
   TCPSocket::TCPSocket(int backlog) : 
         m_error(0), m_backlog(backlog), m_blocking(true), 
         m_currentState(UNKNOWN),
         m_maxRead(MAX_READ_START), 
         m_readStatus(KErrNone),
         m_readLength(0),
         m_session(getArchGlobalData().m_socketServ)
   {
      TInt err;
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
      // FIXME - check for errors
      m_connMgr = new RConnection();
      err = m_connMgr->Open(*m_session);
#elif defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
      // FIXME - check for errors
      m_netAgent = new RGenericAgent();
      err = m_netAgent->Open();
#endif
      if( err != KErrNone ){
      }
   }
   
   TCPSocket::TCPSocket(SOCKET sock, int backlog, TCPSocketState state) :
         m_error(0), m_socket( sock ), m_backlog(backlog),
         m_blocking(true), m_currentState(state),
         m_maxRead(MAX_READ_START),
         m_session(getArchGlobalData().m_socketServ)
   {
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
      // FIXME - check for errors
      m_connMgr = new RConnection();
      m_connMgr->Open(*m_session);
#elif defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
      // FIXME - check for errors
      m_netAgent = new RGenericAgent();
      m_netAgent->Open();
#endif

      /* init() sets m_currentState to UNKNOWN, so we need to set it to */
      /* the correct value afterwards. */
      setupSocket();
   }

   int
   TCPSocket::setupSocket()
   {
      /* Make sure we have may setup. */
      if ( m_currentState != OPEN && m_currentState != CONNECTED ) {
         return -1;
      }

      m_blocking = true;
      /* The socket should send data as soon as it is available. */
      m_socket.SetOpt( KSoTcpNoDelay, KSolInetTcp );

      /* Try to advice the OS that we want to reuse the port */
      /* as soon as possible (default behaviour in Linux */
      /* (but not in Solaris)) */
//      m_socket.SetOpt( KSoReuseAddr, KSolInetIp );

      /* Make the OS send keepalive packets when no data (for about 2 hours) */
      /* has been received or sent. The keepalive will result in one of */
      /* three actions: */
      /* 1. An ACK is received, and the keepalive timer is reset. */
      /* 2. A RST is received, indicating that the connection is broken. */
      /* 3. Nothing is received and the OS retries the keepalive for an */
      /*    additional 9 times 75 seconds apart. If no answer is gotten */
      /*    the connection is broken. */
      // m_socket.SetOpt( KSoTcpKeepAlive, KSolInetTcp );

      return 1;
   }

   bool
   TCPSocket::getHostAddr( TInetAddr& addr, const char *hostname )
   {
      if ( m_currentState == UNKNOWN || m_currentState == CLOSED ) {
         return false;
      }
      bool ok = true;
      class RHostResolver resolver;
      TNameEntry hostEntry;

#if defined(_UNICODE)
      uint16* uName = new uint16[ strlen( hostname ) + 1 ];
      const char* pos = hostname;
      TInt i = 0;
      while ( *pos != '\0' ) {
         uName[ i ] = *pos;
         i++;
         pos++;
      }
      uName[ i ] = 0;
      TPtrC16 Tname ( uName, strlen( hostname ) );
#else 
      TPtrC8 Tname( (uint8*)hostname, strlen( hostname ) );
#endif

#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
      resolver.Open( *m_session, KAfInet, KProtocolInetTcp, *m_connMgr );
#else
      resolver.Open( *m_session, KAfInet, KProtocolInetTcp );
#endif
      TInt result;
#if defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_UIQ3)
      result = resolver.GetByName( Tname, hostEntry );
#else
      class TRequestStatus resolveStatus;
      resolver.GetByName( Tname, hostEntry, resolveStatus );
      User::WaitForRequest( resolveStatus );
      result = resolveStatus.Int();
#endif
      if ( result == KErrNone ) {
         TNameRecord hostRecord;
         hostRecord = hostEntry();
         TBuf<15> ipAddr;
         TInetAddr::Cast( hostRecord.iAddr ).Output(ipAddr);
         addr = TInetAddr::Cast( hostRecord.iAddr );
//         addr.SetAddress( KInetAddrLoop/*INET_ADDR( 195,84,119,66 )*/ );
      } else {
         ok = false;
      }

#if defined(_UNICODE)
   delete [] uName;
#endif
      resolver.Close();
      return ok;
   }

   bool
   TCPSocket::create()
   {
      if ( m_currentState == UNKNOWN || m_currentState == CLOSED ) {
         int res;
         
         /* Create a new TCP socket. */
/*          m_socket.Close(); */
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
         res = m_socket.Open( *m_session, KAfInet, KSockStream, 
                              KProtocolInetTcp, *m_connMgr );
#else
         res = m_socket.Open( *m_session, KAfInet, KSockStream, 
                              KProtocolInetTcp );
#endif

         if ( res != KErrNone ) {
            return false;
         }

         m_currentState = OPEN;
         if ( setupSocket() < 0 ) {
            /* Failed setup. */
            m_currentState = UNKNOWN;
            m_socket.Close();
   //         m_session.Close();
            return false;
         }
      }
      return true;
   }

   TCPSocket::~TCPSocket()
   {
      close();
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
      m_connMgr->Close();
      delete m_connMgr;
#elif defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
      m_netAgent->Close();
      delete m_netAgent;
#endif
   }

   void
   TCPSocket::ignorePipe() {
      // No need, no disturbing signals in symbian
   }  

   void
   TCPSocket::setPipeDefault() {
      // No need, no disturbing signals in symbian
   }

   int
   TCPSocket::close()
   {
      m_socket.Close();

      m_currentState = CLOSED;

      return 1;
   }

   int
   TCPSocket::bind(const char *hostname, uint16 portnumber)
   {
      TInetAddr addr;
      int res = 0;

      if ( hostname == NULL ) {
         addr.SetAddress( KInetAddrAny );
      } else {
         if ( !getHostAddr( addr, hostname ) ) {
            res = -2;
         }
      }

      if ( res == 0 ) { // Ok so far
         addr.SetPort( portnumber );
         TInt result = m_socket.Bind( addr );
         if ( result == KErrNone ) {
            m_currentState = TCPSocket::BOUND;
            res = 1;
         } else {
            res = -2;
         }
      }

      return res;
   }

   uint16
   TCPSocket::listen(uint16 portnumber, TCPPortChoice pc)
   {
      bool done = false;
      int nbrTries;

      if (!create()) {
         return 0;
      }

      if ( pc == FINDFREEPORT ) {
         nbrTries = MAX_NBR_TRIES_PORTFIND;
      } else {
         nbrTries = 1;
      }

      if ( m_currentState != TCPSocket::BOUND ) {
         // Bind to port
         while ( nbrTries > 0 && !done ) {
            switch ( pc ) {
               case GENERIC:
                  /* Bind to all interfaces and this portnumber. */
                  if ( bind( NULL, portnumber ) < 0 ) {
                     /* Can't bind the port. */
                     return 0;
                  }
                  done = true;
                  break;
               case DONTCARE:
                  /* Get an ephemeral port. */
                  if ( bind( NULL, 0 ) < 0 ) { // KInetPortAny == 0
                     /* Can't bind the port. */
                     /* This is actually a more serious error, */
                     /* since we asked for _any_ port. */
                     return 0;
                  }
                  done = true;
                  break;
               case FINDFREEPORT:
                  /* Get the first free port over the port specified. */
                  if ( bind( NULL, portnumber ) < 0 ) {
                     /* Failed, try the next higher portnumber. */
                     portnumber++;
                     nbrTries--;
                     // Port already in use, trying the next one
                  } else {
                     done = true;
                  }
                  break;
            }
         }
      }

      /* We only decrement nbrTries when we're doing a FINDFREEPORT */
      if ( nbrTries == 0 ) {
         // No free port found!
         return 0;
      }

      /* Now listen on the socket. */
      if ( m_socket.Listen( m_backlog ) != KErrNone ) {
         /* Failed system call. */
         return 0;
      }

      m_currentState = LISTEN;

      return portnumber;
   }

   TCPSocket*
   TCPSocket::accept()
   {
      TCPSocket* tmpsock = new TCPSocket( DEFAULT_BACKLOG );
      
      if ( !tmpsock->createBlank( ) ) { // Create blank socket
         // Error
         delete tmpsock;
         return NULL;
      }

      TRequestStatus acceptStatus;
      m_socket.Accept( tmpsock->m_socket, acceptStatus );
      
      User::WaitForRequest( acceptStatus ); // Wait until done

      if ( acceptStatus == KErrNone ) {
         // Ok have socket!
         m_socket.CancelAccept();
         tmpsock->m_currentState = CONNECTED;
         setupSocket();
         return tmpsock;
      } else {
         delete tmpsock;
         return NULL;
      }
   }

   TCPSocket*
   TCPSocket::accept(uint32 micros)
   {
      RTimer timer;
      if ( timer.CreateLocal() != KErrNone ) {
         // Error
         return NULL;
      }
      
      TCPSocket* tmpsock = new TCPSocket( DEFAULT_BACKLOG );
      
      if ( !tmpsock->createBlank( ) ) { // Create blank socket
         // Error
         delete tmpsock;
         return NULL;
      }

      TRequestStatus timerStatus;
      TTimeIntervalMicroSeconds32 interval = micros;
      TRequestStatus acceptStatus;

      timer.After( timerStatus, interval );
      m_socket.Accept( tmpsock->m_socket, acceptStatus );
      
      User::WaitForRequest( acceptStatus, timerStatus );

      if ( timerStatus == KRequestPending ) {
         // acceptStatus returned
         timer.Cancel(); // Cancels any outstanding request for a timer event
         User::WaitForRequest(timerStatus);
         timer.Close(); // Release timer
         if ( acceptStatus == KErrNone ) { // Accepted ok
            m_socket.CancelAccept();
            tmpsock->m_currentState = CONNECTED;
            setupSocket();
            return tmpsock;
         } else {
            delete tmpsock;
            return NULL;
         }
      } else {
         // Timeout
         m_socket.CancelAccept();
         User::WaitForRequest(acceptStatus);
         delete tmpsock;
         timer.Close(); // Release timer
         return NULL;
      }
   }

   void TCPSocket::cancelAll()
   {
      if(m_currentState != UNKNOWN){
         m_socket.CancelAll();
      }
   }


   int TCPSocket::setupConnection( int32 connParam )
   {
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
      // Series60 release 2 (eg. Nokia 6600)
      //
      // Attempt to open a connection using the selected IAP
      TInt connectResult;
      TCommDbConnPref prefs;
      if (connParam >= 0) {
         prefs.SetIapId(connParam);
         prefs.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
#if defined __WINS__
         connectResult = m_connMgr->Start();
#else
         connectResult = m_connMgr->Start(prefs);
#endif
      } else if (connParam == -2) {
         // Debug mode - use system default
         connectResult = m_connMgr->Start();
      } else {
# undef ALWAYS_ASK_IS_REALLY_SYSTEM_DEFAULT
# ifndef ALWAYS_ASK_IS_REALLY_SYSTEM_DEFAULT
         // Use Always ask
         prefs.SetDialogPreference(ECommDbDialogPrefPrompt);
         prefs.SetDirection(ECommDbConnectionDirectionOutgoing);
#  ifdef NAV2_CLIENT_SERIES60_V2
         prefs.SetBearerSet( EApBearerTypeGPRS | EApBearerTypeHSCSD );
#  else //s80, s90
         prefs.SetBearerSet( ECommDbBearerCSD | ECommDbBearerWcdma | 
                             ECommDbBearerCdma2000 | ECommDbBearerPSD );
                            
#  endif
         connectResult = m_connMgr->Start(prefs);
# else
         // Use system default
         connectResult = m_connMgr->Start();
# endif
      }
      if (connectResult != KErrNone) {
         return Carrier;
      }
      return None;   // FIXME - XXX - add error handling (see nifvar.h and LastProgressError() )
#elif defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
      // Series60 release 1 (eg. Nokia 7650/3650)
      //
      TBool isUp=EFalse;
      m_netAgent->NetworkActive(isUp);
      if (!isUp) {
         // Attempt to open a connection using the selected IAP
# if defined(NAV2_CLIENT_UIQ)
         CStoreableOverrideSettings *CommsOverrides = CStoreableOverrideSettings::NewL(
                   CStoreableOverrideSettings::EParamListPartial );
# else
         CStoreableOverrideSettings *CommsOverrides = CStoreableOverrideSettings::NewL(
                   CStoreableOverrideSettings::EParamListPartial,
                   EDatabaseTypeIAP);
# endif

         CCommsDbConnectionPrefTableView::TCommDbIapConnectionPref pref;
         TRequestStatus status;
         pref.iRanking = 1;
         pref.iDirection = ECommDbConnectionDirectionOutgoing;
         pref.iBearer.iBearerSet = ECommDbBearerUnknown;
         if (connParam >= 0) {
            pref.iDialogPref = ECommDbDialogPrefDoNotPrompt;
            pref.iBearer.iIapId = connParam;
         } else if (connParam == -2) {
            // Debug mode - use system default (do not start a connection)
            return None;
         } else {
# ifndef ALWAYS_ASK_IS_REALLY_SYSTEM_DEFAULT
            // Use Always ask
            pref.iDialogPref = ECommDbDialogPrefPrompt;
            pref.iBearer.iIapId = 0;
# else
            // Use system default
            // Not sure if this is right...
            return None;
# endif
         }
#ifdef __WINS__
         /* Always use system default. */
         return None;
#endif
         CommsOverrides->SetConnectionPreferenceOverride(pref);
         m_netAgent->StartOutgoing(*CommsOverrides, status);
         User::WaitForRequest(status);
         delete CommsOverrides;
         if (status != KErrNone) {
            return Carrier;
         }
      }
      return None;
#else
      return None;
#endif
   }

   int
   TCPSocket::doConnect( TInetAddr& addr )
   {
      TRequestStatus connectStatus = KRequestPending;
      TRequestStatus timerStatus = KRequestPending;
      RTimer timer;
      if(timer.CreateLocal() != KErrNone ) {
         return Unable;
      }

      TTimeIntervalMicroSeconds32 interval;
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
      interval = 10*1000*1000;
#elif defined (NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
      interval = 10*1000*1000;
#else
      //Nifty code to check if GPRS (or other channel) is already up.
      // timeout is set accordingly.
      RNif nif;
      nif.Open();
      TBool open = EFalse;
      if(KErrNone == nif.NetworkActive(open) && open){
         interval = 30*1000*1000;
      } else {
         interval = 60*1000*1000;
      }
      nif.Close();
#endif

      timer.After( timerStatus, interval );
      m_socket.Connect( addr, connectStatus );
      User::WaitForRequest( connectStatus, timerStatus );

      // How does nonblocking IO do?
      if(timerStatus == KRequestPending){
         timer.Cancel(); // cancel request
         User::WaitForRequest(timerStatus);
         timer.Close();
         if ( connectStatus == KErrNone ) {
            m_currentState = CONNECTED;
            setupSocket(); // Didn't this get called in create?
            return None;
         } else if(connectStatus == KErrTimedOut){
            return Timeout;
         } else {
            return Refused;
         }
      } else {
         m_socket.CancelConnect();
         User::WaitForRequest(connectStatus);
         timer.Close();
         return Timeout;
      }
   }


   bool TCPSocket::createBlank( ) {
      if ( m_currentState == UNKNOWN || m_currentState == CLOSED ) {
//         m_session.Duplicate( Thread::currentThread()->m_innerThread->m_threadHandle );

         /* Create a new blank TCP socket. */
//         m_socket.Close();
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
         TInt res = m_socket.Open( *m_session, KAfInet, KSockStream,
                                             KProtocolInetTcp, *m_connMgr);
#else
         TInt res = m_socket.Open( *m_session);
#endif
         
         if ( res != KErrNone ) {
//            m_session.Close();
            return false;
         }
      } else {
         return false;
      }

      m_currentState = OPEN;
      return true;
   }

   int
   TCPSocket::connect(const char *hostname, int portnumber)
   {
      TInetAddr addr;
      addr.SetPort( portnumber );

      if (!create()) {
         return Unable;
      }

      if ( !getHostAddr( addr, hostname ) ) {
         return NoLookup;
      }

      addr.SetPort( portnumber );

      int ret =  doConnect(addr);
      if(ret == None){
         return 1;
      } else {
         return ret;
      }

      return 1;
   }

   int
   TCPSocket::connect(uint32 hostip, uint16 portnumber)
   {
      TInetAddr addr;

      addr.SetPort( portnumber );
      addr.SetAddress( hostip );

      if (!create()) {
         return Unable;
      }

      int ret =  doConnect(addr);
      if(ret == None){
         return 1;
      } else {
         return ret;
      }

      return 1; 
   }

   ssize_t
   TCPSocket::protectedRead(uint8* buffer, ssize_t size)
   {
      TPtr8 tBuff( buffer, size );
      TRequestStatus readStatus;
      TSockXfrLength readLengh = 0;

      m_socket.RecvOneOrMore( tBuff, 0, readStatus, readLengh );
      User::WaitForRequest( readStatus );

      if ( readStatus == KErrNone ) {
         if ( tBuff.Length() <= 0 ) {
            // FIXME: Should the function really return -1 when EOF?
            return -1;
         } else {
            return tBuff.Length();
         }
      } else {
         // XXX: KErrEof?
         return -1;
      }
   }

   // Symbian specific read with timeout, new and improved
   ssize_t 
   TCPSocket::read( uint8 *buffer, ssize_t length, uint32 micros ) {
      if(m_readStatus == KErrNone && m_tBuf.Length() == 0){
         m_socket.RecvOneOrMore( m_tBuf, 0, m_readStatus, m_readLength );
      } else if(m_readStatus != KRequestPending && m_tBuf.Length() > 0){
         int retlen = MIN(length, m_tBuf.Length());
         memcpy(buffer, m_tBuf.Ptr(), retlen); 
         m_tBuf.Delete(0, retlen);
         if(m_tBuf.Length() == 0){
            m_socket.RecvOneOrMore(m_tBuf, 0, m_readStatus, m_readLength);
         }
         return retlen;
      }
      m_error = 0;
      RTimer timer;
      if ( timer.CreateLocal() != KErrNone ) {
         // Error
         m_error = 1;
         return -1;
      }
      TRequestStatus timerStatus;
      TTimeIntervalMicroSeconds32 interval = micros;
      
      timer.After( timerStatus, interval );
      User::WaitForRequest( m_readStatus, timerStatus );

      if ( timerStatus == KRequestPending ) {
         timer.Cancel(); // Cancels any outstanding request for a timer event
         User::WaitForRequest(timerStatus); // just cancelled, it must be finished immediately.
         timer.Close(); // Release timer
         if ( m_readStatus == KErrNone ) {
            if ( m_tBuf.Length() <= 0 ) {
               // FIXME: Should the function really return -1 when EOF?
               m_error = 2;
               return -1;
            } else {
               int retlen = MIN(length, m_tBuf.Length());
               memcpy(buffer, m_tBuf.Ptr(), retlen); 
               m_tBuf.Delete(0, retlen);
               if(m_tBuf.Length() == 0){
                  m_socket.RecvOneOrMore(m_tBuf, 0, 
                                         m_readStatus, m_readLength);
               }
               return retlen;
            }
         } else {
            // XXX: KErrEof?
            m_error = m_readStatus.Int();
            return -1;
         }
      } else { // Timeout
         timer.Close(); // Release timer 
         return -2;
      }
   }

   ssize_t
   TCPSocket::protectedWrite( const uint8* buffer, ssize_t size )
   {
      TPtrC8 tBuff( buffer, size );
      TRequestStatus writeStatus;

      m_socket.Write( tBuff, writeStatus );
      User::WaitForRequest( writeStatus );
      if ( writeStatus == KErrNone ) {
         // FIXME: Should the function really return -1 when EOF?
         return tBuff.Length();
      } else {
         return writeStatus.Int();
         //return -1;
      }
   }


   ssize_t
   TCPSocket::read(uint8 *buffer, ssize_t size)
   {
      if ( m_currentState != CONNECTED ) {
         return -1;
      }

      ignorePipe();
      ssize_t result = protectedRead( buffer, size );
      setPipeDefault();
      return result;
   }

   ssize_t
   TCPSocket::write( const uint8 *buffer,
                     ssize_t length  )
   {
      if ( m_currentState != CONNECTED ) {
         return -1;
      }

      // If the socket is closed by the other side during transmission, we
      // receve the SIGPIPE-signal that normaly leads to program exit.
      // But now this signal is ignord during writing --> write() returns
      // -1 if this happens.
      // No effect in MSC_VER. Nor any in Symbian
      ignorePipe();
      ssize_t writeLength = protectedWrite(buffer, length);
      
      ssize_t totalWrite = writeLength;

      while ( ( totalWrite > -1 ) && ( totalWrite < length) ) {
         writeLength = protectedWrite( (buffer + totalWrite),
                                       length-totalWrite );

         // XXX: Is this correct? What happens if we have written
         //      40 bytes and we want to write 80. Then we don't
         //      know how many we wrote.
         if (writeLength > -1)
            totalWrite += writeLength;
         else
            totalWrite = writeLength;
      }
      

      // Let the default handler take care of the SIGPIPE-signal now when
      // data written.
      // No effect in MSC. Nor any in Symbian
      setPipeDefault();

      if ( totalWrite < 0 ) {
         //ERR("TCPSocket::write RETURN -1 : %s\n", strerror(errno));
         return (-1);
      }
      return totalWrite;
   }


   SOCKET
   TCPSocket::getSOCKET() const {
      return m_socket;
   }


   bool
   TCPSocket::getPeerName(uint32& IP, uint16& port) {
      if ( m_currentState != CONNECTED ) {
         //ERR("TCPSocket::getPeerName failure: NO SOCKET!\n");
         return false;
      }

      TInetAddr peerAddr;

      m_socket.RemoteName( peerAddr );
      IP = peerAddr.Address();
      port = peerAddr.Port();
      return true;
   }


   bool
   TCPSocket::setBlocking(bool blocking)
   {
      if ( blocking ) {
         if ( m_socket.SetOpt( KSOBlockingIO, KSOLSocket ) == KErrNone ) {
            return true;
         } else {
            return false;
         }
      } else {
         if ( m_socket.SetOpt( KSONonBlockingIO, KSOLSocket ) == KErrNone )
         {
            return true;
         } else {
            return false;
         }
      }
   }

   TCPSocket::TCPSocketState
   TCPSocket::getState() const
   {
      return (m_currentState);
   }

   uint32 TCPSocket::getIP() const
   {
      return 0;
   }

} /* namespace isab */


