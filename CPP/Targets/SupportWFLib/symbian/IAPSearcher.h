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

#ifndef IAP_SEARCHER_H
#define IAP_SEARCHER_H

#include <e32base.h>
#include <deque>
#include "IAPHandler.h"
#include "IAPSearchGuiObserver.h"
#include "IAPConnMgrObserver.h"
#include "TimeOutNotify.h"

class CIAPSearcher : public CActive,
                     public MIAPHandler,
                     public MIAPSearchGuiObserver,
                     public MIAPConnMgrObserver,
                     public MTimeOutNotify
{
   CIAPSearcher(
         class MIAPObserver*       aObserver,
         class MIAPSearchGui*      aGui,
         class MIAPTester*         aTester,
         class MIAPConnMgrHandler* aConnMgr);
   void ConstructL(const class CConnectionSettings& aDefaultSettings);
public:
   static class CIAPSearcher* NewLC(
         class MIAPObserver*       aObserver,
         class MIAPSearchGui*      aGui,
         class MIAPTester*         aTester,
         class MIAPConnMgrHandler* aConnMgr,
         const class CConnectionSettings& aConSettings);
   static class CIAPSearcher* NewL(
         class MIAPObserver*       aObserver,
         class MIAPSearchGui*      aGui,
         class MIAPTester*         aTester,
         class MIAPConnMgrHandler* aConnMgr,
         const class CConnectionSettings& aConSettings);

   virtual ~CIAPSearcher();

   TInt InitIAPData(TBool aSort = ETrue);
   void StartSearch();
   void OverrideSearchDone();

   const std::deque<class CConnectionSettings*>& GetConnectionList();
   void ManualTest(class CConnectionSettings* aIAP);

private:
   void CancelTimerAndComplete(TInt aCompleteVal);
   void Complete(TInt aCompleteVal);
   
public: /* MIAPHandler */
   void IAPSearchSuccess();
   void IAPSearchFailed();
   void IAPIdSet();

public: /* MIAPSearchGuiObserver */
   void IAPSearchCancel();

public: /* MIAPConnMgrObserver */
   void ConnMgrSuccess();
   void ConnMgrFailed();

public: /* CActive */
   void RunL();
   void DoCancel();

public: /* MTimeOutNotify */
   void TimerExpired();

private:
   void TestIAPConnMgr();
   void TestIAPDownload();
   void TestNextIAP();

   void ReportProgressIAP();
   void SendIAP();

   void SearchDone();
   void IAPManualNotFound();

private:
   TInt32 iIAPIndex;
   TInt iRepeat;
   TInt32 iTimeOut;
   class CTimeOutTimer* iTimer;

   TInt iSearchState;

   class MIAPObserver*       iObserver;
   class MIAPSearchGui*      iGui;
   class MIAPTester*         iTester;
   class MIAPConnMgrHandler* iConnMgr;

   std::deque<class CConnectionSettings*> iIAPList;

   class CConnectionSettings* iDefaultSettings;
};

#endif


