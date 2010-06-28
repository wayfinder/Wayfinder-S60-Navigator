/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GBDEFS_H
#define GBDEFS_H

#include "machine.h"
#include <stdio.h>

#define GB_DEVEL
#define GB_LOAD_WHOLE_TEXTURE_AT_STARTUP

//--------------------------------------------------------------------------//
// Compiler and platform identification
//--------------------------------------------------------------------------//

#define GB_COMPILER_MSVC		1
#define GB_COMPILER_GCC			2

#define GB_PLATFORM_WIN32		1
#define GB_PLATFORM_SYMBIAN		2

#if defined(_MSC_VER)
#	define GB_COMPILER		GB_COMPILER_MSVC
#	ifdef _MSC_VER
#		if (_MSC_VER < 1300)
#			define GB_COMPILER_VER		6
#		else
#			define GB_COMPILER_VER		7
#		endif
#	endif
#endif

#if defined(GCC) || defined (__GNUC__)
#	define GB_COMPILER		GB_COMPILER_GCC
#endif

#if defined(SYMBIAN) || defined( __SYMBIAN__ ) || defined ( __SYMBIAN32__ )
#	define GB_PIXELFORMAT_565
#	define GB_PLATFORM		GB_PLATFORM_SYMBIAN
#	ifndef GB_COMPILER
#		define GB_COMPILER	0
#	endif
#	ifdef GB_DEVEL
#		undef GB_DEVEL
#	endif
#elif defined(WIN32) || defined(_WIN32)
#	define GB_PLATFORM		GB_PLATFORM_WIN32
#endif

#if defined _DEBUG
#	define GB_DEBUG
#endif

#ifdef __cplusplus
#	define GB_COMPILER_CPP
#endif

/* Unknown compiler version */

#ifndef GB_COMPILER_VER
#	define GB_COMPILER_VER		0
#endif

/* Errors */

#ifndef GB_COMPILER
#	error GB_COMPILER not defined.
#endif

#ifndef GB_PLATFORM
#	error GB_PLATFORM not defined.
#endif

//--------------------------------------------------------------------------//
// Macros
//--------------------------------------------------------------------------//

#if (GB_COMPILER == GB_COMPILER_MSVC && GB_PLATFORM == GB_PLATFORM_WIN32)
#	include <assert.h>

#	define GB_INLINE			__forceinline

#	ifdef GB_DEBUG
#		define GB_ASSERT(c)		assert(c);
#		define GB_BREAKPOINT	__asm int 3
#	endif
#endif

#if GB_COMPILER == GB_COMPILER_GCC
#	define GB_INLINE			inline
#endif

#if 0 && defined (GB_COMPILER_CPP)
#   define GB_EXTERN_C				extern "C" 
#   define GB_EXTERN_C_BLOCK_BEGIN	extern "C" {
#   define GB_EXTERN_C_BLOCK_END	}
#else
#   define GB_EXTERN_C
#   define GB_EXTERN_C_BLOCK_BEGIN
#   define GB_EXTERN_C_BLOCK_END
#endif

#define GB_UNREF(v)				((void*)v)
#define GB_NULL_STATEMENT		GB_UNREF(0)

#ifndef GB_BREAKPOINT
#	define GB_BREAKPOINT		GB_NULL_STATEMENT
#endif

#ifndef GB_ASSERT
#	define GB_ASSERT(c)			//GB_NULL_STATEMENT
#endif

#ifndef GB_INLINE
#	define GB_INLINE
#endif

//--------------------------------------------------------------------------//
// Basic types
//--------------------------------------------------------------------------//

typedef signed int				GBbool;

typedef signed int				GBint32;
typedef signed short			GBint16;
typedef signed char				GBint8;

typedef unsigned int			GBuint32;
typedef unsigned short			GBuint16;
typedef unsigned char			GBuint8;

typedef float					GBfloat;
typedef double					GBdouble;

#if (GB_COMPILER == GB_COMPILER_MSVC) || defined (__COOK_NAV2__)
	typedef int64 GBint64;
	typedef int64 GBuint64;
#elif (GB_COMPILER == GB_COMPILER_GCC) || (GB_PLATFORM == GB_PLATFORM_SYMBIAN)
	typedef signed long long	GBint64;
	typedef unsigned long long	GBuint64;
#endif

//--------------------------------------------------------------------------//
// Constants
//--------------------------------------------------------------------------//

#ifndef NULL
#	define NULL					0
#endif

