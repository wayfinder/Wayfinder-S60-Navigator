/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "arch.h"
#include "IAPSearcher_old.h"
#include "IAPObserver_old.h"
#include "WFTextUtil.h"
#include <commdb.h>
#include "Dialogs.h"
#include "memlog.h"
#include "RsgInclude.h"
#include "PopUpList.h"
#include "WayFinderSettings.h"

   _LIT(KmRouter, "mroute");
   _LIT(KInternet, "internet");
   _LIT(KOutgoingGPRS, "outgoinggprs");

#ifdef NAV2_CLIENT_SERIES60_V2
#define MAX_REPEAT_TEST 1
#elif NAV2_CLIENT_SERIES60_V1
#define MAX_REPEAT_TEST 2
#elif NAV2_CLIENT_SERIES60_V3
#define MAX_REPEAT_TEST 1
#else
#error Undefined number of repeats for IAP test!
#endif


CIAPSearcher_old::CIAPSearcher_old(IAPObserver *aAppUi)
{
   iAppUi = aAppUi;
   iSettingIAP = EFalse;
   iSearching = EFalse;
   iSelectManual = EFalse;
   iIAP = -1;
   iIAPList = NULL;
   iRepeat = 0;
   iCurIAPName = NULL;
   iShowWlan = EFalse;
}

CIAPSearcher_old::~CIAPSearcher_old()
{
   delete iIAPList;
}

void
CIAPSearcher_old::ShowWlan(TBool status)
{
   iShowWlan = status;
}

void
CIAPSearcher_old::InitIAPData(TBool aSort)
{
   if (iIAPList) {
      delete iIAPList;
   }
   iIAPList = new(ELeave) CArrayPtrFlat<CAknEnumeratedText>(10);

#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
   // Open CommsDb without creating a database if it does not exist.
   CCommsDatabase* comdb = NULL;
   TRAPD(err, comdb = CCommsDatabase::NewL(EFalse));
   if (err != KErrNone) {
      // Could not open CommsDB return no iap found.
      return;
   }
#else
   CCommsDatabase * comdb = CCommsDatabase::NewL(EDatabaseTypeUnspecified);
#endif
   CleanupStack::PushL(comdb);
   CCommsDbTableView * iaptable = comdb->OpenTableLC( TPtrC( IAP ) );

   TInt aCurId = 0;
   TInt foo_retval= iaptable->GotoFirstRecord();

   while (foo_retval == KErrNone) {
      HBufC * iap_name = iaptable->ReadLongTextLC( TPtrC( COMMDB_NAME) );
/*       WFDialog::ShowScrollingErrorDialogL(*iap_name); */
      /* Make sure that we don't show APs with mRoute in the name. */
      if ( KErrNotFound == iap_name->FindF(KmRouter) ) {
         TUint32 iap_id;
         iaptable->ReadUintL( TPtrC( COMMDB_ID), iap_id );
         HBufC* iap_type = iaptable->ReadLongTextLC( TPtrC( IAP_SERVICE_TYPE ));
/*          WFDialog::ShowScrollingErrorDialogL(*iap_type); */

         if ((iShowWlan || *iap_type != _L("LANService"))) 
         {
            HBufC* temp = HBufC::NewLC(iap_type->Des().Length()+
                                       iap_name->Des().Length()+16);
            temp->Des().Copy(*iap_name);
#ifdef ADD_IAP_TYPE_TO_NAME
            temp->Des().Append(_L(" - "));
            temp->Des().Append(*iap_type);
#endif
            if (!aSort ||
                KErrNotFound == iap_name->FindF(KInternet) ||
               KErrNotFound == iap_type->FindF(KOutgoingGPRS)) {
               /* Not "internet" in the name or not OutgoingGPRS in type. */
               iIAPList->AppendL(new(ELeave) CAknEnumeratedText(iap_id, temp ));
            } else {
               iIAPList->InsertL(aCurId,
                                 new(ELeave)CAknEnumeratedText(iap_id, temp));
               aCurId++;
            }
            /* CAknEnumeratedText takes ownership of temp string. */
            CleanupStack::Pop(temp); 
         }
         CleanupStack::PopAndDestroy(iap_type);
      }
      CleanupStack::PopAndDestroy(iap_name);
      foo_retval = iaptable->GotoNextRecord();
   }
   CleanupStack::PopAndDestroy(iaptable);
   CleanupStack::PopAndDestroy(comdb);
}

void CIAPSearcher_old::StartSearch()
{
   if (!iSearching && !iSettingIAP ) {
      /* Initialize search variables. */
      iIAP = 0;
      iSettingIAP = ETrue;
      InitIAPData();
      iRepeat = 0;

      if (iIAPList->Count() == 0) {
         /* No access points found! */
         iSettingIAP = EFalse;
         iAppUi->ReportFailureIAP(ErrorNoIAPs);
      } else {
         /* Kickstart the process. */
         ReportProgressIAP();
         SendIAP();
      }
   }
}

