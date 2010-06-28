/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _PEDESTRIAN_POSITION_CONTROL_H_
#define _PEDESTRIAN_POSITION_CONTROL_H_

#include <coecntrl.h>

/**
 * This class draws a control, if the background should be transparent an
 * alpha value should be set in the background color and the
 * control should have a parent, this control should not be
 * window owning in this case.
 */
class CPedestrianPositionControl : public CCoeControl
{
public:
   /**
    * NewL.
    * Two-phased constructor.
    * Create a CPedestrianPositionControl object, which will draw itself to 
    * aRect.
    *
    * @param aParent The parent of this control if not window owning.
    * @param aRect The rectangle this control will be drawn in.
    * @param aColor The color of this control.
    * @param aBgColor The background color of this control.
    */
    static CPedestrianPositionControl* NewLC(CCoeControl* aParent,
                                             const TRect& aRect,
                                             const TSize& aCenterSize,
                                             const TRgb& aColor,
                                             const TRgb& aBgColor);

   /**
    * NewL.
    * Two-phased constructor.
    * Create a CPedestrianPositionControl object, which will draw itself to 
    * aRect.
    *
    * @param aParent The parent of this control if not window owning.
    * @param aRect The rectangle this control will be drawn in.
    * @param aColor The color of this control.
    * @param aBgColor The background color of this control.
    */
    static CPedestrianPositionControl* NewL(CCoeControl* aParent,
                                            const TRect& aRect,
                                            const TSize& aCenterSize,
                                            const TRgb& aColor,
                                            const TRgb& aBgColor);

   /**
    * ~CPedestrianPositionControl
    * Virtual Destructor.
    */
   virtual ~CPedestrianPositionControl();

protected:

   /**
    * CPedestrianPositionControl.
    * C++ default constructor.
    * @param aColor The color of this control.
    * @param aBgColor The background color of this control.
    */   
   CPedestrianPositionControl(const TRgb& aColor, const TRgb& aBgColor);

private:

   /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CPedestrianPositionControl object.
    *
    * @param aRect The rectangle this control will be drawn in.
    */
   void ConstructL(CCoeControl* aParent,
                   const TRect& aRect,
                   const TSize& aCenterSize);

public: // New functions

   void SetCenter(TPoint aCenter);

protected: // Functions from base classes

   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the controls position is changed.
    */
   void PositionChanged();

   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the controls size is changed.
    */
   virtual void SizeChanged();

private: // Functions from base classes

   /**
    * From CoeControl, Draw.
    * Called by framework when a redraw has been triggered.
    */
   void Draw(const TRect& aRect) const;
      
private:
   /// Color of the control.
   TRgb iColor;
   /// Color of the background.
   TRgb iBgColor;
   /// Thikness of drawn lines.
   TSize iLineSize;
   // The center points rect.
   TRect iCenterRect;
};

#endif // PEDESTRIAN_POSITION_CONTROL
