/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TDESCREDIRECT_H
#define TDESCREDIRECT_H
#include <e32def.h>
#include <e32std.h>

/**
 * This class is a redirect class that can be used instead of <code>
 * const TDesC& </code> as a function argument. It is constructable
 * from <code> const TDesC16&<code>, <code>const TDesC16*</code>, or
 * <code>const TLitC16</code>. Since the constructors are not
 * explicit, you can use either of the three types as arguments when
 * calling the function. If the <code>const TDesC16*</code> is NULL,
 * the TDesC16Redirect object will refer to <code>KNullDesC16</code>
 * instead.
 *
 * The class has a conversion operator for <code>const TDesC16</code>
 * for automatic conversion and a <code>operator()</code> for explicit
 * conversion to <code>const TDesC16&</code>
 *
 * Example:
 * <code>
 *  TBool IsLongerThanFive(const TDesC16& aArg)
 *  {
 *     return aArg.Length() > 5;
 *  }
 * 
 *  TBool MyFunction(const TDesC16Redirect& aFirst, 
 *                   const TDesC16Redirect& aSecond,
 *                   const TDesC16Redirect& aThird)
 *  {
 *     return aFirst() == aSecond() && IsLongerThanFive(aThird);
 *  }
 *
 *  void TestL(const TDesC16& aArg)
 *  {
 *     _LIT16(KTestLit, "TestLit");
 *     HBufC16* testBuf = KTestLit().AllocLC();
 *     HBufC16* nullBuf = NULL;
 *     TBool test = MyFunction(aArg, KTestLit, testBuf) ||
 *                  MyFunction(KTestLit, aArg, testBuf) ||
 *                  MyFunction(nullBuf, nullBuf, KTestLit);
 *  }
 * </code>
 *
 * The exmple shows that <code>MyFunction</code> can be called with
 * any combination of <code>TDesC16&, TLitC16</code>, and
 * <code>TDesC16*</code> arguments. If the argument is a pointer, it
 * may be NULL. The <code>TDesC16Redirect</code> class can be
 * converted to <code>const TDesC16&</code> by using the
 * <code>operator()</code> and then tested for equality. It can also
 * be automatically converted to a <code>const TDesC&</code> as shown
 * in the function call to <code>IsLongerThanFive</code>.
 */
class TDesC16Redirect 
{
   //This object is not assignable.
   const class TDesC16Redirect& operator=(const class TDesC16Redirect&);
   /** The reference to the real descriptor. */
   const class TDesC16& iRef;
public:
   /** Constructs from descriptor reference. */
   inline TDesC16Redirect(const class TDesC16& aRef);
   /** 
    * Constructs from descriptor pointer. 
    * @param aRef Descriptor pointer. If the pointer is NULL the
    *             object will refer to KNullDesC16.
    */
   inline TDesC16Redirect(const class TDesC16* aRef);
   /** Constructs from literal descriptor. */
   template<TInt S>
   inline TDesC16Redirect(const TLitC16<S>& aRef);
   /** Auto convert to descriptor reference. */
   inline operator const class TDesC16&() const;
   /** Explicit convert to descriptor reference. */
   inline const class TDesC16& operator()() const;
};

/**
 * This class is a redirect class that can be used instead of <code>
 * const TDesC& </code> as a function argument. It is constructable
 * from <code> const TDesC8&<code>, <code>const TDesC8*</code>, or
 * <code>const TLitC8</code>. Since the constructors are not
 * explicit, you can use either of the three types as arguments when
 * calling the function. If the <code>const TDesC8*</code> is NULL,
 * the TDesC8Redirect object will refer to <code>KNullDesC8</code>
 * instead.
 *
 * The class has a conversion operator for <code>const TDesC8</code>
 * for automatic conversion and a <code>operator()</code> for explicit
 * conversion to <code>const TDesC8&</code>
 *
 * See the example i <code>TDesC16Redirect</code>.
 */
class TDesC8Redirect 
{
   const class TDesC8Redirect& operator=(const class TDesC8Redirect&);
   const class TDesC8& iRef;
public:
   /** Constructs from descriptor reference. */
   inline TDesC8Redirect(const class TDesC8& aRef);
   /** 
    * Constructs from descriptor pointer. 
    * @param aRef Descriptor pointer. If the pointer is NULL the
    *             object will refer to KNullDesC16.
    */
   inline TDesC8Redirect(const class TDesC8* aRef);
   /** Constructs from literal descriptor. */
   template<TInt S>
   inline TDesC8Redirect(const TLitC8<S>& aRef);
   /** Auto convert to descriptor reference. */
   inline operator const class TDesC8&() const;
   /** Explicit convert to descriptor reference. */
   inline const class TDesC8& operator()() const;
};

#ifdef _UNICODE
typedef TDesC16Redirect TDesCRedirect;
#else
typedef TDesC8Redirect TDesCRedirect;
#endif

inline TDesC16Redirect::operator const TDesC16&() const
{
   return iRef;
}

inline TDesC16Redirect::TDesC16Redirect(const TDesC16& aRef) :
   iRef(aRef)
{
}

inline TDesC16Redirect::TDesC16Redirect(const TDesC16* aRef) :
   iRef(aRef ? *aRef : KNullDesC16)
{
}

template<TInt S>
inline TDesC16Redirect::TDesC16Redirect(const TLitC16<S>& aRef) :
   iRef(aRef())
{
}

inline const TDesC16& TDesC16Redirect::operator()() const
{
   return iRef;
}


inline TDesC8Redirect::operator const TDesC8&() const
{
   return iRef;
}

inline TDesC8Redirect::TDesC8Redirect(const TDesC8& aRef) :
   iRef(aRef)
{
}

inline TDesC8Redirect::TDesC8Redirect(const TDesC8* aRef) :
   iRef(aRef ? *aRef : KNullDesC8)
{
}

template<TInt S>
inline TDesC8Redirect::TDesC8Redirect(const TLitC8<S>& aRef) : 
   iRef(aRef())
{
}

inline const TDesC8& TDesC8Redirect::operator()() const
{
   return iRef;
}

#endif
