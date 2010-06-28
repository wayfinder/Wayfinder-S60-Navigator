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
#include <avkon.hrh>

#include <eikmenup.h>
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>
#include <stringloader.h>
#include <arch.h>
#include "RouteEnums.h"
#include "DistancePrintingPolicy.h"
#include "TimeOutNotify.h"
#include "GuiProt/GuiProtEnums.h"
#include "WayFinderAppUi.h"
#include "WFTextUtil.h"
#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "GuideView.h"
#include "MapView.h"
#include "SettingsData.h"

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
# include "GuideContainerPreviewController.h"
#else
# include "GuideContainer.h"
#endif

#include "memlog.h"

#include "PathFinder.h"

using namespace std;
using namespace isab;
using namespace RouteEnums;
using namespace RouteInfoParts;

// ================= MEMBER FUNCTIONS =======================

CGuideView::CGuideView(CWayFinderAppUi* aUi, isab::Log* aLog) : 
   CViewBase(aUi),
   iLog(aLog) 
{}
// ---------------------------------------------------------
// CGuideView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CGuideView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_GUIDE_VIEW );

   iCurrentPicture = ENoPicture;
   iNextTurn = ENoPicture;
   iCoeEnv->ReadResource( iCurrentStreet, R_WAYFINDER_CURRENT_STREET_TEXT );
   iCoeEnv->ReadResource( iNextStreet, R_WAYFINDER_NEXT_STREET_TEXT );
   iLastDistance = 0;
   iLeftSideTraffic = EFalse;

   iCurrentTurn = 0;
   iIsLastTurn = EFalse;
   iTurnCoordinate.iY = MAX_INT32;
   iTurnCoordinate.iX = MAX_INT32;
}

CGuideView* CGuideView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CGuideView* self = new (ELeave) CGuideView(aUi, aLog);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CGuideView* CGuideView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CGuideView *self = CGuideView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CGuideView::~CGuideView()
// ?implementation_description
// ---------------------------------------------------------
//
CGuideView::~CGuideView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
}

void
CGuideView::SetLeftSideTraffic(TBool aLeftSideTraffic)
{
   iLeftSideTraffic = aLeftSideTraffic;
}

void
CGuideView::UpdateStatus( OnTrackEnum aStatus,
                          TDesC &aCurrStreet,
                          TDesC &aNextStreet,
                          TUint &aDistance )
{
   switch( aStatus )
   {
   case RouteEnums::OffTrack:
      iCurrentPicture = EOffTrack;
      break;
   case RouteEnums::WrongWay:
      iCurrentPicture = EWrongDirection;
      break;
   case RouteEnums::Goal:
      iCurrentPicture = EFinishFlag;
      aDistance = 0;
      break;
   case RouteEnums::OnTrack:
      break; //XXX real code?
   }
   iNextTurn = ENoPicture;
   if( aCurrStreet.Length() < iCurrentStreet.MaxLength() )
      iCurrentStreet.Copy( aCurrStreet );
   else
      iCurrentStreet.Copy( aCurrStreet.Ptr(), iCurrentStreet.MaxLength() );
   if( aNextStreet.Length() < iNextStreet.MaxLength() )
      iNextStreet.Copy( aNextStreet );
   else
      iNextStreet.Copy( aNextStreet.Ptr(), iNextStreet.MaxLength() );
   if( iContainer != NULL ){
      iContainer->SetStreetsL( aCurrStreet, aNextStreet );
      TInt mbmIndex = TTurnPictures::GetTurnPicture( iCurrentPicture, iLeftSideTraffic );
      iContainer->SetGuidePictureL( mbmIndex );
      TInt mbmMaskIndex = -1;
      TTurnPictures::GetSmallTurnPicture( iNextTurn, iLeftSideTraffic , mbmIndex, mbmMaskIndex);
      iContainer->SetNextTurnPictureL( mbmIndex , mbmMaskIndex );
      iContainer->SetExitL( -1 );
      //iContainer->SetExitL( 0 );
   }
   UpdateDistance( aDistance );
}

