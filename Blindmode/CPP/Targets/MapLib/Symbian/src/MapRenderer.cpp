/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "MapRenderer.h"

#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES60_V3
#else
   // Release the fonts for all devices except buggy s80 and s90.
   // Not releasing the fonts for those devices seem to cause
   // freezing of the phone.
# define RELEASE_FONTS
#endif

#include "Image.h"
#include <eikenv.h>
#include <gdi.h>
#include <e32math.h>
#include <coecntrl.h>
#ifdef NAV2_CLIENT_SERIES60_V3
#include <aknutils.h>
#endif
#include "MapUtility.h"
#include "sincos.h"
#include <math.h>
#include "MC2Point.h"
#include "Surface.h"
#include "PixelBox.h"
#include "ScalableFonts.h"

#ifndef SYMBIAN_9
#   include <hal.h>
#endif

/* TESTING only */
#include "TileMapUtil.h"

/* degree <-> radian conversion routines */
#define DEG2RAD(x)   ((float)( 0.017460317*(float)x))
#define RAD2DEG(x)   ((float)(57.272727272*(float)x))

using namespace std;
using namespace isab;

/* PointArray class for polylines, polygons and splines */
PointArray::PointArray()
{
   iArray = NULL;
   iMaxPoints = 0;
}

int PointArray::Construct(int size)
{
   iArray = new CArrayFixFlat<TPoint>(8);
   if(iArray == NULL) return(KErrNoMemory);

   TRAPD(errCode, iArray->SetReserveL(size));
   if(errCode != KErrNone)
   {
      delete iArray;
      iArray = NULL;
      return(KErrNoMemory);
   }

   iMaxPoints = size;

   return(KErrNone);
}

PointArray::~PointArray()
{
   iArray->Reset();
   delete iArray;
}

// get a specified point from the array as a TPoint
const TPoint& PointArray::getPoint(int index)
{
   return(iArray->At(index));
}

// returns the internal array as a pointer (for use in Symbian GC routines)
const CArrayFixFlat<TPoint>* PointArray::getArray()
{
   return(iArray);
}

/* from PointVector */
void PointArray::addPoint(int x, int y)
{
   TRAPD(err, iArray->AppendL(TPoint(x,y)));
}

// frees all allocated memory for the array
void PointArray::reset()
{
   iArray->Reset();
}


void PointArray::setPoints(const vector<MC2Point>& points)
{
   reset();
   for(vector<MC2Point>::const_iterator it = points.begin();
       it != points.end();
       ++it ) {
      addPoint(it->getX(), it->getY());
   }
}

// gets the number of elements in the array
int PointArray::getSize() const
{
   return(iArray->Count());
}

// gets the number of elements in the array
int PointArray::getAllocatedSize() const
{
   return(iMaxPoints);
}

/* returns the Array which stores the points */
const CArrayFix<TPoint>* PointArray::getPointArray() const
{
   return(iArray);
}

// **************************************************************************

/* filter for averaging */
static const int averageFilter[3][3] = {
	{  1,  2,  1  },
	{  2,  4,  2  },
	{  1,  2,  1  }
};
/* returns the averaging value for the filter */
static int averageFilterValue()
{
	return( averageFilter[0][0] + averageFilter[0][1] + averageFilter[0][2] + 
           averageFilter[1][0] + averageFilter[1][1] + averageFilter[1][2] + 
           averageFilter[2][0] + averageFilter[2][1] + averageFilter[2][2] );
}

/* private constructor */
alphaMap::alphaMap() :
   m_data(NULL),
   m_dataPtr(NULL,0),
   m_width(0), m_height(0),
   m_dataSize(0)
{
}

/* second-phase constructor */
/* returns true on success, false otherwise */
bool alphaMap::create(int32 w, int32 h)
{
   /* calculate the total size */
   m_dataSize = w * h;

   /* allocate the data array */
   m_data = new uint8[m_dataSize];
   if(m_data == NULL) {
      /* could not allocate array */
      return(false);
   }

   /* allocate the scanline offset array */
   m_scanOffset = new int32[h];
   if(m_scanOffset == NULL) {
      /* no memory */
      delete m_data;
      return(false);
   }

   /* set the scanline offsets */
   for(int32 scanY = 0; scanY < h; scanY++) {
      m_scanOffset[scanY] = scanY*w;
   }

   /* set the variables */
   m_width = w;
   m_height = h;
   /* data pointer has to be given size in bytes */
   m_dataPtr.Set((TUint8*)m_data, 
                 m_dataSize,
                 m_dataSize);

   /* success */
   return(true);
}

/* allocates a bitmap */
alphaMap* alphaMap::allocate(int32 w, int32 h)
{
   /* create the object */
   alphaMap* newObj = new alphaMap();
   /* call second-phase constructor */
   if(newObj->create(w, h) != true) {
      /* failure, return NULL */
      delete newObj;
      return(NULL);
   }

   /* success */
   return(newObj);
}

/* allocates a bitmap from the specified CSurface 
   and sets all mask colored pixels to alpha = 0 */
alphaMap* alphaMap::allocate(CSurface& bmp, uint8 r, uint8 g, uint8 b)
{
   /* create the object */
   alphaMap* newObj = new alphaMap();
   /* call second-phase constructor */
   if(newObj->create(bmp.Width(), bmp.Height()) != true) {
      /* failure, return NULL */
      delete newObj;
      return(NULL);
   }

   /* copy the surface's data to the bitmap */
   TRgb srcColor;
   register uint32 index = 0;
   register uint32 srcX, srcY;
   uint32 height = bmp.Height();
   uint32 width = bmp.Width();
   TRgb maskColor( r, g, b );
   for(srcY = 0; srcY < height; ++srcY) {
      for(srcX = 0; srcX < width; ++srcX) {
         /* get the color, extract components and fill the bitmap */
         srcColor = bmp.ReadPixel(srcX, srcY);

         /* if the color is equal to the mask, then set alpha to 0 */
         if ( srcColor == maskColor ) {

            newObj->m_data[index] = MIN_ALPHA;

         }
         /* else set alpha to 255 */
         else {

            newObj->m_data[index] = MAX_ALPHA;
            
         }

         /* increment index */
         ++index;
      }
   }

   /* success */
   return(newObj);
}


