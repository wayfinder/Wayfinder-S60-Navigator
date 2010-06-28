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

#include "IAPSearcher.h"

#include <e32base.h>
#include <algorithm>
#include <commdb.h>
#include <badesca.h>
#include <eikenv.h>

#include "arch.h"
#include "TimeOutNotify.h"
#include "TimeOutTimer.h"

#include "IAPHandler.h"
#include "IAPSearchGuiObserver.h"
#include "IAPConnMgrObserver.h"
#include "IAPObserver.h"
#include "IAPSearchGui.h"
#include "IAPTester.h"
#include "IAPConnMgrHandler.h"
#include "ConnectionSettings.h"

_LIT(KmRouter, "mroute");
_LIT(KInternet, "internet");
_LIT(KOutgoingGPRS, "outgoinggprs");
_LIT(KSurf, "surf");
_LIT(KOnline, "online");
_LIT(KMMS, "mms");
_LIT(KLANService, "LANService");

#ifdef NAV2_CLIENT_SERIES60_V1
# define MAX_REPEAT_TEST 2
#else
# define MAX_REPEAT_TEST 1
#endif

using namespace std;

enum TIAPSearcherStatus {
   ETimedOut      = 1,
   EConnMgrFailed = 2,
   EIAPTestFailed = 3,
};

enum TSearchState {
   EIdle                   = 0,
   EConnMgrTest            = 1,
   ESettingIAP             = 2,
   EIAPTest                = 3,
   EFailed                 = 4,
   ESuccess                = 5,
   EInitIAPDb              = 6,
   ECancelled              = 7,
   EManualConnMgrTest      = 8,
};


#define KTimeOut (30*1000*1000)

CIAPSearcher::CIAPSearcher(
      class MIAPObserver*       aObserver,
      class MIAPSearchGui*      aGui,
      class MIAPTester*         aTester,
      class MIAPConnMgrHandler* aConnMgr)
 : CActive(EPriorityStandard),
   iIAPIndex ( -1 ),
   iRepeat ( 0 ),
   iTimeOut ( KTimeOut ),
   iSearchState( EIdle ),
   iObserver(aObserver),
   iGui(aGui),
   iTester(aTester),
   iConnMgr(aConnMgr)
{
}

void
Delete(CConnectionSettings* setting)
{
   delete setting;
}

CIAPSearcher::~CIAPSearcher()
{
   if( iTimer ){
      iTimer->Cancel();
   }
   for_each(iIAPList.begin(), iIAPList.end(), Delete);
   iIAPList.clear();
   delete iTimer;
   iConnMgr->ConnMgrCancel();
   delete iConnMgr;
   delete iDefaultSettings;
}

void
CIAPSearcher::ConstructL(const class CConnectionSettings& aConSettings)
{
   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);
   CActiveScheduler::Add(this);

   iGui->SetIAPSearchGuiObserver(this);
   iTester->IAPSearchSetIAPHandler(this);
   iConnMgr->SetIAPConnMgrObserver(this);

   iDefaultSettings = CConnectionSettings::NewL(aConSettings);
}

class CIAPSearcher*
CIAPSearcher::NewLC(
      class MIAPObserver*       aObserver,
      class MIAPSearchGui*      aGui,
      class MIAPTester*         aTester,
      class MIAPConnMgrHandler* aConnMgr,
      const class CConnectionSettings& aConSettings)
{
   class CIAPSearcher* self = 
      new (ELeave) CIAPSearcher(aObserver, aGui, aTester, aConnMgr);
   CleanupStack::PushL(self);
   self->ConstructL(aConSettings);
   return self;
}

class CIAPSearcher*
CIAPSearcher::NewL(
      class MIAPObserver*       aObserver,
      class MIAPSearchGui*      aGui,
      class MIAPTester*         aTester,
      class MIAPConnMgrHandler* aConnMgr,
      const class CConnectionSettings& aConSettings)
{
   class CIAPSearcher* self = 
      CIAPSearcher::NewLC(aObserver, aGui, aTester, aConnMgr, aConSettings);
   CleanupStack::Pop(self);
   return self;
}


