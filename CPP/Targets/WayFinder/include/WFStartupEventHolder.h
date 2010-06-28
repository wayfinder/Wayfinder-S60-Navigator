/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WF_STARTUP_EVENT_HOLDER_H
#define WF_STARTUP_EVENT_HOLDER_H

#include <arch.h>

class WFStartupEvent;

/**
 * Class that holds a WFStartupEvent and settings for it.
 */
class WFStartupEventHolder
{
public:
   /**
    * Constructor.
    *
    * @param event The WFStartupEvent this class should hold, is deleted
    *              in destructor.
    * @param deleteAfterStartUp If event can be removed after startup,
    *                           default false.
    */
   WFStartupEventHolder( WFStartupEvent* event, 
                         bool deleteAfterStartUp = false )
      :  m_event( event ), m_deleteAfterStartUp( deleteAfterStartUp ) {}

   /**
    * Destructor, deletes event.
    */
   ~WFStartupEventHolder();

   /**
    * Star operator that returns contained event.
    */
   WFStartupEvent& operator*() const {
      return *m_event;
   }

   /**
    * Operator that returns contained event.
    */
   WFStartupEvent* operator->() const {
      return m_event;
   }

   /**
    * Metof that returns contained event.
    */
   WFStartupEvent* get() const {
      return m_event;
   }

   /**
    * If alright to delete after startup.
    */
   bool deleteAfterStartUp() const {
      return m_deleteAfterStartUp;
   }

private:
   /// The event this class holds
   class WFStartupEvent* m_event;

   /**
    * If event can be removed after startup or if it is used after start.
    * Default is false.
    */
   bool m_deleteAfterStartUp;
};

#endif  /* WF_STARTUP_EVENT_HOLDER_H */