void CGuideView::UpdateWayPoint( RouteAction aCurrAction,
                                 RouteAction aNextAction,
                                 RouteCrossing aCurrCrossing,
                                 RouteCrossing aNextCrossing,
                                 TDesC &aCurrStreet, TDesC &aNextStreet,
                                 TUint &aCurrDistance, TUint &aNextDistance,
                                 TInt &aExit, TInt32 aLat, TInt32 aLon,
                                 TBool aHighway,
                                 TInt detour, TInt speedcam)
{
   if( aCurrStreet.Length() < iCurrentStreet.MaxLength() )
      iCurrentStreet.Copy( aCurrStreet );
   else
      iCurrentStreet.Copy( aCurrStreet.Ptr(), iCurrentStreet.MaxLength() );
   if( aNextStreet.Length() < iNextStreet.MaxLength() )
      iNextStreet.Copy( aNextStreet );
   else
      iNextStreet.Copy( aNextStreet.Ptr(), iNextStreet.MaxLength() );

   iCurrentPicture = TTurnPictures::GetPicture( aCurrAction, aCurrCrossing, aCurrDistance, aHighway );
   if( aCurrAction == Finally )
      iNextTurn = ENoPicture;
   else if( iWayfinderAppUi->IsGpsAllowed() && aCurrDistance > 2500 ){
      iNextTurn = iCurrentPicture;
      if( aHighway )
         iCurrentPicture = EHighWayStraight;
      else
         iCurrentPicture = EStraight;
   }
   else
      iNextTurn = TTurnPictures::GetPicture( aNextAction, aNextCrossing, aNextDistance, aHighway );
   if( iContainer != NULL ){
      iContainer->SetStreetsL( iCurrentStreet, iNextStreet );
      TInt mbmIndex = TTurnPictures::GetTurnPicture( iCurrentPicture, iLeftSideTraffic );
      iContainer->SetGuidePictureL( mbmIndex );
      TInt mbmMaskIndex = -1;
      TTurnPictures::GetSmallTurnPicture( iNextTurn, iLeftSideTraffic , mbmIndex, mbmMaskIndex);
      iContainer->SetNextTurnPictureL( mbmIndex , mbmMaskIndex );
      iContainer->SetDetourPicture( detour );
      iContainer->SetSpeedCamPicture( speedcam );
      if( iCurrentPicture == EMultiWayRdb ){
         iContainer->SetExitL( TTurnPictures::GetExit( aExit ),
                               TTurnPictures::GetExitMask( aExit ) );
         //iContainer->SetExitL( aExit );
      } else {
         iContainer->SetExitL( -1 );
         //iContainer->SetExitL( 0 );
      }
   }
   iTurnCoordinate.iY = aLat;
   iTurnCoordinate.iX = aLon;
   UpdateDistance( aCurrDistance );
}

void CGuideView::UpdateDistance( TUint aDistance )
{
   if( iLastDistance > 2500 && aDistance <= 2500 )
      iWayfinderAppUi->SetForceUpdate( ETrue );
   iLastDistance = aDistance;
   if( iContainer !=NULL )
      iContainer->SetDistanceL( aDistance );
}

void CGuideView::UpdateCurrentTurn( TInt aTurn )
{
   iWayfinderAppUi->SetCurrentTurn( aTurn );
}

void CGuideView::SetCurrentTurn( TInt aTurn )
{
   iCurrentTurn = aTurn;
   if( iCurrentTurn == 0 )
      iIsLastTurn = EFalse;
}

TInt CGuideView::GetCurrentTurn()
{
   return iCurrentTurn;
}

void CGuideView::UpdateSpeedcam( TUint aSpeedcam ){
   if( iContainer  )
      iContainer->SetSpeedCamPicture( aSpeedcam );
}

void CGuideView::UpdateDetour( TUint aDetour ){
   if( iContainer )
      iContainer->SetDetourPicture( aDetour );
}


void CGuideView::GoToTurn( TBool aNextTurn )
{
   if( aNextTurn && !iIsLastTurn ){
      iCurrentTurn++;
      RequestRouteListL();
   }
   else if( !aNextTurn && iCurrentTurn != 0 ){
      iIsLastTurn = EFalse;
      iCurrentTurn--;
      RequestRouteListL();
   }
}

void CGuideView::RequestRouteListL()
{
   if( iCurrentTurn > 0 )
      iWayfinderAppUi->RequestRouteListL( iCurrentTurn-1, 3 );
   else
      iWayfinderAppUi->RequestRouteListL( iCurrentTurn, 2 );
}

