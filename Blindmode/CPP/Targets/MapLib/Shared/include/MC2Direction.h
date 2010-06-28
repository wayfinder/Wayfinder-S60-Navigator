/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2DIRECTION_ITEM_H
#define MC2DIRECTION_ITEM_H

class MC2Direction {
public:
   MC2Direction();
   MC2Direction(int angle);
public:   
   enum ClockType {
      HOUR_0 = 0,
      HOUR_1,
      HOUR_2,
      HOUR_3,
      HOUR_4,
      HOUR_5,
      HOUR_6,
      HOUR_7,
      HOUR_8,
      HOUR_9,
      HOUR_10,
      HOUR_11,
      NUM_HOURS
   };
   
   enum DirectionType {
      NORTH = 0,
      NORTH_EAST,
      EAST,
      SOUTH_EAST,
      SOUTH,
      SOUTH_WEST,
      WEST,
      NORTH_WEST,
      NUM_DIRECTIONS
   };
   
   enum RepresentationType {
      ClockBased = 0,
      DirectionBased
   };
public:
   void             SetAngle(int newAngle);
   int              GetAngle() const;

   bool             InFrontOfUser(int headingAngle) const;
   bool             InFrontOfUser(const MC2Direction& headingDirection) const;

   
   ClockType        Hour(int headingAngle) const;
   ClockType        Hour(const MC2Direction& headingDirection) const;
   DirectionType    CompassPoint() const;
private:
   int m_angle;
};

#endif
