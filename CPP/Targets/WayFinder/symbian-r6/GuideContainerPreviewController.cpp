/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <coemain.h>
#include <aknsutils.h>
#include <eiklabel.h> 
#include <gulcolor.h>
#include <e32std.h>
#include <aknviewappui.h>
#include <aknutils.h>
#include <aknsdrawutils.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <e32math.h>

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
# include <aknpreviewpopupcontroller.h>
#endif

#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "WayFinderConstants.h"
#include "DistancePrintingPolicy.h"
#include "WFTextUtil.h"
#include "WFLayoutUtils.h"
#include "RectTools.h"
#include "DistancePrintingPolicy.h"
#include "ImageHandler.h"
#include "GuideView.h"
#include "GuidePreviewPopUpContent.h"
#include "GuideContainerPreviewController.h"

#define GUIDE_PIC_HEIGHT 0.23
#define SMALL_PIC_HEIGHT 0.08
#define DIST_TEXT_HEIGHT 0.12
//#define DIST_TEXT_HEIGHT_OF_GUIDE_PIC 0.55

//_LIT(KDefaultEmptyText, "9234km");
_LIT(KDefaultEmptyText, "");

CGuideContainerPreviewController* 
CGuideContainerPreviewController::NewL(const TRect& aRect, 
                                       CGuideView& aView)
{
   CGuideContainerPreviewController* self = 
      CGuideContainerPreviewController::NewLC(aRect, aView);
   CleanupStack::Pop(self);
   return self;
}

CGuideContainerPreviewController* 
CGuideContainerPreviewController::NewLC(const TRect& aRect, 
                                        CGuideView& aView)
{
   CGuideContainerPreviewController* self = 
      new (ELeave) CGuideContainerPreviewController(aView);
   CleanupStack::PushL(self);
   self->ConstructL(aRect);
   return self;
}

void CGuideContainerPreviewController::ConstructL(const TRect& aRect)
{
   CreateWindowL();

   InitComponentArrayL();

   // Skin the background.
   iBgContext = 
      CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain,
                                               aRect, ETrue);
   // Get the skin text color.
   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   TRgb fgcolor(0,0,0);
   AknsUtils::GetCachedColor(skin, fgcolor, KAknsIIDQsnTextColors, 
                             EAknsCIQsnTextColorsCG6);

   // Set the guide picture size to a percentile of the available rect.
   TInt gpHeight = TInt(aRect.Height() * GUIDE_PIC_HEIGHT);
   TSize guidePicSize = TSize(gpHeight, gpHeight);

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   // Setup the preview pane if we're on s60v5 where it exists.
   TRect contentRect(aRect.iTl, TSize(aRect.Width(), aRect.Height() / 3));
   iPreviewContent = CGuidePreviewPopUpContent::NewL();
   iPreviewController = 
      CAknPreviewPopUpController::NewL(*iPreviewContent, 
                                       CAknPreviewPopUpController::EPermanentMode);
   iPreviewContent->InitialiseL(aRect);
   iPreviewController->SetPopUpShowDelay(0);
   iPreviewController->ShowPopUp();
   TInt borderPadding = 
      (iPreviewController->Size().iWidth - contentRect.Width()) / 2;

   // Set nightmode on the PreviewContent if in nightmode at construct time.
   TRgb fgColor;
   iView.GetForegroundColor(fgColor);
   TRgb bgColor;
   iView.GetNightmodeBackgroundColor(bgColor);
   iPreviewContent->SetNightModeL(iView.IsNightMode(), fgColor, bgColor);
#else
   // If not s60v5 we have no preview pane so we set the border padding to 
   // a fixed value to be used later.
   TInt borderPadding = 5;
