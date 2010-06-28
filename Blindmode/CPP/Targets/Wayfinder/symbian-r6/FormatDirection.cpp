/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FormatDirection.h"
#include "RsgInclude.h" 
#include <coemain.h>
#include <coeutils.h>


int lookupHour(MC2Direction::ClockType hour)
{
   int resourceIdentifier = -1;

   switch(hour) {
      case MC2Direction::HOUR_0:
         resourceIdentifier = R_WF_DIRECTION_AHEAD;
         break;
      case MC2Direction::HOUR_1:
         resourceIdentifier = R_WF_DIRECTION_1_HOUR;
         break;
      case MC2Direction::HOUR_2:
         resourceIdentifier = R_WF_DIRECTION_2_HOUR;
         break;
      case MC2Direction::HOUR_3:
         resourceIdentifier = R_WF_DIRECTION_3_HOUR;
         break;
      case MC2Direction::HOUR_4:
         resourceIdentifier = R_WF_DIRECTION_4_HOUR;
         break;
      case MC2Direction::HOUR_5:
         resourceIdentifier = R_WF_DIRECTION_5_HOUR;
         break;
      case MC2Direction::HOUR_6:
         resourceIdentifier = R_WF_DIRECTION_6_HOUR;
         break;
      case MC2Direction::HOUR_7:
         resourceIdentifier = R_WF_DIRECTION_7_HOUR;
         break;
      case MC2Direction::HOUR_8:
         resourceIdentifier = R_WF_DIRECTION_8_HOUR;
         break;
      case MC2Direction::HOUR_9:
         resourceIdentifier = R_WF_DIRECTION_9_HOUR;
         break;
      case MC2Direction::HOUR_10:
         resourceIdentifier = R_WF_DIRECTION_10_HOUR;
         break;
      case MC2Direction::HOUR_11:
         resourceIdentifier = R_WF_DIRECTION_11_HOUR;
         break;
      case MC2Direction::NUM_HOURS:
         break;
   }

   return resourceIdentifier;
}

int lookupDirection(MC2Direction::DirectionType direction)
{
   int resourceIdentifier = -1;
   switch(direction)
   {
      case MC2Direction::NORTH:
         resourceIdentifier = R_WF_DIRECTION_NORTH;
         break;
      case MC2Direction::NORTH_EAST:
         resourceIdentifier = R_WF_DIRECTION_NORTH_EAST;
         break;
      case MC2Direction::EAST:
         resourceIdentifier = R_WF_DIRECTION_EAST;
         break;
      case MC2Direction::SOUTH_EAST:
         resourceIdentifier = R_WF_DIRECTION_SOUTH_EAST;
         break;
      case MC2Direction::SOUTH:
         resourceIdentifier = R_WF_DIRECTION_SOUTH;
         break;
      case MC2Direction::SOUTH_WEST:
         resourceIdentifier = R_WF_DIRECTION_SOUTH_WEST;
         break;
      case MC2Direction::WEST:
         resourceIdentifier = R_WF_DIRECTION_WEST;
         break;
      case MC2Direction::NORTH_WEST:
         resourceIdentifier = R_WF_DIRECTION_NORTH_WEST;
         break;
      case MC2Direction::NUM_DIRECTIONS:
         break;
   }

   return resourceIdentifier;
}

HBufC* makeResourceLC(int resourceIdentifier)
{
   HBufC* option =
      CCoeEnv::Static()->AllocReadResourceLC(resourceIdentifier);

   return option;
}

HBufC* FormatDirection::ParseLC(MC2Direction::ClockType hour)
{
   return makeResourceLC( lookupHour(hour) );
}

HBufC* FormatDirection::ParseLC(MC2Direction::DirectionType direction)
{
   return makeResourceLC( lookupDirection(direction) );
}
