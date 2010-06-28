/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SdpRepeater.h"
#include "SdpExaminer.h"
#include "TimeOutTimer.h"
#include "SdpRepeaterObserver.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

const TInt TSdpRepeatSettings::KDefaultConnectionAttempts = 5;
const TInt TSdpRepeatSettings::KDefaultAttemptPeriod = 5*1000*1000;
const TInt TSdpRepeatSettings::KInfiniteRepeats = KMaxTInt;
const TInt TSdpRepeatSettings::KMinAttemptPeriod = 1*1000*1000;

TSdpRepeatSettings::TSdpRepeatSettings() :
   iRepeats(KDefaultConnectionAttempts), iPeriod(KDefaultAttemptPeriod)
{
}

TSdpRepeatSettings::TSdpRepeatSettings(TInt aRepeats, 
                                       const class TTimeIntervalMicroSeconds32& aPeriod) :
   iRepeats(aRepeats), iPeriod(Max(aPeriod.Int(), KMinAttemptPeriod))
{
}

TSdpRepeatSettings::TSdpRepeatSettings(const class TTimeIntervalMinutes& aTotalTime, 
                                       const class TTimeIntervalSeconds& aPeriod) : 
   iRepeats(KDefaultConnectionAttempts), iPeriod(KDefaultAttemptPeriod)
{
   iRepeats = (aTotalTime.Int() * 60) / aPeriod.Int();
   iPeriod = Max(aPeriod.Int() * 1000000, KMinAttemptPeriod);
}

TSdpRepeatSettings::TSdpRepeatSettings(const class TTimeIntervalSeconds& aPeriod) : 
   iRepeats(KInfiniteRepeats), iPeriod(KDefaultAttemptPeriod)
{
   iPeriod = Max(aPeriod.Int() * 1000000, KMinAttemptPeriod);
}

CSdpRepeater::CSdpRepeater(class MSdpRepeaterObserver& aObserver,
                           const class TBTDevAddr& aAddr) : 
   CActiveLog("SdpRepeater"), 
   iObserver(aObserver), 
   iRepeatCount(iRepeatSettings.iRepeats), 
   iAddress(aAddr)
{
}

void CSdpRepeater::ConstructL()
{
   CActiveScheduler::Add(this);
   iTimer = CTimeOutTimer::NewL(EPriorityStandard, *this);
}


class CSdpRepeater* CSdpRepeater::NewL(class MSdpRepeaterObserver& aObserver,
                                       const class TBTDevAddr& aAddress)
{
   class CSdpRepeater* self = new (ELeave) CSdpRepeater(aObserver, aAddress);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

CSdpRepeater::~CSdpRepeater()
{
   if(IsActive()){
      DBG("Destroyed while active. That sure sucks!");
      Cancel();
   }
   delete iExaminer;
   delete iTimer;
}

void CSdpRepeater::RunL()
{
   if(iStatus != KErrNone){
      DBG("Complete with iStatus == %d", iStatus.Int());
      if(iStatus == KErrHardwareNotAvailable){
         DBG("Hardware not available. Low battery or flight mode.");
         Complete(iStatus);
      } else if((iRepeatCount == TSdpRepeatSettings::KInfiniteRepeats) || 
                (iRepeatCount-- > 0)){ //not decremeted if inf.
         DBG("set timer since connect failed.");
         //XXX notify user
         iTimer->After(iRepeatSettings.iPeriod);
      } else {
         DBG("Out of retries, completeing with KErrNotFound");
         iIsRunning = EFalse;
         Complete(KErrNotFound);
      }
   } else {
      DBG("Found it, completing with KErrNone.");
      iIsRunning = EFalse;
      Complete(KErrNone);
   }
}

void CSdpRepeater::DoCancel()
{
   DBG("DoCancel");
   DBG("Completeing the request");
   Complete(KErrCancel);
   
   if(iExaminer){
      DBG("Canceling examiner");
      iExaminer->CancelFind();
   }
   delete iExaminer;
   iExaminer = NULL;
   if(iTimer && iTimer->IsActive()){
      DBG("Canceling timer");
      iTimer->Cancel();
   }
   delete iTimer;
   iTimer = NULL;
   DBG("DoCancel done");
}

void CSdpRepeater::CreateAndStartExaminerL()
{
   iObserver.SdpRepeatInfo(iRepeatSettings.iRepeats - iRepeatCount, 
                           iRepeatSettings.iRepeats);
   //new SdpExaminer
   delete iExaminer;
   iExaminer = NULL;
   iExaminer = CSdpExaminer::NewL(iAddress);
   UpdateLogMastersL();
   
   //start the examiner 
   iExaminer->FindSerialPortL(&iStatus);
   SetActive();
}

void CSdpRepeater::FindSerialPortL(class TRequestStatus* aStatus)
{
   if(! iIsRunning){
      //reset repeat counter
      iRepeatCount = iRepeatSettings.iRepeats; 

      CreateAndStartExaminerL();

      //set status
      Activate(aStatus);
      iIsRunning = ETrue;
      DBG("A search has been started.");
   } else {
      WARN("FindSerialPortL called while running.");
   }
}

void CSdpRepeater::CancelFind()
{
   Complete(KErrCancel);

}

TInt CSdpRepeater::Port()
{
   return iExaminer->Port();
}

void CSdpRepeater::SetPeriod(class TTimeIntervalMicroSeconds32 aPeriod)
{
   SetRepeatSettings(TSdpRepeatSettings(GetTotalAttempts(), aPeriod));
}

class TTimeIntervalMicroSeconds32 CSdpRepeater::GetPeriod() const
{
   return iRepeatSettings.iPeriod;
}

void CSdpRepeater::SetTotalAttempts(TInt aAttempts)
{
   SetRepeatSettings(TSdpRepeatSettings(aAttempts, GetPeriod()));
}

TInt CSdpRepeater::GetTotalAttempts() const
{
   return iRepeatSettings.iRepeats;
}

void CSdpRepeater::SetRepeatSettings(const class TSdpRepeatSettings& aSettings)
{
   iRepeatSettings = aSettings;
   iRepeatCount = iRepeatSettings.iRepeats;
}

const class TBTDevAddr& CSdpRepeater::GetAddress() const
{
   return iAddress;
}

CArrayPtr<CActiveLog>* CSdpRepeater::SubLogArrayLC()
{
   CArrayPtr<CActiveLog>* array = new (ELeave) CArrayPtrFlat<CActiveLog>(2);
   CleanupStack::PushL(array);
   array->AppendL(iExaminer);
   return array;
}

void CSdpRepeater::TimerExpired()
{
   DBG("TimerExpired");
   CreateAndStartExaminerL();
}

