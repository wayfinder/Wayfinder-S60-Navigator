/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "PolyLine.h"
#include "MC2Point.h"
#include <math.h>
#include "MathUtility.h"

void PolyLine::calculatePolyLine(const MC2Point& p0, 
                                 const MC2Point& p1,
                                 int widthInPixels,
                                 std::vector<MC2Point>& result,
                                 MC2Point& factor)
{
   // Get the half width
   float d = widthInPixels >> 1;
   // Calc the v coord
   MC2Point v = p1 - p0;
   // Calc the inverse denominator
   float invDenom = MathUtility::fastInvSqrt( ( ( v.getX() * v.getX() ) + ( v.getY() * v.getY() ) ) );
   // Calc the w coord
   MC2Point w( ( static_cast<int> (rint( ( d * v.getY() ) * invDenom ) )), 
               ( static_cast<int> (rint( ( ( -d ) * v.getX() ) * invDenom ) ) ));
   // Reserve space for the points
   result.reserve( 4 );
   // Calc first point p0 + w
   result.push_back( p0 + w );
   // Calc second point p0 - w
   result.push_back( p0 - w );
   // Calc third point p1 - v
   result.push_back( p1 - w );
   // Calc fourth point p1 + v
   result.push_back( p1 + w );
   // Calc the end factor
   factor.getX() = -w.getY();
   factor.getY() = w.getX(); 
//    factor = MC2Point( -w.getY(), w.getX() );
//    mc2dbg << result[0] << result[1] << result[2] << result[3] << endl;
//    mc2dbg << "factor = " << factor << endl;
}
