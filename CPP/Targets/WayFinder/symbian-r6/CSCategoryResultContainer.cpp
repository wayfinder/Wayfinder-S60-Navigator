/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <barsread.h>
#include <stringloader.h>
#include <aknlists.h>
#include <eikenv.h>
#include <akniconarray.h>
#include <eikclbd.h>
#include <aknviewappui.h>
#include <eikappui.h>

#if defined NAV2_CLIENT_SERIES60_V5
# include <aknstyluspopupmenu.h>
#endif

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "wficons.mbg"
#include "WayFinderConstants.h"
#include "WFTextUtil.h"
#include "WFLayoutUtils.h"

#include "CSCategoryResultContainer.h"
#include "CSCategoryResultView.h"
#include "CombinedSearchDataTypes.h"
#include "CombinedSearchDataHolder.h"

#define ICON_SIZE TSize(10, 10)

CCSCategoryResultContainer::CCSCategoryResultContainer(CCSCategoryResultView* aView) :
   iView(aView)
{
   iListBox = NULL;
}

CCSCategoryResultContainer::~CCSCategoryResultContainer()
{
   delete iListBox;
   iListBox = NULL;
   delete iEventGenerator;

#ifdef NAV2_CLIENT_SERIES60_V5
   delete iLongTapDetector;
   delete iPopUpMenu;
#endif
}
				
CCSCategoryResultContainer* CCSCategoryResultContainer::NewL(const TRect& aRect, 
                                                             CCSCategoryResultView* aView)
{
   CCSCategoryResultContainer* self = CCSCategoryResultContainer::NewLC(aRect, 
                                                                        aView);
   CleanupStack::Pop( self );
   return self;
}

CCSCategoryResultContainer* CCSCategoryResultContainer::NewLC(const TRect& aRect, 
                                                              CCSCategoryResultView* aView )
{
   CCSCategoryResultContainer* self = new ( ELeave ) CCSCategoryResultContainer(aView);
   CleanupStack::PushL( self );
   self->ConstructL( aRect );
   return self;
}
			
void CCSCategoryResultContainer::ConstructL(const TRect& aRect)
{
   CreateWindowL();
   iEventGenerator = CSEventGenerator::NewL(*this);
   InitializeControlsL();

#ifdef NAV2_CLIENT_SERIES60_V5
   iLongTapDetector = CAknLongTapDetector::NewL( this );
#endif

   SetRect( aRect );
   ActivateL();
}
			
TInt CCSCategoryResultContainer::CountComponentControls() const
{
   return ( int ) ELastControl;
}
				
CCoeControl* CCSCategoryResultContainer::ComponentControl( TInt aIndex ) const
{
   switch ( aIndex ) {
   case EListBox:
      return iListBox;
   }
   return NULL;
}

void CCSCategoryResultContainer::SizeChanged()
{
   CCoeControl::SizeChanged();
   iListBox->SetExtent( TPoint( 0, 0 ), iListBox->MinimumSize() );

   // Disable scrollbars per default.
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EOff);
   
   TInt listBoxHeight = iListBox->ItemHeight() * iListBox->Model()->NumberOfItems();
   if (listBoxHeight > iListBox->Size().iHeight) {
      // Show scroll bar only if the height of the listbox is larger then
      // the rect.
      iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                          CEikScrollBarFrame::EOn);
      iListBox->UpdateScrollBarsL();
   }
}
				
TKeyResponse CCSCategoryResultContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                 TEventCode aType )
{
   if (!IsTopHit()) {
      if ( aKeyEvent.iCode == EKeyLeftArrow ) /*|| aKeyEvent.iCode == EKeyRightArrow )*/ {
         // Listbox takes all events even if it doesn't use them
         return EKeyWasNotConsumed;
      } else if ( aKeyEvent.iCode == EKeyRightArrow ) {
         iView->HandleCommandL( EWayFinderCmdCSDetailedResultView );
         return EKeyWasConsumed;
      }
   }
   if ( iListBox != NULL
	&& iListBox->OfferKeyEventL( aKeyEvent, aType ) == EKeyWasConsumed ) {
      return EKeyWasConsumed;
   }
   return CCoeControl::OfferKeyEventL( aKeyEvent, aType );
}
				
