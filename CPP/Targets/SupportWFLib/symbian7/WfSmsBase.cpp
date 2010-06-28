/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if defined SYMBIAN_CKON || defined NAV2_CLIENT_UIQ

#include "WfSmsBase.h"
#include <smut.h>
#include <smsclnt.h>
#include <txtrich.h>
#include <smscmds.h> 
#include <smutset.h>
#include <smuthdr.h>
#include <eikenv.h>
#include "WfSmsHandler.h"
#include "SmsParser.h"

#include "TraceMacros.h"
#include "CleanupSupport.h"

#ifdef ENABLE_TRACE
# define TRACE_FUNC_FMT1(fmt, arg1)             \
   do {                                         \
      char trace[32];                           \
      sprintf( trace, fmt, arg1 );              \
      TRACE_FUNC1( trace );                     \
   } while(0)
#else
# define TRACE_FUNC_FMT1(fmt, arg1) do{}while(0)
#endif

CWfSmsBase::CWfSmsBase(class MWfSmsHandler* aHandler, class TUid aMarkerUid) : 
   iHandler(aHandler), iMarkerUid(aMarkerUid)
{
}

void CWfSmsBase::BaseConstructL()
{
   //create early as it may take some time to be properly created.
   iSendAs = CSendAs::NewL(*this);

   //Create message server session. The synchronous operation is not
   //desireable, but it's simpler this way.
   iWfSmsSession = CMsvSession::OpenSyncL( *this );
   //The registry gives access to the different MTMs
   iWfSmsMtmRegistry = CClientMtmRegistry::NewL(*iWfSmsSession);
   //We just need an SMS MTM.
   class CBaseMtm* mtm = iWfSmsMtmRegistry->NewMtmL(KUidMsgTypeSMS);
   iWfSmsMtm = static_cast<CSmsClientMtm*>(mtm);
   //The CSmsParser parses Wayfinder messages.
   iSmsParser = CSmsParser::NewL();
}

CWfSmsBase::~CWfSmsBase()
{
   delete iSendAs;
   delete iMsgArray;
   delete iDestArray;
   iMsgIDArray.Close();
   delete iSenderArray;
   delete iWfSmsMtm;
   delete iMtm;
   delete iWfSmsMtmRegistry;
   delete iWfSmsSession;
}

void CWfSmsBase::HandleSingleChangedEntryL(TMsvId aMessageId)
{
   if (iWfSmsMtm == NULL){
      return;
   }
   TRAPD(lcode1,iWfSmsMtm->SwitchCurrentEntryL( aMessageId ));
   if(lcode1 == KErrNotFound ){
      ShowInfoL(_L("entry does not exist "));
      return;
   }
   else if(lcode1 == KErrNoMemory ){
      ShowInfoL(_L("Insufficient memory"));
   }
   TRAPD(l, iWfSmsMtm->LoadMessageL());
   if (l != KErrNone) {
      return;
   }
   //extract the body of the message
   const class CRichText& body = iWfSmsMtm->Body();
   if( iSmsParser->IsWayfinderSms( body ) ){
      const TInt textLen = body.DocumentLength();
      /// Message body of the last SMS parsed
      HBufC* bodybuf = HBufC::NewLC(textLen);
      TPtr messageBody = bodybuf->Des();
      body.Extract( messageBody );
      //parse the message
      ParseAndHandleMessageBodyL(messageBody, aMessageId);
      CleanupStack::PopAndDestroy(bodybuf);
   }
   iWfSmsMtm->RestoreServiceAndSettingsL();
}

void CWfSmsBase::HandleMsvEntriesChangedL(class CMsvEntrySelection& aSelection,
                                          TMsvId aFolderId)
{
   if(aFolderId == KMsvGlobalInBoxIndexEntryId){
      //the changed messages are in the inbox, and thus of
      //interest to us.
      for(TInt i = 0; i < aSelection.Count(); ++i){
         const TMsvId messageID = aSelection.At(i);
         //check if this is a message that we are "watching"
         const TInt pos = iCreated.Find(messageID);
         if(pos >= 0){
            //it is a new message.
            class CMsvEntry* entry = iWfSmsSession->GetEntryL(messageID);
            CleanupStack::PushL(entry);
            if(entry->Entry().Complete()){
               //the new message is completed.
               //remove the id from the watchlist
               iCreated.Remove(pos);
               //handling the entry is refactored into a separate function.
               HandleSingleChangedEntryL(messageID);
            }
            CleanupStack::PopAndDestroy(entry);
         }
      }
   }
}


