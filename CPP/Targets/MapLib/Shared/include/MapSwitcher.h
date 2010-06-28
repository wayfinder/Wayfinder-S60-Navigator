/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MAP_SWITCHER_H
#define MAP_SWITCHER_H

#include "config.h"
#include <vector>
#include <vector>
#include <map>

#include "MapMovingInterface.h"
#include "MapDrawingInterface.h"

class VisibilityAdapterBase;
class MapRectInterface;

/**
 *    Class that helps switching between different type of maps,
 *    for instance between vector maps and globe.
 */
class MapSwitcher : public MapMovingInterface, public MapDrawingInterface
{
   public:

      /**
       *    Switcher notice. Contains information about the
       *    map view.
       */
      struct SwitcherNotice
      {
         /**
          *    The maximum scale that the component should be active at.
          */
         int m_scale;

         /**
          *    The map mover.
          */
         MapMovingInterface* m_mapMover;

         /**
          *    The map drawer.
          */
         MapDrawingInterface* m_mapDrawer;

         /**
          *    The associated components that should be shown when this
          *    component is active.
          */
         std::vector<VisibilityAdapterBase*> m_visibles;


         /**
          *    Set visibility for all associated components.
          */
         void setVisible( bool visible );

         /**
          *
          */
         bool m_hasStaticCursor;

         /**
          *    The map rectangle.
          */
         const MapRectInterface* m_mapRect;

      };

      /**
       *    Constructor.
       *    Supply the SwitcherNotices.
       */
      MapSwitcher( const std::vector<SwitcherNotice>& notices,
                   int scale );
     
      /**
       *    To be called when the size has changed. 
       */
      void updateSize();

      /**
       *    Updates the visibility.
       */
      void forceVisibilityUpdate();

      /**
       *    Sets the cursor handler.
       */
      void setCursorHandler( class TileMapCursorInterface* cursorHandler );

     // --- Implementation of MapMovingInterface.

     /**
       *   Called by the keyhandler when it is moving
       *   the map because of a key being held pressed.
       *   Will be called with the value false when
       *   movement ends. <br />
       *   Can be used to draw the map quicker and uglier
       *   when moving and then drawing a nice one when it
       *   stops.
       *   @param moving True if moving, false if not.
       */
      inline void setMovementMode( bool moving );

      /**
       *   Returns a reference to info for the feature at the specified
       *   position on the screen.
       *   @param point    The point on the screen where
       *                   the info should be found.
       *   @param onlyPois True if only poi info should be returned.
       */
      inline const ClickInfo&
      getInfoForFeatureAt( const MC2Point& point,
                           bool onlyPois,
                           TileMapInfoCallback* infoCallback = NULL );
      
      /**
       *   Returns a reference to the center coordinate.
       */
      inline const MC2Coordinate& getCenter() const;

      /**
       *   Transforms a world coordinate to a screen coordinate.
       */
      inline void transform( MC2Point& point,
                              const MC2Coordinate& coord ) const;
      
      /**
       *   Transforms the point on the screen to a world
       *   coordinate.
       */
      inline void inverseTransform( MC2Coordinate& coord,
                                     const MC2Point& point ) const;

      /**
       *   Sets the center coordinate to newCenter.
       */
      inline void setCenter(const MC2Coordinate& newCenter);
      
      /**
       *   Sets the specified point on the screen to the
       *   specified coordinate.
       *   @param newCoord    The new coordinate to move the specified
       *                      point to.
       *   @param screenPoint The point on the screen to set to the
       *                      specified coordinate.
       */
      inline void setPoint(const MC2Coordinate& newCoord,
                            const MC2Point& screenPoint );

      /**
       *   Sets the specified point on the screen to the
       *   specified coordinate and rotate around that point.
       *
       *   This method call result in the same thing as below, 
       *   but more efficiently implemented:
       *
       *   setPoint( newCoord, screenPoint );
       *   setAngle( angleDegrees, screenPoint );
       *   
       *   
       *   @param newCoord    The new coordinate to move the specified
       *                      point to.
       *   @param screenPoint The point on the screen to set to the
       *                      specified coordinate.
       *   @param angle       The angle in degrees to rotate around 
       *                      the screenPoint.
       */
      inline void setPoint(const MC2Coordinate& newCoord,
                           const MC2Point& screenPoint, 
                           double angleDegrees );
      /**
       *   Moves the screen. Unit pixels.
       */
      inline void move(int deltaX, int deltaY);
      
      /**
       *   Sets the angle to the number of degrees in the
       *   parameter. Rotates around the center.
       */
      inline void setAngle(double angleDegrees);

      /**
       *   Sets the angle to the number of degrees in the
       *   parameter.
       *   @param angleDegrees  Angle in degrees.
       *   @param rotationPoint Point to rotate around.
       */
      inline void setAngle( double angleDegrees,
                             const MC2Point& rotationPoint );

