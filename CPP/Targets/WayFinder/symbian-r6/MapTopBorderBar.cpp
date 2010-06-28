/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "MapTopBorderBar.h"

#include <e32std.h>
#include <eiklabel.h> 
#include <gdi.h>
#include <aknsutils.h> 
#include <gulcolor.h> 
#include <aknutils.h> 
#include <w32std.h>
#include <eikcolor.hrh>

#include "ImageHandler.h"
#include "WFLayoutUtils.h"
#include "RectTools.h"
#include "DrawComponent.h"
#include "DistancePrintingPolicy.h"
#include "WFTextUtil.h"
#include "RsgInclude.h"

_LIT(KDistExLong, "9999km");
_LIT(KDistExMedium, "999m");
_LIT(KDistExShort, "99m");
_LIT(KDefaultText, "");

const TInt KPadding = 2;
const TReal KDividerPosPartOfRect = 0.6;
const TReal KDistanceUnitVerticalDivider = 0.68;
const TReal KETGSpeedVerticalDivider = 0.5;

_LIT(KETGEx, "55min 55s");
_LIT(KSpeedEx, "150 km/h");
_LIT(KSpace, " ");

CMapTopBorderBar* CMapTopBorderBar::NewLC(CCoeControl* aParent,
                                          const TRect& aRect,
                                          const TDesC& aMbmFilePath,
                                          const TRgb& aTextColor,
                                          const TRgb& aNightModeTextColor)
{
   CMapTopBorderBar* self = new (ELeave) CMapTopBorderBar();
   CleanupStack::PushL(self);
   self->ConstructL(aParent, aRect, aMbmFilePath, aTextColor, aNightModeTextColor);
   return self;
}

CMapTopBorderBar* CMapTopBorderBar::NewL(CCoeControl* aParent,
                                         const TRect& aRect,
                                         const TDesC& aMbmFilePath,
                                         const TRgb& aTextColor,
                                         const TRgb& aNightModeTextColor)
{
   CMapTopBorderBar* self = CMapTopBorderBar::NewLC(aParent,
                                                    aRect,
                                                    aMbmFilePath, 
                                                    aTextColor, aNightModeTextColor);
   CleanupStack::Pop(self);
   return self;
}

void CMapTopBorderBar::ConstructL(CCoeControl* aParent,
                                  const TRect& aRect,
                                  const TDesC& aMbmFilePath,
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

   iMbmFilePath = aMbmFilePath.AllocL();

   iTextColor = aTextColor;
   iNightModeTextColor = aNightModeTextColor;
   // Create a default rect that is (0, 0) (0, 0), the correct rects
   // will be calculated and set in SizeChanged
   TRect defaultRect;

   // Create the divider image and add it to the CCoeControlArray
   iDividerImage = CImageHandler::NewL(defaultRect, NULL);
   Components().AppendLC(iDividerImage);
   CleanupStack::Pop(iDividerImage);

   // Create the turn image and add it to the CCoeControlArray
   iTurnImage = CImageHandler::NewL(defaultRect, NULL);
   Components().AppendLC(iTurnImage);
   CleanupStack::Pop(iTurnImage);

   // Create the exit count picture.
   iExitPicture = CImageHandler::NewL(defaultRect);
   Components().AppendLC(iExitPicture);
   CleanupStack::Pop(iExitPicture);

   // Create the distance control and add it to the CCoeControlArray
   iDistanceControl = CDrawComponent::NewL(this, aTextColor, CGraphicsContext::ELeft);
   Components().AppendLC(iDistanceControl);
   CleanupStack::Pop(iDistanceControl);

   // Create the distance control and add it to the CCoeControlArray
   iDistanceUnitControl = CDrawComponent::NewL(this, aTextColor, CGraphicsContext::ELeft);
   Components().AppendLC(iDistanceUnitControl);
   CleanupStack::Pop(iDistanceUnitControl);

   // Create the ETG control and add it to the CCoeControlArray
   iETGControl = CDrawComponent::NewL(this, aTextColor, CGraphicsContext::ERight);
   Components().AppendLC(iETGControl);
   CleanupStack::Pop(iETGControl);

   // Create the speed control and add it to the CCoeControlArray
   iSpeedControl = CDrawComponent::NewL(this, aTextColor, CGraphicsContext::ERight);
   Components().AppendLC(iSpeedControl);
   CleanupStack::Pop(iSpeedControl);

   ShowDistanceUnitOnSameRow(WFLayoutUtils::LandscapeMode());

   SetRect(aRect);

   if (!aParent) {
      // Activate the window, which makes it ready to be drawn
      ActivateL();
   }
}

