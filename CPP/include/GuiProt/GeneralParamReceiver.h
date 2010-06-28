/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GENERAL_PARAM_RECEIVER_H
#define GENERAL_PARAM_RECEIVER_H

#include "GuiProt/GuiProtMessageSender.h"

class GenericSettingsData;
class GuiProtMessageHandler;

namespace isab {

/**
 */
class GeneralParamReceiver
 : public GuiProtMessageReceiver
{
public:

   /**
    * Constructor          -  Initialize object.
    *
    * @param settingsData     The settings data object.
    */
   GeneralParamReceiver(
      class GenericSettingsData* settingsData,
      class GuiProtMessageHandler* handler) :
      m_settingsData(settingsData),
      m_guiProtHandler(handler)
   {}

   /**
    * Init                 -  Register as receiver for general parameters.
    */
   void init();

   /**
    * Destructor
    */
   virtual ~GeneralParamReceiver();

   /**
    * decodedParamNoValue           Called when a general parameter message
    *                               has been received from Nav2 with no data.
    */
   virtual bool decodedParamNoValue(uint32 paramId);
   /**
    * decodedParamValue             Called when a general parameter message
    *                               has been received from Nav2 with binary
    *                               data.
    */
   virtual bool decodedParamValue(uint32 paramId,
         const uint8* data, int32 numEntries);
   /**
    * decodedParamValue             Called when a general parameter message
    *                               has been received from Nav2 with float
    *                               data.
    */
   virtual bool decodedParamValue(uint32 paramId,
         const float* data, int32 numEntries);
   /**
    * decodedParamValue             Called when a general parameter message
    *                               has been received from Nav2 with string
    *                               data.
    */
   virtual bool decodedParamValue(uint32 paramId,
         const char** data, int32 numEntries);
   /**
    * decodedParamValue             Called when a general parameter message
    *                               has been received from Nav2 with integer        *                               data.
    */
   virtual bool decodedParamValue(uint32 paramId,
         const int32* data, int32 numEntries);


   /**
    * From GuiProtMessageReceiver
    *                      -  Called when a message is received from Nav2.
    *
    */
   virtual bool GuiProtReceiveMessage(class GuiProtMess *mess);

public:
   /* Interface */

   /**
    * sendIAPId            -  Send the IAP id.
    * Must be implemented by platform specific code.
    */
   virtual void sendIAPId() = 0;

      
   /**
    * sendServerString - Takes a server string delimited with ";" and
    *                    converts to the server param format.
    *                    WARNING, will destroy the serverString parameter
    *
    * Must be implemented by platform specific code.
    */
   virtual void sendServerString(int32 paramId, char* serverString) = 0;


public:
   class GenericSettingsData *m_settingsData;
   class GuiProtMessageHandler*  m_guiProtHandler;
};

}

#endif

