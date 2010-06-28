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
#include "WayFinderAppUi.h"
#include <avkon.hrh>
#include <eikmenup.h>
#include <aknnotewrappers.h>
#include <aknstaticnotedialog.h>
#include <aknmessagequerydialog.h>
#include <akncontext.h>
#include <eiklabel.h>  // for labels
#include <aknutils.h>  // for fonts
#include <barsread.h>  // for resource reader
//#include <etbscbuf.h>
#include <etel.h>
#include <sysutil.h>
#include "DetailFetcher.h"
#include <aknkeylock.h> // For RAknKeyLock
#include "VicinityFeedView.h"
#include "HotKeys.h"
#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <aknlists.h>
#include <mtclreg.h>
//#include "CommunicationWizard.h"

#include <bttypes.h>
#include <utf.h>

#include <deque>

#ifndef NAV2_CLIENT_SERIES60_V3
#include <hal.h>
#endif
#include <e32math.h> 

/* For RApaLsSession */
#include <apgcli.h>


#include "AnimatorFrame.h"
#include "ContextPaneAnimator.h"

#include "nav2util.h"
#include "arch.h"
#include "WayFinderConstants.h" 
#ifdef RELEASE_CELL
# ifndef NAV2_CLIENT_SERIES60_V2
#  include <etelbgsm.h>
# else
#  include "my_etelbgsm.h"
# endif
# include "CellMapper.h"
#endif
#include "WayFinderApp.h"

#include "Buffer.h"
/* #include "GetIMEI.h" */

#include "GuiProt/ServerEnums.h"
#include "GuiProt/ServerEnumHelper.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtRouteMess.h"
#include "GuiProt/SearchArea.h"
#include "GuiProt/SearchRegion.h"
#include "GuiProt/SearchItem.h"
/* #include "GuiProt/AdditionalInfo.h" */
/* #include "GuiProt/FullSearchItem.h" */
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiProtMapMess.h"
#include "GuiProt/SearchPrintingPolicyFactory.h"
/* #include "GuiProt/SearchPrintingPolicy.h" */
/* #include "GuiProt/SearchPrintingPolicySwedish.h" */
#include "GuiProt/VectorMapCoordinates.h"
#include "GuiProt/GuiParameterEnums.h"

#include "TopRegion.h"
#include "TDesTrackPIN.h"
#include "MapResourceFactory.h"
#include "IAPSearchGui_old.h"
#include "SplashView.h"
#include "PositionSelectView.h"
#include "StartPageView.h"
#include "WelcomeView.h"
#include "NewDestView.h"
#include "MyDestView.h"
#include "ContactsView.h"
#include "GuideView.h"
#include "RouteView.h"
#include "MapView.h"
#include "InfoView.h"
#include "ItineraryView.h"
#include "VicinityView.h"
#include "MessageHandler.h"
#include "BufferArray.h"
#include "DataHolder.h"
#include "SearchHistory.h"
#include "NewSlaveAudio.h"
#include "OldSlaveAudio.h"
#include "DummySlaveAudio.h"
#include "TimeOutTimer.h"
#include "SettingsView.h"
#include "SettingsData.h"
#include "CallObserver.h"
#include "ConnectDialog.h"
#include "CommunicateHelper.h"
/* #include "UpgradeDialog.h" */
/* #include "ConnectSendAsObserver.h" */
#include "GpsSocket.h"
#include "QueryDialog.h"

#include "LockedNavigationView.h"

#include "S60StartupHandler.h"
#include "CallBackDialog.h"

#include "VicinityItem.h"

/* #include "Item.h" */

#include "buildid.h"
#include "WayFinderSettings.h"
#include "CleanupSupport.h"
#include "WFSymbianUtil.h"
#include "WFTextUtil.h"
#include "WFArrayUtil.h"
#include "WFS60Util.h"
#include "LaunchWapBrowserUtil.h"
/* #include "WayfinderSMSMessage.h" */

/* #include "Nav2UiProtHandler.h" */

#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
/* For RConnection (connection manager) */
#include <es_sock.h>
#include <commdbconnpref.h>
#include <apengineconsts.h>
#endif

#include "TopRegionHack.h"

#include "Nav2.h"
#include "Nav2ErrorSymbian.h"

#include "NewSyntax.h"
#include "AudioCtrlLanguage.h"
#include "AudioClipTable.h"
#include "AudioCtrlNone.h"
/* #include "wayfinder.loc" */

#include "NokiaLbs.h"
#include "Dialogs.h"
#include "IAPSearcher_old.h"
#include "PopUpList.h"
#include "ActivationSms.h"
#include "A1DebugSms.h"

#include "HelpUrlHandler.h"
#include "WayfinderServices.hlp.hrh"

#include "NetworkInfoHandler.h"

#include "GuiDataStore.h"

#include "GuidePicture.h"

#include "MoveWFLauncher.h"

#include "LogMem.h"
#include "TileMapUtil.h"

#include "GuiProt/GuiProtFileMess.h"
#include "GuiFileOperation.h"
#include "S60NavTaskGuiHandler.h"

#include "CallNumber.h"
#include "SpreadWFDialogs.h"
#include "PathFinder.h"
#include "IAPFileHandler.h"
#include "IAPObject.h"
#include "SimpleConnectionManager.h"
#include "DiscoveryUtils.h"

#include "ServiceWindowView.h"
#include "WayfinderCommandHandler.h"
#include "WfCmdCallbackHandler.h"
#include "IniFile.h"
#include "MC2Coordinate.h"
#include "ImageHandler.h"

#include "SmsParser.h"

#if defined NAV2_CLIENT_SERIES60_V3
#include "SmsHandlerFactory.h"
#include "Symbian9CallHandler.h"
#endif

#include "DiscoveryUtils.h"
#include "CleanupSupport.h"
#include "WFLayoutUtils.h"

#include "WFStartupEvent.h"
#include "WFStartupHandler.h"
#include "WFCommonStartupEvents.h"
#include "GuiProt/GuiProtMessageHandler.h"
#include "IAPDataStore.h"
#include "WFAccountData.h"
#include "WFNewsData.h"
#include "WFGpsData.h"
#include "GuiProt/UserTermsEnums.h"
#include "ProgressDlg.h"
#include "WFDataHolder.h"
#include "WFCmdCallbackStartup.h"
#include "WFStartupEventCallbackParam.h"
#include "WFStartupCommonBlocks.h"

#include "StringUtility.h"
#include "ErrorFilter.h"
#include "TDesCHolder.h"
#include "OfflineTester.h"

#include "TileMapControl.h"         // CTileMapControl
#include "MapFeatureHolder.h"       // CMapFeatureHolder
#include "MapInfoControl.h"         // CMapInfoControl
#include "VectorMapConnection.h"    // CVectorMapConnection

#include "FavoriteFetcher.h"
#include "FormatDirection.h"
#include "DirectionHelper.h"

// enables or disables execution of all(?!) cell related code that is in a release state, 
// and controls(should control) which is put into the executable.
// #define RELEASE_CELL
#ifdef RELEASE_CELL
# define RELEASE_CELL_BLOCK(a) a

// enables/disables cell related code that is still in a debug state (currently most of it).
// # define DEBUG_CELL
# ifdef DEBUG_CELL
#  define DEBUG_CELL_BLOCK(a) a
// allows stand alone phone operation (no (or fewer) popup dialogs (e.g., errors))
#  define DEBUG_CELL_AUTONOMOUS
# else
#  define DEBUG_CELL_BLOCK(a)
# endif 
#else
# define RELEASE_CELL_BLOCK(a)
# define DEBUG_CELL_BLOCK(a)
#endif
 

#include "TraceMacros.h"

#define WAYFINDER_RESOURCE_VERSION_FILE "resource_version.short"
#define WAYFINDER_APPLICATION_VERSION_FILE "version.txt"

// How often the device availabel memory is checked
#define WAYFINDER_MEMORY_CHECK_INTERVAL 2

//these literal declarations are probably more complicated than they
//need to be.The MLIT() macro is just an alias for _LIT. This is
//needed for macro strings to be expanded _before_ the _LIT macro. The
//ML macro adds an 'L' before the string it holds. This is necessary
//for proper string concatenation.
MLIT(KaudioFormatPrefix, WAYFINDER_AUDIO_FORMAT_PREFIX);
MLIT(KAudioDirName, WAYFINDER_AUDIO_DIR_NAME);
MLIT(KWayfinderResourceVersion, WAYFINDER_RESOURCE_VERSION_FILE);

_LIT8(KwfType, NAV2_CLIENT_TYPE_STRING);
_LIT8(KwfOpts, NAV2_CLIENT_OPTIONS_STRING);

MLIT(KWayfinderServiceWindowBuyExtensionURL, WAYFINDER_BUY_EXTENSION_SERVICE_WINDOW_URL);
MLIT(KWayfinderBuyExtensionBaseURL, WAYFINDER_BUY_EXTENSION_BASEURL);
MLIT(KWayfinderExternalBrowserBaseURL, WAYFINDER_EXTERNAL_BROWSER_BASEURL);
MLIT(KWayfinderServiceWindowBaseURL, WAYFINDER_SERVICE_WINDOW_BASEURL);
MLIT(KWayfinderServiceWindowShowInfoURL, WAYFINDER_SERVICE_WINDOW_SHOWINFO_URL);

MLIT(KResourceBaseUrl, RESOURCE_BASE_URL);

MLIT(KApplicationVisibleName, APPLICATION_VISIBLE_NAME);
MLIT(KApplicationPathName, APPLICATION_BASE_DIR_NAME);

MLIT(KAllowedLangs, ALLOWED_LANGS);
MLIT(KDebugSmsNumber, SEND_DEBUG_SMS_NUMBER);
MLIT(KShareWFSmsUrl, SHARE_WF_SMS_URL);

static const TInt KNewsBackgroundRed = NEWS_BACKGROUND_RED;
static const TInt KNewsBackgroundGreen = NEWS_BACKGROUND_GREEN;
static const TInt KNewsBackgroundBlue = NEWS_BACKGROUND_BLUE;

static const TInt GPS_NO_POSITIONS_TIMEOUT = 32; //In gps status updates

_LIT(KWF, "wf");
_LIT(KWTAI, "wtai");
_LIT(KCallto, "callto");
_LIT(KFavorite, "favorite");
_LIT(KMainMenu, "mainmenu");
_LIT(KMapView, "mapview");
_LIT(KRoute, "route");
_LIT(KWP, "wp");
_LIT(KStartup, "startup");

#define KInitialBufferLength 128

static const TInt KRefresh = 125000; // microseconds time-out
/* static const TInt KStartUpWait = 200000; // 0.2 seconds delay */

#include "memlog.h"

using namespace isab::RouteEnums;
using namespace isab;

TBool CWayFinderAppUi::CheckMem(TInt minAmount)
{
   TInt memory = 0;
#ifndef NAV2_CLIENT_SERIES60_V3
   HAL::Get(HALData::EMemoryRAMFree, memory);
   TInt largestBlock;
   memory = User::Available(largestBlock)*1024;
   return memory > minAmount;
#else
   memory = memory;
   minAmount = minAmount;
   return ETrue;
#endif
}

CWayFinderAppUi::CWayFinderAppUi(isab::Log* aLog) :
   WayfinderSymbianUiBase(),
   iCurrentWpId(MAX_UINT32),
   iLastRouteStatus(OnTrack),
   iGpsQuality(QualityMissing),
   iMessageCounter(1), //all messages from GUI are odd :)
   iCurrPos(MAX_INT32, MAX_INT32),
   iCurrentDestination(MAX_INT32, MAX_INT32),
   iCurrentOrigin(MAX_INT32, MAX_INT32),
   iSaveSmsDestination(GuiProtEnums::ask),
   iAutoRouteSmsDestination(ETrue),
   iKeepSmsDestination(GuiProtEnums::ask),
   iBacklightStrategy(GuiProtEnums::backlight_on_during_route),
   iRouteBoxTl( MAX_INT32, MAX_INT32),
   iRouteBoxBr( MAX_INT32, MAX_INT32),
   iLog(aLog),
   m_fsSession(),
   iForeGround(ETrue),
   m_languageCode(-1),
   iMemCheckInterval( WAYFINDER_MEMORY_CHECK_INTERVAL ),
   iUpgradeFromView(),
   m_defaultServerMode(GuiProtEnums::InvalidServer),
   iShowNowGpsWarn(ETrue)
{
   iWfTypeStr = HBufC::NewL(KwfType().Length());
   iWfTypeStr->Des().Copy(KwfType());

   iNotifyGpsUseless = EFalse;
   iNotifyGpsCounter = GPS_NO_POSITIONS_TIMEOUT;
   
   m_favoriteFetcher = new FavoriteFetcher();
   m_favoriteFetcher->SetMessageSender(this);
   m_detailFetcher = new DetailFetcher();
   m_detailFetcher->SetMessageSender(this);
   
   iWfOptsStr = HBufC::NewL(KwfOpts().Length());
   iWfOptsStr->Des().Copy(KwfOpts());

   iIniFile = new IniFile(m_fsSession, iIniFilePath);
#if defined NAV2_CLIENT_SERIES60_V3
   iImeiReceived = EFalse;
#endif
   iErrorFilter = new ErrorFilter(MIN_TIME_BETWEEN_SHOWN_ERRORS,
                                  ERROR_MAX_REPEAT_TIME);
   iOfflineTester = new OfflineTester();
   // Initialize to other than NULL as GetCurrentRouteDestinationName 
   // returns reference to it.
   iCurrentDestinationName = WFTextUtil::AllocL( "" );
   
   SetupHotkeys();
}

void
CWayFinderAppUi::InitPathsL()
{
   TBuf<KBuf32Length> languageCode;
   iCoeEnv->ReadResource( languageCode, R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE );
   languageCode.LowerCase();
   if (m_languageIsoTwoChar) {
      delete[] m_languageIsoTwoChar;
   }
   languageCode.LowerCase();
   m_languageIsoTwoChar = WFTextUtil::newTDesDupL(languageCode);

   //const TDesC& audioFormatPrefix = KaudioFormatPrefix;

   iPathManager = 
      CPathFinder::NewL(this, languageCode, KaudioFormatPrefix, KNullDesC, KAudioDirName);

   // Temporary solution for the ini-file.
   iIniFilePath.Copy(iPathManager->GetDataPath());
}

void
CWayFinderAppUi::PictureError( TInt /*aError*/ )
{
   FetchImei();
}

void
CWayFinderAppUi::ScalingDone()
{
   iWelcomeView->SetImage(iStartupImage);
   FetchImei();
}

void
CWayFinderAppUi::DoStartProgram(TWfStartupType aStartupType)
{
   DYNAMIC_STACKINFO;

   /* Read in cool picture. */
   {
      iStartupImage = new (ELeave) CGuidePicture( iLog );
/*       iStartupImage->SetPriority( CActive::EPriorityHigh ); */
      LOGNEW(iStartupImage, CGuidePicture);
      TBuf<KBuf256Length> fullName;
      fullName.Copy(iPathManager->GetWayfinderPath());
# ifndef NAV2_CLIENT_SERIES60_V3
      fullName.Append( KGifNews );
#  define MAX_WIDTH                176
#  define MAX_HEIGHT               188

      iStartupImage->ConstructL(TRect( TPoint(0,0), TSize( MAX_WIDTH, MAX_HEIGHT ) ), this );
      iStartupImage->SetClear( EFalse );
      iStartupImage->SetShow( EFalse );
      iStartupImage->OpenGif( fullName, EFalse );
# else 
      fullName.Append( KSvgNews );
      TRACE_FUNC();
      TRect rect = WFLayoutUtils::GetFullScreenRect();
      iImageHandler = CImageHandler::NewL(rect,
                                          CEikonEnv::Static()->FsSession(),
                                          this);
      iImageHandler->SetClear(EFalse);
      iImageHandler->SetShow(EFalse);
      TSize size;
      if (WFLayoutUtils::LandscapeMode()) {
         // Phone is in landscape mode. Since we want to
         // use a image that has the same height as width,
         // we use the height as optimal size.
         TSize tmp = WFLayoutUtils::GetMainPaneSize();
         size = TSize(tmp.iHeight, tmp.iHeight);
      } else {
         // Phone is in portrait mode, lets use the width
         // as optimal size.
         TSize tmp = WFLayoutUtils::GetMainPaneSize();
         size = TSize(tmp.iWidth, tmp.iWidth);
      }
      iImageHandler->LoadMifImage(fullName, size);
//       iImageHandler->LoadFileL(fullName, 
//                                TSize(MAX_WIDTH, MAX_HEIGHT), 
//                                EFalse, 0);
# endif
   }

   // Show tabs for main views from resources
   CEikStatusPane* sp = StatusPane();

   // Fetch pointer to the default navi pane control
   iNaviPane = (CAknNavigationControlContainer*)sp->ControlL(
         TUid::Uid(EEikStatusPaneUidNavi) );

   // Tabgroup has been read from resource and it were pushed to the
   // navi pane.  Get pointer to the navigation decorator with the
   // ResourceDecorator() function.  Application owns the decorator
   // and it has responsibility to delete the object.
   iDecoratedTabGroup = iNaviPane->ResourceDecorator();
   if( iDecoratedTabGroup ){
      LOGNEW(iDecoratedTabGroup, CAknNavigationDecorator);
      iTabGroup = (CAknTabGroup*)iDecoratedTabGroup->DecoratedControl();
      iDecoratedTabGroup->MakeScrollButtonVisible( EFalse );
   }

   iContextPane = (CAknContextPane *)sp->ControlL(TUid::Uid(EEikStatusPaneUidContext));

   // ==== Welcome
   AddViewToFrameWorkL(iWelcomeView);
   LOGNEW(iWelcomeView, CWelcomeView);
#if defined NAV2_CLIENT_SERIES60_V3
   iWelcomeView->SetImage(iImageHandler);
#else
   iWelcomeView->SetImage(iStartupImage);
#endif
   SetDefaultViewL(*iWelcomeView);
   iTabGroup->SetActiveTabById( KWelcomeViewId.iUid );

   SetupStartup();

   iNTCommandHandler = CS60NavTaskGuiCommandHandler::NewL(this);
}

void
CWayFinderAppUi::SetupStartup()
{
   iGpsAutoConnectAfterStartup = false;
   iStartupCompleted = false;
   
   /** Initialize the GuiProt handler. */
   iGuiProtHandler = new (ELeave) GuiProtMessageHandler(this);

   /* Creates m_startupHandler */
   WayfinderSymbianUiBase::ConstructL();

   /* Create data store. */
   iDataStore = new (ELeave) CWayfinderDataHolder();
   iDataStore->iWFNewsData = new (ELeave) WFNewsData();
   iDataStore->iWFAccountData = new (ELeave) WFAccountData();
   iDataStore->iWFCallbackStartup =
      CWFCmdCallbackStartup::NewL(m_startupHandler);
   iDataStore->iWFGpsData = new (ELeave) WFGpsData();

   /* Add event for exiting. */
//    m_startupHandler->RegisterStartupEvent(SS_Exit,
//          new WFStartupUiCallback());

//    m_startupHandler->RegisterStartupEvent(SS_Start,
//          new WFStartupNextEvent(SS_CheckFlightMode));

//    m_startupHandler->RegisterStartupEvent(SS_CheckFlightMode,
//          new (ELeave) WFStartupUiCallback());

//    m_startupHandler->RegisterStartupEvent(SS_CheckFlightModeOk,
//          new WFStartupNextEvent(SS_GetWFType));

//    /* Add event for getting the WFtype on startup. */
//    m_startupHandler->RegisterStartupEvent(SS_GetWFType,
//          new WFStartupGetWfType(iGuiProtHandler,
//             SS_WFTypeReceived) );

   /* Create the iIAPDataStore, holds data on current IAP. */
   iDataStore->iIAPDataStore = new (ELeave) IAPDataStore(-1, -1);
   WFStartupCommonBlocks::SetupStartup(m_startupHandler,
                                       this,
                                       iGuiProtHandler,
                                       iDataStore,
                                       iDataStore->iWFNewsData,
                                       iDataStore->iWFAccountData,
                                       iDataStore->iIAPDataStore,
                                       m_languageIsoTwoChar,
                                       WFTextUtil::newTDesDupL(iPathManager->GetLangSyntaxPath()));

//    /* Unnecessary, but keep until startup work is done. */
//    m_startupHandler->RegisterStartupEvent(SS_WFTypeReceived,
//          new WFStartupNextEvent(SS_GetIap));

//    WFStartupCommonBlocks::IAPSearch(m_startupHandler,
//          SS_IapOk, m_languageIsoTwoChar,
//          this,
//          iGuiProtHandler,
//          iDataStore->iIAPDataStore);

//    std::deque<int32> params;
//    params.push_back(SS_SendFavoriteSync);
//    params.push_back(SS_AudioScriptsInit);

//    m_startupHandler->RegisterStartupEvent(SS_IapOk,
//          new WFStartupNextEvent(params));

//    /* Event for sending audio scripts initialization message. */
//    m_startupHandler->RegisterStartupEvent(SS_AudioScriptsInit,
//          new WFStartupAudioScripts(iGuiProtHandler,
//             WFTextUtil::newTDesDupL(iPathManager->GetLangSyntaxPath())));

//    /* Send favorite synchronization message. */
//    m_startupHandler->RegisterStartupEvent(SS_SendFavoriteSync,
//          new WFStartupFavoriteSync(iGuiProtHandler,
//             SS_FavoriteSyncReceived));

//    m_startupHandler->RegisterStartupEvent(SS_StartupFailed,
//          new (ELeave) WFStartupUiCallback());

//    m_startupHandler->RegisterStartupEvent(SS_StartupError,
//          new (ELeave) WFStartupUiCallback());

//    m_startupHandler->RegisterStartupEvent(SS_FavoriteSyncReceived,
//          new WFStartupNextEvent(SS_GetUsername));

//    m_startupHandler->RegisterStartupEvent(SS_GetUsername,
//          new WFStartupSendUsername(iGuiProtHandler, 
//                                    iDataStore->iWFAccountData,
//                                    SS_UsernameReceived));

//    m_startupHandler->RegisterStartupEvent(SS_UsernameReceived,
//          new WFStartupNextEvent(SS_UserTerms));

//    WFStartupCommonBlocks::UserTerms(m_startupHandler,
//          SS_UserTermsDone, m_languageIsoTwoChar, this, iGuiProtHandler);

//    /* Unnecessary, but keep until startup work is done. */
//    m_startupHandler->RegisterStartupEvent(SS_UserTermsDone,
//          new WFStartupNextEvent(SS_ExpiredTest));

//    WFStartupCommonBlocks::ExpiredTest(m_startupHandler,
//          SS_ModeTest, m_languageIsoTwoChar, this);

//    WFStartupCommonBlocks::WFMode(m_startupHandler,
// /*          SS_EarthTest, m_languageIsoTwoChar, this); */
// /*    WFStartupCommonBlocks::EarthTest(m_startupHandler, */
//          SS_USDisclaimer, m_languageIsoTwoChar, this);

//    WFStartupCommonBlocks::UsDisclaimer(m_startupHandler,
//          SS_ShowNewsTest, m_languageIsoTwoChar, this);

//    WFStartupCommonBlocks::ShowNews(m_startupHandler,
//          SS_GotoMainMenu, m_languageIsoTwoChar, this,
//          iDataStore->iWFNewsData,
//          iGuiProtHandler);

//    /* Events to handle upgrade of client */
//    WFStartupCommonBlocks::UpgradeClient(m_startupHandler,
//          m_languageIsoTwoChar, this, iGuiProtHandler);

//    /* Add event for user to accept the US disclaimer. */
//    m_startupHandler->RegisterStartupEvent(SS_GotoMainMenu,
//          new (ELeave) WFStartupUiCallback());

//    WFStartupCommonBlocks::UpgradePage(m_startupHandler,
//          m_languageIsoTwoChar, this,
//          iGuiProtHandler);
}

void
CWayFinderAppUi::CreateSlaveAudio()
{
#if defined USE_NO_AUDIO
   iAudioSlave = CDummySlaveAudio::NewL(*this, m_fsSession,
         iPathManager->GetLangResourcePath());
#elif defined USE_NEW_SLAVE_AUDIO
   iAudioSlave = CNewSlaveAudio::NewL( *this, m_fsSession,
         iPathManager->GetLangResourcePath());
#else
   iAudioSlave = COldSlaveAudio::NewL( *this, m_fsSession,
         iPathManager->GetLangResourcePath() );
#endif
}

void
CWayFinderAppUi::SecondStageStartup()
{
#ifdef SERVICE_WINDOW_IS_XHTML_BROWSER
   iCmdHandler = CWayfinderCommandHandler::NewL();
   iCmdCallback =
      CWFCmdCallbackHandler::NewL(this, EWayFinderCmdServiceWindowCall);
#endif
   iDataHolder = new (ELeave) CDataHolder(iLog);
   LOGNEW(iDataHolder, CDataHolder);
   iDataHolder->ConstructL();

   if ( iPathManager->GetLangResourcePath() != KNullDesC ) {
      CreateSlaveAudio();
   }

   ViewNameMapping viewNameMappings[] = {
      {-1, HELP_INDEX_PAGE},
      {EWayFinderStartPageTab, HELP_START_VIEW},
      {EWayFinderMapTab, HELP_MAP_VIEW},
      {EWayFinderGuideTab, HELP_GUIDE_VIEW},
      {EWayFinderItineraryTab, HELP_ITINERARY_VIEW},
      {EWayFinderMyDestTab, HELP_FAVORITES_VIEW},
      {EWayFinderNewDestTab, HELP_SEARCH_VIEW},
      {EWayFinderSettingsTab, HELP_SETTINGS_VIEW},
      {EWayFinderContentWindowTab, HELP_SERVICES_VIEW},
      {EWayFinderInfoTab, HELP_INFORMATION_VIEW},
      {EWayFinderDestinationTab, HELP_DESTINATION_VIEW},
      {EWayFinderPositionSelectTab, HELP_POSITION_SELECT_VIEW},
      {EWayFinderConnectTab, HELP_CONTACTS_VIEW},
      {EWayFinderWelcomeTab, HELP_WELCOME_VIEW},
      {EWayFinderConnectDialog, HELP_CONNECT_DIALOG},
      {EWayFinderVicinityAllTab, HELP_VICINITY_VIEW},
      {EWayFinderVicinityCrossingTab, HELP_VICINITY_VIEW},
      {EWayFinderVicinityFavouriteTab, HELP_VICINITY_VIEW},
      {EWayFinderVicinityPOITab, HELP_VICINITY_VIEW}
   };
   TInt noOfMappings = sizeof(viewNameMappings) / sizeof(ViewNameMapping);

   iHelpUrlHandler = CHelpUrlHandler::NewL(viewNameMappings, 
                                           noOfMappings, 
                                           R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE);

   iNetworkInfoHandler = CNetworkInfoHandler::NewL(this);

   LOGNEW(iAudioSlave, CSlaveAudio);

   {
      HBufC *temp = iPathManager->GetMbmName().AllocLC();
      //temp.Append(KWayfinderMbm);
      /*       WFTextUtil::TDesCopy(temp, m_wayfinder_path + TPtrC(KWayfinderMbm)); */
      iContextPaneAnimator = new (ELeave) CContextPaneAnimator(*temp);

      iGuiDataStore = GuiDataStore::NewL(*temp);
      CleanupStack::PopAndDestroy();
   }

   //Create the views. The template functtion AddViewToFrameWorkL does
   //all the dirtywork. It creates the objects on the heap, assigns
   //the member variables and transfers the ownership to the
   //framework.

   iEventGenerator = CWayFinderEventGenerator::NewL(*this);

#ifdef SHOW_PRIVACY_STATEMENT
   iUrgentShutdown = !iIniFile->doOnce(1,
                                       &IniFile::showPrivacyStatement,
                                       MemFunPtr<CWayFinderAppUi,TBool>
                                       (this, &CWayFinderAppUi::AcceptPrivacyStatementL));
#endif

#ifdef SEND_REGISTRATION_SMS
   if (!iUrgentShutdown) {
      iIniFile->doOnce(0, &IniFile::sendtSilverSms,
                       MemFunPtr<CWayFinderAppUi, TBool>(this,
                                                         &CWayFinderAppUi::CheckSilverSMSQueryL));
   }
#endif

#if defined (SHOW_GPRS_NOTICE)
   if (!iUrgentShutdown) {
      iUrgentShutdown = !iIniFile->doOnce(1,
                                          &IniFile::firstRun2,
                                          MemFunPtr<CWayFinderAppUi,TBool>
                                          (this, &CWayFinderAppUi::AcceptGPRSNoticeL));
   }
#endif

   // ==== Start
#ifdef HIDE_SERVICES
   iStartView = CStartPageView::NewLC(this, iLog, EFalse);
#else
   iStartView = CStartPageView::NewLC(this, iLog, ETrue);
#endif
   AddViewL(iStartView);
   CleanupStack::Pop(iStartView);

   iMapLib = NULL;
   
   // ==== PositionSelect
   AddViewToFrameWorkL( iSelectView );      
   LOGNEW(iSelectView, CPositionSelectView);

   // ==== NewDest
   AddViewToFrameWorkL( iNewDestView );      
   LOGNEW(iNewDestView, CNewDestView);

   // ==== MyDest
   AddViewToFrameWorkL( iMyDestView );
   LOGNEW(iMyDestView, CMyDestView);

   iMyDestView->SetFavoriteFetcher(m_favoriteFetcher);

   // ==== Guide
   AddViewToFrameWorkL( iGuideView );      
   LOGNEW(iGuideView, CGuideView);

   // ==== Itinerary
   AddViewToFrameWorkL( iItineraryView );      
   LOGNEW(iItineraryView, CItineraryView);

   // ==== Map
   AddViewToFrameWorkL( iMapView );      
   LOGNEW(iMapView, CMapView);
   
   iMrFactory = new MapResourceFactory(iMapView, this);
   
   // ==== Info
   AddViewToFrameWorkL( iInfoView );      
   LOGNEW(iInfoView, CInfoView);

   iMapView->SetDetailFetcher(m_detailFetcher);
   iMapView->SetFavoriteFetcher(m_favoriteFetcher);
   
   // ==== Locked Navigation View

   iLockedNavView = CLockedNavigationView::NewLC(this,
                                                 iLog);

   AddViewL(iLockedNavView);
   CleanupStack::Pop(iLockedNavView);


   iVicinityFeedView = CVicinityFeedView::NewLC(this,
                                                iGuideView,
                                                iLog);

   AddViewL(iVicinityFeedView);

   CleanupStack::Pop(iVicinityFeedView);

   
   iVicinityViewAll = CVicinityView::NewLC( this, 
                                            iMapView,
                                            iLog, 
                                            KVicinityViewAllId );

   iVicinityViewAll->SetDetailFetcher(m_detailFetcher);
   iVicinityViewAll->SetFavoriteFetcher(m_favoriteFetcher);
   AddViewL(iVicinityViewAll);

   CleanupStack::Pop(iVicinityViewAll);
		
   iVicinityViewCrossing = CVicinityView::NewLC( this, 
                                                 iMapView,
                                                 iLog, 
                                                 KVicinityViewCrossingId );

   iVicinityViewCrossing->SetDetailFetcher(m_detailFetcher);
   iVicinityViewCrossing->SetFavoriteFetcher(m_favoriteFetcher);
   AddViewL(iVicinityViewCrossing);
   CleanupStack::Pop(iVicinityViewCrossing);
		
   iVicinityViewFavourite = CVicinityView::NewLC( this, 
                                                  iMapView,
                                                  iLog, 
                                                  KVicinityViewFavouriteId );
   
   iVicinityViewFavourite->SetDetailFetcher(m_detailFetcher);
   iVicinityViewFavourite->SetFavoriteFetcher(m_favoriteFetcher);
   AddViewL(iVicinityViewFavourite);
   
   CleanupStack::Pop(iVicinityViewFavourite);
		
   iVicinityViewPOI = CVicinityView::NewLC( this, 
                                            iMapView,
                                            iLog, 
                                            KVicinityViewPOIId );
   iVicinityViewPOI->SetDetailFetcher(m_detailFetcher);
   iVicinityViewPOI->SetFavoriteFetcher(m_favoriteFetcher);
   AddViewL(iVicinityViewPOI);
   CleanupStack::Pop(iVicinityViewPOI);

#ifdef SERVICE_WINDOW_IS_XHTML_BROWSER
   // ==== Content Window
   {
      _LIT(KBackSlash, "\\");
      _LIT(KSlash, "/");

      TInt busyIcons[] = {
         BUSY_ICON_LIST
      };

      HBufC* tmp = WFTextUtil::SearchAndReplaceL(iPathManager->GetDataPath(),
                                                 KBackSlash(), KSlash());

      CleanupStack::PushL(tmp);
      iServiceWindowView = CServiceWindowView::NewL(this,
                                                    iGuiProtHandler,
                                                    iCmdHandler,
                                                    this,
                                                    KWayfinderServiceWindowBaseURL,
                                                    *tmp,
                                                    iPathManager->GetMbmName(),
                                                    BUSY_ICON_NUM_FRAMES,
                                                    busyIcons
                                                    );
      CleanupStack::PopAndDestroy(tmp);
      AddViewL(iServiceWindowView);
   }
#endif

   // ==== Settings
   AddViewToFrameWorkL( iSettingsView );
   LOGNEW(iSettingsView, CSettingsView);
   iSettingsData = CSettingsData::NewL(iLog);
   LOGNEW(iSettingsData, CSettingsData);
   iSettingsView->SetCache(iSettingsData);
   iSettingsData->iApplicationPath.Copy(iPathManager->GetWayfinderPath());
   iSettingsData->iApplicationDllName.Copy(iPathManager->GetApplicationDllName());
   iSettingsData->iAudioBasePath.Copy(iPathManager->GetAudioBasePath());
   iSettingsData->iResourceNoExt.Copy(iPathManager->GetResourceNoExt());
   iSettingsData->iApplicationNameAsPath.Copy(KApplicationPathName);
   iSettingsData->iAudioFormatPrefix.Copy(KaudioFormatPrefix);
   iSettingsData->iCanShowWebUsername = CanShowWebUsername();
   iSettingsData->iCanShowWebPassword = CanShowWebPassword();
   iSettingsData->iAllowGps = EarthAllowGps();
   iSettingsData->iIsIron = IsIronVersion();
   iSettingsData->iShowLangsWithoutVoice = ShowLangsWithoutVoice();
   iSettingsData->iShowLangsWithoutResource = ShowLangsWithoutResource();

   TPtrC allowedLangs = KAllowedLangs();
   if (allowedLangs != KNullDesC) {
      // Create list of allowed langs (mostly for branding).
      iSettingsData->iAllowedLangs = new (ELeave) CDesCArrayFlat(2);
      TLex lexer(allowedLangs);
      while (!lexer.Eos()) {
         iSettingsData->iAllowedLangs->AppendL(lexer.NextToken());
      }
   }

   // ==== Route (Destination)
   AddViewToFrameWorkL( iRouteView );      
   LOGNEW(iRouteView, CRouteView);

   iDataHolder->InitCountries();
   /*    iDataHolder->InitCities(); */

#ifndef NAV2_CLIENT_SERIES60_V3
   iCallObserver = CCallObserver::NewL( this );
   iCallObserver->StartL();
#endif

   if( iUrgentShutdown ){
      HandleCommandL( EWayFinderExitAppl );
   }

#ifndef NAV2_CLIENT_SERIES60_V3
   m_connectData = new(ELeave) CConnectData();
   m_connectData->zeroTime();
#endif

   ConnectNav2L();
   SetNaviPaneLabelL(0);

#if defined NAV2_CLIENT_SERIES60_V3
   iCallHandler = CSymbian9CallHandler::NewL( this );
   iCallHandler->FindIMEI();
   iSmsHandler = SmsHandlerFactory::CreateL( this );
#else
   if(!iCommunicateHelper){
      // needed to be able to send registration sms
      iCommunicateHelper = CCommunicateHelper::NewL(this, iLog);
      LOGNEW(iCommunicateHelper, CCommunicateHelper);
   }
#endif

#ifdef SERVICE_WINDOW_IS_XHTML_BROWSER
   /* Setup the command handler for content window. */
   iCmdHandler->AddNewHandler(iMyDestView, KWF, KFavorite);
   iCmdHandler->AddNewHandler(iStartView, KWF, KMainMenu);
   iCmdHandler->AddNewHandler(iMapView, KWF, KMapView);
   iCmdHandler->AddNewHandler(iSelectView, KWF, KRoute);
   iCmdHandler->AddNewHandler(iCmdCallback, KWTAI, KWP);
   iCmdHandler->AddNewHandler(iCmdCallback, KCallto, KNullDesC);
   iCmdHandler->AddNewHandler(iDataStore->iWFCallbackStartup, KWF, KStartup);
#endif

   RELEASE_CELL_BLOCK({
      NEWLC_AND_LOG(iCC, CCellMapper)(this, iCommunicateHelper);
      CleanupStack::Pop(iCC);
      // always read files on startup.
      TFileName bboxFilename;
      bboxFilename.Copy(iPathManager->GetWayfinderPath());
      bboxFilename.Append(_L("cell_bbox_map.dat"));
      TFileName mapFilename;
      mapFilename.Copy(_L("c:\\cell_map.txt"));
      TFileName borderFilename;
      borderFilename.Copy(_L("c:\\cell_borders.txt"));
      iCC->readTextDataFromFile(m_fsSession, &mapFilename, &borderFilename);
      iCC->readBinaryBBoxDataFromFile(m_fsSession, &bboxFilename, &borderFilename);
   });
   
   /* Everything is now set. */
}

