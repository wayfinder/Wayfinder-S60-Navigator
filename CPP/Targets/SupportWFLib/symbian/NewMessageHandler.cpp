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

// INCLUDE FILES
#include <e32std.h>
#include "TCPHandler.h"
#include <es_sock.h>
#include "SocketConstants.h"
#include "NewMessageHandler.h"
#include "sockets.pan"
#include "SocketsEngine.h"

// ================= MEMBER FUNCTIONS =======================
//

// ----------------------------------------------------
// CNewMessageHandler::CNewMessageHandler()
// Constructor
// Frees reserved resources
// ----------------------------------------------------
//
CNewMessageHandler::CNewMessageHandler()
{
}

CNewMessageHandler* CNewMessageHandler::NewL( MTCPHandler* aTCPHandler, RSocketServ* aSockServ )
{
   CNewMessageHandler* self = CNewMessageHandler::NewLC( aTCPHandler, aSockServ );
   CleanupStack::Pop();
   return self;
}

CNewMessageHandler* CNewMessageHandler::NewLC( MTCPHandler* aTCPHandler, RSocketServ* aSockServ )
{
   CNewMessageHandler* self = new (ELeave) CNewMessageHandler();
   CleanupStack::PushL( self );
   self->ConstructL( aTCPHandler, aSockServ );
   return self;
}

// ----------------------------------------------------------
// CNewMessageHandler::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CNewMessageHandler::ConstructL( MTCPHandler* aTCPHandler, RSocketServ* aSockServ)
{
   iSocketsEngine = CNewSocketsEngine::NewL(*this, aSockServ);
   iTCPHandler = aTCPHandler;
}

// ----------------------------------------------------
// CNewMessageHandler::~CNewMessageHandler()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CNewMessageHandler::~CNewMessageHandler()
{
   delete iSocketsEngine;
}



void CNewMessageHandler::ConnectL( TDesC &aServerIP, TInt aPort, TInt aIAP ) // <a name="ConnectL32">
{
   iSocketsEngine->SetPort( aPort );
   iSocketsEngine->SetServerName( aServerIP );
   iSocketsEngine->SetIAP( aIAP );
   iSocketsEngine->ConnectL();
}

void CNewMessageHandler::Disconnect()
{
   iSocketsEngine->Disconnect();
}

TBool CNewMessageHandler::IsConnected()
{
   return iSocketsEngine->IsConnected();
}

const TDesC& CNewMessageHandler::ServerName() const
{
   return iSocketsEngine->ServerName();
}

TInt CNewMessageHandler::PortNumber() const
{
   return iSocketsEngine->Port();
}

TInt CNewMessageHandler::GetIAP() const
{
   return iSocketsEngine->GetIAP();
}

void CNewMessageHandler::SendMessageL( TDesC8 &data, int length )
{
   iSocketsEngine->WriteL( data, length );
}

void CNewMessageHandler::ReceiveMessageL( const TDesC8& aMessage, const TInt aMessageLength )
{
   iTCPHandler->Receive( aMessage, aMessageLength );
}

void CNewMessageHandler::ErrorNotify( TDesC& aErrMessage, TInt aErrCode )
{
   iTCPHandler->ErrorNotify( aErrMessage, aErrCode );
}

void CNewMessageHandler::ErrorNotify( TInt aResourceId, TInt aErrCode )
{
   iTCPHandler->ErrorNotify( aResourceId, aErrCode );
}

void CNewMessageHandler::SetStatus( TSocketsEngineState aNewStatus )
{
   // Update the status (and the status display)
   switch (aNewStatus)
   {
   case ENotConnected:
      break;
   case EStartingConnection:
      break;
   case EConnecting:
      break;
   case EConnected:
      iTCPHandler->Connected();
      break;
   case ETimedOut:
      break;
   case ELookingUp:
      break;
   case ELookUpFailed:
      break;
   case EConnectFailed:
      break;
   case EDisconnecting:
      break;
   }
}

void
CNewMessageHandler::ConnectionCancelled()
{
   iTCPHandler->DownloadCancelled();
}

void
CNewMessageHandler::Cancel()
{
   iSocketsEngine->Cancel();
}

void 
CNewMessageHandler::CancelDownload()
{
   iSocketsEngine->SocketsEngineCancel();
}

// End of File  
