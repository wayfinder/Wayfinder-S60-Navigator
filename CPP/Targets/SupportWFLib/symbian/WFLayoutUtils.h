/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WF_LAYOUT_UTILS_H
#define WF_LAYOUT_UTILS_H

#include <e32def.h>
#include <e32const.h> 

class WFLayoutUtils
{
 public:

   /**
    * Returns the size of the main pane.
    * @return TSize the size of the main pane.
    */
   static class TSize GetMainPaneSize();

   /**
    * Returns the full screen size.
    * @return TSize the full screen size.
    */
   static class TSize GetFullscreenSize();

   /**
    * Returns the rect of the main pane.
    * @return TRect the rect of the main pane.
    */
   static class TRect GetMainPaneRect();

   /**
    * Returns the rect of the main pane. If the screen is in wide 
    * screen format and the caller wants to adapt the rect for that,
    * a centered rect that is aSWidthPerc percentage of the screen is 
    * returned.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the rect
    *                    to be returned.
    * @return TRect the rect of the main pane.
    */
   static class TRect GetMainPaneRect(TBool aWideScreenAdapt, TInt aSWidthPerc);

   /**
    * Returns the full screen rect.
    * @return TRect the full screen rect.
    */
   static class TRect GetFullScreenRect();

   /**
    * Returns the context pane rect.
    * @return TRect the context pane rect.
    */
   static class TRect GetContextPaneRect();

   /**
    * Returns the full screen rect minus the space for the cba buttons.
    * @return TRect the full screen minus cba buttons rect.
    */
   static class TRect GetFullScreenMinusCBARect();

   /**
    * Returns the full screen rect minus the space for the cba buttons
    * in portrait mode. And returns the main pane for landscape to 
    * still keep correctly drawed cba buttons on the side.
    * @return TRect the correct rect.
    */
   static class TRect GetDynamicAdjustedFullScreenRect();

   /**
    * Calculates a size of a component using the relative
    * sizes based on the sizes of the main pane on s60v2.
    * @param aWidth The width of the component on s60v2.
    * @param aHeight The height of the component on s60v2.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TSize The size of the component.
    */
   static class TSize CalculateSizeUsingMainPane(TInt aWidth, TInt aHeight, 
                                                 TBool aWideScreenAdapt = 0, 
                                                 TInt aSWidthPerc = 60);

   /**
    * Calculates a size of a component using the relative
    * sizes based on the sizes of the main pane on s60v2.
    * @param aSize The size of the component on s60v2.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TSize The size of the component.
    */
   static class TSize CalculateSizeUsingMainPane(TSize aSize, 
                                                 TBool aWideScreenAdapt = 0, 
                                                 TInt aSWidthPerc = 60);

   /**
    * Calculates a size of a component using the width and
    * height expressed as a percentage of the size compared
    * to the screen size (main pane).
    * @param aWidth The width in per cent of the screen width.
    * @param aHeight The height in per cent of the screen height.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TSize The size of the component.
    */   
   static class TSize CalculateSizeUsingMainPaneFloat(float aWidth, float aHeight, 
                                                      TBool aWideScreenAdapt = 0, 
                                                      TInt aSWidthPerc = 60);

   /**
    * Calculates a size of a component using the width and
    * height expressed as a percentage of the size compared
    * to a size.
    * @param aSize The size used to calculate the new size.
    * @param aWidth The width in per cent of the screen width.
    * @param aHeight The height in per cent of the screen height.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TSize The size of the component.
    */   
   static class TSize CalculateSizeUsingSize(TSize aSize, float aWidth, float aHeight, 
                                             TBool aWideScreenAdapt = 0, 
                                             TInt aSWidthPerc = 60);

   /**
    * Calculates a size of a component using the width and
    * height expressed as a percentage of the size compared
    * to the screen size (full screen).
    * @param aWidth The width in per cent of the screen width.
    * @param aHeight The height in per cent of the screen height.
    * @return TSize The size of the component.
    */   
   static class TSize CalculateSizeUsingFullScreen(float aWidth, float aHeight);

   /**
    * Calculates a size of a component using the relative
    * sizes based on the sizes of the full screen size 
    * on s60v2.
    * @param aWidth The width of the component on s60v2.
    * @param aHeight The height of the component on s60v2.
    * @return TSize The size of the component.
    */   
   static class TSize CalculateSizeUsingFullScreen(TInt aWidth, TInt aHeight);

   /**
    * Calculates a size of a component using the relative
    * sizes based on the sizes of the full screen size 
    * on s60v2.
    * @param aSize The size of the component on s60v2.
    * @return TSize The size of the component.
    */   
   static class TSize CalculateSizeUsingFullScreen(TSize aSize);

   /**
    * Calculates a size of a component using the relative
    * sizes based on the sizes of the screen width 
    * on s60v2. This function uses only the width of the
    * screen in it�s calculatitions, this comes in handy 
    * when calculating the size of a square, like a picture.
    * @param aWidth The width of the component on s60v2.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TSize The size of the component.
    */   
   static class TSize CalculateSizeUsingWidth(TInt aWidth, 
                                              TBool aWideScreenAdapt = 0, 
                                              TInt aSWidthPerc = 60);

