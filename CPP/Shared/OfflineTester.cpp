/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "OfflineTester.h"

#ifndef SYMBIAN_9
# ifdef OFFLINE_TEST
#  include <settinginfo.h>
#  include <settinginfoids.h>
# endif
#else
# if defined OFFLINE_TEST || defined NETWORK_TEST
#  include <centralrepository.h>
#  include <profileenginesdkcrkeys.h>
#  include <coreapplicationuissdkcrkeys.h>
# endif
#endif

#include "TraceMacros.h"

/* #define ALWAYS_NETWORK */

namespace isab { 
   OfflineTester::OfflineTester()
#ifndef SYMBIAN_9
# if defined NETWORK_TEST
    : m_agentConnected(false)
# endif
# if defined OFFLINE_TEST
      , m_settingInfo(NULL)
# endif
#else
# if defined NETWORK_TEST 
    : m_networkrep(NULL)
# endif
# if defined OFFLINE_TEST
      , m_profilerep(NULL)
# endif
#endif
   {
#ifndef SYMBIAN_9
# ifdef NETWORK_TEST
      m_agentConnected = (KErrNone == m_agent.Connect());
//       DBG("m_agentConnected: %d", int(m_agentConnected));
# endif
# ifdef OFFLINE_TEST
      TRAPD(err, m_settingInfo = CSettingInfo::NewL(NULL));
# endif
#else
# ifdef NETWORK_TEST
      m_networkrep = CRepository::NewL(KCRUidCoreApplicationUIs);
# endif
# ifdef OFFLINE_TEST
      m_profilerep = CRepository::NewL(KCRUidProfileEngine);
# endif
#endif   
   }
   
   OfflineTester::~OfflineTester()
   {
#ifndef SYMBIAN_9
# ifdef NETWORK_TEST
      if(m_agentConnected){
//          DBG("Agent will be closed.");
         m_agent.Close();
//          DBG("Agent was closed.");
         m_agentConnected = false;
      }
# endif
# ifdef OFFLINE_TEST
      delete m_settingInfo;
      m_settingInfo = NULL;
# endif
#else
# ifdef NETWORK_TEST 
      delete m_networkrep;
      m_networkrep = NULL;
# endif
# ifdef OFFLINE_TEST
      delete m_profilerep;
      m_profilerep = NULL;
# endif
#endif
   }
   
   bool OfflineTester::OffLineMode()
   {
#ifdef ALWAYS_NETWORK
      return false;
#endif
      bool offline = false;
#ifdef OFFLINE_TEST
      const TInt KOfflineProfile = 5;
# ifndef SYMBIAN_9
      if(m_settingInfo){
         TInt profile = -1;
         if(KErrNone == m_settingInfo->Get(SettingInfo::EActiveProfile, 
                                           profile)){
            offline = (profile == KOfflineProfile);
         } else {
//             ERR("KProEngActiveProfile: %d", int(profile));
         }
      }
# else
      if(m_profilerep){
         TInt profile = -1;
         if(KErrNone == m_profilerep->Get(KProEngActiveProfile, profile)){
//             ERR("KProEngActiveProfile: %d", int(profile));
            offline = (profile == KOfflineProfile);
         } else {
//             ERR("KProEngActiveProfile: %d", int(profile));
         }
      } else {
//          ERR("no profileref");
      }
# endif
#else
//       INFO("No Offline test performed in this build");
#endif
      return offline;
   }

   bool OfflineTester::NetworkAvailable()
   {
      bool networkavailable = true;
#ifdef ALWAYS_NETWORK
      return true;
#endif
#ifdef NETWORK_TEST 
# ifndef SYMBIAN_9
      if(m_agentConnected){
//          DBG("Agent is connected");
         TInt netState = m_agent.GetState(KUidNetworkStatus);
//          DBG("netstate: %d, ESANetworkAvailable: %d", 
//              netState, int(ESANetworkAvailable));
         networkavailable = ((netState < 0) ||
                             (netState == ESANetworkAvailable));
//          DBG("network %s available", networkavailable ? "is" : "not");
      }
# else
      if(m_networkrep){
         TInt netState = -1;
         if(KErrNone == m_networkrep->Get(KCoreAppUIsNetworkConnectionAllowed,
                                          netState)){
            networkavailable = 
               (netState == ECoreAppUIsNetworkConnectionAllowed);
         }
      }
# endif
#endif
      return networkavailable;
   }
} 
