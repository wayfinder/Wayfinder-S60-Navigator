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
#include <akntitle.h>
#include <eikspane.h>
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

#include <aknkeylock.h> // For RAknKeyLock

#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavilabel.h>
#include <aknnavide.h>
#include <aknlists.h>
#include <mtclreg.h>

#include <bautils.h>
#include <bttypes.h>
#include <utf.h>

#include <deque>

#ifndef NAV2_CLIENT_SERIES60_V3
#include <hal.h>
#endif
#include <e32math.h> 

/* For RApaLsSession */
#include <apgcli.h>

#include <remconcoreapitarget.h>
#include <remconinterfaceselector.h>

#include "AnimatorFrame.h"
#include "ContextPaneAnimator.h"

#include "nav2util.h"
#include "arch.h"
#include "WayFinderConstants.h" 
#include "WayFinderApp.h"

#include "Buffer.h"

#include "GuiProt/ServerEnums.h"
#include "GuiProt/ServerEnumHelper.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/SearchArea.h"
#include "GuiProt/SearchRegion.h"
#include "GuiProt/SearchItem.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiProtMapMess.h"
#include "GuiProt/SearchPrintingPolicyFactory.h"
#include "GuiProt/VectorMapCoordinates.h"
#include "GuiProt/GuiParameterEnums.h"

#include "TopRegion.h"
#include "TDesTrackPIN.h"

#include "IAPSearchGui_old.h"
#include "PositionSelectView.h"
#include "MainMenuListView.h"
#include "WelcomeView.h"
#include "MyDestView.h"
#include "EditFavoriteView.h"
#include "ContactsView.h"
#include "GuideView.h"
#include "MapView.h"
#include "InfoView.h"
#include "ItineraryView.h"
#include "MessageHandler.h"
#include "BufferArray.h"
#include "DataHolder.h"
#include "NewSlaveAudio.h"
#include "OldSlaveAudio.h"
#include "DummySlaveAudio.h"
#include "TimeOutTimer.h"
#include "SettingsView.h"
#include "SettingsData.h"
#include "MyAccountView.h"
#include "CallObserver.h"
#include "GpsSocket.h"

#include "AudioCtrlScript.h"

#include "S60StartupHandler.h"
#include "CallBackDialog.h"

/* #include "Item.h" */

#include "buildid.h"
#include "WayFinderSettings.h"
#include "CleanupSupport.h"
#include "WFSymbianUtil.h"
#include "WFTextUtil.h"
#include "WFArrayUtil.h"
#include "WFS60Util.h"
#include "LaunchWapBrowserUtil.h"

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

#include "NokiaLbs.h"
#include "Dialogs.h"
#include "IAPSearcher_old.h"
#include "PopUpList.h"
#include "ActivationSms.h"
#include "DebugSms.h"

#include "HelpUrlHandler.h"
#include "WayfinderServices.hlp.hrh"

#include "NetworkInfoHandler.h"
#include "NetworkRegistrationNotifier.h"

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
#include "WFCommonErrorBlocks.h"
#include "IAPDataStore.h"
#include "WFAccountData.h"
#include "WFNewsData.h"
#include "WFGpsData.h"
#include "GuiProt/UserTermsEnums.h"
#include "ProgressDlg.h"
#include "WFDataHolder.h"
#include "WFStartupEventCallbackParam.h"
#include "WFStartupCommonBlocks.h"

#include "StringUtility.h"
#include "ErrorFilter.h"
#include "TimeOutErrorFilter.h"
#include "TDesCHolder.h"
#include "OfflineTester.h"

#include "S60SymbianGeneralParamReceiver.h"
#include "S60WaitDialog.h"
#include "CommunicationWizard.h"
#include "QueryDialog.h"
#include "ExitQueryDlg.h"

#include "CSMainView.h"
#include "CSDetailedResultView.h"
#include "CSCategoryResultView.h"
#include "SearchableListBoxView.h"
#include "CombinedSearchDispatcher.h"
#include "CombinedSearchHistoryItem.h"
#include "NavRequestType.h"
#include "CellIDRequestData.h"
#include "CellPosInformation.h"
#include "NavServerComProtoVer.h"
#include "WFLMSManager.h"
#include "FavoriteSyncHandler.h"
#include "crc32.h"

#define WAYFINDER_RESOURCE_VERSION_FILE "resource_version.short"
#define WAYFINDER_APPLICATION_VERSION_FILE "version.txt"

// How often the device availabel memory is checked
#define WAYFINDER_MEMORY_CHECK_INTERVAL 2

//these literal declarations are probably more complicated than they
//need to be.The MLIT() macro is just an alias for _LIT. This is
//needed for macro strings to be expanded _before_ the _LIT macro. The
//ML macro adds an 'L' before the string it holds. This is necessary
//for proper string concatenation.
MLIT(KAudioFormatPrefix, WAYFINDER_AUDIO_FORMAT_PREFIX);
MLIT(KAudioDirName, WAYFINDER_AUDIO_DIR_NAME);
MLIT(KWayfinderResourceVersion, WAYFINDER_RESOURCE_VERSION_FILE);

_LIT8(KwfType, NAV2_CLIENT_TYPE_STRING);
_LIT8(KwfOpts, NAV2_CLIENT_OPTIONS_STRING);

MLIT(KApplicationVisibleName, APPLICATION_VISIBLE_NAME);
MLIT(KApplicationPathName, APPLICATION_BASE_DIR_NAME);

MLIT(KAllowedLangs, ALLOWED_LANGS);
MLIT(KDebugSmsNumber, SEND_DEBUG_SMS_NUMBER);
MLIT(KShareWFSmsUrl, SHARE_WF_SMS_URL);

_LIT(KWayfinderServiceWindowBaseURL, "");
_LIT(KWayfinderServiceWindowFreeDownloadsURL, "");

static const TUid KWayfinderUid = { WayFinder_uid3 };

static const TInt KNewsBackgroundRed = NEWS_BACKGROUND_RED;
static const TInt KNewsBackgroundGreen = NEWS_BACKGROUND_GREEN;
static const TInt KNewsBackgroundBlue = NEWS_BACKGROUND_BLUE;

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
   iRouteBoxTl( MAX_INT32, MAX_INT32),
   iRouteBoxBr( MAX_INT32, MAX_INT32),
   iLog(aLog),
   m_fsSession(),
   iForeGround(ETrue),
   iMemCheckInterval( WAYFINDER_MEMORY_CHECK_INTERVAL ),
   iHaveDisplayedRoamingWarning(EFalse),
   iNightMode(EFalse),
   iGotParamterConnectToGps(EFalse),
   iShowNoGpsWarn(ETrue),
   iNavigateToGpsWizCallbackObject(NULL),
   iNavigateToGpsWizCallbackFunc(NULL),
   iKeepBacklightOn(EFalse),
   iCellCountryCode(-1),	// Can't be 0 for none
   iLastRouteRequest(NULL),
   iLastRouteRequestMessId(-1),
   iHandlingFailedRouteRequestMessage(EFalse),
   iHasCheckedForUpgrade(EFalse)
 {
   iWfTypeStr = HBufC::NewL(KwfType().Length());
   iWfTypeStr->Des().Copy(KwfType());

   iWfOptsStr = HBufC::NewL(KwfOpts().Length());
   iWfOptsStr->Des().Copy(KwfOpts());

   iIniFile = new IniFile(m_fsSession, iIniFilePath);
   iExitDlgActive = EFalse;
#if defined NAV2_CLIENT_SERIES60_V3
   iImeiReceived = EFalse;
   iImsiReceived = EFalse;
#endif
   iErrorFilter = new ErrorFilter(MIN_TIME_BETWEEN_SHOWN_ERRORS,
                                  ERROR_MAX_REPEAT_TIME);
   const TInt timeBetweenTimeOutErrorsShown = 2*60*1000;
   iTimeOutErrorFilter = new TimeOutErrorFilter(timeBetweenTimeOutErrorsShown,
                                  ERROR_MAX_REPEAT_TIME);
   iOfflineTester = new OfflineTester();
   // Initialize to other than NULL as GetCurrentRouteDestinationName 
   // returns reference to it.
   iCurrentDestinationName = WFTextUtil::AllocL( "" );

   m_viewStack = new S60WFViewStack( this, 
                                     TUid::Uid( EWayFinderInvalidViewId ),
                                     TUid::Uid( EWayFinderInvalidViewId ),
                                     KMainMenuViewId /*KStartPageViewId*/,
                                     KPositionSelectViewId /*RoutePlaner*/);
   // XXX: Add 3d Map Guide view and 3d Map Guide text bar view here,
   //      when available.
   m_viewStack->addGroupView( KMapViewId,         KWayFinderTabGroupId );
   m_viewStack->addGroupView( KGuideViewId,       KWayFinderTabGroupId );

   // Set up this class as a listener for volume up/down events. 
   // System calls MrccatoCommand when user presses volume buttons.
   iRemSelector = CRemConInterfaceSelector::NewL();
   iRemTarget   = CRemConCoreApiTarget::NewL(*iRemSelector, *this);
   iRemSelector->OpenTargetL();

   iPendingRouteData = NULL;
}

void
CWayFinderAppUi::InitPathsL()
{
   TBuf<KBuf32Length> languageCode;

   // Get the correct language code.
   languageCode = UpdateLangCodesL();
   languageCode.LowerCase();

   //const TDesC& audioFormatPrefix = KAudioFormatPrefix;
   TBool useRomBuild = EFalse;
#ifdef ONLY_PHONE_LANG_ROM_BUILD
   useRomBuild = ETrue;
#endif

   iPathManager = 
      CPathFinder::NewL(this, languageCode, KAudioFormatPrefix, KNullDesC,
                        KAudioDirName, R_WAYFINDER_LANGUAGE_LIST_NEW, 
                        useRomBuild);

   // Temporary solution for the ini-file.
   iIniFilePath.Copy(iPathManager->GetWritableDataPath());
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
      fullName.Append( KNewsPictureFileName );
#  define MAX_WIDTH                176
#  define MAX_HEIGHT               188

      iStartupImage->ConstructL(TRect( TPoint(0,0), TSize( MAX_WIDTH, MAX_HEIGHT ) ), this );
      iStartupImage->SetClear( EFalse );
      iStartupImage->SetShow( EFalse );
      iStartupImage->OpenGif( fullName, EFalse );
# else 
      fullName.Append( KNewsSvgFileName );
      TRect rect = WFLayoutUtils::GetFullScreenRect();
      iImageHandler = CImageHandler::NewL(rect,
                                          /*CEikonEnv::Static()->FsSession(),*/
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
      //LOGNEW(iDecoratedTabGroup, CAknNavigationDecorator);
      iTabGroup = (CAknTabGroup*)iDecoratedTabGroup->DecoratedControl();
      iDecoratedTabGroup->MakeScrollButtonVisible( EFalse );
   }

   iNaviVolumeControl =
      iNaviPane->CreateVolumeIndicatorL(R_AVKON_NAVI_PANE_VOLUME_INDICATOR);	
   
   iDecoratedNaviLabel = iNaviPane->CreateNavigationLabelL();
   iNaviPaneLabel = (CAknNaviLabel*)iDecoratedNaviLabel->DecoratedControl();

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
   
   if(!iSearchWaitDlg) {
      iSearchWaitDlg = CWaitDlg::NewL(this);
   }

}

void
CWayFinderAppUi::SetupStartup()
{
   /** Initialize the GuiProt handler. */
   iGuiProtHandler = new (ELeave) GuiProtMessageHandler( this );
   GuiProtMessageErrorHandler* iGuiProtMessageErrorHandler = 
      new GuiProtMessageErrorHandler( this );
   iGuiProtHandler->RegisterReceiver( 
      iGuiProtMessageErrorHandler, isab::GuiProtEnums::REQUEST_FAILED );
   iGuiProtHandler->RegisterReceiver( 
      iGuiProtMessageErrorHandler, isab::GuiProtEnums::MESSAGETYPE_ERROR );

   /* Creates m_startupHandler */
   WayfinderSymbianUiBase::ConstructL();

   /* Create data store. */
   iDataStore = new (ELeave) CWayfinderDataHolder();
   iDataStore->iWFNewsData = new (ELeave) WFNewsData();
   iDataStore->iWFAccountData = new (ELeave) WFAccountData(
#ifdef TRANSFORM_USERNAME_CLIP_STRING
      WFTextUtil::strdupL(TRANSFORM_USERNAME_CLIP_STRING)
#else
      NULL
#endif
   );
   iDataStore->iWFGpsData = new (ELeave) WFGpsData();

   /* Create the iIAPDataStore, holds data on current IAP. */
   iDataStore->iIAPDataStore = new (ELeave) IAPDataStore(-1, -1);

   TBuf<KBuf32Length> symbianLang = UpdateLangCodesL();
   symbianLang.LowerCase();
   char* tmpSymbianCode = WFTextUtil::newTDesDupLC(symbianLang);
   WFStartupCommonBlocks::SetupStartup(m_startupHandler,
                                       this,
                                       iGuiProtHandler,
                                       iDataStore,
                                       iDataStore->iWFNewsData,
                                       iDataStore->iWFAccountData,
                                       iDataStore->iIAPDataStore,
                                       tmpSymbianCode,
                                       WFTextUtil::newTDesDupL(iPathManager->GetLangSyntaxPath()),
#ifdef USE_WF_ID
                                       ETrue,
#else
                                       EFalse,
#endif
#ifdef USE_SILENT_STARTUP
                                       ETrue
#else
                                       EFalse
#endif
                                       );

   CleanupStack::PopAndDestroy(tmpSymbianCode);

   iDataStore->iSettingsData = CSettingsData::NewL(iDataStore);
   LOGNEW(iDataStore->iSettingsData, CSettingsData);

   m_generalParamReceiver = S60SymbianGeneralParamReceiver::NewL(iDataStore,
      m_startupHandler, iGuiProtHandler, this);
}

const TUid CWayFinderAppUi::GetAppUid() {
   return KWayfinderUid;
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
   iDataHolder = new (ELeave) CDataHolder(iLog);
   LOGNEW(iDataHolder, CDataHolder);
   iDataHolder->ConstructL();

   if ( iPathManager->GetLangResourcePath() != KNullDesC ) {
      CreateSlaveAudio();
   }

   ViewNameMapping viewNameMappings[] = {
      {-1, HELP_INDEX_PAGE},
      {EWayFinderMainMenuView, HELP_START_VIEW},
      {EWayFinderMapTab, HELP_MAP_VIEW},
      {EWayFinderNavigationMapTab, HELP_NAVIGATION_MAP_VIEW},
      {EWayFinderGuideTab, HELP_GUIDE_VIEW},
      {EWayFinderItineraryTab, HELP_ITINERARY_VIEW},
      {EWayFinderMyDestTab, HELP_FAVORITES_VIEW},
      /*{EWayFinderNewDestTab, HELP_SEARCH_VIEW},*/
      {EWayFinderCSMainView, HELP_SEARCH_VIEW},
      {EWayFinderCSCategoryResultView, HELP_SEARCH_RESULT_VIEW},
      {EWayFinderCSDetailedResultView, HELP_SEARCH_RESULT_VIEW},
      {EWayFinderSearchableListBoxView, HELP_SEARCH_VIEW},
      {EWayFinderSettingsTab, HELP_SETTINGS_VIEW},
      {EWayFinderContentWindowTab, HELP_SERVICES_VIEW},
      {EWayFinderInfoTab, HELP_INFORMATION_VIEW},
      {EWayFinderDestinationTab, HELP_DESTINATION_VIEW},
      {EWayFinderPositionSelectTab, HELP_POSITION_SELECT_VIEW},
      {EWayFinderConnectTab, HELP_CONTACTS_VIEW},
      {EWayFinderWelcomeTab, HELP_WELCOME_VIEW},
   };
   TInt noOfMappings = sizeof(viewNameMappings) / sizeof(ViewNameMapping);

   iHelpUrlHandler = CHelpUrlHandler::NewL(viewNameMappings, 
                                           noOfMappings, 
                                           R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE);

   iNetworkInfoHandler = CNetworkInfoHandler::NewL(this);
   iNetworkRegistrationNotifier = CNetworkRegistrationNotifier::NewL(this);

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
   iMainMenuView = CMainMenuListView::NewLC(this);
   AddViewL(iMainMenuView);
   CleanupStack::Pop(iMainMenuView);
   
   iCurrentView = iMainMenuView;

   // ==== PositionSelect
   iSelectView = CPositionSelectView::NewLC(this, iLog, *iGuiProtHandler);
   AddViewL(iSelectView);
   CleanupStack::Pop(iSelectView);
   LOGNEW(iSelectView, CPositionSelectView);

   // New Combined search views
   iCSMainView = CCSMainView::NewLC(this);
   AddViewL(iCSMainView);
   CleanupStack::Pop(iCSMainView);

   iSearchableListBoxView = CSearchableListBoxView::NewLC(this, *iCSMainView);
   AddViewL(iSearchableListBoxView);
   CleanupStack::Pop(iSearchableListBoxView);

   iCSCategoryResultView = CCSCategoryResultView::NewLC(this);
   AddViewL(iCSCategoryResultView);
   CleanupStack::Pop(iCSCategoryResultView);

   iCSDetailedResultView = CCSDetailedResultView::NewLC(this);
   AddViewL(iCSDetailedResultView);
   CleanupStack::Pop(iCSDetailedResultView);

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   // Edit favorite view
   iEditFavoriteView = CEditFavoriteView::NewLC(this);
   AddViewL(iEditFavoriteView);
   CleanupStack::Pop(iEditFavoriteView);
#endif

   // ==== MyDest
   AddViewToFrameWorkL( iMyDestView );
   LOGNEW(iMyDestView, CMyDestView);

   // ==== Guide
   AddViewToFrameWorkL( iGuideView );      
   LOGNEW(iGuideView, CGuideView);

   // ==== Itinerary
   AddViewToFrameWorkL( iItineraryView );      
   LOGNEW(iItineraryView, CItineraryView);

   // ==== Map
   AddViewToFrameWorkL( iMapView );      
   LOGNEW(iMapView, CMapView);

   // ==== Info
   AddViewToFrameWorkL( iInfoView );      
   LOGNEW(iInfoView, CInfoView);

   // ==== My Account
   AddViewToFrameWorkL( iMyAccountView );
   LOGNEW(iMyAccountView, CMyAccountView);

   // ==== Settings
   AddViewToFrameWorkL( iSettingsView );
   LOGNEW(iSettingsView, CSettingsView);
   iSettingsView->SetCache(static_cast<CSettingsData *>(iDataStore->iSettingsData));
   iDataStore->iSettingsData->m_applicationPath = iPathManager->GetWayfinderPath().AllocL();
   iDataStore->iSettingsData->m_applicationDllName = iPathManager->GetApplicationDllName().AllocL();
   iDataStore->iSettingsData->m_audioBasePath = iPathManager->GetAudioBasePath().AllocL();
   iDataStore->iSettingsData->m_resourceNoExt = iPathManager->GetResourceNoExt().AllocL();
   iDataStore->iSettingsData->m_applicationNameAsPath = KApplicationPathName().AllocL();
   iDataStore->iSettingsData->m_audioFormatPrefix = KAudioFormatPrefix().AllocL();
   iDataStore->iSettingsData->m_canShowWebUsername = CanShowWebUsername();
   iDataStore->iSettingsData->m_canShowWebPassword = CanShowWebPassword();
   iDataStore->iSettingsData->m_allowGps = EarthAllowGps();
   iDataStore->iSettingsData->m_isIron = IsIronVersion();
   iDataStore->iSettingsData->m_showLangsWithoutVoice = ShowLangsWithoutVoice();
   iDataStore->iSettingsData->m_showLangsWithoutResource = ShowLangsWithoutResource();

   TPtrC allowedLangs = KAllowedLangs();
   if (allowedLangs != KNullDesC) {
      // Create list of allowed langs (mostly for branding).
      ((CSettingsData *)(iDataStore->iSettingsData))->iAllowedLangs = new (ELeave) CDesCArrayFlat(2);
      TLex lexer(allowedLangs);
      while (!lexer.Eos()) {
         GetSettingsData()->iAllowedLangs->AppendL(lexer.NextToken());
      }
   }

   iDataHolder->InitCountries();
/*    iDataHolder->InitCities(); */

#ifndef NAV2_CLIENT_SERIES60_V3
   iCallObserver = CCallObserver::NewL( this );
   iCallObserver->StartL();
#endif

   if( iUrgentShutdown ){
      HandleCommandL( EWayFinderExitAppl );
   }

   ConnectNav2L();
   SetNaviPaneLabelL(0);

   iComWiz = CCommunicationWizard::NewL(*this, *iGuiProtHandler);

    iCallHandler = CSymbian9CallHandler::NewL( this );
    iCallHandler->FindIMEI();
    iSmsHandler = SmsHandlerFactory::CreateL( this );

   _LIT(KImgFileExt, ".mif");
   char* iconPath = WFTextUtil::newTDesDupL(iPathManager->GetCSIconPath());
   char* iconExt = WFTextUtil::newTDesDupL(KImgFileExt());
   iCSDispatcher = new (ELeave) CombinedSearchDispatcher(*iGuiProtHandler, 
                                                         *this,
                                                         iconPath,
                                                         iconExt);
   // cannot be done here since it interferes with startup.
   //iCSDispatcher->requestCategories();
   delete[] iconPath;
   delete[] iconExt;
//    iCSMainView->SetSearchDispatcher(iCSDispatcher);

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
   delete iRoamingDialog;
   iRoamingDialog = NULL;
   delete iDestinationName;
   delete iCurrentDestinationName;
   delete iEventSmsErrTxt;

   delete iEventGenerator;
/*    delete iStartupHandler; */
   delete iNTCommandHandler;

   delete iPathManager;

   delete iSpreadWFDialogs;

   iStartupImage = NULL;
   
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
   delete iDecoratedNaviLabel;
   delete iNaviVolumeControl;
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

   // delete the last position if there was one
   if (iCellPosition) {
      delete iCellPosition;
      iCellPosition = NULL;
   }


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
   delete iNetworkRegistrationNotifier;

   delete iLogMaster;
   LOGDEL(this);
   delete iImeiNbr;
   delete iImsiNbr;
   delete[] iImeiCrcHex;
   delete iImsiFetcher;
   
#if defined NAV2_CLIENT_SERIES60_V3
   delete iSmsHandler;
   delete iCallHandler;
#endif
   delete iImageHandler;

   delete iLangCodeInIso;
   delete iLangCodeInSymbian;

   delete iErrorFilter;
   delete iTimeOutErrorFilter;
   delete iOfflineTester;

   delete m_generalParamReceiver;
   delete iGuiProtHandler;
   iGuiProtHandler = NULL;

   delete iGuiProtMessageErrorHandler;
   delete iWaitDlg;
   delete iRouteWaitDlg;
   delete iSearchWaitDlg;
   delete iProgressDlg;

   iCoeEnv->DeleteResourceFile(iLanguageListRsc);

   if (iNaviVolumeTimer) {
      iNaviVolumeTimer->Cancel();
      delete iNaviVolumeTimer;
   }
   delete iFavoriteSyncHandler;
   delete iWFLMSManager;

   delete iLmsId;
   delete[] iLmsIdChar;
   delete m_viewStack;
   delete iPendingRouteData;
   iPendingRouteData = NULL;

   if(iLastRouteRequest) {
      LOGDEL(iLastRouteRequest);
      delete iLastRouteRequest;
      iLastRouteRequest = NULL;
   }
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

   // Create the file to make sure we can write to it in WFSymbianUtil.
   RFs fs;
   fs.Connect();
   RFile file;
   file.Create(fs, filename, EFileWrite);
   file.Close();
   TInt res = WFSymbianUtil::writeBytesToFile(fs, filename, data, 2, iLog);
   fs.Close();

   if (res < 0) {
      /* Failed to write language-file. */
      /* XXX What to do? */
/*          filename.Copy(_L("failed write: ")); */
/*          filename.AppendNum(res, EDecimal); */
/*          WFDialog::ShowScrollingWarningDialogL(filename); */
      //WFDialog::ShowQueryL(_L("Language file failed write"));
      WFDialog::ShowDebugDialogL("language file failed write");
   }
}

