/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYMBIAN_BITMAP_PAINTER_H
#define SYMBIAN_BITMAP_PAINTER_H

#include "SymbianBitmapDrawingInterface.h"

#include <coecntrl.h>

class SymbianBitmapEntry;

class SymbianBitmapPainter : public SymbianBitmapDrawingInterface
{
public:
   SymbianBitmapPainter();
   ~SymbianBitmapPainter();
   
   void DrawBitmaps( CWindowGc& gc ) const;
   void DrawBitmaps( CFbsBitGc& gc ) const;
   
   /**
    *    Add the bitmap at the specified position.
    *    The position is relative to the upper left corner, 
    *    depending on the orientation of the phone (landscape, portrait).
    */
   void addBitmap( const MC2Point& pos, 
                   CFbsBitmap* bitmap, 
                   CFbsBitmap* mask = 0 );
   
   /**
    *    Remove the bitmap.
    *    The bitmap must already have been added using addBitmap.
    */
   void removeBitmap( CFbsBitmap* bitmap );
   
   /**
    *    Move the bitmap to the specified position.
    *    The position is relative to the upper left corner, 
    *    depending on the orientation of the phone (landscape, portrait).
    *    The bitmap must already have been added using addBitmap.
    */
   void moveBitmap( CFbsBitmap* bitmap, const MC2Point& pos );
   
private:
   SymbianBitmapEntry* m_firstBitmap;
};

#endif