CMapTopBorderBar::CMapTopBorderBar() :
   iFontVec(3), iNightMode(EFalse), iDistanceUnitOnSameRow(EFalse),
   iLastDistance(0), iLastDistanceMode(0)
{   
}

CMapTopBorderBar::~CMapTopBorderBar()
{
   // Note: All controls that where added to the CCoeControlArray will be deleted
   // by the framework, hence they should not be deleted here

   delete iMbmFilePath;
   
   CWsScreenDevice* screenDev = CEikonEnv::Static()->ScreenDevice();
   for (TInt i = 0; i < iFontVec.Count(); ++i) {
      // Release all the fonts in the font vec
      screenDev->ReleaseFont(iFontVec[i]);
   }
   iFontVec.Reset();
   iFontVec.Close();
}

void CMapTopBorderBar::SizeChanged()
{
   // This controls complete rect
   TRect completeRect = Rect();

   // Recalculate the dpiCorrectionFactor, probably not changed
   // but do it anyway
   TInt dpiCorrectionFactor = WFLayoutUtils::
      CalculateDpiCorrectionFactor();

   // Calc the padding
   TInt padding = KPadding * dpiCorrectionFactor;

   TRect completeRectWithPadding = completeRect;
   completeRectWithPadding.Shrink(padding, padding);


   TRect firstColumnRect = completeRectWithPadding;
   TRect secondColumnRect = completeRectWithPadding;
   TRect thirdColumnRect = completeRectWithPadding;

   firstColumnRect.SetWidth(firstColumnRect.Height());

   secondColumnRect.iTl.iX = firstColumnRect.iBr.iX + padding;
   // Simple calcing of the left most pos of the divider
   TInt dividerXPos = TInt(completeRect.Width() * KDividerPosPartOfRect);
   secondColumnRect.iBr.iX = dividerXPos;

   thirdColumnRect.iTl.iX = secondColumnRect.iBr.iX + padding;

   // Set the rect of the turn image
   iTurnImage->SetImageRect(firstColumnRect);

   // Set the initial size of the exit image.
   TRect exitRect = 
      Center(iTurnImage->GetRect(), 
             TRect(TPoint(), 
                   WFLayoutUtils::
                   CalculateSizeUsingSize(iTurnImage->GetSize(), 
                                          0.25, 0.35,
                                          WFLayoutUtils::WideScreen())));
   iExitPicture->SetImageRect(exitRect, 
                              EAspectRatioPreservedAndUnusedSpaceRemoved);

   // Get and set the rect again but this time with the actual image 
   // size that the exitpicture got with unused space removed.
   exitRect = Center(iTurnImage->GetRect(), iExitPicture->GetRect());
   // Move down a little bit since the image contains a little nudge below 
   // the square so it looks off center if we dont fake it like this.
   exitRect.Move(0, 2); 
   iExitPicture->SetImageRect(exitRect, 
                              EAspectRatioPreservedAndUnusedSpaceRemoved);

   TInt distanceUnitDividerYPos = TInt(completeRectWithPadding.Height() *
      KDistanceUnitVerticalDivider) + completeRectWithPadding.iTl.iY;
   TInt ETGSpeedDividerYPos = TInt(completeRectWithPadding.Height() *
      KETGSpeedVerticalDivider) + completeRectWithPadding.iTl.iY;


   TRect distanceLabelRect = secondColumnRect;
   distanceLabelRect.iBr.iY = distanceUnitDividerYPos;
   
   if(iDistanceUnitOnSameRow)
      distanceLabelRect.iBr.iY = completeRectWithPadding.Height() + completeRectWithPadding.iTl.iY;

   TRect distanceUnitLabelRect = secondColumnRect;
   distanceUnitLabelRect.iTl.iY = distanceUnitDividerYPos + padding;

   TRect ETGLabelRect = thirdColumnRect;
   ETGLabelRect.iBr.iY = ETGSpeedDividerYPos;
   
   TRect speedLabelRect = thirdColumnRect;
   speedLabelRect.iTl.iY = ETGSpeedDividerYPos + padding;

   
   if (iFontVec.Count() > 0) {
      // If the font vec isn't empty it should be cleared
      // but we need to free the fonts as well.
      CWsScreenDevice* screenDev = 
         CEikonEnv::Static()->ScreenDevice();
      for (TInt i = 0; i < iFontVec.Count(); ++i) {
         screenDev->ReleaseFont(iFontVec[i]);
      }
      iFontVec.Reset();
   }

   // The maxFontHeight will be the height of the complete rect
   // minus the padding above and under.
   TInt maxFontHeight = distanceLabelRect.Height();

   // The width of the dist label is the space between the
   // distLabelLeftX and dividerPos.
   TInt maxFontWidth = distanceLabelRect.Width();;

   TBool ignoreDescent = ETrue;
   if(iDistanceUnitOnSameRow)
      ignoreDescent = EFalse;

   // Get the correct font the short text and add it to iFontVec
   iFontVec.Append(CalculateFontSizeL(KDistExShort, 
                                      maxFontHeight, 
                                      maxFontWidth,
                                      ignoreDescent));

   // Get the correct font the medium text and add it to iFontVec
   iFontVec.Append(CalculateFontSizeL(KDistExMedium, 
                                      maxFontHeight,
                                      maxFontWidth,
                                      ignoreDescent));

   // Get the correct font the longest text and add it to iFontVec
   iFontVec.Append(CalculateFontSizeL(KDistExLong, 
                                      maxFontHeight, 
                                      maxFontWidth,
                                      ignoreDescent));

   TInt textLength = iDistanceControl->Text().Length();
   if (textLength > 0) {
      // If the distance control contains a text we should
      // set the correct font since the layout may have been changed
      iDistanceControl->SetFont(iFontVec[GetFontSize(textLength)]);

      TInt distBaseLineOffset = distanceLabelRect.Height();
      if(iDistanceUnitOnSameRow)
         distBaseLineOffset = (distanceLabelRect.Height() - iFontVec[GetFontSize(textLength)]->FontMaxDescent());

      iDistanceControl->SetRect(distanceLabelRect);
      iDistanceControl->SetBaselineOffset(distBaseLineOffset);
   }
   
   TInt unitTextLength = iDistanceUnitControl->Text().Length();
   if (unitTextLength > 0) {
      // If the distance unit control contains a text we should
      // set the correct font since the layout may have been changed
      CFont* distanceUnitFont = CalculateFontSizeL(iDistanceUnitControl->Text(), 
                                                   distanceUnitLabelRect.Height(), 
                                                   distanceUnitLabelRect.Width());
      iDistanceUnitControl->SetFont(distanceUnitFont);

      TInt distUnitBaseLineOffset = (distanceUnitLabelRect.Height() -
                                     distanceUnitFont->FontMaxDescent());

      iDistanceUnitControl->SetBaselineOffset(distUnitBaseLineOffset);

      //release the font
      CEikonEnv::Static()->ScreenDevice()->ReleaseFont(distanceUnitFont);
   }
   iDistanceUnitControl->SetRect(distanceUnitLabelRect);

   // Set the rect for the background image, it should cover the 
   // complete rect of this control rect.
   if(iBackgroundImage) {
      iBackgroundImage->SetImageRect(completeRect, EAspectRatioNotPreserved);
   }

   CFont* ETGFont = CalculateFontSizeL(KETGEx, ETGLabelRect.Height(), ETGLabelRect.Width());
   CFont* speedFont = CalculateFontSizeL(KSpeedEx, speedLabelRect.Height(), speedLabelRect.Width());

   CFont* fontToUse;
   // Compare the two fonts and use the smalles one
   if (ETGFont->HeightInPixels() < speedFont->HeightInPixels() ) {
      // ETGFont was the smalles, use this and free the speedFont
      fontToUse = ETGFont;
      CEikonEnv::Static()->ScreenDevice()->ReleaseFont(speedFont);
   } else {
      // Speed font was the smalles, use this and free the ETGFont
      fontToUse = speedFont;
      CEikonEnv::Static()->ScreenDevice()->ReleaseFont(ETGFont);
   }

   // The baseline of the ETG text should be positioned so that
   // the descent fits below the baseline
   TInt ETGBaselineOffset = ETGLabelRect.Height() + 
      fontToUse->AscentInPixels() >> 1;

   // Set the font to use
   iETGControl->SetFont(fontToUse);

   // Set the controls rect and baseline offset
   iETGControl->SetRect(ETGLabelRect);
   iETGControl->SetBaselineOffset(ETGBaselineOffset);

   // The baseline of the speed text should be positioned so that
   // the descent fits below the baseline
   TInt speedBaselineOffset = speedLabelRect.Height() + 
      fontToUse->AscentInPixels() >> 1;

   // Set the font to use
   iSpeedControl->SetFont(fontToUse);

   //release the font
   CEikonEnv::Static()->ScreenDevice()->ReleaseFont(fontToUse);

   // Set the controls rect and baseline offset
   iSpeedControl->SetRect(speedLabelRect);
   iSpeedControl->SetBaselineOffset(speedBaselineOffset);
}

