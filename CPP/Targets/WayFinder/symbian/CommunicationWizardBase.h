/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMUNICATION_WIZARD_GENERIC_H
#define COMMUNICATION_WIZARD_GENERIC_H

#include <e32base.h>
#include <badesca.h>
#include "ContactBkObserver.h"               // ContactBkEngineObserver
#include "SmsHandlerObserver.h"              // SmsHandlerObserver
#include "EventGenerator.h"                  // EventGenerator
#include "GuiProt/GuiProtMessageSender.h"    // GuiProtMessageReceiver
#include "GuiProt/ServerEnums.h"
#include <vector>

namespace isab{
   class Favorite;
   class GuiFavorite;
   class GetFavoritesReplyMess;
   class GetFavoritesAllDataReplyMess;
   class GetFavoriteInfoReplyMess;
   class AdditionalInfo;
}

enum TWizardSteps {
   EWhatToSend = 100,
   EValidateContentChoice,
   EGetSelectedFavorite,
   ESendAs,
   EHowToAddRecipient,
   EAddRecipient,
   EShowSelectedContact,
   EStoreContactDetail,
   EValidateManualInput,
   ESubject,
   EFormatMessage,
   EConfirmation,
   ESendMessage,
   EQuit,
};

enum TWizContent
{
   EWfCurRoute,
   EWfGpsPosition,
   EWfMapPosition,
   EWfCurDestination,
   EWfFavorite,
   EWfNotSpecified
};

enum TWizTransmitAS
{
   EWfSMS,
   EWfEmail,
   EWfMMS
};

enum THowToAddRecipient
{
   EFromPhoneBook,
   EEnterManually
};

enum TDialogTypeEvent
{
   ESubjectDialog,
   ENonValidPhoneNbrDialog,
   ENonValidEmailDialog,
   EContactBookBusyDialog
};

/**
 * Class that lets a user send a wayfinder sms or a mail 
 * containing a route or a destinatin etc. It is built
 * up to be a wizard, the user is able to go back and
 * forward through the wizard that is built up by dialogs.
 * This class handles the logic, switching between the different
 * steps in the wizard etc. The subclasses displays all the dialogs.
 */

class CCommunicationWizardBase : public CBase, 
                                 public GuiProtMessageReceiver,
                                 public MSmsHandlerObserver,
                                 public MContactBkObserver
                             
