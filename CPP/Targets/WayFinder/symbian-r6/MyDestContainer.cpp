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
#include <aknlists.h>  // for avrell style listbox
#include <aknsfld.h>   // findbox
#include <barsread.h>  // for resource reader
#include <aknpopup.h>  // for pop up menu
#include <akniconarray.h>
#include <aknsdrawutils.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <eikclbd.h> 
#include <eikfrlb.h> 

#if defined NAV2_CLIENT_SERIES60_V5
# include <aknpreviewpopupcontroller.h>
# include <aknstyluspopupmenu.h>
#endif

#include "RsgInclude.h"
#include "MyDestContainer.h"
#include "MyDestDialog.h"
#include "MyDestView.h"
#include "WayFinderAppUi.h"
#include "wayfinder.hrh"
#include "WFLayoutUtils.h"
#include "WFTextUtil.h"
#include "wficons.mbg"
#include "GuiProt/Favorite.h"
#include "GfxUtility.h"
#include "MC2Coordinate.h"
#include "GuiProt/SearchPrintingPolicyFactory.h"
#include "PreviewPopUpContent.h"
#include "EditFavoriteView.h"

/* #include "IsabTypes.h" */
#include "memlog.h"

// Definitions
#define LISTBOX_POS       TPoint(10, 15)
#define ICON_SIZE TSize(10, 10)

// Enumerations
enum TControls
{
   EListBox,
   EFindBox,
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================

CMyDestContainer::CMyDestContainer(isab::Log* aLog)
   : 
      iListBox (NULL),
      iFindBox (NULL),
      iLog(aLog)
{
   iShowingPreviewController = EFalse;
}


// ---------------------------------------------------------
// CMyDestContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CMyDestContainer::ConstructL( const TRect& aRect, CMyDestView* aView )
{
   CreateWindowL();

   iNumberDests = 0;
   iView = aView;

   iBgContext = 
      CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain,
                                               aRect, ETrue);

   TResourceReader reader;
   if (aView->GetWayFinderUI()->IsTrialVersion()) {
      iCoeEnv->CreateResourceReaderLC( reader, R_WAYFINDER_MYDEST_LIST_NO_SYNCHRONIZE );
   } else {
      iCoeEnv->CreateResourceReaderLC( reader, R_WAYFINDER_MYDEST_LIST );
   }


#if defined NAV2_CLIENT_SERIES60_V5
   TRect contentRect(aRect.iTl, TSize(aRect.Width(), aRect.Height() / 3));
   iPreviewContent = CPreviewPopUpContent::NewL();
   iPreviewController = 
      CAknPreviewPopUpController::NewL(*iPreviewContent, 
                                       CAknPreviewPopUpController::EPermanentMode |
                                       CAknPreviewPopUpController::ELayoutDefault);
   iPreviewContent->InitialiseL(aRect,
                                   iView->GetMbmName(),
                                   4,
                                   EMbmWficonsFavorites,
                                   EMbmWficonsFavorites_mask);
   iPreviewController->SetPopUpShowDelay(0);
   iPreviewController->ShowPopUp();
   TSize previewSize = iPreviewController->Size();
   TInt borderPadding = (previewSize.iWidth - contentRect.Width()) / 2;
   if (WFLayoutUtils::LandscapeMode()) {
      iPreviewContent->
         SetAvailableWidth(contentRect.Width() - borderPadding * 2, 
                           borderPadding,
                           CPreviewPopUpContent::EFourLinesTextImageTopLeftAbove);
   } else {
      iPreviewContent->
         SetAvailableWidth(contentRect.Width() - borderPadding * 2, 
                           borderPadding,
                           CPreviewPopUpContent::EFourLinesIndentedTextImageTopLeft);
   }
   //iPreviewContent->SetTextL(KFirstText, KSecondText, KThirdText);
   iPreviewController->UpdateContentSize();
   iPreviewController->SetPosition(aRect.iTl);
   iPreviewController->HidePopUp();
#endif

#if defined NAV2_CLIENT_SERIES60_V5
   iListBox = new( ELeave ) CAknSingleGraphicStyleListBox();
   iLongTapDetector = CAknLongTapDetector::NewL( this );
#else
   iListBox = new( ELeave ) CAknDoubleGraphicStyleListBox();
#endif

