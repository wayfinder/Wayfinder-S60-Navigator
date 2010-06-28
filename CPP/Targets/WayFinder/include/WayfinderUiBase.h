/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WAYFINDER_UI_BASE_H
#define WAYFINDER_UI_BASE_H

#include <arch.h>
#include "GuiProt/GeneralParamReceiver.h"

/*
 * Base class for UI's using the WFStartupEvents.
 * Any class that wants to use WFStartupEvents need to inherit this
 * class (directly or indirectly) and implement the below pure virtual methods.
 */
class WayfinderUiBase
{
public:
   /**
    * Constructor
    */
   WayfinderUiBase();
   /**
    * ConstructL  -  Will create all resources necessary for WayfinderUiBase.
    *                For example, it will create the m_startupHandler.
    */
   virtual void ConstructL();

   /** 
    * Destructor
    */
   virtual ~WayfinderUiBase();

   /**
    * BaseUiCallback - Called by the WFStartupEvents to perform a callback
    * to the UI which then can perform the correct (platform specific) action.
    * @param   eventType      The event that the UI shall perform.
    * @param   param          A callback parameter object, which can be
    *                         subtyped if needed to allow for more data.
    */
   virtual void BaseUiCallback(int32 eventType, class WFStartupEventCallbackParameter* param) = 0;

   /**
    * BaseUiTriggerEvent - Called by the WFStartupEvents to perform a new
    *                      call to the m_startupHandler but in a different
    *                      context. For Symbian, this is done using the
    *                      EventGenerator classes to give a new Active
    *                      Scheduler context.
    *                      This is also done to minimize stack use.
    *                      The call should ultimately result in a call to
    *                      m_startupHandler->StartupEvent(eventType);
    * @param   eventType   Event to be resent.
    * @param   param       Needed parameters.
    */
   virtual void BaseUiTriggerEvent(int32 eventType, 
                                   class WFStartupEventCallbackParameter* param,
                                   int32 eventDelay) = 0;

protected:

   /**
    * Pointer to config data (unused?)
    */
   class WFConfigurationData* m_configData;
   /**
    * Pointer to the startuphandler.
    */
   class WFStartupHandler* m_startupHandler;
   /**
    * Pointer to the handler of GuiProt parameters
    */
   class isab::GeneralParamReceiver* m_generalParamReceiver;
};

#endif  /* WAYFINDER_UI_BASE_H */
