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
#include "LockedNavigationView.h"
#include "WayFinderConstants.h"		// view ids
#include "RsgInclude.h"					// dodona strings and resources
#include "LockedNavigationContainer.h"
#include "WayFinderAppUi.h"
#include "GfxUtility.h"
#include "MapResourceFactory.h"
#include "TraceMacros.h"
#include "SettingsData.h"

#define READOUT_INTERVAL 18

CLockedNavigationView::CLockedNavigationView(isab::Log * aLog) : 
   iLog(aLog),
   iTargetName(0),
   iContainer(0)
{
   
}

CLockedNavigationView * 
CLockedNavigationView::NewLC(CWayFinderAppUi * aWayFinderUI,
                             isab::Log * aLog) 
{

   CLockedNavigationView * self =
      new (ELeave) CLockedNavigationView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aWayFinderUI);
   
   return self;
}

void
CLockedNavigationView::SetTarget(VicinityItem* aTarget) {
   iTargetCoordinate = aTarget->Coord();

   if(iTargetName)
      delete iTargetName;
   
   iTargetName = WFTextUtil::AllocL(aTarget->Name().c_str());
}

CLockedNavigationView * 
CLockedNavigationView::NewL(CWayFinderAppUi * aWayFinderUI,
                            isab::Log* aLog) 
{
   CLockedNavigationView * self = NewLC(aWayFinderUI, aLog);
   CleanupStack::Pop(self);
   
   return self;
}

void
CLockedNavigationView::ConstructL(CWayFinderAppUi * aWayFinderUI) 
{
   BaseConstructL(R_WAYFINDER_LOCKED_NAVIGATION_VIEW);
   // parent
   iWayFinderUI = aWayFinderUI;  

}

CLockedNavigationView::~CLockedNavigationView() 
{
   TRACE_FUNC1("Entering locked navigation view destructor.");
   if(iContainer) {
      AppUi()->RemoveFromViewStack(* this, iContainer);
      delete iContainer;
   }

   if(iTargetName)
      delete iTargetName;
}

void CLockedNavigationView::DoActivateL( const TVwsViewId& aPrevViewId,
                                         TUid aCustomMessageId,
                                         const TDesC8& aCustomMessage)
{
   if(!iContainer)
   {
      iContainer = new (ELeave) CLockedNavigationContainer;
     
      iContainer->SetMopParent(this);
      iContainer->ConstructL(ClientRect(),
                             this,
                             iWayFinderUI);
   }

   AppUi()->AddToStackL( *this, iContainer );
}

TUid CLockedNavigationView::Id() const
{
   return KLockedNavViewId;
}

void CLockedNavigationView::HandleCommandL(TInt aCommand)
{
   switch ( aCommand )
   {
      case EAknSoftkeyBack:
         iWayFinderUI->GetNavigationDecorator()->MakeScrollButtonVisible( ETrue );
         iWayFinderUI->GotoLastView();
         break;      
      default:
         AppUi()->HandleCommandL(aCommand);
         break;
   }
}

void CLockedNavigationView::DoDeactivate()
{
   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }
	
   delete iContainer;
   iContainer = NULL;
}

void CLockedNavigationView::UpdateState()
{
    
}

void
CLockedNavigationView::SetInputData(int32 lat,
                                    int32 lon,
                                    TInt heading,
                                    TInt speed,
                                    TInt alt,
                                    TInt aGpsState)
{
   MapResourceFactory* mrFactory = iWayFinderUI->GetMapResourceFactory();
   mrFactory->initIfNecessary();
   
   MapLib* mapLib = mrFactory->getMapLib();

   TInt readInterval = iWayFinderUI->iSettingsData->iLockedNavRate;
   
   TTime now;
   now.HomeTime();
   now -= (TTimeIntervalSeconds) readInterval;

   if( now < iReferenceTime || !mapLib->mapFullyLoaded()  || !iContainer )
      return;

   User::ResetInactivityTime();
   
   iReferenceTime.HomeTime();
   
   Nav2Coordinate newCenter(lat, lon);
   
   MC2Coordinate temp(newCenter);
   
   MC2Direction direction =
      GfxUtility::getDirection( temp.lat,
                                temp.lon,
                                iTargetCoordinate.lat,
                                iTargetCoordinate.lon);


   
   MC2SimpleString whereAmI = mapLib->getWhereAmI("\n");

   HBufC* waiBuf = WFTextUtil::AllocLC(whereAmI.c_str());

   TInt distance = mapLib->getDistance(iTargetCoordinate);
   
   iContainer->UpdateText(iTargetName,
                          waiBuf,
                          distance,
                          direction,
                          MC2Direction(heading));
   
   CleanupStack::PopAndDestroy(waiBuf);
}

void CLockedNavigationView::ForceRefresh()
{
   iReferenceTime.HomeTime();
   iReferenceTime-= (TTimeIntervalSeconds)(READOUT_INTERVAL + 2);
}
