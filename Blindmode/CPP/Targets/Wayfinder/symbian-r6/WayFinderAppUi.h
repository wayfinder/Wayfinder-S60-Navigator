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

#include "RouteEnums.h"             // Route enums
#include "WayFinderConstants.h"     // Enum PositionType
#include "TimeOutNotify.h"          // MTimeOutNotify
#include "GuiProt/ServerEnums.h"    // ServerEnums
#include "GuiProt/HelperEnums.h"    // HelperEnums
#include "MapEnums.h"               // MapEnums
#include "Quality.h"                // Isab::Quality
#include "ConnectDialog.h"          // Enums CConnectDialog::ETransmitMode etc
#include "IAPObserver_old.h"        // IAPObserver
#include "GuiDataStoreHolder.h"     // GuiDataStoreHolder
#include "PictureContainer.h"       // PictureContainer
#include "NokiaLbs.h"               // MWfNokiaLbsOwner
#include "SlaveAudioListener.h"     // MSlaveAudioListener
#include "EventGenerator.h"         // CEventGenerator template
#include "BtGpsConnector.h"         // MGpsConnectionObserver
#include "GuiProt/GuiProtMessageSender.h"   // MMessageSender
#include "DialNumber.h"             // MDialNumber
#include "WABrowserUrlFormatter.h"  // MWABrowserUrlFormatter
#include "NetworkInfoObserver.h"    // MNetworkInfoObserver
#include "LineStateListener.h"      // MLineStateListener
#include "ImageHandlerCallback.h"   // MImageHandlerCallback
#include "SmsHandlerObserver.h"     // MSmsHandlerObserver
#include "ImeiFetcher.h"            // CImeiFetcher
#include "WayfinderSymbianUiBase.h" // WayfinderSymbianUiBase
#include "WFServiceViewHandler.h"   // For BackActionEnum
#include "ProgressDlgObserver.h"    // MProgressDlgObserver
#include "NavServerComEnums.h"
#include "MC2Direction.h"
#include "FeedSettings.h"
#include <map>

// FORWARD DECLARATIONS

class MapLib;
class DetailFetcher;
class FavoriteFetcher;
class VicinityItem;
class CLockedNavigationView;
class CVicinityFeedView;
class MapResourceFactory;

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
   class LicenseReplyMess;
   class FullSearchDataReplyMess;
   class GuiProtFileMess;
   class GuiFileOperationSelect;
   class UserRights;
   class Favorite;
   class OfflineTester;
}

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
   public MWABrowserUrlFormatter,
   public MNetworkInfoObserver,
   public MImageHandlerCallback,
   public MSmsHandlerObserver,
   public MProgressDlgObserver,
   public MLineStateListener
{
   friend class CSplashView;
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

   // Called from CWfAkmObserver if using AKM or directly from
   // DoConstructL if not.
   virtual void DoStartProgram(TWfStartupType aStartupType);

   void SetupStartup();

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

   void ShowNoGpsPositionsDialogL();

   /**
    *   Cleans up and exits the application.
    */
   void doExit();

   //  Determines if an error warrants a shutdown
   //  of the application.
   TBool isCriticalError(TInt32 errorNum);

//	TileMapControl * 
//	PreConstructTileMapControl();

public:  // New functions
   void killNav2();
   void deleteNav2();

   void StartupCompleted();
   
   void SaveBtGPSFakeAddress();
   
   static TBool CheckMem(TInt minAmount);

   /**
    * Connects the GUI with Nav2
    */
   void ConnectNav2L();

   void CreateBtGpsL();

   TBool IsHeadingUsable();
   
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

   isab::NavServerComEnums::VehicleType GetTransportationMode();
   
   /**
    * Starts or stops the connection manager. 
    */
private:
   void UpdateConnectionManagerL();
public:
   void ShowDetailsL(class isab::FullSearchDataReplyMess* aMessage,
         TInt aCommand);
   
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
         int32 aLat, int32 aLon );
   void GotoStartViewL();

   void GotoLockedNavigationViewL(VicinityItem* target, MapLib* aMapLib);
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

   TInt32 LastViewId();

   /**
    * Activate a view.
    * @param aUid the Uid of the view to make active
    * @param aMessageId the Uid of a custom message
    * @param aMessage a custom message
    */
   void SetViewL( TUid aUid, TUid aMessageId, TDesC8 aMessage );

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

