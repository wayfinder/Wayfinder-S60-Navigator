/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <windows.h>

#include "TileMapHandler.h"
#include "WinTileMapToolkit.h"

/* the global logging file */
#include <fstream>
extern ofstream gLogFile;

/* the timer list */
static WinTileMapToolkit::timerList_t gTimerList;

/* the idle list */
static WinTileMapToolkit::idleList_t gIdleList;

/* our idle handler's handle */
HINSTANCE gAppInstance = NULL;
HHOOK gIdleHook = NULL;

/* the current ID of Idle timers */
#define INITIAL_IDLE_TIMER_ID 0x100
#define INVALID_IDLE_TIMER_ID 0x0
static uint32 gCurrentIdleTimerId = INITIAL_IDLE_TIMER_ID;
static uint32 gCurrentPeriodicTimerID = 1;

/* the timer event handling procedure */
static VOID CALLBACK TimerHandler(HWND hwnd,
                                  UINT uMsg,
                                  UINT idEvent,
                                  DWORD dwTime)
{
   /* redundant check to make sure that the event is correct */
   if(uMsg == WM_TIMER) {
      /* handle the timer */
      WinTileMapToolkit::timerList_t::iterator it = gTimerList.find(idEvent);
      /* check if timer is in list */
      if(it == gTimerList.end()) return;
      /* kill the timer so it does not repeat again! */
      KillTimer(NULL, idEvent);
      /* call the listener */
      it->second->timerExpired(it->first);
      /* remove the timer from the list */
      gTimerList.erase(it);
   }
   return;
}

/* the hook procedure for the Idle time handling */
static LRESULT CALLBACK IdleHandlerProc(int code, WPARAM wParam, LPARAM lParam)
{
   #define NUM_IDLE_TO_RUN    64

   /* check if we are allowed to process during Idle time */
   if(code == HC_ACTION) {
      /* run multiple idle tasks at once */
      int counter = NUM_IDLE_TO_RUN;

      do {
         /* check if any tasks are pending */
         if(gIdleList.empty()) {
            /* uninstall our idle hook */
            if(gIdleHook) {
               UnhookWindowsHookEx(gIdleHook);
               gIdleHook = NULL;
            }

            int numHandled = NUM_IDLE_TO_RUN - counter;
            if(numHandled) {
               gLogFile << "WINTOOLKIT : Idle Handler Called : " 
                        << numHandled << " tasks handled." << endl;
            }
            return(0);
         }
         /* yup, do the needful */
         WinTileMapToolkit::idleList_t::value_type entry = gIdleList.front();
         /* remove the timer from the list */
         gIdleList.pop_front();
         /* call the listener */
         entry.second->runIdleTask(entry.first);
      } while(--counter);

      gLogFile << "WINTOOLKIT : Idle Handler Called : " 
               << NUM_IDLE_TO_RUN << " tasks handled." << endl;

      /* return success */
      return(0);
   }

   /* we need to pass on to the next hook and 
      return the value we get from it */
   return(CallNextHookEx(gIdleHook, code, wParam, lParam));
}

/* constructor */
WinTileMapToolkit::WinTileMapToolkit(HWND parentWin, HINSTANCE appInst)
:  m_parentWin(parentWin)
{
   gAppInstance = appInst;
}

/* destructor */
WinTileMapToolkit::~WinTileMapToolkit()
{
   /** FIXME : This only makes sense if there is just one instance of 
     *         WinTileMapToolkit. Since, deleting one object will clean
     *         up the timers of the other toolkits too. Maybe we should
     *         not clean up the timers at all since Windows will do that
     *         for us at shutdown (most probably).
     */
   /* clear up the timer list */
   for(timerList_t::iterator it = gTimerList.begin();
       it != gTimerList.end();
       ++it) {
          /* cancel the timer */
          KillTimer(NULL, it->first);
   }
   /* empty the list */
   gTimerList.clear();

   /* clear up the idle list */
   gIdleList.clear();
}

/**
 *   Requests a timer from the system. When the timer
 *   expires client->timerExpired(id) should be called.
 *   @param client  Requester to be called when it is done.
 *   @param timeoutMS Timeout in milliseconds.
 *   @return The id of the timer.
 */
