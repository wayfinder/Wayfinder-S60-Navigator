/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MAP_PLOTTER_H
#define MAP_PLOTTER_H

#include "MapPlotterConfig.h"
#include "MapPlotterTypes.h"

#include <vector>

class MC2Point;

namespace isab {

/**
 *   Abstract class representing a vector of coordinates.
 *   To be used as a base class for specialized classes used
 *   by the MapPlotter subclasses.
 *   Should be templated into the MapPlotter later for performance.
 */
class PointVector {
public:

   /**
    *   Virtual destructor.
    */
   virtual ~PointVector() {};
   
   /**
    *   Adds one coordinate to the vector.
    *   @param x The x coordinate.
    *   @param y The y coordinate.
    */   
   virtual void addPoint( int x, int y ) = 0;

   /**
    *   Adds many points at once to the vector.
    *   Default implementation just calls the reset mehtod and then addPoint
    *   method for each point.
    */
   virtual void setPoints(const std::vector<MC2Point>& points);
   
   /**
    *   Returns the number of coordinates in the vector.
    */
   virtual int getSize() const = 0;

   /**
    *   Resets the addposition ( and size ) back to zero.
    */
   virtual void reset() = 0;

   /**
    *   Returns the number of allocated points.
    */
   virtual int getAllocatedSize() const = 0;
   
};

/**
 *   Abstract class representing a bitmap suitable for use
 *   in the drawing system. Nothing is defined here, but
 *   the pointers can be used for passing the BitmMaps in and
 *   out of the MapPlotter.
 */
class BitMap {   
};

/**
 *   Abstract class that should be used to describe a device
 *   that is able to draw a map. See it as a driver.
 *   <br />
 *   <b>NB!</b> It should be possible to use this file in
 *              many environments so the default implementations
 *              must not use anything other than the functions in
 *              MapPlotter.
 */
class MapPlotter {   
public:

   /**
    *   Virtual destructor.
    */
   virtual ~MapPlotter() {}
   
   // -- Functions that must be implemented ---------------
   
   /**
    *   Fills the screen with the background color.
    */
   virtual void clearScreen() = 0;
   
   /**
    *   Sets the background color.
    *   <br />
    *   NB! The server will not send impossible values once the
    *   description formats are done.
    *   @param red   Red value   0-255.
    *   @param green Green value 0-255.
    *   @param blue  Blue value  0-255.
    */
   virtual void setBackgroundColor( unsigned int red,
                                    unsigned int green,
                                    unsigned int blue ) = 0;

   /**
    *   Sets the color of the pen.
    *   <br />
    *   NB! The server will not send impossible values once the
    *   description formats are done.
    *   @param red   Red value   0-255.
    *   @param green Green value 0-255.
    *   @param blue  Blue value  0-255.
    */
   virtual void setPenColor( unsigned int red,
                             unsigned int green,
                             unsigned int blue ) = 0;

   /**
    *   Sets the color of the pen using one 24bit color value.
    *   @param rgb24bits Red in bits 23-16 etc.
    */
   inline void setPenColor(unsigned int  rgb24bits ) {
      setPenColor( ( rgb24bits >> 16) & 0xff,
                   ( rgb24bits >>  8) & 0xff,
                   ( rgb24bits >>  0) & 0xff );
   }
   
   /**
    *   Puts the dimensions of the displayed map in the
    *   variables.
    */
   virtual void getMapSizePixels(Rectangle& size) const = 0;

   /**
    *   Creates a pointvector of size <code>size</code>.
    */
   virtual PointVector* createPointVector(int size) = 0;

   /**
    *   Returns the pointvector to the MapPlotter.
    */
   virtual void deletePointVector(PointVector* vect) = 0;

   // Functions that should be re-implemented. ---------------------->

   virtual int nbrMaskedExtraBitmaps() const { return 0; }

   virtual int switchBitmap( int nbr, int mask ) {
      return nbr == 0 && mask == false;
   }
   
   // Text functions -------------------------------------------------

   /**
    *   Creates a text string for use with the other text methods.
    *   Default implementation returns NULL.
    */
   virtual STRING* createString(const char* text);

   /**
    *   Deletes a text string.
    *   Default implementation does nothing.
    */   
   virtual void deleteString(STRING* text);
  
   /**
    *   @return The number of characters in the string.
    */
   virtual int getStringLength( const STRING& text );
   
   /**
    *   Sets font. The description format in the server
    *   and the MapPlotter must know what needs to be put
    *   here.
    *   @param fontName Name of the font in client-specific format.
    *   @param size     Size of the font in UNIT.
    */
   virtual void setFont( const STRING& fontName, int size );
   
