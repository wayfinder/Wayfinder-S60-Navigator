/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Conditional.h"
#include "Synchronized.h"
#include "Mutex.h"
#include "RecursiveMutexImpl.h"
#include "RecursiveMutex.h"
#include "Thread.h"

namespace isab {

   class ConditionalImpl {

      /**
       *   Creates a new ConditionalImpl.
       *   @param mut The mutex to operate on.
       */
      ConditionalImpl(RecursiveMutex& mut);

      /**
       *   Deletes the handles used.
       */
      ~ConditionalImpl();

      /**
       *   Wake up one waiter.
       */
      void signal();

      /**
       *   Wait function.
       *   @param millis Maximum wait time.
       */
      int internalWait(long millis);

      /**
       *   Wake up all waiters and wait until they
       *   have woken up.
       */
//      void broadcast();

      /// Windows semaphore object.
      RSemaphore m_sema;

      /// The mutex.
      RecursiveMutex& m_mutex;

      /// The number of waiters.
      TInt m_waiters;

      /// Mutex to protect the waiters count.
      Mutex m_waitersMutex;

      /// Event to send when waking up all.
//      WinEvent m_broadcastEvent;

      /// True if broadcast is in progress.
//      bool m_haveBroadcast;

      friend class Conditional;
   };
}


isab::ConditionalImpl::ConditionalImpl(RecursiveMutex& mut) : m_mutex(mut)
{
//   m_haveBroadcast = false;
   m_waiters = 0;
   m_sema.CreateLocal( 0/* Initial count*/ ); 
}

isab::ConditionalImpl::~ConditionalImpl()
{
   m_sema.Close();
}

void
isab::ConditionalImpl::signal()
{
   bool waitersPresent;
   {
      m_waitersMutex.lock();
      waitersPresent = m_waiters > 0;
      m_waitersMutex.unlock();
   }
   if ( waitersPresent ) {
      // Signal the semaphore once.
      m_sema.Signal();
   }
}

int
isab::ConditionalImpl::internalWait(long millis)
{
   m_waitersMutex.lock();
   // Increase the number of waiters
   ++m_waiters;
   m_waitersMutex.unlock();

   int count;
   if ( millis < 0 ) {
      millis = MAX_UINT32;
   }
   
   m_mutex.m_mutexImpl->m_internal.lock();
   count = m_mutex.m_mutexImpl->m_count;
   m_mutex.m_mutexImpl->m_count = 0;
   m_mutex.m_mutexImpl->m_owner = ThreadID::currentThreadID();
   m_mutex.m_mutexImpl->m_crit.Signal();
   m_mutex.m_mutexImpl->m_internal.unlock();

   m_sema.Wait(); /* No timeout */
   
   // OK - restore the lock * count
   m_waitersMutex.lock();
   --m_waiters;

   m_waitersMutex.unlock();

   m_mutex.m_mutexImpl->lock(count);
   return 0;
}

isab::Conditional::Conditional(RecursiveMutex& mutex)
{
   m_condImpl = new ConditionalImpl(mutex);
}

isab::Conditional::~Conditional()
{
   delete m_condImpl;
}

int
isab::Conditional::wait()
{
   return m_condImpl->internalWait(-1);
}

void
isab::Conditional::signal()
{
   m_condImpl->signal();
}

