/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <eikmenup.h>
#include <aknview.h>
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h> 

#include "VicinityView.h"
#include "VicinityContainer.h"		// CVicinityContainer
#include "RsgInclude.h"					// dodona strings and resources
#include "WayFinderConstants.h"		// view ids
#include "WayFinderAppUi.h"			// CWayFinderAppUi
#include "MapView.h"						// TileMapEventListener + CMapView
#include "TileMapControl.h"			// CTileMapControl
#include "MapFeatureHolder.h"			// CMapFeatureHolder
#include "MapInfoControl.h"			// CMapInfoControl
#include "VectorMapConnection.h"		// CVectorMapConnection
#include "Quality.h"
#include "GuiProt/GuiProtMess.h"
#include "DetailFetcher.h"
#include "FavoriteFetcher.h"

CVicinityView::CVicinityView(isab::Log * aLog) : iContainer(NULL), iLog(aLog) { }

CVicinityView * 
CVicinityView::NewLC(CWayFinderAppUi * aWayFinderUI, 
                     CMapView * aMapView,
                     isab::Log * aLog, 
                     TUid aViewId) 
{
   CVicinityView * self = new (ELeave) CVicinityView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aWayFinderUI, 
                    aMapView,
                    aViewId);
   
   return self;
}

CVicinityView * 
CVicinityView::NewL(CWayFinderAppUi * aWayFinderUI, 
                    CMapView * aMapView,
                    isab::Log * aLog,
                    TUid aViewId) 
{
   CVicinityView * self = NewLC(aWayFinderUI, 
                                aMapView,
                                aLog, 
                                aViewId);
   CleanupStack::Pop(self);
   
   return self;
}

void
CVicinityView::ConstructL(CWayFinderAppUi * aWayFinderUI,
                          CMapView * aMapView,
                          TUid aViewId) 
{
   BaseConstructL(R_WAYFINDER_VICINITY_VIEW);
   // parent
   iWayFinderUI = aWayFinderUI;
   
   // map related pointers (used to create and handle the map control)
   iMapView = aMapView;
   
   // view id (to decide what content is shown.
   iVicinityId = aViewId;
   
   // map position
   iCenter.iX = MAX_INT32;
   iCenter.iY = MAX_INT32;
   iCurrentScale = MAX_INT32;
}

CVicinityView::~CVicinityView() 
{
   TRACE_FUNC();
   if(iContainer) {
      AppUi()->RemoveFromViewStack(* this, iContainer);
   }
   
   delete iContainer;
}

void
CVicinityView::SetDetailFetcher(DetailFetcher* detailFetcher)
{
   m_detailFetcher = detailFetcher;

   if(iContainer)
      iContainer->SetDetailFetcher(detailFetcher);
}

TUid
CVicinityView::Id() const 
{
   return iVicinityId;
}

void
CVicinityView::HandleCommandL(TInt aCommand) 
{
   switch(aCommand)
      {
      case EWayFinderCmdRefreshVicinityList:
         if(iContainer) {
            iContainer->ForceUpdate();
         }
         break;
      case EWayFinderCmdVicinityNavigate:
         NavigateTo();
         break;
      case EWayFinderCmdVicinityDetails:
         if(iContainer) {
            iContainer->ShowVicinityDetails();
         }
         break;
      case EWayFinderCmdVicinityLockTo:
      {
         if(!iContainer->IsListEmpty() && iWayFinderUI->IsGpsConnected()) {
            VicinityItem* curSelected =
               iContainer->GetSelectedItem();
            iWayFinderUI->GotoLockedNavigationViewL(curSelected,
                                                    iContainer->GetMapLib());
         }

         break;
      }
      case EWayFinderCmdWhereAmI: 
         {
            if(iContainer) {
               iContainer->TriggerWhereAmIL();
            }
         }
         break;
      default:
         AppUi()->HandleCommandL( aCommand );
         break;
      }
}

