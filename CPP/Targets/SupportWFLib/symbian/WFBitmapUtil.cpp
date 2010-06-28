/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "WFBitmapUtil.h"
#include <fbs.h>
#include <bitdev.h>
#include <bitstd.h>
#include <string.h>

void WFBitmapUtil::FlipImageL(class CFbsBitmap* aBitmap)
{
   if (!aBitmap) {
      return;
   }
   aBitmap->LockHeap();
   // Get the adress to the bitmap
   TUint32* pCurrScanLine = aBitmap->DataAddress();
   TSize imageSize = aBitmap->SizeInPixels();
   TInt stride = CFbsBitmap::ScanLineLength(imageSize.iWidth, 
                                            aBitmap->DisplayMode());
   // Nbr of bytes per pixel
   TInt nbrBytesPixel = stride / imageSize.iWidth;   

   for(TInt y = 0; y < imageSize.iHeight; y++) {
      // for each scanline
      for(TInt x = 0; x < stride / 2; x+=nbrBytesPixel)  {
         // for each byte in the scanline
         // Get the pointer to the first value in the scanline
         TUint8* pixelStart = (TUint8*)pCurrScanLine;
         TUint8* pixelEnd = pixelStart;
         // Incremeant with the counter, so that we are working with
         // one pixel at a time.
         pixelStart += x;
         // Find the pixel from the end that corresponds with the
         // index from the beginning, eg last pixel/first pixel.
         // Note that pixel here is note the same thing as a byte,
         // the last pixel, if there is two bytes per pixel is
         // last element - nbr of bytes per pixel. 
         pixelEnd += stride - x - nbrBytesPixel;
         for (TInt p = 0; p < nbrBytesPixel; p++) {
            // Switch places on the pixels, first byte in the first
            // pixel trades place with the first byte in the last
            // pixel and so on.
            pixelStart += p;
            pixelEnd += p;
            TUint8 tmp  = *pixelStart;
            *pixelStart = *pixelEnd;
            *pixelEnd = tmp;
         }
      }
      // typecast to avoid pesky pointer aritmetics.
      pCurrScanLine = (TUint32*)((TUint8*)pCurrScanLine + stride); 
   }
   aBitmap->UnlockHeap();
}
   
void WFBitmapUtil::FilterImage(class CFbsBitmap* aBitmap,
                               TBool aInvertRed)
{
   if (!aBitmap) {
      return;
   }
   aBitmap->LockHeap();
   // Get the adress to the bitmap
   TUint32* pCurrScanLine = aBitmap->DataAddress();
   TSize imageSize = aBitmap->SizeInPixels();
   TDisplayMode mode = aBitmap->DisplayMode();
   TInt stride = CFbsBitmap::ScanLineLength(imageSize.iWidth, 
                                            aBitmap->DisplayMode());
   // Nbr of bytes per pixel
   TInt nbrBytesPixel = stride / imageSize.iWidth;   

   for(TInt y = 0; y < imageSize.iHeight; y++) {
      // for each scanline
      for(TInt x = 0; x < stride; x+=nbrBytesPixel)  {
         // for each byte in the scanline
         // Get the pointer to the first value in the scanline
         TUint8* pixel = (TUint8*)pCurrScanLine;
         // Incremeant with the counter, so that we are working with
         // one pixel at a time.
         pixel += x;
         FilterImageInternal(pixel, mode, aInvertRed);
      }
      // typecast to avoid pesky pointer aritmetics.
      pCurrScanLine = (TUint32*)((TUint8*)pCurrScanLine + stride); 
   }
   aBitmap->UnlockHeap();
}
   
void WFBitmapUtil::AlfaBlendImage(class CFbsBitmap* /*aUnderlayingImage*/,
                                  class CFbsBitmap* /*aOverlayingImage*/)
{
   
}

void WFBitmapUtil::InvertMaskL(class CFbsBitmap* aMask)
{
   TSize imageSize = aMask->SizeInPixels();
   TInt stride = CFbsBitmap::ScanLineLength(imageSize.iWidth, aMask->DisplayMode());
   aMask->LockHeap();
   TUint32* pCurrScanLine = aMask->DataAddress();   
   for (TInt y = 0; y < imageSize.iHeight; y++) {
      // for each scanline
      for (TInt x = 0; x < stride; x++) {
         TUint8* byte = (TUint8*)pCurrScanLine + x;
         // Invert the byte
         *byte = ~(*byte);
      }
      pCurrScanLine = (TUint32*)((TUint8*)pCurrScanLine + stride);
   }
   aMask->UnlockHeap();
}

void WFBitmapUtil::FilterImageInternal(TUint8* aPixel, 
                                       TDisplayMode aDisplayMode,
                                        TBool aInvertRed)
{
   switch (aDisplayMode) 
      {
      case EColor64K: 
         {
            if (aInvertRed) {
               *(aPixel+1) = ~(*(aPixel+1));
            }
            *aPixel     &= 0x0;
            *(aPixel+1) &= 0xf8; 
         }
         break;
      case EColor16M:
#ifdef NAV2_CLIENT_SERIES60_V3
      case EColor16MU:
      case EColor16MA:
#endif
         {
            // The bytes are in the order BGR
            *aPixel = 0;
            *(aPixel+1) = 0;
            if (aInvertRed) {
               *(aPixel+2) = ~(*(aPixel+2));
            }
         }
         break;
      default:
         {
         }
         // Do nothing!
      }   
}

class CFbsBitmap* WFBitmapUtil::CopyBitmapL(class CFbsBitmap* aBitmap)
{
   if (!aBitmap) {
      return NULL;
   }
   class CFbsBitmap* copiedBitmap = new (ELeave) CFbsBitmap();
   copiedBitmap->Create(aBitmap->SizeInPixels(), aBitmap->DisplayMode());

   CFbsBitmapDevice* fbsdev = CFbsBitmapDevice::NewL(copiedBitmap);
   CleanupStack::PushL(fbsdev);
   CFbsBitGc* fbsgc = CFbsBitGc::NewL();
   CleanupStack::PushL(fbsgc);
   fbsgc->Activate(fbsdev);
   fbsgc->BitBlt(TPoint(0,0),aBitmap);
   CleanupStack::PopAndDestroy(2);
   return copiedBitmap;
}
