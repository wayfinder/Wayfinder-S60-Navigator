/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SYMBIANSPRITE_H
#define SYMBIANSPRITE_H 

#include "config.h"
#include "SpriteMover.h"
#include <w32std.h>
#include <fbs.h>

#include "PixelBox.h"
#include "Cursor.h"

class MC2Point;

/**
 *    Symbian sprite.
 */
class SymbianSprite : public SpriteMover {
   public:

      /**
       *    Constructor.
       *    @param   sprite   The sprite. Will not be deleted by
       *                      this class, since it's not new:ed here.
       *    @param   box      PixelBox for the sprite.
       */
      SymbianSprite( RWsSprite* sprite, const PixelBox& box );
     
      /**
       *    Destructor.
       */
      virtual ~SymbianSprite() {}

      /**
       *    Move the sprite to the specified position.
       */
      void moveTo( const MC2Point& pos );

      /**
       *    Get the box of the sprite.
       */
      void getBox( PixelBox& box ) const;

      /**
       *    Get the position of the sprite.
       */
      MC2Point getPosition() const;

      /**
       *    Set if the sprite should be visible.
       */
      void setVisible( bool visible );

   private:
      
      /**
       *    The sprite.
       */
      RWsSprite* m_sprite;

      /**
       *    The box of the sprite.
       */
      PixelBox m_box;

      /**
       *    If the sprite is visible or not.
       */
      bool m_visible;
};

/**
 *    Class that holds a symbian sprite and all it's data.
 */
class SymbianSpriteHolder : public SymbianSprite {
   public:

      /**
       *    Creates a SymbianSpriteHolder with the given params.
       */
      static SymbianSpriteHolder* createSymbianSpriteHolder(
            RWsSession &ws,
            RWindowTreeNode& window, 
            const TPoint& pos, 
            const TDesC& mbmFilename,
            TInt32 bitmapID,
            TInt32 maskID,
            int visible );
      
      /**
       *    Constructor.
       */
      SymbianSpriteHolder( RWsSprite* sprite, 
                           const PixelBox& box,
                           CFbsBitmap* bitmap,
                           CFbsBitmap* bitmapMask );
     
      /**
       *    Destructor. Will delete the sprite and the bitmaps.
       */
      ~SymbianSpriteHolder();

   private:
      /**
       *    The bitmap.
       */
      CFbsBitmap* m_bitmap;

      /**
       *    The mask.
       */
      CFbsBitmap* m_bitmapMask;

      /**
       *    The sprite.
       */
      RWsSprite* m_sprite;
};

#endif
