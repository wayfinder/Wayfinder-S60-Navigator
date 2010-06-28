/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMUNICATIONWIZARD_H
#define COMMUNICATIONWIZARD_H

#include <e32base.h>
#include <badesca.h>
#include "CommunicationWizardBase.h"
#include "GuiProt/GuiProtMessageSender.h"    // GuiProtMessageReceiver
#include "GuiProt/ServerEnums.h"
#include "wayfinder.hrh"
#include <vector>

namespace isab{
   class Favorite;
   class GuiFavorite;
   class GetFavoritesReplyMess;
   class GetFavoritesAllDataReplyMess;
   class GetFavoriteInfoReplyMess;
   class AdditionalInfo;
}


/**
 * Class that lets a user send a wayfinder sms or a mail 
 * containing a route or a destinatin etc. It is built
 * up to be a wizard, the user is able to go back and
 * forward throug the wizard that is built up by dialogs.
 * forward throug the wizard that is built up by dialogs.
 *
 * Subclass to CCommunicationWizardBase. The base class
 * handles all the logic. This class only displays the
 * correct dialogs and populates arrays with strins from
 * the rss file.
 */

class CCommunicationWizard : public CCommunicationWizardBase
{
   CCommunicationWizard(class CWayFinderAppUi& aAppUi, 
                        class GuiProtMessageSender& aSender);
   void ConstructL();

public:

   static class CCommunicationWizard* NewL(class CWayFinderAppUi& aAppUi, 
                                           class GuiProtMessageSender& aSender);
   
   ~CCommunicationWizard();

protected: // From CCommunicationWizardBase

   /**
    * Displays a list of the content that is possible to send.
    */
   void ShowWhatToSendDialogL();

   /**
    * Displays a list of all the stored favorites.
    * @param aMessage A message from server/nav2 that contains all
    *                 favorites in a vector.
    */
   void ShowFavoriteListL(isab::GetFavoritesReplyMess* aMessage); 

   /**
    * Displays a list of how the selcted content should be sent.
    */
   void ShowSendAsDialogL();

   /**
    * List that lets the user select how to add the recipient,
    * from the contact book or manually.
    */
   void ShowHowToAddRecipientDialogL();

   /**
    * Dialog that lets the user enter a phone number or 
    * email adress.
    */
   void ShowEnterManuallyDialogL();

   /**
    * Displays all contact info for a selected contact.
    */
   void ShowContactDetailsDialogL();

   /**
    * Dislplays a dialog that lets the user confirm that en email/sms
    * is going to be sent.
    */
   void ShowConfirmationDialogL();

   /**
    * Dislplays a single selection list dialog.
    * @param aNextEvent The event to be triggered when user presses 
    *                   the left soft key.
    * @param aBackEvent The event to be triggered when user presses 
    *                   the right soft key.
    * @param aTitleResourceId The id for the title.
    * @param aPreSelectedIndex Index for pre selection in the list.
    * @param aSelectionIndex Holds the selected index.
    */
   void ShowGenericListDialogL(TWizardSteps aNextEvent, 
                               TWizardSteps aBackEvent,
                               TInt aTitleResourceId, 
                               TInt aPreSelectedIndex,
                               TInt& aSelectionIndex);

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
   void ShowGenericInfoDialogL(TWizardSteps aNextEvent, 
                               TWizardSteps aBackEvent, 
                               TDialogTypeEvent aEvent);

   /**
    * Displays dialog, if aError is KErrNone the dialog displays
    * a list of contacts. If not KErrNone an error dialog should
    * be displayed.
    */
   void ShowContactBookDialogL();

   /**
    * Called by super class when ContactBkInUse is called
    * by the ContactBkEngine.
    */
   void ShowContactBookBusyDialogL();

   /**
    * Information dialog displaying information about the sms that has
    * been sent.
    * @param aStatus,  KErrNone if successful otherwise one of the other system
    *                  panic codes.
    * @param aCount,   Total number of emails to be sent.
    * @param aNbrSent, The number of emails sent.
    */
   void ShowSmsSentDialogL(TInt aStatus, TInt aCount, TInt aNbrSent);

   /**
    * Returns the current origin.
    * @return Current origin.
    */
   TPoint GetOrigin();

   /**
    * Returns the current destination.
    * @return Current destination.
    */
   TPoint GetDestination();

   /**
    * Returns the current position.
    * @return Current position.
    */
   TPoint GetCurrentPosition();

   /**
    * Returns the name of the current destination.
    * @return Name of the current destination.
    */
   const TDesC& GetCurrentRouteDestinationName();

   /**
    * Get the coordinate of the current postion in the map.
    * @return The map position.
    */
   TPoint GetCoordinate();

   /**
    * Gets the id of the current route.
    * @return The id of the current route.
    */
   TInt64 GetRouteId();

   /**
    * Calls wayfinder appui to send a GuiProtMess
    * to nav2.
    * @param aMessage, The message to send to nav2.
    */
   void SendMessageL(isab::GuiProtMess* aMessage);

   /**
    * Allocates the message to be sent, body in the sms.
    */
   void SetSignatureL();

   /**
    * Adds items to the iContentArray. This is done
    * in the sub classes since the base class cant
    * use resource ids from the rss file.
    * @param aResourceId, The id of the text resource to be added.
    */
   void AddItemToContentArray(TInt aResourceId);

   /**
    * Validates a phone nbr. 
    * @param aPhoneNbr, the phone nbr to validate.
    * @return, True if the phone nbr is valid
    *          False if not.
    */
   TBool IsValidPhoneNumber(const TDesC& aPhoneNbr);

   /**
    * Returns the text that is displayed in the blue note.
    * @return The info text that is displayed in the blue note in the map.
    */
   const char* GetMapMoverInfoText();

   /**
    * Function letting the sub class know that a sms is about
    * to be sent. Good when wanting to display an info message.
    */
   void SmsAboutToBeSent();

protected:

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
   void ShowGenericInfoDialogL(TWizardSteps aNextEvent, 
                               TWizardSteps aBackEvent, 
                               TInt aHeader, TInt aText, 
                               TInt aResourceId);


private:
   /// Reference to WayFinderAppUi.
   class CWayFinderAppUi& iAppUi;
};

#endif   //_COMMUNICATIONWIZARD_H_
