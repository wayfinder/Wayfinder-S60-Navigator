/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "WinMapPlotter.h"
#include "MC2Point.h"
#include "WinBitmap.h"
#include "WinTileMapToolkit.h"
#include "TextConv.h"
#include <cstring>
#include <cstdlib>
#include <math.h>
#include <tchar.h>
#include <mbstring.h>

#undef EXTRACT_ATTRIBS_FROM_FONTNAME
// This must be defined at least for windows xp.
#define EXTRACT_ATTRIBS_FROM_FONTNAME

/* the global logging file */
#include <fstream>
extern ofstream gLogFile;

// The max length of the strings supported by the WinMapPlotter.
#define MAX_STR_LEN 512

namespace isab {

// -- PointArray 

/* constructor */
PointArray::PointArray(int maxSize)
{
   /* reserve specified number of point cells in vector */
   reserve(maxSize);
}

/* destructor */
PointArray::~PointArray()
{
   /* clear the point vector */
   clear();
}

/* adds a point to the vector */
void PointArray::addPoint( int x, int y )
{
   push_back( MC2Point(x,y) );
   return;
}

/* Returns the number of coordinates in the vector. */
int PointArray::getSize() const
{
   return( size() );
}

/* Resets the add position ( and size ) back to zero. */
void PointArray::reset()
{
   clear();
   return;
}

/* Returns the number of allocated points. */
int PointArray::getAllocatedSize() const
{
   return( capacity() );
}


// -- WinMapPlotter 
/* private destructor */
WinMapPlotter::WinMapPlotter(HWND parentWin)
: m_parentWindow(parentWin),
  m_backBuffer(NULL),
  m_oldBitmap(NULL),
  m_backDC(NULL),
  m_width(0),
  m_height(0),
  m_penWidth(1), m_penStyle(PS_SOLID),
  m_penColor( RGB(255,255,255) ),
  m_pen(NULL),
  m_brushColor( RGB(0,0,0) ),
  m_brush(NULL),
  m_fillColor( RGB(128,128,128) ),
  m_fillBrush(NULL),
  m_curFont(NULL),
  m_fontWidth(0), m_fontHeight(0),
  m_curFontName(""), m_curFontSize(0),
  m_dragRect(NULL),
  m_frameRect(NULL),
  m_xform(NULL),
  m_frameWidth(0), 
  m_frameRed(0), m_frameGreen(0), m_frameBlue(0)
{
}

/* second-phase constructor */
int WinMapPlotter::construct(int width, int height)
{
   /* get the parent window's DC */
   HDC parentDC = GetDC(m_parentWindow);

   /* create a compatible DC from the passed DC */
   m_backDC = CreateCompatibleDC( parentDC );
   if(m_backDC == NULL) {
      /* the creation failed */
      return(-1);
   }

   /* release it, now that we have used it */
   ReleaseDC(m_parentWindow, parentDC);

   /* create the backbuffer */
   m_backBuffer = CreateBitmap(width, height, 1,
                               GetDeviceCaps(m_backDC, BITSPIXEL),
                               NULL);

   if(m_backBuffer == NULL) {
      /* cannot create bitmap */
      return(-2);
   }

   /* select the backbuffer into the device context */
   m_oldBitmap = (HBITMAP)SelectObject(m_backDC, m_backBuffer);
   if(m_oldBitmap == NULL) {
      /* selection failed */
      return(-3);
   }

   /* set the width and the height */
   m_width = width;
   m_height = height;

   /* set the default position */
   m_dX = m_dY = 0;

   /* set the default attributes */
   updateCurrentPen();
   updateCurrentBrush();
   updateFillingBrush();

   /* create the null pen */
   m_nullPen = CreatePen(PS_NULL, 1, RGB(0,0,0));

   /* create the null brush */
   m_nullBrush = (HBRUSH)GetStockObject( NULL_BRUSH );

   /* clear the buffer */
   clearScreen();

   /* setup the basic properties of the font */
   ::memset(&m_fontData, 0, sizeof(LOGFONT));
   m_fontData.lfWidth = 0;
   m_fontData.lfWeight = FW_MEDIUM;
   m_fontData.lfItalic = FALSE;
   m_fontData.lfStrikeOut = FALSE;
   m_fontData.lfUnderline = FALSE;
   m_fontData.lfCharSet = OEM_CHARSET;
   m_fontData.lfOutPrecision = OUT_TT_ONLY_PRECIS;
   m_fontData.lfClipPrecision = CLIP_DEFAULT_PRECIS;
   m_fontData.lfQuality = PROOF_QUALITY;
   m_fontData.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;

   /* set the default font */
   setFont(STRING("Lucida Console"), 12);

   /* complete success */
   return(0);
}

/* allocates a new WinMapPlotter */
WinMapPlotter* WinMapPlotter::allocate(int width,
                                       int height,
                                       HWND parentWin)
{
   /* allocate a new object */
   WinMapPlotter* newObj = new WinMapPlotter(parentWin);
   if(newObj == NULL) {
      /* memory not there */
      return(NULL);
   }

   /* do second-phase construction */
   if(newObj->construct(width, height) != 0) {
      /* nope, didnt happen */
      delete newObj;
      return(NULL);
   }

   return(newObj);
}

/* destructor */
WinMapPlotter::~WinMapPlotter()
{
   /* clean up the resources */
   DeleteObject(m_nullBrush);
   DeleteObject(m_nullPen);
   if(m_pen) DeleteObject(m_pen);
   if(m_brush) DeleteObject(m_brush);
   if(m_fillBrush) DeleteObject(m_fillBrush);
   if(m_curFont) DeleteObject(m_curFont);

   /* restore the old DC */
   SelectObject(m_backDC, m_oldBitmap);
   DeleteObject(m_backBuffer);
   DeleteDC(m_backDC);
}

/**
 *   Fills the screen with the background color.
 */
void WinMapPlotter::clearScreen()
{
   /* create a rectangle the size of the buffer */
   RECT screenRect;
   SetRect(&screenRect, 0, 0, m_width, m_height);
   FillRect(m_backDC, &screenRect, m_brush);
   return;
}

/**
 *   Sets the background color.
 *   <br />
 *   NB! The server will not send impossible values once the
 *   description formats are done.
 *   @param red   Red value   0-255.
 *   @param green Green value 0-255.
 *   @param blue  Blue value  0-255.
 */
void WinMapPlotter::setBackgroundColor( unsigned int red,
                                        unsigned int green,
                                        unsigned int blue )
{
   /* set the brush color */
   m_brushColor = RGB(red, green, blue);
   /* update the brush */
   updateCurrentBrush();
   return;
}

/**
 *   Sets the color of the pen.
 *   <br />
 *   NB! The server will not send impossible values once the
 *   description formats are done.
 *   @param red   Red value   0-255.
 *   @param green Green value 0-255.
 *   @param blue  Blue value  0-255.
 */
void WinMapPlotter::setPenColor( unsigned int red,
                                 unsigned int green,
                                 unsigned int blue )
{
   /* set the text color */
   SetTextColor( m_backDC, RGB(red,green,blue) );
   /* set the pen color */
   m_penColor = RGB(red, green, blue);
   /* update the pen */
   updateCurrentPen();
   return;
}

/**
 *   Sets the fill color for polygons.
 *   Default implementation sets the pen color.
 *   @see setPenColor.
 */
void WinMapPlotter::setFillColor( unsigned int red,
                                  unsigned int green,
                                  unsigned int blue )
{
   /* update the fill color */
   m_fillColor = RGB(red,green,blue);
   /* update the filling brush */
   updateFillingBrush();
}

/**
 *   Draws a polyline defined by the array of points.
 */
void WinMapPlotter::drawPolyLine( vector<MC2Point>::const_iterator begin,
                                  vector<MC2Point>::const_iterator end )
{
   /* move to the first point */
   MoveToEx(m_backDC, begin->getX(), begin->getY(), NULL);
   /* go thru the points and draw them */
   for(vector<MC2Point>::const_iterator it = begin;
       it != end;
       ++it) {
          /* draw the lines */
          LineTo(m_backDC, it->getX(), it->getY());
   }
   return;
}

void 
WinMapPlotter::drawPolyLineWithColor( vector<MC2Point>::const_iterator begin,
                                      vector<MC2Point>::const_iterator end,
                                      uint32 color,
                                      int lineWidth )
{
   // XXX: Optimize.
   WinMapPlotter::setLineWidth( lineWidth );
   WinMapPlotter::setPenColor( uint8( color >> 16 ),
                               uint8( color >>  8 ),
                               uint8( color ) );
   
   WinMapPlotter::drawPolyLine( begin, end );
   
}

/**
 *   Puts the dimensions of the displayed map in the
 *   variables.
 */
void WinMapPlotter::getMapSizePixels(Rectangle& size) const
{
   size = Rectangle(0, 0, m_width, m_height);
   return;
}

/**
 *   Creates a pointvector of size <code>size</code>.
 */
PointVector* WinMapPlotter::createPointVector(int size)
{
   PointArray* newArray = new PointArray(size);
   return(newArray);
}

/**
 *   Returns the pointvector to the MapPlotter.
 */
void WinMapPlotter::deletePointVector(PointVector* vect)
{
   PointArray* array = static_cast<PointArray*>(vect);
   vect = NULL;
   return;
}

// Functions that should be re-implemented. ---------------------->

// Text functions -------------------------------------------------

/**
 *   Creates a text string for use with the other text methods.
 *   Default implementation returns NULL.
 */
STRING* WinMapPlotter::createString(const char* text)
{
   STRING* newStr = new STRING(text);
   return(newStr);
}


/**
 *   Deletes a text string.
 *   Default implementation does nothing.
 */   
void WinMapPlotter::deleteString(STRING* text)
{
   delete text;
   text = NULL;
   return;
}

/**
 *   @return The number of characters in the string.
 */
int WinMapPlotter::getStringLength( const STRING& text )
{
   // Unfortunately neither _mbslen or _mbstrlen seem to work in 
   // windows. However MultiByteToWideChar will return the correct
   // number of characters. Not very efficient though.

   wchar_t uniStr[MAX_STR_LEN];
   int uniLen = 0;
   return MultiByteToWideChar(CP_UTF8,
                              0,
                              text.c_str(),
                              strlen(text.c_str()), // nbr bytes in str.
                              uniStr,
                              MAX_STR_LEN);
}

/**
 *   Sets font. The description format in the server
 *   and the MapPlotter must know what needs to be put
 *   here.
 *   @param fontName Name of the font in client-specific format.
 *   @param size     Size of the font in UNIT.
 */
void WinMapPlotter::setFont( const STRING& fontName, int size )
{
   /* check if the current font is the same as the one requested */
   if( (m_curFontName == fontName) && (m_curFontSize == size) ) return;

   STRING faceName = fontName;
#ifdef EXTRACT_ATTRIBS_FROM_FONTNAME
   // Reset the boldness.
   m_fontData.lfWeight = FW_MEDIUM;
   // Let's hope that we don't have any fonts called "New Boldania" 
   // or something like that.
   STRING boldMarker = " Bold";
   
   STRING::size_type pos = faceName.find( boldMarker );
   if ( pos != STRING::npos ) {
      // Found the keyword " Bold".
      // Remove it, and set the bold flag instead.
      faceName.replace( pos, boldMarker.size(), STRING("") );
      m_fontData.lfWeight = FW_BOLD;
   }
   
#endif

   /* add the fontface & size to the font data */
   ::strcpy(m_fontData.lfFaceName, faceName.c_str());
   m_fontData.lfHeight = size;
   
   /* select our font */
   m_curFont = createFont(0.0f);
   if(m_curFont == NULL) return;
   /* select the new font into our DC */
   HFONT oldFont = (HFONT)SelectObject(m_backDC, m_curFont);
   if(oldFont) {
      /* delete the old font */
      DeleteObject(oldFont);
   }

   /* set the current font variables */
   m_curFontName = fontName;
   m_curFontSize = size;

   /* get the Text Metrics for the font */
   GetTextMetrics(m_backDC, &m_textMetrics);
   m_fontHeight = m_textMetrics.tmHeight;
   m_fontWidth = m_textMetrics.tmMaxCharWidth;

   gLogFile << "PLOTTER : Font Set : " << fontName 
            << "[" << size << "]" << endl;

   return;
}

/**
 *   Returns the rotation step possible by the plotter.
 *   <br />Default implementation returns 0.
 *   @return The minimum number of degrees that characters can
 *           be rotated.
 */
int WinMapPlotter::getMinTextRotation()
{
   return(0);
}

int
WinMapPlotter::utf8ToUnicode( const STRING& text,
                              int startIdx,
                              int nbrChars,
                              wchar_t* uniStr,
                              int uniStrLen )
{
   // Convert the full string to unicode first.
   wchar_t fullUniStr[MAX_STR_LEN];
   int fullUniLen = TextConverter::UTF2Unicode( text.c_str(), 
                               fullUniStr, 
                               MAX_STR_LEN );

  if( nbrChars == -1 ) {
      // Get length of string from startIdx to end. 
      nbrChars = fullUniLen - startIdx;
   }
 
   // Extract the portion of it that we are interested in.
   wcsncpy( uniStr, fullUniStr + startIdx, nbrChars );
   int uniLen = nbrChars;    
   uniStr[ uniLen ] = 0; 

   return uniLen;
}

/**
 *   Draws text on the display using the pen color and transparent
 *   background.
 *   @param text      The start of the text to draw.
 *   @param points    The center point of the text.
 *   @param startIdx  The first character to draw.
 *   @param nbrChars  The number of characters to draw.
 */
void WinMapPlotter::drawText( const STRING& text,
                              const MC2Point& point,
                              int startIdx,
                              int nbrChars,
                              float angle )
{
   // Extract the portion of it that we are interested in.
   wchar_t uniStr[MAX_STR_LEN];

   int uniLen = utf8ToUnicode( text, startIdx, nbrChars, uniStr, MAX_STR_LEN);

   
   /* set the background drawing mode to transparent */
   SetBkMode(m_backDC, TRANSPARENT);

   /* set text alignment */
   // Note that the specified point is the point of the center, 
   // but the text will be drawn at the baseline.
   // Need to compensate for this.
   SetTextAlign(m_backDC, TA_CENTER | TA_BASELINE | TA_NOUPDATECP);

   /* axis adjust deltas */
   int xDelta = 0, yDelta = 0;

   /* select our font */
   HFONT newFont = createFont(angle);
   if(newFont == NULL) return;
   /* select the new font into our DC */
   HFONT oldFont = (HFONT)SelectObject(m_backDC, newFont);
   
   SIZE strSize;
   GetTextExtentPoint32W(m_backDC, uniStr, uniLen, &strSize);

   // Assume that the difference from baseline to center is roughly
   // the height / 4.
   int delta = strSize.cy / 4;
  
   // Compensate for that the text will be drawn at the baseline and
   // not at the center.
   float radAngle = angle / 180 * M_PI;
   xDelta = int( delta * sin( radAngle ) );
   yDelta = int( delta * cos( radAngle ) );

   
   /* draw the text */
   TextOutW(m_backDC,
            point.getX() + xDelta, point.getY() + yDelta,
            uniStr,
            uniLen);

/* delete our font */
   SelectObject(m_backDC, oldFont);
   DeleteObject(newFont);

   /* restore the background drawing mode to opaque */
   SetBkMode(m_backDC, OPAQUE);

#if 0
   // XXX: Debug.
   setLineWidth( 4 );
   vector<MC2Point> pointVec;
   pointVec.push_back( point );
   setPenColor( 0, 0, 0 );
   drawPolyLine( pointVec.begin(), pointVec.end() );
   MC2Point otherPoint( point.getX() + xDelta, point.getY() + yDelta );
   pointVec.clear();
   pointVec.push_back( otherPoint );
   setPenColor( 0, 255, 0 );
   drawPolyLine( pointVec.begin(), pointVec.end() );
#endif   
   return;
}

/**
 *   Puts the bounding boxes of each character in str
 *   into the vector boxes.
 *   <br />
 *   Default implementation does nothing and returns 0.
 *   @param boxes  Vector to add the boxes to.
 *   @see drawText
 *   @return Number of characters in str.
 */
isab::Rectangle WinMapPlotter::getStringAsRectangle( const STRING& text,
                                                     const MC2Point& point,
                                                     int startIdx,
                                                     int nbrChars,
                                                     float angle )
{

   // Extract the portion of it that we are interested in.
   wchar_t uniStr[MAX_STR_LEN];

   int uniLen = utf8ToUnicode( text, startIdx, nbrChars, uniStr, MAX_STR_LEN);

   /* calculate the size of the string */
   SIZE strSize;
   GetTextExtentPoint32W(m_backDC, uniStr, uniLen, &strSize);

   /* the rectangle's dimensions are based on the center point and the 
      font height and the total string width */
   int rectX = point.getX() - (strSize.cx >> 1);
   int rectY = point.getY() - (strSize.cy >> 1);

   Rectangle rect;
   
   /* zero angle specified */
   if(angle == 0.0f) {
      /* return the new rectangle */
      rect = Rectangle(rectX, rectY, strSize.cx, strSize.cy);
   } else {
      // XXX: This probably doesn't work very well.
      /* angle specified, calculate the rotated rectangle */
      rect = rotatedRect(rectX, rectY, 
                         rectX+strSize.cx, rectY+strSize.cy, 
                         angle);
   }


   return rect;
}


/**
 *   Puts the bounding boxes of each character in str
 *   into the vector boxes.
 *   <br />
 *   Default implementation does nothing and returns 0.
 *   @param boxes  Vector to add the boxes to.
 *   @see drawText
 *   @return Number of characters in str.
 */
int WinMapPlotter::getStringAsRectangles( vector<Rectangle>& boxes,
                                          const STRING& text,
                                          const MC2Point& point,
                                          int startIdx,
                                          int nbrChars,
                                          float angle )
{
   
   // Extract the portion of it that we are interested in.
   wchar_t uniStr[MAX_STR_LEN];

   int uniLen = utf8ToUnicode( text, startIdx, nbrChars, uniStr, MAX_STR_LEN);
   
   /* select our font */
   HFONT newFont = createFont((int)angle);
   if(newFont == NULL) return 0;
   /* select the new font into our DC */
   HFONT oldFont = (HFONT)SelectObject(m_backDC, newFont);
   
   /* calculate the size of the string */
   SIZE strSize;
   GetTextExtentPoint32W(m_backDC, uniStr, uniLen, &strSize);

   /* delete our font */
   SelectObject(m_backDC, oldFont);
   DeleteObject(newFont);
   
   // End of new stuff.
   
   /* counter variable */
   int i;   
   
   /* Loop thru the text and set the bounding rectangle for each character 
      Each rectangle holds absolute coords for the begininning of 
      each character and their width and height */

   /* height info */
   int textH = strSize.cy;
   int halfTextH = textH >> 1;

   /* number of characters to be printed */
   int maxTextLen = strSize.cx;
   int halfTextLen = maxTextLen >> 1;

   /* get the center points of all the characters */
   vector<MC2Point> origArray;

   int startX = point.getX() - halfTextLen;
   // Lets guess that all chars are approx. equally wide.
   int textW = strSize.cx / nbrChars;

   for(i = 0; i < nbrChars; i++) {

      /* get the relative coord for the center */
      int origX = (startX + (textW >> 1)) - point.getX();
      int origY = 0;

      /* get the rotated center */
      int rotOrigX, rotOrigY;
      rotate2D(origX, origY, 
            angle, 
            &rotOrigX, &rotOrigY,
            point.getX(), point.getY());

      /* add to list */
      origArray.push_back(MC2Point(rotOrigX,
               rotOrigY));

      /* move the x-axis forward */
      startX += textW;
   }

   /* get the rectangles */
   int curOrig = 0;
   for(i = 0; i < nbrChars; i++) {
      // XXX: If Varun's rotatedRect method would work 
      // then it should be used.
      // However it doesn't work, so instead we don't rotate the boxes.
      // The nonrotated boxes may cause incorrect collision detection,
      // however it will not happen very often.

      //         Rectangle rotBox = rotatedRect(origArray[curOrig].getX(),
      //                                        origArray[curOrig].getY(),
      //                                        origArray[curOrig].getX()+textW,
      //                                        origArray[curOrig].getY()+textH, 
      //                                        angle);
      Rectangle rotBox = Rectangle(origArray[curOrig].getX() - textW/2,
            origArray[curOrig].getY() - textH/2,
            textW,
            textH );

      boxes.push_back(rotBox);
      curOrig++;
   }
   /* clear origin vector */
   origArray.clear();

   /* returns the number of characters processed */
   return(nbrChars);
}

// Misc ----------------------------------------------------------

/**
 *   Returns true if the drawing is double buffered.
 *   Default implementation returns false.
 *   @return True if the drawing is double buffered.
 */
bool WinMapPlotter::doubleBuffered() const
{
   return(true);
}

/**
 *   Tells the MapPlotter that it is ok to change buffer
 *   if it is doubleBuffered. Default implementation does
 *   nothing.
 *   @return doubleBuffered().
 */
bool WinMapPlotter::drawBuffer( const Rectangle& rect )
{
   return(true);
}

/**
 *   If the MapPlotter needs to prepare the plotting in
 *   any way it should do it in this method which is called
 *   before drawing.
 *   <br />
 *   The default implementation does not do anything.
 */
void WinMapPlotter::prepareDrawing()
{
   gLogFile << "PLOTTER : Drawing Begins!" << endl;
   return;
}

/**
 *   Must be called by the drawing engine everytime a drawing
 *   is completed. This is to be able to free up the resources
 *   that may have been used during the draw.
 *   <br />
 *   The default implementation does not do anything.
 */
void WinMapPlotter::drawingCompleted()
{
   gLogFile << "PLOTTER : Drawing Ends!" << endl;

   HDC dc = GetDC(m_parentWindow);
   blitTo(dc, m_dX, m_dY);
   ReleaseDC(m_parentWindow, dc);

   return;
}

/**
 *   Converts the bytes into a bitmap that can be drawn by the
 *   MapPlotter.
 *   Default implementation returns NULL.
 *   FIXME: Should we add the type of image too, e.g. PNG etc.?
 */
BitMap* WinMapPlotter::createBitMap(bitMapType type,
                                    const uint8* bytes, int nbrBytes)
{
   /* create the bitmap */
   WinBitmap* newObj = WinBitmap::allocate(bytes, nbrBytes);
   if(newObj == NULL) return(NULL);

   /* return the newly created object */
   return(newObj);
}


/**
 *   Draws the bitmap at x and y.
 *   Default implementation draws a small rectangle.
 */
void WinMapPlotter::drawBitMap( const MC2Point& center,
                                const BitMap* bitMap)
{
   /* get our platform specific bitmap */
   const WinBitmap* winBmp = static_cast<const WinBitmap*>(bitMap);
   /* draws the bitmap centred around the specified point */
   int bX = center.getX() - (winBmp->width() >> 1);
   int bY = center.getY() - (winBmp->height() >> 1);
   /* do the blit depending on bitmap type */
   if(winBmp->isMasked()) {   /* masked BMP */
      
      MaskBlt(m_backDC, bX, bY,
              winBmp->width(), winBmp->height(),
              winBmp->getDC(),
              0, 0,
              winBmp->getMask(),
              0, 0,
              MAKEROP4(SRCCOPY,0x00AA0029));

   }
   else {                     /* normal BMP */
      BitBlt(m_backDC, bX, bY, 
             winBmp->width(), winBmp->height(),
             winBmp->getDC(),
             0, 0,
             MAKEROP4(SRCCOPY,0));
   }
   return;
}

/**
 *   Gets the size of the specified BitMap in pixels
 *   Has to be implemented in derived class.
 *   @param size Rectangle which the bitmap is inside.
 *   @param bmp  Bitmap to get the size from.
 *   @param centerXY The center of the bitmap (position where it is drawn).
 */
void WinMapPlotter::getBitMapAsRectangle(Rectangle& size,
                                         const MC2Point& origCenter,
                                         const BitMap* bmp)
{
   /* get our platform specific bitmap */
   const WinBitmap* winBmp = static_cast<const WinBitmap*>(bmp);
   /* draws the bitmap centred around the specified point */
   int bX = origCenter.getX() - (winBmp->width() >> 1);
   int bY = origCenter.getY() - (winBmp->height() >> 1);

   /* set the Rectangle */
   size = Rectangle(bX, bY, winBmp->width(), winBmp->height());

   return;
}

/**
 *   Deletes the supplied bitmap.
 */
void WinMapPlotter::deleteBitMap( BitMap* bitMap )
{
   WinBitmap* winBmp = static_cast<WinBitmap*>(bitMap);
   delete winBmp;
   winBmp = NULL;
   return;
}

/**
 *   Returns the wanted bitmap extension.
 *   Default implementation returns png. Dot will be appended
 *   by TileMapHandler.
 */
const char* WinMapPlotter::getBitMapExtension() const
{
   return( "tga" );
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
void WinMapPlotter::drawPolygon( vector<MC2Point>::const_iterator begin,
                                 vector<MC2Point>::const_iterator end )
{
   /* go thru the points and add them to the point buffer */
   /* NOTE : This will crash if more than 1024 points are specified */
   int index = 0;
   for(vector<MC2Point>::const_iterator it = begin;
       it != end;
       ++it) {
          m_pointBuffer[index].x = it->getX();
          m_pointBuffer[index].y = it->getY();
          ++index;
       }

   /* draw the polygon */
   SelectObject(m_backDC, m_nullPen);
   Polygon(m_backDC, m_pointBuffer, index);
   SelectObject(m_backDC, m_pen);

   return;
}

void WinMapPlotter::
drawPolygonWithColor( vector<MC2Point>::const_iterator begin,
                      vector<MC2Point>::const_iterator end,
                      uint32 fillColor )
{
   // XXX: Optimize.
   WinMapPlotter::setFillColor( uint8( fillColor >> 16 ),
                                uint8( fillColor >>  8 ),
                                uint8( fillColor ) );
   WinMapPlotter::drawPolygon( begin, end );
}


/**
 *   Draws a spline of some sort.
 *   <br />
 *   Default implementation draws a polyline if not filled else polygon.
 */
void WinMapPlotter::drawSpline( bool filled,
                                vector<MC2Point>::const_iterator begin,
                                vector<MC2Point>::const_iterator end )
{
   return;
}

/**
 *   Same as drawPolygons, but the polygons must be convex. Can be
 *   implemented to speed things up.
 *   <br />
 *   Default implementation calls drawPolygons.
 */
void WinMapPlotter::drawConvexPolygon( vector<MC2Point>::const_iterator begin,
                                       vector<MC2Point>::const_iterator end )
{
   return;
}

/**
 *   Draws an arc.
 *   Default implementation draws a line from one corner to the other
 *   in the rectangle.
 */
void WinMapPlotter::drawArc( bool filled,
                             const Rectangle& rect,
                             int startAngle,
                             int stopAngle)
{

   return;
}

/**
 *   Draws a rectangle.
 *   @param filled     If true the rectangle is filled.
 *   @param rect       Dimensions of the rectangle.
 *   @param cornerSize Radius of corners.
 *   Default implementation cannot draw rounded corners and uses
 *   drawPolyLine or drawConvexPolygon.
 */
void WinMapPlotter::drawRect( bool filled,
                              const Rectangle& rect,
                              int cornerSize)
{
   /* create the Rectangle */
   RECT winRect;
   winRect.left = rect.getX();
   winRect.top = rect.getY();
   winRect.right = rect.getX() + rect.getWidth();
   winRect.bottom = rect.getY() + rect.getHeight();

   /* check if rectangle is filled */
   if(filled) {
      SelectObject(m_backDC, m_nullPen);
      FillRect(m_backDC, &winRect, m_fillBrush);
      SelectObject(m_backDC, m_pen);
   }
   else {
      
      HBRUSH oldBrush = (HBRUSH)SelectObject(m_backDC, m_nullBrush);
      /* draw the outline */
      ::Rectangle(m_backDC, 
                  winRect.left, winRect.top,
                  winRect.right, winRect.bottom);
      SelectObject(m_backDC, oldBrush );
   }

   return;
}

/**
 *   Sets the width of the lines.
 *   Default implementation does nothing.
 */
void WinMapPlotter::setLineWidth( int width )
{
   /* set the new width */
   m_penWidth = width;
   /* update our pen */
   updateCurrentPen();
   return;
}

/**
 *   Sets the cap style of the pen.
 *   Default implementation does nothing.
 */
void WinMapPlotter::setCapStyle( enum capStyle )
{
   return;
}

/**
 *   Sets the dash style of the pen.
 *   Default implementation does nothing.
 */
void WinMapPlotter::setDashStyle( enum dashStyle style )
{
   switch(style)
   {
      case(nullDash): m_penStyle = PS_NULL; break;
      case(solidDash): m_penStyle = PS_SOLID; break;
      case(dottedDash): m_penStyle = PS_DOT; break;
      case(dashedDash): m_penStyle = PS_DASH; break;
      default: m_penStyle = PS_NULL; break;
   }
   return;
}

/**
 *   Puts the dimensions of the displayed map in the
 *   variables. Must be overridden if the drawing units
 *   of the map is not pixels.
 *   <br />
 *   Default implementation returns getMapSizePixels.
 */
void WinMapPlotter::getMapSizeDrawingUnits(Rectangle& size) const
{
   getMapSizePixels(size);
   return;
}

// -- Windows Specific Methods

/* creates a pen using the current pen parameters, deletes the
   old pen if required */
void WinMapPlotter::updateCurrentPen()
{
   /* delete any previously allocated pen */
   if(m_pen) {
      DeleteObject(m_pen);
      m_pen = NULL;
   }

   /* allocate a new HPEN using the current parameters */
   m_pen = CreatePen(m_penStyle, m_penWidth, m_penColor);

   /* set to the DC if valid */
   if(m_pen) {
      SelectObject(m_backDC, m_pen);
   }

   /* return */
   return;
}

/* creates a brush using the current brush parameters, deletes the
   old brush if required */
void WinMapPlotter::updateCurrentBrush()
{
   /* delete any previously allocated brush */
   if(m_brush) {
      DeleteObject(m_brush);
      m_brush = NULL;
   }

   /* allocate a new HBRUSH using the current parameters */
   m_brush = CreateSolidBrush(m_brushColor);

   /* return */
   return;
}

/* creates a fill brush using the current brush parameters, 
   deletes the old brush if required */
void WinMapPlotter::updateFillingBrush()
{
   /* delete any previously allocated brush */
   if(m_fillBrush) {
      DeleteObject(m_fillBrush);
      m_fillBrush = NULL;
   }

   /* allocate a new HBRUSH using the current parameters */
   m_fillBrush = CreateSolidBrush(m_fillColor);

   /* set to the DC if valid */
   if(m_fillBrush) {
      SelectObject(m_backDC, m_fillBrush);
   }

   return;
}

/* blits the internal back buffer to the specified DC */
void WinMapPlotter::blitTo(HDC destDC, int32 dX, int32 dY)
{
   if(destDC) {

      /* draw a frame if required */
      if((m_xform) && (m_frameRect)) {
         /* get the points from coords */
         MC2Point topLeft(0,0), bottomRight(0,0);

         /* get top-left corner */
         m_xform->transformPointUsingCosLat(topLeft.getX(), topLeft.getY(),
                                            MC2Coordinate(m_frameRect->getMaxLat(), 
                                                          m_frameRect->getMinLon()));
         /* get bottom-right corner */
         m_xform->transformPointUsingCosLat(bottomRight.getX(), bottomRight.getY(),
                                            MC2Coordinate(m_frameRect->getMinLat(), 
                                                          m_frameRect->getMaxLon()));

         /* draw the frame */
         RECT fRect;
         fRect.left   = topLeft.getX();
         fRect.top    = topLeft.getY();
         fRect.right  = bottomRight.getX();
         fRect.bottom = bottomRight.getY();

         drawFrame(m_backDC, &fRect,
                   m_frameWidth,
                   m_frameRed, m_frameGreen, m_frameBlue);
      }

      BitBlt( destDC, dX, dY, m_width, m_height, 
              m_backDC, 0, 0,
              SRCCOPY);

      /* draw the drag rectangle if required */
      if(m_dragRect) {
         /* draw the non-filled rectangle */
         drawFrame(destDC, m_dragRect, 1, 0, 0, 0);
      }

   }
   return;
}

/* resizes the plotter to the specified size */
/* Returns true on success, false on error */
bool WinMapPlotter::resize(int32 newWidth, int32 newHeight)
{
   HBITMAP newBuffer = NULL;

   /* try to allocate a bitmap of the new dimesnions and DC's color depth */
   newBuffer = CreateBitmap(newWidth, newHeight, 1,
                            GetDeviceCaps(m_backDC, BITSPIXEL),
                            NULL);

   if(newBuffer == NULL) {
      /* cannot create bitmap */
      return(false);
   }

   /* remove our backbuffer from the DC */
   SelectObject(m_backDC, m_oldBitmap);

   /* select the new backbuffer into the device context */
   if(SelectObject(m_backDC, newBuffer) == NULL) {
      SelectObject(m_backDC, m_backBuffer);
      return(false);
   }

   /* destroy the old buffer, and set it to the new one */
   DeleteObject(m_backBuffer);
   m_backBuffer = newBuffer;

   /* set the width and the height */
   m_width = newWidth;
   m_height = newHeight;

   /* success */
   return(true);
}

/* creates a Win32 specific font - supports rotated fonts */
HFONT WinMapPlotter::createFont(float angle)
{
   HFONT newFont = NULL;

   /* add the font angle to the font data */
   m_fontData.lfEscapement = m_fontData.lfOrientation = int(angle*10.0f);

   /* try to create the font */
   newFont = ::CreateFontIndirect(&m_fontData);

   /* success, return the created font */
   return(newFont);
}

/* gets the rotated bounding rectangle for the specified angle and points */
isab::Rectangle WinMapPlotter::rotatedRect(int x1, int y1, int x2, int y2, 
                                           float angle)
{
   // XXX: This method doesn't work very well.

   /* max\min bounds of the final rectangle */
   POINT minBound, maxBound;

   /* the points of the rectangle */
   POINT rotVerts[4];

   /* get half dimensions of the rectangle */
   int halfW = (x2-x1) >> 1;
   int halfH = (y2-y1) >> 1;

   /* get the center point */
   int cx = x1 + halfW;
   int cy = y1 + halfH;

   /* rotate the points */
   rotate2D(-halfW, -halfH, angle, (int*)&rotVerts[0].x, (int*)&rotVerts[0].y, cx, cx);
   rotate2D( halfW, -halfH, angle, (int*)&rotVerts[1].x, (int*)&rotVerts[1].y, cx, cx);
   rotate2D( halfW,  halfH, angle, (int*)&rotVerts[2].x, (int*)&rotVerts[2].y, cx, cx);
   rotate2D(-halfW,  halfH, angle, (int*)&rotVerts[3].x, (int*)&rotVerts[3].y, cx, cx);

   /* get the minimum and maximum bound points */
   minBound.x = min(rotVerts[0].x, min(min(rotVerts[1].x,rotVerts[2].x), rotVerts[3].x));
   minBound.y = min(rotVerts[0].y, min(min(rotVerts[1].y,rotVerts[2].y), rotVerts[3].y));
   maxBound.x = max(rotVerts[0].x, max(max(rotVerts[1].x,rotVerts[2].x), rotVerts[3].x));
   maxBound.y = max(rotVerts[0].y, max(max(rotVerts[1].y,rotVerts[2].y), rotVerts[3].y));

   /* get the dimensions of the rectangle */
   int w = maxBound.x - minBound.x;
   int h = maxBound.y - minBound.y;

   /* return the rotated rectangle */
   return( Rectangle(minBound.x, minBound.y, w, h) );
}

/* Draws a frame on the map. It uses a Win32 RECT and the specified 
   frame width and color */
void WinMapPlotter::drawFrame(HDC dc,
                              LPRECT frame, 
                              int fWidth,
                              int r, int g, int b)
{
   /* set the color and width */
   HPEN newPen = CreatePen(PS_SOLID,
                           fWidth,
                           RGB(r,g,b));
   HPEN oldPen = (HPEN)SelectObject(dc, newPen);
   /* set a null-brush, and save the old brush in use */
   HBRUSH oldBrush = (HBRUSH)SelectObject(dc, m_nullBrush);
   /* draw the rectangle */
   ::Rectangle(dc, 
               frame->left, frame->top,
               frame->right, frame->bottom);
   /* restore the old brush */
   SelectObject(dc, oldBrush);
   /* restore old pen */
   SelectObject(dc, oldPen);
   return;
}

/* Sets a drag rectangle to be drawn on the target DC after 
   blitting the backbuffer to it.
   Pass NULL to disable the drag rectangle */
void WinMapPlotter::setDragRect(LPRECT dragPtr)
{
   m_dragRect = dragPtr;
   return;
}


/* Sets a frame to be drawn on the map. 
   Uses given TransformMatrix to do the MC2Coord to 
   screen-point translation. Uses given width and color.
   Pass NULL to disable the frame rectangle */
void WinMapPlotter::setFrameRect(TransformMatrix* tMatrix,
                                 MC2BoundingBox* frame,
                                 int fWidth,
                                 int r, int g, int b)
{
   m_xform = tMatrix;
   m_frameRect = frame;
   m_frameWidth = fWidth;
   m_frameRed = r;
   m_frameGreen = g;
   m_frameBlue = b;
   return;
}


}; /* namespace isab */

