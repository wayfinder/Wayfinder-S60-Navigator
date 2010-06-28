/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NO_LOG_OUTPUT
# define NO_LOG_OUTPUT
# define LOGPOINTER (&log)
#else
# define LOGPOINTER if(0)((Log*)0)
#endif 

#include "Buffer.h"
#include "MsgBuffer.h"
#include "ModuleQueue.h"
#include "Log.h"
#include "LogMacros.h"
#define THREADNAME \
(Thread::currentThread() ? Thread::currentThread()->getName() : "")
#ifndef NO_LOG_OUTPUT
# include <set>
#endif

namespace isab{

   uint32 ModuleQueue::size(){
      uint32 length = 0;
      lock();
      QueueItem* currListItem = &m_queueHead;
      while(currListItem->next != &m_queueHead){
         currListItem = currListItem->next;
         length++;          
      }
      unlock();
      return length;
   }

  void ModuleQueue::insert(MsgBuffer* msg){
    lock();
    //A QueueItem is linked into its list by its constructor 
    new QueueItem(&m_queueHead, m_queueHead.prev, msg);
    //Wake up anyone waiting for buffers.
    notifyAll();
    unlock();
  }
  
  void ModuleQueue::insertUnique(MsgBuffer* msg){
    lock();

    bool replacedExisting = false;
    QueueItem* currListItem = &m_queueHead;
    while (currListItem->next != &m_queueHead) {
       if ( ( currListItem->buffer != NULL ) &&
            ( currListItem->buffer->getMsgType() == msg->getMsgType() ) ) {
          // Delete the old buffer.
          delete currListItem->buffer;
          // And replace it with the new one.
          currListItem->buffer = msg;
          replacedExisting = true;
          break; // XXX: Just to make sure.
       }
       currListItem = currListItem->next;
    }
    
    if ( ! replacedExisting ) {
       //A QueueItem is linked into its list by its constructor 
       new QueueItem(&m_queueHead, m_queueHead.prev, msg);
    }
    //Wake up anyone waiting for buffers.
    notifyAll();
    unlock();
  }

  uint16 ModuleQueue::defineTimer()
  {
     lock();
     uint16 id = generateID();
     if(id != 0xffff){ // a free id was found
        ListTimer** root;
        isabTime fakeexpire;
        root = &m_expiredTimers;
        new ListTimer(id, root, fakeexpire);
     } 
     unlock();
     return id;
  }

  


  uint16 ModuleQueue::defineTimer(isabTime expire)
  {
#ifndef NO_LOG_OUTPUT 
     Log log("defineTimer(expire)");
#else
# undef LOGPOINTER
# define LOGPOINTER if(0)((Log*)0)
#endif
     lock();
     uint16 id = generateID();
     if(id != 0xffff){ // a free id was found
        ListTimer** root;
        isabTime now;
        if(now < expire || now.diff(expire) <= m_expireOffset){
           DBG("New timer %u for thread %s on timers.", id,
               THREADNAME );
           root = &m_timers;
        } else {
           DBG("New timer %u for thread %s on timers.", id, 
               THREADNAME);
           root = &m_expiredTimers;
        }
        ListTimer* newest = new ListTimer(id, root, expire);
        if(newest == m_timers){
           //The new timer will be the first to expire. 
           //notifying the waiting process will fix that the wait call
           //will use the right timeout argument.
           notifyAll();
        }
     } 
     unlock();
     return id;
  }

  uint16 ModuleQueue::defineTimer(int32 offset = 0)
  {
     isabTime expire;
     expire.add(offset);
     return defineTimer(expire);
  }

   bool ModuleQueue::stopTimer(uint16 timerID)
   {
      lock();
      bool ret = false;
      ListTimer* timer = NULL;
#ifndef NO_LOG_OUTPUT
      Log log("stopTimer");
#endif
      DBG("(%s) %i", THREADNAME, timerID);
      // find the timer.
      if(m_timers != NULL){
         timer = ListTimer::findID(m_timers, timerID);
         if(timer != NULL){
            if(timer == m_timers){
               notifyAll();
               DBG("was head");
            }
            DBG("found it on nonexpired");
            timer->unlink(&m_timers);
         }
      }
      if(timer != NULL){
         // the timer was found
         ret = true;
         
         DBG("put it on expired");
         timer->insert(&m_expiredTimers);
      }
      unlock();
      return ret;
   }

