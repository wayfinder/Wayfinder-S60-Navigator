/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2Direction.h"
#include <cassert>

MC2Direction::MC2Direction(int angle)
{
   SetAngle(angle);
}

void MC2Direction::SetAngle(int newAngle)
{
   if(newAngle==360)
      newAngle = 0;

   if(newAngle>360)
      newAngle = newAngle % 360;
   
   m_angle = newAngle;
}

MC2Direction::ClockType
MC2Direction::Hour(int headingAngle) const
{
   const float divFactor = 360.0f / static_cast<float>(NUM_HOURS);
   
   int angle = m_angle - headingAngle;

   if(angle<0)
      angle+=360;
   
   float indexFloat = static_cast<float>(angle) / divFactor;
   int index = static_cast<int>(indexFloat + 0.5f);
   
   // Note that we may get NUM_DIRECTIONS as index, (for instance 359 degrees)
   // and this should be wrapped to index 0.
   index = index % NUM_HOURS;
  
   assert(index >= 0 && index<NUM_HOURS);

   return ClockType(index) ;
}


bool MC2Direction::InFrontOfUser(const MC2Direction& headingDirection) const
{
   return InFrontOfUser(headingDirection.m_angle);
}

bool MC2Direction::InFrontOfUser(int headingAngle) const
{

   MC2Direction::ClockType ct = Hour(headingAngle);

   return
      (ct >= HOUR_0 && ct <= HOUR_3) ||
      (ct >= HOUR_9 && ct <= HOUR_11);
}


MC2Direction::DirectionType
MC2Direction::CompassPoint() const
{
   const float divFactor = 360.0f / static_cast<float>(NUM_DIRECTIONS);

   float indexFloat = static_cast<float>(m_angle) / divFactor;

   int index = static_cast<int>(indexFloat + 0.5f);

   // Note that we may get NUM_DIRECTIONS as index, (for instance 359 degrees)
   // and this should be wrapped to index 0.
   index = index % NUM_DIRECTIONS;

   assert(index >= 0 && index<NUM_DIRECTIONS);

   return DirectionType(index);
}

MC2Direction::MC2Direction()
{
   SetAngle(0);
}

MC2Direction::ClockType
MC2Direction::Hour(const MC2Direction& headingDirection) const
{
   return Hour(headingDirection.m_angle);
}


int MC2Direction::GetAngle() const
{
   return m_angle;
}