   iListBox->SetContainerWindowL(*this);
   iListBox->ConstructFromResourceL(reader);
   iListBox->SetListBoxObserver( this );
   CleanupStack::PopAndDestroy();  // Resource reader
   //iListBox->SetExtent(LISTBOX_POS, iListBox->MinimumSize());

   iListBox->CreateScrollBarFrameL(ETrue);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOn,
         CEikScrollBarFrame::EAuto);
#if defined NAV2_CLIENT_SERIES60_V5
   // Enable text scrolling when text is truncated
   iListBox->ItemDrawer()->ColumnData()->SetMarqueeParams(5, 6, 1000000, 200000);
   iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
#else
   // Enable text scrolling when text is truncated
   iListBox->ItemDrawer()->FormattedCellData()->SetMarqueeParams(5, 6, 1000000, 200000);
   iListBox->ItemDrawer()->FormattedCellData()->EnableMarqueeL(ETrue);
#endif 
   // Gets pointer of CAknFilteredTextListBoxModel.
   CAknFilteredTextListBoxModel* model =
      STATIC_CAST( CAknFilteredTextListBoxModel*, iListBox->Model() );
   // Creates FindBox.
   iFindBox = CAknSearchField::NewL( *this,
                                     CAknSearchField::EPopup, //XXX ESearch
                                     NULL,
                                     KMyDestFindBoxTextLength );
   LOGNEW(iFindBox, CAknSearchField);
   CleanupStack::PushL(iFindBox);
   // Creates CAknListBoxFilterItems class.
   model->CreateFilterL( iListBox, iFindBox );
   //Filter can get by model->Filter();
   CleanupStack::Pop(iFindBox); // findbox

   iFindBox->MakeVisible( EFalse );
   //iFindBox->SetFocus(ETrue);

   iIcons = new (ELeave) CAknIconArray(ETotalNbrIcons);
   CFbsBitmap* bitmap;
   CFbsBitmap* mask;
   AknIconUtils::CreateIconL( bitmap, mask, iView->GetMbmName(), 
                              EMbmWficonsFavorites, EMbmWficonsFavorites_mask );
   AknIconUtils::SetSize( bitmap, WFLayoutUtils::CalculateSizeUsingMainPane(ICON_SIZE), 
                          EAspectRatioPreservedAndUnusedSpaceRemoved );
   iIcons->InsertL(EFavoriteIconIndex, CGulIcon::NewL(bitmap, mask));

   AknIconUtils::CreateIconL( bitmap, mask, iView->GetMbmName(), 
                              EMbmWficonsFavoriteslms, EMbmWficonsFavoriteslms_mask );
   AknIconUtils::SetSize( bitmap, WFLayoutUtils::CalculateSizeUsingMainPane(ICON_SIZE), 
                          EAspectRatioPreservedAndUnusedSpaceRemoved );
   iIcons->InsertL(ELandmarkIconIndex, CGulIcon::NewL(bitmap, mask));

   iListBox->ItemDrawer()->ColumnData()->SetIconArray( iIcons );

   SetRect(aRect);

   ActivateL();
}

// Destructor
CMyDestContainer::~CMyDestContainer()
{
   if( iListBox ){
      LOGDEL(iListBox);
      delete iListBox;
      iListBox = NULL;
   }
   if( iFindBox ){
      LOGDEL(iFindBox);
      delete iFindBox;
      iFindBox = NULL;
   }
#if defined NAV2_CLIENT_SERIES60_V5
   delete iPreviewContent;
   delete iPreviewController;
   delete iLongTapDetector;
   delete iPopUpMenu;
#endif 
   delete iBgContext;
}

void CMyDestContainer::SetFindBoxFocus(TBool aFocus)
{
   if (iFindBox) {
      iFindBox->SetFocus(aFocus);
   }
}

// ----------------------------------------------------------------------------
// TBool CMyDestContainer::IsListEmpty()
// Returns true if the list is empty.
// ----------------------------------------------------------------------------
//
TBool CMyDestContainer::IsListEmpty()
{
   return (iNumberDests == 0 || (GetSelectedIndex() < 0));
}

