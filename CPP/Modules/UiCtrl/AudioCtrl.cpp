/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "arch.h"
#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"

#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "RouteEnums.h"
#include "NavTask.h"

#include "ParameterEnums.h"
#include "Parameter.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "AudioCtrlLanguage.h"
#include "DistancePrintingPolicy.h"

#include "AudioCtrl.h"

#include "Nav2Error.h"
#include "ErrorModule.h"
#include "Destinations.h"
#include "NavPacket.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "NavServerCom.h"
#include "CtrlHub.h"
#include "UiCtrl.h"

#include "RouteEnums.h"
#include "AudioClipsEnum.h"

#include "LogMacros.h"

namespace isab {
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguage::~AudioCtrlLanguage()
   {
   }


   AudioCtrl::AudioCtrl(UiCtrl* uiCtrl,
         SerialProviderPublic* serialProvPubl,
         ParameterProviderPublic* paramProvPubl,
         AudioCtrlLanguage *audioSyntax) :
      m_uiCtrl(uiCtrl), 
      m_log(NULL), 
      m_serialProvPubl(serialProvPubl),
      m_paramProvPubl(paramProvPubl),
      m_lastRouteInfoValid(false),
      m_lang(audioSyntax),
      m_state(Empty),
      m_currentSoundLatency(0),
      m_waitingPrioritySound(NULL),
      m_verbosity(AudioCtrlLanguage::VerbosityNormal),
      m_gpsQuality( QualityMissing ),
      m_badGPStimerID( 0 ),
      m_badGpsQuality( QualityMissing ),
      m_sentBadGPSLevel( 0 )
   {
      m_log = new Log("AudioCtrl");
   }

   AudioCtrl::~AudioCtrl()
   {
      delete m_lang;
      delete m_log;
      if (m_waitingPrioritySound) {
         delete m_waitingPrioritySound;
      }
   }

   void AudioCtrl::messageReceived( const GuiProtMess* guiMessage )
   {
      GuiProtEnums::MessageType messageType = guiMessage->getMessageType();
      switch (messageType){
         case GuiProtEnums::PLAY_SOUNDS_REPLY:
            {
               DBG("Got PLAY_SOUNDS_REPLY from GUI.");
               handleSoundPlayComplete();
            } break;
         case GuiProtEnums::PREPARE_SOUNDS_REPLY:
            {
               DBG("Got PREPARE_SOUNDS_REPLY from GUI.");
               handleSoundPrepareComplete( static_cast<const GenericGuiMess*>(guiMessage) );
            } break;
         case GuiProtEnums::LOAD_AUDIO_SYNTAX:
            {
               DBG("Got LOAD_AUDIO_SYNTAX from GUI.");
               handleLoadAudioSyntax( static_cast<const GenericGuiMess*>(guiMessage) );
            } break;
         default:
            break;
      }

   }

   void AudioCtrl::handleRouteInfo(const RouteInfo& r)
   {
      bool onTrackStatusChanged;

      DBG("AudioCtrl::handleRouteInfo called (state %i)", m_state);

      if (m_lastRouteInfoValid) {
         onTrackStatusChanged = (m_lastRouteInfo.onTrackStatus != r.onTrackStatus);
      } else {
         onTrackStatusChanged = true;
         m_lastRouteInfoValid = true;
      }
      const char * camOld = m_lastRouteInfo.hasSpeedCameraLandmark();
      m_lastRouteInfo = r;
      const char * camNew = r.hasSpeedCameraLandmark();

      // Do nothing if the sound is muted.
      if (m_verbosity == AudioCtrlLanguage::VerbosityMuted) {
         return;
      }

      if ((r.currCrossing.changed || onTrackStatusChanged) && 
          (r.onTrackStatus == OnTrack) ) {
         soundNewCrossing();
      } else if( camNew && !camOld ) {
         // New camera appears sound the alarm!
         soundCameraWarning();
      } else if (onTrackStatusChanged) {
         switch (r.onTrackStatus) {
            case OnTrack:
               /* Can not happen */
               break;
            case OffTrack:
            case WrongWay:
            case Goal:
               calculatePrioritySound();
               break;
         }
      } else if (m_state == Waiting) {
         checkDeadlines();
      }
   }

#ifdef UNDER_CE
   //no MIN function defined in windows mobile. /Ola 2007-01-09
   template<class T>
   const T& MIN(const T& a, const T&b)
   {
      return a < b ? a : b;
   }
#endif