// Performs the rotation on a mimimal bitmap.
alphaMap* alphaMap::allocateDoubleAndRotate(CSurface& bmp, 
                                            uint8 r, uint8 g, uint8 b,
                                            float angle)
{
   /* create the object */
   alphaMap* newObj = new alphaMap();

   // Double the height and width since rotation should be done on 
   // a doubled bitmap.

   int width = bmp.Width();
   int height = bmp.Height();
   int doubledWidth = width<<1;
   int doubledHeight = height<<1;

   // The factor to multiply the sin / cos values with to avoid
   // floating point calculations.
#define TRIG_FACTOR 0x8000
   
   /* get the trig ratios */
   int sine = int(SIN(angle) * TRIG_FACTOR);
   int cosine = int(COS(angle) * TRIG_FACTOR);
   
   // The box of the bitmap.
   PixelBox box( MC2Point( 0, 0 ), 
                 MC2Point( doubledWidth - 1, doubledHeight - 1 ) );

   // Now rotate this rectangle to see how large the rotated bitmap
   // has to be.

   // The rotated box.
   PixelBox rotatedBox;
   
   // Rotate each corner and create the rotated box from that.
   for ( int i = 0; i < 4; ++i ) {
      MC2Point corner = box.getCorner( i );
      
      // Move corner to center and rotate.
      MC2Point rotatedPoint( 0, 0 );

      // XXX: When using fixpoint calculations here instead, the
      // resulting boxes became incorrect. Very strange. Worked in
      // the emulator. Anyways, therefore we stick with floating point
      // calculations here.
      rotatedPoint.getX() = int( 
         (float)(corner.getY() - height ) * SIN(angle) +
         (float)(corner.getX() - width  ) * COS(angle) );
      rotatedPoint.getY() = int(
         (float)(corner.getY() - height ) * COS(angle) -
         (float)(corner.getX() - width  ) * SIN(angle) );

      // Update the rotated box.
      rotatedBox.update( rotatedPoint );
   }

   // Add a little extra space to the height and width to compensate for
   // rounding errors.
   int rotHeight = rotatedBox.getHeight() + 2;
   int rotWidth = rotatedBox.getLonDiff()  + 2;
   
   /* call second-phase constructor */
   if(newObj->create( rotWidth, rotHeight ) != true) {
      /* failure, return NULL */
      delete newObj;
      return(NULL);
   }

   // Clear the surface with mask.
   newObj->clear( MIN_ALPHA );
   
   /* copy the surface's data to the bitmap */
   TRgb srcColor;
   register int32 srcX, srcY;

   int rotx, roty;
   
   TRgb maskColor( r, g, b );

   // Rotation width and height multiplied with the TRIG_FACTOR
   // for fixpoint calculations.
   int rotWidthTrig = rotWidth * TRIG_FACTOR;
   int rotHeightTrig = rotHeight * TRIG_FACTOR;   
   
   for(srcY = 0; srcY < height; ++srcY) {
      for(srcX = 0; srcX < width; ++srcX) {
         /* get the color, extract components and fill the bitmap */
         srcColor = bmp.ReadPixel(srcX, srcY);

         // Only do something if the color is not the mask.
         if ( srcColor != maskColor ) {
            uint8 color = MAX_ALPHA;
        
            // Rotate around origo using fixpoint.
            rotx = (2*((srcY - height/2) * sine + 
                      (srcX - width/2) * cosine) + 
                    rotWidthTrig/2 )  / TRIG_FACTOR;
            roty = (2*((srcY - height/2) * cosine - 
                       (srcX - width/2) * sine) + 
                    rotHeightTrig/2 ) / TRIG_FACTOR;

            // The pixel should really be within the bounds so
            // the error checking in setPixel() is not necessary.
            // Didn't seem to improve the speed very much though.
            newObj->setPixel( rotx,       roty,       color );
            newObj->setPixel( rotx + 1,   roty,       color );
            newObj->setPixel( rotx,       roty + 1,   color );
            newObj->setPixel( rotx + 1,   roty + 1,   color );
            
//            newObj->m_data[ newObj->m_scanOffset[roty] + rotx ] = color;
//            newObj->m_data[ newObj->m_scanOffset[roty] + rotx + 1 ] = color;
//            newObj->m_data[ newObj->m_scanOffset[roty + 1] + rotx ] = color;
//            newObj->m_data[ newObj->m_scanOffset[roty + 1] + rotx + 1 ] = color;
         
         }
         
      }
   }

   /* success */
   return(newObj);
}
/* destructor */
alphaMap::~alphaMap()
{
   delete m_scanOffset;
   delete m_data;
}

/* clear to color */
void alphaMap::clear(uint8 alpha)
{
   m_dataPtr.Fill( alpha );
   return;
}

/* sets a pixel */
void alphaMap::setPixel(uint32 x, uint32 y, uint8 alpha)
{
   /* check bounds */
   if( x >= (uint32)m_width || y >= (uint32)m_height) {
      /* pixel is out of bounds */
      return;
   }

   /* set the pixel to the given color */
   m_data[m_scanOffset[y]+x] = alpha;

   return;
}

/* gets the alpha of a pixel */
/* if the pixel is out of bounds, 
   doesnt change the value of the value of the variable. */
void alphaMap::getPixel(uint32 x, uint32 y, uint8& alpha)
{
   /* check bounds */
   if( x >= (uint32)m_width || y >= (uint32)m_height) {
      /* pixel is out of bounds */
      return;
   }

   /* get the pixel's color */
   alpha = m_data[m_scanOffset[y]+x];

   return;
}

/* gets the filtered alpha value of a pixel */
/* if the pixel is out of bounds, 
   doesnt change the value of the variable. */
void alphaMap::getFilteredPixel(uint32 x, uint32 y, uint8& alpha)
{
   /* check bounds */
   if( x >= (uint32)m_width || y >= (uint32)m_height) {
      /* pixel is out of bounds */
      return;
   }

   /* average the surrounding pixels and get the filtered color */
   uint32 sum = 0;
   uint8 readAlpha;
   uint32 avgValue;
   uint32 filterValue = averageFilterValue();
   uint8 nullAlpha = 0xDE;
   int32 dx, dy;

   for(dy = -1; dy <= 1; dy++) {
      for(dx = -1; dx <= 1; dx++) {
         /* set color to null alpha */
         readAlpha = nullAlpha;
         /* get the average filter value */
         avgValue = averageFilter[dy+1][dx+1];
         /* get the Alpha value */
         getPixel(x+dx, y+dy, readAlpha);
         if(readAlpha != nullAlpha) {
            /* add the color components to the sum */
            sum += (readAlpha * avgValue);
         }
         else {
            /* remove the weightage from the filter value */
            filterValue -= avgValue;
         }
      }
   }

   /* check filter value */
   if(filterValue == 0) {
      filterValue = 1;
   }

   /* average the total color values */
   sum = sum / filterValue;

   /* create the color */
   alpha = (uint8)(sum & MAX_ALPHA);

   return;
}

/* gets the array as a TDes */
TDes8& alphaMap::getDataPtr()
{
   return(m_dataPtr);
}

/* get the array */
const uint8* alphaMap::getArray()
{
   return(m_data);
}

/* get the width and height */
int32 alphaMap::getWidth() const
{
   return(m_width);
}

int32 alphaMap::getHeight() const
{
   return(m_height);
}

/* blits the bitmap to a GraphicsContext */
void alphaMap::blitTo(CSurface& outSurface, int32 dX, int32 dY, 
                      uint8 r, uint8 g, uint8 b)
{
   register int32 x, y;
   register uint32 alpha, invAlpha;
   uint32 blendR, blendG, blendB;
   TRgb drawColor(r, g, b), backColor;

   outSurface.Gc().SetPenStyle(CGraphicsContext::ESolidPen);
   outSurface.Gc().SetPenSize(TSize(1,1));

   /* get the blit origin */
   const register int32 origX = dX - (m_width >> 1);
   const register int32 origY = dY - (m_height >> 1);
   /* loop thru the bitmap pixels and blit the non-mask pixels */
   for(y = m_height-1; y != 0; y--) {
      for(x = m_width-1; x != 0; x--) {
         /* get the color and alpha */
         alpha = m_data[m_scanOffset[y]+x];
         invAlpha = MAX_ALPHA - alpha;
         /* ALPHA = 1.0, Opaque Pixel */
         if(alpha == MAX_ALPHA) {
            /* draw the solid pixel */
            outSurface.WritePixel(origX+x, origY+y, drawColor);
         }
         /* ALPHA > 0.0 && ALPHA < 1.0, Translucent Pixel */
         else if( (alpha > MIN_ALPHA) && (alpha < MAX_ALPHA) ) {

            /* get the background color */
            backColor = outSurface.ReadPixel(origX+x, origY+y);

            /* calculate the final blended color */
            blendR = ( (backColor.Red()*invAlpha) + (r*alpha) ) / MAX_ALPHA;
            blendG = ( (backColor.Green()*invAlpha) + (g*alpha) ) / MAX_ALPHA;
            blendB = ( (backColor.Blue()*invAlpha) + (b*alpha) ) / MAX_ALPHA;

            /* draw the solid pixel */
            outSurface.WritePixel(origX+x, origY+y,
                                  TRgb(blendR, blendG, blendB) );  
         }
      }
   }

   return;
}

/* blits the bitmap to another alphaMap and resizes it  */
void alphaMap::resizeBlitTo(alphaMap& dstBmp, int32 dX, int32 dY, 
                            int32 newWidth, int32 newHeight,
                            bool filtered)
{
   /* get the scale factors for the source pixels */
   const float srcXDelta = (float)m_width / (float)newWidth;
   const float srcYDelta = (float)m_height / (float)newHeight;

   /* get the halves of the width and height */
   const int halfDestWidth = newWidth >> 1; 
   const int halfDestHeight = newHeight >> 1; 

   float srcX = 0.0f, srcY = 0.0f;
   register int dstX, dstY;
   register uint8 alpha;
   register int32 finalX, finalY;
   /* loop thru the pixels and do the blit */
   for(dstY = -halfDestHeight; dstY <= halfDestHeight; dstY++) {
      
      for(dstX = -halfDestWidth; dstX <= halfDestWidth; dstX++) {

         finalX = (int32)srcX;
         finalY = (int32)srcY;
         if(finalX == m_width) finalX--;
         if(finalY == m_height) finalY--;

         if(filtered) {
            /* get filtered color */
            getFilteredPixel(finalX, finalY, alpha);
         }
         else {
            /* get normal color */
            alpha = m_data[m_scanOffset[finalY]+finalX];
         }         

         /* plot the point */
         dstBmp.setPixel(dX+dstX, dY+dstY, alpha);

         /* increment y-axis index */
         srcX += srcXDelta;
       
      }

      /* reset x-axis source pixel index */
      srcX = 0.0f;
      /* increment y-axis index */
      srcY += srcYDelta;

   }

   return;
}

