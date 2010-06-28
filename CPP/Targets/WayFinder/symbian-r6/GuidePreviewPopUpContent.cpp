/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32

// INCLUDE FILES
#include <coemain.h>
#include <coecontrolarray.h>
#include <aknutils.h>
#include <eiklabel.h>
#include <aknsutils.h> 
#include <gdi.h>
#include <eikapp.h>
#include <gulicon.h>

#include "RsgInclude.h"
#include "WFLayoutUtils.h"
#include "GuidePreviewPopUpContent.h"

#define TITLE_LABEL_HEIGHT 0.8

_LIT(KDefaultText, "");

enum TControls
{
   ECurrStreetTitleId,
   ECurrStreetLabelId,
   ENextStreetTitleId,
   ENextStreetLabelId,
   EDistLeftTitleId,
   EDistLeftLabelId,
   ETimeLeftTitleId,
   ETimeLeftLabelId,
   EArrTimeTitleId,
   EArrTimeLabelId
};

CGuidePreviewPopUpContent* CGuidePreviewPopUpContent::NewL()
{
   CGuidePreviewPopUpContent* self = 
      CGuidePreviewPopUpContent::NewLC();
   CleanupStack::Pop(self);
   return self;
}

CGuidePreviewPopUpContent* CGuidePreviewPopUpContent::NewLC()
{
   CGuidePreviewPopUpContent* self = new (ELeave) CGuidePreviewPopUpContent();
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

void CGuidePreviewPopUpContent::ConstructL()
{
   /* Do not call CreateWindowL() as the parent CAknPreviewPopUpController has a 
      window. But when ConstructL() is called this has not yet been created (as 
      the CAknPreviewPopUpController has not been created) so defer all construction
      which requires a window to InitialiseL() which is called after 
      CAknPreviewPopUpController has been constructed. */

}

void CGuidePreviewPopUpContent::InitialiseL(const TRect& aRect)
{
   // Do not call CreateWindowL() as parent CAknPreviewPopUpController owns window
   
   InitComponentArrayL();

   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   AknsUtils::GetCachedColor(skin, iSkinTextColor, KAknsIIDQsnTextColors, 
                             EAknsCIQsnTextColorsCG55);

   HBufC* tmp = NULL;

   // Create all the labels.
   tmp = CCoeEnv::Static()->AllocReadResourceLC(R_WF_YOU_ARE_ON);
   iCurrStreetTitle = new (ELeave) CEikLabel();
   iCurrStreetTitle->SetContainerWindowL(*this);
   Components().AppendLC(iCurrStreetTitle, ECurrStreetTitleId);
   iCurrStreetTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
   iCurrStreetTitle->SetTextL(*tmp);
   CleanupStack::Pop(iCurrStreetTitle);
   CleanupStack::PopAndDestroy(tmp);

   iCurrStreetLabel = new (ELeave) CEikLabel();
   iCurrStreetLabel->SetContainerWindowL(*this);
   Components().AppendLC(iCurrStreetLabel, ECurrStreetLabelId);
   iCurrStreetLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
   iCurrStreetLabel->SetTextL(KDefaultText);
   CleanupStack::Pop(iCurrStreetLabel);

   tmp = CCoeEnv::Static()->AllocReadResourceLC(R_WF_NEXT_STREET);
   iNextStreetTitle = new (ELeave) CEikLabel();
   iNextStreetTitle->SetContainerWindowL(*this);
   Components().AppendLC(iNextStreetTitle, ENextStreetTitleId);
   iNextStreetTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
   iNextStreetTitle->SetTextL(*tmp);
   CleanupStack::Pop(iNextStreetTitle);
   CleanupStack::PopAndDestroy(tmp);

   iNextStreetLabel = new (ELeave) CEikLabel();
   iNextStreetLabel->SetContainerWindowL(*this);
   Components().AppendLC(iNextStreetLabel, ENextStreetLabelId);
   iNextStreetLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
   iNextStreetLabel->SetTextL(KDefaultText);
   CleanupStack::Pop(iNextStreetLabel);

   tmp = CCoeEnv::Static()->AllocReadResourceLC(R_WF_DISTANCE_LEFT);
   iDistLeftTitle = new (ELeave) CEikLabel();
   iDistLeftTitle->SetContainerWindowL(*this);
   Components().AppendLC(iDistLeftTitle, EDistLeftTitleId);
   iDistLeftTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
   iDistLeftTitle->SetTextL(*tmp);
   CleanupStack::Pop(iDistLeftTitle);
   CleanupStack::PopAndDestroy(tmp);

   iDistLeftLabel = new (ELeave) CEikLabel();
   iDistLeftLabel->SetContainerWindowL(*this);
   Components().AppendLC(iDistLeftLabel, EDistLeftLabelId);
   iDistLeftLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
   iDistLeftLabel->SetTextL(KDefaultText);
   CleanupStack::Pop(iDistLeftLabel);

   tmp = CCoeEnv::Static()->AllocReadResourceLC(R_WF_TIME_LEFT);
   iTimeLeftTitle = new (ELeave) CEikLabel();
   iTimeLeftTitle->SetContainerWindowL(*this);
   Components().AppendLC(iTimeLeftTitle, ETimeLeftTitleId);
   iTimeLeftTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
   iTimeLeftTitle->SetTextL(*tmp);
   CleanupStack::Pop(iTimeLeftTitle);
   CleanupStack::PopAndDestroy(tmp);

   iTimeLeftLabel = new (ELeave) CEikLabel();
   iTimeLeftLabel->SetContainerWindowL(*this);
   Components().AppendLC(iTimeLeftLabel, ETimeLeftLabelId);
   iTimeLeftLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
   iTimeLeftLabel->SetTextL(KDefaultText);
   CleanupStack::Pop(iTimeLeftLabel);

   tmp = CCoeEnv::Static()->AllocReadResourceLC(R_WF_WAYFINDER_ARRIVAL_TIME);
   iArrTimeTitle = new (ELeave) CEikLabel();
   iArrTimeTitle->SetContainerWindowL(*this);
   Components().AppendLC(iArrTimeTitle, EArrTimeTitleId);
   iArrTimeTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
   iArrTimeTitle->SetTextL(*tmp);
   CleanupStack::Pop(iArrTimeTitle);
   CleanupStack::PopAndDestroy(tmp);

   iArrTimeLabel = new (ELeave) CEikLabel();
   iArrTimeLabel->SetContainerWindowL(*this);
   Components().AppendLC(iArrTimeLabel, EArrTimeLabelId);
   iArrTimeLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
   iArrTimeLabel->SetTextL(KDefaultText);
   CleanupStack::Pop(iArrTimeLabel);

   // Create a font for the title labels based on the standard label 
   // font but a little bit smaller.
   UpdateLabelsFont();

   // Set the standard rect.
   iComponentRect = aRect;

   // Set the windows size
   SetRect(aRect);

   // Activate the window, which makes it ready to be drawn
   ActivateL();
}

CGuidePreviewPopUpContent::CGuidePreviewPopUpContent() : 
   iIsNightMode(EFalse)
{
}

CGuidePreviewPopUpContent::~CGuidePreviewPopUpContent()
{
}

void CGuidePreviewPopUpContent::SizeChanged()
{
   TRect rect = Rect();
   iComponentRect = rect;
   iComponentRect.Shrink(iPadding, iPadding);

   // Update the font for the title labels based on the standard label 
   // font but a bit smaller. (We need to do this before using fonts below).
   UpdateLabelsFont();

   // Get some size info on the different fonts.
   TInt titleFontHeight = iCurrStreetTitle->Font()->HeightInPixels();
   TInt titleFontDescent = iCurrStreetTitle->Font()->DescentInPixels();
   // The height of one title label.
   TInt titleLabelHeight = titleFontHeight + titleFontDescent;
   TInt textFontHeight = iCurrStreetLabel->Font()->HeightInPixels();
   TInt textFontDescent = iCurrStreetLabel->Font()->DescentInPixels();
   // The height of one text label.
   TInt textLabelHeight = textFontHeight + textFontDescent;

   if (WFLayoutUtils::LandscapeMode()) {
      // Phone is in landscape layout it accordingly.

      // Get the max width needed by any of the title labels..
      TInt maxWidth= Max(iCurrStreetTitle->MinimumSize().iWidth, 
                         iNextStreetTitle->MinimumSize().iWidth);
      maxWidth = Max(maxWidth, iDistLeftTitle->MinimumSize().iWidth);
      maxWidth = Max(maxWidth, iTimeLeftTitle->MinimumSize().iWidth);
      maxWidth = Max(maxWidth, iArrTimeTitle->MinimumSize().iWidth);

      // The rect available to title labels.
      TRect titleLabelRect = TRect(iComponentRect.iTl, 
                                   TSize(maxWidth + iPadding * 3, 
                                         titleLabelHeight));
      // The rect available to text labels.
      TRect textLabelRect = 
         TRect(TPoint(titleLabelRect.iBr.iX, iComponentRect.iTl.iY),
               TSize(iComponentRect.Width() - titleLabelRect.Width(), 
                     textLabelHeight));

      // The difference in height between the two fonts.
      TInt fontHeightDiff = textFontHeight - titleFontHeight;

      // Set the rects for the labels in landscape mode.
      titleLabelRect.Move(0, fontHeightDiff);
      iCurrStreetTitle->SetRect(titleLabelRect);
      iCurrStreetLabel->SetRect(textLabelRect);

      titleLabelRect.Move(0, textLabelHeight);
      textLabelRect.Move(0, textLabelHeight);
      iNextStreetTitle->SetRect(titleLabelRect);
      iNextStreetLabel->SetRect(textLabelRect);

      titleLabelRect.Move(0, textLabelHeight);
      textLabelRect.Move(0, textLabelHeight);
      iDistLeftTitle->SetRect(titleLabelRect);
      iDistLeftLabel->SetRect(textLabelRect);

      titleLabelRect.Move(0, textLabelHeight);
      textLabelRect.Move(0, textLabelHeight);
      iTimeLeftTitle->SetRect(titleLabelRect);
      iTimeLeftLabel->SetRect(textLabelRect);

      titleLabelRect.Move(0, textLabelHeight);
      textLabelRect.Move(0, textLabelHeight);
      iArrTimeTitle->SetRect(titleLabelRect);
      iArrTimeLabel->SetRect(textLabelRect);
   } else {
      // Phone is in portrait use that layout.

      // The rect available to title labels.
      TRect titleLabelRect = TRect(iComponentRect.iTl,  
                                   TSize(iComponentRect.Width(), 
                                         titleLabelHeight));
      // The rect available to text labels.
      TRect textLabelRect = TRect(iComponentRect.iTl,
                                  TSize(iComponentRect.Width(), 
                                        textLabelHeight));

      // Set the rects for the labels in portrait mode.
      iCurrStreetTitle->SetRect(titleLabelRect);

      textLabelRect.Move(0, titleLabelHeight);
      iCurrStreetLabel->SetRect(textLabelRect);

      titleLabelRect.Move(0, titleLabelHeight + textLabelHeight + iPadding);
      iNextStreetTitle->SetRect(titleLabelRect);

      textLabelRect.Move(0, titleLabelHeight + textLabelHeight + iPadding);
      iNextStreetLabel->SetRect(textLabelRect);

      titleLabelRect.Move(0, titleLabelHeight + textLabelHeight + iPadding);
      iDistLeftTitle->SetRect(titleLabelRect);

      textLabelRect.Move(0, titleLabelHeight + textLabelHeight + iPadding);
      iDistLeftLabel->SetRect(textLabelRect);

      titleLabelRect.Move(0, titleLabelHeight + textLabelHeight + iPadding);
      iTimeLeftTitle->SetRect(titleLabelRect);

      textLabelRect.Move(0, titleLabelHeight + textLabelHeight + iPadding);
      iTimeLeftLabel->SetRect(textLabelRect);

      titleLabelRect.Move(0, titleLabelHeight + textLabelHeight + iPadding);
      iArrTimeTitle->SetRect(titleLabelRect);

      textLabelRect.Move(0, titleLabelHeight + textLabelHeight + iPadding);
      iArrTimeLabel->SetRect(textLabelRect);
   }

   // Hide labels if they are outside our allowed rect.
   UpdateLabelsVisibility();

   //Window().Invalidate();
}

void CGuidePreviewPopUpContent::UpdateLabelsFont()
{
   // Create a font for the title labels based on the standard label 
   // font but a little bit smaller.
   CWsScreenDevice* screenDev = CEikonEnv::Static()->ScreenDevice();
   const CFont* tmpFont = iCurrStreetLabel->Font();
   TFontSpec fontSpec = tmpFont->FontSpecInTwips();
   fontSpec.iHeight =  TInt(fontSpec.iHeight * TITLE_LABEL_HEIGHT);
   fontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightNormal);
   CFont* font;
   screenDev->GetNearestFontToDesignHeightInTwips(font, fontSpec);
   // Set the title labels fonts, we need to re-set them in SizeChanged 
   // since a layout switch might cause us having to use a different font. 
   iCurrStreetTitle->SetFont(font);
   iCurrStreetTitle->SetAlignment(EHLeftVCenter);
   iNextStreetTitle->SetFont(font);
   iNextStreetTitle->SetAlignment(EHLeftVCenter);
   iDistLeftTitle->SetFont(font);
   iDistLeftTitle->SetAlignment(EHLeftVCenter);
   iTimeLeftTitle->SetFont(font);
   iTimeLeftTitle->SetAlignment(EHLeftVCenter);
   iArrTimeTitle->SetFont(font);
   iArrTimeTitle->SetAlignment(EHLeftVCenter);
   // Release the font.
   screenDev->ReleaseFont(font);
}

