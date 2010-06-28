/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYMBIAN_GENERAL_PARAM_RECEIVER_H
#define SYMBIAN_GENERAL_PARAM_RECEIVER_H

#include "GuiProt/GeneralParamReceiver.h"

namespace isab {
   class GeneralParameterMess;
}

class SymbianGeneralParamReceiver
 : public isab::GeneralParamReceiver
{
public:
   /**
    * Constructor
    *
    * @param aDataStore       Symbian data store
    * @param aStartupHandler  Startup handler
    * @param aHandler         GuiProt message handler
    */
   SymbianGeneralParamReceiver(
         class CWayfinderDataHolder* aDataStore,
         class WFStartupHandler* aStartupHandler,
         class GuiProtMessageHandler* aHandler,
         class WayfinderSymbianUiBase* aAppUi);
   /**
    * Initialize stuff that need allocations.
    */
   virtual void ConstructL();
   /**
    * From SymbianGeneralParamReceiver.
    * decodedParamNoValue           Called when a general parameter message
    *                               has been received from Nav2 with no data.
    */                                                         
   virtual bool decodedParamNoValue(uint32 paramId);
   /**
    * From GeneralParamReceiver.
    * decodedParamValue             Called when a general parameter message
    *                               has been received from Nav2 with binary
    *                               data.
    */
   virtual bool decodedParamValue(uint32 paramId,
         const uint8* data, int32 numEntries);
   /**
    * From GeneralParamReceiver.
    * decodedParamValue             Called when a general parameter message
    *                               has been received from Nav2 with string
    *                               data.
    */
   virtual bool decodedParamValue(uint32 paramId,
         const char** data, int32 numEntries);
   /**
    * From GeneralParamReceiver.
    * decodedParamValue             Called when a general parameter message
    *                               has been received from Nav2 with integer
    *                               data.
    */
   virtual bool decodedParamValue(uint32 paramId,
         const int32* data, int32 numEntries);

   /**
    * sendIAPId               Send IAP id parameter to Nav2.
    */
   virtual void sendIAPId();

   /**
    * sendServerString - Takes a server string delimited with ";" and
    *                    converts to the server param format.
    *                    WARNING, will destroy the serverString parameter
    */
   virtual void sendServerString(int32 paramId, char* serverString);
   
   /**
    * Destructor
    */
   virtual ~SymbianGeneralParamReceiver();

private:
   /**
    * handleLatestShownNewsChecksum    Handle shown news checksum
    */
   void handleLatestShownNewsChecksum(const char* checksum);
   /**
    * handleLatestNewsChecksum         Handle news checksum
    */
   void handleLatestNewsChecksum(const char* checksum);
   /**
    * checkLatestNewsChecksum          Called by both methods above to test
    *                                  if we should show news.
    */
   void checkLatestNewsChecksum();
   /**
    * setWayfinderType                 Sets the wayfinder type.
    */
   void setWayfinderType(int32 wft);
   /**
    * handleWayfinderType              Sets the wayfinder type and triggers
    *                                  a startup event.
    */
   void handleWayfinderType(int32 wft);

   /**
    * convertToServerString            Converts the server array to a single
    *                                  string, divided using ;
    */
   HBufC* convertToServerString(const char** data, int32 numEntries);

   /**
    * Settings                         Returns the settings data as type
    *                                  SymbianSettingsData.
    */
   class SymbianSettingsData* Settings();
protected:
   /**
    * Symbian data store.
    */
   class CWayfinderDataHolder*   m_dataStore;
   /**
    * Startup handler pointer.
    */
   class WFStartupHandler*       m_startupHandler;
   /**
    * AppUI interface pointer.
    */
   class WayfinderSymbianUiBase* m_appUi;
};

#endif

