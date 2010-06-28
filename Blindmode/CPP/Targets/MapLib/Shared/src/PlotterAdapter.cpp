/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PlotterAdapter.h"

using namespace isab;

PlotterAdapter::PlotterAdapter( MapPlotter* plotter )
{
   m_plotter = plotter;
   m_lineWidth = 0;
}

PlotterAdapter::~PlotterAdapter() 
{

}

void PlotterAdapter::clearScreen()
{
   m_plotter->clearScreen();
}

void PlotterAdapter::setBackgroundColor( unsigned int red,
      unsigned int green,
      unsigned int blue )
{
   m_plotter->setBackgroundColor( red, green, blue );
}

void PlotterAdapter::setPenColor( unsigned int red,
      unsigned int green,
      unsigned int blue )
{
   m_plotter->setPenColor( red, green, blue );
}

void PlotterAdapter::getMapSizePixels(Rectangle& size) const
{
   m_plotter->getMapSizePixels( size );
}

PointVector* PlotterAdapter::createPointVector(int size)
{
   return m_plotter->createPointVector( size );
}

void PlotterAdapter::deletePointVector(PointVector* vect)
{
   m_plotter->deletePointVector( vect );
}


int PlotterAdapter::nbrMaskedExtraBitmaps() const
{
   return m_plotter->nbrMaskedExtraBitmaps();
}

int PlotterAdapter::switchBitmap( int nbr, int mask )
{
   return m_plotter->switchBitmap( nbr, mask );
}


STRING* PlotterAdapter::createString(const char* text)
{
   return m_plotter->createString( text );
}

void PlotterAdapter::deleteString(STRING* text)
{
   m_plotter->deleteString( text );
}

int PlotterAdapter::getStringLength( const STRING& text )
{
   return m_plotter->getStringLength( text );
}

void PlotterAdapter::setFont( const STRING& fontName, int size )
{
   m_plotter->setFont( fontName, size );
}

int PlotterAdapter::getMinTextRotation()
{
   return m_plotter->getMinTextRotation();
}

void PlotterAdapter::drawTextAtBaselineLeft( const STRING& text,
      const MC2Point& point )
{
   m_plotter->drawTextAtBaselineLeft( text, point );
}

void PlotterAdapter::drawText( const STRING& text,
      const MC2Point& point,
      int startIdx,
      int nbrChars,
      float angle )
{
   m_plotter->drawText( text, point, startIdx, nbrChars, angle );
}

Rectangle PlotterAdapter::getStringAsRectangle( const STRING& text,
      const MC2Point& point,
      int startIdx,
      int nbrChars,
      float angle )
{
   return m_plotter->getStringAsRectangle( text, point, startIdx, nbrChars, angle );
}


int PlotterAdapter::getStringAsRectangles( std::vector<Rectangle>& boxes,
      const STRING& text,
      const MC2Point& point,
      int startIdx,
      int nbrChars,
      float angle )
{
   return m_plotter->getStringAsRectangles( boxes, text, point, startIdx, nbrChars, angle );
}

bool PlotterAdapter::returnsVisibleSizeOfBitmaps() const
{
   return m_plotter->returnsVisibleSizeOfBitmaps();
}

bool PlotterAdapter::doubleBuffered() const
{
   return m_plotter->doubleBuffered();
}

bool PlotterAdapter::drawBuffer( const Rectangle& rect )
{
   return m_plotter->drawBuffer( rect );
}

void PlotterAdapter::prepareDrawing()
{
   m_plotter->prepareDrawing();
}

void PlotterAdapter::drawingCompleted()
{
   m_plotter->drawingCompleted();
}

BitMap* PlotterAdapter::createBitMap(bitMapType type,
      const uint8* bytes,
      int nbrBytes,
      int dpiCorrectionFactor )
{
   return m_plotter->createBitMap( type, bytes, nbrBytes, dpiCorrectionFactor );
}

void PlotterAdapter::drawBitMap( const MC2Point& center,
      const BitMap* bitMap)
{
   m_plotter->drawBitMap( center, bitMap );
}

void PlotterAdapter::getBitMapAsRectangle(Rectangle& size,
      const MC2Point& origCenter,
      const BitMap* bmp)
{
   m_plotter->getBitMapAsRectangle( size, origCenter, bmp );
}

void PlotterAdapter::deleteBitMap( BitMap* bitMap )
{
   m_plotter->deleteBitMap( bitMap );
}

const char* PlotterAdapter::getBitMapExtension() const
{
   return m_plotter->getBitMapExtension();
}

void PlotterAdapter::setFillColor( unsigned int red,
      unsigned int green,
      unsigned int blue )
{
   m_plotter->setFillColor( red, green, blue );
}


void PlotterAdapter::drawPolygon( std::vector<MC2Point>::const_iterator begin,
      std::vector<MC2Point>::const_iterator end )
{
   m_plotter->drawPolygon( begin, end ); 
}

void PlotterAdapter::drawPolygonWithColor( std::vector<MC2Point>::const_iterator begin,
      std::vector<MC2Point>::const_iterator end,
      uint32 fillcolor )
{
   m_plotter->drawPolygonWithColor( begin, end, fillcolor );
}

void PlotterAdapter::drawPolyLine( std::vector<MC2Point>::const_iterator begin,
      std::vector<MC2Point>::const_iterator end )
{
   m_plotter->drawPolyLine( begin, end );
}

void PlotterAdapter::drawPolyLineWithColor( std::vector<MC2Point>::const_iterator begin,
      std::vector<MC2Point>::const_iterator end,
      uint32 color,
      int lineWidth )
{
   m_plotter->drawPolyLineWithColor( begin, end, color, lineWidth );
}


void PlotterAdapter::drawSpline( bool filled,
      std::vector<MC2Point>::const_iterator begin,
      std::vector<MC2Point>::const_iterator end )
{
   m_plotter->drawSpline( filled, begin, end );
}


void PlotterAdapter::drawConvexPolygon( std::vector<MC2Point>::const_iterator begin,
      std::vector<MC2Point>::const_iterator end )
{
   m_plotter->drawConvexPolygon( begin, end );
}

void PlotterAdapter::drawArc( bool filled,
      const Rectangle& rect,
      int startAngle,
      int stopAngle)
{
   m_plotter->drawArc( filled, rect, startAngle, stopAngle );
}

void PlotterAdapter::drawRect( bool filled,
      const Rectangle& rect,
      int cornerSize)
{
   m_plotter->drawRect( filled, rect, cornerSize );
}

void PlotterAdapter::setLineWidth( int width )
{
   m_lineWidth = width;
   m_plotter->setLineWidth( width );
}

void PlotterAdapter::setCapStyle( enum capStyle cap )
{
   m_plotter->setCapStyle( cap );
}

void PlotterAdapter::setDashStyle( enum dashStyle dash )
{
   m_plotter->setDashStyle( dash );
}

void PlotterAdapter::getMapSizeDrawingUnits(Rectangle& size) const
{
   m_plotter->getMapSizeDrawingUnits( size );
}

bool PlotterAdapter::snapshotHandlingImplemented() const
{
   return m_plotter->snapshotHandlingImplemented();
}

void PlotterAdapter::makeScreenSnapshot()
{
   m_plotter->makeScreenSnapshot();
}

void PlotterAdapter::moveScreenSnapshot( const MC2Point& deltaPoint )
{
   m_plotter->moveScreenSnapshot( deltaPoint );
}

void PlotterAdapter::zoomScreenSnapshot(double factor, const MC2Point& zoomPoint)
{
   m_plotter->zoomScreenSnapshot( factor, zoomPoint );
}

