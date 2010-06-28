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
#include "WayFinderConstants.h"
#include "TimeOutNotify.h"
#include "WayFinderAppUi.h"

#include "DataHolder.h"

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "RouteView.h"
#include "MapView.h"
#include "RouteContainer.h"
#include "memlog.h"

#include "PathFinder.h"

using namespace std;
using namespace isab;
using namespace RouteEnums;
using namespace RouteInfoParts;

// ================= MEMBER FUNCTIONS =======================

CRouteView::CRouteView(CWayFinderAppUi* aUi, isab::Log* aLog) : 
   CViewBase(aUi),
   iLog(aLog) 
{}

// ---------------------------------------------------------
// CRouteView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CRouteView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_DESTINATION_VIEW );
   iDestination.Copy( _L("") );
   iDistance = -1;
   iEta = 0;
   iDestinationLat = MAX_INT32;
   iDestinationLon = MAX_INT32;
   iCurrentTurn = 0;
}

CRouteView* CRouteView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CRouteView* self = new (ELeave) CRouteView(aUi, aLog);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CRouteView* CRouteView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CRouteView *self = CRouteView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CRouteView::~CRouteView()
// ?implementation_description
// ---------------------------------------------------------
//
CRouteView::~CRouteView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
}

void CRouteView::SetDestination( const TDesC &aDestination )
{
   TBuf<KBuf256Length> tmp;
   tmp.Zero();
   if (aDestination.Length() > iDestination.MaxLength()-2) {
      tmp.Copy( aDestination.Ptr(), iDestination.MaxLength()-2 );
      tmp.PtrZ();
   } else {
      tmp.Copy(aDestination);
   }

   if ((tmp.Compare(iDestination)) && (tmp.Length() > 0)) {
      /* Strings are different. */
      iDestination.Copy(tmp);
      if( iContainer != NULL ) {
         iContainer->SetDestination( iDestination );
      }
   }
}

void CRouteView::SetCoordinates( TInt32 aLat, TInt32 aLon )
{
   iDestinationLat = aLat;
   iDestinationLon = aLon;
}

void
CRouteView::SetInputData(int32 /* lat */, int32 /* lon */,
                         TInt heading, TInt speed, TInt /* alt */)
{
   if( iContainer != NULL ){
      if (heading >= 0) {
         iContainer->SetHeading( heading );
      }
      iContainer->SetSpeed( speed );
      iContainer->DrawNow();
   }
}

void CRouteView::UpdateRouteDataL( TInt32 aDistanceToGoal,
      TInt32 aEstimatedTimeToGoal,
      TInt aAngleToGoal )
{
/*    CDataHolder *iDataHolder = iWayfinderAppUi->GetDataHolder(); */

   iDistance = aDistanceToGoal;
   iEtg = aEstimatedTimeToGoal;
   iGoalHeading = (TInt)((360*aAngleToGoal/256.0)+0.5);

/*    iDataHolder->SetRouteData( aDistanceToGoal, */
/*          aEstimatedTimeToGoal, */
/*          aAngleToGoal); */
   if( iContainer != NULL ){
      iContainer->SetDistanceToGoal( iDistance );
      iContainer->SetETG( iEtg );
      iContainer->SetHeading( iGoalHeading );
      iContainer->DrawNow();
   }
}

void CRouteView::SetCurrentTurn( TInt aTurn )
{
   iCurrentTurn = aTurn;
}

void CRouteView::GoToTurn( TInt aNextTurn )
{
   iWayfinderAppUi->RequestRouteListL( aNextTurn, 1 );
}

void CRouteView::SetRouteList( const RouteList* aRouteList )
{
   if( aRouteList->crossings.size() > 0 ){
      vector<RouteListCrossing *> crossings = aRouteList->crossings;
      vector<RouteListCrossing *>::iterator crossIt;
      RouteInfoParts::RouteListCrossing* routeCrossing;

      //The previous wp
      crossIt = crossings.begin();
      routeCrossing = *crossIt;
      iDistance = routeCrossing->distToGoal;
      iEtg = routeCrossing->timeToGoal;
      iGoalHeading = (TInt)((360*routeCrossing->toTarget/256.0)+0.5);

      /* Nonsensical code */
/*       TInt wrongTurn; */
/*       if(routeCrossing->wptNo != iCurrentTurn ) */
/*          wrongTurn = routeCrossing->wptNo; */
      
      iContainer->SetHeading( iGoalHeading );
      iContainer->SetDistanceToGoal( iDistance );
      iContainer->SetETG( iEtg );
      iContainer->DrawNow();
   }
}

