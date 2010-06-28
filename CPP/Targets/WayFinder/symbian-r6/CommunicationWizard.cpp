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

#include "RsgInclude.h"
#include "CommunicationWizard.h"
#include "ContactBkEngine.h"
#include "CallBackDialog.h"
#include "WayFinderAppUi.h"
#include "WFTextUtil.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/Favorite.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "nav2util.h"

#include <algorithm>

using namespace isab;

CCommunicationWizard::CCommunicationWizard(class CWayFinderAppUi& aAppUi, 
                                           class GuiProtMessageSender& aSender) : 
   CCommunicationWizardBase(aSender),
   iAppUi(aAppUi)
{
   
}

void CCommunicationWizard::ConstructL()
{
   iPersonalMessageIds.Append(R_CONNECT_PERSONAL_CURRENT_ROUTE);
   iPersonalMessageIds.Append(R_CONNECT_PERSONAL_CURRENT_POSITION);
   iPersonalMessageIds.Append(R_CONNECT_SELECTED_POSITION);
   iPersonalMessageIds.Append(R_CONNECT_PERSONAL_CURRENT_DESTINATION);
   iPersonalMessageIds.Append(R_CONNECT_PERSONAL_MY_FAVORITE);
}

class CCommunicationWizard* CCommunicationWizard::NewL(class CWayFinderAppUi& aAppUi, 
                                                       class GuiProtMessageSender& aSender)
{
   class CCommunicationWizard* self = new (ELeave) CCommunicationWizard(aAppUi, aSender);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

CCommunicationWizard::~CCommunicationWizard()
{
}

void 
CCommunicationWizard::ShowWhatToSendDialogL()
{
}

void
CCommunicationWizard::ShowFavoriteListL(GetFavoritesReplyMess* aMessage)
{
   std::vector<GuiFavorite*>* tmpFavVector = aMessage->getGuiFavorites();
   { for(std::vector<GuiFavorite*>::iterator it = iFavVector.begin(); 
       it != iFavVector.end();
       ++it) { 
      delete *it;
   } }
   iFavVector.clear();
   iFavVector.reserve(tmpFavVector->size());

   GuiFavorite* tmpFav;
   ResetContentArray(tmpFavVector->size());
   std::vector<GuiFavorite*>::iterator it;
   for(it = tmpFavVector->begin(); it != tmpFavVector->end(); it++) {   
      tmpFav = *it;
      iFavVector.push_back(new GuiFavorite(*tmpFav));
      HBufC* name = WFTextUtil::AllocLC(tmpFav->getName());
      iContentArray->AppendL(*name);
      CleanupStack::PopAndDestroy(name);
   }

   if (iFavoriteSelection >= iContentArray->Count()) {
      // To be sure we don't step outside the list.
      iFavoriteSelection = 0;
   }

   // Display a list with all the favorites
   typedef TCallBackEvent<CCommunicationWizard, TWizardSteps> cb_t;
   typedef CCallBackListDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this, EGetSelectedFavorite, EQuit),
                   iFavoriteSelection,
                   R_MM_FAVORITES, 
                   iContentArray, 
                   R_WAYFINDER_CALLBACK_DOUBLE_LIST_QUERY_OK_BACK,
                   ETrue, iFavoriteSelection);   
}

void 
CCommunicationWizard::ShowSendAsDialogL()
{
   ResetContentArray(2);
   AddItemToContentArray(R_CONNECT_SMS_ITEM);
   AddItemToContentArray(R_CONNECT_EMAIL_ITEM);
   TWizardSteps cancelAction;

   if (iPreDefinedContent == EWfNotSpecified) {
      cancelAction = EWhatToSend;
   } else {
      cancelAction = EQuit;
   }

   ShowGenericListDialogL(EHowToAddRecipient, 
                          cancelAction, 
                          R_WF_SEND_AS_TITLE, 
                          iTransmitAsSelection, 
                          iTransmitAsSelection);
}

