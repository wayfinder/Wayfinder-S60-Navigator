/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "WFLayoutUtils.h"
#ifdef NAV2_CLIENT_SERIES60_V3
#include <aknutils.h>
#include <aknappui.h>
#endif
#include <e32std.h>
#include <e32math.h>

#define V2_MAIN_PANE_HEIGHT    144
#define V2_SCREEN_WIDTH        176
#define V2_FULL_SCREEN_HEIGHT  208
#define V2_CONTEXT_PANE_HEIGHT  44

TSize WFLayoutUtils::GetMainPaneSize()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TSize size;
   AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, size);
   return size;
#else
   return TSize(V2_SCREEN_WIDTH, V2_MAIN_PANE_HEIGHT);
#endif
}

TSize WFLayoutUtils::GetFullscreenSize()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TSize size;
   AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EApplicationWindow, size);
   return size;
#else
   return TSize(V2_SCREEN_WIDTH, V2_FULL_SCREEN_HEIGHT);
#endif   
}

TRect WFLayoutUtils::GetMainPaneRect()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
   return rect;
#else
   return TRect(TPoint(0, V2_CONTEXT_PANE_HEIGHT), 
                TSize(V2_SCREEN_WIDTH, V2_MAIN_PANE_HEIGHT));
#endif
}

TRect WFLayoutUtils::GetMainPaneRect(TBool aWideScreenAdapt, TInt aSWidthPerc)
{
   TRect rect = GetMainPaneRect();
#if defined NAV2_CLIENT_SERIES60_V3
   if (aWideScreenAdapt && WideScreen()) {
      TSize size = GetMainPaneSize();
      TInt width = TInt(float(size.iWidth) * float(aSWidthPerc / 100.0));
      TInt diff  = size.iWidth - width;
      TInt margin = diff >> 1;
      rect.iTl.iX += margin;
      rect.iBr.iX -= margin;
   }
#endif
   return rect;
}

TRect WFLayoutUtils::GetFullScreenRect()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
   return rect;
#else
   return TRect(TSize(V2_SCREEN_WIDTH, V2_FULL_SCREEN_HEIGHT));
#endif   
}

TRect WFLayoutUtils::GetContextPaneRect()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EContextPane, rect);
   return rect;
#else
   return TRect(TSize(V2_SCREEN_WIDTH, V2_CONTEXT_PANE_HEIGHT));
#endif   
}

TRect WFLayoutUtils::GetFullScreenMinusCBARect()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
   rect.iTl = TPoint(0,0);
   return rect;
#else
   return TRect(TSize(V2_SCREEN_WIDTH, V2_MAIN_PANE_HEIGHT));
#endif   
}

TRect WFLayoutUtils::GetDynamicAdjustedFullScreenRect()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   if (LandscapeMode()) {
      AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
   } else {
      AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
      rect.iTl = TPoint(0,0);
   }
   return rect;
#else
   return TRect(TSize(V2_SCREEN_WIDTH, V2_FULL_SCREEN_HEIGHT));
#endif   
}

TSize WFLayoutUtils::CalculateSizeUsingMainPane(TInt aWidth, TInt aHeight, 
                                                TBool aWideScreenAdapt, 
                                                TInt aSWidthPerc)
{
   return TSize(CalculateXValue(aWidth, aWideScreenAdapt, aSWidthPerc), 
                CalculateYValueUsingMainPane(aHeight));
}

TSize WFLayoutUtils::CalculateSizeUsingMainPane(TSize aSize, 
                                                TBool aWideScreenAdapt, 
                                                TInt aSWidthPerc)
{
   return CalculateSizeUsingMainPane(aSize.iWidth, aSize.iHeight, 
                                     aWideScreenAdapt, aSWidthPerc);
}

TSize WFLayoutUtils::CalculateSizeUsingMainPaneFloat(float aWidth, float aHeight, 
                                                     TBool aWideScreenAdapt, 
                                                     TInt aSWidthPerc)
{
   return TSize(CalculateXValueFloat(aWidth, aWideScreenAdapt, aSWidthPerc),
                CalculateYValueUsingMainPane(aHeight));
}

