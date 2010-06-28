/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef QUERY_DIALOG_H
#define QUERY_DIALOG_H

#include <aknmessagequerydialog.h> 

/**
 * Dialog that gives a message to the user. Often used
 * when user has a choise to make.
 */
class CQueryDlg : public CAknMessageQueryDialog
{
   CQueryDlg(const TDesC& aTitle,
             const TDesC& aText, 
             TInt aTimeOut);

   /**
    * Virtual destructor. 
    */
   virtual ~CQueryDlg();

   /**
    * Inherited from base class.
    * @param aButtonId The id of the pressed button.
    * @return ETrue if user pressed yes or no.
    *         EFalse if user pressed any other button.
    */
   virtual TBool OkToExitL(TInt aButtonId);

   /**
    * Inherited from base class. When user presses a button
    * this function gets called. This function does also gets 
    * called when user closes the slider or the flip on the phone.
    * The KeyCode for this is EKeyEscape. By catching this event
    * we can prevent that the dialog closes when user does this.
    * @param aKeyEvent Information about the button that was pressed.
    * @param aType What kind of event up, down or event.
    * @return EKeyWasConsumed if we swallowed the event.
    *         EKeyWasNotConsumed if we want the system to handle it.
    */
   virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                       TEventCode aType);

public:

   /**
    * Static function that creates a CAknMessageQueryDialog
    * with a title and a message.
    * @param aDlgResId The id of dialog resource, from the .rss file.
    * @param aTitle The title of the dialog.
    * @param aText The text in the dialog.
    * @return The button id for the pressed button.
    */
   static TInt RunDlgLD(const TInt aDlgResId,
                        const TDesC& aTitle,
                        const TDesC& aText, 
                        TInt aTimeOut = -1);

private:
   static TInt HideDialogCallback(TAny* aAny);

private:
   class CPeriodic* iPeriodic;
};


#endif
