/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <eikapp.h>
#include <eikdoc.h>
#include <eikappui.h>
#include <eikenv.h>
#include <bautils.h>
#include <e32math.h>
#include <e32cmn.h>
#include <aknlists.h>     // for avrell style listbox
#include <aknpopup.h>     // for pop up menu
#ifndef NAV2_CLIENT_SERIES60_V3
 #include <hal.h>
#endif
#include <eiklabel.h>  // for labels
#include <eikedwin.h>  // for labels
#include <aknutils.h>  // for Fonts.

#ifdef NAV2_CLIENT_SERIES60_V5
# include <aknstyluspopupmenu.h>
#endif

#include "wficons.mbg"
#include "InterpolationHintConsumer.h"
#include "RsgInclude.h"
#include "userbitmap.h"
#include "BitmapControl.h"
#include "VectorMapContainer.h"
#include "MapView.h"
#include "TileMapControl.h"
#include "MapRenderer.h"
#include "MapMovingInterface.h"
#include "MapDrawingInterface.h"

#include "GlobeControl.h"
#include "CursorVisibilityAdapter.h"

#include "TileMapKeyHandler.h"
#include "SymbianTilemapToolkit.h"
#include "TileMapUtil.h"
#include "SharedDBufRequester.h"
#include "VectorMapConnection.h"
#include "WayFinderAppUi.h"
#include "GuiProt/GuiProtMess.h"
#include "RouteID.h"
#include "GuidePicture.h"

#include "DistancePrintingPolicy.h"
#include "DistanceBitmap.h"
#include "AnimatorFrame.h"

#include "MapTopBorderBar.h"
#include "BackgroundTextContainer.h"
//#include "PedestrianPositionControl.h"

#include "WFDRMUtil.h"
#include "SymbianTCPConnectionHandler.h"
#include "HttpClientConnection.h"
#include "SharedHttpDBufRequester.h"
#include "DirectedPolygon.h"
#include "MC2Point.h"
#include "UserDefinedBitMapFeature.h"
#include "UserDefinedScaleFeature.h"
#include "SymbianSprite.h"

#include "GuiProt/Favorite.h"

#include "MapFeatureHolder.h"

#include "Dialogs.h"
#include "PopUpList.h"

#include "LogFile.h"

#include "memlog.h"

#include "Log.h"
#include "LogMacros.h"

#include "SettingsData.h"
#include "TileMapTextSettings.h"

#include "MapInfoControl.h"
#include "MapMover.h"

#include "PathFinder.h"
#include "WFTextUtil.h"
#include "StringUtility.h"

#include "WayFinderSettings.h"
#include "ImageHandler.h"
#include "WFLayoutUtils.h"
/* #include "WFTimeUtils.h" */
#include "TileMapEvent.h"

#define ROTATE_NORTH_SCALE    EScale7

using namespace std;
using namespace isab;

/* #define ONLY_LOCAL_TILEMAPS */
#ifndef NAV2_winscw_symbian_series60r3
# define USE_FILE_CACHE
#endif

/* error messages */
_LIT(KMsgGenericMsgTitle, "TMap : ");
_LIT(KMsgGenericErrorTitle, "TMap : Error :");
_LIT(KMsgHandlerError, "Err : TileMapHandler Allocation!");

_LIT(KWfAntiAlias, "_wfantialias");
_LIT(KEmptyDefaultText, "");

/* macros to display above messages */
#define SHOWMSGWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericMsgTitle, x)
#define SHOWMSG(x)      CEikonEnv::Static()->InfoMsgWithDuration(x, 1000000)
#define SHOWERRWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericErrorTitle, x)
#define SHOWERR(x)      CEikonEnv::Static()->InfoMsgWithDuration(x, 2000000)
#define SHOWBUSY(x)     CEikonEnv::Static()->BusyMsgL(x)
#define CANCELBUSY()    CEikonEnv::Static()->BusyMsgCancel()

#define X_PADDING                5
#define Y_PADDING                5
#define ZOOM_BUTTON_SIZE         0.2

#define COMPASS_POLY_HEIGHT      28 // set by arrowPoly in MapFeatureHolder.cpp
#define INFO_IMAGES_SIZE         24
#define BLUE_BAR_HEIGHT          36

#define GPS_POSITION_POLY_SCALE_IN_3D_VIEW   0.5
#define GPS_POSITION_POLY_SCALE_IN_2D_VIEW   1.0
#define PEDESTRIAN_POSITION_POLY_SCALE_BIGGER_SCREENS   0.024
#define PEDESTRIAN_POSITION_POLY_SCALE_SMALLER_SCREENS   0.032

/* How many drag events do we accept for a single tap? */
#define SINGLE_TAP_MAX_NR_DRAG_EVENTS 10
/* How far away from the original down event do we accept the up event to be to still interpret it as a single tap event?*/
#define SINGLE_TAP_MAX_DISTANCE 15

/* general key codes */
#define KEY_PEN_SHIFT_LEFT  EStdKeyLeftShift
#define KEY_ABC_SHIFT_RIGHT EStdKeyRightShift
#define KEY0                0x30
#define KEY1                0x31
#define KEY2                0x32
#define KEY3                0x33
#define KEY4                0x34
#define KEY5                0x35
#define KEY6                0x36
#define KEY7                0x37
#define KEY8                0x38
#define KEY9                0x39
#define KEY_HASH            EStdKeyHash
#ifdef __WINS__
#define KEY_STAR            EStdKeyNkpAsterisk
#else
#define KEY_STAR            0x2a
#endif
#define KEY_SOFTKEY_LEFT    EStdKeyDevice0
#define KEY_SOFTKEY_RIGHT   EStdKeyDevice1
#define KEY_POWER           EStdKeyDevice2
#define KEY_OK              EStdKeyDevice3
#define KEY_MENU            EStdKeyApplication0
#define KEY_GREEN_PHONE     EStdKeyYes
#define KEY_RED_PHONE       EStdKeyNo
#define ASCII_KEY_R         TInt('R')
#define ASCII_KEY_T         TInt('T')
#define ASCII_KEY_Y         TInt('Y')
#define ASCII_KEY_U         TInt('U')
#define ASCII_KEY_F         TInt('F')
#define ASCII_KEY_G         TInt('G')
#define ASCII_KEY_H         TInt('H')
#define ASCII_KEY_J         TInt('J')
#define ASCII_KEY_V         TInt('V')
#define ASCII_KEY_B         TInt('B')
#define ASCII_KEY_N         TInt('N')
#define ASCII_KEY_M         TInt('M')
#define ASCII_KEY_SPACE     EStdKeySpace

#ifdef NAV2_CLIENT_SERIES60_V3
#define CURSOR_SCROLLING       EMbmWficonsCursor_in_map_scrolling
#define CURSOR_SCROLLING_MASK  EMbmWficonsCursor_in_map_scrolling_mask 
#define CURSOR_MARKING         EMbmWficonsCursor_in_map_marking
#define CURSOR_MARKING_MASK    EMbmWficonsCursor_in_map_marking_mask
#else
#define CURSOR_SCROLLING       EMbmWficonsS60_browse_up
#define CURSOR_SCROLLING_MASK  EMbmWficonsS60_browse_up_m
#define CURSOR_MARKING         EMbmWficonsS60_browse_on
#define CURSOR_MARKING_MASK    EMbmWficonsS60_browse_on_m
#endif
// Key bindings for movement.
const CVectorMapContainer::key_array_pair_t
CVectorMapContainer::c_keyBindings[] = {
   { KEY2,              TileMapKeyHandler::MOVE_UP_KEY },
   { ASCII_KEY_T,       TileMapKeyHandler::MOVE_UP_KEY },
   { KEY1,              TileMapKeyHandler::ZOOM_OUT_KEY },
   { ASCII_KEY_R,       TileMapKeyHandler::ZOOM_OUT_KEY },
   { KEY3,              TileMapKeyHandler::ZOOM_IN_KEY },
   { ASCII_KEY_Y,       TileMapKeyHandler::ZOOM_IN_KEY },
   { KEY8,              TileMapKeyHandler::MOVE_DOWN_KEY },
   { ASCII_KEY_B,       TileMapKeyHandler::MOVE_DOWN_KEY },
   { KEY_STAR,          TileMapKeyHandler::AUTO_ZOOM_KEY },
   { ASCII_KEY_U,       TileMapKeyHandler::AUTO_ZOOM_KEY },
   { KEY4,              TileMapKeyHandler::MOVE_LEFT_KEY },
   { ASCII_KEY_F,       TileMapKeyHandler::MOVE_LEFT_KEY },
   { KEY6,              TileMapKeyHandler::MOVE_RIGHT_KEY },
   { ASCII_KEY_H,       TileMapKeyHandler::MOVE_RIGHT_KEY },
   { KEY7,              TileMapKeyHandler::RESET_ROTATION_KEY },
   { ASCII_KEY_V,       TileMapKeyHandler::RESET_ROTATION_KEY },
#ifndef AUTOMOVE_ON // Can be defined in TileMapKeyHandler.h
   //{ KEY1,              TileMapKeyHandler::HIGHLIGHT_NEXT_FEATURE_KEY },
   //{ ASCII_KEY_R,       TileMapKeyHandler::HIGHLIGHT_NEXT_FEATURE_KEY },
//   { KEY1,              TileMapKeyHandler::ROTATE_LEFT_KEY },
//   { KEY3,              TileMapKeyHandler::ROTATE_RIGHT_KEY },
   //{ KEY3,              TileMapKeyHandler::CENTER_MAP_AT_CURSOR_KEY },
   //{ ASCII_KEY_Y,       TileMapKeyHandler::CENTER_MAP_AT_CURSOR_KEY },
#else
   // These keys can be used for automatic movement.
   { KEY1,              TileMapKeyHandler::SLOW_TEST_KEY },
   { ASCII_KEY_R,       TileMapKeyHandler::SLOW_TEST_KEY },
   { KEY3,              TileMapKeyHandler::FAST_TEST_KEY },
   { ASCII_KEY_Y,       TileMapKeyHandler::FAST_TEST_KEY },
#endif
      // Only for fullscreen
   { EStdKeyDownArrow,  TileMapKeyHandler::MOVE_CURSOR_DOWN_KEY },
   { EStdKeyRightArrow, TileMapKeyHandler::MOVE_CURSOR_RIGHT_KEY },
   { EStdKeyLeftArrow,  TileMapKeyHandler::MOVE_CURSOR_LEFT_KEY },
   { EStdKeyUpArrow,    TileMapKeyHandler::MOVE_CURSOR_UP_KEY },
};

const CVectorMapContainer::key_array_pair_t
CVectorMapContainer::c_landscapeKeyBindings[] = {
   { KEY2,              TileMapKeyHandler::MOVE_LEFT_KEY },
   { KEY4,              TileMapKeyHandler::MOVE_DOWN_KEY },
   { KEY6,              TileMapKeyHandler::MOVE_UP_KEY },
   { KEY8,              TileMapKeyHandler::MOVE_RIGHT_KEY },
};

const CVectorMapContainer::key_array_pair_t
CVectorMapContainer::c_portraitKeyBindings[] = {
   { KEY2,              TileMapKeyHandler::MOVE_UP_KEY },
   { KEY4,              TileMapKeyHandler::MOVE_LEFT_KEY },
   { KEY6,              TileMapKeyHandler::MOVE_RIGHT_KEY },
   { KEY8,              TileMapKeyHandler::MOVE_DOWN_KEY },
};


class CVectorMapContainerKeyHandlerCallBack
   : public TileMapKeyHandlerCallback {
public:
   CVectorMapContainerKeyHandlerCallBack(CVectorMapContainer* container) {
      m_container = container;
   }
   
   ~CVectorMapContainerKeyHandlerCallBack() {
      m_container = NULL;
   }

   // Will not repaint if this returns false.
   bool keyHandlerCallback() {
      if(m_container) {
         m_container->CheckFavoriteRedraw();	
      }

      return false;
   }
private:

   CVectorMapContainer* m_container;
   
};


/* controls present */
enum {
   EMapControl = 0,
#ifdef USE_GLOBE
   EGlobeControl,
#endif
   //EPedestrianPositionIndicator,
   EMapInfoControl,
   ETopBorderBitmap,
   ENextStreetCtrl,
   EPedestrianModeIndicator,
   EConBitmap,
   EDetourPicture,
   ESpeedCamPicture,
   EGpsIndicator,
   EZoomInPicture,
   EZoomOutPicture,
   ENumControls
};

enum TTouchScreenControls {
   EZoomIn,
   EZoomOUt,
   ENumTouchScreenControls
};

// Constructor
CVectorMapContainer::CVectorMapContainer( CMapView* aMapView, isab::Log* aLog )
   : iLog(aLog), iConstructDone(EFalse), iNrDragEventsInARow(0), 
     iUpdateCursorAndInfoAtNextRedraw(EFalse)
{
   iMapControl = NULL;
   iIsConnected = EFalse;

   iView = aMapView;
   
   m_timerPeriod = 50000; // Note that this is microseconds.
   iInfoTextShown = EFalse;
   m_detailedInfoTextShown = false;
   iInfoTextPersistant = EFalse;
   //iCurrentTurn = ENoPicture;

   m_keyHandler = NULL;
   m_cursor = NULL;
   m_cursorSprite = NULL;
   m_highlightCursorSprite = NULL;
   m_hideInfoTimerID = MAX_UINT32;
   m_mapMover = NULL;
   iConStatusPos = TPoint(-1, -1);
}

TInt
CVectorMapContainer::GetLogicalFont(enum TScalableFonts aFontId, TFontSpec& fontSpec)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   TInt fontId;
   switch (aFontId) {
   case EPrimaryLogicalFont:
      fontId = EAknLogicalFontPrimaryFont;
      break;
   case EPrimarySmallLogicalFont:
      fontId = EAknLogicalFontPrimarySmallFont;
      break;
   case ESecondaryLogicalFont:
      fontId = EAknLogicalFontSecondaryFont;
      break;
   case ETitleLogicalFont:
      fontId = EAknLogicalFontTitleFont;
      break;
   case EDigitalLogicalFont:
      fontId = EAknLogicalFontDigitalFont;
      break;
   default:
      return 0;
   }
   const CFbsFont* font = dynamic_cast <const CFbsFont *>(AknLayoutUtils::
                                                          FontFromId(fontId));
   if (font && font->IsOpenFont()) {
      TOpenFontFaceAttrib fontAttrib;
      font->GetFaceAttrib(fontAttrib);
      fontSpec = TFontSpec(fontAttrib.ShortFullName(), font->HeightInPixels());
      return 1;
   } else {      
      return 0;
   }
