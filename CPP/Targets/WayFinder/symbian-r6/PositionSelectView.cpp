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
#include "WayFinderAppUi.h"
#include "WFTextUtil.h"
#include "PathFinder.h"
#include "Dialogs.h"

#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "MyDestView.h"
#include "GuiDataStore.h"
#include "PositionSelectView.h"
#include "PositionSelectContainer.h" 
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtRouteMess.h"
#include "GuiProt/GuiParameterEnums.h"

#include "MC2Coordinate.h"

using namespace isab;
// ================= MEMBER FUNCTIONS =======================

CPositionSelectView::CPositionSelectView(CWayFinderAppUi* aUi, 
                                         isab::Log* aLog, 
                                         GuiProtMessageSender& aSender) : 
   CViewBase(aUi),
   iLog(aLog),
   iSender(aSender)
{
}
// ---------------------------------------------------------
// CPositionSelectView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CPositionSelectView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_POSITIONSELECT_VIEW );
   iOriginName = iCoeEnv->AllocReadResourceL(
         R_POSITIONSELECT_ORIGIN_TEXT );
   iDestinationName = iCoeEnv->AllocReadResourceL(
         R_POSITIONSELECT_DESTINATION_TEXT );
   iIsOriginSet = EFalse;
   iIsDestinationSet = EFalse;
   iOriginSelected = ETrue;
   iOriginPoint.iX = MAX_INT32;
   iOriginPoint.iY = MAX_INT32;
}