void CCSCategoryResultContainer::HandleListBoxEventL( CEikListBox* aListBox,
                                                      TListBoxEvent aEventType )
{
   if( aListBox == iListBox && aEventType == EEventEnterKeyPressed ||
       aListBox == iListBox && aEventType == EEventItemDoubleClicked ) {
      if (IsTopHit()) {
         if( iView->SettingOrigin() ) {
            // In case of coming from the route planner and selected
            // set as origin, send this command to the view
            iView->HandleCommandL( EWayFinderCmdNewDestOrigin );
         } else if( iView->SettingDestination() ) {
            // In case of coming from the route planner and selected
            // set as destination, send this command to the view
            iView->HandleCommandL( EWayFinderCmdNewDestDestination );
         } else /*if (iView->IsGpsAllowed())*/ {
            // Let the view handle this command, if there is a gps
            // connected and we get valid positions from it, this command
            // will do a navigate to else we start the gps conn wizard.
#ifdef NAV2_CLIENT_SERIES60_V5
            if (WFLayoutUtils::IsTouchScreen()) {
               // If a dialog gets displayed the EButton1Up does not happen 
               // here in HandlePointerEventL so we need to manually cancel 
               // the long tap detector.
               iLongTapDetector->Cancel();
            }
#endif
            iView->HandleCommandL( EWayFinderCmdNewDestNavigate );
         } /*else {
            // If none of the above we show information in service window.
            iView->HandleCommandL( EWayFinderCmdNewDestShowInfo );
         }*/
      } else {
         iView->HandleCommandL( EWayFinderCmdCSDetailedResultView );
      }
   } 
}

void CCSCategoryResultContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
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
         iPopUpMenuDisplayed = EFalse;
      }
      if (IsTopHit()) {
         iLongTapDetector->PointerEventL( aPointerEvent );   
      }
   }
#endif
}

void CCSCategoryResultContainer::HandleLongTapEventL( const TPoint& aPenEventLocation, 
                                                      const TPoint& aPenEventScreenLocation )
{
#ifdef NAV2_CLIENT_SERIES60_V5
   iPopUpMenuDisplayed = ETrue;
   if ( !iPopUpMenu ) {
       // Launch stylus popup menu here
       // Construct the menu from resources
       iPopUpMenu = CAknStylusPopUpMenu::NewL( iView, aPenEventScreenLocation, NULL );
       TResourceReader reader;
       iCoeEnv->CreateResourceReaderLC( reader , R_CS_POP_UP_STYLUS_MENU );
       iPopUpMenu->ConstructFromResourceL( reader );
       CleanupStack::PopAndDestroy(); // reader
    }
    iPopUpMenu->SetPosition( aPenEventScreenLocation );

    // Hide/show set as origin depending on the view state.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdNewDestOrigin, !iView->SettingOrigin());
    // Hide/show set as dest depending on the view state.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdNewDestDestination, !iView->SettingDestination());
    // Hide/show navigate to depending on the view state, should not be visible when
    // set as orig or set as dest is true.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdNewDestNavigate, (iView->SettingOrigin() || 
                                                             iView->SettingDestination()));

    iPopUpMenu->ShowMenu();
#endif
}

void CCSCategoryResultContainer::HandleResourceChange( TInt aType )
{
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
   CCoeControl::HandleResourceChange(aType);
   //   SetRect( iAvkonViewAppUi->View( TUid::Uid( EWayFinderCSCategoryResultView ) )->ClientRect() );
}

void CCSCategoryResultContainer::InitializeControlsL()
{
   iListBox = new ( ELeave ) CAknSingleGraphicHeadingStyleListBox;
   iListBox->SetContainerWindowL( *this );
   {
      TResourceReader reader;
      iEikonEnv->CreateResourceReaderLC( reader, R_WAYFINDER_CS_CATEGORY_RESULT_LISTBOX );
      iListBox->ConstructFromResourceL( reader );
      CleanupStack::PopAndDestroy(); // reader internal state
   }
   // the listbox owns the items in the list and will free them
   iListBox->Model()->SetOwnershipType( ELbmOwnsItemArray );
	
   iListBox->SetFocus( ETrue );
   iListBox->SetListBoxObserver(this);

   iListBox->CreateScrollBarFrameL(ETrue);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EAuto);

   iListBox->ItemDrawer()->ColumnData()->SetMarqueeParams(5, 6, 1000000, 200000);
   iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);

   iListBox->UpdateScrollBarsL();
}
				
void CCSCategoryResultContainer::Draw( const TRect& aRect ) const
{
   CWindowGc& gc = SystemGc();
   gc.Clear( aRect );
}
				
