/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include<arch.h>

#ifdef __SYMBIAN32__
#include <e32std.h>
typedef TThreadId thread_id_t;
typedef int       thread_handle_t;
#elif defined (_MSC_VER)
#include <windows.h>
#include <Kfuncs.h>
typedef DWORD  thread_id_t;
typedef HANDLE thread_handle_t;
#else
#include <JTC/JTC.h>
typedef JTCThreadId thread_id_t;
typedef int         thread_handle_t;
#endif
namespace isab {
   
   class ThreadID {
   public:
      /**
       *   Creates a thread id suitable for use in symbian.
       *   @param id     The id from currentThreadID().
       *   @param handle The handle from GetCurrentThread()
       */
      ThreadID(thread_id_t id = 0,
               thread_handle_t handle = 0) : m_threadID(id), m_handle(handle)
      {  }
      
      /**
       *   Equality operator. Does not check handle.
       */
      bool operator==(const ThreadID& other) const {
         return ((thread_id_t&)m_threadID == other.m_threadID) != false;
      }
      
      /**
       *   Inequality operator
       */
      bool operator!=(const ThreadID& other) const {
         return !(*this == other);
      }

      /**
       *   Returns the id for the currently running thread.
       */
      static ThreadID currentThreadID() {
#ifdef __SYMBIAN32__
         RThread thisThread; // Generic handle meaning "the current thread"
         TThreadId id = thisThread.Id();
         thisThread.Close();
         return id;
#elif defined (_MSC_VER)
         return ThreadID(GetCurrentThreadId(), 0);
#else
         return ThreadID(JTCThreadId::self(), 0);
#endif
      }
   private:      
      thread_id_t m_threadID;
      thread_handle_t m_handle;
   };
}
