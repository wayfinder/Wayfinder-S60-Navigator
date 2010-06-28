/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef POLY_LINE_H
#define POLY_LINE_H

#include "config.h"

#include <vector>

#ifdef __SYMBIAN32__
#include <e32math.h>
#else
#include <math.h>
#endif


class MC2Point;

/**
 * Class that calculates a rect from to points.
 */
class PolyLine 
{
 public:
   /**
    * Calculates the coords for a polygon given
    * two coords, with a given width.
    * @param p0 The start coord.
    * @param p1 The end coord.
    * @param widthInPixels The width of the polygon.
    * @param result A vector holding the four points that
    *               that will result in a polygon.
    * @param factor A factor that makes it possible to draw a
    *               ending or starting polygon at the end or
    *               at the start of the main polygon. This can
    *               be used to draw a starting or ending triangle.
    */
   static void calculatePolyLine(const MC2Point& p0, 
                                 const MC2Point& p1,
                                 int widthInPixels,
                                 std::vector<MC2Point>& result,
                                 MC2Point& factor);

};
#endif
