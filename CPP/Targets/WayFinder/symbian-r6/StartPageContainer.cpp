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
#include <gulicon.h>

#include "RsgInclude.h"
#include "wficons.mbg"

#include "StartPageContainer.h"
#include "WayFinderConstants.h"
#include "wayfinder.hrh"
#include "GuiDataStore.h"
#include "StartPageView.h"
#include "WFLayoutUtils.h"

#include "StartPageView.h"       // For StartPageMode enum

// Definitions
#define LISTBOX_POS     TPoint(10, 15)

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CStartPageContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CStartPageContainer::ConstructL( const TRect& aRect, 
                                      CStartPageView* aView )
{
   CreateWindowL();

   iView = aView;

   GuiDataStore* gds = iView->GetGuiDataStore();
   HBufC* wfmbmname = gds->iWayfinderMBMFilename;

   TResourceReader reader;
   TInt resource = R_WAYFINDER_STARTPAGE_LIST;
   switch (iMode) {
      case StartPageModeServices:
/*          resource = R_WAYFINDER_STARTPAGE_LIST; */
         break;
      case StartPageModeShareWf:
         resource = R_WAYFINDER_STARTPAGE_SHARE_WF_LIST;
         break;
      case StartPageModeCommunicate:
         resource = R_WAYFINDER_STARTPAGE_COMMUNICATE_LIST;
         break;
   }
   iCoeEnv->CreateResourceReaderLC( reader, resource );
   iListBox = new( ELeave ) CAknSingleLargeStyleListBox();
   LOGNEW(iListBox, CAknSingleLargeStyleListBox);
   iListBox->SetContainerWindowL(*this);
   iListBox->ConstructFromResourceL(reader);
   iListBox->SetListBoxObserver( this );
   CleanupStack::PopAndDestroy();  // Resource reader

   // Creates gul icon.
   CArrayPtr<CGulIcon>* icons = new( ELeave ) CAknIconArray( 3 );
   LOGNEW(icons, CAknIconArray);

   CleanupStack::PushL( icons );

#ifdef NAV2_CLIENT_SERIES60_V3
   AddToIconList(icons, *wfmbmname, EMbmWficonsSearch, EMbmWficonsSearch_mask);
   AddToIconList(icons, *wfmbmname, EMbmWficonsFavorites, EMbmWficonsFavorites_mask);
   TInt img = EMbmWficonsServices;
   TInt mask = EMbmWficonsServices_mask;
   switch (iMode){
      case StartPageModeServices:
      case StartPageModeShareWf:
         // Default
         break;
      case StartPageModeCommunicate:
/*          img = EMbmWficonsCommunicate; */
/*          mask = EMbmWficonsCommunicate_m; */
         break;
   }
   AddToIconList(icons, *wfmbmname, img, mask);
   AddToIconList(icons, *wfmbmname, EMbmWficonsMap, EMbmWficonsMap_mask);
#else
   // Appends graphic data.
   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
            EMbmWficonsSearch,
            EMbmWficonsSearch_m));
   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
            EMbmWficonsFavorites,
            EMbmWficonsFavorites_m));
   TInt img = EMbmWficons42x29_services;
   TInt mask = EMbmWficons42x29_services_m;
   switch (iMode){
      case StartPageModeServices:
      case StartPageModeShareWf:
         // Default
         break;
      case StartPageModeCommunicate:
         img = EMbmWficonsCommunicate;
         mask = EMbmWficonsCommunicate_m;
         break;
   }
   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname, img, mask));
   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
            EMbmWficonsMap,
            EMbmWficonsMap_m));
#endif
   // Sets graphics as ListBox icon.
   iListBox->ItemDrawer()->ColumnData()->SetIconArray( icons );

   CleanupStack::Pop(); //icons
   
   iListBox->SetExtent( LISTBOX_POS, iListBox->MinimumSize() );

   SetRect(aRect);
   ActivateL();

/*    SetList(iView->IsGpsAllowed()); */
}

// Destructor
CStartPageContainer::~CStartPageContainer()
{
   delete iListBox;
   LOGDEL(iListBox);
   iListBox = NULL;
   iView = NULL;
}

// Go to the selected page
void CStartPageContainer::GoToSelection()
{
   TInt command = -1;
   switch( iListBox->CurrentItemIndex() ){
   case 0:
      command = EWayFinderCmdMap;
      break;
   case 1:
      command = EWayFinderCmdStartPageNew;
      break;
   case 2:
      command = EWayFinderCmdMyDest;
      break;
   case 3:
      switch (iMode) {
         case StartPageModeCommunicate:
            command = EWayFinderCmdConnect;
            break;
         case StartPageModeShareWf:
            command = EWayFinderCmdServicesSpreadWayfinder;
            break;
         case StartPageModeServices:
            command = EWayFinderCmdService;
            break;
      }
      break;
   }
   if(command != -1){
      iView->HandleCommandL(command);
   }
}

// ----------------------------------------------------------------------------
// void CStartPageContainer::HandleListBoxEventL( CEikListBox*,
//  TListBoxEvent )
// Handles listbox event.
// ----------------------------------------------------------------------------
void CStartPageContainer::HandleListBoxEventL( CEikListBox* aListBox,
                                               TListBoxEvent aEventType )
{
   if( aListBox == iListBox &&
      (aEventType == EEventEnterKeyPressed ||
       aEventType == EEventItemClicked )){
      GoToSelection();
   }
}

// ---------------------------------------------------------
// CStartPageContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CStartPageContainer::SizeChanged()
{
   if ( iListBox ){
      iListBox->SetRect( Rect() ); // Sets rectangle of lstbox.
   }
}

// ---------------------------------------------------------
// CStartPageContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CStartPageContainer::CountComponentControls() const
{
   return 1; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CStartPageContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CStartPageContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case 0:
      return iListBox;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CStartPageContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CStartPageContainer::Draw(const TRect& aRect) const
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
// CStartPageContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CStartPageContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                               TCoeEvent /*aEventType*/ )
{
   // TODO: Add your control event handler code here
}

// ----------------------------------------------------------------------------
// TKeyResponse CMyFavoritesContainer::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
TKeyResponse CStartPageContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                                  TEventCode aType )
{
   if (aKeyEvent.iScanCode == EStdKeyEnter &&
      aType == EEventKeyUp) {
      GoToSelection();
      return EKeyWasConsumed;
   }
   if (aType != EEventKey){ // Is not key event?
      return EKeyWasNotConsumed;
   }

   switch ( aKeyEvent.iCode ) // The code of key event is...
   {
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

void CStartPageContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
}


#ifdef NAV2_CLIENT_SERIES60_V3
void CStartPageContainer::AddToIconList(CArrayPtr<CGulIcon>* aIcons, 
                                        const TDesC& aFilename, 
                                        TInt aIconId, 
                                        TInt aIconMaskId)
{
   CFbsBitmap* bitmap;
   CFbsBitmap* mask;
   AknIconUtils::CreateIconL(bitmap, 
                             mask, 
                             aFilename, 
                             aIconId, 
                             aIconMaskId);
   
   // Append the icon to icon array
   aIcons->AppendL(CGulIcon::NewL(bitmap, mask));
   
}

#endif

// End of File  
