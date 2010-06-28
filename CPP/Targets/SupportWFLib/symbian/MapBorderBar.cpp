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

#include "MapBorderBar.h"
#include <stddef.h>
#ifdef NAV2_CLIENT_SERIES60_V3
#include <akniconutils.h> 
#include <aknutils.h>
#endif
#include "BitmapControl.h"
#include "DistanceBitmap.h"
#include "DistancePrintingPolicy.h"
#include "WFLayoutUtils.h"
#include "WFBitmapUtil.h"
#include "Surface.h"
#include "BackBufBlitter.h"

#define IMAGE_HPADDING_SIZE    2
#define IMAGE_VPADDING_SIZE    3
#define GPSIMAGE_WIDTH         32
#define DISTIMAGE_WIDTH        104
#define DISTIMAGE_HEIGHT       30
#define BLUE_BAR_HEIGHT        36

#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V1 
# define TURNIMAGE_MASK_SIZE    5
# define TURNIMAGE_SIZE         42
# define TURN_IMAGE_HEIGHT      36
#else
# define TURNIMAGE_MASK_SIZE    0
# define TURNIMAGE_SIZE         32
#endif

enum {
   EBorderBitmap = 0,
   ETurnBitmap,
   EGpsBitmap,
   EDistanceBitmap,
   //EBackBufBlitter, // Removed since its not needed for uiq3 (only for uiq2).
   ENumControls
};

CMapBorderBar::CMapBorderBar() : 
   iCurrentTurnMbmIndex(-1),
   iVisible(EFalse),
   iNightMode(EFalse)
{
}

void CMapBorderBar::ConstructL(CCoeControl& aParent,
                               const TRect& aRect,
                               const TDesC& aMbmFile,
                               const TInt* aMbmImageIds,
                               const TReal* aMbmImageRelations,
                               class CSurface* aSurface)
{
   SetContainerWindowL(aParent);

   iMbmName = aMbmFile.AllocL();
   iMbmImageIds = aMbmImageIds;
   iMbmImageRelations = aMbmImageRelations;
   iBlitSurface = aSurface;
   InitBitmaps(aRect);
   SetRect(iBorderBitmap->Rect());
   iBorderBitmap->MakeVisible(EFalse); //should never be visible now.
   ActivateL();
}

void CMapBorderBar::InitBitmaps(const TRect& aRect)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   // Mask id for the left and right number border.
   CFbsBitmap* borderBitmap = AknIconUtils::CreateIconL(*iMbmName, 
                                                        iMbmImageIds[CDistanceBitmapHelper::ImageIdMap_top_border]);
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
   TSize size(rect.Width(), ((TInt)((float)rect.Height() * 
                                    ((float)BLUE_BAR_HEIGHT / 208))));
   AknIconUtils::SetSize(borderBitmap, size, EAspectRatioNotPreserved);

   TRect tmpRect = CalculateSizeAndPos(rect);
   rect.SetRect(tmpRect.iTl, tmpRect.Size());
#else
   CFbsBitmap *borderBitmap = new (ELeave) CFbsBitmap();
   CleanupStack::PushL(borderBitmap);
   borderBitmap->Load(*iMbmName, iMbmImageIds[CDistanceBitmapHelper::ImageIdMap_top_border]);
   CleanupStack::Pop(borderBitmap);

   TInt distPos = aRect.Width() - DISTIMAGE_WIDTH - 
      (TURNIMAGE_SIZE - TURNIMAGE_MASK_SIZE * 2) - IMAGE_HPADDING_SIZE * 2;
   TRect rect(TPoint(distPos, IMAGE_VPADDING_SIZE),
                TSize(DISTIMAGE_WIDTH, DISTIMAGE_HEIGHT));
