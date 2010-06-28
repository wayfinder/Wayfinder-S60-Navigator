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

#include "AppHandler.h"   //CAppHandler
#include <e32base.h>      //CBase 
#include <e32std.h>       //TDesC
#include <eikenv.h>       //CEikonEnv
#include <apgtask.h>      //TApaTask
#include <apgcli.h>       //RApaLsSession
#include "AppObserver.h"  //MAppObserver
#include "TimeOutTimer.h" //CTimeOutTimer


#define KTimeOut (3*1000*1000)

CAppHandler::CAppHandler(class MAppObserver* aAppObserver, TInt aLength) :
   iAppObserver(aAppObserver), 
   iNumAppObjs(aLength)
{
}


void CAppHandler::ConstructL(struct TAppObject** aAppList)
{
   SetAppListL(aAppList, iNumAppObjs);
   iIndex = 0;
   iNbrOfRetries = 3;
   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);
   iGenerator = CAppEventGenerator::NewL(*this);
}

void CAppHandler::ConstructL()
{
   iIndex = 0;
   iNumAppObjs = 0;
   iNbrOfRetries = 3;
   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);
   iGenerator = CAppEventGenerator::NewL(*this);
}

CAppHandler::~CAppHandler()
{
   for(TInt i = 0; i < iNumAppObjs; ++i) {
      delete iAppList[i];
   }
   delete[] iAppList;
   delete iGenerator;
   delete iTimer;
}

class CAppHandler* CAppHandler::NewLC(class MAppObserver* aAppObserver)
{
   class CAppHandler* self = new (ELeave) CAppHandler(aAppObserver, 0);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CAppHandler* CAppHandler::NewLC(class MAppObserver* aAppObserver,
                                      struct TAppObject** aAppList,
                                      TInt aLength)
{
   class CAppHandler* self = new (ELeave) CAppHandler(aAppObserver, aLength);
   CleanupStack::PushL(self);
   self->ConstructL(aAppList);
   return self;
}

class CAppHandler* CAppHandler::NewL(class MAppObserver* aAppObserver)
{
   class CAppHandler* self = CAppHandler::NewLC(aAppObserver);
   CleanupStack::Pop(self);
   return self;
}

class CAppHandler* CAppHandler::NewL(class MAppObserver* aAppObserver,
                                     struct TAppObject** aAppList,
                                     TInt aLength)
{
   class CAppHandler* self = CAppHandler::NewLC(aAppObserver, 
                                                aAppList, 
                                                aLength);
   CleanupStack::Pop(self);
   return self;
}

void CAppHandler::SetAppListL(struct TAppObject** aAppList, TInt aLength)
{
   // Size of the array
   iNumAppObjs = aLength;
   if(iAppList) {
      delete[] iAppList;
      iAppList = NULL;
   }
   // Allocate necessary space
   iAppList = new(ELeave) TAppObject*[iNumAppObjs];

   // Build a list of installed applications.
   RApaLsSession apaSession;
   User::LeaveIfError(apaSession.Connect());
   CleanupClosePushL(apaSession);
   TInt ret = apaSession.GetAllApps();
   if(ret != KErrNone) {
      GenerateEventL(EFailedToInitList);
      return;
   }
   TApaAppInfo appInfo;
   TInt appIndex = 0;
   while (apaSession.GetNextApp(appInfo)==KErrNone) {
      for(TInt i = 0; i < iNumAppObjs; ++i) {
         if(appInfo.iFullName.CompareF(*aAppList[i]->appName) == 0) {
            // We found a matching app, add to our list
            TAppObject* appObj = new(ELeave) TAppObject();
            appObj->appName   = appInfo.iFullName.AllocL();
            appObj->appUid    = appInfo.iUid.iUid;
            appObj->appAction = aAppList[i]->appAction;
            appObj->pendingAction = ENonePending;
            iAppList[appIndex] = appObj;
            appIndex++;
         }
      }
   }
   iNumAppObjs = appIndex;
   CleanupStack::PopAndDestroy(&apaSession);
}

void CAppHandler::ProcessAppListL()
{
   if (!iRunning) {
      iIndex = 0;
      iEndTaskIndex = 0;
      iNbrOfRetries = 3;
      iRunning = ETrue;
      GenerateEventL(EStartProcessingApp);
   }
}

void CAppHandler::HandleGeneratedEventL(enum TAppProcessEvent aEvent)
{
   switch(aEvent) {
   case EStartProcessingApp:
      // Check if app is running.
      ProcessActions();
      break;
   case EAppIsRunning:
      // We should notify obeserver.
      NotifyObserver();
      break;
   case ECheckPendingActions:
      CheckPendingActions();
      break;
   case EFailedToEndTask:
      // Timer has expired three times but the
      // application is still running. Notify
      // the observer and quit.
      NotifyObserverEndTaskFailed();
      break;
   case EFinishedProcessingList:
      iRunning = EFalse;
      iAppObserver->FinishedProcessingList();
      break;
   case EFailedToInitList:
      iAppObserver->ListInitFailed();
      break;
   }
}

void CAppHandler::GenerateEventL(enum TAppProcessEvent aEvent)
{
   iGenerator->SendEventL(aEvent);
}

void CAppHandler::NotifyObserver()
{
   iAppObserver->AppIsRunning(*iAppList[iIndex]->appName);
}

void CAppHandler::NotifyObserverEndTaskFailed()
{
   iAppObserver->EndTaskFailed(*iAppList[iEndTaskIndex]->appName);
}

void CAppHandler::CheckPendingActions() 
{
   class TApaTaskList taskList(CEikonEnv::Static()->WsSession());
   class TUid uid; 
   struct TAppObject* appObj;
   for(TInt i = 0; i < iNumAppObjs; ++i) {
      appObj = iAppList[i];
      uid    = TUid::Uid(appObj->appUid);
      class TApaTask task = taskList.FindApp(uid);
      if(appObj->pendingAction == EEndingApp) {
         if(task.Exists()) { 
            // App is running
            iEndTaskIndex = i;
            iTimer->After(KTimeOut);
            return;
         } else {
            appObj->pendingAction = ENonePending;
         }
      }
   }
   GenerateEventL(EFinishedProcessingList);
}

void CAppHandler::ProcessActions()
{
   // New task to end
   class TApaTaskList taskList(CEikonEnv::Static()->WsSession());
   class TUid uid;
   struct TAppObject* appObj;
   for(TInt i = 0; i < iNumAppObjs; ++i) {
       appObj = iAppList[i];
       uid = TUid::Uid(appObj->appUid);
       class TApaTask task = taskList.FindApp(uid);
       if(!task.Exists()) {
          // Current task is not running, 
          // continue with next one.
          continue;
       }
       if(appObj->appAction == EEndApp) {
          // End task.
#if defined(NAV2_CLIENT_UIQ)
          // We have to kill the task on P800,
          // otherwise the task will not end.
          task.KillTask();
#else
          task.EndTask();
#endif
          appObj->pendingAction = EEndingApp;
       } else if(appObj->appAction == ENotifyObserver) {
          // Notify observer and quit processing list.
          iIndex = i;
          GenerateEventL(EAppIsRunning);
          return;
       }
   }
   iTimer->After(KTimeOut);
}

void CAppHandler::TimerExpired()
{
   if(iNbrOfRetries > 0) {
      iNbrOfRetries--;
      GenerateEventL(ECheckPendingActions);
      return;
   }
   GenerateEventL(EFailedToEndTask);
}
      
