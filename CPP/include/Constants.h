/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef ISAB_CONSTANTS_H
#define ISAB_CONSTANTS_H
#include "machine.h"
namespace isab{
   /** All constants of a real-world nature should be placed here.*/
   namespace Constants{
      /** Pi * 1e8. */
      const int32 piE8 = 314159265;
      const int32 latRadE8Max =  piE8/2;
      const int32 latRadE8Min = -piE8/2;
      const int32 lonRadE8Max =  piE8;
      const int32 lonRadE8Min = -piE8;
      /// Pi as a double
      const double pi = 3.14159265358979323846;
      /// 2^32
      const double pow2_32 = 4294967296.0;
      /// Radius of the earth in meters.
      const int32 earthRadius = 6378137;
      /// Earth Circumference in meters. 2*pi*earthRadius
      const int32 earthCircumference = 40075017;
   }

   /** Tests if the supplied coordinates are within the coordinate
    * system covering the world.
    * @param lat             the latitude in rad * 1e8;
    * @param lon             the longitude in rad * 1e8;
    * @return true if (lat, lon) are within ([-pi/2, pi/2], [-pi, pi]).
    */
   inline bool onTheWorld(int32 lat, int32 lon)
   {
//       int32 latMax = Constants::latRadE8Max;
//       int32 latMin = Constants::latRadE8Min; 
//       int32 lonMax = Constants::lonRadE8Max; 
//       int32 lonMin = Constants::lonRadE8Min;
//       bool ret = true;
//       ret = ret && (lat <= latMax); 
//       ret = ret && (lat >= latMin); 
//       ret = ret && (lon <= lonMax); 
//       ret = ret && (lon >= lonMin);
//       return ret;

      return (lat <= Constants::latRadE8Max) 
         &&  (lat >= Constants::latRadE8Min) 
         &&  (lon <= Constants::lonRadE8Max) 
         &&  (lon >= Constants::lonRadE8Min);
   }

   inline bool validCoordinate(int32 lat, int32 lon)
   {
      if (lat == 0 && lon == 0) {
         return false;
      }
      return (onTheWorld(lat, lon));
   }

}
#endif
