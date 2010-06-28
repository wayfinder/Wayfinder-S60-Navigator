/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * Attach to a serial connection and decode Nmea gps data
 */

#include "arch.h"
#include <math.h>

#ifndef M_PI
#define M_PI      3.14159265358979323846
#endif

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "Gps.h"
#include "Log.h"
#include "GpsCommon.h"

#include "GpsParser.h"

namespace isab {


isabTime GpsParser::time_from_gps(int /*weekno*/, float /*tow*/)
{
   isabTime t;
   return t;
}


/* ********************************************************
 * Setup routines
 * ********************************************************/


GpsParser::GpsParser(SerialProviderPublic *p, Log *l)
   : m_serial(p), m_consumer(NULL), m_log(l)
{
}

GpsParser::~GpsParser() {}

int32 GpsParser::readNDigitsInt(char *s, int N)
{
   int32 sum=0;
   bool negative = false;

   if (*s == '-') {
      negative = true;
      ++s;
      --N;
   }

   while ( (N>0) && (*s>='0' && *s<='9') ) {
      sum = sum * 10 + (*s - '0');
      ++s;
      --N;
   }
   if (negative) {
      sum = -sum;
   }
   return sum;
}

int32 GpsParser::readInt32(char *s)
{
   int32 sum=0;
   bool negative = false;
   char *orgstr = s;

   if (*s == '-') {
      negative = true;
      ++s;
   }

   while ( *s >= '0' && *s <='9' ) {
      sum = sum * 10 + (*s - '0');
      ++s;
   }
   if (negative) {
      sum = -sum;
   }

   if (orgstr == s) {
      /* Nothing read. */
      return 0;
   }
   return sum;
}

float GpsParser::readFloat(char *s)
{
   int32 sum=0;
   uint32 divisor=1;

   // Read the integral part.
   while ( *s >= '0' && *s <='9' ) {
      sum = sum * 10 + (*s - '0');
      ++s;
   }

   // If there is a decimal point, skip it and read the fractional part.
   if ( *s == '.' ) {
      ++s;
      while ( *s >= '0' && *s <='9' ) {
         sum = sum * 10 + (*s - '0');
         divisor *= 10;
         ++s;
      }
   }

   if ( divisor > 1 ) {
      return (float(sum) / divisor);
   } else {
      return float(sum);
   }
}




} /* namespace isab */

