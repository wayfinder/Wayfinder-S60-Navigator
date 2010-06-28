/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef PLOTTER_ADAPTER_H 
#define PLOTTER_ADAPTER_H

#include "MapPlotter.h"

namespace isab {
/**
 *   Adapter class for MapPlotter, i.e. forwards all calls
 *   to the MapPlotter member.
 */
class PlotterAdapter : public isab::MapPlotter {   
public:
   
   PlotterAdapter( isab::MapPlotter* plotter );

   /**
    *   Virtual destructor.
    */
   virtual ~PlotterAdapter();
   
   // -- Functions that must be implemented ---------------
   
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
   virtual void setPenColor( unsigned int red,
                             unsigned int green,
                             unsigned int blue );

   /**
    *   Puts the dimensions of the displayed map in the
    *   variables.
    */
   void getMapSizePixels(Rectangle& size) const;

   /**
    *   Creates a pointvector of size <code>size</code>.
    */
   PointVector* createPointVector(int size);

   /**
    *   Returns the pointvector to the MapPlotter.
    */
   void deletePointVector(PointVector* vect);

   // Functions that should be re-implemented. ---------------------->

   int nbrMaskedExtraBitmaps() const;

   int switchBitmap( int nbr, int mask );
   
   // Text functions -------------------------------------------------

   /**
    *   Creates a text string for use with the other text methods.
    *   Default implementation returns NULL.
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
    *   Sets font. The description format in the server
    *   and the MapPlotter must know what needs to be put
    *   here.
    *   @param fontName Name of the font in client-specific format.
    *   @param size     Size of the font in UNIT.
    */
   void setFont( const STRING& fontName, int size );
   
   /**
    *   Returns the rotation step possible by the plotter.
    *   <br />Default implementation returns 0.
    *   @return The minimum number of degrees that characters can
    *           be rotated.
    */
   int getMinTextRotation();

   /**
    *    Draw straight text. The specified point is at the baseline, 
    *    left of the text.
    */
   void drawTextAtBaselineLeft( const STRING& text,
                                        const MC2Point& point );
   
   /**
    *   Draws text on the display using the pen color and transparent
    *   background.
    *   @param text      The start of the text to draw.
    *   @param points    The center point of the text.
    *   @param startIdx  The first character to draw.
    *   @param nbrChars  The number of characters to draw.
    */
   void drawText( const STRING& text,
                          const MC2Point& point,
                          int startIdx,
                          int nbrChars,
                          float angle );

   /**
    *   Puts the bounding boxes of each character in str
    *   into the vector boxes.
    *   <br />
    *   Default implementation does nothing and returns 0.
    *   @param boxes  Vector to add the boxes to.
    *   @see drawText
    *   @return Number of characters in str.
    */
   Rectangle getStringAsRectangle( const STRING& text,
                                           const MC2Point& point,
                                           int startIdx,
                                           int nbrChars,
                                           float angle );

   
   /**
    *   Puts the bounding boxes of each character in str
    *   into the vector boxes.
    *   <br />
    *   Default implementation does nothing and returns 0.
    *   @param boxes  Vector to add the boxes to.
    *   @see drawText
    *   @return Number of characters in str.
    */
   int getStringAsRectangles( std::vector<Rectangle>& boxes,
                                      const STRING& text,
                                      const MC2Point& point,
                                      int startIdx,
                                      int nbrChars,
                                      float angle );

   // Misc ----------------------------------------------------------

   /**
    *   True if the mapplotter looks at the mask of the images
    *   to create the visible boundingbox.
    */
   bool returnsVisibleSizeOfBitmaps() const;
   
   /**
    *   Returns true if the drawing is double buffered.
    *   Default implementation returns false.
    *   @return True if the drawing is double buffered.
    */
   bool doubleBuffered() const;
   
   /**
    *   Tells the MapPlotter that it is ok to change buffer
    *   if it is doubleBuffered. Default implementation does
    *   nothing.
    *   @return doubleBuffered().
    */
   bool drawBuffer( const Rectangle& rect );

   /**
    *   If the MapPlotter needs to prepare the plotting in
    *   any way it should do it in this method which is called
    *   before drawing.
    *   <br />
    *   The default implementation does not do anything.
    */
   void prepareDrawing();
   
