/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SymbianTilemapToolkit.h"
#include "TileMapHandler.h"
#include <eikenv.h>
#include <coecntrl.h>
#ifndef SYMBIAN_9
#include <hal.h>
#endif

#include "LogFile.h"

/* 
 * Constructor
 * Parameters : 
 * aPeriod - time period in milliseconds
 * aId - ID given to the timer
 * aTk - pointer to the creating TileMapToolkit
 */

static inline CActive::TPriority
prio_convert( TileMapToolkit::priority_t inPrio )
{
   switch ( inPrio ) {
      case TileMapToolkit::PRIO_IDLE:
         return CActive::EPriorityIdle;
      case TileMapToolkit::PRIO_LOW:
         return CActive::EPriorityLow;
      case TileMapToolkit::PRIO_STANDARD:
         return CActive::EPriorityStandard;
      case TileMapToolkit::PRIO_USERINPUT:
         return CActive::EPriorityUserInput;
      case TileMapToolkit::PRIO_HIGH:
         return CActive::EPriorityHigh;
      default:
         return CActive::EPriorityStandard;
   }
   return CActive::EPriorityStandard;
}

class SymbianTileMapPeriodicTimer 
   : public  TileMapPeriodicTimer {
   
public:
   
   SymbianTileMapPeriodicTimer( TileMapTimerListener* listener,
                                uint32 periodicID,
                                TileMapToolkit::priority_t prio )
         : TileMapPeriodicTimer( listener, periodicID ) {
      m_timer = CPeriodic::NewL( prio_convert( prio ) );
   }

   ~SymbianTileMapPeriodicTimer() {
      stop();
      delete m_timer;
   }
      
   bool active() {
      return m_timer->IsActive();
   }

   void start( uint32 period_millis ) {
      uint32 periodMicros = period_millis * 1000;
      if ( ! m_timer->IsActive() ) {
         m_timer->Start( periodMicros, periodMicros,
                         TCallBack(SymbCallback,this) );
      }
   }

   void stop() {
      if ( m_timer->IsActive() ) {
         m_timer->Cancel();
      }
   }

   

private:
   inline void periodic() {
      m_listener->timerExpired( m_timerID );
   }
   
   /* key timer callback */
   static TInt SymbCallback(TAny* aThisPtr) {
      SymbianTileMapPeriodicTimer* thisPtr =
         (SymbianTileMapPeriodicTimer*) aThisPtr;
      thisPtr->periodic();
      return 1;
   }

   CPeriodic* m_timer;
};


class SymbianTileMapTimer : public CTimer
{
   private:
      /* 
       * Contains a pointer to the specified TileMapToolkit
       */
      TileMapToolkit* m_Toolkit;
      /* 
       * Id of the timer
       */
      uint32 m_Id;
      /* 
       * Gets called when the timer expires.
       */
      TileMapTimerListener* m_Listener;

   protected:
      /* 
       * Inherited from CTimer.
       * Callback for when the timer has expired.
       */
      void RunL();

   public:
      /* 
       * Constructor
       * Parameters : 
       * aPeriod - time period in milliseconds
       * aId - ID given to the timer
       * aTk - pointer to the creating TileMapToolkit
       */
      SymbianTileMapTimer(uint32 aId, 
                          TileMapTimerListener* aListener,
                          TileMapToolkit* aTk,
                          TileMapToolkit::priority_t prio );
                          
      /* 
       * Destructor
       */
      virtual ~SymbianTileMapTimer();

      /* 
       * Starts the timer countdown
       */
      bool startTimer(uint32 aPeriod);

};


// ilde timer class
class SymbianIdleTimer
{
private:
   
   /* 
    *   The CIdle object which does the actual job
    */
   CIdle* m_idleTimer;
   
   typedef std::list<std::pair<uint32, TileMapIdleListener*> > queue_t;
   
   ///  Queue of idle objects waiting to run
   queue_t m_idleQueue;
   
   /// Current idle id.
   uint32 m_curIdleID;