void
CCommunicationWizard::ShowHowToAddRecipientDialogL()
{
   ResetContentArray(2);
   AddItemToContentArray(R_WF_OPTION_CONTACTS);
   if(iWizardData.iTransmitAs == EWfSMS) {
      AddItemToContentArray(R_WF_OPTIONS_HEADER);   
   } else {
      AddItemToContentArray(R_WF_ENTER_EMAIL);      
   }

   ShowGenericListDialogL(EAddRecipient, 
                          ESendAs, 
                          R_WF_RECIPIENT_TITLE, 
                          iHowToAddRecSelection, 
                          iHowToAddRecSelection);
}

void
CCommunicationWizard::ShowEnterManuallyDialogL()
{
   // Dialog that accepts a number from the user to 
   // send sms to.
   TInt title;
   TInt dlgId;
   TDes* recipient = NULL;
   if (iWizardData.iTransmitAs == EWfSMS) {
      title = R_WF_MOBILE_PHONE_NUMBER;
      dlgId = R_WF_NON_BLOCKING_NUMBER_DATA_QUERY;
      recipient = &iWizardData.iSmsRecipient;
   } else {
      title = R_CONNECT_EMAIL_ITEM;
      dlgId = R_WF_BLOCKING_DATA_QUERY_OK_BACK;
      recipient = &iWizardData.iEmailRecipient;
   }   
   typedef TCallBackEvent<CCommunicationWizard, TWizardSteps> cb_t;
   typedef CCallBackEditDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this, EValidateManualInput, 
                        EHowToAddRecipient),
                   title, 
                   *recipient,
                   dlgId);
}

void
CCommunicationWizard::ShowContactDetailsDialogL()
{
   ResetContentArray(4);
   CDesCArray* contactDetailArray = new (ELeave) CDesCArrayFlat(4);
   TDes* recipient = NULL;
   if(iWizardData.iTransmitAs == EWfSMS) {
      // Sms
      iContactBkEngine->GetContactNumbers(*iContentArray);
      iContactBkEngine->GetContactNumbers(*contactDetailArray, ETrue);
      recipient = &iWizardData.iSmsRecipient;
   } else {
      // Email
      iContactBkEngine->GetContactEmails(*iContentArray);
      iContactBkEngine->GetContactEmails(*contactDetailArray, ETrue);
      recipient = &iWizardData.iEmailRecipient;
   }
   TInt preselectedIndex;
   if (iContentArray->Find(*recipient, preselectedIndex) != 0) {
      preselectedIndex = 0;
   }
   if (iContentArray->Count() > 0) {
      typedef TCallBackEvent<CCommunicationWizard, TWizardSteps> cb_t;
      typedef CCallBackListDialog<cb_t> cbd_t;
      cbd_t::RunDlgLD(cb_t(this, EStoreContactDetail, EAddRecipient),
                      iContactDetailSelection,
                      R_WF_SEND_TO,
                      contactDetailArray,
                      R_WAYFINDER_CALLBACK_DOUBLE_LIST_QUERY_OK_BACK,
                      EFalse,
                      preselectedIndex);
   }
   else {
      //Display dialog saying no entries found.
      //Then let the user select other contact.
      ShowGenericInfoDialogL(EAddRecipient, 
                             EAddRecipient, 
                             R_WAYFINDER_INFORMATION_TITLE, 
                             R_WF_CONTACT_NO_DETAILS, 
                             R_NON_BLOCKING_INFO_MESSAGE);
   }   
}

void
CCommunicationWizard::ShowConfirmationDialogL()
{
   TInt title;
   if(iWizardData.iTransmitAs == EWfSMS) {
      title = R_WAYFINDER_SEND_SMS_REG_TITLE;
   } else {
      title = R_WAYFINDER_SEND_EMAIL;
   }
   ShowGenericInfoDialogL(ESendMessage, ESubject, title,
                          R_CONNECT_SEND_MESSAGE_QUERY,
                          R_NON_BLOCKING_QUERY_MESSAGE_OK_BACK);
}