#else
   return 0;
#endif
}

void
CVectorMapContainer::setTileMapFonts()
{
   // First in every pair is the street-width that triggers the
   // change into a larger font.
   // The fonts have been tested on a 6600 but without wide streets.

#ifdef NAV2_CLIENT_SERIES60_V3
   TFontSpec primaryFont;
   TileMapTextSettings::font_pair_t horizontalFont;
   if (GetLogicalFont(EPrimaryLogicalFont, primaryFont)) {
      TBuf<KMaxTypefaceNameLength+24> typeName;
      typeName.Copy(primaryFont.iTypeface.iName);
      typeName.Append(KWfAntiAlias);
      horizontalFont = TileMapTextSettings::font_pair_t( typeName, 
                                                         primaryFont.iHeight );
   } else {
      horizontalFont = TileMapTextSettings::font_pair_t ( "LatinBold19_wfantialias", 19 );
   }
   TFontSpec primarySmallFont;
   TFontSpec secondaryFont;
   map<int, TileMapTextSettings::font_pair_t> lineFonts;
   TileMapTextSettings::font_pair_t roundRectFont;
   TileMapTextSettings::font_pair_t insidePolygonFont;
   TileMapTextSettings::font_pair_t copyrightFont;
   TileMapTextSettings::font_pair_t progressIndicatorFont;
   if (GetLogicalFont(EPrimarySmallLogicalFont, primarySmallFont)) {
      lineFonts.insert(
         make_pair( 16,
                    TileMapTextSettings::
                    font_pair_t( primaryFont.iTypeface.iName, 16 ) ) );
      if (GetLogicalFont(EPrimaryLogicalFont, primaryFont)) {
         lineFonts.insert(
            make_pair( primaryFont.iHeight,
                       TileMapTextSettings::
                       font_pair_t( primaryFont.iTypeface.iName, 
                                    primaryFont.iHeight ) ) );
      } else {
         lineFonts.insert(
            make_pair( 17,
                       TileMapTextSettings::
                       font_pair_t( primaryFont.iTypeface.iName, 17 ) ) );         
      }
      lineFonts.insert(
         make_pair( primarySmallFont.iHeight,
                    TileMapTextSettings::
                    font_pair_t( primarySmallFont.iTypeface.iName, 
                                 primarySmallFont.iHeight ) ) );

      roundRectFont = TileMapTextSettings::
         font_pair_t( primarySmallFont.iTypeface.iName, 
                      primarySmallFont.iHeight );
      insidePolygonFont = TileMapTextSettings::
         font_pair_t( primarySmallFont.iTypeface.iName, 
                      primarySmallFont.iHeight );
      progressIndicatorFont = TileMapTextSettings::
         font_pair_t ( primarySmallFont.iTypeface.iName, 
                       primarySmallFont.iHeight );
      if (GetLogicalFont(ESecondaryLogicalFont, secondaryFont)) {
         copyrightFont = TileMapTextSettings::
            font_pair_t( secondaryFont.iTypeface.iName, 
                         secondaryFont.iHeight );
      } else {
         copyrightFont = TileMapTextSettings::
            font_pair_t( "LatinBold17", 17 );
      }
   } else {
      lineFonts.insert(
         make_pair( 16,
                    TileMapTextSettings::
                    font_pair_t( "LatinBold16", 16 ) ) );
      lineFonts.insert(
         make_pair( 17,
                    TileMapTextSettings::
                    font_pair_t( "LatinBold17", 17 ) ) );
      lineFonts.insert(
         make_pair( 19,
                    TileMapTextSettings::
                    font_pair_t( "LatinBold19", 19 ) ) );
      roundRectFont = TileMapTextSettings::
         font_pair_t ( "LatinBold17", 17 );
      insidePolygonFont = TileMapTextSettings::
         font_pair_t ( "LatinBold17", 17 );  
      copyrightFont = TileMapTextSettings::
         font_pair_t ( "LatinBold17", 17 ); 
      progressIndicatorFont = TileMapTextSettings::
         font_pair_t ( "LatinBold17", 17 );
   }
#else
   map<int, TileMapTextSettings::font_pair_t> lineFonts;
   lineFonts.insert(
      make_pair( 12,
                 TileMapTextSettings::font_pair_t( "LatinBold12", 12 ) ) );
   lineFonts.insert(
      make_pair( 17,
                 TileMapTextSettings::font_pair_t( "LatinBold17", 17 ) ) );
   lineFonts.insert(
      make_pair( 19,
                 TileMapTextSettings::font_pair_t( "LatinBold19", 19 ) ) );
   TileMapTextSettings::font_pair_t horizontalFont( "LatinBold12", 12 );
   TileMapTextSettings::font_pair_t roundRectFont( "LatinBold12", 12 );
   TileMapTextSettings::font_pair_t insidePolygonFont( "LatinBold12", 12 );   
   TileMapTextSettings::font_pair_t 
      progressIndicatorFont( "LatinBold17", 17 );
   TileMapTextSettings::font_pair_t copyrightFont( "LatinPlain12", 12 );
#endif
   
   iMapControl->setTextSettings( 
         TileMapTextSettings( lineFonts,
                              horizontalFont,
                              roundRectFont,
                              insidePolygonFont,
                              progressIndicatorFont,
                              copyrightFont ) );   
}

void
CVectorMapContainer::initKeyHandler()
{
   /// Set the keyhandler
   m_khCallBack = new CVectorMapContainerKeyHandlerCallBack(this);

// Enable cursor.
#define CURSOR
   
#ifdef CURSOR   
   // Get the center point.
   MC2Point center( Size().iWidth >> 1, Size().iHeight >> 1 );

   m_cursorSprite = SymbianSpriteHolder::createSymbianSpriteHolder( 
                  ControlEnv()->WsSession(),
                  Window(),
                  TPoint( center.getX(), center.getY() ),
                  iView->GetMbmName(),
                  CURSOR_SCROLLING,
                  CURSOR_SCROLLING_MASK,
                  true ); // visible.

   m_highlightCursorSprite = SymbianSpriteHolder::createSymbianSpriteHolder(
                  ControlEnv()->WsSession(),
                  Window(),
                  TPoint( center.getX(), center.getY() ),
                  iView->GetMbmName(),
                  CURSOR_MARKING,
                  CURSOR_MARKING_MASK,
                  false ); // not visible.

   m_cursor = new CursorSprite( m_cursorSprite, 
                                m_highlightCursorSprite );

   m_keyHandler = new TileCursorKeyHandler(iMapMovingInterface,
                                           iMapDrawingInterface,
                                           iMapControl->getToolkit(),
                                           m_cursor,
                                           m_khCallBack,
                                           this);
   m_keyHandler->setStaticCursor(ETrue);
#else
   // Just use the static crosshair.
   m_keyHandler = new TileMapKeyHandler(iMapMovingInterface,
                                        iMapDrawingInterface,
                                        iMapControl->getToolkit(),
                                        iMapControl,
                                        m_khCallBack);
#endif
   // This is probably not necessary.
   m_keyHandler->setMapBox( iMapControl->Rect() );
   iMapControl->setKeyHandlerAndOwnership( m_keyHandler );
}

void
CVectorMapContainer::ConstructL( const TRect& aRect,
                                 CWayFinderAppUi* aWayFinderUI,
                                 CTileMapControl** aMapControl,
                                 CMapFeatureHolder** aFeatureHolder,
                                 CMapInfoControl** aMapInfoControl,
                                 CVectorMapConnection** aVectorMapConnection,
                                 const TDesC& /*aResourcePath*/)
{
   iWayFinderUI = aWayFinderUI;
   CreateWindowL();

   LogFile::Create();

   iNormalRect = aRect;

   TInt dpiCorrFact = WFLayoutUtils::CalculateDpiCorrectionFactor();
   iXPadding = X_PADDING * dpiCorrFact;
   iYPadding = Y_PADDING * dpiCorrFact;

   /* If aRequester is NULL, that means that the saved variables */
   /* are not set (they are created and deleted together). */
   if( *aFeatureHolder == NULL) {
      iFeatureHolder = new (ELeave) CMapFeatureHolder();
      User::LeaveIfNull(iFeatureHolder);

      iFeatureHolder->InitPolygons();
      DistancePrintingPolicy::DistanceMode mode =
         DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());
      TFontSpec fontSpec;
      char* fontName = NULL;
      TInt  fontSize = 0;
      if (GetLogicalFont(ESecondaryLogicalFont, fontSpec)) {
         fontName = WFTextUtil::TDesCToUtf8LC(fontSpec.iTypeface.iName);
         fontSize = fontSpec.iHeight;
      } else {
         _LIT(KFontName, "LatinBold16");
         fontName = WFTextUtil::TDesCToUtf8LC(KFontName);
         fontSize = 16;
      }
      TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
      switch (mode) {
      case DistancePrintingPolicy::ModeImperialFeet:
         iFeatureHolder->iScaleFeature = new UserDefinedScaleFeature(
               fontName, fontSize,
               UserDefinedScaleFeature::getMilesFeetSettings(),
               dpiCorrectionFactor);
         CleanupStack::PopAndDestroy(fontName);
         iFeatureHolder->iScaleFeature->rescalePoints();
         break;
      case DistancePrintingPolicy::ModeImperialYards:
         iFeatureHolder->iScaleFeature = new UserDefinedScaleFeature(
               fontName, fontSize,
               UserDefinedScaleFeature::getMilesYardsSettings(),
               dpiCorrectionFactor);
         CleanupStack::PopAndDestroy(fontName);
         iFeatureHolder->iScaleFeature->rescalePoints();
         break;
      case DistancePrintingPolicy::ModeMetric:
         /* FALLTHROUGH */
      default:
         iFeatureHolder->iScaleFeature = new UserDefinedScaleFeature(
               fontName, fontSize,
               UserDefinedScaleFeature::getMeterSettings(),
               dpiCorrectionFactor);
         CleanupStack::PopAndDestroy(fontName);
         iFeatureHolder->iScaleFeature->rescalePoints();
         break;
      }

      // NEW
      // Setup the connection where the maps are requested
      CVectorMapConnection* vectorMapConnection =
         new (ELeave) CVectorMapConnection( aWayFinderUI );
      User::LeaveIfNull(vectorMapConnection);

      // Create the TileMapControl with the vectormap connection.
      iMapControl = CTileMapControl::NewL( vectorMapConnection,
                                           aRect,
                                           iView );
      iMapControl->Handler().addEventListener(this);
#ifndef USE_GLOBE
      iMapMovingInterface = &iMapControl->Handler();
      iMapDrawingInterface = &iMapControl->Handler();
#endif
      MapLib* mapLib = iMapControl->getMapLib();
      
#ifdef ENABLE_3D_OUTLINES
      mapLib->setOutlinesIn3dEnabled( true );
#endif
      // Set memcache size
      mapLib->setMemoryCacheSize( getMemCacheSize() );

      // #undef USE_FILE_CACHE
#ifdef USE_FILE_CACHE
      // Add file cache      
      char* cache_path = WFTextUtil::TDesCToUtf8LC(iView->GetWritableAutoMapCachePath());
      
      mapLib->addDiskCache( cache_path,
                            iView->GetMapCacheSize()*1024*1024 );

      CleanupStack::PopAndDestroy( cache_path );
#endif      
      // END NEW
      AddSfdCacheFiles();
      
      // TODO calculate the correctin factor here, the calculation 
      // should actually be done in WFLayoutUtils but it shold
      // be set here.
      mapLib->setDPICorrectionFactor(dpiCorrFact);

      m_mapInfoControl = new CMapInfoControl();
      m_mapInfoControl->ConstructL( aRect );
      m_mapInfoControl->SetContainerWindowL(*this);
      
      iMapControl->SetContainerWindowL(*this);
      iMapControl->Plotter().setBackgroundColor(255,255,255);
      iMapControl->Plotter().clearScreen();
      TFontSpec primaryFont;
      if (GetLogicalFont(EPrimaryLogicalFont, primaryFont)) {
         iMapControl->Plotter().setFont(primaryFont.iTypeface.iName, 
                                        primaryFont.iHeight );
      } else {
         iMapControl->Plotter().setFont(_L("LatinBold19"), 19);
      }
      iMapControl->Plotter().setPenColor(0,0,64);

      TInt topBarHeight =
         WFLayoutUtils::CalculateYValueUsingFullScreen(BLUE_BAR_HEIGHT);
      TInt nextStreetBarHeight = m_mapInfoControl->getDefaultHeight() + 2;
      HBufC* text =
         CEikonEnv::Static()->AllocReadResourceLC(R_WAYFINDER_IAP_SEARCH2_MSG);
      Rectangle textRectangle =
         iMapControl->Plotter().getStringAsRectangle(*text, MC2Point(0, 0));
      TInt textHeight = textRectangle.getHeight();
      MC2Point point(aRect.Center().iX,
                     topBarHeight + nextStreetBarHeight + (textHeight/2) + 2);
      iMapControl->Plotter().drawText(*text, point );
      CleanupStack::PopAndDestroy(text);

      *aFeatureHolder = iFeatureHolder;
      *aVectorMapConnection = vectorMapConnection;
      *aMapControl = iMapControl;
      *aMapInfoControl = m_mapInfoControl;
      iIsConnected = EFalse;
      /* Load favorites. */
      //       iView->SearchForMapFiles(ETrue);
      iView->FavoriteChanged();

      iMapControl->Handler().setScreenSize( MC2Point( 176, 208));

      if (iView->IsIronVersion()) {
         iScale = EScaleGlobe;
      } else {
         iScale = EScale12;
      }
      iCenter = TPoint(DefaultVectorMapCenterLon, DefaultVectorMapCenterLat);
   } else {
      iFeatureHolder = *aFeatureHolder;
      iMapControl = *aMapControl;
      iMapControl->Handler().addEventListener(this);
#ifndef USE_GLOBE
      iMapMovingInterface = &iMapControl->Handler();
      iMapDrawingInterface = &iMapControl->Handler();
#endif
      m_mapInfoControl = *aMapInfoControl;
      iIsConnected = ETrue;

      m_mapInfoControl->SetContainerWindowL(*this);
      iMapControl->SetContainerWindowL(*this);
      iScale = iView->GetScale();
      iCenter = iView->GetCenter();
   }
   // Clear any old info texts.
   m_mapInfoControl->setInfoText( NULL );

   MapLib* mapLib = iMapControl->getMapLib();

   mapLib->setCopyrightPos(MC2Point(iXPadding, aRect.Height() - iYPadding));
   mapLib->showCopyright(ETrue);

   //Convert to LangTypes::language_t via method in LangTypes.
   //Then convert from language_t to const char* via getLangAsIso639.
   //     mapLib->setLanguageAsISO639(
   //       LangTypes::getLanguageAsISO639(
   //          LangTypes::getNavLangAsLanguage(aWayFinderUI->m_languageCode)));

   mapLib->setLanguageAsISO639(
      LangTypes::getLanguageAsISO639(
         LangTypes::getNavLangAsLanguage(iView->GetLanguageCode())));

   iMapControl->Handler().setUserDefinedFeatures(iFeatureHolder->GetUDFVector());

