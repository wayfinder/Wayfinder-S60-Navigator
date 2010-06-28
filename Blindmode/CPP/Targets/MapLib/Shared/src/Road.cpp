/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"
#include "Crossings.h"
#include "Road.h"
#include "TileMapHandler.h"
#include "GfxUtility.h"
#include "GfxConstants.h"


/*
bool
Road::checkCrossingsLeveled(Road& r2,
                            CrossingsMap& crossCollection,
                            TileMapHandler* mapHandler)
{
   bool changingLevel = startLevel() != endLevel();

   if(changingLevel)
      return false;

   bool otherChangingLevel = r2.startLevel() != r2.endLevel();

   if(otherChangingLevel)
      return false;

   //The two roads are truly on separate levels.
   if(startLevel() != r2.startLevel())
      return true;
   else
      return false;
}
*/
void
Road::getCrossings(Road& r2,
                   CrossingsMap& crossCollection,
                   TileMapHandler* mapHandler)
{
   // If boxes don't overlap, then there are no intersections.
   if ( ! getBox().overlaps( r2.getBox() ) ) {
      return;
   }

   /* Perhaps revive this if we get better map data..

   bool performLevelCheck = mapHandler->shouldCheckLevels();

   if(performLevelCheck) {
      if(checkCrossingsLeveled(r2, crossCollection, mapHandler))
         return;
         }*/

   //If we made it here, we should check all segments
   for(std::vector<RoadSegment>::iterator inner = m_segments.begin();
       inner!=m_segments.end();
       inner++) {
      for(std::vector<RoadSegment>::iterator outer = r2.m_segments.begin();
          outer!=r2.m_segments.end();
          outer++)
         {
            crossCollection.detectIntersection(mapHandler,
                                               *inner,
                                               name(),
                                               *outer,
                                               r2.name());
         }
   }
}
