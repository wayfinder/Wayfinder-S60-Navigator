/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ERROR_FILTER_H
#define ERROR_FILTER_H

#include "arch.h"

/**
 * Class that controls wether a error should be displayed
 * or not.
 */
class ErrorFilter {
   
 public: 
   ErrorFilter(uint32 timeBetweenShownErrors, 
               uint32 maxReproducedErrorTim);
   /**
    * Controls if a reported error should be displayed
    * for the user or not.
    * @return true if message should be displayed.
    *         false if not.
    */
   virtual bool displayErrorMessage();

   /**
    * Reset the parameters that holds the last time
    * error was shown and received.
    */
   void resetParameters();

   /**
    * Sets the parameters that controls when an
    * error should be displayed.
    */
   void setControlParameters(uint32 timeBetweenShownErrors, 
                             uint32 maxReproducedErrorTime);

 protected:
   // holds the time in millsek when the last error was displayed.
   uint32 m_lastTimeErrorShown;
   // holds the time in millsek when the last error was received.
   uint32 m_lastTimeErrorReceived;
   // holds the number of errors we have received.
   uint32 m_nbrOfErrorsReceived;
   // minimum time between errors. errors that is reported before
   // this time has passed will not be displayed. 
   uint32 m_minTimeBetweenShownErrors;
   // time which some errors needs to be repeated within otherwise they
   // will not be shown to user.
   uint32 m_maxReproducedErrorTime;
   // The number of errors we need to receive before displaying them.
   uint32 m_minNbrOfErrorsToReceive;
};


#endif
