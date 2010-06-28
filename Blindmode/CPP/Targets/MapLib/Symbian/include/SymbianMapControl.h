/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SYMBIAN_MAP_CONTROL_H
#define SYMBIAN_MAP_CONTROL_H

#include <coecntrl.h>

class CEikLabel;
class DBufConnection;
class KeyMap;
class SymbianMapLib;
class TileMapKeyHandler;
class MC2Point;
class PixelBox;

#include "Cursor.h"

/**
 *    Control that will be a map.
 *    Will include key-handling.
 */
class CSymbianMapControl : public CCoeControl,
                           public Cursor {
public:
   
   static CSymbianMapControl* NewL( CCoeControl& parent,
                                    DBufConnection* mapConn );

   virtual ~CSymbianMapControl();

   /// Implements Cursor::getCursorPos
   MC2Point getCursorPos() const;

   /// Implements Cursor::getCursorBox
   void getCursorBox( PixelBox& box ) const;

   /// Returns the maplib
   SymbianMapLib* getMapLib() const { return m_mapLib; }

protected:
   
   CSymbianMapControl();

   void ConstructL( CCoeControl& parent,
                    DBufConnection* mapConn );

   /// Handle map moving. Return EKeyWasNotConsumed if no map button.
   TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,
                               TEventCode aType);

   /// Handle pointer event
   void HandlePointerEventL( const TPointerEvent& aPointerEvent );

   /// Returns the number contained controls
   TInt CountComponentControls() const;

   /// Return contained control number index
   CCoeControl* ComponentControl(TInt aIndex) const;
      
   typedef struct {
      int first; 
      int second; 
   } key_array_pair_t;
   
   static const key_array_pair_t c_defaultKeyBindings[];

   /// Key map to be used in OfferKeyEventL
   KeyMap* m_keyMap;

   /// Key handler
   TileMapKeyHandler* m_keyHandler;

   /// MapLib
   SymbianMapLib* m_mapLib;

   /// Temporary info display
   CEikLabel* m_label;
   
private:

   /** Show info about a poi */
   void showInfo( const char* infoText );
   
   /** draws the map ... from CCoeControl */
   void Draw(const TRect& aRect) const;

   /// Handles a size change. Resizes the plotter.
   void SizeChanged();

};

#endif
