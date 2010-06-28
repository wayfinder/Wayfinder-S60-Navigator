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
#include <stringloader.h>
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
#define ILOG_POINTER iWayfinderAppUi->iLog
#include "memlog.h"

#include "DistancePrintingPolicy.h"
#include "TrackPIN.h"
#include "TDesTrackPIN.h"
#include "TraceMacros.h"

#include "IAPFileHandler.h"
#include "PathFinder.h"
#include "WFDataHolder.h"
#include "WFAccountData.h"

using namespace isab;

// ================= MEMBER FUNCTIONS =======================

CSettingsView::CSettingsView(CWayFinderAppUi* aUi) : 
   CViewBase(aUi)
{
}

// ---------------------------------------------------------
// CSettingsView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void
CSettingsView::ConstructL()
{
   iTabsDecorator = NULL;
/*    iData = CSettingsData::NewL(ILOG_POINTER); */
   iData = NULL;
   iShutdownNeeded = EFalse;
   BaseConstructL( R_WAYFINDER_SETTINGS_VIEW );
}

CSettingsView*
CSettingsView::NewLC(CWayFinderAppUi* aUi, isab::Log* /*aLog*/)
{
   CSettingsView* self = new (ELeave) CSettingsView(aUi);
   CleanupStack::PushL(self);
   self->ConstructL();
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
CSettingsView::InitData()
{
   WFTextUtil::char2TDes(iData->iWebUsername,
         iData->iDataStore->iWFAccountData->getWebUserName() );
   WFTextUtil::char2TDes(iData->iWebPassword,
         iData->iDataStore->iWFAccountData->getWebPassword() );

   // As we always run with the traffic update timer turned off
   // We turn it off in m_trafficUpdate
   iData->m_trafficUpdate = 0;
   // Since we need to take care of the possibility of
   // pointing outside the resource array on old clients,
   // they had the possibility to set this setting to a number
   // of different values. Now, user can only choose between
   // on or off. This means we need to send down 0 or 
   // live (5) since Nav2 is checking the parameter and if
   // having this parameter to 3 and not sending down 5
   // user will get some strange logging interval.
   iData->m_trackingLevelOld = iData->m_trackingLevel;
   if (iData->m_trackingLevel != 0) {
      iData->m_trackingLevel = 1;
   }
   
   // The setting: fastest is no longer allowed so we change it to traffic
   if(iData->m_routeCostType == 1) {
      iData->m_routeCostType = 2;
   }
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
CSettingsView::SetDistanceMode( DistancePrintingPolicy::DistanceMode aMode )
{
   if( aMode == 0 || aMode == DistancePrintingPolicy::ModeInvalid ) {
      iWayfinderAppUi->SetDistanceMode(DistancePrintingPolicy::ModeMetric);
   } else {
      iWayfinderAppUi->SetDistanceMode(aMode);
   }
}

void
CSettingsView::SetUseSpeaker( TBool aUseSpeaker )
{
   iData->iUseSpeaker = aUseSpeaker;
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
   iData->m_language = language;
   iData->m_languageOld = language;
}

void CSettingsView::SetMapCacheSize( TInt aSize )
{
   iData->iMapCacheSize = aSize;
}

void CSettingsView::SetPreferredGuideMode( enum preferred_guide_mode aGuideMode )
{
   iData->iPreferredGuideMode = aGuideMode;
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
CSettingsView::SendGuiProtMessage(GuiProtMess* mess)
{
   iWayfinderAppUi->SendMessageL( mess);
   mess->deleteMembers();
   delete mess;
}

void
CSettingsView::SendGeneralParam(int32 paramId, int32 value)
{
   GeneralParameterMess* gen_mess;
   gen_mess = new (ELeave) GeneralParameterMess( paramId, value );
   SendGuiProtMessage(gen_mess);
}

void
CSettingsView::SendVectorMapSettings()
{
   SendVectorMapSettings(iData->iMapCacheSize, 0,
                         iData->iMapTrackingOrientation, 
                         iData->iPreferredGuideMode);
}

void
CSettingsView::SendVectorMapSettings(int32 cacheSize,
      int32 mapType,
      int32 trackingOrientation,
      int32 preferredGuideMode)
{
   // WARNING! The vector map blob is generated in the make_seed.pl file!

   int32 *vectorMapSettings = new int32[vector_map_settings_num];
   LOGNEW(vectorMapSettings, int32[vector_map_settings_num]);
   vectorMapSettings[vmap_set_version_pos] = VECTOR_MAP_SETTINGS_VERSION;
   vectorMapSettings[vmap_set_cache_pos] = cacheSize;
   vectorMapSettings[vmap_set_maptype_pos] = mapType;
   vectorMapSettings[vmap_set_orientation_pos] = trackingOrientation;   
   vectorMapSettings[vmap_set_favorite_show_pos] = 0;
   vectorMapSettings[vmap_set_guide_mode_pos] = preferredGuideMode;
   vectorMapSettings[vmap_set_gui_mode_pos] = 0;

   GeneralParameterMess* gen_mess = new (ELeave) GeneralParameterMess(
         GuiProtEnums::paramVectorMapSettings,
         vectorMapSettings,
         vector_map_settings_num );

   /* Will delete vectorMapSettings */
   SendGuiProtMessage(gen_mess);
}


void
CSettingsView::SaveSettings()
{
   iContainer->StoreSettingsL();

   SendGeneralParam( GuiProtEnums::paramSoundVolume, (iData->iVolume*10));
   SendGeneralParam( GuiProtEnums::paramUseSpeaker, iData->iUseSpeaker );
   SendGeneralParam( GuiProtEnums::paramTurnSoundsLevel, iData->m_turnSoundsLevel);
   SendGeneralParam( GuiProtEnums::paramAutoReroute, iData->m_autoReroute );
   SendGeneralParam( GuiProtEnums::paramDistanceMode, iData->m_distanceMode );
   SendGeneralParam( GuiProtEnums::paramTollRoads, iData->m_routeTollRoads );
   SendGeneralParam( GuiProtEnums::paramHighways, iData->m_routeHighways );
   SendGeneralParam( GuiProtEnums::paramTimeDist, iData->m_routeCostType );
   SendGeneralParam( GuiProtEnums::paramBacklightStrategy, iData->m_backlightStrategy );
   SendGeneralParam( GuiProtEnums::paramMapACPSetting, iData->m_acpSettingEnabled);
   SendGeneralParam( GuiProtEnums::paramCheckForUpdates, iData->iCheckForUpdates);

   // These are new default values for settings that we've removed from gui.
   //SendGeneralParam( GuiProtEnums::paramTransportationType, iData->m_transportationType );
   //SendGeneralParam( GuiProtEnums::paramAutoTracking, iData->m_autoTracking );
   SendGeneralParam( GuiProtEnums::paramAutoTracking, 1 ); // always on
   //SendGeneralParam( GuiProtEnums::paramStoreSMSDestInMyDest, iData->m_storeSMSDestInFavorites );
   SendGeneralParam( GuiProtEnums::paramStoreSMSDestInMyDest, GuiProtEnums::ask );
   //SendGeneralParam( GuiProtEnums::paramAutomaticRouteOnSMSDest, iData->m_autoRouteOnSMSDest );
   SendGeneralParam( GuiProtEnums::paramAutomaticRouteOnSMSDest, GuiProtEnums::no );
   //SendGeneralParam( GuiProtEnums::paramKeepSMSDestInInbox, iData->m_keepSMSDestInInbox );
   SendGeneralParam( GuiProtEnums::paramKeepSMSDestInInbox, GuiProtEnums::yes);
   //SendGeneralParam( GuiProtEnums::paramFavoriteShow, iData->m_favoriteShowInMap );
   SendGeneralParam( GuiProtEnums::paramFavoriteShow, 1 ); // city zoom level

   if ( iData->m_trackingLevel != iData->m_trackingLevelOld ) {
      if (iData->m_trackingLevel > 0) {
         // Need to be sure that we don't point outside the resource array.
         // paramTrackingLevel could have a lot more values that 0 and 1,
         // in the ui the only possible values should be on or off.
         iData->m_trackingLevel = GuiProtEnums::tracking_level_live;
      }
      SendGeneralParam( GuiProtEnums::paramTrackingLevel, iData->m_trackingLevel );
   }

   if (iData->iOldIAP != iData->iIAP) {
      //User selected new iap so delete iap.txt file.
      iWayfinderAppUi->DeleteIapTxt();

      iWayfinderAppUi->SettingsSetIAP(iData->iIAP);
      SendGeneralParam( GuiProtEnums::paramSelectedAccessPointId, iData->iIAP );
      SendGeneralParam( GuiProtEnums::paramSelectedAccessPointId2, iData->iIAP );
   }

   SendGeneralParam( GuiProtEnums::paramLinkLayerKeepAlive, iData->m_linkLayerKeepAlive);

   char* tmp_str;
   GeneralParameterMess* gen_mess;


   if (iData->iServer.Length() > 0 &&
       iData->iServer.Compare(iData->iServerOld)) {
      iWayfinderAppUi->handleSendServerNameAndPort(iData->iServer);
   }

   SetDistanceMode(DistancePrintingPolicy::DistanceMode(iData->m_distanceMode));

   {
      HBufC* buf = CCoeEnv::Static()->AllocReadResourceLC(R_WAYFINDER_SET_PASSWORD_TEXT );
      if (iData->iWebPassword.Length() > 0 &&
          iData->iWebPassword.Compare(*buf)) {
         tmp_str = WFTextUtil::newTDesDupL(iData->iWebPassword);
         gen_mess = new (ELeave) GeneralParameterMess(
               GuiProtEnums::paramWebPassword, tmp_str);
         SendGuiProtMessage(gen_mess);
      } else {
         /* Web password string was "Set Password"... */
      }
      CleanupStack::PopAndDestroy(buf);
   }


   if (iData->iUsername.Compare(iData->iOldUsername) ||
       iData->iPassword.Compare(iData->iOldPassword)) {
      char **a = new (ELeave) char*[2];
      a[0] = WFTextUtil::newTDesDupL(iData->iUsername);
      a[1] = WFTextUtil::newTDesDupL(iData->iPassword);
      gen_mess = new (ELeave) GeneralParameterMess(
            GuiProtEnums::paramUserAndPassword, a, 2);
      SendGuiProtMessage(gen_mess);
   }

   SendVectorMapSettings(10, /*iData->iMapCacheSize*/ 0, /* iData->iMapType */
         iData->iMapTrackingOrientation, iData->iPreferredGuideMode);

   if(iData->iGpsSetting == 1/*EGpsNewGps*/){
      delete iData->iGpsName; 
      iData->iGpsName = NULL;
      iData->iGpsName = CEikonEnv::Static()->AllocReadResourceL(R_WAYFINDER_SETTINGSITEM_GPS_NONE);
      iWayfinderAppUi->ReConnectGps();
      iData->iGpsSetting = 0;
   }

   TBuf<KBuf256Length> serverPort;

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
      tmp_str = WFTextUtil::newTDesDupL( serverPort );
      char** s = new(ELeave) char*[ 2 ];
      s[ 0 ] = tmp_str;
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
   if ( iData->m_trafficUpdate != iData->m_trafficOldUpdate ||
        iData->m_trafficUpdatePeriod != iData->m_trafficOldUpdatePeriod) {
      // Setting changed, this setting variable now controls both the 
      // userTrafficUpdatePeriod and paramMapLayerSettings parameters.

      // Save the parameter and maplayer settings for traffic info in map.
      iWayfinderAppUi->SaveMapLayerSettings();

      // Below is the functionality to update the route traffic info 
      // parameter as well. This one has the highest bit as a flag to say if 
      // the setting is enabled or not.
      TInt updTime = iData->m_trafficUpdatePeriod;
      if (!iData->m_trafficUpdate) {
         // Let nav2 know that user has switched this option off.
         // Set the second highest bit, which is used as a flag.
         updTime |= 0x40000000;
      }
      SendGeneralParam(GuiProtEnums::userTrafficUpdatePeriod, updTime);
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

#ifndef ONLY_PHONE_LANG_ROM_BUILD
   /* Save language code in special file. */
   iWayfinderAppUi->SaveLanguageCode(iData->m_language);
   
   if (iData->m_language != iData->m_languageOld) {
      /* Language changed. */
      WFDialog::ShowScrollingDialogL(iCoeEnv,
            R_WAYFINDER_LANGUAGE_CHANGED_HEADER, 
            R_WAYFINDER_LANGUAGE_CHANGED_TEXT,
            EFalse);
   }
#endif

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
            iWayfinderAppUi->ShutdownNow();
         } else {
            iWayfinderAppUi->pop();
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
         iWayfinderAppUi->ShowHelpL();
         break;
      }
   case EAknSoftkeyExit:
      iWayfinderAppUi->HandleCommandL(aCommand);
      break;
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
                                  TUid aCustomMessageId,
                                  const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_SETTINGS_TEXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   class CAknNavigationControlContainer *np = 
      iWayfinderAppUi->getNavigationControlContainer();
   class TResourceReader reader;
   iCoeEnv->CreateResourceReaderLC(reader, R_SETTINGS_TAB_GROUP);
   iTabsDecorator = np->ConstructNavigationDecoratorFromResourceL(reader);
   iTabsDecorator->MakeScrollButtonVisible( ETrue );
   iTabGroup = static_cast<CAknTabGroup*>(iTabsDecorator->DecoratedControl());
   if (iWayfinderAppUi->IsReleaseVersion() || !iData->showDebug) {
      iTabGroup->DeleteTabL(EWayFinderSettingsDebugTab);
   }
   iTabGroup->SetActiveTabByIndex(0);
   CleanupStack::PopAndDestroy();  // resource reader
   np->PushL(*iTabsDecorator);

   if (!iContainer){
      if ( iWayfinderAppUi->IsTrialVersion() ) {
         iData->isTrialVersion = 1;
      } else {
         iData->isTrialVersion = 0;
      }
/*       iData->iCanChangeMapType = iWayfinderAppUi->CanChangeMapType(); */
      InitData();
      iContainer = new (ELeave) CSettingsContainer(
            iTabGroup,
            iWayfinderAppUi->IsReleaseVersion(), 
            iWayfinderAppUi->IsOnlyOneLanguage(),
            ILOG_POINTER);
      LOGNEW(iContainer, CSettingsContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect(), iData );
      AppUi()->AddToStackL( *this, iContainer );
      iContainer->MakeVisible( ETrue );
   } else {
      iContainer->SetTabGroup(iTabGroup);
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

   CAknNavigationControlContainer * np = iWayfinderAppUi->getNavigationControlContainer();
   np->Pop(iTabsDecorator);
   delete iTabsDecorator;
}

// End of File

