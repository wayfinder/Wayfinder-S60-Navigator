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

#ifndef DISTANCE_BITMAP_H
#define DISTANCE_BITMAP_H

class CAnimatorFrame;

#define MAX_NUM_DISTANCE_IMGS 9

class CDistanceBitmapHelper : public CBase
{
private:
   void ConstructL(const TDesC& aMbmName, 
                   const TInt* aMbmImageIds,
                   const TReal* aMbmImageRelations);
   CDistanceBitmapHelper();
public:
   static class CDistanceBitmapHelper* NewL(const TDesC& aMbmName, 
                                            const TInt* aMbmImageIds, 
                                            const TReal* aMbmImageRelations = NULL);
   static class CDistanceBitmapHelper* NewLC(const TDesC& aMbmName, 
                                             const TInt* aMbmImageIds,
                                             const TReal* aMbmImageRelations = NULL);
   ~CDistanceBitmapHelper();

   TPtrC GetMbmName() const;
   TInt GetMbmBitmapId( TInt idx ) const;
   CAnimatorFrame *GetBitmapL(TInt id);
   CAnimatorFrame *GetDistanceBitmapL(const char character);
   void UpdateBitmapSize();
   void SetNightModeL( TBool aOn );


   enum ImageIds {
      ImageIdNumber_0 = 0,
      ImageIdNumber_1,
      ImageIdNumber_2,
      ImageIdNumber_3,
      ImageIdNumber_4,
      ImageIdNumber_5,
      ImageIdNumber_6,
      ImageIdNumber_7,
      ImageIdNumber_8,
      ImageIdNumber_9,
      ImageIdNumber_d,
      ImageIdNumber_dot2,
      ImageIdNumber_f,
      ImageIdNumber_i,
      ImageIdNumber_k,
      ImageIdNumber_m2,
      ImageIdNumber_t,
      ImageIdNumber_y,
      ImageIdNumber_infinite,
      ImageIdNumber_night_0,
      ImageIdNumber_night_1,
      ImageIdNumber_night_2,
      ImageIdNumber_night_3,
      ImageIdNumber_night_4,
      ImageIdNumber_night_5,
      ImageIdNumber_night_6,
      ImageIdNumber_night_7,
      ImageIdNumber_night_8,
      ImageIdNumber_night_9,
      ImageIdNumber_night_d,
      ImageIdNumber_night_dot2,
      ImageIdNumber_night_f,
      ImageIdNumber_night_i,
      ImageIdNumber_night_k,
      ImageIdNumber_night_m2,
      ImageIdNumber_night_t,
      ImageIdNumber_night_y,
      ImageIdNumber_night_infinite,
      ImageIdNumber_border_right,
      ImageIdNumber_border_left,
      ImageIdNumber_night_border_right,
      ImageIdNumber_night_border_left,
      ImageIdNumberBorder_right_m,
      ImageIdNumberBorder_left_m,
      ImageIdMap_top_border,
      ImageIdMedium_mask
   };

private:
   const TInt* iMbmImageIds;
   const TReal* iMbmImageRelations;
   CAnimatorFrame **iBitmapCache;
   HBufC* iMbmName;
   TBool iNightMode;
};


class CDistanceBitmap : public CCoeControl
{
private:
   void ConstructL( CDistanceBitmapHelper* aFontCache, 
                    const TRect& aRect, 
                    TInt aNumber_border_right,
                    TInt aNumber_border_right_night,
                    TInt aNumber_border_left,
                    TInt aNumber_border_left_night,
                    TInt aNumber_border_right_m,
                    TInt aNumber_border_left_m,
                    TInt aNumber_border_right_night_m = 0,
                    TInt aNumber_border_left_night_m = 0,
                    class CSurface* aSurface = NULL );
   CDistanceBitmap();
public:
   static class CDistanceBitmap* NewL( class CCoeControl& aParent,
                                       class CDistanceBitmapHelper* aFontCache,
                                       const TRect& aRect,
                                       TInt aNumber_border_right,
                                       TInt aNumber_border_right_night,
                                       TInt aNumber_border_left,
                                       TInt aNumber_border_left_night,
                                       TInt aNumber_border_right_m,
                                       TInt aNumber_border_left_m,
                                       TInt aNumber_border_right_night_m = 0,
                                       TInt aNumber_border_left_night_m = 0,
                                       class CSurface* aSurface = NULL );

   static class CDistanceBitmap* NewLC( class CCoeControl& aParent,
                                        class CDistanceBitmapHelper* aFontCache,
                                        const TRect& aRect,
                                        TInt aNumber_border_right,
                                        TInt aNumber_border_right_night,
                                        TInt aNumber_border_left,
                                        TInt aNumber_border_left_night,
                                        TInt aNumber_border_right_m,
                                        TInt aNumber_border_left_m,
                                        TInt aNumber_border_right_night_m = 0,
                                        TInt aNumber_border_left_night_m = 0,
                                        class CSurface* aSurface = NULL );
   ~CDistanceBitmap();


   /**
    * Will take ownership of distanceString.
    */
   void SetDistanceL(char *distanceString);
   void SetDistanceL(const TDesC& aDistance);
   void SetDistanceBitmapL(TInt index,
      CFbsBitmap* bitmap,
      CFbsBitmap* mask,
      TPoint pos);
   void SetNightModeL( TBool aOn );
   void RecalculateBitmaps(char *distanceString);
   void UpdatePosAndSize(const TRect& aRect);

public:
   /* From base class */

   /**
    * From CCoeControl,ComponentControl.
    */
   virtual void SizeChanged();

   /**
    * From CoeControl,CountComponentControls.
    */
   TInt CountComponentControls() const;

   /**
    * From CCoeControl,ComponentControl.
    */
   CCoeControl* ComponentControl(TInt aIndex) const;

   /**
    * From CCoeControl,Draw.
    */
   void Draw(const TRect& aRect) const;

   TRect getDistanceRect() const { return iDistanceRect; }

private:
   class CBitmapControl** iDistanceBitmaps;
   TPoint iDistancePos[MAX_NUM_DISTANCE_IMGS];
   class CDistanceBitmapHelper* iHelper;
   char* iLastDistanceSet;

   class CFbsBitmap* iRightBorderBitmap;
   class CFbsBitmap* iRightBorderBitmapBu;
   class CFbsBitmap* iLeftBorderBitmap;
   class CFbsBitmap* iLeftBorderBitmapBu;
   class CFbsBitmap* iRightBorderMask;
   class CFbsBitmap* iLeftBorderMask;

   class CSurface* iSurface;
   TInt iRightBorderId;
   TInt iRightBorderNightId;
   TInt iRightBorderMaskId;
   TInt iRightBorderNightMaskId;
   TInt iLeftBorderId;
   TInt iLeftBorderNightId;
   TInt iLeftBorderMaskId;
   TInt iLeftBorderNightMaskId;

   TBool iNightMode;

   TRect iDistanceRect;
};

#endif /* DISTANCE_BITMAP_H */