#ifndef NAV2_CLIENT_SERIES60_V3
   /**
    * Shows the connect dialog.
    * @param favorite NULL or a favorite to be sent. Favorite will be deleted in here so don't reference it afterwards.
    * @param transmitMode Initial transmit mode.
    * @param contentsType Initial contents type.
    * @param messageFormat Initial message format.
    */
   void ShowConnectDialog(const class isab::Favorite* favorite = NULL, 
      CConnectDialog::ETransmitMode transmitMode = CConnectDialog::DefaultTransmitMode, 
      CConnectDialog::EContentsType contentsType = CConnectDialog::DefaultContentsType, 
      CConnectDialog::EMessageFormat messageFormat = CConnectDialog::DefaultMessageFormat,
      TPoint* pos = NULL);
#endif

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
   void ShowUpgradeInfoL(enum TUpgradeMode mode, TUid aBackToView);

/*    void ShowUpgradeHelp(); */
/*    void ShowUpgradeDialogL(enum TUpgradeMode mode); */

/*    void CloseUpgradeDialog( TBool aDoClose ); */

/*    void SendUpgradeDetails(TUint32 aCountryId, const TDesC &aPhone,  */
/*                            const TDesC &aRegNumber, const TDesC& aName, */
/*                            const TDesC &aEmail); */

   void RealShutdown();
   void ForceGotoSplashView();
   void GotoSplashView();

   TBool ChooseHomeServerL();
   TBool ShowChooseHomeServerL();
   void SetDefaultServer();
   void SetServerUS();
   void SetServerEU();
/*    TBool AcceptLegalNoticeL(); */
   TBool AcceptPrivacyStatementL(); 
   TBool AcceptGPRSNoticeL();

   TBool CanChangeMapType();
   TBool CanShowWebUsername();
   TBool CanShowWebPassword();
   TBool CanUseSubmitCode();
   TBool HaveVoiceInstructions();
   TBool CanUseGPSForEveryThing();

   TBool EarthShowGPSNoticeL();
   TBool EarthAllowGps();
   /**
    * @return true if gps is connected to the device.
    */
   TBool IsGpsConnected();
   TBool IsGpsConnectedAndNotSimulating();
   TBool IsGpsConnectedOrSearching();
   TBool IsGpsAllowed();
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

   void SetIAP(TInt aIAP);
   int32 GetIAP();
   void ReportProgressIAP(TInt aVal, TInt aMax, HBufC* aName);
   TBool ShowIAPMenu();
   void CheckIAPChosen(TInt ch);
   void ReportFailureIAP(TInt error, TBool temporary = EFalse);
   void SendIAP(int32 aIAPid, TBool isReal = EFalse, TBool aShow = ETrue);

   void SendSyncParameters();
   void SendSyncFavorites();

   void HandleWayfinderType(isab::GuiProtEnums::WayfinderType wft);
   void SetWayfinderType(isab::GuiProtEnums::WayfinderType wft);

   /**
    * Shows a popuplist.
    * @param aDescArray The alternatives.
    * @param aOkChosen If true, ok was chosen. Output parameter.
    * @param selection The current selection, or maxuint32 if none.
    * @return The index if ok was chosen.
    */
#if 0
   TInt ShowPopupListL( const CDesCArrayFlat& aDescArray,
                               TBool& aOkChosen,
                               TInt selection,
                               TBool aShowTitle = EFalse,
                               TDesC* aTitle = NULL);
#endif

   TInt GetSymbianLanguageCode();
   void SymbianLanguageToChar(TInt symCode, uint8* outCode);
   void SaveLanguageCode( TInt language );

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

/* #if defined NAV2_CLIENT_SERIES60_V3 */
   void ImeiNumberSet(const TDesC& aImeiNumber);
/* #endif */

   /**
    * Callback from MImageHandlerCallback when
    * gif is converted and scaled.
    */
   virtual void ImageOperationCompletedL(TInt aError);

private: // New functions
   TInt iHeading;
   TBool iStartupCompleted;
   
   TBool InfoViewActive();
   TBool MapViewActive();

   void AttemptViewRefresh(TUid aViewId);
   
   void ActivateLocalViewL (TUid aViewId);
   void ActivateLocalViewL (TUid aViewId,
                            TUid aCustomMessageId,
                            const TDesC8 &aCustomMessage);


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

#ifndef NAV2_CLIENT_SERIES60_V3
   /**
    * Shows the connect dialog using data from the previous session.
    * This function is called, e.g., when communication fails etc.
    */
   void ShowConnectDialog(CConnectData* connectData);
#endif