TSize WFLayoutUtils::CalculateSizeUsingSize(TSize aSize, float aWidth, float aHeight, 
                                            TBool aWideScreenAdapt, 
                                            TInt aSWidthPerc)
{
   return TSize(CalculateXValue(aSize.iWidth, aWidth, aWideScreenAdapt, aSWidthPerc), 
                CalculateYValue(aSize.iHeight, aHeight));   
}

TSize WFLayoutUtils::CalculateSizeUsingFullScreen(float aWidth, float aHeight)
{
   return TSize(CalculateXValueFloat(aWidth),
                CalculateYValueUsingFullScreen(aHeight));
}

TSize WFLayoutUtils::CalculateSizeUsingFullScreen(TInt aWidth, TInt aHeight)
{
   return TSize(CalculateXValue(aWidth), 
                CalculateYValueUsingFullScreen(aHeight));
}

TSize WFLayoutUtils::CalculateSizeUsingFullScreen(TSize aSize)
{
   return CalculateSizeUsingFullScreen(aSize.iWidth, aSize.iHeight);
}

TSize WFLayoutUtils::CalculateSizeUsingWidth(TInt aWidth, 
                                             TBool aWideScreenAdapt, 
                                             TInt aSWidthPerc)
{
   return TSize(CalculateXValue(aWidth, aWideScreenAdapt, aSWidthPerc), 
                CalculateXValue(aWidth, aWideScreenAdapt, aSWidthPerc));
}

TSize WFLayoutUtils::CalculateSizeUsingContextPane(TInt aWidth, TInt aHeight)
{
   return TSize(CalculateXValue(aWidth),
                CalculateYValueUsingContextPane(aHeight));
}

TSize WFLayoutUtils::CalculateSizeUsingContextPane(TSize aSize)
{
   return CalculateSizeUsingContextPane(aSize.iWidth, aSize.iHeight);  
}

TPoint WFLayoutUtils::CalculatePosUsingMainPane(TInt aX, TInt aY, 
                                                TBool aWideScreenAdapt, 
                                                TInt aSWidthPerc)
{
   if (WideScreen() && aWideScreenAdapt) {
      TInt tmp = CalculateXValue(aX, aWideScreenAdapt, aSWidthPerc);
      TSize size = GetMainPaneSize();
      TInt width = TInt(float(size.iWidth) * float(aSWidthPerc / 100.0));
      TInt diff  = size.iWidth - width;
      TInt margin = diff >> 1;
      TInt tmp2 = tmp + margin;

      return TPoint(tmp2,
                    CalculateYValueUsingMainPane(aY));
   } 
   return TPoint(CalculateXValue(aX), 
                 CalculateYValueUsingMainPane(aY));
}

TPoint WFLayoutUtils::CalculatePosUsingMainPane(TPoint aPos, 
                                                TBool aWideScreenAdapt, 
                                                TInt aSWidthPerc)
{
   return CalculatePosUsingMainPane(aPos.iX, aPos.iY, 
                                    aWideScreenAdapt, aSWidthPerc);
}

TPoint WFLayoutUtils::CalculatePosUsingMainPaneFloat(float aX, float aY, 
                                                     TBool aWideScreenAdapt, 
                                                     TInt aSWidthPerc)
{
   if (WideScreen() && aWideScreenAdapt) {
      TInt tmp = CalculateXValueFloat(aX, aWideScreenAdapt, aSWidthPerc);
      TSize size = GetMainPaneSize();
      TInt width = TInt(float(size.iWidth) * float(aSWidthPerc / 100.0));
      TInt diff  = size.iWidth - width;
      TInt margin = diff >> 1;
      TInt tmp2 = tmp + margin;

      return TPoint(tmp2,
                    CalculateYValueUsingMainPane(aY));
   } 

   return TPoint(CalculateXValueFloat(aX),
                 CalculateYValueUsingMainPane(aY)); 
}

TPoint WFLayoutUtils::CalculatePosUsingFullScreen(float aX, float aY)
{
   return TPoint(CalculateXValueFloat(aX),
                 CalculateYValueUsingFullScreen(aY));
}

TPoint WFLayoutUtils::CalculatePosUsingFullScreen(TInt aX, TInt aY)
{
   return TPoint(CalculateXValue(aX),
                 CalculateYValueUsingFullScreen(aY));
}

TPoint WFLayoutUtils::CalculatePosUsingFullScreen(TPoint aPos)
{
   return CalculatePosUsingFullScreen(aPos.iX, aPos.iY);
}