{
protected:

   CCommunicationWizardBase(class GuiProtMessageSender& aSender);

public:   

   virtual ~CCommunicationWizardBase();
   
   /**
    * The initial phase for the wizard, gets called when
    * the wizard is to be launched. By defined aPreDefinedContent
    * with a value other than EWfNotSpecified the wizard becomes
    * context sensitive. The wizard the skipps the "What to send"
    * step and jump right to the next step.   
    * @param aPreDefinedContent, if set to other value then EWfNotSpecified
    *                            the wizard will skipp "What to send".
    *                            If this step is wanted then don't specify
    *                            a value for this though it is defaulted to 
    *                            EWfNotSpecified.
    */
   void StartWizardL(TWizContent aPreDefinedContent = EWfNotSpecified,
                     isab::Favorite* aFavorite = NULL);
   
   /** 
    * Generates av event.
    * @param aEvent The event to be generated.
    */
   void GenerateEvent(enum TWizardSteps aEvent);

   /**
    * Handles the generated event.
    * @param eEvent The event to be handled.
    */
   void HandleGeneratedEventL(enum TWizardSteps aEvent);

public: // From base classes

   /**
    * From MContactBkEngineObserver
    */
   void ContactBkSearchDone(TInt aError);
   void ContactBkInUse();

   /**
    * From MSmsHandlerObserver
    */
   void SmsSent(TInt aStatus, TInt aCount, TInt aNbrSent);
   void SmsReceived(class CSmsParser* aSmsParser, TInt32 aMsvId);

   /**
    * From GuiProtMessageReceiver
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess* mess);

protected:

   /**
    * If user selected to send a favorite we need to ask the
    * server for it, if not just generate an event and move on.
    */
   void ValidateContentChoiceL();

   /**
    * Validates the manually entered phone number.
    */
   void ValidatePhoneNbrL();

   /**
    * Validates the manually entered email adress.
    */
   void ValidateEmailAdressL();

   /**
    * Resets, deletes and allocates the iContentArray.
    * @param aNbrIndex The granularity of the array.
    */
   void ResetContentArray(TUint aNbrIndex);

   /**
    * Asks nav2 about all information about a specific favorite.
    */
   void GetSelectedFavoriteL();

   /** 
    * Formates the message to be sent.
    * @aMessage The variable holding the formated message when done.
    */
   void FormatMessageL(HBufC*& aMessage);

   /**
    * Sends the message, email/sms.
    */
   void SendMessageL();

   /**
    * Cleans up
    */
   void CleanUp();

protected: // Struct holding the data to be sent
   
   /**
    * Struct that holds information about the message 
    * that is about to be sent.
    */
   struct TWizardData
   {
      /// The content, route/destination etc. 
      enum TWizContent iContent;
      /// How to send the message, sms/email. 
      enum TWizTransmitAS iTransmitAs;
      /// Objecttype, needed when formating the message. 
      enum isab::GuiProtEnums::ObjectType iObjectType;
      /// The recipient, phone nbr. 
      TBuf<256> iSmsRecipient;
      /// The recipient, email adress.
      TBuf<256> iEmailRecipient;
      /// The formated message to be sent. 
      HBufC* iMessage;
      /// The resource id of the user text that should 
      ///  be a part of the message. 
      TInt iPersonalMessageId;
      /// Signature, the personal message converted to a char*. 
      char* iSignature;
   };

protected: // Pure virtual functions

   /**
    * Displays a list of the content that is possible to send.
    */
   virtual void ShowWhatToSendDialogL() = 0;

   /**
    * Displays a list of all the stored favorites.
    * @param aMessage A message from server/nav2 that contains all
    *                 favorites in a vector.
    */
   virtual void ShowFavoriteListL(isab::GetFavoritesReplyMess* aMessage) = 0;

   /**
    * Displays a list of how the selcted content should be sent.
    */
   virtual void ShowSendAsDialogL() = 0;

   /**
    * List that lets the user select how to add the recipient,
    * from the contact book or manually.
    */
   virtual void ShowHowToAddRecipientDialogL() = 0;

   /**
    * Dialog that lets the user enter a phone number or 
    * email adress.
    */
   virtual void ShowEnterManuallyDialogL() = 0;

   /**
    * Displays all contact info for a selected contact.
    */
   virtual void ShowContactDetailsDialogL() = 0;

   /**
    * Dislplays a dialog that lets the user confirm that en email/sms
    * is going to be sent.
    */
   virtual void ShowConfirmationDialogL() = 0;

   /**
    * Dislplays a dialog.
    * @param aNextEvent The event to be triggered when user presses 
    *                   the left soft key.
    * @param aBackEvent The event to be triggered when user presses 
    *                   the right soft key.
    * @param aHeader The id for the title.
    * @param aText The id for the text to be displayed in the body.
    * @param aResourceId The resource id for the dialog to be displayed.
    */   
   virtual void ShowGenericInfoDialogL(TWizardSteps aNextEvent, 
                                       TWizardSteps aBackEvent, 
                                       TDialogTypeEvent aEvent) = 0;

   /**
    * Displays dialog, if aError is KErrNone the dialog displays
    * a list of contacts. If not KErrNone an error dialog should
    * be displayed.
    */
   virtual void ShowContactBookDialogL() = 0;

   /**
    * Called by super class when ContactBkInUse is called
    * by the ContactBkEngine.
    */
   virtual void ShowContactBookBusyDialogL() = 0;

   /**
    * Information dialog displaying information about the sms that has
    * been sent.
    * @param aStatus,  KErrNone if successful otherwise one of the other system
    *                  panic codes.
    * @param aCount,   Total number of emails to be sent.
    * @param aNbrSent, The number of emails sent.
    */
   virtual void ShowSmsSentDialogL(TInt aStatus, TInt aCount, TInt aNbrSent) = 0;

   /**
    * Returns the current origin.
    * @return Current origin.
    */
   virtual TPoint GetOrigin() = 0;

   /**
    * Returns the current destination.
    * @return Current destination.
    */
   virtual TPoint GetDestination() = 0;

   /**
    * Returns the current position.
    * @return Current position.
    */
   virtual TPoint GetCurrentPosition() = 0;

   /**
    * Returns the name of the current destination.
    * @return Name of the current destination.
    */
   virtual const TDesC& GetCurrentRouteDestinationName() = 0;

   /**
    * Get the coordinate of the current postion in the map.
    * @return The map position.
    */
   virtual TPoint GetCoordinate() = 0;

   /**
    * Gets the id of the current route.
    * @return The id of the current route.
    */
   virtual TInt64 GetRouteId() = 0;

   /**
    * Calls wayfinder appui to send a GuiProtMess
    * to nav2.
    * @param aMessage, The message to send to nav2.
    */
   virtual void SendMessageL(isab::GuiProtMess* aMessage) = 0;

   /**
    * Allocates the message to be sent, body in the sms.
    */
   virtual void SetSignatureL() = 0;

   /**
    * Allocates a text from the resources and appends
    * it to the iCOntentArray.
    * @param aResourceId The resource id to be added to the array.
    */
   virtual void AddItemToContentArray(TInt aResourceId) = 0;

   /**
    * Validates a phone nbr. 
    * @param aPhoneNbr, the phone nbr to validate.
    * @return, True if the phone nbr is valid
    *          False if not.
    */
   virtual TBool IsValidPhoneNumber(const TDesC& aPhoneNbr) = 0;

   /**
    * Returns the text that is displayed in the blue note.
    * @return The info text that is displayed in the blue note in the map.
    */
   virtual const char* GetMapMoverInfoText() = 0;

   /**
    * Function letting the sub class know that a sms is about
    * to be sent. Good when wanting to display an info message.
    */
   virtual void SmsAboutToBeSent() = 0;

protected:
   
   typedef CEventGenerator<CCommunicationWizardBase, enum TWizardSteps>
   CCommunicationWizardGenerator;

 
   /// Event generator, used to generate event 
   CCommunicationWizardGenerator* iEventGenerator;

   /// Used to send sms. 
   class MSmsHandler* iSmsHandler;
   /// Used to get contacts from the contact book. 
   class CContactBkEngine* iContactBkEngine;
   /// Used to send messages to Nav2. 
   class GuiProtMessageSender& iSender;
   /// Stores information about the selected favorite to be send. 
   isab::Favorite* iFavorite;
   /// Holds all stored favorites. 
   std::vector<isab::GuiFavorite*> iFavVector;
   /// Used to display data in all the list dialogs. 
   CDesCArray* iContentArray;
   /// Holds data about the wizard/message to be sent 
   TWizardData iWizardData;
   /// Used when formating a wayfinder sms. 
   static const uint8 SmsStringDelimiter;
   /// Holds the index for the selected content to be sent. 
   TInt iContentSelection;
   /// Holds the index for the selected format to send the message in. 
   TInt iTransmitAsSelection;
   /// Holds the index for the selected way to add the recipient. 
   TInt iHowToAddRecSelection;
   /// Holds the index for the selected favorite to be send. 
   TInt iFavoriteSelection;
   /// Holds the index for the selected contact. 
   TInt iContactSelection;
   /// Holds the index for the selected cantact detail. 
   TInt iContactDetailSelection;
   /// Acts like a map between the data in contentArray and TWizContent,
   /// makes it possible to set the content dynamically 
   TInt* iContentMapArray;
   /// Array containing resource ids for personal message, the message
   /// depens on what to send.
   RArray<TInt> iPersonalMessageIds;
   /// This is set when calling StartWizardL with a type of message as
   /// parameter. By doing this we get a context sensitive wizard,
   /// this means that the what to send dialog is skipped.
   TWizContent iPreDefinedContent;
};

#endif
