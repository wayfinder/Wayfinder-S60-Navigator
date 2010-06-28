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
#include "RsgInclude.h" 
#include "wayfinder.hrh"

#include <coeutils.h>
#include <eikappui.h>
#include <eikapp.h>

#include "WayFinderConstants.h"
#include "SettingsListBox.h"
#include "SettingsData.h"
#include "IAPSettingItem.h"
#include "GpsSettingItem.h"
#include "MapLayersCheckBox.h"
#include "LanguageSettingItem.h"
#include <aknsettingitemlist.h>

#include "memlog.h"

// ================= MEMBER FUNCTIONS =========================================

// ----------------------------------------------------------------------------
// CSettingsListBox::CreateSettingItemL
// Updates descripter array for list box
// ----------------------------------------------------------------------------
//
CAknSettingItem* CSettingsListBox::CreateSettingItemL( TInt aIdentifier )
{
   CAknSettingItem* settingItem = NULL;
   switch ( aIdentifier )
   {
   case EWayFinderSettingMapSetting1:
      if (iData->iShowTraffic) {
         settingItem = new (ELeave)CAknBinaryPopupSettingItem(
               aIdentifier, iData->iTrafficEnabled );
      }
      break;
   case EWayFinderSettingMapSetting2:
      if (iData->iShowTraffic) {
         settingItem = new (ELeave)CAknTextSettingItem(
               aIdentifier, iData->iTrafficTime );
      }
      break;

   case EWayFinderSettingTrackingAutoOn:
      if (!iData->iIsIron) {
         settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, iData->iAutoTrackingOn );
         LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      }
      break;
   case EWayFinderSettingPreferredGuideMode:
      if (!iData->iIsIron) {
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iPreferredGuideMode );
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      }
      break;
   case EWayFinderSettingMapLayers:
      if (iData->iShowCategories) {
         settingItem = new (ELeave) CMapLayersCheckBox (
               aIdentifier,
               (iData->CheckboxArray()),
               (iData->CheckboxTextArray()));
      }
      break;
   case EWayFinderSettingWebUsername:
      if (!iData->isTrialVersion && iData->iCanShowWebUsername) {
         settingItem = new (ELeave)CAknTextSettingItem( aIdentifier,
                                                        iData->iWebUsername );
         LOGNEW(settingItem, CAknTextSettingItem);
      }
      break;
   case EWayFinderSettingWebPassword:
   {
      if (!iData->isTrialVersion && iData->iCanShowWebPassword) {
         CCoeEnv::Static()->ReadResource( iData->iWebPassword,
               R_WAYFINDER_SET_PASSWORD_TEXT );
         
         settingItem = new (ELeave)CAknTextSettingItem( aIdentifier,
                                                        iData->iWebPassword );
         LOGNEW(settingItem, CAknTextSettingItem);
      }
      }
      break;
   case EWayFinderSettingUsername:
      settingItem = new (ELeave)CAknTextSettingItem( aIdentifier,
                                                     iData->iUsername );
      LOGNEW(settingItem, CAknTextSettingItem);
      break;
   case EWayFinderSettingPassword:
      settingItem = new (ELeave)CAknPasswordSettingItem( aIdentifier,
            CAknPasswordSettingItem::EAlpha,
            iData->iPassword );
      LOGNEW(settingItem, CAknPasswordSettingItem);
      break;
   case EWayFinderSettingServer:
      settingItem = new (ELeave)CAknTextSettingItem( aIdentifier,
            iData->iServer );
      LOGNEW(settingItem, CAknTextSettingItem);
      break;
   case EWayFinderSettingVolume:
      if (!iData->iIsIron) {
         settingItem = new (ELeave)CAknVolumeSettingItem( aIdentifier, iData->iVolume );
         LOGNEW(settingItem, CAknVolumeSettingItem);
      }
      break;
   case EWayFinderSettingGpsSpeedLimit:
      settingItem = 
         new (ELeave)CAknIntegerEdwinSettingItem(aIdentifier, 
                                                 iData->iWalkingSpeedCutOff);
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingVicinityFeedRate:
      settingItem = 
         new (ELeave)CAknIntegerEdwinSettingItem(aIdentifier, 
                                                 iData->iVicinityFeedRate);
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingLockedNavRate:
      settingItem = 
         new (ELeave)CAknIntegerEdwinSettingItem(aIdentifier, 
                                                 iData->iLockedNavRate);
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingVicinityListRate:
      settingItem = 
         new (ELeave)CAknIntegerEdwinSettingItem(aIdentifier, 
                                                 iData->iVicinityListPeriod);
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingDirection:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iDirectionType );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingFeed:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iFeedOutputFormat );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingAutoRoute:
      if (!iData->iIsIron) {
         settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, iData->iAutoReroute );
         LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      }
      break;
   case EWayFinderSettingSpeaker:
#ifdef ONLY_MUTE_SETTING
      if (!iData->iIsIron) {
         settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, iData->iUseSpeaker );
         LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      }
#endif
      break;
   case EWayFinderSettingTurnSoundsLevel:
