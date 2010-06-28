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

#include <arch.h>
#include "RouteEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/GuiProtEnums.h"
#include "WayFinderAppUi.h"
#include "WFTextUtil.h"
#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "GuideView.h"
#include "MapView.h"
#include "ConnectDialog.h"
#include "SettingsData.h"

#include "memlog.h"
#include "TraceMacros.h"
#include "PathFinder.h"


using namespace std;
using namespace isab;
using namespace RouteEnums;
using namespace RouteInfoParts;

#define DPB iDistVector.push_back

void CGuideView::InitDistanceTableMetric()
{
   iDistVector.clear();
   DPB(DistInfo(2000, 1700,  2200, false ));
   DPB(DistInfo(1000,  800,  2050, false ));
   DPB(DistInfo( 500,  450,  1050, false ));
   DPB(DistInfo( 200,  170,  550,  false ));
   DPB(DistInfo( 100,   60,  210,  false ));
   DPB(DistInfo(  55,   35,  110,  false ));
   DPB(DistInfo(  30,   20,  60,   false ));
   DPB(DistInfo(  15,   10,  30,   false ));
   DPB(DistInfo(   0,   -1,  8,    false ));
   iCurrentUnits = DistancePrintingPolicy::ModeMetric;
}
       
void CGuideView::InitDistanceTableFeetMiles()
{
   iDistVector.clear();
   DPB(DistInfo(3219, 2919, 3600, false ));
   DPB(DistInfo(1609, 1409, 3269, false ));
   DPB(DistInfo( 803,  753, 1659, false ));
   DPB(DistInfo( 402,  352, 853,  false ));
   DPB(DistInfo( 152,  122, 452,  false ));
   DPB(DistInfo(  66,   46, 162,  false ));
   DPB(DistInfo(  36,   26, 71,   false ));
   DPB(DistInfo(  18,   15, 36,   false ));
   DPB(DistInfo(   0,   -1, 12,   false ));
   iCurrentUnits = DistancePrintingPolicy::ModeImperialFeet;
}

void CGuideView::InitDistanceTableYardsMiles()
{
   iDistVector.clear();
   DPB(DistInfo(3219, 2919, 3600, false ));
   DPB(DistInfo(1609, 1409, 3269, false ));
   DPB(DistInfo( 803,  753, 1659, false ));
   DPB(DistInfo( 402,  352, 853,  false ));
   DPB(DistInfo( 182,  152, 452,  false ));
   DPB(DistInfo(  91,   51, 192,  false ));
   DPB(DistInfo(  51,   31, 101,  false ));
   DPB(DistInfo(  28,   18, 56,   false ));
   DPB(DistInfo(  10,    8, 17,   false ));
   DPB(DistInfo(   0,   -1, 7,    false ));
   iCurrentUnits = DistancePrintingPolicy::ModeImperialYards;
}

// ================= MEMBER FUNCTIONS =======================
CGuideView::CGuideView(isab::Log* aLog) : iContainer(NULL), iLog(aLog)  {}
// ---------------------------------------------------------
// CGuideView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CGuideView::ConstructL( CWayFinderAppUi* aWayFinderUI )
{
   BaseConstructL( R_WAYFINDER_GUIDE_VIEW );

   iCurrentPicture = ENoPicture;
   iNextTurn = ENoPicture;
   iCoeEnv->ReadResource( iCurrentStreet, R_WAYFINDER_CURRENT_STREET_TEXT );
   iCoeEnv->ReadResource( iNextStreet, R_WAYFINDER_NEXT_STREET_TEXT );
   iLastDistance = 0;
   iLeftSideTraffic = EFalse;

   iCurrentInstruction = NULL;
   
   iWayFinderUI = aWayFinderUI;
   iCurrentTurn = 0;
   iIsLastTurn = EFalse;
   iTurnCoordinate.iY = MAX_INT32;
   iTurnCoordinate.iX = MAX_INT32;
   
//    iReferenceTime = TTime(0);
//    iChangeTurn = ETrue;
}

