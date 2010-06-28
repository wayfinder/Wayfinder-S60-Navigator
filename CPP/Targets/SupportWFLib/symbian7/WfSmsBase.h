/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WF_SMS_BASE_H
#define WF_SMS_BASE_H
#if defined SYMBIAN_CKON || defined NAV2_CLIENT_UIQ

#include <msvapi.h>
#include <sendas.h>

namespace isab{
   class Favorite;
}

class CWfSmsBase : public CBase, public MMsvSessionObserver, 
                   public MSendAsObserver
{
protected:
   /** @name Constructors and destructor. */
   //@{
   CWfSmsBase(class MWfSmsHandler* aHandler, class TUid aMarkerUid);
   void BaseConstructL();

   virtual ~CWfSmsBase();
   //@}

private:
   /** @name From MMsvSessionObserver. */
   //@{
   virtual void HandleSessionEventL(enum TMsvSessionEvent aEvent, 
                                    TAny* aArg1, TAny* aArg2, TAny* /*aArg3*/);
   //@}



   /** @name Internal convenience functions. */
   //@{
   void HandleSingleChangedEntryL(TMsvId aMessageId);
   void HandleMsvEntriesChangedL(class CMsvEntrySelection& aSelection,
                                 TMsvId aFolderId);
   TBool ParseAndHandleMessageBodyL(const TDesC& aBody, TMsvId aId);
   void InitMsgArraysL();

   TMsvId CreateNewMessageL();
   void SetEntryL(TMsvId aEntryId);
   TBool CapabilityOK(TUid /*aCapabilty*/, TInt /*aResponse*/);
   void SetScheduledSendingStateL(class CMsvEntrySelection* aSelection);
   //@}
public:
   class CSendAs& getSendAs();
private:

   /** @name Functions that access Wayfinder SMSes.*/
   //@{
   const MDesCArray* GetSignatures() const;
public:

   CDesCArray* GetDestArray();
   CDesCArray* GetSenderArray();
   TArray<TMsvId> GetMsgIdArray();
   TBool GetAllSmsMsgL();

   void DeleteSmsL( TInt32 aMsvId );
   TPtrC GetSignature(TMsvId aId) const;
   HBufC* GetSenderLC(TMsvId aId);
   TInt IndexOf(TMsvId aId);
   void SetWfSmsBodyTextL( TInt aIdIdx );
   //@}
   bool SendSMSMessageL( const TDesC& recipientAddress, 
                         const TDesC& /*recipientAlias*/,
                         const TDesC& /*messageSubject*/,
                         const TDesC& messageText );

private:

   /** @name Pure abstracte member functions. */
   //@{
   virtual TBool VerifySms(class CMsvSession& aSession, TMsvId aEntryId) = 0;
   virtual void ShowInfoL(const TDesC& aMessage) = 0;
   //@}

   //member variables
   class MWfSmsHandler* iHandler;

   class CSmsClientMtm *iWfSmsMtm;

   class CClientMtmRegistry *iWfSmsMtmRegistry;

   class CMsvSession* iWfSmsSession;

   CDesCArray* iMsgArray;
   CDesCArray* iDestArray;
   CDesCArray* iSenderArray;
   CDesCArray* iSignatures;
   RArray<TMsvId> iMsgIDArray;

   //holds id of messages that have been created but are not yet
   //complete.
   RArray<TMsvId> iCreated;
   //data members for sms sending 
   TMsvId iMsvId;

   class CBaseMtm* iMtm;

   class CSendAs* iSendAs;

   class CSmsParser* iSmsParser;
   class TUid iMarkerUid;
};


#endif
#endif
