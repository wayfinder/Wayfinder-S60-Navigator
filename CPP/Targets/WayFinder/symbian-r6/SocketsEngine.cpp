/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <eikgted.h>
#include "SocketsEngine.h"
#include "TimeOutTimer.h"
#include "SocketsRead.h"
#include "SocketsWrite.h"
#include "sockets.pan"
//#include "UIConnection.h"
#include "RsgInclude.h"
#include "MessageHandler.h"
#include "Buffer.h"
#define ILOG_POINTER aConsole.iAppUi->iLog
#include "memlog.h"
//#define GUI_LISTEN

static const TInt KNav2Port1 = 11112;

_LIT( KDefaultNav2Name, "10.11.3.28" );

static const TInt KTimeOut = 30000000; // 30 seconds time-out

CSocketsEngine* CSocketsEngine::NewL(CMessageHandler& aConsole)
{
   CSocketsEngine* self = CSocketsEngine::NewLC(aConsole);
   CleanupStack::Pop();
   return self;
}

CSocketsEngine* CSocketsEngine::NewLC(CMessageHandler& aConsole)
{
   CSocketsEngine* self = new (ELeave) CSocketsEngine(aConsole);
   LOGNEW(self, CSocketsEngine); 
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

#undef ILOG_POINTER
#define ILOG_POINTER iConsole.iAppUi->iLog
#include "memlog.h"

CSocketsEngine::CSocketsEngine(CMessageHandler& aConsole)
: CActive(CActive::EPriorityStandard),
  iConsole(aConsole),
  iPort(KNav2Port1),
  iServerName(KDefaultNav2Name)
{
}


CSocketsEngine::~CSocketsEngine()
{
   Cancel();

   LOGDEL(iSocketsRead);
   delete iSocketsRead;
   iSocketsRead = NULL;

   LOGDEL(iSocketsWrite);
   delete iSocketsWrite;
   iSocketsWrite = NULL;

   LOGDEL(iTimer);
   delete iTimer;
   iTimer = NULL;
   
   iListenSocket.Close();

   iSocket.Close();
   iSocketServ.Close();
   /* Is the below close needed? XXX Needs checking. */
/*    iResolver.Close(); */
}


void CSocketsEngine::ConstructL()
{
   ChangeStatus(ENotConnected);

   // Start a timer
   //	iTimeOut = KTimeOut; 
   iTimer = CTimeOutTimer::NewL(CActive::EPriorityHigh, *this/*, ILOG_POINTER*/);
   CActiveScheduler::Add(this); 

   // Open channel to Socket Server
   User::LeaveIfError(iSocketServ.Connect());

   // Create socket read and write active objects
   iSocketsRead = CSocketsRead::NewL(iConsole, iSocket );
   iSocketsWrite = CSocketsWrite::NewL(iConsole, iSocket);
}

void CSocketsEngine::ConnectL()
{
   // Initiate connection process
   if( (iEngineStatus == ENotConnected) || (iEngineStatus == ETimedOut) ){
#ifdef GUI_LISTEN
      Listen();
#else
      TInetAddr addr;
      if( addr.Input(iServerName) == KErrNone ){
         // server name is already a valid ip address
         ConnectL( addr.Address() );
      }
      else{ // need to look up name using dns
         // Initiate DNS
         User::LeaveIfError(iResolver.Open(iSocketServ, KAfInet, KProtocolInetUdp));
         // DNS request for name resolution
         iResolver.GetByName(iServerName, iNameEntry, iStatus);

         ChangeStatus(ELookingUp);
         // Request time out
         iTimer->After(KTimeOut);
         SetActive();
      }
#endif
   }
}

void CSocketsEngine::ConnectL( TUint32 aAddr ) // <a name="ConnectL32">
{
   // Initiate attempt to connect to a socket by IP address	
   if( (iEngineStatus == ENotConnected) || (iEngineStatus == ETimedOut) ){
      // Open a TCP socket
      iSocket.Close(); // Just in case
      User::LeaveIfError(iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp));

      // Set up address information
      iAddress.SetPort(iPort);
      iAddress.SetAddress(aAddr);

      // Initiate socket connection
      iSocket.Connect(iAddress, iStatus);
      ChangeStatus(EConnecting);
      SetActive();

      // Start a timeout
      iTimer->After(KTimeOut);
   }
}

// XXX This function should become depricated
void CSocketsEngine::Listen()
{
   // Listen on a socket	
   // Open a TCP socket
   iListenSocket.Close();
   User::LeaveIfError( iListenSocket.Open( iSocketServ, KAfInet, KSockStream, KProtocolInetTcp ) );

   iSocket.Close(); // Just in case
   iSocket.Open( iSocketServ ); // Open as a blank socket

   iListenSocket.SetLocalPort( iPort );

   // Initiate socket connection
   iListenSocket.Listen( 1 );

   iListenSocket.Accept( iSocket, iStatus );

   ChangeStatus(EConnecting);
   SetActive();

   // Start a timeout
   iTimer->After(KTimeOut);
}

void CSocketsEngine::Disconnect()
{
   iTimer->Cancel();
   iSocketsRead->Cancel();
   iSocketsWrite->Cancel();
   iSocket.Shutdown(RSocket::ENormal, iStatus);
   
   ChangeStatus(EDisconnecting);
   SetActive();
   iTimer->After(KTimeOut);
}

