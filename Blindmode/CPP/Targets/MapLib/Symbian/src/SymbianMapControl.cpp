/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "config.h"

#include "SymbianMapControl.h"

#include "MapRenderer.h"
#include "TileMapKeyHandler.h"
#include "SymbianMapLib.h"
#include "MapDrawingInterface.h"

#include "MapUtility.h"

#include <map>
#include <coemain.h>

using namespace std;

// Temporarily for the label
#include <eiklabel.h>

#include <e32std.h>

using namespace std;

// Key bindings follow

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

// Key bindings for movement.
const CSymbianMapControl::key_array_pair_t
CSymbianMapControl::c_defaultKeyBindings[] = {
   { KEY2,              TileMapKeyHandler::MOVE_UP_KEY },
   { KEY0,              TileMapKeyHandler::ZOOM_OUT_KEY },
   { KEY5,              TileMapKeyHandler::ZOOM_IN_KEY },
   { KEY8,              TileMapKeyHandler::MOVE_DOWN_KEY },
   { KEY_STAR,          TileMapKeyHandler::AUTO_ZOOM_KEY },
   { KEY4,              TileMapKeyHandler::MOVE_LEFT_KEY },
   { KEY6,              TileMapKeyHandler::MOVE_RIGHT_KEY },
   { KEY7,              TileMapKeyHandler::RESET_ROTATION_KEY },
   { KEY1,              TileMapKeyHandler::ROTATE_LEFT_KEY },
   { KEY3,              TileMapKeyHandler::ROTATE_RIGHT_KEY },
      // Only for fullscreen
   { EStdKeyDownArrow,  TileMapKeyHandler::MOVE_DOWN_KEY },
   { EStdKeyRightArrow, TileMapKeyHandler::MOVE_RIGHT_KEY },
   { EStdKeyLeftArrow,  TileMapKeyHandler::MOVE_LEFT_KEY },
   { EStdKeyUpArrow,    TileMapKeyHandler::MOVE_UP_KEY },
};


// Trick to avoid having to include stl in the header file.
class KeyMap : public map<int,int> {};

// -- Implementation of Cursor

MC2Point
CSymbianMapControl::getCursorPos() const
{
   return MC2Point( Size().iWidth >> 1,
                    Size().iHeight >> 1 );
}

void
CSymbianMapControl::getCursorBox( PixelBox& box ) const
{
   box = PixelBox( MC2Point( 0, 0), 
                   MC2Point( Size().iWidth, Size().iHeight ) );
}

// -- End implementation of CURSOR

CSymbianMapControl*
CSymbianMapControl::NewL( CCoeControl& parent,
                          DBufConnection* mapConn )
{
   CSymbianMapControl* tmp = new CSymbianMapControl;
   tmp->ConstructL( parent, mapConn );
   return tmp;
}

void
CSymbianMapControl::ConstructL( CCoeControl& parent,
                                DBufConnection* mapConn )
{
   // Create key map, which could be a sorted vector instead.
   m_keyMap = new KeyMap;
   // Insert the key bindings into the map
   int nbrBindings = sizeof(c_defaultKeyBindings) /
                         sizeof(c_defaultKeyBindings[0]);
   for ( int i = nbrBindings - 1; i >= 0; --i ) {
      m_keyMap->insert( make_pair( c_defaultKeyBindings[i].first,
                                   c_defaultKeyBindings[i].second ) );
   }

   // Create the MapLib
   m_mapLib = new SymbianMapLib( mapConn, *this,
                                 CCoeEnv::Static()->FsSession() );

   m_mapLib->setMemoryCacheSize( 102*1024 );

   m_keyHandler = new TileCursorKeyHandler(
      m_mapLib->getMapMovingInterface(),
      m_mapLib->getMapDrawingInterface(),
      m_mapLib->getToolkit(),
      this );

   m_label = new CEikLabel();
   m_label->SetContainerWindowL( *this );
   m_label->SetTextL( KNullDesC ); // Roligt
   m_label->MakeVisible( true );

   SetContainerWindowL( parent );   
   EnableDragEvents();
   MakeVisible( true );
   SetFocus( true );

   ActivateL();
}

