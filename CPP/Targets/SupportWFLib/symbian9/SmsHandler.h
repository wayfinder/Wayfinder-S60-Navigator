/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#ifndef SYMBIAN9_SMS_H_
#define SYMBIAN9_SMS_H_

#include <smsclnt.h>
#include <bamdesca.h>

#include "SmsHandlerFactory.h"


namespace isab
{
   class Favorite;
}

class CSmsHandler : public CActive, 
                    public MMsvSessionObserver,
                    public MSmsHandler
{

public:
   friend class CMessage;
   static CSmsHandler* NewL( class MSmsHandlerObserver* aObserver );
   CSmsHandler( class MSmsHandlerObserver* aObserver );
   void ConstructL();
   virtual ~CSmsHandler();

public: // From MSmsHandler
   
   /**
    * Public entry for sending one or more
    * sms.
    * @aSmsList List of all sms to be sent
    * @aNumber the recipients number
    */
   void SendSmsL(const MDesCArray& aSmsList,
                 const TDesC& aNumber);

   /**
    * Public entry for sending one or more
    * sms.
    * @aSmsText Text to be sent, is divided into
    *                several sms if needed.
    * @aNumber the recipients number
    */
   void SendSmsL(const TDesC& aSmsText,
                 const TDesC& aNumber);

   /**
    * Checks the inbox for wayfinder SMS messages and
    * creates arrays containing the messages.
    * @returns ETrue if there are any wayfinder SMS in the inbox
    */
   TBool CheckSmsInboxL();

   /**
    * Sends the content of the selected SMS to the SMS handler class. 
    */
   void SetSelectedWfSmsL( TInt aIdIdx );

   /**
    * Removes the sms from the inbox with the id aId.
    */
   void DeleteSmsL( TInt32 aMsvId );

   /**
    * Returns an array of wayfinder destination names.
    */
   CDesCArrayFlat* GetDestinationArray();

   /**
    * Returns an array of wayfinder destination names.
    */
   CDesCArrayFlat* GetSenderArray();

   /**
    * Only to be used on wins, sends a sms to the inbox.
    * @param aSmsText, The text in the message.
    * @param aNumber, The number to send to.
    */
   void CreateLocalMessageL(const TDesC &aMsgText,
                            const TDesC &aAddress);

private:

   /**
    * Checks that a sms service exists, if not
    * we will create one.
    */
   void InitSmsEntry();

   /**
    * Checks the queue for sms to be sent.
    */
   void ProcessQueueL();

   /**
    * Picks the first text to be sent from
    * the list.
    */
   void ProcessSmsListL(const MDesCArray* SmsList, const TDesC& aPhoneNbr);
   /**
    * This function sets the receipents phone number
    * and the text of the message to be sent.
    * @param aSmsText Text to be sent.
    * @param aPhoneNbr Number to send to.
    */
   void ProcessMessageL(const TDesC& aSmsText, const TDesC& aPhoneNbr);

   /** 
    * Saves the message to the outbox.
    * @param aMtm Needed for creating an entry
    *             and for saving the message.
    */
   void SaveMessageL();

   /**
    * Sends the sms, we will get a callback to RunL
    * when sms is sent.
    */
   void SendMessageL();

   /**
    * Gets called when an sms has been sent.
    */
   void SmsSent(TInt aStatus, TInt aCount, TInt aSent);

   /**
    * Gets called when an sms has been received.
    */
   TBool CheckIncommingSmsL( CMsvEntrySelection* aEntries );

   /**
    * Check if we need to send the sms as unicode or not.
    */
   TBool UnicodeToSMS7BitL(const TDesC& aUnicode, TDes8& aSms7Bit);

private: // From MMsvSessionObserver
   /**
    * Callback from framework when an sms event has
    * occured.
    */
   void HandleSessionEventL(TMsvSessionEvent , TAny* , TAny* , TAny* );

private: // From CActive   
   void DoCancel();
   void RunL();

private:
   // Sends a sms and contains progress information 
   // about sending process.
   class CMsvOperation* iMsvOperation;
   // Message server session
   class CMsvSession* iMsvSession;   
   // Client MTM registry
   class CClientMtmRegistry* iClientMtmRegistry;
   // SMS client MTM object
   class CSmsClientMtm* iSmsClientMtm;
   // Callback class that will be notified when
   // sending is done.
   class MSmsHandlerObserver* iObserver;
   // List of CSmsMessages that will be processed
   // one by one.
   class CArrayPtr<CMessage>* iSmsQueue;
   // Keeps track of the status, true if we are
   // sending a sms, false otherwise.
   TBool iRunning;
   // Current CSmsMessage to be sent.
   class CMessage* iMessage;
   // Counter to keep track of how many sms being sent
   // when text has to be divided into more than one sms
   TInt iNbrSmsSent;
   // Total nbr sms to be sent.
   TInt iNbrSmsTotal;

   CDesCArrayFlat* iMessageArray;

   CDesCArrayFlat* iDestinationArray;

   CDesCArrayFlat* iSenderArray;

   TMsvId *iMsgIDArray;

   class CSmsParser* iSmsParser;

};

#endif
