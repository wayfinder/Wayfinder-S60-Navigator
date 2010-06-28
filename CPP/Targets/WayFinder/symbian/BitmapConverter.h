/*  Copyright (c) 2004-2006, Nokia. */

#ifndef IMAGE_CONVERTER_H
#define IMAGE_CONVERTER_H

//  INCLUDE FILES

#include <e32base.h>
//#include <e32cmn.h>
#include <gdi.h>


class MImageObserver
{
public:
   virtual void ImageConversionDone(TInt aResult) = 0;
};

/**
*  CBitmapConverter
*  Image loader class.
*/
class CBitmapConverter : public CActive
{
public:
   /** 
    * Factory method that constructs a CBitmapConverter 
    * by using the NewLC method and then cleans the 
    * cleanup stack. 
    */
   static CBitmapConverter* NewL(class MImageObserver* aObserver, 
                                 class RFs& aFs);
   /** 
    * Factory method that constructs a CBitmapConverter 
    * and leaves it to the cleanup stack. 
    */
   static CBitmapConverter* NewLC(class MImageObserver* aObserver,
                                  class RFs& aFs);

   /** 
    * Desctructor. Destroys the CImageDecoder used 
    * by the image handler. 
    */
   virtual ~CBitmapConverter();
   
public:
   /**
    * Sets the target bitmap where the bitmap data is loaded to.
    * @param aBitmap Bitmap where the image data is loaded to.
    * @return True if operation is successfull, false if not.
    */
//    TBool SetBitmapL( CFbsBitmap*& aBitmap );
   
   /**
    * Loads a the given frame from the given file.
    * @return TInt EFalse if the ao is active
    * @param aFileName Filename wherefrom the bitmap data is loaded.
    * @param aSize The size of the bitmap.
    * @param aDoScale True if the picture should be scaled.
    * @param aSelectedFrame A single frame index in a multi-frame 
    *        file. If not given the first frame is loaded.
    */
   TInt ConvertFileL(const TDesC& aFileName, 
                     class TSize aSize, 
                     TDisplayMode aDisplayMode,
                     TBool aDoScale = EFalse, 
                     TInt  aSelectedFrame = 0);

   TInt GetConvertState();

   enum TConvertionState {
      EIdle              = 0,
      EConverting        = 1,
      EScaling           = 2,
      EConvertStateReady = 3,
   };

   /** Current state of the convertion.*/
   enum TConvertionState iState;

   const CFbsBitmap* GetImage();
   const CFbsBitmap* GetMask();
   void CompleteRequest(TInt aStatus);

   
private: // From CActive
   
   /**
    * CActive::RunL() implementation. Called on image 
    * load success/failure.
    */
   void RunL(); 
   /**
    * CActive::Cancel() implementation. Stops decoding.
    */
   void DoCancel();
   
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
   CBitmapConverter(class MImageObserver* aObserver,
                    class RFs& aFs);
   /**
    * 2nd phase constructor. Adds this object to the active scheduler.
    */
   void ConstructL();

private:
   void ScaleImage();
   void Complete();
   
private: // Data
   /** Size of the bitmap when convertion and scaling is done. */
   class TSize iSize;
   /** Observer that gets a notification when convertion and
       scaling is completed */
   class MImageObserver* iObserver;
   /** Image decoder that is used to load the image data from file. */
   class CImageDecoder* iDecoder;
   /** Bitmap (owned by the user of this class) where the 
       loaded image data is put. */
   class CFbsBitmap *iBitmap; 
   class CFbsBitmap *iMask; 
   /** File server (owned by the user of this class) that is used 
       to load the raw image data from files. */
   class RFs& iFs;
//    /** Current image frame information. */
//    class TFrameInfo* iFrameInfo;
   /** For scaling the bitmap.*/
   class CBitmapScaler* iScaler;
   /** Flag that tells us if we should scale the image or not. */
   TBool iDoScale;
                                
};

#endif