TPtrC 
CWayFinderAppUi::UpdateLangCodesL()
{
   if (!iLangCodeInIso || !iLangCodeInSymbian) {
      CLanguageList* langList = CLanguageList::NewLC(R_WAYFINDER_LANGUAGE_LIST_NEW);
#ifdef ONLY_PHONE_LANG_ROM_BUILD
      class RFs fs;
      if(KErrNone != fs.Connect()){   
         // Not good at all, nothing to do. The program will most probably crash like hell!
         return KNullDesC();
      }
      _LIT(KRscExt, ".rsc");
      _LIT(KResourcePath, "\\resource\\apps\\");
      TParse parser;
      parser.Set(Application()->DllName(), NULL, NULL);

      HBufC* rscFileName = AllocLC(parser.Drive() + 
                                   KResourcePath +
                                   parser.Name() +
                                   KRscExt);
      TLanguage lang;
      TBuf<256> tmp = rscFileName->Des();
      BaflUtils::NearestLanguageFile(fs, tmp, lang);
      CleanupStack::PopAndDestroy(rscFileName);
      fs.Close();
      if (lang == ELangNone) {
         // Probably just have a rsc file, not a r01 or similar.
         // Need to get the language code by calling User::Language
         lang = User::Language();
      }
      // Get the correct iso name for the syntax language that BaflUtils
      // selected for us.
      CLanguageList::TWfLanguageList& newLang =
         langList->FindBySymbianCode(lang);
#else
      TBuf<5> tmpSymbianCode;
      CCoeEnv::Static()->ReadResourceL(tmpSymbianCode, R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE);
      CLanguageList::TWfLanguageList& newLang =
         langList->FindBySymbianName(tmpSymbianCode);
#endif
      iLangCodeInIso = newLang.isoname.AllocL();
      iLangCodeInSymbian = newLang.symbianname.AllocL();
      iLanguageCode = NavServerComEnums::languageCode(newLang.nav2code);
      CleanupStack::PopAndDestroy(langList);
   }
   return *iLangCodeInSymbian;
}

TPtrC 
CWayFinderAppUi::GetLangCodeInIso()
{
   UpdateLangCodesL();
   return *iLangCodeInIso;
}

NavServerComEnums::languageCode CWayFinderAppUi::GetLanguageCode()
{
   UpdateLangCodesL();
   return iLanguageCode;
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
   char *tmp = WFTextUtil::strdupL(iImeiNbr);
   iDataStore->iWFAccountData->setImei(tmp);

   if (iImeiCrcHex) {
      delete[] iImeiCrcHex;
   }
   iImeiCrcHex = new char[9]; // Eight chars and room for ending zero
   sprintf(iImeiCrcHex, "%08x", 
           crc32::calcCrc32((uint8*)iImeiNbr, strlen(iImeiNbr)));
   iImeiReceived = ETrue;

   ImeiFetched();
#endif
}

