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
#include "GpsSettingItem.h"
#include "RsgInclude.h"
#include <commdb.h>



// ================= MEMBER FUNCTIONS =========================================

CGpsSettingItem::CGpsSettingItem(TInt aResourceId, TInt& aValue, 
                                 HBufC*& aName) :
   //CAknEnumeratedTextPopupSettingItem(aResourceId, aValue),
   CAknBinaryPopupSettingItem(aResourceId, (TBool&)aValue),
   iName(aName)
{ }

// ----------------------------------------------------------------------------
// CIAPSettingItem::CompleteConstructionL()
// Update the IAP list
// ----------------------------------------------------------------------------

enum TGpsSettingChoices {
   EGpsNoChange = 0,
   EGpsNewGps = 1,
};

void CGpsSettingItem::AppendEnumTextL(CArrayPtr<CAknEnumeratedText>& aList,
                                      TInt aValue, TInt aResource)
{
   CAknEnumeratedText* text = new (ELeave) CAknEnumeratedText(aValue, 0);
   CleanupStack::PushL(text);
   HBufC* txt = NULL;
   if(aResource != 0){
      txt = CCoeEnv::Static()->AllocReadResourceL(aResource);
   } else {
      txt = iName->AllocL();
   }
   text->SetValue(txt);
   aList.AppendL(text);
   CleanupStack::Pop(text);
}

void CGpsSettingItem::CompleteConstructionL()
{
   CArrayPtr<CAknEnumeratedText>* items = 
      new (ELeave) CArrayPtrSeg<CAknEnumeratedText>(2);
   CleanupStack::PushL(items);
   CArrayPtr<HBufC>* poppedUp = new(ELeave) CArrayPtrSeg<HBufC>(2);
   CleanupStack::PushL(poppedUp);

   //CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();
   CAknBinaryPopupSettingItem::CompleteConstructionL();

   AppendEnumTextL(*items, EGpsNoChange, 
                   iName == NULL ? R_WAYFINDER_SETTINGSITEM_GPS_NONE : 0);
   AppendEnumTextL(*items, EGpsNewGps, R_WAYFINDER_SETTING_GPS_NEW);

   SetEnumeratedTextArrays(items, poppedUp);
   CleanupStack::Pop(poppedUp);
   CleanupStack::Pop(items);
}
