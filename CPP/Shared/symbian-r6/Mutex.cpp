/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "arch.h"
#include "Mutex.h"

namespace isab {
   class MutexImpl {
   private:
      /**
       * Critical section. Works like a mutex but only for one process 
       * and is faster
       */
      RMutex m_crit;
      
      /** Mutex can use the variable. */
      friend class Mutex;
   };
}

// ============= MUTEX ================

isab::Mutex::Mutex()
{
   m_mutexImpl = new MutexImpl();
   TInt res = m_mutexImpl->m_crit.CreateLocal();
   if ( res != KErrNone ) { // Error
       // XXX: PANIC!
      User::Exit( 1 );
   }
}

isab::Mutex::~Mutex()
{
   m_mutexImpl->m_crit.Close();
   delete m_mutexImpl;
}

void
isab::Mutex::lock()
{
   m_mutexImpl->m_crit.Wait();
}

void
isab::Mutex::unlock()
{
   m_mutexImpl->m_crit.Signal();
}
