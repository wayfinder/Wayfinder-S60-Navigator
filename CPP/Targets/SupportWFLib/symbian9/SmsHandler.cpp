/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#ifdef SYMBIAN_9

#include <eikenv.h>
#include <txtrich.h>
#include <mtclreg.h>
#include <smuthdr.h>

#include "SmsHandler.h"
#include "WFTextUtil.h"
#include "SmsHandlerObserver.h"
#include "SmsParser.h"

#include "TraceMacros.h"

using namespace isab;

const TMsvId KInbox = KMsvGlobalInBoxIndexEntryId;

class CMessage : public CBase
{
public:
   static CMessage* NewL(const MDesCArray& aSmsList,
                         const TDesC& aPhoneNbr)
   {
      CMessage* self = new (ELeave) CMessage();
      CleanupStack::PushL(self);
      self->ConstructL(aSmsList, aPhoneNbr);
      CleanupStack::Pop(self);
      return self;      
   }

   static CMessage* NewL(const TDesC& aSmsText,
                         const TDesC& aPhoneNbr)
   {
      CMessage* self = new (ELeave) CMessage();
      CleanupStack::PushL(self);
      self->ConstructL(aSmsText, aPhoneNbr);
      CleanupStack::Pop(self);
      return self;            
   }

   void ConstructL(const MDesCArray& aSmsList,
                   const TDesC& aPhoneNbr)
   {
      iSmsList = new (ELeave) CDesCArrayFlat(aSmsList.MdcaCount());
      for (TInt i = 0; i < aSmsList.MdcaCount(); ++i) {
         iSmsList->AppendL(aSmsList.MdcaPoint(i));
      }
      iPhoneNbr = aPhoneNbr.AllocL();
   }

   void ConstructL(const TDesC& aSmsText,
                   const TDesC& aPhoneNbr)
   {
      iSmsList = new (ELeave) CDesCArrayFlat(1);
      iSmsList->AppendL(aSmsText);
      iPhoneNbr = aPhoneNbr.AllocL();
   }

   virtual ~CMessage()
   {
      if (iSmsList) {
         iSmsList->Reset();
         delete iSmsList;
      }
      delete iPhoneNbr;
   }

private:
   CMessage()
   {
   }

public: 
   CDesCArray* iSmsList;
   HBufC* iPhoneNbr;
};
 
CSmsHandler* CSmsHandler::NewL( class MSmsHandlerObserver* aObserver )
{
   CSmsHandler* self = new (ELeave) CSmsHandler( aObserver );
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}
  
CSmsHandler::~CSmsHandler()
{
   Cancel();
   delete iSmsClientMtm;
   delete iClientMtmRegistry;
   delete iMsvSession;
   if (iSmsQueue) {
      iSmsQueue->ResetAndDestroy();
      delete iSmsQueue;
   }
   if( iMessageArray ){
      delete iMessageArray;
      iMessageArray = NULL;
   }
   if( iDestinationArray ){
      delete iDestinationArray;
      iDestinationArray = NULL;
   }
   if( iSenderArray ){
      delete iSenderArray;
      iSenderArray = NULL;
   }
   if( iMsgIDArray ){
      delete[] iMsgIDArray;
      iMsgIDArray = NULL;
   }
   delete iSmsParser;
}

CSmsHandler::CSmsHandler( class MSmsHandlerObserver* aObserver ) :
   CActive(EPriorityStandard),
   iObserver(aObserver)
{
}

void CSmsHandler::ConstructL() 
{
   CActiveScheduler::Add(this);
   iMsvSession = CMsvSession::OpenSyncL(*this);
   // Create client registry object
   iClientMtmRegistry = CClientMtmRegistry::NewL(*iMsvSession);
   iSmsClientMtm = (CSmsClientMtm*)iClientMtmRegistry->
      NewMtmL(KUidMsgTypeSMS);
   //InitSmsEntry();
   // Allocate memory for 3 objects, probably not very
   // often we need to queue up any sms. 
   iSmsQueue = new (ELeave) CArrayPtrFlat<CMessage>(3);
   iSmsParser = CSmsParser::NewL();
}

