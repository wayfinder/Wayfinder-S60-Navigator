/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "TimeOutErrorFilter.h"
#include "TileMapUtil.h"

TimeOutErrorFilter::TimeOutErrorFilter(uint32 timeBetweenShownErrors, 
                                       uint32 maxReproducedErrorTime)
                                       : ErrorFilter(timeBetweenShownErrors, 
                                                     maxReproducedErrorTime)
{
   m_minNbrOfErrorsToReceive = 0;	
}

bool TimeOutErrorFilter::displayErrorMessage() 
{
   if ((TileMapUtil::currentTimeMillis() - m_lastTimeErrorShown)  
       < m_minTimeBetweenShownErrors) {
      // Not long enough since last shown error message.
      return false; 
   } else if ((TileMapUtil::currentTimeMillis() - m_lastTimeErrorReceived)
              < m_maxReproducedErrorTime) { 
      // If it was less than ERROR_MAX_REPEAT_TIME since we received
      // the last error then show the error message.
      m_nbrOfErrorsReceived++;
      m_lastTimeErrorReceived = TileMapUtil::currentTimeMillis();
      m_lastTimeErrorShown    = TileMapUtil::currentTimeMillis();
      return true;
   } else if (m_nbrOfErrorsReceived <= m_minNbrOfErrorsToReceive) {
      // Show errors until we have received too many
      m_nbrOfErrorsReceived++;
      m_lastTimeErrorReceived = TileMapUtil::currentTimeMillis();
      m_lastTimeErrorShown    = TileMapUtil::currentTimeMillis();
      return true;
   } else {
      // It was more than sec since we received last error
      // we do not want to show the error to the user.
      m_lastTimeErrorReceived = TileMapUtil::currentTimeMillis();
      return false;
   }
}
