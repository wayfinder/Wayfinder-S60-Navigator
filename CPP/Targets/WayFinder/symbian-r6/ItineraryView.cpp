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
#include  <avkon.hrh>

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>
#include <eikmenup.h>
#include <stringloader.h>
#include <arch.h>
#include <vector>
#include "WayFinderConstants.h"
#include "TimeOutNotify.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "WayFinderAppUi.h"
#include "WFTextUtil.h"

#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "GuideView.h"
#include "ItineraryView.h"
#include "ItineraryContainer.h" 

using namespace std;
using namespace isab;
using namespace RouteEnums;
using namespace RouteInfoParts;

// ================= MEMBER FUNCTIONS =======================

CItineraryView::CItineraryView(CWayFinderAppUi* aUi, isab::Log* aLog) : 
   CViewBase(aUi),
   iLog(aLog) 
{}
// ---------------------------------------------------------
// CItineraryView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CItineraryView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_ITINERARY_VIEW );
   iExternalTurn = 0;
   iCurrentTurn = 0;
   iRouteArray = NULL;
   iTurnPointArray = NULL;
   iTranslationArray = NULL;
}

CItineraryView* CItineraryView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CItineraryView* self = new (ELeave) CItineraryView(aUi, aLog);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CItineraryView* CItineraryView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CItineraryView *self = CItineraryView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CItineraryView::~CItineraryView()
// ?implementation_description
// ---------------------------------------------------------
//
CItineraryView::~CItineraryView()
{
   if( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;

   if( iTurnPointArray )
      delete [] iTurnPointArray;
}

GuiDataStore*
CItineraryView::GetGuiDataStore()
{
   return iWayfinderAppUi->GetGuiDataStore();
}

void CItineraryView::ClearRoute()
{
   delete iTranslationArray;
   iTranslationArray = NULL;
}

void CItineraryView::SetRouteList( const RouteList* aRouteList )
{
   if(iContainer && aRouteList->crossings.size() > 0 ){
      /* Clear old list. */
      iContainer->RemoveAllItemsL();
      vector<RouteListCrossing *> crossings = aRouteList->crossings;
      vector<RouteListCrossing *>::iterator crossIt;
      RouteInfoParts::RouteListCrossing* routeCrossing;
      RouteInfoParts::RouteListCrossing* prevCrossing;

      crossIt = crossings.begin();
      routeCrossing = *crossIt;
      TUint totalDistance = routeCrossing->distToGoal;
      crossIt = crossings.end();
      TInt crossNum = crossings.size();
      if( iTurnPointArray ) {
         delete[] iTurnPointArray;
      }
      iTurnPointArray = new TPoint[ crossNum ];

      /* Reset and initiate translation array. */
      if (iTranslationArray) {
         delete[] iTranslationArray;
      }
      /* The array can at most get to the double size. */
      iTranslationArray = new TInt[ crossNum * 2 +1 ];
      TInt* tempArray = new TInt[ crossNum * 2 +1];
      TInt i;
      for (i = 0; i < crossNum * 2; i++) {
         iTranslationArray[i] = -1;
         tempArray[i] = -1;
      }
      iTranslationArray[crossNum] = 0;
      tempArray[crossNum] = 0;
      i = 0;

      while( crossIt != crossings.begin() ){
         crossIt--;
         crossNum--;
         routeCrossing = *crossIt;
         Crossing crossing = routeCrossing->crossing;
         Crossing fromCrossing;
         TUint distance = 0;
         TUint accDistance = totalDistance - routeCrossing->distToGoal;
         if( crossIt != crossings.begin() ){
            prevCrossing = *(crossIt-1);
            fromCrossing = prevCrossing->crossing;
            distance = fromCrossing.distToNextCrossing;
         }
         iTurnPointArray[ crossNum ].iY = crossing.lat;
         iTurnPointArray[ crossNum ].iX = crossing.lon;

         vector<Segment *> segments = routeCrossing->segments;
         vector<Segment *>::iterator segIt;
         segIt = segments.begin();
         TBuf<KBuf256Length> streetName(_L(""));
         TBool isHighway = EFalse;
         TBool isLeftSideTraffic = EFalse;
         if( segIt != NULL ){
            Segment* segment = *segIt;
            WFTextUtil::char2TDes( streetName, segment->streetName );
            isHighway = segment->isHighway();
            isLeftSideTraffic = segment->isLeftTraffic();
         }
         TPictures turn = TTurnPictures::GetPicture(
               (RouteEnums::RouteAction)crossing.action,
               (RouteEnums::RouteCrossing)crossing.crossingType,
                                                  distance, isHighway );
         if( turn == EFinishArrow || turn == EFinishFlag ){
             streetName.Copy( iWayfinderAppUi->GetCurrentRouteDestinationName() );
         }
         const char * detourInfo;
         HBufC* detourName = HBufC::NewLC(256);
         if ((detourInfo=routeCrossing->hasDetourLandmark(Landmark::Begins))) {
            /* Start of detour. */
            WFTextUtil::char2HBufC( detourName, detourInfo );
            iContainer->AddItemL( EDetour, ETrue, 0, 0, 0, *detourName );
            tempArray[i++] = crossNum;
         } else if ((detourInfo=routeCrossing->hasDetourLandmark(Landmark::Ends))) {
            /* End of detour. */
            WFTextUtil::char2HBufC( detourName, detourInfo );
            iContainer->AddItemL( EDetour, EFalse, 0, 0, 0, *detourName );
            tempArray[i++] = crossNum;
         } else if ((detourInfo=routeCrossing->hasSpeedCameraLandmark(Landmark::Begins))) {
            /* Start of speed cam. */
            HBufC* tmp = HBufC::NewLC(strlen(detourInfo)+1);
            WFTextUtil::char2HBufC( tmp, detourInfo );
/*             tmp = CCoeEnv::Static()->AllocReadResourceLC(R_WAYFINDER_ITINERARY_SPEEDCAM_START2); */
            iContainer->AddItemL( ESpeedCam, ETrue, 0, 0, 0, *tmp );
            tempArray[i++] = crossNum;
            CleanupStack::PopAndDestroy(tmp);
         } else if ((detourInfo=routeCrossing->hasSpeedCameraLandmark(Landmark::Ends))) {
            /* End of speed cam. */
            HBufC* tmp = HBufC::NewLC(strlen(detourInfo)+1);
            WFTextUtil::char2HBufC( tmp, detourInfo );
/*             tmp = CCoeEnv::Static()->AllocReadResourceLC(R_WAYFINDER_ITINERARY_SPEEDCAM_END2); */
            iContainer->AddItemL( ESpeedCam, EFalse, 0, 0, 0, *tmp );
            tempArray[i++] = crossNum;
            CleanupStack::PopAndDestroy(tmp);
         }
         CleanupStack::PopAndDestroy(detourName);
         iContainer->AddItemL( turn, !isLeftSideTraffic, distance, accDistance,
                               crossing.exitCount, streetName );
         tempArray[i++] = crossNum;
      }
      /* Reverse array. */
      TInt j = 0;
      /* Step one backwards to point to first _used_ entry. */
      i--;
      while (i >= 0) {
         iTranslationArray[j++] = tempArray[i--];
      }
      delete[] tempArray;
   }
   // We need to update and calculate the current position before
   // we set the selection. If we dont do this and if it is the first
   // time we entered itineraryview our position will be displayed at
   // the start of the route no matter where we are.
   SetCurrentTurn( iExternalTurn );
   SetSelection( iCurrentTurn, iCurrentTurn );
}

TBool CItineraryView::IsGpsAllowed()
{
   return iWayfinderAppUi->IsGpsAllowed();
}

TInt
CItineraryView::GetDistanceMode()
{
   return iWayfinderAppUi->GetDistanceMode();
}

void CItineraryView::UpdateCurrentTurn( TBool aNextTurn )
{
   TInt previousTurn = iCurrentTurn;
   if( aNextTurn ) {
      iCurrentTurn++;
   } else {
      iCurrentTurn--;
   }
   if (iCurrentTurn > 0 &&
         GetCurrentTurn(iCurrentTurn) == GetCurrentTurn(iCurrentTurn-1)) {
      /* Same "real crossing". */
      /* Don't send events to the other views, just move listbox. */
      SetSelection( iCurrentTurn, previousTurn );
   } else if( !iWayfinderAppUi->IsGpsAllowed() ){
      /* This will result in a call back to SetCurrentTurn() */
      iWayfinderAppUi->SetCurrentTurn( GetCurrentTurn(iCurrentTurn ));
      SetSelection( iCurrentTurn, previousTurn );
   }
}

void CItineraryView::SetCurrentTurn( TInt aTurn )
{
   iExternalTurn = aTurn;
   if( aTurn < 0 ) {
      iCurrentTurn = 0;
   } else {
      /* The aTurn value is the index of the crossing, we need */
      /* to find the correct entry in the padded list, so search for it. */
      TInt i = 0;

      if ( iTranslationArray ) {
         while (iTranslationArray[i] != aTurn) {
            i++;
         }
      }
      iCurrentTurn = i;
   }
}

TInt
CItineraryView::GetCurrentTurn(TInt currentIndex)
{
   if (iTranslationArray == NULL || iTranslationArray[currentIndex] == -1) {
      /* Mapping problem! */
      return 0;
   }
   return (iTranslationArray[currentIndex]);
}


void CItineraryView::SetSelection( TInt aCurrentTurn, TInt aPreviousTurn )
{
   if( iContainer ) {
      iContainer->SetSelection( aCurrentTurn, aPreviousTurn );
   }
}

// ----------------------------------------------------------------------------
// void CItineraryView::AddItemL()
// Adds list item to the list.
// ----------------------------------------------------------------------------
//
/*void CItineraryView::AddItemL( TPictures aTurn, TBool aRightTraffic,
                               TInt aDistance, TInt aExit, TDesC &aText )
{
   TInt iconPos = 1;
   TBool multiwayRdbt = EFalse;
   switch( aTurn )
   {
   case EFerry:
      iconPos = 0; // Ferry
      break;
   case EFinishArrow:
   case EFinishFlag:
      iconPos = 1; // Flag
      break;
   case EEnterHighWay:
   case EEnterMainRoad:
   case EKeepLeft:
      iconPos = 2; // KeepLeft
      break;
   case EExitHighWay:
   case EExitMainRoad:
   case EKeepRight:
      iconPos = 3; //KeepRight
      break;
   case E4WayLeft:
   case ELeftArrow:
   case ERdbLeft:
   case E3WayTeeLeft:
      iconPos = 4; // LeftArrow
      break;
   case EMultiWayRdb:
      if (aRightTraffic) {
         iconPos = 5; // MultiWayRdb
      } 
      else {
         iconPos = 6; // MultiWayRdbLeft
      }
      multiwayRdbt = ETrue;
      break;
   case EPark:
      iconPos = 7; // Park
      break;
   case E4WayRight:
   case ERdbRight:
   case ERightArrow:
   case E3WayTeeRight:
      iconPos = 8; // RightArrow
      break;
   case EStart:
      iconPos = 9; // StraightArrow
      break;
   case E4WayStraight:
   case EHighWayStraight:
   case EStraight:
   case EStraightArrow:
   case ERdbStraight:
      iconPos = 10; // StraightArrow
      break;
   case ERdbUTurn:
   case EUTurn:
      if (aRightTraffic) {
         iconPos = 11; // UTurn
      } else {
         iconPos = 12; // UTurnLeft
      }
      break;
   case EOffTrack:
   case EWrongDirection:
   case ENoPicture:
      break;
   }
   TBuf<KBuf32Length> distance;
   CItineraryContainer::GetDistance( aDistance, distance );
   if( multiwayRdbt ){
      distance.AppendNum( aExit );
      TBuf<KBuf32Length> exit;
      iCoeEnv->ReadResource( exit, R_WAYFINDER_ITINERARY_EXIT );
      distance.Append( exit );
   }
   TBuf<KBuf256Length> item( _L("") );
   item.Num( iconPos );
   item.Append( KTab );
   item.Append( distance );
   item.Append( KTab );
   if( aText.Length() < item.MaxLength()-item.Length() )
      item.Append( aText );
   else
      item.Append( aText.Ptr(), item.MaxLength()-item.Length() );
   iRouteArray->AppendL( item );
}*/

// ---------------------------------------------------------
// TUid CItineraryView::Id()
// ---------------------------------------------------------
TUid CItineraryView::Id() const
{
   return KItineraryViewId;
}

// ---------------------------------------------------------
// CItineraryView::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
void CItineraryView::HandleCommandL(TInt aCommand)
{
   switch ( aCommand )
   {
   case EAknSoftkeyExit:
      {
         AppUi()->HandleCommandL(EAknSoftkeyExit);
         break;
      }
   case EWayFinderCmdItineraryShowRoute:
      {
         iWayfinderAppUi->RequestMap(MapEnums::OverviewRoute,
                                     MAX_INT32, MAX_INT32, 
                                     EShowMapFromItineraryView);
         break;
      }
   case EWayFinderCmdItineraryShowTurn:
      {
         TPoint pos;
         pos.iY = MAX_INT32;
         pos.iX = MAX_INT32;
         if( iTurnPointArray != NULL ){
            TInt turn = iContainer->GetCurrentTurn();
            if (turn > -1) {
               iCurrentTurn = turn;
            }
            pos= iTurnPointArray[ GetCurrentTurn(iCurrentTurn) ];
         }
         if( pos.iY != MAX_INT32 && pos.iX != MAX_INT32 ){
            iWayfinderAppUi->RequestMap(MapEnums::UnmarkedPosition,
                  pos.iY, pos.iX, EShowMapFromItineraryView);
         } else {
            pos = iWayfinderAppUi->GetOrigin();
            iWayfinderAppUi->RequestMap(MapEnums::UnmarkedPosition,
                  pos.iY, pos.iX, EShowMapFromItineraryView);
         }
         break;
      }
   case EWayFinderCmdItineraryReroute:
   case EWayFinderCmdItineraryVoice:
      {
         if (iWayfinderAppUi->IsGpsAllowed()) {
            AppUi()->HandleCommandL( EWayFinderCmdItineraryReroute );
         } else {
            AppUi()->HandleCommandL( EWayFinderCmdItineraryVoice );
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
// CItineraryView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CItineraryView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CItineraryView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CItineraryView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_ITINERARY_MENU ){
      if (iWayfinderAppUi->HasRoute() ){
         aMenuPane->SetItemDimmed( EWayFinderCmdItineraryShowRoute, EFalse );
         aMenuPane->SetItemDimmed( EWayFinderCmdItineraryShowTurn, EFalse );
         if( iWayfinderAppUi->IsGpsAllowed()) {
            aMenuPane->SetItemDimmed( EWayFinderCmdItineraryReroute, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdItineraryVoice, ETrue );
         } else {
            aMenuPane->SetItemDimmed( EWayFinderCmdItineraryReroute, ETrue );
            if (iWayfinderAppUi->HaveVoiceInstructions()) {
               aMenuPane->SetItemDimmed( EWayFinderCmdItineraryVoice, EFalse );
            } else {
               aMenuPane->SetItemDimmed( EWayFinderCmdItineraryVoice, ETrue );
            }
         }
} else {
         /* No route, hence can't reroute or play voice instruction. */
         aMenuPane->SetItemDimmed( EWayFinderCmdItineraryShowRoute, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdItineraryShowTurn, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdItineraryReroute, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdItineraryVoice, ETrue );
      }
/*    } else if (aResourceId==R_WAYFINDER_VIEWS_MENU) { */
/*       aMenuPane->SetItemDimmed( EWayFinderCmdItinerary, ETrue); */
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CItineraryView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CItineraryView::DoActivateL( const TVwsViewId& aPrevViewId,
                                  TUid aCustomMessageId,
                                  const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   // Explicitly hide the arrows in navipane since on s60v5 they tend to 
   // show up in some views otherwise.
   iWayfinderAppUi->setNaviPane(EFalse);

   HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_ITINERARY_TEXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   if (!iContainer){
      iContainer = new (ELeave) CItineraryContainer(iLog);
      LOGNEW(iContainer, CItineraryContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect(), this );
      AppUi()->AddToStackL( *this, iContainer );
      iContainer->DrawNow();
   }
   if( iWayfinderAppUi->HasRoute() ) {
      /* Can't get route list if we have no route. */
      iContainer->SetListArray( ETrue );
      iWayfinderAppUi->RequestRouteListL();
   }
   else{
      iContainer->SetListArray( EFalse );
   }
}

void CItineraryView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

// End of File