CGuideView* CGuideView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CGuideView* self = new (ELeave) CGuideView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aUi);
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

   delete iCurrentInstruction;
   
   LOGDEL(iContainer);
   delete iContainer;

   iDistVector.clear();
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
   TBool offTrack = false;
   switch( aStatus )
   {
   case RouteEnums::OffTrack:
      iCurrentPicture = EOffTrack;
      offTrack = TRUE;
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

   if(offTrack) {
      AlertOffTrack();
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
   UpdateDistance( aDistance, offTrack );
}


void CGuideView::AlertOffTrack()
{
   if(!iContainer)
      return;
   
   HBufC* msg = iCoeEnv->AllocReadResourceLC(R_WF_OFF_TRACK);
    
   iContainer->SetTextInstructionL(*msg);

   CleanupStack::PopAndDestroy(msg);
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
   else if( iWayFinderUI->IsGpsAllowed() && aCurrDistance > 2500 ){
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
         iContainer->SetExitL( TTurnPictures::GetExit( aExit ) );
         //iContainer->SetExitL( aExit );
      } else {
         iContainer->SetExitL( -1 );
         //iContainer->SetExitL( 0 );
      }
   }
   iTurnCoordinate.iY = aLat;
   iTurnCoordinate.iX = aLon;
   
   ResetSupressedNavInst();

   if (iWayFinderUI->GetTransportationMode() == isab::NavServerComEnums::pedestrian) {
   
      GetRouteDescriptionL(aCurrAction,
                           aCurrDistance,
                           aExit,
                           iWayFinderUI->GetDistanceMode(),
                           iWayFinderUI->GetTransportationMode() == 
                           isab::NavServerComEnums::pedestrian);
   } 
//    iChangeTurn = ETrue;
   UpdateDistance( aCurrDistance, FALSE );
}

void CGuideView::UpdateDistance( TUint aDistance,
                                 TBool aSkipTTS )
{
   if( iLastDistance > 2500 && aDistance <= 2500 )
      iWayFinderUI->SetForceUpdate( ETrue );

   iLastDistance = aDistance;
   if( iContainer !=NULL ) {
      iContainer->SetDistanceL( aDistance );
      
      if (iWayFinderUI->GetTransportationMode()
            == isab::NavServerComEnums::pedestrian
          && !aSkipTTS)
      {
         TInt distInstr = ShouldUpdateNavInstruction(aDistance);
         if (distInstr != -1) {
            // re-init textinstruction
            // construct text instruction
            
            delete iCurrentInstruction;
            iCurrentInstruction = HBufC::NewL(512);
            
            //          CombinePhrasesWithDistance(aDistance, instruction);
            DistInfo distInfo;
            GetCurrentDistInfo(aDistance, distInfo);
            if (distInfo.sayAtDistance == 0) {
               // Time to turn here
               GetTurnInstruction(aDistance, iCurrentInstruction);
            } else {
               //GetRouteActionString(iAction, iActionPhrase);
               CombinePhrasesWithDistance((TUint&)distInstr, iCurrentInstruction);
            }
            // set text instruction
            iContainer->SetTextInstructionL(*iCurrentInstruction);         
//            CleanupStack::PopAndDestroy(instruction);
         } 
      } 
   }
}

void CGuideView::UpdateCurrentTurn( TInt aTurn )
{
   // FIXME: The change is not reported to the screen reader. This could be 
	//        done by an update message on the screen.
	//        I thINK It would be better to update the text field with the string 
	//        message.
   iWayFinderUI->SetCurrentTurn( aTurn );
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
      iWayFinderUI->RequestRouteListL( iCurrentTurn-1, 3 );
   else
      iWayFinderUI->RequestRouteListL( iCurrentTurn, 2 );
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
   }
}

TInt
CGuideView::GetDistanceMode()
{
   return iWayFinderUI->GetDistanceMode();
}

void CGuideView::PictureError( TInt aError )
{
   switch( aError )
   {
   case KErrUnderflow:
      //This error occurs if the gif file contains insufficient information
      //This is usually because the file is being opened in a cache so a futher attempt to open
      //should be performed
      iWayFinderUI->SetForceUpdate( ETrue );
      break;
   default:
      iWayFinderUI->ResourceErrorL( aError, 0 );
   }
}

void CGuideView::ShowOnMap( TBool aShowRoute )
{
   if( aShowRoute ){
      iWayFinderUI->RequestMap( MapEnums::OverviewRoute, MAX_INT32, MAX_INT32 );
   } else {
      iWayFinderUI->RequestMap( MapEnums::UnmarkedPosition, iTurnCoordinate.iY, iTurnCoordinate.iX );
   }
}

