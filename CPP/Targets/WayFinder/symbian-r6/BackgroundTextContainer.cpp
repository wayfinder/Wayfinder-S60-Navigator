/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "BackgroundTextContainer.h"
#include <gdi.h>
#include <aknsutils.h> 
#include <gulcolor.h> 
#include <aknutils.h> 
#include <w32std.h>
#include "DrawComponent.h"

CBackGroundTextContainer* CBackGroundTextContainer::NewLC(CCoeControl* aParent,
                                                          const TRect& aRect,
                                                          const TDesC& aText,
                                                          const TRgb& aBgColor,
                                                          const TRgb& aNightModeBgColor,
                                                          const TRgb& aTextColor,
                                                          const TRgb& aNightModeTextColor)
{
   CBackGroundTextContainer* self = new (ELeave) 
      CBackGroundTextContainer(aBgColor, aNightModeBgColor);
   CleanupStack::PushL(self);
   self->ConstructL(aParent, aRect, aText, aTextColor, aNightModeTextColor);
   return self;
}

CBackGroundTextContainer* CBackGroundTextContainer::NewL(CCoeControl* aParent,
                                                         const TRect& aRect,
                                                         const TDesC& aText,
                                                         const TRgb& aBgColor,
                                                         const TRgb& aNightModeBgColor,
                                                         const TRgb& aTextColor,
                                                         const TRgb& aNightModeTextColor)
{
   CBackGroundTextContainer* self = CBackGroundTextContainer::NewLC(aParent,
                                                                    aRect,
                                                                    aText,
                                                                    aBgColor,
                                                                    aNightModeBgColor,
                                                                    aTextColor,
                                                                    aNightModeTextColor);
   CleanupStack::Pop(self);
   return self;
}

void CBackGroundTextContainer::ConstructL(CCoeControl* aParent,
                                          const TRect& aRect,
                                          const TDesC& aText,
                                          const TRgb& aTextColor,
                                          const TRgb& aNightModeTextColor)
{   
   if (!aParent) {
      // Create a window for this control
      CreateWindowL();
   } else {
      // We got an valid parent, we should not be window owning
      SetContainerWindowL(*aParent);      
   }
   InitComponentArrayL();
   iTextColor = aTextColor;
   iNightModeTextColor = aNightModeTextColor;
   // Create the distance control and add it to the CCoeControlArray
   iLabel = CDrawComponent::NewL(this, aTextColor, CGraphicsContext::ECenter);
   Components().AppendLC(iLabel);
   CleanupStack::Pop(iLabel);

   SetRect(aRect);

   if (!aParent) {
      // Activate the window, which makes it ready to be drawn
      ActivateL();
   }
}

CBackGroundTextContainer::CBackGroundTextContainer(const TRgb& aBgColor, const TRgb& aNightModeBgColor) :
   iBgColor(aBgColor),
   iNightModeBgColor(aNightModeBgColor),
   iNightMode(EFalse)
{
   
}

CBackGroundTextContainer::~CBackGroundTextContainer()
{
}

void CBackGroundTextContainer::SizeChanged()
{
   // Get the complete rect of this control
   TRect cmplRect = Rect();
   // The padding (top and bottom) should be a fifth
   // of this controls rect.
   TInt padding = cmplRect.Height() / 5;
   // The rect of this control minus the padding * 2
   // (top and bottom) is the desired font height.
   TInt maxFontHeight = cmplRect.Height() - padding * 2;
   // Get the best font that fits within the given space
   CFont* fontToUse = GetFontL(maxFontHeight);
   iLabel->SetFont(fontToUse);
   // First calc the width and height of the label.
   TInt fontHeight = iLabel->Font()->HeightInPixels();
   TInt descent = iLabel->Font()->DescentInPixels();
   TInt labelHeight = fontHeight + descent;
   // Calc the top y coord. It should be the height of this rect / 2
   // - the height of the label. This gives us the baseline of the
   // text in the middle.
   TInt topY = (cmplRect.iBr.iY - (cmplRect.Height() >> 1)) - 
      (labelHeight >> 1);
   // Create the top left point of the label given the topY and leftX
   TPoint labelTl(cmplRect.iTl.iX, topY);
   // Create the size of the label giben the width and height of the 
   // label.
   TSize labelSize(cmplRect.Width(), labelHeight);
   // Create the rect for the label
   TRect labelRect(labelTl, labelSize);
   // Calculate the baselineOffset
   TInt baselineOffset = (labelRect.Height() >> 1) + 
      ((fontToUse->AscentInPixels() >> 1));

   //release the font
   CEikonEnv::Static()->ScreenDevice()->ReleaseFont(fontToUse);

   iLabel->SetBaselineOffset(baselineOffset);
   // Set the rect of the label using the top left point and the width
   // of the label.
   iLabel->SetRect(labelRect);
   ReAlignText();
}

