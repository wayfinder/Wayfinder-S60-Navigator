/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <fbs.h>
#include <eikenv.h>
#include <coemain.h>
#include <f32file.h>
#include "MapPlotter.h"
#include "MapUtility.h"
#include "Image.h"
#include "RandomFuncs.h"
#include "MapUtility.h"
#include "MC2SimpleString.h"

#if defined USE_AKN_LIB
# include <akniconutils.h>
#endif

#ifndef USE_TRACE
 #define USE_TRACE
#endif

#undef USE_TRACE

#ifdef USE_TRACE
 #include "TraceMacros.h"
#endif

using namespace std;

_LIT(KMifPath, ":\\data\\");
/* private constructor */
ImageConverter::ImageConverter()
      : CActive(EPriorityStandard), iMemoryLoader(NULL),
        iState(EIdle)
{
}

/* second-phase constructor */
int ImageConverter::Construct()
{
   /* allocate the memory loader */
   int errCode;
#if defined USE_AKN_LIB
   if ((errCode = CreateMifFolder()) == KErrAlreadyExists) {
      errCode = KErrNone;
   }
#elif defined IMAGEDECODER
   //new imagedecoders have to be created as we go.
   errCode = KErrNone;
   CActiveScheduler::Add(this);
#else
   TRAP( errCode, iMemoryLoader = 
         CMdaImageDescToBitmapUtility::NewL(*this) );
#endif
   if(errCode != KErrNone) {
      /* could not allocate loader, return error */
      return(KErrNoMemory);
   }

   /* success */
   return(KErrNone);
}

int ImageConverter::CreateMifFolder()
{

   TDriveList driveList;
   TInt err;
   class RFs& fs = CEikonEnv::Static()->FsSession();
   if ((err = fs.DriveList(driveList)) != KErrNone) {
      // Could not get the drive list, return the error.
      return err;
   }
#if defined __WINS__
   TChar driveLetter = 'C';
   return MkDir(driveLetter);
#else
   TChar driveLetter; 
   TInt driveNumber=EDriveA; 
   for(;driveNumber<=EDriveZ;driveNumber++) {
      if (driveList[driveNumber]) {
         TDriveInfo driveInfo;
         fs.Drive(driveInfo, driveNumber);
         if (KErrNone != (err = (fs.DriveToChar(driveNumber,driveLetter)))) {
            return err;
         }
         if (driveInfo.iType == EMediaRam) { 
            // We found the ram drive
            if (driveInfo.iMediaAtt & KMediaAttWriteProtected) {
               // Drive is write protected, we cant write to it.
               return KErrLocked;
            }
            while ((err = MkDir(driveLetter)) != KErrNone) {
               // Could not create the path, hopefully it 
               // allready exists
               if (err != KErrAlreadyExists) {
                  // Something else went wrong, return the error.
                  return err;
               }
               // The path did allready exist, try again.
               err = MkDir(driveLetter);
            }
            // Path created, return KErrNone
            return KErrNone;
         }
      }
   }
   // If we get here we could not find the ram drive
   return KErrNotFound;
#endif
}

int ImageConverter::MkDir(TChar drive)
{
   if (iMifPath) {
      delete iMifPath;
      iMifPath = NULL;
   }
   // Randomized pathName
   _LIT(KBackSlash, "\\");
   HBufC* tmpName = MapUtility::utf8ToUnicode(RandomFuncs::newRandString(10));
   iMifPath = HBufC::NewL(1 + KMifPath().Length() + 
                          tmpName->Length() + KBackSlash().Length());
   iMifPath->Des().Append(drive);
   iMifPath->Des().Append(KMifPath);
   iMifPath->Des().Append(*tmpName);
   iMifPath->Des().Append(KBackSlash);
   delete tmpName;
   tmpName = NULL;
   class RFs& fs = CEikonEnv::Static()->FsSession();
   return fs.MkDirAll(*iMifPath);
}

void ImageConverter::RmDirAndFiles(const TDesC& aPath)
{
   if (iMifPath) {
      class RFs& fs = CEikonEnv::Static()->FsSession();
      class CFileMan* fileMan = CFileMan::NewL(fs);
      fileMan->RmDir(*iMifPath);
      delete fileMan;
   }
}