#if 0
   //State of the audio playback
   enum TSoundState
   {
      EIdle,
      EBusy,
      ELoadingInstruction,
      EInstructionReady,
      EPlayingInstruction,
      ELoadingNextTurn,
      EPlayingNextTurn,
      EPlayingInformation,
      EPlayingTurnConfirm
   };
#endif

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
   HBufC* FormatWapLinkLC(const TDesC& aBase, TInt aWABrowser);
   void LaunchWapLinkL(const TDesC& aUrl);

   /**
    * From MWABrowserUrlFormatter
    */
   char *FormatXHTMLWapLinkLC(const TDesC& aUrl);

#if 0
   /* Obsolete */
   /**
    * Launch the MoveWayfinder application.
    */
   void LaunchMoveWayfinder();

   /**
    * Launch the GetWayfinder application.
    */
   void LaunchGetWayfinder();
#endif

   /**
    * Updates the contextpane during a wait for a reply,
    * called when the timer expires.
    */
   TBool CheckWait();

   /**
    * Request the settings.
    */
   void RequestSettingsL();

   /**
    * Switch to the settings view.
    */
   void SwitchToSettingsL();

   /**
    * Show the about dialog
    */
   void ShowAboutL();

   /**
    * Show the fleet information dialog
    */
   void ShowFleetInfoL();

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

public:
   TBool DontHandleAsterisk();

   void NetworkInfoDone();
   void NetworkInfoInUse();
   void NetworkInfoFailed(TInt aError);

   /**
    * Delete the iap.txt file.
    */
   void DeleteIapTxt();

private:
   void HandleReportEventMenu(TInt command);
   void ShowReportEventMenu();
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
   void UpdateParametersL( class isab::GenericGuiMess* aMessage );

   /**
    * Handle a top region reply message.
    * @param aMessage a top region reply message
    */
   void HandleTopRegionReplyL(class isab::GetTopRegionReplyMess* aMessage );

   /**
    * Handle a simple paramater reply message.
    * @param aMessage a simple paramater reply message
    */
   void HandleSetSimpleParametersL(class isab::SimpleParameterMess* aMessage );

   /**
    * Handle a search reply.
    * @param aSearchReply a packet containing the reply
    */
   void HandleSearchReplyL(class isab::SearchAreaReplyMess* aSearchAreas );
   void HandleSearchReplyL(class isab::SearchItemReplyMess* aSearchItems );

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

   void HandleSetGeneralParameter(class isab::GeneralParameterMess* message );
   void HandleGetGeneralParameter(class isab::GeneralParameterMess* message );

   void HandleLatestShowNewsChecksum(uint32 checksum);
   void HandleLatestNewsChecksum(uint32 checksum);
   void CheckLatestNewsChecksum();

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
    * Checks if we are ready to use the gps.
    * Send registration sms is hooked in 
    * before this function, the code in
    * this function was eariler located in
    * GotoStartViewL().
    */
   void CheckReadyToUseGps();


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

   void GotoLastView();

public: // Functions from base classes

   // from WayfinderSymbianUiBase
   virtual void StartIapSearch();
   // from WayfinderSymbianUiBase
   virtual void RestartIapSearch();
   // from WayfinderSymbianUiBase
   virtual void ShutdownNow();
   // from WayfinderSymbianUiBase
   virtual void GotoMainMenu();
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

   isab::GuiProtEnums::YesNoAsk SaveSmsDestination() const
   {
      return iSaveSmsDestination;
   }

   isab::GuiProtEnums::YesNoAsk KeepSmsDestination() const
   {
      return iKeepSmsDestination;
   }

   inline class CCoeEnv* CoeEnv() const { return iCoeEnv; }

   class CMyDestView* MyDestView() const { return iMyDestView; }

   enum isab::Quality GpsQuality() const { return iGpsQuality; }   

   class CConnectData* getConnectData() const { return m_connectData; }

/*    class CNav2UiProtHandler* GetNav2UiProtHandler() { return NULL; } */

