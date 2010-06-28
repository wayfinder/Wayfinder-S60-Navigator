/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * machine.h
 *
 * Define global types for the Symbian systems.
 *
 *
 */

#ifndef SYMBIAN32_MACHINE_H
#define SYMBIAN32_MACHINE_H

#include <e32def.h>
#include <es_sock.h>
#include <stdio.h>
#include "../factored/systemindependenttypes.h"

//integer bit width and sign type definitions.
typedef TInt8            int8;
typedef TUint8           uint8;
typedef TInt16           int16;
typedef TUint16          uint16;
typedef TInt32           int32;
typedef TUint32          uint32;
// floating point types
typedef TReal32  float32;
typedef TReal64  float64;

#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
typedef TInt64          int64;

inline int64 MakeInt64(uint32 high, uint32 low) 
{
   TUint64 ret = high;
   ret <<= 32;
   ret |= low;
   return int64(ret);
}

inline uint32 HIGH(int64 x) 
{
   return x >> 32;
}

inline uint32 LOW(int64 x) 
{
   return x & 0xffffffff;   
}

inline void SET(int64& x, uint32 high, uint32 low ) 
{  
   x = MakeInt64(high, low);
}

#else
// TInt64 is implemented as a C++ class.

// Macros used to create the operators for int64
// Templates would be nice, but VC++

#define MAKE_INT64_OPERATOR_REF(x) \
    inline const int64& x (const int64& other) { \
       TInt64::x(other); \
       return *this; \
    } \
    inline const int64& x (const char& other) { \
       TInt64::x(TInt(other)); \
       return *this; \
    } \
    inline const int64& x (const unsigned char& other) { \
       TInt64::x(TUint(other)); \
       return *this; \
    } \
    inline const int64& x (const int& other) { \
       TInt64::x(TInt(other)); \
       return *this; \
    } \
    inline const int64& x (const unsigned int& other) { \
       TInt64::x(TUint(other)); \
       return *this; \
    } \
    inline const int64& x (const long& other) { \
       TInt64::x(TInt(other)); \
       return *this; \
    } \
    inline const int64& x (const unsigned long& other) { \
       TInt64::x(TUint(other)); \
       return *this; \
    }
    

#define MAKE_INT64_OPERATOR(x) \
    inline int64 x ( const int64& other ) const {\
       return TInt64::x(other);\
    } \
    inline int64 x ( const char& other ) const {\
       return TInt64::x(int64(other));\
    } \
    inline int64 x ( const unsigned char& other ) const {\
       return TInt64::x(int64(other));\
    } \
    inline int64 x ( const int& other ) const {\
       return TInt64::x(int64(other));\
    } \
    inline int64 x ( const unsigned int& other ) const {\
       return TInt64::x(int64(other));\
    } \
    inline int64 x ( const long& other ) const {\
       return TInt64::x(int64(other));\
    } \
    inline int64 x ( const unsigned long& other ) const {\
       return TInt64::x(int64(other));\
    } \
    friend inline int64 x ( const char& other, const int64& b )  {\
       return int64(other).x(b); \
    } \
    friend inline int64 x ( const unsigned char& other, const int64& b  )  {\
       return int64(other).x(b); \
    } \
    friend inline int64 x ( const int& other, const int64& b ) {\
       return int64(other).x(b); \
    } \
    friend inline int64 x ( const unsigned int& other, const int64& b )  {\
       return int64(other).x(b); \
    } \
    friend inline int64 x ( const long& other, const int64& b )  {\
       return int64(other).x(b); \
    } \
    friend inline int64 x ( const unsigned long& other, const int64& b )  {\
       return int64(other).x(b); \
    } 


#define MAKE_INT64_OR_AND_OPERATOR(x,y) \
    inline int64 x ( const char& other ) const {\
       return y(other); \
    } \
    inline int64 x ( const unsigned char& other ) const {\
       return y(other); \
    } \
    inline int64 x ( const int& other ) const {\
       return y(other); \
    } \
    inline int64 x ( const unsigned int& other ) const {\
       return y(other); \
    } \
    inline int64 x ( const long& other ) const {\
       return y(other); \
    } \
    inline int64 x ( const unsigned long& other ) const {\
       return y(other); \
    } 

