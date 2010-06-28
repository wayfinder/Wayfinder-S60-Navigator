/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Surface.h"

// constructor
CSurface::CSurface()
{
   iBitmap = NULL;
   iDevice = NULL;
   iGc = NULL;
}

// destructor
CSurface::~CSurface()
{
   delete iGc;
   delete iDevice;
   delete iBitmap;
}

// static creation methods
// creates object, calls second-phase, leaves on stack
CSurface* CSurface::NewLC(RWsSession& arWs, TInt aW, TInt aH, TInt aBpp)
{
   // allocate object on heap
   CSurface* newobj = new (ELeave) CSurface();
   // put on cleanup stack
   CleanupStack::PushL(newobj);
   // call second-phase constructor
   newobj->ConstructL(arWs, aW, aH, aBpp);
   return(newobj);
}

// creates object, calls second-phase, removes from stack
CSurface* CSurface::NewL(RWsSession& arWs, TInt aW, TInt aH, TInt aBpp)
{
   // call NewLC for allocation
   CSurface* newobj = CSurface::NewLC(arWs, aW, aH, aBpp);
   // if all success, pop off the stack
   CleanupStack::Pop(newobj);
   return(newobj);
}

// protected second-phase contsructor
void CSurface::ConstructL(RWsSession& arWs, TInt aW, TInt aH, TInt aBpp)
{
   // allocate, allocate and more allocate
   // first, the main bitmap; depending on the color depth specified
   TDisplayMode dispMode;
   switch(aBpp)
   {
      case 1:      // 1-bit monochrome bitmap (EGray2)
         dispMode = EGray2;
         break;
      case 4:
         dispMode = EColor16;
         break;
      case 8:
         dispMode = EColor256;
         break;
      case 12:
         dispMode = EColor4K;
         break;
      case 16:
         dispMode = EColor64K;
         break;
      case 24:
         dispMode = EColor16M;
         break;
      default:
         // Don't crash.
         dispMode = EColor4K;
         break;
   }
   
   // create the surface bitmap
   iBitmap = new(ELeave) CWsBitmap( arWs );
   //iBitmap = new(ELeave) CFbsBitmap( );
   //CleanupStack::PushL(iBitmap);
   
   User::LeaveIfError(iBitmap->Create(TSize(aW,aH), dispMode));
   // create the bitmap device
   iDevice = CFbsBitmapDevice::NewL(iBitmap);
   // create the graphics context for the device
   User::LeaveIfError(iDevice->CreateContext(iGc));   
}


// blitting routines
void CSurface::BlitTo(CSurface& aDest, TInt aX, TInt aY)
{
   aDest.Gc().BitBlt(TPoint(aX,aY), iBitmap);
   return;
}

void CSurface::BlitTo(CWindowGc& aDest, TInt aX, TInt aY)
{
   aDest.BitBlt(TPoint(aX,aY), iBitmap);
   return;
}

void CSurface::BlitTo(CFbsBitGc& aDest, TInt aX, TInt aY)
{
   aDest.BitBlt(TPoint(aX,aY), iBitmap);
   return;
}

/* resizes the surface to the specified size */
/* returns true, if resize was successful, false otherwise */
bool CSurface::Resize(TInt aNewWidth, TInt aNewHeight)
{
   /* try to resize the bitmap */
   if(iDevice->Resize(TSize(aNewWidth, aNewHeight)) != KErrNone) {
      /* couldn't resize, return false */
      return(false);
   }

   /* inform the GC of the resize */
   iGc->Resized();

   /* surface resize successful */
   return(true);
}

// clearing
void CSurface::Clear(TInt r, TInt g, TInt b)
{
   iGc->SetBrushColor(TRgb(r,g,b));
   iGc->SetBrushStyle(CGraphicsContext::ESolidBrush);
   iGc->Clear();
   return;
}

// get width and height of surface
TInt CSurface::Width() const
{
   return(iBitmap->SizeInPixels().iWidth);
}

TInt CSurface::Height() const
{
   return(iBitmap->SizeInPixels().iHeight);
}

// read and set a pixel
TRgb CSurface::ReadPixel(TInt aX, TInt aY)
{
   TRgb colVal(0xFFFFFFFF);
	iDevice->GetPixel(colVal, TPoint(aX,aY));
	return(colVal);
}

void CSurface::WritePixel(TInt aX, TInt aY, TRgb aColor)
{
	iGc->SetPenColor(aColor);
	iGc->Plot(TPoint(aX,aY));
	return;
}

void CSurface::ClearRectangle(TRect aRect, TInt aRed, TInt aGreen, TInt aBlue)
{
   ClearRectangle( aRect, TRgb(aRed,aGreen,aBlue) );
}

void CSurface::ClearRectangle(TRect aRect, TRgb aColor )
{
   iGc->SetBrushColor(aColor);
   iGc->SetBrushStyle(CGraphicsContext::ESolidBrush);
   iGc->Clear(aRect);
}

void CSurface::lockBitmapBuffer()
{
   iBitmap->LockHeap();
}

void CSurface::unlockBitmapBuffer()
{
   iBitmap->UnlockHeap();
}

TUint32* CSurface::getBitmapBuffer()
{
   return iBitmap->DataAddress();
}
