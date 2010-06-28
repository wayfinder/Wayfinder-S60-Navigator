/*  Copyright (c) 2004-2006, Nokia. */

// INCLUDE FILES

#include "ImageHandler.h"
#include <imageconversion.h>
#include <bitmaptransforms.h>
#include <eikapp.h> 
#include <eikappui.h> 
#include <eikenv.h>
#include <aknutils.h>
#include "ImageHandlerCallback.h"
#include "WFLayoutUtils.h"
#include "TraceMacros.h"
#include "WFBitmapUtil.h"

// =============================================================================
// CImageHandler, a utility class for loading images.
// =============================================================================

// ============================= MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CImageHandler::CImageHandler
// C++ constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CImageHandler::CImageHandler(/*class RFs& aFs,*/
                             class MImageHandlerCallback* aCallback) :
   /*iFs(aFs),*/
     iCallback(aCallback)
{
}

CImageHandler::CImageHandler() /*: iFs(CEikonEnv::Static()->FsSession())*/
{
}

// -----------------------------------------------------------------------------
// CImageHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CImageHandler::ConstructL(const TRect& aImageRect)
{
   iFs.Connect();
   iImageConverter = CBitmapConverter::NewL(this, iFs);
   iImageRect = aImageRect;
   iShow  = EFalse;
   iClear = EFalse;
   iUseMifStartup = EFalse;
   iScreenDevice = CCoeEnv::Static()->ScreenDevice();
   SetRect(aImageRect);
}

// -----------------------------------------------------------------------------
// CImageHandler::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CImageHandler* CImageHandler::NewL(const TRect& aImageRect,
                                   /*class RFs& aFs,*/
                                   class MImageHandlerCallback* aCallback)
{
   CImageHandler* self = NewLC(aImageRect,
                               /*aFs,*/
                               aCallback);
   CleanupStack::Pop();
   return self;
}

// -----------------------------------------------------------------------------
// CImageHandler::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CImageHandler* CImageHandler::NewLC(const TRect& aBitMapRect, 
                                    /*class RFs& aFs,*/
                                    class MImageHandlerCallback* aCallback)
{
   CImageHandler* self = new (ELeave) CImageHandler(/*aFs,*/
                                                    aCallback);
   CleanupStack::PushL( self );
   self->ConstructL(aBitMapRect);  
   return self;
}

CImageHandler* CImageHandler::NewL(const TRect& aImageRect)
{
   CImageHandler* self = NewLC(aImageRect);
   CleanupStack::Pop();
   return self;
}

CImageHandler* CImageHandler::NewLC(const TRect& aBitMapRect)
{
   CImageHandler* self = new (ELeave) CImageHandler();
   CleanupStack::PushL( self );
   self->ConstructL(aBitMapRect);  
   return self;
}

// Destructor
CImageHandler::~CImageHandler()
{
   delete iImage;
   delete iImageMask;
   delete iImageConverter;
   delete iConvertedImage;
   iFs.Close();
}

void CImageHandler::CreateIconL(const TDesC& aIconFilename,  
                                TInt aImageId,  
                                TInt aImageMaskId,
                                TScaleMode aScaleMode,
                                TBool aCenterRealImageRect,
                                TBool aIsNightModeOn)
{
   if (iImage) {
      iImage->Reset();
      delete iImage;
   }
   if (iImageMask) {
      iImageMask->Reset();
      delete iImageMask;
   }
#if defined NAV2_CLIENT_SERIES60_V3
   AknIconUtils::CreateIconL(iImage, iImageMask, aIconFilename, aImageId, aImageMaskId);
   AknIconUtils::SetSize(iImage, iImageRect.Size(), aScaleMode);
   if (aCenterRealImageRect) {
      CenterImageRect(iImage, iImageRect);
   } 
   iImageRect.SetSize(iImage->SizeInPixels());
   SetRect(TRect(iImageRect.iTl, iImage->SizeInPixels()));
#else
   iImage = new (ELeave) CFbsBitmap();
   iImage->Load(aIconFilename, aImageId);
   iImageMask = new (ELeave) CFbsBitmap();
   iImageMask->Load(aIconFilename, aImageMaskId);
#endif
}

