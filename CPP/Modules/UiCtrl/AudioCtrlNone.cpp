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
#include "AudioCtrlNone.h"
#include "AudioClipsEnum.h"

namespace isab{
   using namespace AudioClipsEnum;


   AudioCtrlLanguageNone::AudioCtrlLanguageNone()
   {
   }

   AudioCtrlLanguageNone::~AudioCtrlLanguageNone()
   {
   }

   AudioCtrlLanguage* AudioCtrlLanguageNone::New()
   { 
      return new AudioCtrlLanguageNone(); 
   }


   int AudioCtrlLanguageNone::newCrossingSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      soundClips.push_back(SoundNoKnownSound);
      soundClips.push_back(SoundEnd);

      return 0;
   }

    int AudioCtrlLanguageNone::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      soundClips.push_back(SoundNoKnownSound);
      soundClips.push_back(SoundEnd);

      return 0;
   }

   int AudioCtrlLanguageNone::badSoundList(
      DistanceInfo &deadlines,
      SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      soundClips.push_back(SoundNoKnownSound);
      soundClips.push_back(SoundEnd);

      return 0;
   }

   int AudioCtrlLanguageNone::goodSoundList(
      DistanceInfo &deadlines,
      SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      soundClips.push_back(SoundNoKnownSound);
      soundClips.push_back(SoundEnd);

      return 0;
   }

   int AudioCtrlLanguageNone::statusOfGPSChangedSoundList( 
      const Quality& q, DistanceInfo &deadlines,
      SoundClipsList &soundList)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      soundList.push_back( SoundNoKnownSound );
      soundList.push_back( SoundEnd );

      return 0;
   }

   int AudioCtrlLanguageNone::syntheziseSoundList(const RouteInfo &/*r*/,
         bool /*onTrackStatusChanged*/, 
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      soundClips.push_back(SoundNoKnownSound);
      soundClips.push_back(SoundEnd);

      return 0;
   }

   int AudioCtrlLanguageNone::syntheziseCrossingSoundList(const RouteInfoParts::Crossing &/*c*/,
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      soundClips.push_back(SoundNoKnownSound);
      soundClips.push_back(SoundEnd);

      return 0;
   }


   void AudioCtrlLanguageNone::resetState()
   {
   }

   void AudioCtrlLanguageNone::selectUnits(enum SpokenUnits /*units*/)
   {
   }

   int AudioCtrlLanguageNone::selectSyntaxVersion(const char * which, int & numSounds, char ** & clipNames)
   {
      // Normally there are no selectable versions of a syntax.
      numSounds = 0;
      return 0;
   }

   int AudioCtrlLanguageNone::setVerbosity(AudioVerbosity verbosity)
   {
      return 0;
   }

   int AudioCtrlLanguageNone::supportsFeetMiles()
   {
      return 0;
   }

   int AudioCtrlLanguageNone::supportsYardsMiles()
   {
      return 0;
   }


} /* namespace isab */