// ----------------------------------------------------
// CWayFinderAppUi::~CWayFinderAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CWayFinderAppUi::~CWayFinderAppUi()
{
   TRACE_FUNC();
   delete m_detailFetcher;
   delete m_favoriteFetcher;

   delete iEventGenerator;
//    delete iStartupHandler; 
   delete iNTCommandHandler;

   delete iPathManager;

   delete iSpreadWFDialogs;

   if (iStartupImage) {
      iStartupImage = NULL;
   }

   if( iTempDecorator != NULL ){
      if (iNaviPane) {
         iNaviPane->Pop( iTempDecorator );
         LOGDEL(iTempDecorator);
         delete iTempDecorator;
         iTempDecorator = NULL;
      }
   }
   LOGDEL(iDecoratedTabGroup);
   delete iDecoratedTabGroup;
   LOGDEL(iMessageHandler);
   delete iMessageHandler;
   iMessageHandler = NULL;
   LOGDEL(iDataHolder);
   delete iDataHolder;

   LOGDEL(iAudioSlave);
   delete iAudioSlave;

   if( iTimer != NULL ){
      iTimer->Cancel();
      LOGDEL(iTimer);
      delete iTimer;
   }
   //Not deleted in head..
   delete iSettingsData;
   LOGDEL(iSettingsData);
   iSettingsData = NULL;

   if (iCallObserver) {
#ifndef NAV2_CLIENT_SERIES60_V3
      iCallObserver->Cancel();
#endif
      LOGDEL(iCallObserver);
      delete iCallObserver;
   }

   if( iBtHandler ){
      LOGDEL(iBtHandler);
      MPositionInterface::CloseAndDeleteL(iBtHandler);
   }

   if( iGpsSocket ){
      LOGDEL(iGpsSocket);
      delete iGpsSocket;
      iGpsSocket = NULL;
   }

   delete iDataStore;

#ifdef NOKIALBS
   if( iPositionProvider != NULL ){
      LOGDEL(iPositionProvider);
      delete iPositionProvider;
   }
#endif

#ifdef asddfssdf
	// may be risky
   // cannot delete nav2 until here, since logging etc is needed above.
   LOGDEL(m_nav2);
   delete m_nav2;
#endif
			
#ifdef MEMORY_LOG
   for(TInt z = 0; z < iViews->Count(); ++z){
      LOGDEL(iViews->At(z));
   }
#endif

#ifndef NAV2_CLIENT_SERIES60_V3
   LOGDEL(m_connectData);
   delete m_connectData;
#endif

#ifndef NAV2_CLIENT_SERIES60_V3
   delete iCommunicateHelper;
#endif

#ifdef RELEASE_CELL
   delete iCC;
#endif

#if defined SYMBIAN_7S //|| defined NAV2_CLIENT_SERIES60_V3
   delete iConnMgr;
   iConnMgr = NULL;
#endif
   delete iCmdHandler;
   delete iCmdCallback;

   delete iWfTypeStr; 
   delete iWfOptsStr;

   delete iHelpUrlHandler;

   delete iNetworkInfoHandler;

   delete iLogMaster;
   LOGDEL(this);
   delete iImeiNbr;

#if defined NAV2_CLIENT_SERIES60_V3
   delete iSmsHandler;
   delete iCallHandler;
#endif
   delete iImageHandler;

   if (m_languageIsoTwoChar) {
      delete[] m_languageIsoTwoChar;
   }
   delete iErrorFilter;
   delete iOfflineTester;

   // XXX This is not deleted now since then we get a KERN-EXEC 3 if 
   // the user presses hang-up button or exits the app through the 
   // phone's program manager while in service view. XXX
   iServiceWindowView->ReleaseGuiProtSender();
   delete iGuiProtHandler;
   iGuiProtHandler = NULL;

   TRACE_FUNC();
}


TInt
CWayFinderAppUi::GetSymbianLanguageCode()
{
   TBuf<128> filename;
   //filename.Copy(iPathManager->GetCommonDataPath());
   //XXX This should be changed to commondatapath on all platforms some day.
   filename.Copy(iPathManager->GetDataPath());
   //CEikonEnv::Static()->InfoWinL(_L("GetSymbianLanguageCode"), filename);
   filename.Append(KLangFileName);
/*    WFTextUtil::TDesCopy(filename, m_parameter_path + KLangFileName); */
   TInt errorCode;
   TBuf8<20> selection;
   if (WFSymbianUtil::getBytesFromFile(m_fsSession, filename,
                                       selection, errorCode)) {
      if (selection.Length() < 2) {
         return 0;
      }
      return ((selection[0]-'0')*10)+(selection[1]-'0');
   } else {
      return 0;
   }
}


void
CWayFinderAppUi::SymbianLanguageToChar(TInt symCode, uint8* outCode)
{
   if (outCode) {
      outCode[0] = '0' + (symCode/10);
      outCode[1] = '0' + (symCode%10);
   }
}

void
CWayFinderAppUi::SaveLanguageCode( TInt language )
{
   /* Get file path. */
   uint8 data[16];
   TBuf<128> filename;
   //filename.Copy(iPathManager->GetCommonDataPath());
   //XXX This should be changed to commondatapath on all platforms some day.
   filename.Copy(iPathManager->GetDataPath());
   //CEikonEnv::Static()->InfoWinL(_L("SaveLanguageCode"), filename);
   filename.Append(KLangFileName);
/*    WFTextUtil::TDesCopy(filename, m_parameter_path + KLangFileName); */

   SymbianLanguageToChar(language, data);

   TInt res = WFSymbianUtil::writeBytesToFile(m_fsSession, filename, 
                                              data, 2, iLog);
   if (res < 0) {
      /* Failed to write language-file. */
      /* XXX What to do? */
/*          filename.Copy(_L("failed write: ")); */
/*          filename.AppendNum(res, EDecimal); */
/*          WFDialog::ShowScrollingWarningDialogL(filename); */
      WFDialog::ShowDebugDialogL( "language file failed write");
   }
}

void 
CWayFinderAppUi::FetchImei()
{
#if !defined __WINS__
   if (!iImeiReceived) {
      iImeiFetcher = CWayfinderImeiFetcher::NewL(*this, ETrue);
   }
#else
   _LIT(KImeiFile, "c:\\imei.txt");

   RFs fs;
   User::LeaveIfError(fs.Connect());
   CleanupClosePushL(fs);
   RFile imeiFile;
   TFileName filename;
   filename.Copy(KImeiFile);

   TInt res = imeiFile.Open(fs, filename, EFileShareReadersOnly|EFileRead);
   TBuf8<16> imei;
   if(res == KErrNotFound){
      User::LeaveIfError(imeiFile.Create(fs, filename,
               EFileShareExclusive | EFileWrite));
      CleanupClosePushL(imeiFile);       
      imei.SetLength(15);
      for(TInt i = 0; i < 15; ++i){
         imei[i] = int(25.0*rand()/(RAND_MAX+1.0)) + 'a';
      }  
      User::LeaveIfError(imeiFile.Write(imei));
   } else if(res == KErrNone){                              
      CleanupClosePushL(imeiFile);
      User::LeaveIfError(imeiFile.Read(imei,15));
   } else {
      User::LeaveIfError(res);
   }
   TBuf<16> wideimei(imei.Length());
   for(TInt i = 0; i < imei.Length(); ++i){
      wideimei[i] = imei[i];
   }
   iImeiNbr = WFTextUtil::newTDesDupL(wideimei);
   CleanupStack::PopAndDestroy(2);//imeiFile and fs

   SecondStageStartup();
#endif
}

void
CWayFinderAppUi::ConnectNav2L()
{
#ifndef GUI_ONLY
   typedef class CleanupClass<char> CCC;
   typedef class CleanupClass<Nav2Error::Nav2ErrorTable> CCE;

   //Locate the dataPaths.
   //FindDataPathsL();

   /* We don't care about failure, we'll handle that elsewhere. */
   char* resPath = WFTextUtil::newTDesDupL(iPathManager->GetLangResourcePath());
   LOGNEWA(resPath, char, strlen(resPath) + 1);
   CleanupStack::PushL(TCleanupItem(CCC::CleanupArray, resPath));

   char* commondataPath  = WFTextUtil::newTDesDupL(iPathManager->GetCommonDataPath());
   LOGNEWA(commondataPath, char, strlen(commondataPath) + 1);
   CleanupStack::PushL(TCleanupItem(CCC::CleanupArray, commondataPath));

   char* dataPath  = WFTextUtil::newTDesDupL(iPathManager->GetDataPath());
   LOGNEWA(dataPath, char, strlen(dataPath) + 1);
   CleanupStack::PushL(TCleanupItem(CCC::CleanupArray, dataPath));

   Nav2Error::Nav2ErrorTable *errorTable = 
      new Nav2Error::Nav2ErrorTableSymbian(R_WAYFINDER_ERROR_TABLE);
   LOGNEW(errorTable, Nav2Error::Nav2ErrorTableSymbian);
   CleanupStack::PushL(TCleanupItem(CCE::Cleanup, errorTable));

   //Cleanup here! not all these checks are necessary
   if(!(
/*             resPath && */
            commondataPath && errorTable &&
        errorTable->checkErrorTable())){
      User::LeaveNoMemory();
   }

   // Decide which audio system to use.
   AudioCtrlLanguage *audioSyntax = NewSyntaxLC(m_languageCode, 
         R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE);
   LOGNEW(audioSyntax, AudioCtrlLanguage);


   // Find out and remember what distance units the language supports.
   iSettingsData->iAudioHasFeetMiles  = audioSyntax->supportsFeetMiles();
   iSettingsData->iAudioHasYardsMiles = audioSyntax->supportsYardsMiles();

   // Set up parameters for Nav2
   Nav2StartupData startdata(GuiProtEnums::InvalidWayfinderType, resPath, commondataPath, dataPath, errorTable, audioSyntax);
   startdata.setSimulation(SIMINPUT, Nav2StartupData::SIM_PROTO);
   startdata.setProgramVersion(isab::Nav2_Major_rel,
                               isab::Nav2_Minor_rel,
                               isab::Nav2_Build_id);

   startdata.clientType          = NAV2_CLIENT_TYPE_STRING;
   startdata.clientTypeOptions   = NAV2_CLIENT_OPTIONS_STRING;
   startdata.debugOutput         = DEBUGOUTPUT;

#ifdef USE_TCP_SYMBIAN_SERIAL
   startdata.tcpsymbianserial = true;
#endif

   startdata.setSerialNumber(iImeiNbr);

# ifdef SIMFILE
   startdata.setSimulation(NULL, 0, Nav2StartupData::SIM_PROTO);
   startdata.simulateFromFile = SIMFILE;
   startdata.fakeNavServerCom = true;
# endif
# ifndef USE_EXCEPTION_HANDLER
   startdata.useExceptionHandler = false;
# endif
# ifndef USE_TRAP_HARNESS_ON_THREADS
   startdata.useTrapHarness = false;
# endif
# ifdef HTTP_PROTO
   startdata.setHttpProto( true );
#endif
# ifdef HTTP_USER_AGENT
   startdata.setHttpUserAgent( HTTP_USER_AGENT );
# endif
# ifdef HTTP_REQUEST_STRING
   startdata.httpRequest = HTTP_REQUEST_STRING;
# endif
# ifdef HTTP_HOST_OVERRIDE
   startdata.httpHostOverride = HTTP_HOST_OVERRIDE;
# endif
   TBool no_proxy_file = EFalse;
# ifdef TEST_FOR_NO_PROXY_FILE
   TInt error = 0;
   _LIT(KNoProxyFile, "c:\\noproxy.txt");
   no_proxy_file = WFSymbianUtil::doesFileExist(m_fsSession,
      KNoProxyFile, error);
# endif
   if (!no_proxy_file){
# ifdef USE_IAP_PROXY
      startdata.setUseIAPProxy( true );
# endif
# ifdef HARD_IAP_PROXY_HOST
      startdata.setHardIapProxyHost( HARD_IAP_PROXY_HOST );
# endif
# ifdef HARD_IAP_PROXY_PORT
      startdata.setHardIapProxyPort( HARD_IAP_PROXY_PORT );
# endif
   }
# ifdef TRANSFORM_USERNAME_CLIP_STRING
   startdata.setUsernameClipString ( TRANSFORM_USERNAME_CLIP_STRING );
#endif
# ifdef USE_TRACKING
   startdata.setUseTracking( true );
# endif

# if defined(__RELEASE__) && !defined(MOCKUP)
   User::LeaveIfNull(m_nav2 = new Nav2Release(startdata));
   LOGNEW(m_nav2, Nav2Release);
# else
#  ifndef MOCKUP
   startdata.setTestShell(TESTSHELL);
#  endif
   User::LeaveIfNull(m_nav2 = new Nav2Develop(startdata));
   LOGNEW(m_nav2, Nav2Develop);
# endif
   LOGDEL(resPath);
   LOGDEL(commondataPath);
   LOGDEL(dataPath);
   CleanupStack::Pop(2, errorTable); //errorTable, audioSyntax
   CleanupStack::PopAndDestroy(dataPath);
   CleanupStack::PopAndDestroy(commondataPath);
   CleanupStack::PopAndDestroy(resPath);
#endif

   // Create engine
   iMessageHandler = new (ELeave) CMessageHandler();
   LOGNEW(iMessageHandler, CMessageHandler);

#ifdef DONT_USE_TCP_TO_NAV2
   {
      isab::Nav2::Channel* guiC = m_nav2->getGuiChannel();
      LOGNEW(guiC, Nav2::Channel);
      iMessageHandler->ConstructL(this, guiC);
   }
//    iMessageHandler->ConstructL( this, m_gui );
#else
   iMessageHandler->ConstructL( this );
#endif

#ifdef GUI_ONLY
   iMessageHandler->ConnectL(GUIMACHINE, GUIPORT );
#else
   iMessageHandler->ConnectL(_L("127.0.0.1"), GUIPORT);
#endif

   // We won't create the GPS connection untill
   // the gps connect command.
   iBtHandler = NULL;
   iGpsSocket = NULL;
   iPositionProvider = NULL;

   iTimer = CTimeOutTimer::NewL(CActive::EPriorityLow, *this/*, iLog*/);
}

void CWayFinderAppUi::NokiaLbsUpdate( GpsState newState )
{
   if (iDoingShutdown) {
      return;
   }
#ifdef NOKIALBS
   switch (newState) {
      case EBroken:
         {
         if (iShowNowGpsWarn) {   
            WFDialog::ShowInformationL( R_WAYFINDER_LBS_WARN_NO_GPS, iCoeEnv );
            iShowNowGpsWarn = EFalse;
         } 
         iMapView->SetInfoText(R_WAYFINDER_LBS_INFO_SEARCHING_FOR_GPS, ETrue);
         }
         break;
      case EUserDisabled:
         WFDialog::ShowInformationL( R_WAYFINDER_LBS_WARN_USER_DISABLED );
         break;
      case EPreStartup:
         //WFDialog::ShowInformationL( _L("LBS:PreStartup"));
         iMapView->SetInfoText( R_WAYFINDER_LBS_INFO_PRE_STARTUP, ETrue );
         break;
      case EStartup:
         //WFDialog::ShowInformationL( _L("LBS:Startup"));
         iMapView->SetInfoText( R_WAYFINDER_LBS_INFO_SEARCHING_FOR_GPS, ETrue);
         break;
      case EAvailable:
         //WFDialog::ShowInformationL( _L("LBS:Avail"));
         iMapView->SetInfoText( R_WAYFINDER_LBS_INFO_WAITING_FOR_POS, ETrue);
         SaveBtGPSFakeAddress();
         break;
      case EUnsupported:
         WFDialog::ShowInformationL( R_WAYFINDER_LBS_WARN_UNSUPPORTED );
         break;
      case EAccessDenied:
         WFDialog::ShowInformationL( R_WAYFINDER_LBS_WARN_ACCESS_DENIED);
         break;
      case EUserCanceledBT:
         WFDialog::ShowInformationL( R_WAYFINDER_LBS_WARN_CANCELED_BT );
         break;
      case EGpsBadData:
         WFDialog::ShowInformationL( R_WAYFINDER_LBS_WARN_BAD_GPS_DATA);
         break;
      case EGpsGotFirstPos:
         iMapView->SetInfoText( R_WAYFINDER_LBS_INFO_GOT_FIRST_POS, EFalse);
         break;
# undef SHOW_ALL_MLFW_STATUS
# ifdef SHOW_ALL_MLFW_STATUS
      case ENotFound:
         WFDialog::ShowInformationL( _L("LBS:NotFound"));
         break;
      case EPosBufOverflow:
         WFDialog::ShowInformationL( _L("LBS:PosBufOverflow"));
         break;
      case EStatusCanceled:
         WFDialog::ShowInformationL( _L("LBS:StatCanceled"));
         break;
      case EUnknownInStatus:
         WFDialog::ShowInformationL( _L("LBS:StatUnknown"));
         break;
      case EUnknownInStatus2:
         WFDialog::ShowInformationL( _L("LBS:Stat2Unknown"));
         break;
      case EUnknownInStatus3:
         WFDialog::ShowInformationL( _L("LBS:Stat3Unknown"));
         break;
      case EUnknownInPosition:
         WFDialog::ShowInformationL( _L("LBS:PosUnknown"));
         break;
      case ENewModuleId:
         WFDialog::ShowInformationL( _L("LBS:NewModuleId"));
         break;
      case EDisconnectCalled:
         WFDialog::ShowInformationL( _L("LBS:Disconnect"));
         iMapView->SetInfoText(_L("Disconnected from MLFW"), ETrue);
         break;
# else
      case ENotFound:
      case EPosBufOverflow:
      case EStatusCanceled:
      case EUnknownInStatus:
      case EUnknownInStatus2:
      case EUnknownInStatus3:
      case EUnknownInPosition:
         // Unexpected errors. Ignore for now.
         break;
      case ENewModuleId:
         // Informational only. Ignore.
         break;
      case EDisconnectCalled:
         //iMapView->SetInfoText(_L("Disconnected from MLFW"), ETrue);
         break;
# endif
   }
   if (iGpsQuality == QualityMissing && iPositionProvider && iPositionProvider->IsConnected() ) {
      HandleGpsQuality(QualitySearching);
   } else if (iGpsQuality == QualitySearching && iPositionProvider && ! iPositionProvider->IsConnected() ) {
      HandleGpsQuality(QualityMissing);
   }
#endif 
}

//#define GPS_SOURCE_SELECTION
void CWayFinderAppUi::ConnectToGpsL()
{
   if (!EarthAllowGps()) {
      //show dialog and return
      EarthShowGPSNoticeL();
      return;
   }

/*    if( IsFullVersion() ){ */
#if defined(GPS_SOURCE_SELECTION) || ! defined(NOKIALBS)
      TBool useBTGps = ETrue;
#endif
#if defined GPS_SOURCE_SELECTION
      TBuf<64> query( _L("Use external Bluetooth GPS?") );
      if( WFDialog::ShowQueryL( query ) ){
         useBTGps = ETrue;
      } else {
         useBTGps = EFalse;
      }
#elif defined __WINS__ || !defined USE_BLUETOOTH
      useBTGps = EFalse;
#endif
#ifdef NOKIALBS
//       CEikonEnv::Static()->InfoWinL(_L("CWayFinderAppUi::ConnectToGpsL()"), _L("2"));
      //Location base framework.
      if (!iPositionProvider) {
         iPositionProvider = CNokiaLbsReader::NewL(*this,
                                                   m_nav2->getBluetoothChannel());
      }
#else
      if( useBTGps ){
         CreateBtGpsL(); //assigns iBtHandler.
         iBtHandler->ConnectL();
      } else {
# ifndef GPS_SOCKET_ADDRESS
         iGpsSocket = CGpsSocket::NewL(m_nav2->getBluetoothChannel());
# else
         MLIT(KGpsIPAddress, GPS_SOCKET_ADDRESS);
         iGpsSocket = CGpsSocket::NewL(m_nav2->getBluetoothChannel(), 
                                       KGpsIPAddress);
# endif

         LOGNEW(iGpsSocket, CGpsSocket);
         iGpsSocket->ConnectL();
      }
#endif

#ifdef NOKIALBS
      if(iPositionProvider && ! iPositionProvider->IsConnected()){
         TRAPD(err, iPositionProvider->ConnectL());
         if(err != KErrNone){
            // FIXME - XXX - These error must be handled
# if 0
            TBuf<50> tmp;
            tmp.Copy(_L("Could not connect to gps: "));
            tmp.AppendNum(err);
             CEikonEnv::Static()->InfoWinL(_L("CWayFinderAppUi::ConnectToGpsL"), tmp);
#endif
            /*::Panic(EWayfinderLbsFailed);*/
         } else {
//             CEikonEnv::Static()->InfoWinL(_L("CWayFinderAppUi::ConnectToGpsL"), _L("Connect: OK"));
         }
      }
#endif
}

void CWayFinderAppUi::CreateBtGpsL()
{
   //this array can be used to rss file
   struct TBtErrorToResource mapping[] = {
      //no message when user cancelled selection.
      //{EBtEngineSelectionFailed,            0},
      {EBtEngineSelectionSdpFailed,           R_WF_BT_SELECTIONSDPFAILED},
      {EBtEngineConnectionFailed,             R_WF_BT_GPSCONNECTIONFAILED},
      {EBtEngineSocketConnectionFailed,       R_WF_BT_GPSCONNECTIONFAILED},
      //no message when use canceled power on
      //{EBtEnginePowerOnCancelled,           0},
      {EBtEnginePowerOnFailed,                R_WF_BT_POWERONFAILED},
      {EBtEngineRepeatedSearchFailed,         R_WF_BT_REPEATEDSEARCHFAILED},
      {EBtEngineHardwareUnavailable,          R_WF_BT_HARDWAREFAILED},
      
      //no message, reconnect
      //{EBtEngineConnectionLostReadError,    0},
      //{EBtEngineConnectionLostWriteTimeout, 0},
      //{EBtEngineConnectionLostWriteError,   0},
      {EBtEngineConnectionLostHardwareFailed, R_WF_BT_HARDWAREFAILED},
      //no message when connection was successfull
      //{EBtEngineConnectionComplete,         0},
      //no message when disconnection is complete
      //{EBtEngineDisconnectionComplete,      0}
   };

//    if(!iLogMaster){
//       iLogMaster = new (ELeave) isab::LogMaster();
//       LOGNEW(iLogMaster, LogMaster);
//       iLogMaster->setDefaultOutput("file:c:\\bttest.txt");
//    }

   if(!iBtHandler){
      class CBtGpsConnector* gps = 
         CBtGpsConnector::NewL(this, mapping, 
                               sizeof(mapping)/sizeof(*mapping), 
                               m_nav2->getBluetoothChannel());
      LOGNEW(gps, CBtGpsConnector);
      if(iLogMaster){
         gps->SetLogMaster(iLogMaster);
      }
      iBtHandler = gps;
      iBtHandler->SetAddress(*(iSettingsData->iGpsParam));
   }
}

void CWayFinderAppUi::DisconnectFromGPSL()
{
   if(iBtHandler && (iBtHandler->IsConnected() || iBtHandler->IsBusy())){
      //disconnect is handled in the destructor.
      LOGDEL(iBtHandler);
      MPositionInterface::CloseAndDeleteL(iBtHandler);
   } else if( iGpsSocket ){
      iGpsSocket->Disconnect();
      LOGDEL(iGpsSocket);
      delete iGpsSocket;
      iGpsSocket = NULL;
   }
#ifdef NOKIALBS
   if(iPositionProvider){
      if( iPositionProvider->IsConnected() ){
         iPositionProvider->DisconnectL();
         iShowNowGpsWarn = ETrue;
      }
      //      delete iPositionProvider;
      //      iPositionProvider = NULL;
   }
#endif
   iIniFile->setGpsId(0);
   iGotBTData = 0;
   GpsStatus(EFalse);
}

void CWayFinderAppUi::ReConnectGps()
{
   if(iBtHandler){
      iBtHandler->ReConnect();
   } else {
      BluetoothDeviceChosen(TBTDevAddr(), KNullDesC);
      ConnectToGpsL(); //creates a new iBtHandler.
   }
}


void
CWayFinderAppUi::SetDistanceMode(TInt mode)
{
   iMapView->SetDistanceMode(mode);
}

TInt
CWayFinderAppUi::GetDistanceMode()
{
   return TInt(iSettingsData->iDistanceMode);
}

void
CWayFinderAppUi::BTDataReceived(const TDesC& /*aName*/, const TDesC8& aAddr)
{
   /* Data received from Bluetooth. */
   /* XXX FIXME This should probably come from Nav2 XXX */

   if (!iGotBTData) {
      /* Try to get the address etc for later use... */
#ifdef DEBUG_BT_SEARCH
      TBuf<275> foo;
      foo.Copy( _L("Get btname"));
      WFDialog::ShowScrollingWarningDialogL(foo);

      foo.Copy( _L("BTName: "));
      foo.Append(aName);
      WFDialog::ShowScrollingWarningDialogL(foo);
      TInt64 lladdr = 0;

      TUint32 high = 0, low = 0;
      high |= aAddr[0] << 8;
      high |= aAddr[1];
      low  |= aAddr[2] << (8*3);
      low  |= aAddr[3] << (8*2);
      low  |= aAddr[4] << 8;
      low  |= aAddr[5];

      lladdr.Set(high, low);
      TBTDevAddr *addr1 = NULL;
      TBTDevAddr *addr2 = NULL;
      TRAPD(foostatus, addr1 = new (ELeave) TBTDevAddr(aAddr));
      if (foostatus != KErrNone) {
         foo.Copy(_L("BTaddr create 1 failed ") );
      } else {
         TRAPD(foostatus2, addr2 = new (ELeave) TBTDevAddr(lladdr));
         if (foostatus != KErrNone) {
            foo.Copy(_L("BTaddr create 2 failed ") );
            delete addr1;
         } else {
            if (*addr1 != *addr2) {
               /* Höh! */
               foo.Copy(_L("BTAddr mismatch: "));
               foo.AppendNum((*addr1)[0], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr1)[1], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr1)[2], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr1)[3], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr1)[4], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr1)[5], EHex);
               foo.Append(_L(" != "));
               foo.AppendNum((*addr2)[0], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr2)[1], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr2)[2], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr2)[3], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr2)[4], EHex);
               foo.Append(_L(":"));
               foo.AppendNum((*addr2)[5], EHex);
            } else {
               /* Addresses are the same! */
               foo.Copy(_L("BTAddr ok: "));
            }
         }
      }
      foo.Append(_L(" = "));
      foo.AppendNum(lladdr);
      foo.Append(_L(", "));
      foo.AppendNum(aAddr[0], EHex);
      foo.Append(_L(":"));
      foo.AppendNum(aAddr[1], EHex);
      foo.Append(_L(":"));
      foo.AppendNum(aAddr[2], EHex);
      foo.Append(_L(":"));
      foo.AppendNum(aAddr[3], EHex);
      foo.Append(_L(":"));
      foo.AppendNum(aAddr[4], EHex);
      foo.Append(_L(":"));
      foo.AppendNum(aAddr[5], EHex);
      WFDialog::ShowScrollingWarningDialogL(foo);
#endif
      iGotBTData = 1;
      GpsStatus(ETrue);
   }
}

void
CWayFinderAppUi::GpsStatus(TBool on)
{
   if (CanUseGPSForEveryThing()) {
      if (iStartView) {
         iStartView->GpsStatus(on);
      }
      if (iNewDestView) {
         iNewDestView->GpsStatus(on);
      }
      if (iMapView) {
         iMapView->GpsStatus(on);
      }
   }
}

const CAnimatorFrame*
CWayFinderAppUi::GetGpsStatusImage()
{
   return iContextPaneAnimator->GetLargeGpsFrameL();
}

void
CWayFinderAppUi::BTError( TDesC& aDescription )
{
   /* Bluetooth error. Bluetooth is disconnected. */
   iIniFile->setGpsId(0);
   iGotBTData = 0;

#ifndef __RELEASE__
   WFDialog::ShowScrollingErrorDialogL(aDescription, iCoeEnv);
#endif
   GpsStatus(EFalse);
}

void
CWayFinderAppUi::BTError( TInt aResourceId )
{
   /* Bluetooth error. Bluetooth is disconnected. */
   iIniFile->setGpsId(0);
   iGotBTData = 0;

   WFDialog::ShowScrollingErrorDialogL(aResourceId, iCoeEnv);
   GpsStatus(EFalse);
}

_LIT(KBTGPS, "BTGPS");
void BTGPSLOG(const TDesC& aFmt, const TDesC* aArg)
{
#ifdef DEBUG_BTGPS
   RFileLogger::WriteFormat(KBTGPS, KBTGPS, EFileLoggingModeAppend, 
                            aFmt, aArg);
#else
   aFmt.Length();
   aArg->Length();
#endif
}
void BTGPSLOG(const TDesC& aFmt, TInt aArg)
{
#ifdef DEBUG_BTGPS
   RFileLogger::WriteFormat(KBTGPS, KBTGPS, EFileLoggingModeAppend, 
                            aFmt, aArg);
#else
   aFmt.Length();
   aArg=aArg;
#endif
}

void CWayFinderAppUi::BluetoothDebug(const TDesC& aDbgMsg)
{
   BTGPSLOG(_L("DBG: %S"), &aDbgMsg);
   
}

void CWayFinderAppUi::BluetoothError(const TDesC& aErrorMsg)
{
   BTGPSLOG(_L("ERR: %S"), &aErrorMsg);
}

void CWayFinderAppUi::BluetoothError(TInt aErrorMsg)
{
   HBufC* err = iCoeEnv->AllocReadResourceLC(aErrorMsg);
   BTGPSLOG(_L("ERR: %S"), err);
   CleanupStack::PopAndDestroy(err);
}

void CWayFinderAppUi::BluetoothStatus(TBool aOk)
{
   BTGPSLOG(_L("BT status %d"), aOk ? 1 : 0);
}

void CWayFinderAppUi::SaveBtGPSFakeAddress()
{
   TInt64 address = 0;
   TBuf8<32> buf;
   _LIT8(KHexFormat, "0x%x");
   char* array[3] = {0};
  	 
   buf.Format(KHexFormat, HIGH(address));
   array[0] = WFTextUtil::newTDesDupLC(buf);
   buf.Format(KHexFormat, LOW(address));
   array[1] = WFTextUtil::newTDesDupLC(buf);
   array[2] = "MLFW";
  	 
   class GeneralParameterMess mess(GuiProtEnums::paramBtGpsAddressAndName,
                                   array, 3);
   SendMessageL( &mess );
   CleanupStack::PopAndDestroy(2, array[0]);
}

void CWayFinderAppUi::BluetoothDeviceChosen(const class TBTDevAddr& aAddr, 
                                            const TDesC& aName)
{
   TInt64 address = 0;
   for(TInt a = 0; a < 6; ++a){
      address = (address << 8) + TInt64(aAddr[a] & 0x0ff);
   }
   TBuf8<32> buf;
   _LIT8(KHexFormat, "0x%x");
   char* array[3] = {0};
   class CDesC8Array& gpsParam = *(iSettingsData->iGpsParam);
   TBool newdevice = gpsParam.MdcaCount() < 3;

   buf.Format(KHexFormat, HIGH(address));
   array[0] = WFTextUtil::newTDesDupLC(buf);
   if(!newdevice && gpsParam.MdcaCount() > 0){
      newdevice = newdevice || (buf != gpsParam[0]);
   }

   buf.Format(KHexFormat, LOW(address));
   array[1] = WFTextUtil::newTDesDupLC(buf);
   if(!newdevice && gpsParam.MdcaCount() > 1){
      newdevice = newdevice || (buf != gpsParam[1]);
   }

   array[2] = WFTextUtil::newTDesDupLC(aName);
   if(!newdevice){
      HBufC8* name8 = WFTextUtil::NarrowLC(aName);
      newdevice = newdevice || (*name8 == gpsParam[2]);
      CleanupStack::PopAndDestroy(name8);
   }

   if(newdevice){
      class GeneralParameterMess mess(GuiProtEnums::paramBtGpsAddressAndName,
                                      array, 3);
      SendMessageL( &mess );
      iSettingsData->SetBtGpsAddressL((const char**)array, 3);
   }
   CleanupStack::PopAndDestroy(3, array[0]);
   if(address != 0){
      BTDataReceived(aName, aAddr.Des());
   }
}

void CWayFinderAppUi::BluetoothConnectionLost(TBool /*aWillTryReconnect*/)
{
//    class CAknWarningNote* btwarning = new CAknWarningNote;
//    if(btwarning){
//       btwarning->ExecuteLD(_L("GPS connection lost!"));
//    }
}

void CWayFinderAppUi::UpdateConnectionManagerL()
{
#ifdef SYMBIAN_7S
   if(!iSettingsData->iLinkLayerKeepAlive || 
      (iConnMgr && GetIAP() != iConnMgr->Iap())){
      delete iConnMgr;
      iConnMgr = NULL;
   }
   if(iSettingsData->iLinkLayerKeepAlive && !iConnMgr && GetIAP() >= 0){
      iConnMgr = CSimpleConnectionManager::NewL(GetIAP());
   }
#endif
}

void
CWayFinderAppUi::ShowDetailsL(FullSearchDataReplyMess* aMessage, TInt aCommand)
{
   iNewDestView->ShowDetailsL(aMessage, aCommand);
}

// ----------------------------------------------------
// CWayFinderAppUi::SendMessageL()
// Send a message to Nav2.
// ----------------------------------------------------
//
int32 CWayFinderAppUi::SendMessageL( GuiProtMess* aMessage )
{
   // Log actions Nav2 interaction.
   LOGHEAPSIZE("#2 CWayFinderAppUi::SendMessageL (ver3)");
   if (iLog){
      iLog->debug("Gui sends message (ver3): 0x%"PRIx8,
                  aMessage->getMessageType());
   }

   Buffer buffer( KInitialBufferLength );
   if(0 == buffer.capacity()){
      User::Leave(KErrNoMemory);
   }
   TUint16 messageId = iMessageCounter += 2;
   aMessage->setMessageId( messageId );
   aMessage->serialize( &buffer );
   if(iMessageHandler){
      iMessageHandler->SendMessageL( &buffer );
   }
   return messageId;
}

//this is the MMessageSender version of the SendMessage function
int32 CWayFinderAppUi::SendMessageL(class isab::GuiProtMess &mess)
{
   return SendMessageL(&mess);
}


void CWayFinderAppUi::UpdateData( UpdatePositionMess* aPositionMess )
{
   TInt32 lat = aPositionMess->getLat();
   TInt32 lon = aPositionMess->getLon();
   TInt32 alt = aPositionMess->getAlt();

   TInt heading = -1;
   if( aPositionMess->headingQuality() > QualityUseless ){
      heading = aPositionMess->getHeading()*360;
      heading = (TInt)((heading/256.0)+0.5);
   }

   iHeading = heading;
   
   TInt speed = -1;
   TInt unit_speed = -1;
   
   if( aPositionMess->speedQuality() > QualityUseless ){
      TReal rSpeed = aPositionMess->getSpeed()*3.6;
      speed = (TInt)((rSpeed/32)+0.5);
      rSpeed = DistancePrintingPolicy::convertSpeed(aPositionMess->getSpeed(),
            DistancePrintingPolicy::DistanceMode(GetDistanceMode()));
      unit_speed = (TInt)((rSpeed/32)+0.5);
      iUnitSpeed = unit_speed;
   }  else {
      iUnitSpeed = 0;
   }
   
   iMrFactory->initIfNecessary();
   MapLib* mapLib = iMrFactory->getMapLib();

   Nav2Coordinate newCenter(lat, lon);


   // If we are not in map view, appUI is responsible for setting the
   // appropriate scale and center position. Also, if we are not there,
   // disable the actual drawing of stuff.
   
   if(mapLib && !MapViewActive()) {
      CTileMapControl* mapControl = iMrFactory->getMapControl();
      TileMapHandler& handler = mapControl->Handler();
      
      mapLib->disableDrawing();   iMrFactory->getMapLib()->enableDrawing();
      mapLib->setCenter( newCenter );
      mapLib->setScale(2);
      
      handler.setAngle(double(heading));
      handler.requestRepaint();
   } else if(MapViewActive()) {
      mapLib->enableDrawing();
   }
   
   iDataHolder->SetInputData( lat, lon, heading, unit_speed, alt );
   iRouteView->SetInputData( lat, lon, heading, unit_speed, alt );
   iInfoView->SetInputData( lat, lon, heading, unit_speed, alt, iGpsQuality /*IsGpsConnected()*/ );
   iLockedNavView->SetInputData( lat, lon, heading, unit_speed, alt, iGpsQuality /*IsGpsConnected()*/ );
   iVicinityFeedView->SetInputData( lat, lon, heading, unit_speed, alt, iGpsQuality /*IsGpsConnected()*/ );
   
   if( iTabGroup->ActiveTabId() == KMapViewId.iUid ){
      iMapView->UpdateDataL( aPositionMess, speed );
   }
   CVicinityView* activeVicView;
   if( VicinityViewActive(activeVicView) ){
      activeVicView->UpdateDataL( aPositionMess, speed );
   }

   
#ifdef RELEASE_CELL
   iCC->SetInputData( lat, lon, heading, speed );
#endif
   m_speed = speed;
   m_angle = heading;
}


