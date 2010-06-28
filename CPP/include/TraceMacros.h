/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRACEMACROS_H
#define TRACEMACROS_H

#include "arch.h"
#ifdef __SYMBIAN32__
# include <flogger.h>
# include <e32math.h>
# include <stdio.h>
#elif defined _MSC_VER
# include <stdio.h>
#endif

#define STDIO_FILES

#undef NAV2LOGGING
/* #define NAV2LOGGING */
#if defined NAV2_CLIENT_UIQ33 && !defined __WINS__
# define TRACE_FILE "d:\\media files\\other\\trace.txt"
#elif defined NAV2_CLIENT_UIQ3 && !defined __WINS__
# define TRACE_FILE "d:\\other\\trace.txt"
#elif defined NAV2_CLIENT_UIQ && !defined __WINS__
# define TRACE_FILE "d:\\media files\\other\\trace.txt"
#else
# define TRACE_FILE "e:\\trace.txt"
//#define TRACE_FILE "e:\\system\\data\\WayFinder\\trace.txt"
#endif

#ifdef STDIO_FILES
   static inline void
   handleTrace( int line, const char* file, const char* function,
                const char* more, const char* evenMore )
   {
      FILE* f = fopen( TRACE_FILE, "a" );

      if ( !f ) return;
      
      char dateStr[50];
      dateStr[ 2 ] = '\0';

#ifdef __SYMBIAN32__
      TTime um;
      um.HomeTime();
      TDateTime dm = um.DateTime();
      TReal e = 0.0;
      TInt d = Math::Round( e, ( dm.MicroSecond() / 1000.0 ), 0 );
      d = TInt( e );

      sprintf( dateStr, "%04d-%02d-%02d %02d:%02d:%02d.%03d ",
               dm.Year(), dm.Month()+1, dm.Day()+1, dm.Hour(), dm.Minute(), 
               dm.Second(), d );
#endif

      fprintf( f, "%s%s:%d:%s:%s:%s\n", 
               (dateStr + 2), file, line, function, more, evenMore );
      fflush( f );
      fclose( f );
   }

   inline const char* makeDate( char* dateStr )
   { 
      dateStr[ 2 ] = '\0';

#ifdef __SYMBIAN32__
      TTime um;
      um.HomeTime();
      TDateTime dm = um.DateTime();
      TReal e = 0.0;
      TInt d = Math::Round( e, ( dm.MicroSecond() / 1000.0 ), 0 );
      d = TInt( e );

      sprintf( dateStr, "%04d-%02d-%02d %02d:%02d:%02d.%03d ",
               dm.Year(), dm.Month()+1, dm.Day()+1, dm.Hour(), dm.Minute(), 
               dm.Second(), d );
      return dateStr + 2;
#else
      return dateStr + 2;
#endif
   }

#if !defined( _MSC_VER ) && !defined( __MWERKS__ )
#define TRACE_DBG( format, args...) do {\
      FILE* f = fopen( TRACE_FILE, "a" );\
      if ( !f ) break;\
      char tmp_dte[50];\
      char* fmt = new char[ 50 + strlen( format ) ];\
      sprintf( fmt, "%%s%%s:%%d:%%s:" "%s" "\n", format );\
      fprintf( f, fmt,\
               makeDate(tmp_dte), __FILE__, __LINE__,\
               __FUNCTION__ , ##args );\
      fclose(f);\
      delete [] fmt;\
   } while(0)
#define TRACE_DBG2( format, args...) do {\
      FILE* f = fopen( TRACE_FILE, "a" );\
      if ( !f ) break;\
      char* fmt = new char[ 50 + strlen( format ) ];\
      sprintf( fmt, "%s\n", format );\
      fprintf( f, fmt,\
               ##args );\
      fclose(f);\
      delete [] fmt;\
   } while(0)
#else
// Variadic macros do not work in VC++ 5
# if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define TRACE_DBG(format, ...) do {\
      FILE* f = fopen("c:\\trace.txt", "a" );\
      if ( !f ) break;\
      char tmp_dte[50];\
      fprintf( f, "%s%s:%d:%s:" format "\n",\
               makeDate(tmp_dte), __FILE__, __LINE__,\
               __FUNCTION__ , __VA_ARGS__ );\
      fclose(f);\
   } while(0)
#  define TRACE_DBG2(format, ...) do {\
      FILE* f = fopen("c:\\trace.txt", "a" );\
      if ( !f ) break;\
      fprintf( f, format "\n",\
               __VA_ARGS__ );\
      fclose(f);\
   } while(0)
# else
#  define TRACE_DBG
#  define TRACE_DBG2
# endif // __STDC_VERSION__
#endif // Is _MSC_VER

#elif defined( NAV2LOGGING )
   static inline void
   handleTrace( isab::Log* log, int line, const char* file, const char* function,
                const char* more, const char* evenMore )
   {
      log->info("%s:%d:%s:%s:%s\n", file, line, function, more, evenMore);
   }

#else

   static inline void
   handleTrace( int line, const char* file, const char* function,
                const char* more, const char* evenMore )
   //handleTrace( int line, TPtrC8& file, TPtrC8& function,
   //             TPtrC8& extra )
   {
      RFileLogger::WriteFormat(_L("dbg"), _L("dbg.txt"),
                               EFileLoggingModeAppend,
                               _L8("%s:%d:%s:%s:%s\n"),
                               file, line, function,
                               more, 
                               evenMore );
   }
#endif
#endif

#undef ENABLE_TRACE
//#define ENABLE_TRACE
#undef TRACE_FUNC
#undef TRACE_FUNC1
#undef TRACE_FUNC2

#ifndef ENABLE_TRACE

   #define TRACE_FUNC()
   #define TRACE_FUNC1( a )
   #define TRACE_FUNC2( a, b )

   #ifdef TRACE_DBG
      #undef TRACE_DBG
   #endif

   #ifdef TRACE_DBG2
      #undef TRACE_DBG2
   #endif

   #if !defined( _MSC_VER ) && !defined( __MWERKS__ )
      #define TRACE_DBG( format, args...) do {} while(0)
      #define TRACE_DBG2( format, args...) do {} while(0)
   #else
      // Variadic macros do not work in VC++ 5
      #define TRACE_DBG
      #define TRACE_DBG2
   #endif


#else
#if defined( _MSC_VER ) || defined( __MWERKS__ )
#  ifndef __FUNCTION__
#    define __FUNCTION__ ""
#  endif
#endif
   #ifdef NAV2LOGGING
      #define TRACE_FUNC() do { handleTrace( m_log, __LINE__,__FILE__,__FUNCTION__,"", "" );} while(0)
      #define TRACE_FUNC1( a ) do { handleTrace(m_log, __LINE__,__FILE__,__FUNCTION__, a, "" );} while(0)
      #define TRACE_FUNC2( a, b ) do { handleTrace(m_log, __LINE__,__FILE__,__FUNCTION__, a, b );} while(0)
   #else
      #define TRACE_FUNC() do { handleTrace( __LINE__,__FILE__,__FUNCTION__,"", "" );} while(0)
      #define TRACE_FUNC1( a ) do { handleTrace(__LINE__,__FILE__,__FUNCTION__, a, "" );} while(0)
      #define TRACE_FUNC2( a, b ) do { handleTrace(__LINE__,__FILE__,__FUNCTION__, a, b );} while(0)
   #endif
#endif