void 
CWayFinderAppUi::FetchImsi()
{
   if ( iImsiReceived ) {
      // Already received.
      return;
   }
#ifndef __WINS__
   iImsiFetcher = CWayfinderImsiFetcher::NewL(*this, ETrue);
#else
   // Set the imei as imsi for WINS, since not possible to get 
   // the real imsi.
   if ( iImeiReceived && iImeiNbr != NULL ) {
      HBufC* tmp = WFTextUtil::Utf8Alloc( iImeiNbr );
      ImsiNumberSet( tmp->Des() );
      delete tmp;
   } else {
      // No imei for some reason. Just use some hardcoded value instead.
      HBufC* tmp = WFTextUtil::Utf8Alloc( "imsi-imsi-imsi" );
      ImsiNumberSet( tmp->Des() );
      delete tmp;
   }
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
   UpdateLangCodesL();

   _LIT(KParamPath, "params");
   _LIT(KParamSeedName, "paramseed.txt");
   // Set up a TParse object with path information about the paramseed path (writabel path)
   TParse writableParamSeedPath;
   writableParamSeedPath.Set(iPathManager->GetWritableCommonDataPath(), &KParamSeedName, NULL);
   writableParamSeedPath.AddDir(KParamPath);
   RFs fs;
   fs.Connect();
   if (!BaflUtils::FolderExists(fs, writableParamSeedPath.DriveAndPath())) {
      // See if params folder exists in the writable path, if not create it
      fs.MkDirAll(writableParamSeedPath.DriveAndPath());
   } 
   if (!BaflUtils::FileExists(fs, writableParamSeedPath.FullName())){
      // The path exists but the file does not exist in the writable area.
      TFindFile finder(fs);
      // Search for paramseed.txt, first seach in the writable path.
      TInt ret = finder.FindByDir(writableParamSeedPath.FullName(), writableParamSeedPath.Path());
      if (ret == KErrNone) {
         // Found the file, now we need to check if the file was found on a write protected drive
         TParse parser;
         // Setup the parse with the complete name of the found file
         parser.Set(finder.File(), NULL, NULL);
         TChar drive(parser.Drive()[0]);
         TInt driveNbr;
         // Check if the drive is rom or writeprotected, probably writeprotected if rom but still... 
         fs.CharToDrive(drive, driveNbr);
         TDriveInfo driveInfo;
         fs.Drive(driveInfo, driveNbr);
         if (driveInfo.iMediaAtt == KMediaAttWriteProtected || 
             driveInfo.iDriveAtt == KDriveAttRom) {
            // The param seed is not in the writable drive, copy it to the writable area
            ret = BaflUtils::CopyFile(fs, parser.FullName(), writableParamSeedPath.DriveAndPath());
         }
      }
      fs.Close();
   }

   /* We don't care about failure, we'll handle that elsewhere. */
   char* resPath = WFTextUtil::newTDesDupL(iPathManager->GetLangResourcePath());
   LOGNEWA(resPath, char, strlen(resPath) + 1);
   CleanupStack::PushL(TCleanupItem(CCC::CleanupArray, resPath));

   char* commondataPath  = WFTextUtil::newTDesDupL(iPathManager->GetWritableCommonDataPath());
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
//    AudioCtrlLanguage *audioSyntax = NewSyntaxLC(m_laanguageCode, 
//          R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE);
//    LOGNEW(audioSyntax, AudioCtrlLanguage);
   UpdateLangCodesL();
   AudioCtrlLanguage *audioSyntax = AudioCtrlLanguageScript::New();
   CleanupStack::PushL(audioSyntax);

   // Find out and remember what distance units the language supports.
   GetSettingsData()->iAudioHasFeetMiles  = audioSyntax->supportsFeetMiles();
   GetSettingsData()->iAudioHasYardsMiles = audioSyntax->supportsYardsMiles();

   // Set up parameters for Nav2
   Nav2StartupData startdata(GuiProtEnums::InvalidWayfinderType, resPath, commondataPath, dataPath, errorTable, audioSyntax);
   startdata.setSimulation(SIMINPUT, Nav2StartupData::SIM_PROTO);
   startdata.setProgramVersion(isab::Nav2_Major_rel,
                               isab::Nav2_Minor_rel,
                               isab::Nav2_Build_id);

   startdata.clientType          = NAV2_CLIENT_TYPE_STRING;
   startdata.clientTypeOptions   = NAV2_CLIENT_OPTIONS_STRING;
   startdata.debugOutput         = DEBUGOUTPUT;

# ifdef USE_TCP_SYMBIAN_SERIAL
   startdata.tcpsymbianserial = true;
# endif

# ifdef __WINS__
   if (iImsiNbr) {
      delete[] iImsiNbr;
      iImsiNbr = NULL;
   }
   iImsiNbr = WFTextUtil::strdupL( iImeiNbr );
# endif

   const char* licenceKey = iImeiNbr;
   const char* licenceKeyType = "imei";

# ifdef LICENCE_KEY_TYPE_IMSI
   licenceKey = iImsiNbr;
   licenceKeyType = "imsi";
# endif

// # ifdef USE_WF_ID
   // If we use wayfinder id.
   startdata.hardwareIDs.push_back(new isab::HWID(iImeiNbr, "imei"));
   startdata.hardwareIDs.push_back(new isab::HWID(iImsiNbr, "imsi"));
   startdata.wayfinderIDStartUP = true;
// # endif

   // Set the licence key.
   startdata.setSerialNumber( licenceKey );

   // Set the licence key type, typically imei or imsi.
   startdata.setLicenceKeyType( licenceKeyType );

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
# endif
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
# endif
# ifdef USE_TRACKING
   startdata.setUseTracking( true );
# endif

# ifdef MAXNBROUTSTANDING
   startdata.setmaxnbroutstanding( MAXNBROUTSTANDING );
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
         if (iShowNoGpsWarn) { 
            WFDialog::ShowInformationL( R_WAYFINDER_LBS_WARN_NO_GPS, iCoeEnv );
            iShowNoGpsWarn = EFalse;
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
         if (iNavigateToGpsWizCallbackFunc && iNavigateToGpsWizCallbackObject) {
            // We connected to a lbs gps and we came from a "navigate to" view
            // so now call the callback func iNavigateToGpsWizCallbackFunc.
            (*iNavigateToGpsWizCallbackFunc)(iNavigateToGpsWizCallbackObject);
            iNavigateToGpsWizCallbackFunc = NULL;
            iNavigateToGpsWizCallbackObject = NULL;
         }
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
# define SHOW_ALL_MLFW_STATUS
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
         WFDialog::ShowInformationL( R_WAYFINDER_LBS_WARN_USER_DISABLED );
         break;
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

TBool CWayFinderAppUi::ConnectToGpsQueryDialogL()
{
   return WFDialog::ShowQueryL(R_WF_DO_YOU_WANT_TO_CONNECT, iCoeEnv);
}

TBool 
CWayFinderAppUi::CheckAndDoGpsConnectionL(void* pt2Object, 
                                          void (*aCallbackFunction)(void* pt2Object))
{
   if (!IsGpsConnectedBare()) {
      // We are not connected nor connecting to any gps.

      if (!pt2Object) {
         // No callback object so just connect directly without question.
         // This is a special case to be able to automatically connect in 
         // the end of the startup.
         ConnectToGpsL();
         return EFalse;
      }

      if (ConnectToGpsQueryDialogL()) {
         // User wants to connect to gps but we have to wait with the actual
         // calling of navigate to function until we know that lbs connected 
         // proberly to internal or bt gps. (Then we call the callback func
         // iNavigateToGpsWizCallbackFunc.)
         iNavigateToGpsWizCallbackObject = pt2Object;
         iNavigateToGpsWizCallbackFunc = aCallbackFunction;
         ConnectToGpsL();
         return EFalse;
      } else {
         // If no GPS is connected, and the user didnt want to connect one,
         // don't do anything in this function.
         return EFalse;
      }
   }

   // We are already connected to a gps so let the caller order the route 
   // directly.
   return ETrue;
}

//#define GPS_SOURCE_SELECTION
void CWayFinderAppUi::ConnectToGpsL()
{
   if (!EarthAllowGps()) {
      //show dialog and return
      EarthShowGPSNoticeL();
      return;
   }
   TBool useBTGps = ETrue;
/*    if( IsFullVersion() ){ */
#if defined(GPS_SOURCE_SELECTION) || ! defined(NOKIALBS)
      useBTGps = ETrue;
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
      useBTGps = ETrue;
      if (!iPositionProvider) {
         iPositionProvider = CNokiaLbsReader::NewL(*this, 
                                                   m_nav2->getBluetoothChannel(),
                                                   KApplicationVisibleName);
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
      if(iPositionProvider) {
         if (iPositionProvider->IsConnected()) {
            iPositionProvider->DisconnectL();
         } 
         TRAPD(err, iPositionProvider->ConnectL());
         if(err != KErrNone){
            // FIXME - XXX - These error must be handled
         } else {

         }
      }
#endif
   GenericGuiMess message( GuiProtEnums::CONNECT_GPS );
   SendMessageL( &message );
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
      iBtHandler->SetAddress(*(GetSettingsData()->iGpsParam));
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
         iShowNoGpsWarn = ETrue;
      }
      //      delete iPositionProvider;
      //      iPositionProvider = NULL;
   }
#endif
   iIniFile->setGpsId(0);
   iGotBTData = 0;
   GpsStatus(EFalse);
   GenericGuiMess message( GuiProtEnums::DISCONNECT_GPS );
   SendMessageL( &message );
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
   if (iMapView) {
      // Update the scale in map view.
      iMapView->SetDistanceMode(mode);
   }
}

TInt
CWayFinderAppUi::GetDistanceMode()
{
   return TInt(GetSettingsData()->m_distanceMode);
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
      //if (iStartView) {
      //   iStartView->GpsStatus(on);
      //}
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

void CWayFinderAppUi::GetGpsStatusImageId(TInt& aImgId, TInt &aImgMaskId)
{
   if (iGpsQuality > QualityUseless) {
      /// The gps has a fix, we should display the gps icon
      /// displaying we have a signal
      aImgId = EMbmWficonsGps_icon_has_fix;
      aImgMaskId = EMbmWficonsGps_icon_has_fix_mask;
   } else if (iGpsQuality > QualityMissing) {
      /// Connected but no fix, display gps icon explaining
      /// this
      aImgId = EMbmWficonsGps_icon_no_fix;
      aImgMaskId = EMbmWficonsGps_icon_no_fix_mask;      
   } else {
      /// Not connected to the gps
      aImgId = EMbmWficonsGps_icon_not_connected;
      aImgMaskId = EMbmWficonsGps_icon_not_connected;
   }
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
   class CDesC8Array& gpsParam = *(GetSettingsData()->iGpsParam);
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
      GetSettingsData()->SetBtGpsAddressL((const char**)array, 3);
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
   if(!GetSettingsData()->m_linkLayerKeepAlive || 
      (iConnMgr && GetIAP() != iConnMgr->Iap())){

      delete iConnMgr;
      iConnMgr = NULL;
   }
   if(GetSettingsData()->m_linkLayerKeepAlive && !iConnMgr && GetIAP() >= 0){
      iConnMgr = CSimpleConnectionManager::NewL(GetIAP());
   }
#endif
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

   TInt speed = -1;
   TInt unit_speed = -1;

   if( aPositionMess->speedQuality() > QualityUseless ) {
      TReal rSpeed = aPositionMess->getSpeed()*3.6;
      speed = (TInt)((rSpeed/32)+0.5);
      rSpeed = DistancePrintingPolicy::convertSpeed(aPositionMess->getSpeed(),
            DistancePrintingPolicy::DistanceMode(GetDistanceMode()));
      unit_speed = (TInt)((rSpeed/32)+0.5);
   }

   iDataHolder->SetInputData( lat, lon, heading, unit_speed, alt );
   iInfoView->SetInputData( lat, lon, heading, unit_speed, alt );
   
   if( iTabGroup->ActiveTabId() == KMapViewId.iUid ){
      iMapView->UpdateDataL( aPositionMess, speed, unit_speed );
   }
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
   TBool deleteMessageData = ETrue;
   LOGHEAPSIZE("#1 CWayFinderAppUi::ReceiveMessageL");
   if(iMemCheckInterval-- < 1){
      iMemCheckInterval = WAYFINDER_MEMORY_CHECK_INTERVAL;
      if( !CheckMem(80*1024 ) ) {
         WFDialog::ShowScrollingDialogL( iCoeEnv, R_WAYFINDER_MEMORY_LOW_MSG,
                                         R_WAYFINDER_MEMORY_LOW_EXIT_MSG, ETrue);
         iUrgentShutdown = ETrue;
         /* Switch to main view. */
         push( KWelcomeViewId );
      }
   }

   if (!iMessagesReceived) {
      /* Start startup on first message from Nav2. */
      iMessagesReceived = ETrue;
      m_startupHandler->StartupEvent(SS_Start);
//      m_startupHandler->StartupEvent(SS_StartupFinished);
   }

   if (!m_hasSetLanguage) {
      UpdateLangCodesL();
      if (iLanguageCode < 0) {
         /* Don't send the language code, haven't found it... */
         /* What to do? */
      } else {
         TInt code = GetSymbianLanguageCode();
         iSettingsView->SetLanguage(code);
         GeneralParameterMess* gpm = new (ELeave) GeneralParameterMess(
               GuiProtEnums::paramLanguage, int32(iLanguageCode));
         SendMessageL( gpm );
         gpm->deleteMembers(); // not done elsewhere, so do it here
         delete gpm;
      }
      m_hasSetLanguage = ETrue;
   }

   if (GetSettingsData()->m_backlightStrategy == GuiProtEnums::backlight_always_on || 
       iKeepBacklightOn || 
	   (iHasRoute && iMapView->MapAsGeneralNavigation() &&
	    GetSettingsData()->m_backlightStrategy == GuiProtEnums::backlight_on_during_route)) {
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

   if ( message->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED ) {
      if(message->getMessageID() == iLastRouteRequestMessId) {
         iHandlingFailedRouteRequestMessage = ETrue;
      }
   }
   
   // Check new message handling.
   if (iGuiProtHandler) {
      if (iGuiProtHandler->GuiProtReceiveMessage(message)) {
         /* Message handled. */
         iHandlingFailedRouteRequestMessage = EFalse;
         return;
      }
   }
   iHandlingFailedRouteRequestMessage = EFalse;

   GuiProtEnums::MessageType type = message->getMessageType();
   switch( type )
   {
   case GuiProtEnums::MESSAGETYPE_ERROR:
      {
         ErrorMess* errMess = static_cast<ErrorMess*>(message);
         iMyDestView->SetStatesFalse();
         RequestCancelled();

         const TInt errNumber = errMess->getErrorNumber();
         // Do special filtering of a time out message, which can happen
         // if NavTask requests a reroute when we have lost the connection.
         if ((errNumber == Nav2Error::NSC_SERVER_COMM_TIMEOUT_CONNECTED)
            && (!iTimeOutErrorFilter->displayErrorMessage())) {
            // It's not time to show the error message, or it has
            // already been displayed within the allotted time.
           break;
         }
         
         ShowErrorDialogL( errNumber,
                           errMess->getErrorString() );
      }
      break;
   case GuiProtEnums::REQUEST_FAILED:
      {
         RequestFailedMess* rf = static_cast<RequestFailedMess*>(message);
         HandleRequestFailedL(rf);
      }
      break;
   case GuiProtEnums::GET_FAVORITES_REPLY:
      if( iTabGroup->ActiveTabId() == KMyDestViewId.iUid ) {
         deleteMessageData = EFalse;
         iMyDestView->HandleFavoritesReply( (GetFavoritesReplyMess*)message );
      } else if (iTabGroup->ActiveTabId() == KMapViewId.iUid) {
/*    iMapView->HandleFavoritesReply( (GetFavoritesReplyMess*)message ); */
      }
      break;
   case GuiProtEnums::GET_FAVORITES_ALL_DATA_REPLY:
      if( iTabGroup->ActiveTabId() == KMyDestViewId.iUid ) {
         iMyDestView->HandleFavoritesAllDataReply( (GetFavoritesAllDataReplyMess*)message );
      }
      iMapView->HandleFavoritesAllDataReply( (GetFavoritesAllDataReplyMess*)message );
      break;
   case GuiProtEnums::GET_FAVORITE_INFO_REPLY:
      if( iTabGroup->ActiveTabId() == KMyDestViewId.iUid ) {
         iMyDestView->HandleFavoriteInfoReply((GetFavoriteInfoReplyMess*)message);
      } else if (iTabGroup->ActiveTabId() == KMapViewId.iUid) {
         iMapView->HandleFavoriteInfoReply((GetFavoriteInfoReplyMess*)message);
      }
      break;
   case GuiProtEnums::FAVORITES_CHANGED:
      {
         if( iSynchronizing ){
            iSynchronizing = EFalse;
         }
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
         if (iEditFavoriteView->WaitingForFavoriteReply()) {
            iEditFavoriteView->FavoriteReplyGot();
            iEditFavoriteView->ShowConfirmationDialogL();
         }
#endif
         if (iTabGroup->ActiveTabId() == KMyDestViewId.iUid && 
             !IsLmsManagerOpRunning() && !IsFavLmsSyncRunning()) {
            RequestFavoritesL( EFalse );
            GetGuiLandmarksAsyncL(MWFLMSManagerBase::EWFLMSLMFavs);
         } else if (iMyDestView->IsAdding()) {
            iMyDestView->ShowConfirmationDialogL();
            iMyDestView->SetStatesFalse();
         } else {
            iMyDestView->SetStatesFalse();
         }
         iMapView->FavoriteChanged();
         break;
      }
   case GuiProtEnums::UPDATE_ROUTE_INFO:
      if (GetSettingsData()->m_backlightStrategy == GuiProtEnums::backlight_on_during_route) {
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
         ClearLastRouteRequest();

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
            StopWaitDlg();
            StopRouteWaitDlg();
            StartNavigationL(dataMessage->getoLat(), dataMessage->getoLon(),
                             dataMessage->getdLat(), dataMessage->getdLon(),
                             dataMessage->getDestination());
         }
         else{
            if (IsSimulating()) {
               // If we are in simulation mode and we have received this message
               // which means that the route is either finished or removed by user.
               // If so, we need to set the simulation status to off since the NavTask
               // has set it's state to off. We will get strange behaviour if thinking
               // we are simulating when not.
               iSimulationStatus &= ~isab::RouteEnums::simulate_on;
            }
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
      //iMyDestView->ShowDetailsL(static_cast<FullSearchDataReplyMess*>(message));
      break;
   case GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID_REPLY:
/*       WFDialog::ShowDebugDialogL("GOT Reply full search with item id"); */
      iMapView->ShowDetailsL(static_cast<FullSearchDataReplyMess*>(message));
      break;
   case GuiProtEnums::SEND_MESSAGE_REPLY:
      break;
   case GuiProtEnums::PANIC_ABORT:
      // A fatal error was caught by Nav2. Close the program.
      HandleCommandL(EWayFinderExitAppl);
      break;
   case GuiProtEnums::PARAMETERS_SYNC_REPLY:
      {
         if (iIAPSearcher && iIAPSearcher->Searching()) {
            /* IAP request. */
            iIAPSearcher->Reply();
         }
      }
      break;
   case GuiProtEnums::GET_MULTI_VECTOR_MAP_REPLY:
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
   case isab::GuiProtEnums::GUI_TO_NGP_DATA_REPLY:
      {
         isab::DataGuiMess *mess = static_cast<isab::DataGuiMess *>(message);

         int32 type = -1;
         isab::NonOwningBuffer mess_type_data(const_cast<uint8 *>(mess->getData()), mess->getSize());
            
         if(mess_type_data.remaining() >= 2){
            type = mess_type_data.readNextUnaligned16bit();
         }

         switch(type){
         case navRequestType::NAV_COMBINED_SEARCH_REPLY:
            StopSearchWaitDlg();
            if (iCSDispatcher) {
               //isab::DataGuiMess *mess = static_cast<isab::DataGuiMess *>(message);
               iCSDispatcher->handleCombinedSearchResultsGot(mess);
            }
            break;
         case navRequestType::NAV_SEARCH_DESC_REPLY:
            if (iCSDispatcher) {
               //isab::DataGuiMess *mess = static_cast<isab::DataGuiMess *>(message);
               iCSDispatcher->handleCombinedSearchCategoriesGot(mess);
            }
            break;
         case navRequestType::NAV_NOP_REPLY:
            if (iIAPSearcher && iIAPSearcher->Searching()) {
               /* IAP request. */
               iIAPSearcher->Reply();
            }
            break;
         case navRequestType::NAV_CELLID_LOOKUP_REPLY:
            // Cell ID lookup reply
            handleCellIDLookupReply(*mess);
            break;
         default:
            break;
         }
      }
   break;
   case GuiProtEnums::LICENSE_UPGRADE_REPLY:
   case GuiProtEnums::GET_GENERAL_PARAMETER:
   case GuiProtEnums::PARAMETER_CHANGED:
   case GuiProtEnums::GET_TOP_REGION_LIST_REPLY:
      /* Not received here anymore. */
      break;
   case GuiProtEnums::SET_GENERAL_PARAMETER:
      isab::GeneralParameterMess* gpm =
          static_cast<isab::GeneralParameterMess*>(message);
      if (gpm->getParamId() == isab::GuiProtEnums::paramTransportationType) {
        // Got message that transportation type was successfully set
        // Check if we have a pending request - in that case, dispatch it
        if(iPendingRouteData) {
          SendPendingRouteMessageL();
        }
     }
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
   }
   if (deleteMessageData) {
      message->deleteMembers();
   }
   LOGDEL(message);
   delete message;
}

void CWayFinderAppUi::HandleGpsQuality( Quality gpsQuality )
{
   if( iGpsQuality != gpsQuality ){
      iGpsQuality = gpsQuality;
      // Set the correct gps icon in map view
      TInt gpsImgId;
      TInt gpsImgMaskId;
      GetGpsStatusImageId(gpsImgId, gpsImgMaskId);
      if(iGpsQuality <= QualityUseless) {
         iMapView->MakeGpsIndicatorVisible(ETrue);
         iMapView->SetGpsStatusImage(gpsImgId, gpsImgMaskId);
      }
      else {
         iMapView->MakeGpsIndicatorVisible(EFalse);
      }
      iContextPaneAnimator->SetCurrentGpsStatus(iGpsQuality);
      CAnimatorFrame* temp = new CAnimatorFrame();
      temp->CopyFrameL(iContextPaneAnimator->GetLargeGpsFrameL());
      temp->DisownBitmaps();
      delete temp;
      LOGDEL(temp);
      SetContextIconL();
      if( iGpsQuality < QualityUseless ){
         if(!iMapView->MapAsGeneralNavigation())
         {
            // Never hide the user's position while the map is in
            // navigation state.
            iMapView->HideUserPosition();
         }
      }
      if (iTabGroup && iTabGroup->ActiveTabId() == KCSMainViewId.iUid) {
         iCSMainView->UpdateButtonVisibility();
      }
   }
}

void CWayFinderAppUi::HandlePhoneCallL( TBool aConnected )
{
   m_phoneCallInProgress = aConnected;
   updateMute();
   if( aConnected ){
      // Phoning
   } else {
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
                                  TInt32 aLat, TInt32 aLon, 
                                  TInt aViewState )
{
   if( iMapView->VectorMaps() && iTabGroup->ActiveTabId() != KMapViewId.iUid ){
      iMapView->SetMapRequestData( aMapType, aLat, aLon );
      push( KMapViewId, aViewState );
   } else {
      iMapView->RequestMap( aMapType, aLat, aLon );
   }
}

void CWayFinderAppUi::GotoStartViewL()
{
   push( KMainMenuViewId /*KStartPageViewId*/ );

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
      if(!iSynchronizing) {
         iSynchronizing = ETrue;
         SendSyncFavorites();
      }
   } else {
      GenericGuiMess get(GuiProtEnums::GET_FAVORITES, uint16(0),
                         uint16(MAX_UINT16));
      SendMessageL(&get);
   }
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
   iMapView->SetDestination( dLat, dLon );
/*    iMapView->SetOrigin( dLat, dLon ); */
}

void
CWayFinderAppUi::SetCurrentRouteDestinationName( const TDesC &aDescription )
{
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
}

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

void CWayFinderAppUi::SetViewL( TUid aUid, TUid aMessageId, const TDesC8& aMessage )
{
   UpdateCurrentView( aUid );
   setNaviPane(); // Need to set this here, won't update correctly otherwise.
   iTabGroup->SetActiveTabById( aUid.iUid );
   if ( iForeGround ) {
      // Symbian signed says that the application must not
      // pop up when in background.
      // Change to the correct view when the HandleForeground event is
      // received instead.
      ActivateLocalViewL( aUid, aMessageId, aMessage );
   }
}

void CWayFinderAppUi::setTitleText(const TDes& aTitle)
{
	TUid titlePaneUid;
	titlePaneUid.iUid = EEikStatusPaneUidTitle;
 
	//  calling CAknAppUi::StatusPane() which returns CEikStatusPane* //
	CEikStatusPane* statusPane = StatusPane();
	CEikStatusPaneBase::TPaneCapabilities subPane = statusPane->PaneCapabilities(titlePaneUid);
 
	// if title pane is accessible //
 
	if ( subPane.IsPresent() && subPane.IsAppOwned() )
	{
		CAknTitlePane* titlePane = (CAknTitlePane*) statusPane->ControlL(titlePaneUid);
		// read the title text from the resource file
		HBufC* titleText = aTitle.AllocLC();
		// set the title pane's text
		titlePane->SetTextL(*titleText);
		CleanupStack::PopAndDestroy(titleText);
	}
}
void
CWayFinderAppUi::setNaviPane(TBool aShow)
{
   // Set navigation arrows if in group.
#define USE_NAVIPANE_ARROWS
#ifdef USE_NAVIPANE_ARROWS
   if (!aShow) {
      // Override any other logic and just hide them.
      GetNavigationDecorator()->MakeScrollButtonVisible(EFalse);
      return;
   }
   TBool l = m_viewStack->prevViewInGroup() != m_viewStack->invVid();
   TBool r = m_viewStack->sucViewInGroup()  != m_viewStack->invVid();
   if ( l || r ) {
      GetNavigationDecorator()->MakeScrollButtonVisible( ETrue );
      GetNavigationDecorator()->SetScrollButtonDimmed( 
         CAknNavigationDecorator::ELeftButton,  !l );
      GetNavigationDecorator()->SetScrollButtonDimmed( 
         CAknNavigationDecorator::ERightButton, !r );
   } else {
      GetNavigationDecorator()->SetScrollButtonDimmed( 
         CAknNavigationDecorator::ELeftButton,  ETrue );
      GetNavigationDecorator()->SetScrollButtonDimmed( 
         CAknNavigationDecorator::ERightButton, ETrue );
      GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
   }
#else
   GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
#endif
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
      if (iMapView) { 
         iMapView->SetConStatusImage(NULL, NULL);
      } 
      if (iWelcomeView) {
         iWelcomeView->SetConStatusImage(NULL, NULL);
      }
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

void CWayFinderAppUi::SetFromGps()
{
   iSelectView->HandleCommandL(EWayFinderCmdPositionSelectFromGps);
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
}

CAknNavigationDecorator* CWayFinderAppUi::GetNavigationDecorator()
{
   return iDecoratedTabGroup;
}

void CWayFinderAppUi::ActivateMainMenuNaviPaneLabelL()
{
   _LIT(KDefault, "Menu");
   iNaviPane->PushL(*iDecoratedNaviLabel);
   iNaviPaneLabel->SetTextL(KDefault());
}

void CWayFinderAppUi::DeactivateMainMenuNaviPaneLabelL()
{
   iNaviPane->Pop(iDecoratedNaviLabel);
}

void CWayFinderAppUi::SetMainMenuNaviPaneLabelL(TInt aResourceId)
{
   HBufC* label = iCoeEnv->AllocReadResourceL(aResourceId);
   iNaviPaneLabel->SetTextL(*label);
   iNaviPane->DrawDeferred();
   delete label;
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

   // Add code here to test an action by the UI.
//   __UHEAP_MARKEND;
}

void CWayFinderAppUi::GotoServiceViewL(const TDesC& aUrl, enum BackActionEnum aAllowBack)
{
   // Got to the url in the browser
}

void CWayFinderAppUi::GotoInfoInServiceViewL(TInt aLat, TInt aLon,
                                             const HBufC* aSrvString)
{
   // Show information, either in browser or in native view
}

void CWayFinderAppUi::LaunchBuyExtensionWapLinkL()
{
   // Launch buy link
}

void CWayFinderAppUi::LaunchWapLinkL(const TDesC& aUrl)
{
   LaunchWapBrowserUtil::LaunchWapBrowser(aUrl);
}

void CWayFinderAppUi::ShowHelpL(TInt32 aUid)
{
   TInt32 uid = aUid;
   if (uid == EWayFinderMapTab && iMapView->MapAsGeneralNavigation()) {
      // If current view is map view and it is in navigation mode, 
      // we need a special hack for the uid to show the navigation map help.
      uid = EWayFinderNavigationMapTab;
   }
   HBufC* helpUrl = iHelpUrlHandler->FormatLC(uid);
   GotoServiceViewL(*helpUrl, BackIsViewBack);
   CleanupStack::PopAndDestroy(helpUrl);
}

void CWayFinderAppUi::ShowHelpL()
{
#ifdef _FIX_OLD_HELP_
   TInt32 index = iTabGroup->ActiveTabIndex();
   TInt32 uid = iTabGroup->TabIdFromIndex( index );
   TInt resourceId1 = R_WAYFINDER_HELP_NO_HELP;
   TInt resourceId2 = 0;
   TInt resourceId3 = 0;
   TInt resourceId4 = 0;

   if( uid == KMainMenuViewId.iUid ){
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
   } else if( uid == KInfoViewId.iUid ){
      resourceId1 = R_INFO_HELP_COMMON_0;
   }
   ShowHelpL( resourceId1, resourceId2, resourceId3 );
#endif
}

void CWayFinderAppUi::ShowUpgradeInfoL( enum TUpgradeMode mode )
{
   if(iUrgentShutdown){
      return;
   }

   /* XXX show upgrade page in service window. XXX */
   m_startupHandler->StartupEvent(SS_ShowUpgradePage);

}

void
CWayFinderAppUi::GotoMainMenu()
{
   // Display the end user warning message.
   ShowEndUserWarningMessageL();

   // Automatically connect to lbs gps (internal or bt) via the gps wizard.
   CheckAndDoGpsConnectionL(NULL, NULL);

   // Sync the favorites with lms in background
      
   if (iCSDispatcher) {
      iCSDispatcher->requestCategories();
   }

   // Fetch the network info and cell id   
   FetchNetworkInfoL(EUseForGettingPositionAndRoamingState);

   //RequestFavoritesL( EFalse );
   GotoStartViewL();
}

void CWayFinderAppUi::ActivateSplashViewL()
{
   push( KWelcomeViewId );
}

void CWayFinderAppUi::ShowRegistrationSmsDialog()
{
#if defined (_MSC_VER) || defined (DONT_SEND_SMS) 
   class isab::GeneralParameterMess gpm(isab::GuiProtEnums::paramRegistrationSmsSent, int32(1));
   SendMessageL(gpm);
   m_startupHandler->StartupEvent(SS_StartupFinished);
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
      if (sms) {
         if (!iSmsHandler) {
            iSmsHandler = SmsHandlerFactory::CreateL( this );
         }
         iSmsHandler->SendSmsL(*sms, *smsNum);          
         delete sms;
      }
      class isab::GeneralParameterMess gpm(isab::GuiProtEnums::paramRegistrationSmsSent, int32(1));
      SendMessageL(gpm);
      m_startupHandler->StartupEvent(SS_StartupFinished);      
   } else {
      if (IsIronVersion()) {
         // If earth, the application should be closed if user doesn't wants to
         // send registration sms.
         m_startupHandler->StartupEvent(SS_Exit);
      } else {
         // Not earth, generate the next event.
         m_startupHandler->StartupEvent(SS_StartupFinished);
      }
   }
   delete smsNum;
#endif
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
   push( KWelcomeViewId, EShutdown );
}

void
CWayFinderAppUi::ClearBrowserCache()
{
}

void CWayFinderAppUi::RealShutdown()
{
   if (!iUrgentShutdown) {
      iUrgentShutdown = ETrue;
      GenerateEvent(EWayfinderEventShutdownNow);
   }
   
}

void CWayFinderAppUi::StartSpreadWayfinderDialogsL()
{
   //Construct the dialog object and show the first dialog.
   if (!iSpreadWFDialogs) {
      iSpreadWFDialogs = CSpreadWFDialogs::NewL(iSmsHandler, KShareWFSmsUrl);
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

TBool CWayFinderAppUi::DisplayEndUserWarningQueryL()
{
#ifdef SHOW_END_USER_WARNING_MESSAGE
   HBufC* title = iCoeEnv->AllocReadResourceL(R_VODAF_DISPLAY_WARNING_TITLE);
   HBufC* text  = iCoeEnv->AllocReadResourceL(R_VODAF_DISPLAY_WARNING_TXT);

   TInt buttonId = 
      CQueryDlg::RunDlgLD(R_WAYFINDER_DISPLAY_END_USER_WARNING_QUERY_DIALOG,
                          *title, *text);

   return buttonId != EAknSoftkeyYes;   
#else
   return EFalse;
#endif 
}

void CWayFinderAppUi::ShowEndUserWarningMessageL()
{
#ifdef SHOW_END_USER_WARNING_MESSAGE
   if (!iIniFile->showEndUserWarningMessage) {
      // The message has already been shown and the user has decided that
      // he does not want to see it again.
      return;
   }
   _LIT(KTwoNewlines, "\n\n");
   HBufC* title = iCoeEnv->AllocReadResourceL(R_VODAF_STARTUP_WARNING);
   HBufC* text1 = iCoeEnv->AllocReadResourceLC(R_VODAF_STARTUP_SAFETY);
   HBufC* text2 = iCoeEnv->AllocReadResourceLC(R_VODAF_STARTUPDATACOST);
   HBufC* text = AllocLC(*text1 + KTwoNewlines + *text2);
   CleanupStack::Pop(text);
   CleanupStack::PopAndDestroy(text2);
   CleanupStack::PopAndDestroy(text1);

   TInt buttonId = CQueryDlg::RunDlgLD(R_WAYFINDER_END_USER_WARNING_DIALOG,
                                       *title, *text);

   iUrgentShutdown = buttonId != EAknSoftkeyYes;
   if (iUrgentShutdown) {
      HandleCommandL(EWayFinderExitAppl);
      return;
   }

   iIniFile->doOnce(1, 
                    &IniFile::showEndUserWarningMessage,
                    MemFunPtr<CWayFinderAppUi,TBool>
                    (this, &CWayFinderAppUi::DisplayEndUserWarningQueryL));
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
      }
      iProgressDlg->StartProgressDlgL(R_PROGRESS_DIALOG, 
                                      R_WAYFINDER_IAP_SEARCH1_MSG);
      iProgressDlg->ReportProgress(aVal, aMax, aName);
   } else {
      if (iProgressDlg) {
         iProgressDlg->StopProgressDlg();
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
      HBufC* imei = WFTextUtil::AllocLC(iImeiNbr);
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

   TBool retval = EFalse;
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
   CheckIAPChosen(0);
   return retval;
}

void
CWayFinderAppUi::CheckIAPChosen(TInt ch)
{
   if (!iIAPSearcher) {
      iIAPSearcher = new (ELeave) CIAPSearcher_old(this);
   }

#if defined USE_WLAN_IN_IAP_SEARCH
   iIAPSearcher->ShowWlan(ETrue);
#endif
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
CWayFinderAppUi::SettingsSetIAP(TInt aIAP)
{
   iDataStore->iIAPDataStore->SetIap(aIAP);
}

void
CWayFinderAppUi::SetIAP(TInt aIAP)
{
#ifndef MOCKUP
   iSettingsView->SetIAP(aIAP);
   iDataStore->iIAPDataStore->SetIap(aIAP);
   if (aIAP >= 0) {
      UpdateConnectionManagerL();
   } 
   /* Set the session based IAP to the same. */
   SendIAPId2(aIAP);
#endif
}

void
CWayFinderAppUi::SendIAP(int32 aIAPid, TBool isReal, TBool aShow)
{
   if (isReal) {
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

         // Maybe try to make this non-blocking!
         WFDialog::ShowScrollingDialogL(*header, *body, EFalse);
         CleanupStack::PopAndDestroy(body);
         CleanupStack::PopAndDestroy(tmp);
         CleanupStack::PopAndDestroy(header);
      }
      m_generalParamReceiver->sendIAPId();
      m_startupHandler->StartupEvent(SS_IapSearchOk);
   }

   /* Set session based IAP parameter to the correct value. */
   SendIAPId2(aIAPid);
}

void
CWayFinderAppUi::SendIAPId2(int32 iap)
{
   GeneralParameterMess *message = new (ELeave) GeneralParameterMess(
      GuiProtEnums::paramSelectedAccessPointId2, iap);
   SendMessageL( message );
   message->deleteMembers();
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
void
CWayFinderAppUi::SendNop()
{
   isab::NParamBlock params;
   sendAsynchronousNGPRequestToNav2(params, isab::navRequestType::NAV_NOP_REQ);
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

   return accept;
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
CWayFinderAppUi::EarthAllowRoute()
{
   return (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Iron && 
           GetUserRight()->hasUserRight(GuiProtEnums::UR_ROUTE));
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
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Iron && 
       GetUserRight()->hasUserRight(GuiProtEnums::UR_USE_GPS)) return ETrue;
   return EFalse;
}

TBool
CWayFinderAppUi::IsGpsConnectedBare()
{
#ifdef NOKIALBS
   if (iPositionProvider && iPositionProvider->IsConnected()) {
      return ETrue;
   } 
#endif
   if (iBtHandler && iBtHandler->IsConnected()) {
      return ETrue;
   }
   if (iGpsQuality > QualityUseless) {
      return ETrue;
   }
#ifdef __WINS__
   if (iGpsSocket && iGpsSocket->IsConnected()) {
      return ETrue;
   }
#endif

   return EFalse;
}

TBool
CWayFinderAppUi::IsGpsConnected()
{
   TBool isConnected = EFalse;
//    if(iBtHandler && (iBtHandler->IsConnected() || iBtHandler->IsBusy())) {
// Changed the above line to the line on the next row. Need to test if it's the right thing to do
   if(iBtHandler && iBtHandler->IsConnected()) {
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
   if ((IsGpsConnected() || IsSimulating()) && CanUseGPSForEveryThing()) {
      return ETrue;
   } else {
      return EFalse;
   }
}

TBool
CWayFinderAppUi::ValidGpsStrength()
{
   return (iGpsQuality > QualityUseless);
}

TBool
CWayFinderAppUi::IsFullVersion()
{
#ifndef TRIAL_IS_MAPGUIDE
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Trial) return ETrue;
#endif
   if (iDataStore->iWFAccountData->getWfType() == GuiProtEnums::Gold) return ETrue;
   return EFalse;
}
TBool
CWayFinderAppUi::AutoRouteSmsDestination()
{
   if (CanUseGPSForEveryThing()) {
      return GetSettingsData()->m_autoRouteOnSMSDest;
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
   if (GetSettingsData()->m_autoTracking && CanUseGPSForEveryThing()) {
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
   if ( GetSettingsData()->iUserRights->hasUserRight(
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
   return GetSettingsData()->iUserRights;
}


void
CWayFinderAppUi::SaveMapLayerSettings()
{
   iMapView->SaveMapLayerSettings(GetSettingsData());
}

void CWayFinderAppUi::SaveACPSettings(TBool aEnableACP)
{
   iMapView->SaveACPSettings(aEnableACP);
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
#if defined USE_ONLY_ONE_LANGUAGE || defined ONLY_PHONE_LANG_ROM_BUILD
   return ETrue;
#else
   return EFalse;
#endif
}

TBool CWayFinderAppUi::ShowMoveWayfinder()
{
#ifdef SHOW_MOVEWAYFINDER
   return WayfinderHelper::ShowMoveWayfinder();
#else
   return EFalse;
#endif
}

TBool CWayFinderAppUi::ShowGetWayfinder()
{
#ifdef SHOW_MOVEWAYFINDER
   return WayfinderHelper::ShowGetWayfinder();
#else
   return EFalse;
#endif
}

TBool CWayFinderAppUi::IsWaitingForRoute()
{
	return iWaitingForRoute;
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

// loading indicator disabled in the map view 
//         if (!iMapView->SetConStatusImage(a, b)) {
            if (!iWelcomeView->SetConStatusImage(a, b)) {
               /* Not used. */
               delete a;
               delete b;
            }
//         }
      }
      iTimer->After(KRefresh);
      return ETrue;
   }
   return EFalse;
}

void CWayFinderAppUi::HandleRoamingDialogDone(MRoamingCallback::TRoamingStatus aStatus)
{
  if (aStatus != MRoamingCallback::EAccept)
    {
    HandleCommandL(EWayFinderExitAppl);
    }
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

//    // Test if we're trial version, don't show username/password in trial.
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

//    // Add Extra text in about box.
   HBufC* tmp = iCoeEnv->AllocReadResourceL(R_WAYFINDER_ABOUT_EXTRA_MSG );
   text->Append( *tmp );
   delete tmp;

   TBuf<KBuf32Length> temp;
   temp.Zero();
#if !defined(__RELEASE__) || defined SHOW_IMEI
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

   text->Append(_L("\nIMEI: "));
   if (iImeiNbr) {
      HBufC* tmpImei = WFTextUtil::AllocLC(iImeiNbr);
      text->Append(*tmpImei);
      CleanupStack::PopAndDestroy(tmpImei);
   }

   text->Append(_L("\nIMSI: " ));
   if (iImsiReceived) {
      HBufC* tmpImsi = WFTextUtil::AllocLC( iImsiNbr );
      text->Append( *tmpImsi );
      CleanupStack::PopAndDestroy(tmpImsi);
   }

   text->Append( _L("\nWebusername: "));
   if (iDataStore->iWFAccountData->getWebUserName()) {
      HBufC* str = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getWebUserName());
      text->Append( *str );
      CleanupStack::PopAndDestroy(str);
   }
   text->Append( _L("\nUsername: "));
   if (iDataStore->iWFAccountData->getUsername()) {
      HBufC* str = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getUsername());
      text->Append(*str);
      CleanupStack::PopAndDestroy(str);
   }
   text->Append( _L("\nWftype: "));
   text->Append( *iWfTypeStr );
   text->Append( _L("\nWfopts: "));
   text->Append( *iWfOptsStr );
#endif

#ifdef DEBUG_CELL
//    //// code /////
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

    // temp.Copy( KApplicationVisibleName );

   iCoeEnv->ReadResource( temp, R_WAYFINDER_ABOUT_MSG );
   WFDialog::ShowScrollingDialogL(temp, *text, EFalse);
   CleanupStack::PopAndDestroy(text);
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
      if (!iSmsHandler) {
         iSmsHandler = SmsHandlerFactory::CreateL( this );
      }
      iSmsHandler->SendSmsL(*sms, *smsNum); 
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
   return EFalse;
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
      EWayFinderCmdReportMapProblem,
      EWayFinderCmdReportProblem,
      EWayFinderCmdReportPosition,
   };    
   TInt32 elem = (sizeof(commands)/sizeof(*commands));
   if (command < 0 || command >= elem) {
      return;
   }
   HandleCommandL(commands[command]);
}

void CWayFinderAppUi::FetchNetworkInfoL(enum TNetworkInfoState aState)
{
   iNetworkInfoState = aState;
   iNetworkInfoHandler->FetchAllNetworkInfoL();
}

void CWayFinderAppUi::NetworkInfoDone()
{
   // Got all the network info we're interested in.

   if (iNetworkInfoState == EUseForGettingPosition || iNetworkInfoState == EUseForGettingPositionAndRoamingState) {
      // Network info should be used to quickly retrieve an 
      // estimated position from the server.
      CTelephony::TNetworkMode networkMode = iNetworkInfoHandler->iNetworkMode;
      TBuf<16> sigStrengthTxt;
      sigStrengthTxt.Num(iNetworkInfoHandler->iSigStrength);
      char* signalStrength = WFTextUtil::newTDesDupL(sigStrengthTxt);
      char* mcc = WFTextUtil::newTDesDupL(iNetworkInfoHandler->iMcc);
      char* mnc = WFTextUtil::newTDesDupL(iNetworkInfoHandler->iMnc);
      TBuf<16> lacTxt;
      lacTxt.Num(iNetworkInfoHandler->iLac);
      char* lac = WFTextUtil::newTDesDupL(lacTxt);
      TBuf<16> cellIdTxt;
      cellIdTxt.Num(iNetworkInfoHandler->iCellId);
      char* cellId = WFTextUtil::newTDesDupL(cellIdTxt);
      if (networkMode == CTelephony::ENetworkModeUnknown) {
         // We don't know what network mode we're using, do nothing.
      } else if (networkMode == CTelephony::ENetworkModeAmps || 
                 networkMode == CTelephony::ENetworkModeCdma95 || 
                 networkMode == CTelephony::ENetworkModeCdma2000) {
         // CDMA Network.
         char* cdmaType = NULL;
         if (networkMode == CTelephony::ENetworkModeCdma2000) {
            cdmaType = strdup_new("EVDO");
         } else {
            cdmaType = strdup_new("CDMA");
         }
         char* cdmaSID = WFTextUtil::newTDesDupL(iNetworkInfoHandler->iCdmaSID);
         TBuf<16> bidTxt;
         bidTxt.Num(iNetworkInfoHandler->iCdmaBID);
         char* cdmaBID = WFTextUtil::newTDesDupL(bidTxt);
         CDMACellIDRequestData cellIDReq(cdmaType, signalStrength,
                                         cdmaSID, mnc, cdmaBID);
         iNGPCellIdSeqId = sendCellIDRequest(cellIDReq);
         delete[] cdmaBID;
         delete[] cdmaSID;
         delete[] cdmaType;
      } else {
         // GSM/UMTS Network.
         char* tgppType = NULL;
         if (networkMode == CTelephony::ENetworkModeGsm) {
            tgppType = strdup_new("GPRS");
         } else {
            tgppType = strdup_new("UMTS");
         }
         TGPPCellIDRequestData cellIDReq(tgppType, signalStrength, 
                                         mcc, mnc, lac, cellId);
         // Use request number in HandleRequestFailedL to check for correct error.
         iNGPCellIdSeqId = sendCellIDRequest(cellIDReq);
         delete[] tgppType;
      }
      delete[] cellId;
      delete[] lac;
      delete[] mnc;
      delete[] mcc;
      delete[] signalStrength;

      if (iNetworkInfoState == EUseForGettingPositionAndRoamingState) {
         //Register for changes of the network registration status so we can show a dialog when it has changed.

         if(iNetworkInfoHandler->iRegStatus == CTelephony::ERegisteredRoaming) {
            //Show first warning popup if roaming
           ShowRoamingWarningPopup();
         }
         else {
            if(iNetworkRegistrationNotifier) {
               iNetworkRegistrationNotifier->Start();
            }
         }
      }
   } else if (iNetworkInfoState == EUseForDebugDialog) {
      // We have not sent the cell info request to server to get position 
      // info from the network info, just want to show the network info 
      // in a dialog (so create an empty CellPosInfo object).
      CCellPosInformation cellInfo;
      ShowNetworkInfoDialogL(cellInfo);
   } else if (iNetworkInfoState == EUseForDebugSms) {
      // Network info should be used for sending in an SMS.
      if (iEventSmsErrTxt) {
         SendDebugSms(iEventSMSid, *iEventSmsErrTxt);
         delete iEventSmsErrTxt;
         iEventSmsErrTxt = NULL;
      } else {
         SendDebugSms(iEventSMSid, KNullDesC);
      }
   }

   iNetworkInfoState = ENetworkInfoIdle;
}

void CWayFinderAppUi::NetworkInfoInUse()
{
   // Could not get the network info since the AO was busy.

   if (iNetworkInfoState == EUseForDebugSms) {
      // If it should be used for SMS, send the SMS anyway without the info.
      if (iEventSmsErrTxt) {
         SendDebugSms(iEventSMSid, *iEventSmsErrTxt);
         delete iEventSmsErrTxt;
         iEventSmsErrTxt = NULL;
      } else {
         SendDebugSms(iEventSMSid, KNullDesC);
      }
   }

   iNetworkInfoState = ENetworkInfoIdle;
}

void CWayFinderAppUi::NetworkInfoFailed(TInt aError)
{
   // Could not get the network info since the fetching returned an error.

   if (iNetworkInfoState == EUseForDebugSms) {
      // If it should be used for SMS, send the SMS anyway without the info.
      if (iEventSmsErrTxt) {
         SendDebugSms(iEventSMSid, *iEventSmsErrTxt);
         delete iEventSmsErrTxt;
         iEventSmsErrTxt = NULL;
      } else {
         SendDebugSms(iEventSMSid, KNullDesC);
      }
   }

#if defined __WINS__
   // For testing the procedure in emulator we fake that it worked fine 
   // to get the network info from the phone.
   NetworkInfoDone();
#endif

   iNetworkInfoState = ENetworkInfoIdle;
}

void CWayFinderAppUi::NetworkRegistrationStatuschanged()
{
   if(iNetworkRegistrationNotifier->RegistrationStatus() == CTelephony::ERegisteredRoaming) {
      ShowRoamingWarningPopup(ETrue);
   }
}

void CWayFinderAppUi::ShowNetworkInfoDialogL(CCellPosInformation& aCellInfo)
{
   // We just want to show the network info in a dialog.
   TBuf<KBuf64Length> temp;
   temp.Copy(KApplicationVisibleName);
   HBufC* text = HBufC::NewLC(512);
   TPtr textPtr = text->Des();
   textPtr.Copy(_L("Debug info"));
   textPtr.Append(_L("\nSigStrength: "));
   textPtr.AppendNum(iNetworkInfoHandler->iSigStrength);
   textPtr.Append(_L("\nRegStatus: "));
   textPtr.AppendNum(iNetworkInfoHandler->iRegStatus);
   textPtr.Append(_L("\nNetworkMode: "));
   textPtr.AppendNum(iNetworkInfoHandler->iNetworkMode);
   textPtr.Append(_L("\nNetworkAccess: "));
   textPtr.AppendNum(iNetworkInfoHandler->iNetworkAccess);
   textPtr.Append(_L("\nMCC: "));
   textPtr.Append(iNetworkInfoHandler->iMcc);
   textPtr.Append(_L("\nMNC: "));
   textPtr.Append(iNetworkInfoHandler->iMnc);
   textPtr.Append(_L("\nLAC: "));
   textPtr.AppendNum(iNetworkInfoHandler->iLac);
   textPtr.Append(_L("\nCellId: "));
   textPtr.AppendNum(iNetworkInfoHandler->iCellId);
   textPtr.Append(_L("\nCdmaSid: "));
   textPtr.Append(iNetworkInfoHandler->iCdmaSID);
   textPtr.Append(_L("\nCdmaBid: "));
   textPtr.AppendNum(iNetworkInfoHandler->iCdmaBID);
   textPtr.Append(_L("\nIMEI: "));
   if (iImeiNbr) {
      HBufC* tmpImei = WFTextUtil::AllocLC(iImeiNbr);
      textPtr.Append(*tmpImei);
      CleanupStack::PopAndDestroy(tmpImei);
   }
   textPtr.Append(_L("\nIMSI: " ));
   if (iImsiReceived) {
      HBufC* tmpImsi = WFTextUtil::AllocLC( iImsiNbr );
      textPtr.Append( *tmpImsi );
      CleanupStack::PopAndDestroy(tmpImsi);
   }
   TRealFormat numFormat;
   WGS84Coordinate wgs84Coord(aCellInfo.getPosition());
   textPtr.Append(_L("\n"));
   textPtr.Append(_L("\nServer CellInfo"));
   textPtr.Append(_L("\nPosition: "));
   textPtr.AppendNum(TReal(wgs84Coord.latDeg), numFormat);
   textPtr.Append(_L(", "));
   textPtr.AppendNum(TReal(wgs84Coord.lonDeg), numFormat);
   textPtr.Append(_L("\nAltitude: "));
   textPtr.AppendNum(aCellInfo.getAltitude());
   textPtr.Append(_L("\niRadius: "));
   textPtr.AppendNum(aCellInfo.getInnerRadius());
   textPtr.Append(_L("\noRadius: "));
   textPtr.AppendNum(aCellInfo.getOuterRadius());
   textPtr.Append(_L("\nsAngle: "));
   textPtr.AppendNum(aCellInfo.getStartAngle());
   textPtr.Append(_L("\neAngle: "));
   textPtr.AppendNum(aCellInfo.getEndAngle());

   WFDialog::ShowScrollingDialogL(temp, *text, EFalse);
   CleanupStack::PopAndDestroy(text);
}

void
CWayFinderAppUi::SendDebugSms(uint32 errNum, const TDesC& errTxt)
{
#if defined SEND_DEBUG_SMS_ON_ERRORS
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
# else
   tmp.Append( _L(";;;;"));
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
      } else {
         tmp2.Copy(_L("o"));
      }
   }

   HBufC* tmpImei = WFTextUtil::AllocL(iImeiNbr);
   HBufC* tmpName;
   if (iDataStore->iWFAccountData->getUsername()) {
      tmpName = WFTextUtil::AllocLC(iDataStore->iWFAccountData->getUsername());
   } else {
      tmpName = KNullDesC().AllocLC();
   }
   smsText = DebugSms::FormatLC(iDataStore->iWFAccountData->getWfType(),
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
      if (!iSmsHandler) {
         iSmsHandler = SmsHandlerFactory::CreateL( this );
      }
      iSmsHandler->SendSmsL(*smsText, KDebugSmsNumber);
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


const CCSMainView::TFormData& CWayFinderAppUi::GetFormData() const
{
  return iCSMainView->GetFormData();
}
void
CWayFinderAppUi::RequestCancelled()
{
   SetWait( EFalse, 0 );
   iSynchronizing = EFalse;
}

void
CWayFinderAppUi::ShowErrorDialogL(TInt32 aErrNbr,
      const char* aUtf8Text, 
      TBool aBanner)
{
   if (aErrNbr) {

      HBufC* errTxt = WFTextUtil::Utf8ToHBufCL(aUtf8Text);
      CleanupStack::PushL(errTxt);
      HBufC* header = HBufC::NewLC(32);
      //TBuf<32> header;
      TPtr tmpHeader = header->Des();
      iCoeEnv->ReadResource(tmpHeader, R_WAYFINDER_ERROR_MSG);
      tmpHeader.Append(_L(": "));
      tmpHeader.AppendNum(aErrNbr, EHex);
      iEventSMSid = aErrNbr;

#ifdef SEND_DEBUG_SMS_ON_ERRORS
      TBool res = 
         WFDialog::ShowScrollingQueryL(R_WF_DEBUGSMS_MESSAGE_QUERY, 
                                       *header, *errTxt, EFalse);
      if (res) {
         iEventSmsErrTxt = errTxt->AllocL();
# ifdef USE_CELL_INFO_IN_SMS
         //In this define we cant delete iEventSmsErrTxt since 
         //iNetworkInfoHandler is a nonblocking ActiveObject and
         //we need the ErrTxt when it returns.
         FetchNetworkInfoL(EUseForDebugSms);
# else
         SendDebugSms(aErrNbr, *iEventSmsErrTxt);
         delete iEventSmsErrTxt;
         iEventSmsErrTxt = NULL;
# endif
      }
      CleanupStack::PopAndDestroy(header);
      CleanupStack::PopAndDestroy(errTxt);
#else
      //WFDialog::ShowScrollingDialogL(header, *errTxt, EFalse);
      CleanupStack::Pop(header);
      CleanupStack::Pop(errTxt);
      // Dialog takes ownership of texts.
      CQueryDlg::RunDlgLD(R_MESSAGE_DIALOG, *header, *errTxt, 7000000);
#endif
   }
}

void
CWayFinderAppUi::ShowErrorDialogL(TInt32 aErrNbr,
                                  TInt aResourceId, 
                                  TBool aBanner)
{
   if (aErrNbr) {

      HBufC* errTxt = iCoeEnv->AllocReadResourceLC(aResourceId);
      TBuf<32> header;
      iCoeEnv->ReadResource(header, R_WAYFINDER_ERROR_MSG);
      header.Append(_L(": "));
      header.AppendNum(aErrNbr, EHex);
      iEventSMSid = aErrNbr;

#ifdef SEND_DEBUG_SMS_ON_ERRORS
      TBool res = 
         WFDialog::ShowScrollingQueryL(R_WF_DEBUGSMS_MESSAGE_QUERY, 
                                       header, *errTxt, EFalse);
      if (res) {
         iEventSmsErrTxt = errTxt->AllocL();
# ifdef USE_CELL_INFO_IN_SMS
         //In this define we cant delete iEventSmsErrTxt since 
         //iNetworkInfoHandler is a nonblocking ActiveObject and
         //we need the ErrTxt when it returns.
         FetchNetworkInfoL(EUseForDebugSms);
# else
         SendDebugSms(aErrNbr, *iEventSmsErrTxt);
         delete iEventSmsErrTxt;
         iEventSmsErrTxt = NULL;
# endif
      }
#else
      WFDialog::ShowScrollingDialogL(header, *errTxt, EFalse);
#endif
      CleanupStack::PopAndDestroy(errTxt);
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
   TUint failedSeqId = aMessage->getFailedRequestSeqId();

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
         StopWaitDlg();
         StopRouteWaitDlg();
         iWaitingForRoute = EFalse;
         if (eNbr == Nav2Error::NSC_NO_GPS_WARN) {
            // No gps positions is received, show a wating dialog to user
            // and continue waiting for signal
            //ShowWarningDialogL(eNbr, aMessage->getErrorString());
            StartWaitDlg(this, R_WF_WAITING_FOR_GPS);
            return;
         } else if (eNbr == Nav2Error::NSC_SERVER_ROUTE_TOO_LONG) {
            // The route is too long to be calculated, remove the 
            // old route if one exists
            if (m_routeid != 0) {
               m_routeid = 0;
               StopNavigationL();
            }
            iMapView->HandleRouteTooLongL();
         }
         break;
      case GuiProtEnums::GUI_TO_NGP_DATA:
      case GuiProtEnums::GUI_TO_NGP_DATA_REPLY:
         {
            // See if this is part of the iap test (nop_reply)
            if (Nav2Error::Nav2ErrorTable::acceptableServerError(eNbr)) {
               if (iIAPSearcher && iIAPSearcher->Searching()) {
                  iIAPSearcher->Reply();
               }
            } else {
               if (iIAPSearcher && iIAPSearcher->Searching()) {
                  iIAPSearcher->ReplyFailed();
                  handled = ETrue;
               }
            }

            if (iCSDispatcher) {
               StopSearchWaitDlg();
               CombinedSearchDispatcher::SearchRequestType searchReqType = 
                  iCSDispatcher->IsOutstandingSearchRequest(failedSeqId);
               if (searchReqType != CombinedSearchDispatcher::ENotASearchRequest) {
                  // This was a failed search request so tell the 
                  // dispatcher that search failed.
                  iCSDispatcher->searchRequestFailed(failedSeqId);
                  // Allow the combined search view know when a search 
                  // request has failed.
                  iCSMainView->HandleFailedSearchRequest();
                  if (searchReqType == CombinedSearchDispatcher::ERoundOne ||
                      searchReqType == CombinedSearchDispatcher::ERoundTwo) {
                     // Make sure we dont show any request failed dialog.
                     handled = ETrue;
                  }
                  if (searchReqType == CombinedSearchDispatcher::ERoundOne) {
                     // On round one show our no search hits dialog.
                     WFDialog::ShowInformationL(R_WAYFINDER_NOHITS_MSG);
                  }
               }
            }

            if (failedSeqId == TUint(iNGPCellIdSeqId)) {
               // Dont show request failed dialog for cell id requests.
               handled = ETrue;
               iNGPCellIdSeqId = 0;
            }
         }
         break;
      case GuiProtEnums::SYNC_FAVORITES:
         {
            iSynchronizing = EFalse;
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
#else
      if (eNbr == Nav2Error::NSC_EXPIRED_USER) {//Expired user is handled elsewhere(service window)
         return;
      }
#endif
      ShowErrorDialogL(eNbr, aMessage->getErrorString());
   }
}

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
      FetchNetworkInfoL(EUseForDebugSms);
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
      case EWayfinderEventConnectToGps:
#if !defined NOKIALBS
         if(!iBtHandler){
            ConnectToGpsL();
         }
#endif
         break;
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
      case EWayfinderFavLmsSyncDoCleanup:
         if (iFavoriteSyncHandler && !iFavoriteSyncHandler->SyncIsRunning()) {
            // Sync is done so clean up.
            delete iFavoriteSyncHandler;
            iFavoriteSyncHandler = NULL;
         }
         if( iTabGroup->ActiveTabId() == KMyDestViewId.iUid ) {
            // User have entered FavoriteView while we were syncing, this 
            // means that FavoriteView didn't request any favorites or 
            // landmarks to display on activation. So request them now when 
            // we are sure that everything is synced.
            RequestFavoritesL(EFalse);
            GetGuiLandmarksAsyncL(MWFLMSManagerBase::EWFLMSLMFavs);
         }
         break;
      case EWayfinderQuitNavigationOk:
         // Get the current navigation view first, this should be stored as 
         // the default navigation view.
         TInt activeViewUid = iTabGroup->ActiveTabId();
         if ((activeViewUid == KGuideViewId.iUid && 
              GetSettingsData()->iPreferredGuideMode != preferred_guide) ||
             (activeViewUid == KMapViewId.iUid && 
              GetSettingsData()->iPreferredGuideMode != preferred_map)) {
            // The current stored preferred navigation view isnt the same as
            // the navigation view we are going back from, we should store the new
            // value so the same view will be launched the next time a route is 
            // loaded.
            if (activeViewUid == KGuideViewId.iUid) {
               GetSettingsData()->iPreferredGuideMode = preferred_guide;
            } else {
               GetSettingsData()->iPreferredGuideMode = preferred_map;
            }
            iSettingsView->SendVectorMapSettings();
         }
         // Also switch of nightmode if it is on.
         iNightMode = EFalse;
         // User wants to go back from the navigation views to the route planner
         // and has agreed to quit the navigation
         HandleCommandL(EWayFinderCmdStopNavigation);
         HandleCommandL(EAknSoftkeyBack);
         break;
      case EWayfinderQuitNavigationCancel:
         // Really don't do anything, since the user doesn't wants to
         // quit the navigation he/she shouldn't be allowed to go back to
         // the route planner view.
         break;
   case EWayfinderEventCheckAndShowUpgradeDialog:
      if(iTabGroup->ActiveTabId() == KMainMenuViewId.iUid ){
         ShowUpdateToNewVersionDialog();
      }
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

void CWayFinderAppUi::SwitchToSettingsL()
{
   /* Before we can switch to settings, we need to read out some */
   /* volatile variables. */
   iDataStore->iSettingsData->m_allowGps = EarthAllowGps();
   iDataStore->iSettingsData->m_isIron = IsIronVersion();

   /* Switch to view. */
   push( KSettingsViewId );
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

         //iNewDestView->SetSearchDataL(ETrue);
      }
   }
}

void CWayFinderAppUi::HandleSearchReplyL( SearchItemReplyMess* aSearchItems )
{
   StopSearchWaitDlg();
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
               DistancePrintingPolicy::DistanceMode(GetSettingsData()->m_distanceMode));
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
      /*
      if( iTabGroup->ActiveTabId() == KNewDestViewId.iUid ){
         iNewDestView->SetSearchDataL();
      } else {
         push( KNewDestViewId, ESearchReply );
      }
      */
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
   iWaitingForRoute = EFalse;
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
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iGuideView->UpdateRouteDataL(0, 0);
#endif
   iGuideView->UpdateDistance( 0 );
   SetCurrentTurn( 0 );
   iItineraryView->ClearRoute();

   // Clear the start and dest flag that the ui put on the map,
   // MapLib will draw its own bitmaps for this now when the
   // route is downloaded
   iMapView->NewRouteDownloaded();

   // Calling this method seems to result in a
   // CmdaController 7 sometimes.
   //iAudioPlayer->Stop();
   if (iTabGroup->ActiveTabId() == KGuideViewId.iUid) {
      /* Don't do anything. */
   } else if (iTabGroup->ActiveTabId() == KItineraryViewId.iUid) {
      /* Don't do anything. */
   } else if (iTabGroup->ActiveTabId() == KPositionSelectViewId.iUid || 
              iManualRerouteIssued) {
      if (iMyDestView) {
         // Ugly hack to unfocus findbox of favorite view when leaving
         // the view for another one. This is necessary since otherwise
         // there will be an eikcoctl 63 crash. 
         // (look in the view and containers header files for more info)
         iMyDestView->SetFindBoxFocus(EFalse);
      }
      if (GetSettingsData()->iPreferredGuideMode == preferred_guide &&
          !IsIronVersion()) {
         /* Guide mode is Guide. */
         //push( KGuideViewId );
         m_viewStack->setupNavigationViewStack( KGuideViewId );
      } else {
         /* Guide mode is Map. */
         // XXX: push Map Guide view here when available.
         if (IsIronVersion()) {
            m_viewStack->setNextMessagId( EMapAsOverview );
         } else {
            m_viewStack->setNextMessagId( EMapAsNavigation );
         }
         m_viewStack->setupNavigationViewStack( KMapViewId );
         //push( KMapViewId, EMapAsNavigation );
      }
      iManualRerouteIssued = EFalse;
   } else if (((iTabGroup->ActiveTabId() == KMapViewId.iUid) && (!iMapView->MapAsGeneralNavigation())) ||
              iTabGroup->ActiveTabId() == KCSDetailedResultViewId.iUid ||
              iTabGroup->ActiveTabId() == KMyDestViewId.iUid) {
      // Activate the map view for navigation. Make sure we don't activate the map view if we are already
      // navigating (e.g. if we are in 2D mode) as that would force 3D mode.
      HandleCommandL(EWayFinderCmdMap);
   } else {
      // Do nothing since we now dont want to switch views all the time.
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
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iGuideView->UpdateRouteDataL(0, 0);
#endif
   iGuideView->UpdateDistance( 0 );
   SetCurrentTurn( 0 );

   iItineraryView->ClearRoute();

   // Calling this method seems to result in a
   // CmdaController 7 sometimes.
   //iAudioPlayer->Stop();
   // Must always do this!
   iMapView->ClearRoute();
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

   if (GetSettingsData()->m_backlightStrategy == GuiProtEnums::backlight_near_action &&
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
                               currDist, leftSide, exit,
                               (ri.currSeg.isHighway() || nextHighway),
                               detour?1:0, speedcam?1:0);

      iMapView->UpdateNextStreetL(nextStreet);

   } else if( newInfo ){
      if( ri.currCrossing.action == RouteEnums::Finally ){
         nextStreet.Copy( *iDestinationName );
      }
      iGuideView->UpdateStatus( ri.onTrackStatus,
                                currStreet, nextStreet, currDist );

      iMapView->UpdateStatus(ri.onTrackStatus, currDist);

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
      iMapView->UpdateETGL(ri.timeToGoal);
      iMapView->UpdateSpeedcam(speedcam?1:0);
      iMapView->UpdateDetour(detour?1:0);
   }
   iInfoView->UpdateRouteDataL( ri.distToGoal, ri.timeToGoal, ri.currSeg.speedLimit );
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iGuideView->UpdateRouteDataL(ri.distToGoal, ri.timeToGoal);
#endif
}

void CWayFinderAppUi::HandleRouteList( RouteListMess* aMessage )
{
   if( iTabGroup->ActiveTabId() == KItineraryViewId.iUid )
      iItineraryView->SetRouteList( aMessage->getRouteList() );
   else if( iTabGroup->ActiveTabId() == KGuideViewId.iUid )
      iGuideView->SetRouteList( aMessage->getRouteList() );
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
   push( KMapViewId );
}

void
CWayFinderAppUi::handleSetLatestNewsImage(const uint8* data, int32 numEntries)
{
   /* Save image to file. */
   TBuf<256> aFilename;
   aFilename.Copy(iPathManager->GetWritableDataPath());
#if defined NAV2_CLIENT_SERIES60_V3
   aFilename.Append( KNewsSvgFileName );
#else
   aFilename.Append( KNewsPictureFileName );
#endif
   WFSymbianUtil::writeBytesToFile(m_fsSession, aFilename,
         data, numEntries, iLog);
}

void
CWayFinderAppUi::handleSetUseSpeaker(bool on)
{
   iSettingsView->SetUseSpeaker( on );
   updateMute();
}

void
CWayFinderAppUi::handleSetVolume(int32 volume)
{
   iSettingsView->SetVolume( volume );
   if (iAudioSlave) {
      iAudioSlave->SetVolume( volume );
   }
}
void
CWayFinderAppUi::handleSetCategoryIds(
      const char** data, int32 numEntries)
{
   /* Get category id's and insert into dataholder. */
   //iNewDestView->HandleDisableCategory();
   iDataHolder->SetCategoryIds(numEntries, data);
}
void
CWayFinderAppUi::handleSetCategoryNames(
      const char** data, int32 numEntries)
{
   //iNewDestView->HandleDisableCategory();
   iDataHolder->SetCategoryStrings(numEntries, data);
}
void
CWayFinderAppUi::handleSetCategoryIconNames(
      const char** /*data*/, int32 /*numEntries*/)
{   
}
void
CWayFinderAppUi::handleSetCategoryIntIds(
      const int32* /*data*/, int32 /*numEntries*/)
{   
}
void
CWayFinderAppUi::handleSetBtGpsAddressAndName(
      const char** data, int32 numEntries)
{
   iSettingsView->SetBtGpsAddressL(data, numEntries);
#if !defined NOKIALBS
   if (!iBtHandler) {
# ifdef BT_AUTO_CONNECT_DELAY
      //delay connection by BT_AUTO_CONNECT_DELAY if symbian 7s.
      iEventGenerator->SendEventL(EWayfinderEventConnectToGps, 
            IfSymbian7s(BT_AUTO_CONNECT_DELAY, 0));
# else
      if (!iGotParamterConnectToGps) {
         iEventGenerator->SendEventL(EWayfinderEventConnectToGps);
         iGotParamterConnectToGps = ETrue;
      }
# endif
   }
#endif 
   if (iBtHandler) { //XXX will we need this? done in creategpsl
      iBtHandler->SetAddress(data, numEntries);
   }
}

void
CWayFinderAppUi::handleSetServerNameAndPort(HBufC* serverString)
{
#ifndef HTTP_PROTO
   iSettingsView->SetServer( *serverString );
   delete serverString;
#endif
}
void
CWayFinderAppUi::handleSetHttpServerNameAndPort(HBufC* serverString)
{
#ifdef HTTP_PROTO
   iSettingsView->SetServer( *serverString );
   delete serverString;
#endif
}

void
CWayFinderAppUi::handleSetMapLayerSettings(const uint8* data, int32 numEntries)
{
   if (!data) {
      /* Unset */
      iMapView->ConvertMapLayerSettingsToNav2Parameter(NULL, GetSettingsData());
   } else {
      iMapView->ConvertMapLayerSettingsToLayerInfo(
            const_cast<uint8*>(data), numEntries, GetSettingsData());
   }
}

void
CWayFinderAppUi::handleSetPoiCategories(const uint8* data, int32 numEntries)
{
   if (!data) {
      iMapView->ConvertCategoriesSettingsToTileCategories(NULL,
            GetSettingsData());
   } else {
      /* Poi category list. */
      Buffer *buf = new Buffer(const_cast<uint8*>(data),
            numEntries, numEntries);
      CleanupStack::PushL(buf);

      iMapView->ConvertCategoriesSettingsToTileCategories(
            buf,
            GetSettingsData());

      /* Release data, it's not the buffers anyways. */
      buf->releaseData();
      CleanupStack::PopAndDestroy(buf);
   }
}

void
CWayFinderAppUi::handleSetTrackingPIN(const uint8* data, int32 numEntries)
{
   // Make TrackPINList 
   TrackPINList* l = new(ELeave) TrackPINList( data, numEntries );

   iSettingsView->setTrackPINList( l );
}

void
CWayFinderAppUi::handleSetUsernameAndPassword()
{
   const char *user = iDataStore->iWFAccountData->getUsername();
   const char *pwd = iDataStore->iWFAccountData->getPassword();
   if (!user || !pwd) {
      return;
   }
   HBufC* tmp = WFTextUtil::AllocLC(user);
   iSettingsView->SetUsername(*tmp);
   CleanupStack::PopAndDestroy(tmp);
   tmp = WFTextUtil::AllocLC(pwd);
   iSettingsView->SetPassword(*tmp);
   CleanupStack::PopAndDestroy(tmp);

   // Create the lms id for this wayfinder user.
   if (iLmsId) {
      delete iLmsId;
   }
   if (iLmsIdChar) {
      delete[] iLmsIdChar;
   }
   _LIT(KWayfinder, "file://c:wayfinder_");
   _LIT(KLmsExt, ".LDB");
   HBufC* tmpName = WFTextUtil::AllocLC(user);
   iLmsId = AllocLC(KWayfinder + *tmpName + KLmsExt);
   iLmsIdChar = WFTextUtil::newTDesDupL(*iLmsId);
   CleanupStack::Pop(iLmsId);
   CleanupStack::PopAndDestroy(tmpName);

   if ( iMapView ) {
      // Update the MapView so that it can create the pre-cache.
      // (The username is needed there)
      iMapView->tryToAddVectorPrecache();
   }
}

void
CWayFinderAppUi::handleSetVectorMapCoordinates(isab::VectorMapCoordinates* vmc)
{
   if (vmc->Valid()) {
      /* Set coordinates in Mapview. */
      iMapView->SetVectorMapCoordinates(
            vmc->GetScale(),
            vmc->GetLat(),
            vmc->GetLon());
   }
   delete vmc;
}
void
CWayFinderAppUi::handleSetVectorMapSettings(const int32* data, int32 numEntries)
{
   /* Got settings for vector maps. */
   if (data[vmap_set_version_pos] != VECTOR_MAP_SETTINGS_VERSION) {
      /* Failed! Not the same version. */
      return;
   }
   /* Decrease by one to allow comparison with positions. */
   int32 num_data = numEntries - 1;
   if (num_data >= vmap_set_cache_pos) {
      int32 cacheSize = data[vmap_set_cache_pos];
      iSettingsView->SetMapCacheSize(cacheSize);
      iMapView->SetMapCacheSize( cacheSize );
   }
   if (num_data >= vmap_set_guide_mode_pos) {
      iSettingsView->SetPreferredGuideMode(
            (preferred_guide_mode)data[vmap_set_guide_mode_pos]);
   }
}

void
CWayFinderAppUi::handleSetSelectedAccessPointId(int32 iap)
{
   SetIAP(iap);
}
void
CWayFinderAppUi::handleSetSelectedAccessPointId2(int32 iap)
{
   if ( iIAPSearcher ) {
      iIAPSearcher->iapid2Set( iap );
   }
}

void
CWayFinderAppUi::handleSetTopRegionList(isab::TopRegionList* topRegionList)
{
   //TUint selectedCountryId = iDataHolder->GetSelectedCountryId();
   iDataHolder->InitCountries();
   TBuf<KBuf32Length> buf;
   TUint32 id = MAX_UINT32;
   /*iCoeEnv->ReadResource( buf, R_WAYFINDER_FROM_GPS_TEXT );
   iDataHolder->AddCountryL( buf, id );*/
#if defined SHOW_US_POPUP_WARNING
   TBool popUpWarning = EFalse;
#endif
   for( TInt i=0; i < topRegionList->size(); i++ ){
      buf.Zero();
      const TopRegion* region = topRegionList->getRegion(i);
      WFTextUtil::char2TDes( buf, region->getName() );
      id = region->getId();
#if defined SHOW_US_POPUP_WARNING
      popUpWarning = TopRegionHack::TopRegionPopupWarning(popUpWarning, id);
#endif
      iDataHolder->AddCountryL( buf, id );
   }
#if !defined USE_WF_ID
# if defined SHOW_US_POPUP_WARNING
   if (popUpWarning) {
      m_startupHandler->StartupEvent(SS_USDisclaimerNeeded);
   } else {
      m_startupHandler->StartupEvent(SS_USDisclaimerNotNeeded);
   }
# else
   m_startupHandler->StartupEvent(SS_USDisclaimerNotNeeded);
# endif
#endif
   //iNewDestView->ReinitCountriesL( selectedCountryId );
   delete topRegionList;
}

void 
CWayFinderAppUi::handleSetACPSetting(int32 enableACP)
{
   GetSettingsData()->m_acpSettingEnabled = enableACP;   
   SaveACPSettings(enableACP);
}

void 
CWayFinderAppUi::handleCheckForUpdatesSetting(int32 aValue)
{
   GetSettingsData()->iCheckForUpdates = aValue;   
}

void CWayFinderAppUi::setNewVersionNumber(const char* version)
{
   if (iNewVersionData.iVersionNumber) {
      delete iNewVersionData.iVersionNumber;
      iNewVersionData.iVersionNumber = NULL;
   }
   iNewVersionData.iVersionNumber = WFTextUtil::AllocL(version);

   if(iTabGroup->ActiveTabId() == KMainMenuViewId.iUid ){
      ShowUpdateToNewVersionDialog();
   }
}

void CWayFinderAppUi::setNewVersionUrl(const char* versionUrl)
{
   if (iNewVersionData.iVersionUrl) {
      delete iNewVersionData.iVersionUrl;
      iNewVersionData.iVersionUrl = NULL;
   }
   iNewVersionData.iVersionUrl = WFTextUtil::AllocL(versionUrl);

   if(iTabGroup->ActiveTabId() == KMainMenuViewId.iUid ){
      ShowUpdateToNewVersionDialog();
   }
}

bool CWayFinderAppUi::isWFIDStartup()
{
#if defined USE_WF_ID
   return true;
#else
   return false;
#endif
}

_LIT(KSemiColon, ";");
void
CWayFinderAppUi::handleSendServerNameAndPort(const TDesC& serverString)
{
   int32 paramId = isab::GuiProtEnums::paramServerNameAndPort;
#ifdef HTTP_PROTO
   paramId = isab::GuiProtEnums::paramHttpServerNameAndPort;
#endif
   char *serverPortStr = WFTextUtil::newTDesDupL(serverString);
   /* Will take ownership of serverPortStr. */
   m_generalParamReceiver->sendServerString(paramId, serverPortStr);
}

TBool CWayFinderAppUi::IsInTabOrder( TInt32 aUid )
{
   TBool isInTabOrder = ETrue;

   if (HasRoute()) {
      if (aUid == KContactsViewId.iUid ||
          aUid == KMapViewId.iUid ||
          aUid == KMyDestViewId.iUid ||
          aUid == KNewDestViewId.iUid ||
          aUid == KPositionSelectViewId.iUid ||
          aUid == KSettingsViewId.iUid ||
          aUid == KMainMenuViewId.iUid ||
          aUid == KWelcomeViewId.iUid) {
//          CEikonEnv::Static()->InfoWinL(_L("Tab not in order"), _L(""));
         isInTabOrder = EFalse;
      }
   } else {
      if (aUid == KContactsViewId.iUid ||
          aUid == KGuideViewId.iUid ||
          aUid == KItineraryViewId.iUid ||
          aUid == KMapViewId.iUid ||
          aUid == KMyDestViewId.iUid ||
          aUid == KNewDestViewId.iUid ||
          aUid == KPositionSelectViewId.iUid ||
          aUid == KSettingsViewId.iUid ||
          aUid == KMainMenuViewId.iUid ||
          aUid == KWelcomeViewId.iUid) {
         isInTabOrder = EFalse;
      }
   }
   if (IsNightMode() ){
      if (aUid == KInfoViewId.iUid ||
          aUid == KItineraryViewId.iUid) {
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
#if defined (SHOW_CALL_SUPPORT_IN_EXTRAS_MENU)
      aMenuPane->SetItemDimmed(EWayFinderCmdCallSupport, EFalse);
#else
      aMenuPane->SetItemDimmed(EWayFinderCmdCallSupport, ETrue);
#endif
      /*
      TBool showBuyExtension = EFalse;
#if defined(ALWAYS_SHOW_BUYEXTENSION)
      showBuyExtension = ETrue;
#endif
      aMenuPane->SetItemDimmed(EWayFinderCmdServicesBuyExtensionWapLink, !showBuyExtension );
      */
#if defined ENABLE_DEBUG_OPTIONS
      aMenuPane->SetItemDimmed(EWayFinderCmdShowNetworkInfo, EFalse);
      aMenuPane->SetItemDimmed(EWayFinderCmdGotoDebugUrl, EFalse);
#else
      aMenuPane->SetItemDimmed(EWayFinderCmdShowNetworkInfo, ETrue);
      aMenuPane->SetItemDimmed(EWayFinderCmdGotoDebugUrl, ETrue);
#endif
   } else if( aResourceId == R_WAYFINDER_APP_MENU ){
#if defined ENABLE_DEBUG_OPTIONS
      aMenuPane->SetItemDimmed(EWayFinderCmdGPSConnect, IsGpsConnected());
      aMenuPane->SetItemDimmed(EWayFinderCmdGPSDisConnect, !IsGpsConnected());
#else 
      aMenuPane->SetItemDimmed(EWayFinderCmdGPSConnect, ETrue);
      aMenuPane->SetItemDimmed(EWayFinderCmdGPSDisConnect, ETrue);
#endif
      if( iTabGroup->ActiveTabId() != KMainMenuViewId.iUid ){
         aMenuPane->SetItemDimmed( EAknSoftkeyExit, ETrue);
      }
      if (IsIronVersion()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdFreeDownloads, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdFreeDownloads, ETrue );
      }
      if (IsIronVersion()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdExtraServices, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdExtraServices, ETrue );
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
   } else if ( aResourceId == R_WAYFINDER_HELP_MENU ) {
   } else if ( aResourceId == R_AVKON_FORM_MENUPANE ) {
      // Else we don't wan't Fleet Info, at least.
/*       aMenuPane->SetItemDimmed( EWayFinderCmdFleetInfo, ETrue ); */
   } else if ( aResourceId == R_WAYFINDER_COMMUNITY_MENU) {
      aMenuPane->SetItemDimmed( EWayFinderPublishMyPosition, 
                                !showTracking() || iDataStore->iSettingsData->m_trackingLevel != 0);
      aMenuPane->SetItemDimmed( EWayFinderStopPublishMyPosition,
                                !showTracking() || iDataStore->iSettingsData->m_trackingLevel == 0);
#if defined(SHOW_SHARE_WF_SMS)
      aMenuPane->SetItemDimmed(EWayFinderCmdServicesSpreadWayfinder, EFalse);
#else
      aMenuPane->SetItemDimmed(EWayFinderCmdServicesSpreadWayfinder, ETrue);
#endif
   }
}

// void CWayFinderAppUi::HandleScreenDeviceChangedL()
// {
// }

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
   if ( iTabGroup == NULL ) {
      return EKeyWasNotConsumed;
   }

   if ( m_viewStack->currentGroup() == m_viewStack->invUid() ) {
      return EKeyWasNotConsumed;
   }

   if ( iUrgentShutdown ) {
      HandleCommandL( EWayFinderExitAppl );
   }

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

      CViewBase* currentView = static_cast<CViewBase*>(View(m_viewStack->current()));
      if(currentView->IsActivated()) {
         goLeftInGroup();  
      }
      }
      return EKeyWasConsumed;
      break;
   case EKeyRightArrow:
      {
      CViewBase* currentView = static_cast<CViewBase*>(View(m_viewStack->current()));
      if(currentView->IsActivated()) {
         goRightInGroup();
      }
      }
      return EKeyWasConsumed;
      break;
   }

   if (aType != EEventKeyUp) {
      return EKeyWasNotConsumed;
   }
   if (aKeyEvent.iScanCode == KEY_STAR) {
//      TRACE_DBG("Star!!! iCode: %ld iScanCode: %ld iModifiers: %ld iRepeats: %ld", 
//                (long int) aKeyEvent.iCode, (long int) aKeyEvent.iScanCode, 
//                (long int) aKeyEvent.iModifiers, (long int ) aKeyEvent.iRepeats);
      HandleCommandL( EWayfinderCmdAsterisk );
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
         goLeftInGroup();
      }
      break;
   case EStdKeyRightArrow:
      {
         goRightInGroup();
      }
      break;
   case KEY_STAR:
//      TRACE_DBG("Star!!! iCode: %ld iScanCode: %ld iModifiers: %ld iRepeats: %ld", 
//                (long int) aKeyEvent.iCode, (long int) aKeyEvent.iScanCode, (long int) aKeyEvent.iModifiers, 
//                (long int ) aKeyEvent.iRepeats);
      HandleCommandL( EWayfinderCmdAsterisk );
      break;
   }

   return EKeyWasConsumed;

#endif // End else not s60v3
}

