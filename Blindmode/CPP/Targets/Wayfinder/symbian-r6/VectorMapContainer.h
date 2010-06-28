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
                            public TileMapInfoCallback
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
                    class CTileMapControl* aMapControl,
                    class CMapFeatureHolder* aFeatureHolder,
                    class CMapInfoControl* aMapInfoControl,
                    class CVectorMapConnection* aVectorMapConnection,
                    const TDesC& aResourcePath);

   /**
    * Destructor.
    */
   ~CVectorMapContainer();
public:
   TBool ShowingInfoText();
public:
   ///@name Functions from Symbian base classes
   ///@name From CCoeControl
   //@{
   TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

   void HandlePointerEventL( const TPointerEvent& aPointerEvent );
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

   void SetGpsStatusImage(CFbsBitmap* aGpsStatusBitmap,
                          CFbsBitmap* aGpsStatusMask);
   /*
   void SetTurnImage(CFbsBitmap* aTurnBitmap,
                     CFbsBitmap* aTurnMask);
   */
   void SetTopBorder();
   void HideTopBorder();

   void SetTurnPictureL( TInt aMbmIndex );

   /*void SetTurnPicture( TPictures aTurn,
                        TDesC &aFullPath,
                        TBool aRightTraffic );*/

   //void HideTurnPicture();

   void SetDetourPicture(TInt on);
   void SetSpeedCamPicture(TInt on);
   
   void SetDistanceL( TUint aDistance );
   
   void Connect();

   void UpdateRepaint();

   void SetFullScreen( TBool aFullScreen );

   TBool IsFullScreen();

   void SetCenter( TInt32 aLat, TInt32 aLon );

   void SetRotation( TInt aHeading );

   void ResetRotation();

   void RequestMarkedPositionMap( TPoint aPos );

   void RequestPositionMap( TPoint aPos );

   void SetRoute( TInt64 aRouteId );
   void ClearRoute( );

   void RequestRouteMap( TPoint aTl, TPoint aBr );

   void SetZoom( TInt aScale );
   void ZoomToOverview();

   void CheckFavoriteRedraw();

   void ZoomIn();

   void ZoomOut();

   void ShowUserPos( TBool aShow );

   void ResetNorthArrowPos();
   void ShowNorthArrow( TBool aShow );
   void ShowScale( TBool aShow );

   void ShowStart( TBool aShow, TInt32 aLat, TInt32 aLon );
   void ShowEnd( TBool aShow, TInt32 aLat, TInt32 aLon );
   void ShowPoint( TBool aShow, TInt32 aLat, TInt32 aLon );

   void GetCoordinate( TPoint& aRealCoord );
   void GetCoordinate( MC2Coordinate& aMC2Coord );

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

   const unsigned char* GetFeatureName();

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
   void setTrackingMode( TBool aOnOff, TrackingType aType );

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
   bool setGpsPos( const MC2Coordinate& coord, int direction, int scale );

   /**
    * Set iMapControl and others to NULL since it is deleted by MapView.
    */
   void SetMapControlsNull();

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


   void BindMovementKeys(TBool aLandscapeMode);


private: //data

   class CMapView* iView;

	class CWayFinderAppUi * iWayFinderUI;

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

   VisibilityAdapter<class CMapBorderBar>* iTopBorderVisAdapter;
   VisibilityAdapter<class CBitmapControl>* iGpsStatusVisAdapter;
   VisibilityAdapter<class CBitmapControl>* iConStatusVisAdapter;

   class CursorVisibilityAdapter* iCursorVisAdapter;

   //True if the details window is showing
   TBool iShowingDetails;

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

   /// Flag for full screen mode
   TBool iIsFullScreen;

   /// the movement, rotation and zoom delta values
   int32 iMovDelta;
   int32 iRotDelta;
   float32 iZoomInFactor;
   float32 iZoomOutFactor;
   
   class CMapFeatureHolder* iFeatureHolder;

   TInt m_timerPeriod;

#ifdef NAV2_CLIENT_SERIES60_V3
   VisibilityAdapter<class CImageHandler>* iDetourPictureVisAdapter;
   VisibilityAdapter<class CImageHandler>* iSpeedCamPictureVisAdapter;
#else
   VisibilityAdapter<class CGuidePicture>* iDetourPictureVisAdapter;
   VisibilityAdapter<class CGuidePicture>* iSpeedCamPictureVisAdapter;
#endif

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
};

#endif

// End of File
