/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "MapProjection.h"
#include "PixelBox.h"


#define TOP_LAT 912909609
#define BOTTOM_LAT -912909609

using namespace std;

MapProjection::
MapProjection() : TransformMatrix(),
                  m_screenSize(100,100),
                  m_dpiCorrectionFactor( 1 )
{
   m_angle = 0.0;
   MC2Coordinate lower( 664609150, 157263143 );
   MC2Coordinate upper( 664689150, 157405144 );
   MC2BoundingBox bbox(lower, upper);
   
   setBoundingBox(bbox);
}

MapProjection::
MapProjection(const MC2Coordinate& centerCoord,
              int screenX, int screenY, double scale,
              double angle) : TransformMatrix(), m_centerCoord(centerCoord),
                              m_screenSize(screenX, screenY),
                              m_scale(scale),
                              m_angle(angle),
                              m_dpiCorrectionFactor( 1 )
{
}


void
MapProjection::updateBBox()
{
//    static const double mc2scaletometer = 6378137.0*2.0*
//       3.14159265358979323846 / 4294967296.0;
//    static const float meterstomc2scale = 1.0 / mc2scaletometer;

   pair<float,float> widthAndHeight = getWidthAndHeight();
   const float width = widthAndHeight.first;
   const float height = widthAndHeight.second;
   
   {
      const int32 centerLat = getCenter().lat;
      const float halfHeight = height * 0.5;
      const int32 intHalfHeight = int32( halfHeight );
      
      if ( halfHeight + centerLat > ( MAX_INT32 / 2 ) ) {
         m_bbox.setMaxLat( MAX_INT32 / 2 );
      } else {
         m_bbox.setMaxLat( centerLat + intHalfHeight );
      }
      if ( centerLat - halfHeight < ( MIN_INT32 / 2 ) ) {
         m_bbox.setMinLat( MIN_INT32 / 2 );
      } else {
         m_bbox.setMinLat( centerLat - intHalfHeight );
      }
      m_bbox.updateCosLat();
   }
   
   {            
      if ( width >= MAX_UINT32 - 1) {
         m_bbox.setMinLon( MIN_INT32 );
         m_bbox.setMaxLon( MAX_INT32 );
      } else {
         int32 intWidth = int32(width * 0.5 );
         int32 centerLon = getCenter().lon;
         m_bbox.setMinLon( centerLon - intWidth );
         m_bbox.setMaxLon( centerLon + intWidth );
      }
   }

//    MC2Coordinate coord1;
//    inverseTranformCosLatSupplied( coord1,
//                                   m_screenSize.getX(),
//                                   0,
//                                   getCosLat(getCenter().lat));
//    MC2Coordinate coord2;
//    inverseTranformCosLatSupplied( coord2,
//                                   0,
//                                   m_screenSize.getY(),
//                                   getCosLat(getCenter().lat));
//    MC2Coordinate coord3;
//    inverseTranformCosLatSupplied( coord3,
//                                   m_screenSize.getX(),
//                                   m_screenSize.getY(),
//                                   getCosLat(getCenter().lat));
//    MC2Coordinate coord4;
//    inverseTranformCosLatSupplied( coord4,
//                                   0,
//                                   0,
//                                   getCosLat(getCenter().lat) );

#if 0
#ifdef __unix__
   mc2dbg8 << "[TMH]: m_centerCoord = " << m_centerCoord
           << "                                      and center of m_bbox = "
           << m_bbox.getCenter() << endl;
   MC2Point corner1(0,0);
   MC2Point corner2(corner1);
   MC2Point corner3(corner2);
   MC2Point corner4(corner3);

   transformPointCosLatSupplied(corner1, coord1, getCosLat(getCenter().lat));
   transformPointCosLatSupplied(corner2, coord2, getCosLat(getCenter().lat));
   transformPointCosLatSupplied(corner3, coord3, getCosLat(getCenter().lat));
   transformPointCosLatSupplied(corner4, coord4, getCosLat(getCenter().lat));
   
   mc2dbg << "[TMH]: Corners are "
          << corner1.getX() << "," << corner1.getY() << " + "
          << corner2.getX() << "," << corner2.getY() << " + "
          << corner3.getX() << "," << corner3.getY() << " + "
          << corner4.getX() << "," << corner4.getY() << " + "
          << endl;
#endif
#endif
}

