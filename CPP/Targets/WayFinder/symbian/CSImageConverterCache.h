/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CS_IMAGE_CONVERTER_CACHE_H
#define CS_IMAGE_CONVERTER_CACHE_H

#include "arch.h"

#include <map>
#include <vector>
#include <f32file.h>
#include <eikimage.h>

#include "CSImageConverterCacheObserver.h"
#include "BitmapConverter.h"
#include "STLStrComp.h"

typedef std::map<HBufC*, class CEikImage*, strCompareLess> cacheMap_t;
typedef std::map<HBufC*, class CEikImage*>::iterator cacheMapIt_t;

//mplate <class ImageCacheRequester>
class CCSImageConverterCache : public CBase,
                               public MImageObserver
{

 public:

   static CCSImageConverterCache* NewLC(MCSImageConverterCacheObserver& aRequester, 
                                        const TDesC& aImageExt);
   static CCSImageConverterCache* NewL(MCSImageConverterCacheObserver& aRequester, 
                                       const TDesC& aImageExt);
   virtual ~CCSImageConverterCache();

   void ConvertImage(const TDesC& aFilename, TSize aSize);

public: // From MImageObserver

   void ImageConversionDone(TInt aResult);

 protected:

   void ConstructL();
   CCSImageConverterCache(MCSImageConverterCacheObserver& aRequester, 
                          const TDesC& aImageExt);
 public:

   void LocateAndConvertImages(const TDesC& aPath, TSize aSize);
   const cacheMap_t GetImageCache() const;

   cacheMap_t::const_iterator FindImagePair(HBufC* aFilename) const;

   CEikImage* FindImage(HBufC* aFilename); 

 private:
   std::vector<TFileName> iNameVec;
   cacheMap_t iCacheMap;
   MCSImageConverterCacheObserver& iRequester;
   CBitmapConverter* iBitmapConverter;
   RFs iFs;
   TSize iSize;
   TBuf<5> iImageExt;
};

#endif
