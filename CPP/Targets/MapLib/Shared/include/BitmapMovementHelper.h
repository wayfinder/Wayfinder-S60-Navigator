/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef BITMAP_MOVEMENT_HELPER_H 
#define BITMAP_MOVEMENT_HELPER_H 

#include "MC2Point.h"

/**
 *   Class that helps moving / zooming etc the map as a snapshot bitmap.
 */
class BitmapMovementHelper {
public:

   /**
    *    Constructor. 
    */
   BitmapMovementHelper();
   
   /**
    *    Moves the bitmap corresponding to that the map should be
    *    moved the specified delta.
    */
   void move( const MC2Point& delta );

   /**
    *    Moves the dragPoint of the bitmap to a new point.
    */
   void moveTo( const MC2Point& screenPoint );

   /**
    *    Set the drag point.
    */
   void setDragPoint( const MC2Point& dragPoint );
   
   /**
    *    Zooms the display a delta factor. 
    */
   void zoom( double deltaFactor );

   /**
    *    Resets the delta values. To be called when the map bitmap
    *    is in sync with reality (i.e. just have been drawn).
    */
   void reset();
   
   /**
    *    Get the total offset to move the bitmap.
    */
   MC2Point getMoveOffset() const;
  
   /**
    *    Get the total factor to zoom the bitmap.
    */
   double getZoomFactor() const;
   
private:

   /**
    *   The last point that the bitmap was moved to.
    */
   MC2Point m_lastPoint;

   /**
    *    The anchor point for dragging the map, i.e. moveTo.
    */
   MC2Point m_dragPoint;

   /**
    *    The move offset for the bitmap.
    */
   MC2Point m_moveOffset;

   /**
    *    The zoom factor for the bitmap.
    */
   double m_zoomFactor;
   
};

#endif
