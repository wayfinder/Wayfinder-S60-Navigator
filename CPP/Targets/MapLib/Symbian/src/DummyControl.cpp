/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DummyControl.h"

#include <stdio.h>
#include "MC2Point.h"

#if defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
# include <aknutils.h>  // for fonts
#endif

// Dummy implementation sufficient. 
class DummyClickInfo : public ClickInfo
{
   const char* getName() const { return ""; }
   bool shouldHighlight() const { return false; }
};

DummyMapMovingInterface::DummyMapMovingInterface()
{
   m_dummyClickInfo = new DummyClickInfo();
}

DummyMapMovingInterface::~DummyMapMovingInterface()
{
   delete m_dummyClickInfo;
}

void 
DummyMapMovingInterface::setMovementMode( bool moving )
{
   // Dummy implementation sufficient. 
}

const ClickInfo&
DummyMapMovingInterface::getInfoForFeatureAt( 
                     const MC2Point& point,
                     bool onlyPois,
                     TileMapInfoCallback* infoCallback )
{
   // Dummy implementation sufficient. 
   return *m_dummyClickInfo;
}

const MC2Coordinate& 
DummyMapMovingInterface::getCenter() const
{
   return m_center;
}

void 
DummyMapMovingInterface::transform( MC2Point& point,
                                    const MC2Coordinate& coord ) const
{
   // FIXME: Implementation of this method is needed.
}

void 
DummyMapMovingInterface::inverseTransform( MC2Coordinate& coord,
                                           const MC2Point& point ) const
{
   // FIXME: Implementation of this method is needed.
   coord = m_center;
}

void 
DummyMapMovingInterface::setCenter(const MC2Coordinate& newCenter)
{
   m_center = newCenter;
}

void 
DummyMapMovingInterface::setPoint(const MC2Coordinate& newCoord,
                                  const MC2Point& screenPoint )
{
   // FIXME: Implementation of this method is needed.
}

void 
DummyMapMovingInterface::setPoint(const MC2Coordinate& newCoord,
                                  const MC2Point& screenPoint,
                                  double angleDegrees )
{
   // FIXME: Implementation of this method is needed.
}

void 
DummyMapMovingInterface::move(int deltaX, int deltaY)
{
   // FIXME: Better implementation of this method is needed.

   m_center.lon += int( deltaX * m_scale * 100);
   m_center.lat -= int( deltaY * m_scale * 100);
}

void 
DummyMapMovingInterface::setAngle(double angleDegrees)
{
   // No need to implement if getAngle() always returns 0.
}

void 
DummyMapMovingInterface::setAngle( double angleDegrees,
                                   const MC2Point& rotationPoint )
{
   // No need to implement if getAngle() always returns 0.
}

void 
DummyMapMovingInterface::rotateLeftDeg( int nbrDeg )
{
   // No need to implement if getAngle() always returns 0.
}

double 
DummyMapMovingInterface::getAngle() const
{
   // It will not be possible to rotate the globe around the center point.
   // Return 0 to indicate that the globe is not rotated.
   return 0;
}

double 
DummyMapMovingInterface::setScale(double scale)
{
   // FIXME: Choose a good maximum value of the scale so that the 
   // entire globe is visible and looks nice at maximum zoomlevel.
   // Might depend on screen size.
   if ( scale > 100000 ) {
      scale = 100000;
   }
   m_scale = scale;
   return m_scale;
}

double 
DummyMapMovingInterface::getScale() const
{
   return m_scale;
}

double 
DummyMapMovingInterface::zoom(double factor)
{
   return setScale( factor * getScale() );
}

double 
DummyMapMovingInterface::zoom( double factor, 
                               const MC2Coordinate& zoomCoord,
                               const MC2Point& zoomPoint )
{
   // Dummy implementation sufficient. 
   //return m_scale;
   return setScale( factor * getScale() );
}

void 
DummyMapMovingInterface::setPixelBox( const MC2Point& oneCorner,
                                      const MC2Point& otherCorner )
{
   // FIXME: Implementation of this method is needed.
}
   
void 
DummyMapMovingInterface::setWorldBox( const MC2Coordinate& oneCorner,
                                      const MC2Coordinate& otherCorner )
{
   // FIXME: Implementation of this method is needed.
}

bool 
DummyMapMovingInterface::getNextHighlightPoint( MC2Point& point )
{
   // Dummy implementation sufficient. 
   return false;
}

void 
DummyMapMovingInterface::setHighlight( bool visible )
{
   // Dummy implementation sufficient. 
}

CDummyControlX::CDummyControlX()
{

}

void
CDummyControlX::ConstructL( const CCoeControl& container )
{

   CreateWindowL(&container);
//   SetContainerWindowL( container );
//   SetFocus( true );
#if defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
   SetFont( LatinPlain12() );
#endif
   SetExtent( TPoint( 0, 0 ), TSize( 200, 200 )  );
   _LIT(KTemp, "Temporary text");
   SetTextL( KTemp );
}

void
CDummyControlX::prepareDraw()
{
   TBuf<256> buf;

   buf.Format( _L("<Insert globe here>\nScale: %d\nMC2 Lat: %d\nMC2 Lon: %d\nWGS84 Lat: %f\nWGS84 Lon: %f\nAngle: %d"), 
            (int) m_dummyHandler.getScale(),
            (int) m_dummyHandler.getCenter().lat,
            (int) m_dummyHandler.getCenter().lon,
            WGS84Coordinate( m_dummyHandler.getCenter() ).latDeg,
            WGS84Coordinate( m_dummyHandler.getCenter() ).lonDeg,
            (int) m_dummyHandler.getAngle() );
   
   if (buf.Length() > 0) {
      SetTextL( buf );
   }
}

void 
CDummyControlX::repaintNow()
{
   prepareDraw();
   DrawNow();
}

void 
CDummyControlX::requestRepaint()
{
   prepareDraw();
   DrawDeferred();
}

void 
CDummyControlX::Draw( const TRect& rect ) const {

   CWindowGc& gc = SystemGc();   

   // First clear.
   gc.SetBrushColor( TRgb( 0, 0, 0 ) );
   gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
   gc.Clear( rect );

   CEikLabel::Draw( rect );   
}

// --- DummyMapComponentWrapper

DummyMapComponentWrapper::DummyMapComponentWrapper( 
                                 const CCoeControl& container ) 
{
   m_control = new CDummyControlX();
   m_control->ConstructL( container );
}
      
DummyMapComponentWrapper::~DummyMapComponentWrapper() 
{
   delete m_control;
}

CCoeControl& DummyMapComponentWrapper::getControl() const 
{
   return *m_control;
}

MapMovingInterface& DummyMapComponentWrapper::getMapMovingInterface() const
{
   return m_control->getMapMovingInterface();
}

MapDrawingInterface& DummyMapComponentWrapper::getMapDrawingInterface() const
{
   return *m_control;
}

int DummyMapComponentWrapper::getMinScale() const
{
   return 1000;
}

// --- GlobeCreator
/*
MapComponentWrapper*
GlobeCreator::createGlobe( const CCoeControl& container )
{
   return new DummyMapComponentWrapper( container );
}
*/
