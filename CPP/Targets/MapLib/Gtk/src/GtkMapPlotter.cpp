/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GtkMapPlotter.h"

#include "MC2SimpleString.h"
#include <stdio.h>
#include "PixelBox.h"

#include <algorithm>
#include "MC2Point.h"

using namespace isab;

/// for debug
ostream &operator << (ostream &out, const isab::Rectangle &rect) {
   out<<"( "<<rect.getX()<<", "<<rect.getY()<<", "<<rect.getWidth()<<", "<<rect.getHeight()<<" )";
   return out;
}

/**
 * Get UTF-8 substring from an UTF-8 string
 * @param str utf-8 string
 * @param start start index 
 * @param length of sub string
 * @return allocated utf-8 sub string
 */
char *subStringUTF8(const char *str, int startIdx, int length) {

   gchar *start = g_utf8_offset_to_pointer( str, startIdx );
   gchar *end = g_utf8_offset_to_pointer( str, startIdx + length );

   char *subText = new char[end - start + 2]; // +2 for some extra safety :P
   memset(subText, 0, end - start + 2); 
   return g_utf8_strncpy( subText, start, length);
   
}
/**
 *
 */
void getTextSize( PangoLayout *layout, 
                  const char *str,
                  int &width, int &height ) {
   pango_layout_set_text( layout, str, -1 );
   pango_layout_get_pixel_size( layout, &width, &height );
}

GtkPointVector::GtkPointVector(int size)
{
   m_points.reserve(size);
   m_allocatedSize = size;
}

GtkPointVector::~GtkPointVector()
{
   
}

void
GtkPointVector::addPoint(int x, int y)
{
   GdkPoint p;
   p.x = x;
   p.y = y;
   m_points.push_back( p );
}

class ToGDKPointConverter {
public:
   GdkPoint operator()( const pair<int, int>& inpair ) const {
      GdkPoint p;
      p.x = inpair.first;
      p.y = inpair.second;
      return p;
   }
};

void
GtkPointVector::setPoints( const vector<MC2Point>& points)
{
   m_points.clear();
   m_points.reserve( points.size() );
   GdkPoint p;
   for( vector<MC2Point>::const_iterator it = points.begin();
        it != points.end();
        ++it ) {
      p.x = it->getX();
      p.y = it->getY();
      m_points.push_back(p);
   }
           
}

int
GtkPointVector::getSize() const
{
   return m_points.size();
}

int
GtkPointVector::getAllocatedSize() const
{
   return m_allocatedSize;
}

void
GtkPointVector::reset()
{
   m_points.clear();
}

const vector<GdkPoint>&
GtkPointVector::getPoints() const
{
   return m_points;
}

// --- The GtkMapPlotter

GtkMapPlotter::GtkMapPlotter(GdkDrawable *window)
   : m_window( window ),
     m_gc( gdk_gc_new( window ) ),
     m_pixmap( 0 ),
     m_plotCoordinates( false ),
     m_context( gdk_pango_context_get() ), // default context
     m_layout( pango_layout_new( m_context ) )
{

   // set front
   PangoFontDescription *font_desc = 
      pango_font_description_from_string( getFont() );

   pango_layout_set_font_description( m_layout, font_desc );
   pango_font_description_free(font_desc);


   
}

GtkMapPlotter::~GtkMapPlotter()
{
   g_object_unref( m_layout );
   g_object_unref( m_context );
   g_object_unref( m_gc );
   g_object_unref( m_pixmap );
}

void GtkMapPlotter::prepareDrawing()
{
   // Create the pixmap
   gint width, height;
   gdk_drawable_get_size( m_window, &width, &height );
   gint depth = gdk_drawable_get_depth( m_window );

   // note: do we really need to recreate pixmap/gc each time?
   // seem like it could be done once in constructor and then 
   // recreate if window size != pixmap size
   g_object_unref( m_pixmap );
   m_pixmap = gdk_pixmap_new( m_window, width, height, depth );

   g_object_unref( m_gc );
   m_gc = gdk_gc_new( m_pixmap );
}

void GtkMapPlotter::drawingCompleted()
{
   // Blit the pixmap on to window

   // create default gc
   GdkGC *gc = gdk_gc_new( m_window );   
   gdk_draw_drawable( m_window, gc, m_pixmap,
                      0, 0, 0, 0, // no offset
                      -1, -1); // draw entire pixmap

   g_object_unref( gc );
}