#ifdef USE_GLOBE
   //The map switcher, which switches between vector map and globe. 
   //iGlobeComponent = GlobeCreator::createGlobe(*this);
   TSize gbSize = iCoeEnv->ScreenDevice()->SizeInPixels();
   TRect gbRect = TRect(gbSize);

   HBufC* languageCode = iCoeEnv->AllocReadResourceL( 
                                                     R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE );
   HBufC* locationString = iCoeEnv->AllocReadResourceL(
                                                       R_EARTH_IM_IN );
  
   char* languageCodeUtf8 = WFTextUtil::TDesCToUtf8L( languageCode->Des() );
   char* locationStringUtf8 = WFTextUtil::TDesCToUtf8L( locationString->Des() );
   char* pathUtf8 = WFTextUtil::TDesCToUtf8L( iView->GetCommonDataPath() );
  
   iGlobeComponent = 
      GlobeCreator::createGlobe(*this, 
                                pathUtf8,
                                gbRect,
                                locationStringUtf8,
                                languageCodeUtf8 );

                                
   delete locationString;
   delete locationStringUtf8;
   delete languageCode;
   delete languageCodeUtf8;

   iGlobeControl = &iGlobeComponent->getControl();
   iGlobeControl->MakeVisible(false);
#endif

   MapSwitcher::SwitcherNotice tilemapNotice;
#ifdef USE_GLOBE
   tilemapNotice.m_scale = iGlobeComponent->getMinScale();
#else
   tilemapNotice.m_scale = 2000000000;
#endif
   tilemapNotice.m_mapMover = &iMapControl->Handler();
   tilemapNotice.m_mapDrawer = &iMapControl->Handler();
   tilemapNotice.m_hasStaticCursor = ETrue;

   // Set the fonts in the TileMapHandler.
   setTileMapFonts();

   LayoutVisibilityAdaptersL(aRect, tilemapNotice);

#ifdef USE_GLOBE
   MapSwitcher::SwitcherNotice globeNotice;
   globeNotice.m_scale = 2000000000; // This ought to be enough.
   globeNotice.m_mapMover = &iGlobeComponent->getMapMovingInterface();
   globeNotice.m_mapDrawer = &iGlobeComponent->getMapDrawingInterface();
   globeNotice.m_hasStaticCursor = true;
   globeNotice.m_mapRect = &iGlobeComponent->getMapRectInterface();
   
   iGlobeControlVisAdapter = new VisibilityAdapter<CCoeControl>(iGlobeControl);
   iGlobeControlVisAdapter->setVisible(ETrue);
   globeNotice.m_visibles.push_back(iGlobeControlVisAdapter);
#endif

   vector<MapSwitcher::SwitcherNotice> selectorNotices;
   selectorNotices.push_back(tilemapNotice);
#ifdef USE_GLOBE
   selectorNotices.push_back(globeNotice);
#endif

   iMapSwitcher = new MapSwitcher(selectorNotices, iScale);

#ifdef USE_GLOBE
   iMapMovingInterface = iMapSwitcher;
   iMapDrawingInterface = iMapSwitcher;
#endif

   SetZoom(iScale);
   SetCenter(iCenter.iY, iCenter.iX);

   /* set to default values */
   //deltaDefaults();

#ifdef USE_AUTO_TRACKING_TIMER
   /* start the key timer */
   m_autoTrackingOnTimer = CPeriodic::NewL(CActive::EPriorityLow);
#endif

   /*    iTurnPicture = new (ELeave) CGuidePicture( iLog ); */
   /*    LOGNEW(iTurnPicture, CGuidePicture); */
   /*    iTurnPicture->ConstructL( TRect( TPoint( 152, 0 ), TSize( 24, 24 ) ), this); */

   // Create the keyhandler and the cursor.
   initKeyHandler();

   //Needed here since the keyHandler needs the mapSwitcher 
   //and the mapSwitcher needs the keyHandler.
   iMapSwitcher->setCursorHandler(m_keyHandler);
   iCursorVisAdapter->setControl(m_keyHandler);

   m_mapMover = new MapMover( m_keyHandler, 
                              iMapMovingInterface,
                              iFeatureHolder->iPositionPoly,
                              this );
   m_mapMover->setGpsFeature( iFeatureHolder->iDarkShadePoly );
   m_mapMover->setGpsFeature( iFeatureHolder->iLightshadePoly );
   m_mapMover->setGpsFeature( iFeatureHolder->iPedPositionPoly );
   m_mapMover->setGpsFeature( iFeatureHolder->iPedPositionFillPoly );
   m_mapMover->setTrackingPoint( getTrackingPoint());
   m_mapMover->setNonTrackingPoint( getNonTrackingPoint() );
   
   /* set this control to get keyboard focus */
   SetFocus(ETrue);
   iMapControl->SetFocus( ETrue );
   updateCursorVisibility();

   ShowNorthArrow( !iView->MapAsPedestrianNavigation() );

   // Insert the key bindings into the map
   int nbrBindings = sizeof(c_keyBindings) / sizeof(c_keyBindings[0]);
   for ( int i = nbrBindings - 1; i >= 0; --i ) {
      m_keyMap.insert( make_pair( c_keyBindings[i].first,
                                  c_keyBindings[i].second ) );
   }

   // Please to respond to my clickings.
   EnableDragEvents();   
   if (iView->Get3dMode()) {
      // No scale when in 3d mode
      ShowScale( EFalse );
   } else {
      ShowScale( ETrue );
   }

   TInt& oldDpiCorrFact = iView->GetDpiCorrFact();
   if(dpiCorrFact != oldDpiCorrFact) {
      oldDpiCorrFact = dpiCorrFact;
      mapLib->setDPICorrectionFactor(dpiCorrFact);
      iFeatureHolder->RescalePolygons();      
   }

#ifdef NAV2_CLIENT_SERIES60_V5
   iLongTapDetector = CAknLongTapDetector::NewL( this );
#endif


   SetRect(aRect);
   ActivateL();
   
   UpdateRepaint();

   iConstructDone = ETrue;
}

void CVectorMapContainer::LayoutVisibilityAdaptersL(const TRect& aRect,
                                                    MapSwitcher::SwitcherNotice& aTilemapNotice)
{
   iMapControlVisAdapter = new VisibilityAdapter<CTileMapControl>(iMapControl);
   iMapControlVisAdapter->setVisible(ETrue);
   aTilemapNotice.m_visibles.push_back(iMapControlVisAdapter);

   // Create the cursor
   iCursorVisAdapter =
      new CursorVisibilityAdapter(m_keyHandler);
   // If map is in 3d mode, the cursor should not be visible
   iCursorVisAdapter->setVisible(!(iView->MapAsGeneralNavigation() && 
                                   iView->Get3dMode()));
   aTilemapNotice.m_visibles.push_back(iCursorVisAdapter);

   // Create the map border bar
   TInt borderBarHeight = 
      WFLayoutUtils::CalculateYValueUsingFullScreen(BLUE_BAR_HEIGHT);
   TRect borderBarRect(TPoint(aRect.iTl.iX, aRect.iTl.iY), 
                       TSize(aRect.Width(), borderBarHeight));
   iTopBorderVisAdapter =
      new VisibilityAdapter<CMapTopBorderBar>
      (CMapTopBorderBar::NewL(this, borderBarRect, 
                              iView->GetMbmName(), KRgbBlack, TRgb(KForegroundTextColorNightRed,
                                                                   KForegroundTextColorNightGreen,
                                                                   KForegroundTextColorNightBlue)));
   // Set the background color
   iTopBorderVisAdapter->getControl()->SetBackgroundColors(TRgb(0x00ffffff, 170), TRgb(0x00424542, 220));

   // Set the divider image
   iTopBorderVisAdapter->getControl()->SetDividerImageL(EMbmWficonsTop_bar_divider, 
                                                        EMbmWficonsTop_bar_divider_mask);

   iTopBorderVisAdapter->setVisible(EFalse);
   aTilemapNotice.m_visibles.push_back(iTopBorderVisAdapter);

   // Create the next street label
   TInt nextStreetCtrlHeight = m_mapInfoControl->getDefaultHeight() + 2;

   TRect nextStreetCtrlRect(TPoint(aRect.iTl.iX, borderBarRect.iBr.iY), 
                            TSize(aRect.Width(), nextStreetCtrlHeight));
   iNextStreetVisAdapter = 
      new VisibilityAdapter<CBackGroundTextContainer>
      (CBackGroundTextContainer::NewL(this, 
                                      nextStreetCtrlRect, 
                                      KEmptyDefaultText, 
                                      TRgb(0x00ffffff, 170),
                                      TRgb(0x00424542, 220),
                                      KRgbBlack,
                                      TRgb(KForegroundTextColorNightRed,
                                           KForegroundTextColorNightGreen,
                                           KForegroundTextColorNightBlue)));
   iNextStreetVisAdapter->getControl()->SetBorderEdges(
      CBackGroundTextContainer::ETopEdge | CBackGroundTextContainer::EBottomEdge);
   iNextStreetVisAdapter->setVisible(EFalse);
   aTilemapNotice.m_visibles.push_back(iNextStreetVisAdapter);

   // Initial setup of positions
   TSize imgSize = WFLayoutUtils::CalculateSizeUsingMin(INFO_IMAGES_SIZE);
   TPoint imgRPos = borderBarRect.iBr;
   TPoint imgLPos = borderBarRect.iTl;

   // Create the detour image
   imgRPos.iX -= (imgSize.iWidth + WFLayoutUtils::CalculateXValue(X_PADDING));
   imgRPos.iY = nextStreetCtrlRect.iBr.iY + 
      WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   iDetourPictureVisAdapter = new VisibilityAdapter<CImageHandler>
      (CImageHandler::NewL(TRect(imgRPos, imgSize)));
   iDetourPictureVisAdapter->getControl()->SetShow(EFalse);
   iDetourPictureVisAdapter->getControl()->
      CreateIconL(iView->GetMbmName(), 
                  EMbmWficonsDetour_square,
                  EMbmWficonsDetour_square_mask,
                  EAspectRatioPreservedAndUnusedSpaceRemoved);

   // Create the speedcam image
   imgRPos.iY += imgSize.iHeight + 
      WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   iSpeedCamPictureVisAdapter = new VisibilityAdapter<CImageHandler>
      (CImageHandler::NewL(TRect(imgRPos, imgSize)));
   iSpeedCamPictureVisAdapter->getControl()->SetShow(EFalse);
   iSpeedCamPictureVisAdapter->getControl()->
      CreateIconL(iView->GetMbmName(),
                  EMbmWficonsSpeedcamera_square,
                  EMbmWficonsSpeedcamera_square,
                  EAspectRatioPreservedAndUnusedSpaceRemoved);

   // Create the gps indicator image
   iGpsIndicatorVisAdapter = new VisibilityAdapter<CImageHandler>
      (CImageHandler::NewL(TRect(imgRPos, imgSize)));
   iGpsIndicatorVisAdapter->getControl()->SetShow(EFalse);

   PositionRightEdgeFloatingControls();

   // Create the pedestrian indicator image
   if (iNextStreetVisAdapter->isVisible()) {
      imgLPos.iY = nextStreetCtrlRect.iBr.iY + 
         WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   } else {
      imgLPos.iY = WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   }
   if (iFeatureHolder->iCompassPolyShown) {
      // If the compass is shown center the image below the compass.
      TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
      TInt xPos = (COMPASS_POLY_HEIGHT / 2) * dpiCorrectionFactor;
      xPos += WFLayoutUtils::CalculateXValue(X_PADDING);
      xPos -= imgSize.iWidth / 2;
      imgLPos.iX += xPos;
      imgLPos.iY += 
         COMPASS_POLY_HEIGHT * dpiCorrectionFactor + 
         WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   } else {
      // If compass is not shown we position the images with padding instead.
      imgLPos.iX += WFLayoutUtils::CalculateXValue(X_PADDING);
   }
   iPedestrianModeVisAdapter = new VisibilityAdapter<CImageHandler>
      (CImageHandler::NewL(TRect(imgLPos, imgSize)));
   iPedestrianModeVisAdapter->getControl()->
      CreateIconL(iView->GetMbmName(), 
                  EMbmWficonsPedestrian_mode,
                  EMbmWficonsPedestrian_mode_mask,
                  EAspectRatioPreservedAndUnusedSpaceRemoved);
   iPedestrianModeVisAdapter->setVisible(iView->MapAsPedestrianNavigation());
   aTilemapNotice.m_visibles.push_back(iPedestrianModeVisAdapter);

   if (WFLayoutUtils::IsTouchScreen()) {
      // Create the zoom buttons, the will be given the correct size and position
      // in SizeChanged function.
      iZoomInPictureVisAdapter = new VisibilityAdapter<CImageHandler>
         (CImageHandler::NewL(TRect(0, 0, 0, 0)));

      iZoomInPictureVisAdapter->getControl()->SetShow(ETrue);
      iZoomInPictureVisAdapter->getControl()->CreateIconL(iView->GetMbmName(), 
                                                          EMbmWficonsZoom_in,
                                                          EMbmWficonsZoom_in_mask);

      iZoomOutPictureVisAdapter = new VisibilityAdapter<CImageHandler>
         (CImageHandler::NewL(TRect(0, 0, 0, 0)));

      iZoomOutPictureVisAdapter->getControl()->SetShow(ETrue);
      iZoomOutPictureVisAdapter->getControl()->CreateIconL(iView->GetMbmName(), 
                                                           EMbmWficonsZoom_out,
                                                           EMbmWficonsZoom_out_mask);
      iZoomInPictureVisAdapter->setVisible(!iView->Get3dMode());
      aTilemapNotice.m_visibles.push_back(iZoomInPictureVisAdapter);
      iZoomOutPictureVisAdapter->setVisible(!iView->Get3dMode());
      aTilemapNotice.m_visibles.push_back(iZoomOutPictureVisAdapter);
   }

   CBitmapControl* conBitmap = CBitmapControl::NewL(this, TPoint(0,0), NULL);
   iConStatusVisAdapter = 
      new VisibilityAdapter<CBitmapControl>(conBitmap);

   iDetourPictureVisAdapter->setVisible(EFalse);
   aTilemapNotice.m_visibles.push_back(iDetourPictureVisAdapter);
   iSpeedCamPictureVisAdapter->setVisible(EFalse);
   aTilemapNotice.m_visibles.push_back(iSpeedCamPictureVisAdapter);
   iConStatusVisAdapter->setVisible(EFalse);
   aTilemapNotice.m_visibles.push_back(iConStatusVisAdapter);
   iGpsIndicatorVisAdapter->setVisible(EFalse);
   aTilemapNotice.m_visibles.push_back(iGpsIndicatorVisAdapter);

   aTilemapNotice.m_mapRect = iMapControl;   
}