TPoint WFLayoutUtils::CalculatePosUsingContextPane(TInt aX, TInt aY)
{
   return TPoint(CalculateXValue(aX),
                 CalculateYValueUsingContextPane(aY));
}

TPoint WFLayoutUtils::CalculatePosUsingContextPane(TPoint aPos)
{
   return CalculatePosUsingContextPane(aPos.iX, aPos.iY);
}

TPoint WFLayoutUtils::CalculatePosUsingPos(TPoint aPos, float aX, float aY)
{
   return TPoint(CalculateXValue(aPos.iX, aX), 
                 CalculateYValue(aPos.iY, aY));   
}

TPoint WFLayoutUtils::CalculatePosUsingSize(TSize aSize, float aX, float aY)
{
   return TPoint(CalculateXValue(aSize.iWidth, aX),
                 CalculateYValue(aSize.iHeight, aY));
}


TInt WFLayoutUtils::CalculateXValue(TInt aXValue, TBool aWideScreenAdapt, TInt aSWidthPerc)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   if (WideScreen() && aWideScreenAdapt) {
      // If widescreen then use a defined percentage of the width.
      return TInt(float((GetMainPaneRect().Width()) * float(aSWidthPerc / 100.0)) * 
                  (float(aXValue) / V2_SCREEN_WIDTH));
   }
   return TInt(float(GetMainPaneRect().Width()) * 
               (float(aXValue) / V2_SCREEN_WIDTH));
#else
   return aXValue;
#endif   
}

TInt WFLayoutUtils::CalculateXValueFloat(float aXValue, TBool aWideScreenAdapt, TInt aSWidthPerc)
{
   if (WideScreen() && aWideScreenAdapt) {
      // If widescreen then use a defined percentage of the width.
      return TInt((GetMainPaneRect().Width() * float(aSWidthPerc / 100.0)) * aXValue);
   }
   return TInt(float(GetMainPaneRect().Width()) * aXValue);
}

TInt WFLayoutUtils::CalculateXValue(TInt aBaseXValue, float aXValue, 
                                    TBool aWideScreenAdapt, TInt aSWidthPerc)
{
   if (WideScreen() && aWideScreenAdapt) {
      return TInt((float(aBaseXValue * float(aSWidthPerc / 100.0))) * aXValue);
   }
   return TInt(float(aBaseXValue) * aXValue);
}

TInt WFLayoutUtils::CalculateYValueUsingMainPane(TInt aYValue)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   return TInt(float(GetMainPaneRect().Height()) * 
               (float(aYValue) / V2_MAIN_PANE_HEIGHT));
#else
   return aYValue;
#endif   
}

TInt WFLayoutUtils::CalculateYValueUsingMainPane(float aYValue)
{
   return TInt(float(GetMainPaneRect().Height()) * aYValue);
}

TInt WFLayoutUtils::CalculateYValueUsingFullScreen(TInt aYValue)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   return TInt(float(GetFullScreenRect().Height()) * 
               (float(aYValue) / V2_FULL_SCREEN_HEIGHT));
#else
   return aYValue;
#endif   
}

TInt WFLayoutUtils::CalculateYValueUsingContextPane(TInt aYValue)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   return TInt(float(GetContextPaneRect().Height()) * 
               (float(aYValue) / V2_CONTEXT_PANE_HEIGHT));
#else
   return aYValue;
#endif    
}

TInt WFLayoutUtils::CalculateYValueUsingFullScreen(float aYValue)
{
   return TInt(float(GetFullScreenRect().Height()) * aYValue);
}

TInt WFLayoutUtils::CalculateYValue(TInt aBaseYValue, float aYValue)
{
   return TInt(float(aBaseYValue) * aYValue);
}

TInt WFLayoutUtils::CalculateUnitsUsingMin(float aPercentage)
{
   if (LandscapeMode()) {
      return TInt(GetFullScreenRect().Height() * aPercentage);
   } 
   return TInt(GetFullScreenRect().Width() * aPercentage);
}

TInt WFLayoutUtils::CalculateUnitsUsingMax(float aPercentage)
{
   if (LandscapeMode()) {
      return TInt(GetFullScreenRect().Width() * aPercentage);
   } 
   return TInt(GetFullScreenRect().Height() * aPercentage);
}

