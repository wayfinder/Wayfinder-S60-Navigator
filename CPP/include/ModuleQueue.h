/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYNCQUEUE_H
#define SYNCQUEUE_H
#include "arch.h"
#include "isabTime.h"
#include "Monitor.h"
#define DEFAULT_EXPIRE_OFFSET (5*60*1000)
/** When getMessage waits without any active timers, this timeout is
    used. It's just a precaution. This value is in milliseconds. */
#define DEFAULT_TIMEOUT 10000L

namespace isab {
   class Log;
  /** Implements the synchronizing queues used for intermodule
      communication. Extended from the Monitor class to get closer to
      the synchronizing primtives. Timers can be defined which inserts
      a buffer in the queue when there is a timeout.
  */
  class ModuleQueue : public Monitor {
    /** Extension of the Time class. Equipped with pointer to use in
        double linked lists.
    */
    class ListTimer : public isabTime {
    public:
      /** The timers ID. */
      uint16 id;
      /** Points to the neighbouring block with more time. */
      ListTimer* more;
      /** Points to the neighbouring block with less time. */
      ListTimer* less;
      
      /** Constructor. The created timer will time out offset seconds
       * from now. Also inserts the timer in the list.
       * @param ID a unique 16 bit identifier. The caller must make
       *           sure that the id isn't already present in the list
       *           pointed to by *root. It's recommended to use the
       *           ModuleQueue::generateID method to create unique
       *           ID's.
       * @param root a pointer to a pointer to a ListTimer present in
       *             the list this ListTimer shall be inserted into.
       * @param offset offset in milliseconds from now when we want
       *               the timer to expire. Default value 0, which
       *               probably means it will never time out, since
       *               that time has already passed. 
       */ 
      inline ListTimer(uint16 ID, ListTimer** root, int32 offset = 0);

      /** Constructor. The created timer will time out at the time 
       * specified in the expire argument. Also inserts the timer 
       * in a list. 
       * @param ID a unique 16 bit identifier. The caller must make
       *           sure that the id isn't already present in the list
       *           pointed to by *root. It's recommended to use the
       *           ModuleQueue::generateID method to create unique
       *           ID's.
       * @param root a pointer to a pointer to a ListTimer present in
       *             the list this ListTimer shall be inserted into.
       * @param expire the time to expire at. If this time has already 
       *               passed, the timer will still time out if expire 
       *               was less than expireOffset milliseconds ago.
       */
      inline ListTimer(uint16 ID, ListTimer** root, isabTime expire);

      /** Default constructor. Set id to 0, which is illegal, the more
       *   and less pointers to NULL, and the time to now.
       */
      inline ListTimer();

      /** Default destuctor. Unlinks the Listtimer from any list it is
          present in. */
      inline ~ListTimer();
      /** Tests if this ListTimer will expire later than the ListTimer
       * argument. 
       * @param rh the ListTimer to compare with.
       * @return true if this ListTimer will expire later than rh.
       */
      inline bool operator>(const ListTimer& rh) const;
      /** Tests if this ListTimer will expire earlier than the
       * ListTimer argument. 
       * @param rh the ListTimer to compare with.
       * @return true if this ListTimer will expire earlier than rh.
       */
      inline bool operator<(const ListTimer& rh) const;

       inline bool operator<=(const ListTimer& rh) const;
       inline bool operator>=(const ListTimer& rh) const;

      /** Inserts this timer into the list root refers to. Beware, if
       * this Listtimer is already present in some list, that list
       * will probably be broken.
       * @param root pointer to pointer to a ListTimer present in the
       *             list this ListTimer will be inserted into. If
       *             root points to a pointer with value NULL, *root
       *             will point to this Listtimer which will have it's
       *             more and less pointers set to itself. 
       */
      void insert(ListTimer** root);
      /** Unlinks the ListTimer from any list it's part of.
       * @param root a pointer to pointer to a ListTimer that is part
       *             of the same list. If only a single pointer into
       *             the list is maintained, it needs to be updated if
       *             it points to this object.
       */
      void unlink(ListTimer** root);
      /** Finds any ListTimer with the specified id in the list
       * pointed to by root. 
       * @param root pointer to any ListTimer in the list to
       *             search. 
       * @return a pointer to the found ListTimer, NULL if there
       *         was no matching timer in the list. 
       */
      static ListTimer* findID(ListTimer* root, uint16 timerID);
    private:
       ListTimer(const ListTimer& from);
       const ListTimer& operator=(const ListTimer& rhs);
    };
    /** Sorted list of timers that have yet to expire. */
    ListTimer* m_timers;
    /** List of timers that have expired. */
    ListTimer* m_expiredTimers;
    /** Struct used to keep track of queued buffers.*/
    struct QueueItem{
      /** The next QueueItem, inserted after this one. */
      QueueItem* next;
      /** The previous QueueItem, inserted before this one. */
      QueueItem* prev;
      /** The Buffer. */
      class MsgBuffer* buffer;
      /** Default constructor. MsgBuffer pointer set to NULL and more and
          less to point to this QueueItem.*/
      QueueItem();
      /** Constructor. Sets buffer and inserts this QueueItem between
       * two other QueueItems. 
       * @param n the next pointer will have this value.
       * @param p the prev pointer will be set to this value. 
       * @param b the buffer pointer will be set to this value.
       */
      QueueItem(QueueItem* n, QueueItem* p, class MsgBuffer* b); 
      /** Default destuctor. Unlinks this QueueItem from any list it
          is a part of. */
      ~QueueItem();
    };
    /** The head of the linked list used to hold QueueItems. Has no
        buffer. */
    QueueItem m_queueHead;
      
