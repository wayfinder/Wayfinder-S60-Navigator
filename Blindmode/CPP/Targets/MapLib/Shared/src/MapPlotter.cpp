/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "MapPlotter.h"
#include "MC2Point.h"

using namespace std;
using namespace isab;

// -- PointVector
void
PointVector::setPoints(const vector<MC2Point>& points)
{
   reset();
   for( vector<MC2Point>::const_iterator it = points.begin();
        it != points.end();
        ++it ) {
      addPoint( (*it).getX(), (*it).getY() );
   }
}

// -- MapPlotter.

/* default implementations for virtual functions */
void MapPlotter::setFont( const STRING& /*fontName*/, int /*size*/ )
{
	return;
}


int MapPlotter::getMinTextRotation()
{
	return(0);
}

STRING*
MapPlotter::createString(const char*)
{
   return NULL;
}

void
MapPlotter::deleteString(STRING*)
{
}

int
MapPlotter::getStringLength( const STRING& /*text*/ ) 
{
   return -1;
}


void 
MapPlotter::drawTextAtBaselineLeft( const STRING& text,
                                    const MC2Point& point )
{
   // XXX: This method should be overriden by the subclasses for
   // improved efficiency.
   isab::Rectangle rect = getStringAsRectangle( text, point );
   // Convert to bottom center point.
   MC2Point centerPoint( point.getX() + rect.getWidth() / 2,
                         point.getY() - rect.getHeight() / 2 );
   
   drawText( text, centerPoint );
}


void
MapPlotter::drawText( const STRING& /*text*/,
                      const MC2Point& /*point*/,
                      int /*startIdx*/,
                      int /*nbrChars*/,
                      float /*angle*/)
{
}

isab::Rectangle
MapPlotter::getStringAsRectangle( const STRING& /*text*/,
                                  const MC2Point& /*point*/,
                                  int /*startIdx*/,
                                  int /*nbrChars*/,
                                  float /*angle*/ )
{
   return Rectangle(0,0,0,0);
}

int
MapPlotter::getStringAsRectangles( vector<isab::Rectangle>& /*boxes*/,
                                   const STRING& /*text*/,
                                   const MC2Point& /*point*/,
                                   int /*startIdx*/,
                                   int /*nbrChars*/,
                                   float /*angle*/)
{
   return 0;
}

bool MapPlotter::doubleBuffered() const
{
	return 0;
}

bool MapPlotter::drawBuffer( const isab::Rectangle& /*rect*/ )
{
	return 0;
}

void MapPlotter::prepareDrawing()
{
	return;
}

void MapPlotter::drawingCompleted()
{
	return;
}

BitMap* MapPlotter::createBitMap(bitMapType /*type*/,
                                 const unsigned char* /*bytes*/,
                                 int /*nbrBytes*/,
                                 int /*dpiCorrectionFactor*/)
{
	return (NULL);
}

void MapPlotter::drawBitMap( const MC2Point&,
                             const BitMap*)
{
	return;
}

void MapPlotter::deleteBitMap( BitMap* )
{
	return;
}

void
MapPlotter::getBitMapAsRectangle(isab::Rectangle&,
                                 const MC2Point&,
                                 const isab::BitMap*)
{
}

const char*
MapPlotter::getBitMapExtension() const
{
#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1
   return "gif";
#elif defined NAV2_CLIENT_SERIES60_V3
   return "mif";
#else
   return "png";
#endif
}

void MapPlotter::drawPolygon( vector<MC2Point>::const_iterator begin,
                              vector<MC2Point>::const_iterator end )
{
	drawPolyLine(begin, end);
}

void MapPlotter::drawSpline( bool /*filled*/,
                             vector<MC2Point>::const_iterator begin,
                             vector<MC2Point>::const_iterator end )
{
   drawPolyLine(begin, end);
}

void MapPlotter::drawConvexPolygon( vector<MC2Point>::const_iterator begin,
                                    vector<MC2Point>::const_iterator end )
{
   drawPolygon(begin, end);
}


void MapPlotter::drawRect( bool filled,
                           const isab::Rectangle& rect,
                           int /*cornerSize*/)
{
   // Create Rectangle from pointvector
   vector<MC2Point> points;
   points.push_back( MC2Point( rect.getX(), rect.getY() ) );
   points.push_back( MC2Point( rect.getX(), rect.getY() + rect.getHeight() ) );
   points.push_back( MC2Point( rect.getX() + rect.getWidth(),
                               rect.getY() + rect.getHeight() ) );
   points.push_back( MC2Point( rect.getX() + rect.getWidth(),
                               rect.getY() ) );
   points.push_back( MC2Point( rect.getX(), rect.getY() ) );
   // Draw filled rectangle or not.
	if ( filled ) {
      drawPolygon( points.begin(), points.end() );
   } else {
      drawPolyLine( points.begin(), points.end() );
   }   
}

void MapPlotter::drawArc( bool filled, const isab::Rectangle& rect,
                          int /*startAngle*/, int /*stopAngle*/)
{
	drawRect(filled, rect);
}

void MapPlotter::setFillColor( unsigned int /*red*/, unsigned int /*green*/,
                               unsigned int /*blue*/ )
{
	return;
}

void MapPlotter::setLineWidth( int /*width*/ )
{
	return;
}

void MapPlotter::setCapStyle( capStyle /*cStyle*/ )
{
	return;
}

void MapPlotter::setDashStyle( dashStyle /*dStyle*/ )
{
	return;
}

void MapPlotter::getMapSizeDrawingUnits(isab::Rectangle& size) const
{
	getMapSizePixels(size);
	return;
}

bool MapPlotter::snapshotHandlingImplemented() const
{
   // Default is that the snapshot handling is not implemented.
   return false;
}

void MapPlotter::makeScreenSnapshot()
{
   // Default implementation does nothing.
}

void MapPlotter::moveScreenSnapshot( const MC2Point& deltaPoint )
{
   // Default implementation does nothing.
}

void MapPlotter::zoomScreenSnapshot(double factor, const MC2Point& zoomPoint)
{
   // Default implementation does nothing.
}

