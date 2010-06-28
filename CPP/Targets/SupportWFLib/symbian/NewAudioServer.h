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

#ifndef NEWAUDIOSERVER_H
#define NEWAUDIOSERVER_H

#include "NewAudioPlayer.h"
#include "NewAudioConverter.h"
#include "NewAudioServerTypes.h"

#include <e32base.h>
#include <mdaaudiosampleeditor.h>
#include <mdaaudiooutputstream.h>
#include <vector>
#include <badesca.h>

#define TRUST_PLAY_COMPLETE

#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1
# undef TRUST_PLAY_COMPLETE
#endif

// In the emulator, MaoscPlayComplete is called after playing.
#if defined NAV2_CLIENT_SERIES60_V2 && !defined(__WINS__)
# undef TRUST_PLAY_COMPLETE
#endif

#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
class CNewAudioServer : public CServer2
#else
class CNewAudioServer : public CServer
#endif
{
   public:
      static CNewAudioServer * NewLC();
      virtual ~CNewAudioServer();

      /// Returns the special string for the timing marker sound
      static const TDesC& getTimingMarker();

      /// Returns the special string for the end marker.
      static const TDesC& getEndMarker();
   
   public:
      void StopServerL();
      
   protected:
      /**
       *  Internal class that interfaces with the audio
       *  subsystem of the phone. The sessions call on
       *  it to handle the requests.
       *
       *  TODO: Do not copy the memory used by clips containing same data.
       *        Reuse old clips instead.
       *  TODO: Do not convert the same clip more than once for each
       *        sound-list.
       */
      class CPlayerControl : public CBase,
                             public NewAudioPlayerListener,
                             public NewAudioConverterListener {
         
      public:
         static CPlayerControl * NewLC();
         virtual ~CPlayerControl();
         
      protected:
         CPlayerControl();
         void ConstructL();
         
      public:
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
         void HandleRequest( const RMessage2 & aMessage);
#else
         void HandleRequest( const RMessage & aMessage);
#endif
         void StopServerL();
         
      protected:         
            
         /**
          *   Store the given sound length in the buffer
          *   supplied by the client as a part of the call
          *   to PrepareSound(). 
          */
         void StoreSoundLengthReply(TInt aSoundLengthTenthsOfSec);
         
         // From NewAudioPlayerListener
         /// Called by NewAudioPlayer when complete
         void playCompleteL( TInt aError );
         // From NewAudioConverterListener
         /// Called by NewAudioConverter when complete
         void conversionCompleteL( TInt aError, long durationMS );
         
         // Functions not in any interface
         
         /**
          *   Reads clip-names from aMessage and the address space of the
          *   other thread. Then it parses it runs prepClips.
          */
       #if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
         void decodeAndPrepClipsL( const RMessage2& aMessage );
       #else
         void decodeAndPrepClipsL( const RMessage& aMessage );
       #endif
         
         /// Prepare clips. Move from wanted list to load list etc.
         void prepClips( const MDesCArray& filenames );
         
         /// Complete the current request if it is active
         void completeRequest( TInt aError );
            
      protected:
         /// Message that caused the current activity.
      #if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
         RMessage2 iMessage;
      #else
         RMessage iMessage;
      #endif
         /// True if we have an active request and message.
         TBool m_requestActive;
         /// True if we are cancelling the currently active request.
         TBool m_cancelling;
         /// True if we are stopping the server.
         TBool iStopping;
         
         /// The audio player
         NewAudioPlayer* m_audioPlayer;
         
         /// The audio converter
         NewAudioConverter* m_audioConverter;
         
         /// The clips are placed here. Some are loaded, some are not.
         TClips m_clips;
         
         /**
          *   Linear search from begin to end-1 for a clip with
          *   filename <code>filename</code>.
          *   @return <code>end</code> if not found.
          */
         static TClips::iterator findClip(TClips::iterator begin,
                                          TClips::iterator end,
                                          const TDesC& filename );         
      };

      // Handle connections from one client
   #if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
      class CNewAudioSession : public CSession2
   #else
      class CNewAudioSession : public CSharableSession
   #endif
      {
         public:
            CNewAudioSession(CPlayerControl & aPlayerControl);
            virtual ~CNewAudioSession();

         protected:
          #if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
            void ServiceL(const RMessage2 &aMessage);
          #else
            void ServiceL(const RMessage& aMessage);
          #endif

         protected:
            CPlayerControl & iPlayerControl;
      };



   protected:    // From CServer
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   CSession2* NewSessionL(const TVersion &aVersion, const RMessage2& aMessage) const;
#else
   CSharableSession* NewSessionL(const TVersion& aVersion) const;
#endif

   protected:
      CNewAudioServer();
      void ConstructL();

   protected:
      CPlayerControl * iPlayerControl;   

};



#endif /* NEWAUDIOSERVER_H */
