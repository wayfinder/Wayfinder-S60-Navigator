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

#include "ClipUtil.h"

using namespace std;

void
ClipUtil::clipSegment( const MC2Point& prevVertex,
                       const MC2Point& currVertex,
                       int prevInside,
                       int currInside,
                       byte currOutcode,
                       const MC2BoundingBox* bbox,
                       byte boundaryOutcode,
                       vector<byte>& resOutcodes,
                       vector<MC2Point>& resVertices )
{
   // 1) prevVertex inside, currVertex inside: output currVertex
   // 2) prevVertex inside, currVertex outside: output intersection
   // 3) prevVertex outside, currVertex outside: no output
   // 4) prevVertex outside, currVertex inside:
   //                        output currVertex and intersection
   
   if (prevInside && currInside) {
      // Case 1
      // 1) prevVertex inside, currVertex inside: output currVertex
      resVertices.push_back( currVertex );
      resOutcodes.push_back( currOutcode );
      mc2dbg8 << "Case 1: " << "(" << currVertex.getY() << ","
              << currVertex.getX()
              << ") " << endl;
   } else if (prevInside && !currInside) {
      // Case 2
      // 2) prevVertex inside, currVertex outside: output intersection
      MC2Point intersection(0,0);
      bbox->getIntersection( prevVertex.getY(), prevVertex.getX(),
                             currVertex.getY(), currVertex.getX(),
                             boundaryOutcode,
                             (int32&)intersection.getY(),
                             (int32&)intersection.getX() );
      resVertices.push_back( intersection );
      resOutcodes.push_back(
         bbox->getCohenSutherlandOutcode( intersection.getY(),
                                          intersection.getX() ));
      mc2dbg8 << "Case 2: " << "(" << prevVertex.getY() << ","
              << prevVertex.getX()
              << ") " << " % " << "("
              << currVertex.getY() << "," << currVertex.getX()
              << ") " << " -> " << "(" << intersection.getY() << ","
              << intersection.getX() << ") " << endl;
   } else if (!prevInside && currInside) {
      // Case 4
      // 4) prevVertex outside, currVertex inside:
      //                        output intersection and currVertex
      
      MC2Point intersection(0,0);
      bbox->getIntersection( prevVertex.getY(), prevVertex.getX(),
                             currVertex.getY(), currVertex.getX(),
                             boundaryOutcode,
                             (int32&)intersection.getY(),
                             (int32&)intersection.getX() );
      resVertices.push_back( intersection );
      resOutcodes.push_back(
         bbox->getCohenSutherlandOutcode( intersection.getY(),
                                          intersection.getX() ));
      resVertices.push_back( currVertex );
      resOutcodes.push_back( currOutcode );
      mc2dbg8 << "Case 4: " << "(" << prevVertex.getY() << ","
              << prevVertex.getX()
              << ") " << " % " << "(" << currVertex.getY()
              << "," << currVertex.getX()
              << ") " << " -> " << "(" << intersection.getY() << ","
              << intersection.getX() << ") "<< "("
              << currVertex.getY() << ","
              << currVertex.getX() << ") " << endl;
   }  // else (!prevInside && !currInside) 
   // Case 3
   // 3) prevVertex outside, currVertex outside: no output             
}


int
ClipUtil::clipToBoundary( const byte boundaryOutcode,
                          const MC2BoundingBox* bbox,
                          vector<MC2Point>& vertices,
                          vector<byte>& outcodes,
                          vector<MC2Point>& resVertices,
                          vector<byte>& resOutcodes  )  {
   // This method is a block in the Sutherland-Hodgeman 
   // polygon clipping pipeline.
   
   // A polygon must consist of at least three vertices.
   if (vertices.size() < 2) {
      resVertices.clear();
      return true;
   }
   
   resVertices.reserve(vertices.size());
   resOutcodes.reserve(outcodes.size());
   
   // Previous outcode
   vector<byte>::const_iterator prevOcIt = outcodes.begin();
   int prevInside = (((*prevOcIt) & boundaryOutcode) == 0);
   
   // Current outcode
   vector<byte>::const_iterator currOcIt = outcodes.begin();
   ++currOcIt;
   
   // Previous vertex
   vector<MC2Point>::const_iterator prevVxIt = vertices.begin();
   // Current vertex
   vector<MC2Point>::const_iterator currVxIt = vertices.begin();
   ++currVxIt;
   for ( ; currVxIt != vertices.end(); ++currVxIt ) {
      int currInside = (((*currOcIt) & boundaryOutcode) == 0);
      
      clipSegment( *prevVxIt, *currVxIt, prevInside, currInside,
                   *currOcIt, bbox, boundaryOutcode,
                   resOutcodes, resVertices );
      
      // Update prevVxIt
      ++prevVxIt;
      
      // Update prevInside
      prevInside = currInside;
      ++currOcIt;
   }
   
   // The same thing for the last edge:
   currOcIt = outcodes.begin();
   currVxIt = vertices.begin();
   int currInside = (((*currOcIt) & boundaryOutcode) == 0);
   clipSegment( *prevVxIt, *currVxIt, prevInside, currInside,
                *currOcIt, bbox, boundaryOutcode,
                resOutcodes, resVertices );
   
   // Done with clipping boundary. Now reset the input vectors.
   vertices.clear();
   outcodes.clear();
   
   //return (resVertices.size() > 2);
   return true;
}


