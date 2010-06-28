/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TRIALVIEW_H
#define TRIALVIEW_H

#include "ViewBase.h"
#include "Log.h"
#include "memlog.h"
#include "ProgressDlgObserver.h"

#include "GuiProt/ServerEnums.h"
#include "IAPSearchGui_old.h"

namespace isab{
   class Log;
}

/**
*  CTrialView view class.
* 
*/
class CTrialView : 
   public CViewBase,
   public IAPSearchGui_old,
   public MProgressDlgObserver
{

   /** @name Constructors and destructor */
   //@{

   CTrialView(class isab::Log* aLog);

   /**
    * EPOC default constructor.
    */
   void ConstructL(class CWayFinderAppUi* aWayFinderUI);
public:
   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CTrialView object.
   ///        Note that this object is still on the CleanupStack.
   static class CTrialView* NewLC(class CWayFinderAppUi* aUi,
                                  class isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CTrialView object.
   static class CTrialView* NewL(class CWayFinderAppUi* aUi, 
                                 class isab::Log* aLog);

   /**
    * Destructor.
    */
   virtual ~CTrialView();
   //@}
public:
   // New functions
   void SetReRegisterIsUpgrade(TBool aReRegisterIsUpgrade = ETrue);
   ///Handles replies from ReRegister requests. 
   ///@param type the Servers idea of the
   ///            wayfinder type this client should have.
   void  ReRegisterComplete(enum isab::GuiProtEnums::WayfinderType type,
                            TBool verboseError = ETrue);
   ///Handles failures of ReRegister requests. 
   void  ReRegisterFailed();

   void ShowReRegisterResult(enum isab::GuiProtEnums::WayfinderType type,
                             TBool verboseError = ETrue);

   void HandleExpireVectorL(const TInt32* data, TInt32 numEntries);

   TBool DoShowTrialView();
   TBool DoShowIAPProgress();
   TBool SearchingForIAP();
   void CheckIAP();
   void AskForIAP();
   void AskForIAPDone();
   void SearchingForIAP(TBool aOn);
   void ReportProgress(TInt aVal, TInt aMax, HBufC* aName);

   void CheckGoToStart();
   TBool ShowTrialEntryInTrialView();
   void StartTrial();

public: // Functions from base classes

   /**
    * From AknView.
    * @return the ID of the view.
    */
   TUid Id() const;

   /**
    * From AknView, takes care of command handling.
    * @param aCommand command to be handled
    */
   void HandleCommandL(TInt aCommand);

   /**
    * From AknView, handles a change in the view.
    */
   void HandleClientRectChange();

   /**
    * From MProgressDlgObserver
    */
   void ProgressDlgAborted();

private: // Functions from base classes

   /**
    * From AknView, Called when the option softkey is pressed.
    * @param aResourceId the focused resource.
    * @param aMenuPane the current menu.
    */
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoActivateL( const TVwsViewId& aPrevViewId,
                     TUid aCustomMessageId,
                     const TDesC8& aCustomMessage);

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoDeactivate();

private: // Data

   /// A pointer to the creating UI.
   class CWayFinderAppUi*   iWayfinderAppUi;
   
   /// The controls container.
   class CTrialContainer* iContainer;
   class CProgressDlg* iProgressDlg;

   TInt iMode;

   class isab::Log* iLog;
   TBool iInProgress;
   TBool iSearchingForIAP;
   TBool iAskForIAP;
   HBufC* iTrialMessage;
   TBool iReRegisterIsUpgrade;
};

#endif

// End of File
