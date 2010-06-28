/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "TurnPictures.h"

using namespace isab;
using namespace RouteEnums;
using namespace RouteInfoParts;


#include "wficons.mbg"

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
#define EMbmWfturns4way_left EMbmWficonsFour_way_turn_left
#define EMbmWfturns4way_right EMbmWficonsFour_way_turn_right
#define EMbmWfturns4way_straight EMbmWficonsFour_way_drive_straight
#define EMbmWfturnsEnter_highway EMbmWficonsEnter_highway_right
#define EMbmWfturnsEnter_highway_left EMbmWficonsEnter_highway_left
#define EMbmWfturnsEnter_main_road EMbmWficonsEnter_main_road_right
#define EMbmWfturnsEnter_main_road_left EMbmWficonsEnter_main_road_left
#define EMbmWfturnsExit_highway EMbmWficonsExit_highway_right
#define EMbmWfturnsExit_highway_left EMbmWficonsExit_highway_left
#define EMbmWfturnsExit_main_road EMbmWficonsExit_main_road_right
#define EMbmWfturnsExit_main_road_left EMbmWficonsExit_main_road_left
#define EMbmWfturnsExit1 EMbmWficonsExit_1
#define EMbmWfturnsExit2 EMbmWficonsExit_2
#define EMbmWfturnsExit3 EMbmWficonsExit_3
#define EMbmWfturnsExit4 EMbmWficonsExit_4
#define EMbmWfturnsExit5 EMbmWficonsExit_5
#define EMbmWfturnsExit6 EMbmWficonsExit_6
#define EMbmWfturnsExit7 EMbmWficonsExit_7
#define EMbmWfturnsExit8 EMbmWficonsExit_8
#define EMbmWfturnsExit9 EMbmWficonsExit_9
#define EMbmWfturnsExit10 EMbmWficonsExit_10
#define EMbmWfturnsExit11 EMbmWficonsExit_11
#define EMbmWfturnsExit12 EMbmWficonsExit_12
#define EMbmWfturnsFerry EMbmWficonsFerry
#define EMbmWfturnsFinish_arrow EMbmWficonsReached_destination2
#define EMbmWfturnsFinish_flag EMbmWficonsReached_destination1
#define EMbmWfturnsHighway_straight EMbmWficonsHighway_straight_ahead_right
#define EMbmWfturnsHighway_straight_left EMbmWficonsHighway_straight_ahead_left
#define EMbmWfturnsKeep_left EMbmWficonsKeep_left
#define EMbmWfturnsKeep_right EMbmWficonsKeep_right
#define EMbmWfturnsLeft_arrow EMbmWficonsTurn_left
#define EMbmWfturnsMedium_back_arrow EMbmWficonsSm_wrong_direction
#define EMbmWfturnsMedium_blank EMbmWficonsSm_turn_blank
#define EMbmWfturnsMedium_detour EMbmWficonsDetour_square.svg
#define EMbmWfturnsMedium_ferry EMbmWficonsSm_ferry
#define EMbmWfturnsMedium_flag EMbmWficonsSm_reached_destination
#define EMbmWfturnsMedium_keep_left EMbmWficonsSm_keep_left
#define EMbmWfturnsMedium_keep_right EMbmWficonsSm_keep_right
#define EMbmWfturnsMedium_left_arrow EMbmWficonsSm_turn_left
#define EMbmWfturnsMedium_mask EMbmWficonsSm_turn_blank_mask
#define EMbmWfturnsMedium_multiway_rdbt EMbmWficonsSm_multiway_roundabout_right
#define EMbmWfturnsMedium_multiway_rdbt_left EMbmWficonsSm_multiway_roundabout_left
#define EMbmWfturnsMedium_offtrack EMbmWficonsSm_off_track
#define EMbmWfturnsMedium_park_car EMbmWficonsSm_parking_area
#define EMbmWfturnsMedium_right_arrow EMbmWficonsSm_turn_right
#define EMbmWfturnsMedium_speedcam EMbmWficonsSpeedcamera_square
#define EMbmWfturnsMedium_start EMbmWficonsSm_start
#define EMbmWfturnsMedium_straight_arrow EMbmWficonsSm_drive_straight_ahead1
#define EMbmWfturnsMedium_u_turn EMbmWficonsSm_uturn_right
#define EMbmWfturnsMedium_u_turn_left EMbmWficonsSm_uturn_left
#define EMbmWfturnsMultiway_rdbt EMbmWficonsMultiway_roundabout_right
#define EMbmWfturnsMultiway_rdbt_left EMbmWficonsMultiway_roundabout_left
#define EMbmWfturnsOfftrack EMbmWficonsOff_track
#define EMbmWfturnsOpposite_arrow EMbmWficonsWrong_direction
#define EMbmWfturnsPark_car EMbmWficonsParking_area
#define EMbmWfturnsRdbt_left EMbmWficonsTurn_left_roundabout_right
#define EMbmWfturnsRdbt_left_left EMbmWficonsTurn_left_roundabout_left
#define EMbmWfturnsRdbt_right EMbmWficonsTurn_right_roundabout_right
#define EMbmWfturnsRdbt_right_left EMbmWficonsTurn_right_roundabout_left
#define EMbmWfturnsRdbt_straight EMbmWficonsDrive_straight_roundabout_right
#define EMbmWfturnsRdbt_straight_left EMbmWficonsDrive_straight_roundabout_left
#define EMbmWfturnsRdbt_uturn EMbmWficonsUturn_roundabout_right
#define EMbmWfturnsRdbt_uturn_left EMbmWficonsUturn_roundabout_left
#define EMbmWfturnsRight_arrow EMbmWficonsTurn_right
#define EMbmWfturnsSmall_back_arrow EMbmWficonsSm_wrong_direction
#define EMbmWfturnsSmall_back_arrow_m EMbmWficonsSm_wrong_direction_mask
#define EMbmWfturnsSmall_blank EMbmWficonsSm_turn_blank
#define EMbmWfturnsSmall_blank_m EMbmWficonsSm_turn_blank_mask
#define EMbmWfturnsSmall_detour EMbmWficonsDetour_square
#define EMbmWfturnsSmall_detour_m EMbmWficonsDetour_square_mask
#define EMbmWfturnsSmall_ferry EMbmWficonsSm_ferry
#define EMbmWfturnsSmall_ferry_m EMbmWficonsSm_ferry_mask
#define EMbmWfturnsSmall_flag EMbmWficonsSm_reached_destination
#define EMbmWfturnsSmall_flag_m EMbmWficonsSm_reached_destination_mask
#define EMbmWfturnsSmall_keep_left EMbmWficonsSm_keep_left
#define EMbmWfturnsSmall_keep_left_m EMbmWficonsSm_keep_left_mask
#define EMbmWfturnsSmall_keep_right EMbmWficonsSm_keep_right
#define EMbmWfturnsSmall_keep_right_m EMbmWficonsSm_keep_right_mask
#define EMbmWfturnsSmall_left_arrow EMbmWficonsSm_turn_left
#define EMbmWfturnsSmall_left_arrow_m EMbmWficonsSm_turn_left_mask
#define EMbmWfturnsSmall_mask EMbmWficonsSm_turn_blan_mask
#define EMbmWfturnsSmall_multiway_rdbt EMbmWficonsSm_multiway_roundabout_right
#define EMbmWfturnsSmall_multiway_rdbt_m EMbmWficonsSm_multiway_roundabout_right_mask
#define EMbmWfturnsSmall_multiway_rdbt_left EMbmWficonsSm_multiway_roundabout_left
#define EMbmWfturnsSmall_multiway_rdbt_left_m EMbmWficonsSm_multiway_roundabout_left_mask
#define EMbmWfturnsSmall_offtrack EMbmWficonsSm_off_track
#define EMbmWfturnsSmall_offtrack_m EMbmWficonsSm_off_track_mask
#define EMbmWfturnsSmall_park_car EMbmWficonsSm_parking_area
#define EMbmWfturnsSmall_park_car_m EMbmWficonsSm_parking_area_mask
#define EMbmWfturnsSmall_right_arrow EMbmWficonsSm_turn_right
#define EMbmWfturnsSmall_right_arrow_m EMbmWficonsSm_turn_right_mask
#define EMbmWfturnsSmall_speedcam EMbmWficonsSpeedcamera_square
#define EMbmWfturnsSmall_speedcam_m EMbmWficonsSpeedcamera_square_mask
#define EMbmWfturnsSmall_start EMbmWficonsSm_start
#define EMbmWfturnsSmall_start_m EMbmWficonsSm_start_mask
#define EMbmWfturnsSmall_straight_arrow EMbmWficonsSm_drive_straight_ahead1
#define EMbmWfturnsSmall_straight_arrow_m EMbmWficonsSm_drive_straight_ahead1_mask
#define EMbmWfturnsSmall_u_turn EMbmWficonsSm_uturn_right
#define EMbmWfturnsSmall_u_turn_m EMbmWficonsSm_uturn_right_mask
#define EMbmWfturnsSmall_u_turn_left EMbmWficonsSm_uturn_left
#define EMbmWfturnsSmall_u_turn_left_m EMbmWficonsSm_uturn_left_mask
#define EMbmWfturnsStraight_ahead EMbmWficonsDrive_straight_ahead2
#define EMbmWfturnsStraight_arrow EMbmWficonsDrive_straight_ahead1
#define EMbmWfturnsU_turn EMbmWficonsUturn_right
#define EMbmWfturnsU_turn_left EMbmWficonsUturn_left
#define EMbmWfturn_mask EMbmWficonsUturn_right_mask