void CSmsHandler::SendSmsL(const MDesCArray& aSmsList, 
                           const TDesC& aNumber)
{
   // Add the next message in the queue for later
   // processing.
   iSmsQueue->AppendL(CMessage::NewL(aSmsList,
                                     aNumber));
   if (iRunning) {
      //TRACE_FUNC1("RUNNING");
      return;
   } else {
      iRunning = true;
   }
   ProcessQueueL();
}

void CSmsHandler::SendSmsL(const TDesC& aText,
                           const TDesC& aNumber)
{
   iSmsQueue->AppendL(CMessage::NewL(aText,
                                     aNumber));
   if (iRunning ) {
      //TRACE_FUNC1("RUNNING");
      return;
   } else {
      iRunning = true;
   }
   ProcessQueueL();
}

void CSmsHandler::ProcessQueueL()
{
   if (iSmsQueue->Count() > 0) {
      // There is at least one element in the
      // queue waiting to be sent.
      iMessage     = iSmsQueue->At(0); 
      iNbrSmsSent  = 0;
      iNbrSmsTotal = 0;
      ProcessSmsListL(iMessage->iSmsList, *iMessage->iPhoneNbr);
   } else {
      // No more sms in the queue, go back to idle state.
      iRunning = EFalse;
   }
}
   
void CSmsHandler::ProcessSmsListL(const MDesCArray* aSmsList, 
                                  const TDesC& aPhoneNbr)
{
   if (aSmsList->MdcaCount() > 0) {
      ProcessMessageL(aSmsList->MdcaPoint(0), aPhoneNbr);
   } else {
      // Sending is done, complete a request and let
      // runl check what to do.
      class TRequestStatus* status = &iStatus; 
      iStatus = KRequestPending;
      if (!IsActive()) {
         SetActive();
      }
      User::RequestComplete(status, KErrNone);
   }
}

void CSmsHandler::ProcessMessageL(const TDesC& aSmsText, 
                                  const TDesC& aPhoneNbr)
{
   // Set context to Outbox folder
   iSmsClientMtm->SwitchCurrentEntryL(KMsvGlobalOutBoxIndexEntryId);
   // Create new message entry
   iSmsClientMtm->CreateMessageL(0);
   if (aSmsText.Length()) {      

      // Check if we should send it as UCS2 or not. 
      // We can also compare the strings afterwards if we need to.
      HBufC8* sms7bit = HBufC8::NewLC(aSmsText.Length() * 2);
      TPtr8 sms7bitptr = sms7bit->Des();
      if (!UnicodeToSMS7BitL(aSmsText, sms7bitptr)) {
         CSmsSettings& serviceSettings = iSmsClientMtm->ServiceSettings();
         serviceSettings.SetCharacterSet(TSmsDataCodingScheme::ESmsAlphabetUCS2);
         iSmsClientMtm->SmsHeader().SetSmsSettingsL(serviceSettings);
      }
      CleanupStack::PopAndDestroy(sms7bit);

      // Set body text
      CRichText& body = iSmsClientMtm->Body();
      body.InsertL(0, aSmsText);
   }
   if (aPhoneNbr.Length()) { 
      // Set address
      iSmsClientMtm->AddAddresseeL(aPhoneNbr);
   }
   SaveMessageL();
}

