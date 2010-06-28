/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H


namespace isab{
   class UiCtrl; // forward declaration.
   class RouteInfo;

   /**
    * Controls the audio (through the gui). Used by UiCtrl.
    *
    */
   class AudioCtrl
   {
      public:
      /**
       * Constructor.
       *
       * @param uiCtrl Pointer to the top module object UiCtrl.
       *               Used for communicating with the rest of
       *               Nav2.
       * @param log    The log object used for debug prints.
       *
       * @param navServComProvPubl Public provider for communicating
       *               with the server.
       */
      AudioCtrl( class UiCtrl* uiCtrl,
            class SerialProviderPublic* serialProvPubl,
            class ParameterProviderPublic* paramProvPubl,
            class AudioCtrlLanguage *audioSyntax);

      /**
       * Destructor.
       */
      virtual ~AudioCtrl();

      /**
       * Protocol version used for communication with the GUI.
       */
      enum {
         m_guiProtocolVersion = 8,
      };

      /**
       * Used when serializing GUI messages to a Buffer.
       *
       * Determines the size of data buffers needed for
       * serializing messages sent to the GUI.
       */
      enum {
         m_normalGuiPacketSize = 256,
      };

      /**
       * Takes the appropriate action for responding
       * to a message from the GUI.
       *
       * @param guiMessage  The message to respond to.
       */
      void messageReceived(const GuiProtMess* guiMessage);

      /**
       * Methods for handling messages to the GUI.
       */
      //@{

         /**
          * 
          */
         void handleRouteInfo(const RouteInfo& r); 

         /**
          * GPS status.
          */
         void handleGPSInfo( const Quality& q );
      //@}


      /**
       * Handle requests from other part of UiCtrl for
       * the sound for a specific crossing. This sound 
       * is abbreviated and does not contain any 
       * information regarding the current state (such
       * as distance from the crossing etc).
       */
      void playSoundForCrossing(uint16 crossingNo);


      /**
       * Select a different unit system for spoken
       * distances.
       */
      void setDistanceMode(DistancePrintingPolicy::DistanceMode unitMode);

      /**
       * Select a specific version of the syntax. This may be a 
       * wholly different syntax in the case of AudioCtrlLanguageScript.
       */
      int setSyntaxVersion(const char * which);

      /** 
       * Change verbosity, from 0 (muted), 1 (reduced) to 2 (normal)
       */
      int setVerbosity( GuiProtEnums::TurnSoundsLevel verbosity );

      /**
       * Called when timer has expired.
       *
       * @param timerid The id of the timer that has expired.
       */
      bool decodedExpiredTimer( uint16 timerid );

      void decodedShutdownNow( int16 upperTimeout );

   private:

      /**
       * Pointer to the top module object UiCtrl.
       * Used for communicating with the rest of
       * Nav2.
       *
       * Not owned by this object, so it should not be
       * deleted here.
       */
      UiCtrl* m_uiCtrl;

      /**
       * Log object used for debug prints. Not necessarily
       * used only by this object.
       *
       * Not owned by this object, so it should not be
       * deleted here.
       */
      Log* m_log;

      /**
       * Connects to the GUI.
       *
       * Should not be deleted in this class.
       */
      SerialProviderPublic* m_serialProvPubl;
      
      /**
       * Connects to the Parameter module.
       *
       * Should not be deleted in this class.
       */
      ParameterProviderPublic* m_paramProvPubl;


      void sendMessageToGui(GuiProtMess* message);

      void playGPSSound( Quality q, uint32 sentBadGPSLevel );

      /**
       * The current (latest) RouteInfo packet received from 
       * NavTask.
       */
      RouteInfo m_lastRouteInfo;

      /**
       * The variable m_lastRouteInfo is valid.
       */
      bool m_lastRouteInfoValid;

      /** The language specification */
      class AudioCtrlLanguage *m_lang;

      enum StateEnum {
         Empty,
         Loading,
         AbortingLoad,
         Waiting,
         Playing
      };

      StateEnum m_state;

      /** 
       * Latency from sending the PlaySound command to the gui to 
       * the time the timing marker is played. This lets AudioCtrl
       * play the distance marker at the precise moment the distance 
       * is passed. */
      int m_currentSoundLatency;

      /**
       * A priority sound (the latest) awaiting being loaded due
       * to another sound being played/loaded.
       * Set to NULL when no such sound is queued.
       */
      AudioCtrlLanguage::SoundClipsList *m_waitingPrioritySound;

      /**
       * Current setting of the verbosity. Used to reduce the number of
       * spoken turns etc. Also passed to the language module
       * for processing. */ 
      AudioCtrlLanguage::AudioVerbosity m_verbosity;

      /**
       * Deadlines for the currently loaded, loading or to be 
       * loaded sound, i.e. for the next sound to be played. 
       */
      AudioCtrlLanguage::DistanceInfo m_deadlines;

      void loadSound(AudioCtrlLanguage::SoundClipsList &coundClips);
      void playSound();
      void prioritySound(AudioCtrlLanguage::SoundClipsList *prioSound);
      void soundNewCrossing();
      void soundCameraWarning();
      void calculateNextNormalSound();
      void calculatePrioritySound();
      void checkDeadlines();


      void handleSoundPrepareComplete( const GenericGuiMess* guiMessage );
      void handleSoundPlayComplete();
      void handleLoadAudioSyntax( const GenericGuiMess* guiMessage );

      /// The current GPS quality, updated in handleGPSInfo.
      Quality m_gpsQuality;
      /// Timer for bad GPS.
      uint16 m_badGPStimerID;
      /// The bad GPS quality, updated in handleGPSInfo.
      Quality m_badGpsQuality;

      /**
       * The level of bad GPS message sent. 0 nothing sent, 1 beep sent
       * and 2 if voice sent.
       */
      uint32 m_sentBadGPSLevel;
   };  /* AudioCtrl */

} /* namespace isab */

#endif /* AUDIO_CONTROL_H */