/* blits the bitmap to another alphaMap and rotates it  */
void alphaMap::rotateBlitTo(alphaMap& dstBmp, int32 dX, int32 dY, 
                            float angle)
{
 	/* rotate the bitmap pixel-by-pixel */
	register int32 x, y;
	float rotx, roty;

   /* get half width and height */
   const int32 halfW = m_width >> 1;
   const int32 halfH = m_height >> 1;

   /* get the trig ratios */
   float sine = SIN(angle);
   float cosine = COS(angle);

	for(y = -halfH; y < halfH; y++)
	{
		for(x = -halfW; x < halfW; x++)
      {
         uint8 pixelValue = m_data[m_scanOffset[y+halfH]+x+halfW];
         if ( pixelValue > MIN_ALPHA ) {
            /* rotation */
            rotx = (float)y * sine   + (float)x * cosine;
            roty = (float)y * cosine - (float)x * sine;
            /* draw the point */
            dstBmp.setPixel(dX+(int32)rotx, dY+(int32)roty,
                            pixelValue );
         }
      }
   }

   return;
}

uint32
alphaMap::getSizeInMem() const
{
   return m_dataSize + m_height * 4 + sizeof( *this );
}

// **************************************************************************


/* Constructors - Private, so further derivation is not possible */
MapRenderer::MapRenderer(CCoeControl& aParent) :
      iParent(aParent)
{
   iDrawBuffer = NULL;
   iCurFont = NULL;
   iImageConverter = NULL;
   m_maxBufferSwitchedTo = 0;
   m_screenSnapshot = NULL;
}

void MapRenderer::ConstructL(TInt aWidth, TInt aHeight, TInt aColordepth)
{   
   m_colorDepth = aColordepth;
   
   /* create the internal buffer to draw to */
   {
      for ( int i = 0; i < c_nbrDrawBuffers; ++i  ) {
         if ( i != c_secondBufferMask ) {
            m_buffers[i] = CSurface::NewL(iParent.ControlEnv()->WsSession(),
                                          aWidth, aHeight, aColordepth);
         } else {
            // The mask should be 1 bit.
            m_buffers[i] = CSurface::NewL(iParent.ControlEnv()->WsSession(),
                                          aWidth, aHeight, 1 );
         }
      }
   }
   iDrawBuffer = m_buffers[ c_mainBuffer ];   

   
#ifdef HANDLE_SCREEN_AS_BITMAP 
   m_screenSnapshot = CSurface::NewL(iParent.ControlEnv()->WsSession(),
                                     aWidth, aHeight, aColordepth);
#endif
   
   /* create the converter */
   iImageConverter = ImageConverter::allocate();
   User::LeaveIfNull(iImageConverter);

   m_maskBackColor = TRgb( 0,0,0 );
   m_maskForeColor = TRgb( 255,255,255 );
   m_lastFillColor = m_maskBackColor;
   m_lastPenColor  = m_maskForeColor;

   {
      for ( int i = 0; i < c_nbrDrawBuffers; ++i ) {
         /* set some defaults for the bitmap graphics context */
         m_buffers[i]->Gc().SetBrushColor( m_lastFillColor );
         m_buffers[i]->Gc().SetPenColor( m_lastPenColor );
         m_buffers[i]->Gc().SetBrushStyle(CGraphicsContext::ESolidBrush);
         m_buffers[i]->Gc().SetPenStyle(CGraphicsContext::ESolidPen);
      //iDrawBuffer->Gc().SetClippingRect(TRect(0,0,aWidth,aHeight));
         m_buffers[i]->Gc().Clear();
      }
   }
   
   /* set the internal state variables */
   iCurSize = 1;               /* single pixel width */
   m_backColor = m_lastFillColor;     /* Black */
   m_penColor  = m_lastPenColor;      /* White */
   m_fillColor = m_lastFillColor;     /* Black */
   
   setBackgroundColor( 127, 127, 127 );
   clearScreen();
   
   changePenColor( m_penColor );
   changeBrushColor( m_fillColor );
   iDashStyle = solidDash;         /* solid line drawing */
   iCapStyle = flatCap;         /* normal ends to line */

   // get time adjustment value for timing calculations
   uint32 tmpstart = TileMapUtil::currentTimeMillis();
   uint32 tmpend = TileMapUtil::currentTimeMillis();
   m_adjustTimeDelta = tmpend - tmpstart;

   // set last draw time to zero
   m_lastDrawTime = 0;

}

int
MapRenderer::switchBitmap( int nbr, int mask )
{
   if ( ( nbr != 0 ) && mask ) mask = 1;
   if ( nbr > 1 ) nbr = 1;
   // Delete the font from the right device.
   if( iCurFont ) {
#ifdef RELEASE_FONTS
      iDrawBuffer->Gc().DiscardFont();
      iDrawBuffer->Device().ReleaseFont(iCurFont);
#endif
      iCurFont = NULL;
   }

   // Get the right buffer
   if ( nbr == 0 ) {
      iDrawBuffer = m_buffers[ c_mainBuffer ];
   } else {
      int idx = 1 + (nbr-1)*2 + mask;
      iDrawBuffer = m_buffers[ idx ];
      m_maxBufferSwitchedTo = MAX(m_maxBufferSwitchedTo, idx);
   }
   
   // Transfer pen / brush settings
   if ( ! isMask( iDrawBuffer ) ) {
      iDrawBuffer->Gc().SetBrushColor( m_lastFillColor );
   } else {
      iDrawBuffer->Gc().SetBrushColor( m_maskForeColor );
   }
   iDrawBuffer->Gc().SetPenColor( m_lastPenColor );
   iDrawBuffer->Gc().SetPenSize( TSize( iCurSize, iCurSize ) );

   return true;
}

/* destructor */
MapRenderer::~MapRenderer()
{
   /* discard any font if in use - or BITGDI Panic will occur */
   {
      for ( int i = 0; i < c_nbrDrawBuffers; ++i ) {
         m_buffers[i]->Gc().DiscardFont();
      }
   }

   /* delete any allocated fonts */
   if(iCurFont)
   {
      iDrawBuffer->Device().ReleaseFont(iCurFont);
      iCurFont = NULL;
   }

   /* delete the image converter */
   delete iImageConverter;

   /* delete the backbuffers */
   {
      for ( int i = 0; i < c_nbrDrawBuffers; ++i ) {
         delete m_buffers[i];
      }
   }
   iDrawBuffer = NULL;

   // Delete all the cached rotated bmps.
   while ( ! m_rotatedBmps.empty() ) {
      rotBmp_t& bmp = m_rotatedBmps.front();
      // The alloced string.
      delete bmp.first.first;
      // The bitmap.
      delete bmp.second;
      m_rotatedBmps.pop_front();
   }

   delete m_screenSnapshot;
}

/* allocation routines */
/* allocates a new MapRenderer of the specified width, height and colordepth */
MapRenderer* MapRenderer::NewLC(CCoeControl& aParent,
                                TInt aWidth,
                                TInt aHeight,
                                TInt aColordepth)
{
   MapRenderer* tmpobj = new (ELeave) MapRenderer(aParent);
   CleanupStack::PushL(tmpobj);
   tmpobj->ConstructL(aWidth, aHeight, aColordepth);
   return(tmpobj);
}

MapRenderer* MapRenderer::NewL(CCoeControl& aParent,
                               TInt aWidth, TInt aHeight, TInt aColordepth)
{
   MapRenderer* tmpobj = MapRenderer::NewLC(aParent,
                                            aWidth, aHeight, aColordepth);
   CleanupStack::Pop(tmpobj);
   return(tmpobj);
}


/* inherited from MapPlotter */
/* pure virtual functions */


/* sets background color only used for clearing the screen */
void
MapRenderer::setBackgroundColor( unsigned int red,
                                 unsigned int green,
                                 unsigned int blue )
{
   // Set new background color
   if ( ! isMask( iDrawBuffer ) ) {
      m_backColor = TRgb( red, green, blue );
   } else {
      m_backColor = m_maskBackColor;
   }
}

