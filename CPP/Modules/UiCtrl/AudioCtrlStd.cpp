/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Quality.h"
#include "AudioCtrlStd.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageStd::AudioCtrlLanguageStd() : m_lastDistanceIndex(-1)
   {
      // Metric is the safe bet - all languages support it
      InitDistanceTableMetric();
   }

   AudioCtrlLanguageStd::~AudioCtrlLanguageStd()
   {
      distanceTable.clear();
   }

#define DPB distanceTable.push_back
   void AudioCtrlLanguageStd::InitDistanceTableMetric()
   {
      distanceTable.clear();
      /*           spokenDistance,              sayAtDistance, abortTooShortDistance, abortTooFarDistance, ignoreIfFaster       */
      DPB(DistanceTableEntry(  -1,  DistanceInfo(  NoDistance,                   -1,                   -1 ),  9999     ));  /* priosound */
      DPB(DistanceTableEntry(2000,  DistanceInfo(        2000,                 1700,            MAX_INT32 ),  9999     ));
      DPB(DistanceTableEntry(1000,  DistanceInfo(        1000,                  800,                 2050 ),  9999     ));
      DPB(DistanceTableEntry( 500,  DistanceInfo(         500,                  450,                 1050 ),  9999     ));
      DPB(DistanceTableEntry( 200,  DistanceInfo(         200,                  170,                  550 ),  9999     ));
      DPB(DistanceTableEntry( 100,  DistanceInfo(         100,                   60,                  210 ),  9999     ));
      DPB(DistanceTableEntry(  50,  DistanceInfo(          55,                   35,                  110 ),     5     ));
      DPB(DistanceTableEntry(  25,  DistanceInfo(          30,                   20,                   60 ),     2     ));
      DPB(DistanceTableEntry(   0,  DistanceInfo(          18,                   -1,                   30 ),  9999     ));
      DPB(DistanceTableEntry(  -1,  DistanceInfo(    NeverSay,                   -1,                   50 ),  9999     )); /* Never spoken! */
      m_currentUnits = MetricUnits;
   }

   void AudioCtrlLanguageStd::InitDistanceTableFeetMiles()
   {
      distanceTable.clear();
      /*           spokenDistance,              sayAtDistance, abortTooShortDistance, abortTooFarDistance, ignoreIfFaster       */
      DPB(DistanceTableEntry(  -1,  DistanceInfo(  NoDistance,                   -1,                   -1 ),  9999     ));  /* priosound */
      DPB(DistanceTableEntry(3219,  DistanceInfo(        3219,                 2919,            MAX_INT32 ),  9999     ));
      DPB(DistanceTableEntry(1609,  DistanceInfo(        1609,                 1409,                 3269 ),  9999     ));
      DPB(DistanceTableEntry( 803,  DistanceInfo(         803,                  753,                 1659 ),  9999     ));
      DPB(DistanceTableEntry( 402,  DistanceInfo(         402,                  352,                  853 ),  9999     ));
      DPB(DistanceTableEntry( 152,  DistanceInfo(         152,                  122,                  452 ),  9999     ));
      DPB(DistanceTableEntry(  61,  DistanceInfo(          66,                   46,                  162 ),     5     ));
      DPB(DistanceTableEntry(  31,  DistanceInfo(          36,                   26,                   71 ),     2     ));
      DPB(DistanceTableEntry(   0,  DistanceInfo(          18,                   -1,                   36 ),  9999     ));
      DPB(DistanceTableEntry(  -1,  DistanceInfo(    NeverSay,                   -1,                   61 ),  9999     )); /* Never spoken! */
      m_currentUnits = ImperialFeetMilesUnits;
   }

   void AudioCtrlLanguageStd::InitDistanceTableYardsMiles()
   {
      distanceTable.clear();
      /*           spokenDistance,              sayAtDistance, abortTooShortDistance, abortTooFarDistance, ignoreIfFaster       */
      DPB(DistanceTableEntry(  -1,  DistanceInfo(  NoDistance,                   -1,                   -1 ),  9999     ));  /* priosound */
      DPB(DistanceTableEntry(3219,  DistanceInfo(        3219,                 2919,            MAX_INT32 ),  9999     ));
      DPB(DistanceTableEntry(1609,  DistanceInfo(        1609,                 1409,                 3269 ),  9999     ));
      DPB(DistanceTableEntry( 803,  DistanceInfo(         803,                  753,                 1659 ),  9999     ));
      DPB(DistanceTableEntry( 402,  DistanceInfo(         402,                  352,                  853 ),  9999     ));
      DPB(DistanceTableEntry( 182,  DistanceInfo(         182,                  152,                  452 ),  9999     ));
      DPB(DistanceTableEntry(  91,  DistanceInfo(          91,                   51,                  192 ),  9999     ));
      DPB(DistanceTableEntry(  46,  DistanceInfo(          51,                   31,                  101 ),     5     ));
      DPB(DistanceTableEntry(  23,  DistanceInfo(          28,                   18,                   56 ),     2     ));
      DPB(DistanceTableEntry(   0,  DistanceInfo(          18,                   -1,                   34 ),  9999     ));
      DPB(DistanceTableEntry(  -1,  DistanceInfo(    NeverSay,                   -1,                   56 ),  9999     )); /* Never spoken! */
      m_currentUnits = ImperialYardsMilesUnits;
   }
