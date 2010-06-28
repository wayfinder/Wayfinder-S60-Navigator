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

#if !(defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3)
// INCLUDE FILES
#include "InfoCompass.h"

#ifdef NAV2_CLIENT_SERIES60_V3
#include <akniconutils.h>
#endif
#include <e32math.h>
#include <math.h>
#include "RectTools.h"
#include "Constants.h"

#define KPI isab::Constants::pi
#ifdef _MSC_VER
# define FLOOR(d) TInt(floor(d))
#else
# define FLOOR(d) TInt(d)
#endif

class CInfoCompass* CInfoCompass::NewL( class CCoeControl& aParent, 
                                        const TRect& aRect, 
                                        const TDesC& aMbmFile,
                                        TInt aCompassImage,
                                        TInt aCompassMask,
                                        TBool aInvertMask,
                                        TRgb aNorthArrowColor,
                                        TRgb aSouthArrowColor)
{
   class CInfoCompass* self = new (ELeave) CInfoCompass();
   CleanupStack::PushL(self);
   self->ConstructL(aParent, aRect, aMbmFile, 
                    aCompassImage, aCompassMask, 
                    aInvertMask, aNorthArrowColor, aSouthArrowColor);
   CleanupStack::Pop(self);
   return self;
}

CInfoCompass::CInfoCompass() : 
   iHeading(-1),
   iMargin(0.24)
{
}

void CInfoCompass::ConstructL( class CCoeControl& aParent, 
                               const TRect& aRect, const TDesC& aMbmFile, 
                               TInt aCompassImage, TInt aCompassMask,
                               TBool aInvertMask, TRgb aNorthArrowColor, 
                               TRgb aSouthArrowColor)
{
   iInvertMask = aInvertMask;

   iNorthArrowColor = aNorthArrowColor;
   iSouthArrawColor = aSouthArrowColor;

   SetContainerWindowL(aParent);
#ifdef NAV2_CLIENT_SERIES60_V3
   AknIconUtils::CreateIconL(iCompassImage, 
                              iCompassImage_mask, 
                              aMbmFile, 
                              aCompassImage,
                              aCompassMask);

   // The size is based on compass75.bmps size in pixels
   // in relation to the screen size.
   AknIconUtils::SetSize(iCompassImage, aRect.Size(), EAspectRatioPreservedAndUnusedSpaceRemoved);
//    AknIconUtils::SetSize(iCompassImage_mask, aRect.Size());
//    TRect rect(TPoint(tlX, aRect.iTl.iX), iCompassImage->SizeInPixels());
   TRect rect(aRect.iTl, iCompassImage->SizeInPixels());
   SetRect(rect);
#else
   iCompassImage = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iCompassImage->Load(aMbmFile, aCompassImage));

   iCompassImage_mask = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iCompassImage_mask->Load(aMbmFile, aCompassMask));
   SetRect(aRect);
#endif
   ActivateL();
}

TSize CInfoCompass::MinimumSize()
{
   if(iCompassImage){
      return iCompassImage->SizeInPixels();
   } else {
      return TSize();
   }
}

void CInfoCompass::SizeChanged()
{
   UpdatePoints();
}

CInfoCompass::~CInfoCompass()
{
   delete iCompassImage_mask;
   delete iCompassImage;
}

void CInfoCompass::UpdatePoints()
{
   TRect rect = Rect();
   /* Make pointer length 24% less than half height. */
   iLength = Min(rect.Width(), FLOOR((rect.Height())/2 * (1-iMargin)));
   if (iHeading == -1) {
      Clear();
   } else {
      SetHeading(iHeading, ETrue);
   }
}

void CInfoCompass::SetHeading( TInt aHeading, TBool aForceUpdate )
{
   if(aForceUpdate || iHeading != aHeading){
      iHeading = aHeading;

      if( aHeading == -1 ){ //TODO: remove this special case. 
         Clear();
      }else{
         TReal radHead = (aHeading*KPI)/180;
         SetHeading(radHead);
      }
   }
}

void CInfoCompass::SetHeading( TUint8 aHeading )
{
   SetHeading((aHeading*KPI)/128);
}

void CInfoCompass::SetHeading( TReal aHeading )
{
   TReal sin;
   TReal cos;
   Math::Sin( sin, aHeading );
   Math::Cos( cos, aHeading );
   
   TPoint newHead = //the y coord is inverted compared to the math sense.
      TPoint(FLOOR((iLength * sin) +0.5), 
             -FLOOR(((iLength * cos) +0.5))) + Rect().Center();
   SwitchHead(newHead);
}