void CWfSmsBase::HandleSessionEventL(enum TMsvSessionEvent aEvent, 
                                     TAny* aArg1, TAny* aArg2, TAny* /*aArg3*/)
{
   switch(aEvent){
   case EMsvEntriesCreated: 
      {
         //At least one new message has been created. We cannot
         //process them until they are completed, so we keep the id
         //for later. We only save ids from the inbox.
         const TMsvId folderId = *(static_cast<TMsvId*>(aArg2));
         if(folderId == KMsvGlobalInBoxIndexEntryId){
            class CMsvEntrySelection* selection = 
               static_cast<CMsvEntrySelection*>(aArg1);
            for(TInt i = 0; i < selection->Count(); ++i){
               User::LeaveIfError(iCreated.Append(selection->At(i)));
            }
         }
      }
      break;
   case EMsvEntriesChanged:
      {
         //At least one entry has changed. 
         class CMsvEntrySelection* selection = 
            static_cast<CMsvEntrySelection*>(aArg1);
         const TMsvId folder = *static_cast<TMsvId*>(aArg2);
         HandleMsvEntriesChangedL(*selection, folder);
      }
      break;
   case EMsvServerReady:
      //if the creation of the CMsvSession is asynchronous, this event
      //has to happen before we do anything else with the session.
      break;
   default:
      break;
   }
}   

TBool CWfSmsBase::ParseAndHandleMessageBodyL(const TDesC& aBody, TMsvId aId)
{
   TBuf<64> errorStr;
   //parse SMS body
   iSmsParser->ParseSmsL( aBody, errorStr );

   const TPoint destCoord = iSmsParser->GetDestinationCoordinate();
   const TPtrC destDescription = iSmsParser->GetDestinationDescription();
   switch( iSmsParser->GetSmsType() ){
   case CSmsParser::EWFSMSRoute:
      {
         const TPoint orgCoord = iSmsParser->GetOriginCoordinate();
         iHandler->HandleSmsL( orgCoord.iY, orgCoord.iX,
                               destCoord.iY, destCoord.iX,
                               destDescription, aId );
      }
      break;
   case CSmsParser::EWFSMSDestination:
      iHandler->HandleSmsL( destCoord.iY, destCoord.iX,
                            destDescription, aId );
      break;
   case CSmsParser::EWFSMSFavorite:
      iHandler->HandleSmsL( destCoord.iY, destCoord.iX,
                            destDescription, aId,
                            iSmsParser->GetFavoriteD() );
      break;
   default:
      ShowInfoL(_L("WF SMS not handled"));
      return EFalse;
   }
   return ETrue;
}

void CWfSmsBase::DeleteSmsL( TInt32 aMsvId )
{
   class CMsvEntry* wfEntry = iWfSmsSession->GetEntryL(aMsvId);
   CleanupStack::PushL(wfEntry);
   wfEntry->DeleteL(aMsvId);
   CleanupStack::PopAndDestroy(wfEntry);
}

CDesCArray* CWfSmsBase::GetDestArray()
{
   return iDestArray;
}

CDesCArray* CWfSmsBase::GetSenderArray()
{
   return iSenderArray;
}

const MDesCArray* CWfSmsBase::GetSignatures() const
{
   return iSignatures;
}

TPtrC CWfSmsBase::GetSignature(TMsvId aId) const
{
   const TInt pos = iMsgIDArray.Find(aId);
   if(pos >= 0 && pos < iSignatures->Count()){
      return iSignatures->MdcaPoint(pos);
   } else {
      return TPtrC(iSmsParser->GetSignature());
   }
}

TArray<TMsvId> CWfSmsBase::GetMsgIdArray()
{
   return iMsgIDArray.Array();
}

namespace{
   void ResetArrayL(CDesCArray*& aArray, TInt aGranularity)
   {
      delete aArray;
      aArray = NULL;
      aArray = new (ELeave) CDesCArraySeg(aGranularity);
   }
}