      /**
       *   Rotates the display the supplied number of degrees ccw.
       */
      inline void rotateLeftDeg( int nbrDeg );
      
      /**
       *    Returns the current angle in degrees.
       */
      inline double getAngle() const;

      /**
       *   Sets scale in meters per pixel.
       *   @param scale New scale.
       *   @return The scale set.
       */
      inline double setScale(double scale);

      /**
       *   Returns the current scale in meters per pixel.
       */
      inline double getScale() const;

      /**
       *   Zooms the display. Value larger than one means zoom in.
       *   Corresponds to setScale( factor * getScale() )
       */
      inline double zoom(double factor);
      
      /**
       *    Zooms in so that a specific point at the screen is located at a 
       *    a certain coordinate.
       *    
       *    @param   factor      The factor to zoom in. 
       *                         Value larger than one means zoom in.
       *    @param   zoomCoord   The coordinate to zoom in to.
       *    @param   zoomPoint   The point on the screen where the zoomCoord
       *                         should be located.
       */
      inline double zoom( double factor, 
                           const MC2Coordinate& zoomCoord,
                           const MC2Point& zoomPoint );

      /**
       *   Zooms the display to the supplied corners.
       */
      inline void setPixelBox( const MC2Point& oneCorner,
                                const MC2Point& otherCorner );
      
      /**
       *   Zooms the display to the supplied world box.
       */
      inline void setWorldBox( const MC2Coordinate& oneCorner,
                               const MC2Coordinate& otherCorner );

      /**
       *    Get the next point that contains a feature that could be
       *    highlighted.
       *    @param   point [IN] The point to be set to the next highlightable
       *                   feature.
       *    @return  True if a highlightable feature was found and thus
       *             if the point was updated. False otherwise.
       */
      inline bool getNextHighlightPoint( MC2Point& point );

      /**
       *    Shows highlight for last info point if visible is true.
       */
      inline void setHighlight( bool visible );
  

      /**
       *    If the screen can be handled as a bitmap. I.e.
       *    if the methods moveBitmap, setPointBitmap, setBitmapDragPoint
       *    zoomBitmapAtPoint and zoomBitmapAtCenter are properly 
       *    implemented.
       */
      inline bool canHandleScreenAsBitmap() const;

      /**
       *    Move the map as a bitmap. The deltas corresponds to in which
       *    direction the map should be moved, i.e. it should be the 
       *    same values as used when calling the move() method.
       */
      inline void moveBitmap(int deltaX, int deltaY);

      /**
       *    Set the point when moving the screen as a bitmap.
       *    setBitmapDragPoint must have first been called to know
       *    which point on the bitmap that should be moved.
       */
      inline void setPointBitmap( const MC2Point& screenPoint );

      /**
       *    Set the bitmap drag point. This must be set before the
       *    setPointBitmap can be called.
       */
      inline void setBitmapDragPoint( const MC2Point& dragPoint );

      /**
       *    Zoom the map as a bitmap at a specific point.
       */
      inline void zoomBitmapAtPoint( double factor, 
                                     const MC2Point& screenPoint );

      /**
       *    Zoom the map as a bitmap at the center.
       */
      inline void zoomBitmapAtCenter( double factor );
      
   
      // --- Implementation of MapDrawingInterface.
      
         
      /**
       *    Requests that the display should be repainted now.
       */
      inline void repaintNow();

      /**
       *    Requests that the display should be repainted when
       *    convinient.
       */
      inline void requestRepaint();
   
   private:
      /**
       *    Check if it's time to switch to an other type of map.
       *    @return  If the map needs repainting.
       */
      bool update();
   
      /**
       *    Map switcher iterator.
       */
      typedef std::map<int, SwitcherNotice>::iterator switcher_it;
     
      /**
       *    Switcher notices, with scale as key.
       */
      std::map<int, SwitcherNotice> m_notices;

      /**
       *    The currently active MapMovingInterface.
       */
      MapMovingInterface* m_mover;
      
      /**
       *    The currently active MapMovingInterface.
       */
      MapDrawingInterface* m_drawer;

      /**
       *    The cursor handler.
       */
      class TileMapCursorInterface* m_cursorHandler;

      /**
       *    If the currently active switcher has a static cursor or not.
       */
      bool m_currentSwitcherHasStaticCursor;
      
};

// --- Implementation of inlined methods.

inline void 
MapSwitcher::setMovementMode( bool moving )
{
   m_mover->setMovementMode( moving );
}

inline const ClickInfo&
MapSwitcher::getInfoForFeatureAt( const MC2Point& point,
                                  bool onlyPois,
                                  TileMapInfoCallback* infoCallback )
{
   return m_mover->getInfoForFeatureAt( point, onlyPois, infoCallback );
}

