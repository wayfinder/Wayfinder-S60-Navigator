/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "WFArrayUtil.h"
#include "WFTextUtil.h"

WFArrayUtil::TCharChar::TCharChar(TInt aCount) : 
   iArray(NULL), iCount(aCount)
{}

void WFArrayUtil::CleanupCharCharPtr(TAny* aTCharChar)
{
   class TCharChar* tCharChar = static_cast<TCharChar*>(aTCharChar);
   if(tCharChar->iArray){
      for(TInt i = 0; i < tCharChar->iCount; ++i){
         delete[] tCharChar->iArray[i];
         tCharChar->iArray[i] = NULL;
      }
      delete[] tCharChar->iArray;
      tCharChar->iArray = NULL;
   }
   delete tCharChar;
}

char*** WFArrayUtil::AllocCharCharLC(TInt aSize)
{
   class TCharChar* charchar = new (ELeave) TCharChar(aSize);
   CleanupStack::PushL(TCleanupItem(CleanupCharCharPtr, charchar));
   charchar->iArray = new (ELeave) char*[aSize];
   for(TInt i = 0; i < aSize; ++i){
      charchar->iArray[i] = NULL;
   }
   return &(charchar->iArray);
}

char*** WFArrayUtil::AllocCharCharLC(const class MDesC8Array& aArray)
{
   char*** array = AllocCharCharLC(aArray.MdcaCount());
   for(TInt i = 0; i < aArray.MdcaCount(); ++i){
      (*array)[i] = WFTextUtil::newTDesDupL(aArray.MdcaPoint(i));
   }
   return array;
}

char** WFArrayUtil::AllocCharCharL(const class MDesC8Array& aArray)
{
   char*** array = AllocCharCharLC(aArray.MdcaCount());
   for(TInt i = 0; i < aArray.MdcaCount(); ++i){
      (*array)[i] = WFTextUtil::newTDesDupL(aArray.MdcaPoint(i));
   }
   return CleanupPopCharChar(array);
}

char** WFArrayUtil::CleanupPopCharChar(char*** aPtr)
{
   char** retptr = *aPtr;
   void* vp = aPtr;
   class TCharChar* obj = static_cast<TCharChar*>(vp);
   CleanupStack::Pop(obj);
   delete obj;
   return retptr;
}


class CDesC8Array& WFArrayUtil::CopyArrayL(class CDesC8Array& aDst, 
                                           const char*const* aSrc,
                                           TInt aSrcSize)
{
   const TInt minSize = Min(aDst.Count(), aSrcSize);
   for(TInt i = 0; i < minSize; ++i){
      const TText8* tt = reinterpret_cast<const TText8*>(aSrc[i]);
      const TPtrC8 ptr(tt ? tt : _S8(""));
      if(aDst[i] != ptr){
         aDst.InsertL(i, ptr);
         aDst.Delete(i + 1);
      }
   }
   for(TInt j = minSize; j < aSrcSize; ++j){
      const TText8* tt = reinterpret_cast<const TText8*>(aSrc[j]);
      const TPtrC8 ptr(tt ? tt : _S8(""));
      aDst.AppendL(ptr);
   }
   if(aSrcSize < aDst.Count()){
      aDst.Delete(aSrcSize, aDst.Count() - aSrcSize);
      aDst.Compress();
   }
   return aDst;
}


class CDesC8Array* WFArrayUtil::AllocArrayLC(const char*const* aSrc,
                                             TInt aSrcSize)
{
   class CDesC8Array* array = new (ELeave) CDesC8ArraySeg(aSrcSize);
   CleanupStack::PushL(array);
   CopyArrayL(*array, aSrc, aSrcSize);
   return array;
}

class CDesC8Array* WFArrayUtil::AllocArrayL(const char*const* aSrc,
                                             TInt aSrcSize)
{
   class CDesC8Array* self = AllocArrayLC(aSrc, aSrcSize);
   CleanupStack::Pop(self);
   return self;
}

TBool WFArrayUtil::ArrayEqual(const char* const* aLhs, const TInt aSize, 
                              const class MDesC8Array& aRhs)
{
   TBool equal = ((aSize == aRhs.MdcaCount()) && 
                  ArrayNEqual(aSize, aLhs, aRhs));
   return equal;
}

