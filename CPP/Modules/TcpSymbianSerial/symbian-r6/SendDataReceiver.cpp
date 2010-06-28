/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SendDataReceiver.h"
#include <badesca.h>
#include "TcpAdmin.h"
#include "CleanupSupport.h"

namespace{
   _LIT(KSendDataRecv, "SendDataReceiver");
   enum TSendDataRecvPanic{
      EWrongThread,
   };
}
#define ASSERT_IS_THREAD(thread, panic) ((IsThread(thread))||(User::Panic(KSendDataRecv, panic),0))
#define ASSERT_NOT_THREAD(thread, panic) ((!IsThread(thread))||(User::Panic(KSendDataRecv, panic),0))

CSendDataReceiver::CSendDataReceiver(class CTcpAdmin* aAdmin,
                                     class TThreadId aThreadId) : 
   CActive(EPriorityStandard), iAdmin(aAdmin), iThreadId(aThreadId)
{
}

void CSendDataReceiver::ConstructL()
{
   iQueue = new (ELeave) CArrayPtrSeg<HBufC8>(8);
   User::LeaveIfError(iCritical.CreateLocal());
   CActiveScheduler::Add(this);
   RequestData();
}

class CSendDataReceiver* CSendDataReceiver::NewL(class CTcpAdmin* aAdmin,
                                                 class TThreadId aThreadId)
{
   class CSendDataReceiver* self = 
      new (ELeave) CSendDataReceiver(aAdmin, aThreadId);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

CSendDataReceiver::~CSendDataReceiver()
{
   if(IsActive()){
      Cancel();
   }
   iQueue->ResetAndDestroy();
   delete iQueue;
   iCritical.Close();
}

void CSendDataReceiver::RequestData()
{
   ASSERT_IS_THREAD(iThreadId, EWrongThread);
   iCritical.Wait();
   if(!IsActive()){
      iStatus = KRequestPending;
      SetActive();
   }
   if(iQueue->Count() > 0){
      class TRequestStatus* myStatus = &iStatus;
      User::RequestComplete(myStatus, KErrNone);
   }
   iCritical.Signal();
}

TInt CSendDataReceiver::SendData(const TUint8* aData, TUint aLength)
{
   TRAPD(err, SendDataL(aData, aLength));
   return err;
}

void CSendDataReceiver::SendDataL(const TUint8* data, TUint length)
{
   ASSERT_NOT_THREAD(iThreadId, EWrongThread);
   iCritical.Wait();
   CleanupSignalPushL(iCritical);

   TPtrC8 ptr(data, length);
   ptr.Set(data, length);
   HBufC8* buf = ptr.AllocLC();
   iQueue->AppendL(buf);
   CleanupStack::Pop(buf);

   if(IsActive()){
      ///Allow completion? Check that the object is waiting.
      class RThread otherThread;
      otherThread.Open(iThreadId);
      class TRequestStatus* status = &iStatus;
      otherThread.RequestComplete(status, KErrNone);
      otherThread.Close();
   }

   CleanupStack::PopAndDestroy(/*iCritical*/);
}

void CSendDataReceiver::RunL()
{
   iCritical.Wait(); //since this function never leaves we do not have
                     //to leave handle the critical region.

   if(iStatus == KErrNone){
      if(iQueue->Count() > 0){
         HBufC8* front = iQueue->At(0);
         iAdmin->WriteToTcp(front);
         iQueue->Delete(0);
      }
   } else {
      //XXX
      RequestData(); //will this deadlock?      
   }

   iCritical.Signal();
}

void CSendDataReceiver::DoCancel()
{
   iCritical.Wait();
   class TRequestStatus* tmp = &iStatus;
   User::RequestComplete(tmp, KErrCancel);
   iCritical.Signal();
}