void CGuidePreviewPopUpContent::UpdateLabelsVisibility()
{
   // Hide labels if they are outside our allowed rect.
   TInt xPos = iComponentRect.iBr.iX / 2;
   if (!iComponentRect.Contains(TPoint(xPos, iDistLeftLabel->Rect().iBr.iY)) ||
       !iComponentRect.Contains(TPoint(xPos, iDistLeftTitle->Rect().iBr.iY))) {
      Components().RemoveById(EDistLeftTitleId);
      Components().RemoveById(EDistLeftLabelId);
   } else {
      CCoeControlArray::TCursor it = Components().Find(EDistLeftTitleId);
      if (!it.IsValid()) {
         Components().AppendLC(iDistLeftTitle, EDistLeftTitleId);
         CleanupStack::Pop(iDistLeftTitle);
      }
      it = Components().Find(EDistLeftLabelId);
      if (!it.IsValid()) {
         Components().AppendLC(iDistLeftLabel, EDistLeftLabelId);
         CleanupStack::Pop(iDistLeftLabel);
      }
   }
   if (!iComponentRect.Contains(TPoint(xPos, iTimeLeftLabel->Rect().iBr.iY)) ||
       !iComponentRect.Contains(TPoint(xPos, iTimeLeftTitle->Rect().iBr.iY))) {
      Components().RemoveById(ETimeLeftTitleId);
      Components().RemoveById(ETimeLeftLabelId);
   } else {
      CCoeControlArray::TCursor it = Components().Find(ETimeLeftTitleId);
      if (!it.IsValid()) {
         Components().AppendLC(iTimeLeftTitle, ETimeLeftTitleId);
         CleanupStack::Pop(iTimeLeftTitle);
      }
      it = Components().Find(ETimeLeftLabelId);
      if (!it.IsValid()) {
         Components().AppendLC(iTimeLeftLabel, ETimeLeftLabelId);
         CleanupStack::Pop(iTimeLeftLabel);
      }
   }
   if (!iComponentRect.Contains(TPoint(xPos, iArrTimeLabel->Rect().iBr.iY)) ||
       !iComponentRect.Contains(TPoint(xPos, iArrTimeTitle->Rect().iBr.iY))) {
      Components().RemoveById(EArrTimeTitleId);
      Components().RemoveById(EArrTimeLabelId);
   } else {
      CCoeControlArray::TCursor it = Components().Find(EArrTimeTitleId);
      if (!it.IsValid()) {
         Components().AppendLC(iArrTimeTitle, EArrTimeTitleId);
         CleanupStack::Pop(iArrTimeTitle);
      }
      it = Components().Find(EArrTimeLabelId);
      if (!it.IsValid()) {
         Components().AppendLC(iArrTimeLabel, EArrTimeLabelId);
         CleanupStack::Pop(iArrTimeLabel);
      }
   } 
}

