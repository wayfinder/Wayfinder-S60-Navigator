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

#ifndef APP_HANDLER_H
#define APP_HANDLER_H
#include <e32base.h>        //CActive 
#include <e32std.h>         //TDesC
#include "TimeOutNotify.h"  //MTimeOutNotify
#include "EventGenerator.h" //CEventGenerator

/**
 * Enum that defines different actions
 * that can be performed if a specific
 * app is running.
 */
enum TAppAction {
   ENotifyObserver,
   EEndApp,
};

enum TPendingAction {
   ENonePending,
   EEndingApp,
};

/**
 * Struct that represents an app.
 * appName, name of the app.
 * appUid, apps uid.
 * appAction, what to do if app is running.
 */ 
struct TAppObject {
   HBufC* appName;
   TInt32 appUid;
   enum   TAppAction appAction;
   enum   TPendingAction pendingAction;
   
   ~TAppObject()
   {
      delete appName;
      appName = NULL;
   }
};

class CAppHandler : public CBase,
                    public MTimeOutNotify
{
   
   CAppHandler(class MAppObserver* aAppObserver, TInt aLength);
   void ConstructL(struct TAppObject** aAppList);
   void ConstructL();

private:
   enum TAppProcessEvent {
      EStartProcessingApp,
      EAppIsRunning,
      ECheckPendingActions,
      EFailedToEndTask,
      EFinishedProcessingList,
      EFailedToInitList,
   };
   
public:
   /**
    * Empty construction
    */
   static class CAppHandler* NewLC(class MAppObserver* aAppObserver);

   /**
    * Full construction with list of apps. 
    * aAppList, list containing TAppObjects
    */
   static class CAppHandler* NewLC(class MAppObserver* aAppObserver,
                                   struct TAppObject** aAppList,
                                   TInt aLength);

   /**
    * Empty construction
    */
   static class CAppHandler* NewL(class MAppObserver* aAppObserver);

   /**
    * Full construction with list of apps. 
    * aAppList, list containing TAppObjects
    */
   static class CAppHandler* NewL(class MAppObserver* aAppObserver,
                                  struct TAppObject** aAppList,
                                  TInt aLength);

   virtual ~CAppHandler();

   void SetAppListL(struct TAppObject** aAppList, TInt aLenth);
   void ProcessAppListL();
   void HandleGeneratedEventL(enum TAppProcessEvent aEvent);

public: // MTimeOutNotify
   void TimerExpired();

private:
   void GenerateEventL(enum TAppProcessEvent aEvent);
   void NotifyObserver();
   void NotifyObserverEndTaskFailed();
   void CheckPendingActions() ;
   void ProcessActions();

private:
   class  MAppObserver* iAppObserver;
   class  CTimeOutTimer* iTimer;
   struct TAppObject** iAppList;
   TBool  iRunning;
   TInt   iIndex;
   TInt   iEndTaskIndex;
   TInt   iNumAppObjs;
   TInt   iNbrOfRetries;

   typedef CEventGenerator<CAppHandler, 
                           enum TAppProcessEvent> CAppEventGenerator;
   CAppEventGenerator* iGenerator;
};

#endif
