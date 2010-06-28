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
#include "Dialogs.h"

#include "WayFinderAppUi.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"
#include "WFTextUtil.h"
#include "MC2Coordinate.h"

#include "CSDetailedResultView.h"
#include "CSDetailedResultContainer.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/SearchPrintingPolicyFactory.h"

/**
 * First phase of Symbian two-phase construction. Should not contain any
 * code that could leave.
 */
CCSDetailedResultView::CCSDetailedResultView(CWayFinderAppUi* aWayfinderAppUi) :
   CViewBase(aWayfinderAppUi), iOffset(0)
{
   iContainer = NULL;
}

/** 
 * The view's destructor removes the container from the control
 * stack and destroys it.
 */
CCSDetailedResultView::~CCSDetailedResultView()
{
   delete iContainer;
   iContainer = NULL;
}

/**
 * Symbian two-phase constructor.
 * This creates an instance then calls the second-phase constructor
 * without leaving the instance on the cleanup stack.
 * @return new instance of CCSDetailedResultView
 */
CCSDetailedResultView* CCSDetailedResultView::NewL(CWayFinderAppUi* aWayfinderAppUi)
{
   CCSDetailedResultView* self = CCSDetailedResultView::NewLC(aWayfinderAppUi);
   CleanupStack::Pop( self );
   return self;
}

/**
 * Symbian two-phase constructor.
 * This creates an instance, pushes it on the cleanup stack,
 * then calls the second-phase constructor.
 * @return new instance of CCSDetailedResultView
 */
CCSDetailedResultView* CCSDetailedResultView::NewLC(CWayFinderAppUi* aWayfinderAppUi)
{
   CCSDetailedResultView* self = new ( ELeave ) CCSDetailedResultView(aWayfinderAppUi);
   CleanupStack::PushL( self );
   self->ConstructL();
   return self;
}


/**
 * Second-phase constructor for view.  
 * Initialize contents from resource.
 */ 
void CCSDetailedResultView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_CS_DETAILED_RESULT_VIEW );
	
   // add your own initialization code here
}
	
/**
 * @return The UID for this view
 */
TUid CCSDetailedResultView::Id() const
{
   return TUid::Uid( EWayFinderCSDetailedResultView );
}

/**
 * Handle a command for this view (override)
 * @param aCommand command id to be handled
 */
void CCSDetailedResultView::HandleCommandL( TInt aCommand )
{   
   switch ( aCommand ) {
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
   case EWayFinderCmdCSSelect:
      {
         if (iContainer && iContainer->IndexIsNext()) {
            iContainer->ShowNextResults();
         } else if (iContainer && iContainer->IndexIsPrev()) {
            iContainer->ShowPreviousResults();
         }
         break;
      }
   default:
      iWayfinderAppUi->HandleCommandL(aCommand);
      break;
   }	
}

/**
 *	Handles user actions during activation of the view, 
 *	such as initializing the content.
 */
void CCSDetailedResultView::DoActivateL( const TVwsViewId& aPrevViewId,
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
      iContainer = CCSDetailedResultContainer::NewL( ClientRect(), this, iOffset, iIndex );
      iContainer->SetMopParent( this );
      AppUi()->AddToStackL( *this, iContainer );

      iContainer->AddResultsL(iWayfinderAppUi->GetCSCategoryCurrentIndex(),
                              iWayfinderAppUi->GetCombinedSearchResults());
   }
   iContainer->ShowPreviewContent(ETrue);
   iContainer->UpdatePreviewContentL();
}

/**
 */
void CCSDetailedResultView::DoDeactivate()
{
   CEikButtonGroupContainer* cba = AppUi()->Cba();
   if ( cba != NULL ) {
      cba->MakeVisible( ETrue );
      cba->DrawDeferred();
   }
	
   if ( iContainer != NULL ) {
      iContainer->GetIndexes(iOffset, iIndex);
      AppUi()->RemoveFromViewStack( *this, iContainer );
      delete iContainer;
      iContainer = NULL;
   }
}

