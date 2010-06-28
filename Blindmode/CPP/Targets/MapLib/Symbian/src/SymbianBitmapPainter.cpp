/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "SymbianBitmapPainter.h"

#include "MC2Point.h"

class SymbianBitmapEntry
{
public:
   SymbianBitmapEntry() : m_position( 0, 0 )
   {
      m_next = 0;
      m_bitmap = 0;
      m_mask = 0;
   }
   ~SymbianBitmapEntry()
   {
      // If the ownership of CFbsBitmap is tranfered
      // to this class when adding the bitmap,
      // then here should be delete m_bitmap
   }
   
   MC2Point m_position;
   CFbsBitmap* m_bitmap;
   CFbsBitmap* m_mask;
   SymbianBitmapEntry* m_next;
};

SymbianBitmapPainter::SymbianBitmapPainter()
{
}

SymbianBitmapPainter::~SymbianBitmapPainter()
{
   SymbianBitmapEntry* current = m_firstBitmap;
   while (current) {
      SymbianBitmapEntry* next = current->m_next;
      delete current;
      current = next;
   }
}

void SymbianBitmapPainter::DrawBitmaps( CWindowGc& gc ) const
{
   SymbianBitmapEntry* current = m_firstBitmap;
   while (current) {
      if (current->m_mask) {
         gc.BitBltMasked( TPoint( current->m_position.getX(), 
                                  current->m_position.getY() ),
                          current->m_bitmap,
                          TRect(current->m_bitmap->SizeInPixels()), 
                          current->m_mask, 
                          ETrue );
      } else {
         gc.BitBlt( TPoint( current->m_position.getX(), 
                            current->m_position.getY() ), 
                    current->m_bitmap );
      }
      current = current->m_next;
   }
}

void SymbianBitmapPainter::DrawBitmaps( CFbsBitGc& gc ) const
{
   SymbianBitmapEntry* current = m_firstBitmap;
   while (current) {
      if (current->m_mask) {
         gc.BitBltMasked( TPoint( current->m_position.getX(), 
                                  current->m_position.getY() ),
                          current->m_bitmap,
                          TRect(current->m_bitmap->SizeInPixels()), 
                          current->m_mask, 
                          ETrue );
      } else {
         gc.BitBlt( TPoint( current->m_position.getX(), 
                            current->m_position.getY() ), 
                    current->m_bitmap );
      }
      current = current->m_next;
   }
}

void SymbianBitmapPainter::addBitmap( const MC2Point& pos, 
                                      CFbsBitmap* bitmap, 
                                      CFbsBitmap* mask )
{
   SymbianBitmapEntry* entry = new SymbianBitmapEntry();
   if (entry) {
      entry->m_position = pos;
      entry->m_bitmap = bitmap;
      entry->m_mask = mask;
      entry->m_next = m_firstBitmap;
      m_firstBitmap = entry;
   }
}

void SymbianBitmapPainter::removeBitmap( CFbsBitmap* bitmap )
{
	SymbianBitmapEntry* last = 0;
	SymbianBitmapEntry* current = m_firstBitmap;

	while (current && current->m_bitmap != bitmap)
	{
		last = current;
		current = current->m_next;
	}

	if (last)
		last->m_next = current->m_next;
	else
		m_firstBitmap = current->m_next;

	delete current;
}

void SymbianBitmapPainter::moveBitmap( CFbsBitmap* bitmap, const MC2Point& pos )
{
	SymbianBitmapEntry* current = m_firstBitmap;
	
	while (current && current->m_bitmap != bitmap)
		current = current->m_next;

	if (current)
		current->m_position = pos;
}
