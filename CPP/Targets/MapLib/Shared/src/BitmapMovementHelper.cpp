/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BitmapMovementHelper.h"

BitmapMovementHelper::BitmapMovementHelper() : 
                             m_lastPoint( 0, 0 ), 
                             m_dragPoint( 0, 0 ),
                             m_moveOffset( 0, 0 )
{
   reset();
}
   
void 
BitmapMovementHelper::move( const MC2Point& delta )
{
   // The delta is subtracted from m_moveOffset, since
   // the delta corresponds to how the map should be moved.
   // If the map should be moved to the left, then it means
   // that the bitmap should be moved to the right.
   m_moveOffset -= delta;
}

void 
BitmapMovementHelper::moveTo( const MC2Point& screenPoint )
{
   m_lastPoint = screenPoint;
   m_moveOffset = screenPoint - m_dragPoint;
}
   
void 
BitmapMovementHelper::setDragPoint( const MC2Point& dragPoint )
{
   m_dragPoint = dragPoint;
}

void 
BitmapMovementHelper::zoom( double deltaFactor )
{
   m_zoomFactor *= deltaFactor;
}

void 
BitmapMovementHelper::reset()
{
   m_dragPoint = m_lastPoint;
   m_moveOffset = MC2Point( 0, 0 );
   m_zoomFactor = 1.0;
}

MC2Point 
BitmapMovementHelper::getMoveOffset() const
{
   return m_moveOffset;
}

double 
BitmapMovementHelper::getZoomFactor() const
{
   return m_zoomFactor;
}


