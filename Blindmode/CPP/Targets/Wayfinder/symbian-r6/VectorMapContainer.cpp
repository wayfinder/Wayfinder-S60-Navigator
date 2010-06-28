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
#include <aknlists.h>     // for avrell style listbox
#include <aknpopup.h>     // for pop up menu
#ifndef NAV2_CLIENT_SERIES60_V3
 #include <hal.h>
#endif
#include <eiklabel.h>  // for labels
#include <eikedwin.h>  // for labels
#include <aknutils.h>  // for Fonts.

#include "wficons.mbg"

#include "RsgInclude.h"
#include "userbitmap.h"
#include "BitmapControl.h"
#include "VectorMapContainer.h"
#include "MapView.h"
#include "TileMapControl.h"
#include "MapRenderer.h"
#include "MapMovingInterface.h"
#include "MapDrawingInterface.h"

#include "MapSwitcher.h"
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

#include "MapBorderBar.h"

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

#include "MapResourceFactory.h"

/* #include "WFTimeUtils.h" */

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

/* macros to display above messages */
#define SHOWMSGWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericMsgTitle, x)
#define SHOWMSG(x)      CEikonEnv::Static()->InfoMsgWithDuration(x, 1000000)
#define SHOWERRWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericErrorTitle, x)
#define SHOWERR(x)      CEikonEnv::Static()->InfoMsgWithDuration(x, 2000000)
#define SHOWBUSY(x)     CEikonEnv::Static()->BusyMsgL(x)
#define CANCELBUSY()    CEikonEnv::Static()->BusyMsgCancel()

#define W_PADDING                1    
#define DETOUR_POS_TOP_EDGE      (36+W_PADDING)
#define SPEEDCAM_POS_TOP_EDGE    (DETOUR_POS_TOP_EDGE+24+W_PADDING)

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
   { KEY0,              TileMapKeyHandler::ZOOM_OUT_KEY },
   { KEY5,              TileMapKeyHandler::ZOOM_IN_KEY },
   { KEY8,              TileMapKeyHandler::MOVE_DOWN_KEY },
   { KEY_STAR,          TileMapKeyHandler::AUTO_ZOOM_KEY },
   { KEY4,              TileMapKeyHandler::MOVE_LEFT_KEY },
   { KEY6,              TileMapKeyHandler::MOVE_RIGHT_KEY },
   { KEY7,              TileMapKeyHandler::RESET_ROTATION_KEY },
#ifndef AUTOMOVE_ON // Can be defined in TileMapKeyHandler.h
   { KEY1,              TileMapKeyHandler::HIGHLIGHT_NEXT_FEATURE_KEY },
//   { KEY1,              TileMapKeyHandler::ROTATE_LEFT_KEY },
//   { KEY3,              TileMapKeyHandler::ROTATE_RIGHT_KEY },
   { KEY3,              TileMapKeyHandler::CENTER_MAP_AT_CURSOR_KEY },
#else
   // These keys can be used for automatic movement.
   { KEY1,              TileMapKeyHandler::SLOW_TEST_KEY },
   { KEY3,              TileMapKeyHandler::FAST_TEST_KEY },
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

   // Will not repaint if this returns false.
   bool keyHandlerCallback() {
      m_container->CheckFavoriteRedraw();
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
   EMapInfoControl,
   ETopBorderBitmap,
   EGpsStatusBitmap,
   EConBitmap,
   EDetourPicture,
   ESpeedCamPicture,
   ENumControls
};



// Constructor
CVectorMapContainer::CVectorMapContainer( CMapView* aMapView, isab::Log* aLog )
   : iLog(aLog), iConstructDone(EFalse)
{
   iMapControl = NULL;
   iIsConnected = EFalse;

   iView = aMapView;

   iShowingDetails = false;
   
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
}