void
CIAPSearcher::TimerExpired()
   // Implements MTimeOutNotify: called when timeout expired
{  
   class TRequestStatus* p = &iStatus;
   // Signal RunL() imediately
   SetActive();
   User::RequestComplete(p, ETimedOut);
}

TInt
CIAPSearcher::InitIAPData(TBool aSort)
{
   if (!iIAPList.empty()) {
      for_each(iIAPList.begin(), iIAPList.end(), Delete);
      iIAPList.clear();
   }
#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined (NAV2_CLIENT_SERIES80) || defined (NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_UIQ3)
   class CCommsDatabase* comdb = CCommsDatabase::NewL();
#elif defined(NAV2_CLIENT_UIQ)
   class CCommsDatabase* comdb = CCommsDatabase::NewL( EDatabaseTypeIAP );
#elif defined(NAV2_CLIENT_SERIES60_V1)
   class CCommsDatabase* comdb = CCommsDatabase::NewL(EDatabaseTypeUnspecified);
#elif
# error Check how to create comsdatabase!
#endif
   CleanupStack::PushL(comdb);
   class CCommsDbTableView * iaptable = comdb->OpenTableLC( TPtrC( IAP ) );

   TInt foo_retval = iaptable->GotoFirstRecord();
   deque<class CConnectionSettings*> lowPrioList;
   lowPrioList.clear();
   deque<class CConnectionSettings*> preferred;
   preferred.clear();
   deque<class CConnectionSettings*> exact_match;
   exact_match.clear();

   while (foo_retval == KErrNone) {
      HBufC* iap_name = iaptable->ReadLongTextLC( TPtrC( COMMDB_NAME) );
      /* Make sure that we don't show APs with mRoute in the name. */
      if ( KErrNotFound == iap_name->FindF(KmRouter) ) {
         TUint32 iap_id;
         iaptable->ReadUintL( TPtrC( COMMDB_ID), iap_id );
         HBufC* iap_type = iaptable->ReadLongTextLC(TPtrC( IAP_SERVICE_TYPE ));
         if (*iap_type != KLANService ) {

            HBufC* temp = HBufC::NewLC(iap_type->Des().Length()+
                                       iap_name->Des().Length()+16);
            temp->Des().Copy(*iap_name);
#ifdef ADD_IAP_TYPE_TO_NAME
            temp->Des().Append(_L(" - "));
            temp->Des().Append(*iap_type);
#endif
            class CConnectionSettings* setting =
               CConnectionSettings::NewLC(*iDefaultSettings);
            setting->SetIAP(iap_id);
            setting->SetIAPName(*temp);

            if (aSort) {
               CDesCArray* preferredIAPs = iDefaultSettings->GetPreferredIAPs();
               TBool found = EFalse;
               if(preferredIAPs && preferredIAPs->Count() > 0) {
                  // Iterate through the list containing preferred iaps. 
                  // If a match is found, we store it in a temporary list.
                  // When we are finishied interating through the 
                  // comm table, we put our found preferred iaps first in
                  // the iIAPList.
                  for (TInt i = 0; i < preferredIAPs->Count(); i++) {
                     if (!iap_name->CompareF(preferredIAPs->MdcaPoint(i)) &&
                        !found) {
                        exact_match.push_back(setting);
                        found = ETrue;
                     }
                     if (KErrNotFound != iap_name->FindF(preferredIAPs->MdcaPoint(i)) &&
                        !found) {
                        preferred.push_back(setting);
                        found = ETrue;
                     }

                  }
               }
               if(found) {
                  // We already found a match in the list of
                  // preferred iaps. Just go to next record
                  // in the table.
               }
               else if (KErrNotFound != iap_name->FindF(KInternet)) {
                  // Contains "internet" in name, highest prio.
                  iIAPList.push_front(setting);
               } else if (KErrNotFound != iap_name->FindF(KMMS)) {
                  // Contains "MMS" in the name, this we definitely don't want.
                  lowPrioList.push_back(setting);
               } else if (KErrNotFound != iap_name->FindF(KSurf)) {
                  // Contains "surf" in name, medium prio.
                  iIAPList.push_back(setting);
               } else if (KErrNotFound != iap_name->FindF(KOnline)) {
                  // Contains "online" in name, medium prio.
                  iIAPList.push_back(setting);
               } else if (KErrNotFound == iap_type->FindF(KOutgoingGPRS)) {
                  // Not OutgoingGPRS in type.
                  lowPrioList.push_back(setting);
               } else {
                  // Not very interesting, put it before the really bad iaps.
                  lowPrioList.push_front(setting);
               }
               CleanupStack::Pop(setting);
            } else {
               iIAPList.push_front(setting);
            }
            CleanupStack::Pop(temp);
         }
         CleanupStack::PopAndDestroy(iap_type);
      }
         
      CleanupStack::PopAndDestroy(iap_name);
      foo_retval = iaptable->GotoNextRecord();
   }

   deque<class CConnectionSettings*>::reverse_iterator it = preferred.rbegin();
   while (it != preferred.rend()) {
      iIAPList.push_front(*it);
      it++;
   }
   preferred.clear();
   it = exact_match.rbegin();
   while (it != exact_match.rend()) {
      iIAPList.push_front(*it);
      it++;
   }
   exact_match.clear();
   if (aSort && !lowPrioList.empty()) {
      deque<class CConnectionSettings*>::iterator it = lowPrioList.begin();
      while (it != lowPrioList.end()) {
         iIAPList.push_back(*it);
         it++;
      }
   }
   lowPrioList.clear();

   CleanupStack::PopAndDestroy(iaptable);
   CleanupStack::PopAndDestroy(comdb);

   return (iIAPList.size());
}