// ----------------------------------------------------
// CWayFinderAppUi::ReceiveMessageL()
// Recive a message from Nav2.
// ----------------------------------------------------
//
void
CWayFinderAppUi::ReceiveMessageL(const TDesC8& aMessage,
                                 const TInt /*aMessageLength*/ )
{
   LOGHEAPSIZE("#1 CWayFinderAppUi::ReceiveMessageL");
   if(iMemCheckInterval-- < 1){
      iMemCheckInterval = WAYFINDER_MEMORY_CHECK_INTERVAL;
      if( !CheckMem(80*1024 ) ) {
         WFDialog::ShowScrollingDialogL( iCoeEnv, R_WAYFINDER_MEMORY_LOW_MSG,
                                         R_WAYFINDER_MEMORY_LOW_EXIT_MSG, ETrue);
         iUrgentShutdown = ETrue;
         /* Switch to main view. */
         TUid messageId;
         messageId.iUid = EShutdown;
         TBuf8<16> customMessage( _L8("") );
         SetViewL( KWelcomeViewId, messageId, customMessage );
      }
   }
   
   if (!iMessagesReceived) {
      /* Start startup on first message from Nav2. */
      iMessagesReceived = ETrue;
      m_startupHandler->StartupEvent(SS_Start);
   }

   if (!m_hasSetLanguage) {
      if (m_languageCode < 0) {
         /* Don't send the language code, haven't found it... */
         /* What to do? */
      } else {
         TInt code = GetSymbianLanguageCode();
         iSettingsView->SetLanguage(code);
         GeneralParameterMess* gpm = new (ELeave) GeneralParameterMess(
               GuiProtEnums::paramLanguage, m_languageCode);
         CleanupStack::PushL(gpm);
         LOGNEW(gpm, GeneralParameterMess);
         SendMessageL( gpm );
         gpm->deleteMembers(); // not done elsewhere, so do it here
         CleanupStack::Pop(gpm);
         LOGDEL(gpm);
         delete gpm;
      }
      m_hasSetLanguage = ETrue;
   }

   if (iBacklightStrategy == GuiProtEnums::backlight_always_on) {
      TurnBackLightOnL();
   }

   GuiProtMess* message = NULL;
   // copy the data to protect us from blocking dialogs. 
   // this buffer will leak if anything leaves, sorry. 
   Buffer buffer( aMessage.Length() );
   buffer.writeNextByteArray(aMessage.Ptr(), aMessage.Length());

   buffer.setReadPos(4); // Start after version and length   
   message = GuiProtMess::createMsgFromBuf( &buffer );
   LOGNEW(message, GuiProtMess);

   // Log actions Nav2 interaction.
   if (iLog){
      iLog->debug("Gui received message (ver3): 0x%"PRIx8,
                  message->getMessageType() );
   }

   if( message == NULL ){
#ifndef __RELEASE__
      WFDialog::ShowErrorL( R_WAYFINDER_UNHANDLED_MSG_MSG, iCoeEnv );
#endif
      return;
   }

   // Check new message handling.
   if (iGuiProtHandler) {
      if (iGuiProtHandler->GuiProtReceiveMessage(message)) {
         /* Message handled. */
         return;
      }
   }

   GuiProtEnums::MessageType type = message->getMessageType();
   switch( type )
   {
   case GuiProtEnums::MESSAGETYPE_ERROR:
      {
         ErrorMess* errMess = static_cast<ErrorMess*>(message);
         iMyDestView->SetStatesFalse();
         RequestCancelled();
         ShowErrorDialogL(errMess->getErrorNumber(),
               errMess->getErrorString());
      }
      break;
   case GuiProtEnums::REQUEST_FAILED:
      {
         RequestFailedMess* rf = static_cast<RequestFailedMess*>(message);
         HandleRequestFailedL(rf);
      }
      break;
   case GuiProtEnums::PARAMETER_CHANGED:
      UpdateParametersL( (GenericGuiMess*)message );
      break;
   case GuiProtEnums::GET_TOP_REGION_LIST_REPLY:
      HandleTopRegionReplyL( (GetTopRegionReplyMess*)message );
      break;
   case GuiProtEnums::GET_FAVORITES_REPLY:
      if( iTabGroup->ActiveTabId() == KMyDestViewId.iUid ) {
         iMyDestView->HandleFavoritesReply( (GetFavoritesReplyMess*)message );
      } else if (iTabGroup->ActiveTabId() == KMapViewId.iUid) {
/*    iMapView->HandleFavoritesReply( (GetFavoritesReplyMess*)message ); */
      } 
      break;
   case GuiProtEnums::GET_FAVORITES_ALL_DATA_REPLY:
      if( iTabGroup->ActiveTabId() == KMyDestViewId.iUid ) {
         iMyDestView->HandleFavoritesAllDataReply( (GetFavoritesAllDataReplyMess*)message );
      } 
      CVicinityView* vicView;
      if( VicinityViewActive(vicView) ) {
         vicView->HandleFavoritesAllDataReply( 
                     (GetFavoritesAllDataReplyMess*)message );
      }
      iMapView->HandleFavoritesAllDataReply( (GetFavoritesAllDataReplyMess*)message );
      break;
   case GuiProtEnums::GET_FAVORITE_INFO_REPLY:
      m_favoriteFetcher->FavoriteReplyReceived((GetFavoriteInfoReplyMess*)message);
      break;
   case GuiProtEnums::FAVORITES_CHANGED:
      {
         if( iSynchronizing ){
            iSynchronizing = EFalse;
         }
         if( iTabGroup->ActiveTabId() == KMyDestViewId.iUid ){
            RequestFavoritesL( EFalse );
         } else if ( iTabGroup->ActiveTabId() == KNewDestViewId.iUid ){
            iMyDestView->SetStatesFalse();
            iNewDestView->FavoriteSaved();
         } else {
            iMyDestView->SetStatesFalse();
         }
         iMapView->FavoriteChanged();
         break;
      }
   case GuiProtEnums::SET_SIMPLE_PARAMETER:
      HandleSetSimpleParametersL( (SimpleParameterMess*)message );
      break;
   case GuiProtEnums::SET_CALL_CENTER_NUMBERS: //Deprecated
      // XXX Deprecated XXX
      break;
   case GuiProtEnums::UPDATE_ROUTE_INFO:
      if (iBacklightStrategy == GuiProtEnums::backlight_on_during_route) {
         TurnBackLightOnL();
      }
      HandleNavigation(static_cast<UpdateRouteInfoMess*>(message));
      break;
   case GuiProtEnums::FILEOP_GUI_MESSAGE:
      iNTCommandHandler->HandleGuiFileOperation(
            static_cast<GuiProtFileMess*>(message));
      break;
   case GuiProtEnums::SATELLITE_INFO:
      break;
   case GuiProtEnums::ROUTE_LIST:
      HandleRouteList(static_cast<RouteListMess*>(message));
      break;
   case GuiProtEnums::PREPARE_SOUNDS:
      HandlePrepareSounds(static_cast<PrepareSoundsMess*>(message));
      break;
   case GuiProtEnums::PLAY_SOUNDS:
      HandlePlaySounds();
      break;
   case GuiProtEnums::SOUND_FILE_LIST:
      HandleSoundFileList(static_cast<SoundFileListMess*>(message));
      break;
   case GuiProtEnums::PROGRESS_INDICATOR:
      HandleProgressIndicator(static_cast<GenericGuiMess*>(message));
      break;
   case GuiProtEnums::UPDATE_POSITION_INFO:
      {
         //GPS data
         UpdatePositionMess& dataMessage =
            *static_cast<UpdatePositionMess*>(message);

         Quality gpsQuality = Quality(dataMessage.positionQuality());

#ifndef USE_BLUETOOTH
         if( (iCurrPos.iY == dataMessage.getLat() &&
              iCurrPos.iX == dataMessage.getLon()) ||
             (dataMessage.getLat() == MAX_INT32 &&
              dataMessage.getLon() == MAX_INT32) ){
            if( iNoGpsCounter > 5 )
               gpsQuality = QualityMissing;
            else
               iNoGpsCounter++;
         }
         else
            iNoGpsCounter = 0;
#endif
         if (gpsQuality == QualityMissing && IsGpsConnectedOrSearching() ) {
            gpsQuality = QualitySearching;
         }

         HandleGpsQuality(gpsQuality);

         if(iGpsQuality > QualityUseless){
            SetCurrentPosition( dataMessage.getLat(), dataMessage.getLon() );
            UpdateData( &dataMessage );
         }
      }
      break;
   case GuiProtEnums::STARTED_NEW_ROUTE:
      {
         StartedNewRouteMess *dataMessage =
            static_cast<StartedNewRouteMess *>(message);
         /* Cache route id. */
         if (m_routeid >= 0) {
            m_last_routeid = m_routeid;
         }
         m_routeid = dataMessage->getRouteId();

         if (m_routeid != 0) {
            /* Send general parameter to save the route id. */

            uint8 data[sizeof(int64)*2]; //allocate buffer data on stack.
            Buffer buf(data, sizeof(data), 0); //Buffer uses data as mem
            buf.writeNext64bit(m_routeid); //m_routeid written to data

/*             TBuf<128> fff; */
/*             fff.Copy( _L("last route ") ); */
/*             fff.AppendNum( len, EDecimal ); */
/*             fff.Append( _L(" bytes") ); */
/*             WFDialog::ShowScrollingWarningDialogL(fff); */

            //Message on automatic storage.
            GeneralParameterMess gpm(GuiProtEnums::paramLastKnownRouteId,
                                     data, buf.getLength());
            buf.releaseData();
            SendMessageL( &gpm );
            //dont delete members, since thay are on automatic storage.

            iRouteBoxTl.iY = dataMessage->getTopLat();
            iRouteBoxTl.iX = dataMessage->getLeftLon();
            iRouteBoxBr.iY = dataMessage->getBottomLat();
            iRouteBoxBr.iX = dataMessage->getRightLon();
/*             SetWait( EFalse, 0 ); */
            StartNavigationL(dataMessage->getoLat(), dataMessage->getoLon(),
                             dataMessage->getdLat(), dataMessage->getdLon(),
                             dataMessage->getDestination());
         }
         else{
            StopNavigationL();
         }
      }
      break;
   case GuiProtEnums::SEARCH_RESULT_CHANGED:
      {
/*          SetWait( EFalse, 0 ); */
         GenericGuiMess *mess = (static_cast<GenericGuiMess*>(message));
         GenericGuiMess getAreas(GuiProtEnums::GET_SEARCH_AREAS);
         GenericGuiMess getItems(GuiProtEnums::GET_SEARCH_ITEMS, uint16(0));
         if ( mess->getFirstBool() ) {
            /* Areas have been changed. */
            SendMessageL(&getAreas);
            iDataHolder->SetSearchTotalHits(0);
            iDataHolder->SetSearchStartIndex(0);
         }
         if ( mess->getSecondBool() ) {
            /* Search items have changed. */
            SendMessageL(&getItems);
            m_XXX_temp_no_search_items = 0;
            iDataHolder->SetSearchTotalHits(0);
            iDataHolder->SetSearchStartIndex(0);
         } else {
            /* No changed search items. */
            m_XXX_temp_no_search_items = 1;
         }
         if (!mess->getFirstBool() && !mess->getSecondBool() ) {
            WFDialog::ShowInformationL( R_WAYFINDER_NOHITS_MSG, iCoeEnv );
            iDataHolder->SetSearchTotalHits(0);
            iDataHolder->SetSearchStartIndex(0);
         }
      }
      break;
   case GuiProtEnums::GET_SEARCH_AREAS_REPLY:
      HandleSearchReplyL(static_cast<SearchAreaReplyMess*>(message));
      break;
   case GuiProtEnums::GET_SEARCH_ITEMS_REPLY:

      HandleSearchReplyL(static_cast<SearchItemReplyMess*>(message));
      break;
   case GuiProtEnums::GET_MAP_REPLY:
      HandleMapReply(static_cast<MapReplyMess*>(message));
      break;
   case GuiProtEnums::GET_FULL_SEARCH_DATA_REPLY:
      
/*       WFDialog::ShowDebugDialogL("GOT Reply full search"); */
      iNewDestView->ShowDetailsL(static_cast<FullSearchDataReplyMess*>(message));
      break;
   case GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID_REPLY:
/*       WFDialog::ShowDebugDialogL("GOT Reply full search with item id"); */
      m_detailFetcher->ReplyReceived(static_cast<FullSearchDataReplyMess*>(message));
      break;
   case GuiProtEnums::SEND_MESSAGE_REPLY:
#ifndef NAV2_CLIENT_SERIES60_V3
      iCommunicateHelper->HandleSendMessageReplyL(static_cast<MessageSentMess*>(message));
#endif
      break;
   case GuiProtEnums::LICENSE_UPGRADE_REPLY:
      {
         int ap = 22;
         ap++;
      }
/*       HandleLicenseUpgradeReply(static_cast<LicenseReplyMess*>(message)); */
      break;
   case GuiProtEnums::PANIC_ABORT:
      // A fatal error was caught by Nav2. Close the program.
      HandleCommandL(EWayFinderExitAppl);
      break;
   case GuiProtEnums::SET_GENERAL_PARAMETER:
      HandleSetGeneralParameter(static_cast<GeneralParameterMess*>(message));
      break;
   case GuiProtEnums::GET_GENERAL_PARAMETER:
      {
         /* Unset general parameter. */
         HandleGetGeneralParameter(static_cast<GeneralParameterMess*>(message));
      }
      break;
   case GuiProtEnums::PARAMETERS_SYNC_REPLY:
      {
         GenericGuiMess *ggm =
               static_cast<GenericGuiMess*>(message);
         SetWayfinderType((GuiProtEnums::WayfinderType)ggm->getFirstUint32());
         if (iIAPSearcher && iIAPSearcher->Searching()) {
            /* IAP request. */
            iIAPSearcher->Reply();
         }
      }
      break;
   case GuiProtEnums::GET_MULTI_VECTOR_MAP_REPLY:
#if 0
      // Not yet
   case GuiProtEnums::FORCEFEED_MULTI_VECTOR_MAP_REPLY:
#endif
      iMapView->
         HandleMultiVectorMapReply(static_cast<const DataGuiMess*>(message));
      break;
   case GuiProtEnums::GUI_TUNNEL_DATA_REPLY:
      /* Ignore. */
      break;
      // going the other way.
   case GuiProtEnums::SYNC_FAVORITES_REPLY:
      /* Nothing to do here. */
      break;
   case GuiProtEnums::INVALIDATE_ROUTE:
   case GuiProtEnums::GET_SEARCH_AREAS:
   case GuiProtEnums::GET_SEARCH_ITEMS:
   case GuiProtEnums::GET_TOP_REGION_LIST:
   case GuiProtEnums::SEARCH:
   case GuiProtEnums::ROUTE_MESSAGE:
   case GuiProtEnums::ROUTE_TO_POSITION:
   case GuiProtEnums::ROUTE_TO_SEARCH_ITEM:
   case GuiProtEnums::ROUTE_TO_FAVORITE:
   case GuiProtEnums::ROUTE_TO_HOT_DEST:
   case GuiProtEnums::PREPARE_SOUNDS_REPLY:
   case GuiProtEnums::PLAY_SOUNDS_REPLY:
   case GuiProtEnums::GET_SIMPLE_PARAMETER:
   case GuiProtEnums::GET_FAVORITES:
   case GuiProtEnums::GET_FAVORITES_ALL_DATA:
   case GuiProtEnums::SORT_FAVORITES:
   case GuiProtEnums::SYNC_FAVORITES:
   case GuiProtEnums::GET_FAVORITE_INFO:
   case GuiProtEnums::ADD_FAVORITE:
   case GuiProtEnums::ADD_FAVORITE_FROM_SEARCH:
   case GuiProtEnums::REMOVE_FAVORITE:
   case GuiProtEnums::CHANGE_FAVORITE:
   case GuiProtEnums::CONNECT_GPS:
   case GuiProtEnums::DISCONNECT_GPS:
   case GuiProtEnums::GET_MAP:
   case GuiProtEnums::GET_FULL_SEARCH_DATA:
   case GuiProtEnums::REROUTE:
   case GuiProtEnums::GET_FILTERED_ROUTE_LIST:
   case GuiProtEnums::REQUEST_CROSSING_SOUND:
   case GuiProtEnums::GET_MORE_SEARCH_DATA:
   case GuiProtEnums::SEND_MESSAGE:
   case GuiProtEnums::REQUEST_LICENSE_UPGRADE:
   case GuiProtEnums::GET_MULTI_VECTOR_MAP:
      break;
   case GuiProtEnums::GET_CALL_CENTER_NUMBERS: //Deprecated
      // XXX Deprecated XXX
      break;
#if 1
   default:
      {
#if !defined(__RELEASE__)
         TBuf<KBuf64Length> note;
         iCoeEnv->ReadResource( note, R_WAYFINDER_UNKNOWN_MSG_MSG );
         note.Append( _L(" ") );
         note.AppendNumFixedWidth( (uint8)type, EDecimal, 8 );
         WFDialog::ShowWarningL( note );
         // This should never happend.
         // Makes the compiler happy though.
#endif
      }
#endif
   }
   message->deleteMembers();
   LOGDEL(message);
   delete message;
}

void CWayFinderAppUi::ShowNoGpsPositionsDialogL()
{
   WFDialog::ShowWarningDialogL(R_WF_NO_GPS_POSITIONS);
}

void CWayFinderAppUi::HandleGpsQuality( Quality gpsQuality )
{
   TBool changed = EFalse;
   
   if( iGpsQuality != gpsQuality ) {
      changed = ETrue;
      iGpsQuality = gpsQuality;
      iContextPaneAnimator->SetCurrentGpsStatus(iGpsQuality);
      CAnimatorFrame* temp = new CAnimatorFrame();
      temp->CopyFrameL(iContextPaneAnimator->GetLargeGpsFrameL());
      iMapView->SetGpsStatusImage(temp->GetBitmap(), temp->GetMask());
      temp->DisownBitmaps();
      delete temp;
      LOGDEL(temp);
      SetContextIconL();

      iInfoView->UpdateGpsState( gpsQuality );
      
      if( iGpsQuality < QualityUseless) {
         iMapView->HideUserPosition();         
      }
   } else {
      //  return;
   }
   
   if(iGpsQuality == QualityMissing)
      return; //No device..

   //

   if( iGpsQuality <= QualityUseless) {
      if( iNotifyGpsUseless) {
         if(iNotifyGpsCounter <= 0) {
            iNotifyGpsUseless = EFalse;
            iNotifyGpsCounter = MAX_INT32;
            ShowNoGpsPositionsDialogL();
            iNotifyGpsCounter = GPS_NO_POSITIONS_TIMEOUT;
         } else {
            iNotifyGpsCounter--;
         }
      } 
   } else {
      //Now GPS is not useless, but if it is in the future, we would
      //like the user to know about it.
      iNotifyGpsUseless = ETrue;
      iNotifyGpsCounter = GPS_NO_POSITIONS_TIMEOUT;
   }
}

void CWayFinderAppUi::HandlePhoneCallL( TBool aConnected )
{
   m_phoneCallInProgress = aConnected;
   updateMute();
   if( aConnected ){
      //TRACE_FUNC();
      // Phoning
   } else {
      //TRACE_FUNC();
      // Phone ende
      if (iPhoneCallFromWf) {
         TInt tmp = iPhoneCallFromWf;
         iPhoneCallFromWf = 0;
         HandleCommandL(tmp);
         return;
/*       } else if( iMapCall ){ */
/*          iTabGroup->SetActiveTabByIndex( iTabGroup->ActiveTabIndex() ); */
/*          ActivateLocalViewL(TUid::Uid( iTabGroup->TabIdFromIndex( iTabGroup->ActiveTabIndex() ) )); */
/*  */
/*          iMapCall = EFalse; */
/*       } else if( iNewDestCall ){ */
/*          iTabGroup->SetActiveTabById( KNewDestViewId.iUid ); */
/*          ActivateLocalViewL( KNewDestViewId ); */
/*          iNewDestCall = EFalse; */
      } else {
         if (iForeGround) {
            /* Just move to the last view we were at. */
            iTabGroup->SetActiveTabByIndex( iTabGroup->ActiveTabIndex() );
            ActivateLocalViewL(TUid::Uid( iTabGroup->TabIdFromIndex( iTabGroup->ActiveTabIndex() ) ));
         } else {
            /* We weren't in the foreground before call... */
         }
      }
   }
}



TBool CWayFinderAppUi::EditAndCallNumberL(const TDesC& aPhoneNumber, TInt aCommand)
{
   return CallNumber::EditAndCallNumberL(aPhoneNumber, aCommand, this);
}

TBool CWayFinderAppUi::DialNumberL(const TDesC& aPhoneNumber, TInt aCommand)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   TBool result = EFalse;
   if (aPhoneNumber.Compare(KEmpty) != 0) {
      HandleCommandL(aCommand);
      iCallHandler->MakeCall( aPhoneNumber );
      result = ETrue;
   }
   return result;
#else
   return CallNumber::DialNumberL(aPhoneNumber, aCommand, this);
#endif
}

void
CWayFinderAppUi::SoundReadyL( TInt aError,
                                   TInt32 aDuration )
{   
   GenericGuiMess message( GuiProtEnums::PREPARE_SOUNDS_REPLY,
                           (uint32)aDuration );
   SendMessageL( &message );
   if ( aError != KErrNone ) {
      // This was done by SlaveAudio before
      ResourceErrorL( aError, 1 );      
   }
}

void CWayFinderAppUi::SoundPlayedL( TInt aError )
{   
   GenericGuiMess message( GuiProtEnums::PLAY_SOUNDS_REPLY);
   SendMessageL( &message );
   if ( aError != KErrNone ) {
      // This was done by SlaveAudio before
      ResourceErrorL( aError, 1 );
   }
}

void CWayFinderAppUi::SendLogMessage( const TDesC& /*aMessage*/ )
{   
}

void CWayFinderAppUi::ResourceErrorL( TInt aError, TBool aLanguageDependant )
{
   switch( aError )
   {
      case KErrNotSupported:
      case KErrNotFound:
      case KErrCorrupt:
         if (aLanguageDependant) {
            WFDialog::ShowScrollingErrorDialogL(
                  R_WAYFINDER_BADLANGRESOURCEFILE_MSG, iCoeEnv );
         } else {
            WFDialog::ShowScrollingErrorDialogL(
                  R_WAYFINDER_BADRESOURCEFILE_MSG, iCoeEnv );
         }
         break;
      case KErrInUse:
      case KErrNoMemory:
      default:
         {
            break;
         }
   }
}

void CWayFinderAppUi::RequestMap( MapSubject aMapType,
                                  TInt32 aLat, TInt32 aLon )
{
   if( iMapView->VectorMaps() &&
       iTabGroup->ActiveTabId() != KMapViewId.iUid)
   {
      iMapView->SetMapRequestData( aMapType, aLat, aLon );
      if(IsGpsConnected() ) {
         TUid messageId;
         messageId.iUid = ENoMessage;
         TBuf8<16> customMessage( _L8("") );
         SetViewL( KMapViewId, messageId, customMessage );
      } else {
         Nav2Coordinate newCenter(aLat, aLon);

         iMrFactory->initIfNecessary();
            
         MapLib* mapLib = iMrFactory->getMapLib();

         // If we are not in map view, appUI is responsible for setting the
         // appropriate scale and center position. Also, if we are not there,
         // disable the actual drawing of stuff.
   
         if(mapLib && !MapViewActive()) {
            CTileMapControl* mapControl = iMrFactory->getMapControl();
            TileMapHandler& handler = mapControl->Handler();
      
            mapLib->disableDrawing();
            mapLib->setCenter( newCenter );
            mapLib->setScale(2);
      
            handler.requestRepaint();
         }

         /* Move to start page when position has been selected. */
         GotoStartViewL();
      }
   } else {
      iMapView->RequestMap( aMapType, aLat, aLon );
   }
}



void CWayFinderAppUi::StartupCompleted()
{
   iStartupCompleted = ETrue;
   
   if(!iBtHandler && iGpsAutoConnectAfterStartup) {
      ConnectToGpsL();
      iGpsAutoConnectAfterStartup = EFalse;
   }
    
}

void CWayFinderAppUi::GotoStartViewL()
{   
   TUid messageId;
   TBuf8<16> customMessage( _L8("") );
   /* Classic mode. */
   messageId.iUid = ENoMessage;
   SetViewL( KStartPageViewId, messageId, customMessage );

   if (!iDataStore->iWFGpsData->getReadyToUseGps()) {
      iDataStore->iWFGpsData->setReadyToUseGps(true);
      class GeneralParameterMess gpm(GuiProtEnums::paramBtGpsAddressAndName);
      CleanupDeleteMembersPushL(gpm);
      SendMessageL( gpm );
      CleanupStack::PopAndDestroy(&gpm);
   }
}


TInt64
CWayFinderAppUi::GetLastRouteId()
{
   return m_last_routeid;
}

TInt64
CWayFinderAppUi::GetRouteId()
{
   return m_routeid;
}

void CWayFinderAppUi::GetTopRegionsL()
{
   //Request the favorites lists
   GenericGuiMess message( GuiProtEnums::GET_TOP_REGION_LIST );
   SendMessageL( &message );
}

void CWayFinderAppUi::RequestFavoritesL( TBool aSync )
{
   //Request the favorites lists
   if(aSync){
      iSynchronizing = ETrue;
      SendSyncFavorites();
   } else {
      GenericGuiMess get(GuiProtEnums::GET_FAVORITES, uint16(0),
                         uint16(MAX_UINT16));
      SendMessageL(&get);
   }
}

void CWayFinderAppUi::RequestSimpleParameterL( TUint aType )
{
   // aType can be:
   // GuiProtEnums::paramServerNameAndPort
   // GuiProtEnums::paramSoundVolume
   // GuiProtEnums::paramUseSpeaker
   // GuiProtEnums::paramAutoReroute
   // GuiProtEnums::paramTranspotationType
   // GuiProtEnums::paramUserName
   GenericGuiMess message( GuiProtEnums::GET_SIMPLE_PARAMETER, (uint16)aType );
   SendMessageL( &message );
}

void CWayFinderAppUi::RequestRouteListL( TInt aStart, TInt aNumber )
{
   GetFilteredRouteListMess message( aStart, aNumber );
   SendMessageL( &message );
}

void
CWayFinderAppUi::SetCurrentRouteCoordinates(TInt32 dLat, TInt32 dLon,
      TInt32 oLat, TInt32 oLon)
{
   iCurrentDestination.iY = dLat;
   iCurrentDestination.iX = dLon;
   iCurrentOrigin.iY = oLat;
   iCurrentOrigin.iX = oLon;
   iRouteView->SetCoordinates( dLat, dLon );
   iMapView->SetDestination( dLat, dLon );
/*    iMapView->SetOrigin( dLat, dLon ); */
}

void
CWayFinderAppUi::SetCurrentRouteDestinationName( const TDesC &aDescription )
{
   iRouteView->SetDestination( aDescription );
   if (iCurrentDestinationName) {
      delete iCurrentDestinationName;
      iCurrentDestinationName = NULL;
   }
   iCurrentDestinationName = aDescription.AllocL();
}

const TDesC&
CWayFinderAppUi::GetCurrentRouteDestinationName()
{
   return *iCurrentDestinationName;
}

void
CWayFinderAppUi::SetCurrentRouteEndPoints(TDesC &aDescription,
      TInt32 dLat, TInt32 dLon,
      TInt32 oLat, TInt32 oLon)
{
   SetCurrentRouteDestinationName(aDescription);
   SetCurrentRouteCoordinates(dLat, dLon, oLat, oLon);
/*    SaveCurrentRouteCoordinates(dLat, dLon, oLat, oLon); */
}

#if 0
void
CWayFinderAppUi::SaveCurrentRouteCoordinates(int32 /*dLat*/, int32 /*dLon*/,
      int32 /*oLat*/, int32 /*oLon*/)
{
            /* The last known endpoints should not be cached. */
   int32 data[4];
   data[0] = dLat;
   data[1] = dLon;
   data[2] = oLat;
   data[3] = oLon;
   GeneralParameterMess *gpm =
      new (ELeave) GeneralParameterMess(GuiProtEnums::paramLastKnownRouteEndPoints,
                                        (int32 *)&data, 4);
   CleanupStack::PushL(gpm);
   LOGNEW(gpm, GeneralParameterMess);
   SendMessageL( gpm );
   /* XXX Don't do gpm->deleteMembers(), since the data is local. */
   CleanupStack::Pop(gpm);//gpm, hmm?
   LOGDEL(gpm);
   delete gpm;
}
#endif

void
CWayFinderAppUi::RouteToHotDestL()
{
   //Route to the hot dest
   RouteMess message(GuiProtEnums::PositionTypeHotDest,
                     "", MAX_INT32, MAX_INT32, "");
   SendMessageL( &message );
   //no need to delete members, there are none

   TBuf<2> tmp;
   tmp.Copy( _L("") );
   SetCurrentRouteEndPoints( tmp, MAX_INT32, MAX_INT32,
                             MAX_INT32, MAX_INT32);
}

void
CWayFinderAppUi::RouteToCoordinateL(TInt32 aLat, TInt32 aLon,
                                   TDesC &aDescription )
{
   char* description = WFTextUtil::newTDesDupL(aDescription);
   LOGNEWA(description, char, strlen(description) + 1);
   if(!description){
      User::LeaveNoMemory();
   }
   CleanupStack::PushL(TCleanupItem(CleanupClass<char>::CleanupArray,
                                    description));
   RouteMess message(GuiProtEnums::PositionTypePosition,
                     "", aLat, aLon, description);
   SendMessageL( &message );
   SetCurrentRouteEndPoints(aDescription, aLat, aLon, MAX_INT32, MAX_INT32);
   CleanupStack::PopAndDestroy();
}

void CWayFinderAppUi::SetForceUpdate( TBool aForce )
{
   iForceUpdate = aForce;
}

TInt32 CWayFinderAppUi::LastViewId()
{
   return iLastViewId;
}

void CWayFinderAppUi::AttemptViewRefresh(TUid aViewId)
{
   if(iTabGroup->ActiveTabId() != aViewId.iUid) {
      return;
   }


   CVicinityView* vicView;
   if(VicinityViewActive(vicView)) {
      vicView->ForceRefresh();
   } else if(aViewId == KLockedNavViewId) {
      iLockedNavView->ForceRefresh();
   } else if(aViewId == KVicinityFeedId) {
      iVicinityFeedView->ForceRefresh();
   } else if(aViewId == KInfoViewId) {
      iInfoView->ForceRefresh();
   }
}


void CWayFinderAppUi::SetViewL( TUid aUid, TUid aMessageId, TDesC8 aMessage )
{
   AttemptViewRefresh(aUid);
   
   iTabGroup->SetActiveTabById( aUid.iUid );
   if ( iForeGround ) {
      // Symbian signed says that the application must not
      // pop up when in background.
      // Change to the correct view when the HandleForeground event is
      // received instead.
      
      ActivateLocalViewL( aUid, aMessageId, aMessage );
   }
}

void CWayFinderAppUi::SetContextIconL()
{
   CAnimatorFrame * frame = iContextPaneAnimator->GetCurrentFrameL();
   LOGNEW(frame, CAnimatorFrame);

   if (frame) {
/*       WFDialog::ShowDebugDialogL("Got current frame!"); */
      CFbsBitmap *a;
      CFbsBitmap *b;
      a = frame->GetBitmap();
      b = frame->GetMask();
      if (a) {
/*          WFDialog::ShowDebugDialogL("Got bitmap!"); */
      } else {
         WFDialog::ShowDebugDialogL("Bitmap not OK!");
      }
      if (b) {
/*          WFDialog::ShowDebugDialogL("Got mask!"); */
      } else {
         WFDialog::ShowDebugDialogL("Mask not OK!");
      }
      if (a && b) {
         /* Context pane takes ownership of the bitmaps. */
         frame->DisownBitmaps();
         iContextPane->SetPicture(a, b);
      }
      delete frame;
      LOGDEL(frame);
   } else {
      WFDialog::ShowDebugDialogL("Can't get current frame!");
   }

}
void CWayFinderAppUi::SetContextIconL( TInt aIconIdx )
{
   iContextPaneAnimator->SetCurrentFrame(aIconIdx);

   SetContextIconL();
}

void CWayFinderAppUi::SetNaviPaneLabelL( TInt aResourceId )
{
   if( aResourceId != 0 ){
      if( iTempDecorator != NULL ){
         iNaviPane->Pop( iTempDecorator );
         LOGDEL(iTempDecorator);
         delete iTempDecorator;
         iTempDecorator = NULL;
      }
      HBufC* text;
      text = iCoeEnv->AllocReadResourceL( aResourceId );
      LOGNEW(text, HBufC);

      iTempDecorator = iNaviPane->CreateNavigationLabelL( *text );
      LOGNEW(iTempDecorator, CAknNavigationDecorator);

      LOGDEL(text);
      delete text;
      iNaviPane->PushL( *iTempDecorator );
   }
   else{
      iNaviPane->Pop( iTempDecorator );
      LOGDEL(iTempDecorator);
      delete iTempDecorator;
      iTempDecorator = NULL;
   }
}

void CWayFinderAppUi::SetWait( TBool aStart, TInt aResourceId )
{
   if (iDoingShutdown) {
      return;
   }
   if( iTimer != NULL ){
      iTimer->Cancel();
   }
   if( aStart ){
      SetContextIconL();
      SetNaviPaneLabelL( aResourceId );
      iTimer->After(KRefresh);
   } else {
      SetContextIconL(0);
      SetNaviPaneLabelL( 0 );
      iMapView->SetConStatusImage(NULL, NULL);
      iWelcomeView->SetConStatusImage(NULL, NULL);
   }
   iWaiting = aStart;
/*    iMapView->ShowVectorMapWaitSymbol( aStart ); */
}

CDataHolder* CWayFinderAppUi::GetDataHolder()
{
   return iDataHolder;
}

TPoint
CWayFinderAppUi::GetOrigin()
{
   if (iCurrentOrigin.iY == MAX_INT32 &&
       iCurrentOrigin.iX == MAX_INT32) {
      /* No origin position set, try to use the one in */
      /* the position select view. */
      return iSelectView->GetOrigin();
   } else {
      return iCurrentOrigin;
   }
}

TBool
CWayFinderAppUi::IsOriginSet()
{
   if (iCurrentOrigin.iY == MAX_INT32 &&
       iCurrentOrigin.iX == MAX_INT32) {
      /* No origin position set, try to use the one in */
      /* the position select view. */
      return iSelectView->HasOrigin();
   } else {
      return ETrue;
   }
}

TBool
CWayFinderAppUi::HasOrigin()
{
   return (iSelectView->HasOrigin());
}

TBool
CWayFinderAppUi::HasDestination()
{
   return (iSelectView->HasDestination());
}

void
CWayFinderAppUi::PositionSelectRoute()
{
   iSelectView->Route();
}

TPoint
CWayFinderAppUi::GetDestination()
{
   if (iCurrentDestination.iY == MAX_INT32 &&
       iCurrentDestination.iX == MAX_INT32) {
      /* No destination set by routing. */
      return iSelectView->GetDestination();
   } else {
      return iCurrentDestination;
   }
}

TBool CWayFinderAppUi::IsDestinationSet()
{
   if (iCurrentDestination.iY == MAX_INT32 &&
       iCurrentDestination.iX == MAX_INT32) {
      return iSelectView->HasDestination();
   } else {
      return ETrue;
   }
}

void CWayFinderAppUi::SetCurrentPosition( TInt32 aLat, TInt32 aLon )
{
   iCurrPos.iX = aLon;
   iCurrPos.iY = aLat;
}

TPoint CWayFinderAppUi::GetCurrentPosition()
{
   return iCurrPos;
}

void CWayFinderAppUi::SetOrigin( GuiProtEnums::PositionType aType,
      const TDesC& aName, const char* aId, TInt32 aLat, TInt32 aLon )
{
   iSelectView->SetOrigin( aType, aName, aId, aLat, aLon );
}

void CWayFinderAppUi::SetDestination( GuiProtEnums::PositionType aType,
      const TDesC& aName, const char* aId, TInt32 aLat, TInt32 aLon )
{
   iSelectView->SetDestination( aType, aName, aId, aLat, aLon );
}

TBool CWayFinderAppUi::IsSimulationPaused()
{
   return (iSimulationStatus & isab::RouteEnums::simulate_paused);
}
TBool CWayFinderAppUi::IsSimulationRepeating()
{
   return (iSimulationStatus & isab::RouteEnums::simulate_repeat_on);
}
TBool CWayFinderAppUi::IsSimulating()
{
   return (iSimulationStatus & isab::RouteEnums::simulate_on);
}
TBool CWayFinderAppUi::IsSimulationMaxSpeed()
{
   return (iSimulationStatus & isab::RouteEnums::simulate_max_speed);
}
TBool CWayFinderAppUi::IsSimulationMinSpeed()
{
   return (iSimulationStatus & isab::RouteEnums::simulate_min_speed);
}

TBool CWayFinderAppUi::HasRoute()
{
   return iHasRoute;
}

void CWayFinderAppUi::SetCurrentTurn( TInt aTurn )
{
   iItineraryView->SetCurrentTurn( aTurn );
   iGuideView->SetCurrentTurn( aTurn );
   iRouteView->SetCurrentTurn( aTurn );
}

CAknNavigationDecorator* CWayFinderAppUi::GetNavigationDecorator()
{
   return iDecoratedTabGroup;
}

TBool CWayFinderAppUi::VectorMaps()
{
   return iMapView->VectorMaps();
}

void CWayFinderAppUi::DebugAction()
{
//   __UHEAP_MARK;
//   _LIT(KHeader, "Header");
   _LIT(KMessage, "Message");
   WFDialog::ShowQueryL(KMessage);
   WFDialog::ShowScrollingDialogL(iCoeEnv, R_WAYFINDER_FROM_GPS_TEXT, 
                                  R_WAYFINDER_FROM_GPS_TEXT, EFalse);
   isab::GeneralParameterMess *gpm = new (ELeave) GeneralParameterMess(
      isab::GuiProtEnums::paramLatestNewsChecksum, (int32)0);

   SendMessageL( gpm );
   gpm->deleteMembers();
   delete gpm;


   // Add code here to test an action by the UI.
//   __UHEAP_MARKEND;
}

void CWayFinderAppUi::GotoLockedNavigationViewL(VicinityItem* target,
                                                MapLib* aMapLib)
{
   GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
   
   TInt32 lastViewId = iTabGroup->TabIdFromIndex(iTabGroup->ActiveTabIndex());
   if (lastViewId != KLockedNavViewId.iUid) {
      //so we dont go back to locked nav view over and over again.
      iLastViewId = lastViewId;
   }

   TBuf8<16> customMessage( _L8("") );
   TUid messageId;
   messageId.iUid = EExitOnExit;
   iLockedNavView->SetTarget(target);
   SetViewL( KLockedNavViewId, messageId, customMessage );
}

void CWayFinderAppUi::GotoServiceViewL(const TDesC& aUrl, enum BackActionEnum aAllowBack)
{
   GetNavigationDecorator()->MakeScrollButtonVisible(EFalse);
   TInt32 lastViewId = iTabGroup->TabIdFromIndex(iTabGroup->ActiveTabIndex());
   if (lastViewId != KServiceWindowViewId.iUid) { 
      //so we dont go back to service view over and over again.
      iLastViewId = lastViewId;
   }
   if (iServiceWindowView) {
      TUid messageId;
      switch (aAllowBack) {
         case BackIsExit:
            messageId.iUid = EExitOnExit;
            break;
         case BackIsViewBack:
            messageId.iUid = EBackOnExit;
            break;
         case BackIsHistoryThenView:
            messageId.iUid = EBackHistoryThenView;
            break;
         case BackIsHistoryBack:
            messageId.iUid = EBackHistory;
            break;
      }
      TBuf8<16> customMessage( _L8("") );

      iServiceWindowView->SetDefaultUrlL(aUrl);
      SetViewL( KServiceWindowViewId, messageId, customMessage );
   } else {
      LaunchWapLinkL(KWayfinderExternalBrowserBaseURL);
   }
}

void CWayFinderAppUi::GotoInfoInServiceViewL(TInt aLat, TInt aLon, const HBufC* aSrvString)
{
   _LIT(KURLLat, "?lat=");
   _LIT(KURLLon, "&lon=");
   _LIT(KSRVString, "&srvstring=");

   TInt srvStringLength = 0;
   if (aSrvString != NULL) {
      srvStringLength = aSrvString->Length();
   }
   HBufC* baseUrl = HBufC::NewLC(KWayfinderServiceWindowShowInfoURL().Length()+KURLLon().Length()+KURLLat().Length()+KSRVString().Length()+(16*2)+srvStringLength);
   baseUrl->Des().Copy(KWayfinderServiceWindowShowInfoURL);
   baseUrl->Des().Append(KURLLat);
   baseUrl->Des().AppendNum(aLat);
   baseUrl->Des().Append(KURLLon);
   baseUrl->Des().AppendNum(aLon);
   baseUrl->Des().Append(KSRVString);
   if (aSrvString != NULL) {
      baseUrl->Des().Append(*aSrvString);
      //CEikonEnv::Static()->InfoWinL(_L("SrvStr"), *aSrvString);
   }

   GotoServiceViewL(*baseUrl);
   CleanupStack::PopAndDestroy(baseUrl);
}

void CWayFinderAppUi::LaunchBuyExtensionWapLinkL()
{
#ifdef WAYFINDER_BUY_EXTENSION_IN_SERVICE_WINDOW
   GotoServiceViewL(KWayfinderServiceWindowBuyExtensionURL);
#else
   LaunchWapLinkL(KWayfinderBuyExtensionBaseURL);
#endif
}

char *
CWayFinderAppUi::FormatXHTMLWapLinkLC(const TDesC& aURL)
{
   HBufC* tmp = FormatWapLinkLC(aURL, 1);

   char *c = WFTextUtil::newTDesDupL(*tmp);
   CleanupStack::PopAndDestroy(tmp);
   CleanupStack::PushL(c);
   return c;
}