CPositionSelectView* CPositionSelectView::NewLC(CWayFinderAppUi* aUi, 
                                                isab::Log* aLog, 
                                                GuiProtMessageSender& aSender)
{
   CPositionSelectView* self = new (ELeave) CPositionSelectView(aUi, aLog, aSender);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CPositionSelectView* CPositionSelectView::NewL(CWayFinderAppUi* aUi, 
                                                     isab::Log* aLog, 
                                                     GuiProtMessageSender& aSender)
{
   CPositionSelectView *self = CPositionSelectView::NewLC(aUi, aLog, aSender);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CPositionSelectView::~CPositionSelectView()
// ?implementation_description
// ---------------------------------------------------------
//
CPositionSelectView::~CPositionSelectView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;

   delete iDestinationName;
   delete iOriginName;
   delete iEventGenerator;
}

GuiDataStore*
CPositionSelectView::GetGuiDataStore()
{
   return iWayfinderAppUi->GetGuiDataStore();
}

TBool
CPositionSelectView::VectorMaps()
{
   return iWayfinderAppUi->VectorMaps();
}
TPoint
CPositionSelectView::GetDestination()
{
   return iDestinationPoint;
}
TPoint
CPositionSelectView::GetOrigin()
{
   return iOriginPoint;
}
TBool
CPositionSelectView::HasOrigin()
{
   return iIsOriginSet;
}
TBool
CPositionSelectView::HasDestination()
{
   return iIsDestinationSet;
}

void
CPositionSelectView::SetOrigin(isab::GuiProtEnums::PositionType aType,
      const TDesC& aName, const char* aId, TInt32 aLat, TInt32 aLon )
{
   iOriginType = aType;
   if (iOriginName) {
      delete iOriginName;
      iOriginName = NULL;
   }
   if( aName.Find(KTab) == KErrNotFound ){
      iOriginName = HBufC::NewL(aName.Length()+10);
      iOriginName->Des().Copy( _L(" \t") );
      iOriginName->Des().Append( aName );
      iOriginName->Des().Append( KTab );
   } else {
      iOriginName = aName.AllocL();
   }
   if( iOriginId != NULL ){
      LOGDELA(iOriginId);
      delete[] iOriginId;
   }
   if (aId == NULL) {
      aId = "";
   }
   iOriginId = WFTextUtil::strdupL(aId);
   iOriginPoint.iY = aLat;
   iOriginPoint.iX = aLon;
   iIsOriginSet = ETrue;
   if( iContainer ){
      iContainer->SetOrigin( *iOriginName );
      iContainer->DrawNow();
   }
}

void CPositionSelectView::ResetOriginL()
{
   TBuf<128> origin;
   isab::GuiProtEnums::PositionType originType;
   if(iWayfinderAppUi->IsGpsConnected()){
      iCoeEnv->ReadResourceL(origin, R_POSITIONSELECT_CURRENT_POS_TEXT);
      originType = GuiProtEnums::PositionTypeCurrentPos;
   } else {
      iCoeEnv->ReadResourceL(origin, R_POSITIONSELECT_ORIGIN_TEXT);
      originType = GuiProtEnums::PositionTypeInvalid;
   }  
   SetOrigin(originType, origin, NULL, MAX_INT32, MAX_INT32);
   if(!iWayfinderAppUi->IsGpsConnected()){
      iIsOriginSet = EFalse;
   }
}

void CPositionSelectView::SetDestination(
      isab::GuiProtEnums::PositionType aType, const TDesC& aName,
      const char* aId, TInt32 aLat, TInt32 aLon, TBool aResetting )
{
   iDestinationType = aType;
   if (iDestinationName) {
      delete iDestinationName;
      iDestinationName = NULL;
   }
   if( aName.Locate('\t') == KErrNotFound ){
      iDestinationName = HBufC::NewL(aName.Length()+10);
      iDestinationName->Des().Copy( _L(" \t") );
      iDestinationName->Des().Append( aName );
      iDestinationName->Des().Append( KTab );
   } else {
      iDestinationName = aName.AllocL();
   }
   if( iDestinationId != NULL ){
      LOGDELA(iDestinationId);
      delete[] iDestinationId;
   }
   if (aId == NULL) {
      aId = "";
   }
   iDestinationId = WFTextUtil::strdupL(aId);
   iDestinationPoint.iY = aLat;
   iDestinationPoint.iX = aLon;
   iIsDestinationSet = ETrue;
   if( iContainer ){
      iContainer->SetDestination( *iDestinationName );
      iContainer->DrawNow();
   }
}

void CPositionSelectView::ResetDestinationL()
{
   TBuf<128> dest;
   iCoeEnv->ReadResourceL(dest, R_POSITIONSELECT_DESTINATION_TEXT);
   SetDestination(GuiProtEnums::PositionTypeInvalid, dest, 
                  NULL, MAX_INT32, MAX_INT32, ETrue);
   iIsDestinationSet = EFalse;
}

void CPositionSelectView::Route()
{
   if (iIsOriginSet && iIsDestinationSet) {
      if ((iOriginType != GuiProtEnums::PositionTypeCurrentPos && 
           (iOriginPoint.iY == MAX_INT32 || iOriginPoint.iX == MAX_INT32)) || 
          iDestinationPoint.iY == MAX_INT32 || 
          iDestinationPoint.iX == MAX_INT32) {
         // On of the points were invalid, show warning message.
         WFDialog::ShowScrollingInfoDialogL(R_WF_POS_NO_COORDINATES);
      } else {
         // All points valid, request a route.
         TBuf<KBuf64Length> destName;
         TInt tabPos = iDestinationName->LocateReverse('\t');
         if(tabPos != KErrNotFound ){
            WFTextUtil::TDesCopy(destName, iDestinationName->Left( tabPos ) );
            destName.TrimAll();
         }
         char* destStr = WFTextUtil::newTDesDupL( destName);
         LOGNEWA(destStr, char, strlen(destStr) + 1);
         
         /*RouteMess* message = 
            new (ELeave) RouteMess(iOriginType, iOriginId,
                                   iOriginPoint.iY, iOriginPoint.iX,
                                   iDestinationType, iDestinationId,
                                   iDestinationPoint.iY, iDestinationPoint.iX,
                                   destStr);
         LOGNEW(message, RouteMess);
         iWayfinderAppUi->SendMessageL( message );*/
         
         // This blocks deactivation of mapview on some phones (N95).
         //iWayfinderAppUi->StartWaitDlg(this, R_WAYFINDER_ROUTING_MSG);
         /*message->deleteMembers();
         LOGDEL(message);
         delete message;*/
         iWayfinderAppUi->
           SetPendingRouteMessageAndSendVehicleTypeL(iOriginType,
                                                     iOriginId,
                                                     iOriginPoint.iY,
                                                     iOriginPoint.iX,
                                                     iDestinationType,
                                                     iDestinationId,
                                                     iDestinationPoint.iY,
                                                     iDestinationPoint.iX,
                                                     destStr);

         iWayfinderAppUi->SetCurrentRouteEndPoints(destName,
                                                   iDestinationPoint.iY, 
                                                   iDestinationPoint.iX,
                                                   iOriginPoint.iY, 
                                                   iOriginPoint.iX);

         delete[] destStr;
      }
   }
}

TBool
CPositionSelectView::IsCurrentSet()
{
   if( iContainer->IsOriginSelected() ) {
      return iIsOriginSet;
   } else {
      return iIsDestinationSet;
   }
}

TBool
CPositionSelectView::AreSet()
{
   return (iIsOriginSet && iIsDestinationSet);
}

TBool
CPositionSelectView::IsGpsConnected()
{
   return iWayfinderAppUi->IsGpsConnected();
}

// ---------------------------------------------------------
// TUid CPositionSelectView::Id()
// ---------------------------------------------------------
TUid CPositionSelectView::Id() const
{
   return KPositionSelectViewId;
}

// ---------------------------------------------------------
// CPositionSelectView::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
void CPositionSelectView::HandleCommandL(TInt aCommand)
{
   switch ( aCommand )
   {
   case EAknSoftkeyOk:
         break;
   case EAknSoftkeyExit:
      {
         AppUi()->HandleCommandL(EAknSoftkeyExit);
         break;
      }
   case EWayFinderCmdPositionSelectGetRoute:
      {
         ShowRouteAsQueryDlgL();
         break;
      }
   case EWayFinderCmdPositionSelectNavigate:
      {
         Route();
         break;
      }
   case EWayFinderCmdPositionSelectReroute:
      {
         AppUi()->HandleCommandL(EWayFinderCmdPositionSelectReroute);
         break;
      }
   case EWayFinderCmdPositionSelectFromGps:
      {
         TBuf<64> txt;
         iCoeEnv->ReadResource(txt, R_POSITIONSELECT_CURRENT_POS_TEXT);

         SetOrigin(GuiProtEnums::PositionTypeCurrentPos,
                   txt, NULL, MAX_INT32, MAX_INT32);
         break;
      }
   case EWayFinderCmdPositionSelectSearch:
      {
         iWayfinderAppUi->push( KCSMainViewId, GetViewState() );
         break;
      }
   case EWayFinderCmdPositionSelectFavorite:
      {
         iWayfinderAppUi->push( KMyDestViewId, GetViewState() );
         break;
      }
   case EWayFinderCmdPositionSelectFromMap:
      {
         iWayfinderAppUi->push( KMapViewId, GetViewState() );
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
// CPositionSelectView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CPositionSelectView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

TInt CPositionSelectView::GetViewState()
{
   TInt selected;
   if (iContainer) {
      selected = iContainer->GetActiveSelection();
   } else {
      selected = 0;
   }
   if (selected == 0) {
      return ERoutePlannerSetAsOrigin;
   } else if (selected == 1) {
      return ERoutePlannerSetAsDest;
   } else {
      return ENoMessage;
   }
}

// ---------------------------------------------------------
// CPositionSelectView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CPositionSelectView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_POSITIONSELECT_MENU ){
      if( iIsOriginSet && iIsDestinationSet ){
         aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectGetRoute, EFalse );
#ifdef NAV2_CLIENT_SERIES60_V5
         aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectReroute, AknLayoutUtils::PenEnabled());
#else
         aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectReroute, EFalse);
#endif 
      } else {
         aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectGetRoute, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectReroute, ETrue);
      }

      if (IsGpsConnected() && 
          (iContainer && iContainer->GetActiveSelection() == 0)) {
         aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectFromGps, EFalse);
      } else {
         aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectFromGps, ETrue);
      }
      aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectSearch, EFalse);
      aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectFavorite, EFalse);
      aMenuPane->SetItemDimmed(EWayFinderCmdPositionSelectFromMap, EFalse);
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CPositionSelectView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CPositionSelectView::DoActivateL( const TVwsViewId& aPrevViewId,
                                  TUid aCustomMessageId,
                                  const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   // Explicitly hide the arrows in navipane since on s60v5 they tend to 
   // show up in some views otherwise.
   iWayfinderAppUi->setNaviPane(EFalse);

   iCustomMessage = aCustomMessageId.iUid;

   if (!iContainer){
      iContainer = new (ELeave) CPositionSelectContainer(iLog);
      LOGNEW(iContainer, CPositionSelectContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect(), this );
      AppUi()->AddToStackL( *this, iContainer );
      if (!iIsOriginSet) { 
         // Default string depends on if the gps i connected or not.
         if (iOriginName) { 
            delete iOriginName;
            iOriginName = NULL;
         } 
         HandleCommandL(EWayFinderCmdPositionSelectFromGps);
      } 
      iContainer->SetOrigin( *iOriginName );
      iContainer->SetDestination( *iDestinationName );
      iContainer->DrawNow();
      if (!iOriginSelected) {
         /* Set the current item in the list to be the destination. */
         iContainer->SetSelected(1);
      }
      // Dialog will not be launched if not both origin and dest is set.
      // We need to let this go through the event generator since otherwise 
      // the dialog is blocking the view de-activation (of mapview at least).
      GenerateEvent(ERoutePlannerEventShowRouteAsQueryDlg);
   }
}