   /// True if we are in the callback
   int m_inCallBack;
   
protected:
   
   //   callback for the idle timer, this delegates to the listener
   static TInt idleTimerCallback(TAny* aThisPtr);
   
   /// Callback called from the real callback
   TInt idleCallback();         
   
   /// Starts the CIdle
   bool Start();
   
   /// Stops the CIdle
   void Stop();

   
      
public:

   /// constructor
   SymbianIdleTimer();
   
   /// destructor
   ~SymbianIdleTimer();
   
   /// Adds the TileMapIdleListener to the end of the queue.
   uint32 addIdleListener(TileMapIdleListener* tml);
   
   /// Cancels an idle. Returns true if the id was found.
   bool cancelIdle(uint32 id);
      
};


SymbianTileMapTimer::SymbianTileMapTimer(uint32 aId,
                                         TileMapTimerListener* aListener,
                                         TileMapToolkit* aTk,
                                         TileMapToolkit::priority_t prio) : 
                                         CTimer(prio_convert(prio)) {
   m_Id = aId;
   m_Toolkit = aTk;
   m_Listener = aListener;
}

/* 
 * Destructor
 */
SymbianTileMapTimer::~SymbianTileMapTimer()
{
   if ( IsActive() ) {
      Cancel();
   }
   m_Id = NULL;
   m_Toolkit = NULL;
   m_Listener = NULL;
}

/* 
 * Starts the timer countdown
 */
bool SymbianTileMapTimer::startTimer(uint32 aPeriod)
{
   TRAPD(errCode, ConstructL());
   if(errCode != KErrNone) {
      return(false);
   }
   else {
      /* add the timer to the active scheduler for processing */
      CActiveScheduler::Add(this);
      /* multiply by 1000 to get microsecond count */
      After(aPeriod * 1000);
   }
   return(true);
}

/* 
* Inherited from CTimer.
* Callback for when the timer has expired.
*/
void SymbianTileMapTimer::RunL() {
   m_Listener->timerExpired(m_Id);
   ((SymbianTileMapToolkit*)m_Toolkit)->timerDone(m_Id);
   return;
}


/*******************
* SymbianIdleTimer *
********************/

// constructor
SymbianIdleTimer::SymbianIdleTimer()
{
   // This is probably forbidden in Symbian.
   m_idleTimer = CIdle::New(CActive::EPriorityIdle);
   // Means that it will start at 1.
   m_curIdleID = 0;
   m_inCallBack = 0;
}

// destructor
SymbianIdleTimer::~SymbianIdleTimer() {
   // stop the timer if active
   Stop();
   delete m_idleTimer;
}

uint32
SymbianIdleTimer::addIdleListener(TileMapIdleListener* listener)
{
   ++m_curIdleID;
   m_idleQueue.push_back( std::make_pair( m_curIdleID, listener ) );
   // Start if necessary.
   Start();
   return m_curIdleID;
}

bool
SymbianIdleTimer::cancelIdle(uint32 id)
{
   for( queue_t::iterator it = m_idleQueue.begin();
        it != m_idleQueue.end();
        ++it ) {
      if ( it->first == id ) {
         // Idle will stop on next request.
         m_idleQueue.erase( it );
         return true;
      }
   }
   return false;
}

// starts the idle timer
// returns true on success, false on error
bool
SymbianIdleTimer::Start() 
{
   // If we are in the callback, it will continue automatically
   // if the queue is not empty.
   if ( ! m_inCallBack ) {
      if ( ! m_idleTimer->IsActive() ) {
         // timer is allocated, start the callback
         m_idleTimer->Start(TCallBack(idleTimerCallback, this));
      }
   }
   // Assume success
   return true;
}

// stops the idle timer
void
SymbianIdleTimer::Stop()
{
   if( m_idleTimer->IsActive() ) {
      m_idleTimer->Cancel();
   }
}



