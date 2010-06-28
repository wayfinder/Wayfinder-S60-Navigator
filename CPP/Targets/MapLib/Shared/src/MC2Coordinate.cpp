/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "MC2Coordinate.h"
#include "GfxConstants.h"

// -- WGS84Coordinate.
WGS84Coordinate::WGS84Coordinate( double latDeg, double lonDeg ) :
   latDeg( latDeg ), lonDeg( lonDeg ) 
{
   
}

WGS84Coordinate::WGS84Coordinate( const MC2Coordinate& mc2Coord )
{
   const double invDegreeFactor = 1.0 / 11930464.7111;
   latDeg = mc2Coord.lat * invDegreeFactor;
   lonDeg = mc2Coord.lon * invDegreeFactor;
}

WGS84Coordinate::operator MC2Coordinate() const
{
   MC2Coordinate mc2Coord;
   const double degreeFactor = 11930464.7111; // 2^32/360
   
   float64 addLat = ( (latDeg < 0) ? -1: 1) * 0.5;
   float64 addLon = ( (lonDeg < 0) ? -1: 1) * 0.5;
   
   float64 mc2LatFloat = latDeg * degreeFactor + addLat;
   float64 mc2LonFloat = lonDeg * degreeFactor + addLon;
  
   while ( mc2LonFloat > float( MAX_INT32 ) ) {
      mc2LonFloat -= MAX_UINT32;
   }
   while ( mc2LonFloat < float( MIN_INT32 ) ) {
      mc2LonFloat += MAX_UINT32;
   }
   
   mc2Coord.lat = int( mc2LatFloat );
   mc2Coord.lon = int( mc2LonFloat );

   return mc2Coord;
}

// -- Nav2Coordinate

Nav2Coordinate::Nav2Coordinate( const MC2Coordinate& realCoord ) {
   if ( realCoord.isValid() ) {
      nav2lat = 
         int32(realCoord.lat * GfxConstants::invRadianFactor * 100000000 );
      nav2lon = 
         int32(realCoord.lon * GfxConstants::invRadianFactor * 100000000 );
   } else {
      nav2lat = MAX_INT32;
      nav2lon = MAX_INT32;
   }
}

// -- MC2Coordinate

//static const MC2Coordinate
//MC2Coordinate::invalidCoordinate( MAX_INT32, MAX_INT32 );

#ifdef MC2_SYSTEM
const MC2Coordinate
MC2Coordinate::invalidCoordinate( MAX_INT32, MAX_INT32 );
#endif

MC2Coordinate::MC2Coordinate( const Nav2Coordinate& nav2 ) {
   if ( nav2.nav2lat != MAX_INT32 && uint32(nav2.nav2lat) != MAX_UINT32 ) {
      lat = int32( nav2.nav2lat * GfxConstants::radianFactor / 100000000 );
      lon = int32( nav2.nav2lon * GfxConstants::radianFactor / 100000000 );
   } else {
      lat = MAX_INT32;
      lon = MAX_INT32;
   }
}