TBool CSmsHandler::UnicodeToSMS7BitL(const TDesC& aUnicode, TDes8& aSms7Bit)
{
   CCnvCharacterSetConverter* converter = CCnvCharacterSetConverter::NewLC();

   if (converter->PrepareToConvertToOrFromL(KCharacterSetIdentifierSms7Bit, 
                                            CEikonEnv::Static()->FsSession()) !=
       CCnvCharacterSetConverter::EAvailable) {
      CleanupStack::PopAndDestroy(converter);
      return EFalse;
   }

   TInt nbrUnConv = 0;
   TInt ret = converter->ConvertFromUnicode(aSms7Bit, aUnicode, nbrUnConv);
   
   if ((ret == CCnvCharacterSetConverter::EErrorIllFormedInput)) {
      CleanupStack::PopAndDestroy(converter);
      return EFalse;
   }

   CleanupStack::PopAndDestroy(converter);
   if ((nbrUnConv != 0) || (ret > 0)) {
      return EFalse;
   } else {
      return ETrue;
   }
}


/** Commits message to message store. 
*/
void CSmsHandler::SaveMessageL()
{
   TMsvEntry ent = iSmsClientMtm->Entry().Entry();
   ent.SetVisible(ETrue);
   ent.SetInPreparation(EFalse);
   ent.SetSendingState(KMsvSendStateWaiting);
   ent.iDate.HomeTime();
   iSmsClientMtm->Entry().ChangeL(ent);
   iSmsClientMtm->SaveMessageL();
   SendMessageL();
}

void CSmsHandler::SendMessageL()
{
   // get a list of SMS messages in the outbox
   CMsvEntry* entry = iMsvSession->GetEntryL(KMsvGlobalOutBoxIndexEntryId);
   CleanupStack::PushL(entry);
   CMsvEntrySelection* smsEntries = entry->ChildrenWithMtmL(KUidMsgTypeSMS);
   CleanupStack::PushL(smsEntries);
   if(smsEntries->Count()) {
      iMsvOperation = entry->CopyL(iSmsClientMtm->Entry().EntryId(),
                                   iSmsClientMtm->ServiceId(),
                                   iStatus);
//       iMsvOperation = entry->CopyL(*smsEntries,
//                                    iSmsClientMtm->ServiceId(),
//                                    iStatus);
   } else {
      iMsvOperation = CMsvCompletedOperation::NewL(*iMsvSession,
                                                   KUidMsgTypeSMS,
                                                   KNullDesC8,
                                                   iSmsClientMtm->ServiceId(),
                                                   iStatus);
      CEikonEnv::Static()->AlertWin(_L("Count == 0"), _L("B"));
   }
   CleanupStack::PopAndDestroy(smsEntries);
   CleanupStack::PopAndDestroy(entry);
   SetActive();
   // if the operation completed synchronously, 
   // we need to go ahead and complete ourselves
   if(!iMsvOperation->IsActive()){
      TRequestStatus* status = &iStatus ;
      User::RequestComplete(status, KErrNone);
   }   
}

/** On completion, display progress */
void CSmsHandler::RunL() 
{
   if (iStatus == KErrNone) {
      // get progress information after send operation completes
      TSmsProgressBuf progressBuf;
      progressBuf.Copy(iMsvOperation->FinalProgress());
      TSmsProgress progress = progressBuf();
      if(progress.iError) {
         // Error sending sms!
         iNbrSmsTotal += 1;
         if (iMessage->iSmsList->Count() > 0) {
            iMessage->iSmsList->Delete(0);
         }
      } else {
         // Delete the recently sent text.
         iNbrSmsTotal += 1;
         iNbrSmsSent  += 1;
         if (iMessage->iSmsList->Count() > 0) {
            iMessage->iSmsList->Delete(0);
         }
      } 
      if (iMessage->iSmsList->Count() > 0) {
         // More text to be sent!
         ProcessSmsListL(iMessage->iSmsList, 
                         *iMessage->iPhoneNbr);
      } else {
         // Current sms is now done, all text
         // is sent. Notify callback.
         if (progress.iError) {
            // There was an error sending, the phone will show an error dialog.
            // We also make the callback with an error.
            SmsSent(KErrGeneral, iNbrSmsTotal, iNbrSmsSent);
         } else {
            SmsSent(iStatus.Int(), iNbrSmsTotal, iNbrSmsSent);
         }
      }
   } else {
      // Error sending sms, report to observer.
      SmsSent(iStatus.Int(), iNbrSmsTotal, iNbrSmsSent);
   }
}

