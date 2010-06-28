/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _GUIDE_PREVIEW_POP_UP_CONTENT_H_
#define _GUIDE_PREVIEW_POP_UP_CONTENT_H_

#include <coecntrl.h>

class CGuidePreviewPopUpContent : public CCoeControl
{
public:
   /**
    * NewL.
    * Two-phased constructor.
    * Create a CGuidePreviewPopUpContent, which will draw itself to aRect.
    *
    * @return a pointer to the created instance of CGuidePreviewPopUpContent.
    */
   static CGuidePreviewPopUpContent* NewL();

   /**
    * NewLC.
    * Two-phased constructor.
    * Create a CGuidePreviewPopUpContent object, which will draw itself
    * to aRect.
    *
    * @return A pointer to the created instance of CGuidePreviewPopUpContent.
    */
   static CGuidePreviewPopUpContent* NewLC();

   /**
    * ~CGuidePreviewPopUpContent
    * Virtual Destructor.
    */
   virtual ~CGuidePreviewPopUpContent();

protected:
   /**
    * CGuidePreviewPopUpContent.
    * C++ default constructor.
    */
   CGuidePreviewPopUpContent();

private:
   /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CGuidePreviewPopUpContent object.
    *
    * @param aRect The initial rect for the content.
    */
   void ConstructL();

private: // Functions from base classes
   /**
    * From CCoeControl, Draw
    * Draw this CGuidePreviewPopUpContent to the screen.
    * If the user has given a text, it is also printed to the center of
    * the screen.
    *
    * @param aRect the rectangle of this view that needs updating
    */
   virtual void Draw(const TRect& aRect) const;
   
protected: // Functions from base classes
   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the view size is changed.
    */
   virtual void SizeChanged();
   
public: // Functions from base classes
   /**
    * From CoeControl, MinimumSize.
    */
   virtual TSize MinimumSize();

public: // Public functions

   void InitialiseL(const TRect& aRect);

   /**
    * Set up the size and layout of the preview pop up control.
    *
    * @param aRect      The rect that is available to the control.
    * @param aPadding   The padding to use between labels and borders.
    */
   void SetSizeAndLayout(TRect aRect, TInt aPadding);

   /**
    * Set the current and next street labels.
    */
   void SetStreetsL(const TDesC& aCurrName, const TDesC& aNextName);

   /**
    * Sets the distance to goal label.
    */
   void SetDistanceToGoalL(const TDesC& aDistance);

   /**
    * Sets the estimated time to goal label.
    */
   void SetEtgL(const TDesC& aTimeLeft);

   /**
    * Sets the estimated time of arrival label.
    */
   void SetEtaL(const TDesC& aArrivalTime);

   /**
    * Switches night mode on and off.
    */
   void SetNightModeL(TBool aNightMode, TRgb aFgColor, TRgb aBgColor);

private: 
   /**
    * Convenience function to set the correct font in the labels.
    */
   void UpdateLabelsFont();

   /**
    * Function to hide and unhide labels based on available space.
    */
   void UpdateLabelsVisibility();

private: // Private members

   /// Label that writes the current street title text
   class CEikLabel* iCurrStreetTitle;

   /// Label that writes the actaul name of the current street
   class CEikLabel* iCurrStreetLabel;

   /// Label that writes the next street title text
   class CEikLabel* iNextStreetTitle;

   /// Label that writes the actual name of the next street
   class CEikLabel* iNextStreetLabel;

   /// Label that writes the distance left title text
   class CEikLabel* iDistLeftTitle;

   /// Label that writes the distance left
   class CEikLabel* iDistLeftLabel;

   /// Label that writes the time left title text
   class CEikLabel* iTimeLeftTitle;

   /// Label that writes the time left
   class CEikLabel* iTimeLeftLabel;

   /// Label that writes the arrival time title text
   class CEikLabel* iArrTimeTitle;

   /// Label that writes the arrival time
   class CEikLabel* iArrTimeLabel;

   /// The rect available to the different components in the container
   TRect iComponentRect;

   /// The padding to use between the components
   TInt iPadding;

   /// The text color according to the current skin.
   TRgb iSkinTextColor;

   /// Bool flag for night mode true or false.
   TBool iIsNightMode;

   /// The night mode text color.
   TRgb iNightTextColor;

   /// The night mode background color.
   TRgb iNightBackColor;
};

#endif // _GUIDE_PREVIEW_POP_UP_CONTAINER_H_
