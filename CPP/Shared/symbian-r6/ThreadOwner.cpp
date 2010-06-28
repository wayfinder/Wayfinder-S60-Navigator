/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "ThreadOwner.h"

void CThreadOwner::SignalStartupOkFailed(TInt result, 
                                         CThreadOwner::ThreadData& threadData)
{
   // NOTE! There may not be a cleanup stack active here and the heap
   // may be full. Be very very careful.
   RThread otherThread;
   otherThread.Open(threadData.startupStatusThread);
   otherThread.RequestComplete(threadData.startupStatus, result);
   otherThread.Close();
}

TInt CThreadOwner::StartServerThreadFunc(TAny *aPtr)
{
   struct  ThreadData * threadData = static_cast<ThreadData *>(aPtr);
   class CTrapCleanup * cleanupStack = CTrapCleanup::New();
   if (NULL == cleanupStack) {
      // Out of memory - return an error message carefully since we 
      // have no cleanupstack.
      SignalStartupOkFailed(KErrNoMemory, *threadData);
      return KErrNoMemory;
   }

   class CThreadAdmin* admin = NULL;  
   class CStopWaiter * stopWaiter = NULL;
   
   TRAPD(result, StartServerThreadFuncStage2L(*threadData, &admin,
                                              &stopWaiter));
   if (result != KErrNone) {
      SignalStartupOkFailed(result, *threadData);
      return result;
   }

   //If we get here then the second stage must have signalled the parent thread
   // Start the active scheduler to process incoming requests.
   CActiveScheduler::Start();

   delete stopWaiter;
   delete admin;
   delete cleanupStack;
   return KErrNone;
}

void CThreadOwner::StartServerThreadFuncStage2L(class CThreadOwner::ThreadData & threadData,
                                                class CThreadAdmin ** admin,
                                                class CThreadOwner::CStopWaiter **stopWaiter)
{
   class CActiveScheduler *pA=new (ELeave) CActiveScheduler;
   CActiveScheduler::Install(pA);

   // Increase priority
   {
      class RThread thisThread;         // Not pushed on the cleanup stack
      thisThread.SetPriority(threadData.priority);
   }

   // Create the server
   //hook in stuff here!
   class CThreadAdmin* tmpAdmin = threadData.iFactory->CreateAdminLC();
   tmpAdmin->StartL();
   // Create the StopWaiter and share the data with the 
   // creating thread.
   class CStopWaiter* tmpWaiter = CStopWaiter::NewLC(*tmpAdmin, threadData);
   threadData.stopNowStatus = tmpWaiter->getStatusVar();
   {
      class RThread thisThread;
      threadData.stopNowStatusThread = thisThread.Id();
   }

   // Let the thread that started us know we are up and 
   // running.
   SignalStartupOkFailed(KErrNone, threadData);

   // Hand over ownership to the StartServerThreadFunc function.
   CleanupStack::Pop(tmpWaiter);
   (*stopWaiter) = tmpWaiter;
   CleanupStack::Pop(tmpAdmin);
   (*admin) = tmpAdmin;
}






// CThreadOwner::CStopWaiter helper class
CThreadOwner::CStopWaiter* 
CThreadOwner::CStopWaiter::NewLC(CThreadAdmin & aAdmin,
                                 CThreadOwner::ThreadData & threadData)
{
   CStopWaiter *ptr = new(ELeave) CStopWaiter(aAdmin, threadData);
   CleanupStack::PushL(ptr);
   return ptr;
}

CThreadOwner::CStopWaiter::~CStopWaiter()
{
   if ( IsActive() ) {
      Cancel();
   }
}

CThreadOwner::CStopWaiter::CStopWaiter(CThreadAdmin& aAdmin, 
                                       CThreadOwner::ThreadData & threadData)
   : CActive(EPriorityStandard), iAdmin(aAdmin)
{
   CActiveScheduler::Add(this);
   iStatus = KRequestPending;
   SetActive();

   threadData.stopNowStatus = &iStatus;
   RThread thisThread;
   threadData.stopNowStatusThread = thisThread.Id();
   thisThread.Close();
}

TRequestStatus * CThreadOwner::CStopWaiter::getStatusVar()
{
   return &iStatus;
}

void CThreadOwner::CStopWaiter::DoCancel()
{
   TRequestStatus* ptr = &iStatus;
   User::RequestComplete( ptr, KErrCancel );   
}