TPtrC CGuideView::GetMbmName()
{
   return iWayFinderUI->iPathManager->GetMbmName();
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
            if (iWayFinderUI->IsGpsAllowed()) {
               AppUi()->HandleCommandL( EWayFinderCmdGuideReroute );
            } else {
               AppUi()->HandleCommandL( EWayFinderCmdGuideVoice );
            }
            break;
         }
      case EWayFinderCmdStartPageUpgradeInfo:
      case EWayFinderCmdStartPageUpgradeInfo2:
         {
            iWayFinderUI->ShowUpgradeInfoL(CWayFinderAppUi::EUpgrade, Id());
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
      if (iWayFinderUI->IsIronVersion()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdRouteSubMenu, ETrue );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdRouteSubMenu, EFalse );
      }
      if (iWayFinderUI->HasRoute()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdGuideShowRoute, EFalse );
         aMenuPane->SetItemDimmed( EWayFinderCmdGuideShowTurn, EFalse );
         if(iWayFinderUI->IsGpsAllowed()) {
            aMenuPane->SetItemDimmed( EWayFinderCmdGuideReroute, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdGuideVoice, ETrue );
         } else {
            aMenuPane->SetItemDimmed( EWayFinderCmdGuideReroute, ETrue );
            if (iWayFinderUI->HaveVoiceInstructions()) {
               aMenuPane->SetItemDimmed( EWayFinderCmdGuideVoice, EFalse );
            } else {
               aMenuPane->SetItemDimmed( EWayFinderCmdGuideVoice, ETrue );
            }
         }
#if defined NAV2_CLIENT_SERIES60_V3
         aMenuPane->SetItemDimmed( EWayfinderCmdGuideSendTo, ETrue );
#else
         aMenuPane->SetItemDimmed( EWayfinderCmdGuideSendTo, EFalse );
#endif
      } else{
         /* No route, hence can't reroute or play voice instruction. */
         aMenuPane->SetItemDimmed( EWayFinderCmdGuideShowRoute, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdGuideShowTurn,  ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdGuideReroute,   ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdGuideVoice,     ETrue );
         aMenuPane->SetItemDimmed( EWayfinderCmdGuideSendTo,    ETrue );
      }
/*    } else if (aResourceId==R_WAYFINDER_VIEWS_MENU) { */
/*       aMenuPane->SetItemDimmed( EWayFinderCmdGuide, ETrue); */
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}


void CGuideView::CreateContainer()
{
   TRACE_FUNC1("Creating container");
   
   if(iContainer)
      return;
   
   iContainer = new (ELeave) CGuideContainer(iLog);
   LOGNEW(iContainer, CGuideContainer);
   iContainer->SetMopParent(this);
   iContainer->ConstructL(iWayFinderUI, ClientRect(), this );
}


// ---------------------------------------------------------
// CGuideView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CGuideView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
                               TUid /*aCustomMessageId*/,
                               const TDesC8& /*aCustomMessage*/)
{  
   CreateContainer();

   AppUi()->AddToStackL( *this, iContainer );
   
   if( iWayFinderUI->IsGpsAllowed() ){
      iWayFinderUI->SetForceUpdate( ETrue );
      iContainer->SetStreetsL( iCurrentStreet, iNextStreet );
      TInt mbmIndex = TTurnPictures::GetTurnPicture( iCurrentPicture, iLeftSideTraffic );
      iContainer->SetGuidePictureL( mbmIndex );
      TInt mbmMaskIndex = -1;
      TTurnPictures::GetSmallTurnPicture( iNextTurn, iLeftSideTraffic , mbmIndex, mbmMaskIndex);
      iContainer->SetNextTurnPictureL( mbmIndex , mbmMaskIndex );
      UpdateDistance( iLastDistance, FALSE );
   }

   SharedActivation();
}

void CGuideView::SharedActivation() {
   iWayFinderUI->GetNavigationDecorator()->MakeScrollButtonVisible( ETrue );
      
   if( iWayFinderUI->HasRoute() ) {
      /* Can't get route list if we have no route. */
      RequestRouteListL();
   }
}

// ---------------------------------------------------------
// CGuideView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CGuideView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   
   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

// TextInstruction building

const void CGuideView::GetRouteActionInitialString(RouteAction &aAction, 
                                                   TBool &aPedestrianMode,
                                                   TBuf<128> &buffer) {
   switch(aAction) {
   case StartAt:
   case StartWithUTurn:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_DRIVE_START);
      break;
   case Finally:
      if(aPedestrianMode) {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_FINALLY_WALK);
      } else {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_FINALLY);
      }
      break;
   case ExitFerry:
   case ChangeFerry:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_RIDE);
      break;
   default:
      if(aPedestrianMode) {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_WALK);
      } else {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_DRIVE);
      }
      break;
   }
}

