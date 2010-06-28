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

#ifndef LOGMEM_H
#define LOGMEM_H

// #ifndef _MSC_VER
//# undef MEMORY_LOG
// #else
// # define MEMORY_LOG
// #endif
// #undef LOG_HEAP
// #define LOG_HEAP
// #undef ONLY_LOG_HEAP
// #define ONLY_LOG_HEAP

#if !defined(MEMORY_LOG) || !defined(dsfksdafkj)
/// Logs.
#  define  LOGNEW(pointer, type)
/// Logs.
#  define  LOGNEWA(pointer, type, num)
/// Logs.
#  define  LOGNEWL(type, num) ELeave
/// Logs.
#  define  LOGNEWAL(type, num) ELeave
/// Allocates. Logs.
#  define  NEW_AND_LOGL(type) new(ELeave) type
/// Allocates an array. Logs.
#  define  NEWA_AND_LOGL(type, num) new(ELeave) type[num]
/// Logs.
#  define  LOGDEL(pointer) 
/// Logs.
#  define  LOGDELA(pointer)
/// Deletes. Logs. 
#  define  DELETE_AND_LOG(pointer) delete (pointer);
/// Deletes an array. Logs.
#  define  DELETEA_AND_LOG(pointer) delete[] (pointer);
/// Logs a strings.
# define LOGNEWSTRING(str)
/// Pops, destroys, logs.
# define POP_DESTROY_LOG(a) CleanupStack::PopAndDestroy(a)
# define POP_DESTROY_LOGA(a) CleanupStack::PopAndDestroy(a);
/// Pushes. Logs.
# define PUSH_LOGNEW(a, b) CleanupStack::PushL(a);
/// Performs symbian two phase construction. Creates object using constructor taking one pointer. pushes. Calls constructl with parameters. Logs.
# define NEWLC_AND_LOG(a, b) \
   a = new(ELeave) b(NULL); \
   CleanupStack::PushL(a); \
   a->ConstructL
/// declares a pointer variable of the type and then same as NEWLC_AND_LOG
# define DECLARE_NEWLC_AND_LOG(a, b) \
   b* a = new(ELeave) b(NULL); \
   CleanupStack::PushL(a); \
   a->ConstructL

# define LOGANDOPEN(pointer, opencommand) \
   (pointer)->opencommand

# define LOGOPEN(pointer)
# define LOGCLOSE(pointer)
# define LOGANDCLOSE(pointer) \
   (pointer)->Close();
# define LOGCHECKED()
#else
# error not checked in.
#endif

#if (defined MEMORY_LOG) && (defined LOG_HEAP) && defined(xdafsa)
#ifdef __SYMBIAN32__
#include <hal.h>
#include <hal_data.h>

class LogMem
{
public:
   inline static void logheapsize(
      TInt& freeMem,
      TInt& largestBlock,
      TInt& totalSpace,
      TInt& totalSize,
      TInt& nbrCells,
      TInt& allocCells,
      TInt& freeCells,
      TTimeIntervalMicroSeconds& aCpuTime,
      TInt& aHeapSize,TInt& aStackSize,
      TInt& aProcessHandleCount, TInt& aThreadHandleCount,
      TInt& requestCount)
   {
      HAL::Get(HALData::EMemoryRAMFree, freeMem);
#ifndef SYMBIAN_9
      totalSpace = User::Available(largestBlock);
#endif
      nbrCells = User::AllocSize(totalSize);
      allocCells = User::CountAllocCells(freeCells);
      RThread thisthread;
      TInt res = thisthread.GetCpuTime(aCpuTime);
      res = thisthread.GetRamSizes(aHeapSize, aStackSize);
      thisthread.HandleCount(aProcessHandleCount, aThreadHandleCount);
      requestCount = thisthread.RequestCount();
      thisthread.Close();
   }

   inline static char* getlogdatastring(const char* str)
   {
      TInt freeMem; TInt largestBlock; TInt totalSpace; TInt totalSize; TInt nbrCells; TInt allocCells; TInt freeCells;
      TTimeIntervalMicroSeconds aCpuTime;
      TInt aHeapSize;
      TInt aStackSize;
      TInt aProcessHandleCount;
      TInt aThreadHandleCount;
      TInt requestCount;
      LogMem::logheapsize(
         freeMem, largestBlock, totalSpace, totalSize, nbrCells, allocCells, freeCells,
         aCpuTime, aHeapSize, aStackSize, aProcessHandleCount, aThreadHandleCount, requestCount);
      char* tt = new char[512];
      sprintf(tt,
         "\n%s Halfreemem: %d "
         "Total available heap: %d "
         "Largest block: %d "
         "Total size of allocated memory: %d "
         "Number of allocated cells: %d "
         "Number of allocated cells(2): %d "
         "Number of free cells: %d "
         "Cpu time high: %d "
         "cpu time low: %d "
         "Heap size: %d "
         "Stack size: %d "
         "Process handle count: %d "
         "Thread handle count: %d "
         "Request count: %d.",
         str,
         freeMem, totalSpace, largestBlock, totalSize, nbrCells, allocCells, freeCells,
         aCpuTime.Int64().High(), aCpuTime.Int64().Low(), aHeapSize, aStackSize, aProcessHandleCount, aThreadHandleCount, requestCount);
      return tt;
   }

};

#  define  LOGHEAPSIZETO(target, string) \
if (ILOG_POINTER) { \
   char* t = LogMem::getlogdatastring(string); \
   LOGNEWSTRING(t); \
   ILOG_POINTER->target(t); \
   DELETEA_AND_LOG(t); \
}
#  define  LOGHEAPSIZE(string) LOGHEAPSIZETO(debug, string);
#else // -> ! #ifdef __SYMBIAN32__
#  define  LOGHEAPSIZETO(target, string)
#endif
#else // -> ! #if (defined MEMORY_LOG) && (defined LOG_HEAP)
#  define  LOGHEAPSIZETO(target, string)
// if (ILOG_POINTER) { ILOG_POINTER->target(string); }
#  define  LOGHEAPSIZE(string)
#endif // -> #if (defined MEMORY_LOG) && (defined LOG_HEAP)
# define LOG_INFO(a)
# define LOG_INFO_1(a, b)
# define LOG_INFO_2(a, b, c)

#endif
