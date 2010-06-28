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

// INCLUDE FILES
#include <e32base.h>
#include <fbs.h>
#include <coecntrl.h>
#ifdef NAV2_CLIENT_SERIES60_V3
#include <akniconutils.h>
#include <aknutils.h>
#endif
#include "BitmapControl.h"
#include "DistanceBitmap.h"
#include "AnimatorFrame.h"
#include "WFTextUtil.h"
#include "WFBitmapUtil.h"
#include "nav2util.h"
#include "Surface.h"


#define DISTANCE_IMAGE_HEIGHT    30
#define NUMBER_BORDER_SIZE       TSize(5, 30)

using namespace isab;

enum {   
   EDistanceBitmap0,
   EDistanceBitmap1,
   EDistanceBitmap2,
   EDistanceBitmap3,
   EDistanceBitmap4,
   EDistanceBitmap5,
   EDistanceBitmap6,
   ELeftBorder,
   ERightBorder,
   ENumControls
};

CDistanceBitmapHelper::CDistanceBitmapHelper():
iNightMode(EFalse)
{
}

void
CDistanceBitmapHelper::ConstructL(const TDesC& aMbmName,
                                  const TInt* aMbmImageIds,
                                  const TReal* aMbmImageRelations)
{
   /* Copy MBM file name. */
   iMbmName = aMbmName.AllocL();
   iMbmImageIds = aMbmImageIds;
   iMbmImageRelations = aMbmImageRelations;
}

class CDistanceBitmapHelper* 
CDistanceBitmapHelper::NewLC(const TDesC& aMbmName, 
                             const TInt* aMbmImageIds, 
                             const TReal* aMbmImageRelations)
{
   class CDistanceBitmapHelper* self = new (ELeave) CDistanceBitmapHelper();
   CleanupStack::PushL(self);
   self->ConstructL(aMbmName, aMbmImageIds, aMbmImageRelations);
   return self;
}

class CDistanceBitmapHelper* 
CDistanceBitmapHelper::NewL(const TDesC& aMbmName, 
                            const TInt* aMbmImageIds, 
                            const TReal* aMbmImageRelations)
{
   class CDistanceBitmapHelper* self =
      CDistanceBitmapHelper::NewLC(aMbmName, aMbmImageIds, aMbmImageRelations);
   CleanupStack::Pop(self);
   return self;
}

// Destructor
CDistanceBitmapHelper::~CDistanceBitmapHelper()
{
   User::Free(iMbmName);
   if(iBitmapCache){
      for (TInt n = 0; n < ImageIdNumber_night_0; n++) {
         User::Free(iBitmapCache[n]);
      }
      User::Free(iBitmapCache);
   }
}

TPtrC CDistanceBitmapHelper::GetMbmName() const
{
   return TPtrC(*iMbmName);
}

TInt CDistanceBitmapHelper::GetMbmBitmapId( TInt idx ) const
{
   return iMbmImageIds[idx];
}

CAnimatorFrame*
CDistanceBitmapHelper::GetBitmapL(TInt id)
{

   if (!iBitmapCache) {
      iBitmapCache = new (ELeave) CAnimatorFrame* [ImageIdNumber_night_0];
      /* Make sure all data is set to zero. */
      for (TInt n = 0; n < ImageIdNumber_night_0; n++) {
         iBitmapCache[n] = NULL;
      }
   }

   if (!iBitmapCache[id]) {
      /* Create it. */
      CAnimatorFrame *frame = new (ELeave) CAnimatorFrame();
      CleanupStack::PushL(frame);
      TInt imageID;
      if( iNightMode ){
         imageID = iMbmImageIds[id+ImageIdNumber_night_0];
      }
      else{
         imageID = iMbmImageIds[id];
      }
#ifdef NAV2_CLIENT_SERIES60_V3
         TRect rect;
         // Get the size of the main pane
         AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
         // Calculate the size of the image, use the relation between height
         // and width on pre v3 images.
         if (iMbmImageRelations) {
            // iMbmImageRelations is not null we can 
            // safely calculate the size of the image. 
            TInt height = ((TInt)((float)rect.Height() * 
                                  ((float)DISTANCE_IMAGE_HEIGHT / 208)));
            TInt width = ((TInt)((float)height / (float)iMbmImageRelations[id]));
            frame->LoadBitmapL(*iMbmName, imageID, 
                               TSize(width, height), EAspectRatioNotPreserved);
         } else {
            // No way to calculate the size of the image, set the size to 0!
            frame->LoadBitmapL(*iMbmName, imageID, TSize(0, 0));
         }
#else
      frame->LoadBitmapL(*iMbmName, imageID);
#endif
      iBitmapCache[id] = frame;
      CleanupStack::Pop(frame);
   }
   return iBitmapCache[id];
}