void CMapTopBorderBar::Draw(const TRect& /*aRect*/) const
{
   // Get the standard graphics context
   CWindowGc& gc = SystemGc();
   TRect rect(Rect());
   gc.SetClippingRect(rect);
   if(!iBackgroundImage) {
      gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
      if(iNightMode) {
         gc.SetBrushColor(iNightModeBackgroundColor);
      }
      else {
         gc.SetBrushColor(iBackgroundColor);
      }
      gc.SetPenSize(TSize(0, 0));
      gc.DrawRect(rect);
   }

   gc.SetBrushStyle(CGraphicsContext::ENullBrush);
}

CFont* CMapTopBorderBar::CalculateFontSizeL(const TDesC& aTextToFit,
                                            TInt aMaxHeightInPixels,
                                            TInt aMaxWidthInPixels,
                                            bool aIgnoreDescent)
{
   TInt fontMinimizerFactorInPixels = 2;
   // Get the screen device so that we can calc the twips
   // per pixel
   TPixelsTwipsAndRotation twips;
   CWsScreenDevice* screenDev = CEikonEnv::Static()->ScreenDevice();
   screenDev->GetScreenModeSizeAndRotation(screenDev->CurrentScreenMode(),
                                           twips);

   // Calc the twips per pixel
   TReal twipsPerPixel = twips.iTwipsSize.iHeight / 
      TReal(twips.iPixelSize.iHeight);
   
   // Get the largest logical font from the font store
   const CFont* font = AknLayoutUtils::
      FontFromId(EAknLogicalFontTitleFont);
   
   TFontSpec fontSpec = font->FontSpecInTwips();
   fontSpec.iHeight = TInt(aMaxHeightInPixels * twipsPerPixel);
   // Get the font that matches the fontspec the best
   CFont* fontToUse;
   screenDev->
      GetNearestFontToDesignHeightInTwips(fontToUse, fontSpec);
   TInt fontMinimizerFactorInTwips = 
      TInt(fontMinimizerFactorInPixels * twipsPerPixel);

   while (true) {
      TInt fontHeight = fontToUse->AscentInPixels();
      if(!aIgnoreDescent) {
         fontHeight += fontToUse->FontMaxDescent();
      }
      if(aMaxHeightInPixels >= fontHeight && aMaxWidthInPixels >= fontToUse->TextWidthInPixels(aTextToFit)) {
         break;
      }
      // The text didnt fit within the given space, make the font
      // a bit smaller and try again
      screenDev->ReleaseFont(fontToUse);
      fontSpec.iHeight -= fontMinimizerFactorInTwips;
      screenDev->
         GetNearestFontToDesignHeightInTwips(fontToUse, fontSpec);
   }

   // Return the font, the caller has to release the font
   // from the CWsScreenDevice
   return fontToUse;
}

