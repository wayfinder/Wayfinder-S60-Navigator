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
#include "S60StartupPeer.h"
#include "S60StartupHandler.h"

S60StartupHandler::S60StartupHandler(
      uint32 aStartupControl,
      class MS60StartupPeer *aPeer)
   :
   iWelcomeFlags(ENoneReceived),
   iStartupMode(aStartupControl),
   iPeer(aPeer),
   iShowLegalNotice(ETrue),
   iWayfinderType(isab::GuiProtEnums::InvalidWayfinderType),
   iReadyToUseGps(EFalse)
{

}

S60StartupHandler::~S60StartupHandler()
{
}

void
S60StartupHandler::TestAllReceived()
{
   if (! (iWelcomeFlags & EStartupCompleted)) {
      if (CanGotoSplashView()) {
         iPeer->GotoSplashView();
         iWelcomeFlags |= EStartupCompleted;
      } else if (CanGotoTrialView()) {
         iPeer->GotoTrialView();
         iWelcomeFlags |= EStartupCompleted;
      }
   }
}


void
S60StartupHandler::HandleInifile()
{
   iWelcomeFlags |= EIniFileReceivedFlag;
   TestAllReceived();
}

void
S60StartupHandler::HandleUserTerms()
{
   iWelcomeFlags |= EUserTermsReceivedFlag;
   TestAllReceived();
}

void
S60StartupHandler::HandleUsaSafetyPopup()
{
   iWelcomeFlags |= EUsaSafetyPopupAccepted;
   TestAllReceived();
}

void
S60StartupHandler::HandleWfType(isab::GuiProtEnums::WayfinderType wtf)
{
   iWayfinderType = wtf;
   iWelcomeFlags |= EWayfinderTypeReceivedFlag;
   TestAllReceived();
}

void
S60StartupHandler::HandleIAPId(int32 /*aIAP*/)
{
}

void
S60StartupHandler::HandleTrialCommand()
{

}

void
S60StartupHandler::HandleSplashOK()
{

}


TBool
S60StartupHandler::CanGotoSplashView()
{
   if ((iWelcomeFlags == EAllReceived
            || iWelcomeFlags & EStartupCompleted)
         && (iWayfinderType != isab::GuiProtEnums::Trial
            || (iStartupMode & ENoTrial))
         ) {
      return ETrue;
   }
   return EFalse;
}

TBool
S60StartupHandler::CanGotoTrialView()
{
   if ((iWelcomeFlags == EAllReceived
            || iWelcomeFlags & EStartupCompleted)
         && iWayfinderType == isab::GuiProtEnums::Trial
         && !(iStartupMode & ENoTrial)) {
      return ETrue;
   }
   return EFalse;
}