#undef DPB

   int AudioCtrlLanguageStd::setVerbosity(AudioVerbosity /*verbosity*/)
   {
      return 0;
   }

   int AudioCtrlLanguageStd::supportsFeetMiles()
   {
      return 0;
   }

   int AudioCtrlLanguageStd::supportsYardsMiles()
   {
      return 0;
   }

   int AudioCtrlLanguageStd::selectSyntaxVersion(const char * which, int & numSounds, char ** & clipNames)
   {
      // Normally there are no selectable versions of a syntax.
      numSounds = 0;
      clipNames = NULL;
      return 0;
   }

   void AudioCtrlLanguageStd::selectUnits(enum SpokenUnits units)
   {
      if (units == m_currentUnits) {
         return;
      }
      switch (units) {
         case MetricUnits:
            InitDistanceTableMetric();
            resetState();
            break;
         case ImperialFeetMilesUnits:
            if (supportsFeetMiles()) {
               InitDistanceTableFeetMiles();
               resetState();
            } else {
               // This call must not fail!!!
               selectUnits(MetricUnits);
            }
            break;
         case ImperialYardsMilesUnits:
            if (supportsYardsMiles()) {
               InitDistanceTableYardsMiles();
               resetState();
            } else {
               // This call must not fail!!!
               selectUnits(ImperialFeetMilesUnits);
            }
            break;
         default:
            // FIXME - warn here?
            break;
      }
   }


   /* FIXME - r.distToWpt är inte bra, använd extrapolerad position! */
   int32 AudioCtrlLanguageStd::policyNextDistance(DistanceInfo &deadlines)
   {
      const RouteInfo &r = *m_rstate.routeInfo;
      int32 spokenDistance;

      if (r.onTrackStatus != OnTrack) {
         // WARN("policyNextDistance called with onTrackStatus != OnTrack");
         deadlines = distanceTable[0].info;
         m_lastDistanceIndex = 0;
         spokenDistance = 0;  // Placeholder, not used
         return spokenDistance;
      }
      int lastSound;
      if (m_lastDistanceIndex < 0) {
         lastSound = 0;
      } else {
         lastSound = m_lastDistanceIndex;
      }

      int nextSound = lastSound + 1;

      /* Accept any sound within the interval sayAtDistance..abortTooShortDistance
         if this is the first sound after a priority sound. We want to say something
         as soon as possible.
         This is done by switching from sayAtDistance to abortTooShortDistance
         as a criteria.
      */
      if (lastSound == 0) {
         /* Find the first possible distance. This is the next sound to play,
          * were it not for the user override settings. Those settings are 
          * taken into account in the next while loop. */
         while (distanceTable[nextSound].info.abortTooShortDistance > r.distToWpt ) {
            ++nextSound;
         }
      } else {
         /* Find the first possible distance. This is the next sound to play,
          * were it not for the user override settings. Those settings are 
          * taken into account in the next while loop. */
         while (distanceTable[nextSound].info.sayAtDistance > r.distToWpt ) {
            ++nextSound;
         }
      }
#if 1
      int tooFarDeadline = distanceTable[nextSound].info.abortTooFarDistance;
      /* Find the first allowed sound, and carry the abortTooFarDistance from the
       * first match above. */
      // This loop terminates since the last elements in the distance tables 
      // cannot be prohibited by the user.
      while (1) {
         bool prohibited = false;
         // Supress some distances when driving fast.
         if (r.speed > distanceTable[nextSound].ignoreIfFaster)
            prohibited = true;
         // Supress "Finally" when closer than 100 meters to the goal.
         if ( (RouteAction(r.currCrossing.action) == Finally) &&
              (distanceTable[nextSound].info.sayAtDistance <= 50) )
            prohibited = true;
         // Always use the "no next sound" sound.
         if (distanceTable[nextSound].info.sayAtDistance == NeverSay)
            prohibited = false;
         if (!prohibited) {
            deadlines = distanceTable[nextSound].info;
            deadlines.abortTooFarDistance = tooFarDeadline;
            spokenDistance = distanceTable[nextSound].spokenDistance;
            if (deadlines.sayAtDistance >= 0) {
               m_lastDistanceIndex = nextSound;
            }
            return spokenDistance;
         }
         ++nextSound;
      }
#else
      deadlines = distanceTable[nextSound].info;
      spokenDistance = distanceTable[nextSound].spokenDistance;
      // If this is the special last marker sound, reuse the last m_lastDistanceIndex
      if (deadlines.sayAtDistance >= 0) {
         m_lastDistanceIndex = nextSound;
      }
#endif
      return spokenDistance;

   }

   int32 AudioCtrlLanguageStd::policyAdditionalDistance(int32 distance)
   {
      int nextSound = 1;
      /* Find the first possible distance. This is the next sound to play,
       * were it not for the user override settings. Those settings are 
       * taken into account in the next while loop. */
      if (distance < 0) {
         distance = 0;
      }
      while (distanceTable[nextSound].info.sayAtDistance > distance ) {
         ++nextSound;
      }
      return (distanceTable[nextSound].spokenDistance);
   }

   int AudioCtrlLanguageStd::newCrossingSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(SoundNewCrossing);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }


   int AudioCtrlLanguageStd::syntheziseSoundList(const RouteInfo &r,
         bool onTrackStatusChanged, 
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      m_rstate.routeInfo = &r;
      m_rstate.onTrackStatusChanged = onTrackStatusChanged;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      SoundList(deadlines);

      appendClip(SoundEnd);
      m_rstate.routeInfo = NULL;
      m_soundState.soundClips = NULL;

      return 0;
   }

   int AudioCtrlLanguageStd::syntheziseCrossingSoundList(const RouteInfoParts::Crossing &c,
         DistanceInfo &deadlines,
         SoundClipsList &soundList)
   {
      RouteInfo r;
      r.currCrossing=c;

      m_rstate.routeInfo = &r;
      m_rstate.onTrackStatusChanged = true;

      m_soundState.soundClips = &soundList;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      SoundListCrossingOnly(deadlines);

      appendClip(SoundEnd);
      m_rstate.routeInfo = NULL;
      m_soundState.soundClips = NULL;

      return 0;
   }


   void AudioCtrlLanguageStd::immediateDeadline(DistanceInfo &deadlines)
   {
      deadlines = distanceTable[0].info;
      m_lastDistanceIndex = 0;
   }

   void AudioCtrlLanguageStd::SoundList(DistanceInfo &deadlines)
   {  
      const RouteInfo &ri = *(m_rstate.routeInfo);
      switch (ri.onTrackStatus) {
         case OnTrack:
            m_nextXing.crossingNum = 1;
            m_nextXing.xing = &ri.currCrossing;
            m_nextXing.spokenDist = policyNextDistance(deadlines);
            m_nextXing.setTimingMarker = true;
            if (m_nextXing.spokenDist < 0) {
               return;
            }
            FirstCrossing();

            // The next crossing is only spoken when at 100 and 200m, and only if the
            // first crossing was spoken correctly.
            if (m_soundState.noMoreSounds     || 
                (m_nextXing.spokenDist > 200) ||
                (m_nextXing.spokenDist < 100)    ) {
               return;
            }

            m_nextXing.crossingNum = 2;
            m_nextXing.xing = &ri.nextCrossing1;
            m_nextXing.spokenDist = policyAdditionalDistance(ri.currCrossing.distToNextCrossing);
            m_nextXing.setTimingMarker = false;
            if (m_nextXing.spokenDist < 0) {
               return;
            }
            m_soundState.truncPoint=m_soundState.soundClips->size();
            if (m_nextXing.spokenDist < 200) {
               AdditionalCrossing();
            }
            break;
         case OffTrack:
            if (m_rstate.onTrackStatusChanged) {
               genericDeviatedFromRoute();
            } else {
               appendClip(SoundNoKnownSound);
            }
            immediateDeadline(deadlines);
            break;
         case WrongWay:
            if (m_rstate.onTrackStatusChanged) {
//               genericWrongWay();
               appendClip(SoundNoKnownSound);
            } else {
               appendClip(SoundNoKnownSound);
            }
            immediateDeadline(deadlines);
            break;
         case Goal:
            if (m_rstate.onTrackStatusChanged) {
               genericReachedDest();
            } else {
               appendClip(SoundNoKnownSound);
            }
            immediateDeadline(deadlines);
            break;
         default:
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageStd::SoundListCrossingOnly(DistanceInfo &deadlines)
   {  
      const RouteInfo &ri = *(m_rstate.routeInfo);
      m_nextXing.crossingNum = 1;
      m_nextXing.xing = &ri.currCrossing;
      m_nextXing.setTimingMarker = true;
      m_nextXing.spokenDist = 100;  // Hack to get the roundabout sounds right.
      if (m_nextXing.spokenDist < 0) {
         return;
      }
      Action();
      immediateDeadline(deadlines);
   }

   void AudioCtrlLanguageStd::resetState()
   {
      m_lastDistanceIndex = -1;
   }

   void AudioCtrlLanguageStd::appendClip(int16 clip)
   {
#if !defined(NAV2_CLIENT_SERIES60_V1) && !defined(NAV2_CLIENT_SERIES60_V2)
      m_soundState.soundClips->push_back(clip);
#else
      if (m_soundState.soundClips->size() < 9) {
         m_soundState.soundClips->push_back(clip);
      } else {
         truncateThisCrossing();
      }
#endif
   }

   void AudioCtrlLanguageStd::truncateThisCrossing()
   {
      m_soundState.soundClips->resize(m_soundState.truncPoint + 1);
      (*(m_soundState.soundClips))[m_soundState.truncPoint] = SoundNoKnownSound;
      m_soundState.noMoreSounds = true;
   }




} /* namespace isab */