RArray< TInt >* CCSCategoryResultContainer::GetSelectedListBoxItemsLC( CEikTextListBox* aListBox )
{
   CAknFilteredTextListBoxModel* model = 
      static_cast< CAknFilteredTextListBoxModel *> ( aListBox->Model() );
   if ( model->NumberOfItems() == 0 )
      return NULL;
		
   // get currently selected indices
   const CListBoxView::CSelectionIndexArray* selectionIndexes =
      aListBox->SelectionIndexes();
   TInt selectedIndexesCount = selectionIndexes->Count();
   if ( selectedIndexesCount == 0 )
      return NULL;
		
   // copy the indices and sort numerically
   RArray<TInt>* orderedSelectedIndices = 
      new (ELeave) RArray< TInt >( selectedIndexesCount );
	
   // push the allocated array
   CleanupStack::PushL( orderedSelectedIndices );
	
   // dispose the array resource
   CleanupClosePushL( *orderedSelectedIndices );
	
   // see if the search field is enabled
   CAknListBoxFilterItems* filter = model->Filter();
   if ( filter != NULL ) {
      // when filtering enabled, translate indices back to underlying model
      for ( TInt idx = 0; idx < selectedIndexesCount; idx++ ) {
         TInt filteredItem = ( *selectionIndexes ) [ idx ];
         TInt actualItem = filter->FilteredItemIndex ( filteredItem );
         orderedSelectedIndices->InsertInOrder( actualItem );
      }
   } else {
      // the selection indices refer directly to the model
      for ( TInt idx = 0; idx < selectedIndexesCount; idx++ )
         orderedSelectedIndices->InsertInOrder( ( *selectionIndexes ) [ idx ] );
   }	
   return orderedSelectedIndices;
}

TInt CCSCategoryResultContainer::GetSelectedIndex()
{
   if ( iListBox && iListBox->Model()->NumberOfItems() ) {
      return iListBox->CurrentItemIndex() - iTopHitsOffset;
   }
   return -1;
}

TInt CCSCategoryResultContainer::GetCurrentIndex()
{
   if ( iListBox && iListBox->Model()->NumberOfItems() ) {
      return iListBox->CurrentItemIndex();
   }
   return -1;   
}

void CCSCategoryResultContainer::SetSelectedIndex(TInt aIndex)
{
   if (aIndex < iListBox->Model()->NumberOfItems()) {
      iListBox->SetCurrentItemIndexAndDraw(aIndex);
   }
}

void CCSCategoryResultContainer::DeleteSelectedListBoxItemsL( CEikTextListBox* aListBox )
{
   CAknFilteredTextListBoxModel* model = 
      static_cast< CAknFilteredTextListBoxModel *> ( aListBox->Model() );
   if ( model->NumberOfItems() == 0 )
      return;
	
   RArray< TInt >* orderedSelectedIndices = GetSelectedListBoxItemsLC( aListBox );		
   if ( !orderedSelectedIndices )
      return;
		
   // Delete selected items from bottom up so indices don't change on us
   CDesCArray* itemArray = static_cast< CDesCArray* > ( model->ItemTextArray() );
   TInt currentItem = 0;
	
   for ( TInt idx = orderedSelectedIndices->Count(); idx-- > 0; ) {
      currentItem = ( *orderedSelectedIndices )[ idx ];
      itemArray->Delete ( currentItem );
   }
	
   // dispose the array resources
   CleanupStack::PopAndDestroy();
	
   // dispose the array pointer
   CleanupStack::PopAndDestroy( orderedSelectedIndices );
	
   // refresh listbox's cursor now that items are deleted
   AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(aListBox, currentItem, ETrue );
}

CAknSingleGraphicHeadingStyleListBox* CCSCategoryResultContainer::ListBox()
{
   return iListBox;
}

TBool CCSCategoryResultContainer::HandleMarkableListCommandL( TInt aCommand )
{   
   return EFalse;
}

CGulIcon* CCSCategoryResultContainer::LoadAndScaleIconL( const TDesC& aFileName )
{
   _LIT(KMifExt, ".mif");
   CFbsBitmap* bitmap;
   CFbsBitmap* mask;

   TFileName completeName = iView->GetCSIconPath();
   completeName.Append(aFileName);
   completeName.Append(KMifExt);

   TInt bitmapId = 0;
   TInt maskId = 1;

   class RFs fs;
   fs.Connect();
   class RFile imgFile;
   TInt ret = imgFile.Open(fs, completeName, EFileRead);
   if(ret == KErrNone || ret == KErrInUse) {
      imgFile.Close();
      AknIconUtils::ValidateLogicalAppIconId(completeName, bitmapId, maskId);
      AknIconUtils::CreateIconL(bitmap, mask, completeName, bitmapId, maskId);
   } else {
      // Change completeName to fallback icon
      completeName.Copy(iView->GetMbmName());
      AknIconUtils::CreateIconL(bitmap, mask, completeName, 
                                EMbmWficonsTat_default, EMbmWficonsTat_default_mask); 
   }
   fs.Close();

   AknIconUtils::SetSize( bitmap, WFLayoutUtils::CalculateSizeUsingMainPane(ICON_SIZE), 
                          EAspectRatioPreservedAndUnusedSpaceRemoved );

   return CGulIcon::NewL( bitmap, mask );
}

