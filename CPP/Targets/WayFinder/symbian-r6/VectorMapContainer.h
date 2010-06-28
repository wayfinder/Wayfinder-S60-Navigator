/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VECTORMAPCONTAINER_H
#define VECTORMAPCONTAINER_H

// INCLUDES
#include <coecntrl.h>

#include <vector>
#include <map>

#ifdef NAV2_CLIENT_SERIES60_V5
# include <aknlongtapdetector.h>
#endif

#include "config.h"

#include "PictureContainer.h"
#include "WayFinderConstants.h"
#include "Log.h"
#include "TileMapToolkit.h"
#include "CursorHolder.h"
#include "MC2Coordinate.h"
#include "TileMapInfoCallback.h"
#include "VisibilityAdapter.h"
#include "ScalableFonts.h"
#include "MapSwitcher.h"
#include "TileMapEventListener.h"

// FORWARD DECLARATIONS 
//Only for types in other namespaces than the global one. Other are
//best declared in place.

namespace isab {
   class DataGuiMess;
}

// CLASS DECLARATION

/**
*  CVectorMapContainer  container control class.
*  
*/
class CVectorMapContainer : public CCoeControl,
                            public MCoeControlObserver,
                            public PictureContainer,
                            public TileMapTimerListener,
                            public CursorHolder,
                            public TileMapInfoCallback,
                            public TileMapEventListener
#ifdef NAV2_CLIENT_SERIES60_V5
                          , public MAknLongTapDetectorCallBack