void 
CCommunicationWizard::ShowGenericListDialogL(TWizardSteps aNextEvent,
                                             TWizardSteps aBackEvent,
                                             TInt aTitleResourceId, 
                                             TInt aPreSelectedIndex,
                                             TInt& aSelectionIndex)
{
   if (aPreSelectedIndex >= iContentArray->Count()) {
      // We don't wan't to step outside the list.
      aPreSelectedIndex = 0;
   }
   typedef TCallBackEvent<CCommunicationWizard, TWizardSteps> cb_t;
   typedef CCallBackListDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this, aNextEvent, aBackEvent),
                   aSelectionIndex,
                   aTitleResourceId, 
                   iContentArray, 
                   R_WAYFINDER_CALLBACK_SINGLE_LIST_QUERY_OK_BACK, 
                   ETrue,
                   aPreSelectedIndex);   
}

void 
CCommunicationWizard::ShowGenericInfoDialogL(TWizardSteps aNextEvent, 
                                             TWizardSteps aBackEvent, 
                                             TInt aHeader, TInt aText, 
                                             TInt aResourceId)
{
   //Popup dialog showing header and text.
   typedef TCallBackEvent<CCommunicationWizard, TWizardSteps> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this, aNextEvent, aBackEvent),
                   aHeader, aText, aResourceId);
}

void 
CCommunicationWizard::AddItemToContentArray(TInt aResourceId)
{
   HBufC* item = CEikonEnv::Static()->AllocReadResourceLC(aResourceId);
   iContentArray->AppendL(*item);
   CleanupStack::PopAndDestroy(item);
}


void 
CCommunicationWizard::ShowContactBookDialogL()
{
   iContactBkEngine->ClearSearch();
   iContactBkEngine->FindAllContactsL();
   // Callback from the ContactBkEngine.
   if (iContactBkEngine->CountSearchResults() > 0) {
      // If we got some hits from the search, show a selectlist popup.
      ResetContentArray(iContactBkEngine->CountSearchResults());
      iContactBkEngine->GetSearchResultNamesTabbed(*iContentArray);
      
      typedef TCallBackEvent<CCommunicationWizard, TWizardSteps> cb_t;
      typedef CCallBackSelectionListDialog<cb_t> cbd_t;
      cbd_t::RunDlgLD(cb_t(this,
                           EShowSelectedContact,
                           EHowToAddRecipient),
                      iContactSelection,
                      iContentArray,
                      R_WAYFINDER_CALLBACK_SELECT_LIST_QUERY);
   }
   else {
      iContactBkEngine->ClearSearch();
      // Popup dialog saying we didn't find any matching contacts.
      // Then we come back to the options dialog.
      ShowGenericInfoDialogL(EHowToAddRecipient, 
                             EQuit, 
                             R_WAYFINDER_INFORMATION_TITLE, 
                             R_CONNECT_NO_MATCHES_FOUND, 
                             R_NON_BLOCKING_INFO_MESSAGE);
   }  
}

void 
CCommunicationWizard::ShowContactBookBusyDialogL()
{
   // Popup dialog saying that we're busy searching.
   ShowGenericInfoDialogL(EWhatToSend, 
                          EQuit, 
                          R_WAYFINDER_INFORMATION_TITLE, 
                          R_WF_CONTACT_SEARCH_BUSY, 
                          R_NON_BLOCKING_QUERY_MESSAGE);
}

