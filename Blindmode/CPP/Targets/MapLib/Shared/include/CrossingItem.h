/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CROSSING_ITEM_H
#define CROSSING_ITEM_H

#include "MC2SimpleString.h"
#include "MC2Coordinate.h"
#include <vector>
#include "VicinityItem.h"
#include "Crossings.h"

class CrossingItem : public VicinityItem {
public:
   CrossingItem() : VicinityItem() {}
   
   CrossingItem(int distance,
                const MC2SimpleString& name,
                const MC2Coordinate& coord,
                const MC2Direction direction,
                const Crossing& crossing)
      
      :  VicinityItem( distance, 
                       name,
                       name,
                       coord,
                       direction),
         m_crossing(crossing)
   {
      buildSimpleName();
   }

   bool atSameLocation(CrossingItem& c2) {
      return
         Coord().lat == c2.Coord().lat &&
         Coord().lon == c2.Coord().lon;
   }
   
   virtual void Accept(VicinityVisitor* visitor) {
      visitor->Visit(this);
   }

   const Crossing GetCrossingsRelative(int heading);
   const Crossing GetCrossingsAbsolute();


   inline VicinityType Type() const {
      return VI_CROSSING;
   }
   
   
   void printStreets();
   void buildSimpleName();   
private:
   Crossing m_crossing;
   mutable MC2SimpleString m_buildString;
};

#endif