#ifndef ONLY_MUTE_SETTING
      if (!iData->iIsIron) {
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iTurnSoundsLevel );
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      }
#endif
      break;
   case EWayFinderSettingMuteTurnSound:
      break;
   case EWayFinderSettingVehicle:
      iData->iOldVehicle = iData->iVehicle;
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iVehicle );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingDistMode:
      // Only show the distance selection of the audio language supports both feets and yards.
      if (iData->iAudioHasFeetMiles && iData->iAudioHasYardsMiles) {
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iDistanceMode );
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      }
      break;
   case EWayFinderSettingTimeDist:
      if (iRelease) {
/*          if (iData->iTimeDist == ::isab::NavServerComEnums::TIME_WITH_DISTURBANCES) { */
/*             iData->iTimeDist = ::isab::NavServerComEnums::TIME; */
/*          } */
/*          settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iTimeDist ); */
/*          LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem); */
      }
      break;
   case EWayFinderSettingTimeDistDisturbances:
/*       if ( ! iRelease) { */
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iTimeDist );
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
/*       } */
      break;
   case EWayFinderSettingTollRoads:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iTollRoads );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingHighways:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iHighways );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingSaveSMSDest:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iSaveSMSDest );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingAutorouteSMS:
      settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, iData->iAutoSMSRoute );
      LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      break;
   case EWayFinderSettingKeepSMS:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iKeepSMSDest );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingBacklight:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iBacklightStrategy );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingLeftSideTraffic:
      //settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iLeftSideTraffic );
      //LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingIAP:
      settingItem = new (ELeave)CIAPSettingItem( aIdentifier, iData->iIAP, iRelease );
      LOGNEW(settingItem, CIAPSettingItem);
      break;
   case EWayFinderSettingGPRSKeepAlive:
      settingItem =
         new (ELeave) CAknBinaryPopupSettingItem( aIdentifier, 
                                                  iData->iLinkLayerKeepAlive );
      LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      break;
   case EWayFinderSettingLanguage:
      {
         /* Check if we have an .rsc file. */
         HBufC* rscPath = HBufC::NewLC(256);
         rscPath->Des().Copy(iData->iApplicationPath);
         rscPath->Des().Append(iData->iApplicationNameAsPath);
         rscPath->Des().Append(_L(".rsc"));
         TBool aGotRSCFile = ConeUtils::FileExists(*rscPath);
         
         CleanupStack::PopAndDestroy(rscPath);
         if ( !aGotRSCFile ) {
            
            if (!iData->iAllowedLangs) {
               settingItem = 
                  new (ELeave)CLanguageSettingItem(aIdentifier, iData, iData->iLanguage);
               LOGNEW(settingItem, CLanguageSettingItem);
            } else {
               if (iData->iAllowedLangs->MdcaCount() > 1) {
                  // If we only have one specified allowed lang we should not show the lang setting.
                  settingItem = 
                     new (ELeave)CLanguageSettingItem(aIdentifier, iData, iData->iLanguage);
                  LOGNEW(settingItem, CLanguageSettingItem);
               }
            }
         }
      }
      break;
   case EWayFinderSettingMapCacheSize:
   {
      settingItem = new (ELeave)CAknSliderSettingItem( aIdentifier, iData->iMapCacheSize );
      LOGNEW(settingItem, CAknTextSettingItem);
      break;
   }
   case EWayFinderSettingTrackingOrientation:
   {
      if (!iData->iIsIron) {
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iMapTrackingOrientation );
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      }
      break;
   }
   case EWayFinderSettingMapType:
   {
      if (iData->iCanChangeMapType) {
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iMapType);
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      }
      break;
   }
   case EWayFinderSettingFavoriteShow:
   {
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iFavoriteShow);
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   }
   case EWayfinderSettingGPSSelector:
      {
         if (iData->iAllowGps) {
            settingItem = 
               new (ELeave) CGpsSettingItem(aIdentifier, iData->iGpsSetting,
                                            iData->iGpsName);
            LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
         }
      }
      break;
   case EWayFinderSettingTrackingLevel : {
      if ( !iData->isTrialVersion ) {
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->iTrackingLevel);
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      }
   } break;
   case EWayFinderSettingTrackingPIN : {
      if ( !iData->isTrialVersion ) {
         settingItem = new (ELeave)CAknTextSettingItem( 
            aIdentifier, iData->iTrackingPIN );
         LOGNEW( settingItem, CAknTextSettingItem );
      }
   } break;
   case EWayFinderSettingRouteSetting1 : {
      settingItem = 
         new (ELeave)CAknBinaryPopupSettingItem(aIdentifier, 
                                                iData->iRouteTrafficInfoEnabled);
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);      
      break;
   } 
   case EWayFinderSettingRouteSetting2 : {
      settingItem = 
         new (ELeave)CAknIntegerEdwinSettingItem(aIdentifier, 
                                                 iData->iRouteTrafficInfoUpdTime);
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   } 
   default:
      break;
   }
   return settingItem;
}


// ----------------------------------------------------------------------------
// TKeyResponse CSettingsListBox::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
TKeyResponse CSettingsListBox::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                               TEventCode aType )
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }
   else{
      return CAknSettingItemList::OfferKeyEventL(aKeyEvent, aType);
   }
}

// ----------------------------------------------------------------------------
// CSettingsListBox::SizeChanged
// Size changed handler
// ----------------------------------------------------------------------------
//
void CSettingsListBox::SizeChanged()
{
   if (ListBox()){
      ListBox()->SetRect(Rect());
   }
}

// End of File