bool
CWayFinderAppUi::getShouldMute() const
{
   if ( m_phoneCallInProgress ) {
      // Always mute when phone is ringing.
      return true;
   }
   if ( GetSettingsData() == NULL ) {
      // Assume no mute if we have no settings.
      return false;
   }

   if ( ! GetSettingsData()->iMuteWhenInBackground ) {
      return ! GetSettingsData()->iUseSpeaker;
   }
   
   bool keyLock = false;
   if ( ! iForeGround ) {
      // Don't mute if it's keylock.
      RAknKeyLock aAknKeylock ;
      aAknKeylock.Connect () ;
      keyLock = aAknKeylock.IsKeyLockEnabled();
      aAknKeylock.Close ();
   }
   
   // Mute if in background and it is forbidden to play in background
   // Also mute if forbidden to use speaker.
   return ( GetSettingsData()->iMuteWhenInBackground && !iForeGround && !keyLock) 
      || (! GetSettingsData()->iUseSpeaker );
}

void
CWayFinderAppUi::updateMute()
{
   if ( iAudioSlave == NULL ) {
      return;
   }
   bool shouldMute = getShouldMute();
   // Avoid setting needlessly
   if ( shouldMute && !iAudioSlave->IsMute() ) {
      iAudioSlave->SetMute( true );
   } else if ( (!shouldMute) && iAudioSlave->IsMute() ) {
      iAudioSlave->SetMute( false );
   }
}

