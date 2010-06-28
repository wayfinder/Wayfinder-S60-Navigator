/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __WAYFINDER_CONSTANTS__
#define __WAYFINDER_CONSTANTS__

#include <gdi.h>
#include "wayfinder.hrh"

static const TInt KBackgroundRed = 188; //196;

static const TInt KBackgroundGreen = 209; //223;

static const TInt KBackgroundBlue = 231; //155;

static const TInt KBuf32Length = 32;

static const TInt KBuf64Length = 64;

static const TInt KBuf128Length = 128;

static const TInt KBuf256Length = 256;

static const TInt KBuf512Length = 512;

static const TInt KHeaderSize = 4;

static const TInt KMaxMessageLength = 70*1024;

static const TReal K_PI = 3.14159265358979323846;

_LIT(KTab,"\t");
_LIT(KEmpty,"");

_LIT(KIniFileName, "wayfinder.ini");
_LIT(KLangFileName, "lang.txt");


_LIT( KGif3WayLeft,           "3way_left.png" );
_LIT( KGif3WayRight,          "3way_right.png" );
_LIT( KGif3WayTeeLeft,        "3way_T_left.png" );
_LIT( KGif3WayTeeRight,       "3way_T_right.png" );
_LIT( KGif4WayLeft,           "4way_left.png" );
_LIT( KGif4WayRight,          "4way_right.png" );
_LIT( KGif4WayStraight,       "4way_straight.png" );
_LIT( KGifEnterHighWay,       "enter_highway.png" );
_LIT( KGifEnterHighWayLeft,   "enter_highway_left.png" );
_LIT( KGifEnterMainRoad,      "enter_main_road.png" );
_LIT( KGifEnterMainRoadLeft,  "enter_main_road_left.png" );
_LIT( KGifExitHighWay,        "exit_highway.png" );
_LIT( KGifExitHighWayLeft,    "exit_highway_left.png" );
_LIT( KGifExitMainRoad,       "exit_main_road.png" );
_LIT( KGifExitMainRoadLeft,   "exit_main_road_left.png" );
_LIT( KGifFerry,              "ferry.png" );
_LIT( KGifFinishArrow,        "finish_arrow.png" );
_LIT( KGifFinishFlag,         "finish_flag.png" );
_LIT( KGifHighWayStraight,    "highway_straight.png" );
_LIT( KGifHighWayStraightLeft,"highway_straight_left.png" );
_LIT( KGifKeepLeft,           "keep_left.png" );
_LIT( KGifKeepRight,          "keep_right.png" );
_LIT( KGifLeftArrow,          "left_arrow.png" );
_LIT( KGifMultiWayRdb,        "multiway_rdbt.png" );
_LIT( KGifMultiWayRdbLeft,    "multiway_rdbt_left.png" );
_LIT( KGifOffTrack,           "offtrack.png" );
_LIT( KGifWrongDirection,     "opposite_arrow.png" );
_LIT( KGifPark,               "park_car.png" );
_LIT( KGifRdbLeft,            "rdbt_left.png" );
_LIT( KGifRdbLeftLeft,        "rdbt_left_left.png" );
_LIT( KGifRdbRight,           "rdbt_right.png" );
_LIT( KGifRdbRightLeft,       "rdbt_right_left.png" );
_LIT( KGifRdbStraight,        "rdbt_straight.png" );
_LIT( KGifRdbStraightLeft,    "rdbt_straight_left.png" );
_LIT( KGifRdbUTurn,           "rdbt_uturn.png" );
_LIT( KGifRdbUTurnLeft,       "rdbt_uturn_left.png" );
_LIT( KGifRightArrow,         "right_arrow.png" );
_LIT( KGifStraight,           "straight_ahead.png" );
_LIT( KGifStraightArrow,      "straight_arrow.png" );
_LIT( KGifUTurn,              "u_turn.png" );
_LIT( KGifUTurnLeft,          "u_turn_left.png" );

_LIT( KGifSmallFerry,         "small_ferry.gif" );
_LIT( KGifSmallFlag,          "small_flag.gif" );
_LIT( KGifSmallKeepLeft,      "small_keep_left.gif" );
_LIT( KGifSmallKeepRight,     "small_keep_right.gif" );
_LIT( KGifSmallLeftArrow,     "small_left_arrow.gif" );
_LIT( KGifSmallMultiWayRdb,   "small_multiway_rdbt.gif" );
_LIT( KGifSmallMultiWayRdbLeft,"small_multiway_rdbt_left.gif" );
_LIT( KGifSmallPark,          "small_park_car.gif" );
_LIT( KGifSmallRightArrow,    "small_right_arrow.gif" );
_LIT( KGifSmallStraightArrow, "small_straight_arrow.gif" );
_LIT( KGifSmallUTurn,         "small_u_turn.gif" );
_LIT( KGifSmallUTurnLeft,     "small_u_turn_left.gif" );

