/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "TDesCHolder.h"

char* TDesCHolder::StrDupLC() const
{
   const TUint length = Length();
   const TUint size = this->size();
   
   char* const dst = new (ELeave) char[length + 1];
   CleanupStack::PushL(dst);
   
   char* pos = dst;
   for(TUint i = 0; i < size; ++i){
      TPtr8 Pos(reinterpret_cast<TUint8*>(pos), 0, length - (pos - dst));
      const TDesC& src = (*this)[i];
      WFTextUtil::TDesCopy(Pos, src);
      pos += src.Length();
   }
   *pos++ = '\0';

   return dst;
}

char* TDesCHolder::StrDupL() const
{
   char* dst = StrDupLC();
   CleanupStack::Pop(dst);
   return dst;
}

TBool TDesCHolder::TDesCopy(TDes& dst) const
{
   const class TDesCHolder& src = *this;
   TBool ret = WFTextUtil::TDesCopy(dst, src[0]);
   for(TUint i = 1; ret && i < src.size(); ++i){
      ret = WFTextUtil::TDesAppend(dst, src[i]);
   }
   return ret;
}

HBufC* TDesCHolder::AllocLC() const
{
   HBufC* ret = HBufC::NewLC(Length());
   TPtr ptr = ret->Des();
   const TUint size = this->size();
   for(TUint i = 0; i < size; ++i){
      ptr.Append((*this)[i]);
   }
   return ret;
}


TUint OneTDesC::size() const
{
   return 1;
}

TUint OneTDesC::Length() const
{
   return iDescriptor.Length();
}

const TDesC& OneTDesC::operator[](TUint aIndex) const
{
   return aIndex == 0 ? iDescriptor : KNullDesC;
}

OneTDesC::OneTDesC(const TDesC& aFirst) :
   iDescriptor(aFirst)
{}   


TUint TDesCHolderSplit::size() const
{ 
   return iLeft.size() + iRight.size(); 
}

TUint TDesCHolderSplit::Length() const
{
   return iLeft.Length() + iRight.Length();
}

const TDesC& TDesCHolderSplit::operator[](TUint aIndex) const
{
   return (aIndex < iLeft.size()) ?  
      iLeft[aIndex] : iRight[aIndex - iLeft.size()];
}

TDesCHolderSplit::TDesCHolderSplit(const class TDesCHolder& aLeft, 
                                   const class TDesCHolder& aRight) :
   iLeft(aLeft), iRight(aRight)
{}

class TDesCHolderSplit operator+(const class TDesCHolder& aLhs, 
                                 const class TDesCHolder& aRhs)
{
   return TDesCHolderSplit(aLhs, aRhs);
}

class TDesCHolderSplit operator+(const class TDesCHolder& aLhs,
                                 const class OneTDesC& aRhs)
{
   return TDesCHolderSplit(aLhs, static_cast<const TDesCHolder&>(aRhs));
}

class TDesCHolderSplit operator+(const class OneTDesC& aLhs,
                                 const class TDesCHolder& aRhs)
{
   return TDesCHolderSplit(aLhs, aRhs);
}

class TDesCHolderSplit operator+(const class OneTDesC& aLhs, 
                                 const class OneTDesC& aRhs)
{
   return TDesCHolderSplit(aLhs, aRhs);
}


///Calls TDesCHolder::AllocLC and returns the result. 
HBufC* AllocLC(class TDesCHolder const& aCompound)
{
   return aCompound.AllocLC();
}



