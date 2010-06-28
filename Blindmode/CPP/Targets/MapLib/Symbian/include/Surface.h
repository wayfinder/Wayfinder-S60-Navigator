/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Surface class for bitmap surfaces for rendering
// usuable for backbuffers, etc.
// uses two-phase construction
// width, height and color-depth selectable

#ifndef __SURFACE_H__
#define __SURFACE_H__

#include <e32std.h>
#include <e32base.h>
#include <fbs.h>      // for CFbsBitmap
#include <bitstd.h>      // for CFbsBitGc
#include <bitdev.h>      // for CFbsBitmapDevice
#include <w32std.h>      // for CWindowGc

class CSurface : public CBase
{
   private:
      CWsBitmap* iBitmap;
      CFbsBitmapDevice* iDevice;
      CFbsBitGc* iGc;
   protected:
      // protected constructor
      CSurface();
      // protected second-phase contsructor
      void ConstructL(RWsSession& arWs, TInt aW, TInt aH, TInt aBpp);
   public:
      // static creation methods
      static CSurface* NewLC(RWsSession& arWs, TInt aW, TInt aH, TInt aBpp);
      // static creation methods
      static CSurface* NewL(RWsSession& arWs, TInt aW, TInt aH, TInt aBpp);
      // destructor
      ~CSurface();
      // returns the device, gc, etc.
      inline CWsBitmap& Bitmap() const { return *iBitmap; }
      inline CFbsBitGc& Gc() const { return *iGc; }
      inline CFbsBitmapDevice& Device() const { return *iDevice; }
      
      // blitting routines
      void BlitTo(CSurface& aDest, TInt aX, TInt aY);
      void BlitTo(CWindowGc& aDest, TInt aX, TInt aY);
      void BlitTo(CFbsBitGc& aDest, TInt aX, TInt aY);
      // clearing
      void Clear(TInt r=0, TInt g=0, TInt b=0);
      // get width and height of surface
      TInt Width() const;
      TInt Height() const;
      // read and set a pixel
      TRgb ReadPixel(TInt aX, TInt aY);
      void WritePixel(TInt aX, TInt aY, TRgb aColor);

      /* resizes the surface to the specified size */
      /* returns true, if resize was successful, false otherwise */
      bool Resize(TInt aNewWidth, TInt aNewHeight);
      
      /* function which clear the rectangular surface*/
      void ClearRectangle(TRect aRect,TInt aRed=0, TInt aGreen=0, TInt aBlue=0);
      
      /* function which clear the rectangular surface*/
      void ClearRectangle(TRect aRect, TRgb aColor);

};

#endif

