/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include <eikgted.h>
#include "SocketConstants.h"
#include "SocketsEngine.h"
#include "TimeOutTimer.h"
#include "SocketsRead.h"
#include "SocketsWrite.h"
#include "sockets.pan"
#include "SymbianTcpConnection.h"

#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3)
# include <commdbconnpref.h>
# if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3)
#  include <apengineconsts.h>
# endif
#endif

static const TInt KNav2Port1 = 11112;
_LIT( KDefaultNav2Name, "127.0.0.1" );

static const TInt KTimeOut = 30000000; // 30 seconds time-out

class CNewSocketsEngine* 
CNewSocketsEngine::NewL(class SymbianTcpConnection& aConsole, 
                        class RSocketServ* aSockServ )
{
   class CNewSocketsEngine* self = 
      CNewSocketsEngine::NewLC(aConsole, aSockServ );
   CleanupStack::Pop();
   return self;
}

class CNewSocketsEngine* 
CNewSocketsEngine::NewLC(class  SymbianTcpConnection& aConsole, 
                         class RSocketServ* aSockServ )
{
   class CNewSocketsEngine* self = 
      new (ELeave) CNewSocketsEngine( aConsole, aSockServ );
   CleanupStack::PushL( self );
   self->ConstructL();
   return self;
}

CNewSocketsEngine::CNewSocketsEngine(class SymbianTcpConnection& aConsole, 
                                     class RSocketServ* aSockServ ) :
   CActive( EPriorityStandard ),
   iConsole( aConsole ),
   iSocketServ( aSockServ ),
   iPort( KNav2Port1 ),
   iServerName( KDefaultNav2Name )
{
}

CNewSocketsEngine::~CNewSocketsEngine()
{
   Cancel();

   delete iSocketsRead;
   iSocketsRead = NULL;

   delete iSocketsWrite;
   iSocketsWrite = NULL;

   delete iTimer;
   iTimer = NULL;
   
   iListenSocket.Close();
   iSocket.Close();
#ifndef __WINS__
# if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3)
   iConnMgr.Close();
# elif defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
   iNetAgent.Close();
# endif
#endif
}

void CNewSocketsEngine::ConstructL()
{
   ChangeStatus( SymbianTcpConnection::ENotConnected );

   // Start a timer
   //	iTimeOut = KTimeOut; 
   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);
   CActiveScheduler::Add(this); 

   // Create socket read and write active objects
   iSocketsRead = CNewSocketsRead::NewL(iConsole, iSocket );
   iSocketsWrite = CNewSocketsWrite::NewL(iConsole, iSocket);
   iCancel = EFalse;
#ifndef __WINS__
# if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3)
   // FIXME - check for errors
   //iConnMgr = new (ELeave) RConnection();
   iConnMgr.Open(*iSocketServ);
# elif defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ) 
   // FIXME - check for errors
   //iNetAgent = new (ELeave) RGenericAgent();
   iNetAgent.Open();
# endif
#endif
}

void CNewSocketsEngine::ConnectL()
{
#ifndef __WINS__
   // Set up IAP information
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3)
   TInt connectResult;
   class TCommDbConnPref prefs;

   if ( iIAP >= 0 ) {
      prefs.SetIapId( iIAP );
      prefs.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );
      prefs.SetDirection( ECommDbConnectionDirectionOutgoing );

# if defined( __WINS__ )
      connectResult = iConnMgr.Start();
# else
      connectResult = iConnMgr.Start( prefs );
# endif
   } else if ( iIAP == -2 ) { // use System default
      connectResult = iConnMgr.Start();
   } else { // use Always ask
      prefs.SetDialogPreference( ECommDbDialogPrefPrompt );
      prefs.SetDirection( ECommDbConnectionDirectionOutgoing );
# ifdef NAV2_CLIENT_SERIES60_V2
      prefs.SetBearerSet( EApBearerTypeGPRS | EApBearerTypeHSCSD );
# else 
      prefs.SetBearerSet( ECommDbBearerCSD | ECommDbBearerWcdma |
                          ECommDbBearerCdma2000 | ECommDbBearerPSD );
# endif
      connectResult = iConnMgr.Start( prefs );
   }
   FinalizeConnectionL();