// Destructor
CVectorMapContainer::~CVectorMapContainer()
{
   // Cancel the hide info timer.
   if ( iMapControl != NULL && m_hideInfoTimerID != MAX_UINT32 ) {
      iMapControl->getToolkit()->cancelTimer( this, m_hideInfoTimerID );
   }
   ReleaseMapControlDependencies();
   // Keyhandler is deleted by TileMapControl.
   // delete m_keyHandler;
   // Stop the timer and reset all keys that may otherwise be repeated
   StopKeyHandler();
   delete m_mapMover;
#ifdef USE_AUTO_TRACKING_TIMER
   if ( m_autoTrackingOnTimer->IsActive() ) {
      m_autoTrackingOnTimer->Cancel();
   }
   delete m_autoTrackingOnTimer;
   m_autoTrackingOnTimer = NULL;
#endif
   iMapControl = NULL;

/*    LOGDEL(iTurnPicture); */
/*    delete iTurnPicture; */
/*    iTurnPicture = NULL; */
   
   // Set the callback of the key handler to NULL so it does not
   // attempt to use it, since it is deleted here below
   m_keyHandler->cancelTileMapKeyHandlerCallback();
   delete m_khCallBack;
   m_khCallBack = NULL;

   if (iTopBorderVisAdapter) {
      delete iTopBorderVisAdapter->getControl();
      delete iTopBorderVisAdapter;
      iTopBorderVisAdapter = NULL;
   }
   if (iNextStreetVisAdapter) {
      delete iNextStreetVisAdapter->getControl();
      delete iNextStreetVisAdapter;
      iNextStreetVisAdapter = NULL;
   }
   if (iConStatusVisAdapter) {
      delete iConStatusVisAdapter->getControl();
      delete iConStatusVisAdapter;
      iConStatusVisAdapter = NULL;
   }
   if (iDetourPictureVisAdapter) {
      delete iDetourPictureVisAdapter->getControl();
      delete iDetourPictureVisAdapter;
      iDetourPictureVisAdapter = NULL;
   }
   if (iSpeedCamPictureVisAdapter) {
      delete iSpeedCamPictureVisAdapter->getControl();
      delete iSpeedCamPictureVisAdapter;
      iSpeedCamPictureVisAdapter = NULL;
   }
   if (iZoomInPictureVisAdapter) {
      delete iZoomInPictureVisAdapter->getControl();
      delete iZoomInPictureVisAdapter;
      iZoomInPictureVisAdapter = NULL;
   }
   if (iZoomOutPictureVisAdapter) {
      delete iZoomOutPictureVisAdapter->getControl();
      delete iZoomOutPictureVisAdapter;
      iZoomOutPictureVisAdapter = NULL;
   }

   delete iCursorVisAdapter;

   delete iMapControlVisAdapter;
   delete iGlobeControlVisAdapter;
   delete iGlobeComponent;

   delete iMapSwitcher;

   delete m_cursorSprite;
   delete m_highlightCursorSprite;
   delete m_cursor;

#ifdef NAV2_CLIENT_SERIES60_V5
   delete iLongTapDetector;
   delete iPopUpMenu;
#endif
}

TBool CVectorMapContainer::ConstructDone()
{
   return iConstructDone;
}

void CVectorMapContainer::PictureError( TInt /*aError*/ )
{
/*    iTurnPicture->SetShow( EFalse ); */
/*    iView->PictureError( aError ); */
}

void CVectorMapContainer::ScalingDone()
{
   DrawNow();
}


void CVectorMapContainer::SizeChanged()
{
   TRect rect = Rect();

   TInt dpiCorrFact = WFLayoutUtils::CalculateDpiCorrectionFactor();
   iXPadding = X_PADDING * dpiCorrFact;
   iYPadding = Y_PADDING * dpiCorrFact;

   // Recalc the bluebar rect
   TInt borderBarHeight = 
      WFLayoutUtils::CalculateYValueUsingFullScreen(BLUE_BAR_HEIGHT);

   TRect borderBarRect(TPoint(rect.iTl.iX, rect.iTl.iY), 
                       TSize(rect.Width(), borderBarHeight));
   iTopBorderVisAdapter->getControl()->SetRect(borderBarRect);

   if (iMapControl && m_mapInfoControl) { 

      TPoint mapPos(rect.iTl);
      MapLib* mapLib = iMapControl->getMapLib();
      mapLib->setCopyrightPos(MC2Point(5, rect.Height() - 5 - mapPos.iY));

      iMapControl->SetSize(TSize(rect.Width(), (rect.Height()-mapPos.iY)));
      iMapControl->SetPosition(mapPos);

      m_mapInfoControl->SetSize(TSize(rect.Width(),
            (rect.Height()-mapPos.iY)));
      m_mapInfoControl->SetPosition(mapPos);
   }
   // Use the same height for the next street bar as the map info control.
   TInt nextStreetCtrlHeight = m_mapInfoControl->getDefaultHeight() + 2;

   TRect nextStreetCtrlRect(TPoint(rect.iTl.iX, borderBarRect.iBr.iY), 
                            TSize(rect.Width(), nextStreetCtrlHeight));
   iNextStreetVisAdapter->getControl()->SetRect(nextStreetCtrlRect);

   ResetNorthArrowPos();

   // Initial setup of positions
   TSize imgSize = WFLayoutUtils::CalculateSizeUsingMin(INFO_IMAGES_SIZE);
   TPoint imgLPos = borderBarRect.iTl;
   PositionRightEdgeFloatingControls();

   // Position pedestrian indicator image
   if (iNextStreetVisAdapter->isVisible()) {
      imgLPos.iY = nextStreetCtrlRect.iBr.iY + 
         WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   } else {
      imgLPos.iY = WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   }
   if (iFeatureHolder->iCompassPolyShown) {
      // If the compass is shown center the image below the compass.
      TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
      TInt xPos = (COMPASS_POLY_HEIGHT / 2) * dpiCorrectionFactor;
      xPos += WFLayoutUtils::CalculateXValue(X_PADDING);
      xPos -= imgSize.iWidth / 2;
      imgLPos.iX += xPos;
      imgLPos.iY += 
         COMPASS_POLY_HEIGHT * dpiCorrectionFactor + 
         WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   } else {
      // If compass is not shown we position the images with padding instead.
      imgLPos.iX += WFLayoutUtils::CalculateXValue(X_PADDING);
   }
   TRect newRect = TRect(imgLPos, imgSize);
   iPedestrianModeVisAdapter->getControl()->SetImageRect(newRect);

   if (WFLayoutUtils::IsTouchScreen()) {
      // Calc the height of the zoom button, the image should be a square so
      // the height is equal to the width
      TInt imgHeight = 
         WFLayoutUtils::CalculateUnitsUsingMin(float(ZOOM_BUTTON_SIZE));
      // Get the default height of the blue info note, one line height.
      TInt blueInfoHeight = m_mapInfoControl->getDefaultHeight();
      // Calc the top left of the zoom in picture. It should be above the blue info
      // not with some padding in between.
      TPoint topLeft(iXPadding, rect.iBr.iY - (blueInfoHeight + iYPadding + imgHeight));
      // Set the postion and size of the zoom out button
      iZoomOutPictureVisAdapter->getControl()->SetImageRect(TRect(topLeft, TSize(imgHeight, imgHeight)));
      // the top left x position for the zoom out button should be on the right side of the
      // screen
      topLeft.iX = WFLayoutUtils::GetMainPaneRect().iBr.iX - ((iXPadding + imgHeight));
      // Set the postion and size of the zoom in button
      iZoomInPictureVisAdapter->getControl()->SetImageRect(TRect(topLeft, TSize(imgHeight, imgHeight)));
   }
   // Will trigger a repostioning of the con status image
   iConStatusPos.iY = -1;

   bool northUp = (iView->GetTrackingType() != ERotating);
   if (iMapControl && m_keyHandler && m_mapMover) {
      
#ifdef USE_GLOBE
      iMapSwitcher->updateSize();
#else
      PixelBox mapBox(iMapControl->Rect());
      m_keyHandler->setMapBox(mapBox);
#endif
      iMapDrawingInterface->requestRepaint();
      m_mapMover->statusChanged(getTrackingPoint(),
                                getNonTrackingPoint(true),
                                iView->IsTracking(),
                                northUp,
                                false );
      updateCursorVisibility();
   }
   if (iView->Get3dMode()) {
      //we have different scaling of the position polygon when in 3d view and when not.
      if(iFeatureHolder) {
         iFeatureHolder->SetPositionPolyScale(float(GPS_POSITION_POLY_SCALE_IN_3D_VIEW));
      }
      // No scale when in 3d mode
      ShowScale( EFalse );
   } else {
      if(iFeatureHolder) {
         iFeatureHolder->SetPositionPolyScale(float(GPS_POSITION_POLY_SCALE_IN_2D_VIEW));
      }
      ShowScale( ETrue );
   }
   if(iFeatureHolder) {
      TSize screenSize;
      AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EScreen, screenSize);
      if(screenSize.iWidth <= 320 || screenSize.iHeight <= 320) {
         iFeatureHolder->SetPedestrianPositionPolyScale(float(PEDESTRIAN_POSITION_POLY_SCALE_SMALLER_SCREENS));
      }
      else {
         iFeatureHolder->SetPedestrianPositionPolyScale(float(PEDESTRIAN_POSITION_POLY_SCALE_BIGGER_SCREENS));
      }
   }
}

bool
CVectorMapContainer::getCursorVisibility() const
{
   bool visible = 
      IsFocused() && 
      !(iView->IsTracking() && m_mapMover->gotValidGpsCoord()) &&
      !m_detailedInfoTextShown && 
      !(iView->MapAsGeneralNavigation() && iView->Get3dMode());
   return visible;
}

void
CVectorMapContainer::updateCursorVisibility()
{
   bool visible = getCursorVisibility(); 
   iCursorVisAdapter->setVisible(visible);
}
   
void 
CVectorMapContainer::notifyInfoNamesAvailable()
{
   // Only get the feature name if the map didn't move since last call
   // to getInfo.
   const char* feature = m_mapMover->getInfo( true ); 
   setInfoText( feature );
}

MC2Point
CVectorMapContainer::getActivePoint() const
{
   if(iUsingLongPressPosition) {
      return iLongPressPosition;
   }
   else {
      return m_mapMover->getPoint();
   }
}

void
CVectorMapContainer::FocusChanged( TDrawNow /* aDrawNow */ )
{
   /* check if keys should be handled */
   if(!IsFocused()) {
      m_mapMover->stopKeyHandling();
   }

   updateCursorVisibility();
}

TInt CVectorMapContainer::CountComponentControls() const
{
   if (WFLayoutUtils::IsTouchScreen()) {
      return ENumControls;
   }
   return ENumControls - ENumTouchScreenControls;
}


CCoeControl* CVectorMapContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case(EMapControl):
      return iMapControl;
#ifdef USE_GLOBE
   case(EGlobeControl):
      return iGlobeControl;
#endif
   case(EMapInfoControl):
      return m_mapInfoControl;
   case ETopBorderBitmap:
      return iTopBorderVisAdapter->getControl();
   case ENextStreetCtrl:
      return iNextStreetVisAdapter->getControl();
   case EPedestrianModeIndicator:
      return iPedestrianModeVisAdapter->getControl();
   case EConBitmap:
      return iConStatusVisAdapter->getControl();
   case EDetourPicture:
      return iDetourPictureVisAdapter->getControl();
   case ESpeedCamPicture:
      return iSpeedCamPictureVisAdapter->getControl();
   case EZoomInPicture:
      return iZoomInPictureVisAdapter->getControl();
   case EZoomOutPicture:
      return iZoomOutPictureVisAdapter->getControl();
   case EGpsIndicator:
      return iGpsIndicatorVisAdapter->getControl();
   default: break;
   }
   return NULL;
}


void CVectorMapContainer::Draw(const TRect& /*aRect*/) const
{
   return;
}


void CVectorMapContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
}

static inline TileMapKeyHandler::kind_of_press_t
translateKeyEventType( TEventCode aType )
{
   switch ( aType ) {
      case EEventKeyDown:
         return TileMapKeyHandler::KEY_DOWN_EVENT;
      case EEventKeyUp:
         return TileMapKeyHandler::KEY_UP_EVENT;
      case EEventKey:
         return TileMapKeyHandler::KEY_REPEAT_EVENT;
      default:
         return TileMapKeyHandler::KEY_UNKNOWN_EVENT;
   }
   return TileMapKeyHandler::KEY_UNKNOWN_EVENT;
}

static inline TileMapKeyHandler::kind_of_press_t
translatePointerEventType( const TPointerEvent::TType aType )
{
   switch ( aType ) {
      case TPointerEvent::EButton1Down:
      case TPointerEvent::EButton2Down:
      case TPointerEvent::EButton3Down:
         return TileMapKeyHandler::KEY_DOWN_EVENT;
         break;

      case TPointerEvent::EButton1Up:
      case TPointerEvent::EButton2Up:
      case TPointerEvent::EButton3Up:
         return TileMapKeyHandler::KEY_UP_EVENT;
         break;

      case TPointerEvent::EDrag:
         return TileMapKeyHandler::KEY_REPEAT_EVENT;
         break;

      default:
         return TileMapKeyHandler::KEY_UNKNOWN_EVENT;
   }
   return TileMapKeyHandler::KEY_UNKNOWN_EVENT;
}