    /** Source of timer id's for this ModuleQueue. Used by the
        generateID method. */
    uint16 m_idSource;
    /** Generates unique id's for timers. It creates them by useing
     * incrementing m_idSource until it reaches the value
     * 0xffff. After that it searches through the timer list to find
     * unused id's.
     * @return a 16 bit timer id, unique for this ModuleQueue. 
     *         If 0xffff is returned, there were no available ID's.
     */
    uint16 generateID();
    /** If the difference between a set timer and now is less than
        this offset, the timer will expire even if its expiration
        time has already passed. Otherwise the set timer will never
        expire.*/
    int32 m_expireOffset;
    /** Max value for m_expireOffset. */
#ifdef _MSC_VER
    /* Broken Microsoft compiler rejects valid C++ ideoms */
    enum {m_maxExpireOffset = 60 * 60 * 1000 };
#else
    static const int32 m_maxExpireOffset = 60 * 60 * 1000;
#endif

     Log* m_log;
  public:
    /** Sets a new expire offset. The expire offset determines
     * whether a newly set timer will expire even if it's expiration
     * time has already passed. If (now - timer) <= offset the timer
     * will expire immediately, otherwise it will never expire.
     * @param offset the new expire offset. Default value is 5 min.
     *               If negative or larger than 1 hour, the expire
     *               offset will not change.
     * @return the old expire offset.
     */
    int32 setExpireOffset(int32 offset = DEFAULT_EXPIRE_OFFSET);

    /** Gets the current expire offset. */
    int32 getExpireOffset();

    /** This method removes the first buffer from the queue.  If
     * there are no buffers on the queue, the call will block until
     * a buffer arrives.  
     * @return a pointer to a buffer. The buffer
     *         is removed from the queue. It is the callers
     *         responsibility to delete the buffer when it's done.
     */ 
    class MsgBuffer* getMessage();

    /** Puts a buffer at the end of the queue. This will wake
     * any thread that has blocked on the wait call.
     * @param buf a pointer to the buffer to insert.
     */
    void insert(class MsgBuffer* msg);
   
    /**
     * Inserts the buffer if no other messages of the same type can be found.
     * Otherwise replaces the first found message of the same type in
     * the queue with this one.
     */ 
    void insertUnique(class MsgBuffer* msg);

    /** Defines a new timer that will not expire unless setTimer()
     * is called.
     * @return a 16 bit id associated with the timer. If
     *         the return value equals 0xffff no timer was created
     *         since we're out of timer ids. Try releasing some.
     */
    uint16 defineTimer();

    /** Defines a new timer.
     * @param offset the timeout value for the new timer.
     *               A value of zero will
     *               guarantee that the timer will time out at once, as long
     *               as a useful expireOffset is set.
     * @return a 16 bit id associated with the timer. If
     *         the return value equals 0xffff no timer was created
     *         since we're out of timer ids. Try releasing some.
     */
    uint16 defineTimer(int32 offset);

    /** Defines a new timer.
     * @param expire the absolute expiration time of the new timer.
     * @return a 16 bit id associated with the timer. If
     *         the return value equals 0xffff no timer was created
     *         since we're out of timer ids. Try releasing some.
     */
    uint16 defineTimer(isabTime expire);

    /** Stops a timer. Any remaining time on the timer is forgotten.
     * @param timerID a 16 bit timer id. If no currently defined 
     *                timer is associated with this id, the call
     *                is ignored.
     * @return true if the timer was already defined and active, 
     *         false otherwise.
     */
    bool stopTimer(uint16 timerID);

