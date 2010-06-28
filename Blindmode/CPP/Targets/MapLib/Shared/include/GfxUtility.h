/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef GFXUTILITY_H
#define GFXUTILITY_H

#include "config.h"
#include <vector>
#include <list>

class MC2Direction;
class MC2Coordinate;


// /**
//  *    Define the MC2BoundingBox for the getAdjacentRectangle-method and
//  *    the isSegmentInMC2BoundingBox-method.
//  *    MC2BoundingBox.h is included in the cpp-file.
//  */
// class MC2BoundingBox;


/**
 *    This static class contains a lot of utility-methods about 
 *    geographical data.
 *    
 *    @see     Utility.h Non-geographic utility-methods.
 */    
class GfxUtility {
public:


   /**
    *    Get the angle from the north-direction. Used to calculate 
    *    the angle described by the figure:
    *    @verbatim
    (lat2, lon2)
    |     /
    |    /
    |   /
    |A /
    |\/
    |/ (lat1, lon1)
    |
    |
    |
    @endverbatim
    *
    *    @param   lat1  The latitude of the startpoint for the line.   
    *    @param   lon1  The longitude of the startpoint for the line.   
    *    @param   lat2  The latitude of the endpoint for the line.   
    *    @param   lon2  The longitude of the endpoint for the line.   
    *    @return  Angle in radians from the north-direction in 
    *             clockwise direction. The return value is in the 
    *             interval $[0,2\pi[$.
    */
   static float64 getAngleFromNorthRadians( int32 lat1, int32 lon1, 
                                            int32 lat2, int32 lon2 );


   /**
    * Same as above but the result is in degrees rather than in
    * radians.
    *    @param   lat1  The latitude of the startpoint for the line.   
    *    @param   lon1  The longitude of the startpoint for the line.   
    *    @param   lat2  The latitude of the endpoint for the line.   
    *    @param   lon2  The longitude of the endpoint for the line.   
    *    @return  Angle in radians from the north-direction in 
    *             clockwise direction. The return value is in the 
    *             interval $[0,2\pi[$.
    */
   static float64 getAngleFromNorthDegrees( int32 lat1, int32 lon1, 
                                            int32 lat2, int32 lon2 );

   /**
    *		Wrapper function for getAngleFromNorthDegrees.
    *
    *    @param   lat1  The latitude of the startpoint for the line.   
    *    @param   lon1  The longitude of the startpoint for the line.   
    *    @param   lat2  The latitude of the endpoint for the line.   
    *    @param   lon2  The longitude of the endpoint for the line.
    *    		
    *		@return 	MC2Direction object which represents the direction
    *					the user would have to move in to get from
    *					(lat1, lon1) to (lat2, lon2)
    *
    */
   
   static MC2Direction getDirection( int32 lat1, int32 lon1, 
                                     int32 lat2, int32 lon2);

   
   /**
    *    Calculates cosine for the latitude.
    */

   static double getCosLat( int32 lat );

   
   /**
   *    Find the intersection between two lines that can be described
   *    by the following coordinates:
   *    Line 1: (x1, y1) - (x2, y2) and \
   *    Line 2: (v1, w1) - (v2, w2).
   *    @param x1   x1 coordinate of line 1.
   *    @param y1   y1 coordinate of line 1.
   *    @param x2   x2 coordinate of line 1.
   *    @param y2   y2 coordinate of line 1.
   *
   *    @param v1   v1 coordinate of line 2.
   *    @param w1   w1 coordinate of line 2.
   *    @param v2   v2 coordinate of line 2.
   *    @param w2   w2 coordinate of line 2.
   *
   *    @param intersectX Output parameter set to the x coordinate
   *                      of the intersection of the two lines.
   *    @param intersectY Output parameter set to the y coordinate
   *                      of the intersection of the two lines.
   *    @return  True if the two lines intersects each other,
   *             false otherwise.
   */
   static bool getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                               int32 v1, int32 w1, int32 v2, int32 w2,
                               int32& intersectX, int32& intersectY);

   /**
   *    Find the intersection between two lines that can be described
   *    by the following coordinates:
   *    Line 1: (x1, y1) - (x2, y2) and \
   *    Line 2: (v1, w1) - (v2, w2).
   *    @param x1   x1 coordinate of line 1.
   *    @param y1   y1 coordinate of line 1.
   *    @param x2   x2 coordinate of line 1.
   *    @param y2   y2 coordinate of line 1.
   *
   *    @param v1   v1 coordinate of line 2.
   *    @param w1   w1 coordinate of line 2.
   *    @param v2   v2 coordinate of line 2.
   *    @param w2   w2 coordinate of line 2.
   *
   *    @param intersectX Output parameter set to the x coordinate
   *                      of the intersection of the two lines.
   *    @param intersectY Output parameter set to the y coordinate
   *                      of the intersection of the two lines.
   *    @param percentFromx1y1 [Output] How many percent between 
   *                           (x1,y1) and (x2,y2) 
   *                           the intersection lies. 
   *    @param percentFromv1w1 [Output] How many percent between 
   *                           (v1,w1) and (v2,w2) 
   *                           the intersection lies. 
   *    @return  True if the two lines intersects each other,
   *             false otherwise.
   */
   static bool getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                               int32 v1, int32 w1, int32 v2, int32 w2,
                               int32& intersectX, int32& intersectY,
                               float64& percentFromx1y1,
                               float64& percentFromv1w1 );



   /**
   *    Find the intersection between two lines that can be described
   *    by the following coordinates:
   *    Line 1: (x1, y1) - (x2, y2) and \
   *    Line 2: (v1, w1) - (v2, w2).
   *    @param x1   x1 coordinate of line 1.
   *    @param y1   y1 coordinate of line 1.
   *    @param x2   x2 coordinate of line 1.
   *    @param y2   y2 coordinate of line 1.
   *
   *    @param v1   v1 coordinate of line 2.
   *    @param w1   w1 coordinate of line 2.
   *    @param v2   v2 coordinate of line 2.
   *    @param w2   w2 coordinate of line 2.
   *
   *    @param intersectX Output parameter set to the x coordinate
   *                      of the intersection of the two lines.
   *    @param intersectY Output parameter set to the y coordinate
   *                      of the intersection of the two lines.
   *    @param percentFromx1y1 [Output] If not NULL, 
   *                           how many percent between 
   *                           (x1,y1) and (x2,y2) 
   *                           the intersection lies. 
   *    @param percentFromv1w1 [Output] If not NULL,
   *                           how many percent between 
   *                           (v1,w1) and (v2,w2) 
   *                           the intersection lies. 
   *    @return  True if the two lines intersects each other,
   *             false otherwise.
   */
   static bool getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                               int32 v1, int32 w1, int32 v2, int32 w2,
                               int32& intersectX, int32& intersectY,
                               float64* percentFromx1y1,
                               float64* percentFromv1w1 );

      
      /**
       *    Get the distance between two points.
       *    @return  The squared linear distance between two points.
       *             In meters squared.
       */
      static float64 squareP2Pdistance_linear(int32 lat1, int32 lon1, 
                                              int32 lat2, int32 lon2,
                                              float64 cosLat = 2.0);

      /**
       *    Get the distance between two points.
       *    @return  The squared linear distance between two points.
       *             In meters squared.
       */
      static float64 squareP2Pdistance_linear( const MC2Coordinate& p1,
                                               const MC2Coordinate& p2,
                                               float64 cosLat = 2.0 );

   
};

#endif
