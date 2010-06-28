/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlDa.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageDa:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <UndDann> ActionAndWhen

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


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageDa::AudioCtrlLanguageDa() : AudioCtrlLanguageStd()
   {
   }
   class AudioCtrlLanguage* AudioCtrlLanguageDa::New() { 
      return new AudioCtrlLanguageDa(); 
   }

   int AudioCtrlLanguageDa::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(DaSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }
   void AudioCtrlLanguageDa::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(DaSoundOmTuKilometer);
            break;
         case 1000:
            appendClip(DaSoundOmEnKilometer);
            break;
         case 500:
            appendClip(DaSoundOmFemhundredeMeter);
            break;
         case 200:
            appendClip(DaSoundOmTuHundredeMeter);
            break;
         case 100:
            appendClip(DaSoundOmEttHundredeMeter);
            break;
         case 50:
            appendClip(DaSoundOmHalvtredsMeter);
            break;
         case 25:
            appendClip(DaSoundOmFemOgTuveMeter);
            break;
         case 0:
            // Should never happen
            appendClip(DaSoundNu);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageDa::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(DaSoundNu);
         } else {
            appendClip(DaSoundMeddetsamme);
         } 
      } else {
         Distance();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageDa::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(DaSoundDenForste + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageDa::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(DaSoundDrejTillHojre);  /* FIXME - hack */
      } else {
         appendClip(DaSoundTa);
         NumberedExit();
         appendClip(DaSoundVej);
         appendClip(DaSoundIRundkorseln);
      }
   }

   void AudioCtrlLanguageDa::ActionAndWhen()
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

   void AudioCtrlLanguageDa::Action()
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
            appendClip(DaSoundDrejTillVenstre);
            break;
         case Right:
            appendClip(DaSoundDrejTillHojre);
            break;
         case Finally:
            appendClip(DaSoundMaletErRaettfram);
                          // NB! No distance info added to this sound if spokenDist==0
            break;
         case ExitRdbt:
            RoundaboutExit();
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(DaSoundDrejTillHojre); /* FIXME - hack */
            } else {
               appendClip(DaSoundKorLigeUd);
               appendClip(DaSoundIRundkorseln);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(DaSoundDrejTillHojre); /* FIXME - hack */
            } else {
               appendClip(DaSoundDrejTillVenstre);
               appendClip(DaSoundIRundkorseln);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(DaSoundDrejTillHojre); /* FIXME - hack */
            } else {
               appendClip(DaSoundDrejTillHojre);
               appendClip(DaSoundIRundkorseln);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(DaSoundTa);
            appendClip(DaSoundFrakorsel);
            break;
         case KeepLeft:
            appendClip(DaSoundHallTillVenstre);
            break;
         case KeepRight:
            appendClip(DaSoundHallTillHojre);
            break;
         case UTurn:
         case StartWithUTurn:
            appendClip(DaSoundLavEnUVandning);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(DaSoundDrejTillHojre); /* FIXME - hack */
            } else {
               appendClip(DaSoundLavEnUVandning);
               appendClip(DaSoundIRundkorseln);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            return;
      }
   }

   void AudioCtrlLanguageDa::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageDa::AdditionalCrossing()
   {
      appendClip(DaSoundDarefter);
      ActionAndWhen();
   }

   void AudioCtrlLanguageDa::genericDeviatedFromRoute()
   {  
         appendClip(DaSoundDuErAvviketFraRutten);
   }

   void AudioCtrlLanguageDa::genericReachedDest()
   {  
         appendClip(DaSoundDuErFrammeVidMalet);
   }

} /* namespace isab */


