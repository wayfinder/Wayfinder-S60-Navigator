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
#include <akncontext.h>
#include <akntitle.h>
#include <eikbtgpc.h>

#include <arch.h>

#include "WayFinderConstants.h"
#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "RsgInclude.h"

//#include <STest.rsg>
//#include "STest.hrh"
//#include "STestForm.hrh"

#include "WayFinderAppUi.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"
#include "Dialogs.h"
#include "WFTextUtil.h"
#include "MC2Coordinate.h"

#include "CSCategoryResultView.h"
#include "CSCategoryResultContainer.h"
#include "CombinedSearchDispatcher.h"
#include "GuiProt/SearchPrintingPolicyFactory.h"
#include "CallBackDialog.h"

/**
 * First phase of Symbian two-phase construction. Should not contain any
 * code that could leave.
 */
CCSCategoryResultView::CCSCategoryResultView(CWayFinderAppUi* aWayfinderAppUi) : 
   CViewBase(aWayfinderAppUi)
{
   iContainer = NULL;
}

/** 
 * The view's destructor removes the container from the control
 * stack and destroys it.
 */
CCSCategoryResultView::~CCSCategoryResultView()
{
   delete iContainer;
   iContainer = NULL;
   delete iEventGenerator;
}

/**
 * Symbian two-phase constructor.
 * This creates an instance then calls the second-phase constructor
 * without leaving the instance on the cleanup stack.
 * @return new instance of CCSCategoryResultView
 */
CCSCategoryResultView* CCSCategoryResultView::NewL(CWayFinderAppUi* aWayfinderAppUi)
{
   CCSCategoryResultView* self = CCSCategoryResultView::NewLC(aWayfinderAppUi);
   CleanupStack::Pop( self );
   return self;
}

/**
 * Symbian two-phase constructor.
 * This creates an instance, pushes it on the cleanup stack,
 * then calls the second-phase constructor.
 * @return new instance of CCSCategoryResultView
 */
CCSCategoryResultView* CCSCategoryResultView::NewLC(CWayFinderAppUi* aWayfinderAppUi)
{
   CCSCategoryResultView* self = new ( ELeave ) CCSCategoryResultView(aWayfinderAppUi);
   CleanupStack::PushL( self );
   self->ConstructL();
   return self;
}


/**
 * Second-phase constructor for view.  
 * Initialize contents from resource.
 */ 
void CCSCategoryResultView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_CS_CATEGORY_RESULT_VIEW );
}
	
/**
 * @return The UID for this view
 */
TUid CCSCategoryResultView::Id() const
{
   return TUid::Uid( EWayFinderCSCategoryResultView );
}

/**
 * Handle a command for this view (override)
 * @param aCommand command id to be handled
 */
void CCSCategoryResultView::HandleCommandL( TInt aCommand )
{   
   // [[[ begin generated region: do not modify [Generated Code]
   switch ( aCommand ) {
   case EWayFinderCmdCSSelect:
   case EWayFinderCmdCSDetailedResultView:
      if (iContainer) {
         iCurrIndex = iContainer->GetSelectedIndex();
      } else {
         iCurrIndex = 0;
      }
      if (SelectedCategoryEmpty()) {
         // Display message to the user that the selected category
         // contains no hits.
         WFDialog::ShowInformationL(R_WF_NO_RESULTS, iCoeEnv);
      } else if (TypeOfHitsInSelectedCategory() == 1) {
         // Category list contains area matches and not search hits
         DisplayAreaMatchesListL();         
      } else {
         iWayfinderAppUi->HandleCommandL(EWayFinderCmdCSDetailedResultView);
      } 
      break;
   case EWayFinderCmdNewDestNavigate:
      {
         NavigateTo();
         break;
      }
   case EWayFinderCmdNewDestShowInfo:
      {
         ShowMoreInformation();
         break;
      }
   case EWayFinderCmdNewDestOrigin:
      {
         SetAsOrigin();
         break;
      }
   case EWayFinderCmdNewDestDestination:
      {
         SetAsDestination();
         break;
      }
   case EWayFinderCmdNewDestSave:
      {
         SaveAsFavorite();
         break;
      }
   case EWayFinderCmdNewDestShow:
      {
         ShowOnMap();
         break;
      }
   case EWayFinderCmdNewDestSendTo:
      {
         SendAsFavoriteL();
         break;
      }
   default:
      iWayfinderAppUi->HandleCommandL(aCommand);
      break;
   }	
}

