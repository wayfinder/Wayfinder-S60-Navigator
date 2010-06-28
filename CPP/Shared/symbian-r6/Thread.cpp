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
#include <string.h>
#include <e32std.h>

#include "Thread.h"
#include "Runnable.h"
#include "ThreadID.h"
#include "Synchronized.h"
#include "Log.h"
#include "WFTextUtil.h"
#include "GlobalData.h"
#include "nav2util.h"
#include <stdio.h>

namespace isab {

#ifdef NAV2_CLIENT_UIQ3
# define USE_THREAD_EXCEPTION_HANDLER
#endif

#if defined USE_THREAD_EXCEPTION_HANDLER
   void logException(enum TExcType arg)
   {
      class Log log("EXCEPTION");
      
      const char* tName = "UnknownThread";
      const char* eName = NULL;
      if( Thread::currentThread() ){
         tName = Thread::currentThread()->getName();
      }
      switch(arg){
      case EExcGeneral:              eName = "EExcGeneral";              break;
      case EExcIntegerDivideByZero:  eName = "EExcIntegerDivideByZero";  break;
      case EExcSingleStep:           eName = "EExcSingleStep";           break;
      case EExcBreakPoint:           eName = "EExcBreakPoint";           break;
      case EExcIntegerOverflow:      eName = "EExcIntegerOverflow";      break;
      case EExcBoundsCheck:          eName = "EExcBoundsCheck";          break;
      case EExcInvalidOpCode:        eName = "EExcInvalidOpCode";        break;
      case EExcDoubleFault:          eName = "EExcDoubleFault";          break;
      case EExcStackFault:           eName = "EExcStackFault";           break;
      case EExcAccessViolation:      eName = "EExcAccessViolation";      break;
      case EExcPrivInstruction:      eName = "EExcPrivInstruction";      break;
      case EExcAlignment:            eName = "EExcAlignment";            break;
      case EExcPageFault:            eName = "EExcPageFault";            break;
      case EExcFloatDenormal:        eName = "EExcFloatDenormal";        break;
      case EExcFloatDivideByZero:    eName = "EExcFloatDivideByZero";    break;
      case EExcFloatInexactResult:   eName = "EExcFloatInexactResult";   break;
      case EExcFloatInvalidOperation:eName = "EExcFloatInvalidOperation";break;
      case EExcFloatOverflow:        eName = "EExcFloatOverflow";        break;
      case EExcFloatStackCheck:      eName = "EExcFloatStackCheck";      break;
      case EExcFloatUnderflow:       eName = "EExcFloatUnderflow";       break;
      case EExcAbort:                eName = "EExcAbort";                break;
      case EExcKill:                 eName = "EExcKill";                 break;
      case EExcUserInterrupt:        eName = "EExcUserInterrupt";        break;
      case EExcDataAbort:            eName = "EExcDataAbort";            break;
      case EExcCodeAbort:            eName = "EExcCodeAbort";            break;
      case EExcMaxNumber:            eName = "EExcMaxNumber";            break;
      case EExcInvalidVector:        eName = "EExcInvalidVector";        break;
      }
      if(0 == strncmp(tName, "Wayfinder", strlen("Wayfinder"))){
         tName += strlen("Wayfinder");
      }
      log.error("Caught %s in thread %s", eName, tName);
   }
#else
   void logException(enum TExcType)
   {
      //do nothing
   }
#endif

   void exceptionHandler(enum TExcType arg)
   {
      logException(arg);
      class Thread* panicThread = Thread::currentThread();
      if( panicThread ){
         //starts a terminateandjoin process in the main thread.
         ::Panic(panicThread); 
         panicThread->m_alive= false;
      }
      User::Exit(1);
   }

   /**
    *   InnerThread holds symbian specific thread info.
    *   A bit messy.
    */
   class InnerThread {
   public:
      /**
       *   Creates a new inner thread with the name name.
       */
      InnerThread(Thread* thread,
                  Runnable* runnable,
                  const char* name);
      /** Destructor. */
      ~InnerThread();
      