#endif
   iDistanceHelper = 
      CDistanceBitmapHelper::NewL(*iMbmName, iMbmImageIds, iMbmImageRelations);
   iBorderBitmap = CBitmapControl::NewL(this, TPoint(0,0), NULL);
   iBorderBitmap->SetBitmap(TPoint(0,0), borderBitmap);
   iBorderBitmap->MakeVisible(EFalse);

   iTurnBitmap = CBitmapControl::NewL(this, TPoint(0,0), NULL);
   
   iDistanceBitmap = 
      CDistanceBitmap::NewL(*this,
                            iDistanceHelper,
                            rect,
                            iMbmImageIds[CDistanceBitmapHelper::ImageIdNumber_border_right],
                            iMbmImageIds[CDistanceBitmapHelper::ImageIdNumber_night_border_right],
                            iMbmImageIds[CDistanceBitmapHelper::ImageIdNumber_border_left],
                            iMbmImageIds[CDistanceBitmapHelper::ImageIdNumber_night_border_left],
#ifdef NAV2_CLIENT_SERIES60_V3 
                            iMbmImageIds[CDistanceBitmapHelper::ImageIdNumberBorder_right_m],
                            iMbmImageIds[CDistanceBitmapHelper::ImageIdNumberBorder_left_m] );
#elif defined NAV2_CLIENT_UIQ3
                            iMbmImageIds[CDistanceBitmapHelper::ImageIdNumberBorder_right_m],
                            iMbmImageIds[CDistanceBitmapHelper::ImageIdNumberBorder_left_m] );
#else
                            0,0, iBlitSurface);
#endif


   iGpsBitmap = CBitmapControl::NewL(this, TPoint(0,0), NULL);

   // Removed since its not needed for uiq3 (only for uiq2).
   //iBackBufBlitter = new CBackBufBlitter( *this, iDistanceBitmap, iBlitSurface );
}

CMapBorderBar::~CMapBorderBar()
{
   delete iMbmName;
   delete iBorderBitmap;
   delete iTurnBitmap;
   delete iDistanceHelper;
   delete iDistanceBitmap;
   delete iGpsBitmap;
   //delete iBackBufBlitter;
}

CMapBorderBar* CMapBorderBar::NewLC(CCoeControl& aParent,
                                    const TRect& aRect,
                                    const TDesC& aMbmFile,
                                    const TInt* aMbmImageIds,
                                    const TReal* aMbmImageRelations,
                                    class CSurface* aSurface)
{
   CMapBorderBar* self = new (ELeave) CMapBorderBar();
   CleanupStack::PushL(self);
   self->ConstructL(aParent,
                    aRect,
                    aMbmFile,
                    aMbmImageIds,
                    aMbmImageRelations,
                    aSurface);
   return self;
}

CMapBorderBar* CMapBorderBar::NewL(CCoeControl& aParent,
                                   const TRect& aRect,
                                   const TDesC& aMbmFile,
                                   const TInt* aMbmImageIds,
                                   const TReal* aMbmImageRelations,
                                   class CSurface* aSurface)
{
   CMapBorderBar* self = CMapBorderBar::NewLC(aParent,
                                              aRect,
                                              aMbmFile,
                                              aMbmImageIds,
                                              aMbmImageRelations,
                                              aSurface);
   CleanupStack::Pop(self);
   return self;
}

void CMapBorderBar::MakeGpsVisible( TBool aShow )
{
   iGpsBitmap->MakeVisible( aShow );
}

void CMapBorderBar::MakeVisible(TBool aVisible)
{
   iVisible = aVisible;
   iGpsBitmap->MakeVisible(aVisible);
   if (iCurrentTurnMbmIndex != -1) {
      SetTurnPictureL(iCurrentTurnMbmIndex);
   }
   if (!aVisible) {
      HideControlPictures();
   }
}

void CMapBorderBar::SetNightModeL( TBool aOn )
{
   iNightMode = aOn;
   if( iTurnBitmap && iTurnBitmap->IsVisible() ){
      iTurnBitmap->SetNightModeL(aOn);
      iTurnBitmap->DrawDeferred();
      //iBackBufBlitter->DrawDeferred();
   }
   if( iDistanceBitmap && iDistanceBitmap->IsVisible() ){
      iDistanceBitmap->SetNightModeL( aOn );
      iDistanceBitmap->DrawDeferred();
      //iBackBufBlitter->DrawDeferred();
   }
}

void CMapBorderBar::SetGpsStatusImage(CFbsBitmap* aGpsStatusBitmap,
                                      CFbsBitmap* aGpsStatusMask,
                                      TBool aInvertMask)
{
   TPoint gpsStatusPosition(Rect().iTl);
   gpsStatusPosition.iX += IMAGE_HPADDING_SIZE;
   gpsStatusPosition.iY += IMAGE_HPADDING_SIZE;
   if (iGpsBitmap) {
#ifdef NAV2_CLIENT_UIQ3
      iGpsBitmap->SetBitmap(gpsStatusPosition,
                            aGpsStatusBitmap,
                            aGpsStatusMask,
                            EFalse);
#else
      iGpsBitmap->SetBitmap(gpsStatusPosition,
                            aGpsStatusBitmap,
                            aGpsStatusMask,
                            aInvertMask);
#endif
   }
}

