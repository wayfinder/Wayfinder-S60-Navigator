/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>
#include <stdio.h>

#include "arch.h"
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/GuiProtEnums.h"
#include "WayFinderAppUi.h"

#include "MessageHandler.h"
#include "WayFinderConstants.h"
#include "SocketsConnectDialog.h"
#include "sockets.pan"
#include "ErrorResolver.h"

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "QueueSerial.h"
#include "SocketsEngine.h"
#include "SprocketsEngine.h"
#include "Buffer.h"

#include "Dialogs.h"

#define ILOG_POINTER iAppUi->iLog
#include "memlog.h"

_LIT(KNone, "None");

// ================= MEMBER FUNCTIONS =======================
//

// ----------------------------------------------------
// CMessageHandler::CMessageHandler()
// Constructor
// Frees reserved resources
// ----------------------------------------------------
//
CMessageHandler::CMessageHandler()
{
}

// ----------------------------------------------------------
// CMessageHandler::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CMessageHandler::ConstructL( CWayFinderAppUi* aAppUi )
{
   iAppUi = aAppUi;
   iSocketsEngine = CSocketsEngine::NewL(*this);
   iSprocketsEngine = NULL;
}

void CMessageHandler::ConstructL( CWayFinderAppUi* aAppUi,
                                  isab::Nav2::Channel* nav2)
{
   iAppUi = aAppUi;
   char sprocketname[128] = {0};
   snprintf(sprocketname, 127, "Sprocket%p", nav2);
   sprocketname[127] = 0;
   
   iSprocketsEngine = CSprocketsEngine::NewL(this, nav2, sprocketname);
   iSocketsEngine = NULL;
}


// ----------------------------------------------------
// CMessageHandler::~CMessageHandler()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CMessageHandler::~CMessageHandler()
{
   if (iSocketsEngine) {
      LOGDEL(iSocketsEngine);
      delete iSocketsEngine;
   }
   if (iSprocketsEngine) {
      LOGDEL(iSprocketsEngine);
      delete iSprocketsEngine;
   }
}

//XXX this connect method should never be called, unless Nav2 is running on a
// seperate machine.
void CMessageHandler::ConnectL()
{
   // Create dialog to allow user to view/edit connection details
   CSocketsConnectDialog* dialog = CSocketsConnectDialog::NewLC(*this);

   // Display and execute dialog, and act according to return value
   if (dialog->ExecuteLD()){
      CleanupStack::Pop(dialog);
      if (iSocketsEngine) {
         iSocketsEngine->ConnectL(); // Initiate connection
      }
   }
   else{
      CleanupStack::Pop(dialog); // Abandon
   }
}

void CMessageHandler::ConnectL( const TDesC &aServerIP, TInt aPort ) // <a name="ConnectL32">
{
   if (iSocketsEngine) {
      iSocketsEngine->SetPort( aPort );
      iSocketsEngine->SetServerName( aServerIP );
      iSocketsEngine->ConnectL();
   } else {
      iSprocketsEngine->ConnectL();
   }
}

void CMessageHandler::Disconnect()
{
   if (iSocketsEngine) {
      iSocketsEngine->Disconnect();
   } else {
      iSprocketsEngine->Disconnect();
   }
}

void CMessageHandler::Release()
{
   if(iSprocketsEngine){
      iSprocketsEngine->Disconnect();
      iSprocketsEngine->Release();
   }
}
TBool CMessageHandler::IsConnected()
{
   if (iSocketsEngine) {
      return iSocketsEngine->Connected();
   } else {
      return iSprocketsEngine->IsConnected();
   }
}

void CMessageHandler::SetServerName(const TDesC& aName)
{
   if (iSocketsEngine) {
      iSocketsEngine->SetServerName(aName);
   }
}

const TDesC& CMessageHandler::ServerName() const
{
   if (iSocketsEngine) {
      return iSocketsEngine->ServerName();
   } else {
      return KNone;
   }
}

void CMessageHandler::SetPortNumber(TInt aPort)
{
   if (iSocketsEngine) {
      iSocketsEngine->SetPort(aPort);
   }
}

TInt CMessageHandler::PortNumber() const
{
   if (iSocketsEngine) {
      return iSocketsEngine->Port();
   } else {
      return 0;
   }
}

void CMessageHandler::SendMessageL(class isab::Buffer* aBuffer )
{
   if (iSocketsEngine) {
      iSocketsEngine->WriteL( aBuffer );
   } else {
      iSprocketsEngine->WriteL( aBuffer );
   }
}

void CMessageHandler::WriteBytes( const TDesC8& aBytes )
{
   if (iSocketsEngine) {
      iSocketsEngine->WriteL( aBytes, aBytes.Length());
   } else {
      iSprocketsEngine->Write( aBytes);
   }
}

void CMessageHandler::Panic()
{
   iAppUi->Panic(this);
}

void CMessageHandler::ReceiveMessageL( const TDesC8& aMessage,
                                      const TInt aMessageLength )
{
   iAppUi->ReceiveMessageL( aMessage, aMessageLength );
}

void
CMessageHandler::ReceiveMessageL( const TDesC8& aMessage )
{
   iAppUi->ReceiveMessageL( aMessage, aMessage.Length() );
}
   

void CMessageHandler::ErrorNotify( TInt aResourceId, TInt aErrCode )
{
   TInt error;
   // Error up call: inform user and quit
   TBuf<128> errorText;
   CErrorResolver::GetError( aErrCode, &errorText );

   if (iAppUi->IsReleaseVersion()) {
      // Show only the basic error to the user.
      TRAP( error, WFDialog::ShowErrorDialogL( aResourceId ) );
   } else {
      // Show additional error info obtained from CErrorResolver and stored 
      // in errorText.
      HBufC* errMessage = CCoeEnv::Static()->AllocReadResourceLC(aResourceId);
      TRAP( error, CEikonEnv::Static()->InfoWinL( errorText, *errMessage ) );
      CleanupStack::PopAndDestroy(errMessage);
   }

   if(error != KErrNone){
      User::Panic (KPanicSockets, ESocketsAppView);
   }
}

void CMessageHandler::SetStatus( const TDesC& aStatus )
{
   CEikonEnv::Static()->InfoMsg(aStatus);
}

// End of File  
