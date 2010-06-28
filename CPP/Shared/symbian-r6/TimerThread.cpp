/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/


#include "Log.h"

#include "Buffer.h"
#include "MsgBuffer.h"
#include "TimerThread.h"

namespace isab {

TimerThread::TimerThread()
{
   m_mutex.lock(); // Just in case
#ifdef DEBUG_TIMERS
   char fileName[256];
   strcpy(fileName, "file:e:\\");
   /*       strcat(fileName, getGlobalData().m_commondata_base_path); */
   strcat(fileName, "ttlog.txt");

   iLogMaster.setDefaultOutput(fileName);
   log = new Log("timerthread", Log::LOG_ALL, &iLogMaster);
#endif
   m_timeOuts = NULL;
   m_timeOutItemID = 1;
#define TIMERTHREAD "TimerThread"

   /* Note: TInt64 would give at most 20 characters. */
   HBufC* threadName = HBufC::NewL(30+strlen(TIMERTHREAD));

   threadName->Des().Zero();
   char *temp = TIMERTHREAD;
   int length = strlen(temp);
   int i = 0;

   //Crashes if we try to write more than maxlength in the buffer
   if( length >= threadName->Des().MaxLength() ) {
      length = threadName->Des().MaxLength()-1;
   }
   while (i < length) {
      threadName->Des().Append(uint8(temp[i]));
      i++;
   }
   threadName->Des().PtrZ();

   TTime now;
   now.HomeTime();
   threadName->Des().AppendNum(now.Int64());

   TInt res = m_thread.Create(
         *threadName,
         startF,
         8*1024, // Stack Size
         NULL,
         this // Data pointer to send to startF
         );

   delete threadName;
   threadName = NULL;

   if ( res != KErrNone ) {
      // XXX: PANIC!
      User::Panic(_L("Timer failure"), 1);
   }
   m_mutex.unlock();
   // Schedule thread for run, does not fail
#ifdef FAKE_REQUEST
   m_fakeStatus = KRequestPending;
#endif
   m_thread.Logon(m_terminateStatus);
   if(m_terminateStatus != KRequestPending){
      ///XXX PANIC!!
      User::Panic(_L("Timer logon"), 1);
   }
   m_thread.Resume();
}

TimerThread::~TimerThread()
{
#ifdef DEBUG_TIMERS
   delete log;
#endif
}

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
void
TimerThread::setTimer(class ModuleQueue* queue, uint16 id, uint32 millis,
      class MsgBuffer* timerBuf )
{
   class timeOutItem* item = new timeOutItem( queue, id, millis, timerBuf );
   m_mutex.lock();
   item->m_timeOutItemID = m_timeOutItemID++;
#ifdef DEBUG_TIMERS
   log->debug ( "TT:setTimer %p, %i for %i buf %p", queue, id, millis, timerBuf );
#endif
   item->insert( &m_timeOuts );
   // XXX: Add restart here
   m_mutex.unlock();
}

/**
 * Mutex method to unset the timer for ModuleQueue queue.
 *
 * @param queue The ModuleQueue to unset timer for.
 * @param id An unique id for queue's timer.
 */
void
TimerThread::unSetTimer(class ModuleQueue* queue, uint16 id )
{
   m_mutex.lock();
   // Find matching timeOutItem
   class timeOutItem* cur = m_timeOuts;
   while ( cur != NULL && !(cur->m_queue == queue && cur->m_id == id) ) {
      cur = cur->m_next;
   }
   if ( cur != NULL ) { // Found
#ifdef DEBUG_TIMERS
      log->debug( "TT::unsetTimer %p, %i for %i buf %p", cur->m_queue, cur->m_id, cur->m_millis, cur->m_timerBuf );
#endif
      cur->remove( &m_timeOuts );
      // Delete the unreturned buffer
      delete cur->m_timerBuf;
      delete cur;
   }
   // XXX: Add restart here
   m_mutex.unlock();
}

void
TimerThread::terminate()
{
#ifdef DEBUG_TIMERS
      log->debug( "TT::setting terminate");
#endif
   m_terminated = true;
}

int
TimerThread::join()
{
#ifdef DEBUG_TIMERS
      log->debug( "TT::join called");
#endif
   TRequestStatus timerStatus;
   class RTimer timer;
   timer.CreateLocal();
   TInt counter = 0;

#ifdef TRACE_TIMER_THREAD
   TRACE_FUNC();
#endif

   if (m_terminateStatus == KRequestPending) {
#ifdef TRACE_TIMER_THREAD
   TRACE_FUNC();
#endif
   }

   while (1) {
#ifdef DEBUG_TIMERS
      log->debug( "TT::pre wait ");
#endif

#ifdef TRACE_TIMER_THREAD
   TRACE_FUNC();
#endif
      timer.After(timerStatus, 2*1000*1000);
      User::WaitForRequest(m_terminateStatus, timerStatus);

      if (timerStatus == KRequestPending) {
#ifdef TRACE_TIMER_THREAD
   TRACE_FUNC();
#endif
         /* Ok logon. */
         timer.Cancel();
         User::WaitForRequest(timerStatus);
         timer.Close();
#ifdef TRACE_TIMER_THREAD
   TRACE_FUNC();
#endif
         return 0;
      } else {
         if (counter < 6 && !m_goneFishing) {
            counter++;
         } else {
            /* Timeout. */
#ifdef TRACE_TIMER_THREAD
   TRACE_FUNC();
#endif
            m_thread.LogonCancel(m_terminateStatus);
            User::WaitForRequest(m_terminateStatus);
            timer.Close();
#ifdef TRACE_TIMER_THREAD
   TRACE_FUNC();
#endif
            return -1;
         }
      }
   }
#ifdef DEBUG_TIMERS
      log->debug( "TT::post wait ");
#endif
   return 2;
}

int
TimerThread::terminateAndJoin()
{
   terminate();
#ifdef FAKE_REQUEST
#ifdef DEBUG_TIMERS
   log->debug( "TT::faking request ");
#endif
   TRequestStatus* aReq = &m_fakeStatus;
   m_thread.RequestComplete(aReq, 1);
#endif
   if (m_goneFishing == 1) {
#ifdef DEBUG_TIMERS
      log->debug( "TT::already fishing ");
#endif
      m_thread.LogonCancel(m_terminateStatus);
      User::WaitForRequest(m_terminateStatus);
      return 1;
   } else {
      return join();
   }
};

/**
 * The run method.
 */
void
TimerThread::run()
{
   // The timers
   class timerItem* timers = NULL;
   m_terminated = false;
   m_goneFishing = 0;

   // Add maxWaitTimeMillis timer
   class timerItem* tItem = new timerItem( NULL, 0, 0 );
   if ( tItem->m_timer.CreateLocal() != KErrNone ) {
      // PANIC!
   }
   class TTimeIntervalMicroSeconds32 interval( maxWaitTimeMillis*1000 );
   tItem->m_timer.After( tItem->status, interval );
   //m_mutex.lock();
   tItem->insert( &timers );
   //m_mutex.unlock();
#ifdef DEBUG_TIMERS
   log->debug( "TT::run pre while ");
#endif

   while ( !m_terminated ) {
      m_mutex.lock();
      // Check all timeoutitems so they have a timer
      class timeOutItem* cur = m_timeOuts;
      while (!m_terminated && cur != NULL ) {
         // Try to find queue,id in timers
         bool hasTimer = false;
         class timerItem* cTimer = timers;
         while (!m_terminated && cTimer != NULL ) {
            if ( cTimer->m_queue == cur->m_queue &&
                  cTimer->m_id == cur->m_id )
            {
               hasTimer = true;
               break;
            }
            cTimer = cTimer->m_next;
         }
         if ( !hasTimer && !m_terminated) {
            // Add timerItem
            class  timerItem* tItem =
               new timerItem(cur->m_queue, cur->m_id, cur->m_timeOutItemID);
            if ( !tItem || tItem->m_timer.CreateLocal() != KErrNone ) {
               // PANIC!
            }
            class TTimeIntervalMicroSeconds32 interval = cur->m_millis*1000;
            if(interval.Int() < 0) {
               //Safety limit for Symbian since it can only handle signed int intervals.
               //It will panic if we try to use a negative value so we set it to max.
               interval = 0x7FFFFFFF;
            }
            tItem->m_timer.After( tItem->status, interval );
            tItem->insert( &timers );
#ifdef DEBUG_TIMERS
            log->debug("TT adding timeItem, id %d, %p (buf=%p), "
                  "Timeout %dms (%d)", tItem->m_id,
                  tItem->m_queue, cur->m_timerBuf,
                  cur->m_millis, interval.Int() );
#endif
         }

         cur = cur->m_next;
      }
      m_mutex.unlock();

      if(m_terminated){
         break;
      }
      // Wait any timer
      User::WaitForAnyRequest();

      if(m_terminated){
         break;
      }

      // Check if timeout for any requeststatus and
      // find status and ModuleQueue
      class timerItem* cTimer = timers;
      class timerItem* deleteTimer = NULL;
      class ModuleQueue* queue = NULL;
      uint16 id = 0;
      uint32 timeOutItemID = 0;

      while ( !m_terminated && cTimer != NULL ) {
         if ( cTimer->status != KRequestPending ) {
            // Something has happened
            if ( cTimer->m_queue == NULL && cTimer->m_id == 0 ) {
               // maxWaitTimeMillis timer
               // Set again
               class TTimeIntervalMicroSeconds32 interval =
                  maxWaitTimeMillis * 1000;
               tItem->m_timer.After( tItem->status, interval );
            } else { // real timer
#ifdef DEBUG_TIMERS
               log->debug( "TT real timer timed out, id %d, %p",
                     cTimer->m_id, cTimer->m_queue);
#endif
               queue = cTimer->m_queue;
               id = cTimer->m_id;
               timeOutItemID = cTimer->m_timeOutItemID;
               cTimer->remove( &timers );
               //cTimer->m_timer.Close();
               deleteTimer = cTimer;
            }
         }
         cTimer = cTimer->m_next;
         delete deleteTimer;
         deleteTimer = NULL;
      }


      // Lock ModuleQueue, must be locked before m_mutex to avoid deadlo
      if(! m_terminated && queue != NULL){
         queue->lock();

         // Lock m_timeOuts mutex
         m_mutex.lock();
         // Find timeoutitem for status and return if found
         cur = m_timeOuts;
         while(!m_terminated && (cur != NULL)) {
            if((cur->m_queue == queue) && (cur->m_id == id) &&
                  (cur->m_timeOutItemID == timeOutItemID)){
               // Found, now return
#ifdef DEBUG_TIMERS
               log->debug("TT returning %p to %p, id:%i, tID",
                     cur->m_timerBuf, cur->m_queue, id, timeOutItemID);
#endif
               cur->m_queue->insert( cur->m_timerBuf );

               cur->remove( &m_timeOuts );
               delete cur;
               break;
            }
            cur = cur->m_next;
         }

         m_mutex.unlock();

         // Unlock ModuleQueue
         queue->unlock();
      }
   }

#if 1
   // XXX: Cleanup timers
   {
#ifdef DEBUG_TIMERS
      log->debug("Timerthread cleanup");
#endif
      while(timers != NULL){
         class timerItem* tmp = timers;
         tmp->remove(&timers);
#ifdef DEBUG_TIMERS
         log->debug("Cancel timer %d", tmp->m_id);
#endif
         if(tmp->status == KRequestPending){
            tmp->m_timer.Cancel();
            User::WaitForRequest(tmp->status);
         }
#ifdef DEBUG_TIMERS
         log->debug("Delete timer %d", tmp->m_id);
#endif
         delete tmp;
      }

#ifdef DEBUG_TIMERS
      log->debug("timers deleted");
#endif
      while(m_timeOuts != NULL){
         class timeOutItem* tmp = m_timeOuts;
#ifdef DEBUG_TIMERS
         log->debug("Removing timer %d", tmp->m_id);
#endif
         tmp->remove(&m_timeOuts);
         delete tmp->m_timerBuf;
#ifdef DEBUG_TIMERS
         log->debug("Delete timer %d", tmp->m_id);
#endif
         delete tmp;
      }
#ifdef DEBUG_TIMERS
      log->debug("timeOutItems deleted");
#endif

   }
#endif

   m_goneFishing = 1;
}

TInt
TimerThread::startF( TAny *aPtr )
{
   class TimerThread* thread = static_cast<TimerThread*>(aPtr);

   thread->run();

   //      User::Exit(KErrNone);
   return 0;
}

} // namespace
