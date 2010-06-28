/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Log.h"

// settings have moved to here:
#include "LogMem.h"

#ifdef _MSC_VER
#undef typeof
# define typeof(arg) "UNKNOWN"
#endif

#ifdef MEMORY_LOG
#  undef  CONSTRUCTOR_WITH_ILOG
#  undef  DECLARE_ILOG
#  undef  CREATE_LOG_AND_MASTER
#  define  CONSTRUCTOR_WITH_ILOG(type) type(isab::Log* aLog) : iLog(aLog) {}
#  define  DECLARE_ILOG() isab::Log* iLog
#  define  CREATE_LOG_AND_MASTER
#endif // MEMORY_LOG

#ifndef ILOG_POINTER
# define ILOG_POINTER iLog
#endif

#undef LOGNEW
#undef LOGNEWA
#undef LOGDEL
#undef LOGDELA
#undef LOGHEAPSIZE

#if (!defined ONLY_LOG_HEAP) && (defined MEMORY_LOG)
#  define  LOGNEW(pointer, type)                                             \
if(ILOG_POINTER)                                                             \
   ILOG_POINTER->info("MEM "__FILE__":%d - NEW %p " #type "(%u bytes)",      \
                      __LINE__, pointer, sizeof(type))

#  define  LOGNEWA(pointer, type, num)                                      \
   if(ILOG_POINTER)                                                         \
      ILOG_POINTER->info("MEM "__FILE__":%d - NEW[%u] %p " #type            \
                         "(%u bytes)", __LINE__, num, pointer,              \
                         sizeof(type)*num)

/* #ifdef _MSC_VER */
/* # define LOGDEL(pointer) if(ILOG_POINTER) ILOG_POINTER->info("MEM "__FILE__":%d - DELETE %p " "UNKNOWN" "(%u bytes)", __LINE__, pointer, sizeof(*pointer)) */
/* #else */

#  define  LOGDEL(pointer)                                      \
   if(ILOG_POINTER)                                             \
      ILOG_POINTER->info("MEM "__FILE__":%d - DELETE %p "       \
                         STR(typeof(*pointer)) "(%u bytes)",    \
                         __LINE__, static_cast<void*>(pointer), \
                         sizeof(*(pointer)))
/* #endif */

#  define  LOGDELA(pointer)                                                  \
if(ILOG_POINTER)                                                             \
   ILOG_POINTER->info("MEM "__FILE__":%d - DELETE[] %p"                      \
                      STR(typeof(*pointer)) "(%u bytes)",                    \
                      __LINE__, reinterpret_cast<void*>(pointer),            \
                      sizeof(*(pointer)))

#else
#  define  LOGNEW(pointer, type)
#  define  LOGNEWA(pointer, type, num)
#  define  LOGDEL(pointer) 
#  define  LOGDELA(pointer) 
#endif // (!defined ONLY_LOG_HEAP) && (defined MEMORY_LOG)



#if (defined MEMORY_LOG) && (defined LOG_HEAP)
#  define  LOGHEAPSIZE(string)                                              \
if (ILOG_POINTER){                                                          \
   TInt largestBlock;                                                       \
   TInt totalSpace = User::Available(largestBlock);                         \
   TInt totalSize;                                                          \
   TInt nbrCells = User::AllocSize(totalSize);                              \
   ILOG_POINTER->debug(string " ah: %d lb: %d hs: %d nc: %d",               \
                       totalSpace, largestBlock, totalSize, nbrCells);      \
}

#else
#  define  LOGHEAPSIZE(string)
#endif //MEMORY_LOG