   /**
    * Calculates a size of a component using the relative
    * sizes based on the sizes of the context pane size 
    * on s60v2.
    * @param aSize The size of the component on s60v2.
    * @return TSize The size of the component.
    */  
   static class TSize CalculateSizeUsingContextPane(TSize aSize);

   /**
    * Calculates a size of a component using the relative
    * sizes based on the sizes of the context pane size 
    * on s60v2.
    * @param aWidth The width of the component on s60v2.
    * @param aHeight The height of the component on s60v2.
    * @return TSize The size of the component.
    */   
   static class TSize CalculateSizeUsingContextPane(TInt aWidth, TInt aHeight);
   /**
    * Calculates a point using relative points based 
    * on the size of the main pane on s60v2.
    * @param aX The x coordinate on s60v2.
    * @param aY The y coordinate on s60v2.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TPoint The calculated position.
    */   
   static class TPoint CalculatePosUsingMainPane(TInt aX, TInt aY, 
                                                 TBool aWideScreenAdapt = 0, 
                                                 TInt aSWidthPerc = 60);

   /**
    * Calculates a point using relative points based 
    * on the size of the main pane on s60v2.
    * @param pos The position on s60v2.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TPoint The calculated position.
    */   
   static class TPoint CalculatePosUsingMainPane(TPoint aPos, 
                                                 TBool aWideScreenAdapt = 0, 
                                                 TInt aSWidthPerc = 60);

   /**
    * Calculates a position of a component using the x and
    * y coordinate expressed as a percentage of the width 
    * and height of the screen (main pane).
    * @param aX The x coordinate in per cent of the screen width.
    * @param aY The y coordinate in per cent of the screen height.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TPoint The position of the component.
    */   
   static class TPoint CalculatePosUsingMainPaneFloat(float aX, float aY, 
                                                      TBool aWideScreenAdapt = 0, 
                                                      TInt aSWidthPerc = 60);

   /**
    * Calculates a position of a component using the x and
    * y coordinate expressed as a percentage of the width 
    * and height of the screen.
    * @param aX The x coordinate in per cent of the screen width.
    * @param aY The y coordinate in per cent of the screen height.
    * @return TPoint The position of the component.
    */   
   static class TPoint CalculatePosUsingFullScreen(float aX, float aY);

   /**
    * Calculates a point using relative points based 
    * on the size of the full screen on s60v2.
    * @param aX The x coordinate on s60v2.
    * @param aY The y coordinate on s60v2.
    * @return TPoint The calculated position.
    */   
   static class TPoint CalculatePosUsingFullScreen(TInt aX, TInt aY);

   /**
    * Calculates a position of a component using the x and
    * y coordinate expressed as a percentage of the width 
    * and height of the screen.
    * @param aPos The position The position on s60v2.
    * @return TPoint The position of the component.
    */
   static class TPoint CalculatePosUsingFullScreen(TPoint aPos);

   /**
    * Calculates a point using relative points based 
    * on the size of the context pane on s60v2.
    * @param aX The x coordinate on s60v2.
    * @param aY The y coordinate on s60v2.
    * @return TPoint The calculated position.
    */ 
   static class TPoint CalculatePosUsingContextPane(TInt aX, TInt aY);

   /**
    * Calculates a position of a component using the x and
    * y coordinate expressed as a percentage of the width 
    * and height of the screen.
    * @param aPos The position The position on s60v2.
    * @return TPoint The position of the component.
    */
   static class TPoint CalculatePosUsingContextPane(TPoint aPos);

   /**
    * Calculates a position of a component using the x and
    * y coordinate expressed as a percentage of the width 
    * and height of aPos.
    * @param aPos The position that is used to calculate the new pos.
    * @param aX The x coordinate in per cent of aPos.iX.
    * @param aY The y coordinate in per cent of aPos.iY
    * @return TPoint The position of the component.
    */
   static class TPoint CalculatePosUsingPos(TPoint aPos, float aX, float aY);

   static class TPoint CalculatePosUsingSize(TSize aSize, float aX, float aY);

   /**
    * Calculates the x value based on a x value used
    * on s60v2. This x value could be a width or a
    * x coordinate.
    * @param aXValue The x value used on s60v2.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TInt The calculated x value.
    */
   static TInt CalculateXValue(TInt aXValue, 
                               TBool aWideScreenAdapt = EFalse, 
                               TInt aSWidthPerc = 60);

   /**
    * Calculates the x value based on a x value 
    * expressed as a percentage of the screen width
    * @param aXValue The x value in percentage of 
    *                the screen width.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TInt The calculated x value.
    */
   static TInt CalculateXValueFloat(float aXValue, 
                                    TBool aWideScreenAdapt = 0, 
                                    TInt aSWidthPerc = 60);

