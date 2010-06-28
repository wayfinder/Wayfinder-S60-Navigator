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
#include "Cursor.h"
#include "SpriteMover.h"
#include "MC2Point.h"
#include "PixelBox.h"

CursorSprite::CursorSprite( 
      SpriteMover* cursor,
      SpriteMover* highlightCursor )
{
   m_cursor = cursor;
   m_highlightCursor = highlightCursor;
   m_visible = true;
   m_highlighted = false;
}

void 
CursorSprite::setHighlight( bool highlight )
{
   if ( highlight ) {

      // Enable highlighting.
      if ( ! m_highlighted ) {
         // Not already highlighted.
         // Move the cursor.
         m_highlightCursor->moveTo( m_cursor->getPosition() );
         m_cursor->setVisible( false );
         m_highlightCursor->setVisible( true );
         m_highlighted = true;
      }

   } else {
      
      // Disable highlighting. 
      if ( m_highlighted ) {
         // Previously highlighted.
         // Move the cursor.
         m_cursor->moveTo( m_highlightCursor->getPosition() );
         m_cursor->setVisible( true );
         m_highlightCursor->setVisible( false );
         m_highlighted = false;
      }
   }
}
   
void 
CursorSprite::setCursorPos( const MC2Point& pos )
{
   if ( m_highlighted ) {
      // Highlighted.
      m_highlightCursor->moveTo( pos );
   } else {
      // Not highlighted.
      m_cursor->moveTo( pos );
   }
}

MC2Point 
CursorSprite::getCursorPos() const
{
   if ( m_highlighted ) {
      // Highlighted.
      return m_highlightCursor->getPosition();
   } else {
      // Not highlighted.
      return m_cursor->getPosition();
   }
}
 
void 
CursorSprite::setCursorVisible( bool visible )
{
   m_visible = visible;

   if ( m_highlighted ) {
      // Highlighted.
      m_highlightCursor->setVisible( visible );
   } else {
      // Not highlighted.
      m_cursor->setVisible( visible );
   }
}

void
CursorSprite::getCursorBox( PixelBox& box ) const
{
   if ( m_highlighted ) {
      // Highlighted.
      m_highlightCursor->getBox( box );
   } else {
      // Not highlighted.
      m_cursor->getBox( box );
   }
}

