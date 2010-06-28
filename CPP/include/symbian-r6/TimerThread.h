/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TIMER_THREAD_H
#define TIMER_THREAD_H

#include <e32std.h>
/* #undef NO_LOG_OUTPUT */
#include "ModuleQueue.h"
#include "Log.h"
#include "GlobalData.h"
#ifdef TRACE_TIMER_THREAD
/* #include "TraceMacros.h" */
#endif

/* #define DEBUG_TIMERS */

/* #define FAKE_REQUEST */


namespace isab {
/**
 * Symbian class for timing.
 */
class TimerThread {

   // Wait on timer (and wakeup socket in future)

   /// Max Time to wait before setting new timeout
#define maxWaitTimeMillis 200

   /**
    * Holds an timeout request from a ModuleQueue.
    */
   class timeOutItem {
   public:
      /**
       * Constructor.
       *
       * @param queue The ModuleQueue to insert the timerBuf into when
       *              timeout.
       * @param millis The timeout in milli seconds.
       * @param timerBuf The MsgBuffer to insert into the ModuleQueue at
       *                 timeout.
       * @param id An user defined ID.
       */
      timeOutItem(class ModuleQueue* queue, uint16 id, uint32 millis,
                  class MsgBuffer* timerBuf )
            : m_queue( queue ), m_id( id ), m_millis( millis),
              m_timerBuf( timerBuf ), m_next( NULL ), m_timeOutItemID( 0 )
         {}

      ///Pointer to the requesting ModuleQueue.
      class ModuleQueue* m_queue;
      ///ModuleQueue timer id.
      uint16 m_id;
      ///Millisecond timeout
      uint32 m_millis;
      ///Buffer that shall be inserted into m_queue when the timer expires. 
      class MsgBuffer* m_timerBuf;
      ///Pointer to the next timeOutItem in the single linked list.
      class timeOutItem* m_next;
      ///TimerThread tracking id.
      uint32 m_timeOutItemID;

      ///Appends this timeOutItem last in the list that starts with
      ///the timeOutItem pointed to by *aList.
      ///@param list pointer to the pointer pointing to the first
      ///            element in the list. <code>aList</code> may not be
      ///            NULL, but <code>*aList</code> may be NULL, which
      ///            signifies that the list is empty.
      void insert(class timeOutItem** aList )
      {
         if ( *aList == NULL ) {
            // Empty list
            *aList = this;
            m_next = NULL;
         } else {
            // Not empty
            class timeOutItem* cur = *aList;
            while ( cur->m_next != NULL ) {
               cur = cur->m_next;
            }
            cur->m_next = this;
            m_next = NULL;
         }
      }

      ///Finds this timeOutItem object in the list pointed to by
      ///<code>*aList</code>.
      ///@param aList pointer to pointer to the first timeOutItem
      ///             object in the singly linked list. Neither
      ///             <code>aList</code> nor <code>*aList</code> may
      ///             be NULL.
      void remove(class timeOutItem** aList ) {
         if ( *aList == this ) {
            // First in list
            *aList = m_next;
            m_next = NULL;
         } else {
            // Remove from list
            class timeOutItem* cur = *aList;
            class timeOutItem* prev = cur;
            while((cur != this) && (cur != NULL)){
               prev = cur;
               cur = cur->m_next;
            }
            if(cur != NULL){
               prev->m_next = m_next;
               m_next = NULL;
            } else {
               //not found
            }
         }
      }
   };


   /**
    * Holds an timer request that is sent to system.
    */
   class timerItem {
   public:
      ///Constructor
      ///@param aQueue the ModuleQueue that set this timer.
      ///@param id the ModuleQueue timer id.
      ///@param timeOutItemID the id of the matching timeOutTimer.
      timerItem(class ModuleQueue* aQueue, uint16 id, uint32 timeOutItemID )
            : m_queue( aQueue ), m_id( id ), m_timeOutItemID( timeOutItemID )
         {}

      ///Destructor.
      ~timerItem()
      {
         m_timer.Close();
      }

      ///Symbian system timer handle.
      class RTimer m_timer;
      ///Status variabler for the timer request.
      class TRequestStatus status;
      ///The ModuleQueue associated with this timer.
      class ModuleQueue* m_queue;
      ///The ModuleQueue timer id.
      uint16 m_id;
      ///The next timeItem in the singly linked list.
      class timerItem* m_next;
      ///The id for the matching timeOutItem object.
      uint32 m_timeOutItemID;

      /**
       * Inserts this into list.
       */
      void insert(class timerItem** list ) {
         if ( *list == NULL ) {
            // Empty list
            *list = this;
            m_next = NULL;
         } else {
            // Not empty
            class timerItem* cur = *list;
            while ( cur->m_next != NULL ) {
               cur = cur->m_next;
            }
            cur->m_next = this;
            m_next = NULL;
         }
      }

      /**
       * Removes from list.
       */
      void
      remove(class timerItem** list ) {
         if ( *list == this ) {
            // First in list
            *list = m_next;
            m_next = NULL;
         } else {
            // Remove from list
            class timerItem* cur = *list;
            class timerItem* prev = cur;
            while ( cur != this && cur != NULL) {
               prev = cur;
               cur = cur->m_next;
            }
            if(cur != NULL){
               prev->m_next = m_next;
               m_next = NULL;
            } else {
               //not found
            }
         }
      }

   };

public:
#ifdef DEBUG_TIMERS
   class LogMaster iLogMaster;
   class Log *log;
#endif
   /**
    * Constructor.
    */
   TimerThread();

   ~TimerThread();

   /**
    * Mutex method to add a timeout.
    *
    * @param queue The ModuleQueue to insert the timerBuf into when
    *              timeout.
    * @param id An unique id for queue's timer used in unSetTimer.
    * @param millis The timeout in milli seconds.
    * @param timerBuf The MsgBuffer to insert into the ModuleQueue at
    *                 timeout.
    */
   void setTimer(class ModuleQueue* queue, uint16 id, uint32 millis,
            class MsgBuffer* timerBuf );

   /**
    * Mutex method to unset the timer for ModuleQueue queue.
    *
    * @param queue The ModuleQueue to unset timer for.
    * @param id An unique id for queue's timer.
    */
   void unSetTimer(class ModuleQueue* queue, uint16 id );

   void terminate();

   int join();

   int terminateAndJoin();

   /**
    * The run method.
    */
   void run();

private:
   /**
    * The starting method.
    */
   static TInt startF( TAny *aPtr );

   volatile bool m_terminated;
   volatile int m_goneFishing;

#ifdef FAKE_REQUEST
   TRequestStatus m_fakeStatus;
#endif

   /**
    * The timeOutItems in a single linked list.
    */
   class timeOutItem* m_timeOuts;

   /**
    * The actual thread.
    */
   class RThread m_thread;
   class TRequestStatus m_terminateStatus;
   /**
    * The mutex protecting m_timeOuts.
    */
   class Mutex m_mutex;

   /**
    * The timeOutItem id.
    */
   uint32 m_timeOutItemID;
};



} // end namespace isab

#endif /* TIMER_THREAD_H */
