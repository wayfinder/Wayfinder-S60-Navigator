/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CINFOG_H
#define CINFOG_H

#if defined (__GNUC__) && __GNUC__ >= 2
#define MC2_LIKELY(expression) (__builtin_expect(!!(expression), 1))
#define MC2_UNLIKELY(expression) (__builtin_expect(!!(expression), 0))
#else
#define MC2_LIKELY(expression) (expression)
#define MC2_UNLIKELY(expression) (expression)
#endif

/// Macro to get the number of items in an array. Use with care
#define NBR_ITEMS(x) ( sizeof(x)/sizeof(x[0]) )
/// Macro that fails to compile if the number of elements in an array is wrong
#define CHECK_ARRAY_SIZE( x, y ) if ( true ) ; else { \
               uint32 check_array[\
               ( (y) == NBR_ITEMS( x ) ) ? 1 : -1 ] = { 0 };\
               check_array[0] = 8;\
               }

// Include the specialized stuff
#include "ArchDataTypes.h"
#include "ArchConfigMisc.h"

#ifndef TRACE_FUNC
#define TRACE_FUNC()
#endif

// Include logging
#include "MC2Logging.h"

// Quick shutdown, i.e. don't delete everything when shutting down.
#define QUICK_SHUTDOWN

// Enable perforamance debugging.
//#define ENABLE_MAPPERFORMANCE_DEBUGGING

//For less memory consumption
#define CALCULATE_COS_LAT
#define SLIM_TILE_COORDS

// Other common stuff
#  ifdef DEBUG_LEVEL_8
#     define   DEBUG8(a)   a
#  else
#     define   DEBUG8(a)
#  endif
#  ifdef DEBUG_LEVEL_4
#     define   DEBUG4(a)    a
#  else
#     define   DEBUG4(a)
#  endif
#  ifdef DEBUG_LEVEL_2
#     define   DEBUG2(a)   a
#  else
#     define   DEBUG2(a)
#  endif
#  ifdef DEBUG_LEVEL_1
#     define   DEBUG1(a)   a
#  else
#     define   DEBUG1(a)
#  endif


#endif