void
CDistanceBitmapHelper::UpdateBitmapSize()
{
#ifdef NAV2_CLIENT_SERIES60_V3
   // Calculate the size of the image, use the relation between height
   // and width on pre v3 images.
   if (iBitmapCache && iMbmImageRelations) {
      TRect rect;
      AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
      for (TInt n = 0; n < ImageIdNumber_night_0; n++) {
         if (!iBitmapCache[n]) {
            continue;
         }
         TInt height = ((TInt)((float)rect.Height() * 
                               ((float)DISTANCE_IMAGE_HEIGHT / 208)));
         TInt width = ((TInt)((float)height / (float)iMbmImageRelations[n]));
         iBitmapCache[n]->SetBitmapSize(TSize(width, height), EAspectRatioNotPreserved); 
      }
   }
#endif
}

CAnimatorFrame *
CDistanceBitmapHelper::GetDistanceBitmapL(const char character)
{
   CAnimatorFrame* tmp = NULL;
   if (character >= '0' && character <= '9') {
      tmp = GetBitmapL(character-'0');
   } else if (character == '.') {
      tmp = GetBitmapL(ImageIdNumber_dot2);
   } else if (character == 't') {
      tmp = GetBitmapL(ImageIdNumber_t);
   } else if (character == 'f') {
      tmp = GetBitmapL(ImageIdNumber_f);
   } else if (character == 'k') {
      tmp = GetBitmapL(ImageIdNumber_k);
   } else if (character == 'm') {
      tmp = GetBitmapL(ImageIdNumber_m2);
   } else if (character == 'i') {
      tmp = GetBitmapL(ImageIdNumber_i);
   } else if (character == 'd') {
      tmp = GetBitmapL(ImageIdNumber_d);
   } else if (character == 'y') {
      tmp = GetBitmapL(ImageIdNumber_y);
   } else if (character == '_') {
      tmp = GetBitmapL(ImageIdNumber_infinite);
   } else {
      /* Do nothing. */
   }
   return tmp;
}


void
CDistanceBitmapHelper::SetNightModeL( TBool aOn )
{
   iNightMode = aOn;
   if(iBitmapCache){
      for (TInt n = 0; n < ImageIdNumber_night_0; n++) {
         User::Free(iBitmapCache[n]);
      }
      User::Free(iBitmapCache);
      iBitmapCache = NULL;
   }
}


CDistanceBitmap::CDistanceBitmap():
iNightMode(EFalse)
{
}

void
CDistanceBitmap::ConstructL( CDistanceBitmapHelper* aFontCache,
                             const TRect& aRect,
                             TInt aNumber_border_right,
                             TInt aNumber_border_right_night,
                             TInt aNumber_border_left,
                             TInt aNumber_border_left_night,
                             TInt aNumber_border_right_m,
                             TInt aNumber_border_left_m,
                             TInt aNumber_border_right_night_m,
                             TInt aNumber_border_left_night_m,
                             class CSurface* aSurface )
{
   SetComponentsToInheritVisibility();
   iHelper = aFontCache;
   iSurface = aSurface;

   iRightBorderId = aNumber_border_right;
   iRightBorderMaskId = aNumber_border_right_m;
   iRightBorderNightId = aNumber_border_right_night;
   iRightBorderNightMaskId = aNumber_border_right_night_m;
   iLeftBorderId = aNumber_border_left;
   iLeftBorderMaskId = aNumber_border_left_m;
   iLeftBorderNightId = aNumber_border_left_night;
   iLeftBorderNightMaskId = aNumber_border_left_night_m;

