/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "GlobeMapMovingInterface.h"

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
//#include <bitdev.h>
//#include <bitstd.h>

#include "gbDefs.h"
#include "gbGlobe.h"

#include <math.h>

//const double GB_MIN_SCALE = 12000;
#define GB_TOP_LAT 912909609
#define GB_BOTTOM_LAT -912909609

class GlobeClickInfo : public ClickInfo
{
   const char* getName() const { return ""; }
   bool shouldHighlight() const { return false; }
};

GlobeMapMovingInterface::GlobeMapMovingInterface() : 
   m_topLeftGlobePos( 0, 0 )
{
   m_clickInfo = new GlobeClickInfo();

	m_moved = true;
   m_zoomTweak = 0.1075f;
   m_globe = NULL;

//   x = 0.0;
//   y = 0.0;
}

GlobeMapMovingInterface::~GlobeMapMovingInterface()
{
   delete m_clickInfo;
}
   
void 
GlobeMapMovingInterface::setTopLeftGlobePos( const MC2Point& globePos )
{
   m_topLeftGlobePos = globePos;
}

MC2Point 
GlobeMapMovingInterface::screenToGlobe( const MC2Point& screenPoint ) const
{
   return screenPoint - m_topLeftGlobePos;
}

MC2Point 
GlobeMapMovingInterface::globeToScreen( const MC2Point& globePoint ) const
{
   return globePoint + m_topLeftGlobePos;
}

void 
GlobeMapMovingInterface::setMovementMode( bool moving )
{
   // Dummy implementation sufficient. 
}

const ClickInfo&
GlobeMapMovingInterface::getInfoForFeatureAt( 
                     const MC2Point& point,
                     bool onlyPois,
                     TileMapInfoCallback* infoCallback )
{
   // Dummy implementation sufficient. 
   return *m_clickInfo;
}

const MC2Coordinate& 
GlobeMapMovingInterface::getCenter() const
{
   return m_center;
}

void 
GlobeMapMovingInterface::transform( MC2Point& point,
                                    const MC2Coordinate& coord ) const
{
   if (!m_globe)
      return;

   WGS84Coordinate wgsCoord = WGS84Coordinate( coord );
   float latitude = (float)wgsCoord.latDeg;
   float longitude = (float)wgsCoord.lonDeg;
   float result[2];
   GBGlobe_worldToScreen( m_globe, latitude, longitude, result );
   /* result[0] is the screen horizontal coordinate */
   /* result[1] is the screen vertical coordinate */
   point = globeToScreen( MC2Point( (int)result[0], (int)result[1] ) );
}

void 
GlobeMapMovingInterface::inverseTransform( MC2Coordinate& coord,
                                           const MC2Point& point ) const
{
   MC2Point gbPoint = screenToGlobe( point );
   if (!m_globe)
      return;

   float result[2];
   float x = (float)gbPoint.getX(); /* screen horizontal coordinate */
   float y = (float)gbPoint.getY(); /* screen vertical coordinate */
   GBGlobe_screenToWorld( m_globe, x, y, result );
   /* result[0] is the longitude in degrees */
   /* result[1] is the latitude in degrees */
   coord = WGS84Coordinate( (double)result[1], (double)result[0] );
}

void 
GlobeMapMovingInterface::setCenter(const MC2Coordinate& newCenter)
{
   m_center = newCenter;
   if ( m_center.lat > GB_TOP_LAT ) {
      m_center.lat = GB_TOP_LAT;
   } else if ( m_center.lat < GB_BOTTOM_LAT ) {
      m_center.lat = GB_BOTTOM_LAT;
   }
   m_moved = true;
}

void 
GlobeMapMovingInterface::setPoint(const MC2Coordinate& newCoord,
                                  const MC2Point& screenPoint )
{
   MC2Point gbScreenPoint = screenToGlobe( screenPoint );
	if (!m_globe)
		return;

	float c[2];
	GBGlobe_screenToWorld( m_globe, 
                         (float)gbScreenPoint.getX(), 
                         (float)gbScreenPoint.getY(), c );

	WGS84Coordinate wgsCoord = WGS84Coordinate( newCoord );
	float longitude = (float)wgsCoord.lonDeg;
	float latitude = (float)wgsCoord.latDeg;

	if (gbFAbs(c[0] - longitude) > 180.0f)
	{
		if (c[0] < longitude)
			longitude -= 360.0f;
		else if (c[0] > longitude)
			longitude += 360.0f;
	}

	WGS84Coordinate n( m_center );
	
	n.lonDeg -= c[0] - longitude;
	n.latDeg -= c[1] - latitude;
	
	setCenter( n );
}

void 
GlobeMapMovingInterface::move(int deltaX, int deltaY)
{
   // FIXME: Better implementation of this method is needed.

   MC2Coordinate newCenter = m_center;
   newCenter.lon += int( deltaX * m_scale * 100);
   newCenter.lat -= int( deltaY * m_scale * 100);

   setCenter( newCenter );

//	m_zoomTweak += 0.02f * (float)deltaY / 100.0f;
//   x += (double)deltaX / 20.0;
//   y -= (double)deltaY / 20.0;

//   setWorldBox( MC2Coordinate( WGS84Coordinate( 60.10, 24.53 ) ), MC2Coordinate( WGS84Coordinate( y, x ) ) );
}