#define MAKE_INT64_OR_AND_OPERATOR_REF(x) \
    inline const int64& x (const char& other) { \
       return x(int64(other)); \
    } \
    inline const int64& x (const unsigned char& other) { \
       return x(int64(other)); \
    } \
    inline const int64& x (const int& other) { \
       return x(int64(other)); \
    } \
    inline const int64& x (const unsigned int& other) { \
       return x(int64(other)); \
    } \
    inline const int64& x (const long& other) { \
       return x(int64(other)); \
    } \
    inline const int64& x (const unsigned long& other) { \
       return x(int64(other)); \
    }

#define MAKE_INT64_BOOL_OPERATOR(x) \
    inline int x ( const int64& other ) const {\
       return TInt64::x(other);\
    } \
    inline int x ( const char& other ) const { \
       return TInt64::x(TInt(other));   \
    } \
    inline int x ( const unsigned char& other ) const { \
       return TInt64::x(TUint(other));           \
    } \
    inline int x ( const int& other ) const { \
       return TInt64::x(TInt(other));   \
    } \
    inline int x ( const unsigned int& other ) const { \
       return TInt64::x(TUint(other));           \
    } \
    inline int x ( const long& other ) const { \
       return TInt64::x(TInt(other));   \
    } \
    inline int x ( const unsigned long& other ) const { \
       return TInt64::x(TUint(other));           \
    } \
    friend inline int x ( const char& other, const int64& b )  {\
       return int64(other).x(b); \
    } \
   friend inline int x ( const int& other, const int64& b ) {\
       return int64(other).x(b); \
    } \
    friend inline int x ( const unsigned int& other, const int64& b )  {\
       return int64(other).x(b); \
    } \
    friend inline int x ( const long& other, const int64& b )  {\
       return int64(other).x(b); \
    } \
    friend inline int x ( const unsigned long& other, const int64& b )  {\
       return int64(other).x(b); \
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
     inline int64(const TInt64& i64)         : TInt64(i64.High(), i64.Low()) {}
     inline int64(const float&  aVal)        : TInt64(aVal) {}
     inline int64(const double& aVal)        : TInt64(aVal) {}
     inline int64(const long& i)             : TInt64(TInt(i)) {}
     inline int64(const unsigned long& u)    : TInt64(TUint(u)) {}
     inline int64(const int& i)              : TInt64(TInt(i)) {}
     inline int64(const unsigned int& u)     : TInt64(TUint(u)) {}
     inline int64(const short& i)            : TInt64(TInt(i)) {}
     inline int64(const unsigned short& u)   : TInt64(TUint(u)) {}
     inline int64(const char& i)             : TInt64(TInt(i)) {}
     inline int64(const unsigned char& u)    : TInt64(TUint(u)) {}
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
     // Function to call when overloading and
     inline int64 doAnd(const int64& other) const {
        return int64(High() & other.High(), Low() & other.Low() );
     }
     // Function to call when overloading or
     inline int64 doOr(const int64& other) const {
        return int64(High() | other.High(), Low() | other.Low() );        
     }

  public:
     /// Strange. This was not implemented in Symbian class.
     inline int64 operator&(const int64& other) const {
        return int64(High() & other.High(), Low() & other.Low() );
     }
     /// Strange. This was not implemented in Symbian class.
     inline const int64& operator&=(const int64& other) {
        Set(High() & other.High(), Low() & other.Low() );
        return *this;
     }
     ///  Strange. This was not implemented in Symbian class.
     inline int64 operator|(const int64& other) const {
        return int64(High() | other.High(), Low() | other.Low() );        
     }
     /// Strange. This was not implemented in Symbian class.
     inline const int64& operator|=(const int64& other) {
        Set(High() | other.High(), Low() | other.Low() );
        return *this;
     }

     inline int operator!() const {
        return *this == 0;
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

     MAKE_INT64_BOOL_OPERATOR( operator== );
     MAKE_INT64_BOOL_OPERATOR( operator!= );
     MAKE_INT64_BOOL_OPERATOR( operator> );
     MAKE_INT64_BOOL_OPERATOR( operator< );
     MAKE_INT64_BOOL_OPERATOR( operator>= );
     MAKE_INT64_BOOL_OPERATOR( operator<= );
     
     MAKE_INT64_OR_AND_OPERATOR( operator|, doOr );
     MAKE_INT64_OR_AND_OPERATOR( operator&, doAnd );
     
     MAKE_INT64_OR_AND_OPERATOR_REF( operator|= );
     MAKE_INT64_OR_AND_OPERATOR_REF( operator&= );

     
     
     /// Operator needed for MC2BoundingBox.
     inline friend double operator*(const double& a, const int64& b) {
        return a*double(b);
     }
     
     /// Needed for Tremor
     inline friend int& operator+=( int& a, const int64& b) {
        return a += b.Low();
     }
     
     /// Needed for Tremor
     inline friend int& operator-=( int& a, const int64& b) {
        return a -= b.Low();
     }

   inline int64 operator-() const {
      return 0-*this;
   }

     // Conversion operator to float64 / double
     inline operator double() const { return GetTReal(); }
     // Conversion operator to float32 / float
     inline operator float() const { return float(GetTReal()); }
     // Conversion operator to uint32.
     inline operator int() const { return Low(); }
     // Conversion operator to int32. 
     inline operator unsigned int() const { return Low(); }
     // Conversion operator to long
     inline operator long() const { return Low(); }
     // Conversion operator to unsigned long
     inline operator unsigned long() const { return Low(); }
     // Conversion operator to long
     inline operator char() const { return char(Low()); }
     // Conversion operator to unsigned long
     inline operator unsigned char() const { return (unsigned char)(Low()); }
   // Conversion operator to unsigned long
   inline operator short() const { return short(Low()); }
   // Conversion operator to unsigned long
   inline operator unsigned short() const { return (unsigned short)(Low()); }


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
     friend int64 MakeInt64(uint32 high, uint32 low);
     /// Returns the low part of the uint64
     friend inline TUint LOW(const int64& x) { return x.Low(); }
     /// Returns the high 32 bits of the uint64
     friend inline TUint HIGH(const int64& x) { return x.High(); }
     /// Sets the value of a uin32
     friend inline void SET(int64& x,TUint aHigh, TUint aLow) {
        x.Set(aHigh, aLow);
     }
};
//moved outside so that vc2003 can compile it.
inline int64 MakeInt64(uint32 high, uint32 low) {
  return int64(high,low);
}

#endif

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

/** @name Macros for printing Bluetooth addresses. */
//@{
/**
 * This macro can be included in a printf format string. It's
 * recommended that the PRIxBTEXPAND macro is used to supply the 6
 * needed arguments.
 */
#define PRIxBTADDR "%02x:%02x:%02x:%02x:%02x:%02x"
/**
 * This macro expands it's argument to a series of 6 arguments
 * <code>unsigned(bt[0]),...,unsigned(bt[5])</code> that matches the
 * format in the PRIxBTADDR macro. 
 * @param bt anything that supports the subscript operator can be used
 *           as an argument.
 */
#define PRIxBTEXPAND(bt) unsigned(bt[0]),unsigned(bt[1]),unsigned(bt[2]),unsigned(bt[3]),unsigned(bt[4]),unsigned(bt[5])
//@}

// For scanf
#ifndef SCNd8
# define SCNd8 "hhd"
# define SCNi8 "hhi"
# define SCNo8 "hho"
# define SCNu8 "hhu"
# define SCNx8 "hhx"
#endif
#ifndef SCNd16
//int16 is a short in symbian
# define SCNd16 "hd"
# define SCNi16 "hi"
# define SCNo16 "ho"
# define SCNu16 "hu"
# define SCNx16 "hx"
#endif
#ifndef SCNd32
// int32 is defined as long int in Symbian.
# define SCNd32 "ld"
# define SCNi32 "li"
# define SCNo32 "lo"
# define SCNu32 "lu"
# define SCNx32 "lx"
#endif
// SCNd64
// SCNi64
// SCNo64
// SCNu64
// SCNx64



// Convenience types
typedef uint8 byte;

//integer MAX and MINS
#define MAX_UINT8  0xff
#define MAX_INT8   int8(0x7f)
#define MIN_INT8   int8(-128)
#define MAX_BYTE   MAX_UINT8

#define MAX_UINT16 0xffff
#define MAX_INT16  int16(0x7fff)
#define MIN_INT16  int16(0x8000)

#define MAX_UINT32 0xffffffff
#define MAX_INT32  int32(0x7fffffff)
#define MIN_INT32  int32(0x80000000)
// 64 bit Not available
#define MAX_UINT64 int64(TInt64(-1, -1))
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
 #define MAX_INT64  int64(MakeInt64(MAX_INT32, MAX_UINT32))
 #define MIN_INT64  int64(MakeInt64(MIN_INT32, 0))
#else
 #define MAX_INT64  int64(TInt64(MAX_INT32, MAX_UINT32))
 #define MIN_INT64  int64(TInt64(MIN_INT32, 0))
#endif

#define MAX Max
#define MIN Min
//#define abs Abs

#define va_list __e32_va_list

//floating point max and mins
// MAX float 32 0111 1111 0111 1111 1111 1111 1111 1111 3.40282347e+38f
// Min float 32 0000 0000 0000 0000 0000 0000 0000 0001 1.40239846e-45f
// Max float 64 1.79769313486231570e+308
// Min float 64 4.94065645841246544e-324
//#define MAX_FLOAT32 FLT_MAX
//#define MIN_FLOAT32 FLT_MIN
//#define MAX_FLOAT64 DBL_MAX
//#define MIN_FLOAT64 DBL_MIN


/*** Symbian Stackinfo defines. */
#include <stdio.h>
#include <e32hal.h>

# define STACKINFO2 char fileline__[] = "[" __FILE__ ":" STR(__LINE__) "]"; \
                            Suddare xoxSudd( fileline__, sizeof( fileline__ ) );
#define STACKINFO char memstr__[32]; \
                  { TMemoryInfoV1Buf info; \
                    UserHal::MemoryInfo(info); \
                    TInt freeMemory = info().iFreeRamInBytes; \
                    sprintf(memstr__, "F=%d ", freeMemory); } STACKINFO2
# define DYNAMIC_STACKINFO char dynfileline__[128] = "[" __FILE__ ":" STR(__LINE__) "]";\
                            Suddare xoxSudd2( dynfileline__, sizeof( dynfileline__ ) );
# define UPDATE_STACKINFO snprintf(dynfileline__, sizeof(dynfileline__) - 1, "<" __FILE__":" STR(__LINE__) ">")

// #undef STACKINFO2
// #undef STACKINFO
// #undef DYNAMIC_STACKINFO
// #undef UPDATE_STACKINFO
// #define STACKINFO2
// #define STACKINFO
// #define DYNAMIC_STACKINFO
// #define UPDATE_STACKINFO



namespace isab {

      /* 
       * Support routines for converting litte- and big-endian numbers to
       * native form.
       */

      static inline uint16 conv_betoh_uint16(uint8 *ptr)
      {
         return BigEndian::Get16( ptr );
      }

      static inline void conv_htobe_uint16(uint16 value, uint8 *ptr)
      {
         BigEndian::Put16( ptr, value );
      }

      static inline int16 conv_betoh_int16(uint8 *ptr)
      {
          return BigEndian::Get16( ptr );
      }

      static inline void conv_htobe_int16(int16 value, uint8 *ptr)
      {
         BigEndian::Put16( ptr, value );
      }

      static inline int32 conv_betoh_int32(uint8 *ptr)
      {
         return BigEndian::Get32( ptr );
      }

      static inline uint32 conv_betoh_uint32(uint8 *ptr)
      {
         return BigEndian::Get32( ptr );
      }

      static inline float32 conv_betoh_float(uint8 *ptr)
      {
        float value;

        *(((uint8 *) &value)+3) = *ptr++;
        *(((uint8 *) &value)+2) = *ptr++;
        *(((uint8 *) &value)+1) = *ptr++;
        *(((uint8 *) &value)+0) = *ptr++;
        return value;
      }

      static inline void conv_htobe_float(float32 value, uint8 *ptr)
      {
        *ptr++=*(((uint8 *) &value)+3);
        *ptr++=*(((uint8 *) &value)+2);
        *ptr++=*(((uint8 *) &value)+1);
        *ptr++=*(((uint8 *) &value)+0);
      }

      static inline float64 conv_betoh_double(uint8 *ptr)
      {
        float64 value;

        *(((uint8 *) &value)+3) = *ptr++;
        *(((uint8 *) &value)+2) = *ptr++;
        *(((uint8 *) &value)+1) = *ptr++;
        *(((uint8 *) &value)+0) = *ptr++;
        return value;
      }

}

#endif