   iDistanceBitmaps = new (ELeave) CBitmapControl*[MAX_NUM_DISTANCE_IMGS];

   bool ownWindow = false;

   const TPoint noll(0,0);
   for ( int n = 0; n < MAX_NUM_DISTANCE_IMGS; n++) {
      iDistanceBitmaps[n] = CBitmapControl::NewL( this,        // Parent
                                                  noll,        // TopLeft
                                                  NULL,        // bitmap
                                                  NULL,        // mask
                                                  true,        // invertmask
                                                  false,       // ownbitmaps
                                                  ownWindow ); // ownwindow - set to false 
                                                               // to allow transparency.
                                                               // But to true to avoid flicker.
      if ( iSurface ) {
         iDistanceBitmaps[n]->ChangeGc( &iSurface->Gc() );
      }
   }

#ifdef NAV2_CLIENT_SERIES60_V3
   if (iLeftBorderMaskId != 0) {
      AknIconUtils::CreateIconL(iLeftBorderBitmap, 
                                iLeftBorderMask,
                                iHelper->GetMbmName(), 
                                iLeftBorderId,
                                iLeftBorderMaskId);
   } else {
      iLeftBorderBitmap  = AknIconUtils::CreateIconL(iHelper->GetMbmName(),
                                                     iLeftBorderId);
   }
   if (iRightBorderMaskId != 0) {
      AknIconUtils::CreateIconL(iRightBorderBitmap,
                                iRightBorderMask,
                                iHelper->GetMbmName(),
                                iRightBorderId,
                                iRightBorderMaskId);
   } else {
      iRightBorderBitmap = AknIconUtils::CreateIconL(iHelper->GetMbmName(),
                                                     iRightBorderId);
   }
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
   TInt width  = ((TInt)((float)rect.Width() * 
                         ((float)NUMBER_BORDER_SIZE.iWidth / 176))); 
   TInt height = ((TInt)((float)rect.Height() * 
                         ((float)NUMBER_BORDER_SIZE.iHeight / 208))); 
   AknIconUtils::SetSize(iLeftBorderBitmap,
                         TSize(width, height), EAspectRatioNotPreserved);
   AknIconUtils::SetSize(iRightBorderBitmap,
                         TSize(width, height), EAspectRatioNotPreserved);
#else
   iRightBorderBitmap = new (ELeave) CFbsBitmap();
   CleanupStack::PushL(iRightBorderBitmap);
   iRightBorderBitmap->Load(iHelper->GetMbmName(), 
                            iRightBorderId);
   CleanupStack::Pop(iRightBorderBitmap);

   iLeftBorderBitmap = new (ELeave) CFbsBitmap();
   CleanupStack::PushL(iLeftBorderBitmap);
   iLeftBorderBitmap->Load(iHelper->GetMbmName(), 
                           iLeftBorderId);
   CleanupStack::Pop(iLeftBorderBitmap);
   
   if( iRightBorderMaskId != 0 ){
      iRightBorderMask = new (ELeave) CFbsBitmap();
      CleanupStack::PushL(iRightBorderMask);
      iRightBorderMask->Load(iHelper->GetMbmName(), 
                             iRightBorderMaskId);
      CleanupStack::Pop(iRightBorderMask);
   }
   if( iLeftBorderMaskId != 0 ){
      iLeftBorderMask = new (ELeave) CFbsBitmap();
      CleanupStack::PushL(iLeftBorderMask);
      iLeftBorderMask->Load(iHelper->GetMbmName(), 
                            iLeftBorderMaskId);
      CleanupStack::Pop(iLeftBorderMask);
   }
#endif
   SetRect(aRect);
/*    SetCanDrawOutsideRect(); */
   ActivateL();
}

