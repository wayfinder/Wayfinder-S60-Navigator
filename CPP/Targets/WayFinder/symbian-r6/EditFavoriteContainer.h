/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef EDIT_FAVORITE_CONTAINER_H
#define EDIT_FAVORITE_CONTAINER_H

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
// s60v3fp2 and s60v5 only class

#include <aknform.h>
#include <aknlocationed.h>

#include "EditFavoriteView.h"

namespace isab {
   class Favorite;
}

class MEikCommandObserver;

/**
 * @class CEditFavoriteContainer.h
 */
class CEditFavoriteContainer : public CAknForm
{
public:
   /**
    * Creates an instance and initializes it.
    * Instance is not left on cleanup stack.
    * @param aCommandObserver Command observer
    * @param aView The owning view
    * @return Initialized instance of CEditFavoriteContainer
    */
   static CEditFavoriteContainer* NewL(MEikCommandObserver* aCommandObserver,
                                  class CEditFavoriteView& aView);

   /**
    * Creates an instance and initializes it.
    * Instance is left on cleanup stack.
    * @param aCommandObserver Command observer
    * @param aView The owning view
    * @return Initialized instance of CEditFavoriteContainer
    */
 
   static CEditFavoriteContainer* NewLC(MEikCommandObserver* aCommandObserver,
                                   class CEditFavoriteView& aView);

   /** 
    * Destroy any instance variables
    */
   virtual ~CEditFavoriteContainer();

protected:
   /**
    *
    */
   CEditFavoriteContainer(MEikCommandObserver* aCommandObserver, 
                          class CEditFavoriteView& aView);

private:
   /**
    *
    */
   void ConstructL();

public:
   /** 
    * Handle key event (override)
    * @param aKeyEvent key event
    * @param aType event code
    * @return EKeyWasConsumed if the event was handled, else EKeyWasNotConsumed
    */
   TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                               TEventCode aType);

   /** 
    * Handle global resource changes, such as scalable UI or skin events
    */
   void HandleResourceChange(TInt aType);

protected:
   /**
    * Initialize controls and settings before a Form is laid out.
    */
   void PreLayoutDynInitL();

   /**
    * Handles the commands in a form / dialog.
    */
   //void ProcessCommandL(TInt aCommandId);

   /**
    * Handle a button press and tell whether it closes the dialog.
    * @param aButtonId the identifier for the button (avkon.hrh)
    */
   TBool OkToExitL(TInt aButtonId);

   /**
    *
    */
   void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

   /**
    * Called to handle "Save" menu item. Displays save query.
    * @return TBool ETrue if the form data is to be saved, EFalse otherwise
    */
   TBool QuerySaveChangesL();

   /**
    * Called from QuerySaveChangesL when changes made to the form are to be saved.
    * @return TBool ETrue if the form data has been saved, EFalse otherwise
    */
   TBool SaveFormDataL(); 

   /**
    * Called from QuerySaveChangesL when changes made to the form are discarded.
    */
   void DoNotSaveFormDataL();

private:
   /**
    * Called from DoNotSaveFormDataL when changes are cancelled.
    * Called from PreLayoutDynInitL to load initial values if needed.
    */
   void LoadFromDataL();

public:
   /**
    *
    */
   void SetFavoriteDataL(class isab::Favorite& aFav);

   /**
    *
    */
   void SaveFavoriteDataL();

private: 
   class CEditFavoriteView& iView;
   MEikCommandObserver* iCommandObserver;

   class CEikEdwin* iNameEdwin;
   class CEikEdwin* iDescEdwin;
   class CAknLocationEditor* iLatEditor;
   class CAknLocationEditor* iLonEditor;

   /// Local copy of the favorite to be edited.
   class isab::Favorite* iFav;	
};

#endif // NAV2_CLIENT_SERIES60_V5 || NAV2_CLIENT_SERIES60_V32

#endif // EDIT_FAVORITE_CONTAINER_H
