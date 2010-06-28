/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <aknviewappui.h>
#include <eikmenub.h>
#include <avkon.hrh>
#include <barsread.h>
#include <stringloader.h>
#include <aknlists.h>
#include <eikenv.h>
#include <akniconarray.h>
#include <eikclbd.h>
#include <aknsfld.h>
#include <aknutils.h>
#include <akncontext.h>
#include <akntitle.h>
#include <eikbtgpc.h>

#include <arch.h>

#include "WayFinderConstants.h"
#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "WayFinderAppUi.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"
#include "DataHolder.h"

#include "SearchableListBoxView.h"
#include "SearchableListBoxContainer.h"

#include "CountrySelectInterface.h"
 
CSearchableListBoxView::CSearchableListBoxView( CWayFinderAppUi* aAppUi,
                                                MCountrySelectInterface& aListboxInterface ) : 
   CViewBase(aAppUi), 
   iListboxListener( aListboxInterface )
{
   iContainer = NULL;
}

CSearchableListBoxView::~CSearchableListBoxView()
{
   if (iList) {
      iList->Reset();   
      delete iList;
      iList = NULL;
   }

   delete iContainer;
   iContainer = NULL;
}

CSearchableListBoxView* CSearchableListBoxView::NewL( CWayFinderAppUi* aAppUi,
                                                      MCountrySelectInterface& aListboxInterface)
{
   CSearchableListBoxView* self = CSearchableListBoxView::NewLC( aAppUi, 
                                                                 aListboxInterface );
   CleanupStack::Pop( self );
   return self;
}

CSearchableListBoxView* CSearchableListBoxView::NewLC( CWayFinderAppUi* aAppUi,
                                                       MCountrySelectInterface& aListboxInterface )
{
   CSearchableListBoxView* self = new ( ELeave ) CSearchableListBoxView( aAppUi,
                                                                         aListboxInterface);
   CleanupStack::PushL( self );
   self->ConstructL();
   return self;
}

void CSearchableListBoxView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_SEARCHABLE_LISTBOX_VIEW );
}
	
TUid CSearchableListBoxView::Id() const
{
   return TUid::Uid( EWayFinderSearchableListBoxView );
}

void CSearchableListBoxView::HandleCommandL( TInt aCommand )
{   
   switch ( aCommand ) {
   case EAknSoftkeyBack:
      iWayfinderAppUi->HandleCommandL(aCommand);	
      break;
   case EAknSoftkeyExit:
      iWayfinderAppUi->HandleCommandL(aCommand);	
      break;
   case EWayFinderCmdCSSelect:
      {
         TInt id = iContainer->GetSelectedId();
         if (id >= 0) {
            // if it returns -1 the index is not usable.
            iListboxListener.SetValue(id);
         }
         iWayfinderAppUi->HandleCommandL(EAknSoftkeyBack);
      }
      break;
   default:
      break;	
   }
}

_LIT( KStringFormat, "\t%S\t\t" );
void CSearchableListBoxView::CreateListBoxItemL( TDes& aBuffer, 
                                                 const TDesC& aMainText )
{
   aBuffer.Format( KStringFormat(), &aMainText );
}

void CSearchableListBoxView::AddListBoxItemL( const TDesC& aString )
{
   HBufC* listString = HBufC::NewLC(aString.Length() + 8);
   TPtr listStringPtr = listString->Des();
   CreateListBoxItemL( listStringPtr, aString );
   iList->AppendL( *listString );
   CleanupStack::PopAndDestroy(listString);
}

void CSearchableListBoxView::SetupCachedCountryListL()
{
   if (iList) {
      iList->Reset();   
      delete iList;
      iList = NULL;
   }
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   int preallocatedListPositions = MAX(5, dataHolder->GetNbrCountries()/*+1*/);
   iList = new (ELeave) CDesCArrayFlat(preallocatedListPositions);

   /*
   if( iWayfinderAppUi->IsGpsAllowed() ){
      HBufC* buf = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_FROM_GPS_TEXT);
      AddListBoxItemL( *buf );
      CleanupStack::PopAndDestroy(buf);
   }
   */
   
   iActiveCountryIndex = 0;
   if( dataHolder->GetNbrCountries() > 0 )
     {
       for( TInt i=0; i < dataHolder->GetNbrCountries(); ++i )
         {
           HBufC* country = dataHolder->GetCountry(i);
           if (!country->Compare(iWayfinderAppUi->GetFormData().iCountry))
             {
               iActiveCountryIndex = i;
             }
           AddListBoxItemL(*country);
         }
     }
     if (iWayfinderAppUi->GetStoredCountryIndex() == 0)
     {
     	iWayfinderAppUi->SetStoredCountryIndex(iActiveCountryIndex); // save away the country
     }
}