      /**
       *   Returns the Thread of the InnerThread.
       */
      Thread* getThread() const;
      
      /**
       *  Returns the handle of the SymbianThread.
       */
      class RThread& getHandle();
      
      /**
       *    Starts a new thread.
       */
      int start();

      /**
       *    Returns the thread id of the thread.
       */
      class ThreadID getID() const;

      /**
       *    Returns the runnable of the inner thread.
       */
      class Runnable* getRunnable() const;

      /**
       *    Returns the name of the innner thread.
       */
      const char* getName() const;

      int getThreadInfo(char* data, int datalength);

   private:

      /** Runnable - the object to start the run thread in */
      class Runnable* m_runnable;
      /** The Thread object that belong to the thread */
      class Thread* m_thread;

      /** The ThreadID thread id of the inner thread */
      class ThreadID m_id;

      /** The thread handle of the inner thread */
      class RThread m_threadHandle;

      /** The name of the thread */
      char* m_name;

      /** Keeps track of wether the call to m_threadHandle.Create()
       * was successfull or not*/
      bool m_created;

      /**
       * Sets alive in Thread, used by start_hook.
       */
      void setAlive( bool alive );

      /**
       * Start function when creating thread.
       */
      static TInt start_hook( TAny *aPtr );
   };
}

isab::InnerThread::
InnerThread(class Thread* thread,
            class Runnable* runnable,
            const char* name) :
   m_runnable(runnable),
   m_thread(thread),
   m_created(false)
{
   if ( name == NULL ) {
      name = "NULL";
   }
   m_name = strdup_new(name);
   // Don't use the name yet
}

isab::InnerThread::~InnerThread()
{
   if(m_created && m_threadHandle.ExitReason() == 0){
      m_threadHandle.Kill(1);
   }
   m_threadHandle.Close();
   delete[] m_name;
   m_name = NULL;
//    GlobalData* gd = getGlobalData();
//    gd->m_innerThread = NULL;
//    delete gd;
}

class isab::ThreadID
isab::InnerThread::getID() const
{
   return m_id;
}

class isab::Thread*
isab::InnerThread::getThread() const
{
   return m_thread;
}

class isab::Runnable*
isab::InnerThread::getRunnable() const
{
	return m_runnable;
}

class isab::Thread*
isab::Thread::currentThread()
{
   // Get from TLS, stored in start_hook.
   class InnerThread* innerThread = getTlsGlobalData()->m_innerThread;
   return innerThread ? innerThread->getThread() : NULL;
}

class RThread&
isab::InnerThread::getHandle()
{
   return m_threadHandle;
}

const char* 
isab::InnerThread::getName() const
{
   return m_name;
}

void 
isab::InnerThread::setAlive( bool alive ) {
   m_thread->setAlive( alive );
}

int isab::InnerThread::getThreadInfo(char* data, int dataLength)
{
   snprintf(data, dataLength, 
            "%s : threadId : 0x%"PRIx32" : handle: 0x%"PRIx32,
            getName(), TUint(m_threadHandle.Id()), m_threadHandle.Handle());
   data[dataLength - 1] = '\0';
   return strlen(data);
}

TInt SetExceptionHandler(class RThread& aThread, TExceptionHandlerPtr aHandler)
{
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   aThread.Handle(); //shut up the compiler.
   return User::SetExceptionHandler(*aHandler, MAX_UINT32);
#else
   return aThread.SetExceptionHandler(aHandler, MAX_UINT32);
#endif
}