// ----------------------------------------------------------------------------
// TBool CMyDestContainer::GetSelectedIndex()
// Returns the index of the selected favorite.
// ----------------------------------------------------------------------------
//
TInt CMyDestContainer::GetSelectedIndex()
{
   CAknFilteredTextListBoxModel* model =
   STATIC_CAST( CAknFilteredTextListBoxModel*, iListBox->Model() );
   CAknListBoxFilterItems* filter = model->Filter();
   TInt index = iListBox->CurrentItemIndex();
   if (index < 0) {
      return -1;
   } else {
      return filter->FilteredItemIndex( index );
   }
}

// ----------------------------------------------------------------------------
// TBool CMyDestContainer::SetNumberFavorites()
// Set the number of favorites.
// ----------------------------------------------------------------------------
//
void CMyDestContainer::SetNumberFavorites( TInt aNbrFavs )
{
   iNumberDests = aNbrFavs;
}

// ----------------------------------------------------------------------------
// void CMyDestContainer::ShowDialogL( TBool aNew )
// Adds list item at the top of the list.
// ----------------------------------------------------------------------------
//
TBool CMyDestContainer::ShowDialogL( TDes &aName, 
                                     TDes &aDescription, 
                                     TDes &aCategory,
                                     TDes &aAlias,
                                     TInt32 &aLat, TInt32 &aLon, TBool &aCanceled )
{
   CMyDestDialog* dialog = CMyDestDialog::NewL(iLog);
   dialog->SetName( &aName );
   dialog->SetDescription( &aDescription );
   dialog->SetCategory( &aCategory );
   dialog->SetAlias( &aAlias );

   dialog->SetLatitude( &aLat );
   dialog->SetLongitude( &aLon );

   if( dialog->ExecuteLD( R_WAYFINDER_MYDEST_DIALOG ) ){
      aCanceled = EFalse;
      if( aName.Compare( KEmpty ) != 0 ){
         return ETrue;
      }
   } else {
      aCanceled = ETrue;
   }
   return EFalse;
}

// ----------------------------------------------------------------------------
// void CMyDestContainer::AddItemL()
// Adds list item to the list.
// ----------------------------------------------------------------------------
//
void CMyDestContainer::AddItemL(const TDesC& aListItem, 
                                enum TFavIconEnum aIconIndex)
{
   if ( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );

      HBufC* text = HBufC::NewLC(aListItem.Length() + 8);
      TPtr textPtr = text->Des();
      textPtr.AppendNum(aIconIndex);
      textPtr.Append(KTab);
      textPtr.Append(aListItem);
      itemArray->AppendL(*text);
      CleanupStack::PopAndDestroy(text);
      iNumberDests++;
      iListBox->HandleItemAdditionL(); // Updates listbox.
   }
}

void CMyDestContainer::RemoveItemL(TInt aIndex)
{
   CDesCArray* itemArray = static_cast<CDesCArrayFlat*>(iListBox->Model()->ItemTextArray());
   if(itemArray->Count() > aIndex && aIndex >= 0){
      itemArray->Delete(aIndex);
      iListBox->HandleItemRemovalL();
      iListBox->DrawDeferred();
   }
}

// ----------------------------------------------------------------------------
// void CMyDestContainer::GetSelItem( TDesC &aItem )
// Get the selected item.
// ----------------------------------------------------------------------------
//
TBool CMyDestContainer::GetSelItem( TDes& aItem )
{
   TBool gotItem = EFalse;
   if ( iListBox ){
      TInt currentItem( GetSelectedIndex() );
      if (currentItem >= 0) {
         MDesCArray* itemList = iListBox->Model()->ItemTextArray();
         CDesCArray *itemArray = STATIC_CAST( CDesCArray*, itemList );

         // Removes an item at pos.
         TPtrC ptr = itemArray->MdcaPoint(currentItem);
         aItem.Copy( ptr );
         gotItem = ETrue;
      }
   }
   return gotItem;
}