TBool
CVectorMapContainer::ShowingInfoText()
{
   return iShowingDetails;
   
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
      fontSpec = TFontSpec(fontAttrib.Name(), font->HeightInPixels());
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
      horizontalFont = TileMapTextSettings::font_pair_t( primaryFont.iTypeface.iName, 
                                                         primaryFont.iHeight );
   } else {
      horizontalFont = TileMapTextSettings::font_pair_t ( "LatinBold19", 19 );
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
                                 CTileMapControl* aMapControl,
                                 CMapFeatureHolder* aFeatureHolder,
                                 CMapInfoControl* aMapInfoControl,
                                 CVectorMapConnection* aVectorMapConnection,
                                 const TDesC& /*aResourcePath*/)
{
	iWayFinderUI = aWayFinderUI;
	
   CreateWindowL();

   LogFile::Create();

   iNormalRect = aRect;
   iIsFullScreen = EFalse;

   iMapControl = 					aMapControl;
   iFeatureHolder = 		      aFeatureHolder;
   m_mapInfoControl = 	 		aMapInfoControl;
   
   m_mapInfoControl->SetContainerWindowL(* this);      
   iMapControl->SetContainerWindowL(* this);
      
   /* Load favorites. */
   iView->FavoriteChanged();

   if (iWayFinderUI->IsIronVersion()) {
      iScale = EScaleGlobe;
   } else {
      iScale = EScale12;
   }
      
   iView->SetScale(iScale);
      
#ifndef USE_GLOBE
   iMapMovingInterface = &iMapControl->Handler();
   iMapDrawingInterface = &iMapControl->Handler();
#endif
   iIsConnected = ETrue;

   // Clear any old info texts.
   m_mapInfoControl->setInfoText( NULL );

   MapLib* mapLib = iMapControl->getMapLib();

   mapLib->setCopyrightPos(MC2Point(3, aRect.Height() - 3));
   mapLib->showCopyright(ETrue);

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
   tilemapNotice.m_hasStaticCursor = false;

   iMapControlVisAdapter = new VisibilityAdapter<CTileMapControl>(iMapControl);
   iMapControlVisAdapter->setVisible(ETrue);
   tilemapNotice.m_visibles.push_back(iMapControlVisAdapter);

   iCursorVisAdapter = 
      new CursorVisibilityAdapter(m_keyHandler);
   iCursorVisAdapter->setVisible(ETrue);
   tilemapNotice.m_visibles.push_back(iCursorVisAdapter);

#ifdef NAV2_CLIENT_SERIES60_V3
   iTopBorderVisAdapter = 
      new VisibilityAdapter<CMapBorderBar>(CMapBorderBar::NewL(*this, 
                                                               aRect,
                                                               iView->GetMbmName(),
                                                               iView->GetMbmfileIds(),
                                                               iView->GetMbmFileRelations()));
#else
   iTopBorderVisAdapter = 
      new VisibilityAdapter<CMapBorderBar>(CMapBorderBar::NewL(*this, 
                                                               aRect,
                                                               iView->GetMbmName(),
                                                               iView->GetMbmfileIds()));
#endif

   iTopBorderVisAdapter->setVisible(EFalse);
   tilemapNotice.m_visibles.push_back(iTopBorderVisAdapter);
   
   CBitmapControl* gpsBitmap = CBitmapControl::NewL(this, TPoint(0,0), NULL);
   iGpsStatusVisAdapter = 
      new VisibilityAdapter<CBitmapControl>(gpsBitmap);
   iGpsStatusVisAdapter->setVisible(EFalse);
   tilemapNotice.m_visibles.push_back(iGpsStatusVisAdapter);

   CBitmapControl* conBitmap = CBitmapControl::NewL(this, TPoint(0,0), NULL);
   iConStatusVisAdapter = 
      new VisibilityAdapter<CBitmapControl>(conBitmap);
   iConStatusVisAdapter->setVisible(EFalse);
   tilemapNotice.m_visibles.push_back(iConStatusVisAdapter);

#ifdef NAV2_CLIENT_SERIES60_V3
   TSize detourSize = WFLayoutUtils::CalculateSizeUsingFullScreen(24, 24);
   TInt xPos = (WFLayoutUtils::GetFullScreenRect().Width() - 
                MIN(detourSize.iWidth, detourSize.iHeight)) - 
                WFLayoutUtils::CalculateXValue(W_PADDING);
   TInt detourPosY = WFLayoutUtils::CalculateYValueUsingFullScreen(DETOUR_POS_TOP_EDGE);
   iDetourPictureVisAdapter = 
      new VisibilityAdapter<CImageHandler>(CImageHandler::NewL(
                         TRect(TPoint(xPos,
                                      detourPosY),
                               detourSize)));

   iDetourPictureVisAdapter->getControl()->SetShow(EFalse);
   iDetourPictureVisAdapter->getControl()->CreateIconL(iView->GetMbmName(), 
                                                       EMbmWficonsDetour_square,
                                                       EMbmWficonsDetour_square_mask);

   iSpeedCamPictureVisAdapter = 
      new VisibilityAdapter<CImageHandler>(CImageHandler::NewL(
            TRect(TPoint(xPos, 
                         detourPosY + MIN(detourSize.iWidth, detourSize.iHeight) + 
                         WFLayoutUtils::CalculateXValue(W_PADDING)),
                  detourSize)));

   iSpeedCamPictureVisAdapter->getControl()->SetShow(EFalse);
   iSpeedCamPictureVisAdapter->getControl()->CreateIconL(iView->GetMbmName(), 
                                                         EMbmWficonsSpeedcamera_square,
                                                         EMbmWficonsSpeedcamera_square);
#else
   CGuidePicture* detourPic = new (ELeave) CGuidePicture(iLog);
   LOGNEW(detourPic, CGuidePicture);
   iDetourPictureVisAdapter = 
      new VisibilityAdapter<CGuidePicture>(detourPic);
   iDetourPictureVisAdapter->getControl()->ConstructL(TRect(TPoint(aRect.Width()-24-1,
                                                                   DETOUR_POS_TOP_EDGE),
                                                            TSize(24, 24)), this);
   iDetourPictureVisAdapter->getControl()->OpenBitmapFromMbm(iView->GetMbmName(), 
                                                             EMbmWficonsSmall_detour);
   iDetourPictureVisAdapter->getControl()->SetShow(EFalse);

   CGuidePicture* speedCamPic = new (ELeave) CGuidePicture(iLog);
   LOGNEW(speedCamPic, CGuidePicture);
   iSpeedCamPictureVisAdapter = 
      new VisibilityAdapter<CGuidePicture>(speedCamPic);
   iSpeedCamPictureVisAdapter->getControl()->ConstructL(TRect(TPoint(aRect.Width()-24-1,
                                                                     SPEEDCAM_POS_TOP_EDGE), 
                                                              TSize(24, 24)), this);
   iSpeedCamPictureVisAdapter->getControl()->OpenBitmapFromMbm(iView->GetMbmName(), 
                                                               EMbmWficonsSmall_speedcam);
   iSpeedCamPictureVisAdapter->getControl()->SetShow(EFalse);
#endif
   iDetourPictureVisAdapter->setVisible(EFalse);
   tilemapNotice.m_visibles.push_back(iDetourPictureVisAdapter);
   iSpeedCamPictureVisAdapter->setVisible(EFalse);
   tilemapNotice.m_visibles.push_back(iSpeedCamPictureVisAdapter);

   tilemapNotice.m_mapRect = iMapControl;
   
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

   /* start the key timer */
   m_autoTrackingOnTimer = CPeriodic::NewL(CActive::EPriorityLow);

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
   m_mapMover->setTrackingPoint( getTrackingPoint());
   m_mapMover->setNonTrackingPoint( getNonTrackingPoint() );
   
   /* set this control to get keyboard focus */
   SetFocus(ETrue);
   iMapControl->SetFocus( ETrue );
   updateCursorVisibility();

   ShowNorthArrow( ETrue );

   // Set the fonts in the TileMapHandler.
   setTileMapFonts();

   // Insert the key bindings into the map
   int nbrBindings = sizeof(c_keyBindings) / sizeof(c_keyBindings[0]);
   for ( int i = nbrBindings - 1; i >= 0; --i ) {
      m_keyMap.insert( make_pair( c_keyBindings[i].first,
                                  c_keyBindings[i].second ) );
   }

   EnableDragEvents();   
   ShowScale( ETrue );

   SetRect(aRect);
   ActivateL();

   UpdateRepaint();

   iConstructDone = ETrue;
}

