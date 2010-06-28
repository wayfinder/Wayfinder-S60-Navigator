/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAPPLATFORMA_H
#define TILEMAPPLATFORMA_H

#include "TileMapConfig.h"

#include "DBufRequester.h"

class TileMapHandler;

/**
 *   Class containing the calls that the TileMapPlatform should
 *   call.
 */
class TileMapTimerListener {
public:
   /**
    *   To be called by TileMapToolkit when a timer expires.
    *   @param id The id of the timer as given when reqTimeout was
    *             called.
    */
   virtual void timerExpired(uint32 id) = 0;
};

class TileMapIdleListener {
public:
   /**
    *   To be called by TileMapToolkit when the rest of the application
    *   is idle.
    */
   virtual void runIdleTask(uint32 id) = 0;
};

class TileMapToolkit;

/**
 *   Template class to be able to avoid inheritance in some classes.
 */
template<class T> class TileMapTimerListenerTemplate 
   : public TileMapTimerListener {
public:
   TileMapTimerListenerTemplate( T* listener ) : m_listener( listener ) {}
   
   void timerExpired( uint32 id ) {
      m_listener->timerExpired( id );
   }

private:
   T* m_listener;
   
};

class TileMapPeriodicTimer {
protected:
   TileMapPeriodicTimer( TileMapTimerListener* listener,
                         uint32 id ) :
         m_listener( listener ),
         m_timerID( id )
      {
      }
public:
   virtual ~TileMapPeriodicTimer() {    
   }
   
   TileMapTimerListener* m_listener;
   uint32 m_timerID;
   
public:
   uint32 getTimerID() { return m_timerID; }
   
   virtual void start( uint32 period_millis ) = 0;
   virtual void stop() = 0;
   virtual bool active() = 0;
   
   
};


/**
 *    Class containing the calls needed by the TileMap-handler.
 */ 
class TileMapToolkit {
public:

   /**
    *  Priorities for timers etc. Not necessarily supported by all
    *  platforms
    */
   enum priority_t {
      PRIO_IDLE      = 0,
      PRIO_LOW       = 1,
      PRIO_STANDARD  = 2,
      PRIO_USERINPUT = 3,
      PRIO_HIGH      = 4,
   };

   /// Creates a new periodic timer. 
   virtual TileMapPeriodicTimer*
              createPeriodicTimer(TileMapTimerListener* listener,
                                  priority_t prio ) = 0;
   
   /**
    *   Requests a timer from the system. When the timer
    *   expires client->timerExpired(id) should be called.
    *   @param client  Requester to be called when it is done.
    *   @param timeoutMS Timeout in milliseconds.
    *   @return The id of the timer.
    */
   virtual uint32 requestTimer(TileMapTimerListener* client,
                               uint32 timeoutMS,
#if defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3
                               priority_t prio = PRIO_LOW
#else                               
                               priority_t prio = PRIO_STANDARD 
#endif                               
                               ) = 0;

   /**
    *   Requests that the client should not be called.
    */
   virtual bool cancelTimer(TileMapTimerListener* client,
                            uint32 timerID) = 0;

   /**
    *   Requests to be notified once when the system is idle.
    *   0 should not be returned.
    *   @param client The client to be called.
    *   @return An id which will be used when calling the client
    */
   virtual uint32 requestIdle(TileMapIdleListener* client) = 0;

   /**
    *   Request not to be called when the system is idle for the 
    *   supplied client and id. 
    */
   virtual bool cancelIdle(TileMapIdleListener* client, uint32 id ) = 0;

   /**
    *   Returns the available memory and the biggest block.
    */
   virtual uint32 availableMemory(uint32& biggestBlock) const = 0;

   /**
    *   Returns if availableMemory() is implemented and working 
    *   on the current platform.
    */
   virtual bool trustAvailableMemory() const { return true; }

   /**
    *   Returns an id string containing info about the client
    *   which will be used when requesting the TileMapFormatDesc
    *   from the server. Max 20 chars.
    */
   virtual const char* getIDString() const = 0;
   
};

/**
 *   Helper class to quickly implement a (bad) periodic timer.
 */
class GenericTileMapPeriodicTimer
   :  public TileMapPeriodicTimer,
      public TileMapTimerListener {
public:
   /**
    *   Creates a TileMapPeriodicTimer with the supplied listener.
    *   The listener must implement periodicCallback(uint32)
    */
   GenericTileMapPeriodicTimer( TileMapTimerListener* listener,
                                TileMapToolkit* toolkit,
                                uint32 timerID,                                
                                const TileMapToolkit::priority_t& prio ) :
         TileMapPeriodicTimer( listener, timerID ) {
      m_running  = false;
      m_toolkit = toolkit;
      m_prio = prio;
   }

   ~GenericTileMapPeriodicTimer() {
      stop();
   }

   void start( uint32 period_millis ) {
      m_periodMS = period_millis;
      if ( ! m_running ) {
         m_currentID = m_toolkit->requestTimer( this, m_periodMS, m_prio );
         m_running = true;
      }
   }

   bool active() {
      return m_running != 0;
   }
   
   void stop() {
      if ( m_running ) {
         m_running = false;
         m_toolkit->cancelTimer( this, m_currentID );
      }
   }

   void timerExpired( uint32 id ) {
      if ( m_running ) {
         uint32 newID = m_toolkit->requestTimer( this, m_periodMS, m_prio );
         if ( m_currentID == id ) {
            m_listener->timerExpired( m_timerID );
         }
         m_currentID = newID;               
      }
   }


   TileMapToolkit* m_toolkit;
   TileMapToolkit::priority_t m_prio;
   uint32 m_periodMS;
   int m_running;
   uint32 m_currentID;
};


#endif
