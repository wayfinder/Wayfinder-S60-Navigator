/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GTKTILEMAPTOOLKIT_H 
#define GTKTILEMAPTOOLKIT_H 

#include "config.h"

#include <gtk/gtk.h>
#include "TileMapToolkit.h"
#include <map>

class GtkTileMapToolkit : public TileMapToolkit {
public:
   
   /**
    *    Creates the toolkit.
    */
   GtkTileMapToolkit();

   /**
    *    Destructor.
    */
   virtual ~GtkTileMapToolkit();
   
   /**
    *    Requests a timer, @see TileMapPlatform.
    */
   uint32 requestTimer( TileMapTimerListener* client,
                        uint32 timeoutMS,
                        TileMapToolkit::priority_t );

   /**
    *    Cancels a timer, @see TileMapPlatform.
    */
   bool cancelTimer( TileMapTimerListener* client,
                     uint32 timerID );

   /**
    *    Requests to be called the next time the application
    *    is idle.
    */
   uint32 requestIdle( TileMapIdleListener* client );

   /**
    *    Cancels an idle call.
    */
   bool cancelIdle( TileMapIdleListener* client, uint32 id );

   /**
    *    Creates a new periodic timer with the supplied listener and prio.
    */
   TileMapPeriodicTimer* createPeriodicTimer( TileMapTimerListener* listen,
                                              TileMapToolkit::priority_t p);

   uint32 availableMemory( uint32& largestBlock ) const {
      largestBlock = MAX_UINT32;
      return largestBlock;
   }
       
   /**
    *
    */
   const char* getIDString() const {
      return "gtk";
   }
      
private:

   typedef struct CallbackInfo {
      /**
       * current drawing area
       */
      GtkTileMapToolkit* area;

      /**
       * id of either timer or idle depending on context
       */
      uint32 context_id;
   };

   static gboolean idleCallback(CallbackInfo* info);
   static gboolean timeoutCallback(CallbackInfo* info);

   typedef std::map<uint32, TileMapTimerListener*> TimerMap_t;
   /**
    *    Tajmers
    */
   TimerMap_t m_timers;

   /**
    *    Idles.
    */
   std::map<uint32, TileMapIdleListener*>  m_idles;

   /// Id of last timer.
   int m_lastTimer;
   /// Id of last idle
   int m_lastIdle;

};


#endif 