void ImageConverter::SkipAndRetry()
{
   imageQueue_t::iterator it = iReadQueue.begin();   
   // Clear bitmaps.
   it->first->iBitmap->Reset();
   it->first->iMask->Reset();
   /* delete the buffer */
   delete it->second;
   /* pop the first entry */
   iReadQueue.pop_front();
   /* set state */
   iState = EIdle;
   /* will keep trying loads until queue is empty */
   tryNextLoad(KErrNone);
}

int ImageConverter::CreateBitmaps(const TFrameInfo& aFrameInfo)
{
   /* get the native color depth */
   TDisplayMode nativeColorDepth = 
      CCoeEnv::Static()->ScreenDevice()->DisplayMode();


   /* get the bitmaps from the image */
   imageQueue_t::iterator it = iReadQueue.begin();
   CFbsBitmap* bmp = it->first->iBitmap;
   CFbsBitmap* mask = it->first->iMask;


   int errCode = KErrNone;
   /* create the correct size bitmap */
   if( KErrNone != (errCode = bmp->Create(aFrameInfo.iOverallSizeInPixels, 
                                          nativeColorDepth) ) ){
      SkipAndRetry();
   }
   
   /* set the width and height of the image */
   it->first->iWidth  = aFrameInfo.iOverallSizeInPixels.iWidth;
   it->first->iHeight = aFrameInfo.iOverallSizeInPixels.iHeight;
   
   /* check if alpha information is present for mask */
   if(aFrameInfo.iFlags & TFrameInfo::ETransparencyPossible) {
      
      /* create the correct size masking bitmap */
      if(KErrNone != 
         (errCode = mask->Create(aFrameInfo.iOverallSizeInPixels, EGray2)))
         {
            /* reset the data for the bitmap */
            bmp->Reset();
            SkipAndRetry();
            return errCode;
         }
      
      /* set mask flag */
      it->first->iHasMask = ETrue;
   }
   return KErrNone;
}


/* from the base class - MMdaImageUtilObserver */
void ImageConverter::MiuoOpenComplete(TInt aError)
{
   switch(aError) {
      /* success */
      case(KErrNone): {
#if 0
         /* get the native color depth */
         TDisplayMode nativeColorDepth = 
            CCoeEnv::Static()->ScreenDevice()->DisplayMode();

			/* get image info */
			TFrameInfo frameInfo;
			iMemoryLoader->FrameInfo(0, frameInfo);

         /* get the bitmaps from the image */
         imageQueue_t::iterator it = iReadQueue.begin();
         CFbsBitmap* bmp = it->first->iBitmap;
         CFbsBitmap* mask = it->first->iMask;

		   /* create the correct size bitmap */
			if(bmp->Create(frameInfo.iOverallSizeInPixels, 
                        nativeColorDepth) != KErrNone)
			{
            /* delete the buffer */
            delete it->second;
            /* pop the first entry */
            iReadQueue.pop_front();
            /* set state */
            iState = EIdle;
            /* will keep trying loads until queue is empty */
            tryNextLoad(KErrNone);
			   return;
			}

         /* set the width and height of the image */
         it->first->iWidth = frameInfo.iOverallSizeInPixels.iWidth;
         it->first->iHeight = frameInfo.iOverallSizeInPixels.iHeight;

         /* check if alpha information is present for mask */
         if(frameInfo.iFlags & TFrameInfo::ETransparencyPossible) {

				/* create the correct size masking bitmap */
				if(mask->Create(frameInfo.iOverallSizeInPixels, EGray2) != KErrNone)
				{
               /* reset the data for the bitmap */
					bmp->Reset();
               /* delete the buffer */
               delete it->second;
               /* pop the first entry */
               iReadQueue.pop_front();
               /* set state */
               iState = EIdle;
               /* will keep trying loads until queue is empty */
               tryNextLoad(KErrNone);
					return;
				}

            /* set mask flag */
            it->first->iHasMask = ETrue;

      		/* set status to converting in progress */
	   		iState = EConverting;

            /* do the actual loading */
            iMemoryLoader->ConvertL(*bmp, *mask);
         }
         /* without mask */
         else {
      		/* set status to converting in progress */
	   		iState = EConverting;

            /* do the actual loading */
            iMemoryLoader->ConvertL(*bmp);
         }
#else

#ifndef IMAGEDECODER         
         /* get image info */
         TFrameInfo frameInfo;
         iMemoryLoader->FrameInfo(0, frameInfo);

         if(KErrNone == CreateBitmaps(frameInfo)){
            /* set status to converting in progress */
            iState = EConverting;            
            imageQueue_t::value_type& front = iReadQueue.front();
            if(front.first->HasMask()){
               /* do the actual loading */
               iMemoryLoader->ConvertL(*(front.first->GetBitmap()), 
                                       *(front.first->GetMask()));
            } else {             /* without mask */
               /* do the actual loading */
               iMemoryLoader->ConvertL(*(front.first->GetBitmap()));
            }
         } else {
            SkipAndRetry();
         }
#endif
   }
            
#endif
         break;
      default: 
         SkipAndRetry();
   };

   return;
}