// ----------------------------------------------------------------------------
// void CMyDestContainer::RemoveAllItemsL()
// Remove the selected item.
// ----------------------------------------------------------------------------
//
void CMyDestContainer::RemoveAllItemsL()
{
   if ( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray *itemArray = STATIC_CAST( CDesCArray*, itemList );

      itemArray->Reset();

      iListBox->HandleItemRemovalL(); // Updates listbox.
      // Actually, the HandleItemRemovalL does not provoke a redraw...
      iListBox->Reset();

      iNumberDests = 0;
      HBufC* text = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_MYDEST_EMPTY_LIST_ITEM );
//       itemArray->InsertL( 0, *text );

//       iListBox->HandleItemAdditionL();
      iListBox->View()->SetListEmptyTextL(*text);
      CleanupStack::PopAndDestroy(text);
   }
}

//------------------------------------------------------------------
// CMyDestContainer::ShowPopupList()
// shows "List pane for single item"
//------------------------------------------------------------------
void CMyDestContainer::ShowPopupListL()
{
   // Create listbox and PUSH it.
   CEikTextListBox* list = new(ELeave) CAknSinglePopupMenuStyleListBox;
   LOGNEW(list, CAknSinglePopupMenuStyleListBox);
   CleanupStack::PushL(list);

   // Create popup list and PUSH it.
   CAknPopupList* popupList = CAknPopupList::NewL( list,
                                                   R_WAYFINDER_SOFTKEYS_OK_CANCEL,
                                                   AknPopupLayouts::EMenuWindow);
   CleanupStack::PushL(popupList);

   // initialize listbox.
   list->ConstructL( popupList, CEikListBox::ELeftDownInViewRect );
   list->CreateScrollBarFrameL(ETrue);
   list->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff,
                                                    CEikScrollBarFrame::EAuto);

   // Set listitems.
   CDesCArrayFlat* descArray = new (ELeave) CDesCArrayFlat(5);
   LOGNEW(descArray, CDesCArrayFlat);
   CleanupStack::PushL( descArray );
   HBufC* item;
   item = iCoeEnv->AllocReadResourceLC( R_MYDEST_SET_ORIGIN_TEXT );
   descArray->AppendL( *item );
   CleanupStack::PopAndDestroy(item);
   item = iCoeEnv->AllocReadResourceLC( R_MYDEST_SET_DESTINATION_TEXT );
   descArray->AppendL( *item );
   CleanupStack::PopAndDestroy(item);
   item = iCoeEnv->AllocReadResourceLC( R_MYDEST_SHOW_ON_MAP_TEXT );
   descArray->AppendL( *item );
   CleanupStack::PopAndDestroy(item);
   item = iCoeEnv->AllocReadResourceLC( R_MYDEST_VIEW_DETAILS_TEXT );
   descArray->AppendL( *item );
   CleanupStack::PopAndDestroy(item);
   CleanupStack::Pop(descArray);

   CTextListBoxModel* model = list->Model();
   model->SetItemTextArray( descArray );
   model->SetOwnershipType(ELbmOwnsItemArray);

   // Show popup list
   if( popupList->ExecuteLD() ){
      TInt index = list->CurrentItemIndex();
      TInt command;
      switch( index )
      {
      case 0:
         command = EWayFinderCmdMyDestOrigin;
         break;
      case 1:
         command = EWayFinderCmdMyDestDestination;
         break;
      case 2:
         command = EWayFinderCmdMyDestShow;
         break;
      case 3:
         command = EWayFinderCmdMyDestViewDetails;
         break;
      default:
         command = EWayFinderCmdMyDestOrigin;
      }
      iView->HandleCommandL( command );
   } 
   CleanupStack::Pop(popupList);            // popuplist
   CleanupStack::PopAndDestroy(list);  // list
}

