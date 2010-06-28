/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __TURNPICTURES_H__
#define __TURNPICTURES_H__

//#include <e32def.h>
#include "RouteEnums.h"
#include "RouteInfo.h"

//Enum for possible turn actions
enum TPictures{
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
};


/** 
 * @class TTurnPictures
 *
 * @discussion Class for finding the right turn picture to display.
 */
class TTurnPictures
{

public:

   /**
    * Returns the correct action to display,
    * depending of the values
    */
   static TPictures GetPicture( const enum isab::RouteAction aAction,
                                const enum isab::RouteCrossing aCrossing,
                                const TUint aDistance,
                                const TBool aHighway );

   /**
    * Returns the index in the mbm file
    * for the picture of the next action.
    */
   static TInt GetTurnPicture( const TPictures aTurn,
                               const TBool aLeftSide );

   /**
    * Returns the index in the mbm file
    * for the picture of the following action.
    */
   static TInt GetMediumTurnPicture( const TPictures aTurn,
                                     const TBool aLefSide );

   /**
    * Returns the index in the mbm file
    * for the picture of the following action.
    */
   static void GetSmallTurnPicture( const TPictures aTurn,
                                    const TBool aLefSide,
                                    TInt &mbmIndex,
                                    TInt &mbmMaskIndex );

   /**
    * Returns the index in the mbm file
    * for the picture of the exit number.
    */
   static TInt GetExit( const TInt exit );

   static TInt GetExitMask( const TInt exit );


};

#endif // __TURNPICTURES_H__