TInt WFLayoutUtils::CalculateUnitsUsingMin(TInt aOrgUnit)
{
   if (LandscapeMode()) {
      return TInt(float(GetFullScreenRect().Height()) * 
                  (float(aOrgUnit) / V2_SCREEN_WIDTH));
   } 
   return TInt(float(GetFullScreenRect().Width()) * 
               (float(aOrgUnit) / V2_SCREEN_WIDTH));
}

TInt WFLayoutUtils::CalculateUnitsUsingMax(TInt aOrgUnit)
{
   if (LandscapeMode()) {
      return TInt(float(GetFullScreenRect().Width()) * 
                  (float(aOrgUnit) / V2_FULL_SCREEN_HEIGHT));
   } 
   return TInt(float(GetFullScreenRect().Height()) * 
               (float(aOrgUnit) / V2_FULL_SCREEN_HEIGHT));
}

TSize WFLayoutUtils::CalculateSizeUsingMin(float aPercentage)
{
   TInt size;
   if (LandscapeMode()) {
      size = TInt(GetFullScreenRect().Height() * aPercentage);
   } else {
      size = TInt(GetFullScreenRect().Width() * aPercentage);
   }
   return TSize(size, size);
}

TSize WFLayoutUtils::CalculateSizeUsingMax(float aPercentage)
{
   TInt size;
   if (LandscapeMode()) {
      size = TInt(GetFullScreenRect().Width() * aPercentage);
   } else {
      size = TInt(GetFullScreenRect().Height() * aPercentage);
   }
   return TSize(size, size);
}

TSize WFLayoutUtils::CalculateSizeUsingMin(TInt aOrgSize)
{
   TInt size;
   if (LandscapeMode()) {
      size = TInt(float(GetFullScreenRect().Height()) * 
                  (float(aOrgSize) / V2_SCREEN_WIDTH));
   } else {
      size = TInt(float(GetFullScreenRect().Width()) * 
                  (float(aOrgSize) / V2_SCREEN_WIDTH));
   }
   return TSize(size, size);
}

TSize WFLayoutUtils::CalculateSizeUsingMax(TInt aOrgSize)
{
   TInt size;
   if (LandscapeMode()) {
      size = TInt(float(GetFullScreenRect().Width()) * 
                  (float(aOrgSize) / V2_FULL_SCREEN_HEIGHT));
   } else {
      size = TInt(float(GetFullScreenRect().Height()) * 
                  (float(aOrgSize) / V2_FULL_SCREEN_HEIGHT));
   }
   return TSize(size, size);
}

TBool WFLayoutUtils::LandscapeMode()
{
   if (GetFullscreenSize().iWidth == GetFullscreenSize().iHeight) {
      // This is not good but it is a fix for the 5500 that has a
      // square screen, 208x208. When sizing images (like news) based
      // on the main pane we need to use the smalles size (in this case
      // the height)
      return ETrue;
   } 
   return (GetFullscreenSize().iWidth > GetFullscreenSize().iHeight);
}

#ifdef NAV2_CLIENT_SERIES60_V3
TBool WFLayoutUtils::LandscapeMode(class CAknAppUiBase& aAppUi)
{
   return (aAppUi.Orientation() == CAknAppUiBase::EAppUiOrientationLandscape);
}
#endif

TInt WFLayoutUtils::CalculateDpiCorrectionFactor()
{
   TSize aBaseSize(V2_SCREEN_WIDTH,  V2_FULL_SCREEN_HEIGHT);
   TReal aBaseArea = aBaseSize.iWidth * aBaseSize.iHeight / 2;
   TSize size = GetFullscreenSize();
   TReal aArea = size.iWidth * size.iHeight / 2;
   TReal corrFact;
   Math::Sqrt(corrFact, (aArea / aBaseArea));
   return (TInt)corrFact;
}

TBool WFLayoutUtils::WideScreen()
{
   return ((GetFullscreenSize().iWidth >= TInt(GetFullscreenSize().iHeight * 2)));
}

TBool WFLayoutUtils::IsTouchScreen()
{
#ifdef NAV2_CLIENT_SERIES60_V5
   return AknLayoutUtils::PenEnabled();
#endif
   return EFalse;
}