void
CVectorMapContainer::HandlePointerEventL( const TPointerEvent& aPointerEvent )
{
   iUsingLongPressPosition = EFalse;
   if (iView->Get3dMode()) {
      // if we're in 3d mode we dont accept pointer events.
      return;
   }

   const char* info = NULL;
      
   if (aPointerEvent.iType == TPointerEvent::EButton1Down &&
       WFLayoutUtils::IsTouchScreen()) {
      iView->SetTracking(EFalse);

      if (iZoomInPictureVisAdapter->getControl()->GetRect().Contains(aPointerEvent.iPosition)) {
         // User pressed on zoom in image
         iOnZoomIn = ETrue;
         iOnZoomOut = EFalse;
         m_mapMover->handleKeyEvent(TileMapKeyHandler::ZOOM_IN_KEY,
                                    TileMapKeyHandler::KEY_DOWN_EVENT,
                                    info);
      } else if (iZoomOutPictureVisAdapter->getControl()->GetRect().Contains(aPointerEvent.iPosition)) {
         // User pressed on zoom out image
         iOnZoomOut = ETrue;
         iOnZoomIn = EFalse;
         m_mapMover->handleKeyEvent( TileMapKeyHandler::ZOOM_OUT_KEY,
                                     TileMapKeyHandler::KEY_DOWN_EVENT,
                                     info );
      }
   } else if (aPointerEvent.iType == TPointerEvent::EDrag) {
      if(iOnZoomIn && !iZoomInPictureVisAdapter->getControl()->
         GetRect().Contains(aPointerEvent.iPosition)) {
         // If the user has clicked on the zoom in button but have
         // draged the finger or pen away from the image, send
         // key up event to the map mover. This results in stop
         // zooming.
         m_mapMover->handleKeyEvent(TileMapKeyHandler::ZOOM_IN_KEY,
                                    TileMapKeyHandler::KEY_UP_EVENT,
                                    info);  
         return;
      } else if (iOnZoomOut && !iZoomOutPictureVisAdapter->
                 getControl()->GetRect().Contains(aPointerEvent.iPosition)) {
         // If the user has clicked on the zoom out button but have
         // draged the finger or pen away from the image, send
         // key up event to the map mover. This results in stop
         // zooming.
         m_mapMover->handleKeyEvent(TileMapKeyHandler::ZOOM_OUT_KEY,
                                    TileMapKeyHandler::KEY_UP_EVENT,
                                    info);
         return;
      }
   }
   if (!iOnZoomIn && !iOnZoomOut) {
      MC2Point pos( aPointerEvent.iPosition );
      TileMapKeyHandler::kind_of_press_t upOrDown =
         translatePointerEventType(aPointerEvent.iType);
      // The user doesn't do a "perfect" tap on a phone so we check the distance from
      // the original down event as well as the number of drag events between the down
      // and up event to decide if it shall be interpreted as a normal drag or a single tap.
      TBool withinSingleTapRange = EFalse;
      TSize distance(abs(iLastPointerDownEventPosition.iX - pos.iX),
                     abs(iLastPointerDownEventPosition.iY - pos.iY));
      if(distance.iHeight < SINGLE_TAP_MAX_DISTANCE && distance.iWidth < SINGLE_TAP_MAX_DISTANCE) {
         withinSingleTapRange = ETrue;
      }
      if(upOrDown == TileMapKeyHandler::KEY_UP_EVENT &&
         iNrDragEventsInARow <= SINGLE_TAP_MAX_NR_DRAG_EVENTS &&
         withinSingleTapRange) {
         m_mapMover->handleKeyEvent(TileMapKeyHandler::CENTER_MAP_AT_POSITION_KEY,
                                    translatePointerEventType( aPointerEvent.iType ),
                                    info,
                                    &pos );
         iUpdateCursorAndInfoAtNextRedraw = ETrue;
      }
      else {
         m_mapMover->handleKeyEvent(TileMapKeyHandler::DRAG_TO,
                                    translatePointerEventType( aPointerEvent.iType ),
                                    info,
                                    &pos );
      }
      if (aPointerEvent.iType == TPointerEvent::EDrag) {
         iNrDragEventsInARow++;
      }
      else {
         iNrDragEventsInARow = 0;
         if(upOrDown == TileMapKeyHandler::KEY_DOWN_EVENT) {
            iLastPointerDownEventPosition = aPointerEvent.iPosition;
         }
      }
      
#ifdef NAV2_CLIENT_SERIES60_V5
      if (!iView->MapAsGeneralNavigation() && AknLayoutUtils::PenEnabled()) {
         iLongTapDetector->PointerEventL( aPointerEvent );
      }
#endif
   }

   if (iOnZoomIn || iOnZoomOut) {
      // If the cell position is being displayed it may need to be resized
      // if we're zooming in or out.
      UpdateCellIdIconDimensions();
   }

   if (aPointerEvent.iType == TPointerEvent::EButton1Up) {
      if (iOnZoomIn) {
         // User has clicked on zoom in and now stopped pressing,
         // send key up to the map mover
         m_mapMover->handleKeyEvent(TileMapKeyHandler::ZOOM_IN_KEY,
                                    TileMapKeyHandler::KEY_UP_EVENT,
                                    info);  
         iOnZoomIn = EFalse;
      } else if (iOnZoomOut) {
         // User has clicked on zoom out and now stopped pressing,
         // send key up to the map mover
         m_mapMover->handleKeyEvent(TileMapKeyHandler::ZOOM_OUT_KEY,
                                    TileMapKeyHandler::KEY_UP_EVENT,
                                    info);
         iOnZoomOut = EFalse;
      }
   }
   
   if ( info != NULL && ! iInfoTextPersistant ) {
      setInfoText( info, NULL, NULL, EFalse, 3000 );
   }
}

void CVectorMapContainer::HandleLongTapEventL( const TPoint& aPenEventLocation, 
                                               const TPoint& aPenEventScreenLocation )
{
#ifdef NAV2_CLIENT_SERIES60_V5
   if ( !iPopUpMenu ) {
       // Launch stylus popup menu here
       // Construct the menu from resources
       iPopUpMenu = CAknStylusPopUpMenu::NewL( iView, aPenEventScreenLocation, NULL );
       TResourceReader reader;
       iCoeEnv->CreateResourceReaderLC( reader , R_MAP_POP_UP_STYLUS_MENU );
       iPopUpMenu->ConstructFromResourceL( reader );
       CleanupStack::PopAndDestroy(); // reader
    }
    iPopUpMenu->SetPosition( aPenEventScreenLocation );
    iLongPressPosition = aPenEventLocation;
    iUsingLongPressPosition = ETrue;

    // Hide/show set as origin depending on the view state.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdMapSetOrigin, !iView->SettingOrigin());
    // Hide/show set as dest depending on the view state.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdMapSetDestination, !iView->SettingDestination());
    // Hide/show navigate to depending on the view state, should not be visible when
    // set as orig or set as dest is true.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdMapRouteTo, (iView->SettingOrigin() || 
                                                        iView->SettingDestination()));

    iPopUpMenu->ShowMenu();
#endif
}

// key event handling method
TKeyResponse CVectorMapContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                                 TEventCode aType)
{
   /**
    *   Interpolation related debug keys
    */
   
   // if( aType == EEventKeyUp &&
   //     ( aKeyEvent.iScanCode == '1' || aKeyEvent.iScanCode == '3' ) ) {
      
   //    switch( aKeyEvent.iScanCode ) {
   //    case '1':
   //       iMapControl->getMapLib()->getInterpolationHintConsumer()->
   //          cycleConfigurationBackward();
   //       break;
   //    case '3':
   //       iMapControl->getMapLib()->getInterpolationHintConsumer()->
   //          cycleConfigurationForward();
   //       break;
   //    }

   //    return EKeyWasConsumed;
   // }
   if (iWayFinderUI && iWayFinderUI->IsWaitingForRoute())
   {
   	return EKeyWasConsumed;
   }
   /* if not a key event, return */
   if(aType != EEventKeyUp && aType != EEventKeyDown && aType != EEventKey) {
      return(EKeyWasNotConsumed);
   }

   if (iView->DontHandleAsterisk() && aKeyEvent.iScanCode == KEY_STAR) {
      return EKeyWasNotConsumed;
   }

   // Turn off the autotracking timer
   // Should only be done before starting the timer.
   /*
   if ( m_autoTrackingOnTimer->IsActive() ) {
      m_autoTrackingOnTimer->Cancel();
   }
   */

   if (iView->Get3dMode() && !(aType == EEventKeyDown && aKeyEvent.iScanCode == KEY9)) {
      return EKeyWasNotConsumed;
   }
   
   if (m_detailedInfoTextShown) {
      /* The info text is shown. Check for softkey actions. */
      if (aType == EEventKeyDown) {
         if (aKeyEvent.iScanCode == KEY_SOFTKEY_RIGHT ||
             aKeyEvent.iScanCode == EStdKeyDevice3) {
            return (EKeyWasConsumed);
         } else if (aKeyEvent.iScanCode == KEY_SOFTKEY_LEFT) {
            /* Back. */
            return (EKeyWasConsumed);
         }

      } else if (aType == EEventKeyUp) {
         if (aKeyEvent.iScanCode == KEY_SOFTKEY_RIGHT ||
             aKeyEvent.iScanCode == EStdKeyDevice3 ||
             aKeyEvent.iScanCode == KEY_SOFTKEY_LEFT) {
            return (EKeyWasConsumed);
         }
      } else if (aType == EEventKey) {
         if (aKeyEvent.iScanCode == KEY_SOFTKEY_LEFT ||
             aKeyEvent.iScanCode == EStdKeyDevice3) {
            //iView->HandleCommandL(EWayFinderCmdMapShowInfo);
            //Old way of doing it in a blocking popup dialog.
            GetFeatureName();
            ShowDetailedFeatureInfo(ETrue);
            return (EKeyWasConsumed);
         } else if (aKeyEvent.iScanCode == KEY_SOFTKEY_RIGHT) {
            setInfoText(NULL);
            return (EKeyWasConsumed);
         }
      }
   }

   if( iView->Get3dMode() ) {
      if( aKeyEvent.iScanCode == KEY1 || aKeyEvent.iScanCode == ASCII_KEY_R ) {
         if( aType != EEventKeyDown ) {
            return EKeyWasNotConsumed;
         } else {
            ZoomIn3d();
            return EKeyWasConsumed;
         }
      } else if( aKeyEvent.iScanCode == KEY3 || aKeyEvent.iScanCode == ASCII_KEY_Y ) {
         if( aType != EEventKeyDown ) {
            return EKeyWasNotConsumed;
         } else {
            ZoomOut3d();
            return EKeyWasConsumed;
         }         
      }
   }

   // Check for movement keys and call the keyhandler if appropriate.
   map<int,int>::const_iterator it = m_keyMap.find( aKeyEvent.iScanCode );
   if ( it != m_keyMap.end() ) {
      if (it->second == TileMapKeyHandler::ZOOM_IN_KEY ||
          it->second == TileMapKeyHandler::ZOOM_OUT_KEY) {
         // If zooming in or out we need to update the size.
         UpdateCellIdIconDimensions();
      }
      const char* info = NULL;
      // Use the new call to the keyhandle once maplib is imported to ext-cvs.
      bool handled =
         m_mapMover->handleKeyEvent( TileMapKeyHandler::key_t( it->second ),
                                      translateKeyEventType( aType ),
                                      info );
      // The key handler will return true if the key was consumed by it.
      if ( handled || info != NULL ) {
         if ( ! iInfoTextPersistant ) {
            setInfoText( info, NULL, NULL, EFalse, 3000 );
         }
      }
      if ( handled ) {
         iView->SetTracking(EFalse);

#ifdef USE_AUTO_TRACKING_TIMER
         if (aType == EEventKeyUp) {
            /* Make sure the auto tracking on feature works. */
            if ( !iView->IsTracking() &&
                  iView->IsGpsAllowed()) {
               /* Set timer to reenable tracking. */
               if ( m_autoTrackingOnTimer->IsActive() ) {
                  m_autoTrackingOnTimer->Cancel();
               }
               m_autoTrackingOnTimer->Start(8000000, 8000000 ,
                                            TCallBack(KeyCallback,this));
            }
         }
#endif
         return EKeyWasConsumed;
      }
   }

   /* set the appropriate key in the framework array */
   if(aType == EEventKeyDown) {
      TBool removeInfoText = EFalse;
      
      // if movement, then do the first one right now
      if(aKeyEvent.iScanCode == KEY_GREEN_PHONE)
      {
         /* Call. */
         iView->Call();
      }
      // No hash for you, for now anyway
//       else if(aKeyEvent.iScanCode == EStdKeyHash)
//       {
//          if (iView->IsIronVersion()) {
//             iView->HandleCommandL(EWayFinderCmdDestination);
//             removeInfoText = ETrue;
//          } else {
//             iView->HandleCommandL(EWayFinderCmdGuide);
//             removeInfoText = ETrue;
//          }
//       }
      else if(aKeyEvent.iScanCode == KEY9)
      {
         if (iView->IsTracking() && !iView->Get3dMode()) {
            iView->SetTracking(EFalse);
         } else if (iView->IsGpsAllowed()) {
            iView->SetTracking(ETrue);
         }
         removeInfoText = ETrue;
      }

      if (removeInfoText && !iInfoTextPersistant) {
         setInfoText(NULL);
      }
      
   } else if ( aType == EEventKeyUp ) {
      /* nothing for now */
      /* since this event comes when entering the view. */
   } else if( aType == EEventKey ){
      /* feature name key */
      if(aKeyEvent.iScanCode == EStdKeyDevice3) {
         //Stop tracking
         iView->SetTracking(EFalse);
         GetFeatureName();
         if( iView->SettingDestination() ){
            iView->HandleCommandL( EWayFinderCmdMapSetDestination );
         } else if( iView->SettingOrigin() ){
            iView->HandleCommandL( EWayFinderCmdMapSetOrigin );
         } else {
            ShowDetailedFeatureInfo();
         }       
      }
#ifdef USE_AUTO_TRACKING_TIMER
      if (!iView->IsTracking() && iView->IsGpsAllowed()
          && !(aKeyEvent.iScanCode == KEY9)) {
         /* Set timer to reenable tracking. */
         if (m_autoTrackingOnTimer->IsActive()) {
            m_autoTrackingOnTimer->Cancel();
         }
         m_autoTrackingOnTimer->Start(8000000, 8000000,
                                      TCallBack(KeyCallback, this));
      }
#endif
   }
   
   return EKeyWasConsumed;
}