#endif

   // Create and setup the GuidePicture.
   iGuidePicture = 
      CImageHandler::NewL(TRect(TPoint(borderPadding, borderPadding), 
                                guidePicSize));
   Components().AppendLC(iGuidePicture);
   CleanupStack::Pop(iGuidePicture);

   // Get twips to pixel relationship.
   TPixelsTwipsAndRotation twips;
   CWsScreenDevice* screenDev = CEikonEnv::Static()->ScreenDevice();
   screenDev->GetScreenModeSizeAndRotation(screenDev->CurrentScreenMode(),
                                           twips);
   float twipsPerPixel = 
      twips.iTwipsSize.iHeight / float(twips.iPixelSize.iHeight);

   // Create a large enough font.
   const CFont* tmpFont = 
      AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont);
   TFontSpec fontSpec = tmpFont->FontSpecInTwips();
   //fontSpec.iHeight = twipsPerPixel * guidePicSize.iHeight * DIST_TEXT_HEIGHT_OF_GUIDE_PIC;
   fontSpec.iHeight = TInt(twipsPerPixel * aRect.Height() * DIST_TEXT_HEIGHT);
   fontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
   CFont* font;
   screenDev->GetNearestFontToDesignHeightInTwips(font, fontSpec);

   // Create and setup label with size and font.
   iDistanceLabel = new (ELeave) CEikLabel();
   iDistanceLabel->SetContainerWindowL(*this);
   Components().AppendLC(iDistanceLabel);
   iDistanceLabel->OverrideColorL(EColorLabelText, fgcolor);
   iDistanceLabel->SetTextL(KDefaultEmptyText);
   iDistanceLabel->SetFont(font);
   iDistanceLabel->SetLabelAlignment(ELayoutAlignRight);
   screenDev->ReleaseFont(font);
   CleanupStack::Pop(iDistanceLabel);

   // Create the small pics (next turn, detour, speedcam).
   TInt spHeight = TInt(aRect.Height() * SMALL_PIC_HEIGHT);
   TSize smallPicSize = TSize(spHeight, spHeight);
   iNextTurnPicture = 
      CImageHandler::NewL(TRect(TPoint(borderPadding, borderPadding), 
                                smallPicSize));
   Components().AppendLC(iNextTurnPicture);
   CleanupStack::Pop(iNextTurnPicture);

   iSpeedCamPicture = 
      CImageHandler::NewL(TRect(TPoint(borderPadding, borderPadding), 
                                smallPicSize));
   Components().AppendLC(iSpeedCamPicture);
   iSpeedCamPicture->CreateIconL(iView.GetMbmName(), 
                                 EMbmWficonsSpeedcamera_square, 
                                 EMbmWficonsSpeedcamera_square,
                                 EAspectRatioPreservedAndUnusedSpaceRemoved);
   CleanupStack::Pop(iSpeedCamPicture);

   iDetourPicture = 
      CImageHandler::NewL(TRect(TPoint(borderPadding, borderPadding), 
                                smallPicSize));
   Components().AppendLC(iDetourPicture);
   iDetourPicture->CreateIconL(iView.GetMbmName(), 
                               EMbmWficonsDetour_square, 
                               EMbmWficonsDetour_square_mask,
                               EAspectRatioPreservedAndUnusedSpaceRemoved);
   CleanupStack::Pop(iDetourPicture);

   // Create the exit count picture.
   iExitPicture = 
      CImageHandler::NewL(TRect(TPoint(borderPadding, borderPadding), 
                                WFLayoutUtils::
                                CalculateSizeUsingSize(iGuidePicture->GetSize(), 
                                                       0.25, 0.35, WFLayoutUtils::WideScreen())));
   Components().AppendLC(iExitPicture);
   CleanupStack::Pop(iExitPicture);

   // Set the controls rect.
   SetRect(aRect);

   // Activate control.
   ActivateL();
}

CGuideContainerPreviewController::CGuideContainerPreviewController(CGuideView& aView) : 
   iView(aView)
{
}

CGuideContainerPreviewController::~CGuideContainerPreviewController()
{
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   delete iPreviewContent;
   delete iPreviewController;
#endif
   delete iBgContext;
}

TTypeUid::Ptr CGuideContainerPreviewController::MopSupplyObject(TTypeUid aId)
{
   if (iBgContext) {
      return MAknsControlContext::SupplyMopObject(aId, iBgContext);
   }
   return CCoeControl::MopSupplyObject(aId);
}

void CGuideContainerPreviewController::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
//    gc.SetPenStyle(CGraphicsContext::ESolidPen);
//    gc.SetPenColor(TRgb(0, 0, 0));
//    gc.SetBrushStyle(CGraphicsContext::ENullBrush);
//    gc.DrawRect(aRect);

   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
   AknsDrawUtils::Background(skin, cc, this, gc, aRect);

   if (iView.IsNightMode()) {
      TRgb bgColor;
      iView.GetNightmodeBackgroundColor(bgColor);
      gc.SetPenColor(bgColor);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetBrushColor(bgColor);
      gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
      gc.DrawRect(Rect());
   }