// Destructor
CVectorMapContainer::~CVectorMapContainer()
{
   // Cancel the hide info timer.
   if ( iMapControl != NULL && m_hideInfoTimerID != MAX_UINT32 ) {
      iMapControl->getToolkit()->cancelTimer( this, m_hideInfoTimerID );
   }

   // Keyhandler is deleted by TileMapControl.
   // delete m_keyHandler;
   delete m_mapMover;
   if ( m_autoTrackingOnTimer->IsActive() ) {
      m_autoTrackingOnTimer->Cancel();
   }
   delete m_autoTrackingOnTimer;
   m_autoTrackingOnTimer = NULL;

   iMapControl = NULL;

/*    LOGDEL(iTurnPicture); */
/*    delete iTurnPicture; */
/*    iTurnPicture = NULL; */
   
   delete m_khCallBack;
   m_khCallBack = NULL;

   if (iTopBorderVisAdapter) {
      delete iTopBorderVisAdapter->getControl();
      delete iTopBorderVisAdapter;
      iTopBorderVisAdapter = NULL;
   }
   if (iGpsStatusVisAdapter) {
      delete iGpsStatusVisAdapter->getControl();
      delete iGpsStatusVisAdapter;
      iGpsStatusVisAdapter = NULL;
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
   delete iCursorVisAdapter;

   delete iMapControlVisAdapter;
   delete iGlobeControlVisAdapter;
   delete iGlobeComponent;

   delete iMapSwitcher;

   delete m_cursorSprite;
   delete m_highlightCursorSprite;
   delete m_cursor;

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

   if (iMapControl && m_mapInfoControl) { 
      TPoint mapPos(rect.iTl);
      if (iTopBorderVisAdapter->getControl()->IsBackgroundVisible()) {
         mapPos.iY = iTopBorderVisAdapter->getControl()->Rect().iBr.iY;
         ResetNorthArrowPos();
      }
      MapLib* mapLib = iMapControl->getMapLib();
      mapLib->setCopyrightPos(MC2Point(3, rect.Height() - 3 - mapPos.iY));

      iMapControl->SetSize(TSize(rect.Width(), (rect.Height()-mapPos.iY)));
      iMapControl->SetPosition(mapPos);

      m_mapInfoControl->SetSize(TSize(rect.Width(),
            (rect.Height()-mapPos.iY)));
      m_mapInfoControl->SetPosition(mapPos);
   }

#ifdef NAV2_CLIENT_SERIES60_V3
   TSize detourSize = WFLayoutUtils::CalculateSizeUsingFullScreen(24, 24); 
   TInt xPos = (WFLayoutUtils::GetFullScreenRect().Width() - MIN(detourSize.iWidth, detourSize.iHeight)) - 
      WFLayoutUtils::CalculateXValue(W_PADDING);
   TInt detourPosY = WFLayoutUtils::CalculateYValueUsingFullScreen(DETOUR_POS_TOP_EDGE);
   TRect newRect(TPoint(xPos, 
                        detourPosY),
                 detourSize);
   iDetourPictureVisAdapter->getControl()->SetImageRect(newRect);

   newRect = TRect(TPoint(xPos, 
                          detourPosY + MIN(detourSize.iWidth, detourSize.iHeight) + 
                          WFLayoutUtils::CalculateXValue(W_PADDING)),
                   detourSize);
   iSpeedCamPictureVisAdapter->getControl()->SetImageRect(newRect);
#endif

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
                                northUp);
      updateCursorVisibility();
   }
   ShowScale(ETrue);
}

