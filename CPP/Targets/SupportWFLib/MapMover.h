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

#ifndef MAPMOVER_H
#define MAPMOVER_H

#include "MC2Point.h"
#include "MC2Coordinate.h"
#include "TileMapKeyHandler.h"
#include <vector>

class DirectedPolygon;
class CursorHolder;

/**
 *    Class that can perform certain operations on the map, 
 *    such as movement etc.
 */
class MapMover {
   
   public:

      /**
       *    Constructor.
       *    @param   keyHandler     Pointer to the keyhandler.
       *    @param   mapHandler     Pointer to the tilemaphandler.
       *    @param   gpsFeature     Pointer to the userdefined 
       *                            gps feature.
       *    @param   cursorHolder   Pointer to the cursor holder.
       */
      MapMover( TileMapKeyHandler* keyHandler,
                MapMovingInterface* mapHandler,
                DirectedPolygon* gpsFeature,
                const CursorHolder* cursorHolder );
        
      /**
       *    Get the active coordinate in the map.
       */
      MC2Coordinate getCoordinate() const;

      /**
       *    Get the active screen point in the map.
       */
      MC2Point getPoint() const;
      
      /**
       *    Set a new gps position.
       *
       *    @param   coord       The coordinate of the new gps position.
       *    @param   direction   The angle (360 degrees).
       *    @param   scale       The scalelevel (dependent on the 
       *                         speed).
       *    @return If the map was updated due to the new gps position.
       */
      bool setGpsPos( const MC2Coordinate& coord, 
                      int direction,
                      int scale,
                      bool interpolated );

      /**
       *    Set the tracking point on the screen.
       */
      void setTrackingPoint( const MC2Point& p );

      /**
       *    Set the non tracking point on the screen, i.e. the active point
       *    when going from tracking on to off.
       */
      void setNonTrackingPoint( const MC2Point& p );

      /**
       *    Set the gps feature polygon.
       */
      void setGpsFeature( DirectedPolygon* gpsFeature );
      
      /**
       *    Indicate that the status has changed.
       *
       *    @param   trackingPoint     The tracking point on the screen.
       *    @param   nonTrackingPoint  The non tracking point, i.e. the
       *                               active point when going from
       *                               tracking on to off.
       *    @param   trackingOn        True if tracking is on.
       *    @param   northUp           True if north is up during tracking, 
       *                               or false if rotating mode.
       *    @param   interpolating     True if we are currently interpolating
       *                               positions.
       */
   
      void statusChanged( const MC2Point& trackingPoint,
                          const MC2Point& nonTrackingPoint,
                          bool trackingOn, 
                          bool northUp,
                          bool interpolating );

      /**
       *   Handles the press of a key.
       *   @param key      The key which was pressed.
       *   @param upordown Type of event.
       *   @param info     [OUT] The pointer is set to point at the name
       *                   of the feature that the cursor is pointing at,
       *                   or NULL if there is no such feature (with a name).
       *   @param dragToPoint [Optional] Only necessary to supply if a
       *                      DRAG_TO key event is made. Should contain
       *                      the current position of the pointer device.
       *   @return True if the key was consumed.
       */
      inline bool handleKeyEvent(
                        TileMapKeyHandler::key_t key,
                        TileMapKeyHandler::kind_of_press_t upordown,
                        const char*& info,
                        const MC2Point* dragToPoint = NULL );


      /**
       *   Stops the current keyhandling action.
       */
      inline void stopKeyHandling();

      /**
       *    Get if there's a valid gps coordinate yet.
       */
      inline bool gotValidGpsCoord() const;

      /**
       *    Get info about the feature at the active point on the map.
       *
       *    @param   onlyIfMapDidntMoveSinceLastTime  
       *                [Optional] If only to get info if the map 
       *                didn't move since last time info was requested.
       *                This should be set to true when the call to
       *                getInfo is initiated by 
       *                TileMapInfoCallback::notifyInfoNamesAvailable().
       */
      const char* getInfo( bool onlyIfMapDidntMoveSinceLastTime = false );
      
   private:
      /**
       *    Set tracking mode.
       *    @param   trackingOn        True if tracking is on.
       *    @param   northUp           True if north is up during tracking, 
       */
      void setTracking( bool trackingOn, bool northUp );
      
      /**
       *    Update the positions of the map.
       *
       *    @param      interpolating    True if we are currently interpolating
       *                                 positions.
       */
   
      void updatePositions( bool interpolating );

      /**
       *    Round the direction angle.
       *    @param   direction   The angle in degrees.
       *    @return  The rounded angle in degress.
       */
      static int roundDirectionAngle( int direction );

      /**
       *    Get the scale.
       */
      int getScale() const;

      ///   True if tracking is on.
      bool m_trackingOn;

      ///   True if north is up during tracking, false if rotating mode.
      bool m_trackingNorthUp;

      /**
       *    The non tracking point, i.e. the active point when going from
       *    tracking on to off.
       */
      MC2Point m_nonTrackingPoint;

      ///   The tracking point on the screen.
      MC2Point m_trackingPoint;

      ///   The last scale level of the map.
      int m_lastScale;
      
      ///   The last angle (360 degrees) of the gps.
      int m_lastDirection;
      
      ///   The last gps coordinate.
      MC2Coordinate m_lastGpsCoord;
      
      ///   Pointer to the keyhandler.
      TileMapKeyHandler* m_keyHandler;

      ///   Pointer to the tilemaphandler.
      MapMovingInterface* m_mapHandler;

      ///   Pointer to the userdefined gps feature.
      std::vector<DirectedPolygon*> m_gpsFeatureList;
      
      ///   Pointer to the cursor holder.
      const CursorHolder* m_cursorHolder;

      /// If the map has moved since last call to getInfo(). 
      bool m_movementSinceLastGetInfo;
};

// --- Implementation of inlined methods ---

inline bool
MapMover::handleKeyEvent( TileMapKeyHandler::key_t key,
                          TileMapKeyHandler::kind_of_press_t upordown,
                          const char*& info,
                          const MC2Point* dragToPoint )
{
   return m_keyHandler->handleKeyEvent( key, 
                                        upordown, 
                                        info, 
                                        dragToPoint,
                                        &m_movementSinceLastGetInfo );
}

inline void
MapMover::stopKeyHandling()
{
   m_keyHandler->stop();
}

inline bool
MapMover::gotValidGpsCoord() const 
{
   return m_lastGpsCoord.isValid();
}


#endif