#define	GB_TRUE					1
#define	GB_FALSE				0

#define	GB_UINT32_MAX			0xffffffff
#define	GB_UINT16_MAX			0xffff
#define	GB_UINT8_MAX			0xff

#define	GB_INT32_MAX			0x7fffffff
#define	GB_INT16_MAX			0x7fff
#define	GB_INT8_MAX				0x7f

#define	GB_INT32_MIN			(-0x7fffffff - 1)
#define	GB_INT16_MIN			-0x8000
#define	GB_INT8_MIN				-0x80

#define	GB_PI					3.14159265f

#define	GB_FIXED_BITS			16
#define	GB_FIXED_ONE			(1 << GB_FIXED_BITS)
#define	GB_FIXED_TWO			(2 << GB_FIXED_BITS)

#define	GB_FIXED_PI				(GBuint32)(GB_PI * GB_FIXED_ONE)

//--------------------------------------------------------------------------//
// Extended types
//--------------------------------------------------------------------------//

typedef	GBfloat					GBvec2[2];
typedef	GBfloat					GBvec3[3];

typedef	GBint32					GBivec2[2];
typedef	GBint32					GBivec3[3];

//--------------------------------------------------------------------------//
// Helper functions
//--------------------------------------------------------------------------//

GB_EXTERN_C_BLOCK_BEGIN

GBuint32	gbHighestBit		(GBuint32 v);
GBuint32	gbHighestBit64		(GBuint64 v);
GBint64		gbSqrt64			(GBint64 v);

GBint32		gbCeil4				(GBint32 v);
GBint32		gbCeilA				(GBint32 v, GBint32 bits);
GBint32		gbFloor4			(GBint32 v);
GBfloat		gbFAbs				(GBfloat v);
GBint32		gbAbs				(GBint32 v);

void		gbMatrixIdentity	(GBint32* result);
void		gbMatrixRotationX	(GBfloat angle, GBint32* result);
void		gbMatrixRotationY	(GBfloat angle, GBint32* result);
void		gbVec3Transform		(const GBint32* vec, const GBint32* matrix, GBint32* result);
void		gbVec3TransformHalfQ(const GBint32* vec, const GBint32* matrix, GBint32* result);

void		gbMatrixMultiply	(const GBint32* left, const GBint32* right, GBint32* result);

void		gbFMatrixIdentity	(GBfloat* result);
void		gbFMatrixRotationX	(GBfloat angle, GBfloat* result);
void		gbFMatrixRotationY	(GBfloat angle, GBfloat* result);
void		gbFMatrixMultiply	(const GBfloat* left, const GBfloat* right, GBfloat* result);

void		gbFVec3Transform	(const GBfloat* vec, const GBfloat* matrix, GBfloat* result);
void		gbFVec3Normalize	(GBfloat* vec);

FILE*		gbOpenFileFromPath	(const char* path, const char* filename, const char* mode);

GBuint32	gbStrLength			(const char* str);
void		gbStrCopy			(char* tgt, const char* src);
char*		gbStrDuplicate		(const char* str);

void		gbReadWord			(FILE* f, char* out, const char* stopChars);
void		gbSkipChars			(FILE* f, const char* chars);

GBfloat		gbStringToFloat		(const char* str);

GB_EXTERN_C_BLOCK_END

#define gbInt32Swap(x, y) { GBint32 t = x; x = y; y = t; }

#define SAFE_FREE(p)		{ free(p); p = NULL; }

//--------------------------------------------------------------------------//
// Text rectangle struct
//--------------------------------------------------------------------------//

typedef struct GBTextRect_s
{
	GBint32 x;
	GBint32 y;
	GBint32 left;
	GBint32 top;
	GBint32 right;
	GBint32 bottom;
} GBTextRect;

//--------------------------------------------------------------------------//
// Display parameters
//--------------------------------------------------------------------------//

typedef struct GBDisplayParams_s
{
	void*			buffer;
	GBuint32		bufferWidth;

	GBuint32		x;
	GBuint32		y;
	GBuint32		width;
	GBuint32		height;
} GBDisplayParams;

GB_EXTERN_C void	GBDisplayParams_init	(GBDisplayParams* displayParams);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#if defined(GB_PIXELFORMAT_888)
typedef GBuint32 GBOutputFormat;
#elif defined(GB_PIXELFORMAT_444) || defined(GB_PIXELFORMAT_565)
typedef GBuint16 GBOutputFormat;
#endif

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#endif