bool
CVectorMapContainer::getCursorVisibility() const
{
   bool visible = IsFocused() && iIsFullScreen &&
               ! (iView->IsTracking() && m_mapMover->gotValidGpsCoord() ) &&
               ! m_detailedInfoTextShown;
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
   return m_mapMover->getPoint();
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
   return ENumControls;
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
   case EGpsStatusBitmap:
      return iGpsStatusVisAdapter->getControl();
   case EConBitmap:
      return iConStatusVisAdapter->getControl();
/*    case EDistanceLabel: */
/*       return iDistanceText; */
/*    case ETurnPicture: */
/*       return iTurnPicture; */
   case EDetourPicture:
      return iDetourPictureVisAdapter->getControl();
   case ESpeedCamPicture:
      return iSpeedCamPictureVisAdapter->getControl();
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
   MC2Point pos( aPointerEvent.iPosition );
   m_keyHandler->setCursorPos( pos );

   const char* info = NULL;

   m_mapMover->handleKeyEvent( 
                        TileMapKeyHandler::DRAG_TO,
                        translatePointerEventType( aPointerEvent.iType ), 
                        info,
                        &pos );
   if ( info != NULL && ! iInfoTextPersistant ) {
         setInfoText( info, NULL, NULL, EFalse, 3000 );
   }
}