#undef DRAW_IMAGE_RECTS_DBG
//#define DRAW_IMAGE_RECTS_DBG
#ifdef DRAW_IMAGE_RECTS_DBG
   if (iGuidePicture) {
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor(TRgb(255, 0, 0));
      gc.DrawRect(iGuidePicture->Rect());
   }
   if (iDistanceLabel) {
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor(TRgb(0, 255, 0));
      gc.DrawRect(iDistanceLabel->Rect());
   }
# if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   if (iPreviewContent) {
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor(TRgb(0, 0, 255));
      gc.DrawRect(iPreviewContent->Rect());
   }
   if (iGuidePicture && iPreviewContent) {
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor(TRgb(255, 0, 255));
      TRect tmp = iPreviewContent->Rect();
      TInt borderPadding = (Rect().Width() - tmp.Width()) / 2;
      tmp.Move(borderPadding, iGuidePicture->Rect().Height() + borderPadding);
      gc.DrawRect(tmp);
   }
# endif
#endif
}

void CGuideContainerPreviewController::SizeChanged()
{
   if (iBgContext) {
      iBgContext->SetRect(Rect());
      if (&Window()) {
         iBgContext->SetParentPos(PositionRelativeToScreen());
      }
   }
   if (iGuidePicture && iDistanceLabel) {
      TRect rect = Rect();

      // Set the guide picture size to a percentile of the available rect.
      TInt gpHeight = TInt(rect.Height() * GUIDE_PIC_HEIGHT);
      TSize guidePicSize = TSize(gpHeight, gpHeight);

      TInt borderPadding = 0;
#if defined NAV2_CLIENT_SERIES60_V5  || defined NAV2_CLIENT_SERIES60_V32
      if (iPreviewController) {
         // Setup the preview pane.
         TRect contentRect(rect.iTl, TSize(rect.Width(), rect.Height() / 3));
         // Set temporary size to be able to get correct border size.
         iPreviewContent->SetSizeAndLayout(contentRect, 0);
         iPreviewController->UpdateContentSize();
         // Get the correct border size.
         borderPadding = 
            (iPreviewController->Size().iWidth - contentRect.Width()) / 2;

         // Set correct size with the correct border size.
         TRect previewRect(rect);
         previewRect.iTl.iY += (guidePicSize.iHeight + borderPadding * 2);
         previewRect.iBr.iY -= borderPadding;
         previewRect.Shrink(borderPadding * 2, borderPadding);

         TPoint containerPos = this->PositionRelativeToScreen();
         previewRect.Move(0, containerPos.iY);

         // XXX This line is a manual adjust that seems to be necessary now.
         // Remove it if the preview popup is off center. XXX
         previewRect.Move(borderPadding, -borderPadding);

         iPreviewContent->SetSizeAndLayout(previewRect, borderPadding);
         iPreviewController->UpdateContentSize();
         previewRect.Grow(borderPadding, borderPadding);
         iPreviewController->SetPositionByHighlight(previewRect);

         //iPreviewController->HidePopUp();
      } else {
         borderPadding = 5;
      }
#else
      // If not s60v5 we have no preview pane so we set the border padding to 
      // a fixed value to be used later.
      borderPadding = 5;
#endif
      // Set the GuidePicture size.
      iGuidePicture->SetImageRect(TRect(TPoint(borderPadding, borderPadding), 
                                        guidePicSize));

      // Get twips to pixel relationship.
      TPixelsTwipsAndRotation twips;
      CWsScreenDevice* screenDev = CEikonEnv::Static()->ScreenDevice();
      screenDev->GetScreenModeSizeAndRotation(screenDev->CurrentScreenMode(),
                                              twips);
      float twipsPerPixel = 
         twips.iTwipsSize.iHeight / float(twips.iPixelSize.iHeight);

      // Create a large enough font.
      const CFont* tmpFont = 
         AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont);
      TFontSpec fontSpec = tmpFont->FontSpecInTwips();
      //fontSpec.iHeight = twipsPerPixel * guidePicSize.iHeight * DIST_TEXT_HEIGHT_OF_GUIDE_PIC;
      fontSpec.iHeight = TInt(twipsPerPixel * rect.Height() * DIST_TEXT_HEIGHT);
      fontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
      CFont* font;
      screenDev->GetNearestFontToDesignHeightInTwips(font, fontSpec);

      // Setup label with size and font.
      iDistanceLabel->SetFont(font);
      screenDev->ReleaseFont(font);
      TInt distWidth = rect.Width() - (guidePicSize.iWidth + borderPadding *3);
      TRect distLabelRect = 
         TRect(TPoint(guidePicSize.iWidth + borderPadding * 2, 
                      borderPadding), 
               TSize(distWidth, guidePicSize.iHeight));
      iDistanceLabel->SetRect(distLabelRect);

      // Setup the small pics (next turn, detour, speedcam).
      TInt spHeight = TInt(rect.Height() * SMALL_PIC_HEIGHT);
      TSize smallPicSize = TSize(spHeight, spHeight);
      TPoint ntPicPos = TPoint(iGuidePicture->Rect().iBr);
      ntPicPos.iX += borderPadding;
      ntPicPos.iY -= spHeight;
      iNextTurnPicture->SetImageRect(TRect(ntPicPos, smallPicSize));

      ntPicPos.iX += smallPicSize.iWidth + borderPadding;
      iSpeedCamPicture->SetImageRect(TRect(ntPicPos, smallPicSize));

      ntPicPos.iX += smallPicSize.iWidth + borderPadding;
      iDetourPicture->SetImageRect(TRect(ntPicPos, smallPicSize));

      // Setup the exit count image.
      TRect exitRect = 
         Center(iGuidePicture->GetRect(), 
                TRect(TPoint(), 
                      WFLayoutUtils::
                      CalculateSizeUsingSize(iGuidePicture->GetSize(), 
                                             0.25, 0.35, WFLayoutUtils::WideScreen())));
      // Move down a little bit since the image contains a little nudge below 
      // the square so it looks off center if we dont fake it like this.
      exitRect.Move(0, 2);
      iExitPicture->SetImageRect(exitRect, 
                                 EAspectRatioPreservedAndUnusedSpaceRemoved, 
                                 ETrue);
   }
}