private:

   void AddSfdCacheFiles(MapLib * mapLib);
   int getMemCacheSize() const;

   /**
    * Checks if any of the different vicinity views are active,
    * @param aActiveView Will point to the active VicinityView.
    * @return ETrue if any of the VicinityViews are active.
    *         EFalse if none of the VicinityViews are active.
    */
   TBool VicinityViewActive(class CVicinityView*& aActiveView);
   
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

   /** From MLineStateListener.
    * Receives status changes of the phone line.
    */
   virtual void LineStatusChanged( const TLineState aLineState );

   /** From MLineStateListener.
    * Receives errors when trying to make a phone call.
    */
   virtual void CallError( const TCallError aCallError );

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

   /**
    *  From CEikAppUi, handles a message sent via the application
    *  framework.
    *  @param aUid Uid of the message
    *  @param aParams The message parameters
    */
   void ProcessMessageL( TUid aUid, const TDesC8& aParams );

   /**
    *  From CEikAppUi, Processes shell commands. 
    *  The default implementation of this function returns whether
    *  or not the file aDocumentName exists, and does nothing else.
    *  The Uikon application framework calls the three-argument form 
    *  when an application is started by selecting a file from the shell 
    *  or by issuing a Create new file command.
    *
    *  @param TApaCommand aCommand  The shell command sent to the application. 
    *  @param TFileName& aDocumentName  At call time, the name of the document as 
    *           specified on the command line. On return, the name for the file 
    *           which will be created by the calling framework.
    *  @param const TDesC8& aTail  Command line tail. 
    *  @return TBool  Whether the final value of aDocumentName represents 
    *                 an existing file
    */
   TBool ProcessCommandParametersL( enum TApaCommand aCommand,
                                    TFileName& aDocumentName,
                                    const TDesC8& aTail );





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

   void DisplayWhereAmIL();
   
   FeedSettings::OutputFormat
   getFeedOutputFormat();
   
   MC2Direction::RepresentationType
   getDirectionType();
   
   enum TWayfinderEvent {
#if 0
      EWayfinderEventUSAWarningDialogOk            = 0,
      EWayfinderEventUSAWarningDialogCancel        = 1,
#endif
      EWayfinderEventConnectToGps                  = 2,
      EWayfinderEventReportEventListDialogOk       = 3,
      EWayfinderEventReportEventListDialogCancel   = 4,
      EWayfinderEventInboxSmsSelected              = 5,
      EWayfinderEventInboxSmsConfirmed             = 6,
      EWayfinderEventShutdownNow                   = 7,
      EWayfinderEventStartupFailedOk               = 8,
      EWayfinderEventStartupFailedCancel           = 9,
      EWayfinderEventBuyExtension                  = 10,
      EWayfinderEventCheckNetwork                  = 11,
      EWayfinderEventDoNothing                     = 12,
    };
   
   void GenerateEvent(enum TWayfinderEvent aEvent);
   void HandleGeneratedEventL(enum TWayfinderEvent aEvent);

   void ShowUSWarningPopup();
   void SendSMSReportEvent(TInt aId);

   void CheckNetwork();

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
   
private: //Data
   class CWfAkmObserver*                  iWfAkmObserver;
   //
   // 
   /// The default navi pane control.
   class CAknNavigationControlContainer*  iNaviPane;

   TBool iNotifyGpsUseless;
   TInt  iNotifyGpsCounter;

   typedef std::map<TUint, TInt> HotKeyMap;
   HotKeyMap iHotkeyMap;

   void SetupHotkeys();
   TKeyResponse HandleHotKeyEventL(const struct TKeyEvent& aKeyEvent,
                                    enum TEventCode aType);
   
   // Unified detail fetcher
   DetailFetcher* m_detailFetcher;

   // Unified favorite fetcher
   FavoriteFetcher* m_favoriteFetcher;
   
   TInt iUnitSpeed;
   
   /// Contains handles to the tab pages.
   class CAknTabGroup*                    iTabGroup;
   
   /// Pointer to the navigation decorator.
   class CAknNavigationDecorator*         iDecoratedTabGroup;

   /// Pointer to a temporary navigation decorator.
   class CAknNavigationDecorator*         iTempDecorator;

   /// Handle to the context pane.
   class CAknContextPane*                 iContextPane;
   class CContextPaneAnimator*            iContextPaneAnimator;

   /// Handles sending and receiving messages from NavCtrl.
   class CMessageHandler*                 iMessageHandler;

   class CStartPageView*                  iStartView;
   class CWelcomeView*                    iWelcomeView;

   /// The origin destination select view
   class CPositionSelectView*             iSelectView;

   /// The New Destinatiom page.
   class CNewDestView*                    iNewDestView;

   /// The My Destination page.
   class CMyDestView*                     iMyDestView;

   /// The Guide page.
   class CGuideView*                      iGuideView;

   /// The Itinerary View
   class CItineraryView*                  iItineraryView;

   /// The Map page
   class CMapView*                        iMapView;

   class MapLib*                          iMapLib;
   
   /// The Route page.
   class CRouteView*                      iRouteView;

   /// The Information page.
   class CInfoView*                       iInfoView;

   /// The Service window page.
   class CServiceWindowView*              iServiceWindowView;

   /// The Settings view
   class CSettingsView*                   iSettingsView;

	// The Vicinity view
	class CVicinityView * 						iVicinityViewAll;

   // The Vicinity Feed view
	class CVicinityFeedView *              iVicinityFeedView;
   
   // The Locked Navigation View
   class CLockedNavigationView*           iLockedNavView;
   
	// The Vicinity view
	class CVicinityView * 						iVicinityViewCrossing;
   
	// The Vicinity view
	class CVicinityView * 						iVicinityViewFavourite;
   
	// The Vicinity view
	class CVicinityView * 						iVicinityViewPOI;

   /// Dialog for entering the gold registration code
   class CUpgradeDialog*                  iUpgradeDialog;