void
CWayFinderAppUi::HandleForegroundEventL(TBool aForeground)
{
   // Base class call to HandleForegroundEventL.
   CAknAppUi::HandleForegroundEventL(aForeground);
   // We need to save it because it needs to be updated here and used later.
   TInt oldForeGroundValue = iForeGround;
   iForeGround = aForeground;
   updateMute();
   //TRACE_DBG("HandleForegroundEventL, aForeGround = %d", aForeground);

   if (aForeground && (oldForeGroundValue != aForeground)) {
      // XXX Actually we dont want to push anything here w/o reason, correct 
      // view should already be displayed by the framework so we do NOT 
      // need to manually activate any views just because we get into 
      // foreground again. By avoiding any view activations here we get 
      // rid of problems with dialogs disappearing, service window reloading, 
      // and wayfinder jumping back into foreground when switching between 
      // open applications with the task manager. XXX
      // And if current view is service window view we dont push it again, 
      // since that will give an unwanted reload of the currently 
      // displayed html page in the service window view.
      // And that reload causes s60v5 phones to crash if the virtual 
      // keyboard is active, since it will write in a newly deleted edwin. XXX

      TVwsViewId viewId;
      TInt ret = GetActiveViewId(viewId);
      if (ret == KErrNone && 
          m_viewStack->current().iUid != viewId.iViewUid.iUid) {
         // This should hopefully not happen but we have it as a security 
         // so we get into the correct view.
         // The currently active view according to the framework does not 
         // match the view that we last showed according to the view stack 
         // so push the view stack view again to show the correct view.
         push(m_viewStack->current(), EKeepOldStateMessage);
      }
   }
}

