/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "GuideProgressBar.h"
#include <stddef.h>
#ifdef NAV2_CLIENT_SERIES60_V3
#include <akniconutils.h>
#endif
#include "Surface.h"
#include "WFBitmapUtil.h"

const TInt KGPBSpace = 3;

CGuideProgressBar::CGuideProgressBar(CSurface* aBlitUiqSurface) :
   iUiqBuffer(aBlitUiqSurface)
{
}

void CGuideProgressBar::ConstructL(CCoeControl& aParent,
                                   const TRect& aRect,
                                   const TDesC& aMbmFile,
                                   TInt aTriangleImg,
                                   TInt aTriangleActiveImg,
                                   TInt aTriangleMask,
                                   TInt aRectImg,
                                   TInt aRectActiveImg,
                                   TInt aRectMask)
{
   SetContainerWindowL(aParent);

   TInt limits[] = {35, 60, 110, 210, 510, 1010, 1510, 2010};
   size_t numLimits = sizeof(limits)/sizeof(*limits);
   for (size_t a = 0; a < numLimits; ++a) {
      User::LeaveIfError(iLimits.Append(limits[a]));
   }

#ifdef NAV2_CLIENT_SERIES60_V3
   AknIconUtils::CreateIconL(iTriangleImg, iTriangleImg_mask, aMbmFile, 
                             aTriangleImg, aTriangleMask);
   AknIconUtils::CreateIconL(iRectImg, iRectImg_mask, aMbmFile,
                             aRectImg, aRectMask);
   iTriangleActiveImg = AknIconUtils::CreateIconL(aMbmFile, aTriangleActiveImg);
   iRectActiveImg     = AknIconUtils::CreateIconL(aMbmFile, aRectActiveImg);
   SetSizeAndPosition(aRect);
#else
   iTriangleImg = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iTriangleImg->Load(aMbmFile, aTriangleImg));

   iTriangleActiveImg = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iTriangleActiveImg->Load(aMbmFile, aTriangleActiveImg));

   iTriangleImg_mask = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iTriangleImg_mask->Load(aMbmFile, aTriangleMask));

   iRectImg = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iRectImg->Load(aMbmFile, aRectImg));

   iRectActiveImg = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iRectActiveImg->Load(aMbmFile, aRectActiveImg));

   iRectImg_mask = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iRectImg_mask->Load(aMbmFile, aRectMask));
#endif

   SetRect(aRect);
   ActivateL();
}

CGuideProgressBar::~CGuideProgressBar()
{
   delete iTriangleImg;
   delete iTriangleImg_mask;
   delete iRectImg;
   delete iRectImg_mask;
   iLimits.Close();
}

CGuideProgressBar* CGuideProgressBar::NewLC(CCoeControl& aParent,
                                            const TRect& aRect,
                                            const TDesC& aMbmFile,
                                            TInt aTriangleImg,
                                            TInt aTriangleActiveImg,
                                            TInt aTriangleMask,
                                            TInt aRectImg,
                                            TInt aRectActiveImg,
                                            TInt aRectMask,
                                            CSurface* aBlitUiqSurface)
{
   CGuideProgressBar* self = new (ELeave) CGuideProgressBar(aBlitUiqSurface);
   CleanupStack::PushL(self);
   self->ConstructL(aParent,
                    aRect,
                    aMbmFile,
                    aTriangleImg,
                    aTriangleActiveImg,
                    aTriangleMask,
                    aRectImg,
                    aRectActiveImg,
                    aRectMask);
   return self;
}

CGuideProgressBar* CGuideProgressBar::NewL(CCoeControl& aParent,
                                           const TRect& aRect,
                                           const TDesC& aMbmFile,
                                           TInt aTriangleImg,
                                           TInt aTriangleActiveImg,
                                           TInt aTriangleMask,
                                           TInt aRectImg,
                                           TInt aRectActiveImg,
                                           TInt aRectMask,
                                           CSurface* aBlitUiqSurface)
{
   CGuideProgressBar* self = CGuideProgressBar::NewLC(aParent,
                                                      aRect,
                                                      aMbmFile,
                                                      aTriangleImg,
                                                      aTriangleActiveImg,
                                                      aTriangleMask,
                                                      aRectImg,
                                                      aRectActiveImg,
                                                      aRectMask,
                                                      aBlitUiqSurface);
   CleanupStack::Pop(self);
   return self;
}

void CGuideProgressBar::SetSizeAndPosition(const TRect& aRect)
{
#if defined NAV2_CLIENT_SERIES60_V3
   AknIconUtils::SetSize(iTriangleImg, 
                         TSize(aRect.Size().iWidth,
                               (aRect.Size().iHeight / (iLimits.Count() + KGPBSpace))), 
                         EAspectRatioNotPreserved );
   AknIconUtils::SetSize(iRectImg, 
                         TSize(aRect.Size().iWidth,
                               (aRect.Size().iHeight / (iLimits.Count() + KGPBSpace))), 
                         EAspectRatioNotPreserved );
   AknIconUtils::SetSize(iTriangleActiveImg, 
                         TSize(aRect.Size().iWidth,
                               (aRect.Size().iHeight / (iLimits.Count() + KGPBSpace))), 
                         EAspectRatioNotPreserved );
   AknIconUtils::SetSize(iRectActiveImg, 
                         TSize(aRect.Size().iWidth,
                               (aRect.Size().iHeight / (iLimits.Count() + KGPBSpace))), 
                         EAspectRatioNotPreserved );  
#endif
   SetRect(aRect); 
}

