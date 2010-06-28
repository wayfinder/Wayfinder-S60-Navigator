/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "arch.h"
#include "GuiProt/GuiParameterEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GeneralParamReceiver.h"
#include "GuiProt/GuiProtMessageHandler.h"
#include "GenericSettingsData.h"

namespace isab {

GeneralParamReceiver::~GeneralParamReceiver()
{
   /* Deregister from general parameter messages. */
   m_guiProtHandler->DeregisterReceiver(this,
         isab::GuiProtEnums::SET_GENERAL_PARAMETER);
   m_guiProtHandler->DeregisterReceiver(this,
         isab::GuiProtEnums::GET_GENERAL_PARAMETER);
}

void
GeneralParamReceiver::init()
{
   m_guiProtHandler->RegisterReceiver(this,
         isab::GuiProtEnums::SET_GENERAL_PARAMETER);
   m_guiProtHandler->RegisterReceiver(this,
         isab::GuiProtEnums::GET_GENERAL_PARAMETER);
}

bool
GeneralParamReceiver::decodedParamNoValue(uint32 paramId)
{
   /* Parameter was unset. */
   /* Set default value if needed. */

   return false;
}
bool
GeneralParamReceiver::decodedParamValue(uint32 paramId,
         const float* data, int32 numEntries)
{
   return false;
}
bool
GeneralParamReceiver::decodedParamValue(uint32 paramId,
         const uint8* data, int32 numEntries)
{
   return false;
}
bool
GeneralParamReceiver::decodedParamValue(uint32 paramId,
         const char** data, int32 numEntries)
{
   return false;
}
bool
GeneralParamReceiver::decodedParamValue(uint32 paramId,
         const int32* data, int32 numEntries)
{
   switch (paramId) {
      case GuiProtEnums::paramAutoReroute:
      {
         m_settingsData->m_autoReroute = data[0];
      } break;
      case GuiProtEnums::paramBacklightStrategy:
      {
         m_settingsData->m_backlightStrategy = data[0];
      } break;
      case GuiProtEnums::paramAutoTracking:
      {
         m_settingsData->m_autoTracking = data[0];
      } break;
      case GuiProtEnums::paramDistanceMode:
      {
         m_settingsData->m_distanceMode = data[0];
      } break;
      case GuiProtEnums::paramFavoriteShow:
      {
         m_settingsData->m_favoriteShowInMap = data[0];
      } break;
      case GuiProtEnums::paramHighways:
      {
         m_settingsData->m_routeHighways = data[0];
      } break;
      case GuiProtEnums::paramTollRoads:
      {
         m_settingsData->m_routeTollRoads = data[0];
      } break;
      case GuiProtEnums::paramTimeDist:
      {
         m_settingsData->m_routeCostType = data[0];
      } break;
      case GuiProtEnums::paramTrackingLevel:
      {
         m_settingsData->m_trackingLevel = data[0];
      } break;
      case GuiProtEnums::paramTransportationType:
      {
         m_settingsData->m_transportationType = data[0];
      } break;
      case GuiProtEnums::paramTurnSoundsLevel:
      {
         m_settingsData->m_turnSoundsLevel = data[0];
      } break;
      case GuiProtEnums::userTrafficUpdatePeriod:
      {
         int32 val = data[0];
         // Lower bits are time in minutes.
         m_settingsData->m_trafficUpdatePeriod = 0xbfffffff & val;
         m_settingsData->m_trafficOldUpdatePeriod = 0xbfffffff & val;
         // Second highest bit is on/off, but inverted,
         // ie. set when off.
         m_settingsData->m_trafficUpdate = 0x40000000 & val ? 0 : 1;
         m_settingsData->m_trafficOldUpdate = 0x40000000 & val ? 0 : 1;
      } break;
      case GuiProtEnums::paramAutomaticRouteOnSMSDest:
      {
         m_settingsData->m_autoRouteOnSMSDest = data[0];
      } break;
      case GuiProtEnums::paramKeepSMSDestInInbox:
      {
         m_settingsData->m_keepSMSDestInInbox = data[0];
      } break;
      case GuiProtEnums::paramStoreSMSDestInMyDest:
      {
         m_settingsData->m_storeSMSDestInFavorites = data[0];
      } break;
      case GuiProtEnums::paramPositionSymbol:
      {
         m_settingsData->m_positionSymbol = data[0];
      } break;
      case GuiProtEnums::paramCheckForUpdates:
      {
         m_settingsData->m_checkForUpdates = data[0];
      } break;
      default:
      /* Unknown. */
      return false; // Signal as not handled.
      break;
   }

   return true; // Signal as handled.
}
   
bool
GeneralParamReceiver::GuiProtReceiveMessage(class GuiProtMess *mess)
{
   GuiProtEnums::MessageType type = mess->getMessageType();

   if (type == GuiProtEnums::GET_GENERAL_PARAMETER) {
      isab::GeneralParameterMess* gpm =
            (isab::GeneralParameterMess *)mess;
      return decodedParamNoValue(gpm->getParamId());
   } else if (type == GuiProtEnums::SET_GENERAL_PARAMETER) {
      isab::GeneralParameterMess* gpm =
            (isab::GeneralParameterMess *)mess;
      switch (gpm->getParamType()) {
         case isab::GuiProtEnums::paramTypeInt32:
            return decodedParamValue(gpm->getParamId(),
                  gpm->getIntegerData(), gpm->getNumEntries());
            break;
         case isab::GuiProtEnums::paramTypeFloat:
            return decodedParamValue(gpm->getParamId(),
                  gpm->getFloatData(), gpm->getNumEntries());
            break;
         case isab::GuiProtEnums::paramTypeBinary:
            return decodedParamValue(gpm->getParamId(),
                  gpm->getBinaryData(), gpm->getNumEntries());
            break;
         case isab::GuiProtEnums::paramTypeString:
            return decodedParamValue(gpm->getParamId(),
                  (const char**)gpm->getStringData(), gpm->getNumEntries());
            break;
         default:
         case isab::GuiProtEnums::paramTypeInvalid:
            return false;
            break;
      }
   }
   return false;
}

}