#elif defined NAV2_CLIENT_SERIES60_V3

#define EMbmWfturns4way_left EMbmWficonsFour_way_turn_left
#define EMbmWfturns4way_right EMbmWficonsFour_way_turn_right
#define EMbmWfturns4way_straight EMbmWficonsFour_way_drive_straight
#define EMbmWfturnsEnter_highway EMbmWficonsEnter_highway_right
#define EMbmWfturnsEnter_highway_left EMbmWficonsEnter_highway_left
#define EMbmWfturnsEnter_main_road EMbmWficonsEnter_main_road_right
#define EMbmWfturnsEnter_main_road_left EMbmWficonsEnter_main_road_left
#define EMbmWfturnsExit_highway EMbmWficonsExit_highway_right
#define EMbmWfturnsExit_highway_left EMbmWficonsExit_highway_left
#define EMbmWfturnsExit_main_road EMbmWficonsExit_main_road_right
#define EMbmWfturnsExit_main_road_left EMbmWficonsExit_main_road_left
#define EMbmWfturnsExit1 EMbmWficonsExit_1
#define EMbmWfturnsExit2 EMbmWficonsExit_2
#define EMbmWfturnsExit3 EMbmWficonsExit_3
#define EMbmWfturnsExit4 EMbmWficonsExit_4
#define EMbmWfturnsExit5 EMbmWficonsExit_5
#define EMbmWfturnsExit6 EMbmWficonsExit_6
#define EMbmWfturnsExit7 EMbmWficonsExit_7
#define EMbmWfturnsExit8 EMbmWficonsExit_8
#define EMbmWfturnsExit9 EMbmWficonsExit_9
#define EMbmWfturnsExit10 EMbmWficonsExit_10
#define EMbmWfturnsExit11 EMbmWficonsExit_11
#define EMbmWfturnsExit12 EMbmWficonsExit_12
#define EMbmWfturnsFerry EMbmWficonsFerry
#define EMbmWfturnsFinish_arrow EMbmWficonsReached_destination2
#define EMbmWfturnsFinish_flag EMbmWficonsReached_destination1
#define EMbmWfturnsHighway_straight EMbmWficonsHighway_straight_ahead_right
#define EMbmWfturnsHighway_straight_left EMbmWficonsHighway_straight_ahead_left
#define EMbmWfturnsKeep_left EMbmWficonsKeep_left
#define EMbmWfturnsKeep_right EMbmWficonsKeep_right
#define EMbmWfturnsLeft_arrow EMbmWficonsTurn_left
#define EMbmWfturnsMedium_back_arrow EMbmWficonsSm_wrong_direction
#define EMbmWfturnsMedium_blank EMbmWficonsSm_turn_blank
#define EMbmWfturnsMedium_detour EMbmWficonsDetour_square.svg
#define EMbmWfturnsMedium_ferry EMbmWficonsSm_ferry
#define EMbmWfturnsMedium_flag EMbmWficonsSm_reached_destination
#define EMbmWfturnsMedium_keep_left EMbmWficonsSm_keep_left
#define EMbmWfturnsMedium_keep_right EMbmWficonsSm_keep_right
#define EMbmWfturnsMedium_left_arrow EMbmWficonsSm_turn_left
#define EMbmWfturnsMedium_mask EMbmWficonsSm_turn_blank_mask
#define EMbmWfturnsMedium_multiway_rdbt EMbmWficonsSm_multiway_roundabout_right
#define EMbmWfturnsMedium_multiway_rdbt_left EMbmWficonsSm_multiway_roundabout_left
#define EMbmWfturnsMedium_offtrack EMbmWficonsSm_off_track
#define EMbmWfturnsMedium_park_car EMbmWficonsSm_parking_area
#define EMbmWfturnsMedium_right_arrow EMbmWficonsSm_turn_right
#define EMbmWfturnsMedium_speedcam EMbmWficonsSpeedcamera_square
#define EMbmWfturnsMedium_start EMbmWficonsSm_start
#define EMbmWfturnsMedium_straight_arrow EMbmWficonsSm_drive_straight_ahead1
#define EMbmWfturnsMedium_u_turn EMbmWficonsSm_uturn_right
#define EMbmWfturnsMedium_u_turn_left EMbmWficonsSm_uturn_left
#define EMbmWfturnsMultiway_rdbt EMbmWficonsMultiway_roundabout_right
#define EMbmWfturnsMultiway_rdbt_left EMbmWficonsMultiway_roundabout_left
#define EMbmWfturnsOfftrack EMbmWficonsOff_track
#define EMbmWfturnsOpposite_arrow EMbmWficonsWrong_direction
#define EMbmWfturnsPark_car EMbmWficonsParking_area
#define EMbmWfturnsRdbt_left EMbmWficonsTurn_left_roundabout_right
#define EMbmWfturnsRdbt_left_left EMbmWficonsTurn_left_roundabout_left
#define EMbmWfturnsRdbt_right EMbmWficonsTurn_right_roundabout_right
#define EMbmWfturnsRdbt_right_left EMbmWficonsTurn_right_roundabout_left
#define EMbmWfturnsRdbt_straight EMbmWficonsDrive_straight_roundabout_right
#define EMbmWfturnsRdbt_straight_left EMbmWficonsDrive_straight_roundabout_left
#define EMbmWfturnsRdbt_uturn EMbmWficonsUturn_roundabout_right
#define EMbmWfturnsRdbt_uturn_left EMbmWficonsUturn_roundabout_left
#define EMbmWfturnsRight_arrow EMbmWficonsTurn_right
#define EMbmWfturnsSmall_back_arrow EMbmWficonsSmt_wrong_direction
#define EMbmWfturnsSmall_back_arrow_m EMbmWficonsSmt_wrong_direction_mask
#define EMbmWfturnsSmall_blank EMbmWficonsSmt_turn_blank
#define EMbmWfturnsSmall_blank_m EMbmWficonsSmt_turn_blank_mask
#define EMbmWfturnsSmall_detour EMbmWficonsDetour_square
#define EMbmWfturnsSmall_detour_m EMbmWficonsDetour_square_mask
#define EMbmWfturnsSmall_ferry EMbmWficonsSmt_ferry
#define EMbmWfturnsSmall_ferry_m EMbmWficonsSmt_ferry_mask
#define EMbmWfturnsSmall_flag EMbmWficonsSmt_reached_destination
#define EMbmWfturnsSmall_flag_m EMbmWficonsSmt_reached_destination_mask
#define EMbmWfturnsSmall_keep_left EMbmWficonsSmt_keep_left
#define EMbmWfturnsSmall_keep_left_m EMbmWficonsSmt_keep_left_mask
#define EMbmWfturnsSmall_keep_right EMbmWficonsSmt_keep_right
#define EMbmWfturnsSmall_keep_right_m EMbmWficonsSmt_keep_right_mask
#define EMbmWfturnsSmall_left_arrow EMbmWficonsSmt_turn_left
#define EMbmWfturnsSmall_left_arrow_m EMbmWficonsSmt_turn_left_mask
#define EMbmWfturnsSmall_mask EMbmWficonsSmt_turn_blan_mask
#define EMbmWfturnsSmall_multiway_rdbt EMbmWficonsSmt_multiway_roundabout_right
#define EMbmWfturnsSmall_multiway_rdbt_m EMbmWficonsSmt_multiway_roundabout_right_mask
#define EMbmWfturnsSmall_multiway_rdbt_left EMbmWficonsSmt_multiway_roundabout_left
#define EMbmWfturnsSmall_multiway_rdbt_left_m EMbmWficonsSmt_multiway_roundabout_left_mask
#define EMbmWfturnsSmall_offtrack EMbmWficonsSmt_off_track
#define EMbmWfturnsSmall_offtrack_m EMbmWficonsSmt_off_track_mask
#define EMbmWfturnsSmall_park_car EMbmWficonsSmt_parking_area
#define EMbmWfturnsSmall_park_car_m EMbmWficonsSmt_parking_area_mask
#define EMbmWfturnsSmall_right_arrow EMbmWficonsSmt_turn_right
#define EMbmWfturnsSmall_right_arrow_m EMbmWficonsSmt_turn_right_mask
#define EMbmWfturnsSmall_speedcam EMbmWficonsSpeedcamera_square
#define EMbmWfturnsSmall_speedcam_m EMbmWficonsSpeedcamera_square_mask
#define EMbmWfturnsSmall_start EMbmWficonsSmt_start
#define EMbmWfturnsSmall_start_m EMbmWficonsSmt_start_mask
#define EMbmWfturnsSmall_straight_arrow EMbmWficonsSmt_drive_straight_ahead1
#define EMbmWfturnsSmall_straight_arrow_m EMbmWficonsSmt_drive_straight_ahead1_mask
#define EMbmWfturnsSmall_u_turn EMbmWficonsSmt_uturn_right
#define EMbmWfturnsSmall_u_turn_m EMbmWficonsSmt_uturn_right_mask
#define EMbmWfturnsSmall_u_turn_left EMbmWficonsSmt_uturn_left
#define EMbmWfturnsSmall_u_turn_left_m EMbmWficonsSmt_uturn_left_mask
#define EMbmWfturnsStraight_ahead EMbmWficonsDrive_straight_ahead2
#define EMbmWfturnsStraight_arrow EMbmWficonsDrive_straight_ahead1
#define EMbmWfturnsU_turn EMbmWficonsUturn_right
#define EMbmWfturnsU_turn_left EMbmWficonsUturn_left
#define EMbmWfturn_mask EMbmWficonsUturn_right_mask
#endif

