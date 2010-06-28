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

#include "DistancePrintingPolicy.h"

#include "PathFinder.h"

using namespace isab;

// ================= MEMBER FUNCTIONS =======================

CInfoView::CInfoView(CWayFinderAppUi* aUi, isab::Log* aLog) : 
   CViewBase(aUi),
   iLog(aLog) 
{}
// ---------------------------------------------------------
// CInfoView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CInfoView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_INFO_VIEW );
   iDistance = -1;
}

CInfoView* CInfoView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CInfoView* self = new (ELeave) CInfoView(aUi, aLog);
   CleanupStack::PushL(self);
   self->ConstructL();
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
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
}

void
CInfoView::SetInputData(int32 lat, int32 lon,
                        TInt heading, TInt speed, TInt alt)
{
   if (iContainer) {
      iContainer->SetPosL(lat, lon);
/*       iContainer->SetLatitudeL( lat ); */
/*       iContainer->SetLongitudeL( lon ); */
      iContainer->SetAltitude( alt );

      if (heading >= 0) {
         iContainer->SetHeading( heading );
      }
      iContainer->SetSpeed( speed );

/*       iContainer->DrawDeferred(); */
   }
}

void CInfoView::UpdateRouteDataL( TInt32 aDistanceToGoal,
                                  TInt32 aEstimatedTimeToGoal,
                                  TInt aSpeedLimit )
{
   CDataHolder *iDataHolder = iWayfinderAppUi->GetDataHolder();

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
   iWayfinderAppUi->RequestMap( MapEnums::UserPosition, MAX_INT32, MAX_INT32 );
}

TInt
CInfoView::GetDistanceMode()
{
   return iWayfinderAppUi->GetDistanceMode();
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
   case EWayFinderCmdInfoShow:
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
      if (!iWayfinderAppUi->IsGpsAllowed()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdInfoShow, ETrue);
         //aMenuPane->SetItemDimmed( EWayFinderCmdSend, ETrue );
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
                               const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   if ( !iContainer ){
      iContainer = CInfoContainer::NewL(
            this,
            ClientRect(),
            iWayfinderAppUi->iPathManager->GetMbmName(),
            iLog
            );
      AppUi()->AddToStackL( *this, iContainer );
   }
   CDataHolder *iDataHolder = iWayfinderAppUi->GetDataHolder();
   int32 lat, lon;
   TInt heading, speed, alt;
   iDataHolder->GetInputData(lat, lon, heading, speed, alt);

   int32 DTG, ETG;
   TInt speedLimit;

   iDataHolder->GetRouteData( DTG, ETG, speedLimit); 

   SetInputData(lat, lon, heading, speed, alt);
   iContainer->SetSpeedLimit( speedLimit );
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

// End of File