void CCSCategoryResultView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if (aResourceId == R_WAYFINDER_CS_CATEGORY_MENU) {
      /* Dim everything, un-dim it below. */
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSave, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShow, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestOrigin, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestDestination, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSendTo, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdCSSelect, EFalse );

      if (iContainer->IsTopHit()) {
         if (iCustomMessageId == ERoutePlannerSetAsOrigin ||
             iCustomMessageId == ERoutePlannerSetAsDest) {
            if (iCustomMessageId == ERoutePlannerSetAsOrigin) {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestOrigin, EFalse );
            }
            if (iCustomMessageId == ERoutePlannerSetAsDest) {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestDestination, EFalse );
            }
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShow, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSendTo, EFalse );
         } else {
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSave, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShow, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSendTo, EFalse );

            if (iWayfinderAppUi->IsIronVersion()) {
               aMenuPane->SetItemTextL(EWayFinderCmdNewDestNavigate, R_WF_SET_AS_DEST);
            }
            
            if (iWayfinderAppUi->HideServicesInfo()) {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, ETrue );
            } else {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, EFalse );
            }            
            if( IsGpsAllowed() ) {
               // We have GPS.
               // Add "Calculate Route" option. 
               if (iWayfinderAppUi->IsIronVersion()) {
                  aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, ETrue );
               } else {
                  aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, EFalse );
               }
            } else {
               // No GPS available. Don't show "Calculate Route" option.
               // ...but show the set as start/destination at the top of menu. 
               if (iWayfinderAppUi->IsIronVersion()) {
                  aMenuPane->SetItemDimmed(EWayFinderCmdNewDestOrigin, ETrue );
                  aMenuPane->SetItemDimmed(EWayFinderCmdNewDestDestination, ETrue );
               } else {
                  aMenuPane->SetItemDimmed(EWayFinderCmdNewDestOrigin, EFalse );
                  aMenuPane->SetItemDimmed(EWayFinderCmdNewDestDestination, EFalse );
               }
            }
         }
         aMenuPane->SetItemDimmed(EWayFinderCmdCSSelect, ETrue);
      } 
   }
   AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}

void CCSCategoryResultView::DoActivateL( const TVwsViewId& aPrevViewId,
                                         TUid aCustomMessageId,
                                         const TDesC8& aCustomMessage )
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   // Explicitly hide the arrows in navipane since on s60v5 they tend to 
   // show up in some views otherwise.
   iWayfinderAppUi->setNaviPane(EFalse);
    HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_SEARCH_RESULTS_TEXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   CEikButtonGroupContainer* cba = AppUi()->Cba();
   if ( cba != NULL ) {
      cba->MakeVisible( EFalse );
   }
	
   if ( iContainer == NULL ) {
      iContainer = CCSCategoryResultContainer::NewL( ClientRect(), this );
      iContainer->SetMopParent( this );
      AppUi()->AddToStackL( *this, iContainer );
      iContainer->AddCategoryResultsL(iWayfinderAppUi->GetCSDispatcher()->getCombinedSearchResults());
   }

   iContainer->SetSelectedIndex(iCurrentListBoxIndex);
}

void CCSCategoryResultView::DoDeactivate()
{
   CEikButtonGroupContainer* cba = AppUi()->Cba();
   if ( cba != NULL ) {
      cba->MakeVisible( ETrue );
      cba->DrawDeferred();
   }
	
   if ( iContainer != NULL ) {
      iCurrentListBoxIndex = iContainer->GetCurrentIndex();
      AppUi()->RemoveFromViewStack( *this, iContainer );
      delete iContainer;
      iContainer = NULL;
   }
}

TPtrC CCSCategoryResultView::GetCSIconPath()
{
   return iWayfinderAppUi->iPathManager->GetCSIconPath();
}

TPtrC CCSCategoryResultView::GetMbmName()
{
   return iWayfinderAppUi->iPathManager->GetMbmName();
}

void CCSCategoryResultView::AddCategories()
{
   if (iContainer) {
      iContainer->AddCategoryResultsL(iWayfinderAppUi->GetCSDispatcher()->getCombinedSearchResults()); 
   }
}