TInt
isab::InnerThread::start_hook(TAny *aPtr)
{
   class TlsGlobalData* newTlsGlobalData = 
      reinterpret_cast<TlsGlobalData*>(aPtr);
   class ArchGlobalData& archGlobalData = *(newTlsGlobalData->m_archGlobal);

   setTlsGlobalData(aPtr);

   class InnerThread* innerThread = newTlsGlobalData->m_innerThread;
   class CTrapCleanup* tc = CTrapCleanup::New();

   TExceptionHandlerPtr eH = archGlobalData.m_exceptionHandler;
   if(KErrNone != SetExceptionHandler(innerThread->getHandle(), eH)){
      ///XXX
   }

   TRAPD(trappederror, innerThread->m_runnable->run());
   if (trappederror != KErrNone) {
#ifdef STORE_TRAP_INFO_IN_FILE
      char fileName[256];
      strncpy(256, fileName, getGlobalData()->m_commondata_base_path);
      strcat(fileName, "trap.txt"); //XXX may overflow
      
      FILE* foo = fopen(fileName, "w");
      if(foo){
         fprintf(foo, "Error %i in %s\n",trappederror, innerThread->getName());
         fclose(foo);
      }
#endif
      //starts a terminateandjoin process in the main thread.
      //now exit as gracefully as possible.
      ::Panic(innerThread->m_thread);
   }

   delete tc;
   innerThread->m_thread->setAlive(false);

   deleteTlsGlobalData(newTlsGlobalData);
   // CloseSTDLIB();

   if(trappederror != KErrNone){
      User::Exit(1);
   }

   // The thread has stopped.
   return 0;
}

int
isab::InnerThread::start()
{
   //if ( m_id == m_threadHandle.Id() ) {
   if(m_thread->isAlive()){
      return Thread::ALREADY_STARTED;
   }

   // Copy the global section for the new thread. Fill it in as well.
   class TlsGlobalData * newTls = new TlsGlobalData(*getTlsGlobalData(), this);
   if(!newTls){
      return KErrNoMemory;
   }

   //widen thread name. 
   HBufC* tname = WFTextUtil::Alloc(m_name);
   if(!tname){
      delete newTls;
      return KErrNoMemory;
   }
   
   // Uses creators Heap
   TInt res = m_threadHandle.Create(
      *tname,
      /*(TInt (*)(TAny *))*/start_hook,
      KDefaultStackSize,//128*1024, // Stack Size
      NULL, // Use same heap as creator, can delete things created by other
      (TAny*)newTls // Data pointer to send to start_hook
      /*TOwnerType aType=EOwnerProcess*/ ); 

   delete tname;

   if ( res != KErrNone ) {
      delete newTls;
      switch(res){
      case KErrAlreadyExists:
         return Thread::ALREADY_STARTED + 100; ///UUUUaaargh!! XXX
      case KErrBadName:
         return res;
      default:
         return res - 100;
      }
   } else {
      m_created = true;
   }
   m_id = m_threadHandle.Id();
   if (getArchGlobalData().m_use_exception_handler) {
      getArchGlobalData().m_exceptionHandler = &exceptionHandler;
   }
   m_thread->setAlive( true );

   m_threadHandle.Resume();
/*    Thread::sleep( 100, 0 ); // Symbian Sucks! */
/*    Thread::sleep( 100, 0 ); // Symbian Sucks! But Linux BLOWS! */

   return Thread::GOOD_START;
}


isab::Thread::Thread(const char* name) : m_alive(false)
{
   init(this, name);
}

isab::Thread::Thread(class Runnable* runnable,
                     const char* name) : m_alive(false)
{
   init(runnable, name);
}

void isab::Thread::init(class Runnable* runnable, const char* name)
{
   const char threadPrefix[] = "Wayfinder";
   char* wfname = new char[strlen(name) + sizeof(threadPrefix) + 4];
   strcpy(wfname, threadPrefix);
   strcat(wfname, name);
   m_innerThread = new InnerThread(this, runnable, wfname);
   delete[] wfname;
}

isab::Thread::~Thread()
{
   delete m_innerThread;
   // FIXME: Do something here.
}

int
isab::Thread::start()
{
   return m_innerThread->start();
}

void
isab::Thread::terminate()
{
   // Send the terminate on to the runnable if
   // it isn't this thread.
   if ( this != m_innerThread->getRunnable() ) {
      m_innerThread->getRunnable()->terminate();
   }
}

