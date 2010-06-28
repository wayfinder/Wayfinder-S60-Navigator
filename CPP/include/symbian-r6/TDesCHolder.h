/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TDESCHOLDER_H
#define TDESCHOLDER_H

#include "WFTextUtil.h"

///Abstract superclass of all TDesCHolder classes. The TDesCHolder
///classes are used to build a tree of TDesC references which can then
///be iterated through, for example when concatenating strings.
class TDesCHolder{
   friend class TDesCHolderSplit;
   ///@return the number of TDesC referenced by the TDesCHolder tree with 
   ///        this TDesCHolder as root.
   virtual TUint size() const = 0;

   ///@param index into the ordered set of TDesCs held by this tree.
   ///@return the TDesC indexed by the <code>aIndex</code> parameter. 
   ///        If <code>aIndex</code> is larger than or equal to the size 
   ///        of the TDesC set, <code>KNullDesC</code> will be returned.
   virtual const TDesC& operator[](TUint aIndex) const = 0;

public:
   ///Calculate the total length of the concatanated descriptor. 
   ///@return the total length
   virtual TUint Length() const = 0;
   
   ///Allocate a character array of sufficient length and copy the
   ///contents of the TDesCHolder complexe into it. The character
   ///array is left on the CleanupStack. 
   ///If UTF8 is activated the character array is UTF8 encoded.
   char* StrDupLC() const;

   ///Allocate a character array of sufficient length and copy the
   ///contents of the TDesCHolder complexe into it.
   ///If UTF8 is activated the character array is UTF8 encoded.
   char* StrDupL() const;

   ///Safely concatenates the contents of a set of descriptors and
   ///copies the rsults into another descriptor.
   ///Beware! If the total contents of the source descriptor set is
   ///longer than the maximum length of the destination descriptor,
   ///the contents will be truncated to fit.
   ///The recommended usage is: 
   ///<code>TDesCopy(dst, _L("one ") + _L("two ") + _L("three"));</code>
   ///This way the TDesCHolder tree is built automatically by the
   ///<code>operator+</code> functions.
   ///@param dst the destination descriptor. If it is not large enough
   ///           to append the contents of the source set of
   ///           descriptors, the eventual contents of
   ///           <code>src</code> will be as much of the source set
   ///           contents as fit, starting at the beginning.
   ///@param src the set of source descriptors organised into a
   ///           TDesCHolder tree.
   ///@return returns EFalse if truncation occured.
   TBool TDesCopy(TDes& dst) const;

   ///Allocates a HBufC and copies the TDesCHolder complex into it. 
   HBufC* AllocLC() const;
};

///This class forms the leaves of the TDesCHolder trees. Each leaf
///references one TDesC.
class OneTDesC : public TDesCHolder {
   ///This leaf's TDesC.
   const TDesC& iDescriptor;

   ///Returns the size of this subtree, which always is 1.
   ///@return 1
   virtual TUint size() const;

   ///Returns the length of this leafs descriptor. 
   ///@return iDescriptor.Length().
   virtual TUint Length() const;

   ///@param the index of the TDesC wanted.
   ///@return the single TDesC held if aIndex is 0, otherwise KNullDesC.
   virtual const TDesC& operator[](TUint aIndex) const;

public:
   ///Constructor. This is meant to be used as a implicit conversion.
   ///@param aFirst the TDesC this object should refer to.
   OneTDesC(const TDesC& aFirst);

   ///Constructor. This is meant to be used as a implicit conversion.
   ///@param aFirst the TLitC this object should refer to.
   template<TInt const S>
   OneTDesC(const TLitC<S>& aLit) :
      iDescriptor(aLit)
   {}
};

#ifndef CHAR_BITS
# define CHAR_BITS 8
#endif

///@name TDigitCalc
///This template calculates the number of digits needed for a certain base. 
//@{

///The unspecialised template specifies one digit per bit. This is
///correct for binary and is safe for all other bases, if somewhat
///inefficient.
template<int B> 
struct TDigitCalc
{
   enum { BitsPerDigit = 1};
};

///Macro to specialize TDigitCalc
#define DIGIT_CALC(base, bits)      \
template<> struct TDigitCalc<base>{ \
   enum { BitsPerDigit = bits };     \
}

///TDigitCalc specialized for Octal.
DIGIT_CALC(8,3);   
///TDigitCalc specialized for Decimal.
DIGIT_CALC(10,3);  
///TDigitCalc specialized for Hex.
DIGIT_CALC(16,4);
//@}

///@name The TNumBase templates.
///This group of template classes lets the user define the base that
///the number will be printed in.
//@{
///The TNumBase template is the base class of the classes. It is
///templated on the integral type that shall be concatenated into the
///TDesCHolder complex and the base that it should be formatted in.
template<typename T, int B>
class TNumBase{
protected:
   enum { 
      ///The base that the integer should be formatted in.
      EBase = B
   };
   ///The integer that should be formatted. 
   const T& iNum;
   ///Constructor. 
   ///@param aNum the integer that should be formatted. 
   TNumBase(const T& aNum) : iNum(aNum) {}
public:
   ///Fetches the integer value.
   ///@return the integer value. 
   const T& operator()()
   {
      return iNum;
   }
   ///Fetches the integer value.
   ///@return the integer value. 
   T operator()() const
   {
      return iNum;
   }
};

