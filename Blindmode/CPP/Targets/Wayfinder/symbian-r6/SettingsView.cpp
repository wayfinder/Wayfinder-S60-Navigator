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
#include <eikmenup.h>
#include  <avkon.hrh>

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>
#include <barsread.h>

#include <arch.h>
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/GuiProtEnums.h"
#include "WayFinderAppUi.h"

#include "RsgInclude.h"
#include "wayfinder.hrh"

#include "SettingsView.h"
#include "SettingsContainer.h"
#include "SettingsData.h"
#include "WFTextUtil.h"
#include "Dialogs.h"

#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiParameterEnums.h"
#define ILOG_POINTER iWayFinderUI->iLog
#include "memlog.h"

#include "DistancePrintingPolicy.h"
#include "TrackPIN.h"
#include "TDesTrackPIN.h"
#include "TraceMacros.h"

#include "IAPFileHandler.h"
#include "PathFinder.h"

using namespace isab;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSettingsView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void
CSettingsView::ConstructL( CWayFinderAppUi* aWayFinderUI )
{
   iWayFinderUI = aWayFinderUI;
   iTabsDecorator = NULL;
/*    iData = CSettingsData::NewL(ILOG_POINTER); */
   iData = NULL;
   iShutdownNeeded = EFalse;
   BaseConstructL( R_WAYFINDER_SETTINGS_VIEW );
}

CSettingsView*
CSettingsView::NewLC(CWayFinderAppUi* aUi, isab::Log* /*aLog*/)
{
   CSettingsView* self = new (ELeave) CSettingsView();
   CleanupStack::PushL(self);
   self->ConstructL(aUi);
   return self;
}

class CSettingsView*
CSettingsView::NewL(class CWayFinderAppUi* aUi, 
                                         isab::Log* aLog)
{
   CSettingsView *self = CSettingsView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

void
CSettingsView::SetCache(CSettingsData* aData)
{
   iData = aData;
}

CSettingsView::~CSettingsView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }
   LOGDEL(iContainer);
   delete iContainer;
}


void
CSettingsView::SetWebUsername( TDesC &aUsername )
{
   WFTextUtil::TDesCopy(iData->iWebUsername, aUsername );
}
void
CSettingsView::SetWebPassword( TDesC &aPassword )
{
   WFTextUtil::TDesCopy(iData->iWebPassword, aPassword );
}
void
CSettingsView::SetUsername( TDesC &aUsername )
{
   WFTextUtil::TDesCopy(iData->iUsername, aUsername);
   WFTextUtil::TDesCopy(iData->iOldUsername, aUsername );
}
void
CSettingsView::SetPassword( TDesC &aPassword )
{
   WFTextUtil::TDesCopy(iData->iPassword, aPassword);
   WFTextUtil::TDesCopy(iData->iOldPassword, aPassword );
}

void
CSettingsView::SetServer( TDesC &aServer )
{
   WFTextUtil::TDesCopy(iData->iServer, aServer);
   WFTextUtil::TDesCopy(iData->iServerOld, aServer );
}

void
CSettingsView::SetVolume( TInt aVolume )
{
   iData->iVolume = TInt((aVolume+0.5)/10);
}

void
CSettingsView::SetAutoReroute( TBool aAutoReroute )
{
   iData->iAutoReroute = aAutoReroute;
}

void
CSettingsView::SetTollRoads(NavServerComEnums::RouteTollRoads aTollRoads )
{
   iData->iTollRoads = aTollRoads;
}

void
CSettingsView::SetHighways(NavServerComEnums::RouteHighways aHighways )
{
   iData->iHighways = aHighways;
}

void
CSettingsView::SetTimeDist(NavServerComEnums::RouteCostType aTimeDist )
{
   iData->iTimeDist = aTimeDist;
}

void
CSettingsView::SetUseSpeaker( TBool aUseSpeaker )
{
   iData->iUseSpeaker = aUseSpeaker;
}

