/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPLIB_IMAGE_H
#define MAPLIB_IMAGE_H

/* 
	This class is derived from MapPlotter's BitMap class.
   It is Symbian-Specific and implements an
	interface needed for Disk Image loading. It uses Symbian's native
   Media Server to load the following
	formats :- BMP, GIF, JPG, PNG, MBM 
*/

#include <map>
#include <list>
#include "MapPlotter.h"
#include "PixelBox.h"


#if defined NAV2_CLIENT_SERIES60_V1 || NAV2_CLIENT_SERIES60_V2 || NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES80 
//nothing
#elif defined NAV2_CLIENT_SERIES90_V1 
# define IMAGEDECODER
#elif defined NAV2_CLIENT_SERIES60_V3
# define USE_AKN_LIB
#else
# error You have to choose an imageconverter!
#endif

// #undef IMAGEDECODER
// #undef USE_AKN_LIB
// #define IMAGEDECODER

#ifndef IMAGEDECODER

# include <mda/client/utility.h>
# include <mdaimageconverter.h>
class CImageDecoder : public CBase {};

#else

# include <imageconversion.h>
class MMdaImageUtilObserver {};
class CMdaImageDescToBitmapUtility {};

#endif



/* forward declarations */
class isab::BitMap;
class CFbsBitmap;
class CMdaServer;
class Image;

/* common image converter for all formats .. only supports memory loading */
class ImageConverter : public CActive, public MMdaImageUtilObserver
{
   /*** data ***/
private:
   /* the memory loader */
   class CMdaImageDescToBitmapUtility* iMemoryLoader;
   class CImageDecoder* iImageDecoder;

      /* the read queue */
   typedef std::pair<Image*,HBufC8*> imageQueueEntry_t;
   typedef std::list< imageQueueEntry_t > imageQueue_t;
   imageQueue_t iReadQueue;

   /* the converter's state */
   enum
   {
      EIdle,
      EOpening,
      EConverting
   };
   TInt iState;
   HBufC* iMifPath;
   TInt iDpiCorrFact;

   /*** methods ***/
private:
   /* private constructor */
   ImageConverter();

   /* second-phase constructor */
   int Construct();

   /* from the base class - MMdaImageUtilObserver */
   virtual void MiuoConvertComplete(TInt aError);
   virtual void MiuoCreateComplete(TInt aError);
   virtual void MiuoOpenComplete(TInt aError);

   /* Calculates the visible rectangle for the image */
   void CalculateVisibleRect(Image* aImage);

   /* from the base class - CActive */
   virtual void RunL();
   virtual void DoCancel();

   /* initiate a conversion from the first entry in the read queue */
   int beginLoad();

   /* tries to load the entries in the 
      queue until success or queue is empty */
   int tryNextLoad(int initialError);

   void SkipAndRetry();
   int CreateBitmaps(const TFrameInfo& aFrameInfo);

   int LoadIconFromMif(const TDesC8& aData);
   int CreateMifFolder();
   int MkDir(TChar drive);
   void RmDirAndFiles(const TDesC& aPath);
   HBufC* CreateTmpFileNameLC();

public:
   /* allocator */
   static ImageConverter* allocate();

      /* destructor */
   virtual ~ImageConverter();

   /* loads an image using a memory buffer */
   int loadImage(const byte* imgBuf,
                 uint32 numBytes,
                 Image* destImage,
                 int dpiCorrectionFactor);
};

/* main class declaration */
class Image : public isab::BitMap
{
   /* friends, romans and countrymen */
   friend class ImageConverter;

	/* data members */
private:
   /* the actual bitmap */
   CFbsBitmap* iBitmap;
   /* mask bitmap */
   CFbsBitmap* iMask;
   /* width and height of image */
   int32 iWidth, iHeight;
   /* flag set to true if bitmap is present */
   TBool iBitmapLoaded;
   /* flag set to true if mask is present */
   TBool iMaskLoaded;
   /* true if the mask is loaded */
   TBool iHasMask;
   /* visible rect of image */
   PixelBox iVisibleRect;

	/* methods */
private:
   /* constructor */
   Image();
   
   /* second-phase constructor */
   int Construct();

   /* frees internally allocated memory and releases bitmaps */
   void Release();
   
public:

   /* destructor */
   ~Image();

   /* allocator */
   static Image* allocate();

   /* returns ETrue if bitmap and mask are loaded and ready
      it checks the image type to determine whether to check for
      loaded mask or not */
   TBool IsReady() const ;

   /* returns true if there is a a mask */
   TBool HasMask() const;

   /* returns a pointer to the internal Bitmap */
   CFbsBitmap* GetBitmap() const;

   /* returns a pointer to the internal Mask */
   CFbsBitmap* GetMask() const;

   /* gets the width of the bitmap */
   int32 getWidth() const ;

   /* gets the height of the bitmap */
   int32 getHeight() const ;

   TSize GetSize() const;

   /* gets the visible rect of the bitmap */
   PixelBox GetVisibleRect() const;
};

#endif