void CInfoCompass::Clear()
{
   SwitchHead(Rect().Center());
}

void CInfoCompass::SwitchHead(TPoint aNewHead)
{
   if(iHeadPoint != aNewHead){
      TPoint center = Rect().Center();
      TPoint reverseHead = center - (iHeadPoint - center);
      TPoint reverseNew = center - (aNewHead - center);
      if(IsVisible()){
         Window().Invalidate(Shrink(TRect(reverseNew, aNewHead), -(iLength>>2)));
         Window().Invalidate(Shrink(TRect(reverseHead, iHeadPoint), -(iLength>>2)));
      }
      iHeadPoint = aNewHead;
   }
}

void CInfoCompass::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   gc.SetClippingRect(aRect);
   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenSize(TSize(1,1));

   TPoint center  = Rect().Center();
   TPoint newHead = iHeadPoint - center;
   TPoint revHead = TPoint( -newHead.iX/2, -newHead.iY/2 ) + center;
   TPoint side1   = TPoint( -(newHead.iY >> 2), (newHead.iX >> 2)) + center;
   TPoint side2   = TPoint( (newHead.iY >> 2), -(newHead.iX >> 2)) + center;
   newHead += center;

   CArrayFix<TPoint>* north = new CArrayFixFlat<TPoint>(3);
   CArrayFix<TPoint>* south = new CArrayFixFlat<TPoint>(3);

   CleanupStack::PushL(north);
   CleanupStack::PushL(south);

   north->AppendL(side1);
   north->AppendL(newHead);
   north->AppendL(side2);

   south->AppendL(side1);
   south->AppendL(revHead);
   south->AppendL(side2);

   TRect compRect(TPoint(0,0), iCompassImage->SizeInPixels());

   gc.BitBltMasked(Rect().iTl, iCompassImage, compRect, 
                   iCompassImage_mask, iInvertMask);

   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.SetBrushColor(iNorthArrowColor);
   gc.DrawPolygon(north, CGraphicsContext::EWinding);
   gc.SetBrushColor(iSouthArrawColor);
   gc.DrawPolygon(south, CGraphicsContext::EWinding);

   gc.SetBrushStyle(CGraphicsContext::ENullBrush);
   gc.DrawLine(center, newHead);

   CleanupStack::PopAndDestroy(south);
   CleanupStack::PopAndDestroy(north);
}

void CInfoCompass::SetCompassSizeAndPos(const TRect& aRect)
{
#if defined NAV2_CLIENT_SERIES60_V3
   if (iCompassImage) {
      AknIconUtils::SetSize(iCompassImage, aRect.Size(), EAspectRatioPreservedAndUnusedSpaceRemoved);
   }
#endif
   SetRect(aRect);
}

void CInfoCompass::SetNewImage(const TRect& aRect, 
                               const TDesC& aMbmFile, 
                               TInt  aCompassImage,
                               TInt  aCompassMask,
                               TBool aInvertMask,
                               TRgb  aNorthArrowColor,
                               TRgb  aSouthArrowColor)
{
   iNorthArrowColor = aNorthArrowColor;
   iSouthArrawColor = aSouthArrowColor;
#ifdef NAV2_CLIENT_SERIES60_V3
   AknIconUtils::CreateIconL(iCompassImage, 
                             iCompassImage_mask, 
                             aMbmFile, 
                             aCompassImage,
                             aCompassMask);

   // The size is based on compass75.bmps size in pixels
   // in relation to the screen size.
   AknIconUtils::SetSize(iCompassImage, aRect.Size(), EAspectRatioPreservedAndUnusedSpaceRemoved);
   TRect rect(aRect.iTl, iCompassImage->SizeInPixels());
   SetRect(rect);
#else
   delete iCompassImage;
   delete iCompassImage_mask;
   iCompassImage = NULL;
   iCompassImage_mask = NULL;

   iCompassImage = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iCompassImage->Load(aMbmFile, aCompassImage));

   iCompassImage_mask = new (ELeave) CFbsBitmap();
   User::LeaveIfError(iCompassImage_mask->Load(aMbmFile, aCompassMask));
   SetRect(aRect);   
#endif
}
// End of File  
#endif
