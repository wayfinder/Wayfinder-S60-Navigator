/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "ActiveDelay.h"

CActiveDelayBase::~CActiveDelayBase() 
{
   Cancel();
}

CActiveDelayBase::CActiveDelayBase(class TTimeIntervalSeconds aSeconds) : 
   CTimer(EPriorityStandard), iDelayState(EIdle), 
   iDelay(aSeconds.Int() * 1000000)
{}

void CActiveDelayBase::ConstructL()
{
   CActiveScheduler::Add(this);
   CTimer::ConstructL();
}

TInt CActiveDelayBase::BaseStart(class TRequestStatus* aStatus)
{
   Activate(aStatus);
   After(iDelay);
   iDelayState = EWaiting;
   return KErrNone;
}   

void CActiveDelayBase::BaseCancelDelayedCall()
{
   if(IsActive()){
      if(iDelayState == EWaiting){
         Cancel();
      } else if(iDelayState == ERunning){
         CancelRealCall();
      }
   }
}

void CActiveDelayBase::DoCancel()
{
   if(iDelayState == EWaiting){
      CTimer::DoCancel();
      iDelayState = EIdle;
   } else if(iDelayState == ERunning){
      CancelRealCall();
   }
}

void CActiveDelayBase::RunL()
{
   switch(iDelayState){
   case EWaiting:
      if(iStatus == KErrNone){
         StartRealCall();
         iDelayState = ERunning;
      } else {
         Complete(iStatus);
         iDelayState = EIdle;
      }
      break;
   case ERunning:
      Complete(iStatus);
      iDelayState = EIdle;
      break;
   default:
      User::Panic(_L("CActiveDelay"), 0);
   }
}

_LIT(KActiveDelay, "CActiveDelay");

void CActiveDelayBase::Panic(enum TActiveDelayPanics aPanic)
{
   User::Panic(KActiveDelay, aPanic);
}
