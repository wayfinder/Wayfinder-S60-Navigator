/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef S60_SYMBIAN_GENERAL_PARAM_RECEIVER_H
#define S60_SYMBIAN_GENERAL_PARAM_RECEIVER_H

#include "SymbianGeneralParamReceiver.h"

class S60SymbianGeneralParamReceiver
 : public CBase,
   public SymbianGeneralParamReceiver
{
   /**
    * Constructor
    *
    * @param aDataStore             The symbian data store object.
    * @param aStartupHandler        The startup handler.
    * @param aHandler               GuiProt message handler.
    */
   S60SymbianGeneralParamReceiver(
         class CWayfinderDataHolder* aDataStore,
         class WFStartupHandler* aStartupHandler,
         class GuiProtMessageHandler* aHandler,
         class WayfinderSymbianUiBase* aAppUi)
    : SymbianGeneralParamReceiver(aDataStore, aStartupHandler, aHandler, aAppUi)
   { }

   /**
    * ConstructL                    Symbian two-stage construction.
    *                               Will initialize everything that needs
    *                               allocations.
    */
   void ConstructL();

public:
   /**
    * NewL                          Static factory method.
    */
   static class S60SymbianGeneralParamReceiver* NewL(
         class CWayfinderDataHolder* aDataStore,
         class WFStartupHandler* aStartupHandler,
         class GuiProtMessageHandler* aHandler,
         class WayfinderSymbianUiBase* aAppUi);

   /**
    * Destructor.
    */
   virtual ~S60SymbianGeneralParamReceiver();

   /**
    * From SymbianGeneralParamReceiver.
    * decodedParamNoValue           Called when a general parameter message
    *                               has been received from Nav2 with no data.
    */
   virtual bool decodedParamNoValue(uint32 paramId);
   /**
    * From SymbianGeneralParamReceiver.
    * decodedParamValue             Called when a general parameter message
    *                               has been received from Nav2 with integer
    *                               data.
    */
   virtual bool decodedParamValue(uint32 paramId,
         const int32* data, int32 numEntries);

private:

   /**
    * Settings                      Returns the settings data as
    *                               S60SymbianSettingsData*
    */
   class S60SymbianSettingsData* Settings();

private:

};

#endif