// from CActive
void CSocketsEngine::DoCancel()
{
   iTimer->Cancel();

   // Cancel appropriate request to socket
   switch (iEngineStatus)
   {
   case EConnecting:
      iSocket.CancelConnect();
      break;
   case ELookingUp:
      // Cancel look up attempt
      iResolver.Cancel();
      iResolver.Close();
      break;
   case EConnected:
      iSocketsRead->Cancel();
      iSocketsWrite->Cancel();
      iSocket.Close();
      break;
   case EDisconnecting:
      iSocket.Close();
      break;
   default:
      User::Panic(KPanicSocketsEngine, ESocketsBadStatus);
      break;
   }
   ChangeStatus(ENotConnected);
}

void CSocketsEngine::WriteL( Buffer* aBuffer )
{
   // Write data to socket
   if (iEngineStatus == EConnected){
      TPtrC8 message(aBuffer->accessRawData(0), aBuffer->getLength());
      iSocketsWrite->IssueWriteL( message );
   }
}

void CSocketsEngine::WriteL( const TDesC8& aBytes, TInt aLength )
{
   // Write data to socket
   if (iEngineStatus == EConnected){
      TPtrC8 message(aBytes.Ptr(), aLength);
      iSocketsWrite->IssueWriteL( message );
   }
}
	
void CSocketsEngine::Read()
{
   // Initiate read of data from socket
   if ((iEngineStatus == EConnected) && (!iSocketsRead->IsActive())){
      iSocketsRead->Start();
   }
}

// from CActive
void CSocketsEngine::RunL()
{
   // Active object request complete handler.
   // iEngineStatus flags what request was made, so its
   // completion can be handled appropriately
   iTimer->Cancel(); // Cancel TimeOut timer before completion

   switch(iEngineStatus)
   {
   case EConnecting:
      // IP connection request
      if (iStatus == KErrNone){ // Connection completed successfully
         ChangeStatus(EConnected);
         Read(); //Start CSocketsRead Active object
      } else {
         ChangeStatus(EConnectFailed);
         iConsole.ErrorNotify( R_WAYFINDER_INTERNALCOMERROR_MSG, iStatus.Int());
         ChangeStatus(ENotConnected);
      }
      break;
   case ELookingUp:
      iResolver.Close();
      if (iStatus == KErrNone){
         // DNS look up successful
         iNameRecord = iNameEntry();
         // Extract domain name and IP address from name record
         TBuf<32> output = _L("Domain name = ");
         output.Append( iNameRecord.iName );
         Print( output );
         TBuf<15> ipAddr;
         TInetAddr::Cast(iNameRecord.iAddr).Output(ipAddr);
         output = _L("IP address = ");
         output.Append( ipAddr );
         Print( output );
         // And connect to the IP address
         ChangeStatus(ENotConnected);
         ConnectL(TInetAddr::Cast(iNameRecord.iAddr).Address());
      }
      else{ // DNS lookup failed
         ChangeStatus(ELookUpFailed);
         iConsole.ErrorNotify( R_WAYFINDER_INTERNALCOMERROR_MSG, iStatus.Int());
         ChangeStatus(ENotConnected);
      }
      break;
   case EDisconnecting:
      if (iStatus == KErrNone){
         iSocket.Close();
         ChangeStatus(ENotConnected);
      }
      else{
         ChangeStatus(EConnectFailed);
      }
      break;
   default:
      User::Panic(KPanicSocketsEngine, ESocketsBadStatus);
      break;
   };
}

void CSocketsEngine::TimerExpired()
{
   Cancel();
   ChangeStatus(ETimedOut);
   iConsole.ErrorNotify( R_WAYFINDER_INTERNALCOMTIMEOUT_MSG, KErrTimedOut );
   //Disconnect();
}

void CSocketsEngine::ChangeStatus(TSocketsEngineState aNewStatus)
{
   // Update the status (and the status display)
   switch (aNewStatus)
   {
   case ENotConnected:
      iConsole.SetStatus(_L("Not connected"));
      break;
   case EConnecting:
      iConsole.SetStatus(_L("Connecting"));
      break;
   case EConnected:
      iConsole.SetStatus(_L("Connected"));
      break;
   case ETimedOut:
      iConsole.SetStatus(_L("Timed out"));
      break;
   case ELookingUp:
      iConsole.SetStatus(_L("Looking up"));
      break;
   case ELookUpFailed:
      iConsole.SetStatus(_L("Look up failed"));
      break;
   case EConnectFailed:
      iConsole.SetStatus(_L("Failed"));
      break;
   case EDisconnecting:
      iConsole.SetStatus(_L("Disconnecting"));
      break;
   default:
      User::Panic(KPanicSocketsEngine, ESocketsBadStatus);
      break;
   }
   iEngineStatus = aNewStatus;
}

void CSocketsEngine::Print(const TDesC& aDes)
{
   // Print some text on the console
   iConsole.SetStatus( aDes );
}

void CSocketsEngine::SetServerName(const TDesC& aName)
{
   if( aName.Length() < iServerName.MaxLength() )
      iServerName.Copy(aName);
   else
      iConsole.ErrorNotify( R_WAYFINDER_INTERNALCOMERROR_MSG, iStatus.Int() );
}

const TDesC& CSocketsEngine::ServerName() const
{
   return iServerName;
}

void CSocketsEngine::SetPort(TInt aPort)
{
   iPort = aPort;
}

TInt CSocketsEngine::Port() const
{
   return iPort;
}

TBool CSocketsEngine::Connected() const
{
   return (iEngineStatus == EConnected);
}
