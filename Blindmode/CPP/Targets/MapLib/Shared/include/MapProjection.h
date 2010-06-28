/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MAP_PROJECTION_H
#define MAP_PROJECTION_H

// This is due to the fact that TileMapConfig contains the right
// ifdefs
#include "TileMapConfig.h"
#include "TransformMatrix.h"
#include "MC2BoundingBox.h"
#include "MC2Point.h"

#include <vector>
#include <utility>

/**
 *   Class that describes a projection of a map onto a screen.
 *   Handles zooming, rotation etc.
 */
class MapProjection : public TransformMatrix {
public:

   /**
    *   Creates new projection in Lund.
    */
   MapProjection();
   
   /**
    *   Creates a new projection.
    */
   MapProjection(const MC2Coordinate& centerCoord,
                 int screenX, int screenY, double scale,
                 double angle = 0);

   /**
    *   Destructor.
    */
   virtual ~MapProjection() {}

   /**
    *   Sets the screen size.
    */
   void setScreenSize(const MC2Point& size);
   
   /**
    *   Zooms the display. Value larger than one means zoom in.
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
    *   Does not work for angles other than 0 for the moment.
    */
   void setPixelBox( const MC2Point& oneCorner,
                     const MC2Point& otherCorner );

   /**
    *   Sets scale in meters per pixel.
    *   @param scale New scale.
    *   @return The scale set.
    */
   double setPixelScale(double scale);

   /**
    *   Returns the current scale in meters per pixel.
    */
   double getPixelScale() const;

   /**
    *    Set the DPI correction factor. The factor 1 corresponds
    *    a value suitable for old phone models, such as Nokia 6600
    *    and similar. For new devices with displays with higher
    *    DPI, the factor should be correspond with how many times
    *    greater the DPI is compared to the reference display
    *    (e.g. Nokia 6600).
    */
   void setDPICorrectionFactor( uint32 factor );
   
   /**
    *    Get the DPI correction factor. The factor 1 corresponds
    *    a value suitable for old phone models, such as Nokia 6600
    *    and similar. For new devices with displays with higher
    *    DPI, the factor should be correspond with how many times
    *    greater the DPI is compared to the reference display
    *    (e.g. Nokia 6600).
    */
   uint32 getDPICorrectionFactor() const;
   
   /**
    *    Set the scale corrected with the DPI correction factor.
    */
   double setDPICorrectedScale( double scale );
   
   /**
    *    Get the scale corrected with the DPI correction factor.
    */
   double getDPICorrectedScale() const;
   
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
    *   Moves the display 
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
    *    Returns the current angle.
    */
   double getAngle() const;
   
   /**
    *   Returns the screensize of the projection.
    */
   inline const MC2Point& getScreenSize() const;

   /**
    *   Returns a reference to the center coordinate.
    */
   inline const MC2Coordinate& getCenter() const;
   
   /**
    *   Sets new boundingbox.
    *   @param bbox The new bounding box.
    *   @return The bounding box that was really set.
    */
   MC2BoundingBox setBoundingBox(const MC2BoundingBox& bbox);

   /**
    *   Returns the bounding box for the projection.
    */
   inline const MC2BoundingBox& getBoundingBox() const;
   
   /**
    *   Updates the bounding box.
    */
   void updateBBox();
   
   /**
    *   Returns a vector of bboxes to be used for drawing.
    *   Will be many if the world is too wide.
    */
   void getDrawingBBoxes( std::vector<MC2BoundingBox>& outBoxes ) const;
   
   /**
    *   Returns the scale to be used with the given screen size.
    */
   static double getPixelScaleFromBBoxAndSize( int screenXSize, 
                                          int screenYSize,
                                          MC2BoundingBox& bbox );

   /**
    *   Returns the lower left coordinate. Warning!! Calculated.
    *   Should use center instead.
    */
   MC2Coordinate calcLowerLeft() const;

   /**
    *   Returns the width and height of the projection in
    *   mc2 units. Can be larger than the world!
    */
   std::pair<float,float> getWidthAndHeight() const;
   
protected:
   /**
    *   Updates the matrix.
    */
   void updateTransformMatrix();

private:
   /**
    *   Calculated bbox.
    */
   MC2BoundingBox m_bbox;
   
   /**
    *   Center coordinate.
    */
   MC2Coordinate m_centerCoord;

   /**
    *   Last screen size.
    */
   MC2Point m_screenSize;
   
   /**
    *   Current scale.
    */
   double m_scale;

   /**
    *   Current angle in degrees.
    */
   double m_angle;
  
   /**
    *    The DPI correction factor. The factor 1 corresponds
    *    a value suitable for old phone models, such as Nokia 6600
    *    and similar. For new devices with displays with higher
    *    DPI, the factor should be correspond with how many times
    *    greater the DPI is compared to the reference display
    *    (e.g. Nokia 6600).
    */
   uint32 m_dpiCorrectionFactor;
   
};

inline const MC2Point&
MapProjection::getScreenSize() const
{
   return m_screenSize;
}

inline const MC2Coordinate& 
MapProjection::getCenter() const
{
   return m_centerCoord;
}

inline const MC2BoundingBox&
MapProjection::getBoundingBox() const
{
   return m_bbox;
}


#endif
