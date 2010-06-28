/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlNo.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageNo:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <OgSa> ActionAndWhen

SoundListPriority := |
                     <DuKorAtFelHall> |
                     <DuHarAvvikitFranRutten> |
                     <DuArFrammeVidDinDest>

ActionAndWhen := When Action |
                 Action When

Action := <Sving> <TilVanstre>     |
          <Sving> <TilHoyre>     |
          <Hold> <TilVanstre>    | 
          <Hold> <TilHoyre>        | 
          <KjorRettFrem>           |
          <Sving> <TilVanstre>  <IRundkjoringen>  |
          <Sving> <TilHoyre>    <IRundkjoringen>  |
          <KjorRettFrem>       <IRundkjoringen>  |
          <KjorHeleVeienRundt> <IRundkjoringen>  |
          <VennligstSnu>             |
          <GjorEnUSvingNarDetBlirMulig> |
          <ForlatHovedveien>             |
          RoundaboutExit

RoundaboutExit := <KjorUt> |
                  <Ta> NumberedExit <Avkjoring> <IRundkjoringen>

When := <Om> <[Distance]>  (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)
        <Her>
        <Umiddelbart>


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageNo::AudioCtrlLanguageNo() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageNo::New() { 
      return new AudioCtrlLanguageNo(); 
   }

   int AudioCtrlLanguageNo::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(NoSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   void AudioCtrlLanguageNo::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(NoSound2);
            appendClip(NoSoundKilometer);
            break;
         case 1000:
            appendClip(NoSound1);
            appendClip(NoSoundKilometer);
            break;
         case 500:
            appendClip(NoSound500);
            appendClip(NoSoundMeter);
            break;
         case 200:
            appendClip(NoSound200);
            appendClip(NoSoundMeter);
            break;
         case 100:
            appendClip(NoSound100);
            appendClip(NoSoundMeter);
            break;
         case 50:
            appendClip(NoSound50);
            appendClip(NoSoundMeter);
            break;
         case 25:
            appendClip(NoSound25);
            appendClip(NoSoundMeter);
            break;
         case 0:
            // Should never happen
            appendClip(NoSoundHer);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageNo::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(NoSoundHer);
         } else {
            appendClip(NoSoundUmiddelbart);
         }
      } else {
         appendClip(NoSoundOm);
         Distance();
      }
   }

   void AudioCtrlLanguageNo::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(NoSoundForste + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageNo::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(NoSoundKjorUt);
      } else {
         appendClip(NoSoundTa);
         NumberedExit();
         appendClip(NoSoundAvkjoring);
         appendClip(NoSoundIRundkjoringen);
      }
   }

   void AudioCtrlLanguageNo::ActionAndWhen()
   {
      if ( (m_nextXing.spokenDist == 0) &&
           (m_nextXing.crossingNum == 1) ) {
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

   void AudioCtrlLanguageNo::Action() 
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
            appendClip(NoSoundSving);
            appendClip(NoSoundTilVenstre);
            break;
         case Right:
            appendClip(NoSoundSving);
            appendClip(NoSoundTilHoyre);
            break;
         case Finally:
            appendClip(NoSoundVedDestinasjonen);
            break;
         case ExitRdbt:
            RoundaboutExit();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(NoSoundKjorUt);
            } else {
               appendClip(NoSoundKjorRettFrem);
               appendClip(NoSoundIRundkjoringen);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(NoSoundKjorUt);
            } else {
               appendClip(NoSoundSving);
               appendClip(NoSoundTilVenstre);
               appendClip(NoSoundIRundkjoringen);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(NoSoundKjorUt);
            } else {
               appendClip(NoSoundSving);
               appendClip(NoSoundTilHoyre);
               appendClip(NoSoundIRundkjoringen);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(NoSoundForlatHovedveien);
            break;
         case KeepLeft:
            appendClip(NoSoundHold);
            appendClip(NoSoundTilVenstre);
            break;
         case KeepRight:
            appendClip(NoSoundHold);
            appendClip(NoSoundTilHoyre);
            break;
         case UTurn:
            appendClip(NoSoundVennligstSnu);
            break;
         case StartWithUTurn:
            appendClip(NoSoundGjorEnUSvingNarDetBlirMulig);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(NoSoundKjorUt);
            } else {
               appendClip(NoSoundKjorHeleVeienRundt);
               appendClip(NoSoundIRundkjoringen);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguageNo::AdditionalCrossing()
   {
      appendClip(NoSoundOgSa);
      ActionAndWhen();
   }

   void AudioCtrlLanguageNo::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageNo::genericDeviatedFromRoute()
   {  
      appendClip(NoSoundDuErUtenforRuten);
   }

   void AudioCtrlLanguageNo::genericReachedDest()
   {  
      appendClip(NoSoundDuErFremmeVedDestinasjonen);
   }

} /* namespace isab */