void
CSettingsView::SetTurnSoundsLevel( isab::GuiProtEnums::TurnSoundsLevel aLevel)
{
   iData->iTurnSoundsLevel = aLevel;
}

void
CSettingsView::SetVehicle( NavServerComEnums::VehicleType aVehicle )
{
   iData->iOldVehicle = iData->iVehicle;
   if( aVehicle == 0 || aVehicle == NavServerComEnums::invalidVehicleType )
      iData->iVehicle = NavServerComEnums::passengerCar;
   else
      iData->iVehicle = aVehicle;
}

void
CSettingsView::SetDistanceMode( DistancePrintingPolicy::DistanceMode aMode )
{
   if( aMode == 0 || aMode == DistancePrintingPolicy::ModeInvalid ) {
      iData->iDistanceMode = DistancePrintingPolicy::ModeMetric;
   } else {
      iData->iDistanceMode = aMode;
   }
}

void
CSettingsView::SetSaveSMSDestination( GuiProtEnums::YesNoAsk aTripple )
{
   if( aTripple != GuiProtEnums::yes && aTripple != GuiProtEnums::no && aTripple != GuiProtEnums::ask )
      iData->iSaveSMSDest = GuiProtEnums::ask;
   else
      iData->iSaveSMSDest = aTripple;
}

void
CSettingsView::SetAutoTrackingOn( TBool aAutoTrackingOn )
{
   iData->iAutoTrackingOn = aAutoTrackingOn;
}

void
CSettingsView::SetAutorouteSMS( TBool aAutoRoute )
{
   iData->iAutoSMSRoute = aAutoRoute;
}

void
CSettingsView::SetKeepSMS( GuiProtEnums::YesNoAsk aTripple )
{
   if( aTripple != GuiProtEnums::yes && aTripple != GuiProtEnums::no && aTripple != GuiProtEnums::ask )
      iData->iKeepSMSDest = GuiProtEnums::ask;
   else
      iData->iKeepSMSDest = aTripple;
}

void
CSettingsView::SetBacklightStrategy( GuiProtEnums::BacklightStrategy aStrategy )
{
   iData->iBacklightStrategy = aStrategy;
}

void
CSettingsView::SetIAP( TInt iap )
{
   iData->iIAP = iap;
   iData->iOldIAP = iap;
}

void
CSettingsView::SetLanguage( TInt language )
{
   iData->iLanguage = language;
   iData->iOldLanguage = language;
}

void CSettingsView::SetMapCacheSize( TInt aSize )
{
   iData->iMapCacheSize = aSize;
}

void CSettingsView::SetMapType( MapType aMapType )
{
   iData->iMapType = aMapType;
   iData->iOldMapType = aMapType;
}

void CSettingsView::SetPreferredGuideMode( enum preferred_guide_mode aGuideMode )
{
   iData->iPreferredGuideMode = aGuideMode;
}

void CSettingsView::SetFavoriteShow( TInt aFavShow )
{
   iData->iFavoriteShow = aFavShow;
}

void CSettingsView::SetGPSAutoConnect( TInt aAuto )
{
   iData->iGPSAutoConnect = aAuto;
}

void CSettingsView::SetBtGpsAddressL(const char** aString, TInt aCount)
{
   iData->SetBtGpsAddressL(aString, aCount);
}

void CSettingsView::SetTrackingType( TrackingType aMapTrackingOrientation )
{
   iData->iMapTrackingOrientation = aMapTrackingOrientation;
}


void
CSettingsView::setTrackingLevel( int32 aLevel ) {
   iData->iTrackingLevel = aLevel;
   iData->iOldTrackingLevel = aLevel;
}


void
CSettingsView::setTrackingPIN( const TDesC &aPIN )
{
   WFTextUtil::TDesCopy(iData->iTrackingPIN, aPIN);
   WFTextUtil::TDesCopy(iData->iOldTrackingPIN, aPIN);
}