_LIT ( KStringHeader, "%d\t%S\t%S" );   
void CCSCategoryResultContainer::CreateListBoxItemL( TDes& aBuffer, 
                                                     TInt aIconIndex,
                                                     const TDesC& aHeadingText,
                                                     const TDesC& aMainText )
{
   aBuffer.Format( KStringHeader(), aIconIndex, &aHeadingText, &aMainText );
}

HBufC* CCSCategoryResultContainer::CreateListBoxItemL( TInt aHeading,
                                                       const char* aMainText,
                                                       const char* aImageName )
{
   TBuf<8> headingText;
   if (aHeading != -1) {
      headingText.Num(aHeading);
   } else {
      headingText.Copy(KEmpty);
   }

   HBufC* mainText = WFTextUtil::AllocLC(aMainText);
   HBufC* listString = HBufC::NewLC(headingText.Length() + mainText->Length() + 8);

   HBufC* imgName = WFTextUtil::AllocLC(aImageName);
   iIcons->AppendL(LoadAndScaleIconL(*imgName));
   CleanupStack::PopAndDestroy(imgName);

   listString->Des().Format( KStringHeader(), iIcons->Count()-1, &headingText, mainText );
   CleanupStack::Pop(listString);
   CleanupStack::PopAndDestroy(mainText);
   return listString;
}

void CCSCategoryResultContainer::AddListBoxItemL( TInt aHeading, /*const TDesC& aHeadingText,*/
                                                  const char* aMainText, 
                                                  const char* aImageName )
{
   HBufC* listString = CreateListBoxItemL( aHeading, aMainText, aImageName );
   iList->AppendL( *listString );
   delete listString;
}

void CCSCategoryResultContainer::InsertListBoxItemL(TInt aHeading,
                                                    const char* aMainText,
                                                    const char* aImageName,
                                                    TInt aIndex)
{
   HBufC* listString = CreateListBoxItemL( aHeading, aMainText, aImageName );
   iList->InsertL( aIndex, *listString );
   delete listString;   
}

void 
CCSCategoryResultContainer::AddCategoryResultsL( const std::vector<CombinedSearchCategory*>& categories )
{
   if (iBusyAddingCategories) {
      iOutstandingRequest = ETrue;
      return;
   }
   iBusyAddingCategories = ETrue;
   iTopHitsOffset = 0;
   iIcons = new (ELeave) CAknIconArray(5);

   iList = new (ELeave) CDesCArrayFlat(5);

   std::vector<CombinedSearchCategory*>::const_iterator it;
   for( it = categories.begin(); it != categories.end(); ++it) {
      // Iterate through all categories
      if ((*it)->getTopHits() > 0) {
         CSSIVec_t::const_iterator sit = (*it)->getResults().begin();
         if (sit) {
            for (TUint i = 0; i < (*it)->getTopHits(); ++sit, ++i) {
               InsertListBoxItemL(-1, sit->getName(), sit->getImageName(), iTopHitsOffset);
               iTopHitsOffset++;
            }
         }
      }
      AddListBoxItemL((*it)->getTotalNbrHits(), (*it)->getName(), (*it)->getImageName());
   }

   if (iList->Count() > 0 && iIcons->Count() > 0) {
      iListBox->ItemDrawer()->ColumnData()->SetIconArray( iIcons );
      CTextListBoxModel* model = iListBox->Model();
      //model->SetOwnershipType( ELbmDoesNotOwnItemArray );
      model->SetItemTextArray(iList);
      iListBox->HandleItemAdditionL();
   }
   iBusyAddingCategories = EFalse;
   if (iOutstandingRequest) {
      iOutstandingRequest = EFalse;
      GenerateEvent(ECSAddCategories);
   }

   // Disable scrollbars per default.
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EOff);
   TRect rect = TRect(TPoint(0, 0), iListBox->Size());
   TInt listBoxHeight = iListBox->ItemHeight() * iListBox->Model()->NumberOfItems();
   if (listBoxHeight > rect.Height()) {
      // Show scroll bar only if the height of the listbox is larger then
      // the rect.
      iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                          CEikScrollBarFrame::EOn);
   }
   iListBox->UpdateScrollBarsL();

}

void CCSCategoryResultContainer::GenerateEvent(enum TCSEvent aEvent)
{
   if (!iEventGenerator) {
      iEventGenerator = CSEventGenerator::NewL(*this); 
   }
   iEventGenerator->SendEventL(aEvent);
}

void CCSCategoryResultContainer::HandleGeneratedEventL(enum TCSEvent aEvent)
{
   switch(aEvent) {
   case ECSAddCategories:
      iView->RetryAddCategories();
      break;
   }
}

TBool CCSCategoryResultContainer::IsTopHit()
{
   return (iListBox && 
           iListBox->CurrentItemIndex() < iTopHitsOffset &&
           iListBox->Model()->NumberOfItems());
}
