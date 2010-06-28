/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __MAPRENDERER_H__
#define __MAPRENDERER_H__

#include "MapPlotter.h"
#include <e32std.h>
#include <gdi.h>
#include <vector>
#include <list>
#include <map>

#include "Surface.h"

/* forward declarations */
class CFbsFont;
class CCoeControl;
class CMdaServer;
class ImageConverter;
//class CSurface;
class CWindowGc;
class CFbsBitGc;

// Use one draw buffer since it flickers
#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
#define USE_ONE_DRAW_BUFFER
#endif

// Use one draw buffer to save memory
#if defined NAV2_CLIENT_SERIES60_V1
#define USE_ONE_DRAW_BUFFER
#endif

namespace isab {


/**
 * Font notice that can be used in a stl map.
 * The user of this class must keep track of if the fontname should be
 * deleted or not.
 */
class FontNotice
{

public:
   
   /// Constructor. Note that the pointer to the fontName is stored.
   FontNotice( STRING* fontName, int fontSize ) :
      m_fontName( fontName ), m_fontSize( fontSize ) {}

   /// Operator <. Needed for sorting.
   bool operator < ( const FontNotice& other ) const {
      if ( m_fontSize < other.m_fontSize ) {
         return true;
      } else if ( m_fontSize > other.m_fontSize ) {
         return false;
      } else {
         return *m_fontName < *other.m_fontName;
      }
   }

private:

   /// Ptr to the font name. It's not copied, or deleted in the destructor.
   STRING* m_fontName;

   /// The size of the font.
   int m_fontSize;
};

/// The font cache.
typedef std::map<FontNotice, CFbsFont*> fontCache_t;

   
/* class for point arrays */
/* uses TPoint from the Symbian GDI system */
class PointArray : public PointVector
{
   private:
      CArrayFixFlat<TPoint>* iArray;
      int iMaxPoints;
   public:
      /* constructors */
      PointArray();
      int Construct(int size);
      /* destructors */      
      ~PointArray();
      /* gets a specified point from the array */
      const TPoint& getPoint(int index);
      
      /**
       *   Sets the points in the vector.
       */
      void setPoints(const std::vector<MC2Point>& points);
      
      /* returns the internal array used to store points.
         Required for Symbian Drawing routines */
      const CArrayFixFlat<TPoint>* getArray();
      /* from PointVector - adds a point to the array */
      void addPoint(int x, int y);
      int getSize() const;
      void reset();
      int getAllocatedSize() const;
      /* returns the Array which stores the points */
      const CArrayFix<TPoint>* getPointArray() const;
};

/* defines */
#define MIN_ALPHA    0x00
#define MAX_ALPHA    0xFF

/* class for Alpha Maps */
class alphaMap
{
      /* the actual raster data */
      uint8* m_data;
      /* stores the array as a TPtr8 */
      TPtr8 m_dataPtr;
      /* dimensions of the bitmap */
      int32 m_width, m_height;
      /* total size of the data in double-words */
      int32 m_dataSize;

      /* for speed, stores the offset into the array for every scanline. */
      /* Is allocated to the height of the bitmap */
      int32* m_scanOffset;

      /* private constructor */
      alphaMap();

      /* second-phase constructor */
      /* returns true on success, false otherwise */
      bool create(int32 w, int32 h);

   protected:

   public:

      /* allocates a bitmap */
      static alphaMap* allocate(int32 w, int32 h);

      /* allocates a bitmap from the specified CSurface 
         and sets all mask colored pixels to alpha = 0 */
      static alphaMap* allocate(CSurface& bmp, uint8 r, uint8 g, uint8 b);
      
      /**
       *    Allocates a minimal doubled rotated bitmap using fixpoint
       *    calculations in the inner loop.
       */
      static alphaMap* allocateDoubleAndRotate(CSurface& bmp, 
                                       uint8 r, uint8 g, uint8 b,
                                       float angle);

      /* destructor */
      ~alphaMap();

      /* clear to color */
      void clear(uint8 alpha);

      /* sets a pixel */
      void setPixel(uint32 x, uint32 y, uint8 alpha);

      /* gets the alpha of a pixel */
      /* if the pixel is out of bounds, 
         doesnt change the value of the value of the variable. */
      void getPixel(uint32 x, uint32 y, uint8& alpha);

      /* gets the filtered alpha value of a pixel */
      /* if the pixel is out of bounds, 
         doesnt change the value of the variable. */
      void getFilteredPixel(uint32 x, uint32 y, uint8& alpha);

      /* gets the array as a TDes */
      TDes8& getDataPtr();

      /* get the array */
      const uint8* getArray();

      /* get the width and height */
      int32 getWidth() const;
      int32 getHeight() const;