   /**
    *   Returns the rotation step possible by the plotter.
    *   <br />Default implementation returns 0.
    *   @return The minimum number of degrees that characters can
    *           be rotated.
    */
   virtual int getMinTextRotation();

   /**
    *    Draw straight text. The specified point is at the baseline, 
    *    left of the text.
    */
   virtual void drawTextAtBaselineLeft( const STRING& text,
                                        const MC2Point& point );
   
   /**
    *   Draws text on the display using the pen color and transparent
    *   background.
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
    *   Default implementation does nothing and returns 0.
    *   @param boxes  Vector to add the boxes to.
    *   @see drawText
    *   @return Number of characters in str.
    */
   virtual Rectangle getStringAsRectangle( const STRING& text,
                                           const MC2Point& point,
                                           int startIdx =  0,
                                           int nbrChars = -1,
                                           float angle  = 0.0 );

   
   /**
    *   Puts the bounding boxes of each character in str
    *   into the vector boxes.
    *   <br />
    *   Default implementation does nothing and returns 0.
    *   @param boxes  Vector to add the boxes to.
    *   @see drawText
    *   @return Number of characters in str.
    */
   virtual int getStringAsRectangles( std::vector<Rectangle>& boxes,
                                      const STRING& text,
                                      const MC2Point& point,
                                      int startIdx =  0,
                                      int nbrChars = -1,
                                      float angle  = 0.0 );

   // Misc ----------------------------------------------------------

   /**
    *   True if the mapplotter looks at the mask of the images
    *   to create the visible boundingbox.
    */
   virtual bool returnsVisibleSizeOfBitmaps() const {
      return false;
   }
   
   /**
    *   Returns true if the drawing is double buffered.
    *   Default implementation returns false.
    *   @return True if the drawing is double buffered.
    */
   virtual bool doubleBuffered() const;
   
   /**
    *   Tells the MapPlotter that it is ok to change buffer
    *   if it is doubleBuffered. Default implementation does
    *   nothing.
    *   @return doubleBuffered().
    */
   virtual bool drawBuffer( const Rectangle& rect );

   /**
    *   If the MapPlotter needs to prepare the plotting in
    *   any way it should do it in this method which is called
    *   before drawing.
    *   <br />
    *   The default implementation does not do anything.
    */
   virtual void prepareDrawing();
   
   /**
    *   Must be called by the drawing engine everytime a drawing
    *   is completed. This is to be able to free up the resources
    *   that may have been used during the draw.
    *   <br />
    *   The default implementation does not do anything.
    */
   virtual void drawingCompleted();
   
   /**
    *   Converts the bytes into a bitmap that can be drawn by the
    *   MapPlotter.
    *   Default implementation returns NULL.
    *   FIXME: Should we add the type of image too, e.g. PNG etc.?
    */
   virtual BitMap* createBitMap(bitMapType type,
                                const uint8* bytes,
                                int nbrBytes,
                                int dpiCorrectionFactor = 1 );

   /**
    *   Draws the bitmap at x and y.
    *   Default implementation draws a small rectangle.
    */
   virtual void drawBitMap( const MC2Point& center,
                            const BitMap* bitMap);

   /**
    *   Gets the size of the specified BitMap in pixels
    *   Has to be implemented in derived class.
    *   @param size Rectangle which the bitmap is inside.
    *   @param bmp  Bitmap to get the size from.
    *   @param centerXY The center of the bitmap (position where it is drawn).
    */
   virtual void getBitMapAsRectangle(Rectangle& size,
                                     const MC2Point& origCenter,
                                     const BitMap* bmp);

   /**
    *   Gets the size of the specified BitMap in pixels
    *   The other getBitMapAsRectangle has to be implemented in the subclasses.
    *   @param size Rectangle which the bitmap is inside.
    *   @param bmp  Bitmap to get the size from.
    *   @param centerXY The center of the bitmap (position where it is drawn).
    */ 
   Rectangle getBitMapAsRectangle( const MC2Point& origCenter,
                                   const BitMap* bmp) {
      Rectangle rect;
      getBitMapAsRectangle(rect, origCenter, bmp);
      return rect;
   }

   /**
    *   Deletes the supplied bitmap.
    */
   virtual void deleteBitMap( BitMap* bitMap );

   /**
    *   Returns the wanted bitmap extension.
    *   Default implementation returns png. Dot will be appended
    *   by TileMapHandler.
    */
   virtual const char* getBitMapExtension() const;

