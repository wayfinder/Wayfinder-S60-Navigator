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
#include "MapLib.h"
#include "MapResourceFactory.h"
#include "VicinityFeedView.h"
#include "VicinityFeedContainer.h"		// CVicinityContainer
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
#include "TraceMacros.h"
#include "GuideView.h"
#include "Quality.h"
#include "TimeOutTimer.h"
#include "SettingsData.h"

#define READOUT_INTERVAL 23
//#define TORSTEN_BRAND

static const TUint32 KStartRetryTimeOut = 1*1000*10000;

CVicinityFeedView::CVicinityFeedView(isab::Log * aLog) : iLog(aLog) {
   iReferenceTime = TTime(0);
   iContainer = NULL;
}

CVicinityFeedView * 
CVicinityFeedView::NewLC(CWayFinderAppUi * aWayFinderUI,
                         CGuideView* aGuideView,
                         isab::Log * aLog) 
{
   CVicinityFeedView * self = new (ELeave) CVicinityFeedView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aWayFinderUI, aGuideView);
   
   return self;
}

CVicinityFeedView * 
CVicinityFeedView::NewL(CWayFinderAppUi * aWayFinderUI,
                        CGuideView* aGuideView,
                        isab::Log * aLog) 
{
   CVicinityFeedView * self = NewLC(aWayFinderUI,
                                    aGuideView,
                                    aLog);
   CleanupStack::Pop(self);
   
   return self;
}

void
CVicinityFeedView::ConstructL(CWayFinderAppUi * aWayFinderUI,
                              CGuideView* aGuideView) 
{
   BaseConstructL(R_WAYFINDER_VICINITY_FEED_VIEW);
   // parent
   iGuideView = aGuideView;
   iWayFinderUI = aWayFinderUI;
   
   // map related pointers (used to create and handle the map control)

   // map position
   iPrevLat = MAX_INT32;
   iPrevLon = MAX_INT32;
   iCenter.iX = MAX_INT32;
   iCenter.iY = MAX_INT32;
   iCurrentScale = MAX_INT32;
}

CVicinityFeedView::~CVicinityFeedView() 
{

   if(iContainer) {
      AppUi()->RemoveFromViewStack(* this, iContainer);
   }
   
   delete iContainer;
}

TUid
CVicinityFeedView::Id() const 
{
   return KVicinityFeedId;
}

void
CVicinityFeedView::HandleCommandL(TInt aCommand) 
{
   AppUi()->HandleCommandL( aCommand );
}

void
CVicinityFeedView::HandleClientRectChange() 
{
   if (iContainer) {
      iContainer->SetRect(ClientRect());
   }
}

void 
CVicinityFeedView::DoActivateL(const TVwsViewId& aPrevViewId,
                           TUid aCustomMessageId,
                               const TDesC8& aCustomMessage) 
{
//   iWayFinderUI->GetNavigationDecorator()->MakeScrollButtonVisible( ETrue );
   iReferenceTime = TTime(0); //Ensure immediate redraw

   TRACE_FUNC1("Activating feed-view. ");
   
   if (!iContainer) {
      iContainer = new (ELeave) CVicinityFeedContainer;
      iContainer->SetMopParent(this);
      iContainer->ConstructL(ClientRect(), 
                             this,
                             iGuideView,
                             iWayFinderUI);
   }

   TRACE_FUNC1("Container created.");
   
   TRACE_FUNC1("Adding to stack. ");
   AppUi()->AddToStackL(* this, iContainer);
   
   Cba()->MakeVisible(ETrue);
   Cba()->DrawNow();
//   Cba()->MakeCommandVisible(EAknSoftkeyOk, ETrue);
}

void
CVicinityFeedView::DoDeactivate() 
{
   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }
	
   delete iContainer;
   iContainer = NULL;
}

TInt 
CVicinityFeedView::GetDistanceMode()
{
   return iWayFinderUI->GetDistanceMode();
}

void 
CVicinityFeedView::UpdateDataL(UpdatePositionMess* aPositionMess, 
                           TInt aSpeed)
{
   if( iContainer ){
      TInt realHeading = -1;
      if( aPositionMess->headingQuality() > QualityUseless ){
         realHeading = aPositionMess->getHeading();
      }
      TInt heading360 = 0;
      if( realHeading > 0 ){
         heading360 = TInt(realHeading * ( 360.0 / 256.0 ) + 0.5);
      }
      
//      int zoomScale = GetZoomScale( aSpeed );

//      iContainer->setGpsPos( Nav2Coordinate( aPositionMess->getLat(), 
      //                                           aPositionMess->getLon() ),
      //                     (int) heading360,
      //                     zoomScale);
   }
}

void 
CVicinityFeedView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
/*   if (aResourceId == R_WAYFINDER_VICINITY_MENU) {
      
      if (!iWayFinderUI->IsGpsConnected()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdVicinityNavigate, ETrue);
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdVicinityNavigate, EFalse);         
      }
   }
   */
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

void
CVicinityFeedView::SetInputData(int32 lat,
                                int32 lon,
                                TInt heading,
                                TInt speed,
                                TInt alt,
                                TInt aGpsState)
{
   TRACE_FUNC1("New GPS input data");

   TInt readInterval = iWayFinderUI->iSettingsData->iVicinityFeedRate;
   
   TTime now;
   now.HomeTime();
   now -= (TTimeIntervalSeconds) readInterval;

   if(!iContainer)
      return;

   if(now < iReferenceTime && iContainer->GetFeedState() ==
      CVicinityFeedContainer::FEED_VICINITY_INFORMATION)
   {
      return;
   }
      

   User::ResetInactivityTime();
   


   /* We shouldn't update the display if we don't have reliable GPS data */
   if(!iWayFinderUI->IsGpsConnected())
      return;

   Nav2Coordinate newCenter(lat, lon);

   TBool didUpdate = iContainer->UpdateGpsPos( newCenter, heading );

   if(didUpdate) {
      iReferenceTime.HomeTime();
   }
   
   iPrevLat = lat;
   iPrevLon = lon;
}

void CVicinityFeedView::ForceRefresh()
{
   iReferenceTime.HomeTime();
   iReferenceTime -= (TTimeIntervalSeconds) ( READOUT_INTERVAL + 2 );
}