#endif
{
public: // Constructors and destructor

   /**
    * Constructor.
    */
   CVectorMapContainer( class CMapView* aMapView, isab::Log* aLog );

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( const TRect& aRect,
                    class CWayFinderAppUi* aWayFinderUI,
                    class CTileMapControl** aMapControl,
                    class CMapFeatureHolder** aFeatureHolder,
                    class CMapInfoControl** aMapInfoControl,
                    class CVectorMapConnection** aVectorMapConnection,
                    const TDesC& aResourcePath);

   /**
    * Destructor.
    */
   ~CVectorMapContainer();

public:
   ///@name Functions from Symbian base classes
   ///@name From CCoeControl
   //@{
   TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

   void HandlePointerEventL( const TPointerEvent& aPointerEvent );

   /**
    * Gets called by CAknLongTapDetector (iLongTapDetector) when
    * a long tap has occured. We then launch a CAknStylusPopUpMenu
    * enabling the user to select navigate to, show information etc.
    */
   void HandleLongTapEventL( const TPoint& aPenEventLocation, 
                             const TPoint& aPenEventScreenLocation );

private:
   void SizeChanged();
   TInt CountComponentControls() const;
   class CCoeControl* ComponentControl(TInt aIndex) const;
   void Draw(const TRect& aRect) const;
   //@}
   ///@name From MCoeControlObserver
   //@{
   // event handling section
   // e.g Listbox events
   void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
   TCoeInputCapabilities InputCapabilities() const;

   /**
    * From CCoeControl
    * Handles layout awarness
    */
   void HandleResourceChange(TInt aType);

   //@}
   //@}
public:
   ///@name From PictureContainer
   //@{
   void PictureError( TInt aError );
   void ScalingDone();
   //@}
public: // New functions


   enum ZoomScale{
      EScaleMin   = 1,
      EScale1     = 2,
      EScale2     = 4,
      EScale3     = 10,
      EScale4     = 20,
      EScale5     = 50,
      EScale6     = 100,
      EScale7     = 500,
      EScale8     = 1000,
      EScale9     = 2000,
      EScale10    = 5000,
      EScale11    = 10000,
      EScale12    = 14000, 
      EScaleGlobe = 100000
   };

   /**
    * Returns true if we are done with ConstructL
    */
   TBool ConstructDone();

   void SetConStatusImage(CFbsBitmap* aConStatusBitmap,
                          CFbsBitmap* aConStatusMask);

   /**
    * Sets the gps status image base in image ids from a mbm file
    * 
    * @param aImageId, the id of the image.
    * @param aImageMaskId, the id of the mask.
    */
   void SetGpsStatusImage(TInt aImgId, TInt aImgMaskId);

   /**
    * Hides or shows the gps status indicator.
    * aVisible Pass ETrue and the indicator will be shown, otherwise it will be hidden.
    */
   void MakeGpsIndicatorVisible(bool aVisible);

   /*
   void SetTurnImage(CFbsBitmap* aTurnBitmap,
                     CFbsBitmap* aTurnMask);
   */
   void SetTopBorder();
   void HideTopBorder();

   /**
    * Updates the visibility of the pedestrian mode indicator.
    */
   void UpdatePedestrianModeIndicator();

   /**
    * Hides and displayes the next street control
    * 
    * @param aMakeVisible, true if to display the control.
    */
   void NextStreetCtrlMakeVisible(TBool aMakeVisible);

   void SetTurnPictureL( TInt aMbmIndex, TInt aMbmMaskIndex );

   void SetExitCountPictureL(TInt aMbmIndex, TInt aMbmMaskIndex);

   /*void SetTurnPicture( TPictures aTurn,
                        TDesC &aFullPath,
                        TBool aRightTraffic );*/

   //void HideTurnPicture();

   void SetDetourPicture(TInt on);
   void SetSpeedCamPicture(TInt on);
   
   void SetDistanceL( TUint aDistance );

   /**
    * Sets estimated time to goal during navigation.
    * 
    * @aETG, the estimated time to goal in seconds
    */
   void SetETGL(TInt aETG);

   /**
    * Sets current speed during navigation.
    * 
    * @aSpeed, the current speed
    */
   void SetSpeedL(TInt aSpeed);

   /**
    * Updates the container with the new next street.
    * 
    * @param aNextStreet, the new next street.
    */
   void SetNextStreetL(const TDesC& aNextStreet);
   
   void Connect();

   void UpdateRepaint();

   void SetCenter( TInt32 aLat, TInt32 aLon );

   void SetRotation( TInt aHeading );

   void ResetRotation();

   void RequestMarkedPositionMap( TPoint aPos );

   void RequestPositionMap( TPoint aPos );
   void RequestPositionAndZoomMap( TPoint aPos, TInt aScale );

   void SetRoute( TInt64 aRouteId );
   void ClearRoute( );

   void RequestRouteMap( TPoint aTl, TPoint aBr );

   void SetZoom( TInt aScale );
   void ZoomToOverview();

   void CheckFavoriteRedraw();

   void ZoomIn();
   void ZoomIn3d();
   void ZoomOut();
   void ZoomOut3d();

   void ShowUserPos( TBool aShow );

   void ResetNorthArrowPos();
   void ShowNorthArrow( TBool aShow );
   void ShowScale( TBool aShow );

   void ShowStart( TBool aShow, TInt32 aLat, TInt32 aLon );
   void ShowEnd( TBool aShow, TInt32 aLat, TInt32 aLon );
   void ShowPoint( TBool aShow, TInt32 aLat, TInt32 aLon );

   void GetCoordinate( TPoint& aRealCoord );
   void GetLongPressOrNormalCoordinate( TPoint& aRealCoord );
   void GetCoordinate( MC2Coordinate& aMC2Coord );
   void GetLongPressOrNormalCoordinate( MC2Coordinate& aMC2Coord );

   void ShowFeatureInfo();

   HBufC* GetServerStringL();

   /**
    *    Sets info text.
    *
    *    @param hideAfterMS   If specified, the info text will be shown
    *                         this many milliseconds and then be hidden.
    */
   void setInfoText(const char *txt,
      const char *left = NULL,
      const char *right = NULL,
      TBool persistant = EFalse,
      uint32 hideAfterMS = MAX_UINT32 );

   TBool IsInfoTextOn();

   void ShowDetailedFeatureInfo(TBool showWithList = EFalse);

/*    void ShowPopupListL(); */

   void GetFeatureName();

   /// @return If the cursor should be visible or not.
   bool getCursorVisibility() const;
  
   /// Implements the abstract method from TileMapInfoCallback.
   void notifyInfoNamesAvailable();
   
   /// Check if the cursor should be shown or not.
   void updateCursorVisibility();

   /// Get the active point on the map.
   class MC2Point getActivePoint() const;
   
   TInt GetScale();

   /// Set the tracking mode and type.
   void setTrackingMode( TBool aOnOff, TrackingType aType, bool interpolating );

   // Show a wait symbol for the vector maps.
   void ShowVectorMapWaitSymbol( bool start );

   /// Handle the timers. 
   void timerExpired( uint32 id );
   
   /**
    *    Set a new gps position.
    *
    *    @param   coord       The coordinate of the new gps position.
    *    @param   direction   The angle (360 degrees).
    *    @param   scale       The scalelevel (dependent on the 
    *                         speed).
    *    @return If the map was updated due to the new gps position.
    */
   bool setGpsPos( const MC2Coordinate& coord,
                   int direction,
                   int scale,
                   bool interpolated );

   /**
    * Sets the position of the pedestrian gps indicator.
    */
   //void setPedestrianGpsPos(const MC2Coordinate& coord);

   /**
    * Turns night mode on or off depending on aNightMode.
    * @param aNightMode True if nightmode should be switched on
    *                   False if nightmode should be switched off.
    */
   void SetNightModeL(TBool aNightMode);

   /** 
    * If 3d mode is turned on the bluebar should not be
    * visible, if off it should be visible.
    */
   void Set3dMode(TBool aOn);
 
   /**
    * Set iMapControl and others to NULL since it is deleted by MapView.
    */
   void SetMapControlsNull();

   /**
    * Resets the flag that indicates that the long press position shall be
    * used instead of the position of the cursor.
    * Is used by the mapview to let the vectormapcontainer know when the popup
    * has been closed etc.
    */
   void ResetLongPressPositionFlag();

   void SetCellIdIconEnabled(bool enabled);
   
   void UpdateCellIdIconDimensions();
   void UpdateCellIdIconDimensions(int radiusMeters);

   void UpdateCellIdPosition(const MC2Coordinate& coord);

   /**
    * Releases all the pointers and references to the map control.
    * This should be called either when the mapcontrol is about to be deleted
    * or when the vectormapcontainer itself is removed.
    */
   void ReleaseMapControlDependencies();
   
   /**
    * Calls stop() on TileMapKeyHandler to reset all keys and the timer to
	* prevent repeated keys.
	*/
   void StopKeyHandler();
private: // New functions
   
   /**
    * Get the tracking point, 
    * i.e. the point on the screen of the gps symbol.
    */
   class MC2Point getTrackingPoint() const;
   
   /**
    * Get the active point on the screen when leaving tracking.
    */
   class MC2Point getNonTrackingPoint( bool screenSizeChanged = false ) const;

   /* key timer callback */
   static TInt KeyCallback(TAny* aThisPtr) {
      CVectorMapContainer* thisPtr = (CVectorMapContainer*) aThisPtr;
      thisPtr->TimerHandler();
      return(1);
   }

   /* Timer handler */
   void TimerHandler();

   int getMemCacheSize() const;

   TInt GetLogicalFont(enum TScalableFonts fontId, class TFontSpec& fontSpec);

   /**
    *   Sets the fonts used for text placement in the TileMapHandler.
    */
   void setTileMapFonts();

   /**
    *    Init the keyhandler and cursor.
    */
   void initKeyHandler();

   /**
    *   Called when focus changes.
    */
   void FocusChanged( TDrawNow aDrawNow );

   /**
    *   Is called when class is created, adds all
    *   existing single file caches to the server.
    */
   void AddSfdCacheFiles();
   void BindMovementKeys(TBool aLandscapeMode);

   /**
    * Calculates the positions of the visibility adapters of this container.
    */
   void LayoutVisibilityAdaptersL(const TRect& aRect, MapSwitcher::SwitcherNotice& aTilemapNotice);

   /**
    * Calculates the position of the compass image. The position depends on what
    * controls above it that are visible or not.
    *
    * @aX, [OUT] the x position of the compass
    * @aY, [OUT] the y position of the compass
    */
   void CalcCompassPosition(TInt &aX, TInt &aY);

   /**
    * Positions the icons to the right on the map that are dynamically positioned.
    * The icons that are positioned are: gps indicator, detour and speedcam
    * in that prioritized order with the most important one first.
    */
   void PositionRightEdgeFloatingControls();

   /**
    * Handles events received from the TileMapHandler.
    * @param event The event.
    */
   void handleTileMapEvent(const class TileMapEvent &event);

private: //data

   class CMapView* iView;

   /// Map control
   class CTileMapControl* iMapControl;
   VisibilityAdapter<class CTileMapControl>* iMapControlVisAdapter;

   /// Interfaces to TileMapHandler.
   class MapMovingInterface* iMapMovingInterface;
   class MapDrawingInterface* iMapDrawingInterface;   

   class MapComponentWrapper* iGlobeComponent;
   class CCoeControl* iGlobeControl;
   VisibilityAdapter<class CCoeControl>* iGlobeControlVisAdapter;

   class MapSwitcher* iMapSwitcher;

   /// Top border during navigation, containing next turn picture,
   /// distance to next turn, estimated time to goal, current speed
   /// and status of the gps
   VisibilityAdapter<class CMapTopBorderBar>* iTopBorderVisAdapter;
   /// CCoeControl that displays the next street name during navigation
   /// in a semi transparent rect
   VisibilityAdapter<class CBackGroundTextContainer>* iNextStreetVisAdapter;
   VisibilityAdapter<class CBitmapControl>* iConStatusVisAdapter;

   class CursorVisibilityAdapter* iCursorVisAdapter;

   /// True, if start menu command has been issued
   TBool iIsConnected;

   /// True if we start vectormap container for the first time.
   TBool iSetStartupPos;

   /// Object that handles the key repetitions etc.
   class TileMapKeyHandler* m_keyHandler;
   
   /// Tracking on timer. Formerly key timer.
   CPeriodic* m_autoTrackingOnTimer;

   /// The view rect for normal mode
   TRect iNormalRect;

   /// the movement, rotation and zoom delta values
   int32 iMovDelta;
   int32 iRotDelta;
   float32 iZoomInFactor;
   float32 iZoomOutFactor;
   
   class CMapFeatureHolder* iFeatureHolder;

   TInt m_timerPeriod;

   VisibilityAdapter<class CImageHandler>* iZoomInPictureVisAdapter;
   VisibilityAdapter<class CImageHandler>* iZoomOutPictureVisAdapter;
   
   VisibilityAdapter<class CImageHandler>* iDetourPictureVisAdapter;
   VisibilityAdapter<class CImageHandler>* iSpeedCamPictureVisAdapter;

   VisibilityAdapter<class CImageHandler>* iPedestrianModeVisAdapter;
   VisibilityAdapter<class CPedestrianPositionControl>* iPedestrianPositionVisAdapter;
   VisibilityAdapter<class CImageHandler>* iGpsIndicatorVisAdapter;

   enum {
      /// Maximum length allowed for the text in the distance control.
      KVMCMaxDistLen = 32
   };
   
   isab::Log* iLog;

   /// Keep track of when ConstructL has completed intierly.
   TBool iConstructDone;

   /// If the info text is shown at all.
   TBool iInfoTextShown;
   /// If detailed info text is shown.
   bool m_detailedInfoTextShown;
   TBool iInfoTextPersistant;

   /// The type of the struct used for key bindings.
   typedef struct TKeyBinding {
      /// Symbian key
      int first;
      /// TileMapKeyHandler key.
      int second; 
   } key_array_pair_t;

   /// Unsorted key bindings.
   static const key_array_pair_t c_keyBindings[];
   static const key_array_pair_t c_landscapeKeyBindings[];
   static const key_array_pair_t c_portraitKeyBindings[];
   /// Sorted key bindings.
   std::map<int,int> m_keyMap;

   /// Callback to be able to fixup the favourites etc.
   class CVectorMapContainerKeyHandlerCallBack* m_khCallBack;

   /// The cursor sprite.
   class SpriteMover* m_cursorSprite;
   /// The higlighted cursor sprite.
   class SpriteMover* m_highlightCursorSprite;
   /// The cursor.
   class CursorSprite* m_cursor;

   /// Control for viewing feature map info (the blue note).
   class CMapInfoControl* m_mapInfoControl;

   /// Timer used to hide the blue note with feature info after a while.
   uint32 m_hideInfoTimerID;
   
   /// The map mover.
   class MapMover* m_mapMover;

   TPoint iCenter;
   int32 iScale;

#ifdef NAV2_CLIENT_SERIES60_V5
   /// Handles longtap events, calls HandleLongTapEventL when a longtap event is detected
   class CAknLongTapDetector* iLongTapDetector;
   /// Handles the context sensitiv menu that will be launched on longtap detection
   class CAknStylusPopUpMenu* iPopUpMenu;
#endif
   /// The padding on the sides of the controls visible in the map
   TInt iXPadding;
   /// The padding on top of the controls visible in the map
   TInt iYPadding;
   /// The height of the con status image
   TInt iConStatusImageHeight;
   /// The position of the con status image
   TPoint iConStatusPos;
   /// Keeps track if user clicked on the zoom out button
   TBool iOnZoomOut;
   /// Keeps track if user clicked on the zoom in button
   TBool iOnZoomIn;
   /// Holds the current status of the gps, by remembering this we reduces reloading
   /// of images already loaded
   TInt iGpsImgId;
   /// Keeps track how many drag events we received in a row.
   TInt iNrDragEventsInARow;
   /// Stores the position of the last long press.
   TPoint iLongPressPosition;
   /// Keeps track if the long press position shall be used instead of the cursor.
   TBool iUsingLongPressPosition;
   /// Indicates if we want to update the cursor and info text after the next redraw or not.
   TBool iUpdateCursorAndInfoAtNextRedraw;
   /// The position where the last down pointer event took place.
   TPoint iLastPointerDownEventPosition;
   // The last known cell size radius
   int iRadiusMeters;
   
   class CWayFinderAppUi* iWayFinderUI;
};

#endif

// End of File