void CMyDestContainer::UpdatePreviewContentL()
{
#if defined NAV2_CLIENT_SERIES60_V5
   isab::GuiFavorite* guiFav = iView->GetSelectedGuiFav();
   if (guiFav) {
      TPtrC leftText;
      TPtrC rightText;
      HBufC* distance = NULL;
      HBufC* nameDesc = WFTextUtil::AllocL(guiFav->getName());
      TInt pos = nameDesc->Find(KTab);
      if (pos != KErrNotFound) {
         TPtr tmpName = nameDesc->Des().LeftTPtr(pos);
         tmpName.Trim();
         leftText.Set(tmpName);
         TPtr tmpDesc = nameDesc->Des().MidTPtr(pos + 1);
         tmpDesc.Trim();
         rightText.Set(tmpDesc);
      } else {
         leftText.Set(nameDesc->Des());
      }
      MC2Coordinate favPos = 
         MC2Coordinate(Nav2Coordinate(guiFav->getLat(), guiFav->getLon()));
      if (iView->IsGpsAllowed() && iView->ValidGpsStrength() && 
          favPos.isValid()) {
         // If we have valid gps coords and the fav has valid coords,
         // print the distance between them.
         TPoint nav2Pos = iView->GetCurrentPosition();
         MC2Coordinate currPos = 
            MC2Coordinate(Nav2Coordinate(nav2Pos.iY, nav2Pos.iX));
         char* dist = 
            isab::DistancePrintingPolicy::
            convertDistance(TUint32(GfxUtility::rtP2Pdistance_linear(currPos, favPos)),
                            isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode()),
                            DistancePrintingPolicy::Round);
         distance = WFTextUtil::AllocL(dist);
         delete[] dist;
      }
      if (!distance) {
         distance = KNullDesC().AllocL();
      }
      if (CEditFavoriteView::IsWayfinderFavorite(guiFav)) {
         iPreviewContent->SetImageL(EMbmWficonsFavorites, 
                                    EMbmWficonsFavorites_mask);
      } else {
         iPreviewContent->SetImageL(EMbmWficonsFavorites, 
                                    EMbmWficonsFavoriteslms_mask);
      }
      iPreviewContent->SetTextL(leftText, rightText, *distance);
      iPreviewContent->DrawDeferred();
      delete distance;
      delete nameDesc;
   }
#endif
}

void CMyDestContainer::ShowPreviewContent(TBool aShow)
{
#if defined NAV2_CLIENT_SERIES60_V5
   if (!iPreviewController) {
      return;
   }
   if (aShow) {
      iPreviewController->ShowPopUp();
   } else {
      iPreviewController->HidePopUp();
   }
   if (iShowingPreviewController != aShow) {
      iShowingPreviewController = aShow;
      SizeChanged();
   }
#else
   aShow = aShow;
#endif
}

// ----------------------------------------------------------------------------
// TKeyResponse CMyDestContainer::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
TKeyResponse CMyDestContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                               TEventCode aType )
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }

   switch ( aKeyEvent.iScanCode ) // The code of key event is...
   {
   // Switches tab.
   case EStdKeyLeftArrow: // Left key.
   case EStdKeyRightArrow: // Right Key.
      return EKeyWasNotConsumed;
#ifndef NAV2_CLIENT_SERIES60_V5
   case EStdKeyDevice3:
      if( iNumberDests > 0 ){
         if (IsListEmpty()) {
            /* Do nothing. */
         } else {
            if( iView->SettingOrigin() )
               iView->HandleCommandL( EWayFinderCmdMyDestOrigin );
            else if( iView->SettingDestination() )
               iView->HandleCommandL( EWayFinderCmdMyDestDestination );
            else if( iView->IsGpsAllowed() )
               iView->HandleCommandL( EWayFinderCmdMyDestNavigate );
            else if ( iView->IsIronVersion() )
               iView->HandleCommandL( EWayFinderCmdMyDestViewDetails );
            else {
               //ShowPopupListL(); // no popup list anymore.
               iView->HandleCommandL( EWayFinderCmdMyDestNavigate );
            }
         }
         return EKeyWasConsumed;
      } else {
         /* No destinations. */
         iView->HandleCommandL( EWayFinderCmdMyDestSync );
         return EKeyWasConsumed;
      }
      break;
#endif
   case EStdKeyBackspace:
      if (!iFindBox->IsVisible()) {
         // Findbox not visible.
         iView->RemoveFavorite();
         return EKeyWasConsumed;
      }
      break;
   }

   if ( iFindBox ) {
      TBool needRefresh( EFalse );
      // Offers the key event to find box.
      //XXX iFindBox->OfferKeyEventL(aKeyEvent, aType);
      if (AknFind::HandleFindOfferKeyEventL( aKeyEvent,
                                             aType,
                                             this,
                                             (CEikListBox *) iListBox,
                                             iFindBox,
                                             ETrue,
                                             needRefresh ) == EKeyWasConsumed )
      {
         if ( needRefresh ) {
            // SizeChanged();
            AknFind::HandlePopupFindSizeChanged(this, (CEikListBox *) iListBox, iFindBox);
            DrawNow();
         }
         return EKeyWasConsumed;
      }
      else if( iListBox ){
         TKeyResponse retVal = EKeyWasNotConsumed;
         retVal = iListBox->OfferKeyEventL(aKeyEvent, aType);
         if (retVal == EKeyWasConsumed && aType == EEventKey) {
            UpdatePreviewContentL();
         }
         return retVal;
      }
      else{
         return EKeyWasNotConsumed;
      }
   } else if( iListBox ){
      TKeyResponse retVal = EKeyWasNotConsumed;
      retVal = iListBox->OfferKeyEventL(aKeyEvent, aType);
      if (retVal == EKeyWasConsumed && aType == EEventKey) {
         UpdatePreviewContentL();
      }
      return retVal;
   } else{
      return EKeyWasNotConsumed;
   }
}