HBufC*
CWayFinderAppUi::FormatWapLinkLC(const TDesC& aUrl, TInt aXhtml)
{
   HBufC* wapUrl = HBufC::NewLC(1024);
   HBufC* temp = HBufC::NewLC(80);
   TPtr tmp = wapUrl->Des();

   _LIT(KQuestion, "?");

   tmp.Copy(aUrl);
   if (aUrl.Find(KQuestion) == KErrNotFound) {
      tmp.Append(_L("?u="));
   } else {
      tmp.Append(_L("&u="));
   }
   if (iDataStore->iWFAccountData->getUserName()) {
      HBufC* str = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getUserName());
      tmp.Append(*str);
      CleanupStack::PopAndDestroy(str);
   }
   tmp.Append(_L("&l="));
   HBufC* languageCode = iCoeEnv->AllocReadResourceLC(
         R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE );
   tmp.Append(*languageCode);
   CleanupStack::PopAndDestroy(languageCode);
   tmp.Append(_L("&c="));
   tmp.Append(*iWfTypeStr);
   tmp.Append(_L("&o="));
   tmp.Append(*iWfOptsStr);
   tmp.Append(_L("&i="));
/* #ifdef NAV2_CLIENT_SERIES60_V3 */
   WFTextUtil::char2HBufC(temp, iImeiNbr);
/* #else */
/*    WFTextUtil::char2HBufC(temp, GetIMEI()); */
/* #endif */
   tmp.Append(*temp);

   tmp.Append(_L("&v="));
   tmp.AppendNum( isab::Nav2_Major_rel );
   tmp.Append( _L(".") );
   tmp.AppendNum( isab::Nav2_Minor_rel );
   tmp.Append( _L(".") );
   tmp.AppendNum( isab::Nav2_Build_id );
   tmp.Append( _L("&s="));
   TInt pos = iSettingsData->iServer.Find(_L("."));
   if (pos != KErrNotFound) {
      tmp.Append(iSettingsData->iServer.Left(pos));
   }
   tmp.Append(_L("&q="));
   tmp.AppendNum( aXhtml );
   if (iCurrPos.iX != MAX_INT32 &&
       iCurrPos.iY != MAX_INT32) {
      Nav2Coordinate pos(iCurrPos.iY, iCurrPos.iX);
      MC2Coordinate mc2pos(pos);
      tmp.Append(_L("&curlat="));
      tmp.AppendNum((TInt)mc2pos.lat);
      tmp.Append(_L("&curlon="));
      tmp.AppendNum((TInt)mc2pos.lon);
   }

   CleanupStack::PopAndDestroy(temp);
   return wapUrl;
}

void CWayFinderAppUi::LaunchWapLinkL(const TDesC& aUrl)
{
   HBufC* wapCmd = HBufC::NewLC(1024);
   TPtr tmp = wapCmd->Des();
   tmp.Copy(_L("4 "));

   HBufC* wapUrl = FormatWapLinkLC(aUrl, 0);
   tmp.Append(*wapUrl);

   LaunchWapBrowserUtil::LaunchWapBrowser(*wapCmd);

   CleanupStack::PopAndDestroy(wapUrl);
   CleanupStack::PopAndDestroy(wapCmd);
}

#if 0
void CWayFinderAppUi::LaunchMoveWayfinder()
{
#ifndef NAV2_CLIENT_SERIES60_V3
   TBool result = WFDialog::ShowQueryL(R_WAYFINDER_MOVEWF_HELP_TEXT, iCoeEnv);
   if (result) {
      _LIT(KMwfPath, "\\system\\apps\\MoveWayfinder\\MoveWayfinder.app");
      if ( CMoveWayfinderLauncher::DoLaunchL(iPathManager->GetCommonDataPath(), KMwfPath) ) {
         WFDialog::ShowQueryL(R_WAYFINDER_MOVE_WAYFINDER_LAUNCH_FAILED, iCoeEnv);
      } else {
         ShutdownNow();
      }
   }
#endif
}

void CWayFinderAppUi::LaunchGetWayfinder()
{
#ifndef NAV2_CLIENT_SERIES60_V3
   _LIT(KGwfPath, "\\system\\apps\\GetWayfinder\\GetWayfinder.app");
   if ( CMoveWayfinderLauncher::DoLaunchL(iPathManager->GetCommonDataPath(), KGwfPath) ) {
      WFDialog::ShowQueryL(R_WAYFINDER_GET_WAYFINDER_LAUNCH_FAILED, iCoeEnv);
   } else {
      ShutdownNow();
   }
#endif
}
#endif

#ifndef NAV2_CLIENT_SERIES60_V3
void CWayFinderAppUi::ShowConnectDialog(CConnectData* connectData)
{
   connectData->zeroTime();
   ShowConnectDialog(
      connectData->getFavorite(),
      connectData->getTransmitMode(),
      connectData->getContentsType(),
      connectData->getMessageFormat(),
      connectData->getPosition());
}
#endif

#ifndef NAV2_CLIENT_SERIES60_V3
void CWayFinderAppUi::ShowConnectDialog(const isab::Favorite* favorite,
                                        CConnectDialog::ETransmitMode transmitMode,
                                        CConnectDialog::EContentsType contentsType,
                                        CConnectDialog::EMessageFormat messageFormat,
                                        TPoint* pos)
{
//   iCC->SetInputData( 1, 2, 3, 4 );
#if defined(DEBUG_CELL) && defined(TOMAS)
   // self test code
   delete iCC;
   iCC = NULL;
   iCC = new(ELeave) CCellMapper(*this, iCommunicateHelper);
   iCC->ConstructL();
   iCC->SetInputData( 1, 2, 3, 4 );
   iCC->SetInputData( 1, 2, 3, 4 );
   iCC->SetInputData( 5, 6, 3, 4 );
   iCC->SetInputData( 1, 2, 3, 4 );
   iCC->SetInputData( 500, 600, 3, 4 );
   iCC->SetInputData( 1, 2, 3, 4 );
   int i = 0;
   int count = 101;
#ifdef __WINS__
   count = 1001;
#endif
   for (i = 0; i < count; i++) {
      iCC->SetInputData(i*10, i*10, 6, 7);
   }
   TInt64 a;
   a.Set(6,7);
   for (i = 0; i < count; i++) {
      iCC->SetInputData(Math::Rand(a), Math::Rand(a), Math::Rand(a) & 0xff, Math::Rand(a) & 0xff); // test
   }
   iCC->SetInputData( 1, 2, 3, 4 );
   iCC->SetInputData( 1, 2, 3, 4 );
   iCC->SetInputData( 5, 6, 3, 4 );
   iCC->SetInputData( 1, 2, 3, 4 );
   iCC->SetInputData( 500, 600, 3, 4 );
   iCC->SetInputData( 1, 2, 3, 4 );
   TFileName mapFilename;
   mapFilename.Copy(_L("c:\\cell_map.txt"));
   TFileName borderFilename;
   borderFilename.Copy(_L("c:\\cell_borders.txt"));
   iCC->readTextDataFromFile(m_fsSession, &mapFilename, &borderFilename);
#endif

   iConnectDialog = CConnectDialog::NewL( this, iLog, iCommunicateHelper );
   iConnectDialog->PrepareLC( R_WAYFINDER_CONNECT_DIALOG );
   if (iConnectDialog->setInitialState(m_connectData, favorite, transmitMode, contentsType, messageFormat, pos)) {
      iConnectDialog->RunLD();
   } else {
      // failed to open, delete favorite if present.
      delete favorite;
      delete pos;
      CleanupStack::PopAndDestroy(iConnectDialog);
   }
}
#endif

#if 0
void CWayFinderAppUi::ShowHelpL( TInt aResourceId1, TInt aResourceId2, TInt aResourceId3, TBool firstIsTitle, TInt aResourceId4, TInt aResourceId5, TInt aResourceId6 )
{
#ifdef USE_SERVICE_WINDOW_HELP
   HandleCommandL(EWayFinderCmdHelp);
#else 
# if 1
   TDes* message = new TBuf<6200>( _L("") );
   HBufC* header;
   HBufC* text;
   if (!firstIsTitle) {
      header = iCoeEnv->AllocReadResourceL( R_WAYFINDER_HELP_TITLE );
      HBufC* text = iCoeEnv->AllocReadResourceL (aResourceId1);
      message->Append(*text);
      delete text;
   } else {
      header = iCoeEnv->AllocReadResourceL (aResourceId1);
   }
   if (aResourceId2 != 0) {
      text = iCoeEnv->AllocReadResourceL (aResourceId2);
      message->Append(*text);
      delete text;
   }
   if (aResourceId3 != 0) {
      text = iCoeEnv->AllocReadResourceL (aResourceId3);
      message->Append(*text);
      delete text;
   }
   if (aResourceId4 != 0) {
      text = iCoeEnv->AllocReadResourceL (aResourceId4);
      message->Append(*text);
      delete text;
   }
   if (aResourceId5 != 0) {
      text = iCoeEnv->AllocReadResourceL (aResourceId5);
      message->Append(*text);
      delete text;
   }
   if (aResourceId6 != 0) {
      text = iCoeEnv->AllocReadResourceL (aResourceId6);
      message->Append(*text);
      delete text;
   }
   WFDialog::ShowScrollingDialogL( *header, *message, EFalse );
   delete header;
   delete message;
# else
   TBuf<16> header( _L("") );
   TBuf<1024> text( _L("") );
   iCoeEnv->ReadResource( header, R_WAYFINDER_HELP_TITLE );
   iCoeEnv->ReadResource( message, aResourceId1 );
   if( aResourceId2 != 0 ){
      iCoeEnv->ReadResource( text, aResourceId2 );
      message.Append( text );
   }
   if( aResourceId3 != 0 ){
      iCoeEnv->ReadResource( text, aResourceId3 );
      message.Append( text );
   }
# endif
#endif
}
#endif

void CWayFinderAppUi::ShowHelpL(TInt32 aUid)
{
   HBufC* helpUrl = iHelpUrlHandler->FormatLC(aUid);
   GotoServiceViewL(*helpUrl);
   CleanupStack::PopAndDestroy(helpUrl);
}

void CWayFinderAppUi::ShowHelpL()
{
#ifdef USE_SERVICE_WINDOW_HELP
   TInt32 uid = iTabGroup->TabIdFromIndex(iTabGroup->ActiveTabIndex());
   HBufC* helpUrl = iHelpUrlHandler->FormatLC(uid);
   GotoServiceViewL(*helpUrl);
   CleanupStack::PopAndDestroy(helpUrl);
#else
   TInt32 index = iTabGroup->ActiveTabIndex();
   TInt32 uid = iTabGroup->TabIdFromIndex( index );
   TInt resourceId1 = R_WAYFINDER_HELP_NO_HELP;
   TInt resourceId2 = 0;
   TInt resourceId3 = 0;
   TInt resourceId4 = 0;

   if( uid == KStartPageViewId.iUid ){
      resourceId1 = R_STARTPAGE_HELP_COMMON_0;
      resourceId2 = R_STARTPAGE_HELP_GPS_1;
   } else if( uid == KPositionSelectViewId.iUid ){
      resourceId1 = R_POSITIONSELECT_HELP_COMMON_0;
   } else if( uid == KNewDestViewId.iUid ){
      resourceId1 = R_SEARCH_RESULTS_HELP_COMMON_0;
      if (IsGpsAllowed()) {
         resourceId2 = R_SEARCH_RESULTS_HELP_GPS_1;
      } else {
         resourceId2 = R_SEARCH_RESULTS_HELP_NO_GPS_1;
      }
      resourceId3 = R_SEARCH_RESULTS_HELP_COMMON_1;
   } else if( uid == KMyDestViewId.iUid ){
      if (IsGpsAllowed()) {
         resourceId1 = R_FAVORITES_HELP_GPS_1;
      } else {
         resourceId1 = R_FAVORITES_HELP_NO_GPS_1;
      }
      resourceId2 = R_FAVORITES_HELP_COMMON_1;
      if (IsTrialVersion()) {
         resourceId3 = R_FAVORITES_HELP_NO_TRIAL_1;
      }
   } else if( uid == KGuideViewId.iUid ){
      resourceId1 = R_GUIDE_HELP_COMMON_0;
      if (IsGpsAllowed()) {
         resourceId2 = R_GUIDE_HELP_GPS_1;
      } else {
         resourceId2 = R_GUIDE_HELP_NO_GPS_1;
      }
   } else if( uid == KItineraryViewId.iUid ){
      resourceId1 = R_ITINERARY_HELP_COMMON_0;
      if (IsGpsAllowed()) {
         resourceId2 = R_ITINERARY_HELP_GPS_1;
      } else {
         resourceId2 = R_ITINERARY_HELP_NO_GPS_1;
      }
      resourceId3 = R_ITINERARY_HELP_COMMON_1;
   } else if( uid == KDestinationViewId.iUid ){
      resourceId1 = R_DESTINATION_HELP_COMMON_0;
   } else if( uid == KInfoViewId.iUid ){
      resourceId1 = R_INFO_HELP_COMMON_0;
   }
   ShowHelpL( resourceId1, resourceId2, resourceId3 );
#endif
}

#if 0
void
CWayFinderAppUi::ShowUpgradeHelp()
{
   switch (iDataStore->iWFAccountData->getWfType()) {
      case GuiProtEnums::Gold:
         ShowHelpL(R_WAYFINDER_SERVICE_HELP_GOLD_TITLE_TEXT,
                   R_WAYFINDER_SERVICE_HELP_GOLD_INFO1,
                   R_WAYFINDER_SERVICE_HELP_GOLD_INFO2,
                   ETrue);
         break;
      case GuiProtEnums::Silver:
         ShowHelpL(R_WAYFINDER_SERVICE_HELP_SILVER_TITLE_TEXT,
                   R_WAYFINDER_SERVICE_HELP_SILVER_INFO1,
                   R_WAYFINDER_SERVICE_HELP_SILVER_INFO2,
                   ETrue);
         break;
      case GuiProtEnums::Trial:
         /* FALLTHROUGH */
      default:
         ShowHelpL(R_WAYFINDER_SERVICE_HELP_TRIAL_TITLE_TEXT,
                   R_WAYFINDER_SERVICE_HELP_TRIAL_INFO1,
                   R_WAYFINDER_SERVICE_HELP_TRIAL_INFO2,
                   ETrue);
         break;
   }
}
#endif

void CWayFinderAppUi::ShowUpgradeInfoL(enum TUpgradeMode mode,
                                       TUid aBackToView)
{
   if(iUrgentShutdown){
      return;
   }
   iUpgradeFromView = aBackToView;

   /* XXX show upgrade page in service window. XXX */
   m_startupHandler->StartupEvent(SS_ShowUpgradePage);

/*    ShowUpgradeHelp(); */
/*    ShowUpgradeDialogL(mode); */
}

#if 0
void CWayFinderAppUi::ShowUpgradeDialogL(enum TUpgradeMode mode)
{
   iUpgradeDialog = CUpgradeDialog::NewL( this, iLog, mode );
   TInt resourceId = R_WAYFINDER_REGISTER_DIALOG;
   switch(mode){
   case EActivate:
#ifdef ACTIVATE_WITH_CODE_AND_MSISDN
      resourceId = R_WAYFINDER_REGISTER_DIALOG;
#elif defined ACTIVATE_WITH_NAME_AND_EMAIL
//      resourceId = R_EARTH_REGISTER_DIALOG;
#else
# error You really should select an activation resource. 
#endif
       break; 
   case EUpgrade: 
      resourceId = R_WAYFINDER_UPGRADE_DIALOG;
      break; 
   } 
   iUpgradeDialog->ExecuteLD( resourceId );
}
#endif

void
CWayFinderAppUi::GotoMainMenu()
{
//   _LIT(KTestHeader, "Test Release");
   // _LIT(KTestMessage, "Entered through here..");

//   WFDialog::ShowScrollingDialogL( KTestHeader, KTestMessage, EFalse );

   GotoStartViewL();
}

void
CWayFinderAppUi::GotoLastView()
{
   TUid uid = TUid::Uid(LastViewId());
   TUid messageId;
   messageId.iUid = EActivateLastState;
   TBuf8<16> customMessage(_L8(""));
   
   SetViewL(uid, messageId, customMessage);
}

void
CWayFinderAppUi::ReportStartupFailed(
   class WFStartupEventCallbackParameter* param)
{
   HBufC* err = iCoeEnv->AllocReadResourceLC(R_WF_STARTUP_FAILED_BODY);
   HBufC* body = HBufC::NewLC(err->Length()+64);
   body->Des().Copy(*err);
   if (param) {
      body->Des().AppendNum(param->getType(), EHex);
      body->Des().Append(_L("-"));
      body->Des().AppendNum(param->getError(), EHex);
      body->Des().Append(_L("-"));
      body->Des().AppendNum(param->getRequestType(), EHex);
      delete param;
   } else {
      body->Des().Append(_L("0-0-0"));
   }

   typedef TCallBackEvent<CWayFinderAppUi, TWayfinderEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
            EWayfinderEventStartupFailedOk,
            EWayfinderEventStartupFailedCancel),
            R_WF_STARTUP_FAILED_HEADER,
            body,
            R_NON_BLOCKING_QUERY_MESSAGE);

   CleanupStack::Pop(body);
   CleanupStack::PopAndDestroy(err);
}

void 
CWayFinderAppUi::ReportStartupError(
   class WFStartupEventCallbackParameter* param)
{
   typedef TCallBackEvent<CWayFinderAppUi, TWayfinderEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t ;
   
   if (param->getError() == Nav2Error::NSC_EXPIRED_USER) { 
#ifdef SHOW_BUY_EXTENSION_ON_EXPIRED_USER
      // This is a special case, ask user if we should
      // launch external wap browser for buying an extension.
      cbd_t::RunDlgLD(cb_t(this,
                           EWayfinderEventBuyExtension,
                           EWayfinderEventShutdownNow),
                      R_WF_SHOW_BUY_EXTENSION_QUERY_HEADER,
                      R_WF_SHOW_BUY_EXTENSION_QUERY_TEXT,
                      R_NON_BLOCKING_QUERY_MESSAGE);
      return;
#endif
   }

   HBufC* body = WFTextUtil::Utf8ToHBufCL(param->getErrorMsg());
   CleanupStack::PushL(body);
   HBufC* errHeader = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_ERROR_MSG);
   HBufC* header = HBufC::NewLC(errHeader->Length() + 30);
   header->Des().Copy(*errHeader);
   header->Des().Append(_L(": "));
   header->Des().AppendNum(param->getError(), EHex);

   cbd_t::RunDlgLD(cb_t(this,
            EWayfinderEventDoNothing,
            EWayfinderEventDoNothing),
            header,
            body,
            R_NON_BLOCKING_INFO_MESSAGE);

   CleanupStack::Pop(header);
   CleanupStack::PopAndDestroy(errHeader);
   CleanupStack::Pop(body);
   delete param;
}

void CWayFinderAppUi::CheckFlightMode()
{
   GenerateEvent(EWayfinderEventCheckNetwork);
}

void CWayFinderAppUi::ShowUpgradeErrorL(class WFStartupEventCallbackParameter* param) 
{
   if (param->m_type == SP_UpgradeFailed) {
      WFStartupEventUpgradeFailedCallbackParameter* ufParam = 
         static_cast<WFStartupEventUpgradeFailedCallbackParameter*>(param);

      TInt resource = 0;
      if (!ufParam->m_keyStrOk) {
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_REG_KEY_MSG;
      } else if (!ufParam->m_phoneOk) {
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_NUMBER_MSG;
      } else if (!ufParam->m_nameOk) {      
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_NAME_MSG;
      } else if (!ufParam->m_emailOk) {
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_EMAIL_MSG;
      } else if (!ufParam->m_countryIdOk) {
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_REGION_MSG;;
      }
      if (resource != 0) {
         WFDialog::ShowErrorL(resource, iCoeEnv);
      }

      delete ufParam;
   }
}

void CWayFinderAppUi::ShutdownNow()
{
   TUid messageId;
   messageId.iUid = EShutdown;
   TBuf8<16> customMessage( _L8("") );

   SetViewL(KWelcomeViewId, messageId, customMessage);
}

void CWayFinderAppUi::RealShutdown()
{
   if (!iUrgentShutdown) {
      iUrgentShutdown = ETrue;
      GenerateEvent(EWayfinderEventShutdownNow);
   }
   
}

void CWayFinderAppUi::GotoSplashView()
{
/*    if (iStartupHandler->CanGotoSplashView()) { */
/*       ForceGotoSplashView(); */
/*    } */
}

#if 0
void CWayFinderAppUi::CloseUpgradeDialog( TBool aDoClose)
{
   if( iUpgradeDialog && aDoClose ){
      iUpgradeDialog->Close();
   }
   iUpgradeDialog = NULL;
   if (iUpgradeFromView == KMapViewId) {
      TUid messageId;
      /* Classic GUI. */
      messageId.iUid = ENoMessage;
      TBuf8<16> customMessage( _L8("") );
      SetViewL( KMapViewId, messageId, customMessage );
   } else {
      iTabGroup->SetActiveTabById( iUpgradeFromView.iUid );
      ActivateLocalViewL( iUpgradeFromView );
   }
}

void CWayFinderAppUi::SendUpgradeDetails(TUint32 aCountryId,
                                         const TDesC& aPhoneNumber,
                                         const TDesC& aRegNumber,
                                         const TDesC& aName,
                                         const TDesC& aEmail)
{
   char* phone = WFTextUtil::newTDesDupLC(aPhoneNumber);
   LOGNEWA(phone, char, strlen(phone) + 1);
   char* keystr = WFTextUtil::newTDesDupLC(aRegNumber);
   LOGNEWA(keystr, char, strlen(keystr) + 1);
   char* name = WFTextUtil::newTDesDupLC(aName);
   LOGNEWA(name, char, strlen(name) + 1);
   char* email = WFTextUtil::newTDesDupLC(aEmail);
   LOGNEWA(email, char, strlen(email) + 1);

   LicenseKeyMess key(keystr, phone, name, email, "", aCountryId);
   SendMessageL(&key);
   key.deleteMembers();

   LOGDELA(phone);
   LOGDELA(keystr);
   LOGDELA(name);
   LOGDELA(email);
   CleanupStack::PopAndDestroy(4, phone);//email, name, keystr, phone
}
#endif

void CWayFinderAppUi::StartSpreadWayfinderDialogsL()
{
   //Construct the dialog object and show the first dialog.
   if (!iSpreadWFDialogs) {
#if defined NAV2_CLIENT_SERIES60_V3
      iSpreadWFDialogs = CSpreadWFDialogs::NewL(iSmsHandler, KShareWFSmsUrl);
#else
      iSpreadWFDialogs = CSpreadWFDialogs::NewL(iCommunicateHelper, KShareWFSmsUrl);
#endif
   }
   iSpreadWFDialogs->StartDialogsL();
}

TBool
CWayFinderAppUi::AcceptGPRSNoticeL()
{
#if SHOW_GPRS_NOTICE
   TBool accept = EFalse;
   HBufC* title;
   title = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_GPRS_NOTICE_TITLE );

   HBufC* text;
   text = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_GPRS_NOTICE_TEXT );

   CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *text );
   CleanupStack::PushL(dlg);
   dlg->PrepareLC( R_WAYFINDER_LEGAL_NOTICE_DIALOG );
   CAknPopupHeadingPane *header = dlg->QueryHeading();
   header->SetLayout(CAknPopupHeadingPane::EQueryHeadingPane );
   header->SetTextL( *title );

   CleanupStack::Pop(dlg);
   accept = ( dlg->RunLD() == EAknSoftkeyYes );

   CleanupStack::PopAndDestroy(text);
   CleanupStack::PopAndDestroy(title);

   return accept;
#else
   return ETrue;
#endif
}

#if 0
TBool CWayFinderAppUi::AcceptLegalNoticeL()
{
   HBufC* title = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_NOTICE_TITLE );
   HBufC* filename = HBufC::NewLC(KMaxFileName);
   {
      _LIT(KUserTermsFilenameFormat, "%Suser_terms_%S.txt");
      HBufC* languageCode = 
         iCoeEnv->AllocReadResourceLC(R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE);
      TPtrC path =iPathManager->GetWayfinderPath();
      filename->Des().Format(KUserTermsFilenameFormat, &path, languageCode);
//       TPtr fn_ptr = filename->Des();
//       fn_ptr.Copy(path);
//       fn_ptr.Append( _L("user_terms_") );
//       fn_ptr.Append( *languageCode );
//       fn_ptr.Append( _L(".txt") );
      CleanupStack::PopAndDestroy(languageCode);
   }

#define LEGAL_AGREEMENT_UTF8
#ifndef LEGAL_AGREEMENT_UTF8
   TBool accept = EFalse;

   HBufC* message = HBufC::NewL( 30000 );
   LOGNEW(message, HBufC);

   TInt errorCode = KErrNone;
   TPtr dataPtr = message->Des();
   TBool res = WFSymbianUtil::getBytesFromFile( m_fsSession, filename,
                                                dataPtr, errorCode);
   if( res ){
      CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *message );
      CleanupStack::PushL(dlg);
      dlg->PrepareLC( R_WAYFINDER_LEGAL_NOTICE_DIALOG );
      CAknPopupHeadingPane *header = dlg->QueryHeading();
      header->SetLayout(CAknPopupHeadingPane::EQueryHeadingPane );
      header->SetTextL( *title );

      CleanupStack::Pop(dlg);
      accept = ( dlg->RunLD() == EAknSoftkeyYes );
   } else {
      WFDialog::ShowScrollingWarningDialogL( R_WAYFINDER_PROGRAMMISSING_MSG, iCoeEnv );
      HandleCommandL( EEikCmdExit );
   }
   LOGDEL(message);
   delete message;
#else
   HBufC* agreement = WFSymbianUtil::ReadUtf8FileLC(iCoeEnv->FsSession(), 
                                                    *filename);
   class CAknMessageQueryDialog* dlg = 
      CAknMessageQueryDialog::NewL(*agreement);
   CleanupStack::PushL(dlg);
   dlg->PrepareLC( R_WAYFINDER_LEGAL_NOTICE_DIALOG );

   class CAknPopupHeadingPane *header = dlg->QueryHeading();
   header->SetLayout(CAknPopupHeadingPane::EQueryHeadingPane);
   header->SetTextL( *title );

   CleanupStack::Pop(dlg);
   TBool accept = ( dlg->RunLD() == EAknSoftkeyYes );

   CleanupStack::PopAndDestroy(agreement);
#endif
   CleanupStack::PopAndDestroy(filename);
   CleanupStack::PopAndDestroy(title);

   return accept;
}
#endif

TBool CWayFinderAppUi::AcceptPrivacyStatementL()
{
#ifdef SHOW_PRIVACY_STATEMENT
   HBufC* title = iCoeEnv->AllocReadResourceL( R_PRIVACE_STATEMENT_TITLE );
   HBufC* text  = iCoeEnv->AllocReadResourceL( R_PRIVACY_STATEMENT_TEXT );

   TInt buttonId = CQueryDlg::RunDlgLD(R_WAYFINDER_LEGAL_NOTICE_DIALOG,
                                       *title, *text);

   return buttonId == EAknSoftkeyYes;   
#else
   return ETrue;
#endif
}

int32
CWayFinderAppUi::GetIAP()
{
   return iDataStore->iIAPDataStore->m_iapId;
}

void
CWayFinderAppUi::ReportProgressIAP(TInt aVal, TInt aMax, HBufC* aName)
{
   if (iIAPSearcher->Searching()) {
      if (!iProgressDlg) {
         iProgressDlg = CProgressDlg::NewL();
         iProgressDlg->SetProgressDlgObserver(this);
         iProgressDlg->StartProgressDlgL(R_PROGRESS_DIALOG, 
                                         R_WAYFINDER_IAP_SEARCH1_MSG);
      }
      iProgressDlg->ReportProgress(aVal, aMax, aName);
   } else {
      if (iProgressDlg) {
         delete iProgressDlg;
         iProgressDlg = NULL;
      }
   }
}

TBool
CWayFinderAppUi::ShowIAPMenu()
{

   /* Check if we have a saved IAP from the installer. */
   HBufC* dir = iPathManager->GetWayfinderPath().AllocLC();
   _LIT(KIAPFile, "iap.txt");
   HBufC* file = HBufC::NewLC(dir->Length() + KIAPFile().Length());
   file->Des().Copy(*dir);
   file->Des().Append(KIAPFile);

   class CIAPObject* iap = CIAPObject::NewLC();
   if( IAPFileHandler::ReadIAPFile(*file, iap) == KErrNone ) { 
      /* IAP was saved, set it instead of -1! */
/* #ifdef NAV2_CLIENT_SERIES60_V3 */
      HBufC* imei = WFTextUtil::AllocLC(iImeiNbr);
/* #else */
/*       HBufC* imei = WFTextUtil::AllocLC(GetIMEI()); */
/* #endif */
      if (!iap->GetImeiNbr().Compare(*imei)) {
         /* Same phone, use the IAP in the file. */
         TLex lexer (iap->GetIAPNbr());
         TInt32 iapid = -1;
         if (lexer.Val(iapid) != KErrNone) {
            /* Overflow or other error. Fudge it. */
         }
         iDataStore->iIAPDataStore->SetIap(iapid);
      }
      CleanupStack::PopAndDestroy(imei);
   }
   CleanupStack::PopAndDestroy(iap);
   CleanupStack::PopAndDestroy(file);
   CleanupStack::PopAndDestroy(dir);

   if (GetIAP() >= 0) {
      SendIAP(GetIAP(), ETrue, EFalse);
      return EFalse;
   }

#if !defined NAV2_CLIENT_SERIES60_V1
   TBool retval = EFalse;
 #ifdef SHOW_CHOOSE_IAP_SEARCH_METHOD
   /* Show choice for finding IAP. */
   CDesCArrayFlat* descArray = new (ELeave) CDesCArrayFlat(2);

   LOGNEW(descArray, CDesCArrayFlat(2));

   CleanupStack::PushL( descArray );

   HBufC* title = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_QUERY_IAP_AUTO);
   descArray->AppendL(*title);
   CleanupStack::PopAndDestroy(title);

   title = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_QUERY_IAP_MANUAL);

   descArray->AppendL(*title);
   CleanupStack::PopAndDestroy(title);

   title = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_QUERY_IAP_HEADER);

   TBool okChosen = EFalse;

   TInt selection = PopUpList::ShowPopupListL(
         R_WAYFINDER_SOFTKEYS_OK_CANCEL, 
         *descArray, okChosen, 0, 
         ETrue,
         &(*title) );
   CleanupStack::PopAndDestroy(title);
   LOGDEL(descArray);
   CleanupStack::PopAndDestroy(descArray);

   if (okChosen) {
      /* Selected one of the choices. */
      CheckIAPChosen(selection);
      retval = ETrue;
   } else {
      /* User cancelled. */
      ReportFailureIAP(ErrorIAPNotSelected);
   }
 #else
   CheckIAPChosen(0);
 #endif
   return retval;
#else
   if (!iIAPSearcher) {
      iIAPSearcher = new (ELeave) CIAPSearcher_old(this);
   }
   TBool res = iIAPSearcher->SelectManual();
   if (!res) {
      /* User cancelled. */
      m_startupHandler->StartupEvent(SS_IapSearchNotOk);
   }
   return res;
#endif
}

void
CWayFinderAppUi::CheckIAPChosen(TInt ch)
{
   if (!iIAPSearcher) {
      iIAPSearcher = new (ELeave) CIAPSearcher_old(this);
   }
   if ( ch ) {
      /* Manual select. */
      iIAPSearcher->SelectManual();
   } else {
      /* Auto select. */
      iIAPSearcher->StartSearch();
   }
}

void
CWayFinderAppUi::ReportFailureIAP(TInt error, TBool temporary)
{
   /* Failed to find a working access point. */
   /* Inform user. */
   HBufC* header;
   HBufC* text;

   SetWait( EFalse, 0 );
   iSynchronizing = EFalse;

   header = CEikonEnv::Static()->AllocReadResourceLC( R_WAYFINDER_IAP_FAIL_HEADER );
   TInt res = R_BIO_CTRL_UNKNOWN_TYPE;
   if (error == ErrorNoWorkingIAPs) {
      res = R_WAYFINDER_IAP_FAIL_NO_WORKING_IAPS;
   } else if (error == ErrorNoIAPs) {
      res = R_WAYFINDER_IAP_FAIL_NO_IAPS;
   } else if (error == ErrorIAPNotSelected) {
      res = R_WAYFINDER_IAP_FAIL_NOT_SELECTED;
   } else if (error == ErrorIAPNotWorking) {
      res = R_WAYFINDER_IAP_FAIL_NOT_WORKING;
   }
   text = CEikonEnv::Static()->AllocReadResourceLC( res );
   WFDialog::ShowScrollingDialogL(*header, *text, ETrue);
   CleanupStack::PopAndDestroy(text);
   CleanupStack::PopAndDestroy(header);

   SendIAP(-1);

   if (!temporary) {
      /* IAP failed. */
      m_startupHandler->StartupEvent(SS_IapSearchNotOk);
   }
}

void
CWayFinderAppUi::SetIAP(TInt aIAP)
{
#ifndef MOCKUP
   // TRACE_FUNC1("SetIAP");
   iSettingsView->SetIAP(aIAP);

   iDataStore->iIAPDataStore->SetIap(aIAP);
   if (aIAP >= 0) {
      UpdateConnectionManagerL();
   } 
   /* Set the session based IAP to the same. */
   SimpleParameterMess message(GuiProtEnums::paramSelectedAccessPointId2, 
         uint32(GetIAP()));
   SendMessageL( &message );
   message.deleteMembers();
#endif
}

void
CWayFinderAppUi::SendIAP(int32 aIAPid, TBool isReal, TBool aShow)
{
   SimpleParameterMess *message;
#if 0
   TBuf<120> body;
   TBuf<12> header;
   header.Copy(_L("SendIAP"));
   body.Copy(_L("IAP name"));
   if (iIAPSearcher) {
      HBufC* name = iIAPSearcher->GetIAPName(); /* IAPSearcher owns name. */
      if (name) {
         body.Append( _L(":\n"));
         body.Append(*name);
         name = NULL; /* We don't own the name, IAPSearcher does. */
      }
   }
   WFDialog::ShowScrollingDialogL(header, body, EFalse);
#endif
   if (isReal) {
//    TBuf<50> tmpIap;
//    tmpIap.Format(_L("iap: %d"), aIAPid);
//    CEikonEnv::Static()->InfoWinL(_L("CWayfinderAppUi::SendIap, 1"), tmpIap);
       /* This is the real IAP setting. Store it permanently. */
      iDataStore->iIAPDataStore->SetIap(aIAPid);
      iSettingsView->SetIAP(aIAPid);
      UpdateConnectionManagerL();
      /* IAP search OK. */
 
      if (aShow) {
         HBufC* header;
         HBufC* tmp;
         HBufC* body;
         header = CEikonEnv::Static()->AllocReadResourceLC( R_WAYFINDER_IAP_SUCCESS_HEADER );
         tmp = CEikonEnv::Static()->AllocReadResourceLC( R_WAYFINDER_IAP_SUCCESS_WORKING );
         HBufC* name = NULL;
         if (iIAPSearcher) {
            name = iIAPSearcher->GetIAPName(); /* IAPSearcher owns name. */
         }
         if (name) {
            body = HBufC::NewLC( tmp->Des().Length() + name->Des().Length() + 10 );
            body->Des().Copy(*tmp);
            body->Des().Append( _L(":\n"));
            body->Des().Append(*name);
            name = NULL; /* We don't own the name, IAPSearcher does. */
         } else {
            body = HBufC::NewLC( tmp->Des().Length() + 10);
            body->Des().Copy(*tmp);
         }
         WFDialog::ShowScrollingDialogL(*header, *body, EFalse);
         CleanupStack::PopAndDestroy(body);
         CleanupStack::PopAndDestroy(tmp);
         CleanupStack::PopAndDestroy(header);
      }

      message = new (ELeave) SimpleParameterMess(
         GuiProtEnums::paramSelectedAccessPointId, (uint32)GetIAP() );
      LOGNEW(message, SimpleParameterMess);
      SendMessageL( message );
      message->deleteMembers();
      LOGDEL(message);
      delete message;

      m_startupHandler->StartupEvent(SS_IapSearchOk);
   }
   TRACE_FUNC1("SendIap");

//    TBuf<50> tmp;
//    tmp.Format(_L("iap: %d"), aIAPid);
//    CEikonEnv::Static()->InfoWinL(_L("CWayfinderAppUi::SendIap, 2"), tmp);
   /* Set session based IAP parameter to the correct value. */
   message = new (ELeave) SimpleParameterMess(
      GuiProtEnums::paramSelectedAccessPointId2, (uint32)aIAPid );
   LOGNEW(message, SimpleParameterMess);
   SendMessageL( message );
   message->deleteMembers();
   LOGDEL(message);
   delete message;
}

void
CWayFinderAppUi::SendSyncParameters()
{
   GenericGuiMess sp(GuiProtEnums::PARAMETERS_SYNC);
   SendMessageL(&sp); 
}
void
CWayFinderAppUi::SendSyncFavorites()
{
   GenericGuiMess sync(GuiProtEnums::SYNC_FAVORITES);
   SendMessageL(&sync);
}

TBool
CWayFinderAppUi::CanChangeMapType()
{
   return EFalse;
}

TBool
CWayFinderAppUi::CanUseSubmitCode()
{
#ifdef HIDE_SUBMIT_CODE
   return EFalse;
#else
   return ETrue;
#endif
}

TBool
CWayFinderAppUi::CanShowWebUsername()
{
#ifdef HIDE_WEB_USERNAME
   return EFalse;
#else
   return ETrue;
#endif
}

TBool
CWayFinderAppUi::CanShowWebPassword()
{
#ifdef HIDE_WEB_USERNAME
   return EFalse;
#else
#if defined HIDE_WEB_PASSWORD
   return EFalse;
#else
   return ETrue;
#endif
#endif
}

TBool
CWayFinderAppUi::EarthShowGPSNoticeL()
{
/*
   TBool accept = EFalse;
   HBufC* title;
   title = iCoeEnv->AllocReadResourceLC( R_MAP_INFO_TITLE );

   HBufC* text;
   text = iCoeEnv->AllocReadResourceLC( R_EARTH_UPGRADE_GPS );

   CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *text );
   CleanupStack::PushL(dlg);
   dlg->PrepareLC( R_MESSAGE_DIALOG );
   CAknPopupHeadingPane *header = dlg->QueryHeading();
   header->SetLayout(CAknPopupHeadingPane::EQueryHeadingPane );
   header->SetTextL( *title );

   CleanupStack::Pop(dlg);
   accept = ( dlg->RunLD() == EAknSoftkeyOk );

   CleanupStack::PopAndDestroy(text);
   CleanupStack::PopAndDestroy(title);
*/
   return EFalse;
}

TBool
CWayFinderAppUi::EarthAllowGps()
{
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Iron && 
       !GetUserRight()->hasUserRight(GuiProtEnums::UR_USE_GPS)) {
      return EFalse;
   } else {
      return ETrue;
   }
}

