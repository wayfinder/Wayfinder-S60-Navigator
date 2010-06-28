/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlPt.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguagePt:

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

   AudioCtrlLanguagePt::AudioCtrlLanguagePt() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguagePt::New() { 
      return new AudioCtrlLanguagePt(); 
   }

   int AudioCtrlLanguagePt::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(PtSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   int AudioCtrlLanguagePt::supportsFeetMiles()
   {
      return 0;
   }

   int AudioCtrlLanguagePt::supportsYardsMiles()
   {
      return 0;
   }

   void AudioCtrlLanguagePt::Distance()
   {

      switch (m_nextXing.spokenDist) {
         /* Metric units */
         case 2000:
            appendClip(PtSound2Quilometros);
            break;
         case 1000:
            appendClip(PtSound1Quilometro);
            break;
         case 500:
            appendClip(PtSound500Metros);
            break;
         case 200:
            appendClip(PtSound200Metros);
            break;
         case 100:
            appendClip(PtSound100Metros);
            break;
         case 50:
            appendClip(PtSound50Metros);
            break;
         case 25:
            appendClip(PtSound25Metros);
            break;

         /* Imperial units - miles */
         case 3219:
            appendClip(PtSound2Miles);
            break;
         case 1609:
            appendClip(PtSound1Mile);
            break;
         case 803:
            appendClip(PtSoundHalfAMile);
            break;
         case 402:
            appendClip(PtSoundAQuarterOfAMile);
            break;

         /* Imperial units - feet */
         case 152:
            appendClip(PtSound500Feet);
            break;
         case 61:
            appendClip(PtSound200Feet);
            break;
         case 31:
            appendClip(PtSound100Feet);
            break;

         /* Imperial units - yards */
         case 182:
            appendClip(PtSound200Yards);
            break;
         case 91:
            appendClip(PtSound100Yards);
            break;
         case 46:
            appendClip(PtSound50Yards);
            break;
         case 23:
            appendClip(PtSound25Yards);
            break;

         /* Here */
         case 0:
            // Should never happen
            appendClip(PtSoundAqui);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguagePt::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(PtSoundAqui);
         } else {
            appendClip(PtSoundImediatamente);
         }
      } else {
         if (m_nextXing.crossingNum == 1) {
            // Add a "DaquiA" for the first crossing
            appendClip(PtSoundDaquiA);
         } else {
            // Add a "A" for the second crossing
            appendClip(PtSoundA);
         }
         Distance();
      }
   }

   void AudioCtrlLanguagePt::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(PtSoundNaPrimeira + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguagePt::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(PtSoundSaia);
      } else {
         appendClip(PtSoundVire);
         NumberedExit();
         appendClip(PtSoundSaida);
         appendClip(PtSoundNaRotunda);
      }
   }

   void AudioCtrlLanguagePt::ActionAndWhen()
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

      if ( (m_nextXing.spokenDist != 0)  || 
           (m_nextXing.crossingNum > 1) ) {
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

   void AudioCtrlLanguagePt::Action() 
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
            appendClip(PtSoundVireAEsquerda);
            break;
         case Right:
            appendClip(PtSoundVireADireita);
            break;
         case Finally:
            // Can not happen, handled before the switch statement.
            appendClip(PtSoundNoDestino);
            break;
         case ExitRdbt:
            RoundaboutExit();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(PtSoundSaia);
            } else {
               appendClip(PtSoundSigaEmFrente);
               appendClip(PtSoundNaRotunda);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(PtSoundSaia);
            } else {
               appendClip(PtSoundVireAEsquerda);
               appendClip(PtSoundNaRotunda);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(PtSoundSaia);
            } else {
               appendClip(PtSoundVireADireita);
               appendClip(PtSoundNaRotunda);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(PtSoundSaia);
            break;
         case KeepLeft:
            appendClip(PtSoundMantenhaSeAEsquerda);
            break;
         case KeepRight:
            appendClip(PtSoundMantenhaSeADireita);
            break;
         case UTurn:
            appendClip(PtSoundFacaInversaoDeMarcha);
            break;
         case StartWithUTurn:
            appendClip(PtSoundQuandoPossivel);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(PtSoundSaia);
            } else {
               appendClip(PtSoundFacaInversaoDeMarcha);
               appendClip(PtSoundNaRotunda);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguagePt::AdditionalCrossing()
   {
      appendClip(PtSoundEASeguir);
      ActionAndWhen();
   }

   void AudioCtrlLanguagePt::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguagePt::genericDeviatedFromRoute()
   {  
      appendClip(PtSoundVoceEstaForaDeRota);
   }

   void AudioCtrlLanguagePt::genericReachedDest()
   {  
      appendClip(PtSoundVoceChegouAoSeuDestino);
   }

} /* namespace isab */