void CWfSmsBase::InitMsgArraysL()
{
   const TInt granularity = 4;
   ResetArrayL(iMsgArray, granularity);
   ResetArrayL(iDestArray, granularity);
   ResetArrayL(iSenderArray, granularity);
   ResetArrayL(iSignatures, granularity);
   iMsgIDArray.Reset();
}


TBool CWfSmsBase::GetAllSmsMsgL()
{
   InitMsgArraysL(); //clear arrays here, no need to keep the old stuff.
   TRACE_FUNC();
   TBool result = EFalse;
   
   // The Session is already created.
   // We need to construct An Entry Point in the  Tree KMsvDraftEntryId 
   class CMsvEntry* msvEntry = CMsvEntry::NewL( *iWfSmsSession,
                                                KMsvGlobalInBoxIndexEntryId,
                                                TMsvSelectionOrdering() );
   CleanupStack::PushL(msvEntry);
   
   class CMsvEntrySelection* msvSelection = msvEntry->ChildrenL();
   CleanupStack::PushL(msvSelection);

   const TInt numOfSms = msvSelection->Count();
   if(numOfSms > 0){
      TRACE_FUNC1( "numOfSms > 0" );
      TInt numWfSMS = 0;
      for( TInt i=0; i < numOfSms; i++ ){
         TRACE_FUNC_FMT1("sms#: %i", i );
         TMsvId messageMsvID = (*msvSelection)[i];
         if(VerifySms(*iWfSmsSession, messageMsvID)){
            if( iWfSmsMtm == NULL ){
               TRACE_FUNC1( "iWfSmsMtm == NULL" );
               ShowInfoL(_L("Sms Error"));
               continue;
            }
            TRAPD(lcode1,iWfSmsMtm->SwitchCurrentEntryL( messageMsvID ));
            if( lcode1 == KErrNotFound ){
               TRACE_FUNC1( "lcode1 == KErrNotFound" );
               ShowInfoL(_L("entry does not exist "));
               continue;
            } else if( lcode1 == KErrNoMemory ){
               TRACE_FUNC1( "lcode1 == KErrNoMemory" );
               ShowInfoL(_L("Insufficient memory"));
               continue;
            }
            TRAPD(l, iWfSmsMtm->LoadMessageL());
            if( l != KErrNone ){
               TRACE_FUNC1( "l != KErrNone" );
               continue;
            }


            const class CRichText& body = iWfSmsMtm->Body();
            if( iSmsParser->IsWayfinderSms( body ) ){
               TRACE_FUNC1("Is Wayfinder SMS");
               const TInt textLen = body.DocumentLength();
               HBufC* text = HBufC::NewLC(textLen);
               TPtr messageBody = text->Des();
               /// Message body of the last SMS parsed
               TBuf<64> errorStr;
               body.Extract( messageBody );
               iSmsParser->ParseSmsL( messageBody, errorStr );
               const class TMsvEntry& entry = iWfSmsMtm->Entry().Entry();

               CleanupPushAppendL(iMsgArray, messageBody );
               CleanupPushAppendL(iDestArray,
                                  iSmsParser->GetDestinationDescription() );
               CleanupPushAppendL(iSenderArray, entry.iDetails );
               CleanupPushAppendL(iSignatures, iSmsParser->GetSignature());
               User::LeaveIfError(iMsgIDArray.Append(messageMsvID));
               numWfSMS++;

               CleanupStack::Pop(4, iMsgArray);
               CleanupStack::PopAndDestroy(text);
            }   
         }   
      }
      if( numWfSMS > 0 ){
         result = ETrue;
      }
      iWfSmsMtm->RestoreServiceAndSettingsL();
   }
   CleanupStack::PopAndDestroy(2, msvEntry);// msvSelection, msvEntry
   return result;
}

TInt CWfSmsBase::IndexOf(TMsvId aId)
{
   return iMsgIDArray.Find(aId);
}

void CWfSmsBase::SetWfSmsBodyTextL( TInt aIdIdx )
{
   TMsvId messageID = iMsgIDArray[aIdIdx];
   ParseAndHandleMessageBodyL( iMsgArray->MdcaPoint( aIdIdx ), messageID);
}



