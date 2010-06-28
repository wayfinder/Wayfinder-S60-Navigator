/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WIN32_ARCH_CONFIG_MISC_H
#define WIN32_ARCH_CONFIG_MISC_H

/* define our platform */
#define ARCH_OS_WINDOWS

// Include the Win32 main header
#include <windows.h>

#include <math.h>

/* Stupid Win32 headers define ERROR - try to undefine it */
#ifdef ERROR
   #undef ERROR
#endif

/* math constants */
#ifndef M_PI
   #define M_PI   3.14159265358979323846
#endif

/* for MIN/MAX */
// Minimum macro
#ifndef MIN
   #define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

// Maximum macro
#ifndef MAX
   #define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

/* quick replacement for rint */
inline double rint(double x)
{
   return floor( x + 0.5 );
}

/* win32 doesn't have strcasecmp, simple workaround macro */
#define strcasecmp   _stricmp

/* for assertions */
#include <assert.h>
#define MC2_ASSERT(x) assert(x)

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

/* to disable the dumb truncation warning */
#pragma warning( disable : 4786 )
#pragma warning( disable : 4503 )

#endif
