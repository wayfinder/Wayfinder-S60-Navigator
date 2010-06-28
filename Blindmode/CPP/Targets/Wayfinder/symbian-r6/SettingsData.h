/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SETTINGSDATA_H
#define SETTINGSDATA_H

// INCLUDES
#include <e32base.h>

#include "WayFinderConstants.h"
#include "NavServerComEnums.h"
#include "GuiProt/HelperEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "MC2Direction.h"
#include "FeedSettings.h"

// Version number for Vector map settings.
// WARNING! The vector map blob is generated in the make_seed.pl file!
#define VECTOR_MAP_SETTINGS_VERSION 1

#define VECTOR_MAP_POI_CATEGORY_VERSION 5

// FORWARD DECLARATIONS

namespace isab {
   class Log;
   class TrackPINList;
   class UserRights;
}

// CLASS DECLARATION

/**
* CSettingsData 
*/
class CSettingsData : public CBase
{
public:
   static CSettingsData* NewL(isab::Log* aLog);
   ~CSettingsData();

private:
   CSettingsData();
   void ConstructL();

public:
   inline CArrayFix<TInt>& CheckboxArray() { return iCheckboxArray; }
   inline CArrayFix<char *>& CheckboxTextArray() { return iCheckboxTextArray; }
   inline CArrayFix<TInt>& CheckboxIdArray() { return iCheckboxIdArray; }

public:
/*    void SetTi */
public:

   /// The server name
   TBuf<KBuf256Length> iServer;
   TBuf<KBuf256Length> iServerOld;
   TBuf<KBuf64Length> iWebUsername;
   TBuf<KBuf64Length> iWebPassword;
   TBuf<KBuf64Length> iUsername;
   TBuf<KBuf64Length> iOldUsername;
   TBuf<KBuf64Length> iPassword;
   TBuf<KBuf64Length> iOldPassword;

   TBuf<KBuf256Length> iApplicationPath;
   TBuf<KBuf256Length> iApplicationDllName;
   TBuf<KBuf256Length> iAudioBasePath;
   TBuf<KBuf64Length> iApplicationNameAsPath;

   /// Speaker volume
   TInt iVolume;

   /// True if rerouting is done automatically.
   TBool iAutoReroute;

   /// True if sound should be played through the speaker.
   TBool iUseSpeaker;

   /// How talkative is the navigator
   isab::GuiProtEnums::TurnSoundsLevel iTurnSoundsLevel;

   /// The vehicle being used.
   isab::NavServerComEnums::VehicleType iVehicle;

   /// The vehicle being used before user changed it.
   isab::NavServerComEnums::VehicleType iOldVehicle;

   /// Optimize route for time or distance or whatnot.
   isab::NavServerComEnums::RouteCostType iTimeDist;

   /// Avoid toll roads for routes?
   isab::NavServerComEnums::RouteTollRoads iTollRoads;

   /// Avoid highways for routes?
   isab::NavServerComEnums::RouteHighways iHighways;

   /// Save SMS destinations?
   isab::GuiProtEnums::YesNoAsk iSaveSMSDest;

   /// Automatically route to SMS destinations?
   TBool iAutoSMSRoute;

   /// Keep sms destinations in inbox?
   isab::GuiProtEnums::YesNoAsk iKeepSMSDest;

   /// Backlight strategy (always on, on route or normal)
   TInt iBacklightStrategy;

   /// Which internet access point to use
   TInt iIAP;
   TInt iOldIAP;
   TBool iLinkLayerKeepAlive;

   /// Which language to use (only for multi language clients)
   TInt iLanguage;
   TInt iOldLanguage;

   /// Debug variable, set when debug settings should be shown.
   TInt showDebug;

   /// Variable to allow us to hide some settings for trial version
   TInt isTrialVersion;

   /// Which distance units to use: 1 = metric, 2 = yards, 3 = feet.
   TInt iDistanceMode;

   /**
    * Mute turn sounds when in background. Change from const when setting
    * exist in SettingsView.
    */   
   const TInt iMuteWhenInBackground;

   // Determines how directions should be represented
   MC2Direction::RepresentationType iDirectionType;

   FeedSettings::OutputFormat iFeedOutputFormat;

   TInt iVicinityFeedRate;
   TInt iLockedNavRate;
   TInt iWalkingSpeedCutOff;
   TInt iVicinityListPeriod;
   
   /// Set if the active audio syntax supports Feet-Miles.
   TBool iAudioHasFeetMiles;

   /// Set if the active audio syntax supports Yards-Miles.
   TBool iAudioHasYardsMiles;

   /// The size of the vector maps file cache
   TInt iMapCacheSize;

   /// How the map is orientated when tracking
   TrackingType iMapTrackingOrientation;

   /// True if vector maps are displayed
   MapType iMapType;
   MapType iOldMapType;

   /// Preferred Guide mode, map or guide view?
   enum preferred_guide_mode iPreferredGuideMode;
   
   /// How to show favorites in map
   TInt iFavoriteShow;

   /// Connect automatically to GPS?
   TInt iGPSAutoConnect;

   /// Map layer array (checkbox setting)
   CArrayFixFlat<TInt> iCheckboxArray;
   CArrayFixFlat<char *> iCheckboxTextArray;
   CArrayFixFlat<TInt> iCheckboxIdArray;
   TInt iShowCategories;
   TBool iAutoTrackingOn;
   TBool iCanShowWebUsername;
   TBool iCanShowWebPassword;
   TBool iCanChangeMapType;

   // FIXME - this does not belong here!!!!
   // used by the LanguageSettingItem for now.
   TBuf<20> iAudioFormatPrefix;
   TBuf<KBuf256Length> iResourceNoExt;

   // Tracking
   TInt iTrackingLevel;
   TInt iOldTrackingLevel;
   TBuf<KBuf64Length> iTrackingPIN;
   TBuf<KBuf64Length> iOldTrackingPIN;
   isab::TrackPINList* m_trackPINList;

   TBool iShowTraffic;
   TBool iTrafficEnabled;
   TBool iTrafficOldEnabled;
   TBuf<KBuf64Length> iTrafficTime;
   TBuf<KBuf64Length> iTrafficOldTime;
   class TileMapLayerInfoVector *iMapLayerSettingsVector;

   // Keeps track of which languages we define as available.
   CDesCArray* iAllowedLangs;

   isab::UserRights* iUserRights;

   TBool iAllowGps;
   TBool iIsIron;
   void SetBtGpsAddressL(const char** aStrings, TInt aCount);
   TInt iGpsSetting;
   HBufC* iGpsName;
   class CDesC8Array* iGpsParam;

   TBool iShowLangsWithoutVoice;
   TBool iShowLangsWithoutResource;
   // Defines if we should update traffic info for routes and
   // how often to do it.
   TBool iRouteTrafficInfoEnabled;
   TInt  iRouteTrafficInfoUpdTime;
};

#endif

// End of File