void
MapProjection::updateTransformMatrix()
{
   // Constant forever
   static const float mc2scaletometer = 6378137.0*2.0*
      3.14159265358979323846 / 4294967296.0;
   static const float meterstomc2scale = 1.0 / mc2scaletometer;

   float radAngle = getAngle() / 180 * M_PI;
   float maxScale = getCosLat( m_centerCoord.lat) * float(MAX_UINT32-1000) / (
      ( fabs( m_screenSize.getX() *
              meterstomc2scale * cos(radAngle) ) +
        fabs( m_screenSize.getY() *
              meterstomc2scale * sin(radAngle) ) ) );

   m_scale = MIN( (float)m_scale, maxScale );
   
   const double invScale = 1.0/m_scale;     
   const double mc2scale = mc2scaletometer * invScale;

   updateBBox();
   
   TransformMatrix::updateMatrix( m_angle, mc2scale, m_centerCoord,
                                  m_screenSize.getX(),
                                  m_screenSize.getY() );
}

float64
MapProjection::getPixelScaleFromBBoxAndSize( int screenXSize, 
                                        int screenYSize,
                                        MC2BoundingBox& bbox )
{
   static const double mc2scaletometer = 6378137.0*2.0*
      3.14159265358979323846 / 4294967296.0;
   // Strech out the bbox to fit the screen size.

   // This is copied from GfxUtility. I don't want that dependency   
   // so I have copied it here. I might want to change it though.

   int32 minLat = bbox.getMinLat();
   int32 minLon = bbox.getMinLon();
   int32 maxLat = bbox.getMaxLat();
   int32 maxLon = bbox.getMaxLon();
   
   int width = screenXSize;
   int height = screenYSize;
   // width and height should have same proportions as 
   // bbox.width and bbox.height
   float64 bboxHeight = bbox.getHeight();
   float64 bboxWidth = bbox.getWidth();
   if ( bboxHeight == 0.0 ) {
      bboxHeight = 1.0;
   }
   if ( bboxWidth == 0.0 ) {
      bboxWidth = 1.0;
   }
   float64 factor = bboxHeight / bboxWidth * width / height;
   if ( factor < 1 ) {
      // Compensate for that the display is higher than the bbox
//      height = uint16( height * factor );
      int32 extraHeight = 
         int32( rint( ( (bboxHeight / factor ) - 
                        bboxHeight ) / 2 ) );
      minLat -= extraHeight;
      maxLat += extraHeight;
   } else {
      // Compensate for that the display is wider than the bbox
//      width = uint16( width / factor );
      uint32 lonDiff = bbox.getLonDiff();
      if ( lonDiff == 0 ) {
         lonDiff = 1;
      }
      int32 extraWidth = 
         int32( rint( ( (lonDiff * factor ) - 
                        lonDiff ) / 2 ) );
      minLon -= extraWidth;
      maxLon += extraWidth;
      bbox.setMinLon( minLon );
      bbox.setMaxLon( maxLon );
   }

   bbox.setMinLat( MAX( minLat, (int32) BOTTOM_LAT ) );
   bbox.setMaxLat( MIN( maxLat, (int32) TOP_LAT ) );
   bbox.setMinLon( minLon );
   bbox.setMaxLon( maxLon );
   
   float64 scale = 
      double(bbox.getHeight() * mc2scaletometer) /
      screenYSize; // unit meters map / pixel
   
   // Ugglefix.
   if ( scale < 0 || scale > 24000.0 ) {
      scale = 24000.0;
   }
   
   return scale;   
}

MC2BoundingBox
MapProjection::setBoundingBox(const MC2BoundingBox& inbbox)
{
   MC2BoundingBox bbox(inbbox);
   // Set the scale
   m_scale = getPixelScaleFromBBoxAndSize( m_screenSize.getX(),
                                      m_screenSize.getY(),
                                      bbox );
   // Save the corner
   setCenter( bbox.getCenter() );
   
   return bbox;
}

pair<float,float>
MapProjection::getWidthAndHeight() const
{
   static const double mc2scaletometer = 6378137.0*2.0*
      3.14159265358979323846 / 4294967296.0;
   static const float meterstomc2scale = 1.0 / mc2scaletometer;

   const float radAngle = ( getAngle() ) / 180.0 * M_PI;
   const float scale = getPixelScale() * meterstomc2scale;
   const float b = m_screenSize.getX() * scale;
   const float h = m_screenSize.getY() * scale;
   const float width =
      ( fabs( b * cos(radAngle) ) +
        fabs( h * sin(radAngle) ) ) / m_cosLat;
   
   const float height =
      fabs( b * sin(radAngle) ) +
      fabs( h * cos(radAngle) );
   
   return pair<float,float>(width,height);
}

