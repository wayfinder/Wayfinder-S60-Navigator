/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapMover.h"
#include "DirectedPolygon.h"
#include "ScreenOrWorldCoordinate.h"
#include "CursorHolder.h"
#include "MapMovingInterface.h"

#include <w32std.h>
#include <eikenv.h>

MapMover::MapMover( TileMapKeyHandler* keyHandler,
                    MapMovingInterface* mapHandler,
                    DirectedPolygon* gpsFeature,
                    const CursorHolder* cursorHolder ) :
            m_nonTrackingPoint( 0, 0 ),
            m_trackingPoint( 0, 0 )
{
   m_keyHandler = keyHandler;
   m_mapHandler = mapHandler;
   if ( gpsFeature ) {
      m_gpsFeatureList.push_back( gpsFeature );
   }
   m_cursorHolder = cursorHolder;
   // FIXME: Init other members.
   m_trackingOn = false;
}

MC2Coordinate 
MapMover::getCoordinate() const
{
   MC2Coordinate mc2Coord;
   MC2Point mc2Point = getPoint();
   m_mapHandler->inverseTransform( mc2Coord, mc2Point );
   return mc2Coord;
}

MC2Point
MapMover::getPoint() const
{
   // Should return the active point on the map and not the screen.
   MC2Point mc2Point(0,0);
   if( m_keyHandler ){
      mc2Point = m_keyHandler->getCursorPosInMap();
   }
   else{
      // Wrong, the keyhandler should not be NULL.
      mc2Point = m_nonTrackingPoint;
   }
   return mc2Point;
}

int
MapMover::getScale() const 
{
   return int( m_mapHandler->getScale() + 0.5 );
}

void
MapMover::updatePositions( bool interpolating )
{
   if ( m_trackingOn ) {
      
      /** 
       *   If we are interpolating, assume static scale.
       *
       *   TODO: Use a more intelligent scheme
       */
      
      if( interpolating ) {
         m_mapHandler->setScale( 2.0f );         
      } else {
         m_mapHandler->setScale( m_lastScale );         
      }
      
      // Rotation angle.
      int angle = m_lastDirection; 
      if ( m_trackingNorthUp ) {
         // North up means angle should be 0.
         angle = 0;
      }
      
      m_mapHandler->setPoint( m_lastGpsCoord, m_trackingPoint, angle );
      m_movementSinceLastGetInfo = true;
   }

   // Update gps feature position.
   if( !m_gpsFeatureList.empty() ) {
      for( uint16 i = 0; i < m_gpsFeatureList.size(); ++i ) {

         if( interpolating ) {
            CWsScreenDevice* screenDevice = CEikonEnv::Static()->ScreenDevice();
            TPixelsTwipsAndRotation aSizeAndRotation;
            screenDevice->GetDefaultScreenSizeAndRotation( aSizeAndRotation );
            unsigned int x = aSizeAndRotation.iPixelSize.iWidth / 2;
            unsigned int y =
               static_cast<unsigned int>(aSizeAndRotation.iPixelSize.iHeight *
                                         0.94 );
            m_gpsFeatureList[i]->setCenter( MC2Point( x , y ) );
         
         } else {
            m_gpsFeatureList[i]->setCenter( m_lastGpsCoord );
         }
         
         m_gpsFeatureList[i]->setAngle( (float) m_lastDirection );
      }
   }
}
#define DIFF_IN_PIXELS_FOR_MAP_UPDATE 2

int
MapMover::roundDirectionAngle( int direction )
{
   // Round the direction.
   static const int roundFactor = 6;
   int addToDir = roundFactor / 2;
   if ( direction < 0 ) {
      addToDir = -roundFactor / 2;
   }
   
   return ( ( direction + addToDir ) / roundFactor ) * roundFactor;
}

