/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UNIX_MACHINE_H
#define UNIX_MACHINE_H

#if defined(__unix__) || defined(__MACH__)

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <limits.h>

// Hope that we have inttypes.h
// Get the format macros from inttypes.
#define __STDC_FORMAT_MACROS
#include  <inttypes.h>

 
/* Standard integer types */
typedef int8_t	   int8;
typedef uint8_t   uint8;
typedef int16_t   int16;
typedef uint16_t  uint16;
typedef int32_t	int32;
typedef uint32_t	uint32;
typedef int64_t	int64;
typedef uint8_t   byte;

/* floating-point types */
typedef float  float32;
typedef double float64;

/* Minimum and Maximum extents for all types */
#define MAX_UINT8    UINT8_MAX
#define MAX_INT8     INT8_MAX
#define MAX_UINT16   UINT16_MAX
#define MAX_INT16    INT16_MAX
#define MAX_UINT32   UINT32_MAX
#define MAX_INT32    INT32_MAX
#define MAX_INT64    INT64_MAX

#define MAX_FLOAT32	KMaxTReal32
#define MAX_FLOAT64	KMaxTReal64
#define MIN_INT8     INT8_MIN
#define MIN_INT16    INT16_MIN
#define MIN_INT32    INT32_MIN 
#define MIN_FLOAT32	KMinTReal32
#define MIN_FLOAT64	KMinTReal64

#else //__unix__ || __MACH__
/* 
   If this file is included for some other platform,
   stop the compilation!
*/
#error "This Arch DataTypes.h is only for Unix!"

#endif

#endif