TBool
CWayFinderAppUi::HaveVoiceInstructions()
{
   if (iPathManager->GetLangResourcePath() == KNullDesC) {
      return EFalse;
   } else {
      return ETrue;
   }
}

TBool
CWayFinderAppUi::CanUseGPSForEveryThing()
{
#ifndef TRIAL_IS_MAPGUIDE
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Trial) return ETrue;
#endif
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Gold) return ETrue;
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Lithium) return ETrue;
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Iron && 
       GetUserRight()->hasUserRight(GuiProtEnums::UR_USE_GPS)) return ETrue;
   return EFalse;
}

TBool
CWayFinderAppUi::IsGpsConnected()
{
   TBool isConnected = EFalse;
   if(iBtHandler && (iBtHandler->IsConnected() || iBtHandler->IsBusy())) {
      isConnected = ETrue;
#ifdef NOKIALBS
   } else if ( iPositionProvider) {
      isConnected = iPositionProvider->IsConnected() && iGpsQuality >= QualityUseless;
#endif
   } else if( iGpsQuality >= QualityUseless ){
      isConnected = ETrue;
   }
   return isConnected;
}

TBool
CWayFinderAppUi::IsGpsConnectedAndNotSimulating()
{
   TBool isConnected = EFalse;
   if(iBtHandler && (iBtHandler->IsConnected() || iBtHandler->IsBusy())) {
      isConnected = ETrue;
#ifdef NOKIALBS
   } else if ( iPositionProvider) {
      isConnected = iPositionProvider->IsConnected() && iGpsQuality >= QualityUseless;
#endif
   } else if( IsSimulating() ) {
      isConnected = EFalse;
   } else if( iGpsQuality >= QualityUseless ) {
      isConnected = ETrue;
   }
   return isConnected;
}

TBool
CWayFinderAppUi::IsGpsConnectedOrSearching()
{
   TBool isConnected = EFalse;
   if(iBtHandler && (iBtHandler->IsConnected() || iBtHandler->IsBusy())) {
      isConnected = ETrue;
#ifdef NOKIALBS
   } else if ( iPositionProvider) {
      isConnected = iPositionProvider->IsConnected();
#endif
   } else if( iGpsQuality >= QualityUseless ){
      isConnected = ETrue;
   }
   return isConnected;
}

TBool
CWayFinderAppUi::IsGpsAllowed()
{
   if (IsGpsConnected() && CanUseGPSForEveryThing()) {
      return ETrue;
   } else {
      return EFalse;
   }
}

TBool
CWayFinderAppUi::IsFullVersion()
{
#ifndef TRIAL_IS_MAPGUIDE
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Trial) return ETrue;
#endif
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Gold) return ETrue;
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Lithium) return ETrue;
   return EFalse;
}
TBool
CWayFinderAppUi::AutoRouteSmsDestination()
{
   if (CanUseGPSForEveryThing()) {
      return iAutoRouteSmsDestination;
   } else {
      return EFalse;
   }
}
TBool CWayFinderAppUi::ShowUpgradeInfoCba()
{
#if defined (HIDE_UPGRADE_IN_CBA)
   return EFalse;
#else
#if defined (HIDE_UPGRADE_IN_CBA_GOLD)
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Gold) return EFalse;
#else
   return ShowUpgradeInfo();
#endif
#endif
}
TBool CWayFinderAppUi::ShowUpgradeInfo()
{
#ifdef SHOW_UPGRADE_ALWAYS
   return ETrue;
#endif
#ifdef HIDE_UPGRADE_IN_TRIAL
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Trial) return EFalse;
#endif
#ifdef HIDE_UPGRADE_IN_SILVER
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Silver) return EFalse;
#endif
#ifdef HIDE_UPGRADE_IN_GOLD
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Gold) return EFalse;
#endif
#ifdef HIDE_UPGRADE_IN_LITHIUM
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Lithium) return EFalse;
#endif
   return ETrue;
}

TBool CWayFinderAppUi::IsGoldVersion()
{
   return (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Gold);
}

TBool CWayFinderAppUi::IsTrialVersion()
{
   return (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Trial);
}

TBool CWayFinderAppUi::IsIronVersion()
{
   return (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Iron);
}

TBool
CWayFinderAppUi::UseTrackingOnAuto()
{
   if (iSettingsData->iAutoTrackingOn && CanUseGPSForEveryThing()) {
      return ETrue;
   } else {
      return EFalse;
   }
}

TBool 
CWayFinderAppUi::ShowLangsWithoutVoice()
{
#ifdef SHOW_LANGS_WITHOUT_VOICE
   return ETrue;
#else 
   return EFalse;
#endif
}

TBool 
CWayFinderAppUi::ShowLangsWithoutResource()
{
#ifdef SHOW_LANGS_WITHOUT_RESOURCES
   return ETrue;
#else 
   return EFalse;
#endif
}

TBool
CWayFinderAppUi::HideServicesInfo()
{
#ifdef HIDE_SERVICES_INFO
   return ETrue;
#else
   return EFalse;
#endif
}

TBool
CWayFinderAppUi::showTracking() const {
#ifdef USE_TRACKING
   // Check UserRights for UR_POSITIONING and return that
   if ( iSettingsData->iUserRights->hasUserRight(
           GuiProtEnums::UR_POSITIONING ) ) 
   {
      return ETrue;
   } else {
      return EFalse;
   }
#else
   return EFalse;
#endif
}


TBool
CWayFinderAppUi::hasFleetRight() const {
   return GetUserRight()->hasUserRight( GuiProtEnums::UR_FLEET );
}


const UserRights*
CWayFinderAppUi::GetUserRight() const {
   return iSettingsData->iUserRights;
}


void
CWayFinderAppUi::SaveMapLayerSettings()
{
   iMapView->SaveMapLayerSettings(iSettingsData);
}

void
CWayFinderAppUi::HandleWayfinderType(GuiProtEnums::WayfinderType wft)
{
   switch(wft){
      case GuiProtEnums::Trial:
      case GuiProtEnums::Silver:
      case GuiProtEnums::Gold:
      case GuiProtEnums::Iron:
      case GuiProtEnums::Lithium:
         SetWayfinderType(wft);
         break;
      case GuiProtEnums::InvalidWayfinderType:
         break;
   }
}

void
CWayFinderAppUi::SetWayfinderType(GuiProtEnums::WayfinderType wft)
{
   iDataStore->iWFAccountData->setWfType(wft);
   int32 startupEvent = SS_TrialMode;
   switch (wft) {
      case GuiProtEnums::Trial:
         break;
      case GuiProtEnums::Silver:
         startupEvent = SS_SilverMode;
         break;
      case GuiProtEnums::Gold:
         startupEvent = SS_GoldMode;
         break;
      case GuiProtEnums::Iron:
         startupEvent = SS_IronMode;
         break;
      case GuiProtEnums::Lithium:
         startupEvent = SS_LithiumMode;
         break;
      case GuiProtEnums::InvalidWayfinderType:
         // Unhandled type - what to do?
         break;
   }
   m_startupHandler->StartupEvent(startupEvent);
}

TBool CWayFinderAppUi::IsReleaseVersion()
{
   return ::IsReleaseVersion();
}

TBool IsReleaseVersion()
{
#ifdef __RELEASE__
   return ETrue;
#else
   return EFalse;
#endif
}

TBool CWayFinderAppUi::IsOnlyOneLanguage()
{
#ifdef USE_ONLY_ONE_LANGUAGE
   return ETrue;
#else
   return EFalse;
#endif
}

TBool CWayFinderAppUi::ShowMoveWayfinder()
{
#ifdef SHOW_MOVEWAYFINDER
   return WayfinderHelper::ShowMoveWayfinder();
#endif
   return EFalse;
}

TBool CWayFinderAppUi::ShowGetWayfinder()
{
#ifdef SHOW_MOVEWAYFINDER
   return WayfinderHelper::ShowGetWayfinder();
#endif
   return EFalse;
}

TBool CWayFinderAppUi::CheckWait()
{
   if( iWaiting ){
      iContextPaneAnimator->IncCurrentFrame();
      SetContextIconL();

      CAnimatorFrame* frame = new CAnimatorFrame();
#if defined NAV2_CLIENT_SERIES60_V3
      frame->CopyFrameL(iContextPaneAnimator->GetSvgAnimationFrameL());
#else
      frame->CopyFrameL(iContextPaneAnimator->GetAnimationFrameL());
#endif
      LOGNEW(frame, CAnimatorFrame);
      if (frame) {
         CFbsBitmap* a = frame->GetBitmap();
         CFbsBitmap* b = frame->GetMask();

         frame->DisownBitmaps();
         delete frame;
         LOGDEL(frame);

         if (!iMapView->SetConStatusImage(a, b)) {
            if (!iWelcomeView->SetConStatusImage(a, b)) {
               /* Not used. */
               delete a;
               delete b;
            }
         }
      }
      iTimer->After(KRefresh);
      return ETrue;
   }
   return EFalse;
}

void CWayFinderAppUi::RequestSettingsL()
{
   RequestSimpleParameterL(GuiProtEnums::paramWebUsername);
   RequestSimpleParameterL(GuiProtEnums::paramSoundVolume);
   RequestSimpleParameterL(GuiProtEnums::paramUseSpeaker);
   RequestSimpleParameterL(GuiProtEnums::paramTurnSoundsLevel);
   RequestSimpleParameterL(GuiProtEnums::paramAutoReroute);
   RequestSimpleParameterL(GuiProtEnums::paramStoreSMSDestInMyDest);
   RequestSimpleParameterL(GuiProtEnums::paramAutomaticRouteOnSMSDest);
   RequestSimpleParameterL(GuiProtEnums::paramKeepSMSDestInInbox);
   RequestSimpleParameterL(GuiProtEnums::paramBacklightStrategy);
   RequestSimpleParameterL(GuiProtEnums::paramServerNameAndPort);
   RequestSimpleParameterL(GuiProtEnums::paramTransportationType);
   RequestSimpleParameterL(GuiProtEnums::paramDistanceMode);
   GetTopRegionsL();
   RequestSimpleParameterL(GuiProtEnums::paramUsername);
   RequestSimpleParameterL(GuiProtEnums::paramPassword);
   RequestSimpleParameterL(GuiProtEnums::paramMuteTurnSound);

   //Temporarily disabled to see if it helps against locked startup
   RequestSimpleParameterL(GuiProtEnums::userDirectionMode);
   RequestSimpleParameterL(GuiProtEnums::userVicinityFeedMode);

   RequestSimpleParameterL(GuiProtEnums::paramLockedNavWaitPeriod);
   RequestSimpleParameterL(GuiProtEnums::paramFeedWaitPeriod);
   RequestSimpleParameterL(GuiProtEnums::paramGpsMinWalkingSpeed);
   RequestSimpleParameterL(GuiProtEnums::paramVicinityListUpdatePeriod);
}

void
CWayFinderAppUi::ShowAboutL()
{
   TBuf<2048> * text = new(ELeave) TBuf<2048>( _L("") );
   CleanupStack::PushL(text);

   text->AppendNum( isab::Nav2_Major_rel );
   text->Append(_L("."));
   if (isab::Nav2_Minor_rel < 10) {
      text->Append(_L("0"));
   }
   text->AppendNum( isab::Nav2_Minor_rel );
#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
   text->Append(_L(".")); 
   text->AppendNum( isab::Nav2_Build_id );
#endif
   text->Append(_L("\n"));

   // Test if we're trial version, don't show username/password in trial.
   if (CanShowWebUsername()) {
      if (IsTrialVersion()) {
         TBuf<KBuf64Length> buf;
         iCoeEnv->ReadResource( buf, R_WAYFINDER_TRIAL_VERSION_TEXT );
         text->Append( buf );
      } else {
         TBuf<KBuf64Length> buf;
         iCoeEnv->ReadResource( buf, R_WAYFINDER_WEB_USERNAME_TEXT );
         text->Append( buf );
         text->Append( _L(": "));
         if (iDataStore->iWFAccountData->getWebUserName()) {
            HBufC* str = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getWebUserName());
            text->Append( *str );
            CleanupStack::PopAndDestroy(str);
         }
         text->Append( _L("\n"));
      }
      text->Append(_L("\n"));
   }

   // Add Extra text in about box.
   HBufC* tmp = iCoeEnv->AllocReadResourceL(R_WAYFINDER_ABOUT_EXTRA_MSG );
   text->Append( *tmp );
   delete tmp;

   TBuf<KBuf32Length> temp;
   temp.Zero();
#if !defined(__RELEASE__)
   text->Append(_L("\n\nWayfinder "));
   text->AppendNum( isab::Nav2_Major_rel );
   text->Append( _L(".") );
   text->AppendNum( isab::Nav2_Minor_rel );
   text->Append( _L(" Bld: ") );
   text->AppendNum( isab::Nav2_Build_id );
   text->Append( _L("\n") );
   WFTextUtil::char2TDes( temp, isab::Nav2_Build_time );
   text->Append( temp );
   text->Append( _L(" by: ") );
   WFTextUtil::char2TDes( temp, isab::Nav2_Build_user );
   text->Append( temp );
   text->Append( _L("@") );
   WFTextUtil::char2TDes( temp, isab::Nav2_Build_host );
   text->Append( temp );
   temp.Zero();

   text->Append(_L("\nHardware id: "));

   if (iImeiNbr) {
      HBufC* tmpImei = WFTextUtil::AllocLC(iImeiNbr);
      text->Append(*tmpImei);
      CleanupStack::PopAndDestroy(tmpImei);
   }

   text->Append( _L("\nWebusername: "));
   if (iDataStore->iWFAccountData->getWebUserName()) {
      HBufC* str = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getWebUserName());
      text->Append( *str );
      CleanupStack::PopAndDestroy(str);
   }
   text->Append( _L("\nUsername: "));
   if (iDataStore->iWFAccountData->getUserName()) {
      HBufC* str = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getUserName());
      text->Append(*str);
      CleanupStack::PopAndDestroy(str);
   }
   text->Append( _L("\nWftype: "));
   text->Append( *iWfTypeStr );
   text->Append( _L("\nWfopts: "));
   text->Append( *iWfOptsStr );
#endif

#ifdef DEBUG_CELL
   //// code /////
   TInt aMcc = 0;
   TInt aMnc = 0;
   TInt aLocation = 0;
   TInt aCellId = 0;

   TInt enumphone = 1;
   RTelServer server;
   RBasicGsmPhone phone;
   RTelServer::TPhoneInfo info;
   MBasicGsmPhoneNetwork::TCurrentNetworkInfo NetworkInfo;

   if (server.Connect() == KErrNone) {
      CleanupClosePushL(server);
      if (server.LoadPhoneModule(_L("Phonetsy.tsy")) == KErrNone) {
         if (server.EnumeratePhones(enumphone) == KErrNone) {
            if (enumphone >= 1){
               if (server.GetPhoneInfo(0, info) == KErrNone) {
                  if (phone.Open(server, info.iName) == KErrNone) {
                     CleanupClosePushL(phone);
                     if (phone.GetCurrentNetworkInfo(NetworkInfo) == KErrNone) {
                        aMcc = NetworkInfo.iNetworkInfo.iId.iMCC;
                        aMnc = NetworkInfo.iNetworkInfo.iId.iMNC;
                        aLocation = NetworkInfo.iLocationAreaCode;
                        aCellId = NetworkInfo.iCellId;
                        text->Append( _L("\nMCC: "));
                        text->AppendNum(aMcc, EDecimal);
                        text->Append( _L("\nMNC: "));
                        text->AppendNum(aMnc, EDecimal);
                        text->Append( _L("\nLoc: "));
                        text->AppendNum(aLocation, EDecimal);
                        text->Append( _L("\nCell: "));
                        text->AppendNum(aCellId, EDecimal);
                     } else {
                        text->Append( _L("\nFail GetCurrentNetworkInfo") );
                     }
                     CleanupStack::PopAndDestroy(1); // phone
                  } else {
                     text->Append( _L("\nFail phone.Open") );
                  }
               } else {
                  text->Append( _L("\nFail server.getphone") );
               }
            } else {
               text->Append( _L("\nFail enumphone") );
            }
         } else {
            text->Append( _L("\nFail enumerate phones") );
         }
      } else {
         text->Append( _L("\nFail KTsyName") );
      }
      CleanupStack::PopAndDestroy(1); // server
   } else {
      text->Append( _L("\nFail server connect") );
   }
#endif
   
   temp.Copy( KApplicationVisibleName );

   //   iCoeEnv->ReadResource( temp, R_WAYFINDER_ABOUT_MSG );
   WFDialog::ShowScrollingDialogL(temp, *text, EFalse);
   CleanupStack::PopAndDestroy(text);
}


void
CWayFinderAppUi::ShowFleetInfoL()
{
   HBufC* msg = iCoeEnv->AllocReadResourceL( R_WAYFINDER_FLEET_INFO_MSG );
   CleanupStack::PushL( msg );

   HBufC* label = iCoeEnv->AllocReadResourceL( 
      R_WAYFINDER_FLEET_INFO_LABEL );
   CleanupStack::PushL( label );

   WFDialog::ShowScrollingDialogL( label->Des(), msg->Des(), EFalse );
   CleanupStack::PopAndDestroy( label );
   CleanupStack::PopAndDestroy( msg );
}


TBool CWayFinderAppUi::CheckSilverSMSQueryL()
{
#if defined (_MSC_VER) || defined (DONT_SEND_SMS) 
   //GenerateEvent(EWayfinderEventSendRegistrationSmsOk);
   return ETrue;
#else
   HBufC* smsNum;
   TBool accept = EFalse;

   {
      HBufC* title;
      TBuf<256> message;

      title    = CEikonEnv::Static()->AllocReadResourceLC( R_WAYFINDER_SEND_SMS_REG_TITLE );
      CEikonEnv::Static()->ReadResource(message, R_WAYFINDER_SEND_SMS_REG_MSG );
      smsNum   = CEikonEnv::Static()->AllocReadResourceL( R_WAYFINDER_SMS_REG_NUMBER );
      message.Append( *smsNum );

      CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( message );
      CleanupStack::PushL(dlg);
      dlg->PrepareLC( R_WAYFINDER_LEGAL_NOTICE_DIALOG );
      CAknPopupHeadingPane *header = dlg->QueryHeading();
      header->SetLayout(CAknPopupHeadingPane::EQueryHeadingPane );
      header->SetTextL( *title );
      CleanupStack::Pop(dlg);
      accept = ( dlg->RunLD() == EAknSoftkeyYes );

      CleanupStack::PopAndDestroy(title);
   }
   HBufC* sms = NULL;
   if (accept) {
      sms = ActivationSms::FormatLC(iDataStore->iWFAccountData->getWfType(), *iWfTypeStr, *iWfOptsStr, 
                                    isab::Nav2_Major_rel, isab::Nav2_Minor_rel,
                                    isab::Nav2_Build_id, 
                                    R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE, iImeiNbr);
      CleanupStack::Pop(sms); //this is just plain stupid, but I don't
      //care to rewrite this code right now.
   }
   if (sms) {
# if defined NAV2_CLIENT_SERIES60_V3
      if (!iSmsHandler) {
         iSmsHandler = SmsHandlerFactory::CreateL( this );
      }
      iSmsHandler->SendSmsL(*sms, *smsNum); 
# else
      /* Got it. */
      iCommunicateHelper->SendSMSMessageL( *smsNum, NULL, NULL, *sms );
# endif
      delete smsNum;
      delete sms;
      return ETrue;
   } else {
      return EFalse;
   }
#endif
}

void
CWayFinderAppUi::GetRouteBoundingBox( TPoint &aTopLeft, TPoint &aBottomRight )
{
   aTopLeft = iRouteBoxTl;
   aBottomRight = iRouteBoxBr;
}

TBool
CWayFinderAppUi::DontHandleAsterisk()
{
#ifdef USE_ASTERISK_AS_REPORT_EVENT
   return ETrue;
#else
   return EFalse;
#endif
}

void
CWayFinderAppUi::HandleReportEventMenu(TInt command)
{  
   TInt32 commands[] = {
      EWayFinderCmdReportTrafficInfoCenter,
      EWayFinderCmdReportTrafficJam,
      EWayFinderCmdReportSpeedCam,
      EWayFinderCmdReportDangerousPlace,
      EWayFinderCmdReportBreakdown1,
      EWayFinderCmdReportBreakdown2,
      EWayFinderCmdReportWeatherWarning,
/*       EWayFinderCmdReportRouteProblem, */
      EWayFinderCmdReportMapProblem,
      EWayFinderCmdReportProblem,
      EWayFinderCmdReportPosition,
/*       EWayFinderCmdReportEvent, */
   };    
   TInt32 elem = (sizeof(commands)/sizeof(*commands));
   if (command < 0 || command >= elem) {
      return;
   }
   HandleCommandL(commands[command]);
}

void
CWayFinderAppUi::ShowReportEventMenu()
{
#if defined SEND_DEBUG_SMS_ON_ERRORS || defined USE_REPORT_EVENT_MENU
   TInt32 texts[] = {
      R_WF_TRAFFIC_SPECIAL_EVENT,
      R_WF_TRAFFIC_JAM_REPORT,
      R_WF_SPEED_CAM,
      R_WF_TRAFFIC_DANGEROUS_PLACE,                       
      R_WF_BREAKDOWN1,
      R_WF_BREAKDOWN2,
      R_WF_TRAFFIC_WEATHER_WARNING,
      R_WF_REPORT_MAP_PROBLEM,
/*       R_WF_REPORT_ROUTE_PROBLEM, */
      R_WF_REPORT_PROBLEM,
      R_WF_REPORT_POSITION,
/*       R_WAYFINDER_EMPTY_MSG, */
   };
   TInt32 elem = (sizeof(texts)/sizeof(*texts));

   if (iReportList) {
      /* Already selecting */
      return;
   }
   iReportList = new (ELeave) CDesCArrayFlat(elem);
   for (TInt32 i = 0 ; i < elem ; i++) {
      HBufC* txt = iCoeEnv->AllocReadResourceLC(texts[i]);
      HBufC* tmp = HBufC::NewLC(txt->Length()+32);
      TPtr bar = txt->Des();
/*       tmp->Des().Append(KTab); */
# ifdef USE_NUMBERS_ON_SMS_ERROR_REPORT
      TInt j = (i+1);
      if (j == 10) { j = 0; }
      if (j < 10) {
         tmp->Des().AppendNum(j, EDecimal);
      }
      tmp->Des().Append(_L(" "));
# endif
      tmp->Des().Append(bar);
/*       tmp->Des().Append(KTab); */
/*       tmp->Des().Append(KTab); */
      iReportList->AppendL(*tmp);
      CleanupStack::PopAndDestroy(tmp);
      CleanupStack::PopAndDestroy(txt);
   }
   typedef TCallBackEvent<CWayFinderAppUi, TWayfinderEvent> cb_t;
   typedef CCallBackListDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
            EWayfinderEventReportEventListDialogOk,
            EWayfinderEventReportEventListDialogCancel),
         iSelectedReportEvent,
         R_WF_REPORT_EVENT,
         iReportList,
         R_WAYFINDER_CALLBACK_SINGLE_LIST_QUERY,
         ETrue, 
         0);
#endif
}

void CWayFinderAppUi::NetworkInfoDone()
{
   if (iEventSmsErrTxt) {
      SendDebugSms(iEventSMSid, *iEventSmsErrTxt);
      delete iEventSmsErrTxt;
      iEventSmsErrTxt = NULL;
   } else {
      HBufC* errTxt = KNullDesC().AllocLC();
      SendDebugSms(iEventSMSid, *errTxt);
      CleanupStack::PopAndDestroy(errTxt);
   }
}

void CWayFinderAppUi::NetworkInfoInUse()
{
   //Could not get the network info, 
   //so send the SMS anyway without the info.
   if (iEventSmsErrTxt) {
      SendDebugSms(iEventSMSid, *iEventSmsErrTxt);
      delete iEventSmsErrTxt;
      iEventSmsErrTxt = NULL;
   } else {
      HBufC* errTxt = KNullDesC().AllocLC();
      SendDebugSms(iEventSMSid, *errTxt);
      CleanupStack::PopAndDestroy(errTxt);
   }
}

void CWayFinderAppUi::NetworkInfoFailed(TInt aError)
{
   //Could not get the network info, 
   //so send the SMS anyway without the info.
   if (iEventSmsErrTxt) {
      SendDebugSms(iEventSMSid, *iEventSmsErrTxt);
      delete iEventSmsErrTxt;
      iEventSmsErrTxt = NULL;
   } else {
      HBufC* errTxt = KNullDesC().AllocLC();
      SendDebugSms(iEventSMSid, *errTxt);
      CleanupStack::PopAndDestroy(errTxt);
   }
}

#if 0
# ifdef USE_CELL_INFO_IN_SMS
#  ifndef NAV2_CLIENT_SERIES60_V2
#   include <etelbgsm.h>
#  else
#   include "my_etelbgsm.h"
#  endif
# endif
#endif
void
CWayFinderAppUi::SendDebugSms(uint32 errNum, const TDesC& errTxt)
{
#if defined SEND_DEBUG_SMS_ON_ERRORS || defined USE_REPORT_EVENT_MENU
   HBufC* text = HBufC::NewLC(256);
   TPtr tmp = text->Des();
   tmp.Copy(_L(""));
# ifdef USE_CELL_INFO_IN_SMS
   //// code /////
   if (iNetworkInfoHandler->iCellId != MAX_UINT32) {
      tmp.AppendNum(iNetworkInfoHandler->iCellId, EDecimal);
   }
   tmp.Append( _L(";"));
   if (iNetworkInfoHandler->iLac != MAX_UINT32) {
      tmp.AppendNum(iNetworkInfoHandler->iLac, EDecimal);
   }
   tmp.Append( _L(";"));
   if (iNetworkInfoHandler->iMcc != KNullDesC) {
      tmp.Append(iNetworkInfoHandler->iMcc);
   }
   tmp.Append( _L(";"));
   if (iNetworkInfoHandler->iMnc != KNullDesC) {
      tmp.Append(iNetworkInfoHandler->iMnc);
   }
   tmp.Append( _L(";"));

#  if 0
   TInt aMcc = 0;
   TInt aMnc = 0;
   TInt aLocation = 0;
   TInt aCellId = 0;

   TInt enumphone = 1;
   RTelServer server;
   RBasicGsmPhone phone;
   RTelServer::TPhoneInfo info;
   MBasicGsmPhoneNetwork::TCurrentNetworkInfo NetworkInfo;

   if (server.Connect() == KErrNone) {
      CleanupClosePushL(server);
      if (server.LoadPhoneModule(_L("Phonetsy.tsy")) == KErrNone) {
         if (server.EnumeratePhones(enumphone) == KErrNone) {
            if (enumphone >= 1){
               if (server.GetPhoneInfo(0, info) == KErrNone) {
                  if (phone.Open(server, info.iName) == KErrNone) {
                     CleanupClosePushL(phone);
                     if (phone.GetCurrentNetworkInfo(NetworkInfo) == KErrNone) {
                        aMcc = NetworkInfo.iNetworkInfo.iId.iMCC;
                        aMnc = NetworkInfo.iNetworkInfo.iId.iMNC;
                        aLocation = NetworkInfo.iLocationAreaCode;
                        aCellId = NetworkInfo.iCellId;
                        tmp.AppendNum(aCellId, EDecimal);
                        tmp.Append( _L(";"));
                        tmp.AppendNum(aLocation, EDecimal);
                        tmp.Append( _L(";"));
                        tmp.AppendNum(aMcc, EDecimal);
                        tmp.Append( _L(";"));
                        tmp.AppendNum(aMnc, EDecimal);
                        tmp.Append( _L(";"));
                     } else {
/*                         text->Append( _L("\nFail GetCurrentNetworkInfo") ); */
                     }
                     CleanupStack::PopAndDestroy(1); // phone
                  } else {
/*                      text->Append( _L("\nFail phone.Open") ); */
                  }
               } else {
/*                   text->Append( _L("\nFail server.getphone") ); */
               }
            } else {
/*                text->Append( _L("\nFail enumphone") ); */
            }
         } else {
/*             text->Append( _L("\nFail enumerate phones") ); */
         }
      } else {
/*          text->Append( _L("\nFail KTsyName") ); */
      }
      CleanupStack::PopAndDestroy(1); // server
   } else {
/*       text->Append( _L("\nFail server connect") ); */
   }
   if (text->Length() == 0) {
      tmp.Append( _L(";;;;"));
   }
#  endif


# endif
   WFTextUtil::TDesAppend(tmp, errTxt);
   HBufC* viewStr = HBufC::NewLC(2);
   TPtr tmp2 = viewStr->Des();
   HBufC* smsText;

   if (!iTabGroup) {
      tmp2.Copy(_L("-"));
   } else {
      if(iTabGroup->ActiveTabId() == KNewDestViewId.iUid ){
         tmp2.Copy(_L("s"));
      } else if(iTabGroup->ActiveTabId() ==  KSettingsViewId.iUid) {
         tmp2.Copy(_L("o"));
      } else if(iTabGroup->ActiveTabId() ==  KMyDestViewId.iUid) {
         tmp2.Copy(_L("f"));
      } else if(iTabGroup->ActiveTabId() ==  KPositionSelectViewId.iUid) {
         tmp2.Copy(_L("s"));
      } else if(iTabGroup->ActiveTabId() ==  KContactsViewId.iUid) {
         tmp2.Copy(_L("o"));
      } else if(iTabGroup->ActiveTabId() ==  KWelcomeViewId.iUid) {
         tmp2.Copy(_L("o"));
      } else if(iTabGroup->ActiveTabId() ==  KInfoViewId.iUid) {
         tmp2.Copy(_L("o"));
      } else if(iTabGroup->ActiveTabId() == KMapViewId.iUid ){
         tmp2.Copy(_L("m"));
      } else if (iTabGroup->ActiveTabId() == KGuideViewId.iUid) {
         tmp2.Copy(_L("g"));
      } else if (iTabGroup->ActiveTabId() == KItineraryViewId.iUid) {
         tmp2.Copy(_L("i"));
      } else if (iTabGroup->ActiveTabId() == KStartPageViewId.iUid) {
         tmp2.Copy(_L("o"));
      } else if (iTabGroup->ActiveTabId() == KDestinationViewId.iUid) {
         tmp2.Copy(_L("o"));
      } else {
         tmp2.Copy(_L("o"));
      }
   }

   HBufC* tmpImei = WFTextUtil::AllocL(iImeiNbr);
   HBufC* tmpName;
   if (iDataStore->iWFAccountData->getUserName()) {
      tmpName = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getUserName());
   } else {
      tmpName = KNullDesC().AllocLC();
   }
   smsText = A1DebugSms::FormatLC(iDataStore->iWFAccountData->getWfType(),
            *iWfTypeStr, *iWfOptsStr, *tmpName,
            errNum,
            iCurrPos.iY, iCurrPos.iX, m_speed, m_angle,
            *viewStr,
            HIGH(m_routeid), LOW(m_routeid),
            isab::Nav2_Major_rel, isab::Nav2_Minor_rel, isab::Nav2_Build_id, 
            *text,
            tmpImei
            );

   if (smsText) {
      /* Got it. */
#ifdef NAV2_CLIENT_SERIES60_V3
      if (!iSmsHandler) {
         iSmsHandler = SmsHandlerFactory::CreateL( this );
      }
      iSmsHandler->SendSmsL(*smsText, KDebugSmsNumber);
#else
      iCommunicateHelper->SendSMSMessageL( KDebugSmsNumber, NULL, NULL, *smsText );
#endif
      CleanupStack::PopAndDestroy(smsText);
   }
   CleanupStack::PopAndDestroy(tmpName);
#ifdef NAV2_CLIENT_SERIES60_V3
   delete tmpImei;
#endif

   CleanupStack::PopAndDestroy(viewStr);
   CleanupStack::PopAndDestroy(text);
#else
   errNum = errNum;
   errTxt.Length();
#endif
}


void
CWayFinderAppUi::RequestCancelled()
{
   SetWait( EFalse, 0 );
   iSynchronizing = EFalse;
#ifndef NAV2_CLIENT_SERIES60_V3
   if (!m_connectData->isReady()) {
      ShowConnectDialog(m_connectData);
   }
#endif
}

void
CWayFinderAppUi::ShowErrorDialogL(TInt32 aErrNbr,
      const char* aUtf8Text, 
      TBool aBanner)
{
   TBool popped = false;

   if (aErrNbr) {
      
      HBufC* errTxt = WFTextUtil::Utf8ToHBufCL(aUtf8Text);

      CleanupStack::PushL(errTxt);
      TBuf<32> header;
      iCoeEnv->ReadResource(header, R_WAYFINDER_ERROR_MSG);
      header.Append(_L(": "));
      header.AppendNum(aErrNbr, EHex);
      iEventSMSid = aErrNbr;

#ifdef SEND_DEBUG_SMS_ON_ERRORS
/*      Disabled due to resource conservation. 
         TBool res = 
         WFDialog::ShowScrollingQueryL(R_WF_DEBUGSMS_MESSAGE_QUERY, 
                                       header, *errTxt, EFalse);
      if (res) {
      iEventSmsErrTxt = errTxt->AllocL();*/
# ifdef USE_CELL_INFO_IN_SMS
         //In this define we cant delete iEventSmsErrTxt since 
         //iNetworkInfoHandler is a nonblocking ActiveObject and
         //we need the ErrTxt when it returns.
         iNetworkInfoHandler->FetchNetworkInfoL();
# else
         SendDebugSms(aErrNbr, *iEventSmsErrTxt);
         delete iEventSmsErrTxt;
         iEventSmsErrTxt = NULL;
# endif
      }
#else
      WFDialog::ShowScrollingDialogL(header, *errTxt, EFalse, -1, -1);

      if(isCriticalError(aErrNbr)) {
         CleanupStack::PopAndDestroy(errTxt);
         popped = true;
         RealShutdown();
      }
#endif
      if(!popped) {
         CleanupStack::PopAndDestroy(errTxt);
      }  
   }
}

void
CWayFinderAppUi::ShowWarningDialogL(TInt32 aErrNbr, const char* aUtf8Text)
{
   if (aErrNbr) {
      HBufC* errTxt = WFTextUtil::Utf8ToHBufCL(aUtf8Text);
      CleanupStack::PushL(errTxt);
      TBuf<32> header;
      iCoeEnv->ReadResource(header, R_WAYFINDER_WARNING_MSG);
      header.Append(_L(": "));
      header.AppendNum(aErrNbr, EHex);
      HBufC* text = AllocLC(header + _L("\n") + *errTxt);
      WFDialog::ShowWarningL(*text);
      CleanupStack::PopAndDestroy(text);
      CleanupStack::PopAndDestroy(errTxt);
   }
}


void CWayFinderAppUi::HandleRequestFailedL( RequestFailedMess* aMessage )
{
   TBool handled = EFalse;
   uint8 req_id = aMessage->getFailedRequestMessageNbr();
   TUint eNbr = aMessage->getErrorNbr();

   RequestCancelled();

   switch (req_id ) {
      case GuiProtEnums::PARAMETERS_SYNC:
      {
         /* Searching for IAP. Check for error messages that */
         /* shows that we have contacted the server. */
         /* NOTE! This code is also found in the Series90 version. */
         if (Nav2Error::Nav2ErrorTable::acceptableServerError(eNbr)) {
            if (iIAPSearcher && iIAPSearcher->Searching()) {
               if (eNbr == Nav2Error::NSC_EXPIRED_USER) {
                  handled = ETrue;
               }
               iIAPSearcher->Reply();
            }
         } else {
            if (iIAPSearcher && iIAPSearcher->Searching()) {
               iIAPSearcher->ReplyFailed();
               handled = ETrue;
            }
         }
         break;
      }
      case GuiProtEnums::GET_VECTOR_MAP:
      case GuiProtEnums::GET_VECTOR_MAP_REPLY:
      case GuiProtEnums::GET_MULTI_VECTOR_MAP:
      case GuiProtEnums::GET_MULTI_VECTOR_MAP_REPLY:
         // Handle this specially for errors from a vector map request.
         // The error information is passed to MapLib and no error dialog
         // is shown.
         iMapView->HandleVectorMapErrorReply( aMessage->getFailedRequestSeqId() );
         if (!iErrorFilter->displayErrorMessage()) {
            // Not time to show any error message yet.
            return;
         }
         break;
      case GuiProtEnums::ROUTE_TO_FAVORITE:
      case GuiProtEnums::ROUTE_TO_HOT_DEST:
      case GuiProtEnums::ROUTE_MESSAGE:
         if (eNbr == Nav2Error::NSC_NO_GPS_WARN) {
            // No gps positions is received, show a warning to user
            // and continue waiting for signal
            ShowWarningDialogL(eNbr, aMessage->getErrorString());
            return;
         }
         break;
   }
   if (!handled) {
#ifdef SHOW_BUY_EXTENSION_ON_EXPIRED_USER
      using namespace isab;
      using namespace Nav2Error;
      if (eNbr == NSC_EXPIRED_USER) {
         /* Show buy extension. */
         if(WFDialog::ShowScrollingQueryL(iCoeEnv,
                  R_WF_SHOW_BUY_EXTENSION_QUERY_HEADER,
                  R_WF_SHOW_BUY_EXTENSION_QUERY_TEXT,
                  EFalse)) {
            LaunchBuyExtensionWapLinkL();
         }
         /* Avoid ShowErrorDialogL below. */
         return;
      }
      /* Fallthrough to ShowErrorDialogL below. */
#endif
      ShowErrorDialogL(eNbr, aMessage->getErrorString());
   }
}

void CWayFinderAppUi::UpdateParametersL( GenericGuiMess* aMessage )
{
   GuiProtEnums::ParameterType messType =
         (GuiProtEnums::ParameterType)aMessage->getFirstUint16();
   switch( messType )
   {
   case GuiProtEnums::paramTopRegionList:
      GetTopRegionsL();
      break;
   case GuiProtEnums::paramSoundVolume:
   case GuiProtEnums::paramUseSpeaker:
   case GuiProtEnums::paramTurnSoundsLevel:
   case GuiProtEnums::paramStoreSMSDestInMyDest:
   case GuiProtEnums::paramAutomaticRouteOnSMSDest:
   case GuiProtEnums::paramKeepSMSDestInInbox:
   case GuiProtEnums::paramBacklightStrategy:
   case GuiProtEnums::paramSelectedAccessPointId:
   case GuiProtEnums::paramSelectedAccessPointId2:
   case GuiProtEnums::paramUsername:
   case GuiProtEnums::paramPassword:
   case GuiProtEnums::paramWebUsername:
   case GuiProtEnums::paramWebPassword:
   case GuiProtEnums::paramMuteTurnSound:
   case GuiProtEnums::userDirectionMode:
   case GuiProtEnums::userVicinityFeedMode:
   case GuiProtEnums::paramVicinityListUpdatePeriod:
   case GuiProtEnums::paramGpsMinWalkingSpeed:
   case GuiProtEnums::paramFeedWaitPeriod:
   case GuiProtEnums::paramLockedNavWaitPeriod:
      RequestSimpleParameterL(messType);
      break;
   case GuiProtEnums::paramCallCenterNumbers: //Deprecated
      // XXX Deprecated XXX
      break;
   default:
      /* Not wanted. */
      break;
   }
}