void CMapBorderBar::SetTurnPictureL(TInt aMbmIndex)
{
   iCurrentTurnMbmIndex = aMbmIndex;
   if (aMbmIndex != -1) { 
#ifdef NAV2_CLIENT_SERIES60_V3
      CFbsBitmap* bitmap;
      CFbsBitmap* mask;
      AknIconUtils::CreateIconL(bitmap, mask, *iMbmName,
                                aMbmIndex, iMbmImageIds[CDistanceBitmapHelper::ImageIdMedium_mask]);
      //TInt width = WFLayoutUtils::CalculateXValue(TURNIMAGE_SIZE);
      TInt height = WFLayoutUtils::CalculateYValueUsingFullScreen(TURNIMAGE_SIZE);
      TInt width = height; //its supposed to be a square image
      AknIconUtils::SetSize(bitmap, TSize(width, height), EAspectRatioPreservedAndUnusedSpaceRemoved);
      SetTurnImage(bitmap, mask); //Takes ownership of bitmaps
#else
      CFbsBitmap* bitmap = new (ELeave) CFbsBitmap();
      CleanupStack::PushL(bitmap);
      bitmap->Load(*iMbmName, aMbmIndex);
      CleanupStack::Pop(bitmap);

      CFbsBitmap* mask = new (ELeave) CFbsBitmap();
      CleanupStack::PushL(mask);
      mask->Load(*iMbmName, iMbmImageIds[CDistanceBitmapHelper::ImageIdMedium_mask]);
      CleanupStack::Pop(mask);
      SetTurnImage(bitmap, mask); //Takes ownership of bitmaps
#endif
   } else {
      HideControlPictures();
   }
}

void CMapBorderBar::SetTurnImage(CFbsBitmap* aTurnBitmap,
                                 CFbsBitmap* aTurnMask)
{
   TPoint turnPosition(Rect().iTl);
   TSize imgSize(0,0);
   if (aTurnBitmap) {
      //Right edge - image padding - (size of bitmap - masked part).
      imgSize = aTurnBitmap->SizeInPixels();
   }
#if defined NAV2_CLIENT_UIQ3
   turnPosition.iX = (Rect().iBr.iX - imgSize.iWidth) - 2;// + 2;
#elif defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
   TInt imagePadding = WFLayoutUtils::CalculateXValue(IMAGE_HPADDING_SIZE);
   if (!WFLayoutUtils::LandscapeMode()) {
      imagePadding += imagePadding;
   }
   turnPosition.iX = rect.Width() - imagePadding - imgSize.iWidth;
#else
   turnPosition.iX = 
      Rect().Width() - IMAGE_HPADDING_SIZE - 
      (imgSize.iWidth - TURNIMAGE_MASK_SIZE);
#endif
#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V1
   //Top + nothing since the masked part gives us padding.
   turnPosition.iY += 0;
#elif defined NAV2_CLIENT_SERIES60_V3
   turnPosition.iY += WFLayoutUtils::CalculateYValueUsingFullScreen(IMAGE_HPADDING_SIZE);
#else
   //Top + padding.
   turnPosition.iY += IMAGE_HPADDING_SIZE;
#endif
   if( iTurnBitmap && iTurnBitmap->IsVisible() ) {
      iTurnBitmap->SetBitmap(turnPosition,
                             aTurnBitmap,
                             aTurnMask);
   } else {
      HideControlPictures();
   }
}

void CMapBorderBar::ShowDistance( bool show )
{
   if ( ! show ) {
      iDistanceBitmap->SetDistanceL(NULL);
   }
   if ( iDistanceBitmap->IsVisible() != show ) {
      iDistanceBitmap->MakeVisible( show );
      //iBackBufBlitter->MakeVisible( show );
//      Window().Invalidate( iDistanceBitmap->Rect() );
   }
}

