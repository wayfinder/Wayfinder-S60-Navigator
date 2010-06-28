/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES
#include <eikclbd.h>
#include <barsread.h>  // for resource reader
#include <aknlists.h>  // for avrell style listbox
#include <akniconarray.h> // for icon array
#include <eikprogi.h>
#include <eiklabel.h>

#include "RsgInclude.h"
#include "wficons.mbg"

#include "IAPSearchGui_old.h"
#include "GuiProt/ServerEnums.h"
#include "TrialView.h"

#include "TrialContainer.h"

#include <barsread.h>    // for resource reader
#include <aknlists.h>    // for avrell style listbox
#include <eikprogi.h>    //CEikProgressInfo
#if defined NAV2_CLIENT_SERIES60_V2
# include <aknsdrawutils.h>
#endif

#include "wayfinder.hrh" //command ids
#include "RsgInclude.h"  //rsg-files
#include "memlog.h"      //LOGNEW ...
#include "TrialView.h"   //CTrialView


// #include <eikclbd.h>

// #include <akniconarray.h> // for icon array
//
// #include <eiklabel.h>

//
// #include "wficons.mbg"

//
// #include "GuiProt/ServerEnums.h"

// #include "WayFinderConstants.h"
// 
// #include "RectTools.h"
// 


// Definitions
#define LISTBOX_POS     TPoint(10, 15)

// Enumerations
enum TControls
{
   EListBox,
   ENumberControls,  
};

// ================= MEMBER FUNCTIONS =======================

CTrialContainer::CTrialContainer(class isab::Log* aLog) : 
   iLog(aLog), iShowTrial(ETrue) 
{}

// ---------------------------------------------------------
// CTrialContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CTrialContainer::ConstructL( const TRect& aRect, 
                                  class CTrialView* aView )
{
   CreateWindowL();

   iNormalTopLeft = aRect.iTl;

   iView = aView;

   // Create a periodic timer but don't start it yet
   iPeriodicTimer = CPeriodic::NewL(CActive::EPriorityStandard);
   LOGNEW(iPeriodicTimer, CPeriodic);
   SetTimerTick(1000);

   class TResourceReader reader;
   iCoeEnv->CreateResourceReaderLC( reader, R_WAYFINDER_TRIAL_LIST );
   iListBox = new( ELeave ) CAknDoubleStyleListBox();
   LOGNEW(iListBox, CAknDoubleStyleListBox);
   iListBox->SetContainerWindowL(*this);
   iListBox->ConstructFromResourceL(reader);
   iListBox->SetListBoxObserver( this );
   CleanupStack::PopAndDestroy(/*reader*/);  // Resource reader

   iListBox->SetExtent( LISTBOX_POS, iListBox->MinimumSize() );
   if (iView->SearchingForIAP()) {
      /* Don't show selection list. */
      iListBox->MakeVisible(EFalse);
   }
   ResetItems(iView->ShowTrialEntryInTrialView());

   SetRect(aRect);
   ActivateL();
}

// Destructor
CTrialContainer::~CTrialContainer()
{
   StopTimer();
   LOGDEL(iPeriodicTimer);
   delete iPeriodicTimer;
   iPeriodicTimer = NULL;

   LOGDEL(iListBox);
   delete iListBox;
   iListBox = NULL;
   iView = NULL;
}

void CTrialContainer::AppendItemAndUpdateBoxL(TInt aResource, 
                                              CDesCArray& aArray)
{
   HBufC* text = iCoeEnv->AllocReadResourceLC(aResource);
   aArray.AppendL( *text );
   iListBox->HandleItemAdditionL();
   CleanupStack::PopAndDestroy(text);
}

void
CTrialContainer::ResetItems(TBool showTrial)
{
   MDesCArray* itemList = iListBox->Model()->ItemTextArray();
   CDesCArray* itemArray = static_cast<CDesCArray*>( itemList );

   itemArray->Reset();
   iListBox->HandleItemRemovalL(); // Updates listbox.
   // Actually, the HandleItemRemovalL does not provoke a redraw...
   iListBox->Reset();

   if ((iShowTrial = showTrial)) {
      AppendItemAndUpdateBoxL(R_STARTMENU_TRIAL, *itemArray);
   }
   AppendItemAndUpdateBoxL(R_STARTMENU_REGISTER, *itemArray);
   AppendItemAndUpdateBoxL(R_STARTMENU_ALREADYREGISTERED, *itemArray);
}

void CTrialContainer::SetTimerTick(TTimeIntervalMicroSeconds32 aTick)
{
   iTick=aTick;
}

