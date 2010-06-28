/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "config.h"

#include "GtkTileMapToolkit.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>


GtkTileMapToolkit::GtkTileMapToolkit():
   m_lastTimer(0),
   m_lastIdle(0)
{

}

GtkTileMapToolkit::~GtkTileMapToolkit()
{

}

gboolean GtkTileMapToolkit::idleCallback(CallbackInfo* info) 
{
   uint32 idle_number = info->context_id;

   info->area->m_idles[idle_number]->runIdleTask(idle_number);
   info->area->m_idles.erase(idle_number);

   delete info;

   return false;
}

gboolean GtkTileMapToolkit::timeoutCallback(CallbackInfo* info)
{
   uint32 timer_id = info->context_id;

   info->area->m_timers[timer_id]->timerExpired(timer_id);
   info->area->m_timers.erase(timer_id);

   delete info;

   return false;
}

uint32
GtkTileMapToolkit::requestTimer(TileMapTimerListener* client,
                             uint32 timeoutMS,
                             TileMapToolkit::priority_t )
{
   uint32 timerID = ++m_lastTimer;
   
   CallbackInfo *info = new CallbackInfo;
   info->area = this;
   info->context_id = timerID;
   g_timeout_add( timeoutMS,
                  (GSourceFunc)GtkTileMapToolkit::timeoutCallback,
                  info );

   m_timers[timerID] = client;

   return timerID;
}

bool
GtkTileMapToolkit::cancelTimer(TileMapTimerListener* client,
                            uint32 timerID)
{
   return false;
}

TileMapPeriodicTimer*
GtkTileMapToolkit::createPeriodicTimer( TileMapTimerListener* listener,
                                     TileMapToolkit::priority_t prio )
{
   assert(listener);
   return new GenericTileMapPeriodicTimer( listener,
                                           this,
                                           ++m_lastTimer,
                                           prio );
}

uint32
GtkTileMapToolkit::requestIdle(TileMapIdleListener* client)
{   

   uint32 idleID = ++m_lastIdle;

   CallbackInfo *info = new CallbackInfo;
   info->area = this;
   info->context_id = idleID;
   g_idle_add((GSourceFunc)GtkTileMapToolkit::idleCallback, info);

   m_idles[idleID] = client;

   return idleID;
}

bool
GtkTileMapToolkit::cancelIdle(TileMapIdleListener* client,
                           uint32 idleID)
{
   return false;
}

