/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "S60SymbianGeneralParamReceiver.h"
#include "SymbianSettingsData.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMessageHandler.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiParameterEnums.h"
#include "S60SymbianSettingsData.h"
#include "WayfinderSymbianUiBase.h"

void
S60SymbianGeneralParamReceiver::ConstructL()
{
   SymbianGeneralParamReceiver::ConstructL();
}

class S60SymbianSettingsData*
S60SymbianGeneralParamReceiver::Settings()
{
   return static_cast<S60SymbianSettingsData*>(m_settingsData);
}

class S60SymbianGeneralParamReceiver*
S60SymbianGeneralParamReceiver::NewL(
      CWayfinderDataHolder* aDataStore,
      WFStartupHandler* aStartupHandler,
      GuiProtMessageHandler* aSender,
      WayfinderSymbianUiBase* aAppUi)
{
   class S60SymbianGeneralParamReceiver* self =
      new (ELeave) S60SymbianGeneralParamReceiver(aDataStore,
         aStartupHandler, aSender, aAppUi);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

S60SymbianGeneralParamReceiver::~S60SymbianGeneralParamReceiver()
{
}

bool
S60SymbianGeneralParamReceiver::decodedParamNoValue(uint32 paramId)
{
   /* Parameter was unset. */
   /* Set default value if needed. */
   return SymbianGeneralParamReceiver::decodedParamNoValue(
         paramId);
}

bool
S60SymbianGeneralParamReceiver::decodedParamValue(uint32 paramId,
      const int32* data, int32 numEntries)
{
   bool handled = true;

   switch (paramId) {
      case isab::GuiProtEnums::paramLinkLayerKeepAlive:
         Settings()->m_linkLayerKeepAlive = data[0];
      break;
      default:
      /* Do not return or set handled to true, */
      /* since then we will not get any calls to */
      /* GeneralParamReceiver. */
      handled = false;
      break;
   }

   if (handled) {
      return handled;
   }
   return SymbianGeneralParamReceiver::decodedParamValue(
         paramId, data, numEntries);
}

