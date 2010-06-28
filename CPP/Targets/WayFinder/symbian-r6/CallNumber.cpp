/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <etel.h>
#include <aknquerydialog.h>

#include "CallNumber.h"
#include "WayFinderAppUi.h"
#include "RsgInclude.h"

_LIT(KTsyName,"phonetsy.tsy");

TBool CallNumber::EditAndCallNumberL(const TDesC& aPhoneNumber, 
                                      TInt aCommand, 
                                      class CWayFinderAppUi* aAppUi)
{
   TBuf<128> number;
   number.Copy(aPhoneNumber);
   if (ShowEditPopup(number)) {
      /* User didn't cancel, call the number. */
      return DialNumberL(number, aCommand, aAppUi);
   }
   return EFalse;
}

TBool CallNumber::DialNumberL(const TDesC& aPhoneNumber, 
                               TInt aCommand, 
                               class CWayFinderAppUi* aAppUi)
{
   TBool result = EFalse;
   if (aPhoneNumber.Compare(KEmpty) != 0) {
      aAppUi->HandleCommandL(aCommand);
      TRAPD( err, DialNumberL(aPhoneNumber); );
      aAppUi->HandlePhoneCallL(ETrue);
      result = ETrue;
   }
   return result;
}

void CallNumber::DialNumberL(const TDesC& aPhoneNumber)
{
   //Create a connection to the tel server
   RTelServer server;
   CleanupClosePushL(server);
   User::LeaveIfError(server.Connect());

   //Load in the phone device driver
   User::LeaveIfError(server.LoadPhoneModule(KTsyName));
	
   //Find the number of phones available from the tel server
   TInt numberPhones;
   User::LeaveIfError(server.EnumeratePhones(numberPhones));

   //Check there are available phones
   if (numberPhones < 1){
      User::Leave(KErrNotFound);
   }

   //Get info about the first available phone
   RTelServer::TPhoneInfo info;
   User::LeaveIfError(server.GetPhoneInfo(0, info));

   //Use this info to open a connection to the phone, the phone is identified by its name
   RPhone phone;
   CleanupClosePushL(phone);
   User::LeaveIfError(phone.Open(server, info.iName));

   //Get info about the first line from the phone
   RPhone::TLineInfo lineInfo;
   User::LeaveIfError(phone.GetLineInfo(0, lineInfo));

   //Use this to open a line
   RLine line;
   CleanupClosePushL(line);
   User::LeaveIfError(line.Open(phone, lineInfo.iName));

   //Open a new call on this line
   TBuf <100> newCallName;
   RCall call;
   CleanupClosePushL(call);
   User::LeaveIfError(call.OpenNewCall(line, newCallName));
   //newCallName will now contain the name of the call

   //Dial a number
   User::LeaveIfError(call.Dial(aPhoneNumber));

   //Close the phone, line and call connections and remove them from the cleanup stack
   //NOTE: This does not hang up the call
   CleanupStack::PopAndDestroy(3);//phone, line, call

   //Unload the phone device driver
   User::LeaveIfError(server.UnloadPhoneModule(KTsyName));

   //Close the connection to the tel server and remove it from the cleanup stack
   CleanupStack::PopAndDestroy(&server);
}

TInt CallNumber::ShowEditPopup(TDes& aNumber)
{
   CAknTextQueryDialog* dlg = CAknTextQueryDialog::NewL(aNumber);
   dlg->SetMaxLength(31);
   return dlg->ExecuteLD(R_WAYFINDER_CALLCENTER_ADD_NEW_NUMBER_QUERY);
}
