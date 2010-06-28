/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GTKMAPPLOTTER_H
#define GTKMAPPLOTTER_H

#include "config.h"

#include "MapPlotter.h"
#include "MapPlotterTypes.h"
#include "MC2SimpleString.h"

#include <gtk/gtk.h>

class GtkBitMap : public isab::BitMap {
public:
   GtkBitMap():m_pixMap(0), m_bitMap(0) {}

   /// The pixmap
   GdkPixmap *m_pixMap;
   /// Hopefully the mask
   GdkBitmap *m_bitMap;
};


class GtkPointVector : public isab::PointVector {
public:
   /**
    *   Allocates room for size coordinates.
    */
   GtkPointVector(int size);

   /**
    *   Deallocates allocated memory.
    */
   virtual ~GtkPointVector();
   
   /**
    *   Adds one point to the end of the vector.
    */
   void addPoint(int x, int y);

   /**
    *   Adds many points to the end of the vector.
    */
   void setPoints(const vector<MC2Point>& points);

   /**
    *   Returns the size of the vector.
    */
   int getSize() const;

   /**
    *
    */
   void reset();

   /**
    *
    */
   int getAllocatedSize() const;

   /**
    *
    */
   const vector<GdkPoint>& getPoints() const;
       
   
protected:
   /**
    *   Number of added points in vector.
    */
   int m_nbrAdded;

   /**
    *   The allocated size.
    */
   int m_allocatedSize;

   /**
    *   This is where we keep our coordinates.
    */
   vector<GdkPoint> m_points;
   
};


/**
 *    Driver for plotting in the Gtk environment.
 *
 */
class GtkMapPlotter : public isab::MapPlotter {
public:
   /**
    *    Create a new GtkMapPlotter.
    *    @param window The window to draw in.
    */
   explicit GtkMapPlotter(GdkDrawable *window);

   /**
    *    Destructor.
    */
   ~GtkMapPlotter();

   // - Functions that must be implemented from MapPlotter.

   /**
    *   Fills the screen with the background color.
    */
   void clearScreen();
   
   /**
    *   Sets the background color.
    *   <br />
    *   NB! The server will not send impossible values once the
    *   description formats are done.
    *   @param red   Red value   0-255.
    *   @param green Green value 0-255.
    *   @param blue  Blue value  0-255.
    */
   void setBackgroundColor( unsigned int red,
                            unsigned int green,
                            unsigned int blue );

   /**
    *   Sets the color of the pen.
    *   <br />
    *   NB! The server will not send impossible values once the
    *   description formats are done.
    *   @param red   Red value   0-255.
    *   @param green Green value 0-255.
    *   @param blue  Blue value  0-255.
    */
   void setPenColor( unsigned int red,
                     unsigned int green,
                     unsigned int blue );

   // -- TEXT 

   /**
    *   Sets the font that we wants to use.
    */
    void setFont( const STRING& fontName, int size = 0 );

    /**
     *   Returns the current font name.
     */
    const char* getFont() const;
    
   /**
    *   Creates and returns a copy of the char* text.
    */
   STRING* createString(const char* text);

   /**
    *   Deletes a text string.
    *   Default implementation does nothing.
    */   
   void deleteString(STRING* text);
  
   /**
    *   @return The number of characters in the string.
    */
   int getStringLength( const STRING& text );
 
   /**
    *   Sets the fill color for polygons.
    *   Default implementation sets the pen color.
    *   @see setPenColor.
    */
   void setFillColor( unsigned int red,
                      unsigned int green,
                      unsigned int blue );

   /**
    *   Draws text on the display using the pen color and transparent
    *   background.
    *   FIXME: Indeces not taken into account.
    *   FIXME: Rotation not taken into accout.
    *   @param text      The start of the text to draw.
    *   @param points    The center point of the text.
    *   @param startIdx  The first character to draw.
    *   @param nbrChars  The number of characters to draw.
    */
   virtual void drawText( const STRING& text,
                          const MC2Point& point,
                          int startIdx =  0,
                          int nbrChars = -1,
                          float angle  =  0.0 );

   /**
    *   Puts the bounding boxes of each character in str
    *   into the vector boxes.
    *   <br />
    *   FIXME: Indeces not taken into account.
    *   FIXME: Rotation not taken into accout.
    *   @param boxes  Vector to add the boxes to.
    *   @see drawText
    *   @return Number of characters in str.
    */
   virtual isab::Rectangle getStringAsRectangle( const STRING& text,
                                                 const MC2Point& point,
                                                 int startIdx =  0,
                                                 int nbrChars = -1,
                                                 float angle  = 0.0 );

   /**
    *
    */
   virtual int getStringAsRectangles( 
            vector<isab::Rectangle>& boxes,
            const STRING& text,
            const MC2Point& center,
            int startIdx,
            int nbrChars,
            float angle );
    
