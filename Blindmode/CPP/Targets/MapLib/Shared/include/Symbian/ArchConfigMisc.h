/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYMIBAN_CONFIG_MISC_H
#define SYMIBAN_CONFIG_MISC_H

/* 
   Symbian supports zlib 1.1.3, double deletion bug,
   so don't use it right now.
*/
#define HAVE_ZLIB

/*
   If to enable moving and zooming the screen as a bitmap.
 */
#undef HANDLE_SCREEN_AS_BITMAP
//#define HANDLE_SCREEN_AS_BITMAP

/* 
   Define MIN and MAX macros to point to Symbian 
   Min and Max routines. This should be after the
   previous included header as e32std.h is required.
*/
#undef MAX
#undef MIN
#define MAX(a,b)   Max(a,b)
#define MIN(a,b)   Min(a,b)

#if 0
# ifdef __GNUC__
#   define TRACE_FUNC() do { handleTrace(__LINE__,__FILE__,__FUNCTION__ );} while(0)
# else
#   define TRACE_FUNC() do { handleTrace(__LINE__,__FILE__, 0 ); } while (0)
# endif
#else
# undef TRACE_FUNC
# define TRACE_FUNC()
#endif

// Defined in SymbianTilemapToolkit
void handleAssert(const char*,const char*,int);
// Defined in SymbianTilemapToolkit
void handleTrace( int line, const char* file, const char* function );

#define STRING(x) #x

#if 0
// Assert does something
#   ifndef __WINS__
#      define MC2_ASSERT(f)                     \
               if (f) { ;                             \
               } else                                 \
                  handleAssert(STRING(f), __FILE__, __LINE__)
#   else
#      define MC2_ASSERT(f) do { if ( !(f) ) { byte* b=0; *b=0;} } while (0)
#   endif
#else
// Assert does nothing. (Should be optimized away, I suspect)
#define MC2_ASSERT(x) do { if ( false && (x) ) {} } while (0)
#endif

#endif
