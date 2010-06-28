/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SDP_REPEATER_H
#define SDP_REPEATER_H

#include <bttypes.h>

#include "ActiveLog.h"
#include "Completer.h"
#include "TimeOutNotify.h"

class TSdpRepeatSettings
{
public:
   /** The number of connection attempts if not explicitly set.*/
   static const TInt KDefaultConnectionAttempts;
   /**
    * The time interval between two connection attempts unless
    * explicitly set. Microseconds. 
    */
   static const TInt KDefaultAttemptPeriod;
   /**
    * The indicator that we should keep retrying forever. 
    */
   static const TInt KInfiniteRepeats;
   /**
    * The shortest allowed interval. 
    */
   static const TInt KMinAttemptPeriod;

   /**
    * Default constructor. Uses the default number of connection
    * attempts and period.
    */
   TSdpRepeatSettings();
   
   /**
    * Constructor that sets the number of connection attempts and the
    * period.
    * @param aRepeats The number of connection attempts.
    * @param aPeriod The time interval between two intervals in microseconds. 
    */
   TSdpRepeatSettings(TInt aRepeats, 
                      const class TTimeIntervalMicroSeconds32& aPeriod);

   /**
    * Constructor that sets the total search time as well as the
    * interval between two searches. The total time is converted into
    * the number of attempts by deviding the total time by the attempt
    * interval. This will mean that the actual search time will be
    * longer since the connection attempts are not instantaneus.
    *
    * @param aTotalTime The total search time in minutes. 
    * @param aPeriod The time interval between two intervals in seconds. 
    */
   TSdpRepeatSettings(const class TTimeIntervalMinutes& aTotalTime, 
                      const class TTimeIntervalSeconds& aPeriod);
   /**
    * Constructor that sets unlimited total search time and the
    * interval between two searches. 
    *
    * @param aTotalTime The total search time in minutes. 
    * @param aPeriod The time interval between two intervals in seconds. 
    */
   explicit TSdpRepeatSettings(const class TTimeIntervalSeconds& aPeriod);
   TInt iRepeats;
   class TTimeIntervalMicroSeconds32 iPeriod;
};


class CSdpRepeater : public CActiveLog, 
                     public MCompleter, 
                     public MTimeOutNotify
{
   //no copy construction
   CSdpRepeater(const class CSdpRepeater&);
   //no assigmnet operator
   class CSdpRepeater& operator=(const class CSdpRepeater&);
   /** @name Constructors and destructor. */
   //@{
   CSdpRepeater(class MSdpRepeaterObserver& aObserver,
                const class TBTDevAddr& aAddress);
   void ConstructL();
public:
   static class CSdpRepeater* NewL(class MSdpRepeaterObserver& aObserver,
                                   const class TBTDevAddr& aAddress);
   /** Virtual destructor. */
   virtual ~CSdpRepeater();
   //@}
   void FindSerialPortL(class TRequestStatus* aStatus);
   void CancelFind();
   TInt Port();

   void SetPeriod(class TTimeIntervalMicroSeconds32 aPeriod);
   class TTimeIntervalMicroSeconds32 GetPeriod() const;

   void SetTotalAttempts(TInt aAttempts);
   TInt GetTotalAttempts() const;

   void SetRepeatSettings(const class TSdpRepeatSettings& aSettings);

   const class TBTDevAddr& GetAddress() const;

private:
   void CreateAndStartExaminerL();

   /** @name From CActive. */
   //@{
   virtual void RunL();
   virtual void DoCancel();
   //@}
   /** @name From CActiveLog. */
   //@{
   virtual CArrayPtr<CActiveLog>* SubLogArrayLC();
   //@}

   /** @name From MTimeOutNotify. */
   //@{
   virtual void TimerExpired();
   //@}

   class MSdpRepeaterObserver& iObserver;
   class CSdpExaminer* iExaminer; 
   class CTimeOutTimer* iTimer;
   class TSdpRepeatSettings iRepeatSettings;
   TInt iRepeatCount;
   TBool iIsRunning;
   const class TBTDevAddr iAddress;
};

#endif
