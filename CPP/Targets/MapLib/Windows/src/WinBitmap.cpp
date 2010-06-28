/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "WinBitmap.h"
#include <cstring>
#include <cstdio>
#include "BitBuffer.h"

/* the global logging file */
#include <fstream>
extern ofstream gLogFile;

#include <cstdio>
#include <cstring>

namespace isab {
     
/* private constructor */
WinBitmap::WinBitmap()
: m_bmp(NULL),
  m_mask(NULL),
  m_oldBmp(NULL),
  m_dc(NULL),
  m_width(0), m_height(0),
  m_isMasked(false)
{
}

int
WinBitmap::readColorMapData( const TGAHEADER& tgaHeader, 
                             const byte* colorData )
{
   if ( tgaHeader.colorMapType == 0 ) {
      // No colormap information available.
      return 0;
   }

   uint16 colorMapLength = tgaHeader.colorMapSpec[2] + 
                           (tgaHeader.colorMapSpec[3] << 8);
   uint8 colorMapEntrySize = tgaHeader.colorMapSpec[4];
                    
   // The length of the buffer is not necessarily correct, however
   // it doesn't matter. We won't read too long.
   BitBuffer cmBuf( (byte*) colorData, colorMapLength * 4 ); 
   PIXEL pixel;
   
   m_colorMap.reserve( colorMapLength );

   for( int i = 0; i < colorMapLength; ++i ) {
       switch (colorMapEntrySize)
       {
         case 8:
         default:
         {
           //  Gray scale.
           pixel.red=cmBuf.readNextBAByte();
           pixel.green=pixel.red;
           pixel.blue=pixel.red;
           break;
         }
         // XXX: Does not handle case 15 or 16.
         case 24:
         case 32:
         {
           // 8 bits each of blue, green and red.
           pixel.blue=cmBuf.readNextBAByte();
           pixel.green=cmBuf.readNextBAByte();
           pixel.red=cmBuf.readNextBAByte();
           break;
         }
      }
      // Add pixel to color map.
      m_colorMap.push_back( pixel );
   }
   // Return the offset.
   return cmBuf.getCurrentOffset();
}

/* second-phase constrcutor */
bool WinBitmap::construct(const byte* dataBuf, uint32 nbrBytes)
{
   /* get the current DC format */
   m_dc = CreateCompatibleDC(NULL);
   if(m_dc == NULL) {
      return(false);
   }

/** TGA Implementation **/
#if 1
   register int32 row, col;
   register int32 index;
   int32 red, green, blue, alpha;
   int32 bitDepth;
   HDC maskDC = NULL;
   HBITMAP oldMaskBmp = NULL;

   /* get the BMP file information */
   TGAHEADER tgaHeader;
   ::memcpy(&tgaHeader, dataBuf, sizeof(TGAHEADER));

   /* get the location of the color data */
   
   int offset = sizeof(TGAHEADER) + tgaHeader.numCharsID;
   
   const byte* colorData = reinterpret_cast<const byte*>(&dataBuf[offset]);

   bool useColorMap = false;
   if ( tgaHeader.colorMapType == 1 ) {
      // Use color map.
      useColorMap = true;
      // Read the color map data.
      int offsetToColorData = readColorMapData( tgaHeader, colorData );
      // Add the offset to colorData so that the colorData actually
      // points to the color data instead of to the color map.
      colorData += offsetToColorData; 
   }
     
   // colorData should now point at the color data!

   /* set the dimensions */
   m_width = tgaHeader.imageWidth;
   m_height = tgaHeader.imageHeight;

   /* get the bit-depth of the image */
   bitDepth = tgaHeader.bitsPerPixel;

   /* create the bitmap using the given info */
   m_bmp = CreateBitmap(m_width, m_height, 1,
                        GetDeviceCaps(m_dc, BITSPIXEL),
                        NULL);
   if(m_bmp == NULL) {
      /* cannot create bitmap */
      return(false);
   }

   /* select the bitmap into the DC */
   m_oldBmp = (HBITMAP)SelectObject(m_dc, m_bmp);

   /* create the same sized monocrome mask if required */
   if(bitDepth == 32) {
      m_mask = CreateBitmap(m_width, m_height, 1, 1, NULL);
      /* check for errors */
      if(m_mask == NULL) {
         return(false);
      }
      /* create DC, and select the mask into it */
      maskDC = CreateCompatibleDC(NULL);
      /* check for errors */
      if(maskDC == NULL) {
         return(false);
      }
      /* select our monochrome mask into it */
      oldMaskBmp = (HBITMAP)SelectObject(maskDC, m_mask);
   }

   /* Bottom-up TGAs are expected .. read and create */
   index = 0;
   for(row = m_height-1; row >= 0; --row) {
      for(col = 0; col < m_width; ++col) {

         /* 32-bit TGAs need different processing from 24-bit */
         if(bitDepth == 32) {
            if ( useColorMap ) {
               int colorMapIdx = colorData[ index++ ];
               PIXEL p = m_colorMap[ colorMapIdx ];
               blue = p.blue;
               green = p.green;
               red = p.red;
               alpha = 255;
            } else {
               // No color map.
               /* read the color value in components */
               blue  = colorData[index++];
               green = colorData[index++];
               red   = colorData[index++];
               alpha = colorData[index++];
            }
            /* write the color pixel */
            SetPixelV(m_dc, col, row, RGB(red,green,blue));
            /* check the alpha component */
            if(alpha > 127) {
               /* solid pixel */
               SetPixelV(maskDC, col, row, RGB(255,255,255));
            }
            else {
               /* transparent pixel */
               SetPixelV(maskDC, col, row, RGB(0,0,0));
            }
         }
         /* expecting 24-bit RGB Image */
         else {
            if ( useColorMap ) {
               int colorMapIdx = colorData[ index++ ];
               PIXEL p = m_colorMap[ colorMapIdx ];
               blue = p.blue;
               green = p.green;
               red = p.red;
            } else {
               // No color map.
               /* read the color value in components */
               blue  = colorData[index++];
               green = colorData[index++];
               red   = colorData[index++];
            }
            /* write the color pixel */
            SetPixelV(m_dc, col, row, RGB(red,green,blue));
         }

      }
   }

   /* release the mask DC if used */
   if(bitDepth == 32) {
      SelectObject(maskDC, oldMaskBmp);
      DeleteDC(maskDC);
   }

   if(bitDepth == 32) {
      /* set our masked flag */
      m_isMasked = true;
   }
   else {
      m_isMasked = false;
   }

   /* success */
   return(true);
#endif


/** BMP Implementation **/
#if 0
   register int32 row, col;
   register int32 index;
   int32 red, green, blue, alpha;
   HDC maskDC = NULL;
   HBITMAP oldMaskBmp = NULL;

   /* get the BMP file information */
   BITMAPFILEHEADER bmHeader;
   BITMAPINFOHEADER bmInfo;
   ::memcpy(&bmHeader, dataBuf, sizeof(BITMAPFILEHEADER));
   ::memcpy(&bmInfo, &dataBuf[sizeof(BITMAPFILEHEADER)], 
            sizeof(BITMAPINFOHEADER));

   /* get the location of the color data */
   const byte* colorData = reinterpret_cast<const byte*>(&dataBuf[bmHeader.bfOffBits]);

   /* set the dimensions */
   m_width = bmInfo.biWidth;
   m_height = bmInfo.biHeight;

   /* create the bitmap using the given info */
   m_bmp = CreateBitmap(m_width, m_height, 1,
                        GetDeviceCaps(m_dc, BITSPIXEL),
                        NULL);
   if(m_bmp == NULL) {
      /* cannot create bitmap */
      return(false);
   }

   /* select the bitmap into the DC */
   m_oldBmp = (HBITMAP)SelectObject(m_dc, m_bmp);

   /* create the same sized monocrome mask if required */
   if(bmInfo.biBitCount == 32) {
      m_mask = CreateBitmap(m_width, m_height, 1, 1, NULL);
      /* check for errors */
      if(m_mask == NULL) {
         return(false);
      }
      /* create DC, and select the mask into it */
      maskDC = CreateCompatibleDC(NULL);
      /* check for errors */
      if(maskDC == NULL) {
         return(false);
      }
      /* select our monochrome mask into it */
      oldMaskBmp = (HBITMAP)SelectObject(maskDC, m_mask);
   }

   /* Bottom-up BMP's are expected .. read and create */
   index = 0;
   for(row = m_height-1; row >= 0; --row) {
      for(col = 0; col < m_width; ++col) {

         /* 32-bit BMP's need different processing from 24-bit */
         if(bmInfo.biBitCount == 32) {
            /* read the color value in components */
            alpha = colorData[index++];
            blue  = colorData[index++];
            green = colorData[index++];
            red   = colorData[index++];
            /* write the color pixel */
            SetPixelV(m_dc, col, row, RGB(red,green,blue));
            /* check the alpha component */
            if(alpha > 127) {
               /* solid pixel */
               SetPixelV(maskDC, col, row, RGB(255,255,255));
            }
            else {
               /* transparent pixel */
               SetPixelV(maskDC, col, row, RGB(0,0,0));
            }
         }
         /* expecting 24-bit RGB Image */
         else {
            /* read the color value in components */
            blue  = colorData[index++];
            green = colorData[index++];
            red   = colorData[index++];
            /* write the color pixel */
            SetPixelV(m_dc, col, row, RGB(red,green,blue));
         }

      }
   }

   /* release the mask DC if used */
   if(bmInfo.biBitCount == 32) {
      SelectObject(maskDC, oldMaskBmp);
      DeleteDC(maskDC);
   }

   if(bmInfo.biBitCount == 32) {
      /* set our masked flag */
      m_isMasked = true;
   }
   else {
      m_isMasked = false;
   }

   /* success */
   return(true);

#endif

/* TESTING Implemntation */
#if 0
   /* try to create the bitmap -- TESTING */
   m_bmp = CreateBitmap(7, 7, 1,
                        GetDeviceCaps(m_dc, BITSPIXEL),
                        NULL);
   if(m_bmp == NULL) {
      /* delete the DC */
      DeleteDC(m_dc);
      return(false);
   }

   /* select the bitmap into the DC */
   m_oldBmp = (HBITMAP)SelectObject(m_dc, m_bmp);

   /* set the dimensions */
   m_width = 7;
   m_height = 7;

   /* TEST - clear the bitmap */
   HBRUSH clrBrush = CreateSolidBrush( RGB(200,20,20) );
   RECT bmpRect;
   SetRect(&bmpRect, 0, 0, m_width, m_height);
   FillRect(m_dc, &bmpRect, clrBrush);
   DeleteObject(clrBrush);

   /* success */
   return(true);

#endif

}

/* allocator */
WinBitmap* WinBitmap::allocate(const byte* dataBuf, uint32 nbrBytes)
{
   /* create the new object */
   WinBitmap* newObj = new WinBitmap();
   if(newObj == NULL) {
      return(NULL);
   }
   
   /* do second-phase construction */
   if(!newObj->construct(dataBuf, nbrBytes)) {
      /* delete the allocated object */
      delete newObj;
      return(NULL);
   }

   /* return the newly allocated object */
   return(newObj);
}

/* destructor */
WinBitmap::~WinBitmap()
{
   /* delete the mask bitmap */
   if(m_mask) {
      DeleteObject(m_mask);
   }
   if(m_bmp) {
      /* restore the original bitmap */
      SelectObject(m_dc, m_oldBmp);
      /* delete the DC */
      DeleteDC(m_dc);
      /* delete our bitmap */
      DeleteObject(m_bmp);
   }
   if(m_dc) {
      /* delete the DC */
      DeleteDC(m_dc);
   }
}

};
