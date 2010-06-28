/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <e32std.h>

#include <iostream>

#include "WFStartupHandler.h"
#include "WFStartupEventHolder.h"
#include "WFStartupEvent.h"
#include "WayfinderUiBase.h"
#include "TraceMacros.h"
#include "LogMacros.h"
#include "Log.h"

void startup_panic();

#define PANIC startup_panic

WFStartupHandler::WFStartupHandler(class WayfinderUiBase* baseUi)
   :  m_baseUi(baseUi), m_startupFinished(0), m_log(NULL)
{
}

void
WFStartupHandler::ConstructL()
{
}

WFStartupHandler::~WFStartupHandler()
{
   eventMap_t::iterator it = m_events.begin();
   while (it != m_events.end()) {
      delete (*it).second;
      it++;
   }
   m_events.clear();
}

void
WFStartupHandler::StartupEvent(int32 eventType, WFStartupEventCallbackParameter* param)
{
   eventMap_t::iterator findIt = m_events.find( eventType );
   if ( findIt != m_events.end() ) {
      (*(*findIt).second)->EventCallback( param );
   } else {
      /* Event not found! */
      // PANIC();
      if( m_log ){
         ERR( "WFStartupHandler::StartupEvent event %"PRId32" not found", 
              eventType );
      }
      TRACE_DBG( "WFStartupHandler::StartupEvent event %"PRId32
                 " not found", eventType );
   }
}

void
WFStartupHandler::TriggerEvent(int32 eventType, 
                               WFStartupEventCallbackParameter* param, 
                               int32 eventDelay)
{
   m_baseUi->BaseUiTriggerEvent(eventType, param, eventDelay);
}


void
WFStartupHandler::RegisterStartupEvent(
   int32 eventType,
   class WFStartupEventCallback* callback,
   bool deleteAfterStartUp )
{
   callback->SetStartupHandler(this);
   WFStartupEvent* event = new WFStartupEvent(callback, eventType);
   if (m_events.find(eventType) == m_events.end()) {
      m_events[eventType] = new WFStartupEventHolder( 
         event, deleteAfterStartUp );
   } else {
      /* Already registered! */
      if( m_log ){
         ERR( "WFStartupHandler::StartupEvent event %"PRId32
              " already registered!", eventType );
      }
      TRACE_DBG( "WFStartupHandler::StartupEvent event %"PRId32
                 " already registered!", eventType ); 
      PANIC();
   }
}

void
WFStartupHandler::DeRegisterStartupEvent(class WFStartupEventCallback* callback,
         int32 event)
{
   eventMap_t::iterator it = m_events.begin();
   if (event >= 0) {
      /* Find just that event and remove the callback for it. */
      it = m_events.find(event);
      if (it != m_events.end()) {
         WFStartupEventHolder* eventh = (*it).second;
         if ( (*eventh)->GetEventCallback() == callback ) {
            /* Correct match. */
            m_events.erase(it);
            delete eventh;
         } else {
            /* Same event id but different callback! */
            PANIC();
         }
      } else {
         /* No such event. */
      }
   } else {
      while (it != m_events.end()) {
         WFStartupEventHolder* eventh = (*it).second;
         if ( (*eventh)->GetEventCallback() == callback ) {
            /* Match, remove the event. */
            eventMap_t::iterator that = it;
            it++;
            m_events.erase(that);
            delete eventh;
         } else {
            /* No match, just step to the next one. */
            it++;
         }
      }
   }
}

void
WFStartupHandler::BaseUiCallback(int32 eventType,
   WFStartupEventCallbackParameter *param)
{
   m_baseUi->BaseUiCallback(eventType, param);
}

void
WFStartupHandler::StartupFinished()
{
   eventMap_t::iterator it = m_events.begin();
   while (it != m_events.end()) {
      if ( (*it).second->deleteAfterStartUp() ) {
         delete (*it).second;
         m_events.erase( it++ );
      } else {
         it++;
      }
   }
   /* Indicate that startup is done. */
   m_startupFinished = 1;
}


void 
WFStartupHandler::SetLog( isab::Log* logFile )
{
   m_log = logFile;
}