bool
MapMover::setGpsPos( const MC2Coordinate& coord,
                     int direction,
                     int scale,
                     bool interpolated )
{

   // Round the angle.
   direction = roundDirectionAngle( direction ); 

   bool lastCoordValid = m_lastGpsCoord.isValid();
   bool updatePos = false;


   /**
    *   If we are interpolating, then we do not want to
    *   perform the logic for screen diffs, i.e. we always
    *   want to update when there is a new position available.
    */
   
   if ( !interpolated && lastCoordValid ) {
      // Need to check if to upate
      if ( ( scale != getScale() ) ||
           ( m_lastDirection != direction ) ) {
         updatePos = true;
      } else if( !m_gpsFeatureList.empty() ) {

         // Check if the new position is far enough from the current one
         // in pixels for an update.

         ScreenOrWorldCoordinate c = m_gpsFeatureList.front()->getCenter();
         MC2Point curCenter = c.getScreenPoint( *m_mapHandler );

         MC2Point p2(0,0);

         m_mapHandler->transform( p2, coord );

         int xDiff = curCenter.getX() - p2.getX();
         int yDiff = curCenter.getY() - p2.getY();
         if ( ( xDiff*xDiff + yDiff*yDiff ) > 
               DIFF_IN_PIXELS_FOR_MAP_UPDATE * DIFF_IN_PIXELS_FOR_MAP_UPDATE ) {
            // Update position.
            updatePos = true;
         }  
      }
   } else {
      // Last coord was invalid
      updatePos = true;
   }
     
   if ( updatePos ) {
      m_lastScale = scale;
      m_lastGpsCoord = coord;
      m_lastDirection = direction;
      if ( ! lastCoordValid ) {
         // Update stuff.
         statusChanged( m_trackingPoint, 
                        m_nonTrackingPoint, 
                        m_trackingOn,
                        m_trackingNorthUp,
                        interpolated );
      } else {
         updatePositions( interpolated );
      }
   }

   // Return if a redraw is needed.
   return updatePos; 
}
      
const char* 
MapMover::getInfo( bool onlyIfMapDidntMoveSinceLastTime )
{
   if ( !onlyIfMapDidntMoveSinceLastTime || !m_movementSinceLastGetInfo ) {
      m_movementSinceLastGetInfo = false;
      
      const ClickInfo& clickInfo = m_keyHandler->getInfoForFeatureAt(
                        getPoint(),
                        false ); // only POIs
      bool empty = strcmp( clickInfo.getName(), "" ) == 0;
      return empty ? NULL : clickInfo.getName();
   }
   return NULL;
}

void 
MapMover::setTrackingPoint( const MC2Point& p )
{
   m_trackingPoint = p;
}

void 
MapMover::setTracking( bool trackingOn, bool northUp )
{
   m_trackingOn = trackingOn;
   m_trackingNorthUp = northUp;

   if( m_keyHandler ){
      if ( m_trackingOn ) {
         // Tracking enabled.
         m_keyHandler->setCursorPos( m_trackingPoint ); 
      } else {
         // Tracking disabled.
         // Move the cursor to the point.
         m_keyHandler->setCursorPos( m_nonTrackingPoint );
      }
   }
}

void
MapMover::setNonTrackingPoint( const MC2Point& p ) 
{
   m_nonTrackingPoint = p;
}

void 
MapMover::statusChanged( const MC2Point& trackingPoint,
                         const MC2Point& nonTrackingPoint,
                         bool trackingOn, 
                         bool northUp,
                         bool interpolated )
{
   // Set the members.
   setTrackingPoint( trackingPoint );
   setNonTrackingPoint( nonTrackingPoint );
   m_trackingOn = trackingOn;
   m_trackingNorthUp = northUp;

   bool reallyTrackingOn = m_trackingOn && gotValidGpsCoord();

   // When we're tracking we are not moving (with the keys).
   // So we set moving to not tracking.
   m_mapHandler->setMovementMode( !reallyTrackingOn );
   if( m_keyHandler ){
      if ( reallyTrackingOn ) {
         // Tracking enabled.
         m_keyHandler->setCursorPos( m_trackingPoint ); 
      } else {
         // Tracking disabled.
         // Move the cursor to the point.
         m_keyHandler->setCursorPos( m_nonTrackingPoint );
         if ( m_trackingNorthUp ) {
            // North up means angle should be 0.
            m_mapHandler->setAngle( 0.0f );
         }
      }
   }
 
   if ( reallyTrackingOn ) {
      updatePositions( interpolated );
   }

   // Update cursor visibility.
   bool cursorVisible = m_cursorHolder->getCursorVisibility();
   if( m_keyHandler ){
      m_keyHandler->setCursorVisible( cursorVisible );
   }
}

void
MapMover::setGpsFeature( DirectedPolygon* gpsFeature ) 
{
   if( gpsFeature ) {
      m_gpsFeatureList.push_back( gpsFeature );
   }
}