void CCSCategoryResultView::AddCategories(const std::vector<CombinedSearchCategory*>& categories)
{
   if (iContainer) {
      iContainer->AddCategoryResultsL(categories); 
   }
}

void CCSCategoryResultView::RetryAddCategories()
{
   if (iContainer) {
      iContainer->AddCategoryResultsL(iWayfinderAppUi->GetCSDispatcher()->
                                      getCombinedSearchCategoryResults());   
//       iContainer->AddCategoryResultsL(iWayfinderAppUi->GetCSDispatcher()->getCombinedSearchResults());   
   }
}

TInt CCSCategoryResultView::GetCurrentIndex()
{
   return iCurrIndex;
}

TBool CCSCategoryResultView::SelectedCategoryEmpty()
{
   CSCatVec_t catResults = iWayfinderAppUi->GetCSDispatcher()->
      getCombinedSearchDataHolder().getCombinedSearchCategoryResults();
   CombinedSearchCategory* cat = catResults.at(iCurrIndex);
   if (cat && cat->getTotalNbrHits() > 0) {
      return EFalse;
   }
   return ETrue;
}

TInt CCSCategoryResultView::TypeOfHitsInSelectedCategory()
{
   CSCatVec_t catResults = iWayfinderAppUi->GetCSDispatcher()->
      getCombinedSearchDataHolder().getCombinedSearchCategoryResults();
   CombinedSearchCategory* cat = catResults.at(iCurrIndex);
   if (!cat) {
      return -1;
   }
   return cat->getTypeOfHits();
}

TBool CCSCategoryResultView::IsGpsAllowed()
{
   return iWayfinderAppUi->IsGpsAllowed();
}

void CCSCategoryResultView::ShowOnMap()
{
   if (iContainer) {
      const CSSIVec_t& topHits = iWayfinderAppUi->GetCombinedSearchDataHolder().getTopHitResults();
      if (iContainer->IsTopHit() && iContainer->GetCurrentIndex() < (int)topHits.size()) {
         const isab::SearchItem& searchItem = topHits.at(iContainer->GetCurrentIndex());
         if (searchItem.getLat() != MAX_INT32 && 
             searchItem.getLon() != MAX_INT32) {
            iWayfinderAppUi->RequestMap( MapEnums::SearchPosition, 
                                         searchItem.getLat(), searchItem.getLon(), 
                                         EShowOnMap );
         } else {
            WFDialog::ShowScrollingInfoDialogL( R_CS_NO_COORDS_MSG );
         }
      }
   }
}

void CCSCategoryResultView::ShowMoreInformation()
{
   if (iContainer) {
      const CSSIVec_t& topHits = iWayfinderAppUi->GetCombinedSearchDataHolder().getTopHitResults();
      if (iContainer->IsTopHit() && iContainer->GetCurrentIndex() < (int)topHits.size()) {
         const isab::SearchItem& searchItem = topHits.at(iContainer->GetCurrentIndex());
         Nav2Coordinate nav2Coord(searchItem.getLat(), searchItem.getLon());
         MC2Coordinate position(nav2Coord);
         HBufC* id = WFTextUtil::AllocLC( searchItem.getID() );
         iWayfinderAppUi->GotoInfoInServiceViewL(position.lat, position.lon, id);
         CleanupStack::PopAndDestroy( id );
      }
   }
}

void CCSCategoryResultView::SetAsOrigin()
{
   TBuf<KBuf256Length> name;
   if (iContainer) {
      const CSSIVec_t& topHits = iWayfinderAppUi->GetCombinedSearchDataHolder().getTopHitResults();
      if (iContainer->IsTopHit() && iContainer->GetCurrentIndex() < (int)topHits.size()) {
         const isab::SearchItem& searchItem = topHits.at(iContainer->GetCurrentIndex());
         if (searchItem.getLat() != MAX_INT32 && 
             searchItem.getLon() != MAX_INT32) {
            HBufC* name = WFTextUtil::AllocLC( searchItem.getName() );
            iWayfinderAppUi->SetOrigin( GuiProtEnums::PositionTypePosition, 
                                        *name, searchItem.getID(),
                                        searchItem.getLat(), searchItem.getLon() );  
            CleanupStack::PopAndDestroy( name );
            iWayfinderAppUi->push( KPositionSelectViewId );
         } else {
            WFDialog::ShowScrollingInfoDialogL( R_CS_NO_COORDS_MSG );
         }
      }
   }
}