void CSmsHandler::DoCancel()
{
   iMsvOperation->Cancel();
   TRequestStatus* status = &iStatus ;
   User::RequestComplete(status, KErrCancel);   
}

void CSmsHandler::SmsSent(TInt aStatus, TInt aCount, TInt aSent)
{
   //TRACE_FUNC();
   if( iObserver ){ 
      iObserver->SmsSent(aStatus, aCount, aSent);
   }
   if (iSmsQueue->Count() > 0) {
      iSmsQueue->Delete(0);
   }
   ProcessQueueL();
}

void CSmsHandler::CreateLocalMessageL( const TDesC &aMsgText,
                                       const TDesC &aAddress )
{
   // Switch to the given folder
   iSmsClientMtm->SwitchCurrentEntryL( KInbox ); 
   iSmsClientMtm->CreateMessageL( KUidMsgTypeSMS.iUid );

   CMsvStore* aMessageStore = iSmsClientMtm->Entry().EditStoreL();
   CleanupStack::PushL( aMessageStore );
   CleanupStack::PopAndDestroy( aMessageStore );

   CRichText& body = iSmsClientMtm->Body();    
   body.Reset();
   body.InsertL( 0, aMsgText );

   TMsvEntry entry = iSmsClientMtm->Entry().Entry();
   entry.SetInPreparation( EFalse );
   entry.SetVisible( ETrue );
   entry.iDate.HomeTime();
   entry.iDetails.Set( aAddress );
   entry.SetUnread( ETrue );

   iSmsClientMtm->Entry().ChangeL(entry);
   iSmsClientMtm->SaveMessageL();   
   iObserver->SmsSent(KErrNone, 1, 1);
}

