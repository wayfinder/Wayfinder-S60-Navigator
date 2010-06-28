/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WAYFINDER_STARTUP_COMMON_BLOCKS_H
#define WAYFINDER_STARTUP_COMMON_BLOCKS_H

#include "arch.h"
#include <deque>

/**
 * WFStartupCommonBlocks
 *
 * This class encloses the specifics of the startup configuration.
 * Any user of the startup class may use any of these common code
 * blocks, if deemed necessary.
 */
class WFStartupCommonBlocks
{
public:
   /**
    * Add common block for IAP search
    */
   static void IAPSearch(class WFStartupHandler* startupHandler,
      int32 nextEvent, const char *languageIsoTwoChar,
      class WFServiceViewHandler* viewHandler,
      class GuiProtMessageSender* sender,
      class IAPDataStore* iapDataStore);
   
   /**
    * Add common block for send registration sms
    */
   static void SendRegistrationSms(WFStartupHandler* startupHandler,
                                   int32 nextEvent, 
                                   GuiProtMessageSender* guiProtHandler);
      
   /**
    * Add common block to include all the above blocks.
    */
   static void SetupStartup(class WFStartupHandler* startupHandler,
      class WFServiceViewHandler* viewHandler,
      class GuiProtMessageSender* guiProtHandler,
      class CWayfinderDataHolder*  dataHolder,
      class WFNewsData* newData,
      class WFAccountData* accountData,
      class IAPDataStore* iapDataStore,
      const char* languageIsoTwoChar,
      char* langSyntaxPath,
      bool useWFID, 
      bool useSilentStartup);
};

#endif