void
MapProjection::getDrawingBBoxes( vector<MC2BoundingBox>& outBoxes ) const
{
   pair<float,float> widthAndHeight = getWidthAndHeight();
   const float height = widthAndHeight.second;
   // Height
   MC2BoundingBox bbox;
   {
      const int32 centerLat = getCenter().lat;
      const float halfHeight = height * 0.5 ;
      const int32 intHalfHeight = int32( halfHeight );
      
      if ( halfHeight + centerLat > ( MAX_INT32 / 2 ) ) {
         bbox.setMaxLat( MAX_INT32 / 2 );
      } else {
         bbox.setMaxLat( centerLat + intHalfHeight );
      }
      if ( centerLat - halfHeight < ( MIN_INT32 / 2 ) ) {
         bbox.setMinLat( MIN_INT32 / 2 );
      } else {
         bbox.setMinLat( centerLat - intHalfHeight );
      }
      bbox.updateCosLat();
   }
   // Widths
   {
      float width = MIN( widthAndHeight.first, float(MAX_UINT32) );
      int32 centerLon = getCenter().lon;
      int32 curMinLon = int32(int64(centerLon) - int64(width / 2) );
      // Add bounding boxes until there is no width left
      while ( width > 0.0 ) {
         outBoxes.push_back( bbox );
         MC2BoundingBox& curBox = outBoxes.back();
         int32 curWidth = int32( MIN( width, float( MAX_INT32 / 4 ) ) );
         if ( curWidth == 0 ) {
            outBoxes.pop_back();
            break;
         }
         curBox.setMinLon( curMinLon );
         curBox.setMaxLon( curMinLon + curWidth );
         if ( curBox.getMinLon() > curBox.getMaxLon() ) {
            curBox.setMaxLon( MAX_INT32 );
            curMinLon = MIN_INT32;
            width -= ( curBox.getMaxLon() - curBox.getMinLon() );
         } else {            
            curMinLon += curWidth;
            width -= curWidth;
         }
      }      
   }
}

double
MapProjection::setPixelScale(double scale)
{
   m_scale = scale;
   const double minScale = 0.1;
   const double maxScale = 24000.0;
   if ( m_scale < minScale ) {
      m_scale = minScale;
   } else if ( m_scale > maxScale ) {
      m_scale = maxScale;
   }
   updateTransformMatrix();
   return m_scale;
}

double
MapProjection::getPixelScale() const
{
   return m_scale;
}

void 
MapProjection::setDPICorrectionFactor( uint32 factor )
{
   m_dpiCorrectionFactor = factor;
}
   
uint32 
MapProjection::getDPICorrectionFactor() const 
{
   return m_dpiCorrectionFactor;
}
   
double 
MapProjection::setDPICorrectedScale( double scale )
{
   setPixelScale( scale / double( getDPICorrectionFactor() ) );
   return getDPICorrectedScale();
}
   
double 
MapProjection::getDPICorrectedScale() const 
{
   double scale = getPixelScale() * double( getDPICorrectionFactor() );
   if ( scale > 24000.0 ) {
      scale = 24000.0;
   }
   return scale;
}

double
MapProjection::zoom(double factor)
{
   setPixelScale( factor * getPixelScale() );
   return m_scale;
}

double
MapProjection::zoom(double factor,
                    const MC2Coordinate& zoomCoord,
                    const MC2Point& zoomPoint )
{
   double newScale = zoom( factor );
   
   setPoint( zoomCoord, zoomPoint );

   return newScale;
}

void
MapProjection::setPixelBox( const MC2Point& oneCorner,
                            const MC2Point& otherCorner )
{
   PixelBox pixBox( oneCorner, otherCorner );
   MC2BoundingBox bbox;
   for( int i = 0; i < 4; ++i ) {
      MC2Coordinate tmpCoord;
      inverseTranformUsingCosLat( tmpCoord, pixBox.getCorner(i) );
      bbox.update( tmpCoord );
   }
   double oldangle = m_angle;
   setAngle(0);
   setBoundingBox( bbox );
   setAngle( oldangle );
}

void
MapProjection::move(int deltaX,
                    int deltaY )
{
#if 0
   // Only move one pixel.
   if ( deltaX ) {
      deltaX = deltaX/abs<int>(deltaX);
   }
   if ( deltaY ) {
      deltaY = deltaY/abs<int>(deltaY);
   }
#endif
   // Translate the screen coordinates into lat/lon.
   MC2Coordinate center;
   inverseTranformCosLatSupplied(
      center,
      deltaX + (m_screenSize.getX() >> 1),
      deltaY + (m_screenSize.getY() >> 1),
      getCosLat( m_centerCoord.lat ) );
   setCenter( center );
}