   /**
    * Calculates the x value based on a x value 
    * expressed as a percentage of aBaseXValue
    * @param aBaseXValue The value used to calculate the new xValue.
    * @param aXValue The x value in percentage of 
    *                the screen width.
    * @param aWideScreenAdapt True if widescreen adaption should be made,
    *                         False if not.
    * @param aSWidthPerc The percentage of the width to be used for the calculation.
    * @return TInt The calculated x value.
    */
   static TInt CalculateXValue(TInt aBaseXValue, 
                               float aXValue, 
                               TBool aWideScreenAdapt = EFalse, 
                               TInt aSWidthPerc = 60);

   /**
    * Calculates the y value based on a y value used
    * on s60v2 in relation to the screen width.
    * This y value could be a height or a
    * y coordinate.
    * @param aYValue The y value used on s60v2.
    * @return TInt The calculated y value.
    */
   static TInt CalculateYValueUsingMainPane(TInt aYValue);

   /**
    * Calculates the y value based on a y value 
    * expressed as a percentage of the height of 
    * the main pane.
    * @param aYValue The y value in percentage of 
    *                the screen height
    * @return TInt The calculated y value.
    */
   static TInt CalculateYValueUsingMainPane(float aYValue);

   /**
    * Calculates the y value based on a y value used
    * on s60v2 in relation to the screen width. 
    * This y value could be a height or a
    * y coordinate.
    * @param aYValue The y value used on s60v2.
    * @return TInt The calculated y value.
    */
   static TInt CalculateYValueUsingFullScreen(TInt aYValue);

   /**
    * Calculates the y value based on a y value 
    * expressed as a percentage of the height of 
    * the full screen height.
    * @param aYValue The y value in percentage of 
    *                the screen height
    * @return TInt The calculated y value.
    */
   static TInt CalculateYValueUsingFullScreen(float aYValue);

   /**
    * Calculates the y value based on a y value used
    * on s60v2 in relation to the screen width. 
    * This y value could be a height or a
    * y coordinate.
    * @param aYValue The y value used on s60v2.
    * @return TInt The calculated y value.
    */
   static TInt CalculateYValueUsingContextPane(TInt aYValue);

   /** 
    * Calculates the y value based on a y value 
    * expressed as a percentage of aBaseYValue.
    * the full screen height.
    * @param aBaseYValue The y value used to calulate the new yValue.
    * @param aYValue The y value in percentage of 
    *                the screen height
    * @return TInt The calculated y value.
    */
   static TInt CalculateYValue(TInt aBaseYValue, float aYValue);

   /**
    * Calculates the smallest proportion, e.g. if in landscape mode we 
    * use the height of the screen and if we're in portrait mode we use 
    * the width of the screen.
    */
   static TInt CalculateUnitsUsingMin(float aPercentage);

   /**
    * Calculates the largest proportion, e.g. if in landscape mode we 
    * use the width of the screen and if we're in portrait mode we use 
    * the height of the screen.
    */
   static TInt CalculateUnitsUsingMax(float aPercentage);

   /**
    * Calculates the smallest proportion, e.g. if in landscape mode we 
    * use the height of the screen and if we're in portrait mode we use 
    * the width of the screen.
    */
   static TInt CalculateUnitsUsingMin(TInt aOrgUnit);

   /**
    * Calculates the largest proportion, e.g. if in landscape mode we 
    * use the width of the screen and if we're in portrait mode we use 
    * the height of the screen.
    */
   static TInt CalculateUnitsUsingMax(TInt aOrgUnit);

   /**
    * Calculates the smallest proportion, e.g. if in landscape mode we 
    * use the height of the screen and if we're in portrait mode we use 
    * the width of the screen.
    */
   static TSize CalculateSizeUsingMin(float aPercentage);

   /**
    * Calculates the largest proportion, e.g. if in landscape mode we 
    * use the width of the screen and if we're in portrait mode we use 
    * the height of the screen.
    */
   static TSize CalculateSizeUsingMax(float aPercentage);

   /**
    * Calculates the smallest proportion, e.g. if in landscape mode we 
    * use the height of the screen and if we're in portrait mode we use 
    * the width of the screen.
    */
   static TSize CalculateSizeUsingMin(TInt aOrgSize);

   /**
    * Calculates the largest proportion, e.g. if in landscape mode we 
    * use the width of the screen and if we're in portrait mode we use 
    * the height of the screen.
    */
   static TSize CalculateSizeUsingMax(TInt aOrgSize);

   /**
    * Checks if we are in landscape mode or portrait mode.
    * @return ETrue if we are in landscape mode
    *         EFalse if we are in portrait mode.
    */
   static TBool LandscapeMode();

#ifdef NAV2_CLIENT_SERIES60_V3
   /**
    * XXX Does not work because of the framework on s60v3
    * returns EAppUiOrientationUnspecified. XXX
    */
   static TBool LandscapeMode(class CAknAppUiBase& aAppUi);
#endif

   static TInt CalculateDpiCorrectionFactor();

   /**
    * Checks if the phone has a screen size like wide screen,
    * ie E90.
    * @return ETrue if wide screen,
    *         EFalse if not wide screen.
    */
   static TBool WideScreen();

   /**
    * Returns true if the phone has a touch screen, false otherwise.
    *
    * Note that on other platforms then s60 this function always returns false.
    */
   static TBool IsTouchScreen();

};

#endif
