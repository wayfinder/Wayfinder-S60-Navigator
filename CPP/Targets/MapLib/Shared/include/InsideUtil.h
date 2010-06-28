/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef INSIDEUTIL_H
#define INSIDEUTIL_H

/**
 *    Contains static method for determining if a point is inside a polygon.
 */
class InsideUtil {
   public:
      /**
       *    Tests if a point is left/on/right of an infinite line.
       *    @param   p0 The first point of the infinite line.
       *    @param   p1 The second point of the infinte line.
       *    @param   p2 The point to check.
       *
       *    @return >0 for P2 left of the line through P0 and P1
       *            =0 for P2 on the line
       *            <0 for P2 right of the line.
       */
      template<class P, class XY_HELPER>
      static int isLeft( const P& p0, const P& p1, const P& p2, 
                         const XY_HELPER& helper )
      {
         double res = double( helper.getX( p1 ) - helper.getX( p0 ) ) * 
                            ( helper.getY( p2 ) - helper.getY( p0 ) ) -
                      double( helper.getX( p2 ) - helper.getX( p0 ) ) * 
                            ( helper.getY( p1 ) - helper.getY( p0 ) );
       
         if ( res > 0.5 ) {
            return 1;
         } else if ( res < -0.5 ) {
            return -1;
         } else {
            return 0; 
         }
      }

      /**
       *    Tests if a point is left/on/right of an infinite line,
       *    only using integer operations. I.e. use this method
       *    when the input data is integers and limited so no 
       *    overflows can occur.
       *
       *    @param   p0 The first point of the infinite line.
       *    @param   p1 The second point of the infinte line.
       *    @param   p2 The point to check.
       *
       *    @return >0 for P2 left of the line through P0 and P1
       *            =0 for P2 on the line
       *            <0 for P2 right of the line.
       */
      template<class P, class XY_HELPER>
      static int isLeft_int( const P& p0, const P& p1, const P& p2,
                            const XY_HELPER& helper ) 
      {
         int res = ( helper.getX( p1 ) - helper.getX( p0 ) ) * 
                   ( helper.getY( p2 ) - helper.getY( p0 ) ) -
                   ( helper.getX( p2 ) - helper.getX( p0 ) ) * 
                   ( helper.getY( p1 ) - helper.getY( p0 ) );
       
         if ( res > 0 ) {
            return 1;
         } else if ( res < 0 ) {
            return -1;
         } else {
            return 0; 
         }
      }

      /**
       *    Tests if a point is left/on/right of an infinite line.
       *    Uses the default XYHelper of the
       *    coordinate class sent in, for custom XYHelper, see
       *    the other inside function.
       *    @param   p0 The first point of the infinite line.
       *    @param   p1 The second point of the infinte line.
       *    @param   p2 The point to check.
       *
       *    @return >0 for P2 left of the line through P0 and P1
       *            =0 for P2 on the line
       *            <0 for P2 right of the line.
       */
      template<class P>
      static int isLeft( const P& p0, const P& p1, const P& p2 ) { 
         typename P::XYHelper helper;
         return isLeft( p0, p1, p2, helper );
      }
      
      /**
       *    Tests if a point is left/on/right of an infinite line,
       *    only using integer operations. I.e. use this method
       *    when the input data is integers and limited so no 
       *    overflows can occur.
       *    Uses the default XYHelper of the
       *    coordinate class sent in, for custom XYHelper, see
       *    the other inside function.
       *    @param   p0 The first point of the infinite line.
       *    @param   p1 The second point of the infinte line.
       *    @param   p2 The point to check.
       *
       *    @return >0 for P2 left of the line through P0 and P1
       *            =0 for P2 on the line
       *            <0 for P2 right of the line.
       */
      template<class P>
      static int isLeft_int( const P& p0, const P& p1, const P& p2 ) { 
         typename P::XYHelper helper;
         return isLeft_int( p0, p1, p2, helper );
      }

      /**
       *    Contains static method for determining if a point is 
       *    inside a polygon. Use the other inside if you have an
       *    MC2Coordinate or MC2Point, since they have XYHelpers
       *    defined in their class definition.
       *  
       *    @param   begin    Iterator of points containing 
       *                      the beginning of the polygon.
       *    @param   end      End iterator of the polygon, 
       *                      i.e. one position after the last point.
       *    @param   p        The point to check.
       *    @param   helper   The coordinate helper class for the point. 
       *                      Usually P::XYHelper will work.
       *    @return  True if the point was inside the polygon, 
       *             false otherwise.
       */
      template<class POINT_ITERATOR, class P, class XY_HELPER>
      static bool inside( const POINT_ITERATOR& begin,
                          const POINT_ITERATOR& end, 
                          const P& p, 
                          const XY_HELPER& helper )
      {
         int wn = 0;    // the winding number counter
        
         // loop through all edges of the polygon
         POINT_ITERATOR it = begin;
         POINT_ITERATOR prevIt = it;
         ++it;

         bool closePolygon = false;
         
         while( it != end || closePolygon) {
            if(closePolygon) {
               //Close the polygon
               it = begin;
            }
            
            // edge from V[i] to V[i+1]
            if ( helper.getY( *prevIt ) <= 
                 helper.getY( p ) ) {   // start y <= P.y
               if ( helper.getY( *it ) > 
                    helper.getY( p ) )  // an upward crossing
                  if (InsideUtil::isLeft( 
                           *prevIt, *it, p, helper ) > 0)// P left of edge
                     ++wn;              // have a valid up intersect
            } else {                    // start y > P.y (no test needed)
               if ( helper.getY( *it ) <= 
                    helper.getY( p ) )       // a downward crossing
                  if (InsideUtil::isLeft( 
                           *prevIt, *it, p, helper ) < 0)// P right of edge
               --wn;                        // have a valid down intersect
            }
            
            prevIt = it;

            //Now we've finished closing pass of polygon, so exit.
            if(closePolygon) {
               break;
            }

            //If the last point does not close the polygon,
            //we need to do an additional pass after this.
            if(!closePolygon && (it + 1 ) == end) {
               if(*it != *begin) {
                  closePolygon = true;
               }
            }

            ++it;
         }
         // Odd winding number means it's inside.
         return wn & 1;
      }
      
      /**
       *    Contains static method for determining if a point is 
       *    inside a polygon. Uses the default XYHelper of the
       *    coordinate class sent in, for custom XYHelper, see
       *    the other inside function.
       *    <br />
       *    Example:
       *    const vector<MC2Coordinate>& vectorOfCoordinates = something();
       *    InsideUtil::inside(vectorOfCoordinates.begin(),
       *                       vectorOfCoordinates.end(),
       *                       MC2Coordinate(21123,123213) );
       *
       *    @param   begin    Iterator of points containing 
       *                      the beginning of the polygon.
       *    @param   end      End iterator of the polygon, 
       *                      i.e. one position after the last point.
       *    @param   p        The point to check.
       *    @return  True if the point was inside the polygon, 
       *             false otherwise.
       */
      template<class POINT_ITERATOR, class P>
      static bool inside( const POINT_ITERATOR& begin,
                          const POINT_ITERATOR& end, 
                          const P& p ) {
         typename P::XYHelper helper;
         return inside(begin, end, p, helper);
      }

};


#endif