   /**
    *   Must be called by the drawing engine everytime a drawing
    *   is completed. This is to be able to free up the resources
    *   that may have been used during the draw.
    *   <br />
    *   The default implementation does not do anything.
    */
   void drawingCompleted();
   
   /**
    *   Converts the bytes into a bitmap that can be drawn by the
    *   MapPlotter.
    *   Default implementation returns NULL.
    *   FIXME: Should we add the type of image too, e.g. PNG etc.?
    */
   BitMap* createBitMap(bitMapType type,
                                const uint8* bytes,
                                int nbrBytes,
                                int dpiCorrectionFactor );

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
   void getBitMapAsRectangle(Rectangle& size,
                                     const MC2Point& origCenter,
                                     const BitMap* bmp);

   /**
    *   Deletes the supplied bitmap.
    */
   void deleteBitMap( BitMap* bitMap );

   /**
    *   Returns the wanted bitmap extension.
    *   Default implementation returns png. Dot will be appended
    *   by TileMapHandler.
    */
   const char* getBitMapExtension() const;

   /**
    *   Sets the fill color for polygons.
    *   Default implementation sets the pen color.
    *   @see setPenColor.
    */
   void setFillColor( unsigned int red,
                              unsigned int green,
                              unsigned int blue );

   
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
                                      uint32 fillcolor );

   /**
    *   Draws a polyline defined by the array of points.
    */
   virtual void drawPolyLine( std::vector<MC2Point>::const_iterator begin,
                              std::vector<MC2Point>::const_iterator end );

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
                                       int lineWidth );

   
   /**
    *   Default implementation draws a polyline using the fill color.
    */
   //virtual void drawPolyPolygon( const PointVectorVector& polygons );
                        

   /**
    *   Draws a spline of some sort.
    *   <br />
    *   Default implementation draws a polyline if not filled else polygon.
    */
   void drawSpline( bool filled,
                            std::vector<MC2Point>::const_iterator begin,
                            std::vector<MC2Point>::const_iterator end );
                    

   /**
    *   Same as drawPolygons, but the polygons must be convex. Can be
    *   implemented to speed things up.
    *   <br />
    *   Default implementation calls drawPolygons.
    */
   void drawConvexPolygon( std::vector<MC2Point>::const_iterator begin,
                                   std::vector<MC2Point>::const_iterator end );
   
   /**
    *   Draws an arc.
    *   Default implementation draws a line from one corner to the other
    *   in the rectangle.
    */
   void drawArc( bool filled,
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
   void drawRect( bool filled,
                          const Rectangle& rect,
                          int cornerSize);
     
   /**
    *   Sets the width of the lines.
    *   Default implementation does nothing.
    */
   virtual void setLineWidth( int width );
   
   /**
    *   Sets the cap style of the pen.
    *   Default implementation does nothing.
    */
   void setCapStyle( enum capStyle );

   /**
    *   Sets the dash style of the pen.
    *   Default implementation does nothing.
    */
   void setDashStyle( enum dashStyle );

   /**
    *   Puts the dimensions of the displayed map in the
    *   variables. Must be overridden if the drawing units
    *   of the map is not pixels.
    *   <br />
    *   Default implementation returns getMapSizePixels.
    */
   void getMapSizeDrawingUnits(Rectangle& size) const;

   /**
    *   If the plotter can handle the screen as a bitmap snapshot, i.e.
    *   if makeScreenSnapshot(), moveScreenSnapshot() and
    *   zoomScreenSnapshot() are properly implemented.
    */
   bool snapshotHandlingImplemented() const;
   
   /**
    *   Creates a bitmap screen snapshot and stores it.
    *   Default implementation does nothing.
    */
   void makeScreenSnapshot();

   /**
    *   Moves the screen snapshot.
    */
   void moveScreenSnapshot( const MC2Point& deltaPoint );

   /**
    *   Zooms the screen snapshot. Value larger than one means zoom in.
    */
   void zoomScreenSnapshot(double factor, const MC2Point& zoomPoint);

protected:
   /**
    *   The plotter to perform the operations on.
    */
   isab::MapPlotter* m_plotter;
   
   /**
    *   The line width.
    */ 
   int m_lineWidth;

};
} // namespace
#endif
