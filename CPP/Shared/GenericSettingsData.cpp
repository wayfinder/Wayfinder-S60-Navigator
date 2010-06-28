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
#include "GenericSettingsData.h"

GenericSettingsData::GenericSettingsData()
 :
   m_autoReroute(1),             // On
   m_autoTracking(1),            // On
   m_backlightStrategy(1),       // On during route
   m_distanceMode(0),            // Invalid
   m_favoriteShowInMap(1),       // City level
   m_routeHighways(1),           // Allow
   m_routeTollRoads(1),          // Allow
   m_routeCostType(2),           // Traffic
   m_language(0),                // Invalid
   m_trackingLevel(0),           // ?
   m_transportationType(1),      // Passenger car
   m_turnSoundsLevel(3),         // Normal
   m_trafficUpdatePeriod(30),    // 30 minutes
   m_trafficUpdate(1),           // On
   m_trafficOldUpdatePeriod(30), // 30 minutes
   m_trafficOldUpdate(1),        // On
   m_autoRouteOnSMSDest(2),      // S60 - Ask
   m_keepSMSDestInInbox(0),      // S60 - Yes
   m_storeSMSDestInFavorites(2), // S60 - Ask
   m_positionSymbol(0),          // UIQ - 0
   m_acpSettingEnabled(1)        // Enabled
{
}

GenericSettingsData::~GenericSettingsData()
{
}