   void AudioCtrl::handleGPSInfo( const Quality& q ) {
      // GPS change? 
      if ( q >= QualityDemohx ) {
            // Demo don't act on it
      } else if ( q != m_gpsQuality ) {
         if ( q <= QualityUseless ) {
            // Bad GPS
            //       enum called[diss]connecttogps use here
            //      if dissconnect then missing-status is expected.
            bool dissconected = false;
            if ( m_uiCtrl->getcalledGPSConnectionMode() == 
                 UiCtrl::DisconnectToGPSCalled && q == QualityMissing )
            {
               dissconected = true;
               m_uiCtrl->setcalledGPSConnectionMode( 
                  UiCtrl::nothingCalled );
            }
            if ( m_gpsQuality > QualityUseless && m_badGPStimerID == 0 &&
                 !dissconected )
            {
               // Has had gps before and not started timer already
               uint32 gpsTime = 55; // s // Bluetooth takes 5s to get here
               if ( q == QualityUseless ) {
                  gpsTime += 5;
               }
               if ( m_badGPStimerID == 0 ) {
                  m_badGPStimerID = m_uiCtrl->getQueue()->defineTimer( 
                     gpsTime * 1000 );
               } else {
                  m_uiCtrl->getQueue()->setTimer( 
                     m_badGPStimerID, gpsTime * 1000 );
               }
            }
            m_badGpsQuality = q;
         } else { 
            // Ok GPS
            if ( m_badGPStimerID != 0 ) {
               m_uiCtrl->getQueue()->removeTimer( m_badGPStimerID );
               m_badGPStimerID = 0;
            }
            if ( m_uiCtrl->getcalledGPSConnectionMode() == 
                 UiCtrl::ConnectToGPSCalled )
            {
               m_uiCtrl->setcalledGPSConnectionMode( 
                  UiCtrl::nothingCalled );
               m_sentBadGPSLevel = 0;
            }
            if ( m_sentBadGPSLevel > 0 ) {
               if ( m_verbosity != AudioCtrlLanguage::VerbosityMuted ) {
                  playGPSSound( q, m_sentBadGPSLevel );
                  m_sentBadGPSLevel = 0;
               }
            }
         }

         // Store gps
         m_gpsQuality = q;
      } // End if quality changed
   }

   /* Sent from another part of UiCtrl */
   void AudioCtrl::playSoundForCrossing(uint16 crossingNo)
   {
      // Temporary workaround - we cannot request routlists by ourself. Yet. /petersv
      if (m_uiCtrl->m_lastRouteList == NULL) {
         return;
      } 

      // Do nothing if the sound is muted.
      if (m_verbosity == AudioCtrlLanguage::VerbosityMuted) {
         return;
      }

      std::vector <RouteInfoParts::RouteListCrossing *>::iterator i;

      i = m_uiCtrl->m_lastRouteList->crossings.begin();
      while (i != m_uiCtrl->m_lastRouteList->crossings.end()) {
         if ((*i)->wptNo == crossingNo) {
            AudioCtrlLanguage::SoundClipsList * soundClips = new AudioCtrlLanguage::SoundClipsList();
            m_lang->syntheziseCrossingSoundList((*i)->crossing, m_deadlines, *soundClips);
            if ( (soundClips->size()>0) && ((*soundClips)[0] != AudioClipsEnum::SoundNoKnownSound) ) {
               prioritySound(soundClips);
            } else {
               /* Use the new-crossing-sound if no other sound is applicable */
               delete soundClips;
               soundNewCrossing();
            }
            // FIXME - hack to prevent playing of sound due to "calculateNextNormalSound()".
            m_lastRouteInfoValid = false;
            return;
         }
         ++i;
      }
      DBG("Request for the audio cue for a crossing (%i) that was not cached. This is not implemented yet.",
          crossingNo);
   }


   void AudioCtrl::setDistanceMode(DistancePrintingPolicy::DistanceMode unitMode)
   {
      switch (unitMode) {
         case DistancePrintingPolicy::ModeInvalid:
         case DistancePrintingPolicy::ModeMetricSpace:
         case DistancePrintingPolicy::ModeMetric:
            m_lang->selectUnits(AudioCtrlLanguage::MetricUnits);
            break;
         case DistancePrintingPolicy::ModeImperialFeetSpace:
         case DistancePrintingPolicy::ModeImperialFeet:
            m_lang->selectUnits(AudioCtrlLanguage::ImperialFeetMilesUnits);
            break;
         case DistancePrintingPolicy::ModeImperialYardsSpace:
         case DistancePrintingPolicy::ModeImperialYards:
            m_lang->selectUnits(AudioCtrlLanguage::ImperialYardsMilesUnits);
            break;
      }
   }

