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

#ifndef MAP_BORDER_BAR_H
#define MAP_BORDER_BAR_H

#include <coecntrl.h>

/**
 * The Map views border bar.
 * This class provides the border bar in the map view to all platforms.
 * 
 */
class CMapBorderBar : public CCoeControl
{
private:
   /**
    * Class constructor
    *
    * @param 
    */
   CMapBorderBar();

   /**
    * Symbian second stage ConstructL.
    */
   void ConstructL(CCoeControl& aParent,
                   const TRect& aRect,
                   const TDesC& aMbmFile,
                   const TInt* aMbmImageIds,
                   const TReal* aMbmImageRelations,
                   class CSurface* aSurface);

   /** 
    * Inits all class bitmapcontrols and 
    * DistanceBitmapHelpers
    */ 
   void InitBitmaps(const TRect& aRect);

public:
   /**
    * Class Destructor
    */
   virtual ~CMapBorderBar();

   /**
    * Symbian static NewLC function.
    *
    * @param aParent              This controls parent.
    * @param aRect                The rect for this control.
    * @param aMbmFile             The path of the mbm file.
    * @param aMbmImageIds         Array of needed mbm image ids.
    * @return                     A MapBorderBar instance.
    */
   static CMapBorderBar* NewLC(CCoeControl& aParent,
                               const TRect& aRect,
                               const TDesC& aMbmFile,
                               const TInt* aMbmImageIds,
                               const TReal* aMbmImageRelations = NULL,
                               class CSurface* aSurface = NULL);

   /**
    * Symbian static NewL function.
    *
    * @param aParent              This controls parent.
    * @param aRect                The rect for this control.
    * @param aMbmFile             The path of the mbm file.
    * @param aMbmImageIds         Array of needed mbm image ids.
    * @return                     A MapBorderBar instance.
    */
   static CMapBorderBar* NewL(CCoeControl& aParent,
                              const TRect& aRect,
                              const TDesC& aMbmFile,
                              const TInt* aMbmImageIds,
                              const TReal* aMbmImageRelations = NULL,
                              class CSurface* aSurface = NULL);

   /** 
    * Used to tell us is the background of the control is visible or not.
    */
   //TBool IsBackgroundVisible();

   /**
    * Show or hide the background.
    */
   //void MakeBackgroundVisible( TBool aShow );

   /**
    * Show or hide the gps icon.
    */
   void MakeGpsVisible( TBool aShow );

   /**
    * Change the colors to night colors or back to normal.
    */
   void SetNightModeL( TBool aOn );

   /** 
    * Used to hide all controls within this control.
    */
   void HideControlPictures();

   /** 
    * Sets the GPS status image.
    *
    * @param aGpsStatusBitmap    The GPS status image to set.
    * @param aGpsStatusMask      The GPS status mask to set.
    */
   void SetGpsStatusImage(CFbsBitmap* aGpsStatusBitmap,
                          CFbsBitmap* aGpsStatusMask,
                          TBool aInvertMask = ETrue);

   /**
    * Sets the next turn image.
    *
    * @param aMbmIndex     The Mbm image id of the next turn.
    */
   void SetTurnPictureL(TInt aMbmIndex);

   /**
    * Changes the distance to the next turn.
    *
    * @param aDistance   The distance to the next turn.
    * @param aMode       The distance mode, e.g. meters, feet.
    */
   void SetDistanceL(TInt aDistance, TInt aMode);
   TRect CalculateSizeAndPos(const TRect& aRect);

public:
   /**
    * Inherited from CoeControl.
    */
   TInt CountComponentControls() const;

   /**
    * Inherited from CoeControl to make sure we hide and show
    * the controls within this control as well.
    */
   virtual void MakeVisible(TBool aVisible);

   void HandleResourceChange(TInt aType);

protected:
   /**
    * Inherited from CCoeControl.
    */
   CCoeControl* ComponentControl(TInt aIndex) const;

   /**
    * Inherited from CCoeControl.
    */
   virtual void SizeChanged();


private:
   /**
    * Inherited from CCoeControl to control what we redraw.
    */
   virtual void Draw(const TRect& aRect) const;

   void SetTurnImage(CFbsBitmap* aTurnBitmap,
                     CFbsBitmap* aTurnMask);
   
   /// If to show or hide distance.
   void ShowDistance( bool show );
private:
   HBufC* iMbmName;
   const TInt* iMbmImageIds;
   const TReal* iMbmImageRelations;

   class CBitmapControl* iBorderBitmap;
   class CBitmapControl* iTurnBitmap;
   class CDistanceBitmapHelper* iDistanceHelper;
   class CDistanceBitmap* iDistanceBitmap;
   class CBitmapControl* iGpsBitmap;
   class CSurface* iBlitSurface;

   // Removed since its not needed for uiq3 (only for uiq2 which we dont have anymore).
   //class CBackBufBlitter* iBackBufBlitter;

   TInt iCurrentTurnMbmIndex;
   TBool iVisible;

   TBool iNightMode;
};

#endif
