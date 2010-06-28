/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CURSOR_H
#define CURSOR_H

#include "config.h"

class MC2Point;
class PixelBox;
class SpriteMover;
class PixelBox;

/**
 *    Pure abstract class that represents a cursor.
 */
class Cursor
{
   public:
      /**
       *    Sets highlight on or off.
       *    @param   highlight   If true, set highligt on, otherwise off.
       */
      virtual void setHighlight( bool highlight ) {}

      /**
       *    Sets the cursor position to the specified point.
       *    @param   pos   The position.
       */
     virtual void setCursorPos( const MC2Point& pos ) {}
      
      /**
       *    Get the current cursor position.
       *    @return  The cursor position.
       */
      virtual MC2Point getCursorPos() const = 0;
        
      /**
       *    Set if the cursor should be visible or hidden.
       *    @param   visible  If true, the cursor is visible, 
       *                      otherwise hidden.
       */
     virtual void setCursorVisible( bool visible ) {}

      /**
       *    Get the box of the cursor.
       *    @param   box   PixelBox that will be set to the current
       *                   box of the cursor.
       */
      virtual void getCursorBox( PixelBox& box ) const = 0;
};

/**
 *    Class that represents a cursor that can be highlighted or 
 *    non highlighted.
 */
class CursorSprite : public Cursor
{
   public:

      /**
       *    Constructs a cursor sprite, with SpriteMovers for the
       *    nonhighlighted and highlighted cursor.
       *    The SpriteMovers are not deleted by this object.
       *    @param   cursor            The non highlighted cursor.
       *    @param   highlightCursor   The highlighted cursor.
       */
      CursorSprite( SpriteMover* cursor,
                    SpriteMover* highlightCursor );

      
      /**
       *    Implements abstract method in superclass.
       *    Sets highlight on or off.
       *    @param   highlight   If true, set highligt on, otherwise off.
       */
      void setHighlight( bool highlight );

      /**
       *    Implements abstract method in superclass.
       *    Sets the cursor position to the specified point.
       *    @param   pos   The position.
       */
      void setCursorPos( const MC2Point& pos );
      
      /**
       *    Implements abstract method in superclass.
       *    Get the current cursor position.
       *    @return  The cursor position.
       */
      MC2Point getCursorPos() const;

      /**
       *    Implements abstract method in superclass.
       *    Set if the cursor should be visible or hidden.
       *    @param   visible  If true, the cursor is visible, 
       *                      otherwise hidden.
       */
      void setCursorVisible( bool visible );

      /**
       *    Implements abstract method in superclass.
       *    Get the box of the cursor.
       *    @param   box   PixelBox that will be set to the current
       *                   box of the cursor.
       */
      void getCursorBox( PixelBox& box ) const;

   private:

      /**
       *   The highlighted cursor. 
       */
      SpriteMover* m_cursor;
      
      /**
       *    The non highlighted cursor.
       */
      SpriteMover* m_highlightCursor;
      
      /**
       *    If the cursor is visible or hidden.
       */
      bool m_visible;
      
      /**
       *    If the highlighted cursor is active.
       */
      bool m_highlighted;

};


#endif