_LIT( KGifSmallDetour,        "detour.gif" );
_LIT( KGifSmallSpeedCam,      "speed_cam.gif" );
_LIT( KGifSmallBackArrow,     "small_back_arrow.gif" );
_LIT( KGifSmallOfftrack,      "small_offtrack.gif" );

_LIT( KGifNews,               "wayfinder_news.png" );
_LIT( KSvgNews,               "wayfinder_news.mif" );
/* _LIT( KGifWelcome,            "wayfinder_welcome.png" ); */


// Enumeration of custom messages sent to views.
// being activated
enum TCustomMessages
{
   ENoMessage = 0,
   ESearchReply,
   ERouteMap,
   EDestinationMap,
   EFavoriteMap,
   EWhereAmIMap,
   ESetOrigin,
   ESetDestination,
   ESetOriginFromCat,
   ESetDestinationFromCat,
   ENewSearch,
   EShutdown,
   EFromNewDest,
   EFromConnect,
   EShowUpgrade,
   EShowNews,
   EFullScreen,
   EBackOnExit,
   EExitOnExit,
   EBackHistory,
   EBackHistoryThenView,
   EActivateLastState
};

enum PositionType{
   ESearchResult = 0,
   EFavorite,
   ECoordinate,
};

/*enum TPictures{
   E3WayLeft,
   E3WayRight,
   E3WayTeeLeft,
   E3WayTeeRight,
   E4WayLeft,
   E4WayRight,
   E4WayStraight,
   EEnterHighWay,
   EEnterMainRoad,
   EExitHighWay,
   EExitHighWayRight,
   EExitHighWayLeft,
   EExitMainRoad,
   EExitMainRoadRight,
   EExitMainRoadLeft,
   EFerry,
   EFinishArrow,
   EFinishFlag,
   EHighWayStraight,
   EKeepLeft,
   EKeepRight,
   ELeftArrow,
   EMultiWayRdb,
   EOffTrack,
   EWrongDirection,
   EPark,
   ERdbLeft,
   ERdbRight,
   ERdbStraight,
   ERdbUTurn,
   ERightArrow,
   EStart,
   EStraight,
   EStraightArrow,
   EUTurn,
   EDetour,
   ESpeedCam,
   ENoPicture
};*/

enum TrackingType{
   ERotating = 0,
   ENorthUp,
};

enum MapType{
   EVector = 0,
   EBitmap,
};

const TUid KStartPageViewId =      {EWayFinderStartPageTab};
const TUid KPositionSelectViewId = {EWayFinderPositionSelectTab};
const TUid KNewDestViewId =        {EWayFinderNewDestTab};
const TUid KMyDestViewId =         {EWayFinderMyDestTab};
const TUid KContactsViewId =       {EWayFinderConnectTab};
//const TUid KCallCenterViewId =     {EWayFinderCallCenterTab}; //Deprecated
const TUid KServiceWindowViewId =  {EWayFinderContentWindowTab};
const TUid KSettingsViewId =       {EWayFinderSettingsTab};
const TUid KInfoViewId =           {EWayFinderInfoTab};
const TUid KMapViewId =            {EWayFinderMapTab};
const TUid KGuideViewId =          {EWayFinderGuideTab};
const TUid KItineraryViewId =      {EWayFinderItineraryTab};
const TUid KWelcomeViewId =        {EWayFinderWelcomeTab};
const TUid KDestinationViewId =    {EWayFinderDestinationTab};
const TUid KVicinityViewAllId =    {EWayFinderVicinityAllTab};
const TUid KVicinityViewCrossingId = {EWayFinderVicinityCrossingTab};
const TUid KVicinityViewFavouriteId = {EWayFinderVicinityFavouriteTab};
const TUid KVicinityViewPOIId =    {EWayFinderVicinityPOITab};
const TUid KLockedNavViewId =    {EWayFinderLockedNavTab};
const TUid KVicinityFeedId =    {EWayFinderVicinityFeedTab};

extern const TInt32 DefaultVectorMapCenterLat;
extern const TInt32 DefaultVectorMapCenterLon;

#endif //WAYFINDER_CONSTANTS__
