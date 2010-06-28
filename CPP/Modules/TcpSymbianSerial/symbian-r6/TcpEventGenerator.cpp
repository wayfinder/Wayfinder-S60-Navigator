/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TcpEventGenerator.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

void CTcpEventGenerator::ClearBool(TAny* aAny)
{
   TBool* iBool = static_cast<TBool*>(aAny);
   *iBool = EFalse;
}

void CTcpEventGenerator::SetBool(TAny* aAny)
{
   TBool* iBool = static_cast<TBool*>(aAny);
   *iBool = ETrue;
}

void CTcpEventGenerator::PushAndSetRunningL()
{
   iIsRunning = ETrue;
   CleanupStack::PushL(TCleanupItem(ClearBool, &iIsRunning));
}   
   
void CTcpEventGenerator::PopAndClearRunning()
{
   CleanupStack::PopAndDestroy(&iIsRunning);
}

void CTcpEventGenerator::PushAndClearReadyL()
{
   iIsReady = EFalse;
   CleanupStack::PushL(TCleanupItem(SetBool, &iIsReady));
}

void CTcpEventGenerator::PopReady()
{
   CleanupStack::Pop(&iIsReady);
}
   
class CTcpEventGenerator* CTcpEventGenerator::NewL(MTcpEventHandler& aTarget)
{
   class CTcpEventGenerator* self = NewLC(aTarget);
   CleanupStack::Pop(self);
   return self;
}

class CTcpEventGenerator* CTcpEventGenerator::NewL(class MTcpEventHandler& aTarget, class TThreadId aThreadId, class isab::LogMaster* aLogMaster)
{
   class CTcpEventGenerator* self = NewLC(aTarget, aThreadId);
   if(aLogMaster){
      self->iLog = new (ELeave) isab::Log("TcpGenerator", isab::Log::LOG_ALL, aLogMaster);
   }
   CleanupStack::Pop(self);
   return self;
}

class CTcpEventGenerator* CTcpEventGenerator::NewLC(class MTcpEventHandler& aTarget)
{
   class CTcpEventGenerator* self = new CTcpEventGenerator(aTarget);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CTcpEventGenerator* CTcpEventGenerator::NewLC(class MTcpEventHandler& aTarget, class TThreadId aThreadId)
{
   class CTcpEventGenerator* self = new CTcpEventGenerator(aTarget);
   CleanupStack::PushL(self);
   self->ConstructL(aThreadId);
   return self;
}

CTcpEventGenerator::~CTcpEventGenerator()
{
   DBG("~CTcpEventGenerator");
   Cancel();
   iCritical.Close();
   iEventQueue.ResetAndDestroy();
   iEventQueue.Close();
   DBG("Destructor complete, deleting log");
   delete iLog;
   iLog = NULL;
}

CTcpEventGenerator::CTcpEventGenerator(class MTcpEventHandler& aTarget) :
   CActive(EPriorityStandard), iTarget(aTarget), 
   iEventQueue(KGranularity), iIsReady(ETrue)
{
}

void CTcpEventGenerator::ConstructCommonL()
{
   User::LeaveIfError(iCritical.CreateLocal());
   CActiveScheduler::Add(this);      
}

void CTcpEventGenerator::ConstructL()
{
   ConstructCommonL();
   iReceiverThread.Duplicate(RThread());
}

void CTcpEventGenerator::ConstructL(class TThreadId aThreadId)
{
   ConstructCommonL();
   iReceiverThread.Open(aThreadId);
}
   
void CTcpEventGenerator::Complete(TInt aStatus)
{
   iCritical.Wait();
   if(iIsReady && iEventQueue.Count() > 0 && !IsActive()){
      iIsReady = EFalse;
      DBG("Completing with %d", aStatus);
      iStatus = KRequestPending;
      SetActive();
      class TRequestStatus *tmpPtr = &iStatus;
      iReceiverThread.RequestComplete(tmpPtr, aStatus);
   } else {
      DBG("Ignoring Complete(%d) because: "
          "iIsReady: %d, Count(): %d, IsActive(): %d", aStatus, 
          int(!!iIsReady), iEventQueue.Count(), IsActive());
   }
   iCritical.Signal();
}

void CTcpEventGenerator::DoCancel()
{
}

TInt CTcpEventGenerator::RunError(TInt /*aError*/)
{
   return KErrNone;
}

void CTcpEventGenerator::RunL()
{
   DBG("CTcpEventGenerator::RunL %d", iStatus.Int());
   PushAndSetRunningL();
   PushAndClearReadyL();
   if(iStatus != KErrCancel){
      iCritical.Wait();
      class CTcpEvent* event = iEventQueue[0];
      iEventQueue.Remove(0);
      if(iEventQueue.Count() >= KGranularity){
         iEventQueue.Compress();
      }
      iCritical.Signal();
      CleanupStack::PushL(event);
      event->ProcessL(iTarget);
      CleanupStack::PopAndDestroy(event);
   }
   Complete(KErrNone);
   PopReady();
   PopAndClearRunning();
   DBG("CTcpEventGenerator::RunL end");
}
   
void CTcpEventGenerator::SetReady(TBool aReady)
{
   DBG("SetReady(%d): iIsRunning: %d, iIsReady: %d, IsActive(): %d", 
       int(!!aReady), int(!!iIsRunning), int(!!iIsReady), int(!!IsActive()));
   if(iIsReady && aReady){
      ERR("PANIC: TcpEventGenerator 0");
      User::Panic(_L("TcpEventGenerator"), 0);
   }
   iCritical.Wait();
   iIsReady = aReady;
   iCritical.Signal();
   DBG("Complete(KErrNone)");
   Complete(KErrNone); //checks if iIsReady is set
   DBG("SetReady done.");
}
   
TInt CTcpEventGenerator::SendEventL(const class CTcpEvent* aEvent){
   iCritical.Wait();
   TInt errcode = iEventQueue.Append(aEvent);
   TInt count = iEventQueue.Count();
   iCritical.Signal();
   User::LeaveIfError(errcode);
   if(!iIsRunning){
      Complete(KErrNone);
   }
   return count;
}

TInt CTcpEventGenerator::SendEvent(class CTcpEvent* aEvent)
{
   TRAPD(ret, SendEventL(aEvent));
   return ret;
}