void
CSettingsView::setTrackPINList( TrackPINList* l ) {
   delete iData->m_trackPINList;
   iData->m_trackPINList = l;
   if ( l->size() > 0 ) {
      class CDesTrackPIN* pin = CDesTrackPIN::NewL( **(l->begin()) ); 
//      TRACE_DBG( "PIN %s", WFTextUtil::TDesCToUtf8L( *pin->getTDesPIN()) );
      setTrackingPIN( pin->GetTDesPIN() );
      delete pin;
   } else {
      setTrackingPIN( _L( "" ) );
   }
}

void
CSettingsView::setRouteTrafficInfoUpdEnabled( TBool aUpdEnabled ) 
{
   iData->iRouteTrafficInfoEnabled = aUpdEnabled;
}

void
CSettingsView::setRouteTrafficInfoUpdTime( int32 aUpdTime ) 
{
   iData->iRouteTrafficInfoUpdTime = aUpdTime;
}

void
CSettingsView::SendGuiProtMessage(GuiProtMess* mess)
{
   iWayFinderUI->SendMessageL( mess);
   mess->deleteMembers();
   delete mess;
}
/* void */
/* CSettingsView::SendGeneralParamMessage(GeneralParameterMess* mess) */
/* { */
/*    iWayFinderUI->SendMessageL( mess); */
/*    mess->deleteMembers(); */
/*    delete mess; */
/* } */