/**
Called by CMsvSession when a messaging event has occurred. It is used here to 
find out if any new messages have been created.
@param aEvent Event type
@param aArg1 Event arguments
*/
void CSmsHandler::HandleSessionEventL( TMsvSessionEvent aEvent,
                                       TAny* aArg1, TAny* , TAny* )
{
   //TRACE_FUNC();

   // check the event type
   switch (aEvent)
   {
   // new messages have been created in the message store
   case EMsvEntriesCreated:
      //TRACE_FUNC1( "EMsvEntriesCreated" );
      break;
   // One or more index entries have been changed. 
   case EMsvEntriesChanged:
      //TRACE_FUNC1( "EMsvEntriesChanged" );
      CMsvEntrySelection* entries = static_cast<CMsvEntrySelection*>(aArg1);
      if( entries ){
         if( CheckIncommingSmsL( entries ) ){
            //TRACE_FUNC1( "Is a Wayfinder SMS" );
            CRichText& body = iSmsClientMtm->Body();
            const TInt textLen = body.DocumentLength();
            TBuf<64> errorStr;
            TPtrC messageBody;
            messageBody.Set(iSmsClientMtm->Body().Read(0, textLen));
            iSmsParser->ParseSmsL( messageBody, errorStr );
            TMsvId messageID = (*entries)[0];
            iObserver->SmsReceived( iSmsParser, messageID );
         }
         else{
            //TRACE_FUNC1( "Not a Wayfinder SMS" );
         }
      }
      break;
   // One or more entries have been deleted.
   case EMsvEntriesDeleted:
      //TRACE_FUNC1( "EMsvEntriesDeleted" );
      break;
   // One or more entries have been moved.
   case EMsvEntriesMoved:
      //TRACE_FUNC1( "EMsvEntriesMoved" );
      break;
   // A new MTM has been installed. 
   case EMsvMtmGroupInstalled:
      //TRACE_FUNC1( "EMsvMtmGroupInstalled" );
      break;
   // A MTM has been uninstalled. 
   case EMsvMtmGroupDeInstalled:
      //TRACE_FUNC1( "EMsvMtmGroupDeInstalled" );
      break;
   // Received after a client has used CMsvSession::OpenAsyncL() to create a session.
   case EMsvServerReady:
      //TRACE_FUNC1( "EMsvServerReady" );
      break;
   // The Message Server index had been corrupted and had to be rebuilt.
   case EMsvCorruptedIndexRebuilt:
      //TRACE_FUNC1( "EMsvCorruptedIndexRebuilt" );
      break;
   // The Message Server has automatically changed the index location to use the 
   // internal disk.
   case EMsvMediaChanged:
      //TRACE_FUNC1( "EMsvMediaChanged" );
      break;
   // The media (disk) containing the Message Server index has been removed. 
   case EMsvMediaUnavailable:
      //TRACE_FUNC1( "EMsvMediaUnavailable" );
      break;
   // The disk containing the Message Store is available again.
   case EMsvMediaAvailable:
      //TRACE_FUNC1( "EMsvMediaAvailable" );
      break;
   // An incorrect disk is inserted. 
   case EMsvMediaIncorrect:
      //TRACE_FUNC1( "EMsvMediaIncorrect" );
      break;
   // The Message Server has started to rebuild its index after it has been corrupted.
   case EMsvCorruptedIndexRebuilding:
      //TRACE_FUNC1( "EMsvCorruptedIndexRebuilding" );
      break;

   // these cases indicate a problem that can't be recovered from

   // Something has happening in the server, but this client was unable to retrieve
   // the information.
   case EMsvGeneralError:
      //TRACE_FUNC1( "EMsvGeneralError" );
      //CActiveScheduler::Stop();
      break;
   // The client should immediately close the session with the Message Server.
   case EMsvCloseSession:
      //TRACE_FUNC1( "EMsvCloseSession" );
      //CActiveScheduler::Stop();
      break;
   // The Message Server has been terminated. 
   case EMsvServerTerminated:
      //TRACE_FUNC1( "EMsvServerTerminated" );
      //CActiveScheduler::Stop();
      break;
   // Received after a client has used CMsvSession::OpenAsyncL() to create a session.
   case EMsvServerFailedToStart:
      //TRACE_FUNC1( "EMsvServerFailedToStart" );
      //CActiveScheduler::Stop();
      break;
   default:
      char traceStr[32];
      sprintf( traceStr, "default: %i", aEvent );
      //TRACE_FUNC1( traceStr );
      break;
   }
}


TBool CSmsHandler::CheckIncommingSmsL( CMsvEntrySelection* aEntries )
{
   //TRACE_FUNC();
   TBool match = EFalse;
   TInt count = aEntries->Count();

   // check each entry to see if we want to process it
   while (count--){
      // get the index entry
      TMsvId serviceId;
      TMsvEntry entry;
      iSmsClientMtm->Session().GetEntry((*aEntries)[count], serviceId, entry);
      // if the entry is an SMS message
      if( entry.iMtm == KUidMsgTypeSMS &&
          entry.iType == KUidMsvMessageEntry &&
          // and if it's an incoming message (as its in the inbox)
          entry.Parent() == KMsvGlobalInBoxIndexEntryId ){
         iSmsClientMtm->SwitchCurrentEntryL((*aEntries)[count]);
         iSmsClientMtm->LoadMessageL();
         // Check the message body against any specified match text
         CRichText& body = iSmsClientMtm->Body();
         match = iSmsParser->IsWayfinderSms( body );
      }
   }
   return match;
}