void CWayFinderAppUi::HandleTopRegionReplyL( GetTopRegionReplyMess* aMessage )
{
   TopRegionList* list = aMessage->getTopRegionList();

   TUint selectedCountryId = iDataHolder->GetSelectedCountryId();
   iDataHolder->InitCountries();
   TBuf<KBuf32Length> buf;
   TUint32 id = MAX_UINT32;
   /*iCoeEnv->ReadResource( buf, R_WAYFINDER_FROM_GPS_TEXT );
   iDataHolder->AddCountryL( buf, id );*/
#if defined SHOW_US_POPUP_WARNING
   TBool popUpWarning = EFalse;
#endif
   for( TInt i=0; i < list->size(); i++ ){
      buf.Zero();
      const TopRegion* region = list->getRegion(i);
      WFTextUtil::char2TDes( buf, region->getName() );
      id = region->getId();
#if defined SHOW_US_POPUP_WARNING
      popUpWarning = TopRegionHack::TopRegionPopupWarning(popUpWarning, id);
#endif
      iDataHolder->AddCountryL( buf, id );
   }
#if defined SHOW_US_POPUP_WARNING
   if (popUpWarning) {
      m_startupHandler->StartupEvent(SS_USDisclaimerNeeded);
   } else {
      m_startupHandler->StartupEvent(SS_USDisclaimerNotNeeded);
   }
#else
   m_startupHandler->StartupEvent(SS_USDisclaimerNotNeeded);
#endif
   iNewDestView->ReinitCountriesL( selectedCountryId );
}

#if 0
void
CWayFinderAppUi::ShowUSWarningPopup()
{
   // Show popup.
   typedef TCallBackEvent<CWayFinderAppUi, TWayfinderEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
            EWayfinderEventUSAWarningDialogOk,
            EWayfinderEventUSAWarningDialogCancel),
            R_WF_SAFETY_INFORMATION_HEADER,
            R_WF_SAFETY_INFORMATION_BODY,
            R_NON_BLOCKING_QUERY_MESSAGE);
}
#endif

void
CWayFinderAppUi::SendSMSReportEvent(TInt aId)
{
   TBool res = ETrue;
#ifdef ASK_USER_BEFORE_SENDING_REPORT_EVENT
   TInt textResource[] = {
      R_WF_TRAFFIC_JAM_REPORT_MSG,           /* 0xf001 */
      R_WF_TRAFFIC_SPECIAL_EVENT_MSG,        /* 0xf002 */
      R_WF_TRAFFIC_BREAKDOWN1_MSG,           /* 0xf003 */
      R_WF_TRAFFIC_BREAKDOWN2_MSG,           /* 0xf004 */
      R_WF_TRAFFIC_SPEED_CAM_MSG,            /* 0xf005 */
      R_WF_TRAFFIC_REPORT_PROBLEM_MSG,       /* 0xf006 */
      R_WF_TRAFFIC_DANGEROUS_PLACE_MSG,      /* 0xf007 */
      R_WF_TRAFFIC_WEATHER_WARNING_MSG,      /* 0xf008 */
      R_WF_TRAFFIC_REPORT_MAP_ERROR_MSG,     /* 0xf009 */
      R_WF_TRAFFIC_REPORT_POSITION_MSG,      /* 0xf00a */
      R_WF_TRAFFIC_REPORT_ROUTE_ERROR_MSG,   /* 0xf00b */
      R_WF_TRAFFIC_BREAKDOWN2_MSG,           /* 0xf00c */
   };
   HBufC* header = iCoeEnv->AllocReadResourceLC(R_WF_REPORT_EVENT);
   HBufC* message = iCoeEnv->AllocReadResourceLC(textResource[aId-0xf001]);

   iEventSMSid = aId;

   res = WFDialog::ShowScrollingQueryL(
         R_MESSAGE_QUERY, *header, *message, EFalse);
   CleanupStack::PopAndDestroy(message);
   CleanupStack::PopAndDestroy(header);
#endif
   if (res) {
      iEventSmsErrTxt = KNullDesC().AllocL();
#ifdef USE_CELL_INFO_IN_SMS
      //In this define we cant delete iEventSmsErrTxt since 
      //iNetworkInfoHandler is a nonblocking ActiveObject and
      //we need the ErrTxt when it returns.
      iNetworkInfoHandler->FetchNetworkInfoL();
#else
      SendDebugSms(aId, *iEventSmsErrTxt);
      delete iEventSmsErrTxt;
      iEventSmsErrTxt = NULL;
#endif
   }
}

void
CWayFinderAppUi::GenerateEvent(enum TWayfinderEvent aEvent)
{        
   if (iEventGenerator) {
      iEventGenerator->SendEventL(aEvent);
   }
}           

void
CWayFinderAppUi::HandleGeneratedEventL(enum TWayfinderEvent aEvent)
{
   switch(aEvent){
      case EWayfinderEventStartupFailedOk:
         m_startupHandler->StartupEvent(SS_GotoMainMenu);
         break;
      case EWayfinderEventStartupFailedCancel:
      case EWayfinderEventShutdownNow:
         HandleCommandL(EEikCmdExit);
         break;
#if 0
      case EWayfinderEventUSAWarningDialogOk:
         {
            /*             iStartupHandler->HandleUsaSafetyPopupDone(); */
         } break;
      case EWayfinderEventUSAWarningDialogCancel:
         {
            /* Need to shut down. */
            iUrgentShutdown = ETrue;
            ShutdownNow();
         } break;
#endif
      case EWayfinderEventConnectToGps:
         if(!iBtHandler) {
            if(iStartupCompleted) {
               ConnectToGpsL();
            } else {
               iGpsAutoConnectAfterStartup = ETrue;
            }
         } break;
      case EWayfinderEventReportEventListDialogOk:
         delete iReportList;
         iReportList = NULL;
         HandleReportEventMenu(iSelectedReportEvent);
         break;
      case EWayfinderEventReportEventListDialogCancel:
         /* Do nothing. */
         delete iReportList;
         iReportList = NULL;
         break;
      case EWayfinderEventInboxSmsSelected:
#ifndef NAV2_CLIENT_SERIES60_V3
         if(iSelectedSms >= 0 && iSelectedSms < iCommunicateHelper->SmsCount()){
            typedef TCallBackEvent<CWayFinderAppUi, TWayfinderEvent> cb_t;
            typedef CCallBackDialog<cb_t> cbd_t;
            TPtrC description = 
               iCommunicateHelper->SmsDescriptionArray()->MdcaPoint(iSelectedSms);
            TPtrC signature = 
               iCommunicateHelper->SmsSignatureArray()->MdcaPoint(iSelectedSms);
            HBufC* description_buffer = HBufC::NewLC(description.Length() + 
                  signature.Length() + 8);
            description_buffer->Des().Format(_L("%S\n%S"), 
                  &description, &signature);
            TPtrC sender = 
               iCommunicateHelper->SmsSenderArray()->MdcaPoint(iSelectedSms);
            HBufC* sender_buffer = sender.AllocLC();
            cbd_t::RunDlgLD(cb_t(this, EWayfinderEventInboxSmsConfirmed),
                  sender_buffer,
                  description_buffer,
                  R_NON_BLOCKING_QUERY_MESSAGE_OK_CANCEL);

            CleanupStack::Pop(2, description_buffer); //the dlg owns the buffers
         }
#endif
         break;
      case EWayfinderEventInboxSmsConfirmed:
#ifndef NAV2_CLIENT_SERIES60_V3
         if(iSelectedSms >= 0 && iSelectedSms < iCommunicateHelper->SmsCount()){
            TMsvId id = iCommunicateHelper->SmsIdArray()[iSelectedSms];
            iCommunicateHelper->HandleSMS(id);
         }
#endif
         break;
      case EWayfinderEventBuyExtension:
         m_startupHandler->StartupEvent(SS_Exit);
         LaunchBuyExtensionWapLinkL();
         break;
      case EWayfinderEventCheckNetwork:
         CheckNetwork();
         break;
      case EWayfinderEventDoNothing:
         // Do nothing!
         break;
   }
}

void CWayFinderAppUi::CheckNetwork()
{
   int32 type        = SP_NoNetwork;
   int32 requestType = isab::GuiProtEnums::SYNC_FAVORITES;
   HBufC* errorMsg   = NULL;
   int32 error       = 0;

   if (iOfflineTester->OffLineMode()) { 
      errorMsg = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_OFFLINE_MODE_MSG);
      error = isab::Nav2Error::NSC_FLIGHT_MODE;
   } else if (!iOfflineTester->NetworkAvailable()) {
      errorMsg = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_NO_NETWORK_MSG);
      error = isab::Nav2Error::NSC_NO_NETWORK_AVAILABLE;
   } else {
      m_startupHandler->StartupEvent(SS_CheckFlightModeOk);
      return;
   }
   
   class WFStartupEventCallbackParameter* tmp =
      new WFStartupEventCallbackParameter(type, error, requestType, 
                                          WFTextUtil::newTDesDupL(*errorMsg));
   CleanupStack::PopAndDestroy(errorMsg);

   m_startupHandler->StartupEvent(SS_IapSearchNotOk);
   m_startupHandler->StartupEvent(SS_StartupError, tmp);   
}

void CWayFinderAppUi::HandleSetSimpleParametersL(SimpleParameterMess* aMessage)
{
   TBuf<KBuf256Length> serverPort;
   uint16 type = aMessage->getParameterType();
   switch( type )
      {
      case (uint16)GuiProtEnums::paramServerNameAndPort:
         // type_and_string
         // char* serverNameAndPort The server name and
         //                         port in the same string
         //                         separated by colon.
         WFTextUtil::char2TDes( serverPort, aMessage->getFirstString() );
         iSettingsView->SetServer( serverPort );
      break;
   case (uint16)GuiProtEnums::paramSoundVolume:
         // type_and_uint8
         // uint8 volumePercent Value between decimal 0
         //                     and 99 giving apriximately
         //                     the volume percentage.
      iSettingsView->SetVolume( aMessage->getFirstUint8() );
      if (iAudioSlave) {
         iAudioSlave->SetVolume( aMessage->getFirstUint8() );
      }
      break;
   case (uint16)GuiProtEnums::paramUseSpeaker:
         // type_and_bool
         // bool useSpeaker True if the speaker should be
         //                 used, false otherwise.
         //                 I don't think the following has been true ever:
         //                 NB! This only affects the main
         //                     speaker(s). Any handsfree
         //                     or similar will still give
         //                     sound.
         //
         //                     To mute all sound use
         //                     paramSoundVolume = 0 instead.
      iSettingsView->SetUseSpeaker( aMessage->getFirstBool() );
      updateMute();
      break;
   case (uint16)GuiProtEnums::paramTurnSoundsLevel:
         // type_and_uint8
         // isab::GuiProtEnums::TurnSoundsLevel
         //                 How talkative the navigator is.
      iSettingsView->SetTurnSoundsLevel( (GuiProtEnums::TurnSoundsLevel)aMessage->getFirstUint8() );
      break;
   case (uint16)GuiProtEnums::paramAutoReroute:
         // type_and_bool
         // bool autoReroute    This makes Nav2 initiate a
         //                     reroute when getting off-
         //                     track, without any interaction
         //                     from the user.
         iSettingsView->SetAutoReroute( aMessage->getFirstBool() );
      break;
   case (uint16)GuiProtEnums::paramTransportationType:
         // type_and_uint8
         // NavServerComEnums::VehicleType transportationType
         //                     Determines what roads that are
         //                     used when creatign a route.
         iSettingsView->SetVehicle( (NavServerComEnums::VehicleType)aMessage->getFirstUint8() );
      break;
   case (uint16)GuiProtEnums::paramDistanceMode:
         // type_and_uint8
         // DistancePrintingPolicy::DistanceMode distanceMode
         //                     Determines which units are
         //                     used for presentation of distances.
         iSettingsView->SetDistanceMode( (DistancePrintingPolicy::DistanceMode)aMessage->getFirstUint8() );
      break;
   case (uint16)GuiProtEnums::paramStoreSMSDestInMyDest:
      // type_and_uint8
      // GuiProtEnums::YesNoAsk strategy.
      //               Determines if destinations are
      //               stored automatically or if the
      //               user is asked what to do.
      iSaveSmsDestination = (GuiProtEnums::YesNoAsk)aMessage->getFirstUint8();
         iSettingsView->SetSaveSMSDestination( (GuiProtEnums::YesNoAsk)aMessage->getFirstUint8() );
      break;
   case (uint16)GuiProtEnums::paramAutomaticRouteOnSMSDest:
      // type_and_bool
      // bool automaticRoute
      iAutoRouteSmsDestination = aMessage->getFirstBool();
         iSettingsView->SetAutorouteSMS( aMessage->getFirstBool() );
      break;
   case (uint16)GuiProtEnums::paramKeepSMSDestInInbox:
      // type_and_uint8
      // GuiProtEnums::YesNoAsk strategy.
      //               Determines if SMS destinatios
      //               are keept in the inbox or
      //               something else happends.
      iKeepSmsDestination = (GuiProtEnums::YesNoAsk)aMessage->getFirstUint8();
         iSettingsView->SetKeepSMS( (GuiProtEnums::YesNoAsk)aMessage->getFirstUint8() );
      break;
   case (uint16)GuiProtEnums::paramBacklightStrategy:
      // type_and_uint8
      iBacklightStrategy =
         (GuiProtEnums::BacklightStrategy)aMessage->getFirstUint8();
      iSettingsView->SetBacklightStrategy(
         (GuiProtEnums::BacklightStrategy)aMessage->getFirstUint8() );
      break;
      case GuiProtEnums::userVicinityFeedMode: {
         int data = aMessage->getFirstUint8();
         iSettingsView->setFeedOutputFormat(FeedSettings::OutputFormat(data));
      }  break;
      case GuiProtEnums::userDirectionMode: {
         int data = aMessage->getFirstUint8();
         iSettingsView->setDirectionType(MC2Direction::RepresentationType(data));
      } break;
      case GuiProtEnums::paramFeedWaitPeriod: {
         int period = aMessage->getFirstUint8();
         iSettingsView->setVicinityFeedWaitPeriod(period);
      } break;
      case GuiProtEnums::paramLockedNavWaitPeriod: {
         int period = aMessage->getFirstUint8();
         iSettingsView->setLockedNavWaitPeriod(period);
      } break;
      case GuiProtEnums::paramGpsMinWalkingSpeed: {
         int minSpeed = aMessage->getFirstUint8();
         iSettingsView->setGpsMinWalkingSpeed(minSpeed);
      } break;
      case GuiProtEnums::paramVicinityListUpdatePeriod: {
         int updatePeriod = aMessage->getFirstUint8();
         iSettingsView->setVicinityListUpdatePeriod( updatePeriod );
      } break;
   case (uint16)GuiProtEnums::paramSelectedAccessPointId:
      SetIAP(aMessage->getFirstUint32());
      break;
   case (uint16)GuiProtEnums::paramSelectedAccessPointId2:
      // Tell IAPSearcher that IAP now is set and it can start testing it
      if ( iIAPSearcher ) {
         int32 iapId = aMessage->getFirstUint32();
         iIAPSearcher->iapid2Set( iapId );
      }
      break;
   case (uint16)GuiProtEnums::paramWebUsername:
      {
         /* This means that UiCtrl now can respond. */
         if (!m_XXX_settings_gotten) {
            m_XXX_settings_gotten = ETrue;
            RequestSettingsL();
         }

         char *name = aMessage->getFirstString();
         if (!strcmp(name, "unknown")) {
            HBufC* tmp = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_UNKNOWN_TEXT);
            iDataStore->iWFAccountData->setWebUserName(WFTextUtil::strdupL(name));
            CleanupStack::PopAndDestroy(tmp);
         } else {
            iDataStore->iWFAccountData->setWebUserName(WFTextUtil::strdupL(name));
         }
         HBufC* tmp = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getWebUserName());
         iSettingsView->SetWebUsername(*tmp);
         CleanupStack::PopAndDestroy(tmp);
      }

      break;
   case (uint16)GuiProtEnums::paramWebPassword:
      {
         iDataStore->iWFAccountData->setWebPassword(WFTextUtil::strdupL(
               aMessage->getFirstString()));
         HBufC* tmp = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getWebPassword());
         iSettingsView->SetWebPassword(*tmp);
         CleanupStack::PopAndDestroy(tmp);
      }
      break;
   case (uint16)GuiProtEnums::paramUsername:
      {
         iDataStore->iWFAccountData->setUserName(WFTextUtil::strdupL(
               aMessage->getFirstString()));
         HBufC* tmp = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getUserName());
         iSettingsView->SetUsername(*tmp);
         CleanupStack::PopAndDestroy(tmp);
         if ( iMapView ) {
            // Update the MapView so that it can create the pre-cache.
            // (The username is needed there)
            iMapView->tryToAddVectorPrecache();
         }
      }
      break;
   case (uint16)GuiProtEnums::paramPassword:
      {
         iDataStore->iWFAccountData->setPassword(WFTextUtil::strdupL(
               aMessage->getFirstString()));

         if (iDataStore->iWFAccountData->getPassword()) {
            HBufC* str = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getPassword());
            iSettingsView->SetPassword(*str);
            CleanupStack::PopAndDestroy(str);
         }
      }
      break;
   case (uint16)GuiProtEnums::paramMuteTurnSound:
      // Unused now. Hysterical reasons made the meaning unclear
      // so one friday afternoon I decided to rename a variable. That was
      // not good. Now we have removed the setting and are rather
      // content with the result.
      break;
   default:
#if !defined(__RELEASE__)
      TBuf<KBuf64Length> warning( _L("Received an unknown parameter") );
      WFDialog::ShowWarningL( warning );
#endif
      break;
   }
}

void CWayFinderAppUi::SwitchToSettingsL()
{
   /* Before we can switch to settings, we need to read out some */
   /* volatile variables. */
   iSettingsData->iAllowGps = EarthAllowGps();
   iSettingsData->iIsIron = IsIronVersion();

   /* Switch to view. */
   GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
   iTabGroup->SetActiveTabById( KSettingsViewId.iUid );
   ActivateLocalViewL( KSettingsViewId );
}

void
CWayFinderAppUi::ConnectDialogIsClosing()
{
   SetNaviPaneLabelL(0);
}

void CWayFinderAppUi::HandleSearchReplyL( SearchAreaReplyMess* aSearchAreas )
{
   TBuf<KBuf128Length> name;
   /* If we got new search areas, that means that the destinations */
   /* stored are now stale. Reinitialize them. */
   iDataHolder->InitNewDests();
/* #define DEBUG_SEARCH */
#ifdef DEBUG_SEARCH
   name.Copy(_L("Number of area items: "));
   name.AppendNum(aSearchAreas->size(), EDecimal);
   WFDialog::ShowWarningDialogL(name);
#endif

   if (aSearchAreas->size() > 0) {
      /* Got search area hits. */

      if (m_XXX_temp_no_search_items && aSearchAreas->size() == 1) {
         /* Uh-oh, one unique search area and no search items, */
         /* this means that the search failed. */
         WFDialog::ShowInformationL( R_WAYFINDER_NOHITS_MSG, iCoeEnv );
         return;
      }

      /* Check if the city list is changed. */
      TBool newCity = EFalse;
      for(TInt i = 0; i < aSearchAreas->size(); ++i){
         name.Zero();
         WFTextUtil::char2TDes(name, (*aSearchAreas)[i]->getName());
         if(!iDataHolder->IsDuplicateNewCity(name)){
            /* Something has changed. */
            newCity = ETrue;
            break;
         }
      }
      if( newCity ){
         /* Something has changed. */
         /* Initialize the area list. */
         iDataHolder->InitNewCities();

         for( TInt i=0; i < aSearchAreas->size(); i++ ){
            name.Zero();
            WFTextUtil::char2TDes( name, (*aSearchAreas)[i]->getName());
            iDataHolder->AddNewCityL( name );
            WFTextUtil::char2TDes( name, (*aSearchAreas)[i]->getID() );
            iDataHolder->AddNewCityIdL( name );
         }
      }
      if( aSearchAreas->size() > 1 ) {
         /* Only send the "select city" dialog if we have */
         /* more than one area to choose from. */

         iNewDestView->SetSearchDataL(ETrue);
      }
   }
}

void CWayFinderAppUi::HandleSearchReplyL( SearchItemReplyMess* aSearchItems )
{
   iDataHolder->InitNewDests();
   iDataHolder->SetSearchTotalHits(aSearchItems->getTotalHits());
   iDataHolder->SetSearchStartIndex(aSearchItems->getStartIndex());
   if (aSearchItems->size() > 0){

      /* We've got some search items in the answer. */
      for( TInt i=0; i < aSearchItems->size(); i++ ){
         const SearchItem *item = (*aSearchItems)[i];

         /* Create a policy object of the right type, */
         /* and get the string from there. */
         char *parsed = SearchPrintingPolicyFactory::parseSearchItem(item,
               1,
               DistancePrintingPolicy::DistanceMode(iSettingsData->iDistanceMode));
         LOGNEWA(parsed, char, 256);
         HBufC* tmp =
            WFTextUtil::AllocLC(parsed);
         LOGDELA(parsed);
         delete[] parsed;

         iDataHolder->AddNewDestL( *tmp );
         CleanupStack::PopAndDestroy(tmp);

         tmp =
            WFTextUtil::AllocLC( item->getID() );
         iDataHolder->AddNewDestIdL( *tmp );
         CleanupStack::PopAndDestroy(tmp);

         iDataHolder->AddNewDestPosL( item->getLat(), item->getLon());
      }
      if( iTabGroup->ActiveTabId() == KNewDestViewId.iUid ){
         iNewDestView->SetSearchDataL();
      } else {
         iTabGroup->SetActiveTabById( KNewDestViewId.iUid );
         TUid messageId;
         messageId.iUid = ESearchReply;
         TBuf8<16> message( _L8("newResult") );
         ActivateLocalViewL( KNewDestViewId, messageId, message );
      }
   } else {
      WFDialog::ShowInformationL( R_WAYFINDER_NOHITS_MSG, iCoeEnv );
   }
}

void CWayFinderAppUi::StartNavigationL(int32 oLat, int32 oLon,
      int32 dLat,  int32 dLon, const char *destinationName)
{
#ifdef DEBUG_INVALIDATE_ROUTE
   TBuf<128> foo;
   foo.Copy( _L("Got start navigation, coords: "));
   foo.AppendNum(oLat, EDecimal);
   foo.Append(_L(" "));
   foo.AppendNum(oLon, EDecimal);
   foo.Append(_L(" "));
   foo.AppendNum(dLat, EDecimal);
   foo.Append(_L(" "));
   foo.AppendNum(dLon, EDecimal);
   WFDialog::ShowScrollingWarningDialogL(foo);
#endif

   iHasRoute = ETrue;
   iCurrentWpId = MAX_UINT32;
   iForceUpdate = EFalse;
   iLastRouteStatus = OnTrack;

   if (iDestinationName) {
      delete iDestinationName;
      iDestinationName = NULL;
   }
   iDestinationName = WFTextUtil::AllocL(destinationName);
   SetCurrentRouteEndPoints(*iDestinationName, dLat, dLon, oLat, oLon);
   iInfoView->UpdateRouteDataL( 0, 0, 0 );
   iGuideView->UpdateDistance( 0 );
   iRouteView->UpdateRouteDataL( 0, 0, 0 );
   SetCurrentTurn( 0 );
   iItineraryView->ClearRoute();

   // Calling this method seems to result in a
   // CmdaController 7 sometimes.
   //iAudioPlayer->Stop();
   if( iTabGroup->ActiveTabId() == KMapViewId.iUid ){
      iMapView->NewRouteDownloaded();
   } else if (iTabGroup->ActiveTabId() == KGuideViewId.iUid) {
      /* Don't do anything. */
   } else if (iTabGroup->ActiveTabId() == KItineraryViewId.iUid) {
      /* Don't do anything. */
   } else if (iTabGroup->ActiveTabId() == KVicinityFeedId.iUid) {
      /* Don't do anything. */
   } else {
      if (iSettingsData->iPreferredGuideMode == preferred_guide &&
          !IsIronVersion()) {
         /* Guide mode is Guide. */
         TUid messageId;
         messageId.iUid = ENoMessage;
         TBuf8<16> customMessage( _L8("") );
         SetViewL( KGuideViewId, messageId, customMessage );
      } else if(iSettingsData->iPreferredGuideMode == preferred_map) {
         /* Guide mode is Map. */
         TBuf8<16> message( _L8("") );
         TUid messageId;
         messageId.iUid = EFullScreen;
         SetViewL( KMapViewId, messageId, message );
      } else {
         // Guide mode is vicinity feed
         TUid messageId;
         messageId.iUid = ENoMessage;
         TBuf8<16> customMessage( _L8("") );
         TRACE_FUNC1("Activating guide mode - vicfeed.");
         iGuideView->CreateContainer();
         SetViewL( KVicinityFeedId, messageId, customMessage );
      }
   }
}

void CWayFinderAppUi::StopNavigationL()
{
#ifdef DEBUG_INVALIDATE_ROUTE
   TBuf<128> foo;
   foo.Copy( _L("Got stop navigation"));
   WFDialog::ShowScrollingWarningDialogL(foo);
#endif

   iHasRoute = EFalse;
   iCurrentWpId = MAX_UINT32;
   iForceUpdate = EFalse;
   iLastRouteStatus = OnTrack;

   //WFTextUtil::char2TDes(iDestinationName, destinationName);
   //SetCurrentRouteEndPoints(iDestinationName, dLat, dLon, oLat, oLon);
   iInfoView->UpdateRouteDataL( 0, 0, 0 );
   iGuideView->UpdateDistance( 0 );
   iRouteView->UpdateRouteDataL( 0, 0, 0 );
   SetCurrentTurn( 0 );

   iItineraryView->ClearRoute();

   // Calling this method seems to result in a
   // CmdaController 7 sometimes.
   //iAudioPlayer->Stop();
   if( iTabGroup->ActiveTabId() == KMapViewId.iUid )
   {
      iMapView->ClearRoute();
   }
}

void CWayFinderAppUi::HandleNavigation( UpdateRouteInfoMess* aRouteMessage )
{
   const RouteInfo& ri = *(aRouteMessage->getRouteInfo());
   TBool newWP = EFalse;
   TBool newInfo = EFalse;
   TUint currDist = ri.distToWpt;
   TUint nextDist = MAX_UINT32;
   TUint nextExit = 0;

   iSimulationStatus = ri.simInfoStatus;

   RouteAction nextAction = InvalidAction;
   RouteCrossing nextCross = NoCrossing;

   TBool nextHighway = EFalse;
   TBuf<KBuf256Length> nextStreet( _L("") );

#if 0
   nextStreet.Copy( _L("HN ") );
   nextStreet.AppendNum( ri.destLat, EDecimal);
   nextStreet.Append( _L(" ") );
   nextStreet.AppendNum( ri.destLon, EDecimal);
   WFDialog::ShowScrollingWarningDialogL(nextStreet);
   nextStreet.Copy( _L("") );
#endif

   if (iBacklightStrategy == GuiProtEnums::backlight_near_action &&
       currDist < 2200) {
      TurnBackLightOnL();
   }

   if( ri.currCrossing.valid ){
      nextDist = ri.currCrossing.distToNextCrossing;
      nextExit = ri.nextCrossing1.exitCount;
      nextAction = RouteAction(ri.nextCrossing1.action);
      nextCross = RouteCrossing(ri.nextCrossing1.crossingType);
      nextHighway = ri.nextSeg1.isHighway();
      WFTextUtil::char2TDes( nextStreet, ri.nextSeg1.streetName );
   }

   if((ri.onTrackStatus != OnTrack) &&
      (iLastRouteStatus != ri.onTrackStatus)){
         newInfo = ETrue;
   } else if((ri.onTrackStatus == OnTrack) &&
             ( (iLastRouteStatus != OnTrack) ||
               (ri.currCrossing.changed)     ||
               (ri.currSeg.changed)             ) ) {
      newWP = ETrue;
   }
   iLastRouteStatus = ri.onTrackStatus;

   if( iForceUpdate ){
      iForceUpdate = EFalse;
      if( iLastRouteStatus == OnTrack ) {
         newWP = ETrue;
      } else {
         newInfo = ETrue;
      }
   }
   TBuf<KBuf256Length> currStreet;
   if( newWP ){
      if( iTabGroup->ActiveTabId() == KItineraryViewId.iUid )
         iItineraryView->SetSelection( ri.crossingNo, iGuideView->GetCurrentTurn() );
      SetCurrentTurn( ri.crossingNo );
      WFTextUtil::char2TDes( currStreet, ri.currSeg.streetName);
      if(ri.currCrossing.action == RouteEnums::Finally ){
         nextStreet.Copy( *iDestinationName );
      }

      //Set left side traffic
      TBool leftSide = ri.currSeg.isLeftTraffic();
      iGuideView->SetLeftSideTraffic( leftSide );

      const char * detour = ri.hasDetourLandmark();
      const char * speedcam = ri.hasSpeedCameraLandmark();

      TInt exit = ri.currCrossing.exitCount;
      iGuideView->UpdateWayPoint( RouteAction(ri.currCrossing.action),
                                  RouteAction(nextAction),
                                  RouteCrossing(ri.currCrossing.crossingType),
                                  RouteCrossing(nextCross),
                                  currStreet, nextStreet,
                                  currDist, nextDist, exit, ri.lat, ri.lon,
                                  (ri.currSeg.isHighway() || nextHighway),
                                  detour?1:0, speedcam?1:0);


      iMapView->UpdatePicture( RouteAction(ri.currCrossing.action),
                               RouteCrossing(ri.currCrossing.crossingType),
                               currDist, leftSide,
                               (ri.currSeg.isHighway() || nextHighway),
                               detour?1:0, speedcam?1:0);
   } else if( newInfo ){
      if( ri.currCrossing.action == RouteEnums::Finally ){
         nextStreet.Copy( *iDestinationName );
      }
      iGuideView->UpdateStatus( ri.onTrackStatus,
                                currStreet, nextStreet, currDist );
      if( ri.onTrackStatus == Goal )
         iHasRoute = EFalse;
      // Must check if change in detour ( we would normaly turn here) or
      // speedcam.
      const char * detour = ri.hasDetourLandmark();
      const char * speedcam = ri.hasSpeedCameraLandmark();
      iGuideView->UpdateSpeedcam(speedcam?1:0);
      iGuideView->UpdateDetour(detour?1:0);
      iMapView->UpdateSpeedcam(speedcam?1:0);
      iMapView->UpdateDetour(detour?1:0);
   } else {
      
      const char * detour = ri.hasDetourLandmark();
      const char * speedcam = ri.hasSpeedCameraLandmark();
      iGuideView->UpdateSpeedcam(speedcam?1:0);
      iGuideView->UpdateDetour(detour?1:0);
      iGuideView->UpdateDistance( currDist );
      iMapView->SetTopBorder();
      iMapView->UpdateDistance( currDist );
      iMapView->UpdateSpeedcam(speedcam?1:0);
      iMapView->UpdateDetour(detour?1:0);
   }
   iInfoView->UpdateRouteDataL( ri.distToGoal, ri.timeToGoal, ri.currSeg.speedLimit );
   iRouteView->UpdateRouteDataL( ri.distToGoal, ri.timeToGoal, ri.toTarget );
}

void CWayFinderAppUi::HandleRouteList( RouteListMess* aMessage )
{
   if( iTabGroup->ActiveTabId() == KItineraryViewId.iUid )
      iItineraryView->SetRouteList( aMessage->getRouteList() );
   else if( iTabGroup->ActiveTabId() == KGuideViewId.iUid )
      iGuideView->SetRouteList( aMessage->getRouteList() );
   else if( iTabGroup->ActiveTabId() == KDestinationViewId.iUid )
      iRouteView->SetRouteList( aMessage->getRouteList() );
}

void CWayFinderAppUi::HandleSoundFileList( SoundFileListMess* aMessage )
{
   if (iAudioSlave) {
      ScriptAudioClipTable *table = new ScriptAudioClipTable(
                     aMessage->getNumEntries(), aMessage->getStringData());
      iAudioSlave->NewClipTable( table );
   }
}

void CWayFinderAppUi::HandlePrepareSounds( PrepareSoundsMess* aMessage )
{
   if (iAudioSlave) {
      iAudioSlave->PrepareSound( aMessage->getNumSounds(), aMessage->getSounds() );
   }
}

void CWayFinderAppUi::HandlePlaySounds()
{
   if (iAudioSlave) {
      iAudioSlave->Play();
   }
}

void
CWayFinderAppUi::CheckResources()
{

   //if (!m_wayfinder_path_found ) {
   if (iPathManager->GetWayfinderPath() == KNullDesC) {
      WFDialog::ShowScrollingWarningDialogL(R_WAYFINDER_PROGRAMMISSING_MSG, iCoeEnv);
#ifdef __RELEASE__
      HandleCommandL(EWayFinderExitAppl); // exits app.
#endif
      return;
   }

}

void CWayFinderAppUi::CheckOneResourceVersion(TDesC &resource_file, int min_major, int min_minor)
{
   TBuf8<KBuf256Length> fileContent;
   TInt symbErrCode = KErrNone;
   TBool resultOk = WFSymbianUtil::getBytesFromFile(m_fsSession, resource_file,
         fileContent, symbErrCode);
   if ( resultOk ){
      /* Check resource version. */
      _LIT8(underscoreChar, "_");
      TInt versionEnd = fileContent.Find(underscoreChar);
      if (versionEnd == KErrNotFound) {
         /* Failed to parse version string. */
         WFDialog::ShowScrollingWarningDialogL(R_WAYFINDER_RESOURCEMISSING_MSG, iCoeEnv);
#ifdef __RELEASE__
         HandleCommandL(EWayFinderExitAppl); // exits app.
#endif
         return;
      }
      TBuf<64> versionString;
      versionString.Copy(fileContent.Left(versionEnd));
      _LIT8(dotChar, ".");
      versionEnd = fileContent.Find(dotChar);
      if (versionEnd == KErrNotFound) {
         /* Failed to parse version string. */
         WFDialog::ShowScrollingWarningDialogL(R_WAYFINDER_RESOURCEMISSING_MSG, iCoeEnv);
#ifdef __RELEASE__
         HandleCommandL(EWayFinderExitAppl); // exits app.
#endif
         return;
      }

      TBuf<16> temp;

      temp.Copy(versionString.Ptr(), versionEnd);
      char *foo = WFTextUtil::newTDesDupL(temp);
      LOGNEWA(foo, char, strlen(foo) + 1);
      TInt major = 0;
      if(foo){
         //XXX this is not the proper way to signal failure!!!!!
         major= atoi(foo);
         LOGDELA(foo);
         delete[] foo;
      }
      temp.Copy(versionString.Right(
               versionString.Length()-(versionEnd+1)));
#if 0
      tempstring.Copy(_L("Resource version is: "));
      tempstring.AppendNum(major);
      tempstring.Append( _L(".") );
      tempstring.Append(temp);
      WFDialog::ShowScrollingWarningDialogL(tempstring);
#endif

      foo = WFTextUtil::newTDesDupL(temp);
      LOGNEWA(foo, char, strlen(foo) + 1);
      TInt minor= 0;
      if(foo){
         minor=atoi(foo);
         LOGDELA(foo);
         delete[] foo;
      }

      if (major < min_major ||
          (major == min_major &&
           minor < min_minor)) {
         TBuf<256> msg;
         TBuf<256> appendString;
         iCoeEnv->ReadResource( msg, R_TEXT_RESOURCE_TOO_OLD);
         msg.Append(_L(". "));
         iCoeEnv->ReadResource( appendString, R_TEXT_RESOURCE_YOUR_VERSION);
         msg.Append(appendString);
         msg.AppendNum(major);
         msg.Append(_L("."));
         msg.AppendNum(minor);
         msg.Append(_L("\n"));
         iCoeEnv->ReadResource( appendString, R_TEXT_RESOURCE_NEED_VERSION);
         msg.Append(appendString);
         msg.AppendNum(min_major);
         msg.Append(_L("."));
         msg.AppendNum(min_minor);
         msg.Append(_L("\n"));
         iCoeEnv->ReadResource( appendString, R_TEXT_RESOURCE_PROGRAM_EXIT);
         msg.Append(appendString);
         WFDialog::ShowScrollingWarningDialogL(msg, iCoeEnv);
#ifdef __RELEASE__
         HandleCommandL(EWayFinderExitAppl); // exits app.
#endif
         return;
      }

      //  Resources ok!
   }
   else{
      /* ! resultOk */
      if ( symbErrCode == KErrNotFound ){
         // Resource file missing.
         WFDialog::ShowErrorL(R_WAYFINDER_RESOURCEMISSING_MSG, iCoeEnv);
#ifdef __RELEASE__
         HandleCommandL(EWayFinderExitAppl); // exits app.
#endif
         return;
      }
      else {
         // Some other error occured while reading the
         // version file.
         WFDialog::ShowErrorL(R_WAYFINDER_INITPROGRAMPROBLEM_MSG, iCoeEnv);
#ifdef __RELEASE__
         HandleCommandL(EWayFinderExitAppl); // exits app.
#endif
         return;
      }
   }
}

TBool CWayFinderAppUi::IsAudioResourceOk()
{
   if (iPathManager->GetLangResourcePath() == KNullDesC) {
      return EFalse;
   }
   /* Check version number. */

   TBuf<256> filename;
   filename.Copy(iPathManager->GetLangResourcePath());
   filename.Append(KWayfinderResourceVersion);

   /* Try to read file. */
   TBuf8<KBuf256Length> fileContent;
   TInt symbErrCode = KErrNone;
/*    WFDialog::ShowScrollingWarningDialogL(filename); */

   TBool resultOk = WFSymbianUtil::getBytesFromFile(m_fsSession, filename,
         fileContent, symbErrCode);
   if ( resultOk ) {
      /* Check resource version. */
      _LIT8(underscoreChar, "_");
      TInt versionEnd = fileContent.Find(underscoreChar);
      if (versionEnd == KErrNotFound) {
/*          filename.Copy(_L("No find _")); */
/*          WFDialog::ShowScrollingWarningDialogL(filename); */
         return EFalse;
      }
      TBuf<64> versionString;
      versionString.Copy(fileContent.Left(versionEnd));
      _LIT8(dotChar, ".");
      versionEnd = fileContent.Find(dotChar);
      if (versionEnd == KErrNotFound) {
/*          filename.Copy(_L("No find .")); */
/*          WFDialog::ShowScrollingWarningDialogL(filename); */
         return EFalse;
      }

      TBuf<16> temp;

      temp.Copy(versionString.Ptr(), versionEnd);
      char *foo = WFTextUtil::newTDesDupL(temp);
      LOGNEWA(foo, char, strlen(foo) + 1);
      TInt major = 0;
      if(foo){
         //XXX this is not the proper way to signal failure!!!!!
         major= atoi(foo);
         LOGDELA(foo);
         delete[] foo;
      }
      temp.Copy(versionString.Right(
               versionString.Length()-(versionEnd+1)));

      foo = WFTextUtil::newTDesDupL(temp);
      LOGNEWA(foo, char, strlen(foo) + 1);
      TInt minor= 0;
      if(foo){
         minor=atoi(foo);
         LOGDELA(foo);
         delete[] foo;
      }

      if (major < WAYFINDER_MAJOR_LANG_RESOURCE_MINIMUM ||
          (major == WAYFINDER_MAJOR_LANG_RESOURCE_MINIMUM &&
           minor < WAYFINDER_MINOR_LANG_RESOURCE_MINIMUM)) {
/*          filename.Copy(_L("version to low")); */
/*          WFDialog::ShowScrollingWarningDialogL(filename); */
         return EFalse;
      }
/*       filename.Copy(_L("is ok")); */
/*       WFDialog::ShowScrollingWarningDialogL(filename); */
      return ETrue;
   }
/*    filename.Copy(_L("No read fokken file!")); */
/*    WFDialog::ShowScrollingWarningDialogL(filename); */
   return EFalse;
}