void CCSDetailedResultView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_NEWDEST_MENU ){
      /* Dim everything, un-dim it below. */
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSave, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShow, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestOrigin, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestDestination, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSendTo, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdCSSelect, EFalse );

      if ( iContainer && iContainer->HasResults() ){
         /* There are results in the list. */
         TBool nextOrPrev = iContainer->IndexIsPrev() || iContainer->IndexIsNext();
         if (!nextOrPrev) {
            // If user is located at next or previous string in the list, 
            // navigate to, set as destination/origin should not be visible and
            // select prev/nex command should no be visible.
            aMenuPane->SetItemDimmed( EWayFinderCmdCSSelect, ETrue );
      
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

               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, 
                                         iWayfinderAppUi->HideServicesInfo() );

               if (iWayfinderAppUi->CanUseGPSForEveryThing() && 
                    !iWayfinderAppUi->IsIronVersion()) {
                  // Not iron and the user has the correct rights
                  aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, EFalse );
               } else {
                  aMenuPane->SetItemTextL(EWayFinderCmdNewDestNavigate, R_WF_SET_AS_DEST);
                  aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, EFalse );
               }
            }
         } 
      }
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

void CCSDetailedResultView::AddResultsL(TInt aIndex, const CSCatVec_t& aResults, 
                                        TBool aMoreServerHitsReceived)
{
   if (iContainer) {
      iContainer->AddResultsL(aIndex, aResults, aMoreServerHitsReceived);
      iContainer->ShowPreviewContent(ETrue);
      iContainer->UpdatePreviewContentL();
   }
}

TPtrC CCSDetailedResultView::GetCSIconPath()
{
   return iWayfinderAppUi->iPathManager->GetCSIconPath();
}

TPtrC CCSDetailedResultView::GetMbmName()
{
   return iWayfinderAppUi->iPathManager->GetMbmName();
}

TInt CCSDetailedResultView::GetDistanceMode()
{
   DistancePrintingPolicy::DistanceMode mode = 
      DistancePrintingPolicy::DistanceMode(iWayfinderAppUi->GetDistanceMode());
   switch (mode) {
   case DistancePrintingPolicy::ModeImperialYards:
      return DistancePrintingPolicy::ModeImperialYardsSpace;
      break;
   case DistancePrintingPolicy::ModeImperialFeet:
      return DistancePrintingPolicy::ModeImperialFeetSpace;
      break;
   case DistancePrintingPolicy::ModeInvalid:
   case DistancePrintingPolicy::ModeMetric:
   default:
      return DistancePrintingPolicy::ModeMetricSpace;
      break;
   }
}

const std::vector<CombinedSearchCategory*>& 
CCSDetailedResultView::GetSearchResults()
{
   return iWayfinderAppUi->GetCombinedSearchResults();
}

TInt CCSDetailedResultView::GetCSCategoryCurrentIndex()
{
   return iWayfinderAppUi->GetCSCategoryCurrentIndex();
}

void CCSDetailedResultView::RequestMoreHits(TUint32 aLowerBound, TUint32 aHeadingNo)
{
   iWayfinderAppUi->CSReqestMoreHits(aLowerBound, aHeadingNo);
}

TBool CCSDetailedResultView::IsGpsAllowed()
{
   return iWayfinderAppUi->IsGpsAllowed();
}

TBool CCSDetailedResultView::ValidGpsStrength()
{
   return iWayfinderAppUi->ValidGpsStrength();
}

TPoint CCSDetailedResultView::GetCurrentPosition()
{
   return iWayfinderAppUi->GetCurrentPosition();
}

