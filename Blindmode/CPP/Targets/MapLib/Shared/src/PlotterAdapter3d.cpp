/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MathVec.h"
#include "PlotterAdapter3d.h"
#include <vector>
#include "MC2Point.h"

#include "ClipUtil.h"
#include "MC2BoundingBox.h"
#include "PolyLine.h"
#include "InsideUtil.h"
#include <math.h>

using namespace isab;

PlotterAdapter3d::PlotterAdapter3d( MapPlotter* plotter ) : PlotterAdapter( plotter )
{
   m_settings.m_alpha = 30.0 / 180.0 * M_PI;
   m_settings.m_delta = 66.0 / 180.0 * M_PI;
   m_settings.m_horizonPixels = 0;
}

PlotterAdapter3d::~PlotterAdapter3d()
{

}

void 
PlotterAdapter3d::drawPolygon( std::vector<MC2Point>::const_iterator begin,
                               std::vector<MC2Point>::const_iterator end )
{
   std::vector<MC2Point> points3d;
   project3d( begin, end, points3d );
   if ( points3d.size() < 3 ) {
      return;
   }
   PlotterAdapter::drawPolygon( points3d.begin(), points3d.end() );
}

void 
PlotterAdapter3d::drawPolygonWithColor( std::vector<MC2Point>::const_iterator begin,
                                      std::vector<MC2Point>::const_iterator end,
                                      uint32 fillcolor )
{
   std::vector<MC2Point> points3d;
   project3d( begin, end, points3d );
   if ( points3d.size() < 3 ) {
      return;
   }
   PlotterAdapter::drawPolygonWithColor( points3d.begin(), points3d.end(), fillcolor );

}

void 
PlotterAdapter3d::drawPolyLine( std::vector<MC2Point>::const_iterator begin,
                              std::vector<MC2Point>::const_iterator end )
{
   setLineWidth( m_lineWidth );
   setFillColor( m_red, m_green, m_blue );
   plotterLine( begin, end, m_lineWidth );
}

void 
PlotterAdapter3d::drawPolyLineWithColor( std::vector<MC2Point>::const_iterator begin,
                                         std::vector<MC2Point>::const_iterator end,
                                         uint32 color,
                                         int lineWidth )
{
   
   if ( std::distance( begin, end ) < 2 ) {
      return;
   }

   setFillColor( uint8( color >> 16 ),
                 uint8( color >>  8 ),
                 uint8( color ) );
   
   plotterLine( begin, end, lineWidth );
}

void PlotterAdapter3d::setHorizonHeight( uint32 height )
{
   m_settings.m_horizonPixels = height;
}

void PlotterAdapter3d::setPenColor( unsigned int red,
                                    unsigned int green,
                                    unsigned int blue )
{
   // Store the colors
   m_red   = red;
   m_green = green;
   m_blue  = blue;
   PlotterAdapter::setPenColor( red, green, blue );
}

void PlotterAdapter3d::setLineWidth( int lineWidth )
{
   // Store the line width
   m_lineWidth = lineWidth;
   PlotterAdapter::setLineWidth( lineWidth );
}

void PlotterAdapter3d::plotterLine( std::vector<MC2Point>::const_iterator begin,
                                    std::vector<MC2Point>::const_iterator end,
                                    int lineWidth )
{
   std::vector<MC2Point> result;
   std::vector<MC2Point> triangle;

   // Reserve space in vectors
   result.reserve( 4 );
   result.reserve( 3 );

   // Scale the incoming points.
   int n = 3;
   lineWidth <<= n;
   std::vector<MC2Point> fixPoints( begin, end );
   for ( uint32 i = 0; i < fixPoints.size(); ++i ) {
      fixPoints[ i ] <<= n;
   }
   begin = fixPoints.begin();
   end = fixPoints.end();

   std::vector<MC2Point>::const_iterator curIt = begin;
   std::vector<MC2Point>::const_iterator nextIt = curIt;
   ++nextIt;

   MC2Point prevPoint = *curIt;
   MC2Point p0( -1, -1 );
   MC2Point p1( -1, -1 );
   MC2Point factor( -1, -1 );
   
   std::vector<MC2Point> points3d;
   
   while ( nextIt != end ) {
      result.clear();
      PolyLine::calculatePolyLine(*curIt, *nextIt, lineWidth, result, factor);
      project3d( result.begin(), result.end(), points3d, n );
      PlotterAdapter::drawPolygon( points3d.begin(), points3d.end() );
      // Do not add any triangles for the first polyline, only between polylines
      if ( curIt != begin ) {
         // We do have a previous point, this is not 
         // the first polyline.
         int turn = InsideUtil::isLeft( prevPoint, *curIt, *nextIt );
         if ( turn > 0 ) {
            // Right turn
            triangle.clear();
            triangle.push_back( p1 );
            triangle.push_back( *curIt );
            triangle.push_back( result[0] );
            project3d( triangle.begin(), triangle.end(), points3d, n );
            PlotterAdapter::drawPolygon( points3d.begin(), points3d.end() );
         } else if ( turn < 0 ) {
            // Left turn
            triangle.clear();
            triangle.push_back( p0 );
            triangle.push_back( *curIt );
            triangle.push_back( result[1] );
            project3d( triangle.begin(), triangle.end(), points3d, n );
            PlotterAdapter::drawPolygon( points3d.begin(), points3d.end() );
         } else {
            // Do nothing right now!
         }
         prevPoint = *curIt;
      } else {
         // This is the first poly line in this segment
         triangle.clear();
         triangle.push_back( result[0] );
         triangle.push_back( *curIt - factor );
         triangle.push_back( result[1] );
         project3d( triangle.begin(), triangle.end(), points3d, n );
         PlotterAdapter::drawPolygon( points3d.begin(), points3d.end() );
      }
      p0 = result[2];
      p1 = result[3];
      curIt = nextIt;
      ++nextIt;
      if ( nextIt == end ) {
         // This is the last poly line in this segment
         triangle.clear();
         triangle.push_back( result[2]);
         triangle.push_back( *curIt + factor );
         triangle.push_back( result[3] );
         project3d( triangle.begin(), triangle.end(), points3d, n );
         PlotterAdapter::drawPolygon ( points3d.begin(), points3d.end() );
      }
   }  
}

