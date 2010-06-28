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
#include "UserDefinedFeature.h"
#include "MapMovingInterface.h"
#include <math.h>

using namespace std;

UserDefinedFeature::UserDefinedFeature(user_feature_t type,
                                       const vector<MC2Point>& points,
                                       const ScreenOrWorldCoordinate& center,
                                       int dpiCorrectionFactor)
      : m_type( type ),
        m_origPoints( points ),
        m_drawPoints( points ),
        m_center( center ),
        m_angleDegs( 0 ),
        m_angleIsFromNorth ( true ),
        m_visible( true ),
        m_clickable( true ),
        m_dpiCorrectionFactor( dpiCorrectionFactor )
{
}

void
UserDefinedFeature::setVisible(int visible)
{
   m_visible = visible;
}
void
UserDefinedFeature::setClickable(int clickable)
{
   m_clickable = clickable;
}

void
UserDefinedFeature::setCenter( const ScreenOrWorldCoordinate& coord )
{
   m_center = coord;
}

const ScreenOrWorldCoordinate&
UserDefinedFeature::getCenter() const
{
   return m_center;
}

void
UserDefinedFeature::setPoints( const vector<MC2Point>& points )
{
   m_origPoints = points;
   if ( m_origPoints.size() != m_drawPoints.size() ) {
      // Also resize the m_drawPoints-vector
      m_drawPoints = m_origPoints;
   }
}

void
UserDefinedFeature::swapPoints( vector<MC2Point>& points )
{
   m_origPoints.swap( points );
   if ( m_origPoints.size() != m_drawPoints.size() ) {
      // Also resize the m_drawPoints-vector
      m_drawPoints = m_origPoints;
   }
}

void
UserDefinedFeature::rescalePoints()
{
   for ( vector<MC2Point>::iterator it = m_origPoints.begin();
         it != m_origPoints.end();
         ++it ) {
      it->getX() *= m_dpiCorrectionFactor;
      it->getY() *= m_dpiCorrectionFactor;
   }   
}

void
UserDefinedFeature::updateScreenCoords(const MapMovingInterface& matrix)
{
   double angleDegs = m_angleDegs;
   if ( ! m_angleIsFromNorth ) {
      // Adjust angle until it is a heading.
      angleDegs = 90 - angleDegs;
   } else {
      angleDegs  = matrix.getAngle() - angleDegs;
   }
   float sinus   = sin( angleDegs / 180.0 * M_PI );
   float cosinus = cos( angleDegs / 180.0 * M_PI );
   
   int nbrPoints = m_origPoints.size();

   MC2Point orig( m_center.getScreenPoint(matrix) );
   
   for ( int i = 0; i < nbrPoints; ++i ) {
      m_drawPoints[i].getX() = orig.getX() +
         MC2Point::coord_t( m_origPoints[i].getX() * cosinus +
                            m_origPoints[i].getY() * sinus );
      m_drawPoints[i].getY() = orig.getY() +
         MC2Point::coord_t( - m_origPoints[i].getX() * sinus +
                            m_origPoints[i].getY() * cosinus );
   }
}

void 
UserDefinedFeature::setName( const MC2SimpleString& name )
{
   m_name = name;
}
   
const MC2SimpleString& 
UserDefinedFeature::getName() const
{
   return m_name;
}
   
bool 
UserDefinedFeature::hasName() const
{
   return ! m_name.empty();
}