#elif defined(NAV2_CLIENT_SERIES60_V1)
   // S60v1 seems to have problems with using NetAgent that way.
   FinalizeConnectionL();
#elif defined(NAV2_CLIENT_UIQ)
   TBool isUp = EFalse;
   iNetAgent.NetworkActive( isUp );
   if ( !isUp ) { // attempt to open a connection using the selected IAP
# if defined(NAV2_CLIENT_UIQ)
      iCommsOverrides = CStoreableOverrideSettings::NewL( CStoreableOverrideSettings::EParamListPartial );
# else
      iCommsOverrides = CStoreableOverrideSettings::NewL(
            CStoreableOverrideSettings::EParamListPartial,
            EDatabaseTypeIAP );
# endif
      CCommsDbConnectionPrefTableView::TCommDbIapConnectionPref pref;
      class TRequestStatus status;
      pref.iRanking = 1;
      pref.iDirection = ECommDbConnectionDirectionOutgoing;
      pref.iBearer.iBearerSet = ECommDbBearerUnknown;
      if ( iIAP >= 0 ) {
         pref.iDialogPref = ECommDbDialogPrefDoNotPrompt;
         pref.iBearer.iIapId = iIAP;
      } else if ( iIAP == -2 ) { // debug mode - use system default, do nothing
      } else {
         pref.iDialogPref = ECommDbDialogPrefPrompt;
         pref.iBearer.iIapId = 0;
      }
      iCommsOverrides->SetConnectionPreferenceOverride( pref );
      iNetAgent.StartOutgoing( *iCommsOverrides, status );
      ChangeStatus( SymbianTcpConnection::EStartingConnection );
      
      // Request time out
      iTimer->After( KTimeOut );
      SetActive(); // calls FinalizeConnectionL from RunL
   } else {
      FinalizeConnectionL();
   }
#endif
#else
   FinalizeConnectionL();
#endif
}

void CNewSocketsEngine::FinalizeConnectionL()
{
   // Initiate connection process
   if( (iEngineStatus == SymbianTcpConnection::ENotConnected) || 
       (iEngineStatus == SymbianTcpConnection::ETimedOut) ||
       (iEngineStatus == SymbianTcpConnection::EStartingConnection) ){
      TInetAddr addr;
      if( addr.Input( iServerName ) == KErrNone ){
         // server name is already a valid ip address
         ConnectL( addr.Address() );
      } else { // need to look up name using dns
         // Initiate DNS
#if !defined __WINS__ && (defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3))
         User::LeaveIfError( iResolver.Open(*iSocketServ, KAfInet, KProtocolInetUdp, iConnMgr) );
#else
         User::LeaveIfError( iResolver.Open(*iSocketServ, KAfInet, KProtocolInetUdp) );
#endif

         // DNS request for name resolution
         iResolver.GetByName( iServerName, iNameEntry, iStatus );
         ChangeStatus( SymbianTcpConnection::ELookingUp );

         // Request time out
         iTimer->After( KTimeOut );
         SetActive();
      }
   }
}

void CNewSocketsEngine::ConnectL( TUint32 aAddr )
{
   // Initiate attempt to connect to a socket by IP address	
   if( (iEngineStatus == SymbianTcpConnection::ENotConnected) ||
       (iEngineStatus == SymbianTcpConnection::ETimedOut) ||
       (iEngineStatus == SymbianTcpConnection::EStartingConnection) ){
      
      // Initiate socket connection
      // Open a TCP socket
      iSocket.Close(); // Just in case
#if !defined __WINS__  && (defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3))
      User::LeaveIfError( iSocket.Open(*iSocketServ, KAfInet, KSockStream, KProtocolInetTcp, iConnMgr) );
#else
      User::LeaveIfError( iSocket.Open(*iSocketServ, KAfInet, KSockStream, KProtocolInetTcp) );
#endif

      // Set up address information
      iAddress.SetPort( iPort );
      iAddress.SetAddress( aAddr );
     
      // Connect the socket
      iSocket.Connect( iAddress, iStatus );
      ChangeStatus( SymbianTcpConnection::EConnecting );
      SetActive();

      // Start a timeout
      iTimer->After( KTimeOut );
   }
}