void CWayFinderAppUi::HandleProgressIndicator(GenericGuiMess* message)
{
   TInt ret;
   TVwsViewId viewId;
   switch(GuiProtEnums::ServerActionType(message->getSecondUint8())){
   case GuiProtEnums::DownloadingMap:
      SetWait(message->getFirstUint8(), R_WAYFINDER_CREATING_MAP_MSG);
      break;
   case GuiProtEnums::CreatingRoute:
      ret = GetActiveViewId(viewId);
      if (ret == KErrNone && m_viewStack->current().iUid == KMapViewId.iUid)
      {
      iMapView->SetInfoText(R_WAYFINDER_ROUTING_MSG, ETrue);
      }
      else
      {
      // Used to update the navi pane
      SetWait(message->getFirstUint8(), R_WAYFINDER_ROUTING_MSG);
      }
      StopWaitDlg();
      //StartRouteWaitDlgL(this);
      iWaitingForRoute = ETrue;
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
      // We cannot distinguish a search request progress update from other
      // updates (we get InvalidActionType while searching) so we check
      // if the search progress dialog is running, and if so do not show
      // the Processing text in the navi pane.
      if(iSearchWaitDlg && !iSearchWaitDlg->IsRunning()) {
         TVwsViewId viewId;
         TInt ret = GetActiveViewId(viewId);
         
         if((ret == KErrNone) && 
            (viewId.iViewUid.iUid != KCSCategoryResultViewId.iUid)) {
            // After receiving search results, the Processing text may flicker
            // in the category result view, most likely due to round 1 updates
            // being received, so we block it from being displayed here.
            SetWait(message->getFirstUint8(), R_WAYFINDER_PROCESSING_MSG);	
         }
      }
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

   // It seems like sometimes this exits the program
   LOGDEL(m_nav2);
   delete m_nav2;
   m_nav2 = NULL;
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

   Exit();
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
   }
   if(iMessageHandler){
      iMessageHandler->Release();
   }

   LOGDEL(iMessageHandler);
   delete iMessageHandler;
   iMessageHandler = NULL;

   killNav2();

}