void isab::Thread::Kill(int reason, class Thread* originator)
{
   if(this != originator){
      m_innerThread->getHandle().Kill(reason);
      m_alive = false;
   }
}


bool
isab::Thread::isAlive()
{
   class Synchronized sync(m_mutex);
   return m_alive;
}

class TTimeIntervalMicroSeconds32 CreateInterval(long millis, int nanos)
{
  if ( millis > MAX_INT32 ) {
     millis = MAX_INT32;
     nanos = 0; // Avoid overflow
  }
  if ( millis*1000 < millis ) { // Overflow
     millis = MAX_INT32 / 1000;
     nanos = 0; // Avoid overflow
  }

  class TTimeIntervalMicroSeconds32 interval = millis*1000 + nanos/1000;
  return interval;
}

int
isab::Thread::join(long millis, int nanos)
{ 
   // Hmm. If we join ourselves we should return, I think.
   if ( ThreadID::currentThreadID() == m_innerThread->getID() ) {
      return Thread::PROPER_JOIN;
   }
   if ( ! isAlive() ) {
      return Thread::PROPER_JOIN;
   }

   // Logon request
   // Logon: When this thread dies, the request completes and 
   // status is set.
   class TRequestStatus status;
   m_innerThread->getHandle().Logon( status ); 
   if ( ! isAlive() ) {
      m_innerThread->getHandle().LogonCancel( status );
      User::WaitForRequest( status );
      return Thread::PROPER_JOIN;
   }

   // Timer request
   class RTimer timer;
   if ( timer.CreateLocal() != KErrNone ) {
      // Error
      timer.Close();
      m_innerThread->getHandle().LogonCancel( status );
      User::WaitForRequest( status );
      return Thread::FAIL;
   }
   class TRequestStatus statusTimer;
   class TTimeIntervalMicroSeconds32 interval = CreateInterval(millis, nanos);
   timer.After( statusTimer, interval );

   // Wait for logon or timer to return
   User::WaitForRequest( status, statusTimer );

   if ( statusTimer == KRequestPending ) { // Not timer that returned
       timer.Cancel(); // Cancels any outstanding request for a timer event
       User::WaitForRequest(statusTimer);
       timer.Close(); // Release timer

      if ( status == KErrCancel ) {
         return Thread::TIME_OUT;
      } else if ( status == KErrNoMemory ) {
         return Thread::JOIN_FAIL;
      } else {
         // One of TExitType
         if ( status == EExitKill || status == EExitTerminate || 
              status == EExitPanic ){
            // Dead somehow
            return Thread::PROPER_JOIN;            
         } else if ( status == EExitPending ) { // Still Alive!
            return Thread::TIME_OUT;
         } else {
            // Should never come here, but...
            return Thread::PROPER_JOIN;
         }
      }
   } else { // Timer is set
      // Unset Logon request
      m_innerThread->getHandle().LogonCancel( status );
      User::WaitForRequest(status); //not really necessary ???
      timer.Cancel(); // Cancels any outstanding request for a timer event
      timer.Close(); // Release timer

      if ( statusTimer == KErrNone ) {
         return Thread::TIME_OUT;
      } else {
         return Thread::JOIN_FAIL;
      }
   }
}

int
isab::Thread::sleep(long millis, int nanos)
{
    class RTimer timer;
    if ( timer.CreateLocal() != KErrNone ) {
        // Error
        timer.Close();
        return Thread::FAIL;
    }

    class TRequestStatus status;
    class TTimeIntervalMicroSeconds32 interval = CreateInterval(millis, nanos);
    timer.After( status, interval );
    User::WaitForRequest( status );
    timer.Close();

    if ( status == KErrNone ) {
        return AWOKE;
    } else {
        return FAIL;
    }
}

void
isab::Thread::yield()
{
   // Sleep 0 might work as yield. 
   sleep(0, 0);
}

const char* 
isab::Thread::getName() const
{
   return m_innerThread->getName();
}

int isab::Thread::getThreadInfo(char* data, int dataLength)
{
   return m_innerThread->getThreadInfo(data, dataLength);
}

