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
#include <bautils.h>

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
   iData->iShowTraffic = EFalse;
   switch ( aIdentifier )
   {
   case EWayFinderSettingMapLayers:
      if (iData->iShowCategories) {
         settingItem = new (ELeave) CMapLayersCheckBox (
               aIdentifier,
               (iData->CheckboxArray()),
               (iData->CheckboxTextArray()));
      }
      break;
   case EWayFinderSettingWebUsername:
      if (iData->m_canShowWebUsername) {
         settingItem = new (ELeave)CAknTextSettingItem( aIdentifier,
                                                        iData->iWebUsername );
         LOGNEW(settingItem, CAknTextSettingItem);
      }
      break;
   case EWayFinderSettingWebPassword:
      {
      if (iData->m_canShowWebPassword) {
         CCoeEnv::Static()->ReadResource( iData->iWebPassword,
               R_WAYFINDER_SET_PASSWORD_TEXT );
         
//          settingItem = new (ELeave)CAknTextSettingItem( aIdentifier,
//                                                         iData->iWebPassword );
         settingItem = new (ELeave)CAknPasswordSettingItem( aIdentifier,
                                                            CAknPasswordSettingItem::EAlpha,
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
      if (!iData->m_isIron) {
         settingItem = new (ELeave)CAknVolumeSettingItem( aIdentifier, iData->iVolume );
         LOGNEW(settingItem, CAknVolumeSettingItem);
      }
      break;
   case EWayFinderSettingAutoRoute:
      if (!iData->m_isIron) {
         settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, (TBool&)iData->m_autoReroute );
         LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      }
      break;
   case EWayFinderSettingTurnSoundsLevel:
      if (!iData->m_isIron) {
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( (TInt)aIdentifier, (TInt&)iData->m_turnSoundsLevel );
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      }
      break;
   case EWayFinderSettingDistMode:
      // Only show the distance selection of the audio language supports both feets and yards.
      if (iData->iAudioHasFeetMiles && iData->iAudioHasYardsMiles) {
         settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->m_distanceMode );
         LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      }
      break;
   case EWayFinderSettingTimeDistDisturbances:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->m_routeCostType );
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   case EWayFinderSettingTollRoads:
      settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, (TBool&)iData->m_routeTollRoads );
      LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      break;
   case EWayFinderSettingHighways:
      settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, (TBool&)iData->m_routeHighways );
      LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      break;
   case EWayFinderSettingBacklight:
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->m_backlightStrategy );
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
//          new (ELeave) CAknEnumeratedTextPopupSettingItem( aIdentifier, 
//             (TInt&)iData->m_linkLayerKeepAlive );
         new (ELeave) CAknBinaryPopupSettingItem( aIdentifier, 
                                                  (TBool&)iData->m_linkLayerKeepAlive );
      LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      break;
   case EWayFinderSettingCheckForUpdates:
      settingItem =
         new (ELeave) CAknBinaryPopupSettingItem( aIdentifier, 
                                                  (TBool&)iData->iCheckForUpdates );
      break;
   case EWayFinderSettingLanguage:
      {
         if (iOneLanguage) {
            // If we should use only one language or select the phone language 
            // with bafl utils then we should have no setting for lang.
            return NULL;
         }

         /* Check if we have an .rsc file. */
         _LIT(KRscExt, ".rsc");
         HBufC* rscPath = HBufC::NewLC(256);
         rscPath->Des().Copy(*(iData->m_resourceNoExt));
         rscPath->Des().Append(KRscExt);
         RFs fs;
         fs.Connect();
         TBool aGotRSCFile = BaflUtils::FileExists(fs, *rscPath);
         fs.Close();

         CleanupStack::PopAndDestroy(rscPath);
         if ( !aGotRSCFile ) {
            
            if (!iData->iAllowedLangs) {
               settingItem = 
                  new (ELeave)CLanguageSettingItem(aIdentifier, iData, (TInt&)iData->m_language);
               LOGNEW(settingItem, CLanguageSettingItem);
            } else {
               if (iData->iAllowedLangs->MdcaCount() > 1) {
                  // If we only have one specified allowed lang we should not show the lang setting.
                  settingItem = 
                     new (ELeave)CLanguageSettingItem(aIdentifier, iData, (TInt&)iData->m_language);
                  LOGNEW(settingItem, CLanguageSettingItem);
               }
            }
         }
      }
      break;
   case EWayFinderSettingFavoriteShow:
   {
      settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->m_favoriteShowInMap);
      LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
      break;
   }
   case EWayFinderSettingEnableACP:
   {
      if (iData->m_acpSettingEnabled > 0) {
         // Just to be sure we don't step outside the array
         iData->m_acpSettingEnabled = 1;
      }
      settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, (TInt&)iData->m_acpSettingEnabled);
      LOGNEW(settingItem, CAknBinaryPopupSettingItem);
      break;
   }
   case EWayfinderSettingGPSSelector:
#if !defined NOKIALBS && !defined __WINS__
      {
         if (iData->m_allowGps) {
            settingItem = 
               new (ELeave) CGpsSettingItem(aIdentifier, iData->iGpsSetting,
                                            iData->iGpsName);
            LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
         }
      }
#endif
      break;
   case EWayFinderSettingTrackingLevel : {
      if ( !iData->isTrialVersion ) {
         settingItem = new (ELeave)CAknBinaryPopupSettingItem( aIdentifier, (TInt&)iData->m_trackingLevel);
         //settingItem = new (ELeave)CAknEnumeratedTextPopupSettingItem( aIdentifier, (TInt&)iData->m_trackingLevel);
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
      // This setting now also controls the EWayFinderSettingMapSetting1
      //settingItem = 
      //   new (ELeave)CAknBinaryPopupSettingItem(aIdentifier, 
      //         (TBool&)iData->m_trafficUpdate);
      //LOGNEW(settingItem, CAknBinaryPopupSettingItem);      
      break;
   } 
   case EWayFinderSettingRouteSetting2 : {
      // This setting now also controls the EWayFinderSettingMapSetting2
      //settingItem = 
      //   new (ELeave)CAknIntegerEdwinSettingItem(aIdentifier, 
      //         (TInt&)iData->m_trafficUpdatePeriod);
      //LOGNEW(settingItem, CAknEnumeratedTextPopupSettingItem);
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
