/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AUDIO_CONTROL_STD_H
#define AUDIO_CONTROL_STD_H

#include "AudioCtrlLanguage.h"


namespace isab{

   class AudioCtrlLanguageStd : public AudioCtrlLanguage
   {  
   public:
      virtual int syntheziseSoundList(const RouteInfo &r,
            bool onTrackStatusChanged,
            DistanceInfo &deadlines,
            SoundClipsList &soundClips);
      virtual int syntheziseCrossingSoundList(const RouteInfoParts::Crossing &c,
            DistanceInfo &deadlines,
            SoundClipsList &soundList);
      virtual int newCrossingSoundList(
            DistanceInfo &deadlines,
            SoundClipsList &soundClips);
      virtual void resetState();
      virtual void selectUnits(enum SpokenUnits units);
      virtual int selectSyntaxVersion(const char * which, int & numSounds, char ** & clipNames);
      virtual int setVerbosity(AudioVerbosity verbosity);

      // Override if the language supports the feet-miles system
      virtual int supportsFeetMiles();

      // Override if the language supports the yards-miles system
      virtual int supportsYardsMiles();

      AudioCtrlLanguageStd();
      ~AudioCtrlLanguageStd();
   protected:
      // New member variables. Replaces the ugly mess that was 
      // XingDesc / SndInfo 
      struct RouteState {
         const RouteInfo *routeInfo;
         bool onTrackStatusChanged;
      } m_rstate;

      // Filled in by syntheziseSoundList
      struct SoundState {
         SoundClipsList *soundClips;
         int truncPoint;
         bool noMoreSounds;
      } m_soundState;

      // Filled in by SoundList
      struct NextCrossingInfo {
         int crossingNum;
         const RouteInfoParts::Crossing * xing;
         int32 spokenDist;
         bool setTimingMarker;
      } m_nextXing;

      
      struct DistanceTableEntry {
         int32 spokenDistance;
         DistanceInfo info;
         int32 ignoreIfFaster;
         DistanceTableEntry(int32 spk, const DistanceInfo &di, int32 ignspd) : 
            spokenDistance(spk), info(di), ignoreIfFaster(ignspd) { };
      };

      std::vector<DistanceTableEntry> distanceTable;

      // Global state. Given what we spoke the last time we know what is resonable
      // to speak the next time.
      int m_lastDistanceIndex;

      // The currently loaded distance table
      enum SpokenUnits m_currentUnits;

      // Override if another set of distances is desired.
      virtual void InitDistanceTableMetric();
      virtual void InitDistanceTableFeetMiles();
      virtual void InitDistanceTableYardsMiles();

      // Default SoundList, usable for langauges on the form i
      //   <Crossing> <AdditionalCrossing>.
      virtual void SoundList(DistanceInfo &deadlines);
      //
      // Default SoundListCrossingOnly, usable for langauges on the form i
      //   <Action> 
      virtual void SoundListCrossingOnly(DistanceInfo &deadlines);

      // Used by the default SoundList method
      virtual void FirstCrossing() = 0;
      // Used by the default SoundList method
      virtual void AdditionalCrossing() = 0;
      // Used by the default SoundListCrossingOnly method
      virtual void Action() = 0;

      virtual void genericDeviatedFromRoute() = 0;
//      virtual void genericWrongWay() = 0;
      virtual void genericReachedDest() = 0;


      virtual int32 policyNextDistance(DistanceInfo &deadlines);
      virtual int32 policyAdditionalDistance(int32 distance);

      void appendClip(int16 clip);
      void immediateDeadline(DistanceInfo &deadlines);

      void truncateThisCrossing();

   }; /* AudioCtrlLanguageStd */

} /* namespace isab */

#endif /* AUDIO_CONTROL_STD_H */