    /** Sets a timer. Any remaining time on the timer is forgotten.
     * @param timerID a 16 bit timer id. If no currently defined 
     *                timer is associated with this id, the call
     *                is ignored.
     * @param milliseconds the time until the timer will place a 
     *                     buffer on the queue. Specified in 
     *                     milliseconds from now. The default value is  
     *                     0, which represents now. 
     * @return true if the timer was already defined, false otherwise.
     */
    bool setTimer(uint16 timerID, int32 offset = 0);

    /** Sets a timer. Any remaining time on the timer is forgotten.
     * @param timerID a 16 bit timer id. If no currently defined 
     *                timer is associated with this id, the call
     *                is ignored.
     * @param expire the new absolute expiration time of the timer.
     * @return true if the timer was already defined, false otherwise.
     */
    bool setTimer(uint16 timerID, isabTime expire);

    /** Return the time value of a timer.
     * @param timerID the 16 bit ID of the timer to look at.
     * @return a pointer to a Time object. If this object is
     *         modified, unspecified behaviour will occur. Deleting
     *         the Time object can cause undesired side effects. Use
     *         removeTimer instead. If the timerID is unknown, NULL
     *         will be returned.
     */
    const isabTime* getTimer(uint16 timerID);

     /** Checks if a timer will expire, i.e. it has not already.
      * @param timerID the 16 bit ID of the timer to look at.
      * @return true if the timer will time out in the future, false
      *         if the timer already timed out or is set to such a 
      *         time that it never will timeout. */
     bool isTimerSet(uint16 timerID);

    /** Removes a defined timer. If the timer id is unknown, the
     * call is ignored. 
     * @param timerID a 16 bit timer ID.
     */
    void removeTimer(uint16 timerID);

     /** Mark a timer as expired.
      * @param timerID the id of the timer to mark as expired. */
     void cancelTimer(uint16 timerID);

     int logTimers(const char* file, int line, Log* log) ;
     
   /**
    * Size of the queue.
    */
   uint32 size();

    /** Default constructor. */
    ModuleQueue(const char* name);

    /** Default destructor. */
    ~ModuleQueue();

#ifdef __SYMBIAN32__
    void* m_timerThread;
#endif

  };

  //==================================================================
  //===    Inlines for ModuleQueue::ListTimer     ====================

  inline ModuleQueue::ListTimer::ListTimer()
    : isabTime(), id(0), more(NULL), less(NULL)
  {
  }

   inline ModuleQueue::ListTimer::ListTimer(uint16 ID, ListTimer** root, 
                                            isabTime expire)
      : isabTime(expire), id(ID), more(NULL), less(NULL)
   {
      insert(root);
   }


  inline ModuleQueue::ListTimer::ListTimer(uint16 ID, ListTimer** root, 
                                           int32 offset)
    : isabTime(), id(ID), more(NULL), less(NULL)
  {
    add(offset);
    insert(root);
  }

  inline ModuleQueue::ListTimer::~ListTimer()
  {
    //Unlink if linked.
    if(less != NULL){
      less->more = more;
    }
    if(more != NULL){
      more->less = less;
    }
  }

  inline bool ModuleQueue::ListTimer::operator<(const ListTimer& rh) const 
  {
    //cast to base class and use that class's operator<
    const isabTime& rht = *static_cast<const isabTime*>(&rh);
    const isabTime& lht = *static_cast<const isabTime*>(this);
    return lht < rht;
  }

  inline bool ModuleQueue::ListTimer::operator>(const ListTimer& rh) const
  {
    //cast to base class and use that class's operator>
    const isabTime& rht = *static_cast<const isabTime*>(&rh);
    const isabTime& lht = *static_cast<const isabTime*>(this);
    return lht > rht;
  }
   inline bool ModuleQueue::ListTimer::operator<=(const ListTimer& rh) const 
   {
      const ListTimer& thisRef = *this;
      return !(thisRef > rh);
   }
   inline bool ModuleQueue::ListTimer::operator>=(const ListTimer& rh) const
   {
      const ListTimer& thisRef = *this;
      return !(thisRef < rh);
   }

  //=============================================================
  //=== Inlines for ModuleQueue::QueueItem   ====================

  inline ModuleQueue::QueueItem::QueueItem() : buffer(NULL)
  {
    next = prev = this;
  }

  inline ModuleQueue::QueueItem::QueueItem(QueueItem* n, 
                                           QueueItem* p, 
                                           class MsgBuffer* b) 
    : next(n), prev(p), buffer(b)
  {
    //link in
    next->prev = this;
    prev->next = this;
  } 

  inline ModuleQueue::QueueItem::~QueueItem()
  {
    //unlink if linked.
    if(prev != NULL){
      prev->next = next;
    }
    if(next != NULL){
      next->prev = prev;
    }
  }

      
}
#endif