TSize CGuidePreviewPopUpContent::MinimumSize()
{
   TRect rect(Rect());
   TSize size(rect.Width(), rect.Height());
   return size;
}

void CGuidePreviewPopUpContent::Draw(const TRect& /*aRect*/) const
{
   // Get the standard graphics context
   CWindowGc& gc = SystemGc();
   TRect rect(Rect());
   gc.SetClippingRect(rect);

   if (iIsNightMode) {
      TRect rect(Rect());
      gc.SetPenColor(iNightBackColor);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetBrushColor(iNightBackColor);
      gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
      gc.DrawRect(rect);
   }
}

void 
CGuidePreviewPopUpContent::SetSizeAndLayout(TRect aRect, TInt aPadding)
{
   iComponentRect = aRect;
   iComponentRect.Shrink(aPadding, aPadding);
   iPadding = aPadding;

   SetRect(aRect);
}

void CGuidePreviewPopUpContent::SetStreetsL(const TDesC& aCurrName, 
                                            const TDesC& aNextName)
{
   if (iCurrStreetLabel && iNextStreetLabel) {
      iCurrStreetLabel->SetTextL(aCurrName);
      iCurrStreetLabel->CropText();
      Window().Invalidate(iCurrStreetLabel->Rect());
      //iCurrStreetLabel->DrawDeferred();
      iNextStreetLabel->SetTextL(aNextName);
      iNextStreetLabel->CropText();
      Window().Invalidate(iNextStreetLabel->Rect());
      //iCurrStreetLabel->DrawDeferred();
   }
}

