/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAP_HANDLER_UTIL_H
#define TILEMAP_HANDLER_UTIL_H

#include "TileMapConfig.h"

#if defined (__unix__) || defined (__MACH__) || defined (__SYMBIAN32__)
#include <sys/time.h>
#elif defined(ARCH_OS_WINDOWS)
/* needed for compiling on Win32 as there is no sys\time.h */
#else
#error "Unknown system"
#endif

#include <string.h>

/**
 *   Class that contains functions that the TileMapHandler
 *   needs, but that may be target-specific.
 *   When it gets too messy, start subclassing like this
 *   class TileMapUtil :
 #   ifdef PLATFORM_A
 #    public PlatformAUtil
 #   endif
 */
class TileMapUtil {
public:
   /**
    *    Returns the number of milliseconds since something.
    */
   static inline uint32 currentTimeMillis() {
#if defined (__unix__) || defined (__MACH__)
      struct timeval tv;
      gettimeofday(&tv, NULL);
      return (tv.tv_usec/1000 + tv.tv_sec * 1000);
#elif defined(__SYMBIAN32__)
      TTime now;
      now.UniversalTime(); // or UniversalTime
   #if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
      return LOW(now.Int64() / 1000);
   #else
      return (now.Int64() / 1000).GetTInt();
   #endif
#elif defined(ARCH_OS_WINDOWS)
      return( uint32(GetTickCount()) );
#endif
   }

   /**
    *    Returns a copy of the string allocated with new.
    */
   static char* newStrDup(const char* orig);
};

class TileMapClock {
public:
   TileMapClock() { m_startTime = TileMapUtil::currentTimeMillis(); }
   uint32 getTime() const {
      return TileMapUtil::currentTimeMillis() - m_startTime;
   }
private:
   uint32 m_startTime;
   
};

#ifdef USE_DEBUG_COUNTER_IN_TILEMAPUTIL
#include <stdio.h>
#include <string.h>

class DebugCounter {
public:
   DebugCounter(const char* name, uint32 minTimeInterval ) {
      m_name   = new char[strlen(name)+1];
      strcpy(m_name, name);
      m_count  = 0;
      m_lastTime = TileMapUtil::currentTimeMillis();
      m_timeInterval = minTimeInterval;
   }

   ~DebugCounter() {
      delete [] m_name;
   }
   
   const DebugCounter& operator++() {
      ++m_count;
      uint32 curTime = TileMapUtil::currentTimeMillis();
      if ( ( curTime - m_lastTime ) > m_timeInterval ) {
         FILE* fil = fopen("C:\\counters.txt", "a");
         if ( fil ) {
            // FIXME: Nicer date formatting.
            fprintf(fil, "%u:%s:%u",
                    (unsigned int)curTime,
                    m_name,
                    (unsigned int)m_count );
            fclose(fil);                    
         }
      }
      return *this;
   }
private:
   uint32 m_timeInterval;
   uint32 m_count;
   char* m_name;
   uint32 m_lastTime;   
};

#endif

#endif


