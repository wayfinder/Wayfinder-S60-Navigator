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
// ================= MEMBER FUNCTIONS =========================================

const int KCheckBoxDataListGranularity = 10;

// ----------------------------------------------------------------------------
// CAknExSettingListListbox::NewL
// Two-phase constructor
// ----------------------------------------------------------------------------
//
CSettingsData* CSettingsData::NewL(isab::Log* /*aLog*/)
{
   CSettingsData* data = new(ELeave) CSettingsData;
   LOGNEW(data, CSettingsData);
   CleanupStack::PushL(data);
   data->ConstructL();
   CleanupStack::Pop(); // data
   return data;
}

// Destructor
//
CSettingsData::~CSettingsData()
{
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
: iServer(_L("")),
   iServerOld(_L("")),
  iWebUsername(_L("")),
  iWebPassword(_L("")),
  iUsername(_L("")),
  iOldUsername(_L("")),
  iPassword(_L("")),
  iOldPassword(_L("")),
  iApplicationPath(_L("")),
  iApplicationDllName(_L("")),
  iAudioBasePath(_L("")),
  iApplicationNameAsPath(_L("")),
  iVolume(7),
  iAutoReroute(ETrue),
  iUseSpeaker(ETrue),
  iTurnSoundsLevel(isab::GuiProtEnums::turnsound_normal),
  iTimeDist(isab::NavServerComEnums::DISTANCE),
  iTollRoads(isab::NavServerComEnums::TollRoadsDeny),
  iHighways(isab::NavServerComEnums::HighwaysDeny),
  iSaveSMSDest(isab::GuiProtEnums::ask),
  iAutoSMSRoute(ETrue),
  iKeepSMSDest(isab::GuiProtEnums::ask),
  iBacklightStrategy(isab::GuiProtEnums::backlight_always_on),
/*   iLeftSideTraffic(isab::GuiProtEnums::no), */
  iIAP(-1),
  iOldIAP(-1),
  iLanguage(0),
  iOldLanguage(0),
  showDebug(0),
  isTrialVersion(1),
  iDistanceMode(0),
  iMuteWhenInBackground(0),
  iDirectionType(MC2Direction::ClockBased),
  iFeedOutputFormat(FeedSettings::FEED_CONCISE),
  iVicinityFeedRate(10),
  iLockedNavRate(15),
  iWalkingSpeedCutOff(3),
  iVicinityListPeriod(20),
  iAudioHasFeetMiles(EFalse),
  iAudioHasYardsMiles(EFalse),
  iMapCacheSize(1),
  iMapTrackingOrientation(ERotating),
  iMapType(EVector),
  iOldMapType(EVector),
  iPreferredGuideMode(preferred_guide),
  iFavoriteShow(1),
  iGPSAutoConnect(1),
  iCheckboxArray(KCheckBoxDataListGranularity),
  iCheckboxTextArray(KCheckBoxDataListGranularity),
  iCheckboxIdArray(KCheckBoxDataListGranularity),
  iShowCategories(0),
  iAutoTrackingOn(ETrue),
  iCanShowWebUsername(ETrue),
  iCanShowWebPassword(ETrue),
  iCanChangeMapType(ETrue),
  iTrackingLevel( 0 ),
  iOldTrackingLevel( 0 ),
  iTrackingPIN(KNullDesC),
  iOldTrackingPIN(KNullDesC),
  m_trackPINList( NULL ),
  iShowTraffic(EFalse),
  iTrafficEnabled(ETrue),
  iTrafficOldEnabled(ETrue),
  iTrafficTime(KNullDesC),
  iTrafficOldTime(KNullDesC),
  iMapLayerSettingsVector(0),
  iAllowedLangs(NULL),
  iUserRights( NULL),
  iAllowGps(ETrue),
  iIsIron(EFalse),
  iShowLangsWithoutVoice(ETrue),
  iShowLangsWithoutResource(EFalse),
  iRouteTrafficInfoEnabled(ETrue),
  iRouteTrafficInfoUpdTime(25)
{
   //iVehicle = isab::NavServerComEnums::passengerCar;
   // Change to pedestrian mode by default for Access
   iVehicle = isab::NavServerComEnums::pedestrian;
   iOldVehicle = iVehicle;
}

// Epoc default constructor
//
void CSettingsData::ConstructL()
{
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


// End of File