void ImageConverter::MiuoConvertComplete(TInt aError)
{
   switch(aError) {
      /* success */
      case KErrNone:
      {
         /* get the first queue entry */
         imageQueue_t::iterator it = iReadQueue.begin();
         it->first->iBitmapLoaded = ETrue;
         if(it->first->iHasMask) {
            it->first->iMaskLoaded = ETrue;
         } 
         CalculateVisibleRect(it->first);
         /* delete the buffer */
         delete it->second;
         /* pop the first entry */
         iReadQueue.pop_front();
         /* set the state */
         iState = EIdle;
         tryNextLoad(KErrNone);
         break;
      }
      /* error */
      default:
      {
         /* set the state */
         iState = EIdle;
         SkipAndRetry();
         break;
      }
   }
}

void ImageConverter::MiuoCreateComplete(TInt /*aError*/)
{
   /* nothing here */
   return;
}

   /* from the base class - CActive */
void ImageConverter::RunL()
{
   if(iStatus == KErrNone){
      /* get the first queue entry */
      imageQueue_t::iterator it = iReadQueue.begin();
      it->first->iBitmapLoaded = ETrue;
      if(it->first->iHasMask) {
         it->first->iMaskLoaded = ETrue;
      }
      CalculateVisibleRect(it->first);
      /* delete the buffer */
      delete it->second;
      /* pop the first entry */
      iReadQueue.pop_front();
      /* set the state */
      iState = EIdle;
      tryNextLoad(KErrNone);      
   } else {
      SkipAndRetry();
   }
}