TMsvId CWfSmsBase::CreateNewMessageL()
{
   TMsvEntry newEntry;              // This represents an entry in the Message Server index
   newEntry.iMtm = KUidMsgTypeSMS;                         // message type is SMS
   newEntry.iType = KUidMsvMessageEntry;                   // this defines the type of the entry: message 
   newEntry.iServiceId = KMsvLocalServiceIndexEntryId;     // ID of local service (containing the standard folders)
   newEntry.iDate.HomeTime();                              // set the date of the entry to home time
   newEntry.SetInPreparation(ETrue);                       // a flag that this message is in preparation

   // - CMsvEntry accesses and acts upon a particular Message Server entry.
   // - NewL() does not create a new entry, but simply a new object to access an existing entry.
   // - It takes in as parameters the client's message server session,
   //   ID of the entry to access and initial sorting order of the children of the entry. 
   //
   CMsvEntry* entry = CMsvEntry::NewL(*iWfSmsSession, KMsvGlobalOutBoxIndexEntryId ,TMsvSelectionOrdering());
   CleanupStack::PushL(entry);
   
   CMsvOperationWait* wait = CMsvOperationWait::NewLC();
   wait->Start();
   // We create a new entry asynchronously...
   CMsvOperation* oper = entry->CreateL(newEntry,wait->iStatus);
   CleanupStack::PushL(oper);
   CActiveScheduler::Start();
   // ...and keep track of the progress of the create operation.
   TMsvLocalOperationProgress progress = McliUtils::GetLocalProgressL(*oper);
   User::LeaveIfError(progress.iError);
   // Set our entry context to the created one
   entry->SetEntryL(progress.iId); // operation progress contains the ID of the ceated entry

   CleanupStack::PopAndDestroy(3); // entry,oper,wait

   return progress.iId;
}

void CWfSmsBase::SetEntryL(TMsvId aEntryId)
{
   // Get the server entry from our session
   CMsvEntry* entry = iWfSmsSession->GetEntryL(aEntryId);
   CleanupStack::PushL(entry);

   // Check if our mtm is different from the mtm set to our entry
   if (iMtm == NULL || entry->Entry().iMtm != (iMtm->Entry()).Entry().iMtm)
   {
      // If so, we delete the old... 
      delete iMtm;
      iMtm = NULL;
      // ...and get a new one from the MtmRegistry
      iMtm = iWfSmsMtmRegistry->NewMtmL(entry->Entry().iMtm);

      iMtm->SetCurrentEntryL(entry);
   }
   else
   {
      // if there is no need to change our mtm,
      // we only set our entry as current.
      iMtm->SetCurrentEntryL(entry);
   }
   
   CleanupStack::Pop(entry);
   entry = NULL;
}


void CWfSmsBase::SetScheduledSendingStateL(class CMsvEntrySelection* aSelection)
{
   class CBaseMtm* smsMtm = iMtm;
   // Add entry to task scheduler
   TBuf8<1> dummyParams;

   CMsvOperationWait* waiter = CMsvOperationWait::NewLC();
   waiter->Start();
   // invoking async schedule copy command on our mtm
   CMsvOperation* op= smsMtm->InvokeAsyncFunctionL( ESmsMtmCommandScheduleCopy,
                                                    *aSelection,
                                                    dummyParams,
                                                    waiter->iStatus);
   CleanupStack::PushL(op);
   CActiveScheduler::Start();
   CleanupStack::PopAndDestroy(2); // waiter and op
}

class CSendAs& CWfSmsBase::getSendAs()
{
   return *iSendAs;
}

TBool CWfSmsBase::CapabilityOK(TUid /*aCapabilty*/, TInt /*aResponse*/)
{
   return ETrue;
}


HBufC* CWfSmsBase::GetSenderLC(TMsvId aId)
{
   TInt pos = iMsgIDArray.Find(aId);
   if(pos >= 0 && pos < iSenderArray->MdcaCount()){
      return iSenderArray->MdcaPoint(pos).AllocLC();
   } else {
      class CMsvEntry* entry = iWfSmsSession->GetEntryL(aId);
      CleanupStack::PushL(entry);
      HBufC* sender = entry->Entry().iDetails.AllocL();
      CleanupStack::PopAndDestroy(entry);
      CleanupStack::PushL(sender);
      return sender;
   }
}