using namespace isab;
using namespace RouteEnums;
using namespace RouteInfoParts;

TPictures TTurnPictures::GetPicture( const enum isab::RouteAction aAction,
                                     const enum isab::RouteCrossing aCrossing,
                                     const TUint aDistance,
                                     const TBool aHighway )
{
   TPictures picture = ENoPicture; 
   switch( aAction )
   {
   case End:
      picture = ENoPicture;
      break;
   case Finally:
      if( aDistance > 30 ){
         picture = EFinishArrow;
      }
      else{
         picture = EFinishFlag;
      }
      break;
   case Start:
      if( aCrossing == NoCrossing ){
         picture = EStraight;
      }
      else{
         picture = EStraightArrow;
      }
      break;
   case StartWithUTurn:
      picture = EUTurn;
      break;
   case Ahead:
      if( aHighway == EFalse ){
         if( aCrossing == Crossing4Ways ){
            picture = E4WayStraight;
         }
         else if( aCrossing == NoCrossing ){
            picture = EStraight;
         }
         else{
            picture = EStraightArrow;
         }
      }
      else{
         picture = EHighWayStraight;
      }
      break;
   case AheadRdbt:
      picture = ERdbStraight;
      break;
   case EndOfRoadLeft:
      picture = E3WayTeeLeft;
      break;
   case Left:
      if( aCrossing == Crossing4Ways )
         picture = E4WayLeft;
      else
         picture = ELeftArrow;
      break;
   case LeftRdbt:
      picture = ERdbLeft;
      break;
   case KeepLeft:
      picture = EKeepLeft;
      break;
   case EndOfRoadRight:
      picture = E3WayTeeRight;
      break;
   case Right:
      if( aCrossing == Crossing4Ways ){
         picture = E4WayRight;
      }
      else{
         picture = ERightArrow;
      }
      break;
   case RightRdbt:
      picture = ERdbRight;
      break;
   case KeepRight:
      picture = EKeepRight;
      break;
   case EnterRdbt:
      picture = EMultiWayRdb;
      break;
   case ExitRdbt:
      picture = EMultiWayRdb;
      break;
   case UTurn:
      picture = EUTurn;
      break;
   case UTurnRdbt:
      picture = ERdbUTurn;
      break;
   case ExitAt:
      if(aHighway == EFalse){
         picture = EExitMainRoad;
      }
      else{
         picture = EExitHighWay;
      }
      break;
   case On:
      if( aHighway == EFalse ){
         picture = EEnterMainRoad;
      }
      else{
         picture = EEnterHighWay;
      }
      break;
   case StartAt:
      if( aHighway == EFalse ){
         picture = EStraight;
      }
      else{
         picture = EHighWayStraight;
      }
      break;
   case ParkCar:
      picture = EPark;
      break;
   case FollowRoad:
      if( aHighway == EFalse ){
         picture = EStraight;
      }
      else{
         picture = EHighWayStraight;
      }
      break;
   case EnterFerry:
   case ExitFerry:
   case ChangeFerry:
      picture = EFerry;
      break;
   case InvalidAction:
      picture = EOffTrack;
      break;
   case OffRampRight:
      if( aHighway == EFalse ){
         picture = EExitMainRoadRight;
      }
      else{
         picture = EExitHighWayRight;
      }
      break;
   case OffRampLeft:
      if( aHighway == EFalse ){
         picture = EExitMainRoadLeft;
      }
      else{
         picture = EExitHighWayLeft;
      }
      break;
   case Delta:
   case RouteEnums::RouteActionMax: 
      break;
   }
   return picture;
}