void 
CCommunicationWizard::ShowSmsSentDialogL(TInt aStatus, TInt aCount, TInt aNbrSent)
{      
   if (aStatus == KErrNone) {
      // Success
      ShowGenericInfoDialogL(EQuit, 
                             EQuit, 
                             R_CONNECT_SEND_SMS_HEADER, 
                             R_CONNECT_SENT_BY_SERVER, 
                             R_NON_BLOCKING_INFO_MESSAGE);
   } else {
      // Could not send
      ShowGenericInfoDialogL(EWhatToSend,
                             EQuit,
                             R_WAYFINDER_ERROR_MSG,
                             R_WF_ERROR_SENDING_SMS,
                             R_NON_BLOCKING_INFO_MESSAGE);
   }   
}

TPoint CCommunicationWizard::GetOrigin()
{
   return iAppUi.GetOrigin();   
}

TPoint CCommunicationWizard::GetDestination()
{
   return iAppUi.GetDestination();  
}

TPoint CCommunicationWizard::GetCurrentPosition()
{
   return iAppUi.GetCurrentPosition();
}

const TDesC& CCommunicationWizard::GetCurrentRouteDestinationName()
{
   return iAppUi.GetCurrentRouteDestinationName();  
}

TPoint CCommunicationWizard::GetCoordinate()
{
   TPoint currentMapPos;
   iAppUi.GetCoordinate(currentMapPos);
   return currentMapPos;
}

TInt64 CCommunicationWizard::GetRouteId()
{
   return iAppUi.GetRouteId();
}

void CCommunicationWizard::SendMessageL(GuiProtMess* aMessage)
{
   iAppUi.SendMessageL(aMessage);   
}

void CCommunicationWizard::ShowGenericInfoDialogL(TWizardSteps aNextEvent, 
                                                  TWizardSteps aBackEvent, 
                                                  TDialogTypeEvent aEvent)
{
   switch(aEvent) {
   case ESubjectDialog:
      ShowGenericInfoDialogL(aNextEvent,
                             aBackEvent,
                             R_WF_SUBJECT_TITLE, 
                             iWizardData.iPersonalMessageId, 
                             R_NON_BLOCKING_QUERY_MESSAGE_SEND_BACK);
      break;
   case ENonValidPhoneNbrDialog:
      ShowGenericInfoDialogL(aNextEvent, 
                             aBackEvent, 
                             R_WAYFINDER_INFORMATION_TITLE, 
                             R_WAYFINDER_INVALIDNUMBER_MSG, 
                             R_NON_BLOCKING_QUERY_MESSAGE_OK_BACK);
      break;
   case ENonValidEmailDialog:
      ShowGenericInfoDialogL(aNextEvent, 
                             aBackEvent, 
                             R_WAYFINDER_INFORMATION_TITLE, 
                             R_WAYFINDER_RECIPIENT_ADDRESS_INVALID_MSG, 
                             R_NON_BLOCKING_QUERY_MESSAGE_OK_BACK);   
      break;
   case EContactBookBusyDialog:
      // Popup dialog saying that we're busy searching.
      ShowGenericInfoDialogL(aNextEvent, 
                             aBackEvent, 
                             R_WAYFINDER_INFORMATION_TITLE, 
                             R_WF_CONTACT_SEARCH_BUSY, 
                             R_NON_BLOCKING_QUERY_MESSAGE);

   }
}

void CCommunicationWizard::SetSignatureL()
{
   HBufC* tmp = CEikonEnv::Static()->
      AllocReadResourceLC(iPersonalMessageIds[iWizardData.iContent]);
   iWizardData.iSignature = WFTextUtil::newTDesDupL(*tmp);
   CleanupStack::PopAndDestroy(tmp);
}

TBool CCommunicationWizard::IsValidPhoneNumber(const TDesC& aPhoneNbr)
{
   return CommonPhoneParser::
      IsValidPhoneNumber(aPhoneNbr, 
                         CommonPhoneParser::EPhoneClientNumber);
}

const char* CCommunicationWizard::GetMapMoverInfoText()
{
   return NULL; 
}

void CCommunicationWizard::SmsAboutToBeSent()
{
}