// XXX This function should become deprecated
void CNewSocketsEngine::Listen()
{
   // Listen on a socket	
   // Open a TCP socket
   iListenSocket.Close();
   User::LeaveIfError( iListenSocket.Open( *iSocketServ, KAfInet, KSockStream, KProtocolInetTcp ) );

   iSocket.Close(); // Just in case
   iSocket.Open( *iSocketServ ); // Open as a blank socket

   iListenSocket.SetLocalPort( iPort );

   // Initiate socket connection
   iListenSocket.Listen( 1 );

   iListenSocket.Accept( iSocket, iStatus );

   ChangeStatus( SymbianTcpConnection::EConnecting );
   SetActive();

   // Start a timeout
   iTimer->After(KTimeOut);
}

void CNewSocketsEngine::Disconnect()
{
   iTimer->Cancel();
   iSocketsRead->Cancel();
   iSocketsWrite->Cancel();
   if (IsConnected()) {
      iSocket.Shutdown(RSocket::ENormal, iStatus);
   }

   ChangeStatus( SymbianTcpConnection::EDisconnecting );
   if(!IsActive()) {
      SetActive();
   } 
   iTimer->After(KTimeOut);      
}

// from CActive
void CNewSocketsEngine::DoCancel()
{
   iTimer->Cancel();

   // Cancel appropriate request to socket
   switch (iEngineStatus)
   {
#if defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
   case SymbianTcpConnection::EStartingConnection:
      delete iCommsOverrides;
      iCommsOverrides = NULL;
      break;
#endif      
   case SymbianTcpConnection::EConnecting:
      iSocket.CancelConnect();
      break;
   case SymbianTcpConnection::ELookingUp:
      // Cancel look up attempt
      iResolver.Cancel();
      iResolver.Close();
      break;
   case SymbianTcpConnection::EConnected:
      iSocketsRead->Cancel();
      iSocketsWrite->Cancel();
      iSocket.Close();
      break;
   case SymbianTcpConnection::EDisconnecting:
      iSocket.Close();
      break;
   default:
      User::Panic(KPanicSocketsEngine, ESocketsBadStatus);
      break;
   }
   ChangeStatus( SymbianTcpConnection::ENotConnected );
}

void CNewSocketsEngine::WriteL( TDesC8 &data, int length )
{
   HBufC8* message = HBufC8::NewLC(KMaxMessageLength);
   // Write data to socket
   if( iEngineStatus == SymbianTcpConnection::EConnected ){
      TInt currLength = length;
      while( currLength > 0 ){
         TInt sendLength = currLength;
         if( sendLength > KMaxMessageLength )
            sendLength = KMaxMessageLength;
         message->Des().Copy( data.Ptr(), sendLength );
         iSocketsWrite->IssueWriteL( *message );
         currLength -= sendLength;
      }
   }
   CleanupStack::PopAndDestroy(message);
}

void CNewSocketsEngine::Read()
{
   // Initiate read of data from socket
   if( (iEngineStatus == SymbianTcpConnection::EConnected) &&
       !iSocketsRead->IsActive() ) {
      iSocketsRead->Start();
   }
}

