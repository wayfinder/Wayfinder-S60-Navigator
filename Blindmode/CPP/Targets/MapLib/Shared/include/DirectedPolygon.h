/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DIRECTEDPOLYGON_H
#define DIRECTEDPOLYGON_H

#include "config.h"

#include "UserDefinedFeature.h"



class ScreenOrWorldCoordinate;


/**
 *   Represents a polygon that can be filled or unfilled.
 */
class DirectedPolygon : public UserDefinedFeature {
public:
   /**
    *   Creates a new DirectedPolygon with the supplied
    *   points. It will be rotated around zero so...
    *   @param points        The points that define the polygon.
    *   @param centerCoord   The point on the screen where the (0,0)-point
    *                        should be drawn.
    *   @param filled        True if the polygon should be filled.
    *   @param color24bit    Color of the polygon fill if filled and pen
    *                        if not filled.
    *   @param penSize       Size of pen.
    */
   DirectedPolygon( const std::vector<MC2Point>& points,
                    const ScreenOrWorldCoordinate& centerCoord,
                    int filled,
                    uint32 color24bit,
                    int penSize = 1,
                    int dpiCorrectionFactor = 1);

   /**
    *   Sets the angle of the polygon.
    *   @param angleDegs Angle in degrees. If fromNorth is false, the
    *                    angle is the usual mathematical CCW angle (on screen)
    *                    and if fromNorth is true, the angle will be from
    *                    north and CW and rotate with the map.
    */
   void setAngle(float angleDegs, bool fromNorth = true);

   /**
    *   Returns the color of the polygon.
    */
   inline uint32 getColor() const;

   /**
    *   Returns true if the polygon is filled.
    */
   inline int isFilled() const;

   /**
    *   Returns the pensize of the polygon.
    */
   inline int getPenSize() const;
   
private:  
   int m_filled;
   uint32 m_color24;
   int m_penSize;
};


inline uint32
DirectedPolygon::getColor() const
{
   return m_color24;
}

inline int
DirectedPolygon::isFilled() const
{
   return m_filled;
}

inline int
DirectedPolygon::getPenSize() const
{
   return m_penSize;
}

#endif