void
CSettingsView::SaveSettings()
{
   iContainer->StoreSettingsL();

   SimpleParameterMess* message;
   GeneralParameterMess* gen_mess;

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramSoundVolume, (uint8)(iData->iVolume*10) );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramUseSpeaker, (bool)iData->iUseSpeaker );
   SendGuiProtMessage(message);

   isab::GuiProtEnums::TurnSoundsLevel soundsLevel = isab::GuiProtEnums::turnsound_mute;
   if (iData->iOldVehicle == iData->iVehicle) {
      // User did not change this setting.'
      soundsLevel = iData->iTurnSoundsLevel;
   } else {
      // User did change transportation mode
      if (iData->iVehicle == isab::NavServerComEnums::pedestrian) {
         // If user changed to pedestrian, mute
         soundsLevel = isab::GuiProtEnums::turnsound_mute;
      } else {
         // User changed to something else than pedestrian, normal
         soundsLevel = isab::GuiProtEnums::turnsound_normal;
      }
   }
   iData->iOldVehicle = iData->iVehicle;
   //message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramTurnSoundsLevel, (uint8)iData->iTurnSoundsLevel );
   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramTurnSoundsLevel, (uint8)soundsLevel );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramAutoReroute, (bool)iData->iAutoReroute );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramTransportationType, (uint8)iData->iVehicle );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramDistanceMode, (uint8)iData->iDistanceMode );
   SendGuiProtMessage(message);

   gen_mess = new (ELeave) GeneralParameterMess( GuiProtEnums::paramTollRoads, (int32)iData->iTollRoads );
   SendGuiProtMessage(gen_mess);

   gen_mess = new (ELeave) GeneralParameterMess( GuiProtEnums::paramHighways, (int32)iData->iHighways );
   SendGuiProtMessage(gen_mess);

   gen_mess = new (ELeave) GeneralParameterMess( GuiProtEnums::paramTimeDist, (int32)iData->iTimeDist );
   SendGuiProtMessage(gen_mess);

   gen_mess = new (ELeave) GeneralParameterMess( GuiProtEnums::paramAutoTracking, (int32)iData->iAutoTrackingOn );
   SendGuiProtMessage(gen_mess);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramStoreSMSDestInMyDest, (uint8)iData->iSaveSMSDest );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramAutomaticRouteOnSMSDest, (bool)iData->iAutoSMSRoute );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramKeepSMSDestInInbox, (uint8)iData->iKeepSMSDest );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::userDirectionMode, (uint8)iData->iDirectionType );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::userVicinityFeedMode, (uint8)iData->iFeedOutputFormat );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramFeedWaitPeriod, (uint8)iData->iVicinityFeedRate );
   SendGuiProtMessage(message);
   
   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramLockedNavWaitPeriod, (uint8)iData->iLockedNavRate );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramGpsMinWalkingSpeed, (uint8)iData->iWalkingSpeedCutOff );
   SendGuiProtMessage(message);

   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramVicinityListUpdatePeriod, (uint8)iData->iVicinityListPeriod );
   SendGuiProtMessage(message);
   
   message = new (ELeave) SimpleParameterMess(
         GuiProtEnums::paramBacklightStrategy,
         (uint8)iData->iBacklightStrategy );
   SendGuiProtMessage(message);

   TInt updTime = iData->iRouteTrafficInfoUpdTime;
   if (!iData->iRouteTrafficInfoEnabled) {
      // Let nav2 know that user has switched this option off.
      // Set the second highest bit.
      updTime |= 0x40000000;
   }
   gen_mess = new (ELeave) GeneralParameterMess(GuiProtEnums::userTrafficUpdatePeriod,
                                                (int32)updTime);
   SendGuiProtMessage(gen_mess);

   if (iData->iOldIAP != iData->iIAP) {
      //User selected new iap so delete iap.txt file.
      iWayFinderUI->DeleteIapTxt();

      message = new (ELeave) SimpleParameterMess(
            GuiProtEnums::paramSelectedAccessPointId,
            (uint32)iData->iIAP );
      SendGuiProtMessage(message);

      message = new (ELeave) SimpleParameterMess(
            GuiProtEnums::paramSelectedAccessPointId2,
            (uint32)iData->iIAP );
      SendGuiProtMessage(message);
   }

   gen_mess = 
      new (ELeave) GeneralParameterMess(GuiProtEnums::paramLinkLayerKeepAlive,
            (int32)iData->iLinkLayerKeepAlive);
   SendGuiProtMessage(gen_mess);

   char* serverPortStr;

   if (iData->iServer.Compare(iData->iServerOld)) {
      serverPortStr = WFTextUtil::newTDesDupL(iData->iServer);
      message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramServerNameAndPort, serverPortStr );
      SendGuiProtMessage(message);
   }

   serverPortStr = WFTextUtil::newTDesDupL(iData->iWebUsername);
   message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramWebUsername,
         serverPortStr );
   SendGuiProtMessage(message);

   {
      HBufC* buf = CCoeEnv::Static()->AllocReadResourceLC(R_WAYFINDER_SET_PASSWORD_TEXT );
      if ((! iWayFinderUI->IsTrialVersion()) && ( iData->iWebPassword.Length() > 0 ) && iData->iWebPassword.Compare(*buf)) {
         serverPortStr = WFTextUtil::newTDesDupL(iData->iWebPassword);
         message = new (ELeave) SimpleParameterMess(
               GuiProtEnums::paramWebPassword,
               serverPortStr );
         SendGuiProtMessage(message);
      } else {
         /* Web password string was "Set Password"... */
      }
      CleanupStack::PopAndDestroy(buf);
   }

   TBuf<KBuf256Length> serverPort;

   if (iData->iUsername.Compare(iData->iOldUsername)) {
      serverPortStr = WFTextUtil::newTDesDupL(iData->iUsername);
      message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramUsername,
            serverPortStr );
      SendGuiProtMessage(message);
   }

   if (iData->iPassword.Compare(iData->iOldPassword)) {
      serverPortStr = WFTextUtil::newTDesDupL(iData->iPassword);
      message = new (ELeave) SimpleParameterMess( GuiProtEnums::paramPassword,
            serverPortStr );
      SendGuiProtMessage(message);
   }

   {
      // Need to set map type directly, since the return message
      // is slower than the view change when settings are called
      // from the map view.
      // WARNING! The vector map blob is generated in the make_seed.pl file!

      int32 *vectorMapSettings = new int32[vector_map_settings_num];
      LOGNEW(vectorMapSettings, int32[vector_map_settings_num]);
      vectorMapSettings[vmap_set_version_pos] = VECTOR_MAP_SETTINGS_VERSION;
      vectorMapSettings[vmap_set_cache_pos] = iData->iMapCacheSize;
      vectorMapSettings[vmap_set_maptype_pos] = iData->iMapType;
      vectorMapSettings[vmap_set_orientation_pos] = iData->iMapTrackingOrientation;
      vectorMapSettings[vmap_set_favorite_show_pos] = iData->iFavoriteShow;
      vectorMapSettings[vmap_set_guide_mode_pos] = iData->iPreferredGuideMode;
      vectorMapSettings[vmap_set_gui_mode_pos] = 0;

      gen_mess = new (ELeave) GeneralParameterMess(
               GuiProtEnums::paramVectorMapSettings,
               vectorMapSettings,
               vector_map_settings_num );

      SendGuiProtMessage(gen_mess);
   }

   gen_mess = new (ELeave) GeneralParameterMess( GuiProtEnums::paramGPSAutoConnect, (int32)iData->iGPSAutoConnect );
   SendGuiProtMessage(gen_mess);

   if(iData->iGpsSetting == 1/*EGpsNewGps*/){
      delete iData->iGpsName; 
      iData->iGpsName = NULL;
      iData->iGpsName = CEikonEnv::Static()->AllocReadResourceL(R_WAYFINDER_SETTINGSITEM_GPS_NONE);
      iWayFinderUI->ReConnectGps();
      iData->iGpsSetting = 0;
   }

   if ( iData->iTrackingLevel != iData->iOldTrackingLevel ) {
      gen_mess = new (ELeave) GeneralParameterMess( 
         GuiProtEnums::paramTrackingLevel, 
         (int32)iData->iTrackingLevel );
      SendGuiProtMessage(gen_mess);
   }

   if ( iData->iTrackingPIN.Compare( iData->iOldTrackingPIN ) ) {
      // Hack to get at least four numbers in PIN
      serverPort.Copy( iData->iTrackingPIN );
      uint32 addSize = serverPort.Length() < 4 ? 
         4 - serverPort.Length() : 0;
      TBuf<KBuf256Length> zeroBuf;
      zeroBuf.Copy( _L( "0" ) );
      for ( uint32 i = 0 ; i < addSize ; ++i ) {
         serverPort.Insert( 0, zeroBuf );
      }
      // All zero then clear PIN
      bool allZero = true;
      for ( TInt j = 0 ; j < serverPort.Length() && allZero ; ++j ) {
         if ( serverPort[ j ] != '0' ) {
            allZero = false;
         }
      }
      serverPortStr = WFTextUtil::newTDesDupL( serverPort );
      char** s = new(ELeave) char*[ 2 ];
      s[ 0 ] = serverPortStr;
      s[ 1 ] = WFTextUtil::strdupL( "" ); // Comment
//      TRACE_DBG( "New PIN %s", s[ 0 ] );

      if ( iData->m_trackPINList == NULL ) {
         iData->m_trackPINList = new(ELeave) TrackPINList();
      }
      TrackPINList::iterator it = iData->m_trackPINList->begin();
      while ( it != iData->m_trackPINList->end() ) {
//         TRACE_DBG( "Have PIN list size %d", iData->m_trackPINList->size() );
         if ( (*it)->getID() == 0 ) {
            TrackPINList::iterator next = it;
            ++next;
            iData->m_trackPINList->removePIN( (*it) );
            it = next;
         } else {
            iData->m_trackPINList->deletePIN( (*it) );
            ++it;
         }
      }
      if ( !allZero ) {
         iData->m_trackPINList->addPIN( s[ 0 ], s[ 1 ] );
      }

      delete [] s[ 0 ];
      delete [] s[ 1 ];
      delete [] s;

      Buffer buff;
      iData->m_trackPINList->packInto( &buff );
      gen_mess = new (ELeave) GeneralParameterMess( 
         GuiProtEnums::paramTrackingPIN, 
         (uint8*)buff.accessRawData( 0 ), buff.getLength() );
//      TRACE_DBG( "Sending lits size %"PRId32, buff.getLength() );
      buff.releaseData();

      SendGuiProtMessage(gen_mess);
   }

   /** Save Maplib Map layers */
   if ( iData->iTrafficOldEnabled != iData->iTrafficEnabled ||
        iData->iTrafficOldTime != iData->iTrafficTime) {
      /* Setting changed. */
      iWayFinderUI->SaveMapLayerSettings();
   }

   if (iData->iShowCategories) {
      /* Save the disabled poi categories in parameter file. */

      /* Buffer will resize itself if larger than this. */
      Buffer *buf = new Buffer(256);
      LOGNEW(buf, Buffer);

      /* Version number. */
      buf->writeNextUnaligned16bit(VECTOR_MAP_POI_CATEGORY_VERSION);

      for (int32 i = 0; i < iData->iCheckboxArray.Count(); i++) {
         buf->writeNextCharString(iData->iCheckboxTextArray[i]);
         buf->writeNextUnaligned32bit(iData->iCheckboxIdArray[i]);
         buf->writeNext8bit(iData->iCheckboxArray[i]);
      }

      /* Cast necessary to lose warning about const. */
      uint8 *rawData = (uint8 *)buf->accessRawData();
      /* Save the buffer in Nav2. */
      gen_mess = new (ELeave) GeneralParameterMess(
               GuiProtEnums::paramPoiCategories,
               rawData,
               buf->getLength() );

      SendGuiProtMessage(gen_mess);
   }


   /* Save language code in special file. */
   iWayFinderUI->SaveLanguageCode(iData->iLanguage);

   if (iData->iMapType != iData->iOldMapType) {
      WFDialog::ShowScrollingDialogL(iCoeEnv,
            R_WAYFINDER_SETTINGS_CHANGED_HEADER, 
            R_WAYFINDER_SETTINGS_CHANGED_TEXT,
            EFalse);
      iShutdownNeeded = ETrue;
   } else if (iData->iLanguage != iData->iOldLanguage) {
      /* Language changed. */
      WFDialog::ShowScrollingDialogL(iCoeEnv,
            R_WAYFINDER_LANGUAGE_CHANGED_HEADER, 
            R_WAYFINDER_LANGUAGE_CHANGED_TEXT,
            EFalse);
   }

}