CSymbianMapControl::CSymbianMapControl()
{
}

CSymbianMapControl::~CSymbianMapControl()
{
   delete m_label;
   delete m_keyMap;
   delete m_mapLib;
}

TInt
CSymbianMapControl::CountComponentControls() const
{
   return 1;
}

CCoeControl*
CSymbianMapControl::ComponentControl(TInt aIndex) const
{
   switch ( aIndex ) {
      case 0:
         return m_label;
      default:
         break;                  
   }
   return NULL;
}



void
CSymbianMapControl::Draw( const TRect& aRect ) const
{
   if ( ! m_mapLib ) {
      return;
   }
   CWindowGc& gc = SystemGc();
   
   // Move temporary rectangle to top-left corner.
   TRect tmprect( aRect );
   tmprect.Move( MC2Point(0,0)-MC2Point(iPosition) );
   
   isab::MapRenderer* plotter = m_mapLib->getSymbianMapPlotter();
   plotter->RenderTo(gc, MC2Point(aRect.iTl), tmprect );

   // Draw crosshair
   {
      MC2Point crossHairPos = getCursorPos();
      int x = crossHairPos.getX();
      int y = crossHairPos.getY();
      gc.SetPenSize( TSize(2,2) );
      gc.SetPenColor( TRgb( 64, 64, 255 ) );
      gc.DrawLine( MC2Point( x, y-5), MC2Point(x, y+5) );
      gc.DrawLine( MC2Point(x-5, y), MC2Point(x+5, y) );
   }
   
}

void
CSymbianMapControl::showInfo( const char* info )
{
   if ( info ) {
      HBufC* tempText = MapUtility::utf8ToUnicode( info );
      m_label->SetTextL( *tempText );
      delete tempText;
   } else {
      m_label->SetTextL( KNullDesC );
   }
   //m_label->MakeVisible( info != NULL );
}

void 
CSymbianMapControl::SizeChanged()
{
   // Resize the plotter buffer
   m_mapLib->getSymbianMapPlotter()->resizeBuffer(Size().iWidth,
                                                  Size().iHeight);
   // Repaint
  m_mapLib->getMapDrawingInterface()->repaintNow();
   // Move the temporary label.
   m_label->SetRect( TRect( iPosition, TSize( Rect().Width(), 20 ) ) );
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


// Pointer event handling method
void
CSymbianMapControl::HandlePointerEventL( const TPointerEvent& aPointerEvent )
{
   MC2Point pos( aPointerEvent.iPosition );
   m_keyHandler->setCursorPos( pos );
   
   const char* info = NULL;
   
   m_keyHandler->handleKeyEvent( 
      TileMapKeyHandler::DRAG_TO,
      translatePointerEventType( aPointerEvent.iType ), 
      info,
      &pos );
   
   if ( info != NULL ) {
      //setInfoText( info, NULL, NULL, EFalse, 3000 );
   }
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

// key event handling method
TKeyResponse
CSymbianMapControl::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                   TEventCode aType)
{
	/* if not a key event, return */
	if(aType != EEventKeyUp &&
      aType != EEventKeyDown &&
      aType != EEventKey) return(EKeyWasNotConsumed);

   // Check for movement keys and call the keyhandler if appropriate.
   KeyMap::const_iterator it = m_keyMap->find( aKeyEvent.iScanCode );
   if ( it != m_keyMap->end() ) {
      const char* info = NULL;
      bool handled =
         m_keyHandler->handleKeyEvent( TileMapKeyHandler::key_t( it->second ),
                                       translateKeyEventType( aType ),
                                       info );
      if ( handled || info != NULL ) {
         showInfo( info );
      }
      if ( handled ) {
         return EKeyWasConsumed;
      }
   }
   
	/* all keys are marked as processed/consumed */
	return EKeyWasNotConsumed;
}