TBool WFArrayUtil::ArrayNEqual(const TInt aN,
                               const char* const* aLhs,
                               const class MDesC8Array& aRhs)
{
   TBool equal = (aRhs.MdcaCount() >= aN);
   for(TInt i = 0; equal && i < aN; ++i){
      if(aLhs[i]){
         TPtrC8 ptr(reinterpret_cast<const TText8*>(aLhs[i]));
         equal = (0 == ptr.Compare(aRhs.MdcaPoint(i)));
      } else {
         equal = (aRhs.MdcaPoint(i).Length() == 0);
      }
   }
   return equal;
}

namespace {
   class TUnlimitedDesCArray : public MDesCArray
   {
      MDesCArray const *const iArray;
   public:
      TUnlimitedDesCArray(const MDesCArray* aArray) : 
         iArray(aArray)
      {}
      
      TUnlimitedDesCArray(const MDesCArray& aArray) : 
         iArray(&aArray)
      {}
      
      TPtrC operator[](TInt i) const
      {
         TPtrC retval = KNullDesC();
         if(i >= 0 && i < iArray->MdcaCount()){
            retval.Set(iArray->MdcaPoint(i));
         }
         return retval;
      }
      
      virtual TInt MdcaCount() const
      {
         return iArray->MdcaCount();
      }
      
      virtual TPtrC MdcaPoint(TInt aIndex) const
      {
         return operator[](aIndex);
      };
   };

   /**
    */
   static CDesCArray* 
   RealPasteArrayLC(const TDesC& aLeftDelim,
                    const class TUnlimitedDesCArray& aLeft,
                    const TDesC& aDelim, 
                    const class TUnlimitedDesCArray& aRight,
                    const TDesC& aRightDelim)
   {
      _LIT(KPasteFormat, "%S%S%S%S%S");
      
      //the total length of the delimiters
      const TInt delimLength = (aLeftDelim.Length() + aDelim.Length() + 
                                aRightDelim.Length());
      //the length of the longest array.
      const TInt longerLen = Max(aLeft.MdcaCount(), aRight.MdcaCount());

      //declare the new array.
      CDesCArray* result = new (ELeave) CDesCArrayFlat(longerLen);
      CleanupStack::PushL(result);

      //the length of the merge buffer is initially set to twice the
      //length of the result of a merge of the first elements of the
      //arrays.
      TInt buflen = (aLeft[0].Length() + aRight[0].Length() + delimLength);
      buflen = Min(KMaxTInt, buflen * 2);
      HBufC* buf = HBufC::NewLC(buflen);
   
      for(TInt i = 0; i < longerLen; ++i){
         TPtrC left  = aLeft[i];
         TPtrC right = aRight[i];

         //check if the merge buffer is long enough.
         buflen = Min(KMaxTInt, 
                      (left.Length() + right.Length() + delimLength) * 2);
         if(buflen > buf->Des().MaxLength()){ 
            //buffer not long enough, realloc.
            CleanupStack::PopAndDestroy(buf);
            buf = NULL;
            buf = HBufC::NewLC(buflen);
         }
         //merge the strings
         buf->Des().Format(KPasteFormat, &aLeftDelim, &left, 
                           &aDelim, &right, &aRightDelim);
         //append to new aray.
         result->AppendL(*buf);
      }
      CleanupStack::PopAndDestroy(buf);
      return result;
   }
} //anon namspace

CDesCArray* WFArrayUtil::PasteArrayLC(const TDesC& aLeftDelim,
                                      const MDesCArray* aLeft,
                                      const TDesC& aMiddleDelim,
                                      const MDesCArray* aRight,
                                      const TDesC& aRightDelim)
{
   return RealPasteArrayLC(aLeftDelim, aLeft, aMiddleDelim, 
                           aRight, aRightDelim);
}

CDesCArray* WFArrayUtil::PasteArrayLC(const MDesCArray* aLeft,
                                      const TDesC& aMiddleDelim,
                                      const MDesCArray* aRight)
{
   return PasteArrayLC(KNullDesC, aLeft, aMiddleDelim, aRight, KNullDesC);
}