// ---------------------------------------------------------
// CPositionSelectView::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
void CPositionSelectView::DoDeactivate()
{
   iOriginSelected = iContainer->IsOriginSelected();
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }
   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

void CPositionSelectView::ResetSelection()
{
   iOriginSelected = ETrue;
   if (iContainer) {
      iContainer->SetSelected(0);
   }
}

TPtrC CPositionSelectView::GetMbmName()
{
   return iWayfinderAppUi->iPathManager->GetMbmName();
}

void CPositionSelectView::ShowRouteAsQueryDlgL()
{
   if (iIsOriginSet && iIsDestinationSet) {
      TBool showRouteAsQueryDlg = EFalse;
#ifdef NAV2_CLIENT_SERIES60_V5
      if (!AknLayoutUtils::PenEnabled()) {
         // No touch screen, which means no buttons for selecting transportation
         // type, the query list dialog should be displayed.
         showRouteAsQueryDlg = ETrue;
      }
#else
      // No touch for s60v3, display the dialog instead
      showRouteAsQueryDlg = ETrue;
#endif
      /*if (showRouteAsQueryDlg) {
         // Get the selected choice, if user cancelled selected will be
         // EInvalidVehicleType.
         TInt selected = iContainer->ShowRouteAsQueryDialogL();            
         NavServerComEnums::VehicleType vehicleType = 
            GetNav2TransportMode(TTransportationMode(selected));
         // Send vehicleType, if a valid vehicleType then a route will be
         // calculated.
         SendVehicleType(vehicleType);
         }*/
      if (showRouteAsQueryDlg) {
        Route();
      }
   }
}