      /* blits the bitmap to a GraphicsContext */
      void blitTo(CSurface& outSurface, int32 dX, int32 dY, 
                  uint8 r, uint8 g, uint8 b);

      /* blits the bitmap to another alphaMap and resizes it  */
      void resizeBlitTo(alphaMap& dstBmp, int32 dX, int32 dY, 
                        int32 newWidth, int32 newHeight,
                        bool filtered = false);

      /* blits the bitmap to another alphaMap and rotates it  */
      void rotateBlitTo(alphaMap& dstBmp, int32 dX, int32 dY, 
                        float angle);

      /// Get the size in memory in bytes of the alpha map.
      uint32 getSizeInMem() const;
};



/* MapRenderer */
class MapRenderer : public isab::MapPlotter
{
   /* Data */
   public:
      /* overall draw time */
      uint32 m_lastDrawTime;

   private:

      /// Current buffer to draw on      
      CSurface* iDrawBuffer;

      enum {
         /// The index of the buffer for the map
         c_mainBuffer = 0,
         /// The index of the buffer for the user-defs
         c_secondBuffer = 1,
         /// The index of the mask for the user-defs
         c_secondBufferMask = 2,
#ifdef USE_ONE_DRAW_BUFFER
         /// The number of draw buffers.
         c_nbrDrawBuffers = 1,
#else
         /// The number of draw buffers.
         c_nbrDrawBuffers = 3,
#endif
      };
      
      /// The draw buffers
      CSurface* m_buffers[c_nbrDrawBuffers];
     
      /// The snapshot buffer.
      CSurface* m_screenSnapshot;
      
      /* stores color information */
      TRgb m_backColor;   /* Wanted background color */
      TRgb m_penColor;   /* Wanted foreground (Drawing) color */
      TRgb m_fillColor;   /* Wanted set fill color */
      /// Foreground color for the mask
      TRgb m_maskForeColor;
      /// Background color for the mask
      TRgb m_maskBackColor;
      /// Last set foreground color
      TRgb m_lastPenColor;
      /// Last set fill color
      TRgb m_lastFillColor;
      CFbsFont* iCurFont;   /* current font */
      TInt iCurSize;      /* current line drawing size */
      TInt iCapStyle;      /* stores the type of line ending to use */
      TInt iDashStyle;   /* stores the type of line drawing to use */
      TPtrC8 iDataBuffer;   /* used for memory image loading */
      CCoeControl& iParent;   /* the parent control */
      /// The color depth.
      int m_colorDepth;

      /* the image converter for bitmaps */
      ImageConverter* iImageConverter;

      /* for testing drawing speeds */
      uint32 logStart, logEnd;
      uint32 m_adjustTimeDelta;

      /// Max rotated cache size constants.
      enum {
         // At most 50k can be used for the cached bitmaps.
         c_maxCacheBmpSize = 50*1024,
         // And at least 500k memory must be available before using
         // this cache.
         c_minMemFree = 500*1024
      };

      /**
       *    String with angle notice.
       *    first is heap alloced string. Don't forget to delete afterwards.
       *    second is angle.
       */
      typedef std::pair< HBufC*, int > strAngle_t;
 
      /**
       *    Rotated text alphamaps.
       */
      typedef std::pair<strAngle_t, alphaMap*> rotBmp_t;
      
      /**
       *    List of cached rotated text alphamaps.
       */
      std::list< rotBmp_t > m_rotatedBmps;
      
   /* Methods */
   private:   
      /* Constructors - Private, so further derivation is not possible */
      MapRenderer(CCoeControl& aParent);
      void ConstructL(TInt aWidth, TInt aHeight, TInt aColordepth);

      /* returns a rotated point */
      void rotatePoint(float inX, float inY, 
                       float angle, 
                       float* outX, float* outY);

      /* returns the bounds of a rotated rectangle */
      Rectangle getRotatedBBox(const MC2Point& center, 
                               int width, int height,
                               float angle); 
   
      /// Changes the real pen color
      inline void changePenColor( const TRgb& newPen );
      /// Changes the real fill color
      inline void changeBrushColor( const TRgb& newBrush );

public:

      /**
       *   Returns the number of extra masked bitmaps.
       *   (For redraws in steps.
       */
      int nbrMaskedExtraBitmaps() const { 
#ifdef USE_ONE_DRAW_BUFFER
         return 0;
#else
         return 1;
#endif 
      }

      /**
       *   Change to a new bitmap. Returns true if it exists.
       *   0,1 is invalid since the first bitmap never has a mask.
       */
      int switchBitmap( int nbr, int mask );
   
