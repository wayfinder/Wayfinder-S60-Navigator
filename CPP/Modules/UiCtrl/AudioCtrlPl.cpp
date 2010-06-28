/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlPl.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguagePl:

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

   AudioCtrlLanguagePl::AudioCtrlLanguagePl() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguagePl::New() { 
      return new AudioCtrlLanguagePl(); 
   }

   int AudioCtrlLanguagePl::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(PlSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   int AudioCtrlLanguagePl::supportsFeetMiles()
   {
      return 1;
   }

   int AudioCtrlLanguagePl::supportsYardsMiles()
   {
      return 1;
   }

   void AudioCtrlLanguagePl::Distance()
   {

      switch (m_nextXing.spokenDist) {
         /* Metric units */
         case 2000:
            appendClip(PlSoundZa2Kilometry);
            break;
         case 1000:
            appendClip(PlSoundZa1Kilometr);
            break;
         case 500:
            appendClip(PlSoundZa500Metrow);
            break;
         case 200:
            appendClip(PlSoundZa200Metrow);
            break;
         case 100:
            appendClip(PlSoundZa100Metrow);
            break;
         case 50:
            appendClip(PlSoundZa50Metrow);
            break;
         case 25:
            appendClip(PlSoundZa25Metrow);
            break;

         /* Imperial units - miles */
         case 3219:
            appendClip(PlSoundZa2Mile);
            break;
         case 1609:
            appendClip(PlSoundZa1Mile);
            break;
         case 803:
            appendClip(PlSoundZaPuMili);
            break;
         case 402:
            appendClip(PlSoundZaJednotwartoMili);
            break;

         /* Imperial units - feet */
         case 152:
            appendClip(PlSoundZa500Stop);
            break;
         case 61:
            appendClip(PlSoundZa200Stop);
            break;
         case 31:
            appendClip(PlSoundZa100Stop);
            break;

         /* Imperial units - yards */
         case 182:
            appendClip(PlSoundZa200Jardow);
            break;
         case 91:
            appendClip(PlSoundZa100Jardow);
            break;
         case 46:
            appendClip(PlSoundZa50Jardow);
            break;
         case 23:
            appendClip(PlSoundZa25Jardow);
            break;

         /* Here */
         case 0:
            // Should never happen
            appendClip(PlSoundTutaj);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguagePl::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(PlSoundTutaj);
         } else {
            appendClip(PlSoundNatychmiast);
         }
      } else {
         Distance();
      }
   }

   void AudioCtrlLanguagePl::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(PlSoundWPierwszy + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguagePl::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(PlSoundJedz);
      } else {
         appendClip(PlSoundNaRondzie);
         appendClip(PlSoundZjedz);
         NumberedExit();
         appendClip(PlSoundZjazd);
      }
   }

   void AudioCtrlLanguagePl::ActionAndWhen()
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

   void AudioCtrlLanguagePl::Action() 
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
            appendClip(PlSoundSkrecWLewo);
            break;
         case Right:
            appendClip(PlSoundSkrecWPrawo);
            break;
         case Finally:
            appendClip(PlSoundWMiejscuPrzeznaczenia);
            break;
         case ExitRdbt:
            RoundaboutExit();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(PlSoundJedz);
            } else {
               appendClip(PlSoundNaRondzie);
               appendClip(PlSoundJedzProsto);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(PlSoundJedz);
            } else {
               appendClip(PlSoundNaRondzie);
               appendClip(PlSoundSkrecWLewo);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(PlSoundJedz);
            } else {
               appendClip(PlSoundNaRondzie);
               appendClip(PlSoundSkrecWPrawo);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(PlSoundJedz);
            break;
         case KeepLeft:
            appendClip(PlSoundTrzymajSieLewej);
            break;
         case KeepRight:
            appendClip(PlSoundTrzymajSiePrawej);
            break;
         case UTurn:
            appendClip(PlSoundProszeZawrocic);
            break;
         case StartWithUTurn:
            appendClip(PlSoundZawrocGdyBedzieToMozliwe);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(PlSoundJedz);
            } else {
               appendClip(PlSoundNaRondzie);
               appendClip(PlSoundProszeZawrocic);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguagePl::AdditionalCrossing()
   {
      appendClip(PlSoundAPotem);
      ActionAndWhen();
   }

   void AudioCtrlLanguagePl::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguagePl::genericDeviatedFromRoute()
   {  
      appendClip(PlSoundWlasnie);
   }

   void AudioCtrlLanguagePl::genericReachedDest()
   {  
      appendClip(PlSoundDojechales);
   }

} /* namespace isab */


