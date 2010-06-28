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

#include <eikmenup.h>
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include <arch.h>
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "WayFinderAppUi.h"

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "InfoView.h"
#include "MapView.h"
#include "InfoContainer.h"
/* #include "NavPacket.h" */
#include "memlog.h"
#include "BufferArray.h"
#include "DataHolder.h"
#include "Quality.h"
#include "TimeOutTimer.h"
#include "DistancePrintingPolicy.h"

#include "PathFinder.h"

using namespace isab;

static const TInt KTimeOut = 5000000; // 5 seconds time-out

CInfoView::CInfoView(isab::Log* aLog) : iLog(aLog) 
{ }
// ---------------------------------------------------------
// CInfoView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CInfoView::ConstructL(CWayFinderAppUi * aWayFinderUI) 
{
   BaseConstructL( R_WAYFINDER_INFO_VIEW );
   iWayFinderUI = aWayFinderUI;
   iLastReadGpsState = iWayFinderUI->GpsQuality();
   iDistance = -1;
   iReferenceTime.HomeTime();

   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);

   if (!iTimer->IsActive()) {  
      iTimer->After(KTimeOut);
   }
}

CInfoView* CInfoView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   
   CInfoView* self = new (ELeave) CInfoView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aUi);
   return self;
}

class CInfoView* CInfoView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CInfoView *self = CInfoView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CInfoView::~CInfoView()
// ?implementation_description
// ---------------------------------------------------------
//
CInfoView::~CInfoView()
{
   delete iTimer;
   
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
}

void
CInfoView::UpdateContainer()
{
   if (!iContainer)
      return;
   
   iContainer->SetPosL(iCurLat, iCurLon);
   iContainer->SetAltitude( iCurAlt );

   if (iCurHeading >= 0) {
      iContainer->SetHeading( iCurHeading );
   }

   iContainer->SetSpeed( iCurSpeed );
   iContainer->SetGpsState( iCurGpsState );
}


void CInfoView::AttemptReading()
{
   TTime currentTime;
   currentTime.HomeTime();
   TTimeIntervalSeconds interval;
   TInt res = currentTime.SecondsFrom(iReferenceTime, interval);
   
   if( (res == KErrNone) && (interval.Int() > 8) ) {
      iReferenceTime.HomeTime();

      if( iCurGpsState != iLastReadGpsState ) {
         iLastReadGpsState = iCurGpsState;
         
         if( iContainer ) {
            UpdateContainer();
            iContainer->UpdateReading();            
         }
      } 
   }    
}

void
CInfoView::SetInputData(int32 lat, 
                        int32 lon,
                        TInt heading, 
                        TInt speed, 
                        TInt alt,
                        TInt aGpsState) 
{
   iCurLat = lat;
   iCurLon = lon;
   iCurHeading = heading;
   iCurSpeed = speed;
   iCurAlt = alt;
   iCurGpsState = aGpsState;

   AttemptReading();
}


void CInfoView::TimerExpired()
{
   AttemptReading();

   if (!iTimer->IsActive()) {  
      iTimer->After(KTimeOut);
   }
}


void CInfoView::UpdateGpsState( TInt aGpsState )
{
   iCurGpsState = aGpsState;
   AttemptReading();
}

#if 0
void CInfoView::UpdateDataL( UpdatePositionMess* aPositionMess )
{
   TInt32 lat = aPositionMess->getLat();
   TInt32 lon = aPositionMess->getLon();

   TInt heading = -1;
   if( aPositionMess->headingQuality() > QualityUseless ){
      heading = aPositionMess->getHeading();
   }

   TInt speed = -1;
   if( aPositionMess->speedQuality() > QualityUseless ){
      TReal rSpeed =
         DistancePrintingPolicy::convertSpeed(
            aPositionMess->getSpeed(),
            DistancePrintingPolicy::DistanceMode(
               iWayFinderUI->GetDistanceMode()));
      
      speed = (TInt)((rSpeed/32)+0.5);
   }

   CDataHolder *iDataHolder = iWayFinderUI->GetDataHolder();
   iDataHolder->SetInputData( lat, lon, heading, speed );

   SetInputData( lat, lon, heading, speed );
}
#endif

void CInfoView::UpdateRouteDataL( TInt32 aDistanceToGoal,
                                  TInt32 aEstimatedTimeToGoal,
                                  TInt aSpeedLimit )
{
   CDataHolder *iDataHolder = iWayFinderUI->GetDataHolder();

   aSpeedLimit = (TInt)(((aSpeedLimit/32)*3.6)+0.5);
   iDistance = aDistanceToGoal;

   iDataHolder->SetRouteData( aDistanceToGoal,
         aEstimatedTimeToGoal,
         aSpeedLimit);

   if( iContainer != NULL ){
      iContainer->SetSpeedLimit( aSpeedLimit );
   }
}

void CInfoView::ShowOnMap()
{
   iWayFinderUI->RequestMap( MapEnums::UserPosition, MAX_INT32, MAX_INT32 );
}

TInt
CInfoView::GetDistanceMode()
{
   return iWayFinderUI->GetDistanceMode();
}

// ---------------------------------------------------------
// TUid CInfoView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CInfoView::Id() const
{
   return KInfoViewId;
}

// ---------------------------------------------------------
// CInfoView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CInfoView::HandleCommandL(TInt aCommand)
{   
   switch ( aCommand )
   {
   case EWayFinderCmdRefreshInfoList:
      UpdateContainer();
      iContainer->UpdateReading();      
      break;
   case EWayFinderCmdInfoShow:
      {
         ShowOnMap();
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
// CInfoView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CInfoView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CInfoView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CInfoView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if (aResourceId == R_WAYFINDER_INFO_MENU) {
      if (!iWayFinderUI->IsGpsAllowed()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdInfoShow, ETrue);
      }
/*       aMenuPane->SetItemDimmed( EWayFinderCmdMapViews, ETrue ); */
/*    } else if (aResourceId==R_WAYFINDER_VIEWS_MENU) { */
/*       aMenuPane->SetItemDimmed( EWayFinderCmdItinerary, ETrue); */
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CInfoView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CInfoView::DoActivateL( const TVwsViewId& aPrevViewId,
                               TUid aCustomMessageId,
                               const TDesC8& /*aCustomMessage*/)
{
   iWayFinderUI->GetNavigationDecorator()->MakeScrollButtonVisible( ETrue );
   iCustomMessageId = aCustomMessageId;
   iReferenceTime.HomeTime();
   
   if ( !iContainer ){
      iContainer = CInfoContainer::NewL(
            this,
            ClientRect(),
            iWayFinderUI->iPathManager->GetMbmName(),
            iLog
            );
      AppUi()->AddToStackL( *this, iContainer );
   }
   
   CDataHolder *iDataHolder = iWayFinderUI->GetDataHolder();
   int32 lat, lon;
   TInt heading, speed, alt;
   iDataHolder->GetInputData(lat, lon, heading, speed, alt);

   int32 DTG, ETG;
   TInt speedLimit;

   iDataHolder->GetRouteData( DTG, ETG, speedLimit); 

   SetInputData( lat, lon, heading, speed, alt, iWayFinderUI->GpsQuality() );
   iContainer->SetSpeedLimit( speedLimit );

   UpdateContainer();
}

// ---------------------------------------------------------
// CInfoView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CInfoView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

void CInfoView::ForceRefresh()
{
   if( iContainer ) {
      UpdateContainer();
      iContainer->UpdateReading();
   }
}

bool
CInfoView::IsHeadingUsable() const
{  
   return iWayFinderUI->IsHeadingUsable();
}

// End of File