void CCSCategoryResultView::SetAsDestination()
{
   TBuf<KBuf256Length> name;
   if (iContainer) {
      const CSSIVec_t& topHits = iWayfinderAppUi->GetCombinedSearchDataHolder().getTopHitResults();
      if (iContainer->IsTopHit() && iContainer->GetCurrentIndex() < (int)topHits.size()) {
         const isab::SearchItem& searchItem = topHits.at(iContainer->GetCurrentIndex());
         if (searchItem.getLat() != MAX_INT32 && 
             searchItem.getLon() != MAX_INT32) {
            HBufC* name = WFTextUtil::AllocLC( searchItem.getName() );
            iWayfinderAppUi->SetDestination( GuiProtEnums::PositionTypeSearch,
                                             *name, searchItem.getID(),
                                             searchItem.getLat(), searchItem.getLon() );
            CleanupStack::PopAndDestroy( name );
            iWayfinderAppUi->push( KPositionSelectViewId );
         } else {
            WFDialog::ShowScrollingInfoDialogL( R_CS_NO_COORDS_MSG );
         }
      }
   }
}

void CCSCategoryResultView::NavigateTo()
{
   if (iWayfinderAppUi->
       CheckAndDoGpsConnectionL((void*) this, 
                                CCSCategoryResultView::WrapperToNavigateToCallback)) {
      // We're connected or connecting to a gps so go a head.
      iWayfinderAppUi->SetFromGps();
      HandleCommandL(EWayFinderCmdNewDestDestination);
   }
}

void CCSCategoryResultView::NavigateToCallback()
{
   iWayfinderAppUi->SetFromGps();
   HandleCommandL(EWayFinderCmdNewDestDestination);
}

void CCSCategoryResultView::WrapperToNavigateToCallback(void* pt2Object)
{
   CCSCategoryResultView* self = (CCSCategoryResultView*) pt2Object;
   self->NavigateToCallback();
}

void CCSCategoryResultView::SaveAsFavorite()
{
   if( iContainer){
      // Pick up the search item for the selected listbox index
      const CSSIVec_t& topHits = iWayfinderAppUi->GetCombinedSearchDataHolder().getTopHitResults();
      if (iContainer->IsTopHit() && iContainer->GetCurrentIndex() < (int)topHits.size()) {
         const isab::SearchItem& searchItem = topHits.at(iContainer->GetCurrentIndex());
         if (searchItem.getLat() != MAX_INT32 && 
             searchItem.getLon() != MAX_INT32) {
            iWayfinderAppUi->AddFavoriteFromSearch(searchItem);
         } else {
            WFDialog::ShowScrollingInfoDialogL( R_CS_NO_COORDS_MSG );
         }
      }
   }
}

TBool CCSCategoryResultView::SettingOrigin()
{
   // Return true if iCustomMessageId is set as ERoutePlannerSetAsOrigin
   return (iCustomMessageId == ERoutePlannerSetAsOrigin);
}

TBool CCSCategoryResultView::SettingDestination()
{
   // Return true if iCustomMessageId is set as ERoutePlannerSetAsDest
   return (iCustomMessageId == ERoutePlannerSetAsDest);
}

void CCSCategoryResultView::SendAsFavoriteL()
{   
   if( iContainer ) {
      // Pick up the search item for the selected listbox index
      const CSSIVec_t& topHits = iWayfinderAppUi->GetCombinedSearchDataHolder().getTopHitResults();
      if (iContainer->IsTopHit() && iContainer->GetCurrentIndex() < (int)topHits.size()) {
         const isab::SearchItem& searchItem = topHits.at(iContainer->GetCurrentIndex());
         if (searchItem.getLat() != MAX_INT32 && 
             searchItem.getLon() != MAX_INT32) {
            char *parsed = 
               isab::SearchPrintingPolicyFactory::
               parseSearchItem(&searchItem, false,
                               isab::DistancePrintingPolicy::
                               DistanceMode(iWayfinderAppUi->GetDistanceMode()));
         
            Favorite* fav = new Favorite(searchItem.getLat(), 
                                         searchItem.getLon(), 
                                         searchItem.getName(),
                                         "", parsed, "", "");
         
            delete[] parsed;
            iWayfinderAppUi->SendFavoriteL(fav);
         } else {
            WFDialog::ShowScrollingInfoDialogL( R_CS_NO_COORDS_MSG );
         }
      }
   }
}