// key event handling method
TKeyResponse CVectorMapContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
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
/*             iView->HandleCommandL(EWayFinderCmdMapShowInfo); */
            //Old way of doing it in a blocking popup dialog.
            //GetFeatureName();
            iView->FetchCurrentMapFeatureName();
            ShowDetailedFeatureInfo(ETrue);
            return (EKeyWasConsumed);
         } else if (aKeyEvent.iScanCode == KEY_SOFTKEY_RIGHT) {
            setInfoText(NULL);
            return (EKeyWasConsumed);
         }
      }
   }

   if (aType == EEventKeyDown &&
       aKeyEvent.iScanCode == EStdKeyBackspace) {
      iView->HandleCommandL( EAknSoftkeyBack );
      return EKeyWasConsumed;
   }

   /* don't handle keys, if not started yet */
   if( !iIsFullScreen ){
      /* Don't consume joystick press events. */
      if ( aKeyEvent.iScanCode == EStdKeyDevice3 ){
         if (aType != EEventKeyDown) {
            return EKeyWasNotConsumed;
         } else {
            iView->HandleCommandL( EWayFinderCmdMapFullScreen );
            return EKeyWasConsumed;
         }
      }
      /* Don't consume joystick movement events. */
      if ( aKeyEvent.iScanCode == EStdKeyUpArrow ||
           aKeyEvent.iScanCode == EStdKeyDownArrow ||
           aKeyEvent.iScanCode == EStdKeyRightArrow ||
           aKeyEvent.iScanCode == EStdKeyLeftArrow) {
         return EKeyWasNotConsumed;
      }
   }

   // Check for movement keys and call the keyhandler if appropriate.
   map<int,int>::const_iterator it = m_keyMap.find( aKeyEvent.iScanCode );
   if ( it != m_keyMap.end() ) {
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
      else if(aKeyEvent.iScanCode == EStdKeyHash)
      {
         if (iView->IsIronVersion()) {
            iView->HandleCommandL(EWayFinderCmdDestination);
            removeInfoText = ETrue;
         } else {
            iView->HandleCommandL(EWayFinderCmdGuide);
            removeInfoText = ETrue;
         }
      }
      else if(aKeyEvent.iScanCode == KEY9)
      {
         if (iView->IsTracking()) {
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
         //GetFeatureName();
         iView->FetchCurrentMapFeatureName();
         
         if( iView->SettingDestination() ){
            iView->HandleCommandL( EWayFinderCmdMapSetDestination );
         } else if( iView->SettingOrigin() ){
            iView->HandleCommandL( EWayFinderCmdMapSetOrigin );
         } else {
            ShowDetailedFeatureInfo();
         }       
      }
      if (!iView->IsTracking() && iView->IsGpsAllowed()
          && !(aKeyEvent.iScanCode == KEY9)) {
         /* Set timer to reenable tracking. */
         if (m_autoTrackingOnTimer->IsActive()) {
            m_autoTrackingOnTimer->Cancel();
         }
         m_autoTrackingOnTimer->Start(8000000, 8000000,
                                      TCallBack(KeyCallback, this));
      }
   }
   
   return EKeyWasConsumed;
}

void
CVectorMapContainer::SetConStatusImage(CFbsBitmap* aConStatusBitmap,
                                       CFbsBitmap* aConStatusMask)
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
   TPoint aConStatusPosition(((TInt)((float)rect.Width() * 
                                     ((float)2 / 176))), 
                             ((TInt)rect.Height() - 
                              (TInt)((float)rect.Height() * ((float)61 / 208))));
#else
   TPoint aConStatusPosition; 
   aConStatusPosition.iX = 2;
   aConStatusPosition.iY = Rect().Height() - (19 + 12 + 30);
#endif
   if (iConStatusVisAdapter) {
      iConStatusVisAdapter->setVisible(ETrue);
      iConStatusVisAdapter->getControl()->SetBitmap(aConStatusPosition,
                                                    aConStatusBitmap,
                                                    aConStatusMask);
   }
