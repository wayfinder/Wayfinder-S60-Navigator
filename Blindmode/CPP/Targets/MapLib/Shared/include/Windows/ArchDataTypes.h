/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WIN32_DATATYPES_H
#define WIN32_DATATYPES_H

/* Arch Data Types for Win32 Platform */

/* integer types */
typedef signed char int8;
typedef unsigned char uint8;
typedef unsigned short uint16; 
typedef signed short int16; 
typedef unsigned int uint32; 
typedef signed int int32; 
typedef unsigned __int64 uint64; 
typedef __int64 int64; 

/* data types */
typedef uint8 byte; 

/* floating-point types */
typedef float float32; 
typedef double float64; 

/* min\max values for integers */
#define MAX_UINT8 0xff 
#define MAX_INT8 int8(0x7f) 
#define MAX_UINT16 0xffff 
#define MAX_INT16 int16(0x7fff) 
#define MAX_UINT32 0xffffffff 
#define MAX_INT32 int32(0x7fffffff) 
#define MIN_INT8 int8(0x80) 
#define MIN_INT16 int16(0x8000) 
#define MIN_INT32 int32(0x80000000) 
#define MAX_UINT64 0xffffffffffffffff 
#define MAX_INT64 0x7fffffffffffffff 
#define MAX_FLOAT64 DBL_MAX 

/* bit-manipulation macros */
#define REMOVE_UINT32_MSB(a) (uint32(a)&0x7fffffff) 
#define GET_UINT32_MSB(a) (uint32(a)&0x80000000) 
#define TOGGLE_UINT32_MSB(a) (uint32(a)^0x80000000) 

#else 

/* This file should not be used for any other platform! */
#error "This ArchDataTypes.h is only for Win32!"

#endif