TInt TTurnPictures::GetTurnPicture( const TPictures aTurn,
                                    const TBool aLeftSide )
{
   TInt mbmIndex = -1;
   
   switch( aTurn )
   {
   case E4WayLeft:
      mbmIndex = EMbmWfturns4way_left;
      break;
   case E4WayRight:
      mbmIndex = EMbmWfturns4way_right;
      break;
   case E4WayStraight:
      mbmIndex = EMbmWfturns4way_straight;
      break;
   case EEnterHighWay:
     if( !aLeftSide ){
         mbmIndex = EMbmWfturnsEnter_highway;
      }
      else{
         mbmIndex = EMbmWfturnsEnter_highway_left;
      }
      break;
   case EEnterMainRoad:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsEnter_main_road;
      }
      else{
         mbmIndex = EMbmWfturnsEnter_main_road_left;
      }
      break;
  case EExitHighWay:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsExit_highway;
      }
      else {
         mbmIndex = EMbmWfturnsExit_highway_left;
      }
      break;
   case EExitHighWayRight:
      mbmIndex = EMbmWfturnsExit_highway;
      break;
   case EExitHighWayLeft:
      mbmIndex = EMbmWfturnsExit_highway_left;
      break;
   case EExitMainRoad:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsExit_main_road;
      }
      else{
         mbmIndex = EMbmWfturnsExit_main_road_left;
      }
      break;
   case EExitMainRoadRight:
      mbmIndex = EMbmWfturnsExit_main_road;
      break;
   case EExitMainRoadLeft:
      mbmIndex = EMbmWfturnsExit_main_road_left;
      break;
   case EFerry:
      mbmIndex = EMbmWfturnsFerry;
      break;
   case EFinishArrow:
      mbmIndex = EMbmWfturnsFinish_arrow;
      break;
   case EFinishFlag:
      mbmIndex = EMbmWfturnsFinish_flag;
      break;
   case EHighWayStraight:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsHighway_straight;
      }
      else{
         mbmIndex = EMbmWfturnsHighway_straight_left;
      }
      break;
   case EKeepLeft:
      mbmIndex = EMbmWfturnsKeep_left;
      break;
   case EKeepRight:
      mbmIndex = EMbmWfturnsKeep_right;
      break;
   case ELeftArrow:
      mbmIndex = EMbmWfturnsLeft_arrow;
      break;
   case EMultiWayRdb:
     if( !aLeftSide ){
         mbmIndex = EMbmWfturnsMultiway_rdbt;
      }
      else{
         mbmIndex = EMbmWfturnsMultiway_rdbt_left;
      }
      break;
   case EOffTrack:
      mbmIndex = EMbmWfturnsOfftrack;
      break;
   case EWrongDirection:
      mbmIndex = EMbmWfturnsOpposite_arrow;
      break;
   case EPark:
      mbmIndex = EMbmWfturnsPark_car;
      break;
   case ERdbLeft:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsRdbt_left;
      }
      else{
         mbmIndex = EMbmWfturnsRdbt_left_left;
      }
      break;
   case ERdbRight:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsRdbt_right;
      }
      else{
         mbmIndex = EMbmWfturnsRdbt_right_left;
      }
      break;
   case ERdbStraight:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsRdbt_straight;
      } else {
         mbmIndex = EMbmWfturnsRdbt_straight_left;
      } 
      break;
   case ERdbUTurn:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsRdbt_uturn;
      } else {
         mbmIndex = EMbmWfturnsRdbt_uturn_left;
      }
      break;
   case ERightArrow:
      mbmIndex = EMbmWfturnsRight_arrow;
      break;
   case EStraight:
      mbmIndex = EMbmWfturnsStraight_ahead;
      break;
   case EStraightArrow:
      mbmIndex = EMbmWfturnsStraight_arrow;
      break;
   case EUTurn:
     if( !aLeftSide ){
         mbmIndex = EMbmWfturnsU_turn;
      } else {
         mbmIndex = EMbmWfturnsU_turn_left;
      }
      break;
   case ENoPicture:	    
   default:
      break;
   }
   return mbmIndex;
}