//    if (iConBitmap) {
//       iConBitmap->SetBitmap(aConStatusPosition,
//                             aConStatusBitmap,
//                             aConStatusMask,
//                             EFalse);
//    }
}

void
CVectorMapContainer::SetGpsStatusImage(CFbsBitmap* aGpsStatusBitmap,
                                       CFbsBitmap* aGpsStatusMask)
{
   /*
   if (iMapControlVisAdapter->isMainControlVisible()) {
      iTopBorderVisAdapter->getControl()->SetGpsStatusImage(aGpsStatusBitmap, 
                                                            aGpsStatusMask);
      ResetNorthArrowPos();
   }
   */
   TPoint gpsStatusPosition(Rect().iTl);
   gpsStatusPosition.iX += 2;
   gpsStatusPosition.iY += 2;
   TBool invertMask = ETrue;
#if defined NAV2_CLIENT_SERIES60_V3
   if (iView && iView->IsGpsConnected() && !iView->IsSimulating()) {
      // If we are not connected to gps we need to
      // invert the mask in MapBorderBar to make
      // the gps symbol disapear. If we are connected
      // to gps we cant invert the mask since we are
      // using svg images for the gps icon in map view.
      invertMask = EFalse;
   }
#endif
   if (iGpsStatusVisAdapter) {
      iGpsStatusVisAdapter->setVisible(ETrue);
      iGpsStatusVisAdapter->getControl()->SetBitmap(gpsStatusPosition,
                                                    aGpsStatusBitmap,
                                                    aGpsStatusMask,
                                                    invertMask);
   }
   ResetNorthArrowPos();
}
 
void CVectorMapContainer::SetTopBorder()
{
   if (iView->IsIronVersion()) {
      iTopBorderVisAdapter->setVisible(EFalse);
   } else {
      if (!iTopBorderVisAdapter->isVisible()) {
         iTopBorderVisAdapter->setVisible(ETrue);
         SizeChanged();
      }
   }
}

void CVectorMapContainer::HideTopBorder()
{
   if (iTopBorderVisAdapter->isVisible()) {
      iTopBorderVisAdapter->setVisible(EFalse);
      SizeChanged();
      ResetNorthArrowPos();
   }
}

void CVectorMapContainer::SetTurnPictureL( TInt aMbmIndex )
{
   iTopBorderVisAdapter->getControl()->SetTurnPictureL(aMbmIndex);
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
   iDetourPictureVisAdapter->getControl()->SetShow(on == 1);
   iDetourPictureVisAdapter->setVisible(on == 1);
}

void
CVectorMapContainer::SetSpeedCamPicture(TInt on)
{
   iSpeedCamPictureVisAdapter->getControl()->SetShow(on == 1);
   iSpeedCamPictureVisAdapter->setVisible(on == 1);
}