void CMapBorderBar::HideControlPictures()
{
   if (iTurnBitmap) {
      iTurnBitmap->SetBitmap(Rect().iTl, NULL, NULL);
   }
   if (iDistanceBitmap) {
      ShowDistance( false );
   }
//    if (iGpsBitmap) {
//       iGpsBitmap->MakeVisible(EFalse);
//    }
}

void CMapBorderBar::SetDistanceL(TInt aDistance, TInt aMode)
{
   if (iDistanceBitmap) {
      if (aDistance >= 0 && iVisible) {
         ShowDistance( true );
         iDistanceBitmap->SetNightModeL( iNightMode );
         isab::DistancePrintingPolicy::DistanceMode mode =
            isab::DistancePrintingPolicy::DistanceMode(aMode);
         
         char* distanceString = isab::DistancePrintingPolicy::convertDistance(
            aDistance, mode, isab::DistancePrintingPolicy::Round);
         
         iDistanceBitmap->SetDistanceL(distanceString);
      } else {
         HideControlPictures();
      } 
   }
}

TInt CMapBorderBar::CountComponentControls() const
{
   return ENumControls;
}

CCoeControl* CMapBorderBar::ComponentControl(TInt aIndex) const
{
   switch (aIndex)
      {
      case EBorderBitmap:
         return iBorderBitmap;
      case ETurnBitmap:
         return iTurnBitmap;
      case EDistanceBitmap:
         return iDistanceBitmap;
      case EGpsBitmap:
         return iGpsBitmap;
      //case EBackBufBlitter:
      //   return iBackBufBlitter;
      default: 
         break;
      }
   return NULL;
}

void CMapBorderBar::SizeChanged()
{
}

void CMapBorderBar::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   gc.SetClippingRect(aRect);
}

TRect CMapBorderBar::CalculateSizeAndPos(const TRect& aRect)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   TInt distImageWidth = WFLayoutUtils::CalculateXValue(DISTIMAGE_WIDTH);
   //TInt turnImageSize = WFLayoutUtils::CalculateXValue(TURNIMAGE_SIZE);
   //TInt tempImageSize = WFLayoutUtils::CalculateYValueUsingFullScreen(TURNIMAGE_SIZE);
   TInt turnImageSize = WFLayoutUtils::CalculateYValueUsingFullScreen(TURNIMAGE_SIZE);

   TInt imageHPadding = WFLayoutUtils::CalculateXValue(IMAGE_HPADDING_SIZE);
   TInt imageVPadding = WFLayoutUtils::CalculateYValueUsingFullScreen(IMAGE_VPADDING_SIZE);
   if (!WFLayoutUtils::LandscapeMode()) {
      imageHPadding += imageHPadding;
   }
   TInt distPos = aRect.Width() - distImageWidth - turnImageSize - imageHPadding * 2;

   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
   if (iTurnBitmap) {
      //TInt width = WFLayoutUtils::CalculateXValue(TURNIMAGE_SIZE);
      TInt height = WFLayoutUtils::CalculateYValueUsingFullScreen(TURNIMAGE_SIZE);
      TInt width = height; //its supposed to be a square image
      TPoint turnBitmapPos((rect.Width() - imageHPadding - width),
                           WFLayoutUtils::
                           CalculateYValueUsingFullScreen(IMAGE_HPADDING_SIZE));

      iTurnBitmap->SetSizeAndPos(TRect(turnBitmapPos, TSize(width, height)),
                                 EAspectRatioPreservedAndUnusedSpaceRemoved);
   }
   return TRect(TPoint(distPos, imageVPadding),
                TSize(distImageWidth, turnImageSize));
#else
   return TRect(aRect.iTl, aRect.Size());
#endif
}

void CMapBorderBar::HandleResourceChange(TInt aType)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      TRect fullRect;
      AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, fullRect);
      TRect rect = CalculateSizeAndPos(fullRect);
      if (iDistanceBitmap) {
         iDistanceBitmap->UpdatePosAndSize(rect);
      }
      TSize size(fullRect.Width(), ((TInt)((float)fullRect.Height() * 
                                           ((float)BLUE_BAR_HEIGHT / 208))));
      iBorderBitmap->SetSizeAndPos(TRect(TPoint(0, 0), size));
      SetRect(iBorderBitmap->Rect());
   }
#else
   aType = aType;
#endif
}