CFont* CMapTopBorderBar::GetFontL(TInt aMaxFontHeight, 
                                  TInt aMaxFontWidth,
                                  const TDesC& aTextToFit)
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

   TInt fontMinimizerFactorInTwips = TInt(2 * twipsPerPixel);
   // Set the desired height of the font
   fontSpec.iHeight = TInt(twipsPerPixel * aMaxFontHeight);
   
   // Font should be bold
   fontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightNormal);
   // Font should be antialiased
   fontSpec.iFontStyle.SetBitmapType(EAntiAliasedGlyphBitmap);
   // Get the font that matches the fontspec the best
   CFont* fontToUse;
   screenDev->
      GetNearestFontToDesignHeightInTwips(fontToUse, fontSpec);

   while (aMaxFontHeight < (fontToUse->HeightInPixels() + 
                            fontToUse->DescentInPixels()) ||
          aMaxFontWidth < fontToUse->TextWidthInPixels(aTextToFit) ) {
      // The text didnt fit within the given space, make the font
      // a bit smaller and try again
      screenDev->ReleaseFont(fontToUse);
      fontSpec.iHeight -= fontMinimizerFactorInTwips;
      screenDev->
         GetNearestFontToDesignHeightInTwips(fontToUse, fontSpec);
   }

   // Return the font, the caller has to release the font
   // from the CWsScreenDevice
   return fontToUse;
}