// ---------------------------------------------------------
// CTrialContainer::StartTimer() 
// Start the timer (required for animated TrialContainer)
// Optional but useful for static ones.
// ---------------------------------------------------------
//
void CTrialContainer::StartTimer()
{
   //If the timer is not already running, start it
   if (!iPeriodicTimer->IsActive()){
      iPeriodicTimer->Start( iTick, iTick,
                             TCallBack(CTrialContainer::Period, this) );
   }
}

// ---------------------------------------------------------
// CTrialContainer::StopTimer() 
// 
// ---------------------------------------------------------
//
void CTrialContainer::StopTimer()
{
   //Stop the timer if it is active
   if (iPeriodicTimer->IsActive()){
      iPeriodicTimer->Cancel();
   }
}

// This function is called by the periodic timer
TInt CTrialContainer::Period(TAny * aPtr)
{
   class CTrialContainer* container = static_cast<CTrialContainer*>(aPtr);
   container->DoPeriod();
   //returning a value of TRUE indicates the callback should be done again
   return TRUE;
}

// ---------------------------------------------------------
// CTrialContainer::DoPeriod() 
// Function that is called periodically by the timer.
// Call the child Tick() treatment and request a screen 
// redraw if needed.
// ---------------------------------------------------------
//
void CTrialContainer::DoPeriod()
{
   if (iView->DoShowIAPProgress()) {

      StopTimer();

      iView->CheckIAP();
      DrawNow();
   } else if (iView->DoShowTrialView()) {
      iListBox->MakeVisible(ETrue);

      StopTimer();
      iView->CheckGoToStart();
      DrawNow();
   } else {
      iListBox->MakeVisible(EFalse);
      /* Don't show anything. */
      StopTimer();
      iView->CheckGoToStart();
   DrawNow();
}
}

void CTrialContainer::ReportProgress(TInt /*aVal*/, TInt /*aMax*/)
{
   TBool showList;
   if (iView->DoShowIAPProgress()) {
      showList = EFalse;
   } else {
      showList = ETrue;
   }
   SizeChanged();
   DrawNow();
}

void CTrialContainer::AskForIAPDone()
{
   iListBox->MakeVisible(ETrue);
   DrawNow();
}

// Go to the selected page
void CTrialContainer::GoToSelection()
{
   TInt index = iListBox->CurrentItemIndex();
   if (!iShowTrial) {
      /* Trial entry is not shown, increase the index to compensate. */
      index++;
   }
   switch( index ){
   case 0:
      iView->HandleCommandL( EWayFinderCmdTrialTrial );
      break;
   case 1:
      iView->HandleCommandL( EWayFinderCmdTrialRegister );
      break;
   case 2:
      iView->HandleCommandL( EWayFinderCmdTrialReRegister );
      break;
   }
}

// ----------------------------------------------------------------------------
// void CTrialContainer::HandleListBoxEventL( CEikListBox*,
//  TListBoxEvent )
// Handles listbox event.
// ----------------------------------------------------------------------------
void CTrialContainer::HandleListBoxEventL(class CEikListBox* aListBox,
                                          enum TListBoxEvent aEventType )
{
   if( aListBox == iListBox && aEventType == EEventEnterKeyPressed ){
      GoToSelection();
   }
}

// ---------------------------------------------------------
// CTrialContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CTrialContainer::SizeChanged()
{
   if ( iListBox ){
      iListBox->SetRect( Rect() ); // Sets rectangle of listbox.
   }
}

// ---------------------------------------------------------
// CTrialContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CTrialContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CTrialContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CTrialContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case EListBox:
      return iListBox;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CTrialContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CTrialContainer::Draw(const TRect& /*aRect*/) const
{
   class CWindowGc& gc = SystemGc();

   gc.Clear();
}

// ----------------------------------------------------------------------------
// TKeyResponse CMyFavoritesContainer::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
enum TKeyResponse CTrialContainer::OfferKeyEventL(const struct TKeyEvent& aKeyEvent,
                                                  enum TEventCode aType )
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }

   switch ( aKeyEvent.iCode ){ // The code of key event is...
   // Switches tab.
   case EKeyLeftArrow: // Left key.
   case EKeyRightArrow: // Right Key.
      return EKeyWasNotConsumed;
   }
   if ( iListBox ){
      return iListBox->OfferKeyEventL( aKeyEvent, aType );
   } else{
      return EKeyWasNotConsumed;
   }
}

void CTrialContainer::SetTrialMessageL(const TDesC& aMessage)
{
   if (iShowTrial) {
      /* If trial entry isn't shown, then we shouldn't try to change it. */
      class CTextListBoxModel* model = iListBox->Model();
      MDesCArray* itemList = model->ItemTextArray();
      CDesCArray& array = *static_cast<CDesCArray*>(itemList);
      array.Delete(0);
      array.InsertL(0, aMessage);
   }
}

// End of File  
