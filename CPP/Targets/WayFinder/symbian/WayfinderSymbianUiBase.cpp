/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "WayfinderSymbianUiBase.h"
#include "WFStartupHandler.h"
#include "WFTextUtil.h"

WayfinderSymbianUiBase::WayfinderSymbianUiBase()
      : WayfinderUiBase(), m_startup( true )
{
}

void
WayfinderSymbianUiBase::ConstructL()
{
   iSymbianBaseEventGenerator = CWFSymbianUiBaseEventGenerator::NewL(*this);
   WayfinderUiBase::ConstructL();
}

WayfinderSymbianUiBase::~WayfinderSymbianUiBase()
{
   delete iSymbianBaseEventGenerator;
   delete iComWiz;
}

bool
WayfinderSymbianUiBase::inStartup() const {
   return m_startup;
}

void
WayfinderSymbianUiBase::BaseUiCallback(int32 eventType,
      WFStartupEventCallbackParameter* param)
{
   switch (eventType) {
      case SS_IapSearchRestart:
         RestartIapSearch();
         break;
      case SS_IapSearch:
         StartIapSearch();
         break;
      case SS_StartupFinished:
         StartupFinished();
         break;
      case SS_GotoMainMenu:
         GotoMainMenu();
         break;
      case SS_Exit:
         ShutdownNow();
         break;
      case SS_ClearBrowserCache:
         ClearBrowserCache();
         break;
      case SS_StartupFailed:
         ReportStartupFailed(param);
         break;
      case SS_StartupErrorReal:
         ReportStartupError(param);
         break;
      case SS_CheckFlightMode:
         CheckFlightMode();
         break;
      case SS_UpgradeOk:
         if (m_startupHandler->GetStartupFinished()) {
            m_startupHandler->TriggerEvent(SS_UpgradeOkNotStartup);
         } else {
            m_startupHandler->TriggerEvent(SS_UpgradeOkStartup);
         }
         break;
      case SS_UpgradeFailed:
         if (param) {
            ShowUpgradeErrorL(param);
         }
         break;
      case SS_ActivateSplashView:
         ActivateSplashViewL();
         m_startupHandler->StartupEvent(SS_SplashViewActivated);
         break;
      case SS_ShowRegistrationSmsStartup:
         ShowRegistrationSmsDialog();
         break;
      default:
         WayfinderUiBase::BaseUiCallback(eventType, param);
         break;
   }
}

void
WayfinderSymbianUiBase::StartupFinished()
{
   m_startup = false;
   m_startupHandler->StartupFinished();
   m_startupHandler->StartupEvent(SS_GotoMainMenu);
}

void
WayfinderSymbianUiBase::BaseUiTriggerEvent(int32 eventType, 
                                           class WFStartupEventCallbackParameter* param, 
                                           int32 eventDelay)
{
   m_triggerEventParams.push_back(param);
   GenerateEvent((enum StartupState)eventType, eventDelay);
}

void
WayfinderSymbianUiBase::GenerateEvent(enum StartupState aEvent, int32 eventDelay)
{
   if (iSymbianBaseEventGenerator) {
      if (eventDelay > 0) {
         iSymbianBaseEventGenerator->SendEventL(aEvent, 
                                                TTimeIntervalMicroSeconds32(eventDelay));
      } else {
         iSymbianBaseEventGenerator->SendEventL(aEvent);
      }
   }
}

void
WayfinderSymbianUiBase::HandleGeneratedEventL(enum StartupState aEvent)
{
   if (m_startupHandler) {
      class WFStartupEventCallbackParameter* param = NULL;
      if (!m_triggerEventParams.empty()) {
         param = m_triggerEventParams.front();
         m_triggerEventParams.pop_front();
      }
      m_startupHandler->StartupEvent(int32(aEvent), param);
   }
}

void
WayfinderSymbianUiBase::GotoServiceViewUrl(const char* aUrl,
      enum BackActionEnum aAllowBack)
{
   HBufC* url = WFTextUtil::AllocLC(aUrl);
   GotoServiceViewL(*url, aAllowBack);
   CleanupStack::PopAndDestroy(url);
}

void 
WayfinderSymbianUiBase::SendFavoriteL(isab::Favorite* aFavorite)
{
   if (iComWiz) {
      iComWiz->StartWizardL(EWfFavorite, aFavorite);
   }
}

void
WayfinderSymbianUiBase::SendMapPositionL()
{
   if (iComWiz) {
      iComWiz->StartWizardL(EWfMapPosition);
   }
}