TBool CSmsHandler::CheckSmsInboxL()
{
   //TRACE_FUNC();

   TBool result = EFalse;
   TInt numOfSms = 0;
   TInt numWfSMS=0;
   TMsvId messageMsvID;

   // The Session is already created.
   // We need to construct An Entry Point in the  Tree KMsvDraftEntryId 
   CMsvEntry* msvEntry = CMsvEntry::NewL( *iMsvSession,
                                          KMsvGlobalInBoxIndexEntryId,
                                          TMsvSelectionOrdering() );
   CleanupStack::PushL(msvEntry);
   
   CMsvEntrySelection* msvSelection = msvEntry->ChildrenL();
   CleanupStack::PushL(msvSelection);

   numOfSms = msvSelection->Count();
   if(numOfSms > 0){
      delete iMessageArray;
      iMessageArray = NULL;
      iMessageArray = new (ELeave) CDesCArrayFlat(numOfSms);
      
      delete iDestinationArray;
      iDestinationArray = NULL;
      iDestinationArray = new (ELeave) CDesCArrayFlat(numOfSms);

      delete iSenderArray;
      iSenderArray = NULL;
      iSenderArray = new (ELeave) CDesCArrayFlat(numOfSms);
      
      delete[] iMsgIDArray;
      iMsgIDArray = NULL;
      iMsgIDArray = new (ELeave) TMsvId[numOfSms];

      for( TInt i=0; i < numOfSms; i++ ){
         messageMsvID = msvSelection->At(i);
         if( iMsvSession == NULL ){
            // SMS Error
            continue;
         }
         TRAPD(lcode1,iSmsClientMtm->SwitchCurrentEntryL( messageMsvID ));
         if(lcode1 == KErrNotFound ){
            // SMS Error
            continue;
         }
         else if( lcode1 == KErrNoMemory ){
            // Insufficient memory
            continue;
         }
         TRAPD(l, iSmsClientMtm->LoadMessageL());
         if (l != KErrNone){
            // SMS Error
            continue;
         }
         const CRichText& body = iSmsClientMtm->Body();
         if( iSmsParser->IsWayfinderSms( body ) ){
            const TInt textLen = body.DocumentLength();
            /// Message body of the last SMS parsed
            TBuf<64> errorStr;
            TPtrC messageBody;
            messageBody.Set(iSmsClientMtm->Body().Read(0, textLen)); 
            iMessageArray->AppendL(messageBody);
            iSmsParser->ParseSmsL( messageBody, errorStr );
            iDestinationArray->AppendL( iSmsParser->GetDestinationDescription() );
            CMsvEntry* cEntry = iMsvSession->GetEntryL(messageMsvID);
            TMsvEntry tEntry = cEntry->Entry();           
            iSenderArray->AppendL( tEntry.iDetails );
            iMsgIDArray[numWfSMS]=messageMsvID;
            numWfSMS++;
         }
      }
      if( numWfSMS > 0 ){
         result = ETrue;
      }
      iSmsClientMtm->RestoreServiceAndSettingsL();
   }
   CleanupStack::PopAndDestroy(2, msvEntry);// msvSelection, msvEntry
   return result;
}


CDesCArrayFlat* CSmsHandler::GetDestinationArray()
{
   return iDestinationArray;
}


CDesCArrayFlat* CSmsHandler::GetSenderArray()
{
   return iSenderArray;
}


void CSmsHandler::SetSelectedWfSmsL( TInt aIdIdx )
{
   TBuf<64> errorStr;
   TMsvId messageID = iMsgIDArray[aIdIdx];
   iSmsParser->ParseSmsL( iMessageArray->MdcaPoint( aIdIdx ), errorStr );
   iObserver->SmsReceived( iSmsParser, messageID );
}


void CSmsHandler::DeleteSmsL( TInt32 aMsvId )
{
   CMsvEntry* wfEntry = iMsvSession->GetEntryL(aMsvId);
   CleanupStack::PushL(wfEntry);
   wfEntry->DeleteL(aMsvId);
   CleanupStack::Pop(wfEntry);
}

#endif