/* sets pen color used for drawing - lines, rectangles, etc. */
void
MapRenderer::setPenColor( unsigned int red,
                          unsigned int green,
                          unsigned int blue )
{
   if ( ! isMask( iDrawBuffer ) ) {
      m_penColor = TRgb( red, green, blue );
   } else {
      m_penColor = m_maskForeColor;
   }
}


void
MapRenderer::setFillColor( unsigned int red,
                           unsigned int green,
                           unsigned int blue )
{
   if ( ! isMask( iDrawBuffer ) ) {
      m_fillColor = TRgb( red, green, blue );
   } else {
      m_fillColor = m_maskForeColor;
   }
}

inline void
MapRenderer::changePenColor( const TRgb& newPen )
{
   // Only change color if really needed
   if ( newPen != m_lastPenColor ) {
      iDrawBuffer->Gc().SetPenColor( newPen );     
      m_lastPenColor = newPen;
   }
}

inline void
MapRenderer::changeBrushColor( const TRgb& newBrush )
{   
   // Only change the color if it is a new one.
   if ( newBrush != m_lastFillColor ) {
      iDrawBuffer->Gc().SetBrushColor( newBrush );
      m_lastFillColor = newBrush;
   }
}


void MapRenderer::setLineWidth( int width )
{
   if ( iCurSize != width ) {
      iCurSize = width;
      iDrawBuffer->Gc().SetPenSize(TSize(width,width));
   }
}

/* clears the screen to the current background color */
void MapRenderer::clearScreen()
{
   if ( ! isMask( iDrawBuffer ) ) {
      changeBrushColor( m_backColor );
      iDrawBuffer->Gc().Clear();
   } else {
      iDrawBuffer->Gc().SetBrushColor( m_maskBackColor );
      iDrawBuffer->Gc().Clear();
      iDrawBuffer->Gc().SetBrushColor( m_maskForeColor );
   }
   
   if ( m_screenSnapshot ) {
      m_screenSnapshot->Gc().SetBrushColor( m_backColor );
   }

   return;
}

/* draws a polyline */
void MapRenderer::drawPolyLine( vector<MC2Point>::const_iterator begin,
                                vector<MC2Point>::const_iterator end )
{
   changePenColor( m_penColor );
   iDrawBuffer->Gc().DrawPolyLine(&*begin, end - begin );
}

void
MapRenderer::drawPolyLineWithColor( vector<MC2Point>::const_iterator begin,
                                    vector<MC2Point>::const_iterator end,
                                    uint32 color,
                                    int lineWidth )
{
   MapRenderer::setLineWidth( lineWidth );
   // Set pen color if needed.  
   m_penColor = TRgb( uint8( color >> 16 ),
                      uint8( color >>  8 ),
                      uint8( color ) );
   
   MapRenderer::drawPolyLine( begin, end );
}


void MapRenderer::drawPolygon( vector<MC2Point>::const_iterator begin,
                               vector<MC2Point>::const_iterator end )
{
   changePenColor( m_fillColor );
   changeBrushColor( m_fillColor );
   
   const MC2Point* arrPoints = &(*begin);

   iDrawBuffer->Gc().DrawPolygon(arrPoints, end - begin,
                                 CGraphicsContext::EWinding );
}

void MapRenderer::
drawPolygonWithColor( vector<MC2Point>::const_iterator begin,
                      vector<MC2Point>::const_iterator end,
                      uint32 fillColor )
{
   m_fillColor = TRgb( uint8(fillColor >> 16),
                       uint8(fillColor >> 8),
                       uint8(fillColor ) );
   MapRenderer::drawPolygon( begin, end );
}


void
MapRenderer::getBitMapAsRectangle(Rectangle& size,
                                  const MC2Point& origXY,
                                  const BitMap* bmp)
{
   const Image* img = static_cast<const Image*>(bmp);
   PixelBox imgBox = TRect(img->GetSize());
   PixelBox imgVisibleBox = img->GetVisibleRect();
   imgBox.moveCenterTo(origXY);
   //Get the correct alignment of the visible part of the image.
   imgVisibleBox.moveTopLeftTo(imgBox.getTopLeft() + imgVisibleBox.getTopLeft());
   size = imgVisibleBox;
   return;
}

/* returns the size of the internal drawing buffer */
void MapRenderer::getMapSizePixels(Rectangle& size) const
{
   size = Rectangle(0,0,
                    iDrawBuffer->Bitmap().SizeInPixels().iWidth,
                    iDrawBuffer->Bitmap().SizeInPixels().iHeight);
   return;
}

/* returns a created PointVector of the specified size */
PointVector* MapRenderer::createPointVector(int size)
{
   PointArray* tmpobj = new PointArray();
   if(tmpobj == NULL) return(NULL);

   /* do second phase construction with the given size */
   if(tmpobj->Construct(size) != KErrNone)
   {
      delete tmpobj;
      return(NULL);
   }

   return(tmpobj);
}

/* deletes a PointVector and sets pointer to NULL */
void MapRenderer::deletePointVector(PointVector* vect)
{
   if(vect) delete vect;
   vect = NULL;
   return;
}

void
MapRenderer::setFont( CFbsFont* theFont )
{
   iCurFont = theFont;
   /* set the specified font as the default font */
   iDrawBuffer->Gc().UseFont(iCurFont);
   
}

int
MapRenderer::getLogicalFont(const STRING& fontName, TFontSpec& fontSpec)
{
#if defined NAV2_CLIENT_SERIES60_V3
   int fontId;
   if (fontName == KPrimaryLogicalFont()) {
      fontId = EAknLogicalFontPrimaryFont;
   } else if (fontName == KPrimarySmallLogicalFont) {
      fontId = EAknLogicalFontPrimarySmallFont;
   } else if (fontName == KSecondaryLogicalFont){
      fontId = EAknLogicalFontSecondaryFont;
   } else if (fontName == KTitleLogicalFont) {
      fontId = EAknLogicalFontTitleFont;
   } else if (fontName == KDigitalLogicalFont) {
      fontId = EAknLogicalFontDigitalFont;
   } else {
      return 0;
   }
   const CFbsFont* font = dynamic_cast <const CFbsFont *>(AknLayoutUtils::
                                                          FontFromId(fontId));
   if (font && font->IsOpenFont()) {
      TOpenFontFaceAttrib fontAttrib;
      font->GetFaceAttrib(fontAttrib);
      fontSpec = TFontSpec(fontAttrib.Name(), font->HeightInPixels());
//       fontNotice = FontNotice(&fontAttrib.Name(), font->HeightInPixels());
//       return &FontNotice(&fontAttrib.Name(), font->HeightInPixels());
      return 1;
   } else {      
      return 0;
   }
#else
   return 0;
#endif
}

/* virtual functions */

#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES60_V3

/* requests a font from the Font Server using the specified typeface and height */
/* if no font is set, text-drawing will not happen */
void MapRenderer::setFont( const STRING& fontName, int size )
{
   FontNotice notice( (STRING*)&fontName, size );
   fontCache_t::iterator findit = m_fontCache.find( notice );
   if ( findit != m_fontCache.end() ) {
      // Found the font.
      setFont( (*findit).second );
   } else {
      // Font not found.
      // Create the font.
      TFontSpec fontspec(fontName, size);
      CFbsFont* theFont = NULL;
      iDrawBuffer->Device().GetNearestFontInPixels(theFont,
                                                   fontspec);
      MC2_ASSERT( theFont != NULL );

      // Add the font to the font cache.
      FontNotice noticeToStore( fontName.Alloc(), size );
      m_fontCache[ noticeToStore ] = theFont;
      setFont( theFont );
   } 
}

