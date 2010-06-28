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

#include "arch.h"
#include "WayFinderAppUi.h"
#include "TimeOutTimer.h"
#include "GpsSocket.h"
#include "Dialogs.h"
#include "MessageHandler.h"
#include "WayFinderSettings.h"

static const TInt KGpsPort = 3333;

#if defined __WINS__
# if defined GPS_SERVER
MLIT( KGpsName, GPS_SERVER );
# else 
_LIT( KGpsName, "10.11.3.104" ); 
# endif
#else
_LIT( KGpsName, "127.0.0.1" );
#endif

static const TInt KTimeOut = 30000000; // 30 seconds time-out

class CGpsSocket* CGpsSocket::NewL( isab::Nav2::Channel* aNav2Channel )
{
   CGpsSocket* self = CGpsSocket::NewLC( aNav2Channel );
   CleanupStack::Pop();
   return self;
}

class CGpsSocket* CGpsSocket::NewL( isab::Nav2::Channel* aNav2Channel, 
                                    const TDesC& aHost)
{
   class CGpsSocket* self = CGpsSocket::NewLC( aNav2Channel );
   self->SetServerName(aHost);
   CleanupStack::Pop();
   return self;
}

class CGpsSocket* CGpsSocket::NewLC( isab::Nav2::Channel* aNav2Channel )
{
   CGpsSocket* self = new (ELeave) CGpsSocket( aNav2Channel );
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CGpsSocket::CGpsSocket( isab::Nav2::Channel* aNav2Channel ) : 
   CActive(CActive::EPriorityStandard), iNav2Channel(aNav2Channel)
{
}


CGpsSocket::~CGpsSocket()
{
   Cancel();
   
   delete iTimer;
   iTimer = NULL;

   iSocket.Close();
   iSocketServ.Close();
}

void CGpsSocket::ConstructL()
{
   ConstructL(KGpsName);
}

void CGpsSocket::ConstructL(const TDesC& aHost)
{
   iState = ENotConnected;
   iPort = KGpsPort;
   SetServerName( aHost );

   // Start a timer
   iTimer = CTimeOutTimer::NewL( CActive::EPriorityHigh, *this /*,NULL*/ );
   CActiveScheduler::Add(this); 

   // Open channel to Socket Server
   User::LeaveIfError(iSocketServ.Connect());
}

void CGpsSocket::SetServerName(const TDesC& aName)
{
   iServerName.Copy(aName);
}

void CGpsSocket::ConnectL()
{
   // Initiate connection process
   if( iState == ENotConnected ){
      TInetAddr addr;
      if( addr.Input(iServerName) == KErrNone ){
         // server name is already a valid ip address
         // Open a TCP socket
         iSocket.Close(); // Just in case
         User::LeaveIfError(iSocket.Open( iSocketServ,
                                          KAfInet,
                                          KSockStream,
                                          KProtocolInetTcp));

         // Set up address information
         iAddress.SetPort(iPort);
         iAddress.SetAddress( addr.Address() );
         
         // Initiate socket connection
         iSocket.Connect(iAddress, iStatus);
         iState = EConnecting;
         SetActive();

         iCounter = 0;
         // Start a timeout
         iTimer->After(KTimeOut);
      }
   }
}

void CGpsSocket::Disconnect()
{
   Cancel();
   iSocket.Shutdown(RSocket::ENormal, iStatus);
   
   iState = EDisconnecting;
   SetActive();
   iTimer->After(KTimeOut);
}

TBool CGpsSocket::IsConnected()
{
   return (iState == EConnected);
}

void CGpsSocket::TimerExpired()
{
   Cancel();
   switch( iState )
   {
   case EConnecting:
      {
         iState = ENotConnected;
         // Tell the GUI we couldn't connect
         _LIT( KConnectError, "Error connecting to GPS" );
         WFDialog::ShowErrorDialogL( KConnectError );
      }
      break;
   case EConnected:
      {
         iCounter++;
         if( iCounter >= 10 ){ //Give the GPS five minuets before giving up
            _LIT( KReadError, "Error reading data from GPS" );
            WFDialog::ShowErrorDialogL( KReadError );
            Disconnect();
         }
         else{
            ReadData();
         }
      }
      break;
   case EDisconnecting:
      iState = ENotConnected;
      // Assume that we are disconnected
      break;
   case ENotConnected:
      //We should never reach this point
      break;
   }
}

void CGpsSocket::DoCancel()
{
   // Cancel asychronous read request
   iTimer->Cancel();
   iSocket.CancelRead();
}

void CGpsSocket::RunL()
{
   iTimer->Cancel();
   // Active object request complete handler
   if (iStatus == KErrNone){
      // Character has been read from socket
      switch( iState )
      {
      case ENotConnected:
         break;
      case EConnecting:
         iState = EConnected;
         ReadData();
         break;
      case EConnected:
         SendData();
         break;
      case EDisconnecting:
         iState = ENotConnected;
         break;
      }
   }
   else{
      // Error: pass it up to user interface
      switch( iState )
      {
      case ENotConnected:
         Cancel();
         break;
      case EConnecting:
         {
            _LIT( KConnectError, "Error connecting to GPS" );
            WFDialog::ShowErrorDialogL( KConnectError );
            iState = ENotConnected;
         }
         break;
      case EConnected:
         {
            _LIT( KReadError, "Error reading data from GPS" );
            WFDialog::ShowErrorDialogL( KReadError );
            Disconnect();
         }
         break;
      case EDisconnecting:
         {
            _LIT( KDisconnectError, "Error disconnecting from GPS" );
            WFDialog::ShowErrorDialogL( KDisconnectError );
            iState = ENotConnected;
         }
         break;
      }
   }	
}

void CGpsSocket::ReadData()
{
   // Initiate a new read from socket into iBuffer
   iSocket.Read( iBuf, iStatus );
   SetActive();
   iTimer->After(KTimeOut);
}

void CGpsSocket::SendData()
{
   iCounter = 0;
   const TUint8* data = iBuf.Ptr();
   iNav2Channel->writeData( data, iBuf.Length() );
   ReadData(); // Immediately start another read
}

