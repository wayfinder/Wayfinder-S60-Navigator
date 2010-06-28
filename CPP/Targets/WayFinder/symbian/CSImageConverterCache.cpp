/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "CSImageConverterCache.h"

CCSImageConverterCache* CCSImageConverterCache::NewLC(MCSImageConverterCacheObserver& aRequester,
                                                      const TDesC& aImageExt) 
{
   CCSImageConverterCache* self = new (ELeave) CCSImageConverterCache(aRequester, aImageExt);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;  
}

CCSImageConverterCache* CCSImageConverterCache::NewL(MCSImageConverterCacheObserver& aRequester,
                                                     const TDesC& aImageExt) 
{
   CCSImageConverterCache* self = CCSImageConverterCache::NewLC(aRequester, aImageExt);
   CleanupStack::Pop(self);
   return self;
}

CCSImageConverterCache::~CCSImageConverterCache() 
{
   for (cacheMapIt_t it = iCacheMap.begin(); it != iCacheMap.end(); ++it) {
      delete it->first;
      delete it->second;
   }
   iFs.Close();
}

void CCSImageConverterCache::ConvertImage(const TDesC& aFilename, TSize aSize) 
{
   CWsScreenDevice* screenDevice = CCoeEnv::Static()->ScreenDevice();
   iBitmapConverter->ConvertFileL(aFilename, 
                                  aSize,
                                  screenDevice->DisplayMode());
}

void CCSImageConverterCache::ImageConversionDone(TInt aResult) 
{
   if (aResult != KErrNone) {
      // Something wrong, delete from imageVec and ignore this image
      iNameVec.pop_back();
   }
   CEikImage* image = new (ELeave) CEikImage();
   image->SetBitmap(iBitmapConverter->GetImage());
   image->SetMask(iBitmapConverter->GetMask());
   TParse parser;
   parser.Set(iNameVec.back(), NULL, NULL);
   HBufC* name = parser.Name().AllocL();
   iCacheMap.insert(std::make_pair(name, image));
   iNameVec.pop_back();
   if (iNameVec.size() > 0) {
      ConvertImage(iNameVec.back(), iSize);
   } else {
      iRequester.ImagesConverted();
   }
}

void CCSImageConverterCache::ConstructL() 
{
   iFs.Connect();
   iBitmapConverter = CBitmapConverter::NewL(this, iFs);
}

CCSImageConverterCache::CCSImageConverterCache(MCSImageConverterCacheObserver& aRequester, 
                                               const TDesC& aImageExt) :
   iRequester(aRequester),
   iImageExt(aImageExt)
{ }

void CCSImageConverterCache::LocateAndConvertImages(const TDesC& aPath, TSize aSize) 
{
   iSize = aSize;
   CDir* dirList;
   iFs.GetDir(aPath, KEntryAttNormal, ESortByName, dirList);
   class TParse parser;
   if (dirList->Count() <= TInt(iCacheMap.size())) {
      // We can assume that we have converted and cached all existing
      // images. By doing this we skip the text compare below.
      iRequester.ImagesConverted();
      return;
   }
   for (TInt i = 0; i < dirList->Count(); ++i) {
      parser.Set(dirList->operator[](i).iName, &aPath, NULL);
      if (parser.Ext().CompareF(iImageExt) == 0) {
         // Found a file with the correct extension
         HBufC* tmpName = dirList->operator[](i).iName.AllocLC();
         if (iCacheMap.find(tmpName) == iCacheMap.end()) {
            iNameVec.push_back(parser.FullName());
         }
         CleanupStack::PopAndDestroy(tmpName);
      }

   }
   delete dirList;
   
   if (iNameVec.size() > 0) {
      ConvertImage(iNameVec.back(), iSize);
   } else {
      iRequester.ImagesConverted();
   }
}

const cacheMap_t CCSImageConverterCache::GetImageCache() const 
{
   return iCacheMap;
}

cacheMap_t::const_iterator CCSImageConverterCache::FindImagePair(HBufC* aFilename) const 
{
   return iCacheMap.find(aFilename);
}

CEikImage* CCSImageConverterCache::FindImage(HBufC* aFilename) 
{
   cacheMap_t::iterator it = iCacheMap.find(aFilename);
   if (it == iCacheMap.end()) {
      return NULL;
   }
   return it->second;
}
