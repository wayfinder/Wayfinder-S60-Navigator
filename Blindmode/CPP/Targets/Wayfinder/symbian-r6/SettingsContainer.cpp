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
#include <akntabgrp.h>

#include "RsgInclude.h"
#include "wficons.mbg"

#include "SettingsContainer.h"
#include "SettingsListBox.h"
#include "SettingsData.h"
#include "WayFinderConstants.h"
#include "wayfinder.hrh"

#include "memlog.h"
#include "WFLayoutUtils.h"
// Definitions

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSettingsContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSettingsContainer::ConstructL(const TRect& aRect, CSettingsData* aData )
{
   CreateWindowL();

/*    TResourceReader reader; */
/*    iCoeEnv->CreateResourceReaderLC( reader, R_WAYFINDER_SETTINGS_LIST ); */
   iListBox = new( ELeave ) CSettingsListBox(iRelease, 
                                             iOneLanguage, 
                                             ILOG_POINTER, 
                                             aData);
   LOGNEW(iListBox, CSettingsListBox);
   iData = aData;
   iListBox->SetMopParent(this);
/*    CleanupStack::PopAndDestroy();  // Resource reader */
   iRect = aRect;
   ReloadListBox();
}

// Destructor
CSettingsContainer::~CSettingsContainer()
{
   LOGDEL(iListBox);
   delete iListBox;
   iListBox = NULL;
}

void CSettingsContainer::ChangeSelectedSetting()
{
   CEikFormattedCellListBox* listBox = iListBox->ListBox();
   iListBox->EditItemL( listBox->CurrentItemIndex(), ETrue );  
}

void CSettingsContainer::StoreSettingsL()
{
   iListBox->StoreSettingsL();
}

// ---------------------------------------------------------
// CSettingsContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSettingsContainer::SizeChanged()
{
   if ( iListBox ){
      iListBox->HandleChangeInItemArrayOrVisibilityL();
      iListBox->SetRect(iRect);
   }
}

// ---------------------------------------------------------
// CSettingsContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSettingsContainer::CountComponentControls() const
{
   if( iListBox )
      return iListBox->CountComponentControls(); // return nbr of controls inside this container
   else
      return 0;
}

// ---------------------------------------------------------
// CSettingsContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSettingsContainer::ComponentControl(TInt aIndex) const
{
   if( iListBox )
      return iListBox->ComponentControl( aIndex );
   else
      return NULL;
}

// ---------------------------------------------------------
// CSettingsContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CSettingsContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   // TODO: Add your drawing code here
   // example code...
   gc.SetPenStyle(CGraphicsContext::ENullPen);
   gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);
   iListBox->DrawNow();
}

// ---------------------------------------------------------
// CSettingsContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSettingsContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                               TCoeEvent /*aEventType*/ )
{
   // TODO: Add your control event handler code here
}



void CSettingsContainer::ReloadListBox()
{
   TInt listId=R_WAYFINDER_SETTINGS_LIST_PHONE;

   switch (TWayFinderSettingsTabViewId(iTabGroup->ActiveTabId())) {
      case EWayFinderSettingsVicinityTab:
         listId = R_WAYFINDER_SETTINGS_LIST_VICINITY;
         break;
      case EWayFinderSettingsMiscTab:
         listId = R_WAYFINDER_SETTINGS_LIST_PHONE;
         break;
      case EWayFinderSettingsRouteTab:
         listId = R_WAYFINDER_SETTINGS_LIST_ROUTE;
         break;
      case EWayFinderSettingsCommunicateTab:
         listId = R_WAYFINDER_SETTINGS_LIST_COMMUNICATE;
         break;
      case EWayFinderSettingsMapTab:
         listId = R_WAYFINDER_SETTINGS_LIST_MAP;
         break;
      case EWayFinderSettingsTrackingTab:
         listId = R_WAYFINDER_SETTINGS_LIST_TRACKING;
         break;
      case EWayFinderSettingsDebugTab:
         listId = R_WAYFINDER_SETTINGS_LIST_DEBUG;
         break;
   }
   iListBox->ConstructFromResourceL(listId);
   iListBox->MakeVisible(ETrue);
   iListBox->ActivateL();
   iListBox->SetRect(iRect);
   iListBox->DrawNow();
}


void CSettingsContainer::MaybeShowSecretTab()
{
   if (iRelease)
      return;
   if (iData->showDebug)
      return;
   ++iSecret;
   if (iSecret >=3) {
      iData->showDebug = 1;
   }
}



// ----------------------------------------------------------------------------
// TKeyResponse CMyFavoritesContainer::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
TKeyResponse CSettingsContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                                  TEventCode aType )
{
   TKeyResponse respons = EKeyWasNotConsumed;
   if ( aType == EEventKey ){ // Is not key event?
      switch ( aKeyEvent.iCode ) // The code of key event is...
      {
         // Switches tab.
         case EKeyLeftArrow: // Left key.
            {
               TInt current = iTabGroup->ActiveTabIndex();
               if (current > 0) {
                  StoreSettingsL();
                  --current;
                  iTabGroup->SetActiveTabByIndex(current);
                  ReloadListBox();
               } else {
                  MaybeShowSecretTab();
               }
            }
            return EKeyWasConsumed;
         case EKeyRightArrow: // Right Key.
            {
               TInt current = iTabGroup->ActiveTabIndex();
               TInt count   = iTabGroup->TabCount();
               if ( (current+1) < count) {
                  StoreSettingsL();
                  ++current;
                  iTabGroup->SetActiveTabByIndex(current);
                  ReloadListBox();
               }
            }
            return EKeyWasConsumed;
      }
      if ( iListBox ){
         respons = iListBox->OfferKeyEventL( aKeyEvent, aType );
         if (respons != EKeyWasNotConsumed ) 
            return respons;
      }
   }
   return EKeyWasNotConsumed;
}

void CSettingsContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      iRect = WFLayoutUtils::GetMainPaneRect();
      SetRect(iRect);
   }   
}

// End of File  
