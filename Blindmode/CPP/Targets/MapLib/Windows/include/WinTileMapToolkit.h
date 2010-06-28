/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WINTILEMAPTOOLKIT_H
#define WINTILEMAPTOOLKIT_H

#include "TileMapToolkit.h"
#include <list>
#include <map>

/* invalid timer */
#define INVALID_TIMER   0xFFFFFFFF

/* Win32 specific TileMapToolkit */
class WinTileMapToolkit : public TileMapToolkit
{
   private:
      /* the handle to the parent window */
      HWND m_parentWin;

   public:

      /* Types of queues */
      /* the timer list */
      typedef map< uint32,TileMapTimerListener* > timerList_t;

      /* the idle list */
      typedef pair< uint32,TileMapIdleListener* > idleEntry_t;
      typedef list< idleEntry_t > idleList_t;

      /* constructor */
      WinTileMapToolkit(HWND parentWin, HINSTANCE appInst);

      /* destructor */
      ~WinTileMapToolkit();

      /// Creates a new periodic timer. 
      TileMapPeriodicTimer*
         createPeriodicTimer(TileMapTimerListener* listener,
                             priority_t prio );

      /**
       *   Requests a timer from the system. When the timer
       *   expires client->timerExpired(id) should be called.
       *   @param client  Requester to be called when it is done.
       *   @param timeoutMS Timeout in milliseconds.
       *   @return The id of the timer.
       */
      uint32 requestTimer(TileMapTimerListener* client,
                          uint32 timeoutMS,
                          priority_t prio );

      /**
       *   Requests that the client should not be called.
       */
      bool cancelTimer(TileMapTimerListener* client,
                               uint32 timerID);

      /**
       *   Requests to be notified once when the system is idle.
       *   0 should not be returned.
       *   @param client The client to be called.
       *   @return An id which will be used when calling the client
       */
      uint32 requestIdle(TileMapIdleListener* client);

      /**
       *   Request not to be called when the system is idle for the 
       *   supplied client and id. 
       */
      bool cancelIdle(TileMapIdleListener* client, uint32 id );

      /**
       *   Signals the platform that the map needs to be redrawn.
       *   The platform should call client->repaint as soon as possible.
       *   @param client The client to call when repaint is possible.
       */
      uint32 requestRepaint(TileMapHandler* client);

      /**
       *   Returns the available memory and the biggest block.
       */
      uint32 availableMemory(uint32& biggestBlock) const;

      /**
       *   Returns an id string containing info about the client
       *   which will be used when requesting the TileMapFormatDesc
       *   from the server. Max 20 chars.
       */
      const char* getIDString() const;
};

#endif
