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
#include "SymbianSprite.h"
#include <e32std.h>
#ifdef NAV2_CLIENT_SERIES60_V3
#include <akniconutils.h>
#include <aknutils.h>
#endif
#include "MC2Point.h"

SymbianSprite::SymbianSprite( RWsSprite* sprite, 
                              const PixelBox& box ) 
   : m_sprite( sprite ), m_box( box ), m_visible( true )
{
   moveTo( m_box.getCenterPoint() );
}
      
void 
SymbianSprite::moveTo( const MC2Point& pos )
{
   m_box.moveCenterTo( pos );
   if ( m_visible ) {
      m_sprite->SetPosition( TPoint( pos.getX(), pos.getY() ) );
   }
}

void
SymbianSprite::getBox( PixelBox& box ) const
{
   box = m_box;
}

MC2Point
SymbianSprite::getPosition() const
{
   return m_box.getCenterPoint();
}

void 
SymbianSprite::setVisible( bool visible )
{
   m_visible = visible;
   if ( m_visible ) {
      m_sprite->SetPosition( TPoint( m_box.getCenterPoint() ) );
   } else {
      // Hide it by moving it outside the screen... Very nice.
      m_sprite->SetPosition( TPoint( 2000, 2000 ) );
   }
}

void 
SymbianSprite::updateSize()
{   
}

// ----------------------  SymbianSpriteHolder ----------------------------

SymbianSpriteHolder* 
SymbianSpriteHolder::createSymbianSpriteHolder(
                                 RWsSession &ws,
                                 RWindowTreeNode& window, 
                                 const TPoint& pos, 
                                 const TDesC& mbmFilename,
                                 TInt32 bitmapID,
                                 TInt32 maskID,
                                 int visible )
{
   // Create the sprite.
   RWsSprite* sprite = new RWsSprite( ws );
   sprite->Construct( window, pos, 0 );            

   CFbsBitmap* bitmap = new CFbsBitmap;
   CFbsBitmap* bitmapMask = new CFbsBitmap;

#ifdef NAV2_CLIENT_SERIES60_V3
   AknIconUtils::CreateIconL(bitmap, 
                             bitmapMask, 
                             mbmFilename, 
                             bitmapID,
                             maskID);
   TSize iconSize = calcSize();
   AknIconUtils::SetSize(bitmap, iconSize);
#else
   // Create and load the bitmap.
   bitmap->Load( mbmFilename, bitmapID );
   // Create and load the bitmap mask.
   bitmapMask->Load( mbmFilename, maskID );

#endif
   // Create the spritemember
   TSpriteMember spriteMember;
   spriteMember.iBitmap = bitmap;
   spriteMember.iMaskBitmap = bitmapMask;
   // Center the bitmap.
   TSize size = bitmap->SizeInPixels();
   spriteMember.iOffset = TPoint( -(size.iWidth >> 1),
                                  -(size.iHeight >> 1) );

   // XXX: For some strange reason the mask must be inverted even if
   // it isn't inverted.
   spriteMember.iInvertMask = ETrue;

   // Add the spritemember to the sprite.
   sprite->AppendMember( spriteMember );

   // Create the box.
   PixelBox cursorBox( MC2Point( 0, 0 ),
                       MC2Point( size.iWidth, size.iHeight ) );
   cursorBox.moveCenterTo( MC2Point( pos.iX, pos.iY ) );

   SymbianSpriteHolder* ssHolder = 
      new SymbianSpriteHolder( sprite, 
                               cursorBox,
                               bitmap,
                               bitmapMask );
   ssHolder->setVisible( visible );
   // Activate.
   sprite->Activate();

   
   return ssHolder;
}

SymbianSpriteHolder::SymbianSpriteHolder( RWsSprite* sprite, 
                                          const PixelBox& box,
                                          CFbsBitmap* bitmap,
                                          CFbsBitmap* bitmapMask ) : 
                  SymbianSprite( sprite, box ),
                  m_bitmap( bitmap ),
                  m_bitmapMask( bitmapMask ),
                  m_sprite( sprite )
{

}


SymbianSpriteHolder::~SymbianSpriteHolder() 
{
   m_sprite->Close();
   delete m_sprite;
   delete m_bitmap;
   delete m_bitmapMask;
}

TSize
SymbianSpriteHolder::calcSize()
{
#ifdef NAV2_CLIENT_SERIES60_V3
   // Calculating the size of the cursor. Since the cursor is 30x30
   // on V2 phones, the width of the cursor should be the same as the
   // height on V3 phones. We are therefore using the width to calculate
   // the size of the cursor.
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
   TBool landscape = (rect.Width() >= rect.Height());
   TSize iconSize;
   if (landscape) {
      iconSize = TSize(TInt((float(30) / 176) * (rect.Height() * 0.8)),
                       TInt((float(30) / 176) * (rect.Height() * 0.8)));
   } else {
      iconSize = TSize(TInt((float(30) / 176) * (rect.Width() * 0.8)),
                       TInt((float(30) / 176) * (rect.Width() * 0.8)));
   }
   return iconSize;
#else
   TSize iconSize(30,30);
   return iconSize;
#endif
}

void
SymbianSpriteHolder::updateSize()
{
#ifdef NAV2_CLIENT_SERIES60_V3
   if (m_bitmap && m_bitmapMask && m_sprite) {
      TSize iconSize = calcSize();
      AknIconUtils::SetSize(m_bitmap, iconSize);
      
      TSpriteMember spriteMember;
      spriteMember.iBitmap = m_bitmap;
      spriteMember.iMaskBitmap = m_bitmapMask;
      TSize size = m_bitmap->SizeInPixels();
      spriteMember.iOffset = TPoint( -(size.iWidth >> 1),
                                     -(size.iHeight >> 1) );
      spriteMember.iInvertMask = ETrue;
      m_sprite->UpdateMember( 0, spriteMember );
   }
#endif
}
