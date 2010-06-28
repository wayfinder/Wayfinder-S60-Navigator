/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WAYFINDER_SYMBIAN_BASE_UI_H
#define WAYFINDER_SYMBIAN_BASE_UI_H

#include "WayfinderUiBase.h"        // WayfinderUiBase
#include "WFServiceViewHandler.h"   // WFServiceViewHandler
#include "WFStartupEventEnum.h"     // StartupState enum
#include "EventGenerator.h"         // CEventGenerator template
#include "CommunicationWizardBase.h"

#include <deque>

namespace isab {
   class VectorMapCoordinates;
   class TopRegionList;
   class Favorite;
}

class WayfinderSymbianUiBase :
   public WayfinderUiBase,
   public WFServiceViewHandler
{
public:
   WayfinderSymbianUiBase();
   void ConstructL();

   void GenerateEvent(enum StartupState aEvent, int32 eventDelay);
   void HandleGeneratedEventL(enum StartupState aEvent);

protected:
   virtual ~WayfinderSymbianUiBase();

public:
   /**
    * Methods that must be implemented by the real GUI.
    */
   /**
    * Start the IAP search.
    */
   virtual void StartIapSearch() = 0;
   /**
    * Reinitialize variables for IAP and restart the IAP search.
    */
   virtual void RestartIapSearch() = 0;

   /**
    * Move to Service Window using the specified URL.
    */
   virtual void GotoServiceViewL(const TDesC& aUrl, enum BackActionEnum allowBack = BackIsHistoryThenView) = 0;

   /**
    * Shutdown application.
    */
   virtual void ShutdownNow() = 0;

   /**
    * Clear the browser cache.
    */
   virtual void ClearBrowserCache() = 0;

   /**
    * Move to main menu of application.
    */
   virtual void GotoMainMenu() = 0;

   virtual void ActivateSplashViewL() = 0;

   /**
    * Shows a dialog informing the user about sending the sms, 
    * if the user approves we send the sms.
    */
   virtual void ShowRegistrationSmsDialog() = 0;

   /**
    * Startup is finished.
    * Default implementation is to trigger SS_GotoMainMenu.
    */
   virtual void StartupFinished();

   /**
    * If still in startup.
    */
   bool inStartup() const;

   /**
    * Show message to user to inform about startup failure.
    */
   virtual void ReportStartupFailed(class WFStartupEventCallbackParameter* param) = 0;

   /**
    * Show error message to user to inform what went wrong.
    */
   virtual void ReportStartupError(class WFStartupEventCallbackParameter* param) = 0;

   /**
    * Check if phone is in flight mode or not.
    */
   virtual void CheckFlightMode() = 0;

   /**
    * Show message to user to inform about upgrade failure.
    */
   virtual void ShowUpgradeErrorL(class WFStartupEventCallbackParameter* param) = 0;

   /**
    * isWFIDStartup - returns true if USE_WF_ID is defined.
    */
   virtual bool isWFIDStartup() = 0;

public:
   /**
    * Interface for the settings.
    */
   /**
    * handleSetLatestNewsImage - Save the latest news image.
    */
   virtual void handleSetLatestNewsImage(const uint8* data, int32 numEntries) = 0;
   /**
    * handleSetVolume - Set volume.
    */
   virtual void handleSetVolume(int32 vol) = 0;

   /**
    * handleSetUseSpeaker - Set if speaker should be used.
    */
   virtual void handleSetUseSpeaker(bool on) = 0;

   /**
    * handleSetBtGpsAddressAndName - Set the BT gps address data.
    */
   virtual void handleSetBtGpsAddressAndName(const char** data, int32 numEntries) = 0;

   /**
    * handleSetCategoryIds - Set ids for search categories.
    */
   virtual void handleSetCategoryIds(const char** data, int32 numEntries) = 0;
   /**
    * handleSetCategoryNames - Set names for search categories.
    */
   virtual void handleSetCategoryNames(const char** data, int32 numEntries) = 0;
   /**
    * handleSetCategoryIconNames - Set names for the icon files of the search categories.
    */
   virtual void handleSetCategoryIconNames(const char** data, int32 numEntries) = 0;
   /**
    * handleSetCategoryIntIds  - Set integer ids for search categories.
    */
   virtual void handleSetCategoryIntIds(const int32* data, int32 numEntries) = 0;
   /**
    * handleSetServerNameAndPort - Set server name setting.
    * Transfers ownership of string.
    */
   virtual void handleSetServerNameAndPort(HBufC* server) = 0;
   /**
    * handleSetHttpServerNameAndPort - Set server name setting.
    * Transfers ownership of string.
    */
   virtual void handleSetHttpServerNameAndPort(HBufC* server) = 0;
   /**
    * handleSetMapLayerSettings - Set map layer settings.
    * If data is set to NULL, the parameter is treated as unset.
    */
   virtual void handleSetMapLayerSettings(const uint8* data, int32 numEntries) = 0;
   /**
    * handleSetPoiCategories - Set POI category settings.
    * If data is set to NULL, the parameter is treated as unset.
    */
   virtual void handleSetPoiCategories(const uint8* data, int32 numEntries) = 0;
   /**
    * handleSetSearchHistory - Set search history strings from Nav2.
    */
   virtual void handleSetSearchHistory(const uint8* data, int32 numEntries) = 0;
   /**
    * handleSetTrackingPIN - Set tracking pin strings.
    */
   virtual void handleSetTrackingPIN(const uint8* data, int32 numEntries) = 0;
   /**
    * handleSetUsernameAndPassword - Username and password has been set.
    */
   virtual void handleSetUsernameAndPassword() = 0;
   /**
    * handleSetVectorMapSettings - Settings for vector maps.
    */
   virtual void handleSetVectorMapSettings(const int32* data, int32 numEntries) = 0;
   /**
    * handleSetVectorMapCoordinates - Scale and last coordinates for map.
    */
   virtual void handleSetVectorMapCoordinates(isab::VectorMapCoordinates* vmc) = 0;
   /**
    * handleSetSelectedAccessPointId - Saved IAP id.
    */
   virtual void handleSetSelectedAccessPointId(int32 iap) = 0;
   /**
    * handleSetSelectedAccessPointId2 - Currently used IAP id.
    */
   virtual void handleSetSelectedAccessPointId2(int32 iap) = 0;
   /**
    * handleSetTopRegionList - Set the top region list for searches.
    */
   virtual void handleSetTopRegionList(isab::TopRegionList* topRegionList) = 0;

   /**
    * handleSetACPSetting - Set ACP enabled or disabled.
    */
   virtual void handleSetACPSetting(int32 enableACP) = 0;

   /**
    * handleCheckForUpdatesSetting - Check for update settings enabled or disabled.
    */
   virtual void handleCheckForUpdatesSetting(int32 aValue) = 0;

   /**
    * setNewVersionNumber - Set new version number.
    */
   virtual void setNewVersionNumber(const char* version) = 0;

   /**
    * setNewVersionUrl - Set new version url.
    */
   virtual void setNewVersionUrl(const char* versionUrl) = 0;

   /**
    * handleSetTopRegionList - Set the top region list for searches.
    *
    * Default implementation is to do nothing.
    */
   virtual void handleSetAutoGpsConnect(int32 /* autoGpsConnect */) {}
   /**
    * handleSetPositionSymbol - Set the position symbol.
    *
    * Default implementation is to do nothing.
    */
   virtual void handleSetPositionSymbol(int32 /* symbol */) {}
   
   /**
    * From WayfinderUiBase
    */
   virtual void BaseUiCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   virtual void BaseUiTriggerEvent(int32 eventType, 
                                   class WFStartupEventCallbackParameter* param,
                                   int32 eventDelay);

   /**
    * From WFServiceViewHandler
    */
   virtual void GotoServiceViewUrl(const char* url, enum BackActionEnum allowBack);

   /**
    * Starts the Communication wizard to send
    * a favorite.
    * @param aFavorite, the favorite to be send.
    */
   virtual void SendFavoriteL(isab::Favorite* aFavorite);
   
   /**
    * Starts the Communication wizard to send
    * the current map position.
    */
   virtual void SendMapPositionL();

protected:

   /// CommunicationWizard that lets the user send favorite, map pos
   /// etc as a email or sms.
   CCommunicationWizardBase* iComWiz;

private:

   typedef CEventGenerator<WayfinderSymbianUiBase, enum StartupState>
      CWFSymbianUiBaseEventGenerator;

   CWFSymbianUiBaseEventGenerator* iSymbianBaseEventGenerator;

   std::deque<class WFStartupEventCallbackParameter*> m_triggerEventParams;   

   /// If in startup.
   bool m_startup;
};

#endif