void 
GlobeMapMovingInterface::setAngle(double angleDegrees)
{
   // No need to implement if getAngle() always returns 0.
}

void 
GlobeMapMovingInterface::setAngle( double angleDegrees,
                                   const MC2Point& rotationPoint )
{
   // No need to implement if getAngle() always returns 0.
}

void 
GlobeMapMovingInterface::setPoint(const MC2Coordinate& newCoord,
                                  const MC2Point& screenPoint,
                                  double angleDegrees )
{
   setPoint( newCoord, screenPoint );
   setAngle( angleDegrees, screenPoint ); 
}


void 
GlobeMapMovingInterface::rotateLeftDeg( int nbrDeg )
{
   // No need to implement if getAngle() always returns 0.
}

double 
GlobeMapMovingInterface::getAngle() const
{
   // It will not be possible to rotate the globe around the center point.
   // Return 0 to indicate that the globe is not rotated.
   return 0;
}

double 
GlobeMapMovingInterface::setScale(double scale)
{
   // FIXME: Choose a good maximum value of the scale so that the 
   // entire globe is visible and looks nice at maximum zoomlevel.
   // Might depend on screen size.
   if ( scale > 100000 ) {
      scale = 100000;
   }
   m_scale = scale;
	m_moved = true;
   return m_scale;
}

double 
GlobeMapMovingInterface::getScale() const
{
   return m_scale;
}

double 
GlobeMapMovingInterface::zoom(double factor)
{
	m_moved = true;
   return setScale( factor * getScale() );
}

double 
GlobeMapMovingInterface::zoom( double factor, 
                               const MC2Coordinate& zoomCoord,
                               const MC2Point& zoomPoint )
{
   double newScale = zoom( factor );
   setPoint( zoomCoord, zoomPoint );
	m_moved = true;
   return newScale;
}

void 
GlobeMapMovingInterface::setPixelBox( const MC2Point& oneCorner,
                                      const MC2Point& otherCorner )
{
   MC2Point gbOneCorner = screenToGlobe( oneCorner );
   MC2Point gbOtherCorner = screenToGlobe( otherCorner );

   if (!m_globe)
      return;

   float first[2];
   float second[2];

   GBGlobe_screenToWorld( m_globe, (float)gbOneCorner.getX(), (float)gbOneCorner.getY(), first );
   GBGlobe_screenToWorld( m_globe, (float)gbOtherCorner.getX(), (float)gbOtherCorner.getY(), second );

   setWorldBox( WGS84Coordinate( first[1], first[0] ), WGS84Coordinate( second[1], second[0] ) );

   m_moved = true;
}

void 
GlobeMapMovingInterface::setWorldBox( const MC2Coordinate& oneCorner,
                                      const MC2Coordinate& otherCorner )
{
   if (!m_globe)
      return;

   float firstNormal[3];
   float secondNormal[3];

   WGS84Coordinate wgsFirst = WGS84Coordinate( oneCorner );
   WGS84Coordinate wgsSecond = WGS84Coordinate( otherCorner );

   GBGlobe_worldToWorldNormal( m_globe, (float)wgsSecond.lonDeg, (float)wgsFirst.latDeg, firstNormal );
   GBGlobe_worldToWorldNormal( m_globe, (float)wgsFirst.lonDeg, (float)wgsSecond.latDeg, secondNormal );

   float result[3];
   result[0] = firstNormal[0] + secondNormal[0];
   result[1] = firstNormal[1] + secondNormal[1];
   result[2] = firstNormal[2] + secondNormal[2];

   gbFVec3Normalize( result );

   float resultWorld[2];
   GBGlobe_normalToWorld( m_globe, result, resultWorld );

   setCenter( WGS84Coordinate( resultWorld[1], resultWorld[0] ) );

   double a = acos(firstNormal[0] * secondNormal[0] + firstNormal[1] * secondNormal[1] + firstNormal[2] * secondNormal[2]);

   double b = atan(6.0 / 15.0);
   double c = GB_PI - (a + b) / 2.0;

   m_scale = sin(c) / sin(b / 2);
   m_scale = ((m_scale - 2.0f) / 4.0f + m_zoomTweak) * (double)GB_MIN_SCALE * 6.0 + (double)GB_MIN_SCALE;
	
   if ( m_scale > 100000 ) {
      m_scale = 100000;
   }

	m_moved = true;
}

bool 
GlobeMapMovingInterface::getNextHighlightPoint( MC2Point& point )
{
   // Dummy implementation sufficient. 
   return false;
}

void 
GlobeMapMovingInterface::setHighlight( bool visible )
{
   // Dummy implementation sufficient. 
}

void
GlobeMapMovingInterface::setGlobe( GBGlobe* globe )
{
	m_globe = globe;
}

bool GlobeMapMovingInterface::isMoved() const
{
	return m_moved;
}

void GlobeMapMovingInterface::clearMoved()
{
	m_moved = false;
}

void GlobeMapMovingInterface::setMoved( bool moved )
{
   m_moved = moved;
}


