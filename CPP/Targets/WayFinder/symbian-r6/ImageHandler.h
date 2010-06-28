/*  Copyright (c) 2004-2006, Nokia. */

#ifndef IMAGE_HANDLER_H
#define IMAGE_HANDLER_H

//  INCLUDE FILES

//#include <e32cmn.h>
#include <w32std.h>
#include <coecntrl.h> 
#if defined NAV2_CLIENT_SERIES60_V3
# include <akniconutils.h>
#endif
#include "BitmapConverter.h"
#include "ScaleMode.h"

/**
*  CImageHandler
*  Image loader class.
*/
class CImageHandler : public CCoeControl,
                      public MImageObserver
{
public:
   /** 
    * Factory method that constructs a CImageHandler 
    * by using the NewLC method and then cleans the 
    * cleanup stack. 
    */
   static CImageHandler* NewL(const TRect& aBitMapRect, 
                              /*class RFs& aFs,*/
                              class MImageHandlerCallback* aCallback);

   /** 
    * Factory method that constructs a CImageHandler 
    * and leaves it to the cleanup stack. 
    */
   static CImageHandler* NewLC(const TRect& aBitMapRect, 
                               /*class RFs& aFs,*/
                               class MImageHandlerCallback* aCallback);

   static CImageHandler* NewL(const TRect& aImageRect);
   static CImageHandler* NewLC(const TRect& aImageRect);

   /** 
    * Desctructor. Destroys the CImageDecoder used 
    * by the image handler. 
    */
   virtual ~CImageHandler();
   
public:

   /**
    * Creates and loads an icon from the mbm/mif file.
    * @param aIconFileName name of the mbm/mif file.
    * @param aImageId id of the image to be loaded.
    * @param aImageMaskId id of the mask to be loaded.
    * @param aScaleMode defines how the image should be scaled 
    *                   when setting the size on it (s60v3).
    * @param aCenterRealImageRect depending on the scaleMode the 
    *                             image size can be less than the 
    *                             container rect. This variable tells
    *                             us if we want to center the imagerect
    *                             when setting the size on the container
    *                             rect.
    */
   void CreateIconL(const TDesC& aIconFilename,  
                    TInt aImageId,  
                    TInt aImageMaskId,
                    TScaleMode aScaleMode = EAspectRatioPreserved,
                    TBool aCenterRealImageRect = EFalse,
                    TBool aIsNightModeOn = EFalse);


   /**
    * Creates and loads an icon from the mbm/mif file.
    * @param aIconFileName name of the mbm/mif file.
    * @param aImageId id of the image to be loaded.
    * @param aScaleMode defines how the image should be scaled 
    *                   when setting the size on it (s60v3).
    * @param aCenterRealImageRect depending on the scaleMode the 
    *                             image size can be less than the 
    *                             container rect. This variable tells
    *                             us if we want to center the imagerect
    *                             when setting the size on the container
    *                             rect.
    */
   void CreateIconL(const TDesC& aIconFilename,
                    TInt aImageId,
                    TScaleMode aScaleMode = EAspectRatioPreserved,
                    TBool aCenterRealImageRect = EFalse);

   /**
    * Loads a the given frame from the given file.
    * @param aFileName Filename wherefrom the bitmap data is loaded.
    * @param aSize The size of the bitmap.
    * @param aDoScale True if the picture should be scaled.
    * @param aSelectedFrame A single frame index in a multi-frame 
    *        file. If not given the first frame is loaded.
    */
   TInt LoadFileL(const TDesC& aFileName, 
                  class TSize aSize, 
                  TBool aDoScale = EFalse, 
                  TInt  aSelectedFrame = 0);

   /**
    * NightFilters the image using WFBitmapUtil::FilterImage.
    */
   void NightFilter(TBool aOn);

   /**
    * Turns night mode off. On s60v3 the graphic server is caching 
    * the image so for getting the original image back we need to
    * reload the image and change the size one pixel.
    * @param aFileName name of the mbm/mif file.
    * @param aMbmIndex mbm/mif index of the image to be loaded.
    * @param aMaskMbmIndex index of the mask to be loaded.
    */
   void TurnNightModeOff(const TDesC& aFileName, 
                         TInt aMbmIndex,
                         TInt aMaskMbmIndex = -1);

   /**
    * Tells if this image should be visible or not.
    * @return True if image should be visible
    * @return False if image not should be visible
    */
   TBool GetShow();

#ifdef NAV2_CLIENT_SERIES60_V3
   /**
    * Loads a svg image from a mif-file and draws this
    * image to the whole screen.
    */
   void LoadMifImage(const TDesC& aFileName,
                     TSize aSize);
#endif

   
   /**
    * Returns the current frame information.
    * @return Current frame information.
    */
   const class TFrameInfo& FrameInfo() const;

   class TSize GetSize();
   class TRect GetRect();
   class TPoint GetTopLeftPos();

   /**
    * Sets the rect of this container class, on s60v3
    * we resize the image as well.
    * @param aRect The new rect.
    * @param aScaleMode defines how the image should be scaled 
    *                   when setting the size on it (s60v3).
    * @param aCenterRealImageRect depending on the scaleMode the 
    *                             image size can be less than the 
    *                             container rect. This variable tells
    *                             us if we want to center the imagerect
    *                             when setting the size on the container
    *                             rect.
    */
   void SetImageRect(const TRect& aRect, 
                     TScaleMode aScaleMode = EAspectRatioPreserved,
                     TBool aCenterRealImageRect = EFalse);

   void SetShow(TBool aShow);

   void SetClear(TBool aClear);

   enum TConvertionState {
      EConverting        = 0,
      EScaling           = 1,
      EConvertStateReady = 2,
   };
   void HandleLayoutSwitch(TInt aType);
   
public: // From CoeControl
   void SizeChanged();

   virtual void MakeVisible(TBool aVisible);

private: // From CoeControl
   void Draw(const TRect& aRect) const;
//    void HandleResourceChange(TInt aType);

public: // From MImageObserver
   void ImageConversionDone(TInt aResult);

protected:
   /**
    * C++ default constructor. Just stores the given parameters 
    * to corresponding attributes.
    *  @param aBitmap Bitmap where the image data is loaded to.
    * @param aFs File server reference that is used to load the 
    *        image data.
    * @param aCallback Listener interface implementation that 
    *        is notified when an image has been loaded.
    */
   CImageHandler(/*class RFs& aFs,*/
                 class MImageHandlerCallback* aCallback);

   CImageHandler();
   /**
    * 2nd phase constructor. Adds this object to the active scheduler.
    */
   void ConstructL(const TRect& aBitMapRect);

private:
   void ScaleImage();
   
   /**
    * This function sets the size of aRect to the size of
    * aImage, it also centers the new rect inside the old
    * rect.
    * @param aImage the image that should define the new size.
    * @param aRect the rect that is to be changed.
    */
   void CenterImageRect(class CFbsBitmap* aImage, TRect& aRect);

private: // Data
   /** Bitmap (owned by the user of this class) where the 
       loaded image data is put. */
   class CFbsBitmap* iImage; 
   class CFbsBitmap* iImageMask;
   const class CFbsBitmap* iConvertedImage;
   /** File server (owned by the user of this class) that is used 
       to load the raw image data from files. */
   class RFs iFs;
   /** Listener that is notified when an image has been loaded. */
   class MImageHandlerCallback* iCallback;
   /** Using this class we can find out how large the screen is,
       (ownded by the user of this class). */
   class CWsScreenDevice* iScreenDevice;
   /** CWsScreenDevice returns the size of the screen in this class */
   struct TPixelsTwipsAndRotation iPixTwipsRot;
   /** Class that converts from png to bitmap and scales the image
       if wanted. */
   class CBitmapConverter* iImageConverter;
   /** The pictures rect */
   TRect iImageRect;
   /** True if the image should be drawn. */
   TBool iShow;
   /** True if the srean should be cleared */
   TBool iClear;
   /** True if we are using mif-file as startup image */
   TBool iUseMifStartup;
};

#endif
