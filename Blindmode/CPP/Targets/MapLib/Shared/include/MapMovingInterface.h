/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPMOVINGINTERFACE_H
#define MAPMOVINGINTERFACE_H

#include "config.h"

class MC2Coordinate;
class MC2Point;
class TileMapInfoCallback;
class UserDefinedFeature;

/**
 *   Class containing information about a clicked poi / feature.
 */
class ClickInfo {
public:   
   /// Virtual destructor.
   virtual ~ClickInfo() {}
   /// Returns the name of the feature. Must not return NULL.
   virtual const char* getName() const = 0;
   /// Returns true if the highlight should be turned on (change cursor).
   virtual bool shouldHighlight() const = 0;
   
   /**
    *    Get the server string.
    */
   virtual const char* getServerString() const { return NULL; }
   /**
    *    Get the clicked user feature.
    */
   virtual UserDefinedFeature* getClickedUserFeature() const { return NULL; }
   /**
    *    Get the outline feature.
    */
   virtual UserDefinedFeature* getOutlineFeature() const { return NULL; }

   /**
    *    Get the distance to feature.
    */
   virtual int32 getDistance() const { return 0; }
};

/**
 *   Inteface to components that have a moving map.
 */
class MapMovingInterface {
public:

   /**
    *    Virtual destructor.
    */
   virtual ~MapMovingInterface() {}
   
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
   virtual void setMovementMode( bool moving ) = 0;

   /**
    *   Returns a reference to info for the feature at the specified
    *   position on the screen.
    *   @param point    The point on the screen where
    *                   the info should be found.
    *   @param onlyPois True if only poi info should be returned.
    */
   virtual const ClickInfo&
   getInfoForFeatureAt( const MC2Point& point,
                        bool onlyPois,
                        TileMapInfoCallback* infoCallback = NULL ) = 0;
   
   /**
    *   Returns a reference to the center coordinate.
    */
   virtual const MC2Coordinate& getCenter() const = 0;

   /**
    *   Transforms a world coordinate to a screen coordinate.
    */
   virtual void transform( MC2Point& point,
                           const MC2Coordinate& coord ) const = 0;
   
   /**
    *   Transforms the point on the screen to a world
    *   coordinate.
    */
   virtual void inverseTransform( MC2Coordinate& coord,
                                  const MC2Point& point ) const = 0;

   /**
    *   Sets the center coordinate to newCenter.
    */
   virtual void setCenter(const MC2Coordinate& newCenter) = 0;
   
   /**
    *   Sets the specified point on the screen to the
    *   specified coordinate.
    *   @param newCoord    The new coordinate to move the specified
    *                      point to.
    *   @param screenPoint The point on the screen to set to the
    *                      specified coordinate.
    */
   virtual void setPoint(const MC2Coordinate& newCoord,
                         const MC2Point& screenPoint ) = 0;

   /**
    *   Moves the screen. Unit pixels.
    */
   virtual void move(int deltaX, int deltaY) = 0;
   
   /**
    *   Sets the angle to the number of degrees in the
    *   parameter. Rotates around the center.
    */
   virtual void setAngle(double angleDegrees) = 0;

   /**
    *   Sets the angle to the number of degrees in the
    *   parameter.
    *   @param angleDegrees  Angle in degrees.
    *   @param rotationPoint Point to rotate around.
    */
   virtual void setAngle( double angleDegrees,
                          const MC2Point& rotationPoint ) = 0;

   /**
    *   Rotates the display the supplied number of degrees ccw.
    */
   virtual void rotateLeftDeg( int nbrDeg ) = 0;
   
   /**
    *    Returns the current angle in degrees.
    */
   virtual double getAngle() const = 0;

   /**
    *   Sets scale in meters per pixel.
    *   @param scale New scale.
    *   @return The scale set.
    */
   virtual double setScale(double scale) = 0;

   /**
    *   Returns the current scale in meters per pixel.
    */
   virtual double getScale() const = 0;

   /**
    *   Zooms the display. Value larger than one means zoom in.
    *   Corresponds to setScale( factor * getScale() )
    */
   virtual double zoom(double factor) = 0;
   
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
   virtual double zoom( double factor, 
                        const MC2Coordinate& zoomCoord,
                        const MC2Point& zoomPoint ) = 0;

   /**
    *   Zooms the display to the supplied corners.
    */
   virtual void setPixelBox( const MC2Point& oneCorner,
                             const MC2Point& otherCorner ) = 0;

   /**
    *   Zooms the display to the supplied world box.
    */
   virtual void setWorldBox( const MC2Coordinate& oneCorner,
                             const MC2Coordinate& otherCorner ) = 0;

   /**
    *    Get the next point that contains a feature that could be
    *    highlighted.
    *    @param   point [IN] The point to be set to the next highlightable
    *                   feature.
    *    @return  True if a highlightable feature was found and thus
    *             if the point was updated. False otherwise.
    */
   virtual bool getNextHighlightPoint( MC2Point& point ) = 0;

   /**
    *    Shows highlight for last info point if visible is true.
    */
   virtual void setHighlight( bool visible ) = 0;
   
   /**
    *    If the screen can be handled as a bitmap. I.e.
    *    if the methods moveBitmap, setPointBitmap, setBitmapDragPoint
    *    zoomBitmapAtPoint and zoomBitmapAtCenter are properly 
    *    implemented.
    */
   virtual bool canHandleScreenAsBitmap() const { return false; }
   
   /**
    *    Move the map as a bitmap. The deltas corresponds to in which
    *    direction the map should be moved, i.e. it should be the 
    *    same values as used when calling the move() method.
    */
   virtual void moveBitmap(int deltaX, int deltaY) {}

   /**
    *    Set the point when moving the screen as a bitmap.
    *    setBitmapDragPoint must have first been called to know
    *    which point on the bitmap that should be moved.
    */
   virtual void setPointBitmap(const MC2Point& screenPoint ) {}

   /**
    *    Set the bitmap drag point. This must be set before the
    *    setPointBitmap can be called.
    */
   virtual void setBitmapDragPoint( const MC2Point& dragPoint ) {}

   /**
    *    Zoom the map as a bitmap at a specific point.
    */
   virtual void zoomBitmapAtPoint( double factor, 
                                   const MC2Point& screenPoint ) {}

   /**
    *    Zoom the map as a bitmap at the center.
    */
   virtual void zoomBitmapAtCenter( double factor ) {}
   
};

#endif
