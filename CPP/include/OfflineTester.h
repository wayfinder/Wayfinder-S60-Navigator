/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef OFFLINE_TESTER_H
#define OFFLINE_TESTER_H

#if defined __SYMBIAN32__
# ifndef SYMBIAN_9
#  include <saclient.h>
#  include <sacls.h>
# else
#  include <centralrepository.h>
# endif
# define NETWORK_TEST
#endif

#if defined NAV2_CLIENT_SERIES60_V2 || defined SYMBIAN_9
# define OFFLINE_TEST
#endif

#ifdef NAV2_CLIENT_UIQ3
# undef OFFLINE_TEST
# undef NETWORK_TEST
#endif

class CSettingInfo;

namespace isab{

   class OfflineTester
      {
      public:
         
         OfflineTester();
         
         virtual ~OfflineTester();
         /**
          * Tests whether network is available. Platforms that have no
          * support for this test always indicat that network is
          * available.
          * @return True if network is available, false otherwise.
          */
         bool NetworkAvailable();
         /**
          * Tests whether the device is in OffLine mode or not.  Devices
          * that have no offline mode or do not support the test always
          * indicate that offline mode is _NOT_ active.
          * Offline is sometimes called flight mode. 
          * @return True if device is in offline mode, false otherwise.
          */
         bool OffLineMode();
         
      private:
         
#ifndef SYMBIAN_9
# ifdef NETWORK_TEST
         /// Used by Symbian to see if network is available.
         class RSystemAgent m_agent;
         /// Used to keep track of whether the RSystemAgent is available. 
         bool m_agentConnected;
# endif
# ifdef OFFLINE_TEST
         class CSettingInfo* m_settingInfo;
# endif
#else
# if defined NETWORK_TEST 
         class CRepository* m_networkrep;
# endif
# if defined OFFLINE_TEST
         class CRepository* m_profilerep;
# endif
#endif
         
      };
}

#endif
