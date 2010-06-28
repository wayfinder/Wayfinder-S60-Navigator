/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYMBIAN_DATATYPES_H
#define SYMBIAN_DATATYPES_H

/* Symbian-specific data types */

#ifdef __SYMBIAN32__

#include <es_sock.h>

//integer bit width and sign type definitions.
typedef TInt8            int8;
typedef TUint8           uint8;
typedef TInt16           int16;
typedef TUint16          uint16;
typedef TInt32           int32;
typedef TUint32          uint32;
// TInt64 is implemented as a C++ class.
//typedef TInt64          int64;

// floating point types
typedef TReal32  float32;
typedef TReal64  float64;

// Macros used to create the operators for int64

#define MAKE_INT64_OPERATOR_REF(x) \
    inline const int64& x (const int64& other) { \
       TInt64::x(other); \
       return *this; \
    }

#define MAKE_INT64_OPERATOR(x) \
    inline int64 x ( const int64& other ) const {\
       return TInt64::x(other);\
    }

#define MAKE_INT64_BOOL_OPERATOR(x) \
    inline int x ( const int64& other ) const {\
       return TInt64::x(other);\
    }

#define MAKE_INT64_OPERATOR_SHIFT_LIKE_REF(x) \
    inline const int64& x (int amount) {\
       TInt64::x(amount); \
       return *this; \
    }

#define MAKE_INT64_OPERATOR_SHIFT_LIKE(x) \
    inline int64 x ( int amount ) const {\
       return TInt64::x(amount); \
    }

/**
 *   Subclass introduced to get rid of the strange problems
 *   with conversion to/from the typedeffed classes.
 *   All Symbian-specific methods have been removed to make 
 *   it behave more like a real int64.
 */
class int64 : public TInt64 {
public:
     inline int64() {}
     inline int64(const TInt64& i64)  : TInt64(i64.High(), i64.Low()) {}
     inline int64(TInt32 i)           : TInt64(TInt(i)) {}
     inline int64(TUint32 u)          : TInt64(TUint(u)) {}
     inline int64(TInt i)             : TInt64(i) {}
     inline int64(TUint u)            : TInt64(u) {}
     inline int64(const float&  aVal) : TInt64(aVal) {}
     inline int64(const double& aVal) : TInt64(aVal) {}
  protected:
     /// Should not be used outside this class since it is non-standard
     inline int64(TUint aHigh,TUint aLow) : TInt64(aHigh, aLow) {}
     /// Non-standard function.
     void Set(TUint aHigh,TUint aLow) { TInt64::Set(aHigh, aLow); }
     /// Non-standard function.
     TUint Low() const { return TInt64::Low(); }
     /// Non-standard function.
     TUint High() const { return TInt64::High(); }
     /// Non-standard function.
     TReal GetTReal() const { return TInt64::GetTReal(); }
     /// Non-standard function. 
     TInt GetTInt() const { return TInt64::GetTInt(); }
  public:
     /// Strange. This was not implemented in Symbian class.
     inline int64 operator&(const int64& other) {
        return int64(High() & other.High(), Low() & other.Low() );
     }
     /// Strange. This was not implemented in Symbian class.
     inline const int64& operator&=(const int64& other) {
        Set(High() & other.High(), Low() & other.Low() );
        return *this;
     }
     ///  Strange. This was not implemented in Symbian class.
     inline int64 operator|(const int64& other) {
        return int64(High() | other.High(), Low() | other.Low() );        
     }
     /// Strange. This was not implemented in Symbian class.
     inline const int64& operator|=(const int64& other) {
        Set(High() | other.High(), Low() | other.Low() );
        return *this;
     }

     // Make some operators return the right type.
     MAKE_INT64_OPERATOR(operator+);
     MAKE_INT64_OPERATOR(operator-);
     MAKE_INT64_OPERATOR(operator*);
     MAKE_INT64_OPERATOR(operator/);
     MAKE_INT64_OPERATOR(operator%);
     
