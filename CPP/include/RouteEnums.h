/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef ROUTEENUMS_H
#define ROUTEENUMS_H
namespace isab{
   ///This namepace contains enums needed to decode route formats. 
   namespace RouteEnums {
      ///This enum describes the kinds of action the driver need sto
      ///take at a waypoint in order to keep on track.
      enum RouteAction {
         /// Invalid
         InvalidAction  = 0xffff,
         ///The end point of the route. Noone but NavTask should ever
         ///see this one.
         End            = 0x0000, 
         ///The start point of the route. Noone but NavTask should
         ///ever see this one.
         Start          = 0x0001, 
         ///Go straight ahead. Very unusual from a user perspective.
         Ahead          = 0x0002, 
         /// Turn left.
         Left           = 0x0003, 
         /// Turn right.
         Right          = 0x0004, 
         /// Make a U-Turn.
         UTurn          = 0x0005, 
         /// Start your vehicle?
         StartAt        = 0x0006, 
         /// Approaching target.
         Finally        = 0x0007,
         /// Enter a roundabout.
         EnterRdbt      = 0x0008,
         /// Exit from Roundabout.
         ExitRdbt       = 0x0009, 
         /// Go straight ahead in the roundabout.
         AheadRdbt      = 0x000a, 
         /// Turn left in the roundabout.
         LeftRdbt       = 0x000b, 
         /// Turn right in the roundabout.
         RightRdbt      = 0x000c, 
         /// Exit road by ramp.
         ExitAt         = 0x000d, 
         /// Enter road by ramp.
         On             = 0x000e, 
         /// Park your car here.
         ParkCar        = 0x000f, 
         /// Keep left when a road separates into two or more.
         KeepLeft       = 0x0010, 
         /// Keep right when a road separates into two or more.
         KeepRight      = 0x0011, 
         /// Start by makeing a u-turn.
         StartWithUTurn = 0x0012, 
         /// Go back in the roundabout.
         UTurnRdbt      = 0x0013, 
         /// Follow the current road.
         FollowRoad     = 0x0014, 
         /// Drive onto a ferry.
         EnterFerry     = 0x0015,
         /// Leave a ferry. Hopefully the one you entered earlier.
         ExitFerry      = 0x0016, 
         /// Leave one ferry, enter another.
         ChangeFerry    = 0x0017, 
         /// Turn left at the end of the road.
         EndOfRoadLeft  = 0x0018, 
         /// Turn right at the end of the road.
         EndOfRoadRight = 0x0019, 
         /// Turn left at the end of the road.
         OffRampLeft    = 0x001a, 
         /// Turn right at the end of the road.
         OffRampRight   = 0x001b, 
         /// This is something route format specific.
         Delta          = 0x03fe, 
         /// Last allowed value. 
         RouteActionMax = 0x03ff,
      };

      /// Describes different kinds of crossings.
      enum RouteCrossing {
         /// This is not a crossing but something else.
         NoCrossing, 
         /// Three way crossing.
         Crossing3Ways,
         /// four way crossing.
         Crossing4Ways,
         /// Crossing with more than 4 exits.
         CrossingMultiway,
      };

      enum OnTrackEnum {
         OnTrack, 
         OffTrack, 
         WrongWay, 
         Goal
      }; 
      
      enum SimInfo {
         simulate_off         = 0x0,
         simulate_on          = 0x1,
         simulate_paused      = 0x2,
         simulate_repeat_on   = 0x4,
         simulate_max_speed   = 0x8,
         simulate_min_speed   = 0x10
      };

   }
}
#endif