void CGuideContainerPreviewController::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
}

void CGuideContainerPreviewController::SetGuidePictureL(TInt aMbmIndex)
{
   if (aMbmIndex >= 0) {
      iGuidePicture->SetShow(ETrue);
      iGuidePicture->CreateIconL(iView.GetMbmName(), aMbmIndex, 
                                 EMbmWficonsTurn_blank_mask, 
                                 EAspectRatioPreservedAndUnusedSpaceRemoved);
      //iGuidePicture->NightFilter(iView.IsNightMode());
      Window().Invalidate(iGuidePicture->Rect());
   } else {
      iGuidePicture->SetShow(EFalse);
      Window().Invalidate(iGuidePicture->Rect());
   }
}

void CGuideContainerPreviewController::SetNextTurnPictureL(TInt aMbmIndex, 
                                                           TInt aMbmMaskIndex)
{
   if (aMbmIndex >= 0) {
      iNextTurnPicture->SetShow(ETrue);
      iNextTurnPicture->CreateIconL(iView.GetMbmName(), aMbmIndex, aMbmMaskIndex,
                                    EAspectRatioPreservedAndUnusedSpaceRemoved);
      //iNextTurnPicture->NightFilter(iView.IsNightMode());
      Window().Invalidate(iNextTurnPicture->Rect());
   } else {
      iNextTurnPicture->SetShow(EFalse);
      Window().Invalidate(iNextTurnPicture->Rect());
   }
}

void CGuideContainerPreviewController::SetDetourPicture(TInt on)
{
   iDetourPicture->SetShow(on == 1);
   Window().Invalidate(iDetourPicture->Rect());
}

void CGuideContainerPreviewController::SetSpeedCamPicture(TInt on)
{
   iSpeedCamPicture->SetShow(on == 1);
   Window().Invalidate(iSpeedCamPicture->Rect());
}