   bool ModuleQueue::setTimer(uint16 timerID, isabTime expire)
   {
      lock();
      bool ret = false;
      ListTimer* timer = NULL;
#ifndef NO_LOG_OUTPUT
      Log log("setTimer");
#endif
      DBG("(%s) %i, %"PRIx32, THREADNAME, timerID, expire.millis());
      // find the timer.
      if(m_expiredTimers != NULL){
         timer = ListTimer::findID(m_expiredTimers, timerID);
         if(timer != NULL){
            DBG("found it on expired");
            timer->unlink(&m_expiredTimers);
         }
      }
      if(timer == NULL && m_timers != NULL){
         timer = ListTimer::findID(m_timers, timerID);
         if(timer != NULL){
            if(timer == m_timers){
               notifyAll();
               DBG("was head");
            }
            DBG("found it on nonexpired");
            timer->unlink(&m_timers);
         }
      }
      if(timer != NULL){
         // the timer was found
         ret = true;
         
         // set the timer value and reposition it
         timer->setTime(expire);
         isabTime now;
         ListTimer **root;
         DBG("now=%"PRIx32", expire=%"PRIx32, now.millis(), expire.millis());
         if(now < expire || now.diff(expire) <= m_expireOffset){
            root = &m_timers;
            DBG("put it on nonexpired");
         } else {
            DBG("put it on expired");
            root = &m_expiredTimers;
         }
         timer->insert(root);
         if(timer == m_timers){
            //The new timer will be the first to expire. 
            //notifying the waiting process will fix that the wait call
            //will use the right timeout argument.
            DBG("is new head");
            notifyAll();
         }
      }
      unlock();
      return ret;
   }

   bool ModuleQueue::setTimer(uint16 timerID, int32 offset){
#ifndef NO_LOG_OUTPUT
      Log log("resetTimer");
#endif
      isabTime expire;
      DBG("(%s) params %"PRIu16", %"PRIi32", %"PRIx32, THREADNAME,
          timerID, offset, expire.millis());
      expire.add(offset);
      DBG("expire %"PRIx32, expire.millis());
      return setTimer(timerID, expire);
   }

  const isabTime* ModuleQueue::getTimer(uint16 timerID)
  {
    lock();
    ListTimer* q = ListTimer::findID(m_timers, timerID);
    if(q == NULL){
      q = ListTimer::findID(m_expiredTimers, timerID);
    }
    unlock();
    return q;
  }

   bool ModuleQueue::isTimerSet(uint16 timerID)
   {
      bool ret = false;
      lock();
      if (m_timers != NULL){
         ret = (NULL != ListTimer::findID(m_timers, timerID));
      }
      unlock();
      return ret;
   }

   void ModuleQueue::removeTimer(uint16 timerID)
   {
#ifndef NO_LOG_OUTPUT
      Log log("removeTimer");
#endif
      DBG("removeTimer. Thread: %s, timerID: %u", 
          THREADNAME, timerID);
      lock();
      ListTimer* timer = NULL;
      ListTimer** root = NULL;
      if(m_expiredTimers != NULL){
         timer = ListTimer::findID(m_expiredTimers, timerID); 
         root = &m_expiredTimers;
         DBG("Timer %hu was %sfound in expiredTimers", 
             timerID, timer ? "" : "not ");
      }
      if(timer == NULL && m_timers != NULL){
         timer = ListTimer::findID(m_timers, timerID);
         root = &m_timers;
         DBG("Timer %u was %sfound in timers", timerID, timer ? "" : "not ");
      }
      if(timer != NULL){
         if(timer == m_timers){
            DBG("Timer %u was next to expire, call notifyAll", timerID);
            notifyAll();
         }
         DBG("ID: %u, timer: %p, root: %p, *root: %p",
             timerID, timer, root, *root);
         timer->unlink(root);
         DBG("after unlink root: %p, *root: %p", root, *root);
         delete timer;
      }
      unlock();
   }
   
   void ModuleQueue::cancelTimer(uint16 timerID){
      lock();
      ListTimer* timer = ListTimer::findID(m_timers, timerID);
      if(timer != NULL){
         if(timer == m_timers){
            notifyAll();
         }
         timer->unlink(&m_timers);
         timer->insert(&m_expiredTimers);
      }
      unlock();
   }
#undef LOGPOINTER
#define LOGPOINTER log
   int ModuleQueue::logTimers(const char* file, int line, Log* log) 
   {
      int ret = 0;
#ifndef NO_LOG_OUTPUT
      if(log){
         lock();
         ret += DBG("logTimers called from: %s:%d", file, line);
         ret += DBG("m_timers: %p", m_timers);
         ListTimer* q = m_timers;
         std::set<ListTimer*> done;
         while(q != NULL && done.insert(q).second){
            ret += DBG("q(%p): %p <- (%02u) -> %p", q, q->less, q->id,q->more);
            q = q->more;
         }
         ret += DBG("m_expiredTimers: %p", m_expiredTimers);
         q = m_expiredTimers;
         done.clear();
         while(q != NULL && done.insert(q).second){
            ret += DBG("q(%p): %p <- (%02u) -> %p", q, q->less, q->id,q->more);
            q = q->more;
         }
         unlock();
      }
#else
      file = file; // To remove warnings
      line = line;
      log = log;
#endif
      return ret;
   }
   
