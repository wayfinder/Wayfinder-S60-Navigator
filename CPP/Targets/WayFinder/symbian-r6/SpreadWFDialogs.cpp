/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <commonphoneparser.h>
#include "wayfinder.hrh"
#include "SpreadWFDialogs.h"
#include "ContactBkEngine.h"
#include "CallBackDialog.h"
#include "RsgInclude.h"
#include "SmsHandlerFactory.h"

CSpreadWFDialogs::CSpreadWFDialogs(class MSmsHandler* aSmsSender) :
   iSmsSender(aSmsSender)
{
}

class CSpreadWFDialogs* 
CSpreadWFDialogs::NewLC(class MSmsHandler* aSmsSender, 
                        const TDesC& aSmsUrl)
{
   class CSpreadWFDialogs* self = new (ELeave) CSpreadWFDialogs(aSmsSender);
   CleanupStack::PushL(self);
   self->ConstructL(aSmsUrl);
   return self;
}

class CSpreadWFDialogs* 
CSpreadWFDialogs::NewL(class MSmsHandler* aSmsSender, 
                       const TDesC& aSmsUrl)
{
   CSpreadWFDialogs* self = CSpreadWFDialogs::NewLC(aSmsSender, aSmsUrl);
   CleanupStack::Pop(self);
   return self;
}

void CSpreadWFDialogs::ConstructL(const TDesC& aSmsUrl)
{
   iContactBkEngine = CContactBkEngine::NewL(this);
   iEventGenerator = CSpreadWFEventGenerator::NewL(*this);
   iSmsUrl = aSmsUrl.AllocL();
}

CSpreadWFDialogs::~CSpreadWFDialogs()
{
   delete iContactBkNameArray;
   delete iContactBkNumberArray;
   delete iContactBkEngine;
   delete iEventGenerator;
   delete iSmsUrl;
}


void CSpreadWFDialogs::GenerateEvent(enum TSpreadWFEvent aEvent)
{       
   if (!iEventGenerator) {
      iEventGenerator = CSpreadWFEventGenerator::NewL(*this); 
   }
   iEventGenerator->SendEventL(aEvent);
}

void CSpreadWFDialogs::HandleGeneratedEventL(enum TSpreadWFEvent aEvent)
{
   switch (aEvent) {
   case ESpreadWFEventInfoDialogOk:
      {
         //User has seen info dialog and now has to select to 
         //search from phonebook or to enter a number.
         ShowSpreadWayfinderOptionsDialogL(); 
      }
      break;
   case ESpreadWFEventInfoDialogCancel:
      {
         //Cleanup to release the phone book.
         Cleanup();
      }
      break;
   case ESpreadWFEventOptionsOk:
      {
         //See what option the user chose
         if (iOptionsIndex == 0) {
            //Find and then display all contacts.
            iContactBkEngine->FindAllContactsL();
            if (iContactBkEngine->CountSearchResults() > 0) {
               ContactBkSearchDone(KErrNone);
            }
            else {
               iContactBkEngine->ClearSearch();
               ContactBkSearchDone(KErrNotFound);
            }
         }
         else {
            //Let the user enter a phone number
            ShowSpreadWayfinderEnterToDialogL();
         }
      }
      break;
   case ESpreadWFEventOptionsCancel:
      {
         //Cleanup to release the phone book.
         Cleanup();
      }
      break;
   case ESpreadWFEventEnterToOk:
      {
         //Validate phonenumber and show about to send sms info.
         if (CommonPhoneParser::IsValidPhoneNumber(iSpreadWFSmsField, 
                                                   CommonPhoneParser::EPhoneClientNumber)) {
            //Popup dialog saying we're going to send an sms.
            ShowSpreadWayfinderSendToDialogL();
         }
         else {
            //User entered a invalid phone number, tell him and start over.
            ShowGenericInfoDialogL(ESpreadWFEventInfoDialogOk, 
                                   ESpreadWFEventInfoDialogCancel, 
                                   R_WAYFINDER_INFORMATION_TITLE, 
                                   R_WAYFINDER_INVALIDNUMBER_MSG, 
                                   R_NON_BLOCKING_QUERY_MESSAGE_OK_BACK);
         }
      } 
      break;
   case ESpreadWFEventEnterToCancel:
      {
         //User cancelled entering phoneno so we show him the options again.
         ShowSpreadWayfinderOptionsDialogL(); 
      }
      break;
   case ESpreadWFEventContactBkSearchDialogOk:
      {
         //Store the contact id the user selected and 
         //show that contacts phone numbers.
         if (iContactBkIndex < 0) {
            iContactBkEngine->ClearSearch();
            ContactBkSearchDone(KErrNotFound);
         } else {
            iContactBkEngine->SetCurrentContact(iContactBkIndex);
            ShowAllPhoneNumbersDialogL();
         }
      }
      break;
   case ESpreadWFEventContactBkSearchDialogCancel:
      {
         //Clear the ContactBk search cause the user cancelled.
         //Then show the options dialog again.
         iContactBkEngine->ClearSearch();
         ShowSpreadWayfinderOptionsDialogL();
      }
      break;
   case ESpreadWFEventContactBkPhoneNumberDialogOk:
      {
         //Store number the user selected, show the send sms confirm dialog.
         iSpreadWFSmsField = iContactBkNumberArray->MdcaPoint(iContactBkIndex);
         ShowSpreadWayfinderSendToDialogL();
      }
      break;
   case ESpreadWFEventContactBkPhoneNumberDialogCancel:
      {
         //The user cancelled when selecting numbers so show 
         //him available contacts again.
         ContactBkSearchDone(KErrNone);
      }
      break;
   case ESpreadWFEventSendToOk:
      {
         //Now we actually send the sms.
         _LIT(KSpace, " ");
         HBufC* sms = HBufC::NewL(160);
         TPtr smsPtr = sms->Des();
         CCoeEnv::Static()->ReadResourceL(smsPtr, R_WF_SPREAD_WAYFINDER_MESSAGE);
         sms->Des().Append(KSpace);
         sms->Des().Append(*iSmsUrl);
         if (sms) {
            if (iSmsSender) {
               //Make sure the caller did supply
               //an object for sending the smses.
               iSmsSender->SendSmsL(*sms, iSpreadWFSmsField);
            }
            delete sms;
         }
         //Cleanup to release the phone book.
         Cleanup();
      }
      break;
   case ESpreadWFEventSendToCancel:
      {
         //The user cancelled sending the sms, so show him the options dialog.
         iContactBkEngine->ClearSearch();
         ShowSpreadWayfinderOptionsDialogL();
      }
      break;
   default:
      //Do nothing, shouldn't happen.
      break;
   }
}