int
ClipUtil::clipPolyToBBoxFaster( const MC2BoundingBox& bbox, 
                                vector<MC2Point>& vertices ) {
   
   uint32 nbrVertices = vertices.size();
   
   if (nbrVertices < 2) {
      return (false);
   }
   
   
   vector<byte> m_outcodes1;
   m_outcodes1.clear();
   
   m_outcodes1.reserve(nbrVertices);
   // Calculate the outcodes.
   for ( vector<MC2Point>::const_iterator it = vertices.begin();
         it != vertices.end(); ++it ) {
      m_outcodes1.push_back( 
         bbox.getCohenSutherlandOutcode( it->getY(), it->getX() ) );
   }
   
   vector<byte> m_outcodes2;
   m_outcodes2.clear();
   
   vector<MC2Point> m_vertices2;
   m_vertices2.clear();
   
   // Clip using Sutherland-Hodgeman clipping.
   // Clip against a bbox boundary and feed the output as input when
   // clipping against the next bbox boundary...
   
   // Clip to left boundary
   clipToBoundary(MC2BoundingBox::LEFT, &bbox, vertices, m_outcodes1,
                  m_vertices2, m_outcodes2);
   
   // Clip to right boundary
   clipToBoundary(MC2BoundingBox::RIGHT, &bbox, m_vertices2, m_outcodes2,
                  vertices, m_outcodes1);
   
   // Clip to top boundary
   clipToBoundary(MC2BoundingBox::TOP, &bbox, vertices, m_outcodes1,
                  m_vertices2, m_outcodes2);
   
   // Clip to bottom boundary
   bool retVal = 
      clipToBoundary(MC2BoundingBox::BOTTOM, &bbox, m_vertices2, m_outcodes2,
                     vertices, m_outcodes1);
   
   return (retVal);
}

int
ClipUtil::clipPolyToBBoxFast( const MC2BoundingBox& bbox,
                              vector<MC2Point>& vertices ) 
{
   // Make a static version.
   ClipUtil clipUtil;
   return clipUtil.clipPolyToBBoxFaster( bbox, vertices );
}

int
ClipUtil::clipPolyToBBoxFast( const MC2BoundingBox& bboxa,
                              vector<MC2Coordinate>& result,
                              const MC2Coordinate* begin,
                              const MC2Coordinate* end ) {
   MC2BoundingBox bbox(bboxa);
   vector<MC2Point> pointVec;
   pointVec.clear();
   uint32 size = distance( begin, end );
   pointVec.reserve( size );
   int32 lon_correction = 0;
   if ( bbox.getMinLon() > bbox.getMaxLon() ) {
      lon_correction = MAX_INT32 - bbox.getMinLon();
   }
   
   bbox.setMinLon( bbox.getMinLon() + lon_correction );
   bbox.setMaxLon( bbox.getMaxLon() + lon_correction );
   //mc2log << "[CLIP]: " << bbox << endl;
   {
      for ( const MC2Coordinate* it = begin;
            it != end;
            ++it ) {
         pointVec.push_back( MC2Point( it->lon + lon_correction,
                                       it->lat) );
      }
   }
   int resultBool = clipPolyToBBoxFast( bbox, pointVec );
   if ( resultBool ) {
      result.clear();
      result.reserve( pointVec.size() );
      for( vector<MC2Point>::const_iterator it = pointVec.begin();
           it != pointVec.end();
           ++it ) {
         result.push_back( MC2Coordinate( it->getY(),
                                          it->getX() - lon_correction) );
      }
   }
   return resultBool;
}


