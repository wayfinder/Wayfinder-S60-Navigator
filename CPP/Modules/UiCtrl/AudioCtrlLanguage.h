/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AUDIO_CONTROL_LANGUAGE_H
#define AUDIO_CONTROL_LANGUAGE_H

#include "arch.h"
#include "RouteInfo.h"
#include "Quality.h"
#include <vector>

namespace isab{
   class RouteInfo;
   /**
    * An abstract class for converting route directions to 
    * speach. This class is derived from and never instantiated
    * itself.
    *
    */
   class AudioCtrlLanguage
   {
      public:
		 typedef std::vector<int16> SoundClipsList;

         enum {
            NoDistance = -1,
            NeverSay = -2,
         };

         class DistanceInfo {
            public:
               int32 sayAtDistance;
               int32 abortTooShortDistance;
               int32 abortTooFarDistance;
               DistanceInfo() : sayAtDistance(-1), abortTooShortDistance(-1), abortTooFarDistance(-1)
                                  { }
               DistanceInfo(int32 a, int32 b, int32 c) : 
                  sayAtDistance(a), abortTooShortDistance(b), abortTooFarDistance(c)
                                  { }
         };

         enum SpokenUnits {
            MetricUnits,
            ImperialFeetMilesUnits,
            ImperialYardsMilesUnits,
         };

         enum AudioVerbosity {
            VerbosityMuted = 0,
            VerbosityReduced,
            VerbosityNormal
         };

         virtual ~AudioCtrlLanguage();
         virtual int syntheziseSoundList(const RouteInfo &r,
                                         bool onTrackStatusChanged,
                                         DistanceInfo &deadlines,
                                         SoundClipsList &soundList) = 0;
         virtual int syntheziseCrossingSoundList(const RouteInfoParts::Crossing &c,
                                         DistanceInfo &deadlines,
                                         SoundClipsList &soundList) = 0;
         virtual int newCrossingSoundList(
               DistanceInfo &deadlines,
               SoundClipsList &soundList) = 0;
         virtual int newCameraSoundList(
               DistanceInfo &deadlines,
               SoundClipsList &soundList) = 0;
         virtual int badSoundList(
            DistanceInfo &deadlines,
            SoundClipsList &soundList ) = 0;
         virtual int goodSoundList(
            DistanceInfo &deadlines,
            SoundClipsList &soundList ) = 0;
         virtual int statusOfGPSChangedSoundList( 
            const Quality& q, DistanceInfo &deadlines,
            SoundClipsList &soundList) = 0;
         virtual void resetState() = 0;
         virtual void selectUnits(enum SpokenUnits units) = 0;
         virtual int selectSyntaxVersion(const char * which, int & numClips, char ** & clipNames) = 0;
         virtual int setVerbosity(AudioVerbosity verbosity) = 0;

         virtual int supportsFeetMiles() = 0;
         virtual int supportsYardsMiles() = 0;

   }; /* AudioCtrlLanguage */

   typedef class AudioCtrlLanguage *(*AudioCtrlLanguageFactoryFunc)();


} /* namespace isab */

#endif /* AUDIO_CONTROL_LANGUAGE_H */

