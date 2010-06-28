/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef S60_STARTUP_HANDLER
#define S60_STARTUP_HANDLER
#include <e32base.h>
#include "GuiProt/ServerEnums.h"

class S60StartupHandler : public CBase
{
public:
   S60StartupHandler(uint32 aStartupControl, class MS60StartupPeer *aPeer);
   ~S60StartupHandler();

   void HandleInifile();
   void HandleUserTerms();
   void HandleUsaSafetyPopup();
   void HandleWfType(isab::GuiProtEnums::WayfinderType wtf);
   void HandleIAPId(int32 aIAP = -1);
   void HandleTrialCommand();
   void HandleSplashOK();
   TBool CanGotoSplashView();
   TBool CanGotoTrialView();
   void TestAllReceived();

   enum TStartupControl {
      ENoTrial             = 0x01,
   };

/*    enum TStartupState { */
/*       ENotStarted          = 0x01, */
/*       EStarted, */
/*       EWaitForData, */
/*       ETrialView, */
/*       ESplashView, */
/*       ETrialIAPSearch, */
/*       ESplashIAPSearch, */
/*       ESplashAudioDownload, */
/*       EStartupComplete, */
/*       EStartupExitting, */
/*    }; */

private:

   enum TWelcomeFlags {
      ENoneReceived              = 0x00,
      EWayfinderTypeReceivedFlag = 0x01,
      EIniFileReceivedFlag       = 0x02,
      EUserTermsReceivedFlag     = 0x04,
      EUsaSafetyPopupAccepted    = 0x08,

      EStartupCompleted          = 0x10,

      EAllReceived = EWayfinderTypeReceivedFlag |
                     EIniFileReceivedFlag |
                     EUserTermsReceivedFlag |
                     EUsaSafetyPopupAccepted,
   };

   uint32 iWelcomeFlags;
   uint32 iStartupMode;
   class MS60StartupPeer* iPeer;

   uint32 iShowLegalNotice;

   isab::GuiProtEnums::WayfinderType iWayfinderType;
public:
   TBool iReadyToUseGps;

};

#endif /* S60_STARTUP_HANDLER */