void CMapTopBorderBar::SetBackgroundImageL(TInt aImageId,  
                                           TInt aImageMaskId)
{
   if(!iBackgroundImage) {
      // Create the background image and add it to the CCoeControlArray
      iBackgroundImage = CImageHandler::NewL(TRect(), NULL);
      CCoeControlArray::TCursor beginningCursor = Components().Begin();
      Components().InsertLC(beginningCursor, iBackgroundImage);
      CleanupStack::Pop(iBackgroundImage);
      }

   iBackgroundImage->CreateIconL(*iMbmFilePath, aImageId, 
                                 aImageMaskId, EAspectRatioNotPreserved);
   iBackgroundImage->MakeVisible(ETrue);
}

void CMapTopBorderBar::SetBackgroundColors(const TRgb& aColor, const TRgb& aNightModeColor)
{
   iBackgroundColor = aColor;
   iNightModeBackgroundColor = aNightModeColor;
}

void CMapTopBorderBar::SetDividerImageL(TInt aImageId,  
                                        TInt aImageMaskId)
{
   iDividerImage->CreateIconL(*iMbmFilePath, aImageId, 
                              aImageMaskId, EAspectRatioNotPreserved);
   iDividerImage->MakeVisible(ETrue);
}

void CMapTopBorderBar::SetTurnImageL(TInt aImageId,  
                                     TInt aImageMaskId)
{
   iTurnImage->CreateIconL(*iMbmFilePath, aImageId, aImageMaskId);
   //iTurnImage->NightFilter(iNightMode);
   iTurnImage->MakeVisible(ETrue);
   iTurnImage->DrawDeferred();
}

void CMapTopBorderBar::SetExitCountImageL(TInt aImageId, 
                                          TInt aImageMaskId)
{
   if (aImageId >= 0) {
      iExitPicture->SetShow(ETrue);
      iExitPicture->CreateIconL(*iMbmFilePath, aImageId, aImageMaskId, 
                                EAspectRatioPreservedAndUnusedSpaceRemoved,
                                ETrue);
      //iExitPicture->NightFilter(iNightMode);
      Window().Invalidate(iExitPicture->Rect());
   } else {
      iExitPicture->SetShow(EFalse);
      Window().Invalidate(iExitPicture->Rect());
   }
}

void CMapTopBorderBar::SetDistanceL(const TDesC& aDistance, const TDesC& aUnit)
{
   // Get the font from the array and set the font to the control
   CFont* font = iFontVec[GetFontSize(aDistance.Length())];
   iDistanceControl->SetFont(font);

   // An offset from the top of the rectangle to the text baseline: 
   // to centre the text, add half the height of the rectangle to 
   // half the height of the ascent
   TInt distBaseLineOffset = iDistanceControl->Rect().Height();

   if(iDistanceUnitOnSameRow)
      distBaseLineOffset = (iDistanceControl->Rect().Height() - font->FontMaxDescent());

   iDistanceControl->SetBaselineOffset(distBaseLineOffset);

   iDistanceControl->SetTextL(aDistance);
   iDistanceControl->DrawDeferred();

   iDistanceUnitControl->SetTextL(aUnit);
   iDistanceUnitControl->DrawDeferred();
}