      /* destructor */
      ~MapRenderer();
      /* allocation routines */
      static MapRenderer* NewLC(CCoeControl& aParent,
                                TInt aWidth, TInt aHeight, TInt aColordepth);
      static MapRenderer* NewL(CCoeControl& aParent,
                               TInt aWidth, TInt aHeight, TInt aColordepth);

      /**
       *   Blits the internal bitmaps to different kinds of Gc:s.
       */
      template<class DESTGC> void RenderTo( DESTGC& destgc,
                                            const MC2Point& point,
                                            TRect rect = TRect() ) {
         if ( iDrawBuffer == NULL ) {
            return;
         }
         if ( rect == TRect() ) {
            // Old version
            rect = TRect( TPoint(0,0), iDrawBuffer->Device().SizeInPixels() );
         }
         
         // Need a valid rectangle for these.
         destgc.BitBlt(point, &iDrawBuffer->Bitmap(), rect );
#ifndef USE_ONE_DRAW_BUFFER
         if ( m_maxBufferSwitchedTo >= c_secondBufferMask ) {
            destgc.BitBltMasked(point,
                                &(m_buffers[c_secondBuffer]->Bitmap()),
                                rect,
                                &(m_buffers[c_secondBufferMask]->Bitmap()),
                                false ); // True for inverted mask
         }
#endif
      }

      /* blits the internal drawing buffer to the specified graphics context */
      template<class DESTGC> void RenderTo( DESTGC& destgc,
                                            TInt dx, TInt dy,
                                            const TRect& rect = TRect() ) {
         RenderTo( destgc, MC2Point(dx,dy), rect );
      }
      
      /* inherited from MapPlotter */
      /* pure virtual functions */
      void clearScreen();
      void setBackgroundColor( unsigned int red,
                               unsigned int green,
                               unsigned int blue );

      void setPenColor( unsigned int red,
                        unsigned int green,
                        unsigned int blue );

      inline void setPenColor(unsigned int  rgb24bits ) {
         setPenColor( ( rgb24bits >> 16) & 0xff,
                      ( rgb24bits >>  8) & 0xff,
                      ( rgb24bits >>  0) & 0xff );
      }

      void drawPolyLine( std::vector<MC2Point>::const_iterator begin,
                         std::vector<MC2Point>::const_iterator end );
      
      void drawPolyLineWithColor( std::vector<MC2Point>::const_iterator begin,
                                  std::vector<MC2Point>::const_iterator end,
                                  uint32 color,
                                  int lineWidth );
      
      void getMapSizePixels(Rectangle& size) const;
      PointVector* createPointVector(int size);
      void deletePointVector(PointVector* vect);
      
      /**
       *   If the plotter can handle the screen as a bitmap snapshot, i.e.
       *   if makeScreenSnapshot(), moveScreenSnapshot() and
       *   zoomScreenSnapshot() are properly implemented.
       */
      bool snapshotHandlingImplemented() const;

      /**
       *	  Creates a bitmap screen snapshot and stores it.
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
      
      /* virtual functions */

      // -- TEXT

     /**
    *  Should be overridden to return true by platforms
    *  that support image scaling.
    */
   virtual bool supportsBitMapScaling() const {
      return true;
   }

   /**
    *  Should be overridden by platforms that support image
    *  scaling.
    */ 
   virtual bool updateBitmapDimensions(BitMap* image, int newWidth, int newHeight);
   
private:
     
      /// Internal method for setting the symbian font.
      void setFont( CFbsFont* theFont );
      /// Internal function for getting a locigal font.
      //int getLogicalFont(const STRING& fontName, class TFontSpec& fontSpec);

public:
      void setFont( const STRING& fontName, int size );
      
      int getMinTextRotation();

      /**
       *    Will create a rotated alphamap ready to be "blitted".
       *    @param text       The text to be drawn.
       *    @param angle      The angle of the text.
       *    @param textWidth  The width (in pixels) of the text.
       *    @param textHeight The height (in pixels) of the text.
       *    @return A new alphaMap for the rotated and antialiased
       *            text.
       */
      alphaMap* createRotatedBmp( STRING& text, float angle, 
                                  int textWidth, int textHeight );

      void drawText( const STRING& text,
                     const MC2Point& point,
                     int startIdx =  0,
                     int nbrChars = -1,
                     float angle  =  0.0 );

      Rectangle getStringAsRectangle( const STRING& text,
                                      const MC2Point& point,
                                      int startIdx =  0,
                                      int nbrChars = -1,
                                      float angle  = 0.0 );

      int getStringAsRectangles( std::vector<Rectangle>& boxes,
                                 const STRING& text,
                                 const MC2Point& point,
                                 int startIdx =  0,
                                 int nbrChars = -1,
                                 float angle  = 0.0 );