void
CVectorMapContainer::SetConStatusImage(CFbsBitmap* aConStatusBitmap,
                                       CFbsBitmap* aConStatusMask)
{
   if (iConStatusVisAdapter) {
      if (aConStatusBitmap && 
          (iConStatusPos.iY == -1 || iConStatusImageHeight != 
           aConStatusBitmap->SizeInPixels().iHeight)) {

         // Something has changed or is not set, set the pos again.
         if (WFLayoutUtils::IsTouchScreen() && !iView->Get3dMode()) {
            // The phone has a touch screen, the con status image should be positioned above
            // the zoom in button
            TInt iZoomButtonTl = iZoomInPictureVisAdapter->getControl()->
               GetRect().iTl.iY;
            iConStatusImageHeight = aConStatusBitmap->SizeInPixels().iHeight;
            TInt zoomButtonLeftX = iZoomOutPictureVisAdapter->getControl()->
               GetRect().iTl.iX;
            TInt zoomButtonWidth = iZoomOutPictureVisAdapter->getControl()->
               GetRect().Width();
            iConStatusPos.iX = zoomButtonLeftX + ((zoomButtonWidth >> 1) - 
                                                  (aConStatusBitmap->SizeInPixels().iWidth >> 1));
            iConStatusPos.iY = iZoomButtonTl - (iYPadding + iConStatusImageHeight);
         } else {
            iConStatusImageHeight = aConStatusBitmap->SizeInPixels().iHeight;
            iConStatusPos.iX = WFLayoutUtils::CalculateXValue(X_PADDING);
            iConStatusPos.iY = Rect().iBr.iY - 
               m_mapInfoControl->getDefaultHeight() - 
               WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING) -
               iConStatusImageHeight;
         }
      }
      iConStatusVisAdapter->setVisible(ETrue);
      iConStatusVisAdapter->getControl()->SetBitmap(iConStatusPos,
                                                    aConStatusBitmap,
                                                    aConStatusMask);
   }
}

void
CVectorMapContainer::SetGpsStatusImage(TInt aImgId, TInt aImgMaskId)
{
   if (aImgId != iGpsImgId) {
      iGpsImgId = aImgId;

      iGpsIndicatorVisAdapter->getControl()->
         CreateIconL(iView->GetMbmName(), aImgId,aImgMaskId);
   }
}
 
void CVectorMapContainer::MakeGpsIndicatorVisible(bool aVisible) {
   if(iGpsIndicatorVisAdapter) {
      if(iGpsIndicatorVisAdapter->isVisible() != aVisible) {
         iGpsIndicatorVisAdapter->setVisible(aVisible);
         PositionRightEdgeFloatingControls();
      }
   }
}

void CVectorMapContainer::SetTopBorder()
{
   if (iView->IsIronVersion()) {
      iTopBorderVisAdapter->setVisible(EFalse);
   } else {
      if (!iTopBorderVisAdapter->isVisible()) {
         iTopBorderVisAdapter->setVisible(ETrue);
         //iTopBorderVisAdapter->getControl()->SetNightModeL( iView->IsNightMode() );
         SizeChanged();
      }
   }
}

void CVectorMapContainer::HideTopBorder()
{
   if (iTopBorderVisAdapter->isVisible()) {
      iTopBorderVisAdapter->setVisible(EFalse);
      SizeChanged();
   }
}

void CVectorMapContainer::UpdatePedestrianModeIndicator()
{
   if (iPedestrianModeVisAdapter) {
      iPedestrianModeVisAdapter->setVisible(iView->MapAsPedestrianNavigation());
   }
}

void CVectorMapContainer::NextStreetCtrlMakeVisible(TBool aMakeVisible)
{
   iNextStreetVisAdapter->setVisible(aMakeVisible);
}

void CVectorMapContainer::SetTurnPictureL( TInt aMbmIndex, TInt aMbmMaskIndex )
{
   iTopBorderVisAdapter->getControl()->SetTurnImageL(aMbmIndex, aMbmMaskIndex);
}

void CVectorMapContainer::SetExitCountPictureL(TInt aMbmIndex, 
                                               TInt aMbmMaskIndex)
{
   iTopBorderVisAdapter->getControl()->SetExitCountImageL(aMbmIndex, 
                                                          aMbmMaskIndex);
}

/*
void CVectorMapContainer::HideTurnPicture()
{
   SetTurnImage(NULL, NULL);
   iDistanceBitmap->SetDistanceL(NULL);
}
*/

void
CVectorMapContainer::SetDetourPicture(TInt on)
{
   if(iDetourPictureVisAdapter->isVisible() != (on == 1)) {
      iDetourPictureVisAdapter->getControl()->SetShow(on == 1);
      iDetourPictureVisAdapter->setVisible(on == 1);
      PositionRightEdgeFloatingControls();
   }
}

void
CVectorMapContainer::SetSpeedCamPicture(TInt on)
{
   if(iSpeedCamPictureVisAdapter->isVisible() != (on == 1)) {
      iSpeedCamPictureVisAdapter->getControl()->SetShow(on == 1);
      iSpeedCamPictureVisAdapter->setVisible(on == 1);
      PositionRightEdgeFloatingControls();
   }
}


void CVectorMapContainer::SetDistanceL(TUint aDistance)
{
   iTopBorderVisAdapter->getControl()->SetDistanceL(aDistance, 
                                                    iView->GetDistanceMode());
}

void CVectorMapContainer::SetETGL(TInt aETG) 
{
   iTopBorderVisAdapter->getControl()->SetETGL(aETG);
}

void CVectorMapContainer::SetSpeedL(TInt aSpeed)
{
   iTopBorderVisAdapter->getControl()->SetSpeedL(aSpeed, 
                                                 iView->GetDistanceMode());
}

void CVectorMapContainer::SetNextStreetL(const TDesC& aNextStreet)
{
   iNextStreetVisAdapter->getControl()->SetTextL(aNextStreet);
}

void CVectorMapContainer::Connect()
{
   if( !iIsConnected ){
      iMapControl->Connect();
      iIsConnected = ETrue;
   }
   return;
}

void CVectorMapContainer::UpdateRepaint()
{
   if (iMapDrawingInterface) {
      iMapDrawingInterface->requestRepaint();
   }
}

void CVectorMapContainer::SetCenter( TInt32 aLat, TInt32 aLon )
{
   iCenter = TPoint(aLon, aLat);
   iMapMovingInterface->setCenter(Nav2Coordinate(aLat, aLon));
}


void CVectorMapContainer::SetRotation( TInt aHeading )
{
   iMapMovingInterface->setAngle((double)aHeading);
}


void CVectorMapContainer::ResetRotation()
{
   SetRotation(0);
}


void CVectorMapContainer::RequestMarkedPositionMap( TPoint aPos )
{
   SetZoom(EScale2);
   SetCenter(aPos.iY, aPos.iX);
}


void CVectorMapContainer::RequestPositionMap( TPoint aPos )
{
   SetZoom(EScale2);
   SetCenter(aPos.iY, aPos.iX);
}

void CVectorMapContainer::RequestPositionAndZoomMap( TPoint aPos, TInt aScale )
{
   SetZoom(aScale);
   SetCenter(aPos.iY, aPos.iX);
}

void CVectorMapContainer::SetRoute( TInt64 aRouteId )
{   
   iMapControl->getMapLib()->setRouteID( RouteID( aRouteId ) );
}

void CVectorMapContainer::ClearRoute( )
{
   iMapControl->getMapLib()->clearRouteID( );
   HideTopBorder();
   NextStreetCtrlMakeVisible(EFalse);
   iPedestrianModeVisAdapter->setVisible(EFalse);
}


void CVectorMapContainer::RequestRouteMap( TPoint aTl, TPoint aBr )
{
   iMapMovingInterface->setWorldBox(Nav2Coordinate( aTl.iY, aTl.iX ),
                                    Nav2Coordinate( aBr.iY, aBr.iX ));
}


void CVectorMapContainer::SetZoom( TInt aScale )
{
   iScale = aScale;
   iMapMovingInterface->setScale(aScale);
   CheckFavoriteRedraw();
}

void CVectorMapContainer::ZoomToOverview()
{
#ifdef USE_GLOBE
   SetZoom(EScaleGlobe);
#else
   SetZoom(EScale12);
#endif
}

void
CVectorMapContainer::CheckFavoriteRedraw()
{
   TInt aScale = GetScale();
   if ( aScale > ROTATE_NORTH_SCALE ) {
      /* Might just as well change rotation. */
      ResetRotation();
   }
   CSettingsData* aData = iView->GetSettingsData();
   
   /* Check if we want to show favorites. */
   if(aData->m_favoriteShowInMap == isab::GuiProtEnums::ShowFavoriteInMapNever ||
      (aScale > EScale6 &&
       aData->m_favoriteShowInMap != isab::GuiProtEnums::ShowFavoriteInMapAlways)) {
      /* Never show or scale is high and show always is not set. */
      /* Don't show favorites. */

      if (iFeatureHolder->GetCurrentFavIconSize() >= 1) { 
         iFeatureHolder->SetCurrentFavIconSize(0);
         iView->FavoriteChanged();
      }
   }
   else if( aScale > EScale4 &&
         aData->m_favoriteShowInMap != isab::GuiProtEnums::ShowFavoriteInMapNever) {
      /* Scale is medium and never show is not set. */
      /* Show as small stars. */

      if (iFeatureHolder->GetCurrentFavIconSize() >= 2 ||
          iFeatureHolder->GetCurrentFavIconSize() <= 0) { 
         iFeatureHolder->SetCurrentFavIconSize(1);
         iView->FavoriteChanged();
      }
   }
   else if( aScale >= EScaleMin &&
         aData->m_favoriteShowInMap != isab::GuiProtEnums::ShowFavoriteInMapNever) {
      /* Scale is small and never show is not set. */
      /* Show as large stars. */
      if (iFeatureHolder->GetCurrentFavIconSize() <= 1) { 
         iFeatureHolder->SetCurrentFavIconSize(2);
         iView->FavoriteChanged();
      }
   }
}

void CVectorMapContainer::ZoomIn()
{
   TInt scale = (TInt)(iMapMovingInterface->getScale() + 0.5);
   /*if( scale >= EScale12*2 ){
      scale /= 2; 
   }
   else*/ if( scale > EScale12 ){
      scale = EScale12;
   }
   else if( scale > EScale11 ){
      scale = EScale11;
   }
   else if( scale > EScale10 ){
      scale = EScale10;
   }
   else if( scale > EScale9 ){
      scale = EScale9;
   }
   else if( scale > EScale8 ){
      scale = EScale8;
   }
   else if( scale > EScale7 ){
      scale = EScale7;
   }
   else if( scale > EScale6 ){
      scale = EScale6;
   }
   else if( scale > EScale5 ){
      scale = EScale5;
   }
   else if( scale > EScale4 ){
      scale = EScale4;
   }
   else if( scale > EScale3 ){
      scale = EScale3;
   }
   else if( scale > EScale2 ){
      scale = EScale2;
   }
   else if( scale > EScale1 ){
      scale = EScale1;
   }
   else /*if( scale > EScaleMin )*/{
      scale = EScaleMin;
   }
   SetZoom(scale);

}

void CVectorMapContainer::ZoomOut()
{
   TInt scale = (TInt)(iMapMovingInterface->getScale() + 0.5);
   if( scale <= EScaleMin ){
      scale = EScaleMin;
   }
   else if( scale < EScale1 ){
      scale = EScale1;
   }
   else if( scale < EScale2 ){
      scale = EScale2;
   }
   else if( scale < EScale3 ){
      scale = EScale3;
   }
   else if( scale < EScale4 ){
      scale = EScale4;
   }
   else if( scale < EScale5 ){
      scale = EScale5;
   }
   else if( scale < EScale6 ){
      scale = EScale6;
   }
   else if( scale < EScale7 ){
      scale = EScale7;
   }
   else if( scale < EScale8 ){
      scale = EScale8;
   }
   else if( scale < EScale9 ){
      scale = EScale9;
   }
   else if( scale < EScale10 ){
      scale = EScale10;
   }
   else if( scale < EScale11 ){
      scale = EScale11;
   }
   else /*if( scale < EScale12 )*/{
      scale = EScale12;
   }
   /*else if( scale < EScale12*2 ){
      scale = EScale12*2; 
   }
   else{
      scale *= 2; 
   }*/
   SetZoom(scale);
}

void CVectorMapContainer::ZoomOut3d()
{
   TInt scale = (TInt)(iMapMovingInterface->getScale() + 0.5);
   if ( scale < EScale1 ) {
      scale = EScale1;
   } else if ( scale < EScale3 ) {
      scale = EScale3;
   } else {
      scale = EScale5;
   }
   SetZoom( scale );
   iMapDrawingInterface->requestRepaint();
}

void CVectorMapContainer::ZoomIn3d()
{
   TInt scale = (TInt)(iMapMovingInterface->getScale() + 0.5);
   if ( scale > EScale5 ) {
      scale = EScale5;
   } else if ( scale > EScale3 ) {
      scale = EScale3;
   } else {
      scale = EScale1;
   }
   SetZoom( scale );   
   iMapDrawingInterface->requestRepaint();
}

MC2Point
CVectorMapContainer::getTrackingPoint() const
{
   TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
   if ( iView->GetTrackingType() == ERotating ) {
      // Use same point for 2D and 3D. If 3D point should be modified then
      // remove the if false.
      if ( false && iView->Get3dMode() ) { 
         return MC2Point ( iMapControl->Size().iWidth >> 1 ,
                           iMapControl->Size().iHeight - ( 25 * dpiCorrectionFactor ) );
      } else if (iView->HasRoute()) {
         // If we have a route we want the track point to be in the lower 
         // part of the screen.
         return MC2Point( iMapControl->Size().iWidth >>1,
                          iMapControl->Size().iHeight - 
                          TInt( iMapControl->Size().iHeight * 0.22 ) );
      } else {
         // Otherwise we want to center to position in the map.
         return MC2Point( ( iMapControl->Size().iWidth >>1 ),
                          ( iMapControl->Size().iHeight >>1 ) );
      }
   } else {
      // Use same point for 2D and 3D. If 3D point should be modified then
      // remove the if false.
      if ( false && iView->Get3dMode() ) {
         return MC2Point( iMapControl->Size().iWidth >> 1,
                          iMapControl->Size().iHeight - ( 35 * dpiCorrectionFactor ) );
      } else {
         return MC2Point( ( iMapControl->Size().iWidth >>1 ),
                          ( iMapControl->Size().iHeight >>1 ) );
      }
   }
}

MC2Point
CVectorMapContainer::getNonTrackingPoint( bool screenSizeChanged ) const
{
   if (!screenSizeChanged) {
      // Full screen and screen size didn't change.
      // Places cursor at the gps.
      return getTrackingPoint();
   } else {
      // Normal screen or switching to fullscreen. 
      // Place cursor in the center.
      return MC2Point( iMapControl->Size().iWidth >> 1,
                       iMapControl->Size().iHeight >> 1 );
   }
}

