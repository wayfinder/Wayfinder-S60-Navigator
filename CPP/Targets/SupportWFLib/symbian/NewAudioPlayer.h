/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NEWAUDIOPLAYERN_H
#define NEWAUDIOPLAYERN_H

#include <set>
#include <mdaaudiosampleeditor.h>
#include <mdaaudiooutputstream.h>
#include "NewAudioServerTypes.h"

class NewAudioServer;
class NewAudioPlayerListener;

class NewAudioPlayerTimer;
class PlayerDeleter;

/**
 *   Class that can play clips from the NewAudioServer.
 */
class NewAudioPlayer : public CBase,
                       public MMdaAudioOutputStreamCallback {
public:

   /// Constructor
   NewAudioPlayer( NewAudioPlayerListener& listener );

   /// Destructor
   virtual ~NewAudioPlayer();
   
   /// Creates and opens the player etc.
   void ConstructL();

   /**
    *  Plays the converted sounds in the lists. Calls
    *  NewAudioPlayerListener::playComplete
    *  when complete, error or stopped.
    *  If it leaves, don't wait for the completion.
    */
   void playL( const TClips& clips );

   /**
    *  Stops playing. Calls NewAudioPlayerListener::playComplete
    *  if done.
    */
   void stop();

   /**
    *   Sets the volume as soon as possible.
    */
   void setVolumePercent( int percent );

   /**
    *   Sets the number of channels and samplerate.
    *   Not all combinations are possible.
    *   @return true if ok.
    */
   int setAudioProperties( int numchannels,
                           int samplerate );

   /**
    *   Returns the amount of silence before playing the sounds.
    */
   long getDurationOfSilenceBefore() const;

   /// Called by the framework when the player is opened
   void MaoscOpenComplete(TInt aError);
   /// Called by the framework when a buffer is copied. (Sometimes stopped)
   void MaoscBufferCopied(TInt aError, const TDesC8& aBuffer);
   /// Called by the framework when out of buffers. (Sometimes stopped)
   void MaoscPlayComplete(TInt aError);   
   
private:

   /// The timer wants to call functions here.
   friend class NewAudioPlayerTimer;

   /// Plays sounds. May complete the request if no clips were written
   void playSounds();
   
   /// Calls the callback of the listener and sets request active to false
   void completeRequest( TInt aError );
  
   /**
    * Restarts the player, and schedules the old player for deletion.
    * @return The system wide errorcode. KErrNone if all went well.
    */
   TInt restartPlayer();
   
   /// The clips to play.
   const TClips* m_clips;
   /// Reference to object to call back
   NewAudioPlayerListener& m_listener;
  /// The player itself
   CMdaAudioOutputStream* m_player;   
   /// Pointer to a player to delete at a good moment.
   CMdaAudioOutputStream* m_playerToDelete;   
   /// Player settings
   TMdaAudioDataSettings iBufSet;

   /// Wanted sample rate in Symbian enum
   TMdaAudioDataSettings::TAudioCaps m_wantedSampleRate;
   /// Wanted number of channels in Symbian
   TMdaAudioDataSettings::TAudioCaps m_wantedNbrChannels;
   
   /// True if a request is active.
   TBool m_requestActive;

   /// The percent of max volume
   TInt m_wantedVolumePercent;
   
   /// The number of buffers written and not copied
   int m_nbrWrittenBuffers;
   
   /// States used to check if the player is opened.
   enum {
      /// The player is not opened yet
      NOT_OPENED     = 0,
      /// The player is opening
      OPENING,
      /// Start playing as soon as the player is opened.
      PLAYING_WANTED,
      /// The player is opened.
      OPENED,
      /// The player is playing
      PLAYING,
   } m_playerState;

   /// Max amount of silence to write
   enum {
      c_silenceSize = 1024, // 8322 (Size of blippblipp)
#ifdef NAV2_CLIENT_UIQ3
      /// Silence to flush the sound out of the player.
      c_nbrSilenceToWrite = 10,
      /// Silence to start up the player
      c_nbrSilenceBefore = 3,
#else
      /// Silence to flush the sound out of the player.
      c_nbrSilenceToWrite = 8,
      /// Silence to start up the player
      c_nbrSilenceBefore = 2,
#endif
   };
   
   TBuf8<c_silenceSize> m_silence;

   
   
   /// Clip iterator
   ClipBufIterator m_bufIterator;
   /// True if we are stopping
   bool m_stopping;
   /// Number of silence buffers left to write
   int m_silenceToWrite;

   /// The A925 doesn't work like other UIQ phones.
   TBool iIsA925; 

   /// Timer to be used for delayed function calls
   NewAudioPlayerTimer* m_timer;

   /// Active object that deletes players.
   PlayerDeleter* m_playerDeleter;
   
};

#endif // NEWAUDIOPLAYERN_H