void CGuidePreviewPopUpContent::SetDistanceToGoalL(const TDesC& aDistance)
{
   if (iDistLeftLabel) {
      iDistLeftLabel->SetTextL(aDistance);
      Window().Invalidate(iDistLeftLabel->Rect());
   }
}

void CGuidePreviewPopUpContent::SetEtgL(const TDesC& aTimeLeft)
{
   if (iTimeLeftLabel) {
      iTimeLeftLabel->SetTextL(aTimeLeft);
      Window().Invalidate(iTimeLeftLabel->Rect());
   }
}

void CGuidePreviewPopUpContent::SetEtaL(const TDesC& aArrivalTime)
{
   if (iArrTimeLabel) {
      iArrTimeLabel->SetTextL(aArrivalTime);
      Window().Invalidate(iArrTimeLabel->Rect());
   }
}

void CGuidePreviewPopUpContent::SetNightModeL(TBool aNightMode,
                                              TRgb aFgColor, 
                                              TRgb aBgColor)
{
   iIsNightMode = aNightMode;
   iNightTextColor = aFgColor;
   iNightBackColor = aBgColor;
   if (aNightMode) {
      // Night mode on, set label colors to nightmode color.
      iCurrStreetTitle->OverrideColorL(EColorLabelText, iNightTextColor);
      iCurrStreetLabel->OverrideColorL(EColorLabelText, iNightTextColor);
      iNextStreetTitle->OverrideColorL(EColorLabelText, iNightTextColor);
      iNextStreetLabel->OverrideColorL(EColorLabelText, iNightTextColor);
      iDistLeftTitle->OverrideColorL(EColorLabelText, iNightTextColor);
      iDistLeftLabel->OverrideColorL(EColorLabelText, iNightTextColor);
      iTimeLeftTitle->OverrideColorL(EColorLabelText, iNightTextColor);
      iTimeLeftLabel->OverrideColorL(EColorLabelText, iNightTextColor);
      iArrTimeTitle->OverrideColorL(EColorLabelText, iNightTextColor);
      iArrTimeLabel->OverrideColorL(EColorLabelText, iNightTextColor);
   } else {
      // Night mode off, set label colors to standard skin color.
      iCurrStreetTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
      iCurrStreetLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
      iNextStreetTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
      iNextStreetLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
      iDistLeftTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
      iDistLeftLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
      iTimeLeftTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
      iTimeLeftLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
      iArrTimeTitle->OverrideColorL(EColorLabelText, iSkinTextColor);
      iArrTimeLabel->OverrideColorL(EColorLabelText, iSkinTextColor);
   }
   Window().Invalidate();
}

#endif //NAV2_CLIENT_SERIES60_V5