HBufC* CWayFinderAppUi::GetApplicationNameLC()
{
   HBufC* name = KApplicationVisibleName().AllocLC();
   //CleanupStack::PushL( name );
   return name;
}

HBufC* CWayFinderAppUi::GetApplicationVersionLC()
{
   HBufC* version = HBufC::NewLC(32); // leave object on cleanup stack
   version->Des().Num( isab::Nav2_Major_rel );
   version->Des().Append(_L("."));
   version->Des().AppendNum( isab::Nav2_Minor_rel );
   version->Des().Append(_L("."));
   version->Des().AppendNum( isab::Nav2_Build_id );
   return version;
}

TRgb CWayFinderAppUi::GetNewsBgColor() 
{
   return TRgb(KNewsBackgroundRed, KNewsBackgroundGreen, KNewsBackgroundBlue);
}

TPoint CWayFinderAppUi::GetNewsConStatusPos()
{
   return TPoint(NEWS_CONSTATUS_POS_X, NEWS_CONSTATUS_POS_Y);
}

void CWayFinderAppUi::HandleMapReply( MapReplyMess* /*aMessage*/ )
{
   TUid messageId;
   messageId.iUid = ENoMessage;
   TBuf8<16> customMessage( _L8("") );
   SetViewL( KMapViewId, messageId, customMessage );
}

void
CWayFinderAppUi::HandleSaveSearchHistoryL()
{
   Buffer *buf = new Buffer(1024);
   LOGNEW(buf, Buffer);
   uint32 numEntries = iDataHolder->GetNumSearchHistoryItems();
   buf->writeNext16bit(SEARCH_HISTORY_VERSION_NUMBER); /* Version. */
   buf->writeNext16bit(numEntries);
   for (uint32 i = numEntries; i > 0; i--) {
      SearchHistoryItem* shi = iDataHolder->GetSearchHistoryItem(i-1);
      if (shi) {
         buf->writeNextCharString(shi->GetSearchString());
         buf->writeNextCharString(shi->GetHouseNum());
         buf->writeNextCharString(shi->GetCityString());
         buf->writeNextCharString(shi->GetCityId());
         buf->writeNextCharString(shi->GetCountryString());
         buf->writeNextCharString(shi->GetCountryId());
      }
   }
   const uint8 *rawdata = buf->accessRawData();
   int32 len = buf->getLength();

   GeneralParameterMess* gpm = new (ELeave) GeneralParameterMess(
         GuiProtEnums::paramSearchStrings,
         (uint8*)rawdata, len);
   LOGNEW(gpm, GeneralParameterMess);
   SendMessageL( gpm );
   /* Don't delete data, it's owned by the buffer. */
   LOGDEL(gpm);
   delete gpm;
   LOGDEL(buf);
   delete buf;
}

void
CWayFinderAppUi::HandleSetGeneralParameter(GeneralParameterMess* message)
{
   // Which parameter is it?
   switch (message->getParamId()) {
      case GuiProtEnums::paramCategoryIds:
         {
            /* Get category id's and insert into dataholder. */
            iNewDestView->HandleDisableCategory();
            iDataHolder->SetCategoryIds(message->getNumEntries(),
                  message->getStringData());
         }
         break;
      case GuiProtEnums::paramCategoryNames:
         {
            iNewDestView->HandleDisableCategory();
            iDataHolder->SetCategoryStrings(message->getNumEntries(),
                  message->getStringData());
         }
         break;
      case GuiProtEnums::paramLatestNewsImage:
         {
            /* Save image to file. */
            TBuf<256> aFilename;
            aFilename.Copy(iPathManager->GetWayfinderPath());
#if defined NAV2_CLIENT_SERIES60_V3
            aFilename.Append( KSvgNews );
#else
            aFilename.Append( KGifNews );
#endif
            int res = WFSymbianUtil::writeBytesToFile(m_fsSession, aFilename,
                  message->getBinaryData(), message->getNumEntries(), iLog);
            iIniFile->latestNewsImageFailed = 0;
            if (res != 0) {
               /* Error writing file. */
               iIniFile->latestNewsImageFailed = 1;
            }
            iIniFile->Write();
         }
         break;
      case GuiProtEnums::paramLatestShownNewsChecksum:
         HandleLatestShowNewsChecksum(message->getIntegerData()[0]);
         break;
      case GuiProtEnums::paramLatestNewsChecksum:
         HandleLatestNewsChecksum(message->getIntegerData()[0]);
         break;
      case GuiProtEnums::paramSearchStrings:
         {
            /* Search history. */
            Buffer *buf = new Buffer(message->getBinaryData(),
                  message->getNumEntries(), message->getNumEntries());
            LOGNEW(buf, Buffer);
            /* Get version. */
            int16 version = buf->readNext16bit();
            if (version == SEARCH_HISTORY_VERSION_NUMBER) {
               /* OK. */
               int16 numEntries = buf->readNext16bit();
               iDataHolder->InitSearchHistory();
               for (int16 i = 0 ; i < numEntries ; i++) {
                  /* Read 6-tuples of searchstring, housenum, citystring, cityid, */
                  /* countrystring and countryid. */
                  const char *ss = buf->getNextCharString();
                  const char *housenum = buf->getNextCharString();
                  const char *cis = buf->getNextCharString();
                  const char *cii = buf->getNextCharString();
                  const char *cos = buf->getNextCharString();
                  const char *coi = buf->getNextCharString();
                  /* Add it to the dataholder. */
                  iDataHolder->AddSearchHistoryItemL(ss, housenum, cis, cii, cos, coi, ETrue);
               }
            } else {
               /* Unknown search history version. */
               /* The search history will be rewritten when the user */
               /* does his next search. */
            }
            buf->releaseData();
            LOGDEL(buf);
            delete buf;
         }
         break;
      case GuiProtEnums::paramSearchCountry:
      case GuiProtEnums::paramPositionSelectData:
      case GuiProtEnums::paramMyDestIndex:
         break;
      case GuiProtEnums::paramLastKnownRouteEndPoints:
         {
#if 0
            /* The last known endpoints should not be cached. */
            int32 *data = message->getIntegerData();
            if (message->getNumEntries() >= 4) {
               SetCurrentRouteCoordinates(data[0], data[1], data[2], data[3]);
            } else {
               WFDialog::ShowDebugDialogL("GOT param REP < 4!");
            }
#endif
         }
         break;
      case GuiProtEnums::paramLastKnownRouteId:
         {
/* #define USE_LAST_KNOWN_ROUTE_ID_PARAMETER */
#ifdef USE_LAST_KNOWN_ROUTE_ID_PARAMETER
            int32 len = message->getNumEntries();
            uint8 *data = message->getBinaryData();

            Buffer *buf = new Buffer(data, len, 8);
            LOGNEW(buf, Buffer);
            m_last_routeid = buf->readNextUnaligned64bit();

            buf->releaseData();
            LOGDEL(buf);
            delete buf;
#endif
         }
         break;
      case GuiProtEnums::paramLanguage:
            break;
      case GuiProtEnums::paramServerCallCenters: //Deprecated
         // XXX Deprecated XXX
         break;
      case GuiProtEnums::paramSelectedAccessPointId:
         {
            SetIAP(*(message->getIntegerData()));
         }
         break;
      case GuiProtEnums::paramLinkLayerKeepAlive:
         {
            iSettingsData->iLinkLayerKeepAlive = *(message->getIntegerData());
            UpdateConnectionManagerL();
         }
         break;
      case GuiProtEnums::paramWayfinderType:
         {
            HandleWayfinderType(GuiProtEnums::WayfinderType(*(message->getIntegerData())));
         }
         break;
      case GuiProtEnums::paramTollRoads:
         if( message->getNumEntries() > 0 ) {
            iSettingsView->SetTollRoads( NavServerComEnums::RouteTollRoads(*(message->getIntegerData())) );
         }
         break;
      case GuiProtEnums::paramHighways:
         if( message->getNumEntries() > 0 ) {
            iSettingsView->SetHighways( NavServerComEnums::RouteHighways(*(message->getIntegerData())) );
         }
         break;
      case GuiProtEnums::paramAutoTracking:
         if( message->getNumEntries() > 0 ) {
            iSettingsView->SetAutoTrackingOn( *(message->getIntegerData())!=0 );
         }
         break;
      case GuiProtEnums::paramTimeDist:
         if( message->getNumEntries() > 0 ) {
            iSettingsView->SetTimeDist( NavServerComEnums::RouteCostType(*(message->getIntegerData())) );
         }
         break;
      case GuiProtEnums::paramTimeLeft:
/*          iTrialView->HandleExpireVectorL(message->getIntegerData(), */
/*                                          message->getNumEntries()); */
         break;
      case GuiProtEnums::paramGPSAutoConnect:
         if (message->getNumEntries() > 0 ){
            iSettingsView->SetGPSAutoConnect( *message->getIntegerData());
         }
         break;
      case GuiProtEnums::paramVectorMapSettings:
         {
            // WARNING! The vector map blob is generated in the make_seed.pl file!
            int32* data = message->getIntegerData();
            /* Got settings for vector maps. */
            if (data[vmap_set_version_pos] != VECTOR_MAP_SETTINGS_VERSION) {
               /* Failed! Not the same version. */
               break;
            }
            /* Decrease by one to allow comparison with positions. */
            int32 num_data = message->getNumEntries() - 1;
            if (num_data >= vmap_set_cache_pos) {
               int32 cacheSize = data[vmap_set_cache_pos];
               iSettingsView->SetMapCacheSize(cacheSize);
               iMapView->SetMapCacheSize( cacheSize );
            }
            if (num_data >= vmap_set_maptype_pos) {
/*                MapType mapType = (MapType)data[vmap_set_maptype_pos]; */
/*                iSettingsView->SetMapType(mapType); */
/*                iMapView->SetUseVectorMaps( mapType == EVector ); */
            }
            if (num_data >= vmap_set_orientation_pos) {
               TrackingType trackingOrientation =
                  (TrackingType)data[vmap_set_orientation_pos];
               iSettingsView->SetTrackingType(trackingOrientation);
               iMapView->SetTrackingType( trackingOrientation );
            }
            if (num_data >= vmap_set_favorite_show_pos) {
               iSettingsView->SetFavoriteShow(data[vmap_set_favorite_show_pos]);
            }
            if (num_data >= vmap_set_guide_mode_pos) {
               iSettingsView->SetPreferredGuideMode(
                     (preferred_guide_mode)data[vmap_set_guide_mode_pos]);
            }
            if (num_data >= vmap_set_gui_mode_pos) {
/*                iSettingsView->SetGuiMode(data[vmap_set_gui_mode_pos]); */
            }
         }
         break;
      case GuiProtEnums::paramPoiCategories:
         {
            /* Poi category list. */
            int32 len = message->getNumEntries();
            uint8 *data = message->getBinaryData();

            Buffer *buf = new Buffer(data, len, len);
            LOGNEW(buf, Buffer);

            iMapView->ConvertCategoriesSettingsToTileCategories(
                  buf,
                  iSettingsData);

            /* Release data, it's not the buffers anyways. */
            buf->releaseData();
            LOGDEL(buf);
            delete buf;

         }
         break;
      case GuiProtEnums::paramMapLayerSettings:
         {
            /* Map layer info list. */
            int32 len = message->getNumEntries();
            uint8 *data = message->getBinaryData();

            iMapView->ConvertMapLayerSettingsToLayerInfo(
                  data, len,
                  iSettingsData);

            /* Release data, it's not the buffer's anyways. */
         }
         break;
      case GuiProtEnums::paramVectorMapCoordinates:
         {
            VectorMapCoordinates* vmc = new (ELeave) VectorMapCoordinates(message);

            if (vmc->Valid()) {
               /* Set coordinates in Mapview. */
               iMapView->SetVectorMapCoordinates(
                     vmc->GetScale(),
                     vmc->GetLat(),
                     vmc->GetLon());
            }
            delete vmc;
         }
         break;
      case GuiProtEnums::paramTrackingLevel : {
         if ( message->getNumEntries() > 0 ) {
            iSettingsView->setTrackingLevel( *message->getIntegerData() );
         }
      } break;
      case GuiProtEnums::paramTrackingPIN : {
         // Make TrackPINList 
         TrackPINList* l = new(ELeave) TrackPINList( 
            message->getBinaryData(), message->getNumEntries() );

         iSettingsView->setTrackPINList( l );
      } break;
      case GuiProtEnums::userRights : {
         iSettingsData->iUserRights->setUserRights(
            message->getIntegerData(), message->getNumEntries() );
      } break;
      case GuiProtEnums::paramBtGpsAddressAndName:
         if (iDataStore->iWFGpsData->getReadyToUseGps() &&
             !iDoingShutdown){
            char** tmp = message->getStringData();
            const char** tmp2 = (const char**)tmp;
            iSettingsView->SetBtGpsAddressL(tmp2, message->getNumEntries());
            if(!iBtHandler){
#ifdef BT_AUTO_CONNECT_DELAY
               //delay connection by BT_AUTO_CONNECT_DELAY if symbian 7s.
               iEventGenerator->SendEventL(EWayfinderEventConnectToGps, 
                                           IfSymbian7s(BT_AUTO_CONNECT_DELAY,
                                                       0));
#else
               iEventGenerator->SendEventL(EWayfinderEventConnectToGps);
#endif
            } 
            if(iBtHandler){ //XXX will we need this? done in creategpsl
               iBtHandler->SetAddress(tmp2, message->getNumEntries());
            }
         }
         break;
      case GuiProtEnums::paramUserTermsAccepted:
         if (*message->getIntegerData() ==
               isab::GuiProtEnums::UserTermsAccepted) {
            m_startupHandler->StartupEvent(SS_UserTermsNotNeeded);
         } else {
            m_startupHandler->StartupEvent(SS_UserTermsNeeded);
         }
         break;
      case GuiProtEnums::userTrafficUpdatePeriod: {
         TBool routeTrafficInfoEnabled = ETrue;
         int updTime = *message->getIntegerData();
         if (updTime & 0x40000000) {
            // Check if the second highest bit is set.
            routeTrafficInfoEnabled = EFalse;
            updTime &= 0xbfffffff;
         }
         iSettingsView->setRouteTrafficInfoUpdEnabled(routeTrafficInfoEnabled);            
         iSettingsView->setRouteTrafficInfoUpdTime(updTime);
      } 
         break;
      default:
         /* Unknown. Drop it. */
         break;
   }
}

void
CWayFinderAppUi::HandleLatestShowNewsChecksum(uint32 checksum)
{
   iDataStore->iWFNewsData->setLatestShownNewsChecksum(checksum);
   CheckLatestNewsChecksum();
}

void
CWayFinderAppUi::HandleLatestNewsChecksum(uint32 checksum)
{
   iDataStore->iWFNewsData->setLatestNewsChecksum(checksum);
   CheckLatestNewsChecksum();
}

void
CWayFinderAppUi::CheckLatestNewsChecksum()
{
   if (iDataStore->iWFNewsData->checksumMatch()) {
      /* Checksums match, and both data are available. */
      m_startupHandler->StartupEvent(SS_NewsNotNeeded);
   } else {
      m_startupHandler->StartupEvent(SS_NewsNeeded);
   }
}

void
CWayFinderAppUi::HandleGetGeneralParameter(GeneralParameterMess* message)
{
   /* The parameter was not set in Nav2. */
   switch (message->getParamId()) {
      case GuiProtEnums::paramPoiCategories:
         iMapView->ConvertCategoriesSettingsToTileCategories(NULL,
               iSettingsData);
         break;

      case GuiProtEnums::paramLatestShownNewsChecksum:
         HandleLatestShowNewsChecksum(MAX_UINT32-1);
         break;
      case GuiProtEnums::paramLatestNewsChecksum:
         HandleLatestNewsChecksum(MAX_UINT32-2);
         break;
      case GuiProtEnums::paramMapLayerSettings:
         iMapView->ConvertMapLayerSettingsToNav2Parameter(NULL, iSettingsData);
         break;
      case GuiProtEnums::paramUserTermsAccepted:
         m_startupHandler->StartupEvent(SS_UserTermsNeeded);
         break;
      default:
         break;
   }
}


#if 0
namespace {
   void ShowUpgradeErrorL(class LicenseReplyMess& aMess, class CCoeEnv& aEnv)
   {
      TInt resource = 0;
      if( ! aMess.isKeyOk() ){
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_REG_KEY_MSG;
      } else if( ! aMess.isPhoneOk() ){
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_NUMBER_MSG;
      } else if( ! aMess.isRegionOk() ){
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_REGION_MSG;
      } else if( ! aMess.isEmailOk() ){
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_EMAIL_MSG;
      } else if( ! aMess.isNameOk() ){      
         resource = R_WAYFINDER_REGISTER_GOLD_FAIL_NAME_MSG;
      }
      if(resource != 0){
         WFDialog::ShowErrorL(resource, &aEnv);
      }
   }
}

void CWayFinderAppUi::HandleLicenseUpgradeReply(class LicenseReplyMess* mess)
{
   if (iUpgradeDialog) {
      iUpgradeDialog->SetNotInProgress();
   }

   if( !mess->allOk() ) {
      ShowUpgradeErrorL(*mess, *(iCoeEnv));
      return;
   }

   TInt resource = 0;
   switch(GuiProtEnums::WayfinderType(mess->getWayfinderType())){
   case GuiProtEnums::Gold:
      resource = R_WAYFINDER_REGISTER_GOLD_SUCCESS_MSG;
      break;
   case GuiProtEnums::Iron:
      resource = R_WAYFINDER_REGISTER_IRON_SUCCESS_MSG;
      break;
   case GuiProtEnums::Silver:
      //Silver case here
      resource = R_WAYFINDER_REGISTER_SILVER_SUCCESS_MSG;
      break;
   case GuiProtEnums::Lithium:
      resource = R_WAYFINDER_REGISTER_LITHIUM_SUCCESS_MSG;
      break;
   case GuiProtEnums::Trial:
   case GuiProtEnums::InvalidWayfinderType: //impossible :)
      //trial here
      //no message
      break;
   }

   if(0 != resource){
      HBufC* text = iCoeEnv->AllocReadResourceLC(resource);
      class CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL(*text);
      CleanupStack::PopAndDestroy(text);
      dlg->ExecuteLD( R_MESSAGE_DIALOG );
      iUpgradeFromView = KStartPageViewId;
      CloseUpgradeDialog( ETrue );
   }
}
#endif

TBool CWayFinderAppUi::IsInTabOrder( TInt32 aUid )
{
   TBool isInTabOrder = ETrue;

   if (HasRoute()) {
      if (aUid == KContactsViewId.iUid ||
          aUid == KMapViewId.iUid ||
          aUid == KMyDestViewId.iUid ||
          aUid == KNewDestViewId.iUid ||
          aUid == KLockedNavViewId.iUid ||
          aUid == KPositionSelectViewId.iUid ||
          aUid == KServiceWindowViewId.iUid ||
          aUid == KSettingsViewId.iUid ||
          aUid == KWelcomeViewId.iUid) {
//          CEikonEnv::Static()->InfoWinL(_L("Tab not in order"), _L(""));
         isInTabOrder = EFalse;
      }
   } else {
      if (aUid == KContactsViewId.iUid ||
          aUid == KDestinationViewId.iUid ||
          aUid == KGuideViewId.iUid ||
          aUid == KLockedNavViewId.iUid ||
          aUid == KItineraryViewId.iUid ||
          aUid == KMapViewId.iUid ||
          aUid == KMyDestViewId.iUid ||
          aUid == KNewDestViewId.iUid ||
          aUid == KPositionSelectViewId.iUid ||
          aUid == KServiceWindowViewId.iUid ||
          aUid == KSettingsViewId.iUid ||
          aUid == KWelcomeViewId.iUid) {
         isInTabOrder = EFalse;
      }
   }
   if (IsIronVersion()) {
      if (aUid == KGuideViewId.iUid ||
          aUid == KItineraryViewId.iUid) {
         isInTabOrder = EFalse;
      }
   }
   return isInTabOrder;
}

void
CWayFinderAppUi::DeleteIapTxt()
{
   HBufC* dir = iPathManager->GetWayfinderPath().AllocLC();
   _LIT(KIAPFile, "iap.txt");
   HBufC* file = HBufC::NewLC(dir->Length() + KIAPFile().Length());
   file->Des().Copy(*dir);
   file->Des().Append(KIAPFile);
   IAPFileHandler::DeleteIAPFile(*file);
   CleanupStack::PopAndDestroy(file);
   CleanupStack::PopAndDestroy(dir);
}

/***************************************************/
/*************** From Base Classes *****************/
/***************************************************/

void
CWayFinderAppUi::StartIapSearch()
{
   ShowIAPMenu();
}

void
CWayFinderAppUi::RestartIapSearch()
{
   DeleteIapTxt();
   SetIAP(-1);
   ShowIAPMenu();
}

void CWayFinderAppUi::TimerExpired()
{
   if( !CheckWait() ){
      //If the timer is used for something else than the rotateting logo.
      deleteNav2();
   }
}

// ------------------------------------------------------------------------------
// CWayFinderAppUi::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CWayFinderAppUi::DynInitMenuPaneL( TInt aResourceId,
                                        CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_SERVICE_MENU ){

      if (!CanUseSubmitCode()) {
         aMenuPane->SetItemDimmed(EWayFinderCmdStartPageUpgradeInfo2, ETrue);
      }
   } else if( aResourceId == R_WAYFINDER_APP_MENU ){
      if (IsIronVersion()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdRouteSubMenu, ETrue );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdRouteSubMenu, EFalse );
      }
      TBool showBuyExtension = EFalse;
#if defined(ALWAYS_SHOW_BUYEXTENSION)
      showBuyExtension = ETrue;
#endif
      aMenuPane->SetItemDimmed(EWayFinderCmdServicesBuyExtensionWapLink, !showBuyExtension );
#if defined NAV2_CLIENT_SERIES60_V1
      // Always hide Buy extension for s60v1 (no ssl).
      aMenuPane->SetItemDimmed(EWayFinderCmdServicesBuyExtensionWapLink, ETrue);
#endif
      aMenuPane->SetItemDimmed( EWayFinderCmdGPSConnect, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdGPSDisConnect, ETrue );
      if ( !hasFleetRight() ) {
         aMenuPane->SetItemDimmed( EWayFinderCmdFleetInfo, ETrue );
      }
      if( IsGpsConnectedAndNotSimulating() ){
         aMenuPane->SetItemDimmed( EWayFinderCmdGPSDisConnect, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdGPSConnect, EFalse );
      }
#if !defined(USE_REPORT_EVENT_MENU)
      aMenuPane->SetItemDimmed( EWayFinderCmdReportEvent, ETrue);
#endif
      if( iTabGroup->ActiveTabId() != KStartPageViewId.iUid ){
         aMenuPane->SetItemDimmed( EAknSoftkeyExit, ETrue);
      }
   } else if( aResourceId == R_WAYFINDER_VIEWS_MENU ){
      aMenuPane->SetItemDimmed( EWayFinderCmdConnect, ETrue);
      if (HasRoute()) {
/*          aMenuPane->SetItemDimmed( EWayFinderCmdGuide, EFalse); */
/*          aMenuPane->SetItemDimmed( EWayFinderCmdItinerary, EFalse); */
/*          aMenuPane->SetItemDimmed( EWayFinderCmdDestination, EFalse); */
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdGuide, ETrue);
         aMenuPane->SetItemDimmed( EWayFinderCmdItinerary, ETrue);
         aMenuPane->SetItemDimmed( EWayFinderCmdDestination, ETrue);
      }
   } else if( aResourceId == R_WAYFINDER_ROUTE_MENU ){
      if (HasRoute()) {
         if (IsSimulating()) {
            aMenuPane->SetItemDimmed( EWayFinderCmdSave, ETrue);
            aMenuPane->SetItemDimmed( EWayFinderCmdClear, ETrue);
            aMenuPane->SetItemDimmed( EWayFinderCmdSimulateStart, ETrue );
            if (IsSimulationPaused()) {
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulatePause, ETrue );
            } else {
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateResume, ETrue );
            }
            if (IsSimulationRepeating()) {
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateRepeatOn, ETrue );
            } else {
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateRepeatOff, ETrue );
            }
            if (IsSimulationMaxSpeed()) {
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateIncSpeed, ETrue );
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateDecSpeed, EFalse );
            } else if (IsSimulationMinSpeed()) {
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateDecSpeed, ETrue );
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateIncSpeed, EFalse );
            } else {
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateIncSpeed, EFalse );
               aMenuPane->SetItemDimmed(EWayFinderCmdSimulateDecSpeed, EFalse );
            }
         } else {
            aMenuPane->SetItemDimmed( EWayFinderCmdSimulateStop, ETrue );
            aMenuPane->SetItemDimmed( EWayFinderCmdSimulatePause, ETrue );
            aMenuPane->SetItemDimmed( EWayFinderCmdSimulateResume, ETrue );
            aMenuPane->SetItemDimmed( EWayFinderCmdSimulateIncSpeed, ETrue );
            aMenuPane->SetItemDimmed( EWayFinderCmdSimulateDecSpeed, ETrue );
            aMenuPane->SetItemDimmed( EWayFinderCmdSimulateRepeatOn, ETrue );
            aMenuPane->SetItemDimmed( EWayFinderCmdSimulateRepeatOff, ETrue );
         }
      }
      else {
         aMenuPane->SetItemDimmed( EWayFinderCmdClear, ETrue);
         aMenuPane->SetItemDimmed( EWayFinderCmdSave, ETrue);
         aMenuPane->SetItemDimmed( EWayFinderCmdSimulateStart, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdSimulateStop, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdSimulatePause, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdSimulateResume, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdSimulateIncSpeed, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdSimulateDecSpeed, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdSimulateRepeatOn, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdSimulateRepeatOff, ETrue );
      }
   } else if( aResourceId == R_WAYFINDER_TOOLS_MENU ){

      // Vestigal traces...
      aMenuPane->SetItemDimmed(EWayFinderCmdEmptyMapCache, ETrue);
      aMenuPane->SetItemDimmed( EWayFinderCmdStartNavigation, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdStopNavigation, ETrue );

      if( IsGpsConnectedAndNotSimulating() ){
         aMenuPane->SetItemDimmed( EWayFinderCmdGPSConnect, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdGPSDisConnect, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdGPSConnect, EFalse );
         aMenuPane->SetItemDimmed( EWayFinderCmdGPSDisConnect, ETrue );
      }
      if( IsGpsConnected() ){
         aMenuPane->SetItemDimmed( EWayFinderCmdInfo, EFalse );
         aMenuPane->SetItemDimmed( EWayFinderCmdConnectSendCurrentPosition, EFalse );
         if (CanUseGPSForEveryThing()) {
            aMenuPane->SetItemDimmed( EWayFinderCmdInfoShow, EFalse );
         }
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdInfo, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdConnectSendCurrentPosition, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdInfoShow, ETrue );
      }

#ifdef __RELEASE__
      aMenuPane->SetItemDimmed( EWayFinderCmdDebugAction, ETrue );
#endif
   } else if ( aResourceId == R_WAYFINDER_HELP_MENU ) {
#if !defined(USE_REPORT_EVENT_MENU)
      aMenuPane->SetItemDimmed( EWayFinderCmdReportEvent, ETrue);
#endif
   } else if ( aResourceId == R_AVKON_FORM_MENUPANE ) {
      // Else we don't wan't Fleet Info, at least.
/*       aMenuPane->SetItemDimmed( EWayFinderCmdFleetInfo, ETrue ); */
   }
}

// ----------------------------------------------------
// CWayFinderAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// ----------------------------------------------------
#ifdef __WINS__
#define KEY_STAR            EStdKeyNkpAsterisk
#else
#define KEY_STAR            0x2a
#endif
TKeyResponse CWayFinderAppUi::HandleKeyEventL( const TKeyEvent& aKeyEvent,
                                               TEventCode aType)
{
   if ( iTabGroup == NULL ){
      return EKeyWasNotConsumed;
   }
   TInt active = iTabGroup->ActiveTabIndex();
   TInt count = iTabGroup->TabCount();

   if (!IsInTabOrder( iTabGroup->TabIdFromIndex(active) ) &&
         iTabGroup->TabIdFromIndex(active) != KMapViewId.iUid ) {
      return EKeyWasNotConsumed;
   }

   if (iUrgentShutdown) {
      HandleCommandL( EWayFinderExitAppl );
   }
//    TRACE_DBG("iCode: %ld iScanCode: %ld iModifiers: %ld iRepeats: %ld", 
//              (long int) aKeyEvent.iCode, (long int) aKeyEvent.iScanCode, 
//              (long int) aKeyEvent.iModifiers, (long int ) aKeyEvent.iRepeats);

#ifdef NAV2_CLIENT_SERIES60_V3
   
   // XXX We need to handle the KeyEvents specially for s60v3 because every 
   //     time a user presses a key (in this case left or right) the system
   //     generates multiple events for iScanCode on N93. The keyEvent iCode
   //     is just being generated once for each button press.

   switch ( aKeyEvent.iCode )
   {
   case EKeyLeftArrow:
      {
//         TRACE_DBG("Doing left switch: iCode: %ld iScanCode: &ld", 
//                   (long int) aKeyEvent.iCode, (long int) aKeyEvent.iScanCode);
         /* Let start-view and mapview (normal mode) be included */
         /* in tab order. */
         if ( active > 0 ){
            /* Still not on last tab to the left. */
            active--;
            while( !IsInTabOrder( iTabGroup->TabIdFromIndex(active) ) ) {
               /* Jump any tab that shouldn't be visible. */
               active--;
            }
            if (active > 0) {
               iTabGroup->SetActiveTabByIndex( active );

               TUid messageId;

               messageId.iUid = ENoMessage;
               TBuf8<16> customMessage( _L8("") );

               TUid viewId = TUid::Uid(iTabGroup->TabIdFromIndex(active));
   
               ActivateLocalViewL(viewId,
                                  messageId,
                                  customMessage );
            } else if (active == 0) {
               /* Go to start view */
               GotoStartViewL();
            }
         }
      }
      return EKeyWasConsumed;
      break;
   case EKeyRightArrow:
      {
         /** Removed since IsInTabOrder() handles all this now.
         TBool canmove = EFalse;
         if ( active == 0 && HasRoute()) {
            canmove=ETrue;
         } else if (active > 0) {
            canmove=ETrue;
         }
         if ( canmove && (active < count) ) {
         */
         if ( active < count ) {
            active++;
            while( !IsInTabOrder( iTabGroup->TabIdFromIndex(active) ) )
               active++;
            if ((active) < count) {
               iTabGroup->SetActiveTabByIndex( active );
               TUid messageId;
               messageId.iUid = ENoMessage;
               TBuf8<16> customMessage( _L8("") );
               
               TUid viewId = TUid::Uid(iTabGroup->TabIdFromIndex(active));
               
               ActivateLocalViewL( viewId,
                                   messageId,
                                   customMessage );
            }
         }
      }
      return EKeyWasConsumed;
      break;
   }
   
   if(HandleHotKeyEventL(aKeyEvent, aType) == EKeyWasConsumed) {
      return EKeyWasConsumed;
   }   

   if (aType != EEventKeyUp) {
      return EKeyWasNotConsumed;
   }
   if (aKeyEvent.iScanCode == KEY_STAR) {
//      TRACE_DBG("Star!!! iCode: %ld iScanCode: %ld iModifiers: %ld iRepeats: %ld", 
//                (long int) aKeyEvent.iCode, (long int) aKeyEvent.iScanCode, 
//                (long int) aKeyEvent.iModifiers, (long int ) aKeyEvent.iRepeats);
      HandleCommandL(EWayfinderCmdAsterisk);
   }
   return EKeyWasConsumed;

#else

   // XXX Look at the info above, just under #ifdef NAV2_CLIENT_SERIES60_V3
   
   if (aType != EEventKeyUp) {
      return EKeyWasNotConsumed;
   }

   switch ( aKeyEvent.iScanCode )
   {
   case EStdKeyLeftArrow:
      {
         /* Let start-view and mapview (normal mode) be included */
         /* in tab order. */
         if ( active > 0 ){
            /* Still not on last tab to the left. */
            active--;
            while( !IsInTabOrder( iTabGroup->TabIdFromIndex(active) ) ) {
               /* Jump any tab that shouldn't be visible. */
               active--;
            }
            if (active > 0) {
               iTabGroup->SetActiveTabByIndex( active );

               TUid messageId;

               messageId.iUid = ENoMessage;
               TBuf8<16> customMessage( _L8("") );
               ActivateLocalViewL(
                     TUid::Uid( iTabGroup->TabIdFromIndex(active)),
                     messageId,
                     customMessage );
            } else if (active == 0) {
               /* Go to start view */
               GotoStartViewL();
            }
         }
      }
      break;
   case EStdKeyRightArrow:
      {
         /** Removed since IsInTabOrder() handles all this now.
         TBool canmove = EFalse;
         if ( active == 0 && HasRoute()) {
            canmove=ETrue;
         } else if (active > 0) {
            canmove=ETrue;
         }
         if ( canmove && (active < count) ) {
         */
         if ( active < count ) {
            active++;
            while( !IsInTabOrder( iTabGroup->TabIdFromIndex(active) ) )
               active++;
            if ((active) < count) {
               iTabGroup->SetActiveTabByIndex( active );
               TUid messageId;
               messageId.iUid = ENoMessage;
               TBuf8<16> customMessage( _L8("") );
               ActivateLocalViewL( TUid::Uid(iTabGroup->TabIdFromIndex(active)), messageId, customMessage );
            }
         }
      }
      break;
   case KEY_STAR:
//      TRACE_DBG("Star!!! iCode: %ld iScanCode: %ld iModifiers: %ld iRepeats: %ld", 
//                (long int) aKeyEvent.iCode, (long int) aKeyEvent.iScanCode, (long int) aKeyEvent.iModifiers, 
//                (long int ) aKeyEvent.iRepeats);
      HandleCommandL(EWayfinderCmdAsterisk);
      break;
   }

   if(HandleHotKeyEventL(aKeyEvent, aType) == EKeyWasConsumed) {
      return EKeyWasConsumed;
   }   

   return EKeyWasConsumed;
#endif
}

bool
CWayFinderAppUi::getShouldMute() const
{
   if ( m_phoneCallInProgress ) {
      //TRACE_FUNC();
      // Always mute when phone is ringing.
      return true;
   }
   if ( iSettingsData == NULL ) {
      //TRACE_FUNC();
      // Assume no mute if we have no settings.
      return false;
   }

   if ( ! iSettingsData->iMuteWhenInBackground ) {
      return ! iSettingsData->iUseSpeaker;
   }
   
   bool keyLock = false;
   if ( ! iForeGround ) {
      //TRACE_FUNC();
      // Don't mute if it's keylock.
      RAknKeyLock aAknKeylock ;
      aAknKeylock.Connect () ;
      keyLock = aAknKeylock.IsKeyLockEnabled();
      aAknKeylock.Close ();
   }
   
   // Mute if in background and it is forbidden to play in background
   // Also mute if forbidden to use speaker.
   return ( iSettingsData->iMuteWhenInBackground && !iForeGround && !keyLock) 
      || (! iSettingsData->iUseSpeaker );
}

void
CWayFinderAppUi::updateMute()
{
   if ( iAudioSlave == NULL ) {
      //TRACE_FUNC1( "iAudioSlave == NULL" );      
      return;
   }
   bool shouldMute = getShouldMute();
   // Avoid setting needlessly
   if ( shouldMute && !iAudioSlave->IsMute() ) {
      //TRACE_FUNC1( "iAudioSlave->SetMute( true )");
      iAudioSlave->SetMute( true );
   } else if ( (!shouldMute) && iAudioSlave->IsMute() ) {
      //TRACE_FUNC1( "iAudioSlave->SetMute( false )");
      iAudioSlave->SetMute( false );
   }
}

void
CWayFinderAppUi::HandleForegroundEventL(TBool aForeground)
{
   iForeGround = aForeground;
   updateMute();

   if ( iForeGround ) {
      /* Just move to the last view we were at. */
      iTabGroup->SetActiveTabByIndex( iTabGroup->ActiveTabIndex() );
      ActivateLocalViewL(TUid::Uid( iTabGroup->TabIdFromIndex( 
                                    iTabGroup->ActiveTabIndex() ) ));
   }
   
   CEikAppUi::HandleForegroundEventL(aForeground);
}

void
CWayFinderAppUi::ProcessMessageL(TUid aUid,const TDesC8& aParams)
{
   if (aUid == KUidApaMessageSwitchOpenFile){
      TLex8 lex(aParams);
      TInt32 id;
      if (lex.Val(id) == KErrNone) {
#ifndef NAV2_CLIENT_SERIES60_V3
         iCommunicateHelper->HandleSMS(id);
#endif
      }
#if !defined(__RELEASE__)
      else {
         // DEBUG
         WFDialog::ShowWarningL(_L("SMS not a valid ID! "));
      }
#endif
   } else {
      CEikAppUi::ProcessMessageL(aUid, aParams);
   }
}

TBool
CWayFinderAppUi::ProcessCommandParametersL(TApaCommand /*aCommand*/,
                                           TFileName& /*aDocumentName*/,
                                           const TDesC8& aTail)
{
   TLex8 lex(aTail);
   TInt32 id;
   if (lex.Val(id) == KErrNone)
   {
#ifndef NAV2_CLIENT_SERIES60_V3
      iCommunicateHelper->HandleSMS(id);
#endif
   }
   return ETrue;
}