// callback for the idle timer, this delegates to the listener
TInt
SymbianIdleTimer::idleCallback()
{
   // Check for recursive calls to start
   ++m_inCallBack;
   if ( ! m_idleQueue.empty() ) {
      queue_t::value_type curIdle = m_idleQueue.front();
      m_idleQueue.pop_front();

      // Inform the listener
      curIdle.second->runIdleTask( curIdle.first );
   }
   --m_inCallBack;
   // Nothing dangerous in callback now.
   return ! m_idleQueue.empty();
}

// Called by Symbian when idle.
TInt
SymbianIdleTimer::idleTimerCallback(TAny* aThisPtr)
{
   SymbianIdleTimer* timer = (SymbianIdleTimer*)(aThisPtr);
   return timer->idleCallback();
}


/************************
* SymbianTileMapToolkit *
*************************/

/*
 * Constructor
 */
SymbianTileMapToolkit::SymbianTileMapToolkit(CCoeControl& parentControl,
                                             const char* extraInfo) 
      : m_parentContainer(parentControl)
{

   m_idleTimer = new SymbianIdleTimer;
   if ( extraInfo == NULL ) {
      extraInfo = "";
   }
   
   /*
    * This will have to be changed to the size
    * of the map<> so that continous usage of the map doesnt 
    * cause a overflow! TODO    */

   m_curTimerID = 100;

   // Create id string 8 chars for hex id and a - then some for the extraInfo
   m_idString = new char[10+strlen(extraInfo)];

   // Get the machine id
   TInt machineID = 0;
#ifndef SYMBIAN_9
   HAL::Get( HALData::EMachineUid, machineID );
#endif
   
   sprintf(m_idString, "%X-%s", machineID, extraInfo);
   
}

/* 
 * Destructor
 */
SymbianTileMapToolkit::~SymbianTileMapToolkit()
{
   // clean up the left-over TileMap timers
   for(timerMap_t::iterator it = m_timerArray.begin(); 
       it != m_timerArray.end();
       ++it ) {
      /* checks if the timer is non-NULL & active, if it is, cancels it */
      if(it->second) {
         if( it->second->IsActive() ) {
            it->second->Cancel();
         }
         addForDeletion(it->second);
      }
   }
   m_timerArray.clear();
   cleanupOldTimers();
   delete [] m_idString;
   delete m_idleTimer;
}

TileMapPeriodicTimer*
SymbianTileMapToolkit::createPeriodicTimer( TileMapTimerListener* listener,
                                            priority_t prio )
{
   return new SymbianTileMapPeriodicTimer(listener, ++m_curTimerID, prio );
}


inline void
SymbianTileMapToolkit::addForDeletion(SymbianTileMapTimer* timer)
{
   m_garbage.push_back(timer);
}

void
SymbianTileMapToolkit::cleanupOldTimers()
{
   for ( garbageStorage_t::iterator it = m_garbage.begin();
         it != m_garbage.end();
         ++it ) {
      delete *it;
   }
   m_garbage.clear();
}


/* 
 * Handles the timer expiration
 */
void SymbianTileMapToolkit::timerDone(uint32 timerID)
{
   // Delete timers, but not this one. We know that this
   // cannot happen in another timer's context.
   cleanupOldTimers();

   // Try to find the timer.
   timerMap_t::iterator it = m_timerArray.find(timerID);
   if( it != m_timerArray.end() ) {
      addForDeletion(it->second);
      m_timerArray.erase(it);
   }

   return;
}

/* from TileMapToolkit */
/* currently returns 0xFFFFFFFF on error */
uint32 SymbianTileMapToolkit::requestTimer(TileMapTimerListener* client,
                                           uint32 timeoutMS,
                                           TileMapToolkit::priority_t prio) {

   m_curTimerID++;
   // We should not delete old timers here, since we don't
   // know if requestTimer is called in another timer's context (RunL).
   SymbianTileMapTimer* newtimer = new SymbianTileMapTimer(m_curTimerID, 
                                                           client,
                                                           this,
                                                           prio );
   /* on allocation error, return 0xFFFFFFFF */
   if(newtimer == NULL) {
      return(0xFFFFFFFF);
   }

   /* start the timer now */
   if(!newtimer->startTimer(timeoutMS))
   {
      delete newtimer;
      return(0xFFFFFFFF);
   }

   /* add the timer to internal map */
   m_timerArray.insert( std::make_pair( m_curTimerID, newtimer ) );

   /* increment timer ID */
   return m_curTimerID;
}

