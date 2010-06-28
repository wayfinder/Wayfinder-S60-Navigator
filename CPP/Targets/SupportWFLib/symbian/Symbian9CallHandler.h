/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <e32base.h>
#include <etel3rdparty.h>

#include "LineStateListener.h"


class CSymbian9CallHandler : public CActive
{

public:

   static CSymbian9CallHandler* NewL( MLineStateListener* aListener,
                                      TInt aHangUpDlgRsc = -1 );

protected:

   CSymbian9CallHandler( MLineStateListener* aListener,
                         TInt aHangUpDlgRsc = -1 );

   void ConstructL();

public:

   ~CSymbian9CallHandler();

private:

   enum TCallState {
      EIdle = 0,
      EBusy,
      EGettingId,
      EDialing,
      EConnected,
      EHangingUp,
      EListening
   };

   /**
     These are the pure virtual methods from CActive that 
     MUST be implemented by all active objects
    */
   void RunL();

   void DoCancel();

public:

   void MakeCall( const TDesC& aNumber );

   void HangUp();

   void FindIMEI();

   char* GetIMEI();

   /**
    * Request voice line status change notification
    */
   void ListenForVoiceLineStatusChange( TCallState aState = EListening );

   TBool IsHookOn();

private:

   TBool IsDialableCharacter( TChar aChar ) const;

private:

   TCallState iState;

   MLineStateListener* iListener;

   CTelephony* iTelephony;

   CTelephony::TCallId iCallId;

   CTelephony::TPhoneIdV1 iPhoneIdV1;

   CTelephony::TPhoneIdV1Pckg iPhoneIdV1Pckg;

   TBool iHasImei;

   /// When the voice line status changes (and hence the asynchronous
   /// operation completes) the new voice line status is written into
   /// iLineStatus. Until this point, iLineStatus is not valid.
   CTelephony::TCallStatusV1 iLineStatus;

   CTelephony::TCallStatusV1Pckg iLineStatusPckg;

   TInt iHangUpDlgRsc;
};