void CMapTopBorderBar::SetDistanceL(TUint aDistance, TInt aDistanceMode)
{
   isab::DistancePrintingPolicy::DistanceMode mode =
      isab::DistancePrintingPolicy::DistanceMode(aDistanceMode);

   isab::DistancePrintingPolicy::DistanceAndUnit* distanceAndUnit = isab::DistancePrintingPolicy::
      convertDistanceAndUnit(aDistance, mode, 
                             isab::DistancePrintingPolicy::Round);
   HBufC* distanceString = NULL;
   HBufC* unitString = NULL;
   if(iDistanceUnitOnSameRow) {
      char* concatenated = distanceAndUnit->Concatenated(EFalse);
      unitString = WFTextUtil::AllocL(distanceAndUnit->m_unit);
      delete distanceAndUnit;
      distanceString = WFTextUtil::AllocL(concatenated);
      delete concatenated;
   }
   else {
      distanceString = WFTextUtil::AllocL(distanceAndUnit->m_distance);
      unitString = WFTextUtil::AllocL(distanceAndUnit->m_unit);
      delete distanceAndUnit;
   }
   CleanupStack::PushL(distanceString);
   CleanupStack::PushL(unitString);

   SetDistanceL(*distanceString, *unitString);

   CleanupStack::PopAndDestroy(unitString);
   CleanupStack::PopAndDestroy(distanceString);
   iLastDistance = aDistance;
   iLastDistanceMode = aDistanceMode;
}

void CMapTopBorderBar::SetETGL(const TDesC& aETG)
{
   iETGControl->SetTextL(aETG);
   iETGControl->DrawDeferred();
}

void CMapTopBorderBar::SetETGL(TInt aETG)
{
   TBuf<64> etg(KDefaultText);
   if (aETG >= 0) {
      TTime nowTime;
      nowTime.HomeTime();
      TTime arrivalTime;
      arrivalTime.HomeTime();
      arrivalTime += TTimeIntervalSeconds(aETG);
      TInt weeks = 0;
      TTimeIntervalDays days = arrivalTime.DaysFrom(nowTime);
      if (days.Int() > 7) {
         // We have more than 1 week left, append number of weeks and text.
         TBuf<16> weekText;
         iCoeEnv->ReadResource(weekText, R_TEXT_ROUTEV_ETG_WEEKS);
         weeks = days.Int() / 7;
         //TInt days = days.Int() % 7;
         etg.AppendNum(weeks);
         etg.Append(weekText);
         // Subtract the time we just added as text to the string.
         arrivalTime -= TTimeIntervalDays(weeks * 7);
      }
      days = arrivalTime.DaysFrom(nowTime);
      if (days.Int() > 0) {
         // We have more than 1 day left, append number of days and text.
         TBuf<16> dayText;
         iCoeEnv->ReadResource(dayText, R_TEXT_ROUTEV_ETG_DAYS);
         if (etg.Length() > 0) {
            // Add a space since we have already added something in the string.
            etg.Append(KSpace);
         }
         etg.AppendNum(days.Int());
         etg.Append(dayText);
         // Subtract the time we just added as text to the string.
         arrivalTime -= days;
      }
      TTimeIntervalHours hours;
      TInt err = arrivalTime.HoursFrom(nowTime, hours);
      if (err == KErrNone && hours.Int() > 0 && !weeks) {
         // We have more than 1 hour left and we dont have any weeks left 
         // so append number of hours and text.
         TBuf<16> hourText;
         iCoeEnv->ReadResource(hourText, R_TEXT_ROUTEV_ETG_HOURS);
         if (etg.Length() > 0) {
            // Add a space since we have already added something in the string.
            etg.Append(KSpace);
         }
         etg.AppendNum(hours.Int());
         etg.Append(hourText);
         // Subtract the time we just added as text to the string.
         arrivalTime -= hours;
      }
      TTimeIntervalMinutes minutes;
      err = arrivalTime.MinutesFrom(nowTime, minutes);
      if (err == KErrNone && minutes.Int() >= 0 && !weeks && !days.Int()) {
         // We have more than 1 minute left and we have not any weeks or days 
         // left so append number of minutes and text.
         TBuf<16> minuteText;
         iCoeEnv->ReadResource(minuteText, R_TEXT_ROUTEV_ETG_MIN);
         if (etg.Length() > 0) {
            // Add a space since we have already added something in the string.
            etg.Append(KSpace);
         }
         etg.AppendNum(minutes.Int());
         etg.Append(minuteText);
         // Subtract the time we just added as text to the string.
         arrivalTime -= minutes;
      }
   }
   SetETGL(etg);   
}

