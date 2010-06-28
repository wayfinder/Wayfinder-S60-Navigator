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
#include <aknsfld.h>
#include <aknutils.h>
#include <aknviewappui.h>
#include <eikappui.h>

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "WFLayoutUtils.h"

#include "SearchableListBoxContainer.h"
#include "SearchableListBoxView.h"

CSearchableListBoxContainer::CSearchableListBoxContainer(CSearchableListBoxView& aView) :
   iView(aView)
{
   iListBox = NULL;
   iListBoxSearchField = NULL;
}

CSearchableListBoxContainer::~CSearchableListBoxContainer()
{
   delete iListBox;
   iListBox = NULL;
   delete iListBoxSearchField;
   iListBoxSearchField = NULL;
}
				
CSearchableListBoxContainer* CSearchableListBoxContainer::NewL( const TRect& aRect, 
                                                                CSearchableListBoxView& aView,
                                                                MEikCommandObserver* aCommandObserver )
{
   CSearchableListBoxContainer* self = CSearchableListBoxContainer::NewLC( aRect, 
                                                                           aView,
                                                                           aCommandObserver );
   CleanupStack::Pop( self );
   return self;
}

CSearchableListBoxContainer* CSearchableListBoxContainer::NewLC( const TRect& aRect, 
                                                                 CSearchableListBoxView& aView,
                                                                 MEikCommandObserver* aCommandObserver )
{
   CSearchableListBoxContainer* self = new ( ELeave ) CSearchableListBoxContainer(aView);
   CleanupStack::PushL( self );
   self->ConstructL( aRect, aCommandObserver );
   return self;
}

void CSearchableListBoxContainer::ConstructL( const TRect& aRect, 
                                              MEikCommandObserver* aCommandObserver )
{
 
   CreateWindowL();

   iFocusControl = NULL;
   iCommandObserver = aCommandObserver;
   InitializeControlsL();
   iListBox->ItemDrawer()->ColumnData()->SetMarqueeParams(5, 6, 1000000, 200000);
   iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
   iListBox->CreateScrollBarFrameL(ETrue);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOn,
                                                       CEikScrollBarFrame::EAuto);	
   SetRect( aRect );
   ActivateL();
}

TInt CSearchableListBoxContainer::CountComponentControls() const
{
   return ( int ) ELastControl;
}
				
CCoeControl* CSearchableListBoxContainer::ComponentControl( TInt aIndex ) const
{
   switch ( aIndex ) {
   case EListBox:
      return iListBox;
   case EListBoxSearchField:
      return iListBoxSearchField;
   }
   return NULL;
}
				
void CSearchableListBoxContainer::SizeChanged()
{
   CCoeControl::SizeChanged();
   LayoutControls();
			
   AknFind::HandleFixedFindSizeChanged( this, iListBox, iListBoxSearchField );
}
				
void CSearchableListBoxContainer::LayoutControls()
{
   iListBox->SetExtent( TPoint( 0, 0 ), iListBox->MinimumSize() );
}

TKeyResponse CSearchableListBoxContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, 
                                                          TEventCode aType )
{
   if ( aKeyEvent.iCode == EKeyLeftArrow 
        || aKeyEvent.iCode == EKeyRightArrow ) {
      // Listbox takes all events even if it doesn't use them
      return EKeyWasNotConsumed;
   }
   if ( iListBoxSearchField != NULL
        && iListBoxSearchField->OfferKeyEventL( aKeyEvent, aType ) 
        == EKeyWasConsumed ) {
      return EKeyWasConsumed;
   }
	
   if ( iFocusControl != NULL
        && iFocusControl->OfferKeyEventL( aKeyEvent, aType ) == EKeyWasConsumed ) {
      return EKeyWasConsumed;
   }
   return CCoeControl::OfferKeyEventL( aKeyEvent, aType );
}
				
void CSearchableListBoxContainer::InitializeControlsL()
{
   iListBox = new ( ELeave ) CAknSingleStyleListBox;
   iListBox->SetContainerWindowL( *this );
   {
      TResourceReader reader;
      iEikonEnv->CreateResourceReaderLC( reader, R_WAYFINDER_SEARCHABLE_LISTBOX );
      iListBox->ConstructFromResourceL( reader );
      CleanupStack::PopAndDestroy(); // reader internal state
   }
   // the listbox owns the items in the list and will free them
   //iListBox->Model()->SetOwnershipType( ELbmOwnsItemArray );
   iListBox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );

   // setup the icon array so graphics-style boxes work
   SetupListBoxIconsL();
	
   iListBox->SetListBoxObserver( this );
	
   // set up the Search Field	
   CAknSearchField* searchField = 
      CAknSearchField::NewL( *this, CAknSearchField::ESearch, 0, 40 );
	
   CleanupStack::PushL ( searchField );
   CAknFilteredTextListBoxModel* filteredModel = 
      static_cast< CAknFilteredTextListBoxModel* >( iListBox->Model() );
   filteredModel->CreateFilterL( iListBox, searchField );
   CleanupStack::Pop( searchField );
   searchField->MakeVisible( ETrue );
   iListBoxSearchField = searchField;
	
   iListBox->SetFocus( ETrue );
   iFocusControl = iListBox;
	
}

void CSearchableListBoxContainer::HandleResourceChange( TInt aType )
{
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
   CCoeControl::HandleResourceChange(aType);
   //   SetRect( iAvkonViewAppUi->View( TUid::Uid( EWayFinderSearchableListBoxView ) )->ClientRect() );
}
				
void CSearchableListBoxContainer::Draw( const TRect& aRect ) const
{
   //   CWindowGc& gc = SystemGc();
   //   gc.Clear( aRect );
}

