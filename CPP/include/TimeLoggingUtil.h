/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef _TIME_LOGGING_UTIL_H_
#define _TIME_LOGGING_UTIL_H_

#include "arch.h"

#include <vector>

/**
 * Class for meassuring delays in Nav2.
 *
 * Usage:
 * Instantiate a new object in constructor and add a timer 
 * with AddTimer() function for each delay you want to meassure.
 * Then start the timer with StartTimer() and stop it with 
 * StopTimer() for each timer you added. The meassured times 
 * are then written to file when the object is destroyed.
 */

namespace isab 
{
   class TimeLoggingUtil {
   public:

      /**
       * Constructor.
       *
       * @param aName         The filename to write timings to.
       * @param aAppendData   Flag wether we want to append data
       *                      to file or overwrite it.
       */
      TimeLoggingUtil(char* aName = NULL, bool aAppendData = false);

      /**
       * Destructor that also writes the timings to file.
       */
      ~TimeLoggingUtil();

      /**
       * Adds a timer.
       *
       * @param aId     The ID of the timer, of not provided the 
       *                ID gets auto incremented.
       * @param aName   The name (optional) of the timer.
       * @return        The ID of the timer.
       */
      int32 AddTimer(int aId = -1, char* aName = NULL);

      /**
       * Sets the start time of a timer.
       *
       * @param aId     The ID of the timer to start.
       */
      void StartTimer(int aId);

      /**
       * Sets the stop time of a timer and stores the difference 
       * between start time and stop time.
       *
       * @param aId     The ID of the timer to stop.
       */
      void StopTimer(int aId);

      /**
       * Checks if a timer already exists.
       *
       * @param aId     The ID of the timer that might exist.
       */
      bool ExistsTimer(int aId);

   private:

      /**
       * Struct that contains all the necessary information 
       * to meassure different timings.
       */
      struct TimingUnit {
         int id;
         char name[32];
         uint32 startTime;
         uint32 stopTime;
         std::vector<float> timeSpent;
      };
      
      /**
       * Function that writes all the timings to file.
       *
       * @return        Success of not.
       */
      int WriteTimingLog();

      std::vector<TimingUnit> m_timingUnits;
      char m_fileName[128];
      bool m_appendData;

   }; // End class TimeLoggingUtil

} // End namespace isab

#endif