void GtkMapPlotter::clearScreen()
{
   gdk_gc_set_foreground( m_gc, &m_backColor );
   gdk_draw_rectangle( m_pixmap, m_gc, 
                       true,  // filled
                       0, 0, -1, -1 );
   gdk_gc_set_foreground( m_gc, &m_penColor );

}

void
GtkMapPlotter::allocColor(GdkColor &theColor,
                          unsigned int red,
                          unsigned int green,
                          unsigned int blue)
{


   // transform range from 0-255 to 0-65535
   theColor.red = static_cast<int>(red/ 256.0 * 0xFFFF);
   theColor.green = static_cast<int>(green / 256.0 * 0xFFFF);
   theColor.blue = static_cast<int>(blue / 256.0 * 0xFFFF);

   // allocate color
   GdkColormap *colormap = gdk_drawable_get_colormap( m_pixmap );
   gdk_colormap_alloc_color( colormap, &theColor, 
                             true, // color is writable, can be changed with gdk_color_change
                             true ); // try to match the color if allocation failed
   
}

void
GtkMapPlotter::setBackgroundColor( unsigned int red,
                                   unsigned int green,
                                   unsigned int blue )
{
   allocColor(m_backColor, red, green, blue);
   gdk_gc_set_background(m_gc, &m_backColor);
}

void
GtkMapPlotter::setFillColor( unsigned int red,
                             unsigned int green,
                             unsigned int blue )
{
   allocColor(m_fillColor, red, green, blue);
}

void
GtkMapPlotter::setPenColor( unsigned int red,
                            unsigned int green,
                            unsigned int blue )
{
   allocColor(m_penColor, red, green, blue);
}

void 
GtkMapPlotter::setFont( const STRING& fontName, int size )
{
   m_fontName = fontName;
}

const char*
GtkMapPlotter::getFont() const
{
   // override default font
   static const char font[] = "Times Slant 9";
   return font;
   //return m_fontName.c_str();
}

STRING* 
GtkMapPlotter::createString( const char* text)
{
   char** ptr = new char*[1];
   ptr[ 0 ] = new char[ strlen( text ) + 1 ];
   strcpy( ptr[ 0 ], text );
   return ptr;
}

void
GtkMapPlotter::deleteString( STRING* str )
{
   delete [] *str;
   delete [] str;
}

int
GtkMapPlotter::getStringLength( const STRING& text )
{
   return g_utf8_strlen( text, -1 ); 
}



isab::Rectangle 
GtkMapPlotter::getStringAsRectangle( const STRING& text,
                                     const MC2Point& center,
                                     int startIdx,
                                     int nbrChars,
                                     float angle )
{


   if( nbrChars == -1 ) {
      nbrChars = getStringLength( text ); 
   }

   // get sub string from utf-8
   char *subText = subStringUTF8(text, startIdx, nbrChars);

   int width, height;
   getTextSize( m_layout, subText, width, height );

   delete subText;

   return Rectangle(
         center.getX() - ( width >> 1 ),
         center.getY() - height - ( height >> 1 ),
         width, height );
}

int 
GtkMapPlotter::getStringAsRectangles( 
      vector<isab::Rectangle>& boxes,
      const STRING& text,
      const MC2Point& center,
      int startIdx,
      int nbrChars,
      float angle )
{
   if( nbrChars == -1 ){
      nbrChars = getStringLength( text );
   }
   
   MC2_ASSERT( nbrChars!=0 );

   // Get pixel size of substring 
   char *subText = subStringUTF8( text, startIdx, nbrChars );
   int widthOfString, heightOfString;
   getTextSize( m_layout, subText, widthOfString, heightOfString );

   delete subText;

   int halfWidthOfString = widthOfString >> 1;
   // waruns angle goes ccw and we wants it cw so thats why we do -.
   angle = -angle;

   double radAngle = ( angle / 180.0 ) * M_PI;
   double xDiff = cos( radAngle ) * halfWidthOfString; 
   double yDiff = sin( radAngle ) * halfWidthOfString;
   int p1x = (int)(center.getX() - xDiff );
   int p1y = (int)(center.getY() - yDiff );
   int p2x = (int)(center.getX() + xDiff );
   int p2y = (int)(center.getY() + yDiff );
   for ( int i = startIdx; i < startIdx + nbrChars; i++ ) {
      double diffFactor = 
         ( ( i - startIdx + 0.5 ) / (double)nbrChars );
      mc2dbg4 << "diffFactor " << diffFactor << endl;
      MC2Point posOfSubString( 
            (int)(p1x * (1 - diffFactor) + p2x * diffFactor),
            (int)(p1y * (1 - diffFactor) + p2y * diffFactor) );
      
      boxes.push_back( getStringAsRectangle(
               text, posOfSubString, i, 1, angle ) );
   }
   return 1;
}