void
CSettingsView::ToggleServer()
{
   TBuf<KBuf256Length> serverPort;
   serverPort.Copy( iData->iServer );

   if (serverPort.Find(_L("oss-nav.services.wayfinder.com")) != KErrNotFound) {
      /* Set server string to prod */
      serverPort.Copy(_L("oss-nav.services.wayfinder.com:80:0;oss-nav.services.wayfinder.com:80:0;oss-nav.services.wayfinder.com:80:1;oss-nav.services.wayfinder.com:80:1"));
   } else {
      /* Set server string to head. */
      serverPort.Copy(_L("oss-nav.services.wayfinder.com:80"));
   }
   SetServer(serverPort);
}

// ---------------------------------------------------------
// TUid CSettingsView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid
CSettingsView::Id() const
{
   return KSettingsViewId;
}

// ---------------------------------------------------------
// CSettingsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void
CSettingsView::HandleCommandL(TInt aCommand)
{
   switch ( aCommand )
   {
   case EWayFinderCmdSettingServerToggle:
      {
         ToggleServer();
         break;
      }
   case EAknSoftkeyBack:
      {
         SaveSettings();
         if (iShutdownNeeded) {
            iWayFinderUI->ShutdownNow();
         } else {
            iWayFinderUI->GotoStartViewL();
         }
         break;
      }
   case EWayFinderCmdSettingChange:
      {
         iContainer->ChangeSelectedSetting();
         break;
      }
   case EWayFinderCmdHelp:
      {
         iWayFinderUI->ShowHelpL();
      }
   default:
      {
         CAknView::HandleCommandL( aCommand );
         break;
      }
   }
}

