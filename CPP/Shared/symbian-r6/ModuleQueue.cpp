/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "Buffer.h"
#include "MsgBuffer.h"
#include "MsgBufferEnums.h"
#include "ModuleQueue.h"
#include "TimerThread.h"
#include "GlobalData.h"

#ifndef NO_LOG_OUTPUT
# define NO_LOG_OUTPUT
#endif
#include "Log.h"
#include "LogMacros.h"
#define THREADNAME \
(Thread::currentThread() ? Thread::currentThread()->getName() : "")



namespace isab {

   MsgBuffer* ModuleQueue::getMessage(){
      lock();
      MsgBuffer* ret = NULL;
      ListTimer now;
      do{
         // Time is now...
         now.now();
         while(m_timers != NULL && ((*m_timers) <= now)){
            // all timers that should have expired by now are treated
            // here. 
            DBG("(%s) preexpired id %i, %"PRIu32", now %"PRIx32, THREADNAME,
                  m_timers->id, m_timers->millis(), now.millis());
            MsgBuffer* timerBuf = 
               new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), 0, 
                             MsgBufferEnums::TIMER);
            timerBuf->setExtraData(m_timers->id);
            insert(timerBuf);
            ListTimer* timer = m_timers;
            timer->unlink(&m_timers);
            timer->insert(&m_expiredTimers);
         }
         if(m_queueHead.next == &m_queueHead){
            // the list is empty
            // find the timout
            long timeout = DEFAULT_TIMEOUT;
            uint16 timerID = MAX_UINT16;
            if(m_timers != NULL){
               DBG("%s:%d - m_timers = %p", __FILE__, __LINE__,m_timers);
               isabTime timer(*m_timers);
               timer.sub(now);
               timeout = timer.millis();
               timerID = m_timers->id;
               logTimers(THREADNAME, __LINE__, m_log);
            }
            logTimers(THREADNAME, __LINE__, m_log);

            // Set timeout in TimerThread
            MsgBuffer* timerBuf = 
               new MsgBuffer( uint32(MsgBufferEnums::ADDR_LINK_LOCAL), 0,
                              MsgBufferEnums::TIMER );
            timerBuf->setExtraData( timerID );
            DBG("(%s) set tt %i, %"PRIi32", this=%p, buf=%p", 
                      THREADNAME,
                      timerID, timeout, this, timerBuf);
            ((TimerThread *)m_timerThread)->setTimer(this, timerID, 
                                                     timeout, timerBuf );
            DBG("(%s) timer %u set", THREADNAME, timerID);
            logTimers(THREADNAME, __LINE__, m_log);
            // wait for timeout/notify
            wait( timeout );
            logTimers(THREADNAME, __LINE__, m_log);
            // Check if timer then move to expired
            QueueItem* cur = m_queueHead.next;
            while( cur != &m_queueHead ) {
               DBG("%d: cur: %p != &m_queueHead(%p)", 
                   __LINE__, cur, &m_queueHead);
               DBG("%d: cur->buf->msgtype: %#x, TIMER: %#x", __LINE__, 
                   cur->buffer->getMsgType(), MsgBufferEnums::TIMER);
               DBG("%d: cur->buf->xtradata: %#hx, timerID: %#hx", __LINE__, 
                   cur->buffer->getExtraData(), timerID);
               if ( cur->buffer->getMsgType() == MsgBufferEnums::TIMER &&
                    cur->buffer->getExtraData() == timerID ) {
                  if ( m_timers != NULL ) {
                     // Find timer and move to expired
                     logTimers(THREADNAME, __LINE__, m_log);
                     ListTimer* q = ListTimer::findID( m_timers, timerID );
                     logTimers(THREADNAME, __LINE__, m_log);
                     if ( q != NULL ) {
#ifndef NO_LOG_OUTPUT
                        isabTime now2;
                        DBG("(%s) expired id %i (%p), %"PRIu32", now %"PRIu32, 
                            THREADNAME,
                            m_timers->id, cur->buffer, 
                            m_timers->millis(), now2.millis());
                        logTimers(THREADNAME, __LINE__, m_log);
#endif
                        q->unlink( &m_timers );
                        logTimers(THREADNAME, __LINE__, m_log);
                        q->insert( &m_expiredTimers );
                        logTimers(THREADNAME, __LINE__, m_log);
                     }
                  }
                  if ( cur->buffer->getExtraData() == MAX_UINT16 ) {
                     // Dummy timeout timer, remove
                     // deleting a QueueItem unlinks it from its list.
                     delete cur->buffer;
                     delete cur;
                  }
                  break; // Only one outstanding timer
               }
               DBG("%d: cur: %p, cur->next: %p", __LINE__, cur, cur->next);
               cur = cur->next;
            }
            // Unset timer
            ((TimerThread *)m_timerThread)->unSetTimer( this, timerID );
         } // End if m_queueHead.next == &m_queueHead (empty list)
      }while(m_queueHead.next == &m_queueHead);
      DBG("%d: empty queue", __LINE__);

      // The list is not empty
      if(m_queueHead.next != &m_queueHead){
         //this must be a bug ?
         DBG("%d: queue not empty after all?", __LINE__);
         DBG("%s:%d: m_queueHead.next = %p", THREADNAME, __LINE__, 
                   m_queueHead.next);
         ret = m_queueHead.next->buffer;
         DBG("%s:%d: m_queueHead.next->buffer = %p", THREADNAME,__LINE__,
                   m_queueHead.next->buffer);
         // deleting a QueueItem unlinks it from its list. 
         delete m_queueHead.next;
         DBG("%s:%d: Deleting worked", THREADNAME, __LINE__);
      }
      unlock();
      DBG("%s:%d: returning buf %p", THREADNAME, __LINE__, ret);
      return ret;
   }

   ModuleQueue::ModuleQueue(const char* name) 
      : Monitor(), m_timers(NULL), m_expiredTimers(NULL),
        m_idSource(1), m_expireOffset(DEFAULT_EXPIRE_OFFSET), m_log(NULL)
   {
      m_timerThread = getArchGlobalData().m_timerThread;
      char logname[128] = "";
      strcpy(logname, name);
      strcat(logname, "Queue");
      m_log = new Log(logname);
   }

}