void CSpreadWFDialogs::StartDialogsL()
{
   if (!iContactBkEngine) {
      iContactBkEngine = CContactBkEngine::NewL(this);
   }
   ShowGenericInfoDialogL(ESpreadWFEventInfoDialogOk, 
                          ESpreadWFEventInfoDialogCancel, 
                          R_WF_SHARE_WAYFINDER, 
                          R_WF_SHARE_WAYFINDER_INFO_TEXT, 
                          R_NON_BLOCKING_QUERY_MESSAGE_OK_BACK);
}

void CSpreadWFDialogs::Cleanup()
{
   if (iContactBkEngine) {
      delete iContactBkEngine;
      iContactBkEngine = NULL;
   }
   delete iContactBkNameArray;
   iContactBkNameArray = NULL;
   delete iContactBkNumberArray;
   iContactBkNumberArray = NULL;
}

void CSpreadWFDialogs::ShowSpreadWayfinderOptionsDialogL()
{
   //Dialog that lets the user chose between contact bk search
   //and entering a number manually.
   CDesCArray* optionArray = new (ELeave) CDesCArrayFlat(2);
   HBufC* option = CCoeEnv::Static()->AllocReadResourceLC(R_WF_OPTION_CONTACTS);
   optionArray->AppendL(*option);
   CleanupStack::PopAndDestroy(option);
   option = CCoeEnv::Static()->AllocReadResourceLC(R_WF_OPTION_MANUALLY);
   optionArray->AppendL(*option);
   CleanupStack::PopAndDestroy(option);
   TInt preSelIndex = iOptionsIndex;

   typedef TCallBackEvent<CSpreadWFDialogs, TSpreadWFEvent> cb_t;
   typedef CCallBackListDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
                        ESpreadWFEventOptionsOk,
                        ESpreadWFEventOptionsCancel),
                   iOptionsIndex,
                   R_WF_SHARE_WAYFINDER,
                   optionArray,
                   R_WAYFINDER_CALLBACK_SINGLE_LIST_QUERY_OK_BACK,
                   EFalse, 
                   preSelIndex);
}

void CSpreadWFDialogs::ShowSpreadWayfinderEnterToDialogL()
{   
   //Dialog that accepts a number from the user to 
   //send sms to.
   typedef TCallBackEvent<CSpreadWFDialogs, TSpreadWFEvent> cb_t;
   typedef CCallBackEditDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this, 
                        ESpreadWFEventEnterToOk, 
                        ESpreadWFEventEnterToCancel),
                   R_WF_MOBILE_PHONE_NUMBER, 
                   iSpreadWFSmsField,
                   R_WF_NON_BLOCKING_NUMBER_DATA_QUERY);
}

