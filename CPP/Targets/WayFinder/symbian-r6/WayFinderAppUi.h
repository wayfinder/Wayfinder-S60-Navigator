/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef WAYFINDERAPPUI_H
#define WAYFINDERAPPUI_H

// INCLUDES
#include <aknviewappui.h>           // CAknViewAppUi
#include <remconcoreapitargetobserver.h>

#include "RouteEnums.h"             // Route enums
#include "WayFinderConstants.h"     // Enum PositionType
#include "TimeOutNotify.h"          // MTimeOutNotify
#include "GuiProt/ServerEnums.h"    // ServerEnums
#include "GuiProt/HelperEnums.h"    // HelperEnums
#include "MapEnums.h"               // MapEnums
#include "NavServerComEnums.h"
#include "Quality.h"                // Isab::Quality
#include "IAPObserver_old.h"        // IAPObserver
#include "GuiDataStoreHolder.h"     // GuiDataStoreHolder
#include "PictureContainer.h"       // PictureContainer
#include "NokiaLbs.h"               // MWfNokiaLbsOwner
#include "SlaveAudioListener.h"     // MSlaveAudioListener
#include "EventGenerator.h"         // CEventGenerator template
#include "BtGpsConnector.h"         // MGpsConnectionObserver
#include "GuiProt/GuiProtMessageSender.h"   // MMessageSender
#include "DialNumber.h"             // MDialNumber
#include "NetworkInfoObserver.h"    // MNetworkInfoObserver
#include "NetworkRegistrationNotifierObserver.h"    // MNetworkRegistrationNotifierObserver
#include "LineStateListener.h"      // MLineStateListener
#include "ImageHandlerCallback.h"   // MImageHandlerCallback
#include "SmsHandlerObserver.h"     // MSmsHandlerObserver
#include "ImeiFetcher.h"            // CImeiFetcher
#include "ImsiFetcher.h"            // CImsiFetcher
#include "WayfinderSymbianUiBase.h" // WayfinderSymbianUiBase
#include "WFServiceViewHandler.h"   // For BackActionEnum
#include "ProgressDlgObserver.h"    // MProgressDlgObserver
#include "SearchResultObserver.h"
#include "CombinedSearchDataHolder.h"
#include "S60WaitDialogObserver.h"
#include "FavoriteSyncHandlerObserver.h"
#include "WFLMSObserver.h"
#include "WFLMSManagerBase.h"
#include "S60WFViewStack.h"
#include <aknprogressdialog.h>
#include "CSMainView.h"
#include "CSCategoryResultView.h"
#include "ViewBase.h"

#include "GuiProt/GuiProtRouteMess.h"
#include "RouteData.h"
#include "RoamingDialog.h"

// FORWARD DECLARATIONS

namespace isab {
   class GenericGuiMess;
   class ErrorMess;
   class GetTopRegionReplyMess;
   class RequestFailedMess;
   class SimpleParameterMess;
   class UpdateRouteInfoMess;
   class RouteListMess;
   class Log;  
   class SearchAreaReplyMess;
   class SearchItemReplyMess;
   class PrepareSoundsMess;
   class SoundFileListMess;
   class UpdatePositionMess;
   class MapReplyMess;
   class GeneralParameterMess;
   class DataGuiMess;
   class LicenseReplyMess;
   class FullSearchDataReplyMess;
   class GuiProtFileMess;
   class GuiFileOperationSelect;
   class UserRights;
   class Favorite;
   class OfflineTester;
   class VectorMapCoordinates;
   class TopRegionList;
}
class GuiProtMessageErrorHandler;
class S60WFViewStack;
class Nav2Coordinate;
class CRoamingDialog;

///global function
TBool IsReleaseVersion();


// CONSTANTS
//const ?type ?constant_var = ?constant;

// CLASS DECLARATION

/**
 * Application UI class.
 * Provides support for the following features:
 * - EIKON control architecture
 * - view architecture
 * - status pane
 * 
 */
class CWayFinderAppUi :
   public WayfinderSymbianUiBase,
   public CAknViewAppUi, 
   public MTimeOutNotify,
   public MMessageSender,
   public GuiDataStoreHolder,
   public IAPObserver,
   public PictureContainer,
   public MWfNokiaLbsOwner,
   public MSlaveAudioListener,
   public MDialNumber,
   public MGpsConnectionObserver,
   public MNetworkInfoObserver,
   public MImageHandlerCallback,
   public MSmsHandlerObserver,
   public MProgressDlgObserver,
   public MLineStateListener,
   public MRemConCoreApiTargetObserver,
   public MSearchResultObserver,
   public MWaitDialogObserver,
   public MFavoriteSyncHandlerObserver,
   public MWFLMSObserver,
   public MNetworkRegistrationNotifierObserver,
   public MRoamingCallback