   /**
    *   Sets the fill color for polygons.
    *   Default implementation sets the pen color.
    *   @see setPenColor.
    */
   virtual void setFillColor( unsigned int red,
                              unsigned int green,
                              unsigned int blue );
   /**
    *   Sets the color of the pen using one 24bit color value.
    *   @param rgb24bits Red in bits 23-16 etc.
    */
   inline void setFillColor(unsigned int  rgb24bits ) {
      setFillColor( ( rgb24bits >> 16) & 0xff,
                   ( rgb24bits >>  8) & 0xff,
                   ( rgb24bits >>  0) & 0xff );
   }

   
   /**
    *   Draws a polygon filled with the fill color and with a border
    *   with set with setPenColor and linewith as set by setLineWidth
    *   and dashstyle set by setDashStyle.
    *   If borderColor == penColor or lineWidth == 0 no border should
    *   be drawn.
    *   <br />
    *   Default implementation draws a polyline using the fill color.
    */
   virtual void drawPolygon( std::vector<MC2Point>::const_iterator begin,
                             std::vector<MC2Point>::const_iterator end );

   /**
    *   Draws a filled polygon defined by the iterators to the screen coords
    *   and the supplied color and line width. Override this to
    *   reduce the number of virtual functions called by the
    *   TileMapHandler.
    *   @param begin     First point draw.
    *   @param end       Point after last point to draw.
    *   @param fillcolor Color to fill with in rrggbb format.
    */
   virtual void drawPolygonWithColor( std::vector<MC2Point>::const_iterator begin,
                                      std::vector<MC2Point>::const_iterator end,
                                      uint32 fillcolor ) = 0;

   /**
    *   Draws a polyline defined by the array of points.
    */
   virtual void drawPolyLine( std::vector<MC2Point>::const_iterator begin,
                              std::vector<MC2Point>::const_iterator end ) = 0;

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
   virtual void drawPolyLineWithColor( std::vector<MC2Point>::const_iterator begin,
                                       std::vector<MC2Point>::const_iterator end,
                                       uint32 color,
                                       int lineWidth ) = 0;

   
   /**
    *   Default implementation draws a polyline using the fill color.
    */
   //virtual void drawPolyPolygon( const PointVectorVector& polygons );
                        

   /**
    *   Draws a spline of some sort.
    *   <br />
    *   Default implementation draws a polyline if not filled else polygon.
    */
   virtual void drawSpline( bool filled,
                            std::vector<MC2Point>::const_iterator begin,
                            std::vector<MC2Point>::const_iterator end );
                    

   /**
    *   Same as drawPolygons, but the polygons must be convex. Can be
    *   implemented to speed things up.
    *   <br />
    *   Default implementation calls drawPolygons.
    */
   virtual void drawConvexPolygon( std::vector<MC2Point>::const_iterator begin,
                                   std::vector<MC2Point>::const_iterator end );
   
   /**
    *   Draws an arc.
    *   Default implementation draws a line from one corner to the other
    *   in the rectangle.
    */
   virtual void drawArc( bool filled,
                         const Rectangle& rect,
                         int startAngle,
                         int stopAngle);

   /**
    *   Draws a rectangle.
    *   @param filled     If true the rectangle is filled.
    *   @param rect       Dimensions of the rectangle.
    *   @param cornerSize Radius of corners.
    *   Default implementation cannot draw rounded corners and uses
    *   drawPolyLine or drawConvexPolygon.
    */
   virtual void drawRect( bool filled,
                          const Rectangle& rect,
                          int cornerSize = 0);
     
   /**
    *   Sets the width of the lines.
    *   Default implementation does nothing.
    */
   virtual void setLineWidth( int width );
   
   /**
    *   Sets the cap style of the pen.
    *   Default implementation does nothing.
    */
   virtual void setCapStyle( enum capStyle );

   /**
    *   Sets the dash style of the pen.
    *   Default implementation does nothing.
    */
   virtual void setDashStyle( enum dashStyle );

   /**
    *   Puts the dimensions of the displayed map in the
    *   variables. Must be overridden if the drawing units
    *   of the map is not pixels.
    *   <br />
    *   Default implementation returns getMapSizePixels.
    */
   virtual void getMapSizeDrawingUnits(Rectangle& size) const;

   /**
    *   If the plotter can handle the screen as a bitmap snapshot, i.e.
    *   if makeScreenSnapshot(), moveScreenSnapshot() and
    *   zoomScreenSnapshot() are properly implemented.
    */
   virtual bool snapshotHandlingImplemented() const;
   
   /**
    *   Creates a bitmap screen snapshot and stores it.
    *   Default implementation does nothing.
    */
   virtual void makeScreenSnapshot();

   /**
    *   Moves the screen snapshot.
    */
   virtual void moveScreenSnapshot( const MC2Point& deltaPoint );

   /**
    *   Zooms the screen snapshot. Value larger than one means zoom in.
    */
   virtual void zoomScreenSnapshot(double factor, const MC2Point& zoomPoint);

};
} // namespace
#endif