class CDistanceBitmap* 
CDistanceBitmap::NewLC( class CCoeControl& aParent, 
                        class CDistanceBitmapHelper* aFontCache, 
                        const TRect& aRect,
                        TInt aNumber_border_right,
                        TInt aNumber_border_right_night,
                        TInt aNumber_border_left,
                        TInt aNumber_border_left_night,
                        TInt aNumber_border_right_m,
                        TInt aNumber_border_left_m,
                        TInt aNumber_border_right_night_m,
                        TInt aNumber_border_left_night_m,
                        class CSurface* aSurface )
{
   class CDistanceBitmap* self = new (ELeave) CDistanceBitmap();
   CleanupStack::PushL(self);
   self->SetContainerWindowL(aParent);
   self->ConstructL( aFontCache, 
                     aRect,
                     aNumber_border_right,
                     aNumber_border_right_night,
                     aNumber_border_left,
                     aNumber_border_left_night,
                     aNumber_border_right_m,
                     aNumber_border_left_m,
                     aNumber_border_right_night_m,
                     aNumber_border_left_night_m,
                     aSurface );
   return self;
}

class CDistanceBitmap* 
CDistanceBitmap::NewL( class CCoeControl& aParent, 
                       class CDistanceBitmapHelper* aFontCache, 
                       const TRect& aRect,
                       TInt aNumber_border_right,
                       TInt aNumber_border_right_night,
                       TInt aNumber_border_left,
                       TInt aNumber_border_left_night,
                       TInt aNumber_border_right_m,
                       TInt aNumber_border_left_m,
                       TInt aNumber_border_right_night_m,
                       TInt aNumber_border_left_night_m,
                       class CSurface* aSurface )
{
   class CDistanceBitmap* self =
      CDistanceBitmap::NewLC( aParent, 
                              aFontCache, 
                              aRect,
                              aNumber_border_right,
                              aNumber_border_right_night,
                              aNumber_border_left,
                              aNumber_border_left_night,
                              aNumber_border_right_m,
                              aNumber_border_left_m,
                              aNumber_border_right_night_m,
                              aNumber_border_left_night_m,
                              aSurface );
   CleanupStack::Pop(self);
   return self;
}

// Destructor
CDistanceBitmap::~CDistanceBitmap()
{        
   if (iDistanceBitmaps) {
      for (TInt n = 0; n < MAX_NUM_DISTANCE_IMGS; n++) {
         delete iDistanceBitmaps[n];
      }           
   }
   delete(iDistanceBitmaps);
   iDistanceBitmaps = NULL;

   delete iRightBorderBitmap;
   delete iLeftBorderBitmap;
   delete iRightBorderMask;
   delete iLeftBorderMask;
}

void CDistanceBitmap::SetDistanceL(const TDesC& aDistance)
{
   char* tmp = WFTextUtil::newTDesDupL(aDistance);
   SetDistanceL(tmp);
}


void
CDistanceBitmap::SetDistanceL(char *distanceString)
{
   if (!distanceString) {
      distanceString = strdup_new("");
   }
   if (strlen(distanceString) > MAX_NUM_DISTANCE_IMGS-2 ) {
      /* Too large to show! */
      delete distanceString;
      distanceString = strdup_new("_");
   }

   if ( ( iLastDistanceSet != NULL ) && 
        ( strcmp( distanceString, iLastDistanceSet ) == 0 ) ) {
      // The rounded distance was the same as before.
      delete[] distanceString;
      return;
   }
   delete[] iLastDistanceSet;
   iLastDistanceSet = distanceString;

   RecalculateBitmaps(distanceString);
}


