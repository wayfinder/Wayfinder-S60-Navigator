/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WF_STARTUP_EVENT_H
#define WF_STARTUP_EVENT_H

#include <arch.h>
#include "WFStartupEventEnum.h"

/**
 * WFStartupEventCallback
 * Base class used for callback events
 */
class WFStartupEventCallback
{
public:
   /**
    * EventCallback
    * Called when the event is triggered
    *
    * @param eventType     Type of event
    * @param param         Callback parameter data
    */
   virtual void EventCallback(int32 eventType,
         class WFStartupEventCallbackParameter* param) = 0;

   /**
    * SetStartupHandler
    *
    * Sets the startup handler pointer.
    * @param handler       New startup handler
    */
   virtual void SetStartupHandler(class WFStartupHandler* handler)
   { m_handler = handler; }

   /**
    * Destructor
    */
   virtual ~WFStartupEventCallback();
   /**
    * Pointer to startup handler
    */
   class WFStartupHandler* m_handler;
};

/**
 * WFStartupEvent
 * Base class used for startup events.
 */
class WFStartupEvent
{
public:
   /**
    * Constructor
    * @param callback      Callback object pointer
    * @param eventType     Event type
    */
   WFStartupEvent(WFStartupEventCallback* callback, int32 eventType)
      :  m_callback(callback), m_eventType(eventType) {}

   /**
    * EventCallback
    * Called when a callback should be performed.
    * @param param         WFStartupEventCallbackParameter data
    */
   void EventCallback(class WFStartupEventCallbackParameter* param)
   { m_callback->EventCallback(m_eventType, param); }

   /**
    * Compare operator for WFStartupEvents
    */
   bool operator==(const class WFStartupEvent& event) const
   {
      return m_eventType == event.m_eventType;
   }

   /**
    * GetEventCallback
    * @return the callback object
    */
   class WFStartupEventCallback* GetEventCallback() { return m_callback; }

private:
   class WFStartupEventCallback* m_callback;
   int32 m_eventType;
};

#endif  /* WF_STARTUP_EVENT_H */