void 
PlotterAdapter3d::drawBitMap( const MC2Point& center,
                            const BitMap* bitMap)
{
   std::vector<MC2Point> origVec( 1, center );
       
   std::vector<MC2Point> points3d;
   project3d( origVec.begin(), origVec.end(), points3d );
   if ( points3d.empty() ) {
      return;
   }
   const MC2Point& p = points3d.front();
   if ( p.getX() < 0 || p.getX() > m_width ||
        p.getY() > m_height || p.getY() < (int)m_settings.m_horizonPixels ) {
      // Outside the map area, don't draw.
      return;
   }
   m_plotter->drawBitMap( p, bitMap );
}

void PlotterAdapter3d::checkScreenAndUpdateMembers( int scalefactor )
{   
   isab::Rectangle sizePixels;
   m_plotter->getMapSizePixels( sizePixels );

   if ( sizePixels == m_screenSizeInPixels && scalefactor == m_scalefactor) {
      return;
   }

   m_screenSizeInPixels = sizePixels;
   m_scalefactor = scalefactor;

   m_width = m_screenSizeInPixels.getWidth() << m_scalefactor;
   m_height = m_screenSizeInPixels.getHeight() << m_scalefactor;
   m_horizHeight = m_settings.m_horizonPixels << m_scalefactor;

   mc2dbg << "width = " << m_width << ", height = " << m_height << endl;

   float k = m_width / m_height;
   float a = m_height*sin(m_settings.m_alpha);
   float b = m_height*cos(m_settings.m_alpha);

   // The distance from the reference point (rotation point) and the eye.
   float d = ( m_width/2 ) / tan( m_settings.m_delta );
   float tanbeta = ( m_width/2 ) / ( b + d );
   float tan_phiy_by_two = sin(m_settings.m_alpha)*tanbeta/k;
   
   // The eye.
   MathVec<3> E;
   E[0] = m_width / 2;
   E[1] = m_height + d * cos( m_settings.m_alpha );
   E[2] = d * sin( m_settings.m_alpha );

   // The reference point is at the bottom of the screen.
   MathVec<3> R;
   R[0] = m_width / 2;
   R[1] = m_height;
   R[2] = 0;
   
   MathVec<3> V;
   V[0] = 0;
   V[1] = -1;
   V[2] = 0;

   MathVec<3> n = (E - R) / ( E - R ).norm();

   MathVec<3> u = V.cross( n );
   u /= u.norm();

   MathVec<3> v = n.cross( u );

   m_Va = u(0);
   m_Vb = u(1);
   m_Vc = u(2);
   m_Vd = -u(0)*E(0)-u(1)*E(1)-u(2)*E(2);
   m_Ve = v(0);
   m_Vf = v(1);
   m_Vg = v(2);
   m_Vh = -v(0)*E(0)-v(1)*E(1)-v(2)*E(2);
   m_Vi = n(0);
   m_Vj = n(1);
   m_Vk = n(2);
   m_Vl = -n(0)*E(0)-n(1)*E(1)-n(2)*E(2);
   
   vec_t zfront = -d;
   vec_t zback = -(d + b);
   vec_t r = m_width/a;
   
   m_Pa = 1 / (r* tan_phiy_by_two); 
   m_Pb = 1 / tan_phiy_by_two;

   m_Pc = (zfront + zback)/(zfront - zback);
   m_Pd = -2*zfront*zback/(zfront - zback);

#if 0
   mc2dbg << "m_Va " << m_Va<< endl;
   mc2dbg << "m_Vb " << m_Vb<< endl;
   mc2dbg << "m_Vc " << m_Vc<< endl;
   mc2dbg << "m_Vd " << m_Vd<< endl;
   mc2dbg << "m_Ve " << m_Ve<< endl;
   mc2dbg << "m_Vf " << m_Vf<< endl;
   mc2dbg << "m_Vg " << m_Vg<< endl;
   mc2dbg << "m_Vh " << m_Vh<< endl;
   mc2dbg << "m_Vi " << m_Vi<< endl;
   mc2dbg << "m_Vj " << m_Vj<< endl;
   mc2dbg << "m_Vk " << m_Vk<< endl;
   mc2dbg << "m_Vl " << m_Vl<< endl;
   mc2dbg << "m_Pa " << m_Pa << endl;
   mc2dbg << "m_Pb " << m_Pb << endl;
   mc2dbg << "m_Pc " << m_Pc << endl;
   mc2dbg << "m_Pd " << m_Pd << endl;
#endif
}