uint32 WinTileMapToolkit::requestTimer(TileMapTimerListener* client,
                                       uint32 timeoutMS,
                                       priority_t prio )
{
   // XXX: Use prio.
   /* try to create a timer for the specified period */
   uint32 id = SetTimer(NULL, 0, timeoutMS, TimerHandler);
   if(id == 0) {
      /* could not allocate a timer */
      return(INVALID_TIMER);
   }

   /* add the timer to the global timer list */
   gTimerList.insert( make_pair(id,client) );

   /* return the id of the timer */
   return(id);
}

/**
 *   Requests that the client should not be called.
 */
bool WinTileMapToolkit::cancelTimer(TileMapTimerListener* client,
                                    uint32 timerID)
{
   /* remove it from the queue */
   timerList_t::iterator it = gTimerList.find(timerID);
   /* check if timer is in list */
   if(it == gTimerList.end()) return(false);
   /* kill the specified timer */
   if(KillTimer(NULL, timerID)) return(true);
   return(false);
}

TileMapPeriodicTimer*
WinTileMapToolkit::createPeriodicTimer( TileMapTimerListener* listener,
                                        TileMapToolkit::priority_t prio )
{
   return new GenericTileMapPeriodicTimer( listener,
                                           this,
                                           ++gCurrentPeriodicTimerID,
                                           prio );
}
/**
 *   Requests to be notified once when the system is idle.
 *   0 should not be returned.
 *   @param client The client to be called.
 *   @return An id which will be used when calling the client
 */
uint32 WinTileMapToolkit::requestIdle(TileMapIdleListener* client)
{
   /* add the timer to the global timer list using the current ID */
   gIdleList.push_front( make_pair(gCurrentIdleTimerId,client) );

   /* save the current id and increment our id counter */
   uint32 id = gCurrentIdleTimerId;
   ++gCurrentIdleTimerId;

   /* check if the timer ID has wrapped around */
   if(gCurrentIdleTimerId < INITIAL_IDLE_TIMER_ID) {
      gCurrentIdleTimerId = INITIAL_IDLE_TIMER_ID;
   }

   /* check if the idle handler is installed, install if none */
   if(gIdleHook == NULL) {
      /* install our Idle time handler */
      gIdleHook = SetWindowsHookEx(WH_FOREGROUNDIDLE, 
                                   IdleHandlerProc, 
                                   gAppInstance, 
                                   GetCurrentThreadId());
   } 

   /* return the id of the timer */
   return(id);
}

/**
 *   Request not to be called when the system is idle for the 
 *   supplied client and id. 
 */
bool WinTileMapToolkit::cancelIdle(TileMapIdleListener* client, uint32 id )
{
   /* find the timers id */
   idleList_t::iterator it = find(gIdleList.begin(),
                                  gIdleList.end(),
                                  make_pair(id, client));
   /* check if timer is in list */
   if(it == gIdleList.end()) return(false);
   /* remove the timer from the list */
   gIdleList.erase(it);
   /* success */
   return(true);
}

/**
 *   Signals the platform that the map needs to be redrawn.
 *   The platform should call client->repaint as soon as possible.
 *   @param client The client to call when repaint is possible.
 */
uint32 WinTileMapToolkit::requestRepaint(TileMapHandler* client)
{
   gLogFile << "WINTOOLKIT : Repaint Requested..." << endl;
   /* initiate an immediate repaint */
   client->repaint(0);
   return(0);
}


/**
 *   Returns the available memory and the biggest block.
 */
uint32 WinTileMapToolkit::availableMemory(uint32& biggestBlock) const
{
   /* get the memory status of the machine */
   MEMORYSTATUS memStatus;
   /* we need to set the size of the structure ourselves...Windows is 
      finicky about this. */
   memStatus.dwLength = sizeof(MEMORYSTATUS);
   GlobalMemoryStatus(&memStatus);
   /* get the biggest block */
   biggestBlock = memStatus.dwAvailPhys;
   /* return the amount of physical memory available */
   return(memStatus.dwAvailPhys + memStatus.dwAvailVirtual);
}

/**
 *   Returns an id string containing info about the client
 *   which will be used when requesting the TileMapFormatDesc
 *   from the server. Max 20 chars.
 */
const char* WinTileMapToolkit::getIDString() const
{
   return( "WinClientConn/0.01" );
}