void
CIAPSearcher::ReportProgressIAP()
{
   iGui->IAPReportProgress((iIAPIndex*MAX_REPEAT_TEST+iRepeat+1), ((iIAPList.size())*MAX_REPEAT_TEST), iIAPList[iIAPIndex]->GetIAPName());
}

void
CIAPSearcher::TestIAPConnMgr()
{
   iSearchState = EConnMgrTest;

   /* Get current IAP */
   class CConnectionSettings* setting = iIAPList[iIAPIndex];
   iConnMgr->SetIAP(*setting);
   iTimer->After(iTimeOut);
   iConnMgr->Test();
}

void 
CIAPSearcher::CancelTimerAndComplete(TInt aCompleteVal)
{
   iTimer->Cancel();
   Complete(aCompleteVal);
}

void 
CIAPSearcher::Complete(TInt aCompleteVal)
{
   class TRequestStatus* p = &iStatus;
   if (!IsActive()) {
      SetActive();
   }
   User::RequestComplete(p, aCompleteVal);
}

void
CIAPSearcher::ConnMgrSuccess()
{
   if (iSearchState == EConnMgrTest || iSearchState == EManualConnMgrTest) {
      CancelTimerAndComplete(KErrNone);
   }
}

void
CIAPSearcher::ConnMgrFailed()
{
   if (iSearchState == EConnMgrTest || iSearchState == EManualConnMgrTest) {
      CancelTimerAndComplete(EConnMgrFailed);
   }
}

void
CIAPSearcher::SendIAP()
{
   /* Get current IAP */
   class CConnectionSettings* setting = iIAPList[iIAPIndex];
   iSearchState = ESettingIAP;
   iTester->IAPSearchSetIAP(*setting);
}

void
CIAPSearcher::IAPIdSet()
{
   if (iSearchState == ESettingIAP) {
      Complete(KErrNone);
   }
}

void
CIAPSearcher::TestIAPDownload()
{
   iSearchState = EIAPTest;

   iTimer->After(iTimeOut);
   iTester->IAPSearchStart();
}

void
CIAPSearcher::IAPSearchSuccess()
{
   if (iSearchState == EIAPTest) {
      CancelTimerAndComplete(KErrNone);
   }
} 

void
CIAPSearcher::IAPSearchFailed()
{
   if (iSearchState == EIAPTest) {
      CancelTimerAndComplete(EIAPTestFailed);
   }
}

void
CIAPSearcher::TestNextIAP()
{
   if (iSearchState == ECancelled) {
      /* User (or something) aborted search. */
      Complete(KErrNone);
      return;
   }
   if (++iRepeat >= MAX_REPEAT_TEST) {
      iIAPIndex++;
      iRepeat = 0;
   }
   if (iIAPIndex >= (int32)iIAPList.size()) {
      /* Total failure. */
      iSearchState = EFailed;
      Complete(KErrNone);
      //iObserver->IAPSearchFailed(ErrorNoWorkingIAPs);
   } else {
      ReportProgressIAP();
      TestIAPConnMgr();
   }
}

