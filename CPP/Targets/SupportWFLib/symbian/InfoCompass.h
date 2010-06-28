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

#ifndef INFOCOMPASS_H
#define INFOCOMPASS_H

#if !(defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3)
// INCLUDES
#include <bitdev.h>
#include <coecntrl.h>  // for coe control

///This control is used to display a compass.  It is supposed to be
///equipped with a bitmap and a mask, and then paints a line from the
///center of the control in the set direction.
class CInfoCompass : public CCoeControl
{
   ///@name Constructors and destructor
   //@{
   ///Constructor.
   CInfoCompass();
   ///Second phase constructor.
   ///@param aParent the containing control. 
   ///@param aRect the area that this control should occupy. 
   ///@param aMbmFile the MBM-file that images should be loaded from.
   ///@param aCompassImage the resource id of the compass image.
   ///@param aCompassMask the mask used with the compass image.
   ///@param aInvertMask If true, a source pixel that is masked by a
   ///                   black pixel is not transferred to the
   ///                   destination rectangle. If false, a source
   ///                   pixel that is masked by a white pixel is not
   ///                   transferred to the destination rectangle.
   ///@param aNorthArrowColor The color that the north arrow should have.
   ///@param aSouthArrowColor The color that the south arrow should have.
   void ConstructL(class CCoeControl& aParent, const TRect& aRect, 
                   const TDesC& aMbmFile,  TInt aCompassImage, 
                   TInt aCompassMask, TBool aInvertMask,
                   TRgb aNorthArrowColor, TRgb aSouthArrowColor);
public:
   ///Static constructor.
   ///@param aParent the containing control. 
   ///@param aRect the area that this control should occupy. 
   ///@param aMbmFile the MBM-file that images should be loaded from.
   ///@param aCompassImage the resource id of the compass image.
   ///@param aCompassMask the mask used with the compass image. 
   ///2return a new CInfoCompass object.
   ///@param aInvertMask If true, a source pixel that is masked by a
   ///                   black pixel is not transferred to the
   ///                   destination rectangle. If false, a source
   ///                   pixel that is masked by a white pixel is not
   ///                   transferred to the destination rectangle.
   ///@param aNorthArrowColor The color that the north arrow should have.
   ///@param aSouthArrowColor The color that the south arrow should have.
   static class CInfoCompass* NewL(class CCoeControl& aParent, 
                                   const TRect& aRect, const TDesC& aMbmFile, 
                                   TInt aCompassImage, TInt aCompassMask,
                                   TBool aInvertMask = EFalse,
                                   TRgb northArrowColor = TRgb(255,255,255),
                                   TRgb southArrowColor = TRgb(100,100,255));

   ///Virtual destructor.
   virtual ~CInfoCompass();
   //@}

   ///Sets the heading this compass should indicate. The heading value
   ///follows normal trigonometric conventions regarding negative
   ///values and values larger than a whole lap.
   ///@param aHeading the heading in degrees. There is 360 degrees to
   ///                a lap. Degrees are counted clockwise with zero 
   ///                due north. 
   ///                NOTE: Using the value -1 as argument will remove 
   ///                      the arrow.
   void SetHeading(TInt aHeading, TBool aForceUpdate = EFalse );
   ///Sets the heading this compass should indicate.  In this version
   ///of setheadin, all values are valid positions on the lap.
   ///@param aHeading the heading as 256-parts of a lap. There is 256 
   ///                valid positions on a lap. Steps are counted 
   ///                clockwise with zero due north. 
   void SetHeading(TUint8 aHeading );
   ///Sets the heading this compass should indicate. The heading value
   ///follows normal trigonometric conventions regarding negative
   ///values and values larger than a whole lap.
   ///@param aHeading the heading in radians. There is 2*pi radians to
   ///                a lap. Radians are counted clockwise with zero 
   ///                due north. 
   void SetHeading(TReal aHeading );

   ///Clears the compass, that is removes the arrow until a new
   ///heading has been set.
   void Clear();
   
   ///Updates the size and rectangle for the image.
   ///Called when user switches between portrait and
   ///landscape mode.
   void SetCompassSizeAndPos(const TRect& aRect);

   ///Sets a new image, is used when going in to and out of
   ///night mode. 
   ///@param aRect the area that this control should occupy. 
   ///@param aMbmFile the MBM-file that images should be loaded from.
   ///@param aCompassImage the resource id of the compass image.
   ///@param aCompassMask the mask used with the compass image.
   ///@param aInvertMask If true, a source pixel that is masked by a
   ///                   black pixel is not transferred to the
   ///                   destination rectangle. If false, a source
   ///                   pixel that is masked by a white pixel is not
   ///                   transferred to the destination rectangle.
   ///@param aNorthArrowColor The color that the north arrow should have.
   ///@param aSouthArrowColor The color that the south arrow should have.
   void SetNewImage(const TRect& aRect, 
                    const TDesC& aMbmFile, 
                    TInt aCompassImage,
                    TInt aCompassMask,
                    TBool aInvertMask = EFalse,
                    TRgb northArrowColor = TRgb(255,255,255),
                    TRgb southArrowColor = TRgb(100,100,255));


private: // Functions from base classes
   ///Recalculate iLength and set iHeadPoint to center.
   void UpdatePoints();
   ///Replace the current value of iHeadPoint with the
   ///argument. Causes a redraw if aNewHead != iHeadPoint
   ///@param aNewHead the new arrow head position. 
   void SwitchHead(TPoint aNewHead);
   ///@name From CCoeControl.
   //@{
   virtual void Draw(const TRect& aRect) const;
   virtual void SizeChanged(); 
public:
   virtual TSize MinimumSize();
   //@}
private:
   ///The latest set heading.
   TInt iHeading;
   /// The point where the compass arrow's head will lie.
   TPoint iHeadPoint;
   /// The length of the compass arrow
   TInt iLength;
   ///The arrow margin. The arrow length will be min(width, height) - margin
   TReal iMargin;

   ///Compass bitmap
   class CFbsBitmap* iCompassImage;   
   ///Compass image mask. 
   class CFbsBitmap* iCompassImage_mask;
   /**
    * If true, a source pixel that is masked by a black pixel is not
    * transferred to the destination rectangle. If false, a source
    * pixel that is masked by a white pixel is not transferred to the
    * destination rectangle.
    */
   TBool iInvertMask;
   /// Color of the north arrow
   TRgb iNorthArrowColor;
   /// Color of the south arrow
   TRgb iSouthArrawColor;
};

#endif

#endif
// End of File

