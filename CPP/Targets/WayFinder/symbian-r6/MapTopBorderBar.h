/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _MAP_TOP_BORDER_BAR_H_
#define _MAP_TOP_BORDER_BAR_H_

#include <coecntrl.h>
#include <e32cmn.h>
//#include <vector>

class CMapTopBorderBar : public CCoeControl
{
public:
   /**
    * NewL.
    * Two-phased constructor.
    * Create a CMapTopBorderBar object, which contains a number of
    * CCoeControls that are drawn to this controls rect.
    *
    * @param aParent The parent of this control if not window owning.
    * @param aRect The rectangle this view will be drawn to.
    * @param aMbmFilePath The path and filename to the mbm file where 
    *                     to find the image ids.
    * @param aTextColor, The color of the text in the text based components.
    * @param aNightModeTextColor, The nightmode color of the text in the text based components.
    */
    static CMapTopBorderBar* NewLC(CCoeControl* aParent,
                                   const TRect& aRect,
                                   const TDesC& aMbmFilePath,
                                   const TRgb& aTextColor,
                                   const TRgb& aNightModeTextColor);

   /**
    * NewL.
    * Two-phased constructor.
    * Create a CMapTopBorderBar object, which contains a number of
    * CCoeControls that are drawn to this controls rect.
    *
    * @param aRect The rectangle this view will be drawn to.
    * @param aMbmFilePath The path and filename to the mbm file where 
    *                     to find the image ids.
    * @param aTextColor, The color of the text in the text based components.
    * @param aNightModeTextColor, The nightmode color of the text in the text based components.
    */
    static CMapTopBorderBar* NewL(CCoeControl* aParent,
                                  const TRect& aRect,
                                  const TDesC& aMbmFilePath,
                                  const TRgb& aTextColor,
                                  const TRgb& aNightModeTextColor);

   /**
    * ~CMapTopBorderBar
    * Virtual Destructor.
    */
   virtual ~CMapTopBorderBar();

public: // New functions
   
   /**
    * Sets the image of iBackgroundImage.
    *
    * @param aImageId, the id for the image.
    * @param aImageMaskId, the id for the mask.
    */
   void SetBackgroundImageL(TInt aImageId,  
                            TInt aImageMaskId);

   /**
    * Sets the color of the background.
    * Will only be used if no background image has been set.
    *
    * @param aColor, the color for the background.
    * @param aNightModeColor, the color for the background used during night mode.
    */
   void SetBackgroundColors(const TRgb& aColor, const TRgb& aNightModeColor);

   /**
    * Sets the image of iDividerImage, this imaged seperates
    * the distance control from the speed and etg controls.
    *
    * @param aImageId, the id for the image.
    * @param aImageMaskId, the id for the mask.
    */
   void SetDividerImageL(TInt aImageId,  
                         TInt aImageMaskId);

   /**
    * Sets the image of iTurnImage.
    *
    * @param aImageId, the id for the image.
    * @param aImageMaskId, the id for the mask.
    */
   void SetTurnImageL(TInt aImageId,  
                      TInt aImageMaskId);

   /**
    * Sets the exit counts image for round abouts.
    */
   void SetExitCountImageL(TInt aImageId, TInt aImageMaskId);

   /**
    * Sets the text of iDistanceControl.
    *
    * @param aDistance, the new distance.
    * @param aUnit, the unit that the aDistance is in.
    */
   void SetDistanceL(const TDesC& aDistance, const TDesC& aUnit);

   /**
    * Sets the text of iDistanceControl.
    *
    * @param aDistance, the new distance.
    * @param aDistanceMode, the distance mode to use.
    */
   void SetDistanceL(TUint aDistance, TInt aDistanceMode);

   /**
    * Sets the text of iETGControl.
    *
    * @param aETG, the new estimated time to goal.
    */
   void SetETGL(const TDesC& aETG);

   /**
    * Sets the text of iETGControl.
    *
    * @param aETG, the new estimated time to goal.
    */
   void SetETGL(TInt aETG);

   /**
    * Sets the text of iSpeedControl.
    *
    * @param aSpeed, the new speed.
    * @param aDistanceMode, the distance mode to use.
    */
   void SetSpeedL(const TDesC& aSpeed, TInt aDistanceMode);

   /**
    * Sets the text of iSpeedControl.
    *
    * @param aSpeed, the new speed.
    * @param aDistanceMode, the distance mode to use.
    */
   void SetSpeedL(TInt aSpeed, TInt aDistanceMode);

   /**
    * Turns the night mode on or off.
    * @param aNightMode ETrue to turn the nightmode on, EFalse for off
    **/
   void SetNightModeL(TBool aNightMode);