void CGuideView::SetRouteList( const RouteList* aRouteList )
{
   if( aRouteList->crossings.size() > 0 ){
      RouteAction currAction = End;
      RouteAction nextAction = End;
      RouteCrossing currCrossing = NoCrossing;
      RouteCrossing nextCrossing = NoCrossing;
      TBuf<KBuf64Length> currStreet(_L(""));
      TBuf<KBuf64Length> nextStreet(_L(""));
      TUint currDistance = 0;
      TUint nextDistance = 0;
      TInt exit = 0;
      TInt detour = 0;
      TInt speedcam = 0;
      TBool isHighway = EFalse;
      TInt32 lat = MAX_INT32;
      TInt32 lon = MAX_INT32;

      vector<RouteListCrossing *> crossings = aRouteList->crossings;
      vector<RouteListCrossing *>::iterator crossIt;
      RouteInfoParts::RouteListCrossing* routeCrossing;
      Crossing crossing;
      vector<Segment *> segments;;
      vector<Segment *>::iterator segIt;

      //The previous wp
      crossIt = crossings.begin();
      routeCrossing = *crossIt;

      if( iCurrentTurn > 0 ){
         crossing = routeCrossing->crossing;
         currDistance = crossing.distToNextCrossing;
         exit = crossing.exitCount;
         segments = routeCrossing->segments;
         if (routeCrossing->hasDetourLandmark()) {
            detour = 1;
         }
         if (routeCrossing->hasSpeedCameraLandmark()) {
            speedcam = 1;
         }
         if(!segments.empty()){
            segIt = segments.begin();
            Segment* segment = *segIt;
            WFTextUtil::char2TDes( currStreet, segment->streetName );
            if( segment->isHighway() )
               isHighway = ETrue;
            iLeftSideTraffic = segment->isLeftTraffic();
         }
         // The current wp
         crossIt++;
      }
      if( crossIt != crossings.end() ){
         routeCrossing = *crossIt;
         crossing = routeCrossing->crossing;
         currAction = (RouteAction)crossing.action;
         currCrossing = (RouteCrossing)crossing.crossingType;
         exit = crossing.exitCount;
         segments = routeCrossing->segments;
         segIt = segments.begin();
         if( segIt != NULL ){
            Segment* segment = *segIt;
            WFTextUtil::char2TDes( nextStreet, segment->streetName );
            if( segment->isHighway() )
               isHighway = ETrue;
         }
      }
      else{
         iIsLastTurn = ETrue;
      }
      lat = routeCrossing->crossing.lat;
      lon = routeCrossing->crossing.lon;
      // The next wp
      if( !iIsLastTurn )
         crossIt++;
      if( crossIt != crossings.end() ){
         routeCrossing = *crossIt;
         crossing = routeCrossing->crossing;
         nextAction = (RouteAction)crossing.action;
         nextCrossing = (RouteCrossing)crossing.crossingType;
         nextDistance = crossing.distToNextCrossing;
      }
      else{
         iIsLastTurn = ETrue;
      }

      UpdateWayPoint( currAction, nextAction, currCrossing, nextCrossing,
                      currStreet, nextStreet, currDistance, nextDistance,
                      exit, lat, lon, isHighway, detour, speedcam );
      UpdateCurrentTurn( iCurrentTurn );

      RouteInfoParts::RouteListCrossing* tmpCrossing = *crossings.begin();
      if (tmpCrossing) {
         UpdateRouteDataL(tmpCrossing->distToGoal, tmpCrossing->timeToGoal);
      }
   }
}

void CGuideView::UpdateRouteDataL(TInt32 aDistanceToGoal, 
                                  TInt32 aEstimatedTimeToGoal)
{
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   // We only have this functionality in the new guide container 
   // for s60v3fp2 and s60v5.
   if (iContainer) {
      iContainer->UpdateRouteDataL(aDistanceToGoal, aEstimatedTimeToGoal);
   }
#endif
}

TInt CGuideView::GetDistanceMode()
{
   return iWayfinderAppUi->GetDistanceMode();
}

TInt CGuideView::GetDistanceModeWithSpace()
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

void CGuideView::PictureError( TInt aError )
{
   switch( aError )
   {
   case KErrUnderflow:
      //This error occurs if the gif file contains insufficient information
      //This is usually because the file is being opened in a cache so a futher attempt to open
      //should be performed
      iWayfinderAppUi->SetForceUpdate( ETrue );
      break;
   default:
      iWayfinderAppUi->ResourceErrorL( aError, 0 );
   }
}

void CGuideView::ShowOnMap( TBool aShowRoute )
{
   if( aShowRoute ){
      iWayfinderAppUi->RequestMap( MapEnums::OverviewRoute, MAX_INT32, MAX_INT32 );
   } else {
      iWayfinderAppUi->RequestMap( MapEnums::UnmarkedPosition, iTurnCoordinate.iY, iTurnCoordinate.iX );
   }
}

TPtrC CGuideView::GetMbmName()
{
   return iWayfinderAppUi->iPathManager->GetMbmName();
}

TBool CGuideView::IsNightMode()
{
   return iWayfinderAppUi->IsNightMode();
}

void CGuideView::GetForegroundColor(TRgb& aRgb)
{
   iWayfinderAppUi->GetForegroundColor(aRgb);
}

void CGuideView::GetNightmodeBackgroundColor(TRgb& aRgb)
{
   iWayfinderAppUi->GetNightmodeBackgroundColor(aRgb);
}