#else 
// Everything else than s80 or s90.
void MapRenderer::setFont( const STRING& fontName, int size )
{
   
   /* create a TFontSpec to match the font specs */
   TFontSpec fontspec(fontName, size);
   /* release any allocated fonts if we want a new one now */
   if( iCurFont ) {
      // We have a font
      if ( ! ( fontspec == m_lastFontSet ) ) {
         // And it is a new one.
#ifdef RELEASE_FONTS
         iDrawBuffer->Gc().DiscardFont();
         iDrawBuffer->Device().ReleaseFont(iCurFont);
#endif         
         iCurFont = NULL;
      }
   }

   // Check if the font became NULL or was null already.
   if ( iCurFont == NULL ) {
      m_lastFontSet = fontspec;
      /* request the font from the font server */
      iDrawBuffer->Device().GetNearestFontInPixels(iCurFont,
                                                   fontspec);
      MC2_ASSERT( iCurFont != NULL );
      
      /* set the specified font as the default font */
      iDrawBuffer->Gc().UseFont(iCurFont);
   }
   return;
}
#endif
/* returns the bounding rectangle for each charcter of a string as a vector */
int MapRenderer::getStringAsRectangles( vector<Rectangle>& boxes,
                                        const STRING& text,
                                        const MC2Point& point,
                                        int startIdx,
                                        int nbrChars,
                                        float angle )
{
   /* return quietly if no font is set */
   if(iCurFont == NULL) return(0);

   int i;
   
   /* Loop thru the text and set the bounding rectangle for each character 
      Each rectangle holds absolute coords for the begininning of 
      each character and their width and height */

   if(nbrChars == -1) {
      nbrChars = text.Length();
   }

   int textH = iCurFont->HeightInPixels();
   int halfTextH = textH >> 1;

   /* number of characters to be printed */
   int numChars = nbrChars - startIdx;
   int maxTextLen = iCurFont->TextWidthInPixels(text);
   int halfTextLen = maxTextLen >> 1;

   if(angle == 0.0) {
      /* get the center points of all the characters */
      RArray<MC2Point> origArray;

      int startX = point.getX() - halfTextLen;
      int textW;

      for(i = 0; i < numChars; i++) {
         textW = iCurFont->CharWidthInPixels(text[startIdx+i]);
         origArray.Append(MC2Point(startX + (textW >> 1),
                                   point.getY()));
         startX += textW;
      }

      /* get the rectangles */
      int curOrig = 0;
      for(i = startIdx; i < nbrChars; i++) {

         textW = iCurFont->CharWidthInPixels(text[startIdx+i]);
         boxes.push_back(Rectangle(origArray[curOrig].getX()-(textW>>1),
                                   origArray[curOrig].getY()-halfTextH,
                                   textW, textH));
         curOrig++;

      }
      origArray.Reset();
   }
   else {
      /* get the center points of all the characters */
      RArray<MC2Point> origArray;

      int startX = point.getX() - halfTextLen;
      int textW;

      for(i = 0; i < numChars; i++) {
         textW = iCurFont->CharWidthInPixels(text[startIdx+i]);

         /* get the relative coord for the center */
         float origX = (startX + (textW >> 1)) - point.getX();
         float origY = 0;

         /* get the rotated center */
         float rotOrigX, rotOrigY;
         rotatePoint(origX, origY, angle, &rotOrigX, &rotOrigY);
         rotOrigX += point.getX();
         rotOrigY += point.getY();

         origArray.Append(MC2Point((int)rotOrigX,
                                   (int)rotOrigY));
         startX += textW;
      }

      /* get the rectangles */
      int curOrig = 0;
      for(i = startIdx; i < nbrChars; i++) {
         textW = iCurFont->CharWidthInPixels(text[startIdx+i]);

         Rectangle rotBox = getRotatedBBox(MC2Point(origArray[curOrig].getX(),
                                           origArray[curOrig].getY()),
                                           textW, textH, angle);

         boxes.push_back(rotBox);
         curOrig++;
      }
      origArray.Reset();
   }

   /* returns the number of characters processed */
   return(numChars);
}

/* returns the size of the whole string as a rectangle */
Rectangle MapRenderer::getStringAsRectangle( const STRING& text,
                                             const MC2Point& point,
                                             int startIdx,
                                             int nbrChars,
                                             float angle  )
{
   /* return quietly with a null rectangle, if no font is set */
   if(iCurFont == NULL) return(Rectangle(0,0,0,0));
   /* get the width and height of the string */
   if(nbrChars == -1) {
      nbrChars = text.Length();
   }
   int textWidth = iCurFont->TextWidthInPixels(text.Mid(startIdx, nbrChars));
   int halfWidth = textWidth >> 1;
   int textHeight = iCurFont->HeightInPixels();
   int halfHeight = textHeight >> 1;

   /* get the basic text bounding box */
   Rectangle baseRect(point.getX() - halfWidth, 
                      point.getY() - halfHeight,
                      textWidth, textHeight);

   if(angle == 0.0) {
      // zero angle
      return(baseRect);
   }
   else {
      /* return the rotated bounding box */
      return(getRotatedBBox(point, baseRect.getWidth(), 
                            baseRect.getHeight(), angle));
   }
}

/* returns 1 degree */
int MapRenderer::getMinTextRotation()
{
   return(1);
}

alphaMap* 
MapRenderer::createRotatedBmp( STRING& text, float angle, 
                               int textWidth, int textHeight )
{
      /* create the bitmap */
      int errCode;
      CSurface* strBmp = NULL;
            
      /* create the main text bitmap */
      TRAP(errCode, strBmp = CSurface::NewL(iParent.ControlEnv()->WsSession(),  
                                            textWidth, 
                                            textHeight,
                                            m_colorDepth));
      if(errCode !=  KErrNone) {
         // text bitmap could not be created, return
         return NULL;
      }

      /* various colors */
      uint8 maskR = 255;
      uint8 maskG = 0;
      uint8 maskB = 255;

      /* clear the bitmap to mask color */
      strBmp->Clear(maskR, maskG, maskB);

      // Create a font.

      CFbsFont* theFont = NULL;
#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES60_V3
      theFont = iCurFont;
#else      
      strBmp->Device().GetNearestFontInPixels( theFont, m_lastFontSet );
#endif      
      MC2_ASSERT( theFont != NULL );
      
      // Don't reboot the phone if the font server is exhausted.
      if ( theFont == NULL ) {
         return NULL;
      }
      
      /* write the text to the bitmap */
      strBmp->Gc().UseFont(theFont);
      strBmp->Gc().SetPenColor( m_penColor );
      strBmp->Gc().SetPenStyle(CGraphicsContext::ESolidPen);
      strBmp->Gc().DrawText(text, 
                              TPoint(1,textHeight-theFont->DescentInPixels()));
#ifdef RELEASE_FONTS
      strBmp->Gc().DiscardFont();
      strBmp->Device().ReleaseFont(theFont);
#endif

      // Allocate a doubled and rotated bitmap of the text.
      alphaMap* rotBmp = alphaMap::allocateDoubleAndRotate(
            *strBmp, maskR, maskG, maskB, angle );
      if(rotBmp == NULL) {
         /* no memory */
         return NULL;
      }
      
      /* delete the surface as not required anymore */
      delete strBmp; 
      strBmp = NULL;

      // New size is half the old size.
      alphaMap* finalBmp = alphaMap::allocate( rotBmp->getWidth() / 2,
                                               rotBmp->getHeight() / 2 );
      if(finalBmp == NULL) {
         /* no memory */
         delete rotBmp;
         return NULL;
      }
      
      /* filter down to final bitmap */
      rotBmp->resizeBlitTo(*finalBmp, 
                           finalBmp->getWidth() >> 1,
                           finalBmp->getHeight() >> 1,
                           finalBmp->getWidth(),
                           finalBmp->getHeight(),
                           true );
      delete rotBmp;

      return finalBmp;
}