void CThreadOwner::CStopWaiter::RunL()
{
   if ( iStatus != KErrCancel ) {
      iAdmin.StopL();
   }
   CActiveScheduler::Stop();
}



CThreadOwner* CThreadOwner::NewLC(class CThreadAdminFactory* aFactory)
{
   CThreadOwner *ptr = new (ELeave) CThreadOwner();
   CleanupStack::PushL(ptr);
   ptr->ConstructL(aFactory);
   return ptr;
}

CThreadOwner* CThreadOwner::NewL(class CThreadAdminFactory* aFactory)
{
   CThreadOwner *ptr = NewLC(aFactory);
   CleanupStack::Pop(ptr);
   return ptr;
}

CThreadOwner* CThreadOwner::New(class CThreadAdminFactory* aFactory)
{
   ///XXX Install a cleanupstack.
   CThreadOwner *ptr = new CThreadOwner();
   if(ptr){
      TRAPD(res, ptr->ConstructL(aFactory));
      if(res != KErrNone){
         delete ptr;
         ptr = NULL;
      }
   }
   return ptr;
}


CThreadOwner::CThreadOwner()
   : CBase(),
     iThreadData(NULL)
{
}

void CThreadOwner::ConstructL(class CThreadAdminFactory* aFactory)
{
   class TRequestStatus serverStartupStatus;

   iThreadData = new (ELeave) ThreadData;

   iThreadData->iFactory = aFactory;
   iThreadData->serverName = aFactory->AllocNameL();
   iThreadData->priority = aFactory->ThreadPriority();
   iThreadData->startupStatus = &serverStartupStatus;
   class RThread thisThread;
   iThreadData->startupStatusThread = thisThread.Id();
   thisThread.Close();

   serverStartupStatus = KRequestPending;

   TInt result = KErrAlreadyExists;
   TInt threadNum = 0;
   while (result == KErrAlreadyExists){
      result = iServerThread.Create( iThreadData->serverName->Des(), 
                                     StartServerThreadFunc, 
                                     KDefaultStackSize,
                                     NULL,//RHeap, NULL means use parent thread heap
                                     static_cast<TAny *>(iThreadData));
      if(result == KErrAlreadyExists){
         HBufC* oldName = iThreadData->serverName;
         CleanupStack::PushL(oldName);
         iThreadData->serverName = NULL;
         HBufC* newName = HBufC::NewLC(oldName->Length() + 4);
         newName->Des().Format(_L("%S%d"), oldName, threadNum++);
         iThreadData->serverName = newName;
         CleanupStack::Pop(newName);
         CleanupStack::PopAndDestroy(oldName);
      }         
   }
   if (result != KErrNone) {
      delete iThreadData;
      iThreadData = NULL;
      User::Leave(result);
   }

   // Set up the server logon. This needs to be before the 
   // resume since the Logon() call signals oom through
   // the request status. If it is done after the Resume 
   // we can not know if a KErrNoMemory comes from the call
   // or the thread.
   iServerThread.Logon(iServerExitStatus);
   if (iServerExitStatus == KErrNoMemory) {
      // Not enough memory to logon to the thread. Kill
      // it and leave.
      iServerThread.Kill(KErrNoMemory);
      delete iThreadData;
      iThreadData = NULL;
      User::Leave(KErrNoMemory);
   }

   iServerThread.Resume();

   // Wait for the server to be established (or failed)
   User::WaitForRequest(serverStartupStatus);

   // Check if the server failed to start. The thread should be dead by 
   // now if so, deallocate it.
   if (serverStartupStatus != KErrNone) {
      // Reap the thread
      User::WaitForRequest(iServerExitStatus);
      delete iThreadData;
      iThreadData = NULL;
      User::Leave(serverStartupStatus.Int());
   }
}

CThreadOwner::~CThreadOwner()
{
   if (iThreadData) {
      // Stop the server

      // FIXME - race here. What if the server has died and we have not 
      // noticed yet? The threadData.stopNowStatus may be invalid. 
      class RThread otherThread;
      if (otherThread.Open(iThreadData->stopNowStatusThread) == KErrNone) {
         otherThread.RequestComplete(iThreadData->stopNowStatus, KErrNone);
      }
      otherThread.Close();
      
      // Wait for the thread to die
      User::WaitForRequest(iServerExitStatus);

      delete iThreadData;
   }
}

CThreadAdmin::~CThreadAdmin()
{
}

CThreadAdminFactory::~CThreadAdminFactory()
{
   delete iName;
}