// ---------------------------------------------------------
// TUid CGuideView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CGuideView::Id() const
{
   return KGuideViewId;
}

// ---------------------------------------------------------
// CGuideView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CGuideView::HandleCommandL(TInt aCommand)
{   
   switch ( aCommand )
   {
      case EAknSoftkeyOk:
         {
            break;
         }
      case EAknSoftkeyBack:
         {
            iWayfinderAppUi->QueryQuitNavigationL();
            break;
         }
      case EWayFinderCmdGuideShowRoute:
         {
            ShowOnMap( ETrue );
            break;
         }
      case EWayFinderCmdGuideShowTurn:
         {
            ShowOnMap( EFalse );
            break;
         }
      case EWayFinderCmdGuideReroute:
      case EWayFinderCmdGuideVoice:
         {
            if (iWayfinderAppUi->IsGpsAllowed()) {
               AppUi()->HandleCommandL( EWayFinderCmdGuideReroute );
            } else {
               AppUi()->HandleCommandL( EWayFinderCmdGuideVoice );
            }
            break;
         }
      default:
         {
            AppUi()->HandleCommandL( aCommand );
            break;
         }
   }
}

// ---------------------------------------------------------
// CGuideView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CGuideView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CGuideView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CGuideView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_GUIDE_MENU ){

      // Night mode on should be dimmed if iron version
      // or if night mode is on
      aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOn, 
                               (iWayfinderAppUi->IsIronVersion() ||
                                iWayfinderAppUi->IsNightMode()));

      // Night mode off should be dimmed if iron version
      // or if night mode is off
      aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOff, 
                               (iWayfinderAppUi->IsIronVersion() ||
                                !iWayfinderAppUi->IsNightMode()));
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CGuideView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CGuideView::DoActivateL( const TVwsViewId& aPrevViewId,
                               TUid aCustomMessageId,
                               const TDesC8& aCustomMessage)
{
   iActivated = EFalse;
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   // When we're in guide view and we toggle views 
   // (with goLeftInGroup, goRightInGroup) then we need to use EMapAsNav 
   // when going into the map, otherwise the map view will be in wrong state.
   iWayfinderAppUi->setNextMessagId(EMapAsNavigation);

   HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_GUIDE_TEXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   if ( !iContainer ){
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
      iContainer = CGuideContainerPreviewController::NewL(ClientRect(), *this);
      iContainer->SetMopParent(this);
      AppUi()->AddToStackL(*this, iContainer);
#else
      iContainer = new (ELeave) CGuideContainer(iLog);
      LOGNEW(iContainer, CGuideContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL(iWayfinderAppUi, ClientRect(), this );
      AppUi()->AddToStackL( *this, iContainer );
#endif
   }
   if( iWayfinderAppUi->IsGpsAllowed() ){
      iWayfinderAppUi->SetForceUpdate( ETrue );
      iContainer->SetStreetsL( iCurrentStreet, iNextStreet );
      TInt mbmIndex = TTurnPictures::GetTurnPicture( iCurrentPicture, iLeftSideTraffic );
      iContainer->SetGuidePictureL( mbmIndex );
      TInt mbmMaskIndex = -1;
      TTurnPictures::GetSmallTurnPicture( iNextTurn, iLeftSideTraffic , mbmIndex, mbmMaskIndex);
      iContainer->SetNextTurnPictureL( mbmIndex , mbmMaskIndex );
      UpdateDistance( iLastDistance );
   } else {
      if( iWayfinderAppUi->HasRoute() ) {
         /* Can't get route list if we have no route. */
         RequestRouteListL();
      }
   }
   if( iWayfinderAppUi->IsNightMode() ){
      SetNightModeL( ETrue );
   }
   
   iActivated = ETrue;
}

// ---------------------------------------------------------
// CGuideView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CGuideView::DoDeactivate()
{
   if( iWayfinderAppUi->IsNightMode() ){
      SetNightModeL( EFalse );
   }
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
   iActivated = EFalse;
}

void CGuideView::SetNightModeL(TBool aNightMode)
{
   if (iContainer) {
      iContainer->SetNightModeL(aNightMode);
   }
}

TInt CGuideView::GetCurrentTurnPicture()
{
   return TTurnPictures::GetTurnPicture(iCurrentPicture, iLeftSideTraffic);
}

void CGuideView::GetNextTurnPicture(TInt& aMbmIndex, TInt& aMaskMbmIndex)
{
   TTurnPictures::GetSmallTurnPicture(iNextTurn, iLeftSideTraffic, aMbmIndex, aMaskMbmIndex);
}
// End of File
