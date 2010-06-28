/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MAPVIEW_H
#define MAPVIEW_H

// INCLUDES
#include <aknwaitdialog.h>
#include <badesca.h>
//#include "MapContainer.h"
#include "ViewBase.h"
#include "WayFinderConstants.h" 
#include "Log.h"

#include "MapEnums.h"
#include "RouteEnums.h"
#include "NavServerComEnums.h"
#include <deque>
#include <vector>

#include "EventGenerator.h"
#include "TileMapEventListener.h"
#include "TurnPictures.h"
#include "MapFileFinder.h"
#include "MapViewEvent.h"
#include "S60WaitDialogObserver.h"
#include "STLUtility.h"
#include "InterpolationCallback.h"
#include "HintNode.h"
// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CAknWaitDialog;
class CVectorMapContainer;
class CVectorMapConnection;
class CTileMapControl;
class GuiDataStore;
class CMapFeatureHolder;
class TileCategory;
class SharedBuffer;

namespace isab{
   class Buffer;
   class MapReplyMess;
   class UpdatePositionMess;
   class DataGuiMess;
   class Favorite;
   class GetFavoritesAllDataReplyMess;
   class GetFavoriteInfoReplyMess;
   class FullSearchDataReplyMess;
   class SearchRegion;
   class AdditionalInfo;
}

typedef std::vector<const TileCategory*> tc_vector_type_base;
typedef tc_vector_type_base* tc_vector_type;

// CLASS DECLARATION

/**
 *  CMapView view class.
 * 
 */