bool CWfSmsBase::SendSMSMessageL( const TDesC& recipientAddress, 
                                  const TDesC& /*recipientAlias*/,
                                  const TDesC& /*messageSubject*/,
                                  const TDesC& messageText )
{
   bool result = true;
  // From GDSMS
   iMsvId = CreateNewMessageL();
   
   SetEntryL(iMsvId);

   TMsvEntry msvEntry = (iMtm->Entry()).Entry();
   
   // We get the message body from Mtm and insert a bodytext
   CRichText& mtmBody = iMtm->Body();
   mtmBody.Reset();
   mtmBody.InsertL(0, messageText);

   // set iRecipient into the Details of the entry
   msvEntry.iDetails.Set(recipientAddress);  // set recipient info in details
   msvEntry.SetInPreparation(EFalse);         // set inPreparation to false
   
   msvEntry.SetSendingState(KMsvSendStateWaiting);   // set the sending state (immediately)
   msvEntry.iDate.HomeTime();                        // set time to Home Time

   // To handle the sms specifics we start using SmsMtm
   CSmsClientMtm* smsMtm = static_cast<CSmsClientMtm*>(iMtm);

   smsMtm->RestoreServiceAndSettingsL();
   
   // CSmsHeader encapsulates data specific for sms messages,
   // like service center number and options for sending.
   class CSmsHeader& header = smsMtm->SmsHeader();
   class CSmsSettings* sendOptions = CSmsSettings::NewL();
   CleanupStack::PushL(sendOptions);
   sendOptions->CopyL(smsMtm->ServiceSettings()); // restore existing settings
   
   // set send options
   sendOptions->SetDelivery(ESmsDeliveryImmediately);      // set to be delivered immediately
   header.SetSmsSettingsL(*sendOptions);
   
   // let's check if there's sc address
   if (header.Message().ServiceCenterAddress().Length() == 0){
      // no, there isn't. We assume there is at least one sc number set and use
      // the default SC number. 
      class CSmsSettings* serviceSettings = &(smsMtm->ServiceSettings());
      // if number of scaddresses in the list is null
      if (!serviceSettings->NumSCAddresses())
      {
         /* Since the showing of the failure message was already
            commented out, I also commented the reading of the
            message. This means that we do not need to know the
            resource in this common source code. */
         //TBuf<256> failMsg;
         //CEikonEnv::Static()->ReadResource( failMsg, R_CONNECT_SERVICE_NOT_DEFINED );
         // ShowConfirmDialogL(failMsg);
         result = false;
      }else{
         // set sc address to default. 
         class CSmsNumber* sc = 0;
         sc = &(serviceSettings->SCAddress(serviceSettings->DefaultSC()));
         header.Message().SetServiceCenterAddressL(sc->Address());
      }
   }

   CleanupStack::PopAndDestroy(sendOptions); // send options 
   
   // Add our recipient to the list, takes in two TDesCs, first is real address and second is an alias
   // works also without the alias parameter.
   smsMtm->AddAddresseeL(recipientAddress, msvEntry.iDetails);
   
   // Next we mark our message so later on we know which
   // message to delete from the Sent folder
   msvEntry.iMtmData3 = iMarkerUid.iUid; // use our app uid as an identifier
   
   // save message
   class CMsvEntry& entry = iMtm->Entry();
   entry.ChangeL(msvEntry);                // make sure that we are handling the right entry
   smsMtm->SaveMessageL();                 // closes the message

   TMsvId movedId = entry.Entry().Id();
   
   // We must create an entry selection for message copies (although now we only have one message in selection)
   class CMsvEntrySelection* selection = new (ELeave) CMsvEntrySelection;
   CleanupStack::PushL(selection);
   selection->AppendL(movedId);            // add our message to the selection

   SetScheduledSendingStateL(selection);   // schedule the sending with the active scheduler
   CleanupStack::PopAndDestroy(selection); // selection
   
   return result; // at this point the message has been sent, as long as there is a service center.
}

#endif
