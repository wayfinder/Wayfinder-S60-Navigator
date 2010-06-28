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
#include <aknview.h>
#include <aknwaitdialog.h>
#include <badesca.h>
//#include "MapContainer.h"
#include "WayFinderConstants.h" 
#include "Log.h"

#include "MapEnums.h"
#include "RouteEnums.h"
#include <deque>
#include <vector>

#include "EventGenerator.h"
#include "TileMapEventListener.h"
#include "TurnPictures.h"
#include "MapFileFinder.h"
#include "MapViewEvent.h"
#include "WayfinderCommandCallback.h"
#include "DetailFetcher.h"
#include "FavoriteFetcher.h"

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
class CMapInfoControl;

namespace isab{
   class Buffer;
   class MapReplyMess;
   class UpdatePositionMess;
   class DataGuiMess;
   class Favorite;
   class GetFavoritesAllDataReplyMess;
   class GetFavoriteInfoReplyMess;
   class FullSearchDataReplyMess;
}

typedef std::vector<const TileCategory*> tc_vector_type_base;
typedef tc_vector_type_base* tc_vector_type;

// CLASS DECLARATION

/**
 *  CMapView view class.
 * 
 */
class CMapView :
   public CAknView,
   public MProgressDialogCallback,
   public TileMapEventListener,
   public MWayfinderCommandCallback,
   public DetailFetcherCallback,
   public FavoriteRequester
{
public: // Constructors and destructor

   CMapView(isab::Log* aLog);

   /**
    * EPOC default constructor.
    */
   void ConstructL( CWayFinderAppUi* aWayFinderUI );

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CMapView object.
   ///        Note that this object is still on the CleanupStack.
   static class CMapView* NewLC(CWayFinderAppUi* aUi,
                                isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CMapView object.
   static class CMapView* NewL(CWayFinderAppUi* aUi,
                               isab::Log* aLog);

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

   class CSettingsData* GetSettingsData();

   void ToggleSoftKeys(TBool on);

   TBool DontHandleAsterisk();
   void Call();
   
   void GetAndShowDetails( const char* aItemId,
                           const char* aItemName);
   
   void ShowDetailsL( isab::FullSearchItem* aMessage );

   void SetDetailFetcher(DetailFetcher* detailFetcher );
   
   void HandleFavoritesAllDataReply( isab::GetFavoritesAllDataReplyMess* aMessage );

   void HandleFavoriteInfoReply( isab::GetFavoriteInfoReplyMess* aMessage );
   void FavoriteChanged();
   void NewRouteDownloaded();
   void ClearRoute();

   TPtrC GetMbmName();
   const TInt* GetMbmfileIds();
   TPtrC GetMapCachePath();
   TPtrC GetCommonDataPath();
#ifdef NAV2_CLIENT_SERIES60_V3
   const TReal* GetMbmFileRelations();
#endif

   //class CDistanceBitmapHelper* GetOdometerFontCache();

   void GpsStatus(TBool on);
   TBool SetConStatusImage(class CFbsBitmap* bmp, class CFbsBitmap* mask);
   void SetGpsStatusImage(class CFbsBitmap* bmp, class CFbsBitmap* mask);
   TBool UseTrackingOnAuto();

   /**
    * Set the current position.
    * @param aDataReply contains the position and heading
    */
   void UpdateDataL( isab::UpdatePositionMess* aPositionMess, TInt aSpeed );

   int GetZoomScale( TInt aSpeed );

   void HideUserPosition();

   void UpdatePicture( isab::RouteEnums::RouteAction aAction,
         isab::RouteEnums::RouteCrossing aCrossing,
         TUint &aDistance, TBool aLeftSideTraffic,
         TBool aHighway,
         TInt detour, TInt speedcam);

   void UpdateDistance( TUint aDistance );
   void UpdateDetour( TUint aDetour );
   void UpdateSpeedcam( TUint aSpeedcam );
   
   TInt GetDistanceMode();
   void SetDistanceMode(TInt mode);

   void SetDestination( int32 aLat, int32 aLon );

   void SetTopBorder();
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

   TBool IsReleaseVersion();
   TBool IsIronVersion();

   TBool IsTracking();

   TBool IsSimulating();


   void Debug(TDesC& a);

   TBool VectorMaps();

   void SetMapRequestData( isab::MapEnums::MapSubject aMapType,
                           int32 aPosLat = MAX_INT32, 
                           int32 aPosLon = MAX_INT32 );

   void FetchCurrentMapFeatureName();
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
   void SearchForMapFiles();
   void AddSearchDir(CDesCArray* aDirList, 
                     const TDesC& aBasPath,
                     const TDesC& aSecondLevelPath);
   void DeleteMapFiles();
   void DeleteMapFileControlName();
   
   // Callback from CMapFileFinder
   void MapFileFinderBusy();
   void FindAndMoveFilesCompleted(TInt aNbrFoundFiles);
   void DeleteFileCompleted(TInt aResult);
   void CopyFileCompleted(TInt aResult);
   void RequestAborted();

   // EventGenerator
   void GenerateEvent(enum TMapViewEvent aEvent);
   void HandleGeneratedEventL(enum TMapViewEvent aEvent);

   int32 GetScale();
   void SetScale(int32 aScale);

   TPoint GetCenter();
   void SetCenter(int32 aLat, int32 aLon);
   void SetCenter(TPoint center);
   
   class CTileMapControl*       GetMapControl();
   class CMapFeatureHolder*     GetFeatureHolder();
   class CMapInfoControl*       GetMapInfoControl();
   class CVectorMapConnection*  GetVectorMapConnection();
   
private: // New Functions

   char* FormatDetails( isab::FullSearchItem* item,
                        bool& hasSensibleData );

   
   TBool RequestRouteVectorMap();

   TBool RequestCurrPosVectorMap();

   TBool RequestPositionVectorMap( TPoint aPos );

   TBool RequestOriginVectorMap();

   TBool RequestDestinationVectorMap( TPoint aDest );

   void SetFullScreen();

   isab::Favorite* CreateFavorite( TUint aId, TInt aLat, TInt aLon,
                             TDesC &aName, TDesC &aDescription,
                             const char* aIcon, TBool aHasId );

   /// Notify that the tracking status has changed.
   void TrackingStatusChanged();

public: // Functions from base classes

   void handleTileMapEvent(const class TileMapEvent &event);

   virtual void DetailsReceived(isab::GetFavoriteInfoReplyMess* details);
   virtual void DetailsReceived(isab::FullSearchItem* details,
                                bool cachedResult);
public: // Functions from base classes

   void DialogDismissedL( TInt aButtonId );

   /**
    * From MWayfinderCommandCallback
    */
   void HandleCommandL(const class CWAXParameterContainer& aCont);

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
public:
   /// A pointer to the creating UI
   CWayFinderAppUi*   iWayFinderUI;

   void InitResources();
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

   TBool iShowExtendedDetails;

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
   
   /// The xor buffer used to decrypt the tilemapcache.
   SharedBuffer* m_xorBuffer;
   /// The xor buffer used to decrypt the warez tilemapcache.
   SharedBuffer* m_xorBufferWarez;

   DetailFetcher* m_detailFetcher;
   
   /**
    *  Route maps that have been sent from the server together with the
    *  Nav2-route. Must be saved sometimes since the container may not
    *  have been created when the message is received.
    */
   isab::DataGuiMess* m_forcedRoutes;

   TInt iTemp;


   isab::FullSearchItem*          iCurrentDetailsInfo;  

   //class CDistanceBitmapHelper* iOdometerFontCache;

   TInt iNbrCacheFiles;
   TInt iSelectedMapFile;

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

};

#endif

// End of File