void ImageConverter::CalculateVisibleRect(Image* aImage)
{
   if (aImage->iHasMask) {
      CFbsBitmap* mask = aImage->GetMask();
      TInt xmin = aImage->iWidth;
      TInt xmax = 0;
      TInt ymin = aImage->iHeight;
      TInt ymax = 0;
#if defined USE_AKN_LIB
      //Iterating the pixels with GetPixel() works for sure on all symbian,
      //but it's pretty slow and we have to look at every one pixel.
      //On a 800 x 800 image we do 640 000 GetPixel() iterations.
      TRgb color;
      TRgb white(255, 255, 255);
      for (TInt y = 0; y < aImage->iHeight; y++) {
         for (TInt x = 0; x < aImage->iWidth; x++) {
            mask->GetPixel(color, TPoint(x, y));
            if (color == white) {
               if (xmin > x) {
                  xmin = x;
               }
               if (xmax < x) {
                  xmax = x;
               }
               if (ymin > y) {
                  ymin = y;
               }
               if (ymax < y) {
                  ymax = y;
               }
            }
         }
      }
#else
      //Iterating the pixels by hand in memory probably works on all symbian,
      //it's much faster since we look at 8 pixels at a time, 
      //It needs some testing and verification.
      //On a 800 x 800 image we do roughly 83 000 iterations.
      TDisplayMode dMode = mask->DisplayMode();
      if (dMode == EGray2) {
# ifndef NAV2_CLIENT_SERIES60_V1
         mask->LockHeap();
# endif
         TUint32* imgPtr = mask->DataAddress();
         TSize imgSize = mask->SizeInPixels();
         TInt imgByteWidth = imgSize.iWidth >> 3;
         TInt imgBitsLeft = imgSize.iWidth % 8;
         TInt lineLength = CFbsBitmap::ScanLineLength(imgSize.iWidth, dMode);
         TUint8* pCurrByteLine = (TUint8*) imgPtr;
         TUint8 currByte;
         TInt currXPixelOffset;
         TInt currXPixel;
         for (TInt y = 0; y < imgSize.iHeight; y++) {
            for (TInt x = 0; x < imgByteWidth; x++) {
               currByte = pCurrByteLine[x];
               //If currByte is != 0, it contains at least one white pixel.
               if (currByte) {
                  if (ymin > y) {
                     ymin = y;
                  }
                  if (ymax < y) {
                     ymax = y;
                  }
                  currXPixelOffset = x << 3;
                  //Check if this byte of pixels might contain xmin or xmax.
                  if ((currXPixelOffset < xmin) || 
                      ((currXPixelOffset + 7) > xmax)) {
                     for (TInt b = 0; b < 8; b++) {
                        //Some of the 8 pixels in the byte are visible.
                        //Find which ones that mather for the x-axis.
                        if  (currByte & (1 << b)) {
                           currXPixel = currXPixelOffset + b;
                           if (xmin > currXPixel) {
                              xmin = currXPixel;
                           }
                           if (xmax < currXPixel) {
                              xmax = currXPixel;
                           }
                        }
                     }
                  }
               }
            }
            //Here we take care of bit padded bytes when the
            //image width is not evenly dividable by a byte.
            if (imgBitsLeft != 0) {
               currByte = pCurrByteLine[imgByteWidth];
               currXPixelOffset = imgByteWidth << 3;
               for (TInt b = 0; b < imgBitsLeft; b++) {
                  if  (currByte & (1 << b)) {
                     currXPixel = currXPixelOffset + b;
                     if (xmax < currXPixel) {
                        xmax = currXPixel;
                     }
                  }
               }
            }
            //Move to next line in image.
            pCurrByteLine = pCurrByteLine + lineLength;
         }
      }
# ifndef NAV2_CLIENT_SERIES60_V1
      mask->UnlockHeap();
# endif
#endif
      //Set the image visible rectangle.
      //We need to add +1 to xmax and ymax since the image 
      //pixels go from 0 to image width-1 but we're interested
      //in the the actual rectangular size of the image.
      //Default rect set to 0 size and we only set it if we've
      //detected a visible rectangle in the mask.
      if ((xmin != aImage->iWidth) && (ymin != aImage->iHeight)) {
         aImage->iVisibleRect = TRect(xmin, ymin, 
                                      xmax+1, ymax+1);
      }
   } else {
      aImage->iVisibleRect = TRect(0, 0, 
                                   aImage->iWidth, aImage->iHeight);
   }
}

void ImageConverter::DoCancel()
{
#ifdef IMAGEDECODER
   iImageDecoder->Cancel();
#endif
}

int OpenNewImageDecoderL(class CImageDecoder*& aDecoderPtr, 
                         const TDesC8& aData)
{
   int errCode = KErrNone;
#ifdef IMAGEDECODER
   delete aDecoderPtr;
   aDecoderPtr = NULL;

   class RFs& fs = CEikonEnv::Static()->FsSession();
   TBuf8<256> mimeType;
   CImageDecoder::GetMimeTypeDataL(aData, mimeType);
   aDecoderPtr = CImageDecoder::DataNewL(fs, aData);
#else
   aDecoderPtr = aDecoderPtr;
   aData.Length();
#endif
   return errCode;
}

HBufC* ImageConverter::CreateTmpFileNameLC()
{
   if (iMifPath) {
   	_LIT(KExt, ".mif");
      // Was memleak here
   	HBufC* tmpName = MapUtility::utf8ToUnicode(
         MC2SimpleStringNoCopy( RandomFuncs::newRandString(10) ).c_str() );
   	HBufC* completeName = HBufC::NewLC(iMifPath->Length() + 
                                      tmpName->Length() + 
                                      KExt().Length());
   	completeName->Des().Copy(*iMifPath);
   	completeName->Des().Append(*tmpName);
   	completeName->Des().Append(KExt);
   	delete tmpName;
   	return completeName;
   } else {
   	return NULL;
   }
}