   int AudioCtrl::setSyntaxVersion(const char * which)
   {
      int numClips = 0;
      char ** clipNames = NULL;
      int retval =  m_lang->selectSyntaxVersion(which, numClips, clipNames);
      
      if (retval) {
         ERR("selectSyntaxVersion error");
         // Something went wrong - notify the user.
         // FIXME
         return retval;
      }
      // Do not send up a message if there were no clips.
      // This aids in backwards compatibility.
      if (numClips > 0) {
         SoundFileListMess msg(clipNames, numClips);
         sendMessageToGui(&msg);
      }
      for (int i=0; i<numClips; ++i) {
         delete[] clipNames[i];
      }
      delete[] clipNames;
      return retval;
   }

   int AudioCtrl::setVerbosity( GuiProtEnums::TurnSoundsLevel verbosity )
   {
      AudioCtrlLanguage::AudioVerbosity aud_verbosity = AudioCtrlLanguage::VerbosityNormal;
      switch (verbosity) {
         case GuiProtEnums::turnsound_mute:
            aud_verbosity = AudioCtrlLanguage::VerbosityMuted;
            break;
         case GuiProtEnums::turnsound_min:
            aud_verbosity = AudioCtrlLanguage::VerbosityReduced;
            break;
         case GuiProtEnums::turnsound_less:
            aud_verbosity = AudioCtrlLanguage::VerbosityReduced;
            break;
         case GuiProtEnums::turnsound_normal:
            aud_verbosity = AudioCtrlLanguage::VerbosityNormal;
            break;
         case GuiProtEnums::turnsound_more:
            aud_verbosity = AudioCtrlLanguage::VerbosityNormal;
            break;
         case GuiProtEnums::turnsound_max:
            aud_verbosity = AudioCtrlLanguage::VerbosityNormal;
            break;
         case GuiProtEnums::turnsound_invalid:
            aud_verbosity = AudioCtrlLanguage::VerbosityNormal;
            break;
      }
      if (m_verbosity != aud_verbosity && 
          m_verbosity != AudioCtrlLanguage::VerbosityMuted) {
         // Verbosity has changed. Unless muted, reset all state.
         m_lastRouteInfoValid = false;
      }
      m_verbosity = aud_verbosity;
      return m_lang->setVerbosity(aud_verbosity);
   }

   void AudioCtrl::decodedShutdownNow( int16 upperTimeout ) {
      // Stop timer
      m_uiCtrl->getQueue()->removeTimer( m_badGPStimerID );
      m_badGPStimerID = 0;
   }

   bool AudioCtrl::decodedExpiredTimer( uint16 timerID ) {
      if ( m_badGPStimerID == timerID ) {
         m_uiCtrl->getQueue()->removeTimer( m_badGPStimerID );
         m_badGPStimerID = 0;

         // Do nothing if the sound is muted.
         if ( m_verbosity != AudioCtrlLanguage::VerbosityMuted ) {
            // Adding here to skipp the two level sounds.
            m_sentBadGPSLevel++;
            if ( m_sentBadGPSLevel == 0 ) {
               const uint32 gpsTime = 15;
               m_badGPStimerID = m_uiCtrl->getQueue()->defineTimer( 
                  gpsTime * 1000 );
            }
            // Keeping this to get to level two after first timeout
            m_sentBadGPSLevel++;
            playGPSSound( m_badGpsQuality, m_sentBadGPSLevel );
         }

         return true;
      } else {
         return false;
      }
   }