TInt TTurnPictures::GetMediumTurnPicture( const TPictures aTurn,
                                          const TBool aLeftSide )
{
   TInt mbmIndex = -1;

   switch( aTurn )
   {
   case E4WayLeft:
   case ELeftArrow:
   case ERdbLeft:
      mbmIndex = EMbmWfturnsMedium_left_arrow;
      break;
   case E4WayRight:
   case ERdbRight:
   case ERightArrow:
      mbmIndex = EMbmWfturnsMedium_right_arrow;
      break;
   case E4WayStraight:
   case EHighWayStraight:
   case EStraight:
   case EStraightArrow:
   case ERdbStraight:
      mbmIndex = EMbmWfturnsMedium_straight_arrow;
      break;
   case EEnterHighWay:
   case EEnterMainRoad:
      mbmIndex = EMbmWfturnsMedium_keep_left;
      break;
   case EExitHighWayLeft:
   case EExitMainRoadLeft:
   case EKeepLeft:
      mbmIndex = EMbmWfturnsMedium_keep_left;
      break;
   case EExitHighWay:
   case EExitMainRoad:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsMedium_keep_right;
      }
      else{
         mbmIndex = EMbmWfturnsMedium_keep_left;
      }
      break;
   case EExitHighWayRight:
   case EExitMainRoadRight:
   case EKeepRight:
      mbmIndex = EMbmWfturnsMedium_keep_right;
      break;
   case EFerry:
      mbmIndex = EMbmWfturnsMedium_ferry;
      break;
   case EFinishArrow:
   case EFinishFlag:
      mbmIndex = EMbmWfturnsMedium_flag;
      break;
   case EMultiWayRdb:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsMedium_multiway_rdbt;
      } else {
         mbmIndex = EMbmWfturnsMedium_multiway_rdbt_left;
      }
      break;
   case EOffTrack:
      mbmIndex = EMbmWfturnsMedium_offtrack;
      break;
   case EWrongDirection:
      mbmIndex = EMbmWfturnsMedium_back_arrow;
      break;
   case EPark:
      mbmIndex = EMbmWfturnsMedium_park_car;
      break;
   case ERdbUTurn:
   case EUTurn:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsMedium_u_turn;
      } else {
         mbmIndex = EMbmWfturnsMedium_u_turn_left;
      }
      break;
   default:
      break;
   }
   return mbmIndex;
}