int ImageConverter::LoadIconFromMif(const TDesC8& aData)
{
#ifdef USE_AKN_LIB
   class RFs& fs = CEikonEnv::Static()->FsSession();
   class RFile file;
   HBufC* completeName = CreateTmpFileNameLC();
   if (completeName) {
      int errCode = file.Replace(fs, *completeName, EFileWrite | EFileShareAny);
      while (errCode != KErrNone) {
         CleanupStack::PopAndDestroy(completeName);
         if (errCode != KErrAlreadyExists) {
            // We could not create the file, no use in continue trying.
            return errCode;
         }
         // File already exists, try with another name.
         completeName = CreateTmpFileNameLC();
         errCode = file.Replace(fs, *completeName, EFileWrite | EFileShareAny);
      }
      
      file.Write(aData);
      file.Close();
      imageQueue_t::iterator it = iReadQueue.begin();
      TInt bitmapId = 0;
      TInt maskId = 1;
      CFbsBitmap* bmp = it->first->iBitmap;
      CFbsBitmap* mask = it->first->iMask;
      delete bmp;
      delete mask;
      
      AknIconUtils::ValidateLogicalAppIconId(*completeName, bitmapId, maskId);
      AknIconUtils::CreateIconL(bmp, mask, *completeName, bitmapId, maskId);
      TSize size;
      AknIconUtils::GetContentDimensions(bmp, size);
      size.iWidth *= iDpiCorrFact;
      size.iHeight *= iDpiCorrFact;
      AknIconUtils::SetSize(bmp, size);
      it->first->iBitmap = bmp;
      it->first->iMask = mask;
      it->first->iHeight = bmp->SizeInPixels().iHeight;
      it->first->iWidth = bmp->SizeInPixels().iWidth;
      it->first->iBitmapLoaded = ETrue;
      fs.Delete(*completeName);
      CleanupStack::PopAndDestroy(completeName);
      if (mask) {
         it->first->iHasMask    = ETrue;
         it->first->iMaskLoaded = ETrue;
         CalculateVisibleRect(it->first);
         /* delete the buffer */
         delete it->second;
         /* pop the first entry */
         iReadQueue.pop_front();
         /* set the state */
         iState = EIdle;
         tryNextLoad(KErrNone);      
      } else {
         SkipAndRetry();      
      }	
   } else {
      return KErrGeneral;
   }
#endif
   return KErrNone;
}

/* initiate a conversion from the first entry in the read queue */
int ImageConverter::beginLoad()
{
   /* get the first entry in the queue */
   imageQueue_t::iterator it = iReadQueue.begin();

   /* set the converter state */
   iState = EOpening;

   /* initiate the loading */
   int errCode;
#ifdef IMAGEDECODER
   TRAP(errCode, OpenNewImageDecoderL(iImageDecoder, *it->second));
#elif defined USE_AKN_LIB
   TRAP(errCode, LoadIconFromMif(*it->second));
#else
   TRAP(errCode, iMemoryLoader->OpenL(*it->second));
#endif
   if(errCode == KErrUnderflow) {
      /* insufficient data in descriptor */
      iState = EIdle;
      return(errCode);
   }

#ifdef IMAGEDECODER
   // reading of frame data is handled in MiuoOpenComplete when using
   // iMemoryLoader.
   const TFrameInfo& info = iImageDecoder->FrameInfo();
   CreateBitmaps(info);
   if(it->first->HasMask()){
      iImageDecoder->Convert( &iStatus, *(it->first->GetBitmap()),
                         *(it->first->GetMask()) );
   } else {
      iImageDecoder->Convert(&iStatus, *(it->first->GetBitmap()));
   }
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   SetActive();
#endif


   return(errCode);
}

/* tries to load the entries in the 
   queue until success or queue is empty */
int ImageConverter::tryNextLoad(int initialError)
{
   int errCode = initialError;

   /* if more entries, loop thru the queue until a load occurs */
   bool done = false;
   while(done == false) {
      if(iReadQueue.empty()) {
         /* no more entries */
         done = true;
      }
      else {
         /* try to load the next one */
         errCode = beginLoad();
         if(errCode == KErrNone) {
            /* a load succeeded, get out of the loop */
            done = true;
         }
         else {
            /* error .. delete the buffer */
            imageQueue_t::iterator it = iReadQueue.begin();
            delete it->second;
            /* Issues never end .. pop one more entry */
            iReadQueue.pop_front();
         }
      }
   }

   return(errCode);
}

/* allocator */
ImageConverter* ImageConverter::allocate()
{
   /* allocate new object */
   ImageConverter* newObj = new ImageConverter();
   if(newObj == NULL) return(NULL);

   /* do second-phase */
   if(newObj->Construct() != KErrNone) {
      delete newObj;
      return(NULL);
   }

   /* success */
   return(newObj);
}

