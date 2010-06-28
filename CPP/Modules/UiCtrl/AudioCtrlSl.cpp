/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlSl.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageSl:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <AndThen> ActionAndWhen

SoundListPriority := |
                     <DuKorAtFelHall> |
                     <DuHarAvvikitFranRutten> |
                     <DuArFrammeVidDinDest>

ActionAndWhen := When Action |
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

   AudioCtrlLanguageSl::AudioCtrlLanguageSl() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageSl::New()
   { 
      return new AudioCtrlLanguageSl(); 
   }

   int AudioCtrlLanguageSl::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(SlSoundKameriZaNadzorHitrosti);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   int AudioCtrlLanguageSl::supportsFeetMiles()
   {
      return 0;
   }

   int AudioCtrlLanguageSl::supportsYardsMiles()
   {
      return 0;
   }

   void AudioCtrlLanguageSl::Distance()
   {

      switch (m_nextXing.spokenDist) {
         /* Metric units */
         case 2000:
            appendClip(SlSoundCez2Kilometra);
            break;
         case 1000:
            appendClip(SlSoundCez1Kilometer);
            break;
         case 500:
            appendClip(SlSoundCez500Metrov);
            break;
         case 200:
            appendClip(SlSoundCez200Metrov);
            break;
         case 100:
            appendClip(SlSoundCez100Metrov);
            break;
         case 50:
            appendClip(SlSoundCez50Metrov);
            break;
         case 25:
            appendClip(SlSoundCez25Metrov);
            break;

         /* Here */
         case 0:
            // Should never happen
            appendClip(SlSoundTukaj);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageSl::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(SlSoundTukaj);
         } else {
            appendClip(SlSoundNemudoma);
         }
      } else {
         Distance();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageSl::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(SlSoundPrvi + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageSl::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(SlSoundZavijte);
      } else {
         appendClip(SlSoundIzberite);
         NumberedExit();
         appendClip(SlSoundIzhod);
         appendClip(SlSoundNaKroznemKriziscu);
      }
   }

   void AudioCtrlLanguageSl::ActionAndWhen()
   {
      When();
      Action();
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageSl::Action()
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
            appendClip(SlSoundZavijteLevo);
            break;
         case Right:
            appendClip(SlSoundZavijteDesno);
            break;
         case Finally:
            appendClip(SlSoundNaCilju);
                             // NB! No distance info added to this sound if spokenDist==0
            break;
         case ExitRdbt:
            RoundaboutExit();
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(SlSoundZavijte);
            } else {
               appendClip(SlSoundZapeljiteNaravnost);
               appendClip(SlSoundNaKroznemKriziscu);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(SlSoundZavijte);
            } else {
               appendClip(SlSoundZavijteLevo);
               appendClip(SlSoundNaKroznemKriziscu);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(SlSoundZavijte);
            } else {
               appendClip(SlSoundZavijteDesno);
               appendClip(SlSoundNaKroznemKriziscu);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(SlSoundZavijte);
            break;
         case KeepLeft:
            appendClip(SlSoundOstaniteNaLevemPasu);
            break;
         case KeepRight:
            appendClip(SlSoundOstaniteNaDesnemPasu);
            break;
         case UTurn:
            appendClip(SlSoundProsimoObrnite);
            break;
         case StartWithUTurn:
            appendClip(SlSoundKoBoMogoceNareditePolkrozniZavoj);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(SlSoundZavijte);
            } else {
               appendClip(SlSoundProsimoObrnite);
               appendClip(SlSoundNaKroznemKriziscu);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            return;
      }
   }

   void AudioCtrlLanguageSl::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageSl::AdditionalCrossing()
   {
      appendClip(SlSoundInPotem);
      ActionAndWhen();
   }

   void AudioCtrlLanguageSl::genericDeviatedFromRoute()
   {
      appendClip(SlSoundZdajSteZasliSPoti);
   }

   void AudioCtrlLanguageSl::genericReachedDest()
   {
      appendClip(SlSoundPrisliSteNaSvojCilj);
   }

} /* namespace isab */


