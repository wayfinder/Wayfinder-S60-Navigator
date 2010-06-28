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
#include <eikenv.h>
#include "RsgInclude.h"
#include "SettingsData.h"
#include "memlog.h"
#include "TileMapLayerInfo.h"
#include "GuiProt/ServerEnumHelper.h"
#include "WFTextUtil.h"
#include "WFArrayUtil.h"
#include "WFDataHolder.h"

// ================= MEMBER FUNCTIONS =========================================

const int KCheckBoxDataListGranularity = 10;

// ----------------------------------------------------------------------------
// CAknExSettingListListbox::NewL
// Two-phase constructor
// ----------------------------------------------------------------------------
//
CSettingsData* CSettingsData::NewL(CWayfinderDataHolder* aDataStore)
{
   CSettingsData* data = new(ELeave) CSettingsData;
   LOGNEW(data, CSettingsData);
   CleanupStack::PushL(data);
   data->ConstructL(aDataStore);
   CleanupStack::Pop(); // data
   return data;
}

// Destructor
//
CSettingsData::~CSettingsData()
{
   /* Note: Don't delete iDataStore. */
   TInt n;
   iCheckboxArray.Reset();
   for (n = 0; n < iCheckboxTextArray.Count(); n++) {
      /* Get the char* and delete the data. */
      delete[] iCheckboxTextArray[n];
   }
   iCheckboxTextArray.Reset();
   iCheckboxIdArray.Reset();

   delete iMapLayerSettingsVector;
   delete iAllowedLangs;
   delete iUserRights;
   delete iGpsName;
   iGpsParam->Reset();
   delete iGpsParam;
}

// Constructor
//
CSettingsData::CSettingsData()
:
  iServer(KNullDesC),
  iServerOld(KNullDesC),
  iWebUsername(KNullDesC),
  iWebPassword(KNullDesC),
  iUsername(KNullDesC),
  iOldUsername(KNullDesC),
  iPassword(KNullDesC),
  iOldPassword(KNullDesC),
  iVolume(5),
  iUseSpeaker(ETrue),
  iIAP(-1),
  iOldIAP(-1),
  isTrialVersion(1),
  iMuteWhenInBackground(0),
  iMapCacheSize(10),
  iMapTrackingOrientation(ERotating),
  iPreferredGuideMode(preferred_guide),
  iCheckboxArray(KCheckBoxDataListGranularity),
  iCheckboxTextArray(KCheckBoxDataListGranularity),
  iCheckboxIdArray(KCheckBoxDataListGranularity)
{
}

// Epoc default constructor
//
void CSettingsData::ConstructL(CWayfinderDataHolder* aDataStore)
{
   iDataStore = aDataStore;
   iGpsParam = new (ELeave) CDesC8ArraySeg(4);
   iUserRights = new (ELeave) isab::UserRights();
}

void CSettingsData::SetBtGpsAddressL(const char** aStrings, TInt aCount)
{
   delete iGpsName;
   iGpsName = NULL;
   if(aCount >= 3 && (aStrings[2] && strlen(aStrings[2]) > 0)){
      iGpsName = WFTextUtil::AllocL(aStrings[2]);
   } else {
      iGpsName = CEikonEnv::Static()->AllocReadResourceL(R_WAYFINDER_SETTINGSITEM_GPS_NONE);
   }

   WFArrayUtil::CopyArrayL(*iGpsParam, aStrings, aCount);
}

void
CSettingsData::setUserRights(const int32* data, int32 numEntries)
{
   iUserRights->setUserRights(data, numEntries);
}

// End of File
