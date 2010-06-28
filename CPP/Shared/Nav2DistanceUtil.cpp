/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Nav2DistanceUtil.h"
#include <math.h>

#define EARTH_RADIUS 6378137.0
#define NAV2_COORD_RAD_FACTOR 100000000
#define SCALE_TO_CENTIMETER (EARTH_RADIUS / NAV2_COORD_RAD_FACTOR * 100)


#define SQUARE(a) ((a)*(a))

uint32
Nav2DistanceUtil::calcDistanceCentimeters( int32 lat1, int32 lon1, 
                                           int32 lat2, int32 lon2 )
{
   float64 cosLat = cos( float64((lat1 + lat2)/2) / 
                         NAV2_COORD_RAD_FACTOR );
   float64 dlatsq = SQUARE( (float64)  (lat2 - lat1) );
   float64 dlonsq = SQUARE( ((float64) (lon2 - lon1)) * cosLat);
   
   return uint32( sqrt(dlatsq + dlonsq) * SCALE_TO_CENTIMETER );
}