void CSpreadWFDialogs::ShowSpreadWayfinderSendToDialogL()
{
   //The final dialog that tells the user that we're 
   //going to send an sms to this number.
   _LIT(KSpace, " ");
   HBufC* tmpText = 
      CCoeEnv::Static()->AllocReadResourceLC(R_WF_CONFIRM_SEND_SMS_TEXT);
   HBufC* confirmText = 
      HBufC::NewLC(tmpText->Length() + iSpreadWFSmsField.Length() + 2);
   confirmText->Des().Copy(*tmpText);
   confirmText->Des().Append(KSpace);
   confirmText->Des().Append(iSpreadWFSmsField);
   CleanupStack::Pop(confirmText);
   CleanupStack::PopAndDestroy(tmpText);
   typedef TCallBackEvent<CSpreadWFDialogs, TSpreadWFEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
                        ESpreadWFEventSendToOk,
                        ESpreadWFEventSendToCancel),
                   R_WF_SEND_SMS,
                   confirmText,
                   R_NON_BLOCKING_QUERY_MESSAGE);
}

void CSpreadWFDialogs::ContactBkSearchDone(TInt aError)
{
   //Callback from the ContactBkEngine.
   if (aError == KErrNone) {
      //If we got some hits from the search, show a selectlist popup.
      if (!iContactBkNameArray) {
         iContactBkNameArray =
            new (ELeave) CDesCArrayFlat(iContactBkEngine->CountSearchResults());
         iContactBkEngine->GetSearchResultNamesTabbed(*iContactBkNameArray);
      }
      typedef TCallBackEvent<CSpreadWFDialogs, TSpreadWFEvent> cb_t;
      typedef CCallBackSelectionListDialog<cb_t> cbd_t;
      cbd_t::RunDlgLD(cb_t(this,
                           ESpreadWFEventContactBkSearchDialogOk,
                           ESpreadWFEventContactBkSearchDialogCancel),
                      iContactBkIndex,
                      iContactBkNameArray,
                      R_WAYFINDER_CALLBACK_SELECT_LIST_QUERY);
   }
   else {
      //Popup dialog saying we didn't find any matching contacts.
      //Then we come back to the options dialog.
      ShowGenericInfoDialogL(ESpreadWFEventInfoDialogOk, 
                             ESpreadWFEventInfoDialogCancel, 
                             R_WAYFINDER_INFORMATION_TITLE, 
                             R_CONNECT_NO_MATCHES_FOUND, 
                             R_NON_BLOCKING_QUERY_MESSAGE_OK_BACK);
   }
}

void CSpreadWFDialogs::ContactBkInUse()
{
   //Popup dialog saying that we're busy searching.
   ShowGenericInfoDialogL(ESpreadWFEventInfoDialogOk, 
                          ESpreadWFEventInfoDialogCancel, 
                          R_WAYFINDER_INFORMATION_TITLE, 
                          R_WF_CONTACT_SEARCH_BUSY, 
                          R_NON_BLOCKING_QUERY_MESSAGE);
}

void CSpreadWFDialogs::ShowAllPhoneNumbersDialogL()
{
   //Show selectlist popup.
   if (iContactBkNumberArray) {
      delete iContactBkNumberArray;
      iContactBkNumberArray = NULL;
   }
   iContactBkNumberArray = new (ELeave) CDesCArrayFlat(4);
   iContactBkEngine->GetContactNumbers(*iContactBkNumberArray);
   TInt preselectedIndex;
   if (iContactBkNumberArray->Find(iSpreadWFSmsField, preselectedIndex) != 0) {
      preselectedIndex = 0;
   }
   CDesCArray* labeledNumberArray = new (ELeave) CDesCArrayFlat(4);
   iContactBkEngine->GetContactNumbers(*labeledNumberArray, ETrue);
   //Check if the contact had any numbers stored.
   if (iContactBkNumberArray->Count() > 0) {
      typedef TCallBackEvent<CSpreadWFDialogs, TSpreadWFEvent> cb_t;
      typedef CCallBackListDialog<cb_t> cbd_t;
      cbd_t::RunDlgLD(cb_t(this,
                           ESpreadWFEventContactBkPhoneNumberDialogOk,
                           ESpreadWFEventContactBkPhoneNumberDialogCancel),
                      iContactBkIndex,
                      R_WF_SEND_TO,
                      labeledNumberArray,
                      R_WAYFINDER_CALLBACK_DOUBLE_LIST_QUERY_OK_BACK,
                      EFalse,
                      preselectedIndex);
   }
   else {
      //Display dialog saying no entries found.
      //Then let the user select other contact.
      ShowGenericInfoDialogL(ESpreadWFEventContactBkPhoneNumberDialogCancel, 
                             ESpreadWFEventContactBkPhoneNumberDialogCancel, 
                             R_WAYFINDER_INFORMATION_TITLE, 
                             R_WF_CONTACT_NO_DETAILS, 
                             R_NON_BLOCKING_INFO_MESSAGE);
   }
}

void CSpreadWFDialogs::ShowGenericInfoDialogL(TSpreadWFEvent aOkEvent, 
                                              TSpreadWFEvent aCancelEvent, 
                                              TInt aHeader, TInt aText, 
                                              TInt aResourceId)
{
   //Popup dialog showing header and text.
   typedef TCallBackEvent<CSpreadWFDialogs, TSpreadWFEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this, aOkEvent, aCancelEvent),
                   aHeader, aText, aResourceId);
}
