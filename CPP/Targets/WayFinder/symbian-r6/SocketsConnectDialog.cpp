/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MessageHandler.h"
#include "SocketsConnectDialog.h"
#include "RsgInclude.h"
#include "wayfinder.hrh"
#define ILOG_POINTER aAppUi.iAppUi->iLog
#include "memlog.h"

CSocketsConnectDialog* CSocketsConnectDialog::NewL(CMessageHandler& aAppUi)
{
   CSocketsConnectDialog* self = CSocketsConnectDialog::NewLC(aAppUi);
   CleanupStack::Pop();
   return self;
}

	
CSocketsConnectDialog* CSocketsConnectDialog::NewLC(CMessageHandler& aAppUi)
{
   CSocketsConnectDialog* self = new (ELeave) CSocketsConnectDialog(aAppUi);
   LOGNEW(self, CSocketsConnectDialog);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}


CSocketsConnectDialog::CSocketsConnectDialog(CMessageHandler& aAppUi)
: CSocketsBaseDialog(aAppUi)
{
}


CSocketsConnectDialog::~CSocketsConnectDialog()
{
}


void CSocketsConnectDialog::ConstructL()
{
}

TInt CSocketsConnectDialog::ExecuteLD()
{
   // Execute (display and run) the dialog
   return CEikDialog::ExecuteLD(R_SOCKETS_DIALOG_CONNECT);
}

TBool CSocketsConnectDialog::OkToExitL(TInt /*aKeycode*/)
{
   // Called when dialog is finished with - just save settings
   SaveSettings();
   return(ETrue);
}

void CSocketsConnectDialog::SaveSettings()
{
   // Save server name
   TBuf<64> name;
   GetText(ESocketsServerNameEdwin, name);
   AppUi().SetServerName(name);

   // Save port number
   AppUi().SetPortNumber(GetNumber(ESocketsPortNumberEdwin));
}

void CSocketsConnectDialog::PreLayoutDynInitL()
{
   // Set initial values of server name and port number displayed by dialog
   SetTextL(ESocketsServerNameEdwin, AppUi().ServerName());
   SetNumber(ESocketsPortNumberEdwin, AppUi().PortNumber());
}