void TTurnPictures::GetSmallTurnPicture( const TPictures aTurn,
                                         const TBool aLeftSide,
                                         TInt &mbmIndex,
                                         TInt &mbmMaskIndex )
{
   mbmIndex = -1;
   mbmMaskIndex = -1;

   switch( aTurn )
   {
   case E4WayLeft:
   case ELeftArrow:
   case ERdbLeft:
      mbmIndex = EMbmWfturnsSmall_left_arrow;
      mbmMaskIndex = EMbmWfturnsSmall_left_arrow_m;
      break;
   case E4WayRight:
   case ERdbRight:
   case ERightArrow:
      mbmIndex = EMbmWfturnsSmall_right_arrow;
      mbmMaskIndex = EMbmWfturnsSmall_right_arrow_m;
      break;
   case E4WayStraight:
   case EHighWayStraight:
   case EStraight:
   case EStraightArrow:
   case ERdbStraight:
      mbmIndex = EMbmWfturnsSmall_straight_arrow;
      mbmMaskIndex = EMbmWfturnsSmall_straight_arrow_m;
      break;
   case EEnterHighWay:
   case EEnterMainRoad:
      mbmIndex = EMbmWfturnsSmall_keep_left;
      mbmMaskIndex = EMbmWfturnsSmall_keep_left_m;
      break;
   case EExitHighWayLeft:
   case EExitMainRoadLeft:
   case EKeepLeft:
      mbmIndex = EMbmWfturnsSmall_keep_left;
      mbmMaskIndex = EMbmWfturnsSmall_keep_left_m;
      break;
   case EExitHighWay:
   case EExitMainRoad:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsSmall_keep_right;
         mbmMaskIndex = EMbmWfturnsSmall_keep_right_m;
      }
      else{
         mbmIndex = EMbmWfturnsSmall_keep_left;
         mbmMaskIndex = EMbmWfturnsSmall_keep_left_m;
      }
      break;
   case EExitHighWayRight:
   case EExitMainRoadRight:
   case EKeepRight:
      mbmIndex = EMbmWfturnsSmall_keep_right;
      mbmMaskIndex = EMbmWfturnsSmall_keep_right_m;
      break;
   case EFerry:
      mbmIndex = EMbmWfturnsSmall_ferry;
      mbmMaskIndex = EMbmWfturnsSmall_ferry_m;
      break;
   case EFinishArrow:
   case EFinishFlag:
      mbmIndex = EMbmWfturnsSmall_flag;
      mbmMaskIndex = EMbmWfturnsSmall_flag_m;
      break;
   case EMultiWayRdb:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsSmall_multiway_rdbt;
         mbmMaskIndex = EMbmWfturnsSmall_multiway_rdbt_m;
      } else {
         mbmIndex = EMbmWfturnsSmall_multiway_rdbt_left;
         mbmMaskIndex = EMbmWfturnsSmall_multiway_rdbt_left_m;
      }
      break;
   case EOffTrack:
      mbmIndex = EMbmWfturnsSmall_offtrack;
      mbmMaskIndex = EMbmWfturnsSmall_offtrack_m;
      break;
   case EWrongDirection:
      mbmIndex = EMbmWfturnsSmall_back_arrow;
      mbmMaskIndex = EMbmWfturnsSmall_back_arrow_m;
      break;
   case EPark:
      mbmIndex = EMbmWfturnsSmall_park_car;
      mbmMaskIndex = EMbmWfturnsSmall_park_car_m;
      break;
   case ERdbUTurn:
   case EUTurn:
      if( !aLeftSide ){
         mbmIndex = EMbmWfturnsSmall_u_turn;
         mbmMaskIndex = EMbmWfturnsSmall_u_turn_m;
      } else {
         mbmIndex = EMbmWfturnsSmall_u_turn_left;
         mbmMaskIndex = EMbmWfturnsSmall_u_turn_left_m;
      }
      break;
   default:
      break;
   }
   //return mbmIndex;
}


