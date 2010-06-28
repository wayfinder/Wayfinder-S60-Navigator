/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _SYMBIANTILEMAPTOOLKIT_H_
#define _SYMBIANTILEMAPTOOLKIT_H_

#include "TileMapToolkit.h"
#include <e32base.h>
#include <map>
#include <vector>
#include <list>

/* forward declarations */
class CCoeControl;
class SymbianIdleTimer;
class SymbianTileMapTimer;

class SymbianTileMapToolkit : public TileMapToolkit
{
   private:
      /*
       * Type of the allocated timers and their IDs
       */
      typedef std::map<uint32,SymbianTileMapTimer*> timerMap_t;

      /*
       * Holds the timer ID and pointer pair
       */
      timerMap_t m_timerArray;

      /* 
       * Holds the current usuable timer ID
       */
      uint32 m_curTimerID;

      /* holds the parent container */
      CCoeControl& m_parentContainer;

      /// Typedef of the garbage storage
      typedef std::vector<SymbianTileMapTimer*> garbageStorage_t;
      
      /// Contains timers to be deleted
      garbageStorage_t m_garbage;

      /// Deletes the timers in the garbage vector
      void cleanupOldTimers();

      /// Adds the timer for later deletion
      void addForDeletion(SymbianTileMapTimer* timer);

      // The object that handles idle callbacks.
      SymbianIdleTimer* m_idleTimer;

      // callback for Idle Timers, this delegates to the listener
      static TInt idleCallback(TAny* listener);

      /// Id string for requesting map description formats.
      char* m_idString;
      
   public:
      /* 
       *   Constructor
       *   @param parentControl The parent control of the TileMapHandler.
       *   @param extraIDInfo   Extra info to send to the server when
       *                        requesting settings. Note that the server
       *                        string cannot be longer than 20 characters.
       */
      SymbianTileMapToolkit(CCoeControl& parentControl,
                            const char* extraIDInfo = "");

      /* 
       * Destructor
       */
      virtual ~SymbianTileMapToolkit();

      /* 
       * Handles the timer expiration
       */
      void timerDone(uint32 timerID);

      /**
       *   Creates a periodic timer.
       */
      TileMapPeriodicTimer* createPeriodicTimer(TileMapTimerListener* listener,
                                                priority_t prio );
   
      /* from TileMapToolkit */
      uint32 requestTimer(TileMapTimerListener* client,
                          uint32 timeoutMS,
                          TileMapToolkit::priority_t prio );

      bool cancelTimer(TileMapTimerListener* client,
                       uint32 timerID);

      /**
       *    Requests that client should be notified when
       *    the system is idle.
       */
      uint32 requestIdle(TileMapIdleListener* client);

      /**
       *    Cancels the call to the client with the requested
       *    idle id. Uses linear search...
       */
      bool cancelIdle(TileMapIdleListener* client, uint32 id );

      /**
       *    Returns the amount of available memory and the biggest
       *    block.
       */
      static uint32 memFree(uint32& biggestBlock);
       
      /**
       *    Returns the amount of available memory.
       */
      static uint32 memFree();
      
      /**
       *    Returns the amount of available memory and the biggest
       *    block.
       */
      uint32 availableMemory(uint32& biggestBlock) const;

      /**
       *   Returns if availableMemory() is implemented and working 
       *   on the current platform.
       */
      bool trustAvailableMemory() const;

      /**
       *   Returns an id string containing info about the client
       *   which will be used when requesting the TileMapFormatDesc
       *   from the server.
       */
      const char* getIDString() const;
};

#endif