#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3 
   class CSimpleConnectionManager* iConnMgr;
#endif     

   /// Object that handles sound.
   class CSlaveAudio*                     iAudioSlave;

   /// Object that holds all data.
   class CDataHolder*                     iDataHolder;

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

   TInt32 iLastViewId;

   class CHelpUrlHandler* iHelpUrlHandler;

private:
   /// The current position.
   class TPoint                           iCurrPos;
   class TPoint                           iCurrentDestination;
   class TPoint                           iCurrentOrigin;

   int iDirection;
   
   HBufC*                        iCurrentDestinationName;

   /// Force an update of the guide view.
   TBool                            iForceUpdate;

   /// True if waiting for a reply.
   TBool                            iWaiting;

   /// True if synchronizing favorites.
   TBool                            iSynchronizing;

   /// Timer active object
   class CTimeOutTimer*                   iTimer;

   /// 0 save destination, 1 don't save,2 ask.
   enum isab::GuiProtEnums::YesNoAsk           iSaveSmsDestination;

   /// True if automaticaly route to the sms destination. 
   TBool                            iAutoRouteSmsDestination;

   /// 0 keep sms in inbox, 1 don't keep,2 ask.
   enum isab::GuiProtEnums::YesNoAsk           iKeepSmsDestination;

   /// 0 only on keypress, 1 only during route, 2 backlight always on
   enum isab::GuiProtEnums::BacklightStrategy  iBacklightStrategy;
   
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

   /// Pointer to the connect dialog.
   class CConnectDialog* iConnectDialog;
   class CConnectData* m_connectData;

   class CIAPSearcher_old* iIAPSearcher;

   MapResourceFactory* iMrFactory;
#ifdef RELEASE_CELL
public:
   /**
    * Gets pointer to cell mapper.
    * @return cell mapper.
    */
   CCellMapper* getCellMapper() const {return iCC;}
private:
   CCellMapper* iCC;

   friend class CCellMapper;
#endif

public:
   MapResourceFactory* GetMapResourceFactory();

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
   int32 m_languageCode;
   char *m_languageIsoTwoChar;
   TBool m_hasSetLanguage;

   TInt iMemCheckInterval;

   class CCommunicateHelper* iCommunicateHelper;

   TUid iUpgradeFromView;

public:
   TBool m_XXX_settings_gotten;

   class CSettingsData* iSettingsData;

   class CGpsSocket* iGpsSocket;
   class CNokiaLbsReader* iPositionProvider;

   class CGuidePicture* iStartupImage;
   enum isab::GuiProtEnums::ChooseHomeServer m_defaultServerMode;


   TBool iGpsAutoConnectAfterStartup;
   
   TBool iShowNowGpsWarn;

   TBool iDoingShutdown;

   /// True if there is a phone call in progress
   bool m_phoneCallInProgress;

   HBufC* iWfTypeStr;
   HBufC* iWfOptsStr;
   TInt iEventSMSid;
   HBufC* iEventSmsErrTxt;

   class CNetworkInfoHandler* iNetworkInfoHandler;

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

   typedef CImeiFetcher<CWayFinderAppUi> CWayfinderImeiFetcher;
   CWayfinderImeiFetcher* iImeiFetcher;

   TBool iImeiReceived;
#if defined NAV2_CLIENT_SERIES60_V3
   class CSymbian9CallHandler* iCallHandler;
#endif

   class CImageHandler* iImageHandler;

   class GuiProtMessageHandler* iGuiProtHandler;
   TBool iMessagesReceived;

   class CProgressDlg* iProgressDlg;

   class CWayfinderDataHolder* iDataStore;
   class ErrorFilter* iErrorFilter;
   class isab::OfflineTester* iOfflineTester;
};

#endif


// End of File
