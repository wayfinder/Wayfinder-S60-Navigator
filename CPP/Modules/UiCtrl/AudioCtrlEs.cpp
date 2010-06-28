/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlEs.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageEs:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <Quendi> ActionAndWhen

SoundListPriority := |
                     <DuKorAtFelHall> |
                     <DuHarAvvikitFranRutten> |
                     <DuArFrammeVidDinDest>

ActionAndWhen := When Action |
                 Action <Qui>
                 <ADestinazione> When

Action := <SvoltaASinistra>     |
          <SvoltaADestra>     |
          <MantieniLaSinistra> |
          <MantieniLaDestra>   |
          <ProcedoDiritto>           |
          <SvoltaASinistra> <AllaRotonda> |
          <SvoltaADestra>   <AllaRotonda> |
          <ProcedoDiritto>  <AllaRotonda> |
          <EseguiUnInversioneAU> <AllaRotonda> |
          <EsciDallAutostrada>             |
          <EseguiUnInversioneAU>           |
          <QuandoPossibile>                |
          RoundaboutExit

RoundaboutExit := <Esci> <Qui> |
                  <Prendi> NumberedExit <Uscita> <AllaRotunda>

When := <Qui> |
        <Tra[Distance]>  (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageEs::AudioCtrlLanguageEs() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageEs::New() { 
      return new AudioCtrlLanguageEs(); 
   }

   int AudioCtrlLanguageEs::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(EsSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   void AudioCtrlLanguageEs::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(EsSoundEn2Kilometros);
            break;
         case 1000:
            appendClip(EsSoundEn1Kilometro);
            break;
         case 500:
            appendClip(EsSoundEn500Metros);
            break;
         case 200:
            appendClip(EsSoundEn200Metros);
            break;
         case 100:
            appendClip(EsSoundEn100Metros);
            break;
         case 50:
            appendClip(EsSoundEn50Metros);
            break;
         case 25:
            appendClip(EsSoundEn25Metros);
            break;
         case 0:
            // Should never happen
            appendClip(EsSoundAqui);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageEs::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(EsSoundAqui);
         } else {
            appendClip(EsSoundInmeditamente);
         }
      } else {
         Distance();
      }
   }

   void AudioCtrlLanguageEs::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(EsSoundLaPrimera + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageEs::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(EsSoundSal);
      } else {
         appendClip(EsSoundToma);
         NumberedExit();
         appendClip(EsSoundSalida);
         appendClip(EsSoundEnLaRotonda);
      }
   }

   void AudioCtrlLanguageEs::ActionAndWhen()
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

      if (m_nextXing.spokenDist != 0) {
         When();
         Action();
      } else {
         Action();
         When();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageEs::Action() 
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
            appendClip(EsSoundGiraALaIzquierda);
            break;
         case Right:
            appendClip(EsSoundGiraALaDerecha);
            break;
         case Finally:
            // Can not happen, handled before the switch statement.
            appendClip(EsSoundEnElDestino);
            break;
         case ExitRdbt:
            RoundaboutExit();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(EsSoundSal);
            } else {
               appendClip(EsSoundConduceTodoRecto);
               appendClip(EsSoundEnLaRotonda);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(EsSoundSal);
            } else {
               appendClip(EsSoundGiraALaIzquierda);
               appendClip(EsSoundEnLaRotonda);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(EsSoundSal);
            } else {
               appendClip(EsSoundGiraALaDerecha);
               appendClip(EsSoundEnLaRotonda);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(EsSoundSalDeLaAutopista);
            break;
         case KeepLeft:
            appendClip(EsSoundMantenteALaIzquierda);
            break;
         case KeepRight:
            appendClip(EsSoundMantenteALaDerecha);
            break;
         case UTurn:
            appendClip(EsSoundCambiaDeSentido);
            break;
         case StartWithUTurn:
            appendClip(EsSoundCuandoPuedas);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(EsSoundSal);
            } else {
               appendClip(EsSoundCambiaDeSentido);
               appendClip(EsSoundEnLaRotonda);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguageEs::AdditionalCrossing()
   {
      appendClip(EsSoundYDespues);
      ActionAndWhen();
   }

   void AudioCtrlLanguageEs::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageEs::genericDeviatedFromRoute()
   {  
      appendClip(EsSoundEstasFueraDeLaRuta);
   }

   void AudioCtrlLanguageEs::genericReachedDest()
   {  
      appendClip(EsSoundHasLlegadoATuDestino);
   }

} /* namespace isab */