const void CGuideView::GetRouteActionString(RouteAction &aAction,
                                            TUint aDistance,
                                            TBuf<128> &buffer) 
{
   DistInfo distInfo;
   switch(aAction) {
   case Ahead:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_AHEAD);
      break;
   case Left:
      GetCurrentDistInfo(aDistance, distInfo);
      if (distInfo.sayAtDistance == 0) {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TURN_LEFT_HERE);
      } else {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_LEFT);
      }
      break;
   case Right:
      GetCurrentDistInfo(aDistance, distInfo);
      if (distInfo.sayAtDistance == 0) {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TURN_RIGHT_HERE);
      } else {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_RIGHT);
      }
      break;
   case UTurn:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_U);
      break;
   case StartAt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_DRIVE_START);
      break;
   case Finally:
      GetCurrentDistInfo(aDistance, distInfo);
      if (distInfo.sayAtDistance == 0) {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_AT_THE_DESTINATION);
      } else {
         iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_EMPTY);
      }
      break;
   case EnterRdbt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_ENTER_RDBT);
      break;
   case ExitRdbt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_EXIT_RDBT);
      break;
   case AheadRdbt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_AHEAD_RDBT);
      break;
   case LeftRdbt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_LEFT_RDBT);
      break;
   case RightRdbt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_RIGHT_RDBT);
      break;
   case ExitAt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_OFF_RAMP);
      break;
   case On:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_ON_RAMP);
      break;
   case ParkCar:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_PARK_CAR);
      break;
   case KeepLeft:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_KEEP_LEFT);
      break;
   case KeepRight:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_KEEP_RIGHT);
      break;
   case StartWithUTurn:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_DRIVE_START_U);
      break;
   case UTurnRdbt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_U_RDBT);
      break;
   case FollowRoad:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_FOLLOW);
      break;
   case EnterFerry:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_ENTER_FERRY);
      break;
   case ExitFerry:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_EXIT_FERRY);
      break;
   case ChangeFerry:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_CHANGE_FERRY);
      break;
   case EndOfRoadLeft:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_ROAD_END_LEFT);
      break;
   case EndOfRoadRight:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_ROAD_END_RIGHT);
      break;
   case OffRampLeft:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_OFF_RAMP_LEFT);
      break;
   case OffRampRight:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_OFF_RAMP_RIGHT);
      break;
   case Start:
   case End:
   case InvalidAction:
   case Delta:
   case RouteActionMax:
   default:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_EMPTY);
      break;
   }
}

const void CGuideView::GetRouteActionPrePositionString(RouteAction &aAction,
                                                       TBuf<128> &buffer) 
{
   switch(aAction) {
   case ExitAt:
   case OffRampLeft:
   case OffRampRight:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_OFF_RAMP_PP);
      break;
   case ParkCar:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_PARK_CAR_PP);
      break;
   case StartAt:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_DIR_AT);
      break;
   case EnterFerry:
   case ChangeFerry:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_EMPTY);
      break;
   default:
      iCoeEnv->ReadResource(buffer, R_WF_GUIDE_TEXT_TURN_PP);
      break;
   }
}

void CGuideView::GetTurnInstruction(TUint aDistance, HBufC* aInstruction)
{
   GetRouteActionString(iAction, aDistance, iActionPhrase);
   TPtr instruction = aInstruction->Des();
   if(instruction.Length() < iActionPhrase.Length()) {
      // Not enough space, extend
      aInstruction = aInstruction->ReAllocL(iActionPhrase.Length());
   }
   instruction.Copy(iActionPhrase);
//    iCoeEnv->ReadResource(iInstructionPhrase, R_WF_GUIDE_TEXT_EMPTY);
//    iCoeEnv->ReadResource(iPreposition, R_WF_GUIDE_TEXT_EMPTY);
}

