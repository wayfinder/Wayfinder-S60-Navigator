/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef RECURSIVE_MUTEX_IMPL_H
#define RECURSIVE_MUTEX_IMPL_H

#include "Mutex.h"
#include "ThreadID.h"

namespace isab {
   
   class Thread;

   class RecursiveMutexImpl {

      RecursiveMutexImpl();
      ~RecursiveMutexImpl();
      void lockI(int count);
      void unlockI();
      void lock(int count) const;
      void lock() const;
      void unlock() const;
      
      ThreadID getOwner() const;

      /// Symbian critical section.
      RMutex m_crit;
      
      // Internal mutex.
      Mutex m_internal;

      // Number of times the mutex has been aquired.
      int m_count;
      
      // Current owner of the mutex.
      ThreadID m_owner;
      
      friend class RecursiveMutex;
      friend class ConditionalImpl;
      friend class Monitor;
   };
}

#endif
