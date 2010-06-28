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

#ifndef GUIDE_PROGRESS_BAR_H
#define GUIDE_PROGRESS_BAR_H

#include <coecntrl.h>

/**
 * The Guide views progress bar.
 * This class provides the progress bar in the guide view to all platforms.
 */
class CGuideProgressBar : public CCoeControl
{
private:
   /**
    * Class constructor
    *
    * @param 
    */
   CGuideProgressBar(class CSurface* aBlitUiqSurface);

   /**
    * Symbian second stage ConstructL.
    */
   void ConstructL(CCoeControl& aParent,
                   const TRect& aRect,
                   const TDesC& aMbmFile,
                   TInt aTriangleImg,
                   TInt aTriangleActiveImg,
                   TInt aTriangleMask,
                   TInt aRectImg,
                   TInt aRectActiveImg,
                   TInt aRectMask);

public:
   /**
    * Class Destructor
    */
   virtual ~CGuideProgressBar();

   /**
    * Symbian static NewLC function.
    *
    * @param aParent              This controls parent.
    * @param aRect                The rect for this control.
    * @param aMbmFile             The path of the mbm file.
    * @param aTriangleImg         The triangle image of a passed distance.
    * @param aTriangleActiveImg   The triangle image of an unpassed distance.
    * @param aTriangleMask        The triangle image mask.
    * @param aRectImg             The rectangle image of a passed distance.
    * @param aRectActiveImg       The rectangle image of an unpassed distance.
    * @param aRectMask            The rectangle image mask.
    * @param aBlitUiqSurface      Of screen surface to reduce flicker on UIQ.
    * @return                     A GuideProgressBar instance.
    */
   static CGuideProgressBar* NewLC(CCoeControl& aParent,
                                   const TRect& aRect,
                                   const TDesC& aMbmFile,
                                   TInt aTriangleImg,
                                   TInt aTriangleActiveImg,
                                   TInt aTriangleMask,
                                   TInt aRectImg,
                                   TInt aRectActiveImg,
                                   TInt aRectMask,
                                   class CSurface* aBlitUiqSurface = NULL);

   /**
    * Symbian static NewL function.
    *
    * @param aParent              This controls parent.
    * @param aRect                The rect for this control.
    * @param aMbmFile             The path of the mbm file.
    * @param aTriangleImg         The triangle image of a passed distance.
    * @param aTriangleActiveImg   The triangle image of an unpassed distance.
    * @param aTriangleMask        The triangle image mask.
    * @param aRectImg             The rectangle image of a passed distance.
    * @param aRectActiveImg       The rectangle image of an unpassed distance.
    * @param aRectMask            The rectangle image mask.
    * @param aBlitUiqSurface      Of screen surface to reduce flicker on UIQ.
    * @return                     A GuideProgressBar instance.
    */
   static CGuideProgressBar* NewL(CCoeControl& aParent,
                                  const TRect& aRect,
                                  const TDesC& aMbmFile,
                                  TInt aTriangleImg,
                                  TInt aTriangleActiveImg,
                                  TInt aTriangleMask,
                                  TInt aRectImg,
                                  TInt aRectActiveImg,
                                  TInt aRectMask,
                                  class CSurface* aBlitUiqSurface = NULL);

   /**
    * Public function to set the current distance to a turn.
    * This function also invalidates the parts of the control that 
    * needs to be redrawn.
    *
    * @param aDistance   The current distance to the next turn.
    */
   void SetDistance(TInt aDistance);

   /**
    * Calculates and sets the size of the images
    * used to create this progress bar.
    * @param aRect the rect that is used when calculating the size.
    */
   void SetSizeAndPosition(const TRect& aRect);

   /** 
    * Returns current distance.
    * @return Current distance
    */
   TInt GetDistance();


   void SetNightMode();

private:
   /**
    * Inherited from CCoeControl to control what we redraw.
    */
   virtual void Draw(const TRect& aRect) const;

private:
   //Pointer to back buffer needed for UIQ to reduce flicker.
   class CSurface* iUiqBuffer;

   RArray<TInt> iLimits;
   TInt iDistance;
   class CFbsBitmap* iTriangleImg;
   class CFbsBitmap* iTriangleActiveImg;
   class CFbsBitmap* iTriangleImg_mask;
   class CFbsBitmap* iRectImg;
   class CFbsBitmap* iRectActiveImg;
   class CFbsBitmap* iRectImg_mask;
};

#endif