void CBackGroundTextContainer::Draw(const TRect& /*aRect*/) const
{
   // Get the standard graphics context
   CWindowGc& gc = SystemGc();
   TRect rect(Rect());
   gc.SetClippingRect(rect);
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenSize(TSize(0, 0));
   if(iNightMode) {
      gc.SetBrushColor(iNightModeBgColor);
   }
   else {
      gc.SetBrushColor(iBgColor);
   }
   gc.DrawRect(rect); 


   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenSize(TSize(1, 1));

   //draw top line
   if(iDrawBorderEdgesFlag & ETopEdge) {
      gc.DrawLine(rect.iTl, TPoint(rect.iBr.iX, rect.iTl.iY));
   }
   //draw right line
   if(iDrawBorderEdgesFlag & ERightEdge) {
      gc.DrawLine(TPoint(rect.iBr.iX - 1, rect.iTl.iY), TPoint(rect.iBr.iX - 1, rect.iBr.iY));
   }
   //draw bottom line
   if(iDrawBorderEdgesFlag & EBottomEdge) {
      gc.DrawLine(TPoint(rect.iTl.iX, rect.iBr.iY - 1), TPoint(rect.iBr.iX, rect.iBr.iY - 1));
   }
   //draw left line
   if(iDrawBorderEdgesFlag & ELeftEdge) {
      gc.DrawLine(rect.iTl, TPoint(rect.iTl.iX, rect.iBr.iY));
   }
}

void CBackGroundTextContainer::SetTextL(const TDesC& aText)
{
   iLabel->SetTextL(aText);
   ReAlignText();
   iLabel->DrawDeferred();
}

TPtrC CBackGroundTextContainer::Text() const
{
   return iLabel->Text();
}

CFont* CBackGroundTextContainer::GetFontL(TInt aMaxFontHeight)
{
   // Get a font with a good fontspec that we can copy
   const CFont* font = AknLayoutUtils::
      FontFromId(EAknLogicalFontPrimarySmallFont);
   // Get the fontspec
   TFontSpec fontSpec = font->FontSpecInTwips();

   // Get the screen device so that we can calc the twips
   // per pixel
   TPixelsTwipsAndRotation twips;
   CWsScreenDevice* screenDev = CEikonEnv::Static()->ScreenDevice();
   screenDev->GetScreenModeSizeAndRotation(screenDev->CurrentScreenMode(),
                                           twips);

   // Calc the twips per pixel
   TReal twipsPerPixel = twips.iTwipsSize.iHeight / 
      TReal(twips.iPixelSize.iHeight);

   // Set the desired height of the font
   fontSpec.iHeight = TInt(twipsPerPixel * aMaxFontHeight);
   // Font should be bold
   fontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
   // Font should be antialiased
   fontSpec.iFontStyle.SetBitmapType(EAntiAliasedGlyphBitmap);
   // Get the font that matches the fontspec the best
   CFont* fontToUse;
   screenDev->
      GetNearestFontToDesignHeightInTwips(fontToUse, fontSpec);
   // Return the font, the caller has to release the font
   // from the CWsScreenDevice
   return fontToUse;
}

void CBackGroundTextContainer::SetBorderEdges(TInt aBorderEdgesFlags)
{
   iDrawBorderEdgesFlag = aBorderEdgesFlags;
}

void CBackGroundTextContainer::SetNightModeL(TBool aNightMode)
{
   iNightMode = aNightMode;
   if(aNightMode) {
      iLabel->SetTextColor(iNightModeTextColor);
   }
   else {
      iLabel->SetTextColor(iTextColor);
   }
   DrawDeferred();
}

void CBackGroundTextContainer::ReAlignText()
{
   if(iLabel) {
      const CFont* font = iLabel->Font();
      TPtrC text = iLabel->Text();
      if(font) {
         if(font->TextWidthInPixels(text) > Rect().Width()) {
            iLabel->SetAlignment(CGraphicsContext::ELeft);
         }
         else {
            iLabel->SetAlignment(CGraphicsContext::ECenter);
         }
      }
   }
}