void CWayFinderAppUi::HandleProgressIndicator(GenericGuiMess* message)
{
   switch(GuiProtEnums::ServerActionType(message->getSecondUint8())){
   case GuiProtEnums::DownloadingMap:
      SetWait(message->getFirstUint8(), R_WAYFINDER_CREATING_MAP_MSG);
      break;
   case GuiProtEnums::CreatingRoute:
      SetWait(message->getFirstUint8(), R_WAYFINDER_ROUTING_MSG);
      break;
   case GuiProtEnums::PerformingSearch:
      SetWait(message->getFirstUint8(), R_WAYFINDER_SEARCHING_MSG);
      break;
   case GuiProtEnums::Synchronizing:
      SetWait(message->getFirstUint8(), R_WAYFINDER_SYNCH_MSG);
      break;
   case GuiProtEnums::Upgrading:
   case GuiProtEnums::SettingPasswd:
   case GuiProtEnums::RetrievingInfo:
   case GuiProtEnums::CellInfoReport:
   case GuiProtEnums::InvalidActionType:
      SetWait(message->getFirstUint8(), R_WAYFINDER_PROCESSING_MSG);
      break;
   case GuiProtEnums::VectorMap:
      SetWait(message->getFirstUint8(), 0 ); //no change to titlebar
      break;
   }
}

void
CWayFinderAppUi::killNav2()
{
   if( iTimer != NULL ){ 
      iTimer->Cancel(); 
   }
   iWaiting=EFalse;
/*    TRACE_FUNC(); */

   // It seems like sometimes this exits the program
   LOGDEL(m_nav2);
   delete m_nav2;
   m_nav2 = NULL;
/*    TRACE_FUNC(); */
   if (iLog) {
      iLog->info("After delete nav2");
   }

#undef SHUTDOWN_TIMER_USES_CALLBACK
#ifdef SHUTDOWN_TIMER_USES_CALLBACK 
   iTimer->After(200000);
#else
   
   RTimer timer;
   if ( timer.CreateLocal() == KErrNone ) {
      TRequestStatus status;
      timer.After( status, 300000 );
      User::WaitForRequest( status );
      timer.Close();
   }
  
   deleteNav2();
#endif
}

void
CWayFinderAppUi::deleteNav2()
{
   if( iGpsSocket ){
      iGpsSocket->Disconnect();
   }
#ifdef NOKIALBS
   if(iPositionProvider){
      iPositionProvider->DisconnectL();
   }
#endif
   iMapView->ReleaseMapControls();
   if (iLog) {
      iLog->info("just before exit");
   }

/*    TRACE_FUNC(); */
   TRACE_FUNC();
   Exit();
   TRACE_FUNC();
}

void
CWayFinderAppUi::doExit()
{
   iDoingShutdown = ETrue;
   // This must be done. Sometimes the deletion of nav2 exits the prog.
   LOGDEL( iAudioSlave );
   delete iAudioSlave;  
   iAudioSlave = NULL;
   
   if (iBtHandler) {
      LOGDEL(iBtHandler);
      MPositionInterface::CloseAndDeleteL(iBtHandler);
//      /*       TRACE_FUNC(); */
   }
   if(iMessageHandler){ 
      iMessageHandler->Release();
      TRACE_FUNC(); 
   } 
#ifdef RELEASE_CELL
   delete iCC;
   iCC = NULL;
#endif

   LOGDEL(iMessageHandler);
   delete iMessageHandler;
   iMessageHandler = NULL;

   killNav2();
/*       TRACE_FUNC(); */
}


// ----------------------------------------------------
// CWayFinderAppUi::HandleCommandL(TInt aCommand)
// ?implementation_description
// ----------------------------------------------------
//
void CWayFinderAppUi::HandleCommandL(TInt aCommand)
{
   TUid messageId;
   messageId.iUid = ENoMessage;
   TBuf8<16> customMessage( _L8("") );

   if (iDoingShutdown) {
      return;
   }

   if (iUrgentShutdown) {
      // Exits the program
      doExit();
      return;
   }
   switch ( aCommand )
   {
   case EAknCmdExit:
   case EAknSoftkeyExit:
      {
         iIniFile->Write();
         /* Check if the user really wants to exit... */
         if(!iForeGround || 
            WFDialog::ShowQueryL( R_WAYFINDER_CONFIRM_EXIT_MSG, iCoeEnv ) ){
            // Exit the program
            doExit();
         }
         break;
      }
   case EEikCmdExit:
   case EWayFinderExitAppl:
      {
         iIniFile->Write();
         doExit();
         break;
      }
   case EAknSoftkeyBack:
   case EWayFinderSoftkeyStart:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( ETrue );
         GotoStartViewL();
         break;
      }
   case EWayFinderCmdGotoHomepage:
      {
         if (iServiceWindowView) {
            iServiceWindowView->GotoHomePageL();
         }
         break;
      }
   case EWayFinderCmdStartPageNews:
      {
         m_startupHandler->StartupEvent(SS_ShowNews);

/*          messageId.iUid = EShowNews; */
/*          SetViewL( KWelcomeViewId, messageId, customMessage ); */
         break;
      }
   case EWayFinderCmdPositionSelect:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         SetViewL( KPositionSelectViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdPositionSelectOrigin:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         messageId.iUid = ESetOrigin;
         SetViewL( KNewDestViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdPositionSelectDestination:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         messageId.iUid = ESetDestination;
         SetViewL( KNewDestViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdPositionSelectCatOrigin:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         messageId.iUid = ESetOriginFromCat;
         SetViewL( KNewDestViewId, messageId, customMessage );
         //iNewDestView->ShowQueryDialog( ESetOriginFromCat );
/*          iNewDestView->ShowCategoriesL( ESetOrigin ); */
         break;
      }
   case EWayFinderCmdPositionSelectCatDestination:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         messageId.iUid = ESetDestinationFromCat;
         SetViewL( KNewDestViewId, messageId, customMessage );
         //iNewDestView->ShowQueryDialog( ESetDestinationFromCat );
/*          iNewDestView->ShowCategoriesL( ESetDestination ); */
         break;
      }
   case EWayFinderCmdRefreshCurList:
      {
         CVicinityView* curVicView = NULL;
         
         if(InfoViewActive()) {
            iInfoView->HandleCommandL(EWayFinderCmdRefreshInfoList);
         } else if(VicinityViewActive(curVicView)) {
            curVicView->HandleCommandL(EWayFinderCmdRefreshVicinityList);
         }
         break;
      }
      case EWayFinderCmdWhereAmI:
      {
         DisplayWhereAmIL();
         break;
      }
      case EWayFinderCmdStartPageNewSearch:
      {
         iNewDestView->ShowQueryDialog( ENewSearch );
         break;
      }
   case EWayFinderCmdSettingNewSearch:
      {
         iNewDestView->ShowQueryDialog( ENoMessage );
         break;
      }
   case EWayFinderCmdNewDest:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         messageId.iUid = ENewSearch;
         SetViewL( KNewDestViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdNewDest2:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         SetViewL( KNewDestViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdMyDest:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         SetViewL( KMyDestViewId, messageId, customMessage );
         break;
      }
   case EWayfinderCmdGuideSendTo:
      {
#ifndef NAV2_CLIENT_SERIES60_V3
         ShowConnectDialog();
#endif
         break;
      }
   case EWayFinderCmdConnectSendCurrentPosition:
      {
#ifndef NAV2_CLIENT_SERIES60_V3
         ShowConnectDialog(NULL,
            CConnectDialog::DefaultTransmitMode,
            CConnectDialog::CurrentPosition,
            CConnectDialog::DefaultMessageFormat);
#endif
         break;
      }
   case EWayFinderCmdConnect:
      {
#ifndef NAV2_CLIENT_SERIES60_V3
         ShowConnectDialog();
#endif
         break;
      }

   case EWayFinderCmdVicinityAll:
      {
         SetViewL( KVicinityViewAllId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdVicinityCrossings:
      {
         SetViewL( KVicinityViewCrossingId, messageId, customMessage );         
         break;
      }
   case EWayFinderCmdVicinityFavourites:
      {
         SetViewL( KVicinityViewFavouriteId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdVicinityPOIs:
      {
         SetViewL( KVicinityViewPOIId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdVicinityFeed:
      {
         iGuideView->CreateContainer();
         SetViewL( KVicinityFeedId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdGuide:
      {
         SetViewL( KGuideViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdItinerary:
      {
         SetViewL( KItineraryViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdDestination:
      {
         SetViewL( KDestinationViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdMap:
      {
         SetViewL( KMapViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdInfoTab:
      {
         
         SetViewL( KInfoViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdInfo:
      {
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         SetViewL( KInfoViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdNewDestInfo:
      {
         messageId.iUid = EFromNewDest;
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         SetViewL( KInfoViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdConnectInfo:
      {
         messageId.iUid = EFromConnect;
         GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
         SetViewL( KInfoViewId, messageId, customMessage );
         break;
      }
   case EWayFinderCmdSettings:
      {
         SwitchToSettingsL();
         break;
      }
   case EWayFinderCmdAbout:
      {
         ShowAboutL();
         break;
      }
   case EWayFinderCmdFleetInfo:
      {
         ShowFleetInfoL();
         break;
      }
   case EWayFinderCmdHelp:
      {
         ShowHelpL();
         break;
      }
   case EWayFinderCmdGPSConnect:
   case EWayFinderCmdMapGPSConnect:
      {
         //XXX connects to Nav2 via TCP socket,
         //for development reasons.
#ifdef GUI_ONLY
         if(iMessageHandler){
            iMessageHandler->ConnectL(GUIMACHINE, GUIPORT );
         }
#else
         ConnectToGpsL();
#endif
         break;
      }
   case EWayFinderCmdGPSDisConnect:
      {
#ifdef GUI_ONLY
         //XXX disconnects from Nav2,
         //for development reasons.
         if(iMessageHandler){
            iMessageHandler->Disconnect();
         }
#else
         DisconnectFromGPSL();
#endif
         break;
      }
   case EWayFinderCmdDebugAction:
      {
         DebugAction();
         break;
      }
   case EWayFinderCmdService:
      {
         GotoServiceViewL(KWayfinderServiceWindowBaseURL);
         break;
      }
   case EWayFinderCmdServicesBuyExtensionWapLink:
      {
         LaunchBuyExtensionWapLinkL();
         break;
      }
   case EWayFinderCmdServicesSpreadWayfinder:
      {
         StartSpreadWayfinderDialogsL();
         break;
      }
   case EWayFinderCmdClear:
   case EWayFinderCmdStopNavigation:
      {
         if (WFDialog::ShowQueryL( R_WAYFINDER_CONFIRM_CLEAR_ROUTE_MSG, iCoeEnv ) ) {
            /* Send invalidate route with route id zero. */
            GenericGuiMess stop(GuiProtEnums::INVALIDATE_ROUTE,
                  (uint32)0, (uint32)0);
            SendMessageL(&stop);
         }
         break;
      }
   case EWayFinderCmdStartNavigation:
   case EWayFinderCmdMapReroute:
   case EWayFinderCmdGuideReroute:
   case EWayFinderCmdItineraryReroute:
   case EWayFinderCmdDestinationReroute:
   case EWayFinderCmdReroute:
      {
         GenericGuiMess reroute(GuiProtEnums::REROUTE);
         SendMessageL(&reroute);
/*          SetWait( ETrue, R_WAYFINDER_ROUTING_MSG); */
         break;
      }
   case EWayFinderCmdMapVoice:
   case EWayFinderCmdGuideVoice:
   case EWayFinderCmdItineraryVoice:
   case EWayFinderCmdDestinationVoice:
      {
         GenericGuiMess playsound(GuiProtEnums::REQUEST_CROSSING_SOUND,
                                  uint16(iGuideView->GetCurrentTurn()) );
         SendMessageL(&playsound);
         break;
      }
   case EWayFinderCmdServiceWindowCall:
      iPhoneCallFromWf = EWayFinderCmdService;
      break;
   case EWayFinderCmdNewDestCall:
      iPhoneCallFromWf = EWayFinderCmdNewDest2;
/*       iNewDestCall = ETrue; */
      break;
   case EWayFinderCmdMapCall:
      iPhoneCallFromWf = EWayFinderCmdMap;
/*       iMapCall = ETrue; */
      break;
   case EWayFinderCmdEmptyMapCache:
      iMapView->ClearMapCache();
      break;
   case EWayFinderCmdInfoShow:
      RequestMap( MapEnums::UserPosition, MAX_INT32, MAX_INT32 );
      break;
   case EWayFinderCmdSave:
      iNTCommandHandler->SendCommandL(isab::command_save);
      break;
   case EWayFinderCmdLoad:
      iNTCommandHandler->SendCommandL(isab::command_load);
      break;
   case EWayFinderCmdDelete:
      iNTCommandHandler->SendCommandL(isab::command_delete);
      break;
   case EWayFinderCmdSimulateItem:
      iNTCommandHandler->SendCommandL(isab::command_load_and_simulate);
      break;
   case EWayFinderCmdSimulateStart:
      iNTCommandHandler->SendCommandL(isab::command_simulate_start);
      break;
   case EWayFinderCmdSimulateStop:
      iNTCommandHandler->SendCommandL(isab::command_simulate_stop);
      break;
   case EWayFinderCmdSimulateResume:
      iNTCommandHandler->SendCommandL(isab::command_simulate_resume);
      break;
   case EWayFinderCmdSimulatePause:
      iNTCommandHandler->SendCommandL(isab::command_simulate_pause);
      break;
   case EWayFinderCmdSimulateIncSpeed:
      iNTCommandHandler->SendCommandL(isab::command_simulate_inc_speed);
      break;
   case EWayFinderCmdSimulateDecSpeed:
      iNTCommandHandler->SendCommandL(isab::command_simulate_dec_speed);
      break;
   case EWayFinderCmdSimulateRepeatOn:
      iNTCommandHandler->SendCommandL(isab::command_simulate_rep_on);
      break;
   case EWayFinderCmdSimulateRepeatOff:
      iNTCommandHandler->SendCommandL(isab::command_simulate_rep_off);
      break;
   case EWayFinderCmdReportTrafficJam:
      SendSMSReportEvent(0xf001);
      break;
   case EWayFinderCmdReportTrafficInfoCenter:
      SendSMSReportEvent(0xf002);
      break;
   case EWayFinderCmdReportBreakdown1:
      SendSMSReportEvent(0xf003);
      break;
   case EWayFinderCmdReportSpeedCam:
      SendSMSReportEvent(0xf005);
      break;
   case EWayFinderCmdReportProblem:
      SendSMSReportEvent(0xf006);
      break;
   case EWayFinderCmdReportDangerousPlace:
      SendSMSReportEvent(0xf007);
      break;
   case EWayFinderCmdReportWeatherWarning:
      SendSMSReportEvent(0xf008);
      break;
   case EWayFinderCmdReportMapProblem:
      SendSMSReportEvent(0xf009);
      break;
   case EWayFinderCmdReportPosition:
      SendSMSReportEvent(0xf00a);
      break;
   case EWayFinderCmdReportRouteProblem:
      SendSMSReportEvent(0xf00b);
      break;
   case EWayFinderCmdReportBreakdown2:
      SendSMSReportEvent(0xf00c);
      break;
   case EWayfinderCmdAsterisk:
   case EWayFinderCmdReportEvent:
      ShowReportEventMenu();
      break;
   case EWayFinderCmdFindSms:
#ifndef NAV2_CLIENT_SERIES60_V3
      iCommunicateHelper->UpdateSMSListL();
      if(iCommunicateHelper->SmsCount()){
         typedef TCallBackEvent<CWayFinderAppUi, TWayfinderEvent> cb_t;
         typedef CCallBackListDialog<cb_t> cbd_t;
         iSelectedSms = -1;
         const MDesCArray* descriptions = iCommunicateHelper->SmsDescriptionArray();
         const MDesCArray* senders = iCommunicateHelper->SmsSenderArray();
         CDesCArray* dblArray = 
            WFArrayUtil::PasteArrayLC(senders, _L("\t"), descriptions);
         cbd_t::RunDlgLD(cb_t(this, EWayfinderEventInboxSmsSelected),
                         iSelectedSms, //XXX
                         R_WF_INBOX_TITLE,
                         dblArray,
                         R_WAYFINDER_CALLBACK_DOUBLE_LIST_QUERY, 
                         EFalse); //dialog owns array
         CleanupStack::Pop(dblArray);
      } else {
         WFDialog::ShowConfirmationL( R_WF_SMS_INBOX_EMPTY_MSG );
      }
#else
      if( iSmsHandler ){
         if( !iSmsHandler->CheckSmsInboxL() ){
            WFDialog::ShowInformationL( R_WAYFINDER_MESSAGE_NO_SMS );
         }
         else{
            TInt selection(0);
            MDesCArray* destArray = iSmsHandler->GetDestinationArray();
            MDesCArray* senderArray = iSmsHandler->GetSenderArray();
            CDesCArrayFlat* array = new (ELeave) CDesCArrayFlat(4);
            CleanupStack::PushL( array );
            for( TInt i=0; i < destArray->MdcaCount(); i++ ){
               HBufC* mess = HBufC::NewLC( destArray->MdcaPoint(i).Length() +
                                           senderArray->MdcaPoint(i).Length() + 2 );
               mess->Des().Copy( KTab );
               mess->Des().Append( senderArray->MdcaPoint(i) );
               mess->Des().Append( KTab );
               mess->Des().Append( destArray->MdcaPoint(i) );
               array->AppendL( *mess );
               CleanupStack::PopAndDestroy( mess );
            }
            CleanupStack::Pop( array );
            CAknSelectionListDialog*
               dialog = CAknSelectionListDialog::NewL( 
                        selection, 
                        array, 
                        R_NEWDEST_DETAILS_MENUBAR );
            //dialog->SetupFind( CAknSelectionListDialog::ENoFind );
            if( dialog->ExecuteLD( R_NEWDEST_DETAILS_DIALOG ) ){
               iSmsHandler->SetSelectedWfSmsL( selection );
            }
            delete array;
         }
      }
#endif
      break;
   default:
      break;
   }
}

void CWayFinderAppUi::Panic(CMessageHandler* from)
{
   //XXX
   //panic procedures postion
   //XXX
   if(iBtHandler &&
      (!iBtHandler->IsConnected())){
      //      iBtHandler->ReleaseL();
      LOGDEL(iBtHandler);
      MPositionInterface::CloseAndDeleteL(iBtHandler);
   }
   if(iMessageHandler && iMessageHandler == from){
      iMessageHandler->Release();
      LOGDEL(iMessageHandler);
      delete iMessageHandler;
      iMessageHandler = NULL;
   }
   if(!iBtHandler && !iMessageHandler){
      killNav2();
      WFDialog::ShowScrollingDialogL( iCoeEnv, R_WAYFINDER_MEMORY_LOW_MSG,
                            R_WAYFINDER_MEMORY_LOW_EXIT_MSG, ETrue);
   }
   iUrgentShutdown = ETrue;
   /* Switch to main view. */
   TUid messageId;
   messageId.iUid = EShutdown;
   TBuf8<16> customMessage( _L8("") );

   SetViewL(KWelcomeViewId, messageId, customMessage);
}

#if 0
TInt
CWayFinderAppUi::ShowPopupListL(R_WAYFINDER_SOFTKEYS_OK_CANCEL, 
                                const CDesCArrayFlat& aDescArray,
                                TBool& aOkChosen,
                                TInt selection,
                                TBool aShowTitle,
                                TDesC* aTitle)
{
   return PopUpList::ShowPopupListL(R_WAYFINDER_SOFTKEYS_OK_CANCEL, aDescArrayaDescArray, aOkChosen, selection, aShowTitle, aTitle);
}
#endif

CAknNavigationControlContainer *
CWayFinderAppUi::getNavigationControlContainer() const
{
   return iNaviPane;
}

void
CWayFinderAppUi::TurnBackLightOnL()
{
   if ( TileMapUtil::currentTimeMillis() -
         iLastTimeBacklightTrigger  < 10*1000 ) {
      // Not long enough since last shown error message in the map view.
      return;
   }
   iLastTimeBacklightTrigger = TileMapUtil::currentTimeMillis();
   User::ResetInactivityTime();
}



void
CWayFinderAppUi::ImeiNumberSet(const TDesC& aImeiNumber)
{
   if (iImeiNbr) {
      delete iImeiNbr;
      iImeiNbr = NULL;
   }
   iImeiNbr = WFTextUtil::newTDesDupL(aImeiNumber);
   iImeiReceived = ETrue;
   SecondStageStartup();
}

#if defined NAV2_CLIENT_SERIES60_V3
void CWayFinderAppUi::ChangeMenuIcon(int bmpIndex, int maskIndex)
{
	CEikStatusPane* statusPane = iEikonEnv->AppUiFactory()->StatusPane();
	CAknContextPane* contextPane = (CAknContextPane *)statusPane->
           ControlL(TUid::Uid(EEikStatusPaneUidContext));

	if(!bmpIndex && !maskIndex)
	{
           //this works fine, the default image is set back well
           contextPane->SetPictureToDefaultL();
           return;
	}

// 	TFileName pathToMenuIconsMif;
// 	//this builds the full path to the mif file, it works fine
// 	Globals::GetResImgFileName(KImgMenuIconsMif, pathToMenuIconsMif);

	CFbsBitmap *MyBitmap;
	CFbsBitmap *MyMask;
	TSize iconSize(15, 15);

	AknIconUtils::CreateIconL(MyBitmap, MyMask, 
                                  iPathManager->GetMbmName(), 
                                  bmpIndex, maskIndex);	
	AknIconUtils::SetSize(MyBitmap, iconSize);

	//this does not work
	contextPane->SetPicture(MyBitmap, MyMask);
}

#endif

void 
CWayFinderAppUi::ImageOperationCompletedL(TInt /*aError*/)
{
   iWelcomeView->SetImage(iImageHandler);
   FetchImei();
}

void 
CWayFinderAppUi::SmsSent(TInt /*aStatus*/, 
                         TInt /*aCount*/, 
                         TInt /*aNbrSent*/)
{
}


void CWayFinderAppUi::SmsReceived( CSmsParser* aSmsParser, TInt32 aMsvId )
{
   switch( aSmsParser->GetSmsType() )
   {
   case CSmsParser::EWFSMSFavorite:
   {
      isab::Favorite* favorite = aSmsParser->GetFavoriteD();
      TPoint destCord = aSmsParser->GetDestinationCoordinate();
      HandleSmsL( destCord.iY, destCord.iX,
                  aSmsParser->GetDestinationDescription(),
                  aMsvId, favorite );
      break;
   }
   case CSmsParser::EWFSMSRoute:
   {
      TPoint origCord = aSmsParser->GetOriginCoordinate();
      TPoint destCord = aSmsParser->GetDestinationCoordinate();
      HandleSmsL( origCord.iY, origCord.iX,
                  destCord.iY, destCord.iX,
                  aSmsParser->GetDestinationDescription(),
                  aMsvId );
      break;
   }
   case CSmsParser::EWFSMSDestination:
   {
      TPoint destCord = aSmsParser->GetDestinationCoordinate();
      HandleSmsL( destCord.iY, destCord.iX,
                  aSmsParser->GetDestinationDescription(),
                  aMsvId );
      break;
   }
   case CSmsParser::EWFSMSParseError:
   case CSmsParser::EWFSMSNoMessage:
   case CSmsParser::EWFSMSUnknown:
   case CSmsParser::EWFSMSNotWFMessage:
   case CSmsParser::EWFSMSError:
   {
      break;
   }
   }
}


void
CWayFinderAppUi::HandleResourceChangeL(TInt aType)
{
   CAknAppUi::HandleResourceChangeL(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      if (iImageHandler) {
         iImageHandler->HandleLayoutSwitch(aType);
      }
      if (iContextPaneAnimator) {
         // We need to recalculate the sizes of the gps images, 
         // they wont fit in the bluebar otherwise. 
#ifdef NAV2_CLIENT_SERIES60_V3
         iContextPaneAnimator->RecalcLargeGpsFrameSize();
         CAnimatorFrame* temp = new CAnimatorFrame();
         temp->CopyFrameL(iContextPaneAnimator->GetLargeGpsFrameL());
         if (iMapView) { 
            iMapView->SetGpsStatusImage(temp->GetBitmap(), temp->GetMask());
         }
#endif
      }
   }
}

void CWayFinderAppUi::LineStatusChanged( const TLineState aLineState )
{
   switch( aLineState )
   {
   case MLineStateListener::EDialing:
   case MLineStateListener::ERinging:
   case MLineStateListener::EConnecting:
   case MLineStateListener::EConnected:
      HandlePhoneCallL( ETrue );
      break;
   case MLineStateListener::EDisconnected:
   case MLineStateListener::EObserverError:
   case MLineStateListener::EBusy:
      HandlePhoneCallL( EFalse );
      break;
   }
}

void CWayFinderAppUi::CallError( const TCallError aCallError )
{
   TInt resc;
   switch( aCallError )
   {
   case MLineStateListener::EErrorDialing:
      resc = R_WAYFINDER_DIALERROR_MSG;
      break;
   case MLineStateListener::EInvalidPhoneNumber:
      resc = R_WAYFINDER_INVALIDNUMBER_MSG;
      break;
   case MLineStateListener::ELineBusy:
      resc = R_WAYFINDER_DIALERROR_MSG;
      break;
   default:
      return;
   }
   WFDialog::ShowErrorL( resc, iCoeEnv );
}

#ifdef ASSERT_AND_SHOWMESSAGE
# if !defined(TOMAS)
#  error Maybe a mistake in debuggingdefines.h?, or your name is missing from the line above. ASSERT_AND_SHOWMESSAGE should not be defined for any of the release builds.
# endif
#endif


void CWayFinderAppUi::RouteToCoordinateL( TInt32 aLat, TInt32 aLon,
                                          const TDesC &aDescription )
{
   char* description = WFTextUtil::newTDesDupL(aDescription);
   CleanupStack::PushL( description );
   // New Gui messages
   RouteToPositionMess* message = new (ELeave) RouteToPositionMess( 
                                                       description,
                                                       aLat, aLon );
   CleanupStack::PushL( message );
   SendMessageL( *message );
   message->deleteMembers();
   CleanupStack::PopAndDestroy( message );
   CleanupStack::Pop( description );
   delete[] description;
}


void CWayFinderAppUi::RouteToSMSL( TInt32 originlat,
                                  TInt32 originlon,
                                  TInt32 destinationlat,
                                  TInt32 destinationlon,
                                  const TDesC& destNameStr)
{
   char* description = WFTextUtil::newTDesDupL(destNameStr);
   CleanupStack::PushL( description );
   // New Gui messages.
   RouteMess* message = new (ELeave) RouteMess(
                        GuiProtEnums::PositionTypePosition, NULL,
                        originlat, originlon,
                        GuiProtEnums::PositionTypePosition, NULL,
                        destinationlat,destinationlon, description );
   CleanupStack::PushL( message );
   SendMessageL( *message );
   message->deleteMembers();
   CleanupStack::PopAndDestroy( message );
   CleanupStack::Pop( description );
   delete[] description;
}


void CWayFinderAppUi::HandleSmsL( TInt32 aLat, TInt32 aLon,
                                  const TDesC& aDescription,
                                  TInt32 aMsvId,
                                  Favorite* aFavorite )
{
   if(iAutoRouteSmsDestination){
      if( IsFullVersion() ){
         RouteToCoordinateL( aLat, aLon, aDescription );
      }
      else{
         iSelectView->SetDestination( isab::GuiProtEnums::PositionTypePosition,
                                      aDescription, NULL, aLat, aLon );
      }
   }
   if(iSaveSmsDestination==GuiProtEnums::yes){
      if(iMyDestView){
         if( aFavorite ){
            iMyDestView->AddFavoriteD( aFavorite );
         }
         else{
            iMyDestView->AddFavorite( aLat, aLon, aDescription );
         }
      }
   }
   else if(iSaveSmsDestination==GuiProtEnums::no){
   }
   else if(iSaveSmsDestination==GuiProtEnums::ask){
      if( WFDialog::ShowQueryL( R_WAYFINDER_SAVESMSDESTINATION_MSG, iCoeEnv ) ){
         if( iMyDestView ){
            if( aFavorite ){
               iMyDestView->AddFavoriteD( aFavorite );
            }
            else{
               iMyDestView->AddFavorite( aLat, aLon, aDescription );
            }
         }
      }
   }
   if(iKeepSmsDestination==GuiProtEnums::yes)
   {
   }
   else if(iKeepSmsDestination==GuiProtEnums::no){
#ifdef NAV2_CLIENT_SERIES60_V3
      iSmsHandler->DeleteSmsL(aMsvId);
#endif
   }
   else if(iKeepSmsDestination==GuiProtEnums::ask)
   {
      if( !WFDialog::ShowQueryL( R_WAYFINDER_KEEPSMSDESTINATION_MSG, iCoeEnv ) ){
#ifdef NAV2_CLIENT_SERIES60_V3
         iSmsHandler->DeleteSmsL(aMsvId);
#endif
      }
   }
}


void CWayFinderAppUi::HandleSmsL( TInt32 aOriginLat,
                                  TInt32 aOriginLon,
                                  TInt32 aDestinationLat,
                                  TInt32 aDestinationLon,
                                  const TDesC& aDestinationDescription,
                                  TInt32 aMsvId )
{
   if(iAutoRouteSmsDestination){
      RouteToSMSL( aOriginLat, aOriginLon, aDestinationLat,
                   aDestinationLon, aDestinationDescription );

   }
   if(iSaveSmsDestination==GuiProtEnums::yes){
      if( iMyDestView ){
         iMyDestView->AddFavorite( aDestinationLat,
                                   aDestinationLon,
                                   aDestinationDescription );
      }
   }
   else if(iSaveSmsDestination==GuiProtEnums::no){
   }
   else if(iSaveSmsDestination==GuiProtEnums::ask){
      if( WFDialog::ShowQueryL( R_WAYFINDER_SAVESMSDESTINATION_MSG, iCoeEnv ) ){
         if(iMyDestView){
            iMyDestView->AddFavorite( aDestinationLat,
                                      aDestinationLon,
                                      aDestinationDescription );
         }
      }
   }
   if(iKeepSmsDestination==GuiProtEnums::yes){
   }
   else if(iKeepSmsDestination==GuiProtEnums::no){
#ifdef NAV2_CLIENT_SERIES60_V3
      iSmsHandler->DeleteSmsL(aMsvId);
#endif
   }
   else if(iKeepSmsDestination==GuiProtEnums::ask){
      if( !WFDialog::ShowQueryL( R_WAYFINDER_KEEPSMSDESTINATION_MSG, iCoeEnv ) ){
#ifdef NAV2_CLIENT_SERIES60_V3
         iSmsHandler->DeleteSmsL(aMsvId);
#endif
      }
   }
}

void
CWayFinderAppUi::ProgressDlgAborted()
{
   m_startupHandler->StartupEvent(SS_IapSearchNotOk);
}

void CWayFinderAppUi::AddSfdCacheFiles(MapLib * mapLib) {
	if(mapLib == NULL)
		return;

   _LIT(KWfdName, "*.wfd");
   class RFs fs = CEikonEnv::Static()->FsSession();
   TFindFile finder(fs);
   CDir * dir;
   TInt res = finder.FindWildByDir(KWfdName, iPathManager->GetMapCacheBasePath(), dir);
   // Add all sfd-files found.
   while (res == KErrNone) {
      for (TInt i = 0; i < dir->Count(); i++) {
         TParse parser;
         parser.Set((*dir)[i].iName, &finder.File(), NULL);
         char * fullPath = WFTextUtil::TDesCToUtf8LC(parser.FullName());
         mapLib->addSingleFileCache(fullPath, NULL);
         CleanupStack::PopAndDestroy(fullPath);
      }
      delete dir;
      dir = NULL;
      // Continue the search on the next drive.
      res = finder.FindWild(dir);
   }
}

TBool CWayFinderAppUi::MapViewActive() {
   return iTabGroup->ActiveTabId() == KMapViewId.iUid;
}

TBool CWayFinderAppUi::InfoViewActive() {
   return iTabGroup->ActiveTabId() == KInfoViewId.iUid;
}

TBool CWayFinderAppUi::VicinityViewActive(CVicinityView*& aActiveView)
{
   if( iTabGroup->ActiveTabId() == KVicinityViewAllId.iUid ) {
      aActiveView = iVicinityViewAll;
      return ETrue;
   } else if( iTabGroup->ActiveTabId() == KVicinityViewCrossingId.iUid ) {
      aActiveView = iVicinityViewCrossing;
      return ETrue;
   } else if( iTabGroup->ActiveTabId() == KVicinityViewFavouriteId.iUid ) {
      aActiveView = iVicinityViewFavourite;
      return ETrue;
   } else if( iTabGroup->ActiveTabId() == KVicinityViewPOIId.iUid ) {
      aActiveView = iVicinityViewPOI;
      return ETrue;
   }
   return EFalse;
}

int CWayFinderAppUi::getMemCacheSize() const {
#ifdef NAV2_CLIENT_SERIES60_V1
   int maxSize = 64 * 1024;
#else
   int maxSize = 128 * 1024;
#endif
   int memFree = 0;
#ifndef NAV2_CLIENT_SERIES60_V3
   HAL::Get(HALData::EMemoryRAMFree, memFree);
#else
   // We cannot get any usable measure of how much memory is available.
   // Let's assume we can afford the full memory cache.
   memFree = maxSize * 2;
#endif

//   TBuf<64> str;
//   str.Format(_L("Memory free : %i"), memFree );
//   CEikonEnv::Static()->InfoMsg( str );
//   SHOWMSGWIN( str );
   return MIN(maxSize, memFree / 2);
}

isab::NavServerComEnums::VehicleType 
CWayFinderAppUi::GetTransportationMode()
{
   return iSettingsData->iVehicle;
}


TBool 
CWayFinderAppUi::isCriticalError(TInt32 errorNum)
{
   return 
      errorNum == Nav2Error::PARAM_NO_SPACE_ON_DEVICE ||
      errorNum == Nav2Error::PARAM_NO_SPACE_ON_DEVICE_2;
}

FeedSettings::OutputFormat
CWayFinderAppUi::getFeedOutputFormat()
{
   if(iSettingsData)
      return iSettingsData->iFeedOutputFormat;
   else
      return FeedSettings::FEED_CONCISE;
    
}

MC2Direction::RepresentationType
CWayFinderAppUi::getDirectionType()
{
   if(iSettingsData)
      return iSettingsData->iDirectionType;
   else
      return MC2Direction::ClockBased;
}

MapResourceFactory* CWayFinderAppUi::GetMapResourceFactory()
{
   return iMrFactory;
}

void CWayFinderAppUi::ActivateLocalViewL(TUid aViewId)
{
   if(aViewId == KVicinityFeedId) {
      iGuideView->CreateContainer();
   }
   
   CAknViewAppUi::ActivateLocalViewL(aViewId);
}

void CWayFinderAppUi::ActivateLocalViewL(TUid aViewId,
                                         TUid aCustomMessageId,
                                         const TDesC8 &aCustomMessage)
{
   if(aViewId == KVicinityFeedId) {
      iGuideView->CreateContainer();
   }
   
   CAknViewAppUi::ActivateLocalViewL(aViewId,
                                           aCustomMessageId,
                                           aCustomMessage);
}

void CWayFinderAppUi::SetupHotkeys()
{
   iHotkeyMap[HotKeys::START_VIEW] = EWayFinderSoftkeyStart;
   iHotkeyMap[HotKeys::INFO_VIEW] = EWayFinderCmdInfoTab;
   iHotkeyMap[HotKeys::VICINITY_ALL] = EWayFinderCmdVicinityAll;
   iHotkeyMap[HotKeys::VICINITY_CROSSINGS] = EWayFinderCmdVicinityCrossings;
   iHotkeyMap[HotKeys::VICINITY_FAVOURITES] = EWayFinderCmdVicinityFavourites;
   iHotkeyMap[HotKeys::VICINITY_POIS] = EWayFinderCmdVicinityPOIs;
   iHotkeyMap[HotKeys::VICINITY_FEED] = EWayFinderCmdVicinityFeed;
   iHotkeyMap[HotKeys::WHERE_AM_I] = EWayFinderCmdWhereAmI;
   iHotkeyMap[HotKeys::REFRESH_CURRENT_VIEW] = EWayFinderCmdRefreshCurList;
}

TKeyResponse
CWayFinderAppUi::HandleHotKeyEventL(const struct TKeyEvent& aKeyEvent,
                                    enum TEventCode aType)
{
   HotKeyMap::iterator hkm = iHotkeyMap.find(aKeyEvent.iScanCode);
   
   if(hkm==iHotkeyMap.end()) {
      return EKeyWasNotConsumed;
   }

   HandleCommandL(hkm->second);
   
   return EKeyWasConsumed;
}

// End of File

void CWayFinderAppUi::DisplayWhereAmIL()
{
   _LIT(KBreak, "\n");
   _LIT(KSpace, " ");

   HBufC* yourDirection =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_HEADING);

   iMrFactory->initIfNecessary();
   
   MapLib * mapLib = iMrFactory->getMapLib();

   MC2SimpleString wai = mapLib->getWhereAmI(" ");

   HBufC* tmp = WFTextUtil::AllocLC(wai.c_str());
   

   MC2Direction curDirection(iHeading);

   HBufC* concatenated = NULL;

   int allocSize = 512;

   concatenated = HBufC::NewLC(allocSize);
   HBufC* header = CEikonEnv::Static()->AllocReadResourceLC(R_WF_WAI);
   
   TPtr connector = concatenated->Des();
   KBreak().Size();
   connector.Append(*tmp);
   connector.Append(KBreak);
   
   if(IsHeadingUsable()) {
      // DO stuff here.
      connector.Append(*yourDirection);
      connector.Append(KSpace);
      
      DirectionHelper::AppendDirection( connector,
                                        curDirection,
                                        0, // Not used.
                                        true, // Usable here.
                                        MC2Direction::DirectionBased );
   }

#if DEBUG_DIRECTIONS
   // Double check that angles and clock is working as it should.
   char* dbgStr = new char[10];
   sprintf(dbgStr, "%d", iHeading);
   HBufC* dbg = WFTextUtil::AllocL( dbgStr );
   connector.Append(KSpace);
   connector.Append(*dbg);
   delete[] dbgStr;
   delete dbg;

   HBufC* tmpHour = FormatDirection::ParseLC(curDirection.Hour(0));
   connector.Append(KSpace);
   connector.Append(*tmpHour);
   CleanupStack::PopAndDestroy(tmpHour);

#endif   

   WFDialog::ShowScrollingDialogL( *header, *concatenated, EFalse );

   CleanupStack::PopAndDestroy(header);
   CleanupStack::PopAndDestroy(concatenated);
   CleanupStack::PopAndDestroy(tmp);
   CleanupStack::PopAndDestroy(yourDirection);
}

TBool CWayFinderAppUi::IsHeadingUsable()
{
   if(!IsGpsConnected())
      return EFalse;
   
   // If iSpeedMps < 0.0, the speed quality data is not valid.
   // If iSpeedMps < 2.0 (kph)/3.6, the heading is not of much use.


   TReal t = TReal(iUnitSpeed);

   TReal walkingSpeedCutoff = TReal(iSettingsData->iWalkingSpeedCutOff);

   // Note that the unit speed is in the same unit as the walking speed in the
   // settings, so we can compare directly.
   return t > walkingSpeedCutoff;
}