   /**
    *   Draws a polyline defined by the array of points.
    */
   void drawPolyLine( const vector<MC2Point>::const_iterator begin,
                      const vector<MC2Point>::const_iterator end);

   /**
    *   Draws a polyline defined by the iterators to the screen coords
    *   and the supplied color and line width. Override this to
    *   reduce the number of virtual functions called by the
    *   TileMapHandler.
    *   @param begin     First point draw.
    *   @param end       Point after last point to draw.
    *   @param color     Color in rrggbb format.
    *   @param lineWidth Line width in pixels.
    */
   void drawPolyLineWithColor( vector<MC2Point>::const_iterator begin,
                               vector<MC2Point>::const_iterator end,
                               uint32 color,
                               int lineWidth ) {
      setLineWidth( lineWidth );
      MapPlotter::setPenColor( color );
      drawPolyLine( begin, end );
   }
   
   /**
    *   Draws a polygon filled with the fill color and with a border
    *   with set with setPenColor and linewith as set by setLineWidth
    *   and dashstyle set by setDashStyle.
    *   If borderColor == penColor or lineWidth == 0 no border should
    *   be drawn.
    *   <br />
     */
   void drawPolygon( const vector<MC2Point>::const_iterator begin,
                     const vector<MC2Point>::const_iterator end);

   void drawPolygonWithColor( vector<MC2Point>::const_iterator begin,
                              vector<MC2Point>::const_iterator end,
                              uint32 fillcolor ) {
      MapPlotter::setFillColor( fillcolor );
      drawPolygon( begin, end );
   }
   
   /**
    *   Puts the dimensions of the displayed map in the
    *   variables.
    */
   void getMapSizePixels(isab::Rectangle& size) const;

   /**
    *   Creates a pointvector of size <code>size</code>.
    */
   isab::PointVector* createPointVector(int size);

   /**
    *   Returns the pointvector to the MapPlotter.
    */
   void deletePointVector(isab::PointVector* vect);
   
   /**
    *   Sets the width of the lines.
    *   Default implementation does nothing.
    */
   void setLineWidth( int width );

   /**
    *   Creates a bitmap from an xpm. Type is ignored.
    */
   isab::BitMap* createBitMap(isab::bitMapType type,
                              const uint8* bytes,
                              int nbrBytes,
                              int dpiCorrectionFactor = 1);

   /**
    *   Draws a bitmap at the center coordinate.
    */
   void drawBitMap( const MC2Point& center,
                    const isab::BitMap* bitMap);

   /**
    *   Gets the size of the specified BitMap in pixels
    *   Has to be implemented in derived class.
    *   @param size Rectangle which the bitmap is inside.
    *   @param bmp  Bitmap to get the size from.
    *   @param centerXY The center of the bitmap (position where it is drawn).
    */
   void getBitMapAsRectangle( isab::Rectangle& size,
                              const MC2Point& origCenter,
                              const isab::BitMap* bmp );
   
   /**
    *   Deletes the supplied bitmap.
    *   @see MapPlotter
    */
   void deleteBitMap( isab::BitMap* bitMap );

   /**
    *   Returns xpm.
    */
   const char* getBitMapExtension() const { return "png"; }
  
   /**
    *   Prepares drawing.
    */
   void prepareDrawing();

   /**
    *   Blits the pixmap.
    */
   void drawingCompleted();
   
private:

   /**
    *   Draws a background for the text.
    */
   void drawTextBackground(vector<isab::Rectangle> boxes, float angle);

   /**
    *   Plots some stuff at every coordinate.
    */
   void plotCoordinates( vector<MC2Point>::const_iterator begin,
                         vector<MC2Point>::const_iterator end, 
                         unsigned int color = 0);
   
   /**
    *    allocates color in the colormap
    */
   void allocColor(GdkColor& theColor,
                   unsigned int red,
                   unsigned int green,
                   unsigned int blue );

   /**
    *
    */
   void drawPolySomething( vector<MC2Point>::const_iterator begin,
                           vector<MC2Point>::const_iterator end,
                           bool filled);
   
   /**
    *    The window where the drawables will be drawn.
    */
   GdkDrawable *m_window;

   /**
    *    Graphic context
    */
   GdkGC *m_gc;

 
   /**
    *    The color of the pen.
    */
   GdkColor m_penColor;

   /**
    *    The color of the fill.
    */
   GdkColor m_fillColor;

   /**
    *    The color of the background.
    */
   GdkColor m_backColor;

   /**
    *    The color of the foreground.
    */
   GdkColor m_foregroundColor;
      
   /**
    *    Temporary backstore.
    */
   GdkPixmap *m_pixmap;

   /** 
    *    True if all coordinates should be shown.
    */
   bool m_plotCoordinates;

   /**
    * Pango context used for text drawing
    */
   PangoContext *m_context;
   /**
    * Pango layout used for text drawing
    */
   PangoLayout *m_layout;

   /// Current fontname.
   MC2SimpleString m_fontName;
};


#endif