void CImageHandler::CreateIconL(const TDesC& aIconFilename,  
                                TInt aImageId, 
                                TScaleMode aScaleMode,
                                TBool aCenterRealImageRect)
{
   if (iImage) {
      iImage->Reset();
      delete iImage;
   }
   if (iImageMask) {
      iImageMask->Reset();
      delete iImageMask;
   }
#if defined NAV2_CLIENT_SERIES60_V3
   iImage = AknIconUtils::CreateIconL(aIconFilename, aImageId);
   AknIconUtils::SetSize(iImage, iImageRect.Size(), aScaleMode);
   if (aCenterRealImageRect) {
      CenterImageRect(iImage, iImageRect);
   }
   iImageRect.SetSize(iImage->SizeInPixels());
   SetRect(TRect(iImageRect.iTl, iImage->SizeInPixels()));
#else
   iImage = new (ELeave) CFbsBitmap();
   iImage->Load(aIconFilename, aImageId);
#endif
}

void CImageHandler::SetImageRect(const TRect& aRect, 
                                 TScaleMode aScaleMode, 
                                 TBool aCenterRealImageRect)
{
   iImageRect.SetRect(aRect.iTl, aRect.iBr);
#if defined NAV2_CLIENT_SERIES60_V3
   if (iImage) {
      AknIconUtils::SetSize(iImage, iImageRect.Size(), aScaleMode);
      if (aCenterRealImageRect) {
         CenterImageRect(iImage, iImageRect);
      } 
      iImageRect.SetSize(iImage->SizeInPixels());
   }
#else
   // Remove warnings!
   aScaleMode = aScaleMode;
#endif
   SetRect(iImageRect);
}

// -----------------------------------------------------------------------------
// CImageHandler::LoadFileL
// Loads a selected frame from a named file
// -----------------------------------------------------------------------------
//
TInt CImageHandler::LoadFileL(const TDesC& aFileName, 
                              TSize aSize, 
                              TBool aDoScale, 
                              TInt aSelectedFrame)
{
   return iImageConverter->ConvertFileL(aFileName, 
                                        aSize,
                                        iScreenDevice->DisplayMode(),
                                        aDoScale,
                                        aSelectedFrame);
}

#ifdef NAV2_CLIENT_SERIES60_V3
void CImageHandler::LoadMifImage(const TDesC& aFileName, 
                                 TSize aSize)
{
   iUseMifStartup = ETrue;
   if (iConvertedImage) {
      delete iConvertedImage;
      iConvertedImage = NULL;
   }
   TInt bitmapId = 0;
   TInt maskId = 1;
   AknIconUtils::ValidateLogicalAppIconId(aFileName, bitmapId, maskId);
   AknIconUtils::CreateIconL(const_cast<CFbsBitmap *&>(iConvertedImage), 
                             iImageMask, aFileName, bitmapId, maskId);
   AknIconUtils::SetSize(const_cast<CFbsBitmap *>(iConvertedImage), aSize);
   iImageConverter->CompleteRequest(KErrNone);
}
#endif

void CImageHandler::SetShow(TBool aShow)
{
   iShow = aShow;
}

TBool CImageHandler::GetShow()
{
   return iShow;
}

void CImageHandler::SetClear(TBool aClear)
{
   iClear = aClear;
}

void CImageHandler::SizeChanged()
{
   iScreenDevice->GetDefaultScreenSizeAndRotation(iPixTwipsRot);
}

void
CImageHandler::MakeVisible(TBool aVisible)
{
   SetShow(aVisible);
}

void
CImageHandler::Draw(const TRect& /*aRect*/) const
{
   if (iShow) {
      CWindowGc& gc = SystemGc();
      if (iClear) {
         // clear the screen
         gc.Clear(iImageRect);
      }
      if (iImageConverter->GetConvertState() == CBitmapConverter::EConvertStateReady) {
         // We are done with converstion and scaling, it
         // is safe to draw the image.
         // Get the size of the bitmap.
         TSize size = iConvertedImage->SizeInPixels();
         TPoint topLeft = Rect().iTl;
         if (size != iPixTwipsRot.iPixelSize) {
            // The screen size is not equal in size compared to
            // the bitmap. Paint the background just in case.
            // TRgb backgroundColor = TRgb(0, 0, 0);
            // XXX Why paint the backgroundColor!!????! 
            // then we just need to know the bgcolor here as well! XXX
            //TRgb backgroundColor = iAppUi->GetNewsBgColor();
            //gc.SetBrushColor(backgroundColor);
            //gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
            //gc.Clear(iImageRect);
            if (size.iWidth < WFLayoutUtils::GetMainPaneSize().iWidth) {
               // The width of the bitmap is smaller than the
               // width of the screen. 
               topLeft.iX += ((WFLayoutUtils::GetMainPaneSize().iWidth - size.iWidth) / 2);
            }
            if (size.iHeight < WFLayoutUtils::GetMainPaneSize().iHeight) {
               // The height of the bitmap is smaller than the
               // height of the screen. 
               topLeft.iY += ((WFLayoutUtils::GetMainPaneRect().iTl.iY + 
                              WFLayoutUtils::GetMainPaneSize().iHeight - 
                               size.iHeight) / 2);
            }
         }
         gc.SetBrushStyle(CGraphicsContext::ENullBrush);
         if (iImageMask) {
            gc.BitBltMasked(topLeft, iConvertedImage,
                            TRect(iConvertedImage->SizeInPixels()), 
                            iImageMask, EFalse);
         } else { 
            gc.BitBlt(topLeft, iConvertedImage);
         }
      } else if (iImageConverter->GetConvertState() == CBitmapConverter::EIdle) {
         // No conversion to bitmap has been done, just paint the picture.
         if (iImageMask) {
            // If the image comes with a mask.
            gc.BitBltMasked(iImageRect.iTl, iImage, iImageRect.Size(), iImageMask, EFalse);       
         } else {
            // No mask!
            gc.BitBlt(iImageRect.iTl, iImage);
         }
      }
   }
}