void 
GtkMapPlotter::drawText( const STRING& text,
                         const MC2Point& center,
                         int startIdx,
                         int nbrChars,
                         float angle )
{

   mc2dbg4 << "name: " << text << endl;
   mc2dbg4 << "bbstartIdx: " << startIdx << endl;
   mc2dbg4 << "bbnbrChars: " << nbrChars << endl;

   // if nbrChars < 0 use all characters from startIdx to the end
   if ( nbrChars < 0 ) {      
      nbrChars = g_utf8_strlen( text, -1 ) - startIdx;
   }

   if ( nbrChars == 0 ) {
      return;
   }

   mc2dbg4 << "[GTKMP]: text    " << MC2CITE(text) << endl;
   mc2dbg4 << "[GTKMP]: centerX " << center.getX() << endl;
   mc2dbg4 << "[GTKMP]: centerY " << center.getY() << endl;

   // create a box for each character
   vector<isab::Rectangle> boxes;
   getStringAsRectangles( boxes, text, center, startIdx, nbrChars, angle );

   gdk_gc_set_foreground( m_gc, &m_penColor );

   // text iterator, get start
   char *text_i = g_utf8_offset_to_pointer( text, startIdx );
   const char *endOfString = g_utf8_offset_to_pointer( text, nbrChars );
   // length of first utf-8 character
   int char_length = g_utf8_find_next_char( text_i, endOfString ) - text_i;

   // for each character box, draw character
   for ( uint32 i = 0; i < boxes.size(); ++i ) {
      isab::Rectangle& rect = boxes[ i ];

      pango_layout_set_text( m_layout, text_i,  char_length);

      gdk_draw_layout( m_pixmap,
                       m_gc,
                       rect.getX(), 
                       rect.getY() + static_cast<int>(rect.getHeight()),
                       m_layout );

      text_i = g_utf8_find_next_char( text_i, endOfString );
      // if next char is null or previous character length was undefined
      // then exit loop
      if (text_i == NULL || char_length < 0)
         break;

      // calculate length of next utf-8 character
      if (g_utf8_find_next_char( text_i, endOfString) == NULL) {
         // we did not find next character.
         // but end of string - current pos is the next size
         char_length = endOfString - text_i;         
      } else {
         char_length = g_utf8_find_next_char( text_i, endOfString) - text_i;
      }

   }

}

void
GtkMapPlotter::plotCoordinates( vector<MC2Point>::const_iterator begin,
                                vector<MC2Point>::const_iterator end )
{   
   
   GdkColor red;
   allocColor(red, 0xFF, 0, 0);
   gdk_gc_set_foreground( m_gc, &red );

   int offset = 2;
   int rectSize = offset << 1;
   for ( vector<MC2Point>::const_iterator it = begin;
         it != end;
         ++it ) {
      gdk_draw_rectangle( m_pixmap, m_gc,
                          true, // filled
                          it->getX()-offset,
                          it->getY()-offset,
                          rectSize, rectSize );
   }

   gdk_gc_set_foreground( m_gc, &m_penColor );

}

void
GtkMapPlotter::drawPolySomething( vector<MC2Point>::const_iterator begin,
                                  vector<MC2Point>::const_iterator end,
                                  bool filled)
{
  
   int npoints = end - begin;
   GdkPoint* gdkPoints = new GdkPoint[npoints];

   int i = 0;
   GdkPoint tmppoint;
   for ( vector<MC2Point>::const_iterator it = begin;
         it != end;
         ++it ) {
      tmppoint.x = it->getX();
      tmppoint.y = it->getY();
      gdkPoints[i++] = tmppoint;
   }
   
   if ( ! filled ) {
      gdk_draw_lines( m_pixmap, m_gc, gdkPoints, npoints );
   } else {
      gdk_draw_polygon( m_pixmap, m_gc,
                        true, // filled
                        gdkPoints, npoints );
   }
   
   delete[] gdkPoints;
   
   if ( m_plotCoordinates ) {
      plotCoordinates( begin, end );
   }

}

void
GtkMapPlotter::drawPolyLine( vector<MC2Point>::const_iterator begin,
                             vector<MC2Point>::const_iterator end )
   
{
   gdk_gc_set_foreground( m_gc, &m_penColor);


   drawPolySomething( begin, end, false );
}

