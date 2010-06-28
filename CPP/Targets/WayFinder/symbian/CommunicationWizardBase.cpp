/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CommunicationWizardBase.h"
#include "ContactBkEngine.h"
#include "SmsHandlerFactory.h"
#include "WFTextUtil.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/Favorite.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiProtMessageSender.h"  
#include "nav2util.h"
#include <algorithm>

using namespace isab;

const uint8
CCommunicationWizardBase::SmsStringDelimiter = '}'; // the yen-sign in the GSM-character table.

CCommunicationWizardBase::CCommunicationWizardBase(class GuiProtMessageSender& aSender) : 
   iSender(aSender)
{
   iWizardData.iContent = EWfCurRoute;
   iWizardData.iTransmitAs = EWfSMS;

   iWizardData.iMessage = NULL;
   iWizardData.iSmsRecipient.Copy(_L(" "));
   iWizardData.iEmailRecipient.Copy(_L(" "));

   iContactBkEngine = CContactBkEngine::NewL(this);
   iSmsHandler = SmsHandlerFactory::CreateL(this);
}

CCommunicationWizardBase::~CCommunicationWizardBase()
{
   delete iWizardData.iMessage;
   delete iWizardData.iSignature;
   delete iContactBkEngine;
   delete iEventGenerator;
   delete[] iContentMapArray;
   delete iSmsHandler;

   if (iContentArray) {
      iContentArray->Reset();
      delete iContentArray;
   }
   for (std::vector<isab::GuiFavorite*>::iterator it = iFavVector.begin();
       it != iFavVector.end(); 
       it++) {
      delete *it;
   }
   iPersonalMessageIds.Reset();
}

void
CCommunicationWizardBase::StartWizardL(TWizContent aPreDefinedContent, 
                                       Favorite* aFavorite)
{
   if (aFavorite) {
      if (iFavorite) {
         delete iFavorite;
         iFavorite = aFavorite;
      }
      iFavorite = aFavorite;
   }
   iPreDefinedContent = aPreDefinedContent;
   if (aPreDefinedContent == EWfNotSpecified) {
      GenerateEvent(EWhatToSend);
   } else {
      iWizardData.iContent = aPreDefinedContent;
      GenerateEvent(EValidateContentChoice);
   }
}

void 
CCommunicationWizardBase::GenerateEvent(enum TWizardSteps aEvent)
{
   if (!iEventGenerator) {
      iEventGenerator = CCommunicationWizardGenerator::NewL(*this); 
   }
   iEventGenerator->SendEventL(aEvent);   
}

void 
CCommunicationWizardBase::HandleGeneratedEventL(TWizardSteps aEvent)
{
   switch(aEvent)
   {
   case EWhatToSend:
      // Dislplay dialog that lets user decide what to send.
      ShowWhatToSendDialogL();
      break;
   case EValidateContentChoice:
      // Validate what the user wants to send, this is needed because
      // when sending a favorite we need to ask for them from the server,
      // we don't have them stored here.
      ValidateContentChoiceL();
      break;
   case EGetSelectedFavorite:
      // Get information from the server about the selected favorite.
      GetSelectedFavoriteL();
      break;
   case ESendAs:
      // Dislplay dialog that lets user decide how to send, email or sms.
      ShowSendAsDialogL();
      break;
   case EHowToAddRecipient:
      {
         TWizTransmitAS transmitAs = TWizTransmitAS(iTransmitAsSelection);
//          if (transmitAs == EWfEmail) {
//             // We dont want an email adress as recepient if sms is 
//             // was selected and vice versa.
//             iWizardData.iEmailRecipient.Copy(_L(""));
//          }
         iWizardData.iTransmitAs = transmitAs;
         // Display dialog that lets user decide how to add the recipient, by
         // entering the phone nbr manually or by using the phone book.
         ShowHowToAddRecipientDialogL();
      }
      break;
   case EAddRecipient:
      if (iHowToAddRecSelection == EFromPhoneBook) {
         // User wants to add recipient from the phone book, get
         // all contacts from the phone book.
         ShowContactBookDialogL();
      } else {
         // Let user enter nbr/email manually.
         ShowEnterManuallyDialogL();
      }
      break;
   case EValidateManualInput:
      {
         if (iWizardData.iTransmitAs == EWfSMS) {
            ValidatePhoneNbrL();
         } else {
            ValidateEmailAdressL();
         }
      } 
      break;
   case EShowSelectedContact:
      // Store the contact id the user selected and 
      // show that contacts phone numbers.
      if (iContactSelection < 0) {
         iContactBkEngine->ClearSearch();
         GenerateEvent(EAddRecipient);
      } else {
         iContactBkEngine->SetCurrentContact(iContactSelection);
         ShowContactDetailsDialogL();
      }
      break;
   case EStoreContactDetail:
      // Store the selected contact adress.
      if (iWizardData.iTransmitAs == EWfEmail) {
         iWizardData.iEmailRecipient = iContentArray->MdcaPoint(iContactDetailSelection);
      } else {
         iWizardData.iSmsRecipient = iContentArray->MdcaPoint(iContactDetailSelection);
      }
      GenerateEvent(ESubject);
      break;
   case ESubject:
      // XXX Display the standard message, should the user 
      // be allowed to change this??
      ShowGenericInfoDialogL(EFormatMessage, 
                             EAddRecipient, 
                             ESubjectDialog);
      break;
   case EFormatMessage:
      // Format the message to be sent.
      FormatMessageL(iWizardData.iMessage);
      GenerateEvent(ESendMessage);
      break;
   case EConfirmation:
      // Display dialog that notifies the user that a 
      // sms/email will be sent. Send message
      ShowConfirmationDialogL();
      break;
   case ESendMessage:
      // Send the formated message.
      SendMessageL();
      break;
   case EQuit:
      CleanUp();
   }
}