void CCSCategoryResultView::AreaMatchSearchResultReceived(TBool aSwitchToResultView) 
{
   // Cancel the currently displayed info message
   iWayfinderAppUi->InfoMsgCancel();
   if (iContainer) {
      // Add the results and set the index to point to the heading just got the
      // area match results
      iContainer->AddCategoryResultsL(iWayfinderAppUi->GetCSDispatcher()->getCombinedSearchResults());
      iContainer->SetSelectedIndex(iAreaMatchHeadingIndex);
   }
   if (aSwitchToResultView) {
      // Swith to detailed result view
      HandleCommandL(EWayFinderCmdCSDetailedResultView);
   }
}

_LIT ( KStringHeader, "%d\t%S" );
void CCSCategoryResultView::DisplayAreaMatchesListL()
{
   // Store the index that contained area matches
   iAreaMatchHeadingIndex = iContainer->GetCurrentIndex();
   // Get the vector containing all heading in the search result
   CSCatVec_t catResults = iWayfinderAppUi->GetCSDispatcher()->
      getCombinedSearchDataHolder().getCombinedSearchResults();
   // Get the category for the index the user selected
   const CombinedSearchCategory* category = catResults.at(GetCurrentIndex());
   // Get the vector containing the actual results, in this case area matches
   const CSSIVec_t& results = category->getResults();
   TBuf<128> areaName;
   TBuf<128> imageName;
   CDesCArray* areaList = new (ELeave) CDesCArrayFlat(results.size());
   CArrayPtr<CGulIcon>* iconList = new (ELeave) CAknIconArray(results.size());
   TInt i = 0;
   // Iterate through the area matches and format strings, text array and the icon array
   // to be displayed in the list dialog
   for (CSSIVec_t::const_iterator it = results.begin(); it != results.end(); ++it, ++i) {
      WFTextUtil::char2TDes(imageName, it->getImageName());
      iconList->AppendL(iContainer->LoadAndScaleIconL(imageName));
      WFTextUtil::char2TDes(areaName, it->getName());
      HBufC* listString = HBufC::NewLC(areaName.Length() + 5);
      listString->Des().Format(KStringHeader, iconList->Count()-1, &areaName);
      areaList->AppendL(*listString);
      CleanupStack::PopAndDestroy(listString);
   }
   // Launch the list dialog displaying the area matches.
   typedef TCallBackEvent<CCSCategoryResultView, TCSState> cb_t;
   typedef CCallBackListDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
                        EAreaMatchSelected,
                        EAreaMatchCancelled),
                   iSelectedIndex,
                   WFTextUtil::AllocL(category->getName()),
                   areaList,
                   R_WAYFINDER_CALLBACK_SINGLE_ICON_LIST_QUERY,
                   EFalse,
                   0,
                   iconList);
}

void CCSCategoryResultView::GenerateEvent(enum TCSState aEvent)
{
   if (!iEventGenerator) {
      iEventGenerator = CSCatGenerator::NewL(*this); 
   }
   iEventGenerator->SendEventL(aEvent);   
}

void CCSCategoryResultView::HandleGeneratedEventL(enum TCSState aEvent)
{
   switch(aEvent)
   {
   case EAreaMatchSelected:
      {
         // User selected a area to search with
         CombinedSearchCategory* category = iWayfinderAppUi->GetCSDispatcher()->
            getCombinedSearchDataHolder().getCombinedSearchResults().at(GetCurrentIndex());
         const CSSIVec_t& results = category->getResults();
         const isab::SearchItem& item = results.at(iSelectedIndex);
         // Dispatch the area match search
         iWayfinderAppUi->DispatchAreaMatchSearchL(item, category->getHeadingNo());
         iWayfinderAppUi->ShowInfoMsg(R_WAYFINDER_IAP_SEARCH2_MSG, 20*1000*1000);         
      }
      break;
   case EAreaMatchCancelled:
      break;
   }
}

