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
#include <barsread.h>  // for resource reader
#include <aknpopup.h>
#include <aknlists.h>

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "WayFinderConstants.h"
#include "NewDestContainer.h"
#include "NewDestView.h"
#include "memlog.h"
#include "WFLayoutUtils.h"

#define LISTBOX_POS       TPoint(10, 15)

// Enumarations
enum TControls
{
   EResultList,
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CNewDestContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CNewDestContainer::ConstructL( const TRect& aRect, CNewDestView* aView )
{
   CreateWindowL();

   TResourceReader reader;

   iCoeEnv->CreateResourceReaderLC( reader, R_NEWDEST_LIST );
   iListBox = new( ELeave ) CAknDoubleStyleListBox();
   LOGNEW(iListBox, CAknDoubleStyleListBox);
   iListBox->SetContainerWindowL(*this);
   iListBox->ConstructFromResourceL(reader);
   iListBox->SetListBoxObserver( this );
   CleanupStack::PopAndDestroy();  // Resource reader
   iListBox->SetExtent(LISTBOX_POS, iListBox->MinimumSize());

   iListBox->CreateScrollBarFrameL(ETrue);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOn,
                                                        CEikScrollBarFrame::EAuto );
   iNumberDests = 0;

   // Create a periodic timer but don't start it yet
	iPeriodicTimer = CPeriodic::NewL(CActive::EPriorityStandard);
   LOGNEW(iPeriodicTimer, CPeriodic);

   /* Timeout each millisecond. */
   SetTimerTick(1000);

   iView = aView;
   SetRect( aRect );
   ActivateL();
   SetFocus( EFalse );
}

// Destructor
CNewDestContainer::~CNewDestContainer()
{
   StopTimer();
   LOGDEL(iPeriodicTimer);
   delete iPeriodicTimer;
   iPeriodicTimer = NULL;

   LOGDEL(iListBox);
   delete iListBox;
   iListBox = NULL;
}

void
CNewDestContainer::SetTimerTick(TTimeIntervalMicroSeconds32  aTick)
{
   iTick=aTick;
}

void
CNewDestContainer::StartTimer()
{
   //If the timer is not already running, start it
   if (!iPeriodicTimer->IsActive()){
      iPeriodicTimer->Start( iTick, iTick,
            TCallBack(CNewDestContainer::Period,this) );
   }
}

void
CNewDestContainer::StopTimer()
{
   //Stop the timer if it is active
   if (iPeriodicTimer->IsActive()){
      iPeriodicTimer->Cancel();
   }
}

// This function is called by the periodic timer
TInt
CNewDestContainer::Period(TAny * aPtr)
{
   ((CNewDestContainer*)aPtr)->DoPeriod();
   //returning a value of TRUE indicates the callback should be done again
   return TRUE;
}

// ---------------------------------------------------------
// Function that is called periodically by the timer.
// ---------------------------------------------------------
void
CNewDestContainer::DoPeriod()
{
   iView->TimerTick();
}

// ----------------------------------------------------------------------------
// void CNewDestContainer::AddItemL( TBuf<KSearchViewTextBufLength> aItem )
// Adds list item at the top of the list.
// ----------------------------------------------------------------------------
//
void CNewDestContainer::AddItemL( TUint /*aId*/, TDesC &aListItem )
{
   if ( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );

      TBuf<KBuf256Length> item = _L(" ");
      item.Append(aListItem);

      // Adds items at the end of the list.
      itemArray->AppendL( item );

      iListBox->HandleItemAdditionL(); // Updates listbox.
      iNumberDests++;
   }
}

// ----------------------------------------------------------------------------
// void CNewDestContainer::GetSelItemL( TDesC &aItem )
// Get the selected item.
// ----------------------------------------------------------------------------
//
TBool CNewDestContainer::GetSelItem( TDes& aItem )
{
   TBool gotItem = EFalse;
   if ( iListBox ){
      TInt currentItem( GetSelIndex() );
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray *itemArray = STATIC_CAST( CDesCArray*, itemList );

      // Removes an item at pos.
      TPtrC ptr = itemArray->MdcaPoint(currentItem);
      aItem.Copy( ptr );
      gotItem = ETrue;
   }
   return gotItem;
}

// ----------------------------------------------------------------------------
// void CNewDestContainer::RemoveAllItemsL()
// Remove all the item.
// ----------------------------------------------------------------------------
//
TBool CNewDestContainer::RemoveAllItemsL()
{
   if ( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray *itemArray = STATIC_CAST( CDesCArray*, itemList );

      // Removes an item at pos.
      itemArray->Reset();
      iIds.Reset();

      iNumberDests = 0;

      iListBox->HandleItemRemovalL();
      iListBox->Reset();

      iListBox->DrawNow(); // Updates listbox.
   }
   return true;
}

TBool CNewDestContainer::HasResults()
{
   return iNumberDests > 0;
}

TInt CNewDestContainer::GetSelIndex()
{
   return iListBox->CurrentItemIndex();
}

void CNewDestContainer::SetSelected(TInt index)
{
   if ( iListBox ){
      /* Get number of elements in list. */
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      TInt numElements = itemList->MdcaCount();

      if (index >= numElements) {
         /* Set the last item. */
         iListBox->SetCurrentItemIndexAndDraw(numElements-1);
      } else {
         iListBox->SetCurrentItemIndexAndDraw(index);
      }
   }
}

// ---------------------------------------------------------
// CNewDestContainer::OfferKeyEventL()
// Called by framework when a key event occurs.
// ---------------------------------------------------------
//
TKeyResponse CNewDestContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }
	// Use the arrow keys to move focus between the controls.
   switch (aKeyEvent.iScanCode)
   {
   case EStdKeyDownArrow:
   case EStdKeyUpArrow:
      iListBox->OfferKeyEventL( aKeyEvent, aType );
      return EKeyWasConsumed;
      break;
   case EStdKeyLeftArrow:
      //ShowResultList( EFalse );
      iView->HandleCommandL( EWayFinderCmdNewDestNewSearch );
      return EKeyWasConsumed;
      break;
   case EStdKeyDevice3:
      if( iView->SettingOrigin() ) {
         iView->HandleCommandL( EWayFinderCmdNewDestOrigin );
      } else if( iView->SettingDestination() ) {
         iView->HandleCommandL( EWayFinderCmdNewDestDestination );
      } else {
         iView->HandleCommandL( EWayFinderCmdNewDestNavigate );
      }
      break;
   }
   return EKeyWasNotConsumed;
}

// ----------------------------------------------------------------------------
// void CNewDestContainer::HandleListBoxEventL( CEikListBox*,
//  TListBoxEvent )
// Handles listbox event.
// ----------------------------------------------------------------------------
void CNewDestContainer::HandleListBoxEventL( CEikListBox* aListBox,
                                             TListBoxEvent aEventType )
{
   if( aListBox == iListBox && aEventType == EEventEnterKeyPressed ){
   }
}

// ---------------------------------------------------------
// CNewDestContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CNewDestContainer::SizeChanged()
{
   if ( iListBox ){
      iListBox->SetRect( Rect() ); // Sets rectangle of lstbox.
   }
}

// ---------------------------------------------------------
// CNewDestContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CNewDestContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CNewDestContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CNewDestContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case EResultList:
      return iListBox;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CNewDestContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CNewDestContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   // TODO: Add your drawing code here
   // example code...
   gc.SetPenStyle(CGraphicsContext::ENullPen);
   gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);
}

// ---------------------------------------------------------
// CNewDestContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CNewDestContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                             TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

void CNewDestContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }   
}

// End of File  