// ----------------------------------------------------------------------------
// void CMyDestContainer::HandleListBoxEventL( CEikListBox*,
//  TListBoxEvent )
// Handles listbox event.
// ----------------------------------------------------------------------------
void CMyDestContainer::HandleListBoxEventL( CEikListBox* aListBox,
                                            TListBoxEvent aEventType )
{
   if ((aListBox == iListBox && aEventType == EEventEnterKeyPressed) ||
       (aListBox == iListBox && aEventType == EEventItemDoubleClicked)) {
#ifdef NAV2_CLIENT_SERIES60_V5
      if (iView->SettingOrigin()) {
         iView->HandleCommandL(EWayFinderCmdMyDestOrigin);
      } else if (iView->SettingDestination()) {
         iView->HandleCommandL(EWayFinderCmdMyDestDestination);
      } else {
         if (WFLayoutUtils::IsTouchScreen()) {
            // If a dialog gets displayed the EButton1Up does not happen 
            // here in HandlePointerEventL so we need to manually cancel 
            // the long tap detector.
            iLongTapDetector->Cancel();
         }
         iView->HandleCommandL(EWayFinderCmdMyDestNavigate);
      }
#endif
   }
}

void CMyDestContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
#ifdef NAV2_CLIENT_SERIES60_V5
   if ( AknLayoutUtils::PenEnabled() ) {
      if (!(aPointerEvent.iType == TPointerEvent::EButton1Up && iPopUpMenuDisplayed)) {
         // If the type is EButton1Up and the popup menu has been launched this
         // event should not be forwarded to the listbox, if so the popup menu
         // will not be displayed and the listbox will interprete it as a click.
         // E.g. If item 4 is selected and you press a long tap on the already selected
         // item, the listbox will handle the button up as a double click.
         CCoeControl::HandlePointerEventL(aPointerEvent);
      }
      
      if (aPointerEvent.iType == TPointerEvent::EButton1Down) {
         UpdatePreviewContentL();
         iPopUpMenuDisplayed = EFalse;
      }
      iLongTapDetector->PointerEventL( aPointerEvent );   
   }
#endif
}

void CMyDestContainer::HandleLongTapEventL( const TPoint& aPenEventLocation, 
                                                      const TPoint& aPenEventScreenLocation )
{
#ifdef NAV2_CLIENT_SERIES60_V5
   iPopUpMenuDisplayed = ETrue;
   if ( !iPopUpMenu ) {
       // Launch stylus popup menu here
       // Construct the menu from resources
       iPopUpMenu = CAknStylusPopUpMenu::NewL( iView, aPenEventScreenLocation, NULL );
       TResourceReader reader;
       iCoeEnv->CreateResourceReaderLC( reader , R_FAV_POP_UP_STYLUS_MENU );
       iPopUpMenu->ConstructFromResourceL( reader );
       CleanupStack::PopAndDestroy(); // reader
    }
    iPopUpMenu->SetPosition( aPenEventScreenLocation );
    
    // Hide/show set as origin depending on the view state.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdMyDestOrigin, !iView->SettingOrigin());
    // Hide/show set as dest depending on the view state.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdMyDestDestination, !iView->SettingDestination());
    // Hide/show navigate to depending on the view state, should not be visible when
    // set as orig or set as dest is true.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdMyDestNavigate, (iView->SettingOrigin() || 
                                                            iView->SettingDestination()));
    iPopUpMenu->ShowMenu();