     MAKE_INT64_OPERATOR_REF(operator+=);
     MAKE_INT64_OPERATOR_REF(operator-=);
     MAKE_INT64_OPERATOR_REF(operator*=);
     MAKE_INT64_OPERATOR_REF(operator/=);
     MAKE_INT64_OPERATOR_REF(operator%=);
     
     MAKE_INT64_OPERATOR_SHIFT_LIKE(operator<<);
     MAKE_INT64_OPERATOR_SHIFT_LIKE(operator>>);
     MAKE_INT64_OPERATOR_SHIFT_LIKE_REF(operator<<=);
     MAKE_INT64_OPERATOR_SHIFT_LIKE_REF(operator>>=);
     
     /// Operator needed for MC2BoundingBox.
     inline friend double operator*(const double& a, const int64& b) {
        return a*double(b);
     }
     
     // Conversion operator to float64 / double
     inline operator double() const { return GetTReal(); }
     // Conversion operator to float32 / float
     inline operator float() const { return float(GetTReal()); }
     // Conversion operator to uint32.
     inline operator uint32() const { return Low(); }
     // Conversion operator to int32. 
     inline operator int32() const { return GetTInt(); }

protected:     
     /// Non-standard function. Not implemented.
     void Lsr(TInt aShift);
     /// Non-standard function. Not implemented.
     void Mul10();
     /// Non-standard function. Not implemented.
     TInt MulTop(const TInt64& aVal);
     /// Non-standard function. Not implemented.
     void DivMod(const TInt64& aVal,TInt64& aRemainder);
     
     // Makes an int64 from two uint32:s.
     friend int64 MakeInt64(uint32 high, uint32 low) {
        return int64(high,low);
     }
     /// Returns the low part of the uint64
     friend inline TUint LOW(const int64& x) { return x.Low(); }
     /// Returns the high 32 bits of the uint64
     friend inline TUint HIGH(const int64& x) { return x.High(); }
     /// Sets the value of a uin32
     friend inline void SET(int64& x,TUint aHigh, TUint aLow) {
        x.Set(aHigh, aLow);
     }
};

// macros for proper printf formats
// could already be defined in inttypes.h from C99
#ifndef PRId8
# define PRId8 "hhd"
# define PRIi8 "hhi"
# define PRIo8 "hho"
# define PRIu8 "hhu"
# define PRIx8 "hhx"
# define PRIX8 "hhX"
#endif
#ifndef PRId16
# define PRId16 "hd"
# define PRIi16 "hi"
# define PRIo16 "ho"
# define PRIu16 "hu"
# define PRIx16 "hx"
# define PRIX16 "hX"
#endif
#ifndef PRId32
# define PRId32 "ld"
# define PRIi32 "li"
# define PRIo32 "lo"
# define PRIu32 "lu"
# define PRIx32 "lx"
# define PRIX32 "lX"
#endif

// For scanf
#ifndef SCNi32
// int32 is defined as long int in Symbian.
#define SCNi32 "li"
#endif


/* Minimum and Maximum extents for all types */
#define MAX_UINT8    KMaxTUint8
#define MAX_INT8     KMaxTInt8
#define MAX_UINT16   KMaxTUint16
#define MAX_INT16    KMaxTInt16
#define MAX_UINT32   KMaxTUint32
#define MAX_INT32    KMaxTInt32
#define MAX_FLOAT32  KMaxTReal32
#define MAX_FLOAT64  KMaxTReal64
// FIXME: Something more efficient here, please.
#define MIN_INT8     KMinTInt8
#define MIN_INT16    KMinTInt16
#define MIN_INT32    KMinTInt32
#define MIN_FLOAT32	KMinTReal32
#define MIN_FLOAT64	KMinTReal64
// 64 bit Not available
#define MAX_UINT64 int64(TInt64(-1, -1))
#define MAX_INT64  int64(TInt64(MAX_INT32, MAX_UINT32))
#define MIN_INT64  int64(TInt64(MIN_INT32, 0))

// Convenience types
typedef uint8 byte;
#define MAX_BYTE   MAX_UINT8

#else

/* 
   If this file is included for some other platform,
   stop the compilation!
*/
#error "SymbianDataTypes.h is only for the Symbian Platform!"

#endif

#endif