void
CIAPSearcher_old::SendIAP()
{
   /* Get current IAP id. */
   iCurIAPid = ((*iIAPList)[iIAP])->EnumerationValue();
   HBufC *tmp = ((*iIAPList)[iIAP])->Value();
   delete iCurIAPName;
   iCurIAPName = HBufC::NewL(tmp->Des().Length());
   iCurIAPName->Des().Copy(*tmp);

   iAppUi->SendIAP(iCurIAPid);
}
void
CIAPSearcher_old::SendSync()
{
   //iAppUi->SendSyncParameters();
   iAppUi->SendNop();
}

void
CIAPSearcher_old::ReportProgressIAP(TBool aFinnished)
{
   if (aFinnished) {
      iAppUi->ReportProgressIAP(100, 100, NULL);
   } else {
      iAppUi->ReportProgressIAP((iIAP*MAX_REPEAT_TEST+iRepeat+1), 
                                (iIAPList->Count()*MAX_REPEAT_TEST),
                                ((*iIAPList)[iIAP])->Value());
   }
}

void CIAPSearcher_old::Reply()
{
   /* Sync worked! */
   iSearching = EFalse;
   iSelectManual = EFalse;

   /* Set as default access point. */
   iAppUi->SendIAP(iCurIAPid, ETrue);
   ReportProgressIAP(ETrue);
}
void CIAPSearcher_old::ReplyFailed()
{
   if (iSelectManual) {
      /* We're testing a manually selected entry. */
      /* This means that the manually selected entry */
      /* does not work. */
      iSearching = EFalse;
      iSelectManual = EFalse;
      iAppUi->ReportFailureIAP(ErrorIAPNotWorking, ETrue);
      /* Send the user back to the select menu. */
      SelectManual();
      return;
   }
   if (iRepeat >= MAX_REPEAT_TEST-1) {
      /* Failed more than X times */
      iIAP++;
      iRepeat = 0;
      if (iIAP >= iIAPList->Count()) {
         /* All APs tested, failed. */
         iAppUi->ReportFailureIAP(ErrorNoWorkingIAPs);
         return;
      }
      /* Fallthrough */
   } else {
      iRepeat++;
   }
   ReportProgressIAP();
   iSettingIAP = ETrue;
   iSearching  = EFalse;
   SendIAP();
}

HBufC*
CIAPSearcher_old::GetIAPName()
{
   return iCurIAPName;
}

TBool CIAPSearcher_old::Searching()
{
   return (iSearching || iSettingIAP);
}

TBool
CIAPSearcher_old::SelectManual()
{
   TBool retval = ETrue;

   /* Get list of access points, sorted according to best guess. */
   InitIAPData(EFalse);

   /* Create a list for the user to choose from. */
   CDesCArrayFlat* descArray = new (ELeave) CDesCArrayFlat(16);
   //LOGNEW(descArray, CDesCArrayFlat(16));
   CDesCArrayFlat* descArray2 = new (ELeave) CDesCArrayFlat(16);
   //LOGNEW(descArray2, CDesCArrayFlat(16));

   CleanupStack::PushL( descArray );
   CleanupStack::PushL( descArray2 );

   // add all IAPs
   for (TInt i = 0; i < iIAPList->Count(); i++) {
      descArray->AppendL(*(((*iIAPList)[i])->Value()));
      descArray2->AppendL(*(((*iIAPList)[i])->Value()));
   }

   /* Sort array, use folded strings. */
   descArray->Sort();

   HBufC *aTitle = CCoeEnv::Static()->AllocReadResourceLC(R_WAYFINDER_QUERY_IAP_SELECT);

   TBool okChosen = EFalse;
   TInt selection = PopUpList::ShowPopupListL(R_WAYFINDER_SOFTKEYS_OK_CANCEL,
                           *descArray, okChosen, 0,
                           ETrue,
                           &(*aTitle));

   CleanupStack::PopAndDestroy(aTitle);
   if (okChosen) {
      TPtrC16 choice = (*descArray)[selection];
      TInt real_id = 0;
      descArray2->Find(choice, real_id, ECmpNormal);

      iSelectManual = ETrue;
      iSettingIAP = ETrue;
      iIAP = real_id;
      /* Selected an access point. */
      iCurIAPid = ((*iIAPList)[real_id])->EnumerationValue();
      SendIAP();
/*       iAppUi->SendIAP(iCurIAPid, ETrue, EFalse); */

   } else {
      /* No access point selected. */
      iSearching = EFalse;
      iSelectManual = EFalse;
      iAppUi->ReportFailureIAP(ErrorIAPNotSelected);
      retval = EFalse;
   }
   //LOGDEL(descArray2);
   CleanupStack::PopAndDestroy(descArray2);
   //LOGDEL(descArray);
   CleanupStack::PopAndDestroy(descArray);

   return retval;
}

void
CIAPSearcher_old::iapid2Set( int32 iapId )
{
   if ( iSettingIAP && iapId == iCurIAPid ) {
      iSettingIAP = EFalse;
      iSearching  = ETrue;
      // Send sync to test the now set IAP
      SendSync();
   }
}
