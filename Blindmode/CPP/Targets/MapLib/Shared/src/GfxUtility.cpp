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
#include "GfxUtility.h"
#include "GfxConstants.h"
#include <math.h>
#include "MC2Coordinate.h"
#include "MC2Direction.h"

#define SQUARE(a) ((a)*(a))

// XXX: Temporarily disable since doesn't compile.
#if 0

uint64
GfxUtility::closestDistVectorToPoint(int32 x1, int32 y1,
                                     int32 x2, int32 y2,
                                     int32 xP, int32 yP,
                                     int32& xOut, int32& yOut,
                                     double cosF) 
{
   uint64 dist;
   
   //ex and ey are unitvectors
   //
   //Vector V1 = (x2 - x1) * ex + (y2 - y1) * ey
   //Vector V2 = (xP -Removed all  x1) * ex + (yP - y1) * ey
   
   //Calculate the scalarproduct V1 * V2
   int64 scalarProd = int64(cosF * (x2 - x1) * cosF * (xP - x1) + 
                            double(y2 - y1) * double(yP - y1));
   
   if ( ( x1 - x2 == 0 ) && ( y1 == y2 ) ) {
      DEBUG8(cerr << "   CASE 0" << endl);
      //V1 has a length of zero, use plain pythagoras
      dist = SQUARE(int64(cosF * (xP - x1))) + SQUARE(int64(yP - y1) );
      xOut = x1;
      yOut = y1;      
   } else if ( scalarProd < 0 ) {
      DEBUG8( cerr << "   CASE 1" << endl );
      //The angle between V1 and V2 is more than pi/2,
      //this means that (xP, yP) is closest to (x1, y1) 
      dist = SQUARE( int64( cosF * ( xP - x1 ) ) ) + SQUARE( int64( yP - y1 ) );
      xOut = x1;
      yOut = y1;
   } else if ( scalarProd > int64( SQUARE( int64( cosF * ( x2 - x1 ) ) ) + 
                                   SQUARE( int64( y2 - y1 ) ) ) ) {
      DEBUG8( cerr << "   CASE 2" << endl );
      //The scalarproduct V1 * V2 is larger than the length of V1
      //this means that (xP, yP) is closest to (x2, y2) 
      dist = SQUARE( int64( cosF * ( xP - x2 ) ) ) + SQUARE( int64( yP - y2 ) );
      xOut = x2;
      yOut = y2;
   } else {
      //Use the formula for minimumdistance from a point to a line
      //dist = ( V2x * V1y - V2y * V1x )^2 / ( (V1x ^ 2) + (V1y ^ 2) )
      dist = uint64( SQUARE( double( y2 - y1 ) * cosF * ( xP - x1 ) + 
                             cosF * ( x1 - x2 ) * double( yP - y1 ) ) / 
                     ( SQUARE( double( y2 - y1 ) ) + SQUARE( cosF * ( x1 - x2 ) ) ) );
      
      // Find intersection point (xOut,yOut) by writing the equations
      // of the two lines L1 and L2 defined below and solving the
      // upcoming (2*2) equation system .
      // "k" is the slope of the line L1 between (x1,y1) and (x2,y2),
      //  k = (y1 - y2)/(cosF*(x1 - x2));
      //  Since the line L2, between (xP,yP) and (xOut,yOut), is perpendicular
      //  to L1, the slope of L2 is -1/k (known from High School)
      
      //  |y - y1 = k(x - x1)cosF      ( equation for line between (x1,y1) and (x2,y2) 
      //  |y - yP = -1/k*(x - xP)cosF  ( equation for line between (xP,yP) and (xOut,yOut)
      //
      //  |y - k*cosF*x  = -k*x1*cosF + y1 <=>
      //  |y + cosF/k*x  = cosF/a*xP + yp   <=>
      //
      //  |1  -k*cosF || y |    | -k*x1*cosF + y1 |
      //  |           ||   | =  |                 | 
      //  |1   cosF/k || x |    |  cosF/a*xP + yp |
      //
      //  This is a linear equation system on the form Az = b,
      //  where
      //       |1  -k*cosF |      | y |       | -k*x1*cosF + y1 |
      //  A =  |           | , z =|   | , b = |                 |
      //       |1   cosF/k |      | x |       | cosF/a*xP + yp  |
      //
      //  The solution is obtained as z = inv(A)*b where inv(A) is the inverse of A.
      
      if ( x1 == x2 ) {
         // vertical line between (x1,x2) and (y1,y2)
         DEBUG8( cerr << "   CASE 3.1" << endl );
         xOut = x1;
         yOut = yP;
      } else if ( y1 == y2 ) {
         // horizontal line between (x1,x2) and (y1,y2)
         xOut = xP;
         yOut = y1;
      } else {
         // "lutning" k
         double k = double( y1 - y2 ) / ( cosF*( x1 - x2 ) );    // slope 
         float64 oneOverDeterminant = k / cosF / ( SQUARE( k ) + 1 );
         // after simplification of z = inv(A)*b
         yOut = int32( oneOverDeterminant * ( SQUARE( cosF ) * ( xP - x1 ) + 
                                              cosF / k * y1 + cosF * k * yP ) );
         
         xOut = int32(oneOverDeterminant * ( yP - y1 + cosF / k * xP + 
                                             cosF * k * x1 ) );
         
         DEBUG8( cerr << "   CASE 3.2" << endl );
         
         //xOut = int32(1/(SQUARE(k) + 1)*(SQUARE(k)*x1 + xP + k*(yP-y1) ));
         //yOut = int32(1/(SQUARE(k) + 1)*(y1 + k*cosF*(xP-x1) + SQUARE(k)*yP));
         DEBUG8( cerr << "   k=" << k << endl;
                 cerr << "   (x1,y1)=(" << x1 << "," << y1 << endl;
                 cerr << "   (x2,y2)=(" << x2 << "," << y2 << endl;
                 cerr << "   (xP,yP)=(" << xP << "," << yP << endl;
                 cerr << "   (xOut,yOut)=(" << xOut << "," << yOut << endl;);
      }
   }
   return ( dist ); 
}
#endif																																																																																																			