bool SymbianTileMapToolkit::cancelTimer(TileMapTimerListener* /*client*/,
                                        uint32 timerID)
{
   timerMap_t::iterator it = m_timerArray.find(timerID);
   
   if( it != m_timerArray.end() ) {
      if( it->second->IsActive() ) {
         it->second->Cancel();
      }
      // We can delete it right away, but we'll wait.
      addForDeletion( it->second );
      m_timerArray.erase( it );      
      return(true);
   }
   return(false);
}

/**
 *    Requests that client should be notified when
 *    the system is idle.
 */
uint32 SymbianTileMapToolkit::requestIdle(TileMapIdleListener* client)
{
   return m_idleTimer->addIdleListener(client);
}

/**
 *    Cancels the call to the client with the requested
 *    idle id.
 */
bool SymbianTileMapToolkit::cancelIdle(TileMapIdleListener* /*client*/, 
                                       uint32 id )
{
   return m_idleTimer->cancelIdle(id);
}
      
bool SymbianTileMapToolkit::trustAvailableMemory() const
{
#ifdef SYMBIAN_9
   // User::Available seems to cause crashes, and 
   // HAL::Get(HALData::EMemoryRAMFree, memFree ) is not 
   // available any more.
   return false;
#else
   return true;
#endif
}
   

uint32 
SymbianTileMapToolkit::memFree(uint32& biggestBlock)
{
#ifdef SYMBIAN_9
   // This method is not working for symbian 9.
   biggestBlock = 0;
   return 0;
#else
   TInt intbiggestBlock;

   int available = User::Available(intbiggestBlock);
   
   int memFree=0;
   HAL::Get(HALData::EMemoryRAMFree, memFree );

   available += memFree;

   biggestBlock = intbiggestBlock;
   return available;
#endif
}

uint32 
SymbianTileMapToolkit::memFree()
{
   uint32 biggestBlock;
   uint32 res = SymbianTileMapToolkit::memFree(biggestBlock);
   return res;
}

uint32
SymbianTileMapToolkit::availableMemory(uint32& biggestBlock) const
{
   uint32 res = SymbianTileMapToolkit::memFree(biggestBlock);
   return res;
}

const char*
SymbianTileMapToolkit::getIDString() const
{
   return m_idString;
}

// Handles assertions from ArchConfigMisc.h
void
handleAssert( const char* cause, const char* file, int line )
{
#ifndef __WINS__
   char* newStr = new char[strlen(cause) + strlen(file) + 2 + 1 ];
   char* lastSlash = strrchr(file, '/');
   if ( lastSlash == NULL ) {
      lastSlash = strrchr(file, '\\');
   }
   if ( lastSlash != NULL ) {
      file = lastSlash + 1;
   }
   sprintf(newStr, "%s", file);

   int length = strlen( newStr );

   HBufC* buf = HBufC::New( 2*length + 2 );

   // Modifyable buffer from inside the HBufC
   TPtr charBuf = buf->Des();
   
   for ( int i = 0; i < length + 1; ++i ) {
      charBuf.Append( (TChar) newStr[i] );
   }
   charBuf.PtrZ();

   delete [] newStr;
   // Show the assertion
   User::Panic( *buf, line );
#else
   cause = cause; // To remove warnings.
   file = file;
   line = line;
   *((uint8*)(0)) = 0;
#endif
}

// Handles TRACE
void
handleTrace( int line, const char* file, const char* function )
{
   FILE* f = fopen("C:\\trace.txt", "a" );
   if ( !f ) return;
   
   if ( function ) {
      fprintf( f, "%s:%d:%s\n", file, line, function );
   } else {
      fprintf( f, "%s:%d\n", file, line );
   }
   fclose( f );
}