#endif
}

// ---------------------------------------------------------
// CMyDestContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CMyDestContainer::SizeChanged()
{
   if(iBgContext) {
      iBgContext->SetRect(Rect());
      if (&Window()) {
         iBgContext->SetParentPos(PositionRelativeToScreen());
      }
   }

   if (iListBox && iFindBox) {
      AknFind::HandlePopupFindSizeChanged( this, iListBox, iFindBox );
      //XXX AknFind::HandleFixedFindSizeChanged( this, iListBox, iFindBox );
#if defined NAV2_CLIENT_SERIES60_V5
      if (iPreviewController) {
         TRect rect = Rect();
         TSize previewSize = iPreviewController->Size();
         TInt borderPadding = (previewSize.iWidth - 
                               iPreviewContent->Size().iWidth) / 2;
         if (WFLayoutUtils::LandscapeMode()) {
            iPreviewContent->SetAvailableWidth((rect.Width() / 2 - borderPadding * 2), 
                                               borderPadding, 
                                               CPreviewPopUpContent::EFourLinesTextImageTopLeftAbove,
                                               (rect.Height() - borderPadding * 2));
            iPreviewController->UpdateContentSize();
            TRect statusPaneRect;
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EStatusPane, 
                                              statusPaneRect);
            TPoint previewPos = rect.iTl;
            previewPos.iY += statusPaneRect.Height();
            // iPreviewController is not part of this container so we can't 
            // position it with 0,0 since that is the top of the screen and 
            // not the top corner of our container for the PreviewController.
            iPreviewController->SetPosition(previewPos);
            previewSize = iPreviewController->Size();
            // Reposition rect for the listbox.
            rect.iTl.iX += previewSize.iWidth;
         } else {
            iPreviewContent->SetAvailableWidth(rect.Width() - borderPadding * 2, 
                                               borderPadding,
                                               CPreviewPopUpContent::EFourLinesIndentedTextImageTopLeft);
            iPreviewController->UpdateContentSize();
            TRect statusPaneRect;
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EStatusPane, 
                                              statusPaneRect);
            TPoint previewPos = rect.iTl;
            previewPos.iY += statusPaneRect.Height();
            // iPreviewController is not part of this container so we can't 
            // position it with 0,0 since that is the top of the screen and 
            // not the top corner of our container for the PreviewController.
            iPreviewController->SetPosition(previewPos);
            previewSize = iPreviewController->Size();
            // Reposition rect for the listbox.
            rect.iTl.iY += previewSize.iHeight;
         }
         // iListBox is part of this container so position it relatively.
         if (iShowingPreviewController) {
            // if the preview controller is shown set the corresponding size
            // on the listbox.
            iListBox->SetRect(rect);
         } else {
            // if the preview controller is not shown we use the full rect.
            iListBox->SetRect(Rect());
         }
         DrawDeferred();
      }
#endif
   } else {
      if (iListBox) {
         iListBox->SetRect(Rect());
      }
   }
}

// ---------------------------------------------------------
// CMyDestContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CMyDestContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CMyDestContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CMyDestContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case EListBox:
      return iListBox;
   case EFindBox:
      return iFindBox;
   default:
      return NULL;
   }
}

TTypeUid::Ptr CMyDestContainer::MopSupplyObject(TTypeUid aId)
{
   if (iBgContext) {
      return MAknsControlContext::SupplyMopObject(aId, iBgContext);
   }
   return CCoeControl::MopSupplyObject(aId);
}

// ---------------------------------------------------------
// CMyDestContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CMyDestContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   // TODO: Add your drawing code here
   // example code...
//    gc.SetPenStyle(CGraphicsContext::ENullPen);
//    gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
//    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
//    gc.DrawRect(aRect);

   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
   AknsDrawUtils::Background(skin, cc, this, gc, aRect);
}

// ---------------------------------------------------------
// CMyDestContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CMyDestContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/ )
{
   // TODO: Add your control event handler code here
}

void CMyDestContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }   
}


// End of File  
