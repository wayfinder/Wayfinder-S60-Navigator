/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlNl.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageDe:

SoundListNormal := |
                   FirstCrossing |
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <EnDan> ActionAndWhen

SoundListPriority := |
                     <URidjtInDeVerkeerdeRichting> |
                     <UBevindtZichNuBuitenDeRoute> |
                     <UHebtUwBestemminBereikt>

ActionAndWhen := When Action |
                 <BijDeBestemming> When

Action := <LinksAfslaan>     |
          <RechtsAfslaan>     |
          <LinksHouden>    |   FIXME - XXX
          <RechtsHouden>   |   FIXME - XXX
          <RechtdoorRijden>           |
          <OpDeRotonde> <LinksAfslaan>   |
          <OpDeRotonde> <RechstsAfslaan> |
          <OpDeRotonde> <RechtdoorRijden> |
          <OpDeRotonde> <Omkeren> |
          <Omkeren>             |
          <KeerOmIndienMogelijk> |
          <SnelwegVerlaten>             |
          RoundaboutExit

RoundaboutExit := <Hier> <Aftrit> |
                  <OpDeRotonde> NumberedExit <Afrit>

When := <Hier> |
        <Na[Distance]>  (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageNl::AudioCtrlLanguageNl() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageNl::New() { 
      return new AudioCtrlLanguageNl(); 
   }

   int AudioCtrlLanguageNl::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(NlSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   void AudioCtrlLanguageNl::Distance()
   {

      switch (m_nextXing.spokenDist) {
         case 2000:
            appendClip(NlSoundNa2Kilometer);
            break;
         case 1000:
            appendClip(NlSoundNa1Kilometer);
            break;
         case 500:
            appendClip(NlSoundNa500Meter);
            break;
         case 200:
            appendClip(NlSoundNa200Meter);
            break;
         case 100:
            appendClip(NlSoundNa100Meter);
            break;
         case 50:
            appendClip(NlSoundNa50Meter);
            break;
         case 25:
            appendClip(NlSoundNa25Meter);
            break;
         case 0:
            // Should never happen
            appendClip(NlSoundHier);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageNl::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(NlSoundHier);
         } else {
            appendClip(NlSoundEnDan);
         }
      } else {
         Distance();
      }
   }

   void AudioCtrlLanguageNl::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(NlSoundDeEerste + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageNl::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(NlSoundAfrit);
      } else {
         appendClip(NlSoundOpDeRotonde);
         NumberedExit();
         appendClip(NlSoundAfrit);
      }
   }

   void AudioCtrlLanguageNl::ActionAndWhen()
   {
      if (RouteAction(m_nextXing.xing->action) == Finally) {
         Action();
         if (m_nextXing.spokenDist == 0)
            return;// NB! No distance info added to this sound if spokenDist==0
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

   void AudioCtrlLanguageNl::Action() 
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
            appendClip(NlSoundLinksAfslaan);
            break;
         case Right:
            appendClip(NlSoundRechtsAfslaan);
            break;
         case Finally:
            appendClip(NlSoundBijDeBestemming);
            break;
         case ExitRdbt:
            RoundaboutExit();
            //return;    // NB! No distance info added to this sound.
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(NlSoundAfrit);
            } else {
               appendClip(NlSoundOpDeRotonde);
               appendClip(NlSoundRechtdoorRijden);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(NlSoundAfrit);
            } else {
               appendClip(NlSoundOpDeRotonde);
               appendClip(NlSoundNaarLinks);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(NlSoundAfrit);
            } else {
               appendClip(NlSoundOpDeRotonde);
               appendClip(NlSoundNaarRechts);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(NlSoundAfrit);
            break;
         case KeepLeft:
            appendClip(NlSoundLinksHouden);
            break;
         case KeepRight:
            appendClip(NlSoundRechtsHouden);
            break;
         case UTurn:
            appendClip(NlSoundOmkeren);
            break;
         case StartWithUTurn:
            appendClip(NlSoundKeerOmIndienMogelijk);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(NlSoundAfrit);
            } else {
               appendClip(NlSoundOpDeRotonde);
               appendClip(NlSoundOmkeren);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            break;
      }
   }

   void AudioCtrlLanguageNl::AdditionalCrossing()
   {
      appendClip(NlSoundEnDan);
      ActionAndWhen();
   }

   void AudioCtrlLanguageNl::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageNl::genericDeviatedFromRoute()
   {  
      appendClip(NlSoundUBevindtZichNuBuitenDeRoute);
   }

   void AudioCtrlLanguageNl::genericReachedDest()
   {  
      appendClip(NlSoundUHebtUwBestemmingBereikt);
   }

} /* namespace isab */


