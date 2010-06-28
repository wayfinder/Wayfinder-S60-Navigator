/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_MAP_EVENT_H
#define TILE_MAP_EVENT_H

class TileMapEvent {
public:
   /// Types of events
   enum event_t {
      /// New categories have been downloaded from server/cache
      NEW_CATEGORIES_AVAILABLE = 1,
      /// New caches have been loaded
      UPDATE_CACHE_INFO        = 2,
      /// User defined features have been redrawn.
      USER_DEFINED_FEATURES_REDRAWN = 3,
   };

   /// Creates new event with specified type.
   inline TileMapEvent( event_t type );

   /// Returns the type of the event.
   inline event_t getType() const;
private:
   event_t m_type;
};

// -- Inlined functions for the TileMapEvent

inline
TileMapEvent::TileMapEvent( event_t type )
      : m_type( type )
{
}

inline TileMapEvent::event_t
TileMapEvent::getType() const
{
   return m_type;
}

#endif