// ----------------------------------------------------
// CWayFinderAppUi::HandleCommandL(TInt aCommand)
// ?implementation_description
// ----------------------------------------------------
//
void CWayFinderAppUi::HandleCommandL(TInt aCommand)
{
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
   case EWayFinderCmdCallSupport:
      {
#if defined (SHOW_CALL_SUPPORT_IN_EXTRAS_MENU)
         TBuf<128> tmp;
         WFTextUtil::char2TDes(tmp, CALL_SUPPORT_NUMBER);
         DialNumberL(tmp, EWayFinderCmdStartpageCall);
#endif
         break;
      }
   case EAknCmdExit:
   case EAknSoftkeyExit:
      {
         iIniFile->Write();
         HBufC* text  = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_CONFIRM_EXIT_MSG );

         iExitDlgActive = ETrue;
         if(!iForeGround || CExitQueryDlg::RunDlgLD(*text)) {
            // Exit the program
            CleanupStack::PopAndDestroy(text);
            doExit();
         } else {
            iExitDlgActive = EFalse;
            iCurrentView->ShowMenuBar(ETrue);
            CleanupStack::PopAndDestroy(text);
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
      {
         pop();
         break;
      }
   case EWayFinderSoftkeyStart:
      GotoStartViewL();
      break;
   case EWayFinderCmdStartPageNews:
      {
         // Not available in rss any more but functionality kept.
         m_startupHandler->StartupEvent(SS_ShowNews);
         break;
      }
   case EWayFinderCmdPositionSelect:
      {
         push( KPositionSelectViewId );
         break;
      }
   case EWayFinderCmdStartPageNewSearch:
      {
         push( KNewDestViewId, ENewSearch );
         break;
      }
   case EWayFinderCmdSettingNewSearch:
      {
         push( KNewDestViewId, ENoMessage );
         break;
      }
   case EWayFinderCmdNewDest:
      {
         push( KNewDestViewId, ENewSearch );
         break;
      }
   case EWayFinderCmdNewDest2:
      {
         push( KNewDestViewId );
         break;
      }
   case EWayFinderCmdMyDest:
      {
         push( KMyDestViewId );
         break;
      }
   case EWayFinderCmdGuide:
      {
         if (HasRoute()) {
            push( KGuideViewId );
         } else {
            push( KPositionSelectViewId );
         }
         break;
      }
   case EWayFinderCmdItinerary:
      {
         push( KItineraryViewId );
         break;
      }
   case EWayFinderCmdStartpage:
      {
         push( KStartPageViewId );
         break;
      }
   case EWayFinderCmdMap:
      {
         if( HasRoute() && !IsIronVersion() ) {
            push( KMapViewId, EMapAsNavigation );
         } else {
            push( KMapViewId, EMapAsOverview );
         } 
         break;
      }
   case EWayFinderCmdInfo:
      {
         push( KInfoViewId );
         break;
      }
   case EWayFinderCmdNewDestInfo:
      {
         push( KInfoViewId, EFromNewDest );
         break;
      }
   case EWayFinderCmdConnectInfo:
      {
         push( KInfoViewId, EFromConnect );
         break;
      }
   case EWayFinderCmdCSMainView:
      {
         push( KCSMainViewId, ENoMessage );
         break;
      }
   case EWayFinderCmdCSCategoryResultView:
      {
         push( KCSCategoryResultViewId );
         break;
      }
   case EWayFinderCmdCSDetailedResultView:
      {
         if (iCSCategoryResultView->GetCurrentIndex() >= 0) {
            iCSDetailedResultView->ResetIndexes();
            push( KCSDetailedResultViewId, iCSCategoryResultView->GetViewState() );
         }
         //SetDetailedSearchResults();
         break;
      }
   case EWayFinderCmdCSSwitchToCategorySelect:
   case EWayFinderCmdCSSwitchToCountrySelect:
      {
         push( KSearchableLisboxViewId );
         break;
      }
   case EWayFinderCmdCSNewSearch:
      {
         //push( KCSCategoryResultViewId );
         StartSearchWaitDlgL(this);
         iCSDispatcher->combinedSearchStart(iCSMainView->GetSearchRecord());
         break;
      }
   case EWayFinderCmdSettings:
      {
         SwitchToSettingsL();
         break;
      }
   case EWayFinderMyAccountView:
      {
         /* Switch to "My Account" view. */
         iMyAccountView->SetUserName( iDataStore->iWFAccountData->getWebUserName() );
         push( KMyAccountViewId );
         break;
      }
   case EWayFinderCmdAbout:
      {
         ShowAboutL();
         break;
      }
   case EWayFinderCmdHelp:
      {
         ShowHelpL();
         break;
      }
   case EWayFinderCmdInfoViewGPSConnect:
      {
         //XXX connects to Nav2 via TCP socket,
         //for development reasons.
         if (
#ifdef __WINS__
            1 ||
#endif
            IsGpsConnectedAndNotSimulating()) {
            GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
            push( KInfoViewId );
         } else {
#ifdef GUI_ONLY
            if(iMessageHandler){
               iMessageHandler->ConnectL(GUIMACHINE, GUIPORT );
            }
#else
            ConnectToGpsL();
#endif
         }
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
   case EWayFinderCmdFreeDownloads:
      {
         LaunchWapLinkL(KWayfinderServiceWindowFreeDownloadsURL);
         break;
      }
   case EWayFinderCmdService:
   case EWayFinderCmdExtraServices:
      {
         GotoServiceViewL(KWayfinderServiceWindowBaseURL);
         break;
      }
   case EWayFinderCmdServiceWeather:
      {
         // Used to show weather in browser
         break;
      }
   case EWayFinderCmdServiceCityGuide:
      {
         // Used to show city guide in browser
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
   case EWayFinderCmdStopNavigation:
      {
         ClearRouteL();
         break;
      }
   case EWayFinderCmdClear:
      {
         if (WFDialog::ShowQueryL( R_WAYFINDER_CONFIRM_CLEAR_ROUTE_MSG, iCoeEnv ) ) {
            ClearRouteL();
         }
         break;
      }
   case EWayFinderCmdStartNavigation:
   case EWayFinderCmdMapReroute:
   case EWayFinderCmdGuideReroute:
   case EWayFinderCmdItineraryReroute:
   case EWayFinderCmdDestinationReroute:
   case EWayFinderCmdReroute:
   case EWayFinderCmdPositionSelectReroute:
      {
         iManualRerouteIssued = ETrue;
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
   case EWayFinderCmdStartpageCall:
      iPhoneCallFromWf = EWayFinderCmdStartpage;
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
      if (GetSettingsData()->iPreferredGuideMode == preferred_guide) {
         HandleCommandL(EWayFinderCmdGuide);
      } else {
         HandleCommandL(EWayFinderCmdMap);
      }
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
      break;
   case EWayFinderCmdNightModeOn:
   case EWayFinderCmdNightModeOff:
      iNightMode = !iNightMode;
      SetNightModeL(iNightMode);
      break;
   case EWayFinderCmdFindSms:
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
      break;
   case EWayFinderCmdStartPageUpgradeInfo:
   case EWayFinderCmdStartPageUpgradeInfo2:
      {
         ShowUpgradeInfoL( CWayFinderAppUi::EUpgrade );
         break;
      }
   case EWayFinderCmdShowNetworkInfo:
      FetchNetworkInfoL(EUseForGettingPosition);
      break;
   case EWayFinderCmdGotoDebugUrl:
      break;
   default:
      break;
   }
}

_LIT(KSWSmsPhone, "phone");
_LIT(KSWSmsTxt, "smstext");

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
   push( KWelcomeViewId, EShutdown );
}

CAknNavigationControlContainer *
CWayFinderAppUi::getNavigationControlContainer() const
{
   return iNaviPane;
}

void
CWayFinderAppUi::TurnBackLightOnL()
{
   if (!iForeGround && 
       GetSettingsData()->m_backlightStrategy != 
       GuiProtEnums::backlight_near_action) {
      // If we're not in foreground we don't want to keep backlight on.
      // But if we should let the screensaver start and turn on backlight 
      // when reaching a turn (backlight_near_action) we can not return here 
      // since screensaver puts us to background.
      return;
   }
   if (TileMapUtil::currentTimeMillis() -
       iLastTimeBacklightTrigger  < 5*1000) {
      // Not long enough since last shown error message in the map view.
      return;
   }
   // Remember last time we reset the timers.
   iLastTimeBacklightTrigger = TileMapUtil::currentTimeMillis();

   if (GetSettingsData()->m_backlightStrategy == 
       GuiProtEnums::backlight_near_action) {
      // If the keylock has gone on we need to turn it off if we should 
      // be able to get the backlight on when getting near to a turn.
      RAknKeyLock keyLock;
      TInt ret = keyLock.Connect();
      if (ret == KErrNone) {
         keyLock.DisableWithoutNote();
         keyLock.Close();
      }
   }
   // Reset the inactivity timers to keep backlight on.
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
   char *tmp = WFTextUtil::strdupL(iImeiNbr);
   iDataStore->iWFAccountData->setImei(tmp);

   iImeiCrcHex = new char[9]; // Eight chars and room for ending zero
   sprintf(iImeiCrcHex, "%08x", 
           crc32::calcCrc32((uint8*)iImeiNbr, strlen(iImeiNbr)));

   iImeiReceived = ETrue;
  

   // Notify that the imei has been fetched.
   ImeiFetched();
}

void
CWayFinderAppUi::ImeiFetched()
{
   // Need to fetch imsi after the imei.
   FetchImsi();
}

void
CWayFinderAppUi::ImsiNumberSet(const TDesC& aImsiNumber)
{
   if (iImsiNbr) {
      delete iImsiNbr;
      iImsiNbr = NULL;
   }
   iImsiNbr = WFTextUtil::newTDesDupL(aImsiNumber);
   iImsiReceived = ETrue;
  
   ImsiFetched();
}

void
CWayFinderAppUi::ImsiFetched()
{
   // Initiate second stage startup.
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
CWayFinderAppUi::SmsSent(TInt aStatus, 
                         TInt aCount, 
                         TInt aNbrSent)
{
   aCount = aCount;
   aNbrSent = aNbrSent;

   // TODO: Service window removed, show dialog that sms has been sent   
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
         // Recalculate the size of the context pane image and redraw it
         iContextPaneAnimator->RecalcContextAnimSizes();
         SetWait( EFalse, 0 );
#endif
      }
//       TVwsViewId currViewId;
//       GetActiveViewId(currViewId);
// //       TRACE_DBG("Current view id: %d", currViewId.iViewUid.iUid);
//       if (currViewId.iViewUid.iUid == KMainMenuViewId.iUid) {
//          iMainMenuView->ReInitContainer();
//       }
#ifdef NAV2_CLIENT_SERIES60_V5
      // Explicitly hide the arrows in navipane since on s60v5 they tend to
      // show up in some views otherwise.
      setNaviPane(EFalse);
#endif
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

void CWayFinderAppUi::UpdateCurrentView(TUid aToBe) 
{
   if (aToBe.iUid ==  KSettingsViewId.iUid) {
      iCurrentView = iSettingsView;
   } else if (aToBe.iUid ==  KMyAccountViewId.iUid) {
      iCurrentView = iMyAccountView;
   } else if (aToBe.iUid ==  KMyDestViewId.iUid) {
      iCurrentView = iMyDestView;
   } else if (aToBe.iUid ==  KPositionSelectViewId.iUid) {
      iCurrentView = iSelectView;
   } else if (aToBe.iUid ==  KWelcomeViewId.iUid) {
      iCurrentView = iWelcomeView;
   } else if (aToBe.iUid ==  KInfoViewId.iUid) {
      iCurrentView = iInfoView;
   } else if (aToBe.iUid == KMapViewId.iUid ){
      iCurrentView = iMapView;
   } else if (aToBe.iUid == KGuideViewId.iUid) {
      iCurrentView = iGuideView;
   } else if (aToBe.iUid == KItineraryViewId.iUid) {
      iCurrentView = iItineraryView;
   } else if (aToBe.iUid == KMainMenuViewId.iUid) {
      iCurrentView = iMainMenuView;
   } else if (aToBe.iUid == KCSMainViewId.iUid) {
      iCurrentView = iCSMainView;
   } else if (aToBe.iUid == KCSCategoryResultViewId.iUid) {
      iCurrentView = iCSCategoryResultView;
   } else if (aToBe.iUid == KCSDetailedResultViewId.iUid) {
      iCurrentView = iCSDetailedResultView;
   } else if (aToBe.iUid == KSearchableLisboxViewId.iUid) {
      iCurrentView = iSearchableListBoxView;
   } else if (aToBe.iUid == KEditFavoriteViewId.iUid) {
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
      iCurrentView = iEditFavoriteView;
#else
      iCurrentView = iMainMenuView;
#endif
   } else {
      iCurrentView = iMainMenuView;
   }
}

void
CWayFinderAppUi::push( TUid v, TUid g ) {
   // Uncomment this for having a current view pointer.
   // it is now moved to SetViewL since here we didnt update on pop.
   //UpdateCurrentView( v ); 
   m_viewStack->push( v, g );
}

void 
CWayFinderAppUi::push( TUid v, TUid g, TInt messageId )
{
   m_viewStack->setNextMessagId( messageId );
   push( v, g );
}

void 
CWayFinderAppUi::push( TUid v, TUid g, TInt messageId, const TDesC8& aMessage )
{
   m_viewStack->setNextMessagId( messageId );
   m_viewStack->setNextMessage( aMessage );
   push( v, g );
}

void
CWayFinderAppUi::pop() {
   m_viewStack->pop();
}

TUid
CWayFinderAppUi::peek() {
   return m_viewStack->peek();
}

void 
CWayFinderAppUi::pop( TInt messageId )
{
   m_viewStack->setNextMessagId( messageId );
   pop();
}

void
CWayFinderAppUi::goLeftInGroup()
{
   TUid l = m_viewStack->prevViewInGroup();
   if ( l != m_viewStack->invVid() ) {
      push( l );
   }
}

void
CWayFinderAppUi::goRightInGroup()
{
   TUid r = m_viewStack->sucViewInGroup();
   if ( r != m_viewStack->invVid() ) {
      push( r );
   }
}

void 
CWayFinderAppUi::setNextMessagId( TInt messageId )
{
   m_viewStack->setNextMessagId( messageId );
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


void CWayFinderAppUi::HandleSmsL( TInt32 aLat,
                                  TInt32 aLon,
                                  const TDesC& aDescription,
                                  TInt32 aMsvId,
                                  Favorite* aFavorite )
{
   if(GetSettingsData()->m_autoRouteOnSMSDest == GuiProtEnums::yes){
      if( IsFullVersion() ){
         RouteToCoordinateL( aLat, aLon, aDescription );
      }
      else{
         iSelectView->SetDestination( isab::GuiProtEnums::PositionTypePosition,
                                      aDescription, NULL, aLat, aLon );
      }
   }
   if(GetSettingsData()->m_storeSMSDestInFavorites==GuiProtEnums::yes){
      if(iMyDestView){
         if( aFavorite ){
            iMyDestView->AddFavoriteD( aFavorite );
         }
         else{
            iMyDestView->AddFavorite( aLat, aLon, aDescription );
         }
      }
   }
   else if(GetSettingsData()->m_storeSMSDestInFavorites==GuiProtEnums::no){
   }
   else if(GetSettingsData()->m_storeSMSDestInFavorites==GuiProtEnums::ask){
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
   if(GetSettingsData()->m_keepSMSDestInInbox==GuiProtEnums::yes)
   {
   }
   else if(GetSettingsData()->m_keepSMSDestInInbox==GuiProtEnums::no){
#ifdef NAV2_CLIENT_SERIES60_V3
      iSmsHandler->DeleteSmsL(aMsvId);
#endif
   }
   else if(GetSettingsData()->m_keepSMSDestInInbox==GuiProtEnums::ask)
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
   if(GetSettingsData()->m_autoRouteOnSMSDest){
      RouteToSMSL( aOriginLat, aOriginLon, aDestinationLat,
                   aDestinationLon, aDestinationDescription );

   }
   if(GetSettingsData()->m_storeSMSDestInFavorites==GuiProtEnums::yes){
      if( iMyDestView ){
         iMyDestView->AddFavorite( aDestinationLat,
                                   aDestinationLon,
                                   aDestinationDescription );
      }
   }
   else if(GetSettingsData()->m_storeSMSDestInFavorites==GuiProtEnums::no){
   }
   else if(GetSettingsData()->m_storeSMSDestInFavorites==GuiProtEnums::ask){
      if( WFDialog::ShowQueryL( R_WAYFINDER_SAVESMSDESTINATION_MSG, iCoeEnv ) ){
         if(iMyDestView){
            iMyDestView->AddFavorite( aDestinationLat,
                                      aDestinationLon,
                                      aDestinationDescription );
         }
      }
   }
   if(GetSettingsData()->m_keepSMSDestInInbox==GuiProtEnums::yes){
   }
   else if(GetSettingsData()->m_keepSMSDestInInbox==GuiProtEnums::no){
#ifdef NAV2_CLIENT_SERIES60_V3
      iSmsHandler->DeleteSmsL(aMsvId);
#endif
   }
   else if(GetSettingsData()->m_keepSMSDestInInbox==GuiProtEnums::ask){
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

isab::GuiProtEnums::YesNoAsk
CWayFinderAppUi::SaveSmsDestination() const
{
   return (isab::GuiProtEnums::YesNoAsk)GetSettingsData()->m_storeSMSDestInFavorites;
}

isab::GuiProtEnums::YesNoAsk
CWayFinderAppUi::KeepSmsDestination() const
{
   return (isab::GuiProtEnums::YesNoAsk)GetSettingsData()->m_keepSMSDestInInbox;
}

CSettingsData*
CWayFinderAppUi::GetSettingsData() const
{
   return (CSettingsData*)(iDataStore->iSettingsData);
}

void
CWayFinderAppUi::SetNightModeL(TBool aNightMode) 
{
   iGuideView->SetNightModeL(aNightMode);
   iMapView->SetNightModeL(aNightMode);
}

void 
CWayFinderAppUi::GetForegroundColor(TInt& aR, TInt& aG, TInt& aB)
{
   if (iNightMode) {
      aR = KForegroundTextColorNightRed;
      aG = KForegroundTextColorNightGreen;
      aB = KForegroundTextColorNightBlue;
   } else {
      MAknsSkinInstance* skin = AknsUtils::SkinInstance();
      TRgb fgColor(0, 0, 0);
      AknsUtils::GetCachedColor(skin, fgColor, KAknsIIDQsnTextColors, 
                                EAknsCIQsnTextColorsCG6);
      aR = fgColor.Red();
      aG = fgColor.Green();
      aB = fgColor.Blue();
   }
}

void 
CWayFinderAppUi::GetForegroundColor(TRgb& aRgb)
{
   if (iNightMode) {
      aRgb.SetRed(KForegroundTextColorNightRed);
      aRgb.SetGreen(KForegroundTextColorNightGreen);
      aRgb.SetBlue(KForegroundTextColorNightBlue);
   } else {
      MAknsSkinInstance* skin = AknsUtils::SkinInstance();
      TRgb fgColor(0, 0, 0);
      AknsUtils::GetCachedColor(skin, fgColor, KAknsIIDQsnTextColors, 
                                EAknsCIQsnTextColorsCG6);
      aRgb.SetInternal(fgColor.Internal());
   }
}

void 
CWayFinderAppUi::GetBackgroundColor(TInt& aR, TInt& aG, TInt& aB)
{
   if (iNightMode) {
      aR = KBackgroundColorNightRed;
      aG = KBackgroundColorNightGreen;
      aB = KBackgroundColorNightBlue;
   } else {
      aR = KBackgroundRed;
      aG = KBackgroundGreen;
      aB = KBackgroundBlue;
   }
}

void 
CWayFinderAppUi::GetNightmodeBackgroundColor(TRgb& aRgb)
{
   aRgb.SetRed(KBackgroundColorNightRed);
   aRgb.SetGreen(KBackgroundColorNightGreen);
   aRgb.SetBlue(KBackgroundColorNightBlue);
   aRgb.SetAlpha(KBackgroundColorNightAlpha);
}

TBool 
CWayFinderAppUi::IsNightMode()
{
   return iNightMode;
}

void 
CWayFinderAppUi::HandleWsEventL(const TWsEvent &aEvent, 
                                class CCoeControl* aDestination)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   switch(aEvent.Type()) {
   case KAknUidValueEndKeyCloseEvent: 
      {
         class TApaTaskList taskList(CEikonEnv::Static()->WsSession());
         class TApaTask task = taskList.FindApp(KWayfinderUid);
         if(task.Exists()) { 
            // App is running, send it to background
            task.SendToBackground();
         } else {
            // Something went wrong, close the application since
            // this is what would normaly happen.
            HandleCommandL(EEikCmdExit);
         }
         return;
      }
//    case EEventCaseClosed:
//       {
//          class TApaTaskList taskList(CEikonEnv::Static()->WsSession());
//          class TApaTask task = taskList.FindApp(KWayfinderUid);
//          if(iForeGround && task.Exists()) { 
//             // App is running, send it to background
//             task.BringToForeground();
//          }
//       }
//    case EEventCaseOpened:
//       {
//          class TApaTaskList taskList(CEikonEnv::Static()->WsSession());
//          class TApaTask task = taskList.FindApp(KWayfinderUid);
//          if(iForeGround && task.Exists()) { 
//             // App is running, send it to background
//             task.BringToForeground();
//          }         
//       }
   case EEventScreenDeviceChanged:
      { 
         class TApaTaskList taskList(CEikonEnv::Static()->WsSession());
         class TApaTask task = taskList.FindApp(KWayfinderUid);
         if(iForeGround && task.Exists()) { 
            task.BringToForeground();
         }
      }
   } 
#endif
   CAknAppUi::HandleWsEventL(aEvent, aDestination);
}

void
CWayFinderAppUi::StartWaitDlg(class MWaitDialogObserver* aObserver, 
                              TInt  aTextResourceId)
{
   if(!iWaitDlg) {
      iWaitDlg = CWaitDlg::NewL(aObserver);
   }
   iWaitDlg->StartWaitDlg(R_MAP_VIEW_WAITDLG, aTextResourceId, 
                          EWayfinderDialogCancelId);
   // While the wait dialog is shown we want the display active.
   iKeepBacklightOn = ETrue;
}

void
CWayFinderAppUi::StopWaitDlg()
{
   if(iWaitDlg) {
      iWaitDlg->StopWaitDlg();
   }
   // If we stop the wait dialog we want the display to be able to turn off.
   iKeepBacklightOn = EFalse;
}

void
CWayFinderAppUi::StartRouteWaitDlgL(class MWaitDialogObserver* aObserver)
{
   TVwsViewId viewId;
   TInt ret = GetActiveViewId(viewId);
   if(ret == KErrNone) {
      if((viewId.iViewUid.iUid == KMapViewId.iUid) && 
         iMapView->MapAsGeneralNavigation()) {
         // Don't show the route progress dialog if we're in the map view
         // since the spinning icon already shows the route progress.
         return;
      }	else if((viewId.iViewUid.iUid == KGuideViewId.iUid) ||
                 (viewId.iViewUid.iUid == KItineraryViewId.iUid) ||
                 (viewId.iViewUid.iUid == KMainMenuViewId.iUid)) {
         // Also do not show the progress dialog if we get a new route/reroute
         // while we are in the guide or itinerary views since that would obstruct
         // the view contents. 
         // Make sure it does not show up in the main view, which could possibly happen.
         return;
      }
   }

   if(!iRouteWaitDlg) {
      iRouteWaitDlg = CWaitDlg::NewL(aObserver);
   }

   iRouteWaitDlg->StartWaitDlg(R_ROUTE_CALC_WAITDLG, R_WAYFINDER_ROUTING_MSG);
   // While the wait dialog is shown we want the display active.
   iKeepBacklightOn = ETrue;
}

void
CWayFinderAppUi::StopRouteWaitDlg()
{
//   if(iRouteWaitDlg) {
//      iRouteWaitDlg->StopWaitDlg();
//   }
   iMapView->SetInfoText(KNullDesC, EFalse);
   // If we stop the wait dialog we want the display to be able to turn off.
   iKeepBacklightOn = EFalse;
}

void
CWayFinderAppUi::StartSearchWaitDlgL(class MWaitDialogObserver* aObserver)
{
   if(!iSearchWaitDlg) {
      iSearchWaitDlg = CWaitDlg::NewL(aObserver);
   }

   iSearchWaitDlg->StartWaitDlg(R_SEARCH_WAITDLG, R_S60_SEARCHING_TXT);
   // While the wait dialog is shown we want the display active.
   iKeepBacklightOn = ETrue;
}

void
CWayFinderAppUi::StopSearchWaitDlg()
{
   if(iSearchWaitDlg) {
      iSearchWaitDlg->StopWaitDlg();
   }
   // If we stop the wait dialog we want the display to be able to turn off.
   iKeepBacklightOn = EFalse;
}

void CWayFinderAppUi::CancelPressed()
{
   // The user cancelled the route request during waiting for gps positions
   // so cancel the route request.
   GenericGuiMess cancelMess(GuiProtEnums::ROUTE_TO_POSITION_CANCEL);
   SendMessageL(cancelMess);
   iKeepBacklightOn = EFalse;
}

void CWayFinderAppUi::DialogDismissedFromFramework()
{
   // System killed the dialog (by WF going into background or something like
   // that) since the dialog is no longer we cancel the route request as well.
   GenericGuiMess cancelMess(GuiProtEnums::ROUTE_TO_POSITION_CANCEL);
   SendMessageL(cancelMess);
   iKeepBacklightOn = EFalse;
}

void
CWayFinderAppUi::GetCoordinate(TPoint& aRealCoord)
{
   iMapView->GetCoordinate(aRealCoord);
}

void
CWayFinderAppUi::HandleSystemEventL(const TWsEvent& aEvent)
{
   switch( *(TApaSystemEvent*)(aEvent.EventData()) )
      {
      case EApaSystemEventShutdown:
         {
            // We actually need to destroy the vectormap container when the 
            // framework shuts us down. This is since when the framework 
            // closes us the destructor in the AppUi is called before the 
            // currently active view gets de-activated. If we happen 
            // to be in map view it will actually crash the application 
            // instead of closing it nicely when the view gets de-activated.
            iMapView->DestroyVectorMapContainer();
            HandleCommandL(EEikCmdExit);
            break;
         }
      case EApaSystemEventBackupStarting:
      case EApaSystemEventBackupComplete:
      case EApaSystemEventBroughtToForeground:
#if defined(NAV2_CLIENT_SERIES60_V5) || defined(NAV2_CLIENT_SERIES60_V32)
      case EApaSystemEventSecureShutdown:
#endif
         break;
      }
   // Call the base class implementation
   CAknAppUi::HandleSystemEventL(aEvent);
}

TBool
CWayFinderAppUi::IsDialogDisplayed()
{
   return iExitDlgActive;
}

void CWayFinderAppUi::MrccatoCommand(TRemConCoreApiOperationId aOperationId, 
                                     TRemConCoreApiButtonAction aButtonAct)
{
   if (iTabGroup->ActiveTabId() == KSettingsViewId.iUid) {
      return;
   }
   switch(aOperationId) {
   case ERemConCoreApiVolumeDown:
      { 
         // User pressed volume down button, send message to decrease the
         // volume with one.
         int32 volume = GetSettingsData()->iVolume - 1;

         if (volume >= 1) {
            class GeneralParameterMess mess(GuiProtEnums::paramSoundVolume,
                                            (int32)volume*10);
            SendMessageL( &mess );
            
            DeactivateMainMenuNaviPaneLabelL();
            
            STATIC_CAST(CAknVolumeControl*,
                        iNaviVolumeControl->DecoratedControl())->SetValue(volume);
            iNaviPane->PushL(*iNaviVolumeControl);
            if (!iNaviVolumeTimer) {
               iNaviVolumeTimer = CPeriodic::NewL(CActive::EPriorityLow);
            } 
            if (iNaviVolumeTimer->IsActive()) {
               iNaviVolumeTimer->Cancel();
            }
            iNaviVolumeTimer->Start(2000000, 2000000, 
                                    TCallBack(NaviVolumeCallback, this));
         }         
         break;
      } 
   case ERemConCoreApiVolumeUp:
      {
         // User pressed volume down button, send message to increase the
         // volume with one.
         int32 volume = GetSettingsData()->iVolume + 1;
         if (volume <= 10) {
            class GeneralParameterMess mess(GuiProtEnums::paramSoundVolume,
                                            (int32)volume*10);
            SendMessageL( &mess );

            DeactivateMainMenuNaviPaneLabelL();

            STATIC_CAST(CAknVolumeControl*,
                        iNaviVolumeControl->DecoratedControl())->SetValue(volume);
            iNaviPane->PushL(*iNaviVolumeControl);

            if (!iNaviVolumeTimer) {
               iNaviVolumeTimer = CPeriodic::NewL(CActive::EPriorityLow);
            }
            if (iNaviVolumeTimer->IsActive()) {
               iNaviVolumeTimer->Cancel();
            }
            iNaviVolumeTimer->Start(2000000, 2000000, 
                                    TCallBack(NaviVolumeCallback, this));
         }
         break;
      }
   default:
      break;
   }
}

void
CWayFinderAppUi::handleSetSearchHistory(const uint8* data, int32 numEntries)
{
   CombinedSearchDataHolder& dataHolder = 
      iCSDispatcher->getCombinedSearchDataHolder();

   /* Search history. */
   Buffer *buf = new Buffer(const_cast<uint8*>(data), numEntries, numEntries);
   /* Get version. */
   int16 version = buf->readNext16bit();
   if (version == SEARCH_HISTORY_VERSION_NUMBER) {
      /* OK. */
      int16 numEntries = buf->readNext16bit();
      dataHolder.clearCombinedSearchHistory();
      for (int16 i = 0 ; i < numEntries ; i++) {
         // Read 6-tuples of searchstring, housenum, citystring, cityid, 
         // countrystring, countryid and categoryid.
         const char *ss = buf->getNextCharString();
         buf->getNextCharString(); //housenum not used anymore, just move position in buffer
         const char *cis = buf->getNextCharString();
         buf->getNextCharString(); //cityindex not used anymore, just move position in buffer
         const char *cos = buf->getNextCharString();
         const char *coi = buf->getNextCharString();
         const char *cai = buf->getNextCharString();
         // Add it to the dataholder.
         dataHolder.addSearchHistoryItem(ss, cis, cos, coi, cai);
      }
   } else {
      /* Unknown search history version. */
      /* The search history will be rewritten when the user */
      /* does his next search. */
   }
   buf->releaseData();
   delete buf;
}

void
CWayFinderAppUi::HandleSaveSearchHistoryL()
{
   CombinedSearchDataHolder& dataHolder = 
      iCSDispatcher->getCombinedSearchDataHolder();

   Buffer *buf = new Buffer(1280);

   uint32 numEntries = dataHolder.getNumSearchHistoryItems();
   buf->writeNext16bit(SEARCH_HISTORY_VERSION_NUMBER); /* Version. */
   buf->writeNext16bit(numEntries);
   for (uint32 i = numEntries; i > 0; i--) {
      SearchHistoryItem* shi = dataHolder.getSearchHistoryItem(i-1);
      if (shi) {
         buf->writeNextCharString(shi->GetSearchString());
         buf->writeNextCharString(shi->GetHouseNum());
         buf->writeNextCharString(shi->GetCityString());
         buf->writeNextCharString(shi->GetCityId());
         buf->writeNextCharString(shi->GetCountryString());
         buf->writeNextCharString(shi->GetCountryId());
         buf->writeNextCharString(shi->GetCategoryId());
      }
   }
   const uint8 *rawdata = buf->accessRawData();
   int32 len = buf->getLength();

   GeneralParameterMess* gpm = new (ELeave) GeneralParameterMess(
         GuiProtEnums::paramSearchStrings,
         (uint8*)rawdata, len);

   SendMessageL( gpm );
   /* Don't delete data, it's owned by the buffer. */

   delete gpm;
   delete buf;
}

void CWayFinderAppUi::SetupCSCountryListL()
{
   iSearchableListBoxView->SetupCachedCountryListL();
}

void CWayFinderAppUi::SetDefaultCountryIndex()
{
	iSearchableListBoxView->SetActiveCountryIndex(this->GetStoredCountryIndex());
}

void CWayFinderAppUi::SetupCSCategoryListL()
{
   iSearchableListBoxView->SetupCachedCategoryListL();
}

CombinedSearchDispatcher* CWayFinderAppUi::GetCSDispatcher()
{
   return iCSDispatcher;
}

const CombinedSearchDataHolder& CWayFinderAppUi::GetCombinedSearchDataHolder()
{
   return iCSDispatcher->getCombinedSearchDataHolder();
}

void CWayFinderAppUi::SearchCategoriesReceived()
{
   
}

void CWayFinderAppUi::RequestedImagesDownloaded()
{
   
}

void CWayFinderAppUi::SearchResultReceived()
{
   iCSCategoryResultView->AddCategories();
}

void CWayFinderAppUi::MoreSearchResultReceived()
{
   iCSDetailedResultView->AddResultsL(iCSCategoryResultView->GetCurrentIndex(), 
                                      iCSDispatcher->getCombinedSearchResults(), 
                                      ETrue);
}

void CWayFinderAppUi::AreaMatchSearchResultReceived()
{
   iCSCategoryResultView->AreaMatchSearchResultReceived(iCurrentView == iCSCategoryResultView);   
}

void CWayFinderAppUi::TotalNbrHitsReceived(const std::vector<CombinedSearchCategory*>& categories)
{
   if (iCurrentView != iCSCategoryResultView && iCSDispatcher->getCurrentRound() == 0) {
      // Dont push twice
      push( KCSCategoryResultViewId, iCSMainView->GetViewState() );
   }
   iCSCategoryResultView->AddCategories(categories);
}

void CWayFinderAppUi::SetDetailedSearchResults()
{
   iCSDetailedResultView->AddResultsL(iCSCategoryResultView->GetCurrentIndex(), 
                                      iCSDispatcher->getCombinedSearchResults());
}

TInt CWayFinderAppUi::GetCSCategoryCurrentIndex()
{
   return iCSCategoryResultView->GetCurrentIndex();
}

const std::vector<CombinedSearchCategory*>& CWayFinderAppUi::GetCombinedSearchResults()
{
   return iCSDispatcher->getCombinedSearchResults();
}

void CWayFinderAppUi::CSReqestMoreHits(TUint32 aLowerBound, TUint32 aHeadingNo)
{
   iCSDispatcher->requestMoreHits( aLowerBound, aHeadingNo );
}

void CWayFinderAppUi::AddFavoriteFromSearch(const isab::SearchItem& aSearchItem)
{
   char *parsed = 
      isab::SearchPrintingPolicyFactory::parseSearchItem(&aSearchItem, false,
                                                         isab::DistancePrintingPolicy::
                                                         DistanceMode(GetDistanceMode()));
   Favorite* fav = 
      new Favorite(aSearchItem.getLat(), aSearchItem.getLon(), 
                   aSearchItem.getName(), "", parsed, "", "");
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   LaunchEditFavoriteL(fav, ETrue);
   delete fav;
#else
   iMyDestView->AddFavoriteD( fav );
#endif
   delete[] parsed;
   // No deletion of fav is needed since MyDestView::AddFavorite deletes it when 
   // the new favorite message has been sent.
}

void CWayFinderAppUi::AddFavoriteFromMapL(isab::Favorite* aFav)
{
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   LaunchEditFavoriteL(aFav, ETrue);
   delete aFav;
#else
   iMyDestView->AddFavoriteD(aFav);
#endif
}

TBool CWayFinderAppUi::RouteStatusOnTrack()
{
   return (iLastRouteStatus == OnTrack);
}

TBool CWayFinderAppUi::InPedestrianMode()
{
   return (iDataStore->iSettingsData->m_transportationType == 
           NavServerComEnums::pedestrian);
}

void CWayFinderAppUi::SendTrackingLevel(int32 aTrackingLevel)
{
   if (aTrackingLevel != 0) {
      aTrackingLevel = isab::GuiProtEnums::tracking_level_live;
   }
   GeneralParameterMess gpm(GuiProtEnums::paramTrackingLevel, aTrackingLevel);
   SendMessageL(&gpm);   
}

int32 CWayFinderAppUi::SendRouteRequestL(CRouteData& aRouteData)
{
   RouteMess* routeMess = aRouteData.AsRouteMessL();
   LOGNEW(routeMess, RouteMess);
   CleanupStack::PushL(routeMess);
   int32 messageId = SendMessageL(*routeMess);
   LOGDEL(routeMess);
   CleanupStack::PopAndDestroy(routeMess);
   return messageId;
}

void CWayFinderAppUi::SendPendingRouteMessageL()
{
   if(!iPendingRouteData) {
     return;
   }
   int32 messageId = SendRouteRequestL(*iPendingRouteData);

   //We steal the iPendingRouteData and store it as the last route request.
   SetAsLastRouteRequest(iPendingRouteData, messageId);
   //So we must clear iPendingRouteData because the function above will take ownership.
   iPendingRouteData = NULL;
   
   DeletePendingRouteMessage();
}

void CWayFinderAppUi::SetPendingRouteMessageAndSendVehicleTypeL(GuiProtEnums::PositionType aDestinationType,
                                                                char* aDestinationId,
                                                                int32 aDestinationLat,
                                                                int32 aDestinationLon,
                                                                char* aDestinationName)
{
   DeletePendingRouteMessage();

   iPendingRouteData = CRouteData::NewL(aDestinationType,
                                        aDestinationId,
                                        aDestinationLat,
                                        aDestinationLon,
                                        aDestinationName);

   SendVehicleTypeL(NavServerComEnums::passengerCar);
}
  
void CWayFinderAppUi::SetPendingRouteMessageAndSendVehicleTypeL(GuiProtEnums::PositionType aOriginType,
                                                                char* aOriginId,
                                                                int32 aOriginLat,
                                                                int32 aOriginLon,
                                                                GuiProtEnums::PositionType aDestinationType,
                                                                char* aDestinationId,
                                                                int32 aDestinationLat,
                                                                int32 aDestinationLon,
                                                                char* aDestinationName)
{
   DeletePendingRouteMessage();
   iPendingRouteData = CRouteData::NewL(aOriginType,
                                        aOriginId,
                                        aOriginLat,
                                        aOriginLon,
                                        aDestinationType,
                                        aDestinationId,
                                        aDestinationLat,
                                        aDestinationLon,
                                        aDestinationName);

   SendVehicleTypeL(NavServerComEnums::passengerCar);
}

void CWayFinderAppUi::DeletePendingRouteMessage()
{
  if(iPendingRouteData) {
    delete iPendingRouteData;
    iPendingRouteData = NULL;
  }
}

void CWayFinderAppUi::SendVehicleTypeL(NavServerComEnums::VehicleType aVehicleType)
{
   if (aVehicleType == NavServerComEnums::invalidVehicleType) {
      // Do not send invalidVehicleType.
      return;
   }

   // Send set parameter with the given vehicle type
   GeneralParameterMess setgpm(GuiProtEnums::paramTransportationType, 
                               int32(aVehicleType));
   //iGuiProtHandler->SendMessage(setgpm);
   SendMessageL(setgpm);
   
   // Request the paramTransportationType
   GeneralParameterMess getgpm(GuiProtEnums::paramTransportationType);
   // Send a message to also get the transportation type so we end up in 
   // ReceiveMessageL.
   //iGuiProtHandler->SendMessage(getgpm, this);
   SendMessageL(getgpm);
}

void CWayFinderAppUi::DispatchAreaMatchSearchL(const isab::SearchItem& aItem, TUint32 aHeading)
{
   // Make a copy, we don't want to mess up the original one
   SearchRecord& sr = iCSMainView->GetSearchRecord();
   sr.setAreaId(aItem.getID());
   sr.setAreaName(aItem.getName());

   iCSDispatcher->dispatchAreaMatchSearch(sr, aHeading);
}

void CWayFinderAppUi::ShowInfoMsg( TInt aResourceId, TInt aDuration )
{
   HBufC* msgtxt = CCoeEnv::Static()->AllocReadResourceLC( aResourceId );
   ShowInfoMsg(*msgtxt, aDuration);
   CleanupStack::PopAndDestroy(msgtxt);
}


void CWayFinderAppUi::ShowInfoMsg( const TDesC &aText, TInt aDuration )
{
   if (aDuration > 0) {
      iEikonEnv->InfoMsgWithDuration(aText, aDuration);
   } else {
      iEikonEnv->InfoMsg( aText );
   }
}

void CWayFinderAppUi::InfoMsgCancel()
{
   iEikonEnv->InfoMsgCancel();
}

void CWayFinderAppUi::HandleNaviVolumeTimerL()
{
   iNaviVolumeTimer->Cancel();
   delete iNaviVolumeTimer;
   iNaviVolumeTimer = NULL;
   iNaviPane->Pop(iNaviVolumeControl);
   if (iTabGroup->ActiveTabId() == KMainMenuViewId.iUid){
      ActivateMainMenuNaviPaneLabelL();
      iMainMenuView->ResetNaviPaneLabelL();
   }
}

const char* CWayFinderAppUi::GetImeiCrcHex() const
{
   return iImeiCrcHex;
}

TPtrC CWayFinderAppUi::GetLmsId() const
{
   if (!iLmsId) {
      // No LmsId!!
      return KNullDesC();
   }
   return TPtrC(*iLmsId);
}

const char* CWayFinderAppUi::GetLmsIdChar() const
{
   if (!iLmsIdChar) {
      return NULL;
   }
   return iLmsIdChar;
}

void CWayFinderAppUi::DisownGuiLandmarkList()
{
   if (!iWFLMSManager) {
      // Nothing to disown in this case
      return;
   }
   iWFLMSManager->DisownGuiLandmarkList();   
}

void CWayFinderAppUi::DisownLandmarkList()
{
   if (!iWFLMSManager) {
      // Nothing to disown in this case
      return;
   }
   iWFLMSManager->DisownLandmarkList();
}

void CWayFinderAppUi::GetGuiLandmarksAsyncL(MWFLMSManagerBase::TWFLMSFavTypes aFavTypes)
{
   if (!iLmsId) {
      return;
   }
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   // XXX We should probably check that we're not busy fetching already.
   iWFLMSManager->GetGuiLandmarksAsyncL(aFavTypes);
}

void CWayFinderAppUi::GetLandmarksAsyncL(MWFLMSManagerBase::TWFLMSFavTypes aFavTypes)
{
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   iWFLMSManager->GetLandmarksAsyncL(aFavTypes);
}
std::vector<GuiFavorite*>* CWayFinderAppUi::GetGuiLandmarksL(MWFLMSManagerBase::TWFLMSFavTypes aFavTypes)
{
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   return iWFLMSManager->GetGuiLandmarksL(aFavTypes); 
}
std::vector<Favorite*>* CWayFinderAppUi::GetLandmarksL(MWFLMSManagerBase::TWFLMSFavTypes aFavTypes)
{
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   return iWFLMSManager->GetLandmarksL(aFavTypes);   
}

std::vector<GuiFavorite*>* CWayFinderAppUi::GetLmsGuiFavoriteList()
{
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   return iWFLMSManager->GetGuiLandmarkList();
}

std::vector<Favorite*>* CWayFinderAppUi::GetLmsFavoriteList()
{
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   return iWFLMSManager->GetLandmarkList();
}

TUint CWayFinderAppUi::AddFavoriteInLmsL(const isab::Favorite& fav)
{
   if (!iLmsId) {
      return MAX_UINT32;
   }
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   return iWFLMSManager->AddLandmarkL(fav);
}

void CWayFinderAppUi::UpdateFavoriteInLmsL(const isab::Favorite& fav)
{
   if (!iLmsId) {
      return;
   }
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   iWFLMSManager->UpdateLandmarkL(fav);
}

void CWayFinderAppUi::DeleteFavoriteInLmsL(TUint aId, const char* aLmsId)
{
   if (!iLmsId) {
      return;
   }
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   iWFLMSManager->DeleteLandmarkL(aId, aLmsId);
}

Favorite* CWayFinderAppUi::GetFavoriteFromLmsL(TUint aLmId, const char* aLmsId)
{
   if (!iWFLMSManager) {
      iWFLMSManager = CWFLMSManager::NewL(*this, *iLmsId, iImeiCrcHex);
   }
   return iWFLMSManager->GetLandmarkL(aLmId, aLmsId);
}

void CWayFinderAppUi::FavSyncCompleteL()
{
   GenerateEvent(EWayfinderFavLmsSyncDoCleanup);
}

void CWayFinderAppUi::LandmarkImportCompleteL()
{
   
}

void CWayFinderAppUi::GuiLandmarkImportCompleteL()
{
   if (iTabGroup->ActiveTabId() == KMyDestViewId.iUid) {
      iMyDestView->HandleLmsImportReplyL(iWFLMSManager->GetGuiLandmarkList());
   }
}

void CWayFinderAppUi::LandmarkImportedL(TInt aLmsNbrImported, 
                       TInt aTotalNbrLmsToImport,
                       TInt aLmNbrImported, 
                       TInt aTotalNbrLmToImport)
{
   
}

void CWayFinderAppUi::LmsError(TInt aError)
{
   
}

void CWayFinderAppUi::DatabaseInitialized()
{
   
}

TBool CWayFinderAppUi::IsLmsManagerOpRunning()
{
   if (iWFLMSManager && !iWFLMSManager->IsIdle()) {
      return ETrue;
   }
   return EFalse;
}

TBool CWayFinderAppUi::IsFavLmsSyncRunning()
{
   if (iFavoriteSyncHandler && iFavoriteSyncHandler->SyncIsRunning()) {
      return ETrue;
   }
   return EFalse;
}

void CWayFinderAppUi::DoFavLmsSyncL()
{
   if (IsFavLmsSyncRunning()) {
      // TODO: Display dialog saying a sync is already running
      return;
   }
   if (!iLmsId) {
      // Do no sync if we dont have a LMS database id to sync against.
      return;
   }
   if (iFavoriteSyncHandler) {
      delete iFavoriteSyncHandler;
   }

   iFavoriteSyncHandler = CFavoriteSyncHandler::NewL(*this, *iGuiProtHandler,
                                                     *iLmsId, iImeiCrcHex);

   // Transfer ownership of wflmsManager to iFavoriteSyncHandler
   iFavoriteSyncHandler->SetLMSManager(CWFLMSManager::NewL(*iFavoriteSyncHandler, 
                                                           *iLmsId, iImeiCrcHex));
   iFavoriteSyncHandler->DoSyncL();
}

void CWayFinderAppUi::ResetDestinationL() 
{
   if (iSelectView) {
      iSelectView->ResetDestinationL();
   }
}

void CWayFinderAppUi::QueryQuitNavigationL()
{
   typedef TCallBackEvent<CWayFinderAppUi, TWayfinderEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t ;
   
   cbd_t::RunDlgLD(cb_t(this,
                        EWayfinderQuitNavigationOk,
                        EWayfinderQuitNavigationCancel),
                        R_WF_STOP_NAVIGATION_TITLE,
                        R_WF_STOP_NAVIGATION_TEXT,
                        R_NON_BLOCKING_QUERY_MESSAGE);
}

void CWayFinderAppUi::ClearRouteL()
{
   if( IsSimulating() ){
      // The NavTask will now update it's internal state to stopped
      // we will a bit below send INVALIDATE_ROUTE. That message to
      // Nav2 will end up in STARTED_NEW_ROUTE with an route id set to 
      // 0. We will there reset the iSimulationStatus to the same value
      // as in NavTask.
      iNTCommandHandler->SendCommandL( isab::command_simulate_stop );
   }
   /* Send invalidate route with route id zero. */
   GenericGuiMess stop(GuiProtEnums::INVALIDATE_ROUTE,
                       (uint32)0, (uint32)0);
   SendMessageL(&stop);

   // Reset the origin and destination in the route planner view
   iSelectView->ResetOriginL();
   iSelectView->ResetDestinationL();
   // Reset the selection, this means that the origin will be
   // preselected the next time you enter the route planner view
   iSelectView->ResetSelection();
   // Set the route id to 0 in MapView, MapLib will then remove
   // the route layer in the map.
   iMapView->SetRouteIdL(0);
   // Reset the origin and destination coordinates.
   SetCurrentRouteCoordinates(MAX_INT32, MAX_INT32, 
                              MAX_INT32, MAX_INT32);
}

void CWayFinderAppUi::LaunchEditFavoriteL(Favorite* aFav, TBool addFavorite)
{
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iEditFavoriteView->SetFavoriteDataL(aFav);
   if (addFavorite) {
      // This is a new favorite so add it to wayfinder.
      push(KEditFavoriteViewId, EAddFavorite);
   } else {
      // This is a edit of an existing favorite so update it to wayfinder.
      push(KEditFavoriteViewId, EEditFavorite);
   }
#endif
}

int32
CWayFinderAppUi::sendCellIDRequest(const CellIDRequestData& req)
{
   isab::NParamBlock params;
   req.addParamsTo(params);

   return sendAsynchronousNGPRequestToNav2(
      params, isab::navRequestType::NAV_CELLID_LOOKUP_REQ);
}

int32
CWayFinderAppUi::sendAsynchronousNGPRequestToNav2(const isab::NParamBlock& params,
                                                  uint16 navRequestType)
{
   isab::Buffer buf(10);
   isab::Buffer tmpbuf(128);
   std::vector<byte> bbuf;

   params.writeParams(bbuf, NSC_PROTO_VER, false /* NO GZIP */);
   tmpbuf.writeNextByteArray(&bbuf.front(), bbuf.size());
   const uint8* data = tmpbuf.accessRawData();
   uint32 size = tmpbuf.getLength();

   buf.writeNextUnaligned16bit(navRequestType);

   class isab::DataGuiMess mess(isab::GuiProtEnums::GUI_TO_NGP_DATA,
                                buf.getLength(), buf.accessRawData(),
                                size, data, 1);

   int32 messageId = SendMessageL(&mess);

   mess.deleteMembers();

   return messageId;
}

void
CWayFinderAppUi::handleCellIDLookupReply(const isab::DataGuiMess& mess)
{
   isab::NParamBlock params(mess.getAdditionalData(), 
                            mess.getAdditionalSize(),
                            NSC_PROTO_VER);

   // Extract data
   const isab::NParam* coordP  = params.getParam(6300);
   int32 nav2Lat = coordP->getInt32(0);
   int32 nav2Lon = coordP->getInt32(4);
   Nav2Coordinate coord(nav2Lat, nav2Lon);
   int32 altitude = params.getParam(6301)->getInt32();
   uint32 innerRadius = params.getParam(6302)->getUint32();
   uint32 outerRadius = params.getParam(6303)->getUint32();
   uint16 startAngle = params.getParam(6304)->getUint16();
   uint16 endAngle = params.getParam(6305)->getUint16();
   
   const isab::NParam* cellTopLevel = params.getParam(6306);
   if (cellTopLevel) {
      iCellCountryCode = cellTopLevel->getUint32();	
   }
   
   // delete the last position if there was one
   if (iCellPosition) {
      delete iCellPosition;
      iCellPosition = NULL;
   }

   // save the new cell info
   iCellPosition = 
      new (ELeave) CCellPosInformation(coord, altitude, innerRadius, 
                                       outerRadius, startAngle, endAngle);

#ifdef __WINS__
   // For testing show the response on the emulator
   //ShowNetworkInfoDialogL(*iCellPosition);
#endif   
}

TBool CWayFinderAppUi::GetCellPosIfAvailable(Nav2Coordinate& aNavCoOrd, TUint& aRadius )
{
   if (iCellPosition) {
      aNavCoOrd = iCellPosition->getPosition();
      aRadius = iCellPosition->getOuterRadius();
      return ETrue;
   }
   return EFalse;
}

TInt CWayFinderAppUi::GetCellCountry()
{
	return iCellCountryCode;
}

TBool CWayFinderAppUi::IsCellPosAvailable()
{
   if (iCellPosition) {
      return ETrue;
   }
   return EFalse;
}

void CWayFinderAppUi::ShowRoamingWarningPopup(TBool aAutoclose)
{
  if(!iHaveDisplayedRoamingWarning) {
    HBufC* msgBuf = iCoeEnv->AllocReadResourceLC(R_WF_ROAMING_MESS_TEXT);
  	HBufC* titleBuf = iCoeEnv->AllocReadResourceLC(R_WF_ROAMING_MESS_TITLE);
  	_LIT(KNewLine, "\n");
  	const TInt dialogTextLength = 
  	   msgBuf->Length() + titleBuf->Length() + KNewLine().Length();
  	RBuf dialogText;
  	CleanupClosePushL(dialogText);
  	dialogText.Create(dialogTextLength);
    dialogText.Append(*titleBuf);
    dialogText.Append(KNewLine());
    dialogText.Append(*msgBuf);

    iRoamingDialog = CRoamingDialog::NewL(dialogText, this);
    iRoamingDialog->SetTone(CAknNoteDialog::ENoTone);
    if (!aAutoclose)
      {
        iRoamingDialog->SetTimeout(static_cast<CAknNoteDialog::TTimeout>(0));
      }
    else
      {
        iRoamingDialog->SetTimeout(static_cast<CAknNoteDialog::TTimeout>(7000000));
      }
    iRoamingDialog->Start();
    CleanupStack::PopAndDestroy(); // dialogText
    CleanupStack::PopAndDestroy(titleBuf);
    CleanupStack::PopAndDestroy(msgBuf);
    iHaveDisplayedRoamingWarning = ETrue;
  }
}

void CWayFinderAppUi::PreserveMapViewPosition()
{
   if(iMapView) {
      iMapView->PreservePositionOnNextActivation();
   }
}

void CWayFinderAppUi::SetStoredCountryIndex(TInt aCountryIndex)
{
	iStoredCountryIndex = aCountryIndex;
}

TInt CWayFinderAppUi::GetStoredCountryIndex()
{
	return iStoredCountryIndex;
}

TBool CWayFinderAppUi::CheckForNewVersion()
{
   // get the component parts
   int major = 0;
   int minor = 0;
   int build = 0;

   TPtrC p;
   TextUtils::ColumnText(p,0,iNewVersionData.iVersionNumber,'.');
   TLex lexMajor(p);
   lexMajor.Val(major);
   if (isab::Nav2_Major_rel != major )
      return ETrue;

   TextUtils::ColumnText(p,1,iNewVersionData.iVersionNumber,'.');
   TLex lexMinor(p);
   lexMinor.Val(minor);
   if (isab::Nav2_Minor_rel != minor )
      return ETrue;

   TextUtils::ColumnText(p,2,iNewVersionData.iVersionNumber,'.');
   TLex lexBuild(p);
   lexBuild.Val(build);
   if (isab::Nav2_Build_id != build )
      return ETrue;

   return EFalse;
}


void CWayFinderAppUi::ShowUpdateToNewVersionDialog()
{
   if(iHasCheckedForUpgrade)
      return;
   if(!(iNewVersionData.iVersionUrl && iNewVersionData.iVersionNumber)) {
      //We do not yet have both the url and the version number so abort.
      return;
   }
   iHasCheckedForUpgrade = ETrue;

   // if check enabled
   if ( GetSettingsData()->iCheckForUpdates){
   
      // check version number
      if (CheckForNewVersion()) {
      
         // display install new upgrade dialog
         CAknQueryDialog* dlg = CAknQueryDialog::NewL(); 
         if ( dlg->ExecuteLD(R_INSTALL_NEW_VERSION_DIALOG) )
         {
            // New version has been requested so start browser
            LaunchWapLinkL(*iNewVersionData.iVersionUrl);
            GenerateEvent(EWayfinderEventShutdownNow);
         }
         else
         {
            // New version declined so ask if "remind me again"
            CAknQueryDialog* dlgRemindMe = CAknQueryDialog::NewL(); 
            if ( dlgRemindMe->ExecuteLD(R_REMIND_ME_AGAIN_DIALOG) )
            {
               // remind me again selected
               GetSettingsData()->iCheckForUpdates = ETrue;
            }
            else
            {
               // don't remind me again
               GetSettingsData()->iCheckForUpdates = EFalse;
               
               // persist the setting change in the parameter file
               GeneralParameterMess* gen_mess;
               gen_mess = new (ELeave) GeneralParameterMess( GuiProtEnums::paramCheckForUpdates, int32(GetSettingsData()->iCheckForUpdates) );
               SendMessageL( gen_mess);
               gen_mess->deleteMembers();
               delete gen_mess;
            }
         }
      }
   }
}

void CWayFinderAppUi::SetAsLastRouteRequest(CRouteData* aRouteData, TInt32 aID)
{
   ClearLastRouteRequest();

   iLastRouteRequest = aRouteData;
   iLastRouteRequestMessId = aID;
}

void CWayFinderAppUi::ResendLastRouteRequestL()
{
   if(iLastRouteRequest) {
      SendRouteRequestL(*iLastRouteRequest);
   }
}
void CWayFinderAppUi::ClearLastRouteRequest()
{
   if(iLastRouteRequest) {
      LOGDEL(iLastRouteRequest);
      delete iLastRouteRequest;
      iLastRouteRequest = NULL;
   }
   iLastRouteRequestMessId = -1;
}