void CVectorMapContainer::SetDistanceL( TUint aDistance )
{
   iTopBorderVisAdapter->getControl()->SetDistanceL(aDistance, 
                                                    iView->GetDistanceMode());
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

void CVectorMapContainer::SetFullScreen( TBool aFullScreen )
{
   if ( iIsFullScreen == aFullScreen ) {
      return;
   }
   iIsFullScreen = aFullScreen;

   if( iIsFullScreen ){
//       TRect rect;
//       AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
//       SetSize(TSize(rect.Width() / 2, rect.Height() / 2));
      SetExtentToWholeScreen();
   } else{
      SetRect( iNormalRect );
   }
   
/*    DrawNow(); */
}


TBool CVectorMapContainer::IsFullScreen()
{
   return iIsFullScreen;
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


void CVectorMapContainer::SetRoute( TInt64 aRouteId )
{   
   iMapControl->getMapLib()->setRouteID( RouteID( aRouteId ) );   
}

void CVectorMapContainer::ClearRoute( )
{
   iMapControl->getMapLib()->clearRouteID( );
   HideTopBorder();
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
   if(aData->iFavoriteShow == 2 ||
         (aScale > EScale6 && aData->iFavoriteShow != 0)) {
      /* Never show or scale is high and show always is not set. */
      /* Don't show favorites. */

      if (iFeatureHolder->GetCurrentFavIconSize() >= 1) { 
         iFeatureHolder->SetCurrentFavIconSize(0);
         iView->FavoriteChanged();
      }
   }
   else if( aScale > EScale4 &&
         aData->iFavoriteShow != 2) {
      /* Scale is medium and never show is not set. */
      /* Show as small stars. */

      if (iFeatureHolder->GetCurrentFavIconSize() >= 2 ||
          iFeatureHolder->GetCurrentFavIconSize() <= 0) { 
         iFeatureHolder->SetCurrentFavIconSize(1);
         iView->FavoriteChanged();
      }
   }
   else if( aScale >= EScaleMin &&
         aData->iFavoriteShow != 2) {
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

MC2Point
CVectorMapContainer::getTrackingPoint() const
{
   TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
   if ( iView->GetTrackingType() == ERotating ) {
      return MC2Point( iMapControl->Size().iWidth >>1,
                       iMapControl->Size().iHeight - ( 60 * dpiCorrectionFactor ) );
   } else {
      return MC2Point( ( iMapControl->Size().iWidth >>1 ),
                       ( iMapControl->Size().iHeight >>1 ) );
   }
}

MC2Point
CVectorMapContainer::getNonTrackingPoint( bool screenSizeChanged ) const
{
   if ( iIsFullScreen && !screenSizeChanged ) {
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
      CMapFeature* mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iPositionPoly);
      iFeatureHolder->iPositionPolyShown = EFalse;
      delete mf;
   } else if (!iFeatureHolder->iPositionPolyShown) {
      CMapFeature* mf = new (ELeave) CMapFeature(map_feature_gps, 0);
      iFeatureHolder->AddFeature(iFeatureHolder->iPositionPoly, mf);
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
      TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
      if (!iTopBorderVisAdapter->isVisible() && 
          iView->IsGpsConnected()) {
         x = 15 * dpiCorrectionFactor;
         y = 15 * dpiCorrectionFactor + iTopBorderVisAdapter->getControl()->Rect().iBr.iY;
      } else {
         x = 15 * dpiCorrectionFactor;
         y = 15 * dpiCorrectionFactor;
      }
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
      TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
      if (!iTopBorderVisAdapter->isVisible() && 
          iView->IsGpsConnected()) {
         x = 15 * dpiCorrectionFactor;
         y = 15 * dpiCorrectionFactor + iTopBorderVisAdapter->getControl()->Rect().iBr.iY;
      } else {
         x = 15 * dpiCorrectionFactor;
         y = 15 * dpiCorrectionFactor;
      }

      iFeatureHolder->iNorthPoly->setCenter(MC2Point(x,y));
      iFeatureHolder->iSouthPoly->setCenter(MC2Point(x,y));
      iFeatureHolder->iArrowPoly->setCenter(MC2Point(x,y));
   }
}

void
CVectorMapContainer::ShowScale( TBool aShow )
{
   TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
   if( !aShow ) {
      CMapFeature* mf;

      mf = iFeatureHolder->RemoveFeature(iFeatureHolder->iScaleFeature);
      delete mf;
      iFeatureHolder->iScaleFeatureShown = EFalse;
   } else if (!iFeatureHolder->iScaleFeatureShown ) {
      CMapFeature* mf;
      mf = new (ELeave) CMapFeature(map_feature_scale, 0);
      iFeatureHolder->AddFeature( iFeatureHolder->iScaleFeature, mf );
      TInt x = Rect().Width();
      TInt y = iMapControl->Rect().Height() - ( 19 * dpiCorrectionFactor );
      iFeatureHolder->iScaleFeature->setPoint( MC2Point(x,y) );
      iFeatureHolder->iScaleFeatureShown = ETrue;
   } else {
      TInt x = Rect().Width();
      TInt y = iMapControl->Rect().Height()- ( 19 * dpiCorrectionFactor );
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

void CVectorMapContainer::GetCoordinate( MC2Coordinate& aMC2Coord )
{
   aMC2Coord = m_mapMover->getCoordinate();
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
   
   //Cancel the hide timer.
   if ( m_hideInfoTimerID != MAX_UINT32 ) {
      iMapControl->getToolkit()->cancelTimer( this, m_hideInfoTimerID );
   }
   if (txt == NULL) {
      iInfoTextShown = EFalse;
      m_detailedInfoTextShown = false;
      iInfoTextPersistant = EFalse;
      //iMapControl->setInfoText(txt);
      m_mapInfoControl->setInfoText(txt);
      //iCopyrightLabel->MakeVisible(ETrue);
      iView->ToggleSoftKeys(ETrue);
   } else {
      if (iMapControl) {
         if ( left != NULL || right != NULL ) {
            m_detailedInfoTextShown = true;
         } else {
            m_detailedInfoTextShown = false;
         }
         iInfoTextShown = true;
         iInfoTextPersistant = persistant;
         //iMapControl->setInfoText(txt, left, right);
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

   iShowingDetails = showWithList;
   
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
         iView->GetAndShowDetails( serverString,
                                   name);
      } else {
         setInfoText( NULL );
      }
   }
#if 0
   vector<MC2SimpleString> mc2strings;
   int num_hits = iMapControl->Handler().getServerIDStringForFeaturesAt(
         mc2strings,
         MC2Point(iMapControl->Size().iWidth >>1, 
                  iMapControl->Size().iHeight >>1) );

   if (num_hits) {
      MC2SimpleString itemId = mc2strings.at(0);
      iView->GetDetails( itemId.c_str() );
   } else {
      WFDialog::ShowInformationL(R_CONNECT_NO_MATCHES_FOUND, iWayFinderUI->getCoeEnv());
   }
#endif
}
 
const unsigned char* CVectorMapContainer::GetFeatureName()
{
   const char* feature = 
      iMapControl->getFeatureName( getActivePoint() );
   if( feature == NULL ){
      feature = "Unknown";
   }

   return (const unsigned char*)feature;
}


TInt CVectorMapContainer::GetScale()
{
   return (TInt)(iMapMovingInterface->getScale() + 0.5);
}

void CVectorMapContainer::setTrackingMode( TBool /*aOnOff*/,
                                           TrackingType aType )
{
   if (iMapControl) {

      bool northUp = ( aType != ERotating );

      m_mapMover->statusChanged( getTrackingPoint(),
                                 getNonTrackingPoint(),
                                 iView->IsTracking(),
                                 northUp );
      updateCursorVisibility();

   }
}

void 
CVectorMapContainer::ShowVectorMapWaitSymbol( bool start )
{
   // Only start showing the wait symbol in the map in case of full screen. 
   if ( start && IsFullScreen() ) {
      // Lower left corner.
      TInt x = 8;
      TInt y = Rect().Height() - 33;
      iFeatureHolder->iWaitSymbol->setCenter( MC2Point(x,y) );
      iFeatureHolder->iWaitSymbol->setVisible( true );
      UpdateRepaint();
   } else if ( ! start ) {
      // Simply set it to not be visible. No need to remove etc.
      iFeatureHolder->iWaitSymbol->setVisible( false );
      UpdateRepaint();
   }
}

/* key handler */
void CVectorMapContainer::TimerHandler() 
{

   /* Timer used as timeout for tracking. */
   if (iView->UseTrackingOnAuto()) {
      iView->SetTracking(ETrue);
   }
   if ( m_autoTrackingOnTimer->IsActive() ) {
      m_autoTrackingOnTimer->Cancel();
   }
}

bool 
CVectorMapContainer::setGpsPos( const MC2Coordinate& coord, 
                                int direction, 
                                int scale )
{
   return m_mapMover->setGpsPos( coord, direction, scale );
}

// returns Input info about the app 
// required if the derived class is implementing OfferKeyEventL()
TCoeInputCapabilities CVectorMapContainer::InputCapabilities() const
{
	return(TCoeInputCapabilities(TCoeInputCapabilities::EAllText));
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

void
CVectorMapContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
//       iTopBorderControl->HandleResourceChangeTest(KEikDynamicLayoutVariantSwitch);
      
      SetRect(WFLayoutUtils::GetFullScreenRect());
//       BindMovementKeys(WFLayoutUtils::LandscapeMode());
   }   
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

void CVectorMapContainer::SetMapControlsNull()
{
   iMapControl = NULL;
   iFeatureHolder = NULL;
   m_mapInfoControl = NULL;
}