void CGuideContainerPreviewController::SetDistanceL(TUint aDistance)
{
   // Maybe we should also measure text and dynamicaly shrink the font if 
   // the text does not fit in its available rect.
   TBuf<KBuf32Length> distance;
   isab::DistancePrintingPolicy::DistanceMode mode =
      isab::DistancePrintingPolicy::DistanceMode(iView.GetDistanceMode());

   char* tmp = 
      isab::DistancePrintingPolicy::convertDistance(aDistance, mode, 
                                                    isab::DistancePrintingPolicy::Round);

   WFTextUtil::char2TDes(distance, tmp);
   iDistanceLabel->SetTextL(distance);
   iDistanceLabel->DrawDeferred();
}

void CGuideContainerPreviewController::SetExitL(TInt aMbmIndex, 
                                                TInt aMbmMaskIndex)
{
   if (aMbmIndex >= 0) {
      iExitPicture->SetShow(ETrue);
      iExitPicture->CreateIconL(iView.GetMbmName(), aMbmIndex, aMbmMaskIndex, 
                                EAspectRatioPreservedAndUnusedSpaceRemoved,
                                ETrue);
      //iExitPicture->NightFilter(iView.IsNightMode());
      Window().Invalidate(iExitPicture->Rect());
   } else {
      iExitPicture->SetShow(EFalse);
      Window().Invalidate(iExitPicture->Rect());
   }
}

void CGuideContainerPreviewController::SetStreetsL(const TDesC& aCurrName, 
                                                   const TDesC& aNextName)
{
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iPreviewContent->SetStreetsL(aCurrName, aNextName);
#endif
}

void 
CGuideContainerPreviewController::UpdateRouteDataL(TInt32 aDistanceToGoal, 
                                                   TInt32 aEstimatedTimeToGoal)
{
   SetDistanceToGoalL(aDistanceToGoal);
   SetEtgL(aEstimatedTimeToGoal);
   SetEtaL(aEstimatedTimeToGoal);
}

void 
CGuideContainerPreviewController::SetDistanceToGoalL(TInt32 aDistanceToGoal)
{
   TBuf<32> distance(KDefaultEmptyText);
   if (aDistanceToGoal != MAX_INT32 && aDistanceToGoal >= 0) {
      isab::DistancePrintingPolicy::DistanceMode mode =
         isab::DistancePrintingPolicy::DistanceMode(iView.GetDistanceModeWithSpace());
      char* tmp = 
         isab::DistancePrintingPolicy::convertDistance(aDistanceToGoal, mode);
      if (tmp) {
         WFTextUtil::char2TDes(distance, tmp);
         delete[] tmp;
      }
   }
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iPreviewContent->SetDistanceToGoalL(distance);
#endif
}

void CGuideContainerPreviewController::SetEtgL(TInt32 aEstimatedTimeToGoal)
{
   TBuf<64> etg(KDefaultEmptyText);
   if (aEstimatedTimeToGoal >= 0) {
      TTime nowTime;
      nowTime.HomeTime();
      TTime arrivalTime;
      arrivalTime.HomeTime();
      arrivalTime += TTimeIntervalSeconds(aEstimatedTimeToGoal);
      TInt weeks = 0;
      TTimeIntervalDays days = arrivalTime.DaysFrom(nowTime);
      if (days.Int() > 7) {
         // We have more than 1 week left, append number of weeks and text.
         TBuf<16> weekText;
         iCoeEnv->ReadResource(weekText, R_TEXT_ROUTEV_ETG_WEEKS);
         weeks = days.Int() / 7;
         //TInt days = days.Int() % 7;
         etg.AppendNum(weeks);
         etg.Append(KSpace);
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
         etg.Append(KSpace);
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
         etg.Append(KSpace);
         etg.Append(hourText);
         // Subtract the time we just added as text to the string.
         arrivalTime -= hours;
      }
      TTimeIntervalMinutes minutes;
      err = arrivalTime.MinutesFrom(nowTime, minutes);
      if (err == KErrNone && minutes.Int() > 0 && !weeks && !days.Int()) {
         // We have more than 1 minute left and we have not any weeks or days 
         // left so append number of minutes and text.
         TBuf<16> minuteText;
         iCoeEnv->ReadResource(minuteText, R_TEXT_ROUTEV_ETG_MIN);
         if (etg.Length() > 0) {
            // Add a space since we have already added something in the string.
            etg.Append(KSpace);
         }
         etg.AppendNum(minutes.Int());
         etg.Append(KSpace);
         etg.Append(minuteText);
         // Subtract the time we just added as text to the string.
         arrivalTime -= minutes;
      }
      TTimeIntervalSeconds seconds;
      err = arrivalTime.SecondsFrom(nowTime, seconds);
      if (err == KErrNone && seconds.Int() > 0 && minutes.Int() < 5 && 
          !weeks && !days.Int() && !hours.Int()) {
         // We have more than 1 second left and less that 5 minutes left 
         // and we do not have any weeks, days or hours left so append 
         // number of seconds and text.
         TBuf<16> secondText;
         iCoeEnv->ReadResource(secondText, R_TEXT_ROUTEV_ETG_SEC);
         if (etg.Length() > 0) {
            // Add a space since we have already added something in the string.
            etg.Append(KSpace);
         }
         etg.AppendNum(seconds.Int());
         etg.Append(KSpace);
         etg.Append(secondText);
         // Subtract the time we just added as text to the string.
         arrivalTime -= seconds;
      }
   }
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iPreviewContent->SetEtgL(etg);
#endif
}

