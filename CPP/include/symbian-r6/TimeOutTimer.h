/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TIMEOUTTIMER_H
#define TIMEOUTTIMER_H

#include <e32base.h>

//class MTimeOutNotify;
/*! 
  @class CTimeOutTimer
  
  @discussion This class will notify an object after a specified timeout.
  */
class CTimeOutTimer : public CTimer
{

public:

   /*!
    @function NewL
   
    @discussion Create a CTimeOutTimer object
    @param aPriority priority to use for this timer
    @param aTimeOutNotify object to notify of timeout event
    @result a pointer to the created instance of CTimeOutTimer
    */
   static CTimeOutTimer* NewL(const TInt aPriority, class MTimeOutNotify& aTimeOutNotify);

   /*!
    @function NewL
   
    @discussion Create a CTimeOutTimer object
    @param aPriority priority to use for this timer
    @param aTimeOutNotify object to notify of timeout event
    @result a pointer to the created instance of CTimeOutTimer
    */
   static CTimeOutTimer* NewLC(const TInt aPriority, class MTimeOutNotify& aTimeOutNotify);

   /*!
    @function ~CTimeOutTimer
  
    @discussion Destroy the object and release all memory objects
    */
   ~CTimeOutTimer();

protected: // From CTimer

   /*!
    @function RunL

    @discussion Invoked when a timeout occurs
    */
   virtual void RunL();

private:

   /*!
    @function CTimeOutTimer
  
    @discussion Perform the first phase of two phase construction 
    @param aPriority priority to use for this timer
    */
   CTimeOutTimer(const TInt aPriority, class MTimeOutNotify& aTimeOutNotify);

   /*!
    @function ConstructL
  
    @discussion Perform the second phase construction of a CTimeOutTimer 
    */
   void ConstructL();

private:

   // Member variables
   class MTimeOutNotify& iNotify;

};

#endif // TIMEOUTTIMER_H