// from CActive
void CNewSocketsEngine::RunL()
{
   // Active object request complete handler.
   // iEngineStatus flags what request was made, so its
   // completion can be handled appropriately
   iTimer->Cancel(); // Cancel TimeOut timer before completion

   //TBuf<64> error;

   switch(iEngineStatus)
   {
#if defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
   case SymbianTcpConnection::EStartingConnection:
      if (iStatus == KErrNone){
         delete iCommsOverrides;
         iCommsOverrides = NULL;
         FinalizeConnectionL();
      } else {
         ChangeStatus( SymbianTcpConnection::EConnectFailed );
         iConsole.ErrorNotify( socket_errors_connectionfailed_msg, iStatus.Int() );
         ChangeStatus( SymbianTcpConnection::ENotConnected );
      }
      break;
#endif
   case SymbianTcpConnection::EConnecting:
      // IP connection request
      if (iStatus == KErrNone){ // Connection completed successfully
         ChangeStatus( SymbianTcpConnection::EConnected );
         Read(); //Start CNewSocketsRead Active object
      } else {
         ChangeStatus( SymbianTcpConnection::EConnectFailed );
         //error.Copy(_L("Connection failed, check network"));
         iConsole.ErrorNotify( socket_errors_connectionfailed_msg, iStatus.Int() );
         ChangeStatus( SymbianTcpConnection::ENotConnected );
      }
      break;
   case SymbianTcpConnection::ELookingUp:
      iResolver.Close();
      if( iStatus == KErrNone ){
         // DNS look up successful
         iNameRecord = iNameEntry();
         // Extract domain name and IP address from name record
         TBuf<15> ipAddr;
         TInetAddr::Cast(iNameRecord.iAddr).Output(ipAddr);
         // And connect to the IP address
         ChangeStatus( SymbianTcpConnection::ENotConnected );
         ConnectL(TInetAddr::Cast(iNameRecord.iAddr).Address());
      }
      else{ // DNS lookup failed
         ChangeStatus( SymbianTcpConnection::ELookUpFailed );
         //error.Copy(_L("Connection failed, check network"));
         iConsole.ErrorNotify( socket_errors_connectionfailed_msg, iStatus.Int() );
         ChangeStatus( SymbianTcpConnection::ENotConnected );
      }
      break;
   case SymbianTcpConnection::EDisconnecting:
      if (iStatus == KErrNone){
         iSocket.Close();
         ChangeStatus( SymbianTcpConnection::ENotConnected );
         if(iCancel) {
            iConsole.ConnectionCancelled();
         }
      }
      else{
         ChangeStatus( SymbianTcpConnection::EConnectFailed );
      }
      break;
   default:
      User::Panic(KPanicSocketsEngine, ESocketsBadStatus);
      break;
   };
}

void CNewSocketsEngine::TimerExpired()
{
   Cancel();
   ChangeStatus(SymbianTcpConnection::ETimedOut);
   //TBuf<64> error(_L("Connection failed, check network"));
   iConsole.ErrorNotify( socket_errors_connectionfailed_msg, KErrTimedOut);
   Disconnect();
}

void CNewSocketsEngine::ChangeStatus( SymbianTcpConnection::TSocketsEngineState aNewStatus )
{
   // Update the status (and the status display)
   iEngineStatus = aNewStatus;
   switch (aNewStatus)
   {
   case SymbianTcpConnection::ENotConnected:
   case SymbianTcpConnection::EStartingConnection:
   case SymbianTcpConnection::EConnecting:
   case SymbianTcpConnection::EConnected:
   case SymbianTcpConnection::ETimedOut:
   case SymbianTcpConnection::ELookingUp:
   case SymbianTcpConnection::ELookUpFailed:
   case SymbianTcpConnection::EConnectFailed:
   case SymbianTcpConnection::EDisconnecting:
      iConsole.SetStatus( aNewStatus );
      break;
   default:
      User::Panic(KPanicSocketsEngine, ESocketsBadStatus);
      break;
   }
}

void CNewSocketsEngine::SetServerName(const TDesC& aName)
{
   iServerName.Copy(aName);
}

const TDesC& CNewSocketsEngine::ServerName() const
{
   return iServerName;
}

void CNewSocketsEngine::SetPort(TInt aPort)
{
   iPort = aPort;
}

TInt CNewSocketsEngine::Port() const
{
   return iPort;
}

void CNewSocketsEngine::SetIAP(TInt aIAP)
{
   iIAP = aIAP;
}

TInt CNewSocketsEngine::GetIAP() const
{
   return iIAP;
}

TBool CNewSocketsEngine::IsConnected() const
{
   return (iEngineStatus == SymbianTcpConnection::EConnected);
}

void CNewSocketsEngine::SocketsEngineCancel()
{
   iCancel = ETrue;
   Disconnect();
}