void
CCommunicationWizardBase::ValidateContentChoiceL()
{
   if (iPreDefinedContent == EWfNotSpecified) {
      iWizardData.iContent = TWizContent(iContentMapArray[iContentSelection]);
   }
   iWizardData.iPersonalMessageId = iPersonalMessageIds[iWizardData.iContent];
   SetSignatureL();

   if(iWizardData.iContent == EWfFavorite && !iFavorite) {
      // User wants to send a favorite, display all the favorite
      GenericGuiMess message(GuiProtEnums::GET_FAVORITES, uint16(0),
                             uint16(MAX_UINT16));
      iSender.SendMessage(message, this);
   } else {
      GenerateEvent(ESendAs);
   }
}

void
CCommunicationWizardBase::ValidatePhoneNbrL()
{
   iWizardData.iSmsRecipient.TrimLeft();
   if (IsValidPhoneNumber(iWizardData.iSmsRecipient)) {
      GenerateEvent(ESubject);
   } else {
      ShowGenericInfoDialogL(EAddRecipient, 
                             EQuit, 
                             ENonValidPhoneNbrDialog);
   }
}

void
CCommunicationWizardBase::ValidateEmailAdressL()
{
   if (iWizardData.iEmailRecipient.Find(_L("@")) != KErrNotFound) {
      GenerateEvent(ESubject);
   } else {
      ShowGenericInfoDialogL(EAddRecipient, 
                             EQuit, 
                             ENonValidEmailDialog);   
   }
}

void 
CCommunicationWizardBase::ResetContentArray(TUint aNbrIndex)
{
   if (iContentArray && iContentArray->Count() > 0) {
      iContentArray->Reset();
      delete iContentArray;
   }   
   iContentArray = new CDesCArrayFlat(aNbrIndex); 
}

void
CCommunicationWizardBase::GetSelectedFavoriteL()
{
   if(iFavoriteSelection <= (TInt)iFavVector.size()) {
      GenericGuiMess message(GuiProtEnums::GET_FAVORITE_INFO, 
                             iFavVector[iFavoriteSelection]->getID());
      iSender.SendMessage(message, this);      
   }
}