{
public: // // Constructors and destructor

   CWayFinderAppUi(class isab::Log* aLog);

   /**
    * EPOC default constructor.
    */
   void ConstructL();

   /**
    * Destructor.
    */
   ~CWayFinderAppUi();

   /** Startup type.
    * Based on TAKMStartupType from 
    * lcsdefs.h, which is a part of the AKM package.
    * When the AKM package isn't available, we need to define this ourselves.
    * When Application Key has been entered, CWfAkmObserver will pass
    * EWfStartupFull in the DoStartProgram. If trial is used,
    * EWfStartupTrial is passed  
    */   
   enum TWfStartupType {
      EWfStartupFull,
      EWfStartupTrial
   };

   struct TWfNewVersionData {
      HBufC* iVersionNumber;
      HBufC* iVersionUrl;

      TWfNewVersionData() :
         iVersionNumber(NULL), iVersionUrl(NULL)
      {}

      ~TWfNewVersionData() {
         delete iVersionNumber;
         delete iVersionUrl;
      }
   };

   // Called from CWfAkmObserver if using AKM or directly from
   // DoConstructL if not.
   virtual void DoStartProgram(TWfStartupType aStartupType);

   void SetupStartup();

   const TUid GetAppUid();

   void CreateSlaveAudio();
   // Called in the callback when the startupimage has been shown.
   // Contiues the startup, creating all views etc.
   void SecondStageStartup();


private: // New functions
   /* Find the base paths to the Wayfinder application */
   /* and the resource files. We'll do this by searching */
   /* for a well known file along a path decided by the */
   /* language and other settings. */
   void InitPathsL();

   /**
    *   Cleans up and exits the application.
    */
   void doExit();

public:  // New functions
   TBool iExitDlgActive;

   void killNav2();
   void deleteNav2();

   static TBool CheckMem(TInt minAmount);

   /**
    * Connects the GUI with Nav2
    */
   void ConnectNav2L();

   void CreateBtGpsL();

   /**
    * Show dialog to the user if he want to connect to gps or not.
    */
   TBool ConnectToGpsQueryDialogL();

   /**
    * Convenience function to test for gps before navigate to command.
    */
   //TBool CheckAndDoGpsConnectionL(void (*aCallbackFunction)());

   TBool CheckAndDoGpsConnectionL(void* pt2Object, 
                                  void (*aCallbackFunction)(void* pt2Object));

   /**
    * Connect to the GPS.
    */
   void ConnectToGpsL();

   ///Disconnect from GPS.
   void DisconnectFromGPSL();

   void ReConnectGps();

   /**
    * Distribute GPS status (on or off)
    * to all views that need it.
    */
   void GpsStatus(TBool on);
   const class CAnimatorFrame* GetGpsStatusImage();

   /**
    * Checks current gps status and sets aImgId and aImgMaskId
    * to contain the correct mbm values for gps icon.
    *
    * @aImgId,     (IN) paramater for retrieving mbm index of correct gps
    *                   status image.
    * @aImgMaskId, (IN) paramater for retrieving mbm index of correct gps
    *                   status image mask.
    */
   void GetGpsStatusImageId(TInt& aImgId, TInt &aImgMaskId);

   //TDesC& GetMbmName();

   /**
    * Called when an error occurs in the Bluetooth connection.
    */
   void BTError( TDesC &aDescription );

   /**
    * Called when an error occurs in the Bluetooth connection.
    */
   void BTError( TInt aResourceId );

   /**
    * Called when data is received on Bluetooth connection. */
   void BTDataReceived(const TDesC& aName, const TDesC8& aAddr);

   virtual void BluetoothDebug(const TDesC& aDbgMsg);
   virtual void BluetoothError(const TDesC& aErrorMsg);
   virtual void BluetoothError(TInt aErrorMsg);
   virtual void BluetoothStatus(TBool aOk);
   void SaveBtGPSFakeAddress();
   virtual void BluetoothDeviceChosen(const class TBTDevAddr& aAddr, 
                                      const TDesC& aName);
   virtual void BluetoothConnectionLost(TBool aWillTryReconnect);


   /**
    * @return Distance mode (metric, yards or feet)
    */
   TInt GetDistanceMode();
   /**
    * Called when the distance mode is changed.
    * Views that need information on the change can be called here.
    */
   void SetDistanceMode(TInt mode);

   void ChangeMenuIcon(int bmpIndex, int maskIndex);

   /**
    * Returns current map position.
    */
   void GetCoordinate(TPoint& aRealCoord);
   
   TBool IsDialogDisplayed();

   /*
    * Returns true if Cell Position values are available and 
    * copies the coordinate into aNavCoOrd and the cell radius
    * (this is expected to me in metres) into aRadius.
    */
   TBool GetCellPosIfAvailable(Nav2Coordinate& aNavCoOrd, TUint& aRadius);

   /*
    * Returns the id of the Cell ID country 
    */
   TInt CWayFinderAppUi::GetCellCountry();

   /*
    * Returns true if Cell Position values are available 
    */
   TBool IsCellPosAvailable();

   /**
    * Starts or stops the connection manager. 
    */
private:
   void UpdateConnectionManagerL();
public:

   /**
    * Sends a message to the Nav2.
    * @param aMessage the message to send.
    * @return the message id.
    */
   int32 SendMessageL(class isab::GuiProtMess* aMessage );

   void UpdateData(class isab::UpdatePositionMess* aPositionMess );

   /**
    * Recives a message from the navigator.
    * @param aMessage the recived message.
    * @param aMessageLength the length of the recived message.
    */
   void ReceiveMessageL( const TDesC8& aMessage, const TInt aMessageLength );

   /**
    * Called by the call observer when a phone call state change occurs.
    * @param aConnected true if a call has been connected.
    */
   void HandlePhoneCallL( TBool aConnected );
   

   TBool EditAndCallNumberL( const TDesC& aPhoneNumber, TInt aCommand );
   TBool DialNumberL( const TDesC& aPhoneNumber, TInt aCommand );

   /**
    *   From MSlaveAudioListener
    *   Called when a sound has loaded.
    */
   void SoundReadyL( TInt aError, int32 aDuration );

   /**
    *   From MSlaveAudioListener
    *   Called when a sound has finished playing.
    */
   void SoundPlayedL( TInt aError );

   /**
    *   From MSlaveAudioListener
    *   Used for debugging.
    */
   void SendLogMessage( const TDesC& aMessage );

   /**
    * Called when an error in picture or sound handling has occured.
    */
   void ResourceErrorL( TInt aError, TBool aLanguageDependant );

   /**
    * Called when requesting a map.
    */
   void RequestMap( isab::MapEnums::MapSubject aMapType,
                    int32 aLat, int32 aLon, 
                    TInt aViewState = ENoMessage );

   void GotoStartViewL();


   void GotoInfoInServiceViewL(TInt aLat, TInt aLon, const HBufC* aSrvString = NULL);

   TInt64 GetRouteId();
   TInt64 GetLastRouteId();
   /**
    * Retrive the current top regions.
    */
   void GetTopRegionsL();

   /**
    * Send a request for the favorites lists.
    */
   void RequestFavoritesL( TBool aSync );

   /**
    * Send a message to Nav2 requesting a simple paramater.
    * @param aType the type of paramater, a GuiProtEnums::parameterType
    */
   void RequestSimpleParameterL( TUint aType );

   /**
    * Requests the route list from Nav2.
    * @param aStart the first requested crossing.
    * @param aNumber the number of requested crossing.
    */
   void RequestRouteListL( TInt aStart = 0, TInt aNumber = -1 );
   void SetCurrentRouteEndPoints(TDesC &aDescription,
                                 int32 dLat, int32 dLon,
                                 int32 oLat, int32 oLon);
   void SetCurrentRouteCoordinates(int32 dLat, int32 dLon,
                                   int32 oLat, int32 oLon);
   void SetCurrentRouteDestinationName(const TDesC &aDescription);
   /*    void SaveCurrentRouteCoordinates(int32 dLat, int32 dLon, */
   /*          int32 oLat, int32 oLon); */

   /**
    * Send request for a route to a previously stored destination.
    */
   void RouteToHotDestL();

   /**
    * Request a route to a given coordinate.
    * @param aLat the latitude of the coordinate
    * @param aLon the longitude of the coordinate
    * @param aDescription a description of the destination.
    */
   void RouteToCoordinateL( int32 aLat, int32 aLon, TDesC &aDescription );

   /** 
    * Set the force update boolean that forces the Guide page to be updated.
    * @param aForce true if the Guide page is to be updated
    */
   void SetForceUpdate( TBool aForce );

   /**
    * Activate a view.
    * Should not be used directly anymore, use push it is waaaay easier.
    * This is the function used by the viewstack to actually activate 
    * views on both push and pop.
    *
    * @param aUid the Uid of the view to make active
    * @param aMessageId the Uid of a custom message
    * @param aMessage a custom message
    */
   void SetViewL( TUid aUid, TUid aMessageId, const TDesC8& aMessage );

   void setTitleText(const TDes& aTitle);
   /**
    * Change the icon in the context pane.
    * @param aIconIdx an index to the icon to display
    */
   void SetContextIconL( TInt aIcon );
   void SetContextIconL();

   /**
    * Sets the navigation pane label.
    * @param aResourceId the id to a resource containing the label text
    */
   void SetNaviPaneLabelL( TInt aResourceId );

   /**
    * Start or stop waiting for a reply.
    * @param aStart true if starting a wait
    * @param aResourceId id to a resource containing a string describing the action
    */
   void SetWait( TBool aStart, TInt aResourceId );

   /**
    * @return a pointer to the data holder.
    */
   class CDataHolder* GetDataHolder();

   /**
    * @return the last valid position.
    */
   class TPoint GetCurrentPosition();

   /**
    * Sets the origin from gps in route planner.
    */
   void SetFromGps();

   /**
    * Set the position of the active destination.
    * @param aLat the latitude of the destination.
    * @param aLon the longitude of the destination.
    */
   void SetOrigin( enum isab::GuiProtEnums::PositionType aType,
                   const TDesC& aName,
                   const char* aId,
                   int32 aLat = MAX_INT32,
                   int32 aLon = MAX_INT32 );

   TBool HasOrigin();
   class TPoint GetOrigin();
   TBool IsOriginSet();
   void PositionSelectRoute();

   TBool HasDestination();
   class TPoint GetDestination();
   TBool IsDestinationSet();
   const TDesC& GetCurrentRouteDestinationName();

   /**
    * @return the position of the active destination.
    */
   void SetDestination( enum isab::GuiProtEnums::PositionType aType,
                        const TDesC& aName,
                        const char* aId,
                        int32 aLat,
                        int32 aLon);

   TBool IsSimulationPaused();
   TBool IsSimulationRepeating();
   TBool IsSimulating();
   TBool IsSimulationMaxSpeed();
   TBool IsSimulationMinSpeed();

   /**
    * @return true if a route is being followed.
    */
   TBool HasRoute();

   /**
    * Sets the index of the current turn.
    * @param aTurn the index of the current turn.
    */
   void SetCurrentTurn( TInt aTurn );

   /**
    * @return a pointer to the navigation pane.
    */
   class CAknNavigationDecorator* GetNavigationDecorator();

   void ActivateMainMenuNaviPaneLabelL();
   void DeactivateMainMenuNaviPaneLabelL();
   void SetMainMenuNaviPaneLabelL(TInt aResourceId);

   void ShowHelpL( TInt aResourceId1, TInt aResourceId2=0, TInt aResourceId3=0, TBool firstIsTitle = EFalse, TInt aResourceId4=0, TInt aResourceId5=0, TInt aResourceId6=0 );

   void ShowHelpL(TInt32 aUid);
   void ShowHelpL();

   enum TUpgradeMode { 
      //Use the upgrade menu bar and a dialog that contains code only
      EUpgrade, 
      //use the register menubar and a dialog that depends on the
      //ACTIVATE_WITH_CODE_AND_MSISDN and ACTIVATE_WITH_NAME_AND_EMAIL
      //defines.
      EActivate
   };
   void ShowUpgradeInfoL( enum TUpgradeMode mode );

   void RealShutdown();
   void ForceGotoSplashView();

   TBool ChooseHomeServerL();
   TBool ShowChooseHomeServerL();
   void SetDefaultServer();
   void SetServerUS();
   void SetServerEU();
   TBool AcceptPrivacyStatementL(); 

   /**
    * Shows a query dialog to let the user decide if he wants to see the 
    * warning message every time he starts the application.
    */
   TBool DisplayEndUserWarningQueryL();

   /**
    * Shows the end user warning message if it should be shown.
    */
   void ShowEndUserWarningMessageL();

   TBool AcceptGPRSNoticeL();

   TBool CanChangeMapType();
   TBool CanShowWebUsername();
   TBool CanShowWebPassword();
   TBool CanUseSubmitCode();
   TBool HaveVoiceInstructions();
   TBool CanUseGPSForEveryThing();

   TBool EarthShowGPSNoticeL();
   TBool EarthAllowGps();
   TBool EarthAllowRoute();

   /**
    * @return true if gps is connected to the device.
    */
   TBool IsGpsConnectedBare();
   TBool IsGpsConnected();
   TBool IsGpsConnectedAndNotSimulating();
   TBool IsGpsConnectedOrSearching();
   TBool IsGpsAllowed();
   TBool ValidGpsStrength();
   TBool IsFullVersion();
   TBool AutoRouteSmsDestination();
   TBool ShowUpgradeInfo();
   TBool ShowUpgradeInfoCba();

   TBool IsGoldVersion();
   TBool IsTrialVersion();
   TBool IsIronVersion();

   TBool HideServicesInfo();

   TBool ShowLangsWithoutVoice();
   TBool ShowLangsWithoutResource();

   TBool IsReleaseVersion();
   TBool IsOnlyOneLanguage();
   TBool ShowMoveWayfinder();
   TBool ShowGetWayfinder();
   TBool UseTrackingOnAuto();
   TBool showTracking() const;
   TBool hasFleetRight() const;
   const class isab::UserRights* GetUserRight() const;

   void SaveMapLayerSettings();

   /** 
    * Sets the acp setting in map view that sets it in maplib.
    * @param aEnableACP, if 0 the acp is set to minimzed,
    *                    if 1 the acp is set to normal.
    */
   void SaveACPSettings(TBool aEnableACP);

   void SaveCheckForUpdatesSetting(TBool aValue);

   void SettingsSetIAP(TInt aIAP);
   void SetIAP(TInt aIAP);
   int32 GetIAP();
   void ReportProgressIAP(TInt aVal, TInt aMax, HBufC* aName);
   TBool ShowIAPMenu();
   void CheckIAPChosen(TInt ch);
   void ReportFailureIAP(TInt error, TBool temporary = EFalse);
   void SendIAP(int32 aIAPid, TBool isReal = EFalse, TBool aShow = ETrue);

   void SendSyncParameters();
   void SendSyncFavorites();
   void SendNop();
   void SendIAPId2(int32 iap);

   void HandleWayfinderType(isab::GuiProtEnums::WayfinderType wft);
   void SetWayfinderType(isab::GuiProtEnums::WayfinderType wft);

   TInt GetSymbianLanguageCode();
   void SymbianLanguageToChar(TInt symCode, uint8* outCode);
   void SaveLanguageCode( TInt language );

   /**
    * Gets the used language. Made this function since rom builds
    * for paris phones etc only uses phone language, not user
    * defined as for ordinary installations.
    * @return The language code in symbian code format.
    */
   TPtrC UpdateLangCodesL();

   TPtrC GetLangCodeInIso();

   NavServerComEnums::languageCode GetLanguageCode();

   void HandleSaveSearchHistoryL();

   class CAknNavigationControlContainer* getNavigationControlContainer() const;

   void TurnBackLightOnL();

   TRgb GetNewsBgColor();
   TPoint GetNewsConStatusPos();

   /**
    * Asks the users permission to send a silver sms. 
    * Uses a callback dialog, EWayfinderEventSendRegistrationSmsOk
    * is sent if user wants to send registration sms, 
    * EWayfinderEventSendRegistrationSmsCancel if not.
    */
   TBool CheckSilverSMSQueryL();

   /**
    * Member variables return the bounding box of the current route.
    * @param aTopLeft will contain the top left corner of the bounding box
    * @param aBottomRight will contain the bottom right corner of the bounding box
    */
   void GetRouteBoundingBox( TPoint &aTopLeft, TPoint &aBottomRight );

   /**
    * @return ETrue if we're currently using vector maps
    */
   TBool VectorMaps();

   // From MWfNokiaLbsOwner
   virtual void NokiaLbsUpdate(GpsState newState);


   // Start and Show info dialogs about sharing wayfinder
   void StartSpreadWayfinderDialogsL();

   void ImeiNumberSet(const TDesC& aImeiNumber);

   void ImsiNumberSet(const TDesC& aImsiNumber);

   /// Will be called once imei has been fetched.
   void ImeiFetched();

   /// Will be called once imsi has been fetched.
   void ImsiFetched();
   
   /**
    * Callback from MImageHandlerCallback when
    * gif is converted and scaled.
    */
   virtual void ImageOperationCompletedL(TInt aError);

   /**
    * Starts a wait dialog.
    * @param aTextResourceId, the id for the text that should be displayed
    *                         in the dialog.
    */
   void StartWaitDlg(class MWaitDialogObserver* aObserver,
                     TInt  aTextResourceId);

   /**
    * Stops the currently running wait dialog.
    */
   void StopWaitDlg();

   /**
    * Starts a wait dialog without a cancel command.
    * Used for showing progress when calculating a route.
    */
   void StartRouteWaitDlgL(class MWaitDialogObserver* aObserver);

   /**
    * Stops the route wait dialog.
    */
   void StopRouteWaitDlg();
   
   /**
    * Starts a wait dialog without a cancel command.
    * Used for showing progress when making a search.
    */
   void StartSearchWaitDlgL(class MWaitDialogObserver* aObserver);

   /**
    * Stops the search wait dialog.
    */
   void StopSearchWaitDlg();

   /**
    * From MWaitDialogObserver
    * Notifies it's observer that user has pressed
    * cancel button on the wait dialog.
    */
   void CancelPressed();

   /**
    * From MWaitDialogObserver
    * Notifies it's observer that the framework has closed the 
    * wait dialog. (e.g. a background/foreground switch or screensaver)
    */
   void DialogDismissedFromFramework();

   TBool IsWaitingForRoute();
private: // New functions
   /**
    * @return the last valid position.
    */
   void SetCurrentPosition( int32 aLat, int32 aLon );

   /// Updates if it is time to mute.
   void updateMute();

   /// Returns the value that mute should have.
   bool getShouldMute() const;
   
   template<typename T>
   void AddViewToFrameWorkL(T*& variable)
   {
      variable = T::NewLC(this, iLog);
      AddViewL(variable);
      CleanupStack::Pop(variable);
   }

   /**
    * Used to force action for testing and debug purposes,
    * called with the debug action menu choice under the 
    * tools menu.
    */
   void DebugAction();

   /**
    * Launch the built in wapbrowser with the buy extension
    * wap link.
    */
   void LaunchBuyExtensionWapLinkL();

   void LaunchWapLinkL(const TDesC& aUrl);

   /**
    * Updates the contextpane during a wait for a reply,
    * called when the timer expires.
    */
   TBool CheckWait();

   /**
    * Switch to the settings view.
    */
   void SwitchToSettingsL();

   /**
    * Show the about dialog
    */
   void ShowAboutL();

   /**
    * Displays error messages sent from Nav2.
    */
   void ShowErrorDialogL(TInt32 aErrNbr,
                         const char* aUtf8Text, TBool aBanner = EFalse);

   /**
    * Displays warning message sent from Nav2.
    */
   void ShowWarningDialogL(TInt32 aErrNbr, const char* aUtf8Text);
   
   void RequestCancelled();
   
   /*
    * Check version number to see if a new version available
    * return ETrue if new software available
    */
    TBool CheckForNewVersion();

public:

   void ShowErrorDialogL(TInt32 aErrNbr,
                         TInt aResourceId, TBool aBanner = EFalse);

   TBool DontHandleAsterisk();

   /**
    * Enum to control what we want to use the result of 
    * the call to FetchNetworkInfoL for.
    */
   enum TNetworkInfoState {
      ENetworkInfoIdle,
      EUseForGettingPosition,
      EUseForGettingPositionAndRoamingState,
      EUseForDebugDialog,
      EUseForDebugSms
   };

   /**
    * Initializes a asynchronous sequence to get all network information that 
    * we are interested in.
    */
   void FetchNetworkInfoL(enum TNetworkInfoState aState);

   /**
    * From MNetworkInfoObserver, called when all network info is available.
    */
   void NetworkInfoDone();

   /**
    * From MNetworkInfoObserver, called when there already is an 
    * outstanding request to get network info.
    */
   void NetworkInfoInUse();

   /**
    * From MNetworkInfoObserver, called when we failed to fetch the network 
    * info from the phone.
    */
   void NetworkInfoFailed(TInt aError);
   
   /**
    * From MNetworkRegistrationNotifierObserver
    * Called when the network registration status has changed
    */
   void NetworkRegistrationStatuschanged();

   /**
    * Shows a dialog with network and cell information.
    */
   void ShowNetworkInfoDialogL(class CCellPosInformation& aCellInfo);

   /**
    * Delete the iap.txt file.
    */
   void DeleteIapTxt();

   /**
    * Call to the observer from the search module when categories
    * has been received from the server. 
    * Calls CCSCategoryResultView->AddCategories.
    */
   void SearchCategoriesReceived();

   /**
    * Call to the observer from the search module when requested
    * images has been received from the server. 
    * Calls the image cache to locate and convert the images.
    * Calls CCSDetailedResultView->AddResultsL
    */
   void RequestedImagesDownloaded();

   /**
    * Call to the observer from the search module when search
    * results has been received and total number of hits has
    * been calculated.
    * Pushes the CCSCategoryResultView if not allready active view and
    * calls CCSCategoryResultView->AddCategories.
    */
   void TotalNbrHitsReceived(const std::vector<CombinedSearchCategory*>& categories);

   /**
    * Call to the observer from the search module when search
    * results has been received and parsed.
    */
   void SearchResultReceived();

   /**
    * Call to the observer from the search module when search
    * results has been received and parsed, a reply to a request
    * to the search module for fetching more search results for 
    * a given heading.
    */
   void MoreSearchResultReceived();

   /**
    * Call to the observer from the search module when search
    * results has been received and parsed, a reply to a request
    * to the search module for doing a area match search.
    */
   void AreaMatchSearchResultReceived();

   void SetDetailedSearchResults();
   TInt GetCSCategoryCurrentIndex();
   const std::vector<CombinedSearchCategory*>& GetCombinedSearchResults();
   void CSReqestMoreHits(TUint32 aLowerBound, TUint32 aHeadingNo);
   void AddFavoriteFromSearch(const isab::SearchItem& aSearchItem);
   void AddFavoriteFromMapL(isab::Favorite* aFav);
   /**
    * Returns if we're on track or not.
    */
   TBool RouteStatusOnTrack();

   /**
    * Returns true if we're in pedestrian mode.
    */
   TBool InPedestrianMode();

   /**
    * Gets the iImeiCrcHex that is the imei number that has been
    * crc32 calculated.
    *
    * @return the iImeiCrcHex.
    */
   const char* GetImeiCrcHex() const;

   /**
    * Get function for the wayfinder landmark database uri.
    *
    * @return The wayfinder landmark database uri.
    */
   TPtrC GetLmsId() const;

   /**
    * Get function for the wayfinder landmark database uri (char*).
    *
    * @return The wayfinder landmark database uri as a char*.
    */
   const char* GetLmsIdChar() const;

   /**
    * Calls CWFLMSManager::DisownGuiLandmarkList.
    */
   void DisownGuiLandmarkList();

   /**
    * Calls CWFLMSManager::DisownLandmarkList.
    */
   void DisownLandmarkList();

   /**
    * Requests the landmarks from the lms as GuiFavorites. This operation
    * is done asynchronously.
    *
    * @param aFavTypes, specifies what type of landmarks that should be included
    *                   into the export. The enum can be found in WFLMSManagerBase.h.
    */
   void GetGuiLandmarksAsyncL(MWFLMSManagerBase::TWFLMSFavTypes aFavTypes);

   /**
    * Requests the landmarks from the lms as Favorites. This operation
    * is done asynchronously.
    *
    * @param aFavTypes, specifies what type of landmarks that should be included
    *                   into the export. The enum can be found in WFLMSManagerBase.h.
    */
   void GetLandmarksAsyncL(MWFLMSManagerBase::TWFLMSFavTypes aFavTypes);

   /**
    * Requests the landmarks from the lms as Favorites. This operation
    * is done synchronously.
    *
    * @param aFavTypes, specifies what type of landmarks that should be included
    *                   into the export. The enum can be found in WFLMSManagerBase.h.
    * @return A vector containing all the gui favorites from lms.
    */
   std::vector<GuiFavorite*>* GetGuiLandmarksL(MWFLMSManagerBase::TWFLMSFavTypes aFavTypes);

   /**
    * Requests the landmarks from the lms as Favorites. This operation
    * is done synchronously.
    *
    * @param aFavTypes, specifies what type of landmarks that should be included
    *                   into the export. The enum can be found in WFLMSManagerBase.h.
    * @return A vector containing all the favorites from lms.
    */
   std::vector<Favorite*>* GetLandmarksL(MWFLMSManagerBase::TWFLMSFavTypes aFavTypes);

   /**
    * Fetches the vector containing the Favorites that has been exported from the
    * lms. Usually this function is called when GetGuiLandmarksAsyncL is completed
    * the lms manager has reported to GuiLandmarkImportCompleteL.
    *
    * @return A vector containing all the gui favorites from lms.
    */
   std::vector<GuiFavorite*>* GetLmsGuiFavoriteList();

   /**
    * Fetches the vector containing the Favorites that has been exported from the
    * lms. Usually this function is called when GetLandmarksAsyncL is completed
    * the lms manager has reported to LandmarkImportCompleteL.
    *
    * @return A vector containing all the favorites from lms.
    */
   std::vector<Favorite*>* GetLmsFavoriteList();

   /**
    * Adds a favorite to the wayfinder landmark database.
    *
    * @param fav, The favorite to add.
    */
   TUint AddFavoriteInLmsL(const isab::Favorite& fav);

   /**
    * Updates a favorite in the wayfinder landmark database.
    *
    * @param fav, The favorite to be updated.
    */
   void UpdateFavoriteInLmsL(const isab::Favorite& fav);

   /**
    * Deletes a favorite from the wayfinder landmark database.
    *
    * @param aId, the landmark id for the favorite to be deleted.
    */
   void DeleteFavoriteInLmsL(TUint aId, const char* aLmsId);

   /**
    * Gets a favorite from the lms.
    * 
    * @param aLmId, the id of the landmark to be exported.
    * @param aLmsId, the lms where to look for the landmark.
    * @return The landmark with the given id from the given lms.
    */
   Favorite* GetFavoriteFromLmsL(TUint aLmId, const char* aLmsId);

   /**
    * Returns true if the LmsManager is running any async operation.
    */
   TBool IsLmsManagerOpRunning();

   /**
    * Returns true if the favorite lms sync is currently running.
    */
   TBool IsFavLmsSyncRunning();

   /**
    * Starts the lms fav sync if not already running.
    */
   void DoFavLmsSyncL();

   /**
    * Clears the destination from route planner.
    */
   void ResetDestinationL();

   /**
    * Called when user selected back from one of the navigation views.
    * Launches a query dialog that asks the user if he/she wants to 
    * stop the navigation. If yes the route will be cleared and the user
    * will be sent back to the route planner view. If cancel nothing will
    * happen.
    */
   void QueryQuitNavigationL();

   /**
    * Shows the edit favorite view and loads the data into the form.
    */
   void LaunchEditFavoriteL(class Favorite* aFav, TBool addFavorite);

   /*
    * If a upgrade is available prompt user to upgrade
    */
   void ShowUpdateToNewVersionDialog();
protected:
   //    void HandleScreenDeviceChangedL();

private:
   void HandleReportEventMenu(TInt command);
   void SendDebugSms(uint32 errNum, const TDesC& errTxt);

   /**
    * Displays request failed messages sent from Nav2.
    * @param aMessage the error message
    */
   void HandleRequestFailedL(class isab::RequestFailedMess* aMessage );

   /**
    * Requests paramaters that have changed from Nav2.
    * @param aMessage message containing what paramater type that has changed.
    */
   /*    void UpdateParametersL( class isab::GenericGuiMess* aMessage ); */

   /**
    * Handle a top region reply message.
    * @param aMessage a top region reply message
    */
   void HandleTopRegionReplyL(class isab::GetTopRegionReplyMess* aMessage );

   /**
    * Handle a simple paramater reply message.
    * @param aMessage a simple paramater reply message
    */
   /*    void HandleSetSimpleParametersL(class isab::SimpleParameterMess* aMessage ); */

   /**
    * Handle a search reply.
    * @param aSearchReply a packet containing the reply
    */
   void HandleSearchReplyL(class isab::SearchAreaReplyMess* aSearchAreas );
   void HandleSearchReplyL(class isab::SearchItemReplyMess* aSearchItems );

   /** 
    * From MRoamingCallback
    */
   void HandleRoamingDialogDone(MRoamingCallback::TRoamingStatus aStatus);
   
   /**
    * Start following a route.
    */
   void StartNavigationL(int32 oLat, int32 oLon, int32 dLat, int32 dLon,
                         const char *destinationName);

   /**
    * Abandon the route.
    */
   void StopNavigationL();

   /**
    * Handle a update route info mesage.
    * @param aRouteMessage a UpdateRouteInfoMessage
    */
   void HandleNavigation(class isab::UpdateRouteInfoMess* aRouteMessage );

   /**
    * Handle the reply to a route list request
    */
   void HandleRouteList(class isab::RouteListMess* aMessage );

   /**
    * Handle a request to preload sounds
    */
   void HandlePrepareSounds(class isab::PrepareSoundsMess* aMessage );

   /**
    * Handle a request to play the latest preloaded sounds
    */
   void HandlePlaySounds();

   /** 
    * Handle the clip-to-filename mapping from a new audio syntax
    */
   void HandleSoundFileList(class isab::SoundFileListMess* aMessage );

   /**
    * Makes sure that we have the right resources installed.
    *
    * Exits program if not!
    * Uses CheckOneResourceVersion().
    */
   void CheckResources();

   /**
    * Makes sure that we have the right resources installed.
    *
    * Exits program if not!
    */
   void CheckOneResourceVersion(TDesC &resource_file, int min_major, int min_minor);

   TBool IsAudioResourceOk();

   HBufC* GetApplicationNameLC();
   HBufC* GetApplicationVersionLC();

   /**
    * Handles map replies.
    */
   void HandleMapReply(class isab::MapReplyMess* aMessage );

   void HandleLicenseUpgradeReply(class isab::LicenseReplyMess* mess);

   void HandleProgressIndicator(class isab::GenericGuiMess* message);
   /**
    * @param aUid the id of a view.
    * @return true if the view is in the tab order.
    */
   TBool IsInTabOrder( int32 aUid );

   // Maybe update the gps status indicator and mapview.
   void HandleGpsQuality( isab::Quality gpsQuality );

   /**
    * Gets the imei, needed for connecting to Nav2 and
    * when sending registrations sms.
    */
   void FetchImei();
   
   /**
    * Gets the imsi, needed for connecting to Nav2.
    */
   void FetchImsi();

   /**
    * Checks if we are ready to use the gps.
    * Send registration sms is hooked in 
    * before this function, the code in
    * this function was eariler located in
    * GotoStartViewL().
    */
   void CheckReadyToUseGps();

   /**
    * Sets night mode to the views that should
    * be changed.
    * @param aNightMode true if night mode should be switched on
    *                   false if night mode should be switched off
    */
   void SetNightModeL(TBool aNightMode);

   /**
    * Updates iCurrentView to point to the new to be current view.
    * @param aToBe The uid for the view to be current view.
    */
   void UpdateCurrentView(TUid aToBe);

   /**
    * Handles the navi pane timer callback.
    */
   void HandleNaviVolumeTimerL();

   /**
    * Sends INVALIDATE_ROUTE mess to nav2 and clears the plan route view.
    */
   void ClearRouteL();

   int32 sendCellIDRequest(const class CellIDRequestData& req);

   int32 sendAsynchronousNGPRequestToNav2(const isab::NParamBlock& params,
                                          uint16 navRequestType);

   void handleCellIDLookupReply(const isab::DataGuiMess& mess);

public:

   /**
    * From MProgressDlgObserver
    */
   void ProgressDlgAborted();

   /**
    * From PictureContainer
    */

   void PictureError( TInt aError );

   void ScalingDone();

   /** 
    * Returns the current foreground color that should be used.
    * @param aR referens the will contain the R value for the fg color.
    * @param aG referens the will contain the G value for the fg color.
    * @param aB referens the will contain the B value for the fg color.
    */
   void GetForegroundColor(TInt& aR, TInt& aG, TInt& aB);

   /** 
    * Returns the current foreground color that should be used.
    * @param aRgb referens the will contain the rgb for the fg color.
    */
   void GetForegroundColor(TRgb& aRgb);

   /** 
    * Returns the current background color that should be used.
    * @param aR referens the will contain the R value for the bg color.
    * @param aG referens the will contain the G value for the bg color.
    * @param aB referens the will contain the B value for the bg color.
    */
   void GetBackgroundColor(TInt& aR, TInt& aG, TInt& aB);

   /**
    * Returns the background color for nightmode.
    */
   void GetNightmodeBackgroundColor(TRgb& aRgb);

   /**
    * Lets caller now if we are in night mode or not.
    * @return True if night mode is switched on
    *         False if not.
    */
   TBool IsNightMode();

   void SendTrackingLevel(int32 aTrackingLevel);

   /**
    * Callback function for the timer that runs the navi volume control.
    */
   static TInt NaviVolumeCallback(TAny* aThisPtr) {
      CWayFinderAppUi* thisPtr = (CWayFinderAppUi*) aThisPtr;
      thisPtr->HandleNaviVolumeTimerL();
      return(1);
   }
   

  /**
   * Creates a RouteMess based on the input and saves it as a pending
   * route request. Sends NavServerComEnums::passengerCar as vehicle type
   * to Nav2. When Nav2 responds, the request will be dispatched.
   * @param aDestinationType The type of position we are routing to.
   * @param aDestinationId The id of the destination.
   * @param aDestinationLat The latitude of the destination.
   * @param aDestinationLon The longitude of the destination.
   * @param aDestinationName The name of the destination.
   */
   void SetPendingRouteMessageAndSendVehicleTypeL(GuiProtEnums::PositionType aDestinationType,
                                                  char* aDestinationId,
                                                  int32 aDestinationLat,
                                                  int32 aDestinationLon,
                                                  char* aDestinationName);
  
  /**
   * Creates a RouteMess based on the input and saves it as a pending
   * route request. Sends NavServerComEnums::passengerCar as vehicle type
   * to Nav2. When Nav2 responds, the request will be dispatched.
   * @param aOriginType The type of position we are routing from.
   * @param aOriginId The id of the origin.
   * @param aOriginLat The latitude of the origin.
   * @param aOriginLon The longitude of the origin.
   * @param aDestinationType The type of position we are routing to.
   * @param aDestinationId The id of the destination.
   * @param aDestinationLat The latitude of the destination.
   * @param aDestinationLon The longitude of the destination.
   * @param aDestinationName The name of the destination.
   */
   void SetPendingRouteMessageAndSendVehicleTypeL(GuiProtEnums::PositionType aOriginType,
                                                  char* aOriginId,
                                                  int32 aOriginLat,
                                                  int32 aOriginLon,
                                                  GuiProtEnums::PositionType aDestinationType,
                                                  char* aDestinationId,
                                                  int32 aDestinationLat,
                                                  int32 aDestinationLon,
                                                  char* aDestinationName);

private:

  /**
   * Deletes the pending route message and all data belonging to it.
   */
  void DeletePendingRouteMessage();

  /**
   * Sends the vehicle type to Nav2 for setting.
   * @param aVehicleType The vehicle type to send for setting.
   */
  void SendVehicleTypeL(NavServerComEnums::VehicleType aVehicleType);

  /**
   * Creates a route message from a RouteData and sends it.
   * @param aRouteData The route data to send.
   * @return The id of the route message send.
   */
   int32 SendRouteRequestL(CRouteData& aRouteData);

  /**
   * Sends the pending route message, if it exists.
   */
   void SendPendingRouteMessageL();
  
public: // Functions from base classes

   // from WayfinderSymbianUiBase
   virtual void StartIapSearch();
   // from WayfinderSymbianUiBase
   virtual void RestartIapSearch();
   // from WayfinderSymbianUiBase
   virtual void ShutdownNow();
   // from WayfinderSymbianUiBase
   virtual void ClearBrowserCache();
   // from WayfinderSymbianUiBase
   virtual void GotoMainMenu();
   // from WayfinderSymbianUiBase
   virtual void ActivateSplashViewL();
   // from WayfinderSymbianUiBase
   virtual void ShowRegistrationSmsDialog();
   // from WayfinderSymbianUiBase
   virtual void ReportStartupFailed(class WFStartupEventCallbackParameter* param);
   // from WayfinderSymbianUiBase
   virtual void ReportStartupError(class WFStartupEventCallbackParameter* param);
   // from WayfinderSymbianUiBase
   virtual void CheckFlightMode();
   // from WayfinderSymbianUiBase
   virtual void ShowUpgradeErrorL(class WFStartupEventCallbackParameter* param);

   /**
    * From WayfinderSymbianUiBase:
    * Launch the Service window with the appended variables.
    */
   void GotoServiceViewL(const TDesC& aUrl,
                         enum BackActionEnum aAllowBack = BackIsHistoryThenView);
   /**
    * From WayfinderSymbianUiBase
    * handleSetLatestNewsImage - Save latest news image.
    */
   void handleSetLatestNewsImage(const uint8* data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetVolume - Set volume.
    */
   virtual void handleSetVolume(int32 volume);
   /**
    * From WayfinderSymbianUiBase
    * handleSetUseSpeaker - Set if speaker should be used.
    */
   virtual void handleSetUseSpeaker(bool on);
   /**
    * From WayfinderSymbianUiBase
    * handleSetBtGpsAddressAndName - Set the BT gps address data.
    */
   virtual void handleSetBtGpsAddressAndName(const char** data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetCategoryIds - Set ids for search categories.
    */
   virtual void handleSetCategoryIds(const char** data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetCategoryNames - Set names for search categories.
    */
   virtual void handleSetCategoryNames(const char** data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetCategoryIconNames - Set names for the icon files of the search categories.
    */
   virtual void handleSetCategoryIconNames(const char** data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetCategoryIntIds  - Set integer ids for search categories.
    */
   virtual void handleSetCategoryIntIds(const int32* data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetServerNameAndPort - Set server name setting.
    * Transfers ownership of string.
    */
   virtual void handleSetServerNameAndPort(HBufC* serverString);
   /**
    * From WayfinderSymbianUiBase
    * handleSetHttpServerNameAndPort - Set server name setting.
    * Transfers ownership of string.
    */
   virtual void handleSetHttpServerNameAndPort(HBufC* serverString);
   /**
    * From WayfinderSymbianUiBase
    * handleSetMapLayerSettings - Set map layer settings.
    * If data is set to NULL, the parameter is treated as unset.
    */
   virtual void handleSetMapLayerSettings(const uint8* data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetPoiCategories - Set POI category settings.
    * If data is set to NULL, the parameter is treated as unset.
    */
   virtual void handleSetPoiCategories(const uint8* data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetSearchHistory - Set search history strings from Nav2.
    */
   virtual void handleSetSearchHistory(const uint8* data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetTrackingPIN - Set tracking pin strings.
    */
   virtual void handleSetTrackingPIN(const uint8* data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetUsernameAndPassword - Username and password has been set.
    */
   virtual void handleSetUsernameAndPassword();
   /**
    * From WayfinderSymbianUiBase
    * handleSetVectorMapSettings - Settings for vector maps.
    */
   virtual void handleSetVectorMapSettings(const int32* data, int32 numEntries);
   /**
    * From WayfinderSymbianUiBase
    * handleSetVectorMapCoordinates - Scale and last coordinates for map.
    */
   virtual void handleSetVectorMapCoordinates(isab::VectorMapCoordinates* vmc);
   /**
    * From WayfinderSymbianUiBase
    * handleSetSelectedAccessPointId - Saved IAP id.
    */
   virtual void handleSetSelectedAccessPointId(int32 iap);
   /**
    * From WayfinderSymbianUiBase
    * handleSetSelectedAccessPointId2 - Currently used IAP id.
    */
   virtual void handleSetSelectedAccessPointId2(int32 iap);
   /**
    * From WayfinderSymbianUiBase
    * handleSetTopRegionList - Set the top region list for searches.
    */
   virtual void handleSetTopRegionList(isab::TopRegionList* topRegionList);

   /**
    * From WayfinderSymbianUiBase
    * handleSetACPSetting - Set ACP enabled or disabled.
    */
   virtual void handleSetACPSetting(int32 enableACP);

   /**
    * From WayfinderSymbianUiBase
    * handleCheckForUpdatesSetting - Set Check for update enabled or disabled.
    */
   virtual void handleCheckForUpdatesSetting(int32 aValue);

   /**
    * From WayfinderSymbianUiBase
    * setNewVersionNumber - Set new version number.
    */
   virtual void setNewVersionNumber(const char* version);

   /**
    * From WayfinderSymbianUiBase
    * setNewVersionUrl - Set new version url.
    */
   virtual void setNewVersionUrl(const char* versionUrl);

   /**
    * From WayfinderSymbianUiBase
    * isWFIDStartup - returns true if USE_WF_ID is defined.
    */
   virtual bool isWFIDStartup();

   /**
    * handleSendServerNameAndPort - Send correct parameter for server string.
    * (http or regular)
    */
   void handleSendServerNameAndPort(const TDesC& serverString);

   // from MTimeOutNotify

   /**
    * TimerExpired
    * The function to be called when a timeout occurs
    */
   void TimerExpired(); 

   ///Method called by CMessageHandler when Nav2 is in serious trouble!
   void Panic(class CMessageHandler* from);

   /** 
    * From MEikMenuObserver
    * Dynamically change the layout of a menu.
    * @param aResourceId the Id of the menu.
    * @param aMenuPane a handle to the menu.
    */
   void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

   isab::GuiProtEnums::YesNoAsk SaveSmsDestination() const;
   isab::GuiProtEnums::YesNoAsk KeepSmsDestination() const;

   /*    inline class CCoeEnv* CoeEnv() const { return iCoeEnv; } */

   class CMyDestView* MyDestView() const { return iMyDestView; }

   enum isab::Quality GpsQuality() const { return iGpsQuality; }   

   class CConnectData* getConnectData() const { return m_connectData; }

public: // Functions from base classes

   /**
    * From CEikAppUi, takes care of command handling.
    * @param aCommand command to be handled
    */
   void HandleCommandL( TInt aCommand );

   /** From MMessageSender.
    * Sends a message to Nav2.
    * @param mess the message to send.
    * @return The gui protocol message id.
    */
   virtual int32 SendMessageL(class isab::GuiProtMess &mess);

   /** From MSmsHandlerObserver.
    * Sends one or more text messages.
    * Only used on s60v3 for now.
    */
   virtual void SmsSent(TInt aStatus, 
                        TInt aCount, 
                        TInt aNbrSent);

   void SmsReceived(class CSmsParser* aSmsParser, TInt32 aMsvId);
   
   void HandleResourceChangeL(TInt aType);


   /** 
    * From CEikAppUi, gets called for all kinds of key events.
    * Is used by us for supressing the "end call" key, instead
    * of closing the app we send wf to the bacground (3rd edition).
    * @param aEvent the event, in our case what key
    * @param aDestination, not used by us.
    */
   void HandleWsEventL(const TWsEvent &aEvent, 
                       class CCoeControl *aDestination);

   void HandleSystemEventL(const TWsEvent& aEvent);


   /** From MLineStateListener.
    * Receives status changes of the phone line.
    */
   virtual void LineStatusChanged( const TLineState aLineState );

   /** From MLineStateListener.
    * Receives errors when trying to make a phone call.
    */
   virtual void CallError( const TCallError aCallError );

   /**
    * Push on the view stack.
    */
   void push( TUid v, TUid g );

   /**
    * Push on the view stack.
    */
   void push( TUid v ) { 
      push( v, TUid::Uid( EWayFinderInvalidViewId ) ); 
   }

   /**
    * Push on the view stack.
    */
   void push( TUid v, TUid g, TInt messageId );

   void push( TUid v, TUid g, TInt messageId, const TDesC8& aMessage );

   /**
    * Push on the view stack.
    */
   void push( TUid v, TInt messageId ) {
      push( v, TUid::Uid( EWayFinderInvalidViewId ), messageId ); 
   }

   void push( TUid v, TInt messageId, const TDesC8& aMessage ) {
      push( v, TUid::Uid( EWayFinderInvalidViewId ), messageId, aMessage ); 
   }

   /**
    * Pop from view stack.
    */
   void pop();

   /**
    * Peek the view stack.
    */
   TUid peek();

   /**
    * Pop from view stack.
    */
   void pop( TInt messageId );
   /**
    * Go to the view left of current in current group, may do nothing.
    */
   void goLeftInGroup();

   /**
    * Go to the view right of current in current group, may do nothing.
    */
   void goRightInGroup();

   /**
    * Set the next message id for the comming push or pop.
    */
   void setNextMessagId( TInt messageId );

   /**
    * Set the navigation pane on off and which arrow to show.
    */
   void setNaviPane(TBool aShow = ETrue);

   /**
    * Gets called by system when user presses volume buttons.
    * aOperationId Can be a lot of different stuff, the two
    *              thins we check for is ERemConCoreApiVolumeDown
    *              and ERemConCoreApiVolumeUp.
    * aButtonAct   Not used right now.
    */
   void MrccatoCommand(TRemConCoreApiOperationId aOperationId, 
                       TRemConCoreApiButtonAction aButtonAct);

   /**
    * Get function that returns the CombinedSearchDataHolder
    * @return CombinedSearchDispatchers instance of its data holder.
    */
   const CombinedSearchDataHolder& GetCombinedSearchDataHolder();

   /**
    * Sends a request to CombinedSearchDispatchers for a area match search.
    * @param aItem, the SearchItem containing the area id and the area name.
    * @param aHeading, the heading to make the area match search in.
    */
   void DispatchAreaMatchSearchL(const isab::SearchItem& aItem, TUint32 aHeading);

   /*
    * Displays a info message in the top right corner for a specified 
    * period of time.
    * @param aResourceId, the resource id for the text to be displayed.
    * @param aDuration, the duration for the message to be displayed,
    *                   if 0 framework will decide the duration.
    */
   void ShowInfoMsg( TInt aResourceId, TInt aDuration = 0 );

   /*
    * Displays a info message in the top right corner for a specified 
    * period of time.
    * @param aText, the text to be displayed.
    * @param aDuration, the duration for the message to be displayed,
    *                   if 0 framework will decide the duration.
    */
   void ShowInfoMsg( const TDesC &aText, TInt aDuration = 0 );

   /**
    * Cancels the currently displaying info message.
    */
   void InfoMsgCancel();

   /**
    * Simply forwards a request to the SearchableListBoxView to
    * set up the cached country list.
    */
   void SetupCSCountryListL();

   /** Sets the country index to the default (first one received)
    *
    */
   void SetDefaultCountryIndex();
   /**
    * Simply forwards a request to the SearchableListBoxView to
    * set up the cached category list.
    */
   void SetupCSCategoryListL();

   /**
    * Get function that returns its pointer to CombinedSearchDispatchers
    * 
    * @return a pointer to iCSDispatcher.
    */
   class CombinedSearchDispatcher* GetCSDispatcher();

   /**
    * Callback function that are called by CFavoriteSyncHandler when
    * landmarks and wayfinder favorites are synced together. Based on a 
    * call to CFavoriteSyncHandler::DoSyncL().
    */
   void FavSyncCompleteL();

   /**
    * Callback function that are called when GetLandmarksAsyncL
    * has been called.
    */ 
   void LandmarkImportCompleteL();

   /**
    * Callback function that are called when GetGuiLandmarksAsyncL 
    * has been called.
    */ 
   void GuiLandmarkImportCompleteL();

   /**
    * Progress report from CWFLMSManager during exexution of
    * GetLandmarksAsyncL or GetGuiLandmarksAsyncL.
    *
    * @param aLmsNbrImported, the number of databases completed.
    * @param aTotalNbrLmsToImport, the total number of databased to 
    *                              be imported.
    * @param aLmNbrImported, the number of landmarks imported.
    * @param aTotalNbrLmsToImport, the total number of landmarks to 
    *                              be imported.
    */
   void LandmarkImportedL(TInt aLmsNbrImported, 
                          TInt aTotalNbrLmsToImport,
                          TInt aLmNbrImported, 
                          TInt aTotalNbrLmToImport);

   /**
    * Called by CWFLMSManager when an error has occured.
    * 
    * @param aError, probably one of the error codes in 
    *                MWFLMSManagerBase::TWFLMSError
    */
   void LmsError(TInt aError);

   /**
    * Called dby CWFLMSManager when a database has been initialized,
    * if the initialization is done async.
    */
   void DatabaseInitialized();

   /**
    * Sets the route data and message id for the last requested route.
    * This data will be used when we need to resend the last route request we made.
    * @param aRouteData The RouteData to store. This function will take ownership over the CRouteData object.
    * @param aID The id of the route message we send
    */
   void SetAsLastRouteRequest(CRouteData* aRouteData, TInt32 aID);

   /**
    * Resends the last route request we made.
    * See SetAsLastRouteRequest for more info about what will be resend.
    */
   void ResendLastRouteRequestL();

   /**
    * Clears the route request data that is stored for the last route request.
    */
   void ClearLastRouteRequest();

private: // Functions from base classes
   /**
    * From CEikAppUi, handles key events.
    * @param aKeyEvent Event to handled.
    * @param aType Type of the key event. 
    * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
    */
   virtual enum TKeyResponse HandleKeyEventL(const struct TKeyEvent& aKeyEvent,
                                             enum TEventCode aType);

   /**
    *  From CEikAppUi, Handles a change of focus
    *  @param aForeground is ETrue if this application has focus
    */
   void HandleForegroundEventL(TBool aForeground);

   void RouteToCoordinateL( TInt32 aLat, TInt32 aLon,
                            const TDesC &aDescription );


   void RouteToSMSL( TInt32 originlat, TInt32 originlon,
                     TInt32 destinationlat, TInt32 destinationlon,
                     const TDesC& destNameStr);


   void HandleSmsL( TInt32 aLat, TInt32 aLon,
                    const TDesC& aDescription,
                    TInt32 aMsvId, class isab::Favorite* aFavorite = NULL );


   void HandleSmsL( TInt32 aOriginLat, TInt32 aOriginLon,
                    TInt32 aDestinationLat, TInt32 aDestinationLon,
                    const TDesC& aDestinationDescription,
                    TInt32 aMsvId );

   TBool ShowBuyExtensioDialog(TUint aErrorNbr);


public:

   enum TWayfinderEvent {
      EWayfinderEventConnectToGps                  = 2,
      EWayfinderEventReportEventListDialogOk       = 3,
      EWayfinderEventReportEventListDialogCancel   = 4,
      EWayfinderEventShutdownNow                   = 7,
      EWayfinderEventStartupFailedOk               = 8,
      EWayfinderEventStartupFailedCancel           = 9,
      EWayfinderEventBuyExtension                  = 10,
      EWayfinderEventCheckNetwork                  = 11,
      EWayfinderEventDoNothing                     = 12,
      EWayfinderFavLmsSyncDoCleanup                = 13,
      EWayfinderQuitNavigationOk                   = 14,
      EWayfinderQuitNavigationCancel               = 15,
      EWayfinderEventCheckAndShowUpgradeDialog     = 16,
   };
   
   void GenerateEvent(enum TWayfinderEvent aEvent);
   void HandleGeneratedEventL(enum TWayfinderEvent aEvent);

   void ShowUSWarningPopup();
   void SendSMSReportEvent(TInt aId);

   void CheckNetwork();

   /**
    * Displays a warning to the user that he is on a roaming network.
    * @param aAutoClose ETrue for autoclose. Otherwise EFalse.
    */
   void ShowRoamingWarningPopup(TBool aAutoClose = EFalse);

   class CSettingsData* GetSettingsData() const;

   /**
    * Tells the mapview to preserve the view position.
    * I.e. not reset the visible area to the gps position on the next activation of the view.
    */
   void PreserveMapViewPosition();
   
   TInt GetStoredCountryIndex();
   void SetStoredCountryIndex(TInt aCountryIndex);
private:
   typedef CEventGenerator<CWayFinderAppUi, enum TWayfinderEvent>
   CWayFinderEventGenerator;

   CWayFinderEventGenerator* iEventGenerator;

public:
   /**
    * Called by the connect dialog when it is about to close.
    */
   void ConnectDialogIsClosing();

   class CCoeEnv* getCoeEnv() const { return iCoeEnv; } 

  const CCSMainView::TFormData& CWayFinderAppUi::GetFormData() const;
private: //Data
   class CWfAkmObserver*                  iWfAkmObserver;
   //
   /// The default navi pane control.
   class CAknNavigationControlContainer*  iNaviPane;

   /// Contains handles to the tab pages.
   class CAknTabGroup*                    iTabGroup;

   /// Pointer to the navigation decorator.
   class CAknNavigationDecorator*         iDecoratedTabGroup;

   /// Pointer to the navigation decorator that handles the volume control.
   class CAknNavigationDecorator*         iNaviVolumeControl;

   class CAknNavigationDecorator*         iDecoratedNaviLabel;
   class CAknNaviLabel*                   iNaviPaneLabel;

   /// Pointer to a temporary navigation decorator.
   class CAknNavigationDecorator*         iTempDecorator;

   /// Handle to the context pane.
   class CAknContextPane*                 iContextPane;
   class CContextPaneAnimator*            iContextPaneAnimator;

   /// Handles sending and receiving messages from NavCtrl.
   class CMessageHandler*                 iMessageHandler;

   class CViewBase* iCurrentView;

   class CWelcomeView*                    iWelcomeView;

   class CMainMenuListView* iMainMenuView;

   /// The origin destination select view
   class CPositionSelectView*             iSelectView;

   /// The New Destinatiom page.
   class CNewDestView*                    iNewDestView;

   class CCSMainView* iCSMainView;
 
   class CSearchableListBoxView* iSearchableListBoxView;

   class CCSCategoryResultView* iCSCategoryResultView;

   class CCSDetailedResultView* iCSDetailedResultView;


#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   /// The edit favorite view.
   class CEditFavoriteView* iEditFavoriteView;
#endif

   /// The My Destination page.
   class CMyDestView*                     iMyDestView;

   /// The Guide page.
   class CGuideView*                      iGuideView;

   /// The Itinerary View
   class CItineraryView*                  iItineraryView;

   /// The Map page
   class CMapView*                        iMapView;

   /// The Route page.
   class CRouteView*                      iRouteView;

   /// The Information page.
   class CInfoView*                       iInfoView;

   /// The Settings view
   class CSettingsView*                   iSettingsView;

   /// MyAccount view
   class CMyAccountView*                   iMyAccountView;

#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3 
   class CSimpleConnectionManager* iConnMgr;
#endif     

   /// Object that handles sound.
   class CSlaveAudio*                     iAudioSlave;

   /// Object that holds all data.
   class CDataHolder*                     iDataHolder;

   /// Current Cell position information if known
   class CCellPosInformation*			  iCellPosition;
   
   /// Current waypoint ID.
   TInt                             iCurrentWpId;

   /// True if the nav2 is following a route.
   TBool                            iHasRoute;

   /// The name of the current destination.
   HBufC*                      iDestinationName;

   /// Last route status.
   enum isab::RouteEnums::OnTrackEnum          iLastRouteStatus;

   /// The latest GPS quality.
   enum isab::Quality                          iGpsQuality;

   /// Counter for discovering missing GPS while simulating.
   TInt                             iNoGpsCounter;

   /// Incremented by 1 each time a message is sent.
   uint16                           iMessageCounter;

   class CHelpUrlHandler* iHelpUrlHandler;

private:
   /// The current position.
   class TPoint                           iCurrPos;
   class TPoint                           iCurrentDestination;
   class TPoint                           iCurrentOrigin;

   HBufC*                        iCurrentDestinationName;

   /// Force an update of the guide view.
   TBool iForceUpdate;

   /// True if waiting for a reply.
   TBool iWaiting;

   /// True if synchronizing favorites.
   TBool iSynchronizing;

   /// Timer active object
   class CTimeOutTimer*                   iTimer;

   /// Active object that listens after phone calls.
   class CCallObserver*                   iCallObserver;

   /// Handles communication with the GPS
   //class CBTHandler*                      iBtHandler;
   class MPositionInterface*               iBtHandler;

   class isab::Nav2* m_nav2;

   /// The top left corner ot the routes bounding box
   class TPoint iRouteBoxTl;

   /// The bottom right corner ot the routes bounding box
   class TPoint iRouteBoxBr;

   TInt64 m_routeid;
   TInt64 m_last_routeid;

   // Dialogs for the spread Wayfinder functionality
   class CSpreadWFDialogs* iSpreadWFDialogs;

   class CConnectData* m_connectData;

   class CIAPSearcher_old* iIAPSearcher;

public:
   class isab::Log* iLog;

   /* File session object, common for the whole GUI. */
   class RFs m_fsSession;

   /* Holds all the wayfinder paths. */
   class CPathFinder* iPathManager;

   int m_XXX_temp_no_search_items;

   TBool iForeGround;

   TInt m_speed;
   TInt m_angle;

   /** This is only used as a temporary solution 
    * while we have the ini-file. */
   TBuf<256> iIniFilePath;

   class IniFile* iIniFile;

   TBool iUrgentShutdown;

   TBool iGotBTData;

   TBool m_hasSetLanguage;

   TInt iMemCheckInterval;

public:
   class CGpsSocket* iGpsSocket;
   class CNokiaLbsReader* iPositionProvider;

   class CGuidePicture* iStartupImage;

   TBool iDoingShutdown;

   /// True if there is a phone call in progress
   bool m_phoneCallInProgress;

   HBufC* iWfTypeStr;
   HBufC* iWfOptsStr;
   TInt iEventSMSid;
   HBufC* iEventSmsErrTxt;

   class CNetworkInfoHandler* iNetworkInfoHandler;

   class CNetworkRegistrationNotifier* iNetworkRegistrationNotifier;
   /// Keep track if we have displayed a warning or not. We only want to show it once.
   TBool iHaveDisplayedRoamingWarning;

   /// The last time the backlight was triggered.
   uint32 iLastTimeBacklightTrigger;

   class CS60NavTaskGuiCommandHandler* iNTCommandHandler;

   uint16 iSimulationStatus;
   class isab::LogMaster* iLogMaster;

   TInt iSelectedReportEvent;
   CDesCArray* iReportList;
   class CWayfinderCommandHandler* iCmdHandler;
   class CWFCmdCallbackHandler* iCmdCallback;
   TInt iPhoneCallFromWf;
   TInt iSelectedSms;

   class MSmsHandler* iSmsHandler;
   char* iImeiNbr;
   char* iImsiNbr;
   char* iImeiCrcHex;

   typedef CImeiFetcher<CWayFinderAppUi> CWayfinderImeiFetcher;
   CWayfinderImeiFetcher* iImeiFetcher;

   typedef CImsiFetcher<CWayFinderAppUi> CWayfinderImsiFetcher;
   CWayfinderImsiFetcher* iImsiFetcher;

   TBool iImeiReceived;
   TBool iImsiReceived;
#if defined NAV2_CLIENT_SERIES60_V3
   class CSymbian9CallHandler* iCallHandler;
#endif

   class CImageHandler* iImageHandler;

   class GuiProtMessageHandler* iGuiProtHandler;
   class GuiProtMessageErrorHandler* iGuiProtMessageErrorHandler;
   TBool iMessagesReceived;

   class CProgressDlg* iProgressDlg;

   class CWayfinderDataHolder* iDataStore;
   class ErrorFilter* iErrorFilter;
   /// Used to filter out error messages from Nav2 that relate
   /// to network errors, for example when the connection to the
   /// navigation server has been lost.
   class TimeOutErrorFilter* iTimeOutErrorFilter;
   
   class isab::OfflineTester* iOfflineTester;

   class S60WFViewStack* m_viewStack;

   class CWaitDlg* iWaitDlg;
   class CWaitDlg* iRouteWaitDlg;
   class CWaitDlg* iSearchWaitDlg;

   // For getting notified when user adjustes the volume
   // by using hardware buttons on s60v3 phones.
   class CRemConInterfaceSelector* iRemSelector;
   class CRemConCoreApiTarget* iRemTarget;

   /// True if nightmode is selected
   TBool iNightMode;
   TBool iGotParamterConnectToGps;
   TBool iShowNoGpsWarn;

   /// Handles the combined search requests
   class CombinedSearchDispatcher* iCSDispatcher;

private:

   /// The handle to the language list resource
   TInt iLanguageListRsc;

   /// Member that holds current language in iso two letter.
   HBufC* iLangCodeInIso;

   /// Member that holds current language in symbian code
   HBufC* iLangCodeInSymbian;

   /// Name of the wayfinder landmark database
   HBufC* iLmsId;

   /// Name of the wayfinder landmark database (char for convenience)
   char* iLmsIdChar;

   /// The nav2 lang code.
   isab::NavServerComEnums::languageCode iLanguageCode;

   /// Timer that handles the navi volume control visibility
   class CPeriodic* iNaviVolumeTimer;

   /// Handles syncing of landmarks
   class CFavoriteSyncHandler* iFavoriteSyncHandler;

   /// Interface to the landmark database
   class CWFLMSManager* iWFLMSManager;

   void* iNavigateToGpsWizCallbackObject;
   void (*iNavigateToGpsWizCallbackFunc)(void* pt2Object);

   /// Flag to be able to keep backlight on when we need to (to override the 
   /// setting for backlight strategy).
   TBool iKeepBacklightOn;

   /// State to control what the result of FetchNetworkInfoL should be used for.
   TNetworkInfoState iNetworkInfoState;

   TInt32 iNGPCellIdSeqId;

   /// Flag to know if the user issued re-route command.
   TBool iManualRerouteIssued;
   /// Code for the Country of the Cell 
   TInt iCellCountryCode;

   /// Pending route message data that will be used to send a route message
   /// once the vehicle type has been successfully set.
   CRouteData* iPendingRouteData;

   CRoamingDialog* iRoamingDialog;

   TWfNewVersionData iNewVersionData;

   /// The last sent route request.
   CRouteData* iLastRouteRequest;
   /// The message id used when sending the last route request.
   TInt32 iLastRouteRequestMessId;
   /// Keep track if we are  handling a request failed message caused by a route request.
   TBool iHandlingFailedRouteRequestMessage;

   TBool iHasCheckedForUpgrade;
   
   TInt iStoredCountryIndex;
   
   TBool iWaitingForRoute;
};

#endif

// End of File