TInt CSearchableListBoxView::GetActiveCountryIndex()
{
  return iActiveCountryIndex;
}

void CSearchableListBoxView::SetActiveCountryIndex(TInt aIndex)
{
	iActiveCountryIndex = aIndex;
}

void CSearchableListBoxView::SetupCachedCategoryListL()
{
   if (iList) {
      iList->Reset();   
      delete iList;
      iList = NULL;
   }
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   int preallocatedListPositions = MAX(5, dataHolder->GetNbrCategories());
   iList = new (ELeave) CDesCArrayFlat(preallocatedListPositions);

   if( dataHolder->GetNbrCategories() > 0 ){
      for( TInt i=0; i < dataHolder->GetNbrCategories(); ++i ){
         AddListBoxItemL( *dataHolder->GetCategory(i) );
      }
   }
   iActiveCountryIndex = 0;
}

void CSearchableListBoxView::SetupListL( MDesCArray& aArray)
{
   iContainer->AddFormattedListBoxItemsL( &aArray );
}

void CSearchableListBoxView::DoActivateL( const TVwsViewId& aPrevViewId,
                                     TUid aCustomMessageId,
                                     const TDesC8& aCustomMessage )
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   //SetupStatusPaneL();
   // Explicitly hide the arrows in navipane since on s60v5 they tend to 
   // show up in some views otherwise.
   iWayfinderAppUi->setNaviPane(EFalse);

   CEikButtonGroupContainer* cba = AppUi()->Cba();
   if ( cba != NULL ) {
      cba->MakeVisible( EFalse );
   }
	
   if ( iContainer == NULL ) {
      iContainer = CSearchableListBoxContainer::NewL( ClientRect(), *this, this );
      iContainer->SetMopParent( this );
      AppUi()->AddToStackL( *this, iContainer );
   }
   iContainer->AddFormattedListBoxItemsL(iList);
   if (iActiveCountryIndex >= 0)
     {
       iContainer->SetSelectionL(iActiveCountryIndex);
     }
   if (/*IsWFCustomCommandId(aCustomMessageId.iUid) &&*/ aCustomMessage.Length() == 1) {
      TApaTaskList tlist(CEikonEnv::Static()->WsSession());
//       TApaTask app(tlist.FindApp(_L("Wayfinder")));
      TApaTask app(tlist.FindApp(iWayfinderAppUi->GetAppUid()));
      TChar key(*aCustomMessage.Ptr());
      TKeyEvent event;
      event.iCode = key;
      //event.iScanCode = 50;
      event.iModifiers = EModifierNumLock;
      event.iRepeats = 0;
      app.SendKey(event);
      
      //		TKeyEvent keyEvent;
      //		TChar key(*aCustomMessage.Ptr());
      //		keyEvent.iCode = key;
      //		iSTestListBox->OfferKeyEventL(keyEvent, EEventKey);
   }
}

void CSearchableListBoxView::DoDeactivate()
{
   //CleanupStatusPane();
	
   CEikButtonGroupContainer* cba = AppUi()->Cba();
   if ( cba != NULL ) {
      cba->MakeVisible( ETrue );
      cba->DrawDeferred();
   }
	
   if ( iContainer != NULL ) {
      AppUi()->RemoveFromViewStack( *this, iContainer );
      delete iContainer;
      iContainer = NULL;
   }
}

void CSearchableListBoxView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if ( aResourceId == R_WAYFINDER_SEARCHABLE_LISTBOX_MENU ) {
      // nada for now
      if (iContainer && iContainer->CountListBoxItems() > 0) {
         aMenuPane->SetItemDimmed(EWayFinderCmdCSSelect, EFalse);
      } else {
         aMenuPane->SetItemDimmed(EWayFinderCmdCSSelect, ETrue);
      }
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