int
ClipUtil::clipPolyLineLB( const MC2BoundingBox& bboxa,
                          vector<coordVector_t>& result,
                          const MC2Coordinate* begin,
                          const MC2Coordinate* end ) {
   MC2BoundingBox bbox(bboxa);
   vector<MC2Point> pointVec;
   pointVec.clear();
   uint32 size = distance( begin, end );
   pointVec.reserve( size );
   int32 lon_correction = 0;
   if ( bbox.getMinLon() > bbox.getMaxLon() ) {
      lon_correction = MAX_INT32 - bbox.getMinLon();
   }

   bbox.setMinLon( bbox.getMinLon() + lon_correction );
   bbox.setMaxLon( bbox.getMaxLon() + lon_correction );
   //mc2log << "[CLIP]: " << bbox << endl;
   /* Get the first point of the line */
   MC2Point current( begin->lon + lon_correction, begin->lat );
   MC2Point breakPoint( MAX_INT32, MAX_INT32 );
   int breaks = 0;
   for ( const MC2Coordinate* it = begin+1; it != end; it++ ) {
      /* Get the next point of the line */
      MC2Point next( it->lon + lon_correction, it->lat );
      /* Check if the line segment is  inside or outside
         the bounding box, clipping it at the edges if needed */ 
      if( clipLineLiangBarsky( bbox, current, next ) ){
         /* Add the current point, since the line between
            the two returned points is inside the bbox */
         pointVec.push_back( current );
         /* Move to the next line segment */
         current = MC2Point( it->lon + lon_correction, it->lat );
         if( current != next || it+1 == end ){
            /* The next point has been mooved or next
               is the last point in the vecor and
               needs to be added to the vector */
            pointVec.push_back( next );
            /* The poly line has left the bbox or the last
               point is inside, marks where in the vector
               to know how many line segments are visible */
            pointVec.push_back( breakPoint );
            breaks++;
         }
      }
      else{
         /* The whole line is outside the bbox */
         /* Move to the next line segment */
         current = MC2Point( it->lon + lon_correction, it->lat );
      }
   }
   int resultBool = pointVec.size() > 1;
   if ( resultBool ) {
      result.clear();
      result.reserve( breaks );
      coordVector_t oneLine;
      /* If breaks == 0 something is wrong with the algorithm */
      oneLine.reserve( pointVec.size()/breaks );
      for( vector<MC2Point>::const_iterator it = pointVec.begin();
           it != pointVec.end(); ++it ) {
         if( it->getY() != MAX_INT32 && it->getX() != MAX_INT32 ){
            oneLine.push_back( MC2Coordinate( it->getY(),
                                              it->getX() - lon_correction) );
         }
         else{
            result.push_back( oneLine );
            oneLine.clear();
            oneLine.reserve( pointVec.size()/breaks );
         }
      }
   }
   return resultBool;
}