// ---------------------------------------------------------
// CSettingsView::HandleClientRectChange()
// ---------------------------------------------------------
//
void
CSettingsView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CSettingsView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void
CSettingsView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if ( aResourceId == R_WAYFINDER_SETTINGS_MENU ){
      if (iData->showDebug) {
         aMenuPane->SetItemDimmed( EWayFinderCmdSettingServerToggle, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdSettingServerToggle, ETrue );
      }
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CSettingsView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void
CSettingsView::DoActivateL( const TVwsViewId& aPrevViewId,
                                  TUid /*aCustomMessageId*/,
                                  const TDesC8& /*aCustomMessage*/)
{
   iPrevViewId = aPrevViewId;
   class CAknNavigationControlContainer *np = 
      iWayFinderUI->getNavigationControlContainer();
   class TResourceReader reader;
   iCoeEnv->CreateResourceReaderLC(reader, R_SETTINGS_TAB_GROUP);
   iTabsDecorator = np->CreateTabGroupL(reader);
   iTabsDecorator->MakeScrollButtonVisible( ETrue );
   iTabGroup = static_cast<CAknTabGroup*>(iTabsDecorator->DecoratedControl());
   if (iWayFinderUI->IsReleaseVersion() || ! iData->showDebug ) {
      iTabGroup->DeleteTabL(EWayFinderSettingsDebugTab);
   }
   if ( !iWayFinderUI->showTracking() || iWayFinderUI->hasFleetRight() ) {
      iTabGroup->DeleteTabL( EWayFinderSettingsTrackingTab );
   }

   iTabGroup->SetActiveTabByIndex(0);
   CleanupStack::PopAndDestroy();  // resource reader
   np->PushL(*iTabsDecorator);

   if (!iContainer){
      if ( iWayFinderUI->IsTrialVersion() ) {
         iData->isTrialVersion = 1;
      } else {
         iData->isTrialVersion = 0;
      }
      iData->iCanChangeMapType = iWayFinderUI->CanChangeMapType();
      iContainer = new (ELeave) CSettingsContainer(
            iTabGroup,
            iWayFinderUI->IsReleaseVersion(), 
            iWayFinderUI->IsOnlyOneLanguage(),
            ILOG_POINTER);
      LOGNEW(iContainer, CSettingsContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect(), iData );
      AppUi()->AddToStackL( *this, iContainer );
      iContainer->MakeVisible( ETrue );
   }
}

// ---------------------------------------------------------
// CSettingsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void
CSettingsView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
      LOGDEL(iContainer);
      delete iContainer;
   }
   iContainer = NULL;

   CAknNavigationControlContainer * np = iWayFinderUI->getNavigationControlContainer();
   np->Pop(iTabsDecorator);
   delete iTabsDecorator;
}


void CSettingsView::setDirectionType(MC2Direction::RepresentationType type)
{
   iData->iDirectionType = type;
}


void CSettingsView::setFeedOutputFormat(FeedSettings::OutputFormat format)
{
   iData->iFeedOutputFormat = format;
}

void CSettingsView::setVicinityFeedWaitPeriod(int period)
{
   iData->iVicinityFeedRate = period;
}

void CSettingsView::setLockedNavWaitPeriod(int period)
{
   iData->iLockedNavRate = period;
}

void CSettingsView::setGpsMinWalkingSpeed(int minSpeed)
{
   iData->iWalkingSpeedCutOff = minSpeed;
}

void CSettingsView::setVicinityListUpdatePeriod(int period)
{
   iData->iVicinityListPeriod = period;
}

// End of File