void
CVectorMapContainer::ShowUserPos( TBool aShow )
{
   if( !aShow ){
      /* Remove feature from vector. */
      CMapFeature* mf = 
         iFeatureHolder->RemoveFeature(iFeatureHolder->iPositionPoly);
      iFeatureHolder->iPositionPolyShown = EFalse;
      delete mf;
      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iDarkShadePoly);
      delete mf;
      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iLightshadePoly);
      delete mf;
      // Hide the pedestrian position indicator as well.
      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iPedPositionPoly);
      iFeatureHolder->iPedPositionPolyShown = EFalse;
      delete mf;
      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iPedPositionFillPoly);
      delete mf;
   } else if (!iFeatureHolder->iPedPositionPolyShown && 
              iView->MapAsPedestrianNavigation()) {
      // If we're doning pedestrian navigation and the position indicator 
      // for pedestrian mode is not shown, we hide the normal position 
      // indicator and show the pedestrian one.
      if (iFeatureHolder->iPositionPolyShown) {
         CMapFeature* mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iPositionPoly);
         iFeatureHolder->iPositionPolyShown = EFalse;
         delete mf;
         mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iDarkShadePoly);
         delete mf;
         mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iLightshadePoly);
         delete mf;
      }
      CMapFeature* mf = new (ELeave) CMapFeature(map_feature_gps, 0);
      iFeatureHolder->AddFeature(iFeatureHolder->iPedPositionPoly, mf);
      mf = new (ELeave) CMapFeature(map_feature_gps, 1);
      iFeatureHolder->AddFeature(iFeatureHolder->iPedPositionFillPoly, mf);
      iFeatureHolder->iPedPositionPolyShown = ETrue;
   } else if (!iFeatureHolder->iPositionPolyShown && 
              !iView->MapAsPedestrianNavigation()) {
      // We're doing car navigation so hide the pedestrian indicator and 
      // show the car indicator.
      if (iFeatureHolder->iPedPositionPolyShown) {
         CMapFeature* mf = 
            iFeatureHolder->RemoveFeature(iFeatureHolder->iPedPositionPoly);
         iFeatureHolder->iPedPositionPolyShown = EFalse;
         delete mf;
         mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iPedPositionFillPoly);
         delete mf;
      }
      CMapFeature* mf = new (ELeave) CMapFeature(map_feature_gps, 0);
      iFeatureHolder->AddFeature(iFeatureHolder->iPositionPoly, mf);
      mf = new (ELeave) CMapFeature(map_feature_gps, 1);
      iFeatureHolder->AddFeature(iFeatureHolder->iDarkShadePoly, mf);
      mf = new (ELeave) CMapFeature(map_feature_gps, 2);
      iFeatureHolder->AddFeature(iFeatureHolder->iLightshadePoly, mf);
      iFeatureHolder->iPositionPolyShown = ETrue;
   }
}

void
CVectorMapContainer::ShowNorthArrow( TBool aShow )
{
   if( !aShow ) {
      CMapFeature* mf;

      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iNorthPoly);
      delete mf;
      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iSouthPoly);
      delete mf;
      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iArrowPoly);
      delete mf;
      iFeatureHolder->iCompassPolyShown = EFalse;
   } else if (!iFeatureHolder->iCompassPolyShown ) {
      CMapFeature* mf;
      mf = new (ELeave) CMapFeature(map_feature_compass, 0);
      iFeatureHolder->AddFeature( iFeatureHolder->iArrowPoly, mf );
      mf = new (ELeave) CMapFeature(map_feature_compass, 1);
      iFeatureHolder->AddFeature( iFeatureHolder->iNorthPoly, mf );
      mf = new (ELeave) CMapFeature(map_feature_compass, 2);
      iFeatureHolder->AddFeature( iFeatureHolder->iSouthPoly, mf );

      TInt x = 0;
      TInt y = 0;
      CalcCompassPosition(x, y);

      iFeatureHolder->iNorthPoly->setCenter( MC2Point(x,y) );
      iFeatureHolder->iSouthPoly->setCenter( MC2Point(x,y) );
      iFeatureHolder->iArrowPoly->setCenter( MC2Point(x,y) );
      iFeatureHolder->iCompassPolyShown = ETrue;
   }
}

void CVectorMapContainer::ResetNorthArrowPos()
{
   if (iFeatureHolder->iCompassPolyShown) {

      TInt x = 0;
      TInt y = 0;
      CalcCompassPosition(x, y);

      iFeatureHolder->iNorthPoly->setCenter(MC2Point(x,y));
      iFeatureHolder->iSouthPoly->setCenter(MC2Point(x,y));
      iFeatureHolder->iArrowPoly->setCenter(MC2Point(x,y));
   }
}

void CVectorMapContainer::CalcCompassPosition(TInt &aX, TInt &aY)
{
   TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
   aX = (COMPASS_POLY_HEIGHT / 2) * dpiCorrectionFactor;
   aY = aX + WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   aX += WFLayoutUtils::CalculateXValue(X_PADDING);
   // Since the top border bar is not a part of the MapLib rect, it is
   // positioned above the maps drawing rect, there is no need to take
   // this into concideration when positioning the compass poly
   if (iNextStreetVisAdapter->isVisible()) {
      aY += iNextStreetVisAdapter->getControl()->Rect().Height();
   }
   if (iTopBorderVisAdapter->isVisible()) {
      aY += iTopBorderVisAdapter->getControl()->Rect().Height();
   }
}

void CVectorMapContainer::PositionRightEdgeFloatingControls()
{
   //calculate the rect for the first control
   TSize imgSize = WFLayoutUtils::CalculateSizeUsingMin(INFO_IMAGES_SIZE);
   TPoint firstCtrlPos = Rect().iBr;

   firstCtrlPos.iX -= (imgSize.iWidth + WFLayoutUtils::CalculateXValue(X_PADDING));
   firstCtrlPos.iY = WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING);
   if (iNextStreetVisAdapter->isVisible()) {
      firstCtrlPos.iY += iNextStreetVisAdapter->getControl()->Rect().Height();
   }
   if (iTopBorderVisAdapter->isVisible()) {
      firstCtrlPos.iY += iTopBorderVisAdapter->getControl()->Rect().Height();
   }
   TRect currentControlRect(firstCtrlPos, imgSize);

   // position the gps image
   if(iGpsIndicatorVisAdapter->isVisible()) {
      iGpsIndicatorVisAdapter->getControl()->SetImageRect(currentControlRect);
      currentControlRect.Move(0,
         WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING) +
            currentControlRect.Height());
   }

   //position the detour
   if(iDetourPictureVisAdapter->isVisible()) {
      iDetourPictureVisAdapter->getControl()->SetImageRect(currentControlRect);
      currentControlRect.Move(0,
         WFLayoutUtils::CalculateYValueUsingFullScreen(Y_PADDING) +
            currentControlRect.Height());
   }

   // Position the speedcam image
   if(iSpeedCamPictureVisAdapter->isVisible()) {
      iSpeedCamPictureVisAdapter->getControl()->SetImageRect(currentControlRect);
   }
}

void
CVectorMapContainer::ShowScale( TBool aShow )
{
   if( !aShow ) {
      CMapFeature* mf;

      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iScaleFeature);
      delete mf;
      iFeatureHolder->iScaleFeatureShown = EFalse;
   } else if (!iFeatureHolder->iScaleFeatureShown ) {
      CMapFeature* mf;
      mf = new (ELeave) CMapFeature(map_feature_scale, 0);
      iFeatureHolder->AddFeature( iFeatureHolder->iScaleFeature, mf );
      TInt x, y;
      if (WFLayoutUtils::IsTouchScreen()) {
         // If the zoom in and out buttons are valid then the scale should be 
         // positioned above these.
         x = Rect().Width() - iXPadding;
         y = iMapControl->Rect().Height() - 
            m_mapInfoControl->getDefaultHeight() - iYPadding - 
            iZoomInPictureVisAdapter->getControl()->GetRect().Height() - 
            iYPadding;
      } else {
         x = Rect().Width() - iXPadding;
         y = iMapControl->Rect().Height() - 
            m_mapInfoControl->getDefaultHeight() - iYPadding;
      }
      iFeatureHolder->iScaleFeature->setPoint( MC2Point(x,y) );
      iFeatureHolder->iScaleFeatureShown = ETrue;
   } else {
      TInt x, y;
      if (WFLayoutUtils::IsTouchScreen()) {
         // If the zoom in and out buttons are valid then the scale should be 
         // positioned above these.
         x = Rect().Width() - iXPadding;
         y = iMapControl->Rect().Height() - 
            m_mapInfoControl->getDefaultHeight() - iYPadding - 
            iZoomInPictureVisAdapter->getControl()->GetRect().Height() - 
            iYPadding;
      } else {
         x = Rect().Width() - iXPadding;
         y = iMapControl->Rect().Height() - 
            m_mapInfoControl->getDefaultHeight() - iYPadding;
      }
      iFeatureHolder->iScaleFeature->setPoint( MC2Point(x,y) );
      iFeatureHolder->iScaleFeatureShown = ETrue;
   }
}

void
CVectorMapContainer::ShowEnd( TBool aShow, TInt32 aLat, TInt32 aLon )
{
   if( !aShow ){
      CMapFeature* mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iEndBitmap);
      delete mf;
      iFeatureHolder->iEndBitmapShown = EFalse;
   } else {
      CMapFeature* mf = iFeatureHolder->GetFeature(iFeatureHolder->iEndBitmap);
      if (mf != NULL) {
         mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iEndBitmap);
         delete mf;
      }
      mf = new (ELeave) CMapFeature(map_feature_pin, 0);
      iFeatureHolder->AddFeature( iFeatureHolder->iEndBitmap, mf );
      iFeatureHolder->iEndBitmap->setCenter( Nav2Coordinate(aLat, aLon) );
      iFeatureHolder->iEndBitmapShown = ETrue;
   }
}
void
CVectorMapContainer::ShowStart( TBool aShow, TInt32 aLat, TInt32 aLon )
{
   if( !aShow ){
      CMapFeature* mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iStartBitmap);
      delete mf;
      iFeatureHolder->iStartBitmapShown = EFalse;
   } else {
      CMapFeature* mf = iFeatureHolder->GetFeature(iFeatureHolder->iStartBitmap);
      if (mf != NULL) {
         mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iStartBitmap);
         delete mf;
      }
      mf = new (ELeave) CMapFeature(map_feature_pin, 0);
      iFeatureHolder->AddFeature( iFeatureHolder->iStartBitmap, mf );
      iFeatureHolder->iStartBitmap->setCenter( Nav2Coordinate(aLat, aLon) );
      iFeatureHolder->iStartBitmapShown = ETrue;
   }
}


void CVectorMapContainer::ShowPoint( TBool aShow, TInt32 aLat, TInt32 aLon )
{
   if( !aShow ){
      CMapFeature* mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iPointBitmap);
      delete mf;
      iFeatureHolder->iPointBitmapShown = EFalse;
   } else {
      CMapFeature* mf = iFeatureHolder->GetFeature(iFeatureHolder->iPointBitmap);
      if (mf != NULL) {
         mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iPointBitmap);
         delete mf;
      }
      mf = new (ELeave) CMapFeature(map_feature_pin, 0);
      iFeatureHolder->AddFeature( iFeatureHolder->iPointBitmap, mf );
      iFeatureHolder->iPointBitmap->setCenter( Nav2Coordinate(aLat, aLon) );
      iFeatureHolder->iPointBitmapShown = ETrue;
   }
}

void CVectorMapContainer::GetCoordinate( TPoint& aRealCoord )
{
   MC2Coordinate mc2Coord = m_mapMover->getCoordinate(); 

   Nav2Coordinate nav2Coord( mc2Coord );
   aRealCoord.iY = nav2Coord.nav2lat;
   aRealCoord.iX = nav2Coord.nav2lon;
}

void CVectorMapContainer::GetLongPressOrNormalCoordinate( TPoint& aRealCoord )
{
   if(iUsingLongPressPosition) {
      MC2Coordinate mc2Coord;
      iMapMovingInterface->inverseTransform( mc2Coord,  iLongPressPosition);
      Nav2Coordinate nav2Coord( mc2Coord );

      aRealCoord.iY = nav2Coord.nav2lat;
      aRealCoord.iX = nav2Coord.nav2lon;
   }
   else {
      MC2Coordinate mc2Coord = m_mapMover->getCoordinate();

      Nav2Coordinate nav2Coord( mc2Coord );
      aRealCoord.iY = nav2Coord.nav2lat;
      aRealCoord.iX = nav2Coord.nav2lon;
   }
}

void CVectorMapContainer::GetCoordinate( MC2Coordinate& aMC2Coord )
{
   aMC2Coord = m_mapMover->getCoordinate();
}

void CVectorMapContainer::GetLongPressOrNormalCoordinate( MC2Coordinate& aMC2Coord )
{
   if(iUsingLongPressPosition) {
      iMapMovingInterface->inverseTransform( aMC2Coord,  iLongPressPosition);
   }
   else {
      aMC2Coord = m_mapMover->getCoordinate();
   }
}

void CVectorMapContainer::setInfoText(const char *txt,
      const char *left,
      const char *right,
      TBool persistant,
      uint32 hideAfterMS ) 
{
   if ( txt == NULL && !iInfoTextShown ) {
      return;
   }
   
   // Cancel the hide timer.
   if ( m_hideInfoTimerID != MAX_UINT32 ) {
      iMapControl->getToolkit()->cancelTimer( this, m_hideInfoTimerID );
   }
   if (txt == NULL) {
      iInfoTextShown = EFalse;
      m_detailedInfoTextShown = false;
      iInfoTextPersistant = EFalse;
      iView->ReleaseInfoText();
//      iMapControl->setInfoText(txt);
      m_mapInfoControl->setInfoText(txt);
      //iCopyrightLabel->MakeVisible(ETrue);
      iView->ToggleSoftKeys(ETrue);
      if (iZoomInPictureVisAdapter && iZoomOutPictureVisAdapter) {
         // If we have zoom buttons...
         if (WFLayoutUtils::IsTouchScreen() && 
             !iView->Get3dMode() && 
             !iZoomInPictureVisAdapter->isVisible()) {
            // Un-hide the zoom buttons if they should be shown again.
            iZoomInPictureVisAdapter->setVisible(ETrue);
            iZoomOutPictureVisAdapter->setVisible(ETrue); 
         }
      }
   } else {
      if (iMapControl) {
         if ( left != NULL || right != NULL ) {
            m_detailedInfoTextShown = true;
         } else {
            m_detailedInfoTextShown = false;
         }
         iInfoTextShown = true;
         iInfoTextPersistant = persistant;
//         iMapControl->setInfoText(txt, left, right);
         m_mapInfoControl->setInfoText(txt, left, right);
         //iCopyrightLabel->MakeVisible(EFalse);
         iView->ToggleSoftKeys( !m_detailedInfoTextShown );

         // Enable the hide timer if requested.
         if ( hideAfterMS != MAX_UINT32 ) {
            m_hideInfoTimerID = iMapControl->getToolkit()->requestTimer( 
                                          this,
                                          hideAfterMS,
                                          TileMapToolkit::PRIO_LOW );
         }

         if (iZoomInPictureVisAdapter && iZoomOutPictureVisAdapter) {
            // If we have zoom buttons...
            if ((Rect().iBr.iY - m_mapInfoControl->getInfoBox().Height()) < 
                iZoomInPictureVisAdapter->getControl()->GetRect().iBr.iY) {
               // The info text covers the zoom buttons so hide them temporarily.
               iZoomInPictureVisAdapter->setVisible(EFalse);
               iZoomOutPictureVisAdapter->setVisible(EFalse);
            } else if (WFLayoutUtils::IsTouchScreen() && 
                       !iView->Get3dMode() && 
                       !iZoomInPictureVisAdapter->isVisible()) {
               // Un-hide the zoom buttons if they should be shown again and 
               // the info text does not cover them.
               iZoomInPictureVisAdapter->setVisible(ETrue);
               iZoomOutPictureVisAdapter->setVisible(ETrue); 
            }
         }
      }
   }
   updateCursorVisibility();
}

