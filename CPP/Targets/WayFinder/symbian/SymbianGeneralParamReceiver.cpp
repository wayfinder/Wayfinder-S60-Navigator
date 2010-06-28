/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SymbianGeneralParamReceiver.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtMessageHandler.h"
#include "WFDataHolder.h"
#include "SymbianSettingsData.h"
#include "GuiProt/GuiParameterEnums.h"
#include "WFStartupEvent.h"
#include "WFStartupHandler.h"
#include "WFNewsData.h"
#include "WFAccountData.h"
#include "IAPDataStore.h"
#include "WayfinderSymbianUiBase.h"
#include "WFTextUtil.h"
#include "GuiProt/VectorMapCoordinates.h"
#include "Buffer.h"
#include "GuiProt/UserTermsEnums.h"

#define DEFAULT_IAP -1

SymbianGeneralParamReceiver::SymbianGeneralParamReceiver(
      CWayfinderDataHolder* aDataStore,
      WFStartupHandler* aStartupHandler,
      GuiProtMessageHandler* aHandler,
      WayfinderSymbianUiBase* aAppUi)
 : GeneralParamReceiver(aDataStore->iSettingsData, aHandler),
   m_dataStore(aDataStore),
   m_startupHandler(aStartupHandler),
   m_appUi(aAppUi)
{
}
class SymbianSettingsData*
SymbianGeneralParamReceiver::Settings()
{
   return static_cast<SymbianSettingsData*>(m_settingsData);
}

void
SymbianGeneralParamReceiver::ConstructL()
{
   GeneralParamReceiver::init();
}

void
SymbianGeneralParamReceiver::sendIAPId()
{
   isab::GeneralParameterMess *message =
      new (ELeave) isab::GeneralParameterMess(
         isab::GuiProtEnums::paramSelectedAccessPointId,
         m_dataStore->iIAPDataStore->m_iapId);
   m_guiProtHandler->SendMessage( *message );
   message->deleteMembers();
   delete message;
}
void
SymbianGeneralParamReceiver::sendServerString(int32 paramId, char* serverString)
{
   /* At least one string, gets incremented for each ;*/
   int32 numEntries = 1;
   if (!serverString) {
      /* Not set, not allowed! */
      return;
   }
   int32 len = strlen(serverString);

   int32 i;
   for (i = 0; i < len; i++) {
      if (serverString[i] == ';') {
         /* Nul terminate. */
         numEntries++;
         serverString[i] = 0;
      }
   }

   char **serverArray = new char*[numEntries];

   int32 idx = 0;
   serverArray[idx++] = serverString;
   bool nullSeen = false;
   for (i = 0; i < len; i++) {
      if (serverString[i] == 0) {
         nullSeen = true;
      } else if (nullSeen) {
         serverArray[idx++] = &serverString[i];
         nullSeen = false;
      }
   }

   /* Note: idx may be less than numEntries, since the above code */
   /* will ignore empty strings or ";;". */
   numEntries = idx;

   isab::GeneralParameterMess* message = new isab::GeneralParameterMess(
            paramId, serverArray, numEntries );
   m_guiProtHandler->SendMessage( *message );
   /* Must not call deleteMembers(), since the data is not owned by message */
   delete message;
   delete serverArray;
   delete serverString;
}

