/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Monitor.h"
#include "RecursiveMutex.h"
#include "RecursiveMutexImpl.h"

bool
isab::Monitor::ownsLock() const
{
   return m_mutex.m_mutexImpl->getOwner() == ThreadID::currentThreadID();
}

isab::Monitor::Monitor()
      : m_cond( m_mutex )
{
}

isab::Monitor::~Monitor()
{
}

isab::Monitor::waitStatus
isab::Monitor::wait(long timeout)
{
   if ( ! ownsLock() )
      return Monitor::DONT_OWN;
   timeout = 0; // Ununsed...
   m_cond.wait( /*timeout*/ );
   return Monitor::WOKE_ON_NOTIFY;
}

bool
isab::Monitor::notify()
{
   if ( ! ownsLock() ) {
      return false;
   }
   m_cond.signal();
   return true;
}

bool
isab::Monitor::notifyAll()
{
   if ( ! ownsLock() ) {
      return false;
   }
   m_cond.signal();
   return true;
}

isab::Monitor::lockStatus
isab::Monitor::lock()
{
   m_mutex.lock();
   return Monitor::LOCK_LOCKED;
}

isab::Monitor::unlockStatus
isab::Monitor::unlock()
{
   m_mutex.unlock();
   return Monitor::RELEASED;
}