TInt TTurnPictures::GetExit( const TInt exit )
{
   TInt mbmIndex = -1;

   switch( exit )
   {
   case 1:
   	mbmIndex = EMbmWfturnsExit1;
   	break;
   case 2:
   	mbmIndex = EMbmWfturnsExit2;
   	break;
   case 3:
      mbmIndex = EMbmWfturnsExit3;
   	break;
   case 4:
   	mbmIndex = EMbmWfturnsExit4;
   	break;
   case 5:
   	mbmIndex = EMbmWfturnsExit5;
   	break;
   case 6:
   	mbmIndex = EMbmWfturnsExit6;
   	break;
   case 7:
   	mbmIndex = EMbmWfturnsExit7;
   	break;
   case 8:
   	mbmIndex = EMbmWfturnsExit8;
   	break;
   case 9:
   	mbmIndex = EMbmWfturnsExit9;
   	break;
   case 10:
   	mbmIndex = EMbmWfturnsExit10;
   	break;
   case 11:
   	mbmIndex = EMbmWfturnsExit11;
   	break;
   case 12:
   	mbmIndex = EMbmWfturnsExit12;
   	break;
   default:
      break;
   }
   return mbmIndex;
}

TInt TTurnPictures::GetExitMask( const TInt exit )
{
   TInt mbmIndex = -1;
#ifdef NAV2_CLIENT_SERIES60_V3
   switch( exit )
   {
   case 1:
   	mbmIndex = EMbmWficonsExit_1_mask;
   	break;
   case 2:
   	mbmIndex = EMbmWficonsExit_2_mask;
   	break;
   case 3:
        mbmIndex = EMbmWficonsExit_3_mask;
   	break;
   case 4:
   	mbmIndex = EMbmWficonsExit_4_mask;
   	break;
   case 5:
   	mbmIndex = EMbmWficonsExit_5_mask;
   	break;
   case 6:
   	mbmIndex = EMbmWficonsExit_6_mask;
   	break;
   case 7:
   	mbmIndex = EMbmWficonsExit_7_mask;
   	break;
   case 8:
   	mbmIndex = EMbmWficonsExit_8_mask;
   	break;
   case 9:
   	mbmIndex = EMbmWficonsExit_9_mask;
   	break;
   case 10:
   	mbmIndex = EMbmWficonsExit_10_mask;
   	break;
   case 11:
   	mbmIndex = EMbmWficonsExit_11_mask;
   	break;
   case 12:
   	mbmIndex = EMbmWficonsExit_12_mask;
   	break;
   default:
      break;
   }
#endif
   return mbmIndex;
}

