/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CS_MAIN_CONTAINER_H
#define CS_MAIN_CONTAINER_H

#include <aknform.h>

#include "CSMainView.h"

class MEikCommandObserver;

/**
 * @class	CCSMainContainer.h
 */
class CCSMainContainer : public CAknForm// , 
//                          public MCoeControlObserver
{
public:
   /**
    * Creates an instance and initializes it.
    * Instance is not left on cleanup stack.
    * @param aCommandObserver command observer
    * @param aView
    * @param aSavedData
    * @return initialized instance of CCSMainContainer
    */
   static CCSMainContainer* NewL( MEikCommandObserver* aCommandObserver,
                                  class CCSMainView& aView, 
                                  CCSMainView::TFormData& aSavedData );

   /**
    * Creates an instance and initializes it.
    * Instance is left on cleanup stack.
    * @param aCommandObserver command observer
    * @param aView
    * @param aSavedData
    * @return new instance of CCSMainContainer
    */
   static CCSMainContainer* NewLC( MEikCommandObserver* aCommandObserver,
                                   class CCSMainView& aView, 
                                   CCSMainView::TFormData& aSavedData );

   /** 
    * Destroy any instance variables
    */
   virtual ~CCSMainContainer();

protected:
   /**
    * Construct the CCSMainContainer instance
    * @param aCommandObserver command observer
    * @param aView
    * @param aSavedData
    */ 
   CCSMainContainer( MEikCommandObserver* aCommandObserver, 
                     class CCSMainView& aView, 
                     CCSMainView::TFormData& aSavedData );

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
   TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, 
                                TEventCode aType );
	
   /** 
    * Handle global resource changes, such as scalable UI or skin events
    */
   void HandleResourceChange( TInt aType );

   void HandlePointerEventL(const TPointerEvent& aPointerEvent);

   // from base class MCoeControlObserver
   
    /**
     * From MCoeControlObserver.
     * Handles an event of type aEventType
     * @param aControl Pointer to component control
     * @param aEventType Event code
     */
//     void HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType );
protected:

   /**
    *
    */
   void HandleControlEventL(CCoeControl *aControl, 
                            TCoeEvent aEventType);

   /**
    * Initialize controls and settings before a Form is laid out.
    */	
   void PreLayoutDynInitL();

   /**
    *
    */
   void ProcessCommandL( TInt aCommandId );

   /**
    * Handle a button press and tell whether it closes the dialog.
    * @param aButtonId the identifier for the button (avkon.hrh)
    */
   TBool OkToExitL( TInt aButtonId );

   /**
    *
    */
   void SetInitialCurrentLine();

   /**
    *
    */
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

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

   /**
    *
    */
   void SetFieldsFromGps( TBool aFromGps );

   /**
    *
    */
   void SetupSearchMethodFieldL();
   
   /**
    *
    */
   TBool CompareToResource( const TDesC& aString, TInt aResourceId );

   /**
    *
    */
   void SwitchViewL(TInt aViewId);

   /**
    *
    */
   void SwitchViewL(TInt aViewId, TInt aCustomMessageId, 
                    const TDesC8 &aCustomMessage);

public:
   /**
    *
    */
   void SetWhat( const TDesC& aValue );

   /**
    *
    */
   void SetWhere( const TDesC& aValue );

   /**
    *
    */
   void SetCountry( const TDesC& aValue );

   /**
    *
    */
   void InitSearchFields(TBool aIgnoreWhereField = EFalse);

   /**
    *
    */
   void SaveDataL();

   /**
    * Disables the editor indicators for the edwins so our status text in the
    * navi pane will not be overwritten.
    */
   void DisableEditorIndicators();

   /**
    * Enables the editor indicators for the edwins.
    */
   void EnableEditorIndicators();

   void SetUpdate(TBool aState);
private:
   class CCSMainView& iView;
   MEikCommandObserver* iCommandObserver;

   class CEikEdwin* iESearchWhat;
   class CEikEdwin* iESearchWhere;
   class CEikEdwin* iESearchCountry;

   CCSMainView::TFormData& iSavedData;
   TBool iUpdate;
};

#endif // STESTFORM_H
