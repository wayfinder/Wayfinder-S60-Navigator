/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WINBITMAP_H
#define WINBITMAP_H

#include <windows.h>
#include <vector>

#include "MapPlotter.h"

namespace isab {

/* header for a TGA (TARGA) File */
typedef struct
{
   uint8 numCharsID;
   uint8 colorMapType;
   uint8 imageType;
   uint8 colorMapSpec[5];
   uint16 xOrig;
   uint16 yOrig;
   uint16 imageWidth;
   uint16 imageHeight;
   uint8 bitsPerPixel;
   uint8 imageDesc;
} TGAHEADER;

typedef struct
{
   uint8 red;
   uint8 green;
   uint8 blue;
} PIXEL;

class WinBitmap : public BitMap
{
   /** data **/
   private:
      /* the bitmap */
      HBITMAP m_bmp;
      /* the mask */
      HBITMAP m_mask;

      /* the old bitmap from the DC */
      HBITMAP m_oldBmp;

      /* it's DC */
      HDC m_dc;

      /* dimensions of bitmap */
      int32 m_width, m_height;

      /* flag which indicates whether the BMP is maked or not */
      bool m_isMasked;
     
      /// The color map.
      vector<PIXEL> m_colorMap;

   /** methods **/
   private:
      /* private constructor */
      WinBitmap();

      /* second-phase constrcutor */
      bool construct(const byte* dataBuf, uint32 nbrBytes);

      /**
       *    Help method.
       *    Reads the color map data if any is present.
       *    @param   tgaHeader   The tga header.
       *    @param   colorData   Pointer to the color map data.
       *    @return  How much colormap data that has been read, i.e.
       *             how much the colorData should be increased in order
       *             to point at the color data instead of at the color 
       *             map.
       */
      int readColorMapData( const TGAHEADER& tgaHeader, 
                            const byte* colorData );

   public:
      /* allocator */
      static WinBitmap* allocate(const byte* dataBuf, uint32 nbrBytes);

      /* destructor */
      ~WinBitmap();

      /* gets width of bitmap */
      int32 width() const
      {
         return(m_width);
      }

      /* gets height of bitmap */
      int32 height() const
      {
         return(m_height);
      }

      /* gets the handle to the DC */
      HDC getDC() const
      {
         return(m_dc);
      }

      /* gets the mask bitmap */
      HBITMAP getMask() const
      {
         return(m_mask);
      }

      /* returns true, if the image is masked, false otherwise */
      bool isMasked() const
      {
         return(m_isMasked);
      }
};

};

#endif
