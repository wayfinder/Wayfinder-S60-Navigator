/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES
#include "wayfinder.hrh"
#include "WayFinderConstants.h"
#include "IAPSettingItem.h"
#include "RsgInclude.h"
#include <commdb.h>


// ================= MEMBER FUNCTIONS =========================================

// ----------------------------------------------------------------------------
// CIAPSettingItem::CompleteConstructionL()
// Update the IAP list
// ----------------------------------------------------------------------------

void CIAPSettingItem::CompleteConstructionL()
{
   _LIT(KmRouter, "mroute");

   CArrayPtr<CAknEnumeratedText> * iaps = new(ELeave) CArrayPtrSeg<CAknEnumeratedText>(10);
   CleanupStack::PushL(iaps);
   CArrayPtr<HBufC> * poppedUp = new(ELeave) CArrayPtrSeg<HBufC>(10);
   CleanupStack::PushL(poppedUp);

   CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();

   // The selection of IAP that indicates "always ask"
   HBufC * buf;
   buf = CEikonEnv::Static()->AllocReadResourceL( R_SETTINGS_IAP_ALWAYS_ASK );
   iaps->AppendL(new(ELeave) CAknEnumeratedText(-1, buf ));
   // The selection of IAP that indicates "system default is only shown in
   // development mode, not in release mode.
   if ( ! iRelease ) {
      buf = CEikonEnv::Static()->AllocReadResourceL( R_SETTINGS_IAP_SYSTEM_DEFAULT );
      iaps->AppendL(new(ELeave) CAknEnumeratedText(-2, buf ));
   }


#ifdef NAV2_CLIENT_SERIES60_V2
   CCommsDatabase * comdb = CCommsDatabase::NewL();
#else
   CCommsDatabase * comdb = CCommsDatabase::NewL(EDatabaseTypeUnspecified);
#endif
   CleanupStack::PushL(comdb);
   CCommsDbTableView * iaptable = comdb->OpenTableLC( TPtrC( IAP ) );

   TInt retval;
   retval = iaptable->GotoFirstRecord();

   // display all IAPs except mrouter
   while (retval == KErrNone) {
      HBufC * iap_name = iaptable->ReadLongTextLC( TPtrC( COMMDB_NAME) );
      if ( KErrNotFound == iap_name->FindF(KmRouter) ) {
         TUint32 iap_id;
         iaptable->ReadUintL( TPtrC( COMMDB_ID), iap_id );
         iaps->AppendL(new(ELeave) CAknEnumeratedText(iap_id, iap_name ));
      }
      CleanupStack::Pop(iap_name);   // FIXME - Is this right? Does AppendL take over ownership of the iap name?
      retval = iaptable->GotoNextRecord();
   }

   CleanupStack::PopAndDestroy(iaptable);
   CleanupStack::PopAndDestroy(comdb);

   SetEnumeratedTextArrays(iaps, poppedUp);
   CleanupStack::Pop(poppedUp);
   CleanupStack::Pop(iaps);

   // Does the (old) specified iap still exist? Default to 
   // "always ask" otherwise.
   if (IndexFromValue(ExternalValue()) < 0) {
      SetInternalValue(-1);
      SetExternalValue(-1);
   }


}

// End of File