int
ClipUtil::clipLineLiangBarsky( const MC2BoundingBox& bboxa,
                               MC2Point& currVertex,
                               MC2Point& nextVertex ) {
   /* This functions figures out where the lines are, and clips as necessary*/
   MC2BoundingBox bbox(bboxa);

   /*calculate change in x and change in y directions*/
   double dx = nextVertex.getX() - currVertex.getX();
   double dy = nextVertex.getY() - currVertex.getY();

   /*vertical line*/
   if(dx == 0){
      /*see if the line is in the drawing area*/
      if(nextVertex.getX() < bbox.getMinLon()){
         return false; //DONT_DRAW
      }
      if(nextVertex.getX() > bbox.getMaxLon()){
         return false; //DONT_DRAW
      }
      /*if they show, calculate them*/
      /*if the point is too far up, make it the bottom side*/
      if(currVertex.getY() < bbox.getMinLat()){
         currVertex.getY() = bbox.getMinLat();
      /*if the point is too far down, make it the top*/
      }else if(currVertex.getY() > bbox.getMaxLat()){
         currVertex.getY() = bbox.getMaxLat();
      }
      /*if the point is too far up, make it the bottom side*/
      if(nextVertex.getY() < bbox.getMinLat()){
         nextVertex.getY() = bbox.getMinLat();
      /*if the point is too far down, make it the top*/
      }else if(nextVertex.getY() > bbox.getMaxLat()){
         nextVertex.getY() = bbox.getMaxLat();
      }
      /*if it's the same point, then the line was outside drawing area*/
      if(nextVertex.getY() != currVertex.getY()){
         return true; //DRAW
      }else{
         return false; //DONT_DRAW
      }
   }
   /*horizontal line*/
   if(dy == 0){
      /*see if the line is in the drawing area*/
      if(nextVertex.getY() < bbox.getMinLat()){
         return false; //DONT_DRAW
      }
      if(nextVertex.getY() > bbox.getMaxLat()){
         return false; //DONT_DRAW
      }
      /*if they show, calculate them*/
      /*if the point is too far left, make it the left side*/
      if(currVertex.getX() < bbox.getMinLon()){
         currVertex.getX() = bbox.getMinLon();
      /*if the point is too far right, make it the right side*/
      }else if(currVertex.getX() > bbox.getMaxLon()){
         currVertex.getX() = bbox.getMaxLon();
      }
      /*if the point is too far left, make it the left side*/
      if(nextVertex.getX() < bbox.getMinLon()){
         nextVertex.getX() = bbox.getMinLon();
      /*if the point is too far right, make it the right side*/
      }else if(nextVertex.getX() > bbox.getMaxLon()){
         nextVertex.getX() = bbox.getMaxLon();
      }
      /*if it's the same point, then the line was outside drawing area*/
      if(nextVertex.getX() != currVertex.getX()){
         return true; //DRAW
      }
      else{
         return false; //DONT_DRAW
      }
   }
   /*calculate the side values*/
   double left   = (double)((bbox.getMinLon() - currVertex.getX())/dx);
   double right  = (double)((bbox.getMaxLon() - currVertex.getX())/dx);
   double top    = (double)((bbox.getMaxLat() - currVertex.getY())/dy);
   double bottom = (double)((bbox.getMinLat() - currVertex.getY())/dy);

   double minimum = 0;
   double maximum = 1;

   /*ignore if intersection isn't part of the line*/
   if((left >= minimum) && (left <= maximum)){
      /*check if the line is entering or exiting the intersection*/
      if(dx > 0){
         /*entering*/
         minimum = left;
      }
      else if(dx < 0){
         /*exiting*/
         maximum = left;
      }
   }
   /*ignore if intersection isn't part of the line*/
   if((right >= minimum) && (right <= maximum)){
      /*check if the line is entering or exiting the intersection*/
      if(dx < 0){
         /*entering*/
         minimum = right;
      }
      else if(dx > 0){
         /*exiting*/
         maximum = right;
      }
   }
   /*ignore if intersection isn't part of the line*/
   if((top >= minimum) && (top <= maximum)){
      /*check if the line is entering or exiting the intersection*/
      if(dy < 0){
         /*entering*/
         minimum = top;
      }
      else if(dy > 0){
         /*exiting*/
         maximum = top;
      }
   }
   /*ignore if intersection isn't part of the line*/
   if((bottom >= minimum) && (bottom <= maximum)){
      /*check if the line is entering or exiting the intersection*/
      if(dy > 0){
         /*entering*/
         minimum = bottom;
      }
      else if(dy < 0){
         /*exiting*/
         maximum = bottom;
      }
   }
   /*if max is less than min, don't draw*/
   if(minimum < maximum){
      /*if max has changed, recalculate the point*/
      if(maximum < 1){
         nextVertex.getX() = currVertex.getX() + (int32)((dx * maximum)+0.5);
         nextVertex.getY() = currVertex.getY() + (int32)((dy * maximum)+0.5);
      }
      /*if min has changed, recalculate the point*/
      if(minimum > 0){
         currVertex.getX() = currVertex.getX() + (int32)((dx * minimum)+0.5);
         currVertex.getY() = currVertex.getY() + (int32)((dy * minimum)+0.5);
      }
      /*make sure the line is in the box*/
      /*check endpoint X value*/
      if(nextVertex.getX() >= bbox.getMinLon() &&
         nextVertex.getX() <= bbox.getMaxLon()){
         /*check endpoint Y value*/	
         if(nextVertex.getY() >= bbox.getMinLat() &&
            nextVertex.getY() <= bbox.getMaxLat()){
            /*check startpoint X value*/
            if(currVertex.getX() >= bbox.getMinLon() &&
               currVertex.getX() <= bbox.getMaxLon()){
               /*check startpoint Y value*/
               if(currVertex.getY() >= bbox.getMinLat() &&
                  currVertex.getY() <= bbox.getMaxLat()){
                  return true; //DRAW
               }
            }
         }
      }
   }
   return false; //DONT_DRAW
}