/* destructor */
ImageConverter::~ImageConverter()
{
   /* cancel any active conversion */
#if !defined(IMAGEDECODER) && !defined(USE_AKN_LIB)
   iMemoryLoader->Cancel();
#else
   Cancel();
#endif
   /* empty the read queue */
   for(imageQueue_t::iterator it = iReadQueue.begin();
       it != iReadQueue.end();
       ++it) 
   {
      /* delete the remaining buffers */
      delete it->second;
   }
   iReadQueue.clear();
   /* release the loader */
   delete iMemoryLoader;
   delete iImageDecoder;
   RmDirAndFiles(*iMifPath);
   delete iMifPath;
}

/* loads an image using a memory buffer */
int ImageConverter::loadImage(const byte* imgBuf,
                              uint32 numBytes,
                              Image* destImage,
                              int dpiCorrectionFactor)
{
   iDpiCorrFact = dpiCorrectionFactor;
   /* create a HBufC8 from the data */
   TPtrC8 tempPtr;
   tempPtr.Set(imgBuf, numBytes);
   HBufC8* imagePtr = HBufC8::New(numBytes);
   if(imagePtr == NULL) return(KErrNoMemory);
   *imagePtr = tempPtr;

   /* add to read queue */
   iReadQueue.push_back( make_pair(destImage,imagePtr) );

   /* check if any bitmap is currently loading */
   if(iState != EIdle) {
      /* yup, return quietly */
      return(KErrNone);
   }

   /* converter is idle, start loading the bitmap */
   int errCode = beginLoad();
   if(errCode != KErrNone) {
      /* delete the buffer */
      delete imagePtr;
      /* we have issues, pop the first entry */
      iReadQueue.pop_front();
      /* keep trying to load it until we succeed or entries run out */
      errCode = tryNextLoad(errCode);
      return(errCode);
   }

   /* success */
   return(KErrNone);
}


Image::Image() : iBitmap(NULL),
                 iMask(NULL),
                 iWidth(-1),
                 iHeight(-1),
                 iBitmapLoaded(EFalse),
                 iMaskLoaded(EFalse),
                 iHasMask(EFalse),
                 iVisibleRect(TRect(0,0,0,0))
{};


int Image::Construct() {
   /* create an empty bitmap */
   iBitmap = new CFbsBitmap();
   if(iBitmap == NULL) return(-1);
   /* create an empty mask */
   iMask = new CFbsBitmap();
   if(iMask == NULL) {
      delete iBitmap;
      return(-2);
   }
   /* success */
   return(0);
}

/* frees internally allocated memory and releases bitmaps */
void Image::Release() {
   /* free bitmaps; NOTE - the objects still exist but the bitmap 
      data has been released */
   iBitmap->Reset();
   iMask->Reset();
   return;
}
   

/* destructor */
Image::~Image() {
   /* delete the bitmap */
   if(iBitmap) delete iBitmap;
   /* delete the bitmap */
   if(iMask) delete iMask;
}

/* allocator */
Image* Image::allocate() {
   Image* newObj = new Image();
   if(newObj == NULL) return(NULL);
   /* do second-phase construct */
   if(newObj->Construct() != 0) {
      /* error */
      delete newObj;
      return(NULL);
   }
   return(newObj);
}

/* returns ETrue if bitmap and mask are loaded and ready
   it checks the image type to determine whether to check for
   loaded mask or not */
TBool Image::IsReady() const {
   if(iHasMask) {
      if(iBitmapLoaded && iMaskLoaded) {
         return(ETrue);
      }
   }
   else {
      if(iBitmapLoaded) {
         return(ETrue);
      }
   }
   return(EFalse);
}

/* returns true if there is a a mask */
TBool Image::HasMask() const
{
   return(iHasMask);
}

/* returns a pointer to the internal Bitmap */
CFbsBitmap* Image::GetBitmap() const
{
   return(iBitmap);
}

/* returns a pointer to the internal Mask */
CFbsBitmap* Image::GetMask() const
{
   return(iMask);
}

/* gets the width of the bitmap */
int32 Image::getWidth() const 
{
   return(iWidth);
}

/* gets the height of the bitmap */
int32 Image::getHeight() const 
{
   return(iHeight);
}

TSize Image::GetSize() const
{
   TSize size(0,0);
   if(IsReady() && iBitmap){
      size = iBitmap->SizeInPixels();
   }
   return size;
}

/* gets the visible rect of the bitmap */
PixelBox Image::GetVisibleRect() const 
{
   return(iVisibleRect);
}