TBool
CVectorMapContainer::IsInfoTextOn()
{
   return iInfoTextShown;
}

void CVectorMapContainer::ShowFeatureInfo()
{
   const char* feature = m_mapMover->getInfo(); 
   setInfoText( feature );
}

HBufC* CVectorMapContainer::GetServerStringL()
{
   const ClickInfo& clickedFeature = 
      iMapMovingInterface->getInfoForFeatureAt(getActivePoint(), false);
   const char* charServerString = clickedFeature.getServerString();

   HBufC* serverString = NULL;
   if (charServerString != NULL) {
      char* urlencodedString = new char[3 * strlen(charServerString) + 1];
      StringUtility::URLEncode(urlencodedString, charServerString);
      serverString = WFTextUtil::AllocL(urlencodedString);
      delete[] urlencodedString;
   }

   return serverString;
}

void CVectorMapContainer::ShowDetailedFeatureInfo(TBool showWithList)
{

   const ClickInfo& aClickedFeature = 
      iMapMovingInterface->getInfoForFeatureAt( getActivePoint(),
                                                false ); // highlightable

   UserDefinedFeature* udf = aClickedFeature.getClickedUserFeature();

   if (udf) {
      /* User clicked a user defined feature. */
      CMapFeature* mf = iFeatureHolder->GetFeature(udf);
      if (mf) {
         /* Got it. */
         if (mf->Type() == map_feature_favorite) {
            /* Got a favorite. */
            uint32 id = (uint32)mf->Id();
            iView->RequestFavorite(id);
         }
      }
   } else {
      /* No user defined feature, check if we have anything else */
      const char *serverString = aClickedFeature.getServerString();
      const char *name = aClickedFeature.getName();
       if (serverString && (name && strlen(name) != 0)) {
         /* Got a map feature. */
         setInfoText(name);
         /* Send request for additional info. */
         iView->GetDetails( serverString, showWithList );
      } else {
         setInfoText( NULL );
      }
   }
}
 
void CVectorMapContainer::GetFeatureName()
{
   const char* feature = 
      iMapControl->getFeatureName( getActivePoint() );
   if( feature == NULL ){
      HBufC* unknownString = iCoeEnv->AllocReadResourceL(
                                                     R_WAYFINDER_UNKNOWN_TEXT);
      feature = WFTextUtil::TDesCToUtf8L(unknownString->Des());
   }
   iView->SetMapFeatureName( (const unsigned char*)feature );
}


TInt CVectorMapContainer::GetScale()
{
   return (TInt)(iMapMovingInterface->getScale() + 0.5);
}

void CVectorMapContainer::setTrackingMode( TBool /*aOnOff*/,
                                           TrackingType aType,
                                           bool interpolating )
{
   if (iMapControl) {

      bool northUp = ( aType != ERotating );

      m_mapMover->statusChanged( getTrackingPoint(),
                                 getNonTrackingPoint(),
                                 iView->IsTracking(),
                                 northUp,
                                 interpolating );
      updateCursorVisibility();

   }
}


void CVectorMapContainer::SetCellIdIconEnabled(bool enabled)
{
   if(enabled) {
      iFeatureHolder->iCellIdBitmap->setVisible(true);
   } else {
      iFeatureHolder->iCellIdBitmap->setVisible(false);
   }

   UpdateRepaint();
}

void CVectorMapContainer::UpdateCellIdIconDimensions()
{
   if ( iFeatureHolder->iCellIdBitmap->isVisible() ) {
     // use the last known radius
      UpdateCellIdIconDimensions(iRadiusMeters);
      UpdateRepaint();
   }
}

#define MIN_PIXEL_RADIUS 25
void CVectorMapContainer::UpdateCellIdIconDimensions(int aRadiusMeters)
{
   iRadiusMeters = aRadiusMeters;

   int pixelDiameter =
      static_cast<int>((aRadiusMeters * 
                        WFLayoutUtils::CalculateDpiCorrectionFactor() * 2) / 
                       iMapMovingInterface->getScale());
   
   // set a minimum size for the cell id so it is still visible when you zoom out
   if ( pixelDiameter < MIN_PIXEL_RADIUS ) {
      pixelDiameter = MIN_PIXEL_RADIUS;
   }

   // The map/phone gets really slow when rescaling an image that is huge, 
   // so we limit the resize of the cellId bitmap to 1.5 times the largest
   // size of the screen.
   TInt maxSize = WFLayoutUtils::CalculateUnitsUsingMax(float(1.5));
   if (pixelDiameter < maxSize) {
      iFeatureHolder->iCellIdBitmap->setDimensions(pixelDiameter, 
                                                   pixelDiameter);
   }
}

void CVectorMapContainer::UpdateCellIdPosition(const MC2Coordinate& coord)
{
   iFeatureHolder->iCellIdBitmap->setCenter(coord);
}

void 
CVectorMapContainer::ShowVectorMapWaitSymbol( bool start )
{
   // Only start showing the wait symbol in map.
   if (start) {
      // Lower left corner.
      TInt x = 8;
      TInt y = Rect().Height() - 33;
      iFeatureHolder->iWaitSymbol->setCenter( MC2Point(x,y) );
      iFeatureHolder->iWaitSymbol->setVisible( true );
      UpdateRepaint();
   } else {
      // Simply set it to not be visible. No need to remove etc.
      iFeatureHolder->iWaitSymbol->setVisible( false );
      UpdateRepaint();
   }
}

/* key handler */
void CVectorMapContainer::TimerHandler() 
{
#ifdef USE_AUTO_TRACKING_TIMER
   /* Timer used as timeout for tracking. */
   if (iView->UseTrackingOnAuto()) {
      iView->SetTracking(ETrue);
   }
   if ( m_autoTrackingOnTimer->IsActive() ) {
      m_autoTrackingOnTimer->Cancel();
   }
#endif
}

bool 
CVectorMapContainer::setGpsPos( const MC2Coordinate& coord, 
                                int direction, 
                                int scale,
                                bool interpolated )
{
//    if (iView->MapAsPedestrianNavigation()) {
//       setPedestrianGpsPos(coord);
//    }
   return m_mapMover->setGpsPos( coord, direction, scale, interpolated );
}

#if 0
void CVectorMapContainer::setPedestrianGpsPos(const MC2Coordinate& coord)
{
   MC2Point screenPos(0,0);
   iMapMovingInterface->transform(screenPos, coord);
   iPedestrianPositionVisAdapter->getControl()->SetCenter(screenPos);
}
#endif

// returns Input info about the app 
// required if the derived class is implementing OfferKeyEventL()
TCoeInputCapabilities CVectorMapContainer::InputCapabilities() const
{
	return(TCoeInputCapabilities(TCoeInputCapabilities::EAllText));
}

int CVectorMapContainer::getMemCacheSize() const
{
#ifdef NAV2_CLIENT_SERIES60_V1
   int maxSize = 64*1024;
#else
   int maxSize = 128*1024;
#endif
   int memFree = 0;
#ifndef NAV2_CLIENT_SERIES60_V3
   HAL::Get(HALData::EMemoryRAMFree, memFree );
#else
   // We cannot get any usable measure of how much memory is available.
   // Let's assume we can afford the full memory cache.
   memFree = maxSize * 2;
#endif

//   TBuf<64> str;
//   str.Format(_L("Memory free : %i"), memFree );
//   CEikonEnv::Static()->InfoMsg( str );
//   SHOWMSGWIN( str );
   return MIN( maxSize, memFree / 2);
}

void
CVectorMapContainer::timerExpired( uint32 id )
{
   if ( m_hideInfoTimerID == id ) {
      // Hide the blue note containing info.
      setInfoText( NULL );
      
      // Reset the timer id.
      m_hideInfoTimerID = MAX_UINT32;
   }
}

void CVectorMapContainer::AddSfdCacheFiles()
{
   MapLib* mapLib = iMapControl->getMapLib();
   
   if ( mapLib == NULL ) {
      return;
   }
   
   _LIT(KWfdName, "*.wfd");
   class RFs fs;
   fs.Connect();
   TFindFile finder(fs);
   CDir* dir;
   TInt res = finder.FindWildByDir(KWfdName, iView->GetMapCachePath(), dir);
   // Add all sfd-files found.
   while (res == KErrNone) {
      for (TInt i = 0; i < dir->Count(); i++) {
         TParse parser;
         parser.Set((*dir)[i].iName, &finder.File(), NULL);
         char* fullPath = WFTextUtil::TDesCToUtf8LC(parser.FullName());
         mapLib->addSingleFileCache(fullPath, NULL);
         CleanupStack::PopAndDestroy(fullPath);
      }
      delete dir;
      dir = NULL;
      // Continue the search on the next drive.
      res = finder.FindWild(dir);
   }
   fs.Close();
}

void
CVectorMapContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      BindMovementKeys(WFLayoutUtils::LandscapeMode());
      TInt dpiCorrFact = WFLayoutUtils::CalculateDpiCorrectionFactor();
      TRect vectorMapRect;
      // Get location of cba buttons.
      AknLayoutUtils::TAknCbaLocation cbaLocation = 
         AknLayoutUtils::CbaLocation();
      if (WFLayoutUtils::LandscapeMode() && !WFLayoutUtils::IsTouchScreen() && 
          (cbaLocation != AknLayoutUtils::EAknCbaLocationBottom)) {
         // If we're in landscape mode, not touch screen and we dont have 
         // the CBAs in the bottom of the screen (like e71) we have to
         // let the top and bottom pane be visible. 
         vectorMapRect = WFLayoutUtils::GetMainPaneRect();
      } else {
         // Otherwise the only system pane we show is the CBA buttons.
         vectorMapRect = WFLayoutUtils::GetFullScreenMinusCBARect();
      }
      SetRect(vectorMapRect);
      MapLib* mapLib = iMapControl->getMapLib();
      if(mapLib) {
         mapLib->setDPICorrectionFactor(dpiCorrFact);
      }
      iFeatureHolder->RescalePolygons();
      m_cursorSprite->updateSize();
      m_highlightCursorSprite->updateSize();
      iView->GetDpiCorrFact() = dpiCorrFact;
   }
   SetNightModeL( iView->IsNightMode() ); 
}

void
CVectorMapContainer::BindMovementKeys(TBool aLandscapeMode)
{
   int nbrBindings = 0;
   const CVectorMapContainer::key_array_pair_t* keyBindings = NULL;
   if (aLandscapeMode) {
      nbrBindings = sizeof(c_landscapeKeyBindings) / sizeof(c_landscapeKeyBindings[0]);
      keyBindings = c_landscapeKeyBindings;
   } else {
      nbrBindings = sizeof(c_portraitKeyBindings) / sizeof(c_portraitKeyBindings[0]);
      keyBindings = c_portraitKeyBindings;
   }
   for (int i = nbrBindings - 1; i >= 0; --i) {
      m_keyMap[keyBindings[i].first] = keyBindings[i].second;
   }
}

void
CVectorMapContainer::SetNightModeL(TBool aNightMode)
{
   if (iTopBorderVisAdapter->getControl()) {
      iTopBorderVisAdapter->getControl()->SetNightModeL(aNightMode);
   }
   if (iNextStreetVisAdapter->getControl()) {
      iNextStreetVisAdapter->getControl()->SetNightModeL(aNightMode);
   }
   if (m_mapInfoControl) {
      m_mapInfoControl->SetNightMode(aNightMode);
   }
   if (iMapControl) {
      iMapControl->SetNightModeL(aNightMode);
   }
}

void
CVectorMapContainer::Set3dMode(TBool aOn) 
{
   if (aOn) {
      if (iMapControl) {
//          iMapControl->getMapLib()->
//             setHorizonHeight(WFLayoutUtils::CalculateYValueUsingFullScreen(BLUE_BAR_HEIGHT));
      }
   }
   if (iZoomInPictureVisAdapter && iZoomOutPictureVisAdapter) {
      // Zoom buttons should not be visible in 3d mode.
      iZoomInPictureVisAdapter->setVisible(!aOn);
      iZoomOutPictureVisAdapter->setVisible(!aOn);
   }

   SizeChanged();
}

void CVectorMapContainer::SetMapControlsNull()
{
   ReleaseMapControlDependencies();
   iMapControl = NULL;
   iFeatureHolder = NULL;
   m_mapInfoControl = NULL;
   m_keyHandler = NULL;
}

void CVectorMapContainer::ResetLongPressPositionFlag()
{
   iUsingLongPressPosition = EFalse;
}

void CVectorMapContainer::handleTileMapEvent(const class TileMapEvent &event)
{
   switch (event.getType()) {
   case TileMapEvent::USER_DEFINED_FEATURES_REDRAWN:
      if(m_keyHandler && iUpdateCursorAndInfoAtNextRedraw) {
         const char* featureText = m_keyHandler->checkHighlight();
         setInfoText(featureText, NULL, NULL, EFalse, 3000);
         iUpdateCursorAndInfoAtNextRedraw = EFalse;
      }
      break;
   default:
      break;
   }
}

void CVectorMapContainer::ReleaseMapControlDependencies()
{
   if(m_keyHandler) {
      m_keyHandler->cancelInfoCallback();
   }
   if(iMapControl) {
      iMapControl->Handler().removeEventListener(this);
   }
}

void CVectorMapContainer::StopKeyHandler() 
{
   m_keyHandler->stop();	
}

