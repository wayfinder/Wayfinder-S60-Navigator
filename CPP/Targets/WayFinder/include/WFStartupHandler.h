/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WF_STARTUP_HANDLER_H
#define WF_STARTUP_HANDLER_H

#include "arch.h"
#include <map>

class WFStartupEventHolder;

namespace isab{
   class Log;
}

class WFStartupHandler
{
public:
   /**
    * Constructor
    *
    * @param baseUi        Pointer to a object that will handle
    *                      base ui events.
    */
   WFStartupHandler(class WayfinderUiBase* baseUi);

   /**
    * Second stage constructor.
    */
   virtual void ConstructL();

   /**
    * Destructor
    */
   virtual ~WFStartupHandler();

   /**
    * StartupEvent - Handle an event
    *
    * @param eventType     Event to handle
    * @param param         eventual parameter for this event.
    */
   virtual void StartupEvent(int32 eventType,
         class WFStartupEventCallbackParameter* param = NULL);

   /**
    * RegisterStartupEvent - Register an event object
    *
    * @param eventType     Event number to register
    * @param callback      Object to call back when event is received
    * @param deleteAfterStartUp  True if the object can be deleted at
    *                            end of startup.
    */
   virtual void RegisterStartupEvent(
         int32 eventType,
         class WFStartupEventCallback* callback,
         bool deleteAfterStartUp = false );

   /**
    * DeRegisterStartupEvent - Remove the specified object from the
    *                          startup handler lists.
    *
    * @param callback      Object to remove.
    * @param event         If set, the object will be removed only from
    *                      the startup lists for the specific event id.
    *                      If set to -1, all references to the object
    *                      is removed.
    */
   virtual void DeRegisterStartupEvent(class WFStartupEventCallback* callback,
         int32 event = -1);

   /**
    * BaseUiCallback - Perform a base callback for the event.
    *
    * @param eventType     Event to send to base ui.
    * @param param         Optional parameter to send to base ui.
    */
   virtual void BaseUiCallback(int32 eventType,
         class WFStartupEventCallbackParameter* param = NULL);

   /**
    * TriggerEvent - Let the base ui trigger the next event.
    *
    * This method is a bit complicated, but it's purpose is to allow
    * us to avoid very long call chains (where event A calls event B,
    * which calls event C ... etc) in Symbian.
    * 
    * In Symbian, the triggers are actually Active Objects, so the
    * base ui trigger method will register a new active object event,
    * and then return. When the complete call chain has returned back
    * to the system, the system will call the active object event and
    * that event will then call the startup handler to handle the event.
    *
    * For other systems with larger stackspace, it is quite possible
    * that the trigger will be handled just as a callback directly
    * to the startup handler.
    *
    * @param eventType     Event to trigger
    * @param param         Optional parameter to event.
    * @param eventDelay    Optional time to wait before triggering event.
    */
   virtual void TriggerEvent(int32 eventType, 
         class WFStartupEventCallbackParameter* param = NULL, 
         int32 eventDelay = -1);

   /**
    * Call this when the startup is complete and the events registred with
    * deleteAfterStartUp as true will be deleted.
    */
   virtual void StartupFinished();

   /** 
    * GetStartupFinished - Returns zero unless startup is finished.
    */
   int32 GetStartupFinished() { return m_startupFinished; }


   /**
    * Set the pointer to the log file.
    */
   void SetLog( isab::Log* logFile );

private:
   /**
    * Pointer to base ui object.
    */
   class WayfinderUiBase* m_baseUi;

   /**
    * Our list of events and object handlers.
    */
   typedef std::map<int32, class WFStartupEventHolder*> eventMap_t;
   eventMap_t m_events;

   /**
    * Indicates if startup has been completed.
    */
   int32 m_startupFinished;

   /// Pointer to the log.
   isab::Log* m_log;
};

#endif  /* WF_STARTUP_HANDLER_H */