void CImageHandler::ImageConversionDone(TInt aResult)
{
   if (!iUseMifStartup) {
      // Get the converted image. If we have used mif
      // startup we only used the imageconverter to set
      // the correct state and to get out of the call stack.
      iConvertedImage = iImageConverter->GetImage();
   }
   if (iCallback) {
      iCallback->ImageOperationCompletedL(aResult);
   }
}

TSize CImageHandler::GetSize()
{
   return iImageRect.Size();
}

TRect CImageHandler::GetRect()
{
   if (iImage) {
      return TRect(iImageRect.iTl, iImage->SizeInPixels());
   } else {
      return iImageRect;
   }
}

TPoint CImageHandler::GetTopLeftPos()
{
   return iImageRect.iTl;
}

void CImageHandler::HandleLayoutSwitch(TInt aType)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   CCoeControl::HandleResourceChange(aType);   
   if (aType == KEikDynamicLayoutVariantSwitch) {
      iImageRect = WFLayoutUtils::GetMainPaneRect();
      iImageRect.iTl = TPoint(0, 0);
      if (iUseMifStartup && iConvertedImage) {
         if (WFLayoutUtils::LandscapeMode()) {
            // Phone is in landscape mode. Since we want to
            // use a image that has the same height as width,
            // we use the height as optimal size.
            TSize tmp = WFLayoutUtils::GetMainPaneSize();
            AknIconUtils::SetSize(const_cast<CFbsBitmap *>(iConvertedImage), 
                                  TSize(tmp.iHeight, tmp.iHeight));
         } else {
            // Phone is in portrait mode, lets use the width
            // as optimal size.
            TSize tmp = WFLayoutUtils::GetMainPaneSize();
            AknIconUtils::SetSize(const_cast<CFbsBitmap *>(iConvertedImage), 
                                  TSize(tmp.iWidth, tmp.iWidth));
         }
      }
      SetRect(iImageRect);
   }
#endif
}

void CImageHandler::CenterImageRect(class CFbsBitmap* aImage,
                                    TRect& aRect)
{
   if (aImage->SizeInPixels().iWidth < aRect.Width()) {
      aRect.iTl.iX += (TInt)(aRect.Width() - aImage->SizeInPixels().iWidth) >> 1;
   }
   if (aImage->SizeInPixels().iHeight < aRect.Height()) {
      aRect.iTl.iY += (aRect.Height() - aImage->SizeInPixels().iHeight) >> 1;
   }   
}

void CImageHandler::NightFilter(TBool aOn)
{
   if (iImage && aOn) {
      CFbsBitmap* tmpImage2 = iImage;
      CFbsBitmap* tmpImage = WFBitmapUtil::CopyBitmapL(iImage);
      WFBitmapUtil::FilterImage(tmpImage);
      iImage = tmpImage;
      tmpImage2->Reset();
      delete tmpImage2;
   }
}

void CImageHandler::TurnNightModeOff(const TDesC& aFileName,
                                     TInt aMbmIndex,
                                     TInt aMaskMbmIndex) 
{
   if( aMbmIndex < 0 ){
      return;
   }
   if (aMaskMbmIndex < 0) {
      CreateIconL(aFileName, aMbmIndex);
   } else {
      CreateIconL(aFileName, aMbmIndex, aMaskMbmIndex);
   }
}