///Macro that defines classes derived from TNumBase
#define NUMBASECLASS(Name__, Radix__)           \
template<typename T>                            \
class Name__ : public TNumBase<T,Radix__> {     \
public:                                         \
   Name__(const T& aNum) : TNumBase<T,Radix__>(aNum) {}    \
}

///Binary formatting.
NUMBASECLASS(TBin, EBinary);
///Octal formatting. 
NUMBASECLASS(TOct, EOctal);
///Hexadecimal formatting.
NUMBASECLASS(THex, EHex);

//@}

///The TIntTBuf class can be a leaf in the TDesCHolder complex.  It is
///templated on the integral type and the base the value should be
///formatted in. If the class is constructed as a convertion from an
///integral type the base will be set to decimal. Use TBin, TOct, or
///THex to selext another formatting.
template<typename T, int BASE = 10>
class TIntTBuf: public OneTDesC, protected TDigitCalc<BASE>{
   ///Buffer that will hold the formatted value. 
   TBuf<(sizeof(T)*CHAR_BITS)/(1*TDigitCalc<BASE>::BitsPerDigit)> iBuf;
public:
   ///Constructor. 
   ///Sets base to decimal. 
   ///@param aNum the value to format.
   TIntTBuf(const T& aNum) : 
      OneTDesC(iBuf)
   {
      iBuf.Num(aNum, EDecimal);
   }

   ///Constructor. 
   ///The base is set from the TNumBase wrapper.
   ///@param aNum the value to format.
   TIntTBuf(const TNumBase<T,BASE>& aNum) : 
      OneTDesC(iBuf)
   {
      iBuf.Num(aNum(), TNumBase<T,BASE>::EBase);
   }
};

///This class references two TDesCHolder subtrees.
class TDesCHolderSplit : public TDesCHolder {
   /// The left subtree.
   const class TDesCHolder& iLeft;
   /// The right subtree.
   const class TDesCHolder& iRight;

   ///@return the sum of the sizes of the two subtrees.
   virtual TUint size() const;

   ///Calculates the total length of this subtree. 
   ///@return the sum of the length of the left and right subtrees. 
   virtual TUint Length() const;

   ///@param index into the ordered set of TDesCs held by this tree.
   ///@return the TDesC indexed by the <code>aIndex</code> parameter. 
   ///        If <code>aIndex</code> is larger than or equal to the size 
   ///        of the TDesC set, <code>KNullDesC</code> will be returned.
   virtual const TDesC& operator[](TUint aIndex) const;

   ///Constructor 
   ///@param aLeft what shall be the left TDesCHolder subtree.
   ///@param aRight what shall be the right TDesCHolder subtree.
   TDesCHolderSplit(const class TDesCHolder& aLeft, 
                    const class TDesCHolder& aRight);

public:
   ///Creates a TDesCHolder tree with the parameter trees as subtrees.
   ///@param aLhs what will be the left subtree.
   ///@param aRhs what will be the right subtree.
   ///@return a combined TDesCHolder tree.
   friend class TDesCHolderSplit operator+(const class TDesCHolder& aLhs, 
                                           const class TDesCHolder& aRhs);

   ///Creates a TDesCHolder tree with the parameter trees as subtrees.
   ///@param aLhs what will be the left subtree, containing only a leaf.
   ///@param aRhs what will be the right subtree.
   ///@return a combined TDesCHolder tree.
   friend class TDesCHolderSplit operator+(const class TDesCHolder& aLhs,
                                           const class OneTDesC& aRhs);

   ///Creates a TDesCHolder tree with the parameter trees as subtrees.
   ///@param aLhs what will be the left subtree.
   ///@param aRhs what will be the right subtree, containing only a leaf.
   ///@return a combined TDesCHolder tree.
   friend class TDesCHolderSplit operator+(const class OneTDesC& aLhs,
                                           const class TDesCHolder& aRhs);

   ///Creates a TDesCHolder tree with the parameters as leaves.
   ///@param aLhs what will be the left leaf.
   ///@param aRhs what will be the right leaf.
   ///@return a combined TDesCHolder tree.
   friend class TDesCHolderSplit operator+(const class OneTDesC& aLhs, 
                                           const class OneTDesC& aRhs);
};

template<typename T>
inline TBuf<32> ToBuf(T aNum, enum TRadix aRadix = EDecimal)
{
   TBuf<32> buf;
   buf.Num(aNum, aRadix);
   return buf;
}

///Calls TDesCHolder::AllocLC and returns the result. 
HBufC* AllocLC(class TDesCHolder const& aCompound);

#endif