void CMapTopBorderBar::SetSpeedL(const TDesC& aSpeed, TInt aDistanceMode)
{
   HBufC* tmp;
   switch (aDistanceMode) {
   case isab::DistancePrintingPolicy::ModeImperialYards:
   case isab::DistancePrintingPolicy::ModeImperialFeet:
      tmp = iCoeEnv->AllocReadResourceLC( R_INFO_MPH_LABEL );
      break;
   case isab::DistancePrintingPolicy::ModeInvalid:
   case isab::DistancePrintingPolicy::ModeMetric:
   default:
      tmp = iCoeEnv->AllocReadResourceLC( R_INFO_KMH_LABEL );
      break;
   }
   TBuf<56> speedTxt;
   speedTxt.Copy(aSpeed);
   speedTxt.Append(KSpace);
   speedTxt.Append(*tmp);
   CleanupStack::PopAndDestroy(tmp);

   iSpeedControl->SetTextL(speedTxt);
   iSpeedControl->DrawDeferred();
}

void CMapTopBorderBar::SetSpeedL(TInt aSpeed, TInt aDistanceMode)
{
   TBuf<10> speed;
   speed.AppendNum(aSpeed);
   SetSpeedL(speed, aDistanceMode);
}

CMapTopBorderBar::TFontSize 
CMapTopBorderBar::GetFontSize(TInt aLength) const
{
   if (aLength > KDistExMedium().Length()) {
      return ESmallFont;
   } else if (aLength > KDistExShort().Length()) {
      return EMediumFont;
   } else {
      return ELargeFont;
   }
}

void CMapTopBorderBar::SetNightModeL(TBool aNightMode)
{
   iNightMode = aNightMode;
   /*
   //NightFilter disabled for the pictures.
   //Enable the nightfilter in SetTurnImageL(..) as well if you enable it here.
   if(iTurnImage && iTurnImage->GetShow()) {
      iTurnImage->NightFilter(aNightMode);
   }
   */
   if(aNightMode) {
      iDistanceControl->SetTextColor(iNightModeTextColor);
      iDistanceUnitControl->SetTextColor(iNightModeTextColor);
      iETGControl->SetTextColor(iNightModeTextColor);
      iSpeedControl->SetTextColor(iNightModeTextColor);
   }
   else {
      iDistanceControl->SetTextColor(iTextColor);
      iDistanceUnitControl->SetTextColor(iTextColor);
      iETGControl->SetTextColor(iTextColor);
      iSpeedControl->SetTextColor(iTextColor);
   }
   DrawDeferred();
}

void CMapTopBorderBar::ShowDistanceUnitOnSameRow(TBool aSameRow)
{
   iDistanceUnitOnSameRow = aSameRow;

   if(iDistanceUnitOnSameRow) {
      if(iDistanceUnitControl) {
         iDistanceUnitControl->MakeVisible(EFalse);
      }
   }
   else {
      if(iDistanceUnitControl) {
         iDistanceUnitControl->MakeVisible(ETrue);
      }
   }
}

void CMapTopBorderBar::HandleResourceChange(TInt aType)
{
   if(aType == KEikDynamicLayoutVariantSwitch) {
      ShowDistanceUnitOnSameRow(WFLayoutUtils::LandscapeMode());
      SetDistanceL(iLastDistance, iLastDistanceMode);
   }
   CCoeControl::HandleResourceChange(aType);
}
