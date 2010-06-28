/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "GuiProt/HelperEnums.h"
#include "BacklightManager.h"

TInt CBacklightManager::Tick(TAny* aPtr)
{
   class CBacklightManager& self = *static_cast<CBacklightManager*>(aPtr);
   switch(self.Strategy()){
   case isab::GuiProtEnums::backlight_on_during_route:
   case isab::GuiProtEnums::backlight_near_action: //XXX need a separate case.
      if(self.RouteActive()){
   case isab::GuiProtEnums::backlight_always_on:
         User::ResetInactivityTime();
      }
   case isab::GuiProtEnums::backlight_invalid:
   case isab::GuiProtEnums::backlight_always_off:
      break;
   }
   return 0;
};

void CBacklightManager::ConstructL()
{
   iTicker = CPeriodic::NewL(CActive::EPriorityStandard);
   iTicker->Start(EInterval, EInterval, 
                  TCallBack(CBacklightManager::Tick, this));
}

class CBacklightManager* 
CBacklightManager::NewL(class MRouteDataProvider* aProvider)
{
   class CBacklightManager* self = new (ELeave) CBacklightManager();
   CleanupStack::PushL(self);
   self->ConstructL();
   self->SetProvider(aProvider);
   CleanupStack::Pop(self);
   return self;
}

CBacklightManager::~CBacklightManager()
{
   delete iTicker;
}

void CBacklightManager::SetProvider(class MRouteDataProvider* aProvider)
{
   iProvider = aProvider;
}

enum isab::GuiProtEnums::BacklightStrategy CBacklightManager::Strategy() const
{
   return iStrategy;
}

enum isab::GuiProtEnums::BacklightStrategy 
CBacklightManager::SetStrategy(enum isab::GuiProtEnums::BacklightStrategy aStrategy)
{
   enum isab::GuiProtEnums::BacklightStrategy old = iStrategy;
   iStrategy = aStrategy;
   return old;
}

TBool CBacklightManager::RouteActive() const
{
   return iProvider->IsRouteActive();
}
