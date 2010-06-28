/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TimeLoggingUtil.h"
#include "TimeUtils.h"

#include <algorithm>

#define TIME_LOGGING_FILENAME "c:\\system\\data\\timelog.txt"

namespace isab
{

   TimeLoggingUtil::TimeLoggingUtil(char* aName, bool aAppendData)
   { 
      if (aName != NULL) {
         strcpy(m_fileName, aName);
      } else {
         strcpy(m_fileName, TIME_LOGGING_FILENAME);
      }
      m_appendData = aAppendData;
   }

   TimeLoggingUtil::~TimeLoggingUtil()
   {
      WriteTimingLog();
   }
   
   int32 TimeLoggingUtil::AddTimer(int aId, char* aName)
   {
      int32 id = 0;
      if (aId == -1) {
         id = m_timingUnits.back().id + 1;
      } else {
         id = aId;
      }
      TimingUnit timingUnit;
      timingUnit.id = id;
      if (aName != NULL) {
         strcpy(timingUnit.name, aName);
      } else {
         strcpy(timingUnit.name, "");
      }
      timingUnit.startTime = 0;
      timingUnit.stopTime = 0;

      m_timingUnits.push_back(timingUnit);

      return id;
   }

   void TimeLoggingUtil::StartTimer(int aId)
   {
      std::vector<TimingUnit>::iterator it = m_timingUnits.begin();
      while (it != m_timingUnits.end()) {
         if (it->id == aId) {
            it->startTime = TimeUtils::millis();
         }
         it++;
      }
   }
   
   void TimeLoggingUtil::StopTimer(int aId)
   {
      std::vector<TimingUnit>::iterator it = m_timingUnits.begin();
      while (it != m_timingUnits.end()) {
         if (it->id == aId) {
            it->stopTime = TimeUtils::millis();
            it->timeSpent.push_back(float((it->stopTime - it->startTime)) / 1000);
         }
         it++;
      }
   }

   bool TimeLoggingUtil::ExistsTimer(int aId)
   {
      std::vector<TimingUnit>::iterator it = m_timingUnits.begin();
      while (it != m_timingUnits.end()) {
         if (it->id == aId) {
            return true;
         }
         it++;
      }
      return false;
   }
   
   int TimeLoggingUtil::WriteTimingLog()
   {
      FILE* pSaveFile;
      if (m_appendData) {
         pSaveFile = fopen(m_fileName, "at");
      } else {
         pSaveFile = fopen(m_fileName, "wt");
      }

      if (pSaveFile != NULL) {
         std::vector<TimingUnit>::iterator it = m_timingUnits.begin();
         while (it != m_timingUnits.end()) {
            std::vector<float>::iterator timings_it = it->timeSpent.begin();
            while (timings_it != it->timeSpent.end()) {
               fprintf(pSaveFile, "ID: %-d; NAME: [%-16.16s]; TIME: %5.3f sec;\n", 
                       it->id, 
                       it->name, 
                       *timings_it);
               timings_it++;
            }
            it++;
         }
         fclose(pSaveFile);
         return 0;
      } else {
         return -1;
      }
   }

}
