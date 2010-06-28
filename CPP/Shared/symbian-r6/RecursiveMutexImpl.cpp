/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "RecursiveMutexImpl.h"
#include "Thread.h"

isab::RecursiveMutexImpl::RecursiveMutexImpl()
{
   m_count = 0;
   TInt res = m_crit.CreateLocal();
   if ( res != KErrNone ) { // Error
       // XXX: PANIC!
   }
}

isab::RecursiveMutexImpl::~RecursiveMutexImpl()
{
   m_crit.Close();
}

void
isab::RecursiveMutexImpl::lockI(int count)
{
   bool obtained = false;
   while ( ! obtained ) {
      // Lock the internal mutex
      m_internal.lock();

      if ( m_count == 0 ) {
         // The mutex is not aquired
         m_count = count;
         m_owner = ThreadID::currentThreadID();
         obtained = true;
         // Enter the critical section
         m_crit.Wait();
      } else if ( m_owner == ThreadID::currentThreadID() ) {
         // Already owned by this thread.
         m_count += count;
         obtained = true;
      }
      m_internal.unlock();
      
      if ( ! obtained ) {
         // Try to lock the critical section and unlock it again.
         m_crit.Wait();
         m_crit.Signal();
      }
      // Try again
   }
}

void
isab::RecursiveMutexImpl::unlockI()
{
   // Take the internal mutex
   m_internal.lock();
   // XXX: Check for owner!!
   if ( --m_count == 0 ) {
      // Count is zero - unlock
      m_crit.Signal();
   }
   m_internal.unlock();
}

void
isab::RecursiveMutexImpl::lock() const
{
   // Work around const
   ((RecursiveMutexImpl*)this)->lockI(1);
}

void
isab::RecursiveMutexImpl::lock(int count) const
{
   // Work around const
   ((RecursiveMutexImpl*)this)->lockI(count);
}

void
isab::RecursiveMutexImpl::unlock() const
{
   // Work around const
   ((RecursiveMutexImpl*)this)->unlockI();
}

isab::ThreadID
isab::RecursiveMutexImpl::getOwner() const
{
   // Cast const
   RecursiveMutexImpl* non_const = (RecursiveMutexImpl*)this;

   // Lock the internal mutex
   non_const->m_internal.lock();
   ThreadID threadid;
   if ( m_count > 0 ) {
      threadid = m_owner;
   }
   non_const->m_internal.unlock();
   return threadid;
}


