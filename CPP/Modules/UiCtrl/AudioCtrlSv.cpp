/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "AudioCtrlSv.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageSv:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen
AdditionalCrossing := <OchDarefter> ActionAndWhen

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
          <KorUr>                     |
          RoundaboutExit

RoundaboutExit := <KorUr> |
                  <Ta> NumberedExit <Utfarten> <IRondellen>

When := <Haer> |
        <Om[Distance]>  (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)

NextSound := |
             <AndThen> ActionAndWhen


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageSv::AudioCtrlLanguageSv() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageSv::New()
   { 
      return new AudioCtrlLanguageSv();
   }

   
   int AudioCtrlLanguageSv::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(SvSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   void AudioCtrlLanguageSv::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(SvSoundOmTvaKilometer);
            break;
         case 1000:
            appendClip(SvSoundOmEnKilometer);
            break;
         case 500:
            appendClip(SvSoundOmFemHundraMeter);
            break;
         case 200:
            appendClip(SvSoundOmTvaHundraMeter);
            break;
         case 100:
            appendClip(SvSoundOmEttHundraMeter);
            break;
         case 50:
            appendClip(SvSoundOmFemtioMeter);
            break;
         case 25:
            appendClip(SvSoundOmTjugofemMeter);
            break;
         case 0:
            // Should never happen
            appendClip(SvSoundHaer);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageSv::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(SvSoundHaer);
         } else {
            appendClip(SvSoundDirekt);
         }
      } else {
         Distance();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageSv::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(SvSoundForsta + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageSv::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(SvSoundKorUr);
      } else {
         appendClip(SvSoundTa);
         NumberedExit();
         appendClip(SvSoundUtfarten);
         appendClip(SvSoundIRondellen);
      }
   }

   void AudioCtrlLanguageSv::ActionAndWhen()
   {
      Action();
      if (m_soundState.noMoreSounds) {
         return;
      }

      if ( (RouteAction(m_nextXing.xing->action) == Finally) &&
           (m_nextXing.spokenDist == 0) ) {
         return;    // NB! No distance info added to this sound if spokenDist==0
      }

      When();
   }

   void AudioCtrlLanguageSv::Action()
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
            appendClip(SvSoundSvangVanster);
            break;
         case Right:
            appendClip(SvSoundSvangHoger);
            break;
         case Finally:
            appendClip(SvSoundVidDestinationen);
                              // NB! No distance info added to this sound if spokenDist==0
            break;
         case ExitRdbt:
            RoundaboutExit();
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(SvSoundKorUr);
            } else {
               appendClip(SvSoundKorRaktFram);
               appendClip(SvSoundIRondellen);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(SvSoundKorUr);
            } else {
               appendClip(SvSoundSvangVanster);
               appendClip(SvSoundIRondellen);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(SvSoundKorUr);
            } else {
               appendClip(SvSoundSvangHoger);
               appendClip(SvSoundIRondellen);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(SvSoundKorAv);
            break;
         case KeepLeft:
            appendClip(SvSoundHaallVanster);
            break;
         case KeepRight:
            appendClip(SvSoundHaallHoger);
            break;
         case UTurn:
         case StartWithUTurn:
            appendClip(SvSoundGorEnUSvang);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(SvSoundKorUr);
            } else {
               appendClip(SvSoundGorEnUSvang);
               appendClip(SvSoundIRondellen);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            return;
      }
   }

   void AudioCtrlLanguageSv::AdditionalCrossing()
   {
      appendClip(SvSoundOchDarefter);
      ActionAndWhen();
   }

   void AudioCtrlLanguageSv::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageSv::genericDeviatedFromRoute()
   {
      appendClip(SvSoundDuHarAvvikitFranRutten);
   }

   void AudioCtrlLanguageSv::genericReachedDest()
   {
      appendClip(SvSoundDuArFrammeVidDinDest);
   }

} /* namespace isab */


