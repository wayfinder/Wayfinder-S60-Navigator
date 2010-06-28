/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxUtility.h"
#include "GfxConstants.h"
#include "MC2Coordinate.h"
#include <algorithm>
#include <math.h>

#undef SQUARE
#define SQUARE(x) ((x)*(x))

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

float64
GfxUtility::rtP2Pdistance_linear(int32 lat1, int32 lon1, 
                                 int32 lat2, int32 lon2,
                                 float64 cos_lat)
{
   return sqrt( GfxUtility::squareP2Pdistance_linear( lat1, lon1,
                                                      lat2, lon2,
                                                      cos_lat ) );
}

float64
GfxUtility::rtP2Pdistance_linear(const MC2Coordinate& p1,
                                 const MC2Coordinate& p2,
                                 float64 cos_lat )
{
   return sqrt( GfxUtility::squareP2Pdistance_linear( p1.lat, p1.lon,
                                                      p2.lat, p2.lon,
                                                      cos_lat ) );
}


float64
GfxUtility::getAngleFromNorthRadians(int32 lat1, int32 lon1, 
                                     int32 lat2, int32 lon2,
                                     float64 cosLat)
{
   if ( lat1 == lat2 && lon1 == lon2 ) {
      return 0.0;
   }

   //Bringing in coslat. Without coslat one meter north and one meter
   //east will in Linköping give 60 degrees and not 45 as expected.
   float64 A = atan2( cosLat*(lon2-lon1), (float64) (lat2 - lat1) );
   if (A < 0)
      A += M_PI*2;
   
//   MC2_ASSERT(A<0);
//   MC2_ASSERT(A>M_PI*2);

// MC2_ASSERT( A >= 0 );
// MC2_ASSERT( A <= M_PI*2 );

   return (A);
}

float64 
GfxUtility::getAngleFromNorthDegrees(int32 lat1, int32 lon1, 
                                     int32 lat2, int32 lon2,
                                     float64 cosLat)
{
   return getAngleFromNorthRadians(lat1, lon1, lat2, lon2, cosLat) * (180/M_PI);
}