void CRouteView::ShowOnMap()
{
   iWayfinderAppUi->RequestMap( MapEnums::DestinationPosition,
         iDestinationLat, iDestinationLon );
}

TInt
CRouteView::GetDistanceMode()
{
   return iWayfinderAppUi->GetDistanceMode();
}

TBool
CRouteView::IsIronVersion()
{
   return iWayfinderAppUi->IsIronVersion();
}

// ---------------------------------------------------------
// TUid CRouteView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CRouteView::Id() const
{
   return KDestinationViewId;
}

// ---------------------------------------------------------
// CRouteView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CRouteView::HandleCommandL(TInt aCommand)
{   
   switch ( aCommand )
   {
      case EWayFinderCmdDestinationReroute:
      case EWayFinderCmdDestinationVoice:
         {
            if (iWayfinderAppUi->IsGpsAllowed() ) {
               AppUi()->HandleCommandL( EWayFinderCmdDestinationReroute );
            } else {
               AppUi()->HandleCommandL( EWayFinderCmdDestinationVoice );
            }
         }
         break;
      case EWayFinderCmdDestinationShow:
         {
            ShowOnMap();
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
// CRouteView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CRouteView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CRouteView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CRouteView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_DESTINATION_MENU ){
      if (iWayfinderAppUi->IsGpsConnected()) {
         aMenuPane->SetItemDimmed(EWayFinderCmdInfo, EFalse);
      } else {
         aMenuPane->SetItemDimmed(EWayFinderCmdInfo, ETrue);         
      }
      if (iWayfinderAppUi->IsIronVersion()) {
         aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOn, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOff, ETrue );
      } else {
         if (iWayfinderAppUi->IsNightMode()) {
            aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOn, ETrue );
         } else {
            aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOff, ETrue );
         } 
      }
      if (iWayfinderAppUi->IsIronVersion() || !iWayfinderAppUi->IsSimulating()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdRouteSubMenu, ETrue );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdRouteSubMenu, EFalse );
      }
      if (iWayfinderAppUi->HasRoute()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdDestinationShow, EFalse );
         if (iWayfinderAppUi->IsGpsAllowed()) {
            aMenuPane->SetItemDimmed( EWayFinderCmdDestinationReroute, EFalse );
/*             aMenuPane->SetItemDimmed( EWayFinderCmdDestinationVoice, ETrue ); */
         } else {
            aMenuPane->SetItemDimmed( EWayFinderCmdDestinationReroute, ETrue );
/*             if (iWayfinderAppUi->HaveVoiceInstructions()) { */
/*                aMenuPane->SetItemDimmed(EWayFinderCmdDestinationVoice, EFalse); */
/*             } else{ */
/*                aMenuPane->SetItemDimmed(EWayFinderCmdDestinationVoice, ETrue); */
/*             } */
         }
      } else {
         /* No route set, hence no destination. */
         aMenuPane->SetItemDimmed( EWayFinderCmdDestinationShow, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdDestinationReroute, ETrue );
      }
/*    } else if (aResourceId==R_WAYFINDER_VIEWS_MENU) { */
/*       aMenuPane->SetItemDimmed( EWayFinderCmdDestination, ETrue); */
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CRouteView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CRouteView::DoActivateL( const TVwsViewId& aPrevViewId,
                               TUid aCustomMessageId,
                               const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   if ( !iContainer ){
      iContainer = CRouteContainer::NewL(this,
            ClientRect(),
            iWayfinderAppUi->iPathManager->GetMbmName(),
            iLog,
            iWayfinderAppUi);

      AppUi()->AddToStackL( *this, iContainer );
   }
   CDataHolder *iDataHolder = iWayfinderAppUi->GetDataHolder();

   int32 lat = 0, lon = 0;
   TInt heading = 0, speed = 0, alt = 0;
   iDataHolder->GetInputData(lat, lon, heading, speed, alt);

   SetInputData(lat, lon, heading, speed, alt);

   iContainer->SetDestination( iDestination );
   iContainer->SetDistanceToGoal( iDistance );
   iContainer->SetETG( iEtg );

   if (iWayfinderAppUi->HasRoute() && 
       (!iWayfinderAppUi->IsGpsAllowed() || iWayfinderAppUi->IsIronVersion())) {
      GoToTurn(iCurrentTurn);
   }
}

// ---------------------------------------------------------
// CRouteView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CRouteView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

void CRouteView::SetNightModeL(TBool aNightMode)
{
   if (iContainer) {
      iContainer->SetNightModeL(aNightMode);
   }
}


// End of File
