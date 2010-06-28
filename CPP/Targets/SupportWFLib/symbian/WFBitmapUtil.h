/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WF_BITMAP_UTIL_H
#define WF_BITMAP_UTIL_H

#include <e32def.h>
#ifdef SYMBIAN_9
# include <e32const.h>
#endif
#include <gdi.h>

/**
 * Class that handles and manipulates an image. The thought
 * is that we can put all code that handles image manipulation
 * here in a common class that can be used by all symbian 
 * platforms. One thing that should be done (soon) is to put
 * all the calculation code out to a common class that can be used
 * by windows mobile and other platforms as well. 
 */
class WFBitmapUtil
{
 public: 

   /**
    * Flips an image, horizontaly
    * @param aImage, image to flip.
    */
   static void FlipImageL(class CFbsBitmap* aBitmap);
   
   /**
    * Used for night mode, puts a red filter
    * above the image.
    * @param aBitmap, the image to be filtered.
    * @param aInvertRed, true if we want to invert the
    *                    red value.
    */
   static void FilterImage(class CFbsBitmap* aBitmap,
                           TBool aInvertRed = EFalse);
   
   
   /**
    * Should be implemented soon.
    */
   static void AlfaBlendImage(class CFbsBitmap* aUnderlayingImage,
                              class CFbsBitmap* aOverlayingImage);

   /**
    * Copys aOrgBitmap into aNewBitmap using memcopy.
    */
   static void CopyBitmapL(class CFbsBitmap* aNewBitmap,
                           class CFbsBitmap* aOrgBitmap);
   
   /**
    * Inverts a mask, could be usefull when a mask needs to
    * be inverted but bitbltmasked not can be used.
    * @param aMask image to be inverted.
    */
   void InvertMaskL(class CFbsBitmap* aMask);

   /**
    * Copies a bitmap and returns the copy.
    * NOTE: caller takes over ownership of the copied bitmap!
    * @param aBitmap the bitmap to be copied.
    */
   static class CFbsBitmap* CopyBitmapL(class CFbsBitmap* aBitmap);


 private:

   /**
    * Used internal when filtering an image,
    * different color depth has different nbr of
    * bytes per pixel. 
    * TODO: Write general code that works for all
    *       color depths, not a swith/case.
    */
   static void FilterImageInternal(TUint8* aPixel, 
                                   TDisplayMode aDisplayMode,
                                   TBool aInvertRed);
};

#endif
