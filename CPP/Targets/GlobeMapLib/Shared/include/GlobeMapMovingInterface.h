/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GLOBEMAPMOVINGINTERFACECONTROL_H
#define GLOBEMAPMOVINGINTERFACECONTROL_H

#include "config.h"
#include "MapMovingInterface.h"
#include "MC2Coordinate.h"
#include "MC2Point.h"

class GlobeClickInfo;
struct GBGlobe_s;
typedef struct GBGlobe_s GBGlobe;

/**
 *    The minimum scale level for the globe.
 */
#define GB_MIN_SCALE 12000.0f

/**
 *    Globe's MapMovingInterface.
 */
class GlobeMapMovingInterface : public MapMovingInterface
{
public:
   /**
    *    Constructor.
    */
   GlobeMapMovingInterface();

   /**
    *    Destructor.
    */
   virtual ~GlobeMapMovingInterface();
 
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
   void setMovementMode( bool moving );

   /**
    *   Returns a reference to info for the feature at the specified
    *   position on the screen.
    *   @param point    The point on the screen where
    *                   the info should be found.
    *   @param onlyPois True if only poi info should be returned.
    */
   const ClickInfo&
   getInfoForFeatureAt( const MC2Point& point,
                        bool onlyPois,
                        TileMapInfoCallback* infoCallback = NULL );
   
   /**
    *   Returns a reference to the center coordinate.
    */
   const MC2Coordinate& getCenter() const;

   /**
    *   Transforms a world coordinate to a screen coordinate.
    */
   void transform( MC2Point& point,
                   const MC2Coordinate& coord ) const;
   
   /**
    *   Transforms the point on the screen to a world
    *   coordinate.
    */
   void inverseTransform( MC2Coordinate& coord,
                          const MC2Point& point ) const;

   /**
    *   Sets the center coordinate to newCenter.
    */
   void setCenter(const MC2Coordinate& newCenter);
   
   /**
    *   Sets the specified point on the screen to the
    *   specified coordinate.
    *   @param newCoord    The new coordinate to move the specified
    *                      point to.
    *   @param screenPoint The point on the screen to set to the
    *                      specified coordinate.
    */
   void setPoint(const MC2Coordinate& newCoord,
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
   void setPoint(const MC2Coordinate& newCoord,
                 const MC2Point& screenPoint, 
                 double angleDegrees );

   /**
    *   Moves the screen. Unit pixels.
    */
   void move(int deltaX, int deltaY);
   
   /**
    *   Sets the angle to the number of degrees in the
    *   parameter. Rotates around the center.
    */
   void setAngle(double angleDegrees);

   /**
    *   Sets the angle to the number of degrees in the
    *   parameter.
    *   @param angleDegrees  Angle in degrees.
    *   @param rotationPoint Point to rotate around.
    */
   void setAngle( double angleDegrees,
                  const MC2Point& rotationPoint );

   /**
    *   Rotates the display the supplied number of degrees ccw.
    */
   void rotateLeftDeg( int nbrDeg );
   
   /**
    *    Returns the current angle in degrees.
    */
   double getAngle() const;

   /**
    *   Sets scale in meters per pixel.
    *   @param scale New scale.
    *   @return The scale set.
    */
   double setScale(double scale);

   /**
    *   Returns the current scale in meters per pixel.
    */
   double getScale() const;

   /**
    *   Zooms the display. Value larger than one means zoom in.
    *   Corresponds to setScale( factor * getScale() )
    */
   double zoom(double factor);
   
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
   double zoom( double factor, 
                const MC2Coordinate& zoomCoord,
                const MC2Point& zoomPoint );

   /**
    *   Zooms the display to the supplied corners.
    */
   void setPixelBox( const MC2Point& oneCorner,
                     const MC2Point& otherCorner );

   /**
    *   Zooms the display to the supplied world box.
    */
   void setWorldBox( const MC2Coordinate& oneCorner,
                     const MC2Coordinate& otherCorner );
   /**
    *    Get the next point that contains a feature that could be
    *    highlighted.
    *    @param   point [IN] The point to be set to the next highlightable
    *                   feature.
    *    @return  True if a highlightable feature was found and thus
    *             if the point was updated. False otherwise.
    */
   bool getNextHighlightPoint( MC2Point& point );

   /**
    *    Shows highlight for last info point if visible is true.
    */
   void setHighlight( bool visible );
   
   void setGlobe( GBGlobe* globe );
	
   bool isMoved() const;
   void clearMoved();
   void setMoved( bool moved );
   
   float m_zoomTweak;
   
//	double x, y;

   /**
    *    Set the top left position of the globe relative to the control.
    */
   void setTopLeftGlobePos( const MC2Point& globePos );
   
protected:
   
   /**
    *    The top left position of the globe relative to the control.
    */
   MC2Point m_topLeftGlobePos;
   
   /**
    *    Convert between screen point to globe point.
    */
   MC2Point screenToGlobe( const MC2Point& screenPoint ) const;
   
   /**
    *    Convert between globe point to screen point.
    */
   MC2Point globeToScreen( const MC2Point& globePoint ) const;

   /**
    *    The center coordinate.
    */
   MC2Coordinate m_center;

   /**
    *    The scale.
    */
   double m_scale;

   /**
    *    Globe click info to return in getInfoForFeatureAt().
    */
   GlobeClickInfo* m_clickInfo;

   GBGlobe* m_globe;

	bool m_moved;
};

#endif