void CGuideContainerPreviewController::SetEtaL(TInt32 aEstimatedTimeToGoal)
{
   TBuf<64> eta(KDefaultEmptyText);
   if (aEstimatedTimeToGoal >= 0) {
      TTime arrivalTime;
      arrivalTime.HomeTime();
      arrivalTime += TTimeIntervalSeconds(aEstimatedTimeToGoal);
      _LIT(KEstimatedTimeOfArrivalDateString,"%-B%:0%J%:1%T%:3%+B");
      arrivalTime.FormatL(eta, KEstimatedTimeOfArrivalDateString);
   }
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iPreviewContent->SetEtaL(eta);
#endif
}

void CGuideContainerPreviewController::SetNightModeL(TBool aNightMode)
{
   TRgb fgColor;
   iView.GetForegroundColor(fgColor);
   TRgb bgColor;
   iView.GetNightmodeBackgroundColor(bgColor);

   iDistanceLabel->OverrideColorL(EColorLabelText, fgColor);
   iDistanceLabel->DrawDeferred();
   /*
   //NightFilter disabled for the pictures.
   //Enable the NightFilter calls for the images in the following functions
   // if you enable it here: SetGuidePictureL, SetNextTurnPictureL and SetExitL.
   if (aNightMode) {
      iGuidePicture->NightFilter(aNightMode);
      iNextTurnPicture->NightFilter(aNightMode);
      iSpeedCamPicture->NightFilter(aNightMode);
      iDetourPicture->NightFilter(aNightMode);
   } else {
      iGuidePicture->TurnNightModeOff(iView.GetMbmName(), 
                                      iView.GetCurrentTurnPicture(),
                                      EMbmWficonsKeep_left_mask);
      TInt imageIndex, maskIndex;
      iView.GetNextTurnPicture(imageIndex, maskIndex);
      iNextTurnPicture->TurnNightModeOff(iView.GetMbmName(),
                                         imageIndex,
                                         maskIndex);
      iSpeedCamPicture->TurnNightModeOff(iView.GetMbmName(), 
                                         EMbmWficonsSpeedcamera_square, 
                                         EMbmWficonsSpeedcamera_square);
      iDetourPicture->TurnNightModeOff(iView.GetMbmName(), 
                                       EMbmWficonsDetour_square, 
                                       EMbmWficonsDetour_square_mask);
      iExitPicture->NightFilter(aNightMode);
   }
*/
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iPreviewContent->SetNightModeL(aNightMode, fgColor, bgColor);
#endif

   Window().Invalidate();
}

void CGuideContainerPreviewController::FocusChanged(TDrawNow aDrawNow)
{
#if defined NAV2_CLIENT_SERIES60_V32
   // s60v3fp2 phones does not handle the preview pane very well when it 
   // comes to options menus and dialogs that should be displayed on top.
   // So we hide the preview pane manually when we lose focus.
   if (IsFocused()) {
      ShowPreviewContent(ETrue);
   } else {
      ShowPreviewContent(EFalse);
   }
#endif
}

void CGuideContainerPreviewController::ShowPreviewContent(TBool aShow)
{
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   if (!iPreviewController) {
      return;
   }
   if (aShow) {
      iPreviewController->ShowPopUp();
   } else {
      iPreviewController->HidePopUp();
   }
#else
   aShow = aShow;
#endif
}