bool
SymbianGeneralParamReceiver::decodedParamNoValue(uint32 paramId)
{
   bool handled = true;

   /* Parameter was unset. */
   /* Set default value if needed. */
   switch (paramId) {
      case isab::GuiProtEnums::paramUserTermsAccepted:
         if (!m_appUi->isWFIDStartup()) {
            m_startupHandler->StartupEvent(SS_UserTermsNeeded);
         }
         break;
      case isab::GuiProtEnums::paramNeverShowUSDisclaimer:
         if (!m_appUi->isWFIDStartup()) {
            m_startupHandler->StartupEvent(SS_USDisclaimerDoTest);
         }
         break;
      case isab::GuiProtEnums::paramShowNewsServerString:
         handleLatestNewsChecksum("");
         break;
      case isab::GuiProtEnums::paramShownNewsChecksum:
         handleLatestShownNewsChecksum(" ");
         break;
      case isab::GuiProtEnums::paramMapLayerSettings:
         m_appUi->handleSetMapLayerSettings(NULL, 0);
         break;
      case isab::GuiProtEnums::paramPoiCategories:
         m_appUi->handleSetPoiCategories(NULL, 0);
         break;
      case isab::GuiProtEnums::paramRegistrationSmsSent:
         if (!m_appUi->isWFIDStartup()) {
            m_startupHandler->StartupEvent(SS_SendRegistrationSmsNeeded);
         }
         break;
      case isab::GuiProtEnums::paramMapACPSetting:
         m_appUi->handleSetACPSetting(1);
         break;
      case isab::GuiProtEnums::paramCheckForUpdates:
         m_appUi->handleCheckForUpdatesSetting(1);
         break;
      default:
         handled = false;
         break;
   }

   if (handled) {
      return handled;
   }
   return GeneralParamReceiver::decodedParamNoValue(paramId);
}
bool
SymbianGeneralParamReceiver::decodedParamValue(uint32 paramId,
      const uint8* data, int32 numEntries)
{
   bool handled = true;
   switch (paramId) {
      case isab::GuiProtEnums::paramTopRegionList:
         {
            isab::Buffer tmp(const_cast<uint8*>(data), numEntries, numEntries);
            tmp.setReadPos(0);
            isab::TopRegionList *topRegionList = new isab::TopRegionList(tmp);
            /* Will transfer ownership of topRegionList. */
            m_appUi->handleSetTopRegionList(topRegionList);
            /* Make sure that we will not delete data belonging to message. */
            tmp.releaseData();
         }
         break;
      case isab::GuiProtEnums::paramVectorMapCoordinates:
         {
            isab::Buffer *buf = new isab::Buffer(const_cast<uint8*>(data),
                  numEntries, numEntries);

            isab::VectorMapCoordinates* vmc =
               new (ELeave) isab::VectorMapCoordinates(buf);
            m_appUi->handleSetVectorMapCoordinates(vmc);
         }
         break;
      case isab::GuiProtEnums::paramMapLayerSettings:
         m_appUi->handleSetMapLayerSettings(
               data, numEntries);
         break;
      case isab::GuiProtEnums::paramPoiCategories:
         m_appUi->handleSetPoiCategories(
               data, numEntries);
         break;
      case isab::GuiProtEnums::paramTrackingPIN:
         {
            m_appUi->handleSetTrackingPIN(
               data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramSearchStrings:
         {
            m_appUi->handleSetSearchHistory(
               data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramLatestNewsImage:
         {
            m_appUi->handleSetLatestNewsImage(
               data, numEntries);
         }
         break;
      default:
         /* Do not return since then we will not get any calls to */
         /* GeneralParamReceiver. */
         handled = false;
         break;
   }

   if (handled) {
      return handled;
   }
   return GeneralParamReceiver::decodedParamValue(paramId, data, numEntries);
}
bool
SymbianGeneralParamReceiver::decodedParamValue(uint32 paramId,
      const char** data, int32 numEntries)
{
   bool handled = true;
   switch (paramId) {
      case isab::GuiProtEnums::paramShowNewsServerString:
         handleLatestNewsChecksum(data[0]);
         break;
      case isab::GuiProtEnums::paramShownNewsChecksum:
         handleLatestShownNewsChecksum(data[0]);
         break;
      case isab::GuiProtEnums::paramCategoryIds:
         {
            m_appUi->handleSetCategoryIds(
                  data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramCategoryNames:
         {
            m_appUi->handleSetCategoryNames(data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramCategoryIcons:
         {
            m_appUi->handleSetCategoryIconNames(data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramBtGpsAddressAndName:
         {
            m_appUi->handleSetBtGpsAddressAndName(data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramServerNameAndPort:
         {
            m_appUi->handleSetServerNameAndPort(
               convertToServerString(data, numEntries));
         }
         break;
      case isab::GuiProtEnums::paramHttpServerNameAndPort:
         {
            m_appUi->handleSetHttpServerNameAndPort(
               convertToServerString(data, numEntries));
         }
         break;
      case isab::GuiProtEnums::paramUserAndPassword:
         {
            m_dataStore->iWFAccountData->setUserName(
                  WFTextUtil::strdupL(data[0]));
            m_dataStore->iWFAccountData->setPassword(
                  WFTextUtil::strdupL(data[1]));
            m_appUi->handleSetUsernameAndPassword();
         }
         break;
      case isab::GuiProtEnums::paramWebUsername:
         {
            m_dataStore->iWFAccountData->setWebUserName(
                  WFTextUtil::strdupL(data[0]));
         }
         break;
      case isab::GuiProtEnums::paramWebPassword:
         {
            m_dataStore->iWFAccountData->setWebPassword(
                  WFTextUtil::strdupL(data[0]));
         }
         break;
      case isab::GuiProtEnums::paramNewVersion:
         {
            m_appUi->setNewVersionNumber(data[0]);
         }
         break;
      case isab::GuiProtEnums::paramNewVersionUrl:
         {
            m_appUi->setNewVersionUrl(data[0]);
         }
         break;
      default:
      /* Do not return since then we will not get any calls to */
      /* GeneralParamReceiver. */
      handled = false;
      break;
   }
   if (handled) {
      return handled;
   }
   return GeneralParamReceiver::decodedParamValue(paramId, data, numEntries);
}
bool
SymbianGeneralParamReceiver::decodedParamValue(uint32 paramId,
      const int32* data, int32 numEntries)
{
   bool handled = true;
   switch (paramId) {
      case isab::GuiProtEnums::paramWayfinderType:
         handleWayfinderType(data[0]);
         break;
      case isab::GuiProtEnums::paramNeverShowUSDisclaimer:
         if (!m_appUi->isWFIDStartup()) {
            if (data[0]) {
               m_startupHandler->StartupEvent(SS_USDisclaimerDontTest);
            } else {
               m_startupHandler->StartupEvent(SS_USDisclaimerDoTest);
            }
         }
         break;
      case isab::GuiProtEnums::paramUserTermsAccepted:
         if (!m_appUi->isWFIDStartup()) {
            if (data[0] == isab::GuiProtEnums::UserTermsAccepted) {
               m_startupHandler->StartupEvent(SS_UserTermsNotNeeded);
            } else {
               m_startupHandler->StartupEvent(SS_UserTermsNeeded);
            }
         }
         break;
      case isab::GuiProtEnums::paramCategoryIntIds:
         {
            m_appUi->handleSetCategoryIntIds(data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramSoundVolume:
         Settings()->m_soundVolume = data[0];
         m_appUi->handleSetVolume(Settings()->m_soundVolume);
         break;
      case isab::GuiProtEnums::paramUseSpeaker:
         Settings()->m_useSpeaker = data[0];
         m_appUi->handleSetUseSpeaker(Settings()->m_useSpeaker);
         break;
      case isab::GuiProtEnums::paramLastKnownRouteId:
         /* Unused. */
         break;
      case isab::GuiProtEnums::paramVectorMapSettings:
         {
            m_appUi->handleSetVectorMapSettings(data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramSelectedAccessPointId2:
         {
            m_appUi->handleSetSelectedAccessPointId2(data[0]);
         }
         break;
      case isab::GuiProtEnums::paramSelectedAccessPointId:
         {
            m_appUi->handleSetSelectedAccessPointId(data[0]);
         }
         break;
      case isab::GuiProtEnums::userRights:
         {
            Settings()->setUserRights(data, numEntries);
         }
         break;
      case isab::GuiProtEnums::paramRegistrationSmsSent:
         {
            if (!m_appUi->isWFIDStartup()) {
               m_startupHandler->StartupEvent(SS_SendRegistrationSmsNotNeeded);
            }
         }
         break;
      case isab::GuiProtEnums::paramMapACPSetting:
         m_appUi->handleSetACPSetting(data[0]);
         break;
      case isab::GuiProtEnums::paramCheckForUpdates:
         m_appUi->handleCheckForUpdatesSetting(data[0]);
         break;
      default:
         /* Do not return since then we will not get any calls to */
         /* GeneralParamReceiver. */
         handled = false;
         break;
   }

   if (handled) {
      return handled;
   }
   return GeneralParamReceiver::decodedParamValue(paramId, data, numEntries);
}

SymbianGeneralParamReceiver::~SymbianGeneralParamReceiver()
{
}

void  
SymbianGeneralParamReceiver::handleLatestShownNewsChecksum(const char* checksum)
{
   m_dataStore->iWFNewsData->setLatestShownNewsChecksum(checksum);
   checkLatestNewsChecksum();
}

void     
SymbianGeneralParamReceiver::handleLatestNewsChecksum(const char* checksum)
{           
   m_dataStore->iWFNewsData->setLatestNewsChecksum(checksum);
   checkLatestNewsChecksum();
}           

void
SymbianGeneralParamReceiver::checkLatestNewsChecksum()
{
   if (m_dataStore->iWFNewsData->checksumMatch()) {
      /* Checksums match, and both data are available. */
      if (!m_appUi->isWFIDStartup()) {
         m_startupHandler->StartupEvent(SS_NewsNotNeeded);
      }
   } else {
      if (!m_appUi->isWFIDStartup()) {
         m_startupHandler->StartupEvent(SS_NewsNeeded);
      }
   }
}

void     
SymbianGeneralParamReceiver::handleWayfinderType(int32 wft)
{
   switch(wft){
      case isab::GuiProtEnums::Trial:
      case isab::GuiProtEnums::Silver:
      case isab::GuiProtEnums::Gold:
      case isab::GuiProtEnums::Iron:
         setWayfinderType(wft);
         break;
      case isab::GuiProtEnums::InvalidWayfinderType:
         break;
   }
}

void
SymbianGeneralParamReceiver::setWayfinderType(int32 wft)
{
   m_dataStore->iWFAccountData->setWfType((isab::GuiProtEnums::WayfinderType)wft);
   int32 startupEvent = SS_TrialMode;
   switch (wft) {
      case isab::GuiProtEnums::Trial:
         break;
      case isab::GuiProtEnums::Silver:
         startupEvent = SS_SilverMode;
         break;
      case isab::GuiProtEnums::Gold:
         startupEvent = SS_GoldMode;
         break;
      case isab::GuiProtEnums::Iron:
         startupEvent = SS_IronMode;
         break;
      case isab::GuiProtEnums::InvalidWayfinderType:
         // Unhandled type - what to do?
         // Trial will have to suffice.
         break;
   }
   if ( !m_startupHandler->GetStartupFinished() &&
        !m_appUi->isWFIDStartup() ) {
      m_startupHandler->StartupEvent( startupEvent );
   }
}

_LIT(KSemiColon, ";");
#define MAXLEN 1024

HBufC *
SymbianGeneralParamReceiver::convertToServerString(
   const char** data, int32 numEntries)
{
   HBufC* serverString = HBufC::NewLC(MAXLEN);
   TPtr ss = serverString->Des();
   int32 i = 0;
   HBufC* tmp = WFTextUtil::AllocLC(data[i]);
   ss.Copy(*tmp);
   CleanupStack::PopAndDestroy(tmp);
   i++;
   while ( i < numEntries) {
      if (serverString->Length()+strlen(data[i])+2 > MAXLEN) {
         /* Too long, don't copy it... */
         break;
      }
      ss.Append(KSemiColon);
      tmp = WFTextUtil::AllocLC(data[i]);
      ss.Append(*tmp);
      CleanupStack::PopAndDestroy(tmp);
      i++;
   }
   CleanupStack::Pop(serverString);
   return serverString;
}