void CPositionSelectView::GenerateEvent(enum TRoutePlannerEvent aEvent)
{       
   if (!iEventGenerator) {
      iEventGenerator = CRoutePlannerEventGenerator::NewL(*this); 
   }
   iEventGenerator->SendEventL(aEvent);
}

void CPositionSelectView::HandleGeneratedEventL(enum TRoutePlannerEvent aEvent)
{
   switch (aEvent) {
   case ERoutePlannerEventShowRouteAsQueryDlg:
      {
         ShowRouteAsQueryDlgL(); 
      }
      break;
   }
}

bool CPositionSelectView::GuiProtReceiveMessage(isab::GuiProtMess* mess)
{
   switch (mess->getMessageType()) {
   case isab::GuiProtEnums::SET_GENERAL_PARAMETER:
      isab::GeneralParameterMess* gpm =
         static_cast<isab::GeneralParameterMess*>(mess);
      if (gpm->getParamId() == isab::GuiProtEnums::paramTransportationType) {
         // Got message that transportation type was successfully set
         HandleCommandL(EWayFinderCmdPositionSelectNavigate);
      }
      break;
   case isab::GuiProtEnums::REQUEST_FAILED:
      // Something went wrong
   default:
      return EFalse;
   }
   return ETrue;
}

void CPositionSelectView::SendVehicleType(NavServerComEnums::VehicleType aVehicleType)
{
   if (aVehicleType == NavServerComEnums::invalidVehicleType) {
      // Do not send invalidVehicleType.
      return;
   }

   // Send set parameter with the given vehicle type
   GeneralParameterMess setgpm(GuiProtEnums::paramTransportationType, 
                               int32(aVehicleType));
   iSender.SendMessage(setgpm);
   
   // Request the paramTransportationType
   GeneralParameterMess getgpm(GuiProtEnums::paramTransportationType);
   // Send a message to also get the transportation type so we end up in 
   // our GuiProtReceiveMessage function below.
   iSender.SendMessage(getgpm, this);
}

NavServerComEnums::VehicleType 
CPositionSelectView::GetNav2TransportMode(TTransportationMode aMode)
{
   // Convert from TTransportationMode to NavServerComEnums::VehicleType
   switch(aMode) {
   case EPassengerCar:
      return NavServerComEnums::passengerCar;
   case EPedestrian:
      return NavServerComEnums::pedestrian;
   case EInvalidVehicleType:
      break;
   }
   return NavServerComEnums::invalidVehicleType;
}

// End of File