/* draws text */
/* if no font is set, text-drawing will not happen */
void MapRenderer::drawText( const STRING& text,
                            const MC2Point& point,
                            int startIdx,
                            int nbrChars,
                            float angle )
{
   /* if no font is specified, exit quietly */
   if(iCurFont == NULL) return;
   
   /* if nbrChars > max string length, the set it to the same */
   if(nbrChars > text.Length() || nbrChars <= 0) {
      nbrChars = text.Length();
   }

   // Get a pointer to the text to draw.
   TPtrC textToDraw = text.Mid(startIdx, nbrChars);
   
   /* get basic info */
   int textWidth = iCurFont->TextWidthInPixels(textToDraw);
   int textHeight = iCurFont->HeightInPixels()+iCurFont->DescentInPixels();

   changePenColor( m_penColor );
   /* if angle == 0.0, draw horizontal text using normal GC methods */
   if( (int)angle == 0)
   {
      //MC2Point adjPoint(point.getX() - (textWidth >> 1),
      //                  point.getY() + (textHeight >> 1));
      Rectangle rect ( getStringAsRectangle( text, point,
                                             startIdx, nbrChars,
                                             0 ) );
      MC2Point adjPoint( rect.getX(),
                         rect.getY() + rect.getHeight() );
      /* draw the string */
      changePenColor( m_penColor );
      iDrawBuffer->Gc().DrawText(textToDraw,
                                 adjPoint);
   }
   /* draw angular text */
   else {

//#define MEASURE_TIME      
#ifdef MEASURE_TIME      
      // XXX: Keep track of the elapsed time.
      uint32 startTime = TileMapUtil::currentTimeMillis();
#endif
   
      // Try to use a cached rotated string.
      alphaMap* finalBmp = NULL;

      bool usedCacheBmp = false;
      // Create the string and angle notice, to be used to search
      // for a an existing cached bmp.
      // Don't forget to delete the new:ed string in the notice.
      strAngle_t strAngleNotice( textToDraw.Alloc(), (int) angle );
     
      // Find a cached bmp.
      list< rotBmp_t >::iterator it = m_rotatedBmps.begin();
      while ( it != m_rotatedBmps.end() ) {
         strAngle_t& otherStrAngle = (*it).first;
         if ( ( *otherStrAngle.first == *strAngleNotice.first ) && 
              ( otherStrAngle.second == strAngleNotice.second ) ) {
            break;
         }
         ++it;
      }
      
      if ( it != m_rotatedBmps.end() ) {
         // Found cached.
         finalBmp = (*it).second;
         usedCacheBmp = true;
         // Delete the string since we're done with it.
         delete strAngleNotice.first;
         strAngleNotice.first = NULL;
      } else {
         // Couldn't find a cached one. 
         // Create now instead.
         finalBmp = createRotatedBmp( textToDraw, angle, 
                                      textWidth, textHeight );
      }
      
      if ( finalBmp == NULL ) {
         // No mem.
         delete strAngleNotice.first;
         return;
      }

      uint8 textR = m_penColor.Red();
      uint8 textG = m_penColor.Green();
      uint8 textB = m_penColor.Blue();
      
      /* blit the final bitmap to the screen */
      finalBmp->blitTo(*iDrawBuffer,
                       point.getX(),
                       point.getY(),
                       textR, textG, textB );

      // Calculate how much memory that can be used for caching 
      // old rotated bitmap notices.      
      uint32 maxCacheSize = 0;
      // Get the available memory.
      int availMem = 0;
#if defined SYMBIAN_9
   // HAL is not present and  User::Available seems to cause crashes.
   // Skip the caching of rotated bitmaps.
   availMem = 0;
#else
   HAL::Get(HALData::EMemoryRAMFree, availMem );
#endif
      // The memory that can be used for the cache.
      availMem -= c_minMemFree; 
      if ( availMem > 0 ) {
         maxCacheSize = MIN( availMem, 
                             c_maxCacheBmpSize );
      }

      if ( ! usedCacheBmp ) {
         // Didn't used a cached bmp.
         // Cache this one.
         // Add this one first.
         m_rotatedBmps.push_front( rotBmp_t( strAngleNotice, finalBmp ) );


         // Remove cached bmps as long as necessary.
         
         // Go through the list and stop the iterator when the 
         // amount of memory used has exceeded the allowed amount.
         // Then remove the rest of the list.

         uint32 cachedSize = 0;

         list< rotBmp_t >::iterator it = m_rotatedBmps.begin();
         while ( it != m_rotatedBmps.end() ) {
            cachedSize += (*it).second->getSizeInMem();
            if ( cachedSize > maxCacheSize ) {
               // Exit loop.
               break;
            } else {
               // Continue with next.
               ++it;
            }
         }
        
         // Delete data in the elements from it to the end.
         for ( list< rotBmp_t >::iterator removeIt = it; 
               removeIt != m_rotatedBmps.end(); ++removeIt ) {
               // The alloced string.
               delete (*removeIt).first.first;
               // The bitmap.
               delete (*removeIt).second;
         }
         // Remove from list also.
         m_rotatedBmps.erase( it, m_rotatedBmps.end() );
         
      }
      
      /* reset the color to the pen color */
      iDrawBuffer->Gc().SetPenColor(m_penColor);
      iDrawBuffer->Gc().SetPenSize(TSize(iCurSize,iCurSize));      

#ifdef MEASURE_TIME
      // XXX: Write the elapsed time.
      uint32 stopTime = TileMapUtil::currentTimeMillis();
      uint32 elapsedTime = stopTime - startTime;
      TBuf<80> str;
      str.Format(_L("Tot %u"), elapsedTime ); 
      iDrawBuffer->Gc().SetPenColor(TRgb(255,0,0));
      iDrawBuffer->Gc().UseFont(CEikonEnv::Static()->DenseFont());
      iDrawBuffer->Gc().DrawText(str, TPoint(5,70));
      str.Format(_L("C %u"), maxCacheSize); 
      iDrawBuffer->Gc().DrawText(str, TPoint(5,90));
      iDrawBuffer->Gc().SetPenColor(m_penColor);
#endif       
   }
   return;
}

bool MapRenderer::returnsVisibleSizeOfBitmaps() const
{
   return(true);
}

/* returns true, as double-buffering is implemented using CSurface */
bool MapRenderer::doubleBuffered() const
{
   return(true);
}

/* returns true */
/* The BackBuffer is blitted independently, but this returns true */
bool MapRenderer::drawBuffer( const Rectangle& /*rect*/ )
{
   return(true);
}

void MapRenderer::prepareDrawing()
{
   logStart = TileMapUtil::currentTimeMillis();
   return;
}

void MapRenderer::drawingCompleted()
{
#if 0
   // Try drawing something on the second buffer and mask
   m_buffers[ c_secondBuffer ]->Gc().SetPenColor( TRgb( 255, 0, 0 ) );
   m_buffers[ c_secondBuffer ]->Gc().DrawRect( TRect( 50, 50, 80, 80 ) );
   m_buffers[ c_secondBufferMask ]->Gc().SetPenColor( m_maskForeColor );
   m_buffers[ c_secondBufferMask ]->Gc().DrawRect( TRect( 50, 50, 80, 80 ) );
#endif
#if 0
   logEnd = TileMapUtil::currentTimeMillis();
   m_lastDrawTime = logEnd - logStart - m_adjustTimeDelta;

   iDrawBuffer->Gc().SetPenColor(TRgb(255,0,0));

   TBuf<32> str;
   str.Format(_L("Draw Time : %u MS"), m_lastDrawTime);
   iDrawBuffer->Gc().SetPenColor(TRgb(255,0,0));
   iDrawBuffer->Gc().UseFont(CEikonEnv::Static()->DenseFont());
   iDrawBuffer->Gc().DrawText(str, TPoint(5,60));

//   iDrawBuffer->Gc().SetPenColor(iForeColor);
#endif

#if 0   
   // Delete the font from the right device.
   if( iCurFont ) {
#ifdef RELEASE_FONTS
      iDrawBuffer->Gc().DiscardFont();
      iDrawBuffer->Device().ReleaseFont(iCurFont);
#endif
      iCurFont = NULL;
   }
#endif   

#if defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3
   iParent.DrawDeferred();
/*    iParent.DrawNow(); */
#else
//   if( iParent.IsReadyToDraw() ) {
   if( iParent.IsFocused() ) {
      iParent.DrawDeferred();
/*       iParent.DrawNow(); */
   }
#endif

}

/* returns NULL, if bitmap can't be loaded */
BitMap*
MapRenderer::createBitMap(isab::bitMapType /*imgType*/,
                          const unsigned char* bytes,
                          int nbrBytes,
                          int dpiCorrectionFactor )
{
   /* create the object */
   Image* tmpobj = Image::allocate();
   /* if no memory, return NULL */
   if(tmpobj == NULL) return(NULL);

   /* try to load a bitmap */
   if( iImageConverter->loadImage(bytes, nbrBytes, 
                                  tmpobj, dpiCorrectionFactor) != KErrNone)
   {
      delete tmpobj;
      // Never request this image again!!
      return Image::allocate();
   }
   /* return the created object */
   return(tmpobj);
}