   void AudioCtrl::checkDeadlines()
   {
      if (m_deadlines.sayAtDistance == AudioCtrlLanguage::NoDistance) {
         // Priority sound or similar
         playSound();
         m_state = Playing;
         return;
      }

      /* FIXME - add better approximation of the distance to compensate for delay in 
       * speaking. */
      DBG("Checking delays. latency=%i, m_currentSoundLatency=%i, "
          "distToWpt=%"PRId32", speed=%i",
          m_lastRouteInfo.latency, m_currentSoundLatency, 
          m_lastRouteInfo.distToWpt, m_lastRouteInfo.speed);
      int32 totaldelay = m_lastRouteInfo.latency + m_currentSoundLatency + 6;   /* 0.6 s additional delay */
      int32 currDist = m_lastRouteInfo.distToWpt - (totaldelay * m_lastRouteInfo.speed) / 10;
      DBG("currDist=%"PRId32, currDist);
      if (currDist < 0) {
         currDist = 0;
      }
      if (currDist < m_deadlines.abortTooShortDistance) {
         // Missed the deadline (drove past it). Next normal sound.
         calculateNextNormalSound();
      } else if (currDist > m_deadlines.abortTooFarDistance) {
         // Missed deadline by mooving too far back
         m_lang->resetState();
         calculateNextNormalSound();
      } else if ( (currDist != AudioCtrlLanguage::NeverSay) && (
                  currDist <= m_deadlines.sayAtDistance) ) {
         playSound();
         m_state = Playing;
      }
   }

   void AudioCtrl::handleSoundPrepareComplete( const GenericGuiMess* guiMessage )
   {
      switch (m_state) {
         case Loading:
            m_state = Waiting;
            m_currentSoundLatency = guiMessage->getFirstUint32();
            
            checkDeadlines();
            break;
         case AbortingLoad:
            if (!m_waitingPrioritySound) {
               ERR("m_waitingPrioritySound == NULL and m_state==AbortingLoad in handleSoundPrepareComplete");
               m_state = Empty;
               break;
            }
            loadSound(*m_waitingPrioritySound);
            delete(m_waitingPrioritySound);
            m_waitingPrioritySound = NULL;
            m_state = Loading;
            break;
         default:
            WARN("Ignoring unexpected SoundPrepareComplete in state %i", m_state);
            break;
      }
   }


   void AudioCtrl::handleLoadAudioSyntax( const GenericGuiMess* guiMessage )
   {
      const char * synName = guiMessage->getFirstString();
      setSyntaxVersion(synName);
   }


   void AudioCtrl::handleSoundPlayComplete()
   {
      switch (m_state) {
         case Playing:
            calculateNextNormalSound();
            break;
         default:
            WARN("Ignoring unexpected SoundPlayComplete in state %i", m_state);
            break;
      }
   }


   void 
   AudioCtrl::sendMessageToGui(GuiProtMess* message)
   {
#ifndef NO_DATA_TO_GUI

      Buffer* serializedMessage = new Buffer(m_normalGuiPacketSize);
      message->setMessageId(0  /*m_nextGuiMessToSendId*/ );   // FIXME - use real id here!
      message->serialize(serializedMessage);


      if ( m_uiCtrl->connectedToGui() ){

         //DBGDUMP("AudioCtrl: Message data sent to GUI (prot 3):", 
         //          serializedMessage->accessRawData(),
         //          serializedMessage->getLength());
       
         // serializedMessage is deleted inside.

         m_serialProvPubl->sendData(serializedMessage);
      }
      else{
         // Forget all messages from AudioCtrl when not connected.
         // m_messagesToSendToGui->push_back(serializedMessage);
         delete serializedMessage;
      }
#endif
   }

   void AudioCtrl::playGPSSound( Quality q, uint32 sentBadGPSLevel ) {
      AudioCtrlLanguage::SoundClipsList * soundClips = 
         new AudioCtrlLanguage::SoundClipsList();
      if ( sentBadGPSLevel <= 1 ) {
         // Beep!
         if ( q <= QualityUseless ) {
            // Bad
            m_lang->badSoundList( m_deadlines, *soundClips );
         } else {
            // Good
            m_lang->goodSoundList( m_deadlines, *soundClips );
         }
      } else {
         m_lang->statusOfGPSChangedSoundList( 
            MIN( q, QualityPoor ), m_deadlines, *soundClips );
      }
      if ( soundClips->size() <= 0 ) {
         WARN("Empty GPS Change sound from language module, ignoring!!!");
         delete soundClips;
         // No state change here - the gps is just lost
      } else {
         prioritySound( soundClips );
      }
   }


