/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlIt.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageIt:

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

   AudioCtrlLanguageIt::AudioCtrlLanguageIt() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageIt::New() { 
      return new AudioCtrlLanguageIt(); 
   }
   
   int AudioCtrlLanguageIt::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(ItSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   void AudioCtrlLanguageIt::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(ItSoundTra2Chilometri);
            break;
         case 1000:
            appendClip(ItSoundTra1Chilometro);
            break;
         case 500:
            appendClip(ItSoundTra500Metri);
            break;
         case 200:
            appendClip(ItSoundTra200Metri);
            break;
         case 100:
            appendClip(ItSoundTra100Metri);
            break;
         case 50:
            appendClip(ItSoundTra50Metri);
            break;
         case 25:
            appendClip(ItSoundTra25Metri);
            break;
         case 0:
            // Should never happen
            appendClip(ItSoundQui);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageIt::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(ItSoundQui);
         } else {
            appendClip(ItSoundImmediatamente);
         }
      } else {
         Distance();
      }
   }

   void AudioCtrlLanguageIt::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(ItSoundLaPrima + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageIt::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(ItSoundEsci);
      } else {
         appendClip(ItSoundAllaRotonda);
         appendClip(ItSoundPrendi);
         NumberedExit();
         appendClip(ItSoundUscita);
      }
   }

   void AudioCtrlLanguageIt::ActionAndWhen()
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

   void AudioCtrlLanguageIt::Action() 
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
            appendClip(ItSoundSvoltaASinistra);
            break;
         case Right:
            appendClip(ItSoundSvoltaADestra);
            break;
         case Finally:
            // Can not happen, handled before the switch statement.
            appendClip(ItSoundADestinazione);
            break;
         case ExitRdbt:
            RoundaboutExit();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(ItSoundEsci);
            } else {
               appendClip(ItSoundAllaRotonda);
               appendClip(ItSoundProcedoDiritto);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(ItSoundEsci);
            } else {
               appendClip(ItSoundAllaRotonda);
               appendClip(ItSoundSvoltaASinistra);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(ItSoundEsci);
            } else {
               appendClip(ItSoundAllaRotonda);
               appendClip(ItSoundSvoltaADestra);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(ItSoundEsciDallAutostrada);
            break;
         case KeepLeft:
            appendClip(ItSoundMantieniLaSinistra);
            break;
         case KeepRight:
            appendClip(ItSoundMantieniLaDestra);
            break;
         case UTurn:
            appendClip(ItSoundEseguiUnInversioneAU);
            break;
         case StartWithUTurn:
            appendClip(ItSoundQuandoPossibile);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(ItSoundEsci);
            } else {
               appendClip(ItSoundAllaRotonda);
               appendClip(ItSoundEseguiUnInversioneAU);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguageIt::AdditionalCrossing()
   {
      appendClip(ItSoundQuindi);
      ActionAndWhen();
   }

   void AudioCtrlLanguageIt::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageIt::genericDeviatedFromRoute()
   {  
      appendClip(ItSoundSeiFuoriStrada);
   }

   void AudioCtrlLanguageIt::genericReachedDest()
   {  
      appendClip(ItSoundHaiRaggiunto);
   }

} /* namespace isab */