void CSearchableListBoxContainer::CreateListBoxItemL( TDes& aBuffer, 
                                                      const TDesC& aMainText )
{
   _LIT( KStringFormat, "\t%S\t\t" );
   aBuffer.Format( KStringFormat(), &aMainText );
} 
void CSearchableListBoxContainer::SetSelectionL(TInt aIndex)
{
  if (iListBox) {
    TInt selection = 0;
    if(aIndex < (iListBox->Model()->NumberOfItems())) {
  	  selection = aIndex;
    }
    iListBox->ClearSelection();
    iListBox->View()->SetCurrentItemIndex(selection);
    iListBox->View()->ScrollToMakeItemVisible(selection); 
    //iListBox->View()->SelectItemL(index);
  }
}
				
void CSearchableListBoxContainer::AddListBoxItemL( const TDesC& aString )
{
   HBufC* listString = HBufC::NewLC(aString.Length() + 8);
   TPtr listStringPtr = listString->Des();
   CreateListBoxItemL( listStringPtr, aString );
   AddFormattedListBoxItemL( *listString );
   CleanupStack::PopAndDestroy(listString);
}

void CSearchableListBoxContainer::AddFormattedListBoxItemL( const TDesC& aString )
{
   CTextListBoxModel* model = iListBox->Model();
   CDesCArray* itemArray = static_cast< CDesCArray* > ( model->ItemTextArray() );
   itemArray->AppendL( aString );
   iListBox->HandleItemAdditionL();
}

void CSearchableListBoxContainer::AddFormattedListBoxItemsL( MDesCArray* aArray )
{
   CTextListBoxModel* model = iListBox->Model();
   //model->SetOwnershipType( ELbmDoesNotOwnItemArray );
   model->SetItemTextArray( aArray );
   iListBox->HandleItemAdditionL();
}

RArray< TInt >* CSearchableListBoxContainer::GetSelectedListBoxItemsLC( CEikTextListBox* aListBox )
{
   CAknFilteredTextListBoxModel* model = 
      static_cast< CAknFilteredTextListBoxModel *> ( aListBox->Model() );
   if ( model->NumberOfItems() == 0 ) {
      return NULL;
   }
		
   // get currently selected indices
   const CListBoxView::CSelectionIndexArray* selectionIndexes =
      aListBox->SelectionIndexes();
   TInt selectedIndexesCount = selectionIndexes->Count();
   if ( selectedIndexesCount == 0 ) {
      return NULL;
   }
		
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
      for ( TInt idx = 0; idx < selectedIndexesCount; idx++ ) {
         orderedSelectedIndices->InsertInOrder( ( *selectionIndexes ) [ idx ] );
      }
   }	
		
   return orderedSelectedIndices;
}
				
void CSearchableListBoxContainer::DeleteSelectedListBoxItemsL( CEikTextListBox* aListBox )
{
   CAknFilteredTextListBoxModel* model = 
      static_cast< CAknFilteredTextListBoxModel *> ( aListBox->Model() );
   if ( model->NumberOfItems() == 0 ) {
      return;
   }	

   RArray< TInt >* orderedSelectedIndices = GetSelectedListBoxItemsLC( aListBox );		
   if ( !orderedSelectedIndices ) {
      return;
   }
		
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
   AknListBoxUtils::HandleItemRemovalAndPositionHighlightL( aListBox, currentItem, ETrue );
}
				
CAknSingleStyleListBox* CSearchableListBoxContainer::ListBox()
{
   return iListBox;
}

TInt CSearchableListBoxContainer::CountListBoxItems()
{
   if (iListBox) {
      return iListBox->Model()->NumberOfItems();
   }
   return -1;
}
				
void CSearchableListBoxContainer::SetupListBoxIconsL()
{
   CArrayPtr< CGulIcon >* icons = NULL;		
	
   if ( icons != NULL ) {
      iListBox->ItemDrawer()->ColumnData()->SetIconArray( icons );
   }
}
				
TBool CSearchableListBoxContainer::HandleMarkableListCommandL( TInt aCommand )
{   
   return EFalse;
}
				
void CSearchableListBoxContainer::HandleListBoxEventL( CEikListBox* aListBox,
                                                       TListBoxEvent aEventType)
{
   if (aListBox == iListBox && ( aEventType == EEventEnterKeyPressed
                                 || aEventType == EEventItemDoubleClicked )){
      if (CountListBoxItems() > 0) {
         iView.HandleCommandL(EWayFinderCmdCSSelect);
      }
   }
}
				
const TPtrC CSearchableListBoxContainer::GetValue()
{
   CAknFilteredTextListBoxModel* model =
      STATIC_CAST( CAknFilteredTextListBoxModel*, iListBox->Model() );
   CAknListBoxFilterItems* filter = model->Filter();
   TInt index = iListBox->CurrentItemIndex();
   if (index < 0) {
      return _L("");
   } else {
      return iListBox->Model()->ItemTextArray()->MdcaPoint( filter->FilteredItemIndex( index ) );
      //return filter->FilteredItemIndex( index );
   }
}

TInt CSearchableListBoxContainer::GetSelectedId()
{
   if (iListBox) {
      CAknFilteredTextListBoxModel* model =
         STATIC_CAST( CAknFilteredTextListBoxModel*, iListBox->Model() );
      CAknListBoxFilterItems* filter = model->Filter();
      TInt index = iListBox->CurrentItemIndex();
      if (index >= 0) {
         return filter->FilteredItemIndex(index);
      }
   }
   return -1;
}