class CMapView :
   public CViewBase,
   public MProgressDialogCallback,
   public TileMapEventListener,
   public MWaitDialogObserver,
   public InterpolationCallback
{
public: // Constructors and destructor

   // Typedef for the action callback
   typedef DoCallback<CMapView>::Action MapViewCallback;
   // Typedef for the vector containing the function pointers
   typedef std::vector<MapViewCallback> ptmf_vector_type;
 
   CMapView(class CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * EPOC default constructor.
    */
   void ConstructL();

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CMapView object.
   ///        Note that this object is still on the CleanupStack.
   static class CMapView* NewLC(CWayFinderAppUi* aUi, isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CMapView object.
   static class CMapView* NewL(CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CMapView();

public: // New Funtions

   /**
    *   Tries to add the precached maps to the vector map cache
    *   pack. If no username has been received yet (from nav2/server)
    *   then it will not be added.
    */
   void tryToAddVectorPrecache();
   
   void ReleaseMapControls();

   /**
    * Removes and deletes vector map container. Used to be able to do a clean 
    * shutdown from the framework (using the phones task manager).
    */
   void DestroyVectorMapContainer();

   void SetVectorMapCoordinates(int32 scale, int32 lat, int32 lon);
   void SaveVectorMapCoordinates(int32 scale, int32 lat, int32 lon);
   const std::vector<const TileCategory*>* GetPoiCategories();
   void ConvertCategoriesParamsToSettings( isab::Buffer* buf,
         class CSettingsData* aSettingsData);
   void ConvertCategoriesSettingsToTileCategories(isab::Buffer* buf,
         class CSettingsData* aSettingsData);
   void ConvertMapLayerSettingsToNav2Parameter(
         class TileMapLayerInfoVector* aParamVector,
         class CSettingsData* aSettingsData);
   void ConvertMapLayerSettingsToLayerInfo(
         uint8 *data, int32 len,
         class CSettingsData* aSettingsData);
   void SaveMapLayerSettings(class CSettingsData* aSettingsData);

   /**
    * Sets the acp setting in maplib.
    * @param aEnableACP, if 0 the acp is set to minimzed,
    *                    if 1 the acp is set to normal.
    */
   void SaveACPSettings(int32 aEnableACP);

   void ReleaseInfoText();

   class CSettingsData* GetSettingsData();

   void ToggleSoftKeys(TBool on);

   TBool DontHandleAsterisk();
   void Call();

   void GetDetails( const char* aItemId, TBool moreDetails = EFalse);

   isab::FullSearchDataReplyMess* GetDetailsFindCached( const char* aItemId );
   void ShowDetailsL( isab::FullSearchDataReplyMess* aMessage );

   void HandleFavoritesAllDataReply( isab::GetFavoritesAllDataReplyMess* aMessage );

   void HandleFavoriteInfoReply( isab::GetFavoriteInfoReplyMess* aMessage );
   void RequestFavorite(uint32 id);
   void FavoriteChanged();
   void NewRouteDownloaded();
   void ClearRoute();
   void HandleRouteTooLongL();


   TPtrC GetMbmName();
   TPtrC GetMapCachePath();
   TPtrC GetCommonDataPath();
   TPtrC GetWritableAutoMapCachePath();

   //class CDistanceBitmapHelper* GetOdometerFontCache();

   void GpsStatus(TBool on);
   TBool SetConStatusImage(class CFbsBitmap* bmp, class CFbsBitmap* mask);

   /**
    * Sets the gps status image base in image ids from a mbm file
    * 
    * @param aImageId, the id of the image.
    * @param aImageMaskId, the id of the mask.
    */
   void SetGpsStatusImage(TInt aImageId, TInt aImageMaskId);

   /**
    * Hides or shows the gps status indicator.
    * aVisible Pass ETrue and the indicator will be shown, otherwise it will be hidden.
    */
   void MakeGpsIndicatorVisible(bool aVisible);

   TBool UseTrackingOnAuto();

   /**
    * Set the current position.
    * @param aDataReply contains the position and heading
    * @param aSpeed The unconverted speed.
    * @param aUnitSpeed The speed converted to the current unit settings. 
    */
   void UpdateDataL( isab::UpdatePositionMess* aPositionMess, TInt aSpeed, TInt aUnitSpeed );

   int GetZoomScale( TInt aSpeed );

   void HideUserPosition();

   void UpdatePicture( isab::RouteEnums::RouteAction aAction,
         isab::RouteEnums::RouteCrossing aCrossing,
         TUint &aDistance, TBool aLeftSideTraffic,
         TInt aExitCount, TBool aHighway,
         TInt detour, TInt speedcam);

   /**
    * Does appropriates updates for offtrack status.
    * @param aStatus the ontrack status.
    * @param aDistance the current distance to target.
    */
   void UpdateStatus( isab::RouteEnums::OnTrackEnum aStatus, TUint &aDistance );
   
   void UpdateDistance( TUint aDistance );
   void UpdateDetour( TUint aDetour );
   void UpdateSpeedcam( TUint aSpeedcam );

   /**
    * Updates estimated time to goal during navigation.
    * 
    * @aETG, the estimated time to goal in seconds
    */
   void UpdateETGL(TInt aETG);

   /**
    * Updates the container with the new next street.
    * 
    * @param aNextStreet, the new next street.
    */
   void UpdateNextStreetL(const TDesC& aNextStreet);
   
   TInt GetDistanceMode();
   void SetDistanceMode(TInt mode);

   void SetDestination( int32 aLat, int32 aLon );

   void SetTopBorder();

   /**
    * Hides and displayes the next street control
    * 
    * @param aMakeVisible, true if to display the control.
    */
   void NextStreetCtrlMakeVisible(TBool aMakeVisible);

   void SetPicture();

   GuiDataStore* CMapView::GetGuiDataStore();

   void GetBaseDirectory( TDes& baseName);
   void GetTempBaseDirectory( TDes& baseName );

   /**
    * Send a request for a map.
    */
   void RequestMap( isab::MapEnums::MapSubject aMapType,
                    int32 aPosLat = MAX_INT32, 
                    int32 aPosLon = MAX_INT32,
                    int32 aMaxLat = MAX_INT32, 
                    int32 aMinLon = MAX_INT32,
                    int32 aMinLat = MAX_INT32,
                    int32 aMaxLon = MAX_INT32 );

   void HandleMultiVectorMapReply( const isab::DataGuiMess* aMessage );

   void HandleVectorMapErrorReply( uint16 aRequestId );

   TBool IsGpsAllowed();
   TBool IsGpsConnected();
   TBool IsGpsConnectedOrSearching();

   TBool IsReleaseVersion();
   TBool IsIronVersion();

   TBool IsTracking();

   TBool IsSimulating();


   void Debug(TDesC& a);

   TBool VectorMaps();

   void SetMapRequestData( isab::MapEnums::MapSubject aMapType,
                           int32 aPosLat = MAX_INT32, 
                           int32 aPosLon = MAX_INT32 );

   void SetMapFeatureName( const unsigned char* aName );

   void GetMapFeatureName( TDes &aName );

   TBool SettingOrigin();

   TBool SettingDestination();

   void SetUseVectorMaps( TBool aUse );

   void SetMapCacheSize( TInt aSize );

   TInt GetMapCacheSize();

   void ClearMapCache();

   void SetTracking(TBool aOnOff);

   void SetTrackingType( TrackingType aType );

   TrackingType GetTrackingType();
   
   // Show a wait symbol for the vector maps.
   void ShowVectorMapWaitSymbol( bool start );

   void SetInfoText( const TDesC & text, TBool persistant = EFalse );
   void SetInfoText( TInt aResourceId, TBool persistant = EFalse );

   void ShowInstalledMaps();
   void SearchForMapFiles(TBool aDoSilentMapFilesSearch = EFalse);
   void AddSearchDir(CDesCArray* aDirList, 
                     const TDesC& aBasPath,
                     const TDesC& aSecondLevelPath);
   void DeleteMapFiles();
   void DeleteMapFileControlName();
   
   // Callback from CMapFileFinder
   void MapFileFinderBusy();
   void NbrMapFilesFound(TInt aCount);
   void FindAndMoveFilesCompleted(TInt aNbrFoundFiles);
   void DeleteFileCompleted(TInt aResult);
   void CopyFileCompleted(TInt aResult);
   void RequestAborted();

   // EventGenerator
   void GenerateEvent(enum TMapViewEvent aEvent);
   void HandleGeneratedEventL(enum TMapViewEvent aEvent);

   int32 GetScale();
   TPoint GetCenter();

   /**
    * Turns night mode on or off depending on aNightMode.
    * @param aNightMode True if nightmode should be switched on
    *                   False if nightmode should be switched off.
    */
   void SetNightModeL(TBool aNightMode);

   /**
    * True if in night mode, false if not.
    */   
   TBool IsNightMode();

   /**
    * Turns 3d mode on or off.
    */
   void Set3dMode(TBool aOn);

   /**
    * True if in 3d mode, false if not.
    */   
   TBool Get3dMode();

   /**
    * Returns true if we want the map in navigation map mode.
    */
   TBool MapAsGeneralNavigation();

   /**
    * Returns true if we want the map in navigation map mode for cars.
    */
   TBool MapAsCarNavigation();

   /**
    * Returns true if we want the map in navigation map mode for pedestrians.
    */
   TBool MapAsPedestrianNavigation();

   /**
    * Returns true if we currently have a route loaded.
    */
   TBool HasRoute();

   /** 
    * Returns current cursor position.
    */
   void GetCoordinate(TPoint& aRealCoord);

   TInt& GetDpiCorrFact();

   isab::NavServerComEnums::languageCode GetLanguageCode();

   /**
    * Sets the current route id, if VectorMapContainer
    * isnt fully created the request will be cached
    * and performed when activating the view the next
    * time. 
    */
   void SetRouteIdL(TInt64 aRouteId);

   /**
    * Sets the current route id to the VectorMapContainer.
    * This function is used when the call to the 
    * SetRouteIdL(TInt64) gets called but the VectorMapContainer
    * isnt fully created so the request gets cached.
    */
   void SetRouteIdL();

   /**
    * A static wrapper as a callback to be able to call the real 
    * NavigateToCallback() function from appui
    * by a function pointer to this wrapper. This is used in the gps wizard 
    * to be able to know if/when lbs connected to the gps so it is okay
    * to order the route.
    * This is necessary since we have different objects with different 
    * member functions that can call the gps wizard.
    */
   static void WrapperToNavigateToCallback(void* pt2Object);

   /*
    * Return a map scale appropriate for a location found by cell id
    * so the circle marking the cell fits (does not overlap) the screen
    *
    * Note: this will need some fine tuning once after testing with real
    * cell info.
    */
   TUint ConvertCellRadiusToMapScale(TUint aRadius);

   /*
    * Display the cell position graphic at aNavCoOrd, size aCellRadius
    */
   void ShowCellIdPosition(const Nav2Coordinate& aNavCoOrd, const TUint aCellRadius);

   /*
    * if the position is known from the cell id display the cell position
    * svg using know position and size
    */
   void ShowCellIdPositionIfKnown();

private: // New Functions
   /**
    *   If we are interpolating positions, and the right conditions
    *   are met, we update the current state in a special way.
    *   
    */ 
   void UpdateDataInterpolatedL( isab::UpdatePositionMess* aPositionMess,
                                 TInt aSpeed );


   /**
    *   Sets up new interpolation data so that interpolated positions
    *   can be calculated.
    */
   
   bool LoadNewInterpolationDataL( isab::UpdatePositionMess* aPositionMess,
                                   TInt aSpeed );
   
   void UpdatePosition( const MC2Coordinate& newPosition,
                        int heading,
                        TInt aSpeed,
                        bool interpolated );

   /**
    * NavigateToCallback() function from called form appui via the 
    * static WrapperToNavigateToCallback() function.
    */
   void NavigateToCallback();

   TBool RequestRouteVectorMap();

   TBool RequestCurrPosVectorMap();

   TBool RequestPositionVectorMap( TPoint aPos );

   TBool RequestOriginVectorMap();

   TBool RequestDestinationVectorMap( TPoint aDest );

   isab::Favorite* CreateFavorite( TUint aId, TInt aLat, TInt aLon,
                                   TDesC &aName, TDesC &aDescription,
                                   const char* aIcon, TBool aHasId, uint32 aLmID, 
                                   const char* aImeiCrcHex, const char* aLmsID  );

   /// Notify that the tracking status has changed.
   void TrackingStatusChanged();
   
   /**
    * Gets the position of the destination in the map and sends a route
    * request.
    */
   void RouteToMapDestinationCoordL();

public: // Functions from base classes

   void handleTileMapEvent(const class TileMapEvent &event);

   virtual void positionInterpolated( MC2Coordinate newPosition,
                                      double velocityMPS,
                                      double headingDegrees );
public: // Functions from base classes

   void DialogDismissedL( TInt aButtonId );

   /**
    * From WayfinderCommandCallback
    */
//   void HandleCommandL(const class CWAXParameterContainer& aCont);

   /**
    * From AknView.
    * @return the ID of the view.
    */
   TUid Id() const;

   /**
    * From AknView, takes care of command handling.
    * @param aCommand command to be handled
    */
   void HandleCommandL(TInt aCommand);

   void HandleResourceChangeL(TInt aType);

public: // From MWaitDialogObserver
   
   void CancelPressed();

   /**
    * From MWaitDialogObserver
    * Notifies it's observer that the framework has closed the 
    * wait dialog. (e.g. a background/foreground switch or screensaver)
    */
   void DialogDismissedFromFramework();

   /**
    * Tells the mapview to preserve the view position.
    * I.e. not reset the visible area to the gps position on the next activation of the view.
    */
   void PreservePositionOnNextActivation();

private: // Functions from base classes

   /**
    * From AknView, Called when the option softkey is pressed.
    * @param aResourceId the focused resource.
    * @param aMenuPane the current menu.
    */
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoActivateL( const TVwsViewId& aPrevViewId,
                     TUid aCustomMessageId,
                     const TDesC8& aCustomMessage);

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoDeactivate();

private: // Data

   /// Current map type
   isab::MapEnums::MapSubject iCurrentMapType;

   /// Current horizontal view offset
//    TInt iOffsetX;

//    /// Current vertical view offset
//    TInt iOffsetY;

   /// Current latitude
   int32 iCurrentLat;

   /// Current longitude
   int32 iCurrentLon;

   /// The latitude of the viewed destination
   int32 iDestinationLat;

   /// The longitude of the viewed destination
   int32 iDestinationLon;

   /// The latitude of the viewed destination
   int32 iRouteDestLat;

   /// The longitude of the viewed destination
   int32 iRouteDestLon;

   /// Minimum latitude of the currently shown map.
   int32 iNewMinLat;

   /// Minimum longitude of the currently shown map.
   int32 iNewMinLon;

   /// Maximum latitude of the currently shown map.
   int32 iNewMaxLat;

   /// Maximum longitude of the currently shown map.
   int32 iNewMaxLon;

//    /// Minimum latitude of the normal view map.
//    int32 iNormalMinLat;

//    /// Minimum longitude of the normal view map.
//    int32 iNormalMinLon;

//    /// Maximum latitude of the normal view map.
//    int32 iNormalMaxLat;

//    /// Maximum longitude of the normal view map.
//    int32 iNormalMaxLon;

   TUint32 iMapRadius;

   TInt iTrackRequestCounter;

   TBool iLeftSideTraffic;

   TBool iMapRequested;

   TBool iRouteRequested;

   TBool iIsTracking;

   CAknWaitDialog* iWaitDlg;

   TBool iIsWaiting;

   TBool iWaitCanceled;

   isab::Log* iLog;

   TBuf<256> iFullScreenMapName;

   TBuf<256> iNormalScreenMapName;

   TPictures iCurrentTurn;

   TInt iExitCount;
   TInt iExitCountMask;

   TBool iGotMapType;

   TInt64 m_routeid;

   CVectorMapConnection* iVectorMapConnection;
   
   CVectorMapContainer* iVectorMapContainer;

   CTileMapControl* iMapControl;

   /// Control for viewing feature map info (the blue note) for vector maps.
   class CMapInfoControl* m_mapInfoControl;
   
   CMapFeatureHolder *iMapFeatureHolder;
   
   isab::MapEnums::MapSubject iRequestType;
   
   TPoint iRequestCoord;
   TPoint iCenter;
   int32 iCurrentScale;

   TBuf<64> iFeatureName;

   TBool iSetOrigin;
   TBool iSetDestination;

   int32 iCustomMessage;

   TInt iMapCacheSize;
   
   TrackingType iTrackingType;

   TInt  iLastDistance;

   char *iCurrentGetDetailsName;
   char *iCurrentGetDetailsId;

   /// The xor buffer used to decrypt the tilemapcache.
   SharedBuffer* m_xorBuffer;
   /// The xor buffer used to decrypt the warez tilemapcache.
   SharedBuffer* m_xorBufferWarez;

   /**
    *  Route maps that have been sent from the server together with the
    *  Nav2-route. Must be saved sometimes since the container may not
    *  have been created when the message is received.
    */
   isab::DataGuiMess* m_forcedRoutes;

   TInt iTemp;

   isab::FullSearchDataReplyMess* iGetDetailsReplyMessage;
   std::deque<std::pair<char *, isab::FullSearchDataReplyMess*> *> iGetDetailsReplyCache;

   //class CDistanceBitmapHelper* iOdometerFontCache;

   TInt iNbrCacheFiles;
   TInt iSelectedMapFile;
   TInt iDpiCorrFact;

   typedef const class MapLib::CacheInfo* const_cacheinfo_p;
   typedef const const_cacheinfo_p* const_cacheinfo_p_p;

   const_cacheinfo_p_p iCacheInfo;
   CDesCArray* iMapFileList;

   typedef CMapFileFinder<CMapView> CMapFinder;
   CMapFinder* iMapFileFinder;

   typedef CEventGenerator<CMapView, enum TMapViewEvent>
      CMapViewEventGenerator;
   CMapViewEventGenerator* iEventGenerator;

   /// True if the precache maps have been added
   int m_precachedCachesAdded;
   /// True if the map file search is done during first startup, 
   /// we then dont want any dialogs except for the wait dialog.
   TBool iDoSilentMapFilesSearch;
   // Vector containing pointers to functions. It is used when fucntions
   // are called but can't be properly executed since VectorMapContainer
   // isnt created yet. An example is the call to NewRouteDownloaded from
   // WayfinderAppUi when a new route has been downloaded. Since the 
   // VectorMapContainer isnt created when this function is called the
   // end and start bitmap wont be cleared which will result in duplicates
   // when MapLib draws its own start and end bitmaps. 
   ptmf_vector_type m_ptmfVec;
   // Contains the next street during navigation, only stored in this
   // variable if the VectorMapContainer isnt created. Unless doing this
   // the next street control will be empty untill user reached next 
   // way point.
   HBufC* iNextStreet;

   uint32 iPrevTimeStampMillis;


   uint32 iPrevFinalizedTime;
   uint32 iPrevFinalizedPeriod;
   uint32 iNumPositionInterpolations;

   TBool iPreservePositionOnNextActivation;
   
};

#endif

// End of File