void
CDistanceBitmap::RecalculateBitmaps(char *distanceString)
{
   TPoint lastPos; //Last position of inserted bitmap.
   const TInt maxLength = MAX_NUM_DISTANCE_IMGS - 1; //Dist array starts at index 0.
   const TInt dsLength = strlen(distanceString) - 1; //Char array starts at index 0.
   //i is used as an index to the dist position array.
   TInt i = dsLength;
   //dSi is used to access a specific char in the distanceString char array.
   TInt dSi = dsLength;

   //Check so we have space in image array for all numbers + 2 rounded borders.
   if ((i > (maxLength - 2)) || (i < 0)) {
      i = 0;
      dSi = 0;
   }

   i = i + 2; //Add space for the two border bitmaps.
   //Start populating image array from the end.
   TInt currentIndex = maxLength - i;
   lastPos = TPoint(Rect().iBr.iX, Rect().iTl.iY); //Set initial position
   //Insert the right border image.
   iDistancePos[currentIndex].iX = 
      lastPos.iX - iRightBorderBitmap->SizeInPixels().iWidth;
   iDistancePos[currentIndex].iY = lastPos.iY;
   SetDistanceBitmapL(currentIndex,
                      iRightBorderBitmap,
                      iRightBorderMask,
                      iDistancePos[currentIndex]);
   i--;

   while ((i > 0) && (dSi >= 0)) {
      currentIndex = maxLength - i;
      /* For each character, set the correct bitmap. */
      /* Note: The CAnimatorFrame is still owned by CDistanceBitmap object. */

      CAnimatorFrame *frame =
         iHelper->GetDistanceBitmapL(distanceString[dSi]);

      lastPos = iDistancePos[currentIndex - 1];

      if (frame) {

         //Calculate right edge of bitmap by getting position
         //of last bitmap.
         iDistancePos[currentIndex].iX = lastPos.iX - frame->GetSize().iWidth;
         iDistancePos[currentIndex].iY = lastPos.iY;

         /* Now we have the image and the position, */
         /* set it to the BitmapHandler. */
         SetDistanceBitmapL(currentIndex,
               frame->GetBitmap(),
               frame->GetMask(),
               iDistancePos[currentIndex]);
      } else {
         iDistancePos[currentIndex].iX = lastPos.iX; //No change, just copy.
         iDistancePos[currentIndex].iY = lastPos.iY; //No change, just copy.

         SetDistanceBitmapL(currentIndex,
               NULL,
               NULL,
               iDistancePos[currentIndex]);
      }

      //Step to next character and next position.
      i--;
      dSi--;
   }

   currentIndex = maxLength - i;
   lastPos = iDistancePos[currentIndex - 1];
   //Insert the left border image.
   iDistancePos[currentIndex].iX = 
      lastPos.iX - iLeftBorderBitmap->SizeInPixels().iWidth;
   iDistancePos[currentIndex].iY = lastPos.iY;
   SetDistanceBitmapL(currentIndex,
                      iLeftBorderBitmap,
                      iLeftBorderMask,
                      iDistancePos[currentIndex]);

   // Update the distance rect.
   iDistanceRect.iTl.iX = iDistancePos[currentIndex].iX;
   iDistanceRect.iTl.iY = iDistancePos[currentIndex].iY;
   iDistanceRect.iBr.iX = Rect().iBr.iX;
   iDistanceRect.iBr.iY = Rect().iBr.iY;

   // Now, check if we need to erase images that we're not using.
   i = maxLength - strlen(distanceString) - 2;
   while (i >= 0) {
      /* Set it to blank. */
      SetDistanceBitmapL(i,
            NULL,
            NULL,
            iDistancePos[i]);
      i--;
   }
}


void  
CDistanceBitmap::SetDistanceBitmapL(TInt index,
                                    CFbsBitmap* bitmap,
                                    CFbsBitmap* mask,
                                    TPoint pos)
{
   if (iDistanceBitmaps && iDistanceBitmaps[index]) {
#if defined NAV2_CLIENT_SERIES60_V3
      TBool invertMask = ETrue;
#else
      TBool invertMask = EFalse;
#endif
      iDistanceBitmaps[index]->SetBitmap(pos, bitmap, mask, 
                                         invertMask, // Invert masks.
                                         false ); //Don't own the bitmaps.
   } 
}