void
GtkMapPlotter::drawPolygon( vector<MC2Point>::const_iterator begin,
                            vector<MC2Point>::const_iterator end )
{
   gdk_gc_set_foreground( m_gc, &m_fillColor);

   drawPolySomething( begin, end, true );
}


void
GtkMapPlotter::getMapSizePixels(isab::Rectangle& size) const
{
   gint width, height;
   gdk_drawable_get_size(m_window, &width, &height);

   size = Rectangle( 0, 0, width, height );
}

isab::PointVector*
GtkMapPlotter::createPointVector(int size)
{
   // Must implement
   return new GtkPointVector(size);
}

void
GtkMapPlotter::deletePointVector(isab::PointVector* vect)
{
   // Must implement
   delete static_cast<GtkPointVector*>(vect);
}

void
GtkMapPlotter::setLineWidth( int width )
{
   // get old attributes and change the width
   GdkGCValues gcv;
   gdk_gc_get_values( m_gc, &gcv);
   gdk_gc_set_line_attributes( m_gc,
                               width,
                               gcv.line_style,
                               gcv.cap_style,
                               gcv.join_style );
}

isab::BitMap*
GtkMapPlotter::createBitMap(isab::bitMapType type,
                            const uint8* bytes,
                            int nbrBytes,
                            int dpiCorrectionFactor )
{

   GtkBitMap* bitMap = new GtkBitMap;
   // Make temporary file to store the bitmap in
   char tempTemplate[1024];   
   sprintf(tempTemplate, "/tmp/tmpbitmapXXXXXX");
   int tmpDesc = mkstemp(tempTemplate);

   FILE* file = fdopen(tmpDesc, "w");
   if ( file ) {
      if ( fwrite( bytes, nbrBytes, 1, file ) == 1 ) {
         fclose(file);

         // Get "transparent" color
         mc2dbg << "[GTMP]: Filename is " << MC2CITE(tempTemplate)
                << endl;

         GdkColor white;
         allocColor(white, 0xFF, 0xFF, 0xFF);
         bitMap->m_pixMap = gdk_pixmap_create_from_xpm( 
                           m_pixmap,                           
                           &(bitMap->m_bitMap),
                           &white,
                           tempTemplate);

      } else {
         fclose(file);
         delete bitMap;
         bitMap = NULL;
      }
   } else {
      delete bitMap;
      bitMap = NULL;
   }

   return bitMap;
}


void
GtkMapPlotter::getBitMapAsRectangle( isab::Rectangle& size,
                                     const MC2Point& origCenter,
                                     const isab::BitMap* bmp )

{
   
   const GtkBitMap* gtkBitMap = static_cast<const GtkBitMap*>(bmp);
   assert(gtkBitMap);
   assert(gtkBitMap->m_bitMap);

   int x = origCenter.getX();
   int y = origCenter.getY();
   int width, height;
   gdk_drawable_get_size(gtkBitMap->m_pixMap, &width, &height);
   
   size = isab::Rectangle( x - ( width >> 1), y - ( height >> 1),
                           width, height );

   
}

void
GtkMapPlotter::drawBitMap( const MC2Point& center,
                           const isab::BitMap* bitMap )
{
   const GtkBitMap* gtkBitMap = static_cast<const GtkBitMap*>(bitMap);
   
   isab::Rectangle rect;
   getBitMapAsRectangle(rect, center, bitMap);

   gdk_gc_set_clip_origin( m_gc, rect.getX(), rect.getY() );
   gdk_gc_set_clip_mask( m_gc, ((GtkBitMap*)gtkBitMap)->m_bitMap );

   gdk_draw_drawable( m_pixmap, m_gc, 
                      gtkBitMap->m_pixMap,
                      0, 0, 
                      rect.getX(), rect.getY(), 
                      rect.getWidth(), rect.getHeight());

   // clear clip origin and mask
   gdk_gc_set_clip_origin( m_gc, 0, 0);
   gdk_gc_set_clip_mask( m_gc, NULL );

   // Draw center coordinate
   if ( m_plotCoordinates ) {
      vector<MC2Point> points;
      points.push_back(center);
      plotCoordinates(points.begin(), points.end());
   }
   
}

void
GtkMapPlotter::deleteBitMap( isab::BitMap* bitMap )
{
   GtkBitMap* gtkBitMap = static_cast<GtkBitMap*>(bitMap);
   
   delete gtkBitMap;
}

void 
GtkMapPlotter::enableLowQualityDrawing( bool on )
{ 
}

