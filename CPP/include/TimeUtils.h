/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include "machine.h"

#ifdef __SYMBIAN32__
#include <e32base.h>
#include <sys/time.h>
//#include <time.h>
#elif UNDER_CE
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef UNDER_CE
namespace {
   inline ULARGE_INTEGER ToULARGE(const FILETIME& time)
   {
      ULARGE_INTEGER ret;
      ret.QuadPart = 0;
      ret.LowPart  = time.dwLowDateTime;
      ret.HighPart = time.dwHighDateTime;
      return ret;
   }

   inline uint64 operator-(const ULARGE_INTEGER& lhs, 
                           const ULARGE_INTEGER& rhs)
   {
      return lhs.QuadPart - rhs.QuadPart;
   }


}
#endif

namespace isab {
   class TimeUtils {
   public:
#     define WRAP_AROUND_MILLIS uint32(uint32(24)*uint32(60)*uint32(60)*uint32(1000))
      
      /**
       *   Returns a timestamp in milliseconds.
       *   @return milliseconds since midnight.
       */
      inline static uint32 millis();

      /**
       *   Structure containing seconds and milliseconds.
       */
      struct PrecisionTime{
         long seconds;
         long microseconds;
         inline PrecisionTime();
      };

      /**
       *   Returns a precisiontime representing 
       *   the current systemtime. (Micros are millis * 1000 on wince)
       */
      inline static PrecisionTime micros();

      /**
       * Replace the standard function time(), which doesn't seem to
       * be implemented for Microsoft Embedded C++. Since the only
       * implementation at this time that is needed, is when the in
       * parameter is NULL, the function doesn't handle anything else.
       * @return a_timer =  NULL: Time from 1970 in seconds.
       *         a_timer <> NULL: -1 (ERROR)
       */
      static time_t time( time_t* a_timer )
      {
#ifdef UNDER_CE
         time_t ret = 0;

         if( NULL == a_timer ) {
            // The FILETIME structure is defined like this
            // typedef struct _FILETIME { 
            //   DWORD dwLowDateTime; 
            //   DWORD dwHighDateTime; 
            // } FILETIME;
            // it contains the number of 100-nanosecond intervals
            // since January 1st, 1601.
            FILETIME   fileTime;
            // The SYSTEMTIME struct is defined like this: 
            // typedef struct _SYSTEMTIME { 
            //   WORD wYear; 
            //   WORD wMonth; 
            //   WORD wDayOfWeek; 
            //   WORD wDay; 
            //   WORD wHour; 
            //   WORD wMinute; 
            //   WORD wSecond; 
            //   WORD wMilliseconds; 
            // } SYSTEMTIME;
            SYSTEMTIME sysTime = {1970, 1, 0, 1, 0, 0, 0, 0}; //1970/01/01 

            // Get the time sinze 1970 and turn it int a 64 bit integer.
            SystemTimeToFileTime(&sysTime, &fileTime);
            ULARGE_INTEGER t1970 = ToULARGE(fileTime); 

            // Get the time since 1601 and turn it into a 64 bit integer.
            GetSystemTime( &sysTime );
            SystemTimeToFileTime( &sysTime, &fileTime );
            ULARGE_INTEGER now = ToULARGE(fileTime);

            // Calculate the distance from 1970 to now.
            uint64 diff = now - t1970;
            // Calculate from 100 nanoseconds to seconds.
            diff /= 10000000;
            ret = time_t(diff);
         } else {
            // a_timer <> NULL isn't handled at this point in time.
            ret = -1;
         }

         return ret;
#else
         return ::time(a_timer);
#endif   // #ifdef _WIN32
      }
   };
}

inline
isab::TimeUtils::PrecisionTime::PrecisionTime() {
#ifdef __SYMBIAN32__
   // User::TickCount(); not good system dependent size of tick
   TTime now;
   now.HomeTime(); // or UniversalTime
   TDateTime nowDate = now.DateTime();
   TTime timeMidNight = now;
   timeMidNight -= TTimeIntervalHours( nowDate.Hour() );
   timeMidNight -= TTimeIntervalMinutes( nowDate.Minute() );
   timeMidNight -= TTimeIntervalSeconds( nowDate.Second() );
   timeMidNight -= TTimeIntervalMicroSeconds( nowDate.MicroSecond() );
   TTimeIntervalMicroSeconds micros = now.MicroSecondsFrom( timeMidNight );
   seconds = LOW(micros.Int64() / 1000000);
   microseconds = LOW( micros.Int64() - TInt(seconds) * 1000000 );
#elif _MSC_VER
   // Windows version from MC2
   // Not really precision time.
   struct _SYSTEMTIME tv;
   ::GetSystemTime(&tv);
   uint32 millis = ::GetTickCount();
   seconds = millis /1000;
   microseconds = (millis - seconds * 1000) * 1000;
#else
   // use std method - it is possible that Symbian supports this too.
   struct ::timeval tv;
   struct ::timezone tz;
   gettimeofday(&tv, &tz);
   seconds = tv.tv_sec;
   microseconds = tv.tv_usec;
#endif
}

inline uint32
isab::TimeUtils::millis() {
#ifdef __SYMBIAN32__
   TTime now;
   now.HomeTime(); // or UniversalTime
   TDateTime nowDate = now.DateTime();
   TTime timeMidNight = now;
   timeMidNight -= TTimeIntervalHours( nowDate.Hour() );
   timeMidNight -= TTimeIntervalMinutes( nowDate.Minute() );
   timeMidNight -= TTimeIntervalSeconds( nowDate.Second() );
   timeMidNight -= TTimeIntervalMicroSeconds( nowDate.MicroSecond() );
   TTimeIntervalMicroSeconds micros = now.MicroSecondsFrom( timeMidNight );
   return LOW(micros.Int64() / 1000);
#elif _MSC_VER
   return ::GetTickCount();
#else
   time_t now = time(NULL);
   struct tm broken = {0};
   localtime_r(&now, &broken);
   broken.tm_sec  = 0;         /* seconds */
   broken.tm_min  = 0;         /* minutes */
   broken.tm_hour = 0;        /* hours */
   time_t midnight = mktime(&broken);   
   // Standard method
   struct ::timeval tv;
   ::gettimeofday(&tv, NULL);
   tv.tv_sec -= midnight;
   return (tv.tv_usec/1000 + tv.tv_sec * 1000);
#endif
}

inline
isab::TimeUtils::PrecisionTime
isab::TimeUtils::micros(){
  PrecisionTime ret;
  return ret;
}



#endif