void
CCommunicationWizardBase::FormatMessageL(HBufC*& aMessage)
{
   if (aMessage) {
      delete aMessage;
      aMessage = NULL;
   }

   char* identifier = new (ELeave) char[1024]; // on heap, so allocate more than enough.
   identifier[0] = '\0';
   const TDesC& name = GetCurrentRouteDestinationName();
   char* destname = WFTextUtil::newTDesDupL(name);

   TPoint originPoint      = GetOrigin();
   TPoint destinationPoint = GetDestination();
   TPoint currentPoint     = GetCurrentPosition();
   TPoint currentMapPos    = GetCoordinate();

   isab::GuiProtEnums::ObjectType objecttype = isab::GuiProtEnums::invalidObjectType;   
   
   TInt checkNumCalc = 0;
   if (iWizardData.iTransmitAs == EWfSMS) {
      switch (iWizardData.iContent) {
      case EWfCurRoute:
         objecttype = isab::GuiProtEnums::RouteMessage;
         checkNumCalc = (destinationPoint.iY & 0xFF) ^ 
            (destinationPoint.iX & 0xFF) ^ (TInt)'W';
         sprintf(identifier, "//SCKL3F6B //WAYFR%i %i %i %i %i %s%c%s%c%s", 
                 destinationPoint.iY, destinationPoint.iX, originPoint.iY, originPoint.iX,
                 checkNumCalc,
                 ""/*origin name*/, SmsStringDelimiter, 
                 destname, SmsStringDelimiter, 
                 iWizardData.iSignature);
         break;
      case EWfCurDestination:
         objecttype = isab::GuiProtEnums::DestinationMessage;
         checkNumCalc = (destinationPoint.iY & 0xFF) ^ 
            (destinationPoint.iX & 0xFF) ^ (TInt)'W';
         sprintf(identifier, "//SCKL3F6B //WAYFDz%i %i %i %s%c%s", 
                 destinationPoint.iY, destinationPoint.iX, 
                 checkNumCalc,
                 destname, SmsStringDelimiter, 
                 iWizardData.iSignature);
         break;
      case EWfMapPosition:
         objecttype = isab::GuiProtEnums::DestinationMessage;
         checkNumCalc = (currentMapPos.iY & 0xFF) ^ 
            (currentMapPos.iX & 0xFF) ^ (TInt)'W';
         const char* desc = GetMapMoverInfoText();
         if (desc) {
            // If the mapmover could return a valid text
            sprintf(identifier, "//SCKL3F6B //WAYFDz%i %i %i %s%c%s", 
                    currentMapPos.iY, currentMapPos.iX, checkNumCalc,
                    desc, SmsStringDelimiter, 
                    iWizardData.iSignature);
         } else {
            // No text from the mapmover
            sprintf(identifier, "//SCKL3F6B //WAYFDz%i %i %i %s", 
                    currentMapPos.iY, currentMapPos.iX, checkNumCalc, 
                    iWizardData.iSignature);
         }
         break;
      case EWfGpsPosition:
         objecttype = isab::GuiProtEnums::DestinationMessage;
         checkNumCalc = (currentPoint.iY & 0xFF) ^ (currentPoint.iX & 0xFF) ^ (TInt)'W';
         sprintf(identifier, "//SCKL3F6B //WAYFDz%i %i %i %s", 
                 currentPoint.iY, currentPoint.iX, checkNumCalc,
                 iWizardData.iSignature);
         break;
      case EWfFavorite: {
         checkNumCalc = (iFavorite->getLat() & 0xFF) ^ (iFavorite->getLon() & 0xFF) ^ (TInt)'W';
         // Make description and limit to 255 chars.
         char* description = new char[ MAX_that_can(iFavorite->getSize(), 256) ];
         strcpy(description, iFavorite->getDescription());
         Favorite::InfoVect infos = iFavorite->getInfos();
         std::sort(infos.begin(), infos.end(), isab::FavoriteInfoCmp());
         for (Favorite::InfoVect::const_iterator it = infos.begin(); it != infos.end(); ++it) {
            if ((*it)->getType() == GuiProtEnums::dont_show) {
               continue;
            }
            if (description[0] != '\0') {
               strcat(description, ", "); // Used two bytes of type here.
            }
            strcat(description, (*it)->getKey());
            strcat(description, ": "); // Null-byte for key and value used here (size wise).
            strcat(description, (*it)->getValue());
         }
         description[ 255 ] = '\0';
         
         sprintf(identifier, "//SCKL3F6B //WAYFF%li %li %i %s%c%s%c%s%c%s%c%s%c%s", 
                 iFavorite->getLat(), 
                 iFavorite->getLon(), checkNumCalc,
                 iFavorite->getName(), SmsStringDelimiter, 
                 iFavorite->getShortName(), SmsStringDelimiter, 
                 description, SmsStringDelimiter, 
                 iFavorite->getCategory(), SmsStringDelimiter, 
                 iFavorite->getMapIconName(), SmsStringDelimiter, 
                 iWizardData.iSignature);
         delete [] description;
      }  
         break;
      case EWfNotSpecified:
         // This should never happen, quit the wizard
         GenerateEvent(EQuit);
         break;
      };
   } else {
      switch (iWizardData.iContent) {
      case EWfCurRoute:
         iWizardData.iObjectType = isab::GuiProtEnums::ItineraryMessage;
         sprintf(identifier, "0x%08x%x", HIGH(GetRouteId()), LOW(GetRouteId()));
         break;
      case EWfGpsPosition:
         iWizardData.iObjectType = isab::GuiProtEnums::PositionMessage;
         sprintf(identifier, "%i %i %s", 
                 currentPoint.iY, currentPoint.iX, "");
         break;
      case EWfMapPosition:
         iWizardData.iObjectType = isab::GuiProtEnums::PositionMessage;
         sprintf(identifier, "%i %i %s", 
                 currentMapPos.iY, currentMapPos.iX, "");
         break;
      case EWfCurDestination:
         iWizardData.iObjectType = isab::GuiProtEnums::PositionMessage;
         sprintf(identifier, "%i %i %s", 
                 destinationPoint.iY, destinationPoint.iX, destname);
         break;
      case EWfFavorite:  
         if ((iFavorite->getID() != isab::Favorite::INVALID_FAV_ID) && 
             (iFavorite->isSynced())) {
               // synchronized favorite
               iWizardData.iObjectType = isab::GuiProtEnums::DestinationMessage;
               sprintf(identifier, "%lu", iFavorite->getID());
            } else {
               // unsynchronized favorite
               iWizardData.iObjectType = isab::GuiProtEnums::PositionMessage;
               // try to get a name
               const char* name = iFavorite->getName();
               if (!name || (strlen(name) == 0)) {
                  name = iFavorite->getShortName();
               }
               if (!name || (strlen(name) == 0)) {
                  name = iFavorite->getDescription();
               }
               if (!name) {
                  name = ""; // at least a valid pointer to avoid crashing on sprintf.
               }
               sprintf(identifier, "%li %li %s", 
                       iFavorite->getLat(), iFavorite->getLon(), name);
            }
         break;
      case EWfNotSpecified:
         // This should never happen, quit the wizard
         GenerateEvent(EQuit);
         break;
      };
   }
   aMessage = WFTextUtil::AllocL(identifier);
   delete identifier;
}

