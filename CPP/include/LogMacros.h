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
 * Useful macros when using the Log class.
 */

#define NO_LOG_OUTPUT

#undef DBG
#undef ERR
#undef WARN
#undef INFO
#undef DBGDUMP
#undef ERRDUMP
#undef WARNDUMP
#undef INFODUMP

#undef DROP

#ifndef LOGPOINTER 
# define LOGPOINTER m_log
#endif

#if !defined(_MSC_VER) && !defined(__MWERKS__)
//varargmacros supported
# ifndef NO_LOG_OUTPUT
//logging enabled, varargmacros supported
#  define DBG(format, args...)  \
    LOGPOINTER->debug  ("%d: " format, __LINE__ ,##args)
#  define INFO(format, args...) \
    LOGPOINTER->info   ("%d: " format, __LINE__ ,##args)
#  define WARN(format, args...) \
    LOGPOINTER->warning("%d: " format, __LINE__ ,##args)
#  define ERR(format, args...)  \
    LOGPOINTER->error  ("%d: " format, __LINE__ ,##args)
#  define DROP(format, args...)
# else
//logging disabled, varargmacros supported
#  define DBG(format, args...) 
#  define INFO(format, args...)
#  define WARN(format, args...)
#  define ERR(format, args...) 
#  define DROP(format, args...)
# endif
# if !defined(__SYMBIAN32__) && !defined(NO_LOG_OUTPUT)
//logging enabled, varargmacros supported, dump enabled
#   define DBGDUMP(header, args...)  LOGPOINTER->debugDump  (header ,##args)
#   define INFODUMP(header, args...) LOGPOINTER->infoDump   (header ,##args)
#   define WARNDUMP(header, args...) LOGPOINTER->warningDump(header ,##args)
#   define ERRDUMP(header, args...)  LOGPOINTER->errorDump  (header ,##args) 
#  else
//logging enabled, varargmacros supported, dump disabled
#   define DBGDUMP(header, args...)  
#   define INFODUMP(header, args...) 
#   define WARNDUMP(header, args...) 
#   define ERRDUMP(header, args...)  
#  endif
#else
//varargmacros not supported, 
# ifndef NO_LOG_OUTPUT
//logging enabled, varargmacros not supported
#  define DBG      LOGPOINTER->debug
#  define INFO     LOGPOINTER->info
#  define WARN     LOGPOINTER->warning
#  define ERR      LOGPOINTER->error
#  define DROP     ::isab::Log::dropIt 
#  define ERRDUMP  LOGPOINTER->errorDump
#  define DBGDUMP  LOGPOINTER->debugDump
#  define INFODUMP LOGPOINTER->infoDump
#  define WARNDUMP LOGPOINTER->warningDump
# else
//logging disabled, varargmacros not supported
#  define DBG      ::isab::Log::dropIt
#  define INFO     ::isab::Log::dropIt
#  define WARN     ::isab::Log::dropIt
#  define ERR      ::isab::Log::dropIt
#  define DROP     ::isab::Log::dropIt
#  define ERRDUMP  ::isab::Log::dropIt
#  define DBGDUMP  ::isab::Log::dropIt
#  define INFODUMP ::isab::Log::dropIt
#  define WARNDUMP ::isab::Log::dropIt
# endif
#endif


//THREADINFO doesn't use varargs, so it's not so complicated to define.
#ifdef NO_LOG_OUTPUT
# define THREADINFO(thread)
#else
# define THREADINFO(thread) do{                                            \
                 char threadString[1024] = {0};                            \
                 (thread).getThreadInfo(threadString, sizeof(threadString)); \
                 INFO("Started thread: %s", threadString);                 \
              } while(0)
#endif

namespace {
   inline uint8 byteOf(int n, uint32 d)
   {
      return uint8((d>>(n*8)) & 0x0ff);
   }
}
#define PRIuIPFORMAT PRIu8".%"PRIu8".%"PRIu8".%"PRIu8
#define IPEXPAND(x) byteOf(3,x),byteOf(2,x), byteOf(1,x),byteOf(0,x)