void MapRenderer::drawBitMap( const MC2Point& center,
                              const BitMap* bitMap)
{
   const Image* aImg = static_cast<const Image*>(bitMap);
        
   /* check if the image is ready */
   if(aImg->IsReady()) {
      /* check if it is masked */
      if(aImg->HasMask()) {
         /* do masked blit */
         PixelBox imgBox = TRect(aImg->GetSize());
         PixelBox imgVisibleBox = aImg->GetVisibleRect();
         imgBox.moveCenterTo(center);
         //Get the correct alignment of the visible part of the image.
         imgVisibleBox.moveTopLeftTo(imgBox.getTopLeft() + imgVisibleBox.getTopLeft());
         iDrawBuffer->Gc().SetBrushStyle(CGraphicsContext::ENullBrush);
         if ( ! isMask( iDrawBuffer ) ) {
            iDrawBuffer->Gc().BitBltMasked(TPoint(imgBox.getTopLeft()),
                                           aImg->GetBitmap(), 
                                           TRect(aImg->GetSize()), 
                                           aImg->GetMask(),
                                           EFalse);
         } else {
            // Blit the masked mask on the mask.
            iDrawBuffer->Gc().BitBltMasked(TPoint(imgBox.getTopLeft()),
                                           aImg->GetMask(), 
                                           TRect(aImg->GetSize()), 
                                           aImg->GetMask(),
                                           EFalse);
         }
         iDrawBuffer->Gc().SetBrushStyle(CGraphicsContext::ESolidBrush);
      }
      else {
         /* do normal blit */
         PixelBox imgVisibleBox = aImg->GetVisibleRect();
         imgVisibleBox.moveCenterTo(center);
         if ( ! isMask( iDrawBuffer ) ) {
            iDrawBuffer->Gc().BitBlt(TPoint(imgVisibleBox.getTopLeft()), 
                                     aImg->GetBitmap());
         } else {
            iDrawBuffer->Gc().DrawRect(TRect(TPoint(imgVisibleBox.getTopLeft()), 
                                             aImg->GetSize()));
            // Draw a rectangle as large as the bitmap.
         }
      }
      //Debug information that draws a rect around each POI image.
//       setPenColor(0,0,0);
//       setLineWidth(1);
//       Rectangle bmprect;
//       getBitMapAsRectangle(bmprect, center, bitMap);
//       drawRect(false, bmprect);
   }
   //#endif
}

void MapRenderer::deleteBitMap( BitMap* bitMap )
{
   if(bitMap) {
      delete static_cast<Image*>(bitMap); 
      bitMap = NULL;
   }
}

/*
   Currently draws a closed Bezier spline. The drawn curve is an approximation, thereby 
   passing only thru the first and last control points. The algorithm uses floating-point
   math so it will be slow on the target device, as there is no FPU.
*/
void MapRenderer::drawSpline( bool /*filled*/,
                              vector<MC2Point>::const_iterator begin,
                              vector<MC2Point>::const_iterator end)
{   
   // FIXME: Inefficient
   vector<MC2Point> points;
   points.insert(points.end(), begin, end);
   PointVector* pv = createPointVector(points.size());
   pv->setPoints(points);
   
   PointArray* pntVec = (PointArray*)pv;

   // FIX:
   int endIdx = points.size() - 1;
   int startIdx  = 0;
   int numPoints=0;
   if(endIdx == -1 && startIdx == 0)
   {
      numPoints = pntVec->getSize()-1;
      endIdx = numPoints;
   }
   else if(endIdx != -1 && startIdx == 0)
   {
      numPoints = endIdx;
   }
   else if(endIdx == -1 && startIdx != 0)
   {
      numPoints = pntVec->getSize() - startIdx;
      endIdx = numPoints;
   }
   else if(endIdx != -1 && startIdx != 0)
   {
      numPoints = endIdx - startIdx + 1;
   }
   
   float mu;
   int k,kn,nn,nkn;
   float blend,muk,munk;
   float destx, desty;
   TPoint tmppoint;
   TReal tmpresult;

   #define SPLINESTEP 0.001f
   for(mu = 0.0f; mu <= 1.0; mu += SPLINESTEP)
   {
      muk = 1.0;
      Math::Pow(tmpresult, TReal(1.0-mu),TReal((float)numPoints));
      munk = tmpresult;
      destx = 0.0;
      desty = 0.0;
      for(k = startIdx; k <= endIdx; k++)
      {
         nn = numPoints;
         kn = k;
         nkn = numPoints - k;
         blend = muk * munk;
         muk *= mu;
         munk /= (1-mu);
         while (nn >= 1)
         {
            blend *= (float)nn;
            nn--;
            if (kn > 1)
            {
               blend /= (float)kn;
               kn--;
            }
            if (nkn > 1)
            {
               blend /= (float)nkn;
               nkn--;
            }
         }
         tmppoint = pntVec->getPoint(k);
         destx += (float)tmppoint.iX * blend;
         desty += (float)tmppoint.iY * blend;
      }
      drawPoint((int)destx, (int)desty);
   }

   return;
}

/* draws an arc */
void MapRenderer::drawArc( bool filled, const Rectangle& rect,
                           int startAngle, int stopAngle)
{
   /* convert angles to TPoints for use in Symbian specific method */
   TReal result;
   int xradius = rect.getWidth() / 2;
   int yradius = rect.getHeight() / 2;
   int xcentre = rect.getX() + xradius;
   int ycentre = rect.getY() + yradius;

   result = SIN(startAngle);
   int startx = (int)((float)xcentre + ((float)xradius*(float)result));
   result = COS(startAngle);
   int starty = (int)((float)ycentre + ((float)yradius*(float)result));

   result = SIN(stopAngle);
   int endx = (int)((float)xcentre + ((float)xradius*(float)result));
   result = COS(stopAngle);
   int endy = (int)((float)ycentre + ((float)yradius*(float)result));
   
   /* create Symbian TRect from Rectangle */
   TRect drawrect(rect.getX(),
                  rect.getY(),
                  rect.getX() + rect.getWidth(),
                  rect.getY() + rect.getHeight());

   if(filled)
   {
      changePenColor( m_penColor );
      /* set brush to fill color to allow filling */
      changeBrushColor( m_fillColor );
      iDrawBuffer->Gc().DrawPie(drawrect,
                                TPoint(startx,starty),
                                TPoint(endx,endy));
   }
   else
   {
      changePenColor( m_penColor );
      iDrawBuffer->Gc().DrawArc(drawrect,
                                TPoint(startx,starty),
                                TPoint(endx,endy));
   }
}

void MapRenderer::drawRect( bool filled, const Rectangle& rect, int /*cornerSize*/ )
{
   if(filled)   /* draw a filled rectangle */
   {
      changePenColor( m_penColor );
      changeBrushColor( m_fillColor );
      iDrawBuffer->Gc().DrawRect(TRect(rect.getX(),
                                       rect.getY(),
                                       rect.getX()+rect.getWidth(),
                                       rect.getY()+rect.getHeight()));
   }
   else      /* draw a rectangle outline */
   {
      changePenColor( m_penColor );
      TPoint topleft(rect.getX(), rect.getY());
      TPoint topright(rect.getX()+rect.getWidth(), rect.getY());
      TPoint bottomleft(rect.getX(), rect.getY()+rect.getHeight());
      TPoint bottomright(rect.getX()+rect.getWidth(),
                         rect.getY()+rect.getHeight());
      /* draw lines connecting vertices */
      iDrawBuffer->Gc().DrawLine(topleft, topright);
      iDrawBuffer->Gc().DrawLine(topright, bottomright);
      iDrawBuffer->Gc().DrawLine(bottomright, bottomleft);
      iDrawBuffer->Gc().DrawLine(bottomleft, topleft);
   }
   return;
}

void MapRenderer::setCapStyle( capStyle cStyle )
{
   iCapStyle = cStyle;
   return;
}

void MapRenderer::setDashStyle( dashStyle dStyle )
{
   switch(dStyle)
   {
      case(nullDash):
      {
         iDashStyle = dStyle;
         iDrawBuffer->Gc().SetPenStyle(CGraphicsContext::ENullPen);
         break;
      }
      case(solidDash):
      {
         iDashStyle = dStyle;
         iDrawBuffer->Gc().SetPenStyle(CGraphicsContext::ESolidPen);
         break;
      }
      case(dottedDash):
      {
         iDashStyle = dStyle;
         iDrawBuffer->Gc().SetPenStyle(CGraphicsContext::EDottedPen);
         break;
      }
      case(dashedDash):
      {
         iDashStyle = dStyle;
         iDrawBuffer->Gc().SetPenStyle(CGraphicsContext::EDashedPen);
         break;
      }
      default:
         break;
   }
   return;
}