void
CVicinityView::HandleClientRectChange() 
{
   if (iContainer) {
      iContainer->SetRect(ClientRect());
   }
}

void 
CVicinityView::DoActivateL(const TVwsViewId& aPrevViewId,
                           TUid aCustomMessageId,
                           const TDesC8& aCustomMessage) 
{
   iWayFinderUI->GetNavigationDecorator()->MakeScrollButtonVisible( ETrue );
   if (!iContainer) {
      iContainer = new (ELeave) CVicinityContainer;
      iContainer->SetMopParent(this);
      iContainer->ConstructL(ClientRect(), 
                             this,
                             iWayFinderUI,
                             iMapView);
      if( iGpsPos.lat != MAX_INT32 ) {
         iContainer->setGpsPos( iGpsPos,
                                (int) iHeading360 );
      }
      iContainer->SetDetailFetcher(m_detailFetcher);
      iContainer->SetFavoriteFetcher(m_favoriteFetcher);
      AppUi()->AddToStackL(* this, iContainer);
   }


   /* Special case for fixed point vicinity information */
   if(!iWayFinderUI->IsGpsConnected()) {
      iContainer->ForceUpdate();
   }
   
}

void
CVicinityView::DoDeactivate() 
{
   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }
	
   delete iContainer;
   iContainer = NULL;
}

void
CVicinityView::HandleFavoritesAllDataReply(isab::GetFavoritesAllDataReplyMess* aMessage)
{
   if (iContainer) {
      iContainer->UpdateFavorites(aMessage);
   }
}

TInt 
CVicinityView::GetDistanceMode()
{
   return iWayFinderUI->GetDistanceMode();
}

void 
CVicinityView::UpdateDataL(UpdatePositionMess* aPositionMess, 
                           TInt aSpeed)
{
   TInt realHeading = -1;
   
   if( aPositionMess->headingQuality() > QualityUseless ){
      realHeading = aPositionMess->getHeading();
   }
   
   iHeading360 = 0;
   
   if( realHeading > 0 ){
      iHeading360 = TInt(realHeading * ( 360.0 / 256.0 ) + 0.5);
   }

   iGpsPos = MC2Coordinate(Nav2Coordinate( aPositionMess->getLat(), 
                                           aPositionMess->getLon() ) );
   if( iContainer ){
      iContainer->setGpsPos( iGpsPos,
                             (int) iHeading360);
   }
}

void CVicinityView::NavigateTo()
{
   if( !iContainer->IsListEmpty() ){

      VicinityItem* item = iContainer->GetSelectedItem();
      if (item) {
         Nav2Coordinate nav2Coord( item->Coord() );
         HBufC* description = WFTextUtil::AllocLC(item->Name().c_str());
         
         iWayFinderUI->RouteToCoordinateL(nav2Coord.nav2lat, 
                                         nav2Coord.nav2lon,
                                         *description);
         
         CleanupStack::PopAndDestroy(description);
      }
   }
}

void 
CVicinityView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if (aResourceId == R_WAYFINDER_VICINITY_MENU) {
      
      if (!iWayFinderUI->IsGpsConnected()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdVicinityNavigate, ETrue);
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdVicinityNavigate, EFalse);         
      }
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

void
CVicinityView::RequestFavorites()
{
   GenericGuiMess get(GuiProtEnums::GET_FAVORITES_ALL_DATA, uint16(0),
                      uint16(MAX_UINT16));
   iWayFinderUI->SendMessageL(&get);
}

void CVicinityView::SetFavoriteFetcher(FavoriteFetcher* favoriteFetcher)
{
   m_favoriteFetcher = favoriteFetcher;
   
   if(iContainer)
      iContainer->SetFavoriteFetcher(favoriteFetcher);
}

void CVicinityView::ForceRefresh()
{
   if(iContainer)
      iContainer->ForceUpdate();
}