void
PlotterAdapter3d::project3d( std::vector<MC2Point>::const_iterator begin,
                             std::vector<MC2Point>::const_iterator end,
                             std::vector<MC2Point>& res,
                             int scalefactor )
{
   // Do the needful.

#ifdef __unix__
   #undef mc2dbg
   //#define mc2dbg cout
   #define mc2dbg mc2dbg8
#endif
   
   res.clear();
   res.reserve( std::distance( begin, end ) );

   checkScreenAndUpdateMembers( scalefactor );
   
   MC2BoundingBox clipBox;
   clipBox.update( 0, 0 );
   clipBox.update( int(m_height), int(m_width) );
  
   std::vector<MC2Point> clippedPoints( begin, end );

   ClipUtil::clipPolyToBBoxFast( clipBox, clippedPoints );
   
   begin = clippedPoints.begin();
   end = clippedPoints.end();

   mc2dbg << "M_Height: " << m_height << " m_width: " << m_width << endl;
   
   mc2dbg << "3D: tmppoly = [];" << endl;
   for (std::vector<MC2Point>::const_iterator it = begin; it != end; ++it ) {

      MathVec<3> origcoord;
      origcoord[ 0 ] = it->getX();
      origcoord[ 1 ] = it->getY();
      origcoord[ 2 ] = 0;
#if 0

      vec_t x = m_Pa*m_Va*origcoord(0) + m_Pa*m_Vb*origcoord(1) + m_Pa*m_Vc*origcoord(2) + m_Pa*m_Vd;
      vec_t y = m_Pb*m_Ve*origcoord(0) + m_Pb*m_Vf*origcoord(1) + m_Pb*m_Vg*origcoord(2) + m_Pb*m_Vh;
      vec_t z = m_Pc*m_Vi*origcoord(0) + m_Pc*m_Vj*origcoord(1) + m_Pc*m_Vk*origcoord(2) + (m_Pc*m_Vl + m_Pd);
      vec_t w = -m_Vi*origcoord(0)   -m_Vj*origcoord(1)   -m_Vk*origcoord(2)   -m_Vl;

#else
      // Known things: m_Vb = 0, m_Vc = 0, m_Ve = 0, m_Vi = 0, m_Va -1, origcoord[ 2 ] = 0

      vec_t x = -m_Pa*origcoord(0) /*m_Pa*m_Va*origcoord(0)*/ /*+ m_Pa*m_Vb*origcoord(1) + m_Pa*m_Vc*origcoord(2)*/ + m_Pa*m_Vd;
      vec_t y = m_Pb*m_Ve*origcoord(0) + m_Pb*m_Vf*origcoord(1) /*+ m_Pb*m_Vg*origcoord(2)*/ + m_Pb*m_Vh;
//      vec_t z = m_Pc*m_Vi*origcoord(0) + m_Pc*m_Vj*origcoord(1) /*+ m_Pc*m_Vk*origcoord(2)*/ + (m_Pc*m_Vl + m_Pd);
      vec_t w = /*-m_Vi*origcoord(0)*/   -m_Vj*origcoord(1)   /*-m_Vk*origcoord(2)*/   -m_Vl;

#endif

      
//      if ( fabs(x) >= 1 || fabs(y) >= 1 || fabs(z) >= 1 ) {
//         mc2dbg << "Outside [-1,1]^3 :  x " << x << ",y " << y << ",z " << z << endl;
//      }
     
      if ( w == 0 ) {
         MC2_ASSERT( false );
         continue;
      }
      x = x/w;
      y = y/w;
//      z = z/w;
      
      float xw = m_width*(-x+1)/2;
      float yw = m_horizHeight + (m_height - m_horizHeight)*(-y+2)/2;

      if ( w < 0 ) {
         mc2dbg << "w " << w << endl;
      }

      MC2Point p( static_cast<int>(rint(xw)), static_cast<int>(rint(yw)) );

      res.push_back( p >> scalefactor );
   }
   mc2dbg << "3D: newpolys = append( newpolys, tmppoly );" << endl;  
}