/* Added by Varun */
void MapRenderer::drawPoint(int aX, int aY)
{
   changePenColor( m_penColor );
   iDrawBuffer->Gc().Plot(TPoint(aX,aY));
   return;
}

void MapRenderer::drawLine(int aSx, int aSy, int aEx, int aEy)
{
   changePenColor( m_penColor );
   iDrawBuffer->Gc().DrawLine(TPoint(aSx,aSy), TPoint(aEx,aEy));
   /* draw the cap style ends to the line */
   switch(iCapStyle)
   {
      case(flatCap):      /* draw normal ends, do nothing */
      {
         break;
      }
      case(squareCap):   /* draw square ends */
      {
         break;
      }
      case(roundCap):      /* draw round ends */
      {
         break;
      }
      break;
   }
   return;
}

void MapRenderer::drawCharacter(const CHARACTER textchar, int aX, int aY)
{
   changePenColor( m_penColor );
   /* create temporary 16-bit text array */
   unsigned short txt[2];
   txt[0] = textchar;
   txt[1] = 0;

   /* copy it to a descriptor */
   TBuf<4> str;
   str.Copy(txt);

   /* draw it */
   iDrawBuffer->Gc().DrawText(str, TPoint(aX, aY));

   return;
}

/* Returns a heap-allocated string in the Native Character format */
STRING* MapRenderer::createString(const char* text)
{
   if(text == NULL){
      return(NULL);
   }
   /* convert the UTF-8 string to Unicode */
   TBuf<256> uniBuf;
   const int length = strlen(text);
   MapUtility::ConvertUTF8ToUnicode((const unsigned char*)text,
                                    uniBuf, length);
   /* allocate a heap descriptor from the buffer and return it */
   return((STRING*)uniBuf.Alloc());
}

/* Frees a heap-allocated string */
void MapRenderer::deleteString(STRING* text)
{
   delete (static_cast<HBufC16*>(text));
   text = NULL;
   return;
}

int
MapRenderer::getStringLength( const STRING& text ) 
{
   return text.Length();
}

/* 
 * Resizes the MapRenderer to the specified size.
 * Returns true if the resize worked, false on an error 
 */
bool MapRenderer::resizeBuffer(uint32 newXSize, uint32 newYSize) 
{
   bool retVal = true;
   for ( int i = 0; i < c_nbrDrawBuffers; ++i ) {
      retVal = m_buffers[i]->Resize(newXSize, newYSize) && retVal;
      if ( i != 0 ) {
         m_buffers[i]->Gc().SetBrushColor( m_maskBackColor );
         m_buffers[i]->Gc().Clear();
      }
   }
   if ( m_screenSnapshot ) {
      retVal = m_screenSnapshot->Resize(newXSize, newYSize) && retVal;
   }
   return retVal;
}


/*****************
  Private Methods 
******************/

/* returns a rotated point */
void MapRenderer::rotatePoint(float inX, float inY, 
                 float angle, 
                 float* outX, float* outY) {

   // get the trig ratios
   register float sine = SIN(angle);
   register float cosine = COS(angle);

   // calculate the points
   *outX = inY * sine   + inX * cosine;
   *outY = inY * cosine - inX * sine;

   return;
}

/* returns the bounds of a rotated rectangle */
Rectangle MapRenderer::getRotatedBBox(const MC2Point& center,
                                      int width, int height,
                                      float angle) {

   // get the corner vertices
   int halfW = width >> 1;
   int halfH = height >> 1;
   float x1 = -halfW; float y1 = -halfH; 
   float x2 = halfW; float y2 = -halfH; 
   float x3 = halfW; float y3 = halfH; 
   float x4 = -halfW; float y4 = halfH; 

   // rotate the points
   float newx[4], newy[4];
   rotatePoint(x1, y1, angle, &newx[0], &newy[0]);
   rotatePoint(x2, y2, angle, &newx[1], &newy[1]);
   rotatePoint(x3, y3, angle, &newx[2], &newy[2]);
   rotatePoint(x4, y4, angle, &newx[3], &newy[3]);   

   // shift the rotated coords to the center
   for(int i = 0; i < 4; i++) {
      newx[i] += center.getX();
      newy[i] += center.getY();
   }

   // get the max and min of the coords
   float xMax = MAX(MAX(newx[0],newx[1]),MAX(newx[2],newx[3]));
   float yMax = MAX(MAX(newy[0],newy[1]),MAX(newy[2],newy[3]));

   float xMin = MIN(MIN(newx[0],newx[1]),MIN(newx[2],newx[3]));
   float yMin = MIN(MIN(newy[0],newy[1]),MIN(newy[2],newy[3]));

   // calculate the resulting rectangle 
   int finalWidth  = (int)(xMax-xMin);
   int finalHeight = (int)(yMax-yMin);
   int topleftX = (int)xMin;
   int topleftY = (int)yMin;

   return(Rectangle(topleftX, topleftY, finalWidth, finalHeight));
}

bool 
MapRenderer::snapshotHandlingImplemented() const
{
#ifdef HANDLE_SCREEN_AS_BITMAP 
   // Yep, the snapshot handling is implemented.
   return true;
#else
   // It's implemented but we don't want to use it.
   return false;
#endif   
}


void 
MapRenderer::makeScreenSnapshot()
{
   if ( m_screenSnapshot == NULL ) {
      return;
   }

   // Store the main buffer to the screen snapshot.
   TPoint point(0,0);
   TRect rect( point, 
               m_buffers[ c_mainBuffer ]->Device().SizeInPixels() );
   m_screenSnapshot->Gc().BitBlt( point,
                                  &m_buffers[ c_mainBuffer ]->Bitmap(),
                                  rect );
#if 0   
   // Mark the screen shot.
   m_screenSnapshot->Gc().SetPenColor( TRgb( 0, 0, 0 ) );
   m_screenSnapshot->Gc().SetPenSize( TSize( 2, 2 ) );
   m_screenSnapshot->Gc().DrawRect( rect );
#endif
}

void 
MapRenderer::moveScreenSnapshot( const MC2Point& deltaPoint )
{
   
   if ( m_screenSnapshot == NULL ) {
      return;
   }

   // First clear the bitmap.
   m_buffers[ c_mainBuffer ]->Gc().Clear();
   
   TRect rect( MC2Point(0,0), 
               m_buffers[ c_mainBuffer ]->Device().SizeInPixels() );
   m_buffers[ c_mainBuffer ]->Gc().BitBlt( deltaPoint,
                             &m_screenSnapshot->Bitmap(),
                             rect );
}

void 
MapRenderer::zoomScreenSnapshot(double factor, const MC2Point& zoomPoint)
{
   if ( m_screenSnapshot == NULL ) {
      return;
   }
   
   clearScreen();

   // Calculate the zoomed rectangle. 
   int width = m_buffers[ c_mainBuffer ]->Device().SizeInPixels().iWidth;
   int height = m_buffers[ c_mainBuffer ]->Device().SizeInPixels().iHeight;

   // Calculate zoomed size of screen snapshot.
   int nNewWidth  = int(width / factor);
   int nNewHeight = int(height / factor);

   // Calculate helper information.
   int nDeltaX = zoomPoint.getX() - width/2;
   int nDeltaY = zoomPoint.getY() - height/2;
   int nDeltaXZoomed = int(nDeltaX / factor);
   int nDeltaYZoomed = int(nDeltaY / factor);

   // Calculate shift for zooming of screen snapshot.
   int nShiftX = nDeltaXZoomed - nDeltaX;
   int nShiftY = nDeltaYZoomed - nDeltaY;

       
   TPoint upperLeftDestPoint( width/2 - nNewWidth / 2 - nShiftX,
                                 height/2 - nNewHeight / 2 - nShiftY );
   TPoint lowerRightDestPoint( upperLeftDestPoint );
   lowerRightDestPoint.iX += nNewWidth;
   lowerRightDestPoint.iY += nNewHeight;

   TRect destRect( upperLeftDestPoint, lowerRightDestPoint );
  
   TRect sourceRect( TPoint( 0, 0 ), TPoint( width, height ) );

   m_buffers[ c_mainBuffer ]->Gc().DrawBitmap(
         destRect,
         &m_screenSnapshot->Bitmap(),
         sourceRect );
}