void
MapProjection::setCenter(const MC2Coordinate& newCenter)
{
   m_centerCoord = newCenter;
   if ( m_centerCoord.lat > TOP_LAT ) {
      m_centerCoord.lat = TOP_LAT;
   } else if ( m_centerCoord.lat < BOTTOM_LAT ) {
      m_centerCoord.lat = BOTTOM_LAT;
   }
   updateTransformMatrix();
}

void
MapProjection::setPoint(const MC2Coordinate& newCoord,
                        const MC2Point& screenPoint )
{
   // Translate the center to a screen coord.
   MC2Point centerPoint(0,0);
   transformPointInternalCosLat( centerPoint,
                                 m_centerCoord );

   
   // Check the first part of 
   // TransformMatrix::inverseTranformUsingCosLat
   // m_Ty is calculated from this using the screen point and coord.
   
   m_Ty =  - 1.0/m_Sy * m_C * screenPoint.getX()
           + 1.0/m_Sy * m_D * screenPoint.getY()
           + 1.0/m_Sy * m_C * m_tx
           - 1.0/m_Sy * m_D * m_ty 
           - newCoord.lat;

   // Now the calculated m_Ty is used to get the latitude of the center.
   // Also check the first part of inverseTranformUsingCosLat for this.
   
   m_centerCoord.lat = 
         int32(int64(( - 1.0/m_Sy * m_C * centerPoint.getX()
                       + 1.0/m_Sy * m_D * centerPoint.getY()
                       + 1.0/m_Sy * m_C * m_tx 
                       - 1.0/m_Sy * m_D * m_ty
                       - m_Ty ) ) );
  
   // Now that the center latitude is known, the coslat can be
   // calcualted.
   m_cosLat = getCosLat( m_centerCoord.lat );

   double S_x = m_Sx * m_cosLat;
   
   // This next step is from the second part of that method.
   // m_Tx is solved from that equation using the screenpoint and coord.

   m_Tx = 1.0 / S_x * m_A * screenPoint.getX()
          - 1.0 / S_x * m_B * screenPoint.getY()
          - 1.0 / S_x * m_A * m_tx
          + 1.0 / S_x * m_B * m_ty
          - newCoord.lon;

   // Now explicitly calculate the center longitude.
   // This is done since we want to be able to use setCenter
   // to set all the different members etc.
   m_centerCoord.lon = int32( int64( + 1.0 / S_x * m_A * centerPoint.getX()
                                - 1.0 / S_x * m_B * centerPoint.getY()
                                - 1.0 / S_x * m_A * m_tx
                                + 1.0 / S_x * m_B * m_ty
                                - m_Tx) );

   // Now set the center to that center coordinate. Will update
   // all the members.
   setCenter( m_centerCoord );

}

void
MapProjection::setAngle(double angleDeg)
{   
   m_angle = angleDeg;
   updateTransformMatrix(); 
}

void
MapProjection::setAngle(double angleDeg,
                        const MC2Point& rotationPoint )
{
   // Translate the center to a screen coord.
   MC2Point centerPoint(0,0);
   transformPointInternalCosLat( centerPoint,
                                 m_centerCoord );

   int deltaX = centerPoint.getX() - rotationPoint.getX();
   int deltaY = centerPoint.getY() - rotationPoint.getY();

   move(-deltaX, -deltaY);
   setAngle( angleDeg );
   move(deltaX, deltaY);
}

double
MapProjection::getAngle() const
{
   return m_angle;
}

void
MapProjection::setScreenSize(const MC2Point& size)
{
   if ( size != m_screenSize && size != MC2Point(0,0) ) {
      m_screenSize = size;
      updateTransformMatrix();
   }
}

MC2Coordinate
MapProjection::calcLowerLeft() const
{
   // Constant forever
   static const double mc2scaletometer = 6378137.0*2.0*
      3.14159265358979323846 / 4294967296.0;
   const double invScale = 1.0/m_scale;
   const double mc2scale = mc2scaletometer * invScale;

   const int screenWidth  = m_screenSize.getX();
   const int screenHeight = m_screenSize.getY();
   
   return MC2Coordinate( int32(double(m_centerCoord.lat) -
                               (1/mc2scale * screenHeight * 0.5)),
                         int32(double(m_centerCoord.lon) -
                               (1/mc2scale/getCosLat(m_centerCoord.lat) *
                                screenWidth * 0.5 ) ) );
}