void CGuideProgressBar::SetDistance(TInt aDistance)
{
   TRect imgRect(TPoint(0, 0), iRectImg->SizeInPixels());
   TPoint redrawStartPoint(-1, -1);
   TPoint redrawEndPoint(-1, -1);
   for (TInt i = 0; i < iLimits.Count(); i++) {
      if (((aDistance < iLimits[i]) && (iDistance >= iLimits[i])) ||
          ((aDistance > iLimits[i]) && (iDistance <= iLimits[i]))) {
         if (redrawStartPoint == TPoint(-1, -1)) {
            redrawStartPoint.iX = Rect().iTl.iX;
            redrawStartPoint.iY = Rect().iTl.iY + 
               (imgRect.Height() + KGPBSpace) * i;
         }
         redrawEndPoint.iX = Rect().iBr.iX;
         redrawEndPoint.iY = Rect().iTl.iY + 
            (imgRect.Height() + KGPBSpace) * (i + 1);
      }
   }
   if (redrawStartPoint != TPoint(-1, -1)) {
      Window().Invalidate(TRect(redrawStartPoint, redrawEndPoint));
   }
   iDistance = aDistance;
}

void CGuideProgressBar::Draw(const TRect& aRect) const
{
   if (!iUiqBuffer) {
      CWindowGc& gc = SystemGc();
      gc.SetClippingRect(aRect);

      TRect imgRect(TPoint(0,0), iRectImg->SizeInPixels());
      TRect redrawRect;
      TPoint drawStartPoint(Rect().iTl);
      for (TInt i = 0; i < iLimits.Count(); i++) {
         drawStartPoint.iY = Rect().iTl.iY + (imgRect.Height() + KGPBSpace) * i;
         redrawRect.SetRect(drawStartPoint, 
                            TPoint(drawStartPoint.iX + imgRect.Width(), 
                                   drawStartPoint.iY + imgRect.Height()));
         if (redrawRect.Intersects(aRect)) {
            if (iDistance < iLimits[i]) {
               if (i == 0) {
                  gc.BitBltMasked(drawStartPoint, iTriangleImg, imgRect, 
                                  iTriangleImg_mask, ETrue);
               }
               else {
                  gc.BitBltMasked(drawStartPoint, iRectImg, imgRect, 
                                  iRectImg_mask, ETrue);
               }
            }
            else {
               if (i == 0) {
                  gc.BitBltMasked(drawStartPoint, iTriangleActiveImg, imgRect, 
                                  iTriangleImg_mask, ETrue);
               }
               else {
                  gc.BitBltMasked(drawStartPoint, iRectActiveImg, imgRect, 
                                  iRectImg_mask, ETrue);
               }
            }
         }
      }
   }
   else {
      CFbsBitGc& gc = iUiqBuffer->Gc();
      gc.Clear(aRect);
      
      TRect imgRect(TPoint(0,0), iRectImg->SizeInPixels());
      TRect redrawRect;
      TPoint drawStartPoint(Rect().iTl);
      for (TInt i = 0; i < iLimits.Count(); i++) {
         drawStartPoint.iY = Rect().iTl.iY + (imgRect.Height() + KGPBSpace) * i;
         redrawRect.SetRect(drawStartPoint, 
                            TPoint(drawStartPoint.iX + imgRect.Width(), 
                                   drawStartPoint.iY + imgRect.Height()));
         if (redrawRect.Intersects(aRect)) {
            if (iDistance < iLimits[i]) {
               if (i == 0) {
                  gc.BitBltMasked(drawStartPoint, iTriangleImg, imgRect, 
                                  iTriangleImg_mask, ETrue);
               }
               else {
                  gc.BitBltMasked(drawStartPoint, iRectImg, imgRect, 
                                  iRectImg_mask, ETrue);
               }
            }
            else {
               if (i == 0) {
                  gc.BitBltMasked(drawStartPoint, iTriangleActiveImg, imgRect, 
                                  iTriangleImg_mask, ETrue);
               }
               else {
                  gc.BitBltMasked(drawStartPoint, iRectActiveImg, imgRect, 
                                  iRectImg_mask, ETrue);
               }
            }
         }
      }
   }
}

TInt CGuideProgressBar::GetDistance()
{
   return iDistance;
}


void CGuideProgressBar::SetNightMode()
{
   if( iTriangleImg ){
      WFBitmapUtil::FilterImage( iTriangleImg );
   }
   if( iTriangleActiveImg ){
      WFBitmapUtil::FilterImage( iTriangleActiveImg );
   }
   if( iRectImg ){
      WFBitmapUtil::FilterImage( iRectImg );
   }
   if( iRectActiveImg ){
      WFBitmapUtil::FilterImage( iRectActiveImg );
   }
}
