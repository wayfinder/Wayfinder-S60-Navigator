/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROADS_H
#define ROADS_H

#include <algorithm>
#include "MC2SimpleString.h"
#include "MC2BoundingBox.h"
#include "MC2Point.h"
#include <map>
#include <vector>
#include "VicinityItem.h"
#include "PixelBox.h"


class CrossingsMap;
class TileMapHandler;

/**
 *      RoadSegment
 *
 *      Represents a small segment of a road. Has a start
 *      and end position.
 *
 */

class RoadSegment {
public:
   RoadSegment() :  m_positionStart(0, 0),
                    m_positionEnd(0, 0)
   {}
   RoadSegment(   MC2Point positionStart,
                  MC2Point positionEnd) :
      m_positionStart(positionStart),
      m_positionEnd(positionEnd)
   {}

   inline MC2Point startPosition() const {
      return m_positionStart;
   }

   inline MC2Point endPosition() const {
      return m_positionEnd;
   }
private:
   MC2Point  m_positionStart;
   MC2Point  m_positionEnd;
};

/**
 *      Road
 *
 *      A road is made up of a logical sequence of
 *      road segments. In addition it has a name property,
 *      level properties and a bounding box.
 *
 **/

class Road {
public:
   typedef std::vector<RoadSegment>  RoadSegmentVector;

   Road( const MC2SimpleString& name,
         int startLevel,
         int endLevel) :
      m_name(name),
      m_startLevel(startLevel),
      m_endLevel(endLevel)
      {}

   Road(const Road& rhs) :
      m_name(rhs.m_name),
      m_startLevel(rhs.m_startLevel),
      m_endLevel(rhs.m_endLevel)
      {
         m_segments = rhs.m_segments;
         m_box = rhs.m_box;
      }
   
   Road() :
      m_name(""),
      m_startLevel(0),
      m_endLevel(0)
      {}

   /**
    *   Detects all crossings between the road and r2 and stores them
    *   in crossMap.
    *
    *   @param  r2              The road that the object might intersect with.
    *
    *   @param  crossMap        The map that keeps track of all crossings.
    *
    *   @param  mapHandler      The map handler that holds necessary functions.
    *
    */


   void getCrossings(Road& r2,
                     CrossingsMap& crossMap,
                     TileMapHandler* mapHandler);

   /**
    *   Buried method that might be revived if street level data for maps
    *   can be trusted again.
    *
    * /

   bool checkCrossingsLeveled(Road& r2,
                              CrossingsMap& crossMap,
                              TileMapHandler* mapHandler);
   */


   /**
    *   Adds a road segment, further extending the road.
    *
    *
    */

   void addRoadSegment(RoadSegment rSegment) {
      m_segments.push_back(rSegment);
      m_box.update( rSegment.startPosition() );
      m_box.update( rSegment.endPosition() );
   }

   bool isEmpty() {
      return m_segments.empty();
   }

   RoadSegmentVector::const_iterator begin() const {
      return m_segments.begin();
   }

   RoadSegmentVector::const_iterator end() const {
      return m_segments.end();
   }
   
   bool sameRoad(Road& r2) const {
      return name() == r2.name();
   }

   const MC2SimpleString& name() const {
      return m_name;
   }

   int startLevel() const {
      return m_startLevel;
   }

   int endLevel() const {
      return m_endLevel;
   }

   const PixelBox& getBox() const {
      return m_box;
   }

private:
   MC2SimpleString      m_name;
   int m_startLevel;
   int m_endLevel;
   RoadSegmentVector  m_segments;
   PixelBox m_box;
};

bool operator<(const Road& lhs, const Road& rhs);


#endif
