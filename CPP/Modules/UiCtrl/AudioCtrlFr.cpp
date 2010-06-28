/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlFr.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageFr:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <Puis> ActionAndWhen

SoundListPriority := |
                     <DuKorAtFelHall> |
                     <DuHarAvvikitFranRutten> |
                     <DuArFrammeVidDinDest>

ActionAndWhen := When Action1 Action2 |
                 Action1 Action2 <Ici>
                 Action1 <Immediament> Action2
                 <ADestination> When

Action  := Action1 Action2   // Only for SoundListCrossingOnly()

Action1 := <Tournez>     |
           <Continuez>   |
           <ConduisezToutDroit>           |
           <QuittezLautoroute>             |
           <FaitesDemitour>           |
           <DesQuePossible>                |
           RoundaboutExit1

Action2 := <AGauche>     |
           <ADroite>     |
           <AGauche> <AuRondpoint> |
           <ADroite> <AuRondpoint> |
           <AuRondpoint> |              (after ConduisezToutDroit and FaitesDemitour)
           RoundaboutExit2

RoundaboutExit1 := <Sortez> |
                   <Prenez> NumberedExit <Sortie>

RoundaboutExit2 := |                    (after Sortez when immediatly at the exit)
                   <AllaRotunda>        (after Prenez in RoundaboutExit1 )

When := <A[Distance]>  (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageFr::AudioCtrlLanguageFr() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageFr::New() { 
      return new AudioCtrlLanguageFr(); 
   }

   int AudioCtrlLanguageFr::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(FrSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   void AudioCtrlLanguageFr::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(FrSoundA2Kilometres);
            break;
         case 1000:
            appendClip(FrSoundA1Kilometre);
            break;
         case 500:
            appendClip(FrSoundA500Metres);
            break;
         case 200:
            appendClip(FrSoundA200Metres);
            break;
         case 100:
            appendClip(FrSoundA100Metres);
            break;
         case 50:
            appendClip(FrSoundA50Metres);
            break;
         case 25:
            appendClip(FrSoundA25Metres);
            break;
         case 0:
            // Should never happen
            appendClip(FrSoundIci);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageFr::When()
   {
      if (m_nextXing.spokenDist == 0) {
         // Should never happen
         appendClip(FrSoundIci);
      } else {
         Distance();
      }
   }

   void AudioCtrlLanguageFr::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(FrSoundLaPremiere + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageFr::RoundaboutExit1()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(FrSoundSortez);
      } else {
         appendClip(FrSoundPrenez);
         NumberedExit();
         appendClip(FrSoundSortie);
      }
   }

   void AudioCtrlLanguageFr::RoundaboutExit2()
   {
      if (m_nextXing.spokenDist == 0) {
      } else {
         appendClip(FrSoundAuRondpoint);
      }
   }

   void AudioCtrlLanguageFr::ActionAndWhen()
   {
      if (RouteAction(m_nextXing.xing->action) == Finally) {
         appendClip(FrSoundADestination);
         if (m_nextXing.spokenDist == 0)
            return;    // NB! No distance info added to this sound if spokenDist==0
         When();
         if (m_nextXing.setTimingMarker) {
            appendClip(SoundTimingMarker);
         }
         return;
      }


      if (m_nextXing.spokenDist != 0) {
         // A bit left until the action
         When();
         Action1();
         Action2();
      } else {
         // spokenDist == 0
         if (m_nextXing.crossingNum == 1) {
            Action1();
            Action2();
            appendClip(FrSoundIci);
         } else {
            Action1();
            appendClip(FrSoundImmediatement);
            Action2();
         }
      }

      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageFr::Action()
   { 
      Action1();
      Action2();
   }

   void AudioCtrlLanguageFr::Action1() 
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
         case Right:
            appendClip(FrSoundTournez);
            break;
         case Finally:
            // Can not happen, handled before the switch statement.
            appendClip(FrSoundADestination);
            break;
         case ExitRdbt:
            RoundaboutExit1();
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(FrSoundSortez);
            } else {
               appendClip(FrSoundConduisezToutDroit);
            }
            break;
         case LeftRdbt:
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(FrSoundSortez);
            } else {
               appendClip(FrSoundTournez);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(FrSoundQuittezLautoroute);
            break;
         case KeepLeft:
         case KeepRight:
            appendClip(FrSoundContinuez);
            break;
         case UTurn:
            appendClip(FrSoundFaitesDemitour);
            break;
         case StartWithUTurn:
            appendClip(FrSoundDesQuePossible);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(FrSoundSortez);
            } else {
               appendClip(FrSoundFaitesDemitour);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguageFr::Action2() 
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
            appendClip(FrSoundAGauche);
            break;
         case Right:
            appendClip(FrSoundADroite);
            break;
         case Finally:
            // Can not happen, handled before the switch statement.
            break;
         case ExitRdbt:
            RoundaboutExit2();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               // All was said in Action1()
            } else {
               appendClip(FrSoundAuRondpoint);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               // All was said in Action1()
            } else {
               appendClip(FrSoundAGauche);
               appendClip(FrSoundAuRondpoint);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               // All was said in Action1()
            } else {
               appendClip(FrSoundADroite);
               appendClip(FrSoundAuRondpoint);
            }
            break;
         case ExitAt:
         case UTurn:
         case StartWithUTurn:
            // All was said in Action1()
            break;
         case KeepLeft:
            appendClip(FrSoundAGauche);
            break;
         case KeepRight:
            appendClip(FrSoundADroite);
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguageFr::AdditionalCrossing()
   {
      appendClip(FrSoundPuis);
      ActionAndWhen();
   }

   void AudioCtrlLanguageFr::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageFr::genericDeviatedFromRoute()
   {  
      appendClip(FrSoundAPresentVousEtes);
   }

   void AudioCtrlLanguageFr::genericReachedDest()
   {  
      appendClip(FrSoundVousEtesArrive);
   }

} /* namespace isab */


