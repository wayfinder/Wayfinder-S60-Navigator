/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlFi.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageFi:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <JaSitten> ActionAndWhen

SoundListPriority := |
                     <DuKorAtFelHall> |
                     <DuHarAvvikitFranRutten> |
                     <DuArFrammeVidDinDest>

ActionAndWhen := WhenNormalAction Action |
                 Action Now
                 <Määränpääsi on> WhenAtDestination
                 
Now := <Tästä> |
       <Välittömästi>

Action := <KäännyVasemmalle>     |
          <KäännyOikalle>     |
          <PysyVasemmalla>   | 
          <PysyOikealla>    | 
          <Kääny> <Liikenneympyrästä> <Vasemmalle> |
          <Kääny> <Liikenneympyrästä> <Oikalle> |
          <AjaLiikenneympyrästäSuoraanEteenpäin> |
          <Käänny> <Liikenneympyrästä> <Ympäri> |
          <Käänny> <Ympäri>             |
          <TeeUKäännösHeti> |
          <Poistu>             |    FIXME  (Used by exit motorway, exit normal road and exit roundabout)
          RoundaboutExit

RoundaboutExit := <Poistu> |           FIXME
                  <ValitseLiikenneympyrästä> NumberedExit <Uloskäynti>

WhenNormalAction := <[Distance]> <Jälkeen>  |     (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)

WhenAtDestination := <[Distance]> <Päässä> |     (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageFi::AudioCtrlLanguageFi() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageFi::New() { 
      return new AudioCtrlLanguageFi(); 
   }

   int AudioCtrlLanguageFi::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(FiSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   void AudioCtrlLanguageFi::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(FiSound2Kilometrin);
            break;
         case 1000:
            appendClip(FiSound1Kilometrin);
            break;
         case 500:
            appendClip(FiSound500Metrin);
            break;
         case 200:
            appendClip(FiSound200Metrin);
            break;
         case 100:
            appendClip(FiSound100Metrin);
            break;
         case 50:
            appendClip(FiSound50Metrin);
            break;
         case 25:
            appendClip(FiSound25Metrin);
            break;
         case 0:
            // Should never happen
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageFi::Now(bool nowIsTassa)
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            if (nowIsTassa) {
               appendClip(FiSoundTassa);
            } else {
               appendClip(FiSoundTasta);
            }
         } else {
            appendClip(FiSoundValittomasti);
         }
      } else {
         /* Error, should never get here. Say Tästä anyway. */
         appendClip(FiSoundTasta);
      }
   }

   void AudioCtrlLanguageFi::WhenNormalAction()
   {
      Distance();
      appendClip(FiSoundJalkeen);
   }

   void AudioCtrlLanguageFi::WhenAtDestination()
   {
      Distance();
      appendClip(FiSoundPaassa);
   }

   void AudioCtrlLanguageFi::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(FiSoundEnsimmainen + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageFi::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(FiSoundPoistu);
      } else {
         appendClip(FiSoundValitseLiikenneympyrasta);
         NumberedExit();
         appendClip(FiSoundUloskaynti);
      }
   }

   void AudioCtrlLanguageFi::ActionAndWhen()
   {
      /* Special case for Finally */
      if (RouteAction(m_nextXing.xing->action) == Finally) {
         appendClip(FiSoundMaaranpaasiOn);
         WhenAtDestination();
      } else if (m_nextXing.spokenDist == 0) {
         Action();
         switch (RouteAction(m_nextXing.xing->action)) {
            case ExitAt:
            case OffRampLeft:
            case OffRampRight:
            case UTurnRdbt:
               Now(true);
               break;
            default:
               Now(false);
               break;
         }
      } else {
         WhenNormalAction();
         Action();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageFi::Action() 
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
            appendClip(FiSoundKaannyVasemmalle);
            break;
         case Right:
            appendClip(FiSoundKaannyOikealle);
            break;
         case Finally:
            /* We say the "You have reached your destination" here sincewe
             * have nothing better to say. */
            genericReachedDest();
            break;
         case ExitRdbt:
            RoundaboutExit();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(FiSoundPoistu);
            } else {
               appendClip(FiSoundAjaLiikenneymparastaSouraanEteenpain);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(FiSoundPoistu);
            } else {
               appendClip(FiSoundKaanny);
               appendClip(FiSoundLiikenneympyrasta);
               appendClip(FiSoundVasemmalle);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(FiSoundPoistu);
            } else {
               appendClip(FiSoundKaanny);
               appendClip(FiSoundLiikenneympyrasta);
               appendClip(FiSoundOikealle);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(FiSoundPoistu);
            break;
         case KeepLeft:
            appendClip(FiSoundPysyVasemmalla);
            break;
         case KeepRight:
            appendClip(FiSoundPysyOikealla);
            break;
         case UTurn:
            appendClip(FiSoundKaanny);
            appendClip(FiSoundYmpari);
            break;
         case StartWithUTurn:
            appendClip(FiSoundTeeUKaannosHeti);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(FiSoundPoistu);
            } else {
               appendClip(FiSoundKaanny);
               appendClip(FiSoundLiikenneympyrasta);
               appendClip(FiSoundYmpari);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguageFi::AdditionalCrossing()
   {
      appendClip(FiSoundJaSitten);
      ActionAndWhen();
   }

   void AudioCtrlLanguageFi::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageFi::genericDeviatedFromRoute()
   {  
      appendClip(FiSoundEtSeuraaAnnettuaReittia);
   }

   void AudioCtrlLanguageFi::genericReachedDest()
   {  
      appendClip(FiSoundOletTullutPerille);
   }

} /* namespace isab */