void CCSDetailedResultView::ShowOnMap()
{
   if (iContainer) {
      const isab::SearchItem& searchItem = 
         GetSelectedSearchItem(iContainer->GetCalculatedIndex());
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

void CCSDetailedResultView::ShowMoreInformation()
{
   if (iContainer) {
      const isab::SearchItem& searchItem = 
         GetSelectedSearchItem(iContainer->GetCalculatedIndex());     
      Nav2Coordinate nav2Coord(searchItem.getLat(), searchItem.getLon());
      MC2Coordinate position(nav2Coord);
      HBufC* id = WFTextUtil::AllocLC( searchItem.getID() );
      iWayfinderAppUi->GotoInfoInServiceViewL(position.lat, position.lon, id);
      CleanupStack::PopAndDestroy( id );

//       CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
//       TPoint pos = dataHolder->GetNewDestPos(iContainer->GetSelIndex());
//       Nav2Coordinate nav2Coord(pos.iX, pos.iY);
//       MC2Coordinate position(nav2Coord);
//       iWayfinderAppUi->GotoInfoInServiceViewL(position.lat, position.lon);
   }

}

void CCSDetailedResultView::SetAsOrigin()
{
   TBuf<KBuf256Length> name;
   if (iContainer) {
      const isab::SearchItem& searchItem = GetSelectedSearchItem(iContainer->GetCalculatedIndex());
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

void CCSDetailedResultView::SetAsDestination()
{
   TBuf<KBuf256Length> name;
   if (iContainer) {
      const isab::SearchItem& searchItem = GetSelectedSearchItem( iContainer->GetCalculatedIndex() );
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

void CCSDetailedResultView::Route() 
{
   if(iContainer) {
     const isab::SearchItem& searchItem = GetSelectedSearchItem( iContainer->GetCalculatedIndex() );
     const char* name = searchItem.getID();
     char* nameStr = WFTextUtil::strdupLC(name);
     iWayfinderAppUi->
       SetPendingRouteMessageAndSendVehicleTypeL(
          GuiProtEnums::PositionTypeSearch,
          nameStr, searchItem.getLat(), searchItem.getLon(), "");
     CleanupStack::PopAndDestroy(); // nameStr
   }
}

const isab::SearchItem& CCSDetailedResultView::GetSelectedSearchItem( TInt aIndex )
{
   // Get the selected category index
   TInt catIndex = iWayfinderAppUi->GetCSCategoryCurrentIndex();
   // Get the selected searchItem by first getting the correct searchItems vector and
   // then get the correct searchItem from that vector.
   return iWayfinderAppUi->GetCombinedSearchResults().at(catIndex)->getResults().at(aIndex);
}

void CCSDetailedResultView::ResetIndexes()
{
   iOffset = 0;
   iIndex = 0;
}

void CCSDetailedResultView::NavigateTo()
{
   if (iWayfinderAppUi->
       CheckAndDoGpsConnectionL((void*) this, 
                                CCSDetailedResultView::WrapperToNavigateToCallback)) {
      // We're connected or connecting to a gps so go a head.
      iWayfinderAppUi->SetFromGps();
      //HandleCommandL(EWayFinderCmdNewDestDestination);
      Route();
   }
}

void CCSDetailedResultView::NavigateToCallback()
{
   iWayfinderAppUi->SetFromGps();
   //HandleCommandL(EWayFinderCmdNewDestDestination);
   Route();
}

void CCSDetailedResultView::WrapperToNavigateToCallback(void* pt2Object)
{
   CCSDetailedResultView* self = (CCSDetailedResultView*) pt2Object;
   self->NavigateToCallback();
}

TBool CCSDetailedResultView::SettingOrigin()
{
   // Return true if iCustomMessageId is set as ERoutePlannerSetAsOrigin
   return (iCustomMessageId == ERoutePlannerSetAsOrigin);
}

TBool CCSDetailedResultView::SettingDestination()
{
   // Return true if iCustomMessageId is set as ERoutePlannerSetAsDest
   return (iCustomMessageId == ERoutePlannerSetAsDest);
}

void CCSDetailedResultView::SaveAsFavorite()
{
   if( iContainer && iContainer->HasResults() ){
      // Pick up the search item for the selected listbox index
      const isab::SearchItem& searchItem = GetSelectedSearchItem(iContainer->GetCalculatedIndex());
      if (searchItem.getLat() != MAX_INT32 && 
          searchItem.getLon() != MAX_INT32) {
         iWayfinderAppUi->AddFavoriteFromSearch(searchItem);
      } else {
         WFDialog::ShowScrollingInfoDialogL( R_CS_NO_COORDS_MSG );
      }
   }
}

void CCSDetailedResultView::SendAsFavoriteL()
{   
   if( iContainer && iContainer->HasResults() ) {
      // Pick up the search item for the selected listbox index
      const isab::SearchItem& searchItem = GetSelectedSearchItem(iContainer->GetCalculatedIndex());
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