   /**
    * Enum describing a logical font size. Used as in index
    * in iFontVec. 
    */
   enum TFontSize {
      ELargeFont         = 0,
      EMediumFont        = 1,
      ESmallFont         = 2,
      EFontUninitialized = 3,
   };

protected:

   /**
    * CMapTopBorderBar.
    * C++ default constructor.
    */   
   CMapTopBorderBar();

protected: // Functions from base classes

   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the view size is changed.
    */
   virtual void SizeChanged();

private:

   /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CMapTopBorderBar object.
    *
    * @param aParent The parent if the control shouldnt be window owning.
    * @param aRect The rectangle this view will be drawn to.
    * @param aMbmFilePath The path and filename to the mbm file where 
    *                     to find the image ids.
    * @param aTextColor, The color of the text in the text based components.
    * @param aNightModeTextColor, The nightmode color of the text in the text based components.
    */
   void ConstructL(CCoeControl* aParent,
                   const TRect& aRect,
                   const TDesC& aMbmFilePath,
                   const TRgb& aTextColor,
                   const TRgb& aNightModeTextColor);

   /**
    * Calculates the height and width of a font and returns the font
    * that fits the best within the max width and height given. 
    * The descent of the font is also required to fint within the given
    * height.
    * @param aTextToFit The text that should measured.
    * @param aMaxHeightInPixels The maximum height in pixels of the font
    *                           that includes the descent.
    * @param aMaxWidthInPixels  The maximum width in pixels of the font, 
    *                           when measuring the width of the font with
    *                           the given text that should fit.
    * @param aIgnoreDescent    Send ETrue if you want the function to ignore 
    *                          the descent of the font when it tries to find
    *                          the biggest possible font.
    * @return A pointer to a CFont object that fits within the given 
    *         parameters. Note that the font should be released from the
    *         Window server when not using it anymore.
    */
   class CFont* CalculateFontSizeL(const TDesC& aTextToFit,
                                   TInt aMaxHeightInPixels,
                                   TInt aMaxWidthInPixels,
                                   bool aIgnoreDescent = EFalse);

   /**
    * Requests a font from the CWsScreenDevice server with
    * the given font height.
    *
    * @param aMaxFontHeight, the maximum height of the font.
    * @param aMaxFontHeight, the maximum width of the font.
    * @return CFont, the requested font. The caller has to
    *                release the font from the CWsScreenDevice
    *                by a call to CWsScreenDevice::ReleaseFont.
    */ 
   class CFont* GetFontL(TInt aMaxFontHeight, 
                         TInt aMaxFontWidth, 
                         const TDesC& aTextToFit);

   /**
    * Convenience function that compares the given length against
    * the length of example texts and returns the logical font size
    * that should be used.
    * Note that the returned value is a enum value and for it to work
    * the fonts that are pushed to iFontVec needs to be pushed in order,
    * that is the largest font first and the smalles font last.
    *
    * @param aLenght, the length to compare.
    * @return a logical font size used as an index in iFontVec.
    */ 
   TFontSize GetFontSize(TInt aLength) const;

private: // Functions from base classes

   /**
    * From CoeControl, Draw.
    * Called by framework when a redraw has been triggered.
    */
   void Draw(const TRect& aRect) const;

   /**
    * Changes the way the unit for the distance is displayed.
    * @param aSameRow ETrue if the unit should be on the same row, EFalse otherwise.
    */
   void ShowDistanceUnitOnSameRow(TBool aSameRow = ETrue);
      
   void HandleResourceChange(TInt aType);
private:
   /// Background image
   class CImageHandler* iBackgroundImage;   
   /// Divider image
   class CImageHandler* iDividerImage;
   /// Next turn image.
   class CImageHandler* iTurnImage;
   /// The exit count image for roundabouts.
   class CImageHandler* iExitPicture;
   /// Gps status image
   class CImageHandler* iGpsImage;
   /// Control displaying distance to next turn
   class CDrawComponent* iDistanceControl;
   /// Control displaying distance unit to next turn
   class CDrawComponent* iDistanceUnitControl;
   /// Control displaying ETG
   class CDrawComponent* iETGControl;
   /// Control displaying speed
   class CDrawComponent* iSpeedControl;
   /// Array holding the fonts for distance label
   RPointerArray<class CFont> iFontVec;
   /// Path and name of the mbm file in which icons are located
   HBufC* iMbmFilePath;
   TRgb iBackgroundColor;
   TRgb iNightModeBackgroundColor;
   TRgb iTextColor;
   TRgb iNightModeTextColor;
   TBool iNightMode;
   TBool iDistanceUnitOnSameRow;
   TUint iLastDistance;
   TInt iLastDistanceMode;
};

#endif