   void AudioCtrl::loadSound(AudioCtrlLanguage::SoundClipsList &soundClips)
   {
      DBG("Loading sound:");
      AudioCtrlLanguage::SoundClipsList::const_iterator i;
      int * const sounds = new int[soundClips.size()];
      int *ptr = sounds;
      for (i = soundClips.begin(); i != soundClips.end(); ++i) {
         DBG("  Clip: %i", *i);
         if (*i == SoundNoKnownSound) {
            *ptr++ = SoundEnd;
         } else {
            *ptr++ = *i;
         }
      }
      PrepareSoundsMess msg(soundClips.size(), sounds);
      sendMessageToGui(&msg);
      delete[] sounds;
   }


   void AudioCtrl::playSound()
   {
      DBG("Playing loaded sound");
      GenericGuiMess msg(GuiProtEnums::PLAY_SOUNDS);
      sendMessageToGui(&msg);
   }


   void AudioCtrl::prioritySound(AudioCtrlLanguage::SoundClipsList *prioSound)
   {
      DBG("New priority sound in state %i", m_state);
      switch (m_state) {
         case Empty:
         case Waiting:
         case Playing:
            loadSound(*prioSound);
            delete prioSound;
            m_state=Loading;
            break;
         case Loading:
         case AbortingLoad:
            if (m_waitingPrioritySound) {
               delete m_waitingPrioritySound;
            }
            m_waitingPrioritySound = prioSound;
            m_state = AbortingLoad;
            break;
         default:
            ERR("AudioCtrl::prioritySound - unknown state %i", m_state);
            delete prioSound;
      }
   }


   void AudioCtrl::soundNewCrossing()
   {
      AudioCtrlLanguage::SoundClipsList * soundClips = new AudioCtrlLanguage::SoundClipsList();
      m_lang->newCrossingSoundList(m_deadlines, *soundClips);
      if (soundClips->size() <= 0) {
         WARN("Empty new crossing sound from language module!!!");
         soundClips->push_back(AudioClipsEnum::SoundEnd);
      }
      prioritySound(soundClips);
   }

   void AudioCtrl::soundCameraWarning()
   {
      AudioCtrlLanguage::SoundClipsList * soundClips = new AudioCtrlLanguage::SoundClipsList();
      m_lang->newCameraSoundList(m_deadlines, *soundClips);
      if (soundClips->size() <= 0) {
         WARN("Empty speed camera sound from language module, ignoring!!!");
         delete soundClips;
         // No state change here - the speed camera is ignored completely.
      } else {
         prioritySound(soundClips);
      }
   }

   void AudioCtrl::calculateNextNormalSound()
   {
      if (!m_lastRouteInfoValid || 
          (m_lastRouteInfo.onTrackStatus != OnTrack)) {
         m_state = Empty;
         return;
      }

      AudioCtrlLanguage::SoundClipsList * soundClips = new AudioCtrlLanguage::SoundClipsList();
      /* FIXME - add a soundClips->reserve() here for efficiency? */

      m_lang->syntheziseSoundList(m_lastRouteInfo, false, m_deadlines, *soundClips);

//      AudioCtrlLanguage::SoundClipsList::const_iterator i;
//      DBG("Synthesized sound:");
//      for (i = soundClips->begin(); i != soundClips->end(); ++i) {
//         DBG("Clip: %i", *i);
//      }

      if ( (soundClips->size()>0) && 
           ((*soundClips)[0] != AudioClipsEnum::SoundNoKnownSound)  &&
           ((*soundClips)[0] != AudioClipsEnum::SoundEnd) ) {
         loadSound(*soundClips);
         delete soundClips;
         m_state = Loading;
      } else {
         m_state = Empty;
         delete soundClips;
      }

   }

   void AudioCtrl::calculatePrioritySound()
   {
      AudioCtrlLanguage::SoundClipsList * soundClips = new AudioCtrlLanguage::SoundClipsList();
      /* FIXME - add a soundClips->reserve() here for efficiency? */

      m_lang->syntheziseSoundList(m_lastRouteInfo, true, m_deadlines, *soundClips);

//      AudioCtrlLanguage::SoundClipsList::const_iterator i;
//      DBG("Synthesized sound:");
//      for (i = soundClips->begin(); i != soundClips->end(); ++i) {
//         DBG("Clip: %i", *i);
//      }

      if (soundClips->size() <= 0) {
         WARN("Empty priority sound from language module!!!");
         soundClips->push_back(AudioClipsEnum::SoundEnd);
      }
      prioritySound(soundClips);
   }


} /* namespace isab */

