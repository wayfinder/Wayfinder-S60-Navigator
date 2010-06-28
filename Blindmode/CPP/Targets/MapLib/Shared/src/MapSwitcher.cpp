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
#include "MapSwitcher.h"
#include "VisibilityAdapterBase.h"

#include "MapMovingInterface.h"
#include "MapDrawingInterface.h"

#include "TileMapKeyHandler.h"
#include "MapRectInterface.h"

using namespace std;

void
MapSwitcher::SwitcherNotice::setVisible( bool visible ) 
{
   for ( vector<VisibilityAdapterBase*>::iterator vIt = 
         m_visibles.begin(); vIt != m_visibles.end();
         ++vIt ) {
      (*vIt)->setMainControlVisible( visible );
   }
}

MapSwitcher::MapSwitcher( const vector<SwitcherNotice>& notices,
                          int scale )
{
   m_mover = NULL;
   // Add them to the m_notices map.
   for ( uint32 i = 0; i < notices.size(); ++i ) {
      m_notices.insert( make_pair( notices[ i ].m_scale, notices[ i ] ) );
   }

   // Activate the notice that should be used.
   map<int, SwitcherNotice>::iterator it = 
      m_notices.upper_bound( scale );
   MC2_ASSERT( it != m_notices.end() );
   
   // Initialize the notices.
   m_mover = it->second.m_mapMover;
   m_drawer = it->second.m_mapDrawer;
   m_mover->setScale( scale );
   m_currentSwitcherHasStaticCursor = it->second.m_hasStaticCursor;

   forceVisibilityUpdate();
}

void
MapSwitcher::forceVisibilityUpdate()
{
   for ( map<int, SwitcherNotice>::iterator it = m_notices.begin();
         it != m_notices.end(); ++it ) {
      if ( it->second.m_mapMover == m_mover ) {
         // Show.
         it->second.setVisible( true );
      } else {
         // Hide.
         it->second.setVisible( false );
      }
   }
}

void
MapSwitcher::updateSize()
{
   for ( map<int, SwitcherNotice>::iterator it = m_notices.begin();
         it != m_notices.end(); ++it ) {
      if ( it->second.m_mapMover == m_mover ) {
         // Update the map box.
         m_cursorHandler->setMapBox( it->second.m_mapRect->getMapRect() );
      }
   }
}

void
MapSwitcher::setCursorHandler( TileMapCursorInterface* cursorHandler )
{
   m_cursorHandler = cursorHandler;
   if (m_cursorHandler) {
      m_cursorHandler->setStaticCursor(m_currentSwitcherHasStaticCursor);
      // Update the size also.
      updateSize();
   }
}

bool 
MapSwitcher::update()
{
   map<int, SwitcherNotice>::iterator it = 
      m_notices.upper_bound( (int) m_mover->getScale() );
   MC2_ASSERT( it != m_notices.end() );
   if ( it->second.m_mapMover == m_mover ) {
      return false;
   } 

   // Necessary to switch.
   
   SwitcherNotice& nextSwitcher = (*it).second;

   // Find the old.
   map<int, SwitcherNotice>::iterator prevIt = m_notices.begin();
   for ( prevIt = m_notices.begin(); prevIt != m_notices.end(); 
         ++prevIt ) {
      if ( prevIt->second.m_mapMover == m_mover ) {
         break;
      }
   }
   MC2_ASSERT( prevIt != m_notices.end() );
   SwitcherNotice& prevSwitcher = (*prevIt).second;
   
   m_currentSwitcherHasStaticCursor = nextSwitcher.m_hasStaticCursor;
   nextSwitcher.m_mapMover->setScale( 
         prevSwitcher.m_mapMover->getScale() );
   nextSwitcher.m_mapMover->setAngle( 
         prevSwitcher.m_mapMover->getAngle() );

   if (m_cursorHandler) {
      if (!prevSwitcher.m_hasStaticCursor && nextSwitcher.m_hasStaticCursor) {
         // From movable cursor to static cursor (vector -> globe).
         m_cursorHandler->centerMapAtCursor();
         m_cursorHandler->setMapBox( nextSwitcher.m_mapRect->getMapRect() );
         m_cursorHandler->centerCursor();
         m_cursorHandler->setStaticCursor(true);
      } else if (!nextSwitcher.m_hasStaticCursor) {
         m_cursorHandler->resetZoomCoord();
         m_cursorHandler->setMapBox( nextSwitcher.m_mapRect->getMapRect() );
         m_cursorHandler->setStaticCursor(false);
         m_cursorHandler->centerCursor();
      }
   }

   nextSwitcher.m_mapMover->setCenter( 
         prevSwitcher.m_mapMover->getCenter() );
   
   // Perform the switch.
   m_mover = nextSwitcher.m_mapMover;
   m_drawer = nextSwitcher.m_mapDrawer;
   
   // Update visibility
   // Hide the old.
   prevSwitcher.setVisible( false );

   // Show the new.
   nextSwitcher.setVisible( true );

   // Needs repaint.
   return true;
}

