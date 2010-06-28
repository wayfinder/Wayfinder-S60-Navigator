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
#include <eikenv.h>
#include <e32base.h>
#include "arch.h"

#include "WayFinderConstants.h"
#include "NavServerComEnums.h"
#include "GuiProt/HelperEnums.h"
#include "GuiProt/GuiProtEnums.h"

#include "S60SymbianSettingsData.h"

// Version number for Vector map settings.
// WARNING! The vector map blob is generated in the make_seed.pl file!
#define VECTOR_MAP_SETTINGS_VERSION 1

#define VECTOR_MAP_POI_CATEGORY_VERSION 5

// FORWARD DECLARATIONS

namespace isab{
   class Log;
   class TrackPINList;
   class UserRights;
}

// CLASS DECLARATION

/**
* CSettingsData 
*/
class CSettingsData
 : public CBase,
   public S60SymbianSettingsData
{
public:
   static CSettingsData* NewL(class CWayfinderDataHolder* aDataStore);
   ~CSettingsData();

private:
   CSettingsData();
   void ConstructL(class CWayfinderDataHolder* aDataStore);

public:

   virtual void setUserRights(const int32* data, int32 numEntries);

   inline CArrayFix<TInt>& CheckboxArray() { return iCheckboxArray; }
   inline CArrayFix<char *>& CheckboxTextArray() { return iCheckboxTextArray; }
   inline CArrayFix<TInt>& CheckboxIdArray() { return iCheckboxIdArray; }

public:

   class CWayfinderDataHolder* iDataStore;

   /// The server name
   TBuf<KBuf256Length> iServer;
   TBuf<KBuf256Length> iServerOld;
   TBuf<KBuf64Length> iWebUsername;
   TBuf<KBuf64Length> iWebPassword;
   TBuf<KBuf64Length> iUsername;
   TBuf<KBuf64Length> iOldUsername;
   TBuf<KBuf64Length> iPassword;
   TBuf<KBuf64Length> iOldPassword;

   /// Speaker volume
   TInt iVolume;

   /// True if sound should be played through the speaker.
   TBool iUseSpeaker;

   /// Which internet access point to use
   TInt iIAP;
   TInt iOldIAP;

   int32 m_languageOld;

   /// Debug variable, set when debug settings should be shown.
   TInt showDebug;

   /// Variable to allow us to hide some settings for trial version
   TInt isTrialVersion;

   /**
    * Mute turn sounds when in background. Change from const when setting
    * exist in SettingsView.
    */   
   const TInt iMuteWhenInBackground;

   /// Set if the active audio syntax supports Feet-Miles.
   TBool iAudioHasFeetMiles;

   /// Set if the active audio syntax supports Yards-Miles.
   TBool iAudioHasYardsMiles;

   /// The size of the vector maps file cache
   TInt iMapCacheSize;

   /// How the map is orientated when tracking
   TrackingType iMapTrackingOrientation;

   /// Preferred Guide mode, map or guide view?
   enum preferred_guide_mode iPreferredGuideMode;
   
   /// Map layer array (checkbox setting)
   CArrayFixFlat<TInt> iCheckboxArray;
   CArrayFixFlat<char *> iCheckboxTextArray;
   CArrayFixFlat<TInt> iCheckboxIdArray;
   TInt iShowCategories;

   int32 m_trackingLevelOld;
   TBuf<KBuf64Length> iTrackingPIN;
   TBuf<KBuf64Length> iOldTrackingPIN;
   isab::TrackPINList* m_trackPINList;

   TBool iShowTraffic;
   class TileMapLayerInfoVector *iMapLayerSettingsVector;

   // Keeps track of which languages we define as available.
   CDesCArray* iAllowedLangs;

   isab::UserRights* iUserRights;

   void SetBtGpsAddressL(const char** aStrings, TInt aCount);
   TInt iGpsSetting;
   HBufC* iGpsName;
   class CDesC8Array* iGpsParam;

   // If true then check foe a new version on startup
   TBool iCheckForUpdates;
};

#endif

// End of File