inline const MC2Coordinate& 
MapSwitcher::getCenter() const
{
   return m_mover->getCenter();
}

inline void 
MapSwitcher::transform( MC2Point& point,
                        const MC2Coordinate& coord ) const
{
   m_mover->transform( point, coord );
}

inline void 
MapSwitcher::inverseTransform( MC2Coordinate& coord,
                            const MC2Point& point ) const
{
   m_mover->inverseTransform( coord, point );

}

inline void 
MapSwitcher::setCenter(const MC2Coordinate& newCenter)
{
   m_mover->setCenter( newCenter );
}

inline void 
MapSwitcher::setPoint(const MC2Coordinate& newCoord,
                   const MC2Point& screenPoint )
{
   m_mover->setPoint( newCoord, screenPoint );
}

inline void 
MapSwitcher::setPoint(const MC2Coordinate& newCoord,
                      const MC2Point& screenPoint,
                      double angleDegrees )
{
   m_mover->setPoint( newCoord, screenPoint, angleDegrees );
}
inline void 
MapSwitcher::move(int deltaX, int deltaY)
{
   m_mover->move( deltaX, deltaY );
}

inline void 
MapSwitcher::setAngle(double angleDegrees)
{
   m_mover->setAngle( angleDegrees );
}

inline void 
MapSwitcher::setAngle( double angleDegrees,
                    const MC2Point& rotationPoint )
{
   m_mover->setAngle( angleDegrees, rotationPoint );
}

inline void 
MapSwitcher::rotateLeftDeg( int nbrDeg )
{
   m_mover->rotateLeftDeg( nbrDeg );
}

inline double 
MapSwitcher::getAngle() const
{
   return m_mover->getAngle();
}

inline double 
MapSwitcher::setScale(double scale)
{
   double retVal = m_mover->setScale( scale );
   if ( update() ) {
      // Switched mapmoving interface. Set the scale again
      // on the right interface.
      retVal = m_mover->setScale( scale );
      repaintNow();
   }
   return retVal;
}

inline double 
MapSwitcher::getScale() const
{
   return m_mover->getScale();
}

inline double 
MapSwitcher::zoom(double factor)
{
   double retVal = m_mover->zoom( factor );
   if ( update() ) {
      repaintNow();
   }
   return retVal;
}

inline double 
MapSwitcher::zoom( double factor, 
                  const MC2Coordinate& zoomCoord,
                  const MC2Point& zoomPoint )
{
   double retVal = m_mover->zoom( factor, zoomCoord, zoomPoint );
   if ( update() ) {
      repaintNow();
   }
   return retVal;
}

inline void 
MapSwitcher::setPixelBox( const MC2Point& oneCorner,
                       const MC2Point& otherCorner )
{
   m_mover->setPixelBox( oneCorner, otherCorner );
   if ( update() ) {
      // Switched mapmoving interface. Set the pixel box again
      // on the right interface.
      m_mover->setPixelBox( oneCorner, otherCorner );
      repaintNow();
   }
}

inline void 
MapSwitcher::setWorldBox( const MC2Coordinate& oneCorner,
                          const MC2Coordinate& otherCorner )
{
   m_mover->setWorldBox( oneCorner, otherCorner );
   if ( update() ) {
      // Switched mapmoving interface. Set the world box again
      // on the right interface.
      m_mover->setWorldBox( oneCorner, otherCorner );
      repaintNow();
   }
}

inline bool 
MapSwitcher::getNextHighlightPoint( MC2Point& point )
{
   return m_mover->getNextHighlightPoint( point );
}

inline void 
MapSwitcher::setHighlight( bool visible )
{
   m_mover->setHighlight( visible );
}

inline bool 
MapSwitcher::canHandleScreenAsBitmap() const
{
   return m_mover->canHandleScreenAsBitmap();
}
      
inline void 
MapSwitcher::moveBitmap(int deltaX, int deltaY)
{
   m_mover->moveBitmap( deltaX, deltaY );
}
inline void 
MapSwitcher::setPointBitmap( const MC2Point& screenPoint )
{
   m_mover->setPointBitmap( screenPoint );
}

inline void 
MapSwitcher::setBitmapDragPoint( const MC2Point& dragPoint )
{
   m_mover->setBitmapDragPoint( dragPoint );
}

inline void 
MapSwitcher::zoomBitmapAtPoint( double factor, 
                                const MC2Point& screenPoint )
{
   m_mover->zoomBitmapAtPoint( factor, screenPoint );
}
   
inline void 
MapSwitcher::zoomBitmapAtCenter( double factor )
{
   m_mover->zoomBitmapAtCenter( factor );
}

inline void 
MapSwitcher::repaintNow()
{
   m_drawer->repaintNow();
}

inline void 
MapSwitcher::requestRepaint()
{
   m_drawer->requestRepaint();
}

#endif