float64
GfxUtility::getAngleFromNorthRadians(int32 lat1, int32 lon1, 
                                     int32 lat2, int32 lon2)
{
   if ( lat1 == lat2 && lon1 == lon2 ) {
      return 0.0;
   }
   float64 coslat = cos( 
       GfxConstants::invRadianFactor * float64(lat2/2 + lat1/2));
      //Bringing in coslat. Without coslat one meter north and one meter
      //east will in Linköping give 60 degrees and not 45 as expected.
   float64 A = atan2( coslat*(lon2-lon1), (float64) (lat2 - lat1) );
   if (A < 0)
      A += M_PI*2;
   
//   MC2_ASSERT(A<0);
//   MC2_ASSERT(A>M_PI*2);

   MC2_ASSERT( A >= 0 );
   MC2_ASSERT( A <= M_PI*2 );

   return (A);
}

float64 
GfxUtility::getAngleFromNorthDegrees(int32 lat1, int32 lon1, 
                                     int32 lat2, int32 lon2)
{
   return ( getAngleFromNorthRadians(lat1, lon1, lat2, lon2) * (180/M_PI) );
}



bool
GfxUtility::getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                            int32 v1, int32 w1, int32 v2, int32 w2,
                            int32& intersectX, int32& intersectY,
                            float64* percentFromx1y1,
                            float64* percentFromv1w1 )
{
   // Can handle vertical lines.

   // x = x1 + t*alpha
   // y = y1 + t*beta
   // 
   // v = v1 + s*gamma
   // w = w1 + s*theta
   //
   // alpha, beta, gamma, theta as stated below

   int32 alpha    = x2 - x1;
   int32 beta     = y2 - y1;
   int32 gamma    = v2 - v1;
   int32 theta    = w2 - w1;

   int64 denominator = int64(theta)*alpha - int64(gamma)*beta; 

   // Check if the lines are parallell
   if (denominator == 0) {
      return (false);
   }

   // Solve the equation system
   float64 s = float64(int64(y1 - w1)*alpha + int64(v1 - x1)*beta) / 
      float64(denominator);

   float64 t = float64( int64(v1 - x1)*theta + int64(y1 - w1)*gamma ) /
      float64(denominator);


   if ( ( s < 0.0 ) || ( s > 1.0 ) ) {
      mc2dbg8 << "[GU]: s out of range - no intersect" << endl;
      return false;
   }
   if ( ( t < 0.0 ) || ( t > 1.0 ) ) {
      mc2dbg8 << "[GU]: percentFromx1y1 out of range - no intersect" << endl;
      return false;
   }

   if ( percentFromx1y1 ) {
      *percentFromx1y1 = t;
   }
   if ( percentFromv1w1 ) {
      *percentFromv1w1 = s;
   }

   intersectX = int32(v1 + rint(s*gamma));
   intersectY = int32(w1 + rint(s*theta));

   return (true);
}

bool
GfxUtility::getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                            int32 v1, int32 w1, int32 v2, int32 w2,
                            int32& intersectX, int32& intersectY,
                            float64& percentFromx1y1,
                            float64& percentFromv1w1 )
{
   return getIntersection( x1, y1, x2, y2, 
      v1, w1, v2, w2,
      intersectX, intersectY, 
      &percentFromx1y1,
      &percentFromv1w1 );
}

bool
GfxUtility::getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                            int32 v1, int32 w1, int32 v2, int32 w2,
                            int32& intersectX, int32& intersectY)
{
   return getIntersection( x1, y1, 
      x2, y2, 
      v1, w1, 
      v2, w2, 
      intersectX, intersectY, NULL, NULL );
}


float64
GfxUtility::squareP2Pdistance_linear(int32 lat1, int32 lon1, 
                                     int32 lat2, int32 lon2,
                                     float64 cos_lat)
{  
   if (cos_lat >= 1) {
      //Local linearisation being done at the mean latitude.
      cos_lat= cos( (float64) (lat1/2+lat2/2) * M_PI /
                    ((float64) 0x80000000) );
   }
   float64 delta_lat_sq =  SQUARE( (float64) (lat2 - lat1) );
   float64 delta_lon_sq =  SQUARE( ((float64) (lon2-lon1)) * cos_lat);
   return ( (delta_lat_sq + delta_lon_sq) * 
            GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER);
}

float64
GfxUtility::squareP2Pdistance_linear(const MC2Coordinate& p1,
                                     const MC2Coordinate& p2,
                                     float64 cos_lat )
{
   return squareP2Pdistance_linear( p1.lat, p1.lon,
                                    p2.lat, p2.lon,
                                    cos_lat );
}

MC2Direction
GfxUtility::getDirection( int32 lat1, int32 lon1, 
                          int32 lat2, int32 lon2 )
{
   float64 angle = getAngleFromNorthDegrees( lat1, lon1,
                                         lat2, lon2);

   int angleInt = static_cast<int> ( angle + 0.5f );

   return MC2Direction(angleInt);
}

double GfxUtility::getCosLat(int32 lat)
{
   static const double mc2scaletoradians = 1.0 / (11930464.7111*
                                                  180.0/3.14159265358979323846);
   return cos(mc2scaletoradians*lat);
}