void
CCommunicationWizardBase::SendMessageL()
{
   if(iWizardData.iTransmitAs == EWfSMS) {
      SmsAboutToBeSent();
      // Send sms
#ifdef __WINS__
      iSmsHandler->CreateLocalMessageL(*iWizardData.iMessage, iWizardData.iSmsRecipient);
#else
      iSmsHandler->SendSmsL(*iWizardData.iMessage, iWizardData.iSmsRecipient);
#endif
   } else {
      char* emailSender = isab::strdup_new("please_dont_reply@somemailaddresshereTODO");
      isab::SendMessageMess* message =
         new (ELeave) isab::SendMessageMess(isab::GuiProtEnums::HTML_email,
                                            iWizardData.iObjectType, 
                                            WFTextUtil::newTDesDupL(*iWizardData.iMessage), 
                                            emailSender, WFTextUtil::newTDesDupL(iWizardData.iEmailRecipient), 
                                            iWizardData.iSignature);
      SendMessageL(message);      
   }
}

void
CCommunicationWizardBase::CleanUp()
{
   delete iFavorite;
   iFavorite = NULL;
   iContactBkEngine->ClearSearch();
}

void 
CCommunicationWizardBase::ContactBkSearchDone(TInt aError)
{
   ShowContactBookDialogL();
}

void 
CCommunicationWizardBase::ContactBkInUse()
{
   ShowGenericInfoDialogL(EWhatToSend, 
                          EQuit,
                          EContactBookBusyDialog);
}

void 
CCommunicationWizardBase::SmsSent(TInt aStatus, 
                                  TInt aCount, 
                                  TInt aNbrSent)
{      
   ShowSmsSentDialogL(aStatus, aCount, aNbrSent);   
}

void 
CCommunicationWizardBase::SmsReceived(class CSmsParser* aSmsParser, 
                                      TInt32 aMsvId)
{
   
}

bool
CCommunicationWizardBase::GuiProtReceiveMessage(isab::GuiProtMess* mess)
{
   if (mess->getMessageType() == isab::GuiProtEnums::GET_FAVORITES_REPLY) {
      ShowFavoriteListL((GetFavoritesReplyMess*)mess);
      return true;
   } else if (mess->getMessageType() == isab::GuiProtEnums::GET_FAVORITE_INFO_REPLY) {
      // Got all info about a favorite, store and use when sending the sms/email
      if (iFavorite) {
         delete iFavorite;
         iFavorite = NULL;
      }
      GetFavoriteInfoReplyMess* replyMess = (GetFavoriteInfoReplyMess*)mess;
      iFavorite = new Favorite(*replyMess->getFavorite());
      GenerateEvent(ESendAs);
      return true;
   } 
   return false;
}
