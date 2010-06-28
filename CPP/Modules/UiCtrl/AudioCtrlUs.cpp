/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlUs.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageUs:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <AndThen> ActionAndWhen

SoundListPriority := |
                     <DuKorAtFelHall> |
                     <DuHarAvvikitFranRutten> |
                     <DuArFrammeVidDinDest>

ActionAndWhen := Action When |
                 Action

Action := <SvangVanster>      |
          <SvangHoger>        |
          <HallVanster>       |
          <HallHoger>         |
          <KorRaktFram>       |
          <VidDestinationen>  |
          <SvangVanster> <IRondellen> |
          <SvangHoger>   <IRondellen> |
          <KorRaktFram>  <IRondellen> |
          <KorUr>
          RoundaboutExit

RoundaboutExit := <KorUr> |
                  <Ta> NumberedExit <Utfarten> <IRondellen>

When := <Haer> |
        <Om[Distance]>  (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageUs::AudioCtrlLanguageUs() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageUs::New()
   { 
      return new AudioCtrlLanguageUs(); 
   }

   int AudioCtrlLanguageUs::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(UsSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   int AudioCtrlLanguageUs::supportsFeetMiles()
   {
      return 1;
   }

   int AudioCtrlLanguageUs::supportsYardsMiles()
   {
      return 1;
   }

   void AudioCtrlLanguageUs::Distance()
   {

      switch (m_nextXing.spokenDist) {
         /* Metric units */
         case 2000:
            appendClip(UsSoundIn2Kilometers);
            break;
         case 1000:
            appendClip(UsSoundIn1Kilometer);
            break;
         case 500:
            appendClip(UsSoundIn500Meters);
            break;
         case 200:
            appendClip(UsSoundIn200Meters);
            break;
         case 100:
            appendClip(UsSoundIn100Meters);
            break;
         case 50:
            appendClip(UsSoundIn50Meters);
            break;
         case 25:
            appendClip(UsSoundIn25Meters);
            break;

         /* Imperial units - miles */
         case 3219:
            appendClip(UsSoundIn2Miles);
            break;
         case 1609:
            appendClip(UsSoundIn1Mile);
            break;
         case 803:
            appendClip(UsSoundInHalfAMile);
            break;
         case 402:
            appendClip(UsSoundInAQuarterOfAMile);
            break;

         /* Imperial units - feet */
         case 152:
            appendClip(UsSoundIn500Feet);
            break;
         case 61:
            appendClip(UsSoundIn200Feet);
            break;
         case 31:
            appendClip(UsSoundIn100Feet);
            break;

         /* Imperial units - yards */
         case 182:
            appendClip(UsSoundIn200Yards);
            break;
         case 91:
            appendClip(UsSoundIn100Yards);
            break;
         case 46:
            appendClip(UsSoundIn50Yards);
            break;
         case 23:
            appendClip(UsSoundIn25Yards);
            break;

         /* Here */
         case 0:
            // Should never happen
            appendClip(UsSoundHere);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageUs::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(UsSoundHere);
         } else {
            appendClip(UsSoundImmediately);
         }
      } else {
         Distance();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageUs::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(UsSoundTheFirst + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageUs::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(UsSoundExitVerb);
      } else {
         appendClip(UsSoundTake);
         NumberedExit();
         appendClip(UsSoundExitNoun);
         appendClip(UsSoundAtTheRoundabout);
      }
   }

   void AudioCtrlLanguageUs::ActionAndWhen()
   {
      if ( (m_nextXing.spokenDist == 0)                         ||
           (RouteAction(m_nextXing.xing->action) == Finally) 
         ) {
         Action();
         When();
      } else {
         When();
         Action();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageUs::Action()
   {
      switch (RouteAction(m_nextXing.xing->action)) {
         case InvalidAction:
         case Delta:
         case RouteActionMax:
         case End:
         case Start:
            /* Should never occur */
            truncateThisCrossing();
            return;
         case EnterRdbt:
         case Ahead:
         case On:   /* Enter highway from ramp */
         case ParkCar:
         case FollowRoad:
         case EnterFerry:
         case ChangeFerry:
            /* No sound on purpose. */
            truncateThisCrossing();
            return;
         case Left:
            appendClip(UsSoundTurnLeft);
            break;
         case Right:
            appendClip(UsSoundTurnRight);
            break;
         case Finally:
            appendClip(UsSoundAtTheDestination);
                             // NB! No distance info added to this sound if spokenDist==0
            break;
         case ExitRdbt:
            RoundaboutExit();
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(UsSoundExitVerb);
            } else {
               appendClip(UsSoundDriveStraightAhead);
               appendClip(UsSoundAtTheRoundabout);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(UsSoundExitVerb);
            } else {
               appendClip(UsSoundTurnLeft);
               appendClip(UsSoundAtTheRoundabout);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(UsSoundExitVerb);
            } else {
               appendClip(UsSoundTurnRight);
               appendClip(UsSoundAtTheRoundabout);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(UsSoundExitVerb);
            break;
         case KeepLeft:
            appendClip(UsSoundKeepLeft);
            break;
         case KeepRight:
            appendClip(UsSoundKeepRight);
            break;
         case UTurn:
            appendClip(UsSoundMakeAUTurn);
            break;
         case StartWithUTurn:
            appendClip(UsSoundPleaseTurnAround);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(UsSoundExitVerb);
            } else {
               appendClip(UsSoundPleaseTurnAround);
               appendClip(UsSoundAtTheRoundabout);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            return;
      }
   }

   void AudioCtrlLanguageUs::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageUs::AdditionalCrossing()
   {
      appendClip(UsSoundAndThen);
      ActionAndWhen();
   }

   void AudioCtrlLanguageUs::genericDeviatedFromRoute()
   {
      appendClip(UsSoundYouAreNowOffTrack);
   }

   void AudioCtrlLanguageUs::genericReachedDest()
   {
      appendClip(UsSoundYouHaveReachedYourDestination);
   }

} /* namespace isab */