      /**
       *   Get if the text rendering direction is left to right
       *   or not (arabic, hebrew is right to left) for the
       *   specified text.
       *   Default implementation returns true.
       */
      bool textDirectionLeftToRight( const STRING& text );
      
      // - BITMAP
      bool returnsVisibleSizeOfBitmaps() const;

      bool doubleBuffered() const;
      bool drawBuffer( const Rectangle& rect );
      BitMap* createBitMap(isab::bitMapType,
                           const unsigned char* bytes,
                           int nbrBytes,
                           bool shouldScale,
                           int dpiCorrectionFactor = 1);
      
      void drawBitMap( const MC2Point& center,
                       const BitMap* bitMap);
      
      void getBitMapAsRectangle(Rectangle& size,
                                const MC2Point& origCenter,
                                const BitMap* bmp);      
      
      void deleteBitMap( BitMap* bitMap );

      // - POLYGONS ETC
      
      void drawPolygon( std::vector<MC2Point>::const_iterator begin,
                        std::vector<MC2Point>::const_iterator end );

      void drawPolygonWithColor( std::vector<MC2Point>::const_iterator begin,
                                 std::vector<MC2Point>::const_iterator end,
                                 uint32 fillcolor );
      
      void drawSpline( bool filled,
                       std::vector<MC2Point>::const_iterator begin,
                       std::vector<MC2Point>::const_iterator end );
      
      void drawArc( bool filled, const Rectangle& rect, int startAngle,
                    int stopAngle);
      void drawRect( bool filled, const Rectangle& rect, int cornerSize = 0);
      void setFillColor( unsigned int red,
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

      void setLineWidth( int width );
      void setCapStyle( capStyle cStyle );
      void setDashStyle( dashStyle dStyle );
      void prepareDrawing();
      void drawingCompleted();
      
      /**
       *   Gets the size of the specified BitMap in pixels
       *   Has to be implemented in derived class.
       *   @param size Rectangle which the bitmap is inside.
       *   @param bmp  Bitmap to get the size from.
       *   @param centerXY The center of the bitmap
       *                   (position where it is drawn).
       */
      void getBitMapAsRectangle(Rectangle& size,
                                const BitMap* bmp,
                                const MC2Point& origXY);

      /**
       *   Gets the size of the specified BitMap in pixels
       *   The other getBitMapAsRectangle has to be implemented
       *   in the subclasses.
       *   @param size Rectangle which the bitmap is inside.
       *   @param bmp  Bitmap to get the size from.
       *   @param centerXY The center of the bitmap (position
       *                   where it is drawn).
       */ 
      Rectangle getBitMapAsRectangle( const MC2Point& origCenter,
                                      const BitMap* bmp) {
         Rectangle rect;
         getBitMapAsRectangle(rect, origCenter, bmp);
         return rect;
      }
      
      /* Added by Varun */
      /* Returns a heap-allocated string in the Native Character format */
      STRING* createString(const char* text);
      /* Frees a heap-allocated string */
      void deleteString(STRING* text);
      
      /**
       *    @return The number of characters in the string.
       */
      int getStringLength( const STRING& text );
      
      /* few more primitives added */
      void drawPoint(int aX, int aY);
      void drawLine(int aSx, int aSy, int aEx, int aEy);
      void drawCharacter(const CHARACTER textchar, int aX, int aY);

      /* 
       * Resizes the MapRenderer to the specified size.
       * Returns true if the resize worked, false on an error 
       */
      bool resizeBuffer(uint32 newXSize, uint32 newYSize);

      /** 
       * Locks the buffer of the CSurface bitmap currently in used.
       */
      void lockBitmapBuffer();

      /**
       * Unlocks the buffer of the CSurface bitmap currently in used.
       */
      void unlockBitmapBuffer();

      /**
       * Returns the data adress to the bitmap in the currently used CSurface.
       */
      uint32* getBitmapBuffer();

      /**
       * Returns the current color depth.
       */
      int getColorDepth();
      
      /* CHETAN : Returns the internal back-buffer of the MapRenderer ...
         Needed for back-buffered controls in WayFinder UIQ */
      CSurface* getBuffer() const
      {
         return(iDrawBuffer);
      }

      inline bool isMask( const CSurface* surface ) const {
         return surface == m_buffers[ c_secondBufferMask ];
      }

      /// Contains the last font used. Valid if iCurFont != NULL.
      TFontSpec m_lastFontSet;
      
      // Set to the maximum buffer switched to ever.
      int m_maxBufferSwitchedTo;
      
      // For debug
      int m_curLine;

      /// Font cache.
      fontCache_t m_fontCache;

};

} // namespace isab

#endif