TBool CGuideView::CombinePhrasesWithDistance(TUint &aDistance,
                                             HBufC * aInstruction) 
{
   TPtr instruction = aInstruction->Des();
   instruction.Copy(_L(" "));
   TInt length = 1;
   TInt doubledMaxLength = 1;
   TBuf<KBuf32Length> distanceDes;
   isab::DistancePrintingPolicy::DistanceMode mode =
      isab::DistancePrintingPolicy::DistanceMode(iDistanceMode);
	
   char * distanceStr =
      isab::DistancePrintingPolicy::
      convertDistance(aDistance,
                      mode,
                      isab::DistancePrintingPolicy::Round);
   
   _LIT(WhiteSpace, " ");
   _LIT(KBreak, "\n");
   
   // calculation of the needed length
   length += iInstructionPhrase.Length();
   length += WhiteSpace().Length();
   WFTextUtil::char2TDes(distanceDes, distanceStr);
   delete[] distanceStr;
   length += distanceDes.Length();
   if(iAction != Finally) {
      if(iAction != StartAt && iAction != StartWithUTurn) {
         length += WhiteSpace().Length();
         length += iActionPhrase.Length();
      }
      length += WhiteSpace().Length();
      length += iPreposition.Length();
      length += WhiteSpace().Length();
      length += KBreak().Length();
      length += iNextStreet.Length();
   }

   // size check
   if(length > instruction.MaxLength()) {
      doubledMaxLength = instruction.MaxLength() * 2;
      aInstruction = aInstruction->ReAllocL(doubledMaxLength);
   }

   // appending
   instruction.Append(iInstructionPhrase);
   instruction.Append(WhiteSpace);
   instruction.Append(distanceDes);

   if(iAction != Finally) {
      if(iAction != StartAt &&
         iAction != StartWithUTurn) {
         instruction.Append(WhiteSpace);
         instruction.Append(iActionPhrase);
      }
      instruction.Append(WhiteSpace);
      instruction.Append(iPreposition);
      instruction.Append(WhiteSpace);
      instruction.Append(KBreak);
      instruction.Append(iNextStreet);
   }

   return ETrue;
}

TBool CGuideView::GetRouteDescriptionL(RouteAction &aAction,
                                       TUint &aDistance,
                                       TInt aExit,
                                       TInt aDistanceMode,
                                       TBool aPedestrianMode) 
{
   // store action type
   iAction = aAction;
   // constructing instruction phrase
   // taken from MC2 StringTable::getRouteDescription()
   GetRouteActionInitialString(iAction, aPedestrianMode, iInstructionPhrase);
   // store distance mode
   iDistanceMode = aDistanceMode;
   // constructing action phrase
   // taken from MC2 StringTable::getRouteDescription()
   GetRouteActionString(iAction, aDistance, iActionPhrase);
   // constructing preposition
   // taken from MC2 StringTable::getRouteDescription()
   GetRouteActionPrePositionString(iAction, iPreposition);

   return ETrue;
	
}

TInt CGuideView::ShouldUpdateNavInstruction(TInt aDistance)
{
   InitDistVector();
   
   typedef vector<DistInfo>::iterator it_t;
   it_t end = iDistVector.end();
   for (it_t it = iDistVector.begin(); it != end; ++it ) {
      if (aDistance > it->abortTooShortDistance && 
          aDistance < it->abortTooFarDistance)
         {
            if (it->supress) {
               // Instruction has been said, return -1.
               return -1;
            }
            // The distance is within this instructions max and min value and
            // this distance has not been said for this waypoint.
            it->supress = true;
            return it->sayAtDistance;
         }
   }  
   return -1;
}

bool CGuideView::GetCurrentDistInfo(TInt aDistance, DistInfo& aDistInfo)
{
   InitDistVector();
   
   typedef vector<DistInfo>::iterator it_t;
   it_t end = iDistVector.end();
   for (it_t it = iDistVector.begin(); it != end; ++it ) {
      if (aDistance > it->abortTooShortDistance && 
          aDistance < it->abortTooFarDistance)
         {
            // The distance is within this instructions max and min value 
            aDistInfo = *it;
            return true;
         }
   }  
   return false;   
}

void CGuideView::InitDistVector()
{
   isab::DistancePrintingPolicy::DistanceMode units =
      isab::DistancePrintingPolicy::DistanceMode(GetDistanceMode());

   if (iCurrentUnits == units) 
      return;

   switch (units) { 
   case DistancePrintingPolicy::ModeMetric:
      InitDistanceTableMetric();
      break;
   case DistancePrintingPolicy::ModeImperialFeet:
      InitDistanceTableFeetMiles();
      break;
   case DistancePrintingPolicy::ModeImperialYards:
         InitDistanceTableYardsMiles();
      break;
   default:
      // FIXME - warn here?
      break;
   }
}

void CGuideView::ResetSupressedNavInst()
{
   typedef vector<DistInfo>::iterator it_t;
   it_t end = iDistVector.end();
   for (it_t it = iDistVector.begin(); it != end; ++it ) {  
      it->supress = false;
  }
}

HBufC* CGuideView::GetCurrentTextInstruction()
{
   return iCurrentInstruction;
}

TInt CGuideView::GetCurrentDistance()
{
   return iLastDistance;
}

// End of File
