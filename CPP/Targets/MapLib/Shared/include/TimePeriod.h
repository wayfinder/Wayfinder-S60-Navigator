/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _TIMEPERIOD_H_
#define _TIMEPERIOD_H_

#include "config.h"

class TimePeriod {
public:
   /**
    *   The beginning and end of this interval.
    */
   
   unsigned int start, end;

   /**
    *   Creates a new interval, with the default
    *   state set to not valid.
    */
   
   TimePeriod() {
      start = end = MAX_INT32;
   }

   /**
    *   Updates beginning and end explicitly
    */ 
   
   void updatePeriod( unsigned int _start, unsigned int _end ) {
      start = _start;
      end = _end;
   }

   /**
    *   Updates the period by setting the second time
    *   to the argument, and the first time to the previous
    *   time. This gives us sequentially following time periods.
    */
   
   void updatePeriod( unsigned int _end ) {
      start = end;
      end = _end;
   }
   
   /**
    *   Preconditions for being valid:
    *
    *   1) We must have two valid times.
    *   2) The latter of the two times must be later (larger )
    *      than the former.
    */ 
   
   bool isValid() const {
      return
         start != MAX_INT32 &&
         end != MAX_INT32 &&
         end > start;
   }

   /**
    *   Returns true if the supplied time is within the
    *   period defined by the current state.
    */
   
   bool withinPeriod( unsigned int time ) const {
      return time >= start && time < end;
   }

   float64 normalizedTime( unsigned int time ) const {
      if( !isValid() || !withinPeriod( time ) ) {
         return 0.0f;
      }

      return static_cast<float>( (time - start ) ) / length();
   }
   
   /**
    *   Returns the current period, defined as the difference
    *   between the two internal times. If the internal state
    *   is not valid, a period of 0 is returned.
    */ 
   
   unsigned int length() const {
      if( !isValid() ) {
         return 0;
      }
      
      return end - start;
   }
};


#endif /* _TIMEPERIOD_H_ */