void
CIAPSearcher::IAPSearchCancel()
{
   // Stop the timer
   iTimer->Cancel();
   // Cancel the ConnMgr, when it returns
   // we assume it is finnished.
   iConnMgr->ConnMgrCancel();
   // Report to gui.
   iObserver->IAPSearchFailed(ErrorCancelled);
   CActive::Cancel();
}

void
CIAPSearcher::IAPManualNotFound()
{
   // Stop the timer
   iTimer->Cancel();
   // Cancel the ConnMgr, when it returns
   // we assume it is finnished.
   iConnMgr->ConnMgrCancel();
   // Report to gui.
   iObserver->IAPSearchFailed(ErrorNoWorkingIAPs);
}

void
CIAPSearcher::StartSearch()
{
   if ( iSearchState == EIdle ||
        iSearchState == EFailed ||
        iSearchState == ECancelled) {
      /* Initialize search variables. */
      iIAPIndex = 0;
      if (!InitIAPData()) {
         iObserver->IAPSearchFailed(ErrorNoIAPs);
         iSearchState = EFailed;
         return;
      }
      iRepeat = 0;

      /* Kickstart the process. */
      ReportProgressIAP();
      TestIAPConnMgr();
   }
}

const deque<class CConnectionSettings*>&
CIAPSearcher::GetConnectionList()
{
   InitIAPData();
   return iIAPList;
}

void
CIAPSearcher::ManualTest(class CConnectionSettings* aIAP)
{
   iGui->SetIAPSearchGuiObserver(this);
   iTester->IAPSearchSetIAPHandler(this);
   iConnMgr->SetIAPConnMgrObserver(this);

   /* Clean IAP-list and fake just one entry. */
   if (!iIAPList.empty()) {
      for_each(iIAPList.begin(), iIAPList.end(), Delete);
   }
   iIAPList.clear();
   iIAPList.push_front(aIAP);
   iIAPIndex = 0;

   iSearchState = EManualConnMgrTest;

   /* Get current IAP */
   iConnMgr->SetIAP(*aIAP);
   iConnMgr->Test();
   iTimer->After(iTimeOut);
}

void
CIAPSearcher::SearchDone()
{
   iGui->SetIAPSearchGuiObserver(NULL);
   iTester->IAPSearchSetIAPHandler(NULL);
   iConnMgr->SetIAPConnMgrObserver(NULL);

   iObserver->IAPSearchDone(*iIAPList[iIAPIndex]);
   iSearchState = ESuccess;
}

void
CIAPSearcher::OverrideSearchDone()
{
   iGui->SetIAPSearchGuiObserver(NULL);
   iTester->IAPSearchSetIAPHandler(NULL);
   iConnMgr->SetIAPConnMgrObserver(NULL);
   iSearchState = ESuccess;
}

void
CIAPSearcher::RunL()
{
   iTimer->Cancel();
   switch ( iStatus.Int() ) {
      case ETimedOut:
         // Fixa
         break;
      case EConnMgrFailed:
         /** FALLTHROUGH **/
      case EIAPTestFailed:
         TestNextIAP();
         break;
      case KErrNone:
         switch(iSearchState){
            case EIdle:
               break;
            case EConnMgrTest:
               SendIAP();
               break;
            case ESettingIAP:
               TestIAPDownload();
               break;
            case EIAPTest:
               /* IAP search success. */
               SearchDone();
               break;
            case EFailed:
               IAPManualNotFound();
               break;
            case ESuccess:
               break;
            case EInitIAPDb:
               break;
            case ECancelled:
               /* User (or something) aborted search. */
               IAPSearchCancel();
               break;
            case EManualConnMgrTest:
               SendIAP();
               break;
         }
         break;
      case KErrCancel:
          iObserver->IAPSearchFailed(ErrorCancelled);
         break;
      default:
         break;
   }
}

void
CIAPSearcher::DoCancel()
{
   iTimer->Cancel();
   iSearchState = ECancelled;
   iConnMgr->ConnMgrCancel();
}
