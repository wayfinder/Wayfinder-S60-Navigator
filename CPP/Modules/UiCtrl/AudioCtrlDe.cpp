/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlDe.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageDe:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <UndDann> ActionAndWhen

SoundListPriority := |
                     <DuKorAtFelHall> |
                     <DuHarAvvikitFranRutten> |
                     <DuArFrammeVidDinDest>

ActionAndWhen := When Action |
                 <AmZiel> When

Action := <LinksAbbiegen>     |
          <RechtsAbbiegen>     |
          <Links> <Halten>    |   FIXME - XXX
          <Rechts> <Halten>   |   FIXME - XXX
          <GeradeausWeiterfahren>           |
          <ImKreisel> <LinksAbbiegen>   |
          <ImKreisel> <RechstsAbbiegen> |
          <ImKreisel> <GeradeausWeiterfahren> |
          <ImKreisel> <BitteWenden> |
          <BitteWenden>             |
          <WennMoglichBitteWenden> |
          <Abfahren>             |
          RoundaboutExit

RoundaboutExit := <Abfahren> |
                  <ImKreisel> NumberedExit <Ausfahrt>

When := <Hier> |
        <Om[Distance]>  (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageDe::AudioCtrlLanguageDe() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageDe::New() { 
      return new AudioCtrlLanguageDe(); 
   }
   
   int AudioCtrlLanguageDe::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(DeSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }
   
   void AudioCtrlLanguageDe::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(DeSoundInZweiKilometern);
            break;
         case 1000:
            appendClip(DeSoundInEinemKilometer);
            break;
         case 500:
            appendClip(DeSoundInFunfhundertMetern);
            break;
         case 200:
            appendClip(DeSoundInZweihundertMetern);
            break;
         case 100:
            appendClip(DeSoundInEinhundertMetern);
            break;
         case 50:
            appendClip(DeSoundInFunfzigMetern);
            break;
         case 25:
            appendClip(DeSoundInFunfUndZwanzigMetern);
            break;
         case 0:
            // Should never happen
            appendClip(DeSoundHier);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageDe::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(DeSoundHier);
         } else {
            appendClip(DeSoundSofort);
         }
      } else {
         Distance();
      }
   }

   void AudioCtrlLanguageDe::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(DeSoundDieErste + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageDe::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(DeSoundAbfahren);
      } else {
         appendClip(DeSoundImKreisel);
         NumberedExit();
         appendClip(DeSoundAusfahrt);
      }
   }

   void AudioCtrlLanguageDe::ActionAndWhen()
   {
      if (RouteAction(m_nextXing.xing->action) == Finally) {
         Action();
         if (m_nextXing.spokenDist == 0)
            return;    // NB! No distance info added to this sound if spokenDist==0
         When();
         if (m_nextXing.setTimingMarker) {
            appendClip(SoundTimingMarker);
         }
         return;
      }

      When();
      Action();
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageDe::Action() 
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
            appendClip(DeSoundLinksAbbiegen);
            break;
         case Right:
            appendClip(DeSoundRechtsAbbiegen);
            break;
         case Finally:
            appendClip(DeSoundAmZiel);
            break;
         case ExitRdbt:
            RoundaboutExit();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(DeSoundAbfahren);
            } else {
               appendClip(DeSoundImKreisel);
               appendClip(DeSoundGeradeausWeiterfahren);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(DeSoundAbfahren);
            } else {
               appendClip(DeSoundImKreisel);
               appendClip(DeSoundLinksAbbiegen);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(DeSoundAbfahren);
            } else {
               appendClip(DeSoundImKreisel);
               appendClip(DeSoundRechtsAbbiegen);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(DeSoundAbfahren);
            break;
         case KeepLeft:
            appendClip(DeSoundLinksHalten);
            break;
         case KeepRight:
            appendClip(DeSoundRechtsHalten);
            break;
         case UTurn:
            appendClip(DeSoundBitteWenden);
            break;
         case StartWithUTurn:
            appendClip(DeSoundWennMoglichBitteWenden);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(DeSoundAbfahren);
            } else {
               appendClip(DeSoundImKreisel);
               appendClip(DeSoundBitteWenden);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguageDe::AdditionalCrossing()
   {
      appendClip(DeSoundUndDann);
      ActionAndWhen();
   }

   void AudioCtrlLanguageDe::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageDe::genericDeviatedFromRoute()
   {  
      appendClip(DeSoundSieHabenJetztDieGeplanteRouteVerlassen);
   }

   void AudioCtrlLanguageDe::genericReachedDest()
   {  
      appendClip(DeSoundSieHabenDasZielErreicht);
   }

} /* namespace isab */