  ModuleQueue::~ModuleQueue()
  {
     //Any timers left? Destroy them! Muahahah! Hrm, I mean... free them.
     while(m_timers) {
        ListTimer* t = m_timers;
        t->unlink(&m_timers);
        delete t;
     }
     while(m_expiredTimers) {
        ListTimer* t = m_expiredTimers;
        t->unlink(&m_expiredTimers);
        delete t;
     }
     lock();
     notifyAll();
     unlock();
     delete m_log;
  }

  int32 ModuleQueue::setExpireOffset(int32 offset)
  {
    lock();
    int32 ret = m_expireOffset;
    if(offset > 0 && offset <= m_maxExpireOffset){
      m_expireOffset = offset;
    }
    unlock();
    return ret;
  }

  int32 ModuleQueue::getExpireOffset()
  {
    lock();
    int32 ret = m_expireOffset;
    unlock();
    return ret;
  }

  void ModuleQueue::ListTimer::unlink(ListTimer** root)
  {
     if(this->more == this){
        // only object in list
        *root = NULL;
     } else if(*root == this){
        //first object in list.
        *root = this->more;
     } else {
        //just an ordinary object in the list.
     }
     //unlink
     more->less = less;
     less->more = more;
     more = less = this;
  }

   void ModuleQueue::ListTimer::insert(ListTimer** root)
   {
#ifndef NO_LOG_OUTPUT
# undef LOGPOINTER
# define LOGPOINTER (&log)
      Log log("ListTimer::insert");
#else
# undef LOGPOINTER
# define LOGPOINTER if(0)((Log*)0)
#endif
      DBG("thread: %s, root: %p, *root: %p, this: %p",
          THREADNAME, root, *root, this);
      // Shorthands. 
      ListTimer*& Root = *root;
      ListTimer& This = *this;
      if(Root == NULL){
         // the timerlist was empty 
         DBG("The timerlist was empty");
         Root = this;
         more = less = this;
      } else if((Root)->more == Root){
         // There was only one entry in the list. 
         DBG("There was only one entry in the list");
         more = less = Root;
         Root->less = Root->more = this;
      } else {
         DBG("There were two or more entries in the list.");
         // There were two or more timers in the list already 
         ListTimer* q = (Root);
         // find the position to insert this timer.
         if(*q <= This){
            while(*(q->more) < This && *(q->more) >= *q && q->more != Root){
               q = q->more;
            }
         } else {
            while(*(q->less) > This && *(q->less) <= *q && q->less != Root){
               q = q->less;
            }
            q = q->less;
         }
         // the timer shall be inserted after q
         DBG("The timer shall be inserted after q: %p, q.more: %p, q.less: %p",
             q, q->more, q->less);
         DBG("This.more: %p, This.less: %p", more, less);
         more = q->more;
         DBG("This.more = q.more = %p", more);
         less = q;
         DBG("this.less = q = %p", less);
         more->less = this;
         DBG("this.more.less = this = %p. this.more = %p", this, more);
         less->more = this;
         DBG("this.less.more = this = %p, this.less = %p", this, less);
      }
      // Do we need to redirect the root pointer ? 
      if((Root)->less == this && *Root > This){
         ListTimer now;
         if(now < This){
            DBG("Root pointer redirected from %p to %p(this)", Root, this);
            Root = this;
         }
      }
   }

  ModuleQueue::ListTimer* 
  ModuleQueue::ListTimer::findID(ListTimer* root, uint16 timerID)
  {
    ListTimer* q = NULL;
    if(root){
#ifndef NO_LOG_OUTPUT
       Log log("findID");
#endif
       q = root->more;
       while(q != root && q->id != timerID){
          q = q->more;
       }
       if(q->id != timerID){
          q = NULL;
       }
    }
    return q;
  }

  uint16 ModuleQueue::generateID(){
    uint16 ret = 0xffff;
    lock();
    if(m_idSource != 0xffff){
      //Just use the next unused id
      ret =  m_idSource++;
    } else {
      // all id's have been used. see if there is one that is free.
      // This isn't very efficient, but we'll solve that problem when
      // it becomes an issue.
      ListTimer* q = NULL;
      ListTimer* p = NULL;
      for(uint16 id = 1; ret == 0xffff && id < 0xffff; ++id){ 
        p = q = NULL;
        q = ListTimer::findID(m_timers, id);
        if(q == NULL){
          p = ListTimer::findID(m_expiredTimers, id);
        }
        if(q == NULL && p == NULL){
          ret = id;
        }
      }
    }
    unlock();
    return ret;
  }

}
