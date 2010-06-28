/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "DrawComponent.h"

#include <eikenv.h>

CDrawComponent* CDrawComponent::NewLC(CCoeControl* aParent,
                                      TRect aDrawRect,
                                      const TRgb& aTextColor,
                                      TInt aBaselineOffset,
                                      const TDesC& aText,
                                      CGraphicsContext::TTextAlign aAlign)
{
   CDrawComponent* self = new (ELeave) CDrawComponent(aBaselineOffset,
                                                      aTextColor,
                                                      aAlign);
   CleanupStack::PushL(self);
   self->ConstructL(aParent, aDrawRect, aText);
   return self;
}

CDrawComponent* CDrawComponent::NewL(CCoeControl* aParent,
                                     TRect aDrawRect,
                                     const TRgb& aTextColor,
                                     TInt aBaselineOffset,
                                     const TDesC& aText,
                                     CGraphicsContext::TTextAlign aAlign)
{
   CDrawComponent* self = CDrawComponent::NewLC(aParent,
                                                aDrawRect,
                                                aTextColor,
                                                aBaselineOffset,
                                                aText,
                                                aAlign);
   CleanupStack::Pop(self);
   return self;
}

CDrawComponent* CDrawComponent::NewLC(CCoeControl* aParent,
                                      const TRgb& aTextColor,
                                      CGraphicsContext::TTextAlign aAlign)
{
   CDrawComponent* self = new (ELeave) CDrawComponent(aTextColor,
                                                      aAlign);
   CleanupStack::PushL(self);
   self->ConstructL(aParent);
   return self;
}

CDrawComponent* CDrawComponent::NewL(CCoeControl* aParent,
                                     const TRgb& aTextColor,
                                     CGraphicsContext::TTextAlign aAlign)
{
   CDrawComponent* self = CDrawComponent::NewLC(aParent, 
                                                aTextColor,
                                                aAlign);
   CleanupStack::Pop(self);
   return self;
}

CDrawComponent::CDrawComponent(TInt aBaselineOffset,
                               const TRgb& aTextColor,
                               CGraphicsContext::TTextAlign aAlign) :
   iBaselineOffset(aBaselineOffset),
   iTextColor(aTextColor.Internal(), aTextColor.Alpha()),
   iAlign(aAlign)
{
   
}

CDrawComponent::CDrawComponent(const TRgb& aTextColor,
                               CGraphicsContext::TTextAlign aAlign) :
   iBaselineOffset(0),
   iTextColor(aTextColor.Internal(), aTextColor.Alpha()),
   iAlign(aAlign)
{
}

CDrawComponent::~CDrawComponent()
{
   delete iText;
}

void CDrawComponent::ConstructL(CCoeControl* aParent,
                                TRect& aDrawRect,
                                const TDesC& aText)
{
   if (!aParent) {
      CreateWindowL();
   } else {
      SetContainerWindowL(*aParent);
   }
                 
   iText = aText.AllocL();

   SetRect(aDrawRect);

   if (!aParent) {
      ActivateL();
   }
}

void CDrawComponent::ConstructL(CCoeControl* aParent)
{
   if (!aParent) {
      CreateWindowL();
   } else {
      SetContainerWindowL(*aParent);
   }
   iText = KNullDesC().AllocL();

   TRect rect;
   SetRect(rect);
   if (!aParent) {
      ActivateL();
   }
}

void CDrawComponent::SizeChanged()
{
}

void CDrawComponent::Draw(const TRect& /*aRect*/) const
{
   // Get the standard graphics context
   CWindowGc& gc = SystemGc();
   TRect rect(Rect());
   gc.SetClippingRect(rect);
   gc.SetBrushStyle(CGraphicsContext::ENullBrush);

   if (iFont) {
      gc.SetPenColor(iTextColor);
      gc.UseFont(iFont);
      gc.DrawText(*iText, rect, iBaselineOffset, iAlign);
      
      gc.DiscardFont();
   }

   //#define DRAW_BORDER_RECTS
#ifdef DRAW_BORDER_RECTS
   gc.SetBrushStyle(CGraphicsContext::ENullBrush);
   gc.SetPenColor(KRgbBlack);
   gc.DrawRect(rect);

   TInt centerY = rect.iTl.iY + (rect.Height() >> 1);
   gc.DrawLine(TPoint(rect.iTl.iX, centerY),
               TPoint(rect.iBr.iX, centerY));

   TInt fontTopY =  (rect.iTl.iY + iBaselineOffset) - iFont->HeightInPixels();
   gc.DrawLine(TPoint(rect.iTl.iX, fontTopY), TPoint(rect.iBr.iX, fontTopY));

   TInt baselineY = rect.iTl.iY + iBaselineOffset;
   gc.DrawLine(TPoint(rect.iTl.iX, baselineY), TPoint(rect.iBr.iX, baselineY));

#endif
}

TInt CDrawComponent::BaselineOffset() const
{
   return iBaselineOffset;
}
void CDrawComponent::SetBaselineOffset(TInt aBaselineOffset)
{
   iBaselineOffset = aBaselineOffset;
}

TPtrC CDrawComponent::Text() const
{
   return TPtrC(*iText);
}

void CDrawComponent::SetTextL(const TDesC& aText)
{
   delete iText;
   iText = NULL;
   iText = aText.AllocL();
}

const CFont* CDrawComponent::Font() const
{
   return iFont;
}

void CDrawComponent::SetFont(CFont* aFont, TBool aReleaseCurrentFont)
{
   if (iFont && aReleaseCurrentFont) {
      CEikonEnv::Static()->ScreenDevice()->ReleaseFont(iFont);
   }
   iFont = aFont;
}

void CDrawComponent::ReleaseFont()
{
   CEikonEnv::Static()->ScreenDevice()->ReleaseFont(iFont);
}

void CDrawComponent::SetTextColor(TRgb aTextColor)
{
   iTextColor = aTextColor;
}

void CDrawComponent::SetAlignment(CGraphicsContext::TTextAlign aAlign)
{
   iAlign = aAlign;
}