void
CDistanceBitmap::SetNightModeL( TBool aOn )
{
   if( iNightMode != aOn ){
      iNightMode = aOn;
      iHelper->SetNightModeL( aOn );

      TInt rightId;
      TInt leftId;
      TInt rightMaskId;
      TInt leftMaskId;
      if( aOn ){
         rightId = iRightBorderNightId;
         leftId = iLeftBorderNightId;
         rightMaskId = iRightBorderNightMaskId;
         leftMaskId = iLeftBorderNightMaskId;
      }
      else{
         rightId = iRightBorderId;
         leftId = iLeftBorderId;
         rightMaskId = iRightBorderMaskId;
         leftMaskId = iLeftBorderMaskId;
      }
#ifdef NAV2_CLIENT_SERIES60_V3
      TSize iconSize = iRightBorderBitmap->SizeInPixels();

      AknIconUtils::CreateIconL(iRightBorderBitmap,
                                iRightBorderMask,
                                iHelper->GetMbmName(),
                                rightId,
                                iRightBorderMaskId);

      AknIconUtils::SetSize(iRightBorderBitmap, iconSize, EAspectRatioNotPreserved);

      AknIconUtils::CreateIconL(iLeftBorderBitmap,
                                iLeftBorderMask,
                                iHelper->GetMbmName(),
                                leftId,
                                iLeftBorderMaskId);

      AknIconUtils::SetSize(iLeftBorderBitmap, iconSize, EAspectRatioNotPreserved);

#else
      
# if 0
      CFbsBitmap* tmp = iRightBorderBitmap;
      CFbsBitmap* tmp2 = new (ELeave) CFbsBitmap();
      tmp2->Load( iHelper->GetMbmName(), 
                  rightId );
      iRightBorderBitmap = tmp2;
      delete tmp;

      tmp = iLeftBorderBitmap;
      tmp2 = new (ELeave) CFbsBitmap();
      tmp2->Load( iHelper->GetMbmName(), 
                  leftId );
      iLeftBorderBitmap = tmp2;
      delete tmp;
# else
      iRightBorderBitmap->Load( iHelper->GetMbmName(),
                                rightId );
      iLeftBorderBitmap->Load( iHelper->GetMbmName(),
                               leftId );

      if( iRightBorderMask && rightMaskId != 0 ){
         iRightBorderMask->Load( iHelper->GetMbmName(), 
                                 rightMaskId );
      }
      if( iLeftBorderMask && leftMaskId != 0 ){
         iLeftBorderMask->Load( iHelper->GetMbmName(), 
                                leftMaskId );
      }


# endif
#endif
   
      if ( iLastDistanceSet ) {
         // Force an update of the distance bitmaps.
         RecalculateBitmaps( iLastDistanceSet ); 
      }
   }
}

void
CDistanceBitmap::SizeChanged()
{
   if (iLastDistanceSet) {
      RecalculateBitmaps(iLastDistanceSet);
   }
}

TInt
CDistanceBitmap::CountComponentControls() const
{     
   return ENumControls;
}        
      
CCoeControl*
CDistanceBitmap::ComponentControl(TInt aIndex) const
{
   TInt a = 0;
   switch (aIndex) {
      case ELeftBorder:
      case ERightBorder:
      case EDistanceBitmap0:
      case EDistanceBitmap1:
      case EDistanceBitmap2:
      case EDistanceBitmap3:
      case EDistanceBitmap4:
      case EDistanceBitmap5:
      case EDistanceBitmap6:
         a = aIndex - EDistanceBitmap0;
         if (iDistanceBitmaps && iDistanceBitmaps[a]) {
            return iDistanceBitmaps[a];
         }         
         /** FALLTHTOUGH **/
      default: break;
   }
   return NULL;
}

void
CDistanceBitmap::Draw(const TRect& aRect) const
{
}

void 
CDistanceBitmap::UpdatePosAndSize(const TRect& aRect)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   if (iLeftBorderBitmap && iRightBorderBitmap) {
      TRect rect;
      AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
      TInt width  = ((TInt)((float)rect.Width() * 
                            ((float)NUMBER_BORDER_SIZE.iWidth / 176))); 
      TInt height = ((TInt)((float)rect.Height() * 
                            ((float)NUMBER_BORDER_SIZE.iHeight / 208))); 
      AknIconUtils::SetSize(iLeftBorderBitmap,
                            TSize(width, height), EAspectRatioNotPreserved);
      AknIconUtils::SetSize(iRightBorderBitmap,
                            TSize(width, height), EAspectRatioNotPreserved);
   }
   iHelper->UpdateBitmapSize();
#endif
   SetRect(aRect);
}

