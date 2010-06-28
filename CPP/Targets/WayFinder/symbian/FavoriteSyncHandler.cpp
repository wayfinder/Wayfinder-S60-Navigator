/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FavoriteSyncHandler.h"
#include "WFLMSManagerBase.h"
#include "FavoriteSyncManager.h"
#include "WFTextUtil.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "DeleteHelpers.h"

CFavoriteSyncHandler*
CFavoriteSyncHandler::NewLC(MFavoriteSyncHandlerObserver& aObserver, 
                            GuiProtMessageSender& aSender,
                            const TDesC& aLmsId, 
                            const char* aImeiCrcHex)
{
   CFavoriteSyncHandler* self = 
      new (ELeave) CFavoriteSyncHandler(aObserver, aSender, aImeiCrcHex);
   CleanupStack::PushL(self);
   self->ConstructL(aLmsId);
   return self;
}

CFavoriteSyncHandler*
CFavoriteSyncHandler::NewL(MFavoriteSyncHandlerObserver& aObserver, 
                           GuiProtMessageSender& aSender,
                           const TDesC& aLmsId,
                           const char* aImeiCrcHex)
{
   CFavoriteSyncHandler* self = 
      CFavoriteSyncHandler::NewLC(aObserver, aSender, aLmsId, aImeiCrcHex);
   CleanupStack::Pop(self);
   return self;
}

CFavoriteSyncHandler::~CFavoriteSyncHandler()
{
   delete iLMSManager;
   delete iSyncManager;
   delete iEventGenerator;
   delete iLmsId;
   if (iFavoriteList) {
      STLUtility::deleteValues(*iFavoriteList);
      delete iFavoriteList;
   }
   if (iServerFavList) {
      STLUtility::deleteValues(*iServerFavList);
      delete iServerFavList;
   }
}

void CFavoriteSyncHandler::ConstructL(const TDesC& aLmsId)
{
   iLmsId = aLmsId.AllocL();
   iSyncManager = CFavoriteSyncManager::NewL(*this, *iLmsId);
}

CFavoriteSyncHandler::CFavoriteSyncHandler(MFavoriteSyncHandlerObserver& aObserver, 
                                           GuiProtMessageSender& aSender,
                                           const char* aImeiCrcHex) : 
   iObserver(aObserver),
   iSender(aSender),
   iImeiCrcHex(aImeiCrcHex)
{
}

void CFavoriteSyncHandler::SetLMSManager(MWFLMSManagerBase* aLMSManager)
{
   if (iLMSManager) {
      delete iLMSManager;
   }
   iLMSManager = aLMSManager;
}

void CFavoriteSyncHandler::GenerateEvent(enum TWFFavSyncManEvent aEvent)
{
   if (!iEventGenerator) {
      iEventGenerator = CWFSyncManEventGenerator::NewL(*this); 
   }
   iEventGenerator->SendEventL(aEvent);
}

void CFavoriteSyncHandler::HandleGeneratedEventL(enum TWFFavSyncManEvent aEvent)
{
   switch (aEvent) {
   case EWFGetLmsFavs:
      iLMSManager->GetLandmarksAsyncL(MWFLMSManagerBase::EWFLMSWFFavs);
      break;
   case EWFGetServerFavs:
   case EWFGetClientFavs:
      {
         class isab::GenericGuiMess 
            ggm(isab::GuiProtEnums::GET_FAVORITES_ALL_DATA, uint16(0),
                uint16(MAX_UINT16));
         // Send a message with this as receiver to end up in 
         // GuiProtReceiveMessage function below.
         iSender.SendMessage(ggm, this);
      }
      break;
   case EWFUpdateNav2ToServer:
      // Use fall-through to send a sync_favorites message.
      iState |= EWFWaitingForNav2ToServerUpdate;
   case EWFRequestServerFavsSync:
      {
         iState |= EWFWaitingForFavSync;
         class isab::GenericGuiMess 
            ggm(isab::GuiProtEnums::SYNC_FAVORITES);
         // Send a message with this as receiver to end up in 
         // GuiProtReceiveMessage function below.
         iSender.SendMessage(ggm, this);
      }
      break;
   case EWFDoLmsSync:
      {
         std::vector<Favorite*>* favVec = iLMSManager->GetLandmarkList();
         if (favVec) {
            iSyncManager->DoSyncL(favVec, iFavoriteList, EWFServerList);
         } 
      }
      break;
   case EWFDoServerSync:
      iSyncManager->DoSyncL(iServerFavList, iFavoriteList, EWFLmsList);
      break;
   case EWFDoNoWFLMSSync:
      iSyncManager->DoNoWFLMSSyncL(iServerFavList, iFavoriteList);
      break;
   case EWFAddToLms:
      AddToLmsL();
      break;
   case EWFUpdateToLms:
      UpdateToLmsL();
      break;
   case EWFDeleteFromLms:
      DeleteFromLmsL();
      break;
   case EWFAddToServer:
      AddToServerL();
      break;
   case EWFUpdateToServer:
      UpdateToServerL();
      break;
   case EWFDeleteFromServer:
      DeleteFromServerL();
      break;
   case EWFCheckEmptyLms:
      CheckEmptyLmsL();
      break;
   case EWFSyncCompelete:
      iState &= ~EWFSyncRunning;
      iObserver.FavSyncCompleteL();
      break;
   }
}

void CFavoriteSyncHandler::ProcessNextState()
{
   if (iState & EWFEmptyLmsCheckDone) {
      iState &= ~EWFEmptyLmsCheckDone;
      GenerateEvent(EWFSyncCompelete);
      return;
   }
   if (iState & EWFUpdateToLmsDone) {
      iState &= ~EWFUpdateToLmsDone;
      GenerateEvent(EWFCheckEmptyLms);
      return;
   }
   if (iState & EWFDeleteFromLmsDone) {
      iState &= ~EWFDeleteFromLmsDone;
      GenerateEvent(EWFUpdateToLms);
      return;
   }
   if (iState & EWFGotNav2ToServerUpdate) {
      iState &= ~EWFGotNav2ToServerUpdate;
      GenerateEvent(EWFDeleteFromLms);
      return;
   }
   if ((iState & EWFAddToServerDone) && (iState & EWFDeleteFromServerDone) 
       && (iState & EWFUpdateToServerDone) && (iState & EWFServerQueueDone)) {
      iState &= ~EWFAddToServerDone;
      iState &= ~EWFDeleteFromServerDone; 
      iState &= ~EWFUpdateToServerDone;
      iState &= ~EWFServerQueueDone;
      GenerateEvent(EWFUpdateNav2ToServer);
      return;
   }
   if (iState & EWFAddToLmsDone) {
      iState &= ~EWFAddToLmsDone;
      GenerateEvent(EWFUpdateToServer);
      GenerateEvent(EWFDeleteFromServer);
      GenerateEvent(EWFAddToServer);
      return;
   }
   if (iState & EWFServerSyncDone) {
      iState &= ~EWFServerSyncDone;
      GenerateEvent(EWFAddToLms);
      return;
   }
   if (iState & EWFGotServerFavs && iState & EWFNoWFLMStore) {
      iState &= ~EWFGotServerFavs;
      iState &= ~EWFNoWFLMStore;
      GenerateEvent(EWFDoNoWFLMSSync);
      return;
   }
   if (iState & EWFGotServerFavs) {
      iState &= ~EWFGotServerFavs;
      GenerateEvent(EWFDoServerSync);
      return;
   }
   if (iState & EWFGotServerFavsSynced) {
      iState &= ~EWFGotServerFavsSynced;
      iState |= EWFWaitingForServerFavs;
      GenerateEvent(EWFGetServerFavs);
      return;
   }
   if (((iState & EWFNoWFLMStore) && (iState & EWFGotClientFavs)) || (iState & EWFLmsSyncDone)) {
      iState &= ~EWFLmsSyncDone;
      iState &= ~EWFGotClientFavs;
      GenerateEvent(EWFRequestServerFavsSync);
      return;
   }
   if ((iState & EWFGotLmsFavs) && (iState & EWFGotClientFavs)) {
      iState &= ~EWFGotLmsFavs;
      iState &= ~EWFGotClientFavs;
      iState |= EWFSyncingLms;
      GenerateEvent(EWFDoLmsSync);
      return;
   }
   if (iState & EWFFavSyncStart) {
      iState &= ~EWFFavSyncStart;
      GenerateEvent(EWFGetLmsFavs);
      GenerateEvent(EWFGetClientFavs);
      return;
   }
}

void CFavoriteSyncHandler::DoSyncL()
{
   if (SyncIsRunning()) {
      return;
   }
   iState = EWFFavSyncStart | EWFSyncRunning;
   ProcessNextState();
}

bool CFavoriteSyncHandler::GuiProtReceiveMessage(isab::GuiProtMess* mess)
{
   switch (mess->getMessageType()) {
   case GuiProtEnums::REQUEST_FAILED:
      if (IsWaitingForFavSync()) {
         // Waiting for a favorite sync from the server.
         iState &= ~EWFWaitingForFavSync;
         iState |= EWFUpdateToLmsDone;
      } else if (IsWaitingForServerFavReplies()) {
         // If one request failed we will continue with the other requests
         // and remvoe this message id from the list. We will do the same check
         // here as in FAVORITES_CHANGED which means that if this was the last
         // message in the queue we will continue with the next step.
         // This could be even more correct trying to resend the message one 
         // more time or similar fail safe. This requires that we store the 
         // message and not only the message id.
         TInt index = iServerMessageQueue.FindInOrder(mess->getMessageID());
         if (index != KErrNotFound) {
            iServerMessageQueue.Remove(index);
         }
         if (iServerMessageQueue.Count() == 0) {
            iState &= ~EWFWaitingForServerFavReplies;
            iState |= EWFServerQueueDone;
         }
      }
      break;
   case GuiProtEnums::GET_FAVORITES_ALL_DATA_REPLY:
      {
         isab::GetFavoritesAllDataReplyMess* favmess =
            static_cast<isab::GetFavoritesAllDataReplyMess*>(mess);
         std::vector<Favorite*>* tmpFavList = favmess->getFavorites();
         if (iState & EWFWaitingForServerFavs) {
            // We were waiting for the synced server favs.
            iState &= ~EWFWaitingForServerFavs;
            iServerFavList = tmpFavList;
            iState |= EWFGotServerFavs;
         } else {
            // Else we were waiting for the local client favs.
            iFavoriteList = tmpFavList;
            iState |= EWFGotClientFavs;
         }
         delete favmess;
      }
      break;
   case GuiProtEnums::FAVORITES_CHANGED:
      if (IsWaitingForServerFavReplies()) {
         TInt index = iServerMessageQueue.FindInOrder(mess->getMessageID());
         if (index != KErrNotFound) {
            iServerMessageQueue.Remove(index);
         }
         if (iServerMessageQueue.Count() == 0) {
            iState &= ~EWFWaitingForServerFavReplies;
            iState |= EWFServerQueueDone;
         }
      }
      break;
   case GuiProtEnums::SYNC_FAVORITES_REPLY:
      // Got the notification that the favorites have been synced.
      if (IsWaitingForFavSync()) {
         iState &= ~EWFWaitingForFavSync;
         if (iState & EWFWaitingForNav2ToServerUpdate) {
            iState &= ~EWFWaitingForNav2ToServerUpdate;
            iState |= EWFGotNav2ToServerUpdate;
         } else {
            iState |= EWFGotServerFavsSynced;
         }
      }
      // Got reply from a add/change/remove favorite message.
      if (IsWaitingForServerFavReplies()) {
         TInt index = iServerMessageQueue.FindInOrder(mess->getMessageID());
         if (index != KErrNotFound) {
            iServerMessageQueue.Remove(index);
         }
         if (iServerMessageQueue.Count() == 0) {
            iState &= ~EWFWaitingForServerFavReplies;
            iState |= EWFServerQueueDone;
         }
      }
      break;
   default:
      return false;
   }
   ProcessNextState();
   return true;
}

void CFavoriteSyncHandler::FavoriteComparedL(TInt aNbrCompared, 
                                             TInt aTotalNbrToCompare)
{
}

void CFavoriteSyncHandler::SyncDoneL(TInt aNbrFavoritesSynced)
{
   if (iState & EWFSyncingLms) {
      iState &= ~EWFSyncingLms;
      iState |= EWFLmsSyncDone;
   } else {
      iState |= EWFServerSyncDone;
   }
   ProcessNextState();
}

void CFavoriteSyncHandler::SyncError(TInt aError)
{
}

void CFavoriteSyncHandler::DatabaseInitialized()
{
   
}

void CFavoriteSyncHandler::LandmarkImportCompleteL()
{
   if (iLMSManager->LandmarkStoreExistsL(*iLmsId)) {
      iState |= EWFGotLmsFavs;
   } else {
      // The Wayfinder_uid LMS did not exist so we should sync with 
      // server and then just copy all server favorites into the LMS.
      iLMSManager->OpenLandmarkStoreL(*iLmsId, ETrue);

      iState |= EWFNoWFLMStore; 
   }
   ProcessNextState();
}

void CFavoriteSyncHandler::GuiLandmarkImportCompleteL()
{
}

void CFavoriteSyncHandler::LandmarkImportedL(TInt aLmsNbrImported, 
                                             TInt aTotalNbrLmsToImport,
                                             TInt aLmNbrImported, 
                                             TInt aTotalNbrLmToImport)
{
}

void CFavoriteSyncHandler::LmsError(TInt aError)
{
}

TBool CFavoriteSyncHandler::SyncIsRunning()
{
   return ((iState & EWFSyncRunning) || (iEventGenerator && iEventGenerator->IsActive()));
}

TBool CFavoriteSyncHandler::IsWaitingForFavSync()
{
   return (iState & EWFWaitingForFavSync);
}

TBool CFavoriteSyncHandler::IsWaitingForServerFavReplies()
{
   return (iState & EWFWaitingForServerFavReplies);
}

void CFavoriteSyncHandler::AddToLmsL()
{
   RArray<TWFFavSyncListIndex>& indexList = iSyncManager->GetAddList();
   for (TInt i = 0; i < indexList.Count(); ++i) {
      TWFFavSyncListIndex listEntry = indexList[i];
      if (listEntry.type == EWFLmsList) {
         Favorite* fav = iFavoriteList->at(listEntry.index);
         fav->setLmID(iLMSManager->AddLandmarkL(*fav), iImeiCrcHex);
         char* lmsId = WFTextUtil::newTDesDupL(*iLmsId);
         fav->setLmsID(lmsId);
         delete[] lmsId;
      }
   }
   iState |= EWFAddToLmsDone;
   ProcessNextState();
}

void CFavoriteSyncHandler::UpdateToLmsL()
{
   RArray<TWFFavSyncListIndex>& indexList = iSyncManager->GetUpdateList();
   for (TInt i = 0; i < indexList.Count(); ++i) {
      TWFFavSyncListIndex listEntry = indexList[i];
      if (listEntry.type == EWFLmsList) {
         iLMSManager->UpdateLandmarkL(*iFavoriteList->at(listEntry.index));
      }
   }
   iState |= EWFUpdateToLmsDone;
   ProcessNextState();
}

void CFavoriteSyncHandler::DeleteFromLmsL()
{
   RArray<TWFFavSyncListIndex>& indexList = iSyncManager->GetDeleteList();
   for (TInt i = 0; i < indexList.Count(); ++i) {
      TWFFavSyncListIndex listEntry = indexList[i];
      if (listEntry.type == EWFLmsList) {
         if (!iSyncManager->IsInDeleteList(listEntry.index, EWFServerList)) {
            iLMSManager->DeleteLandmarkL(*iFavoriteList->at(listEntry.index));
         }
      }
   }
   iState |= EWFDeleteFromLmsDone;
   ProcessNextState();
}

void CFavoriteSyncHandler::AddToServerL()
{
   RArray<TWFFavSyncListIndex>& indexList = iSyncManager->GetAddList();
   for (TInt i = 0; i < indexList.Count(); ++i) {
      TWFFavSyncListIndex listEntry = indexList[i];
      if (listEntry.type == EWFServerList) {
         class isab::AddFavoriteMess 
            afm(iFavoriteList->at(listEntry.index));
         // Send a message with this as receiver to end up in 
         // GuiProtReceiveMessage function below. Also add the message id 
         // to our array to make sure that we can control that all messages 
         // have returned to us.
         iServerMessageQueue.InsertInOrder(iSender.SendMessage(afm, this));
      }
   }
   iState |= EWFAddToServerDone;
   iState |= EWFWaitingForServerFavReplies;
   if ((iState & EWFAddToServerDone) && (iState & EWFDeleteFromServerDone) && 
       (iState & EWFUpdateToServerDone) && (iServerMessageQueue.Count() == 0)) {
      iState &= ~EWFWaitingForServerFavReplies;
      iState |= EWFServerQueueDone;
   }
   ProcessNextState();
}

void CFavoriteSyncHandler::UpdateToServerL()
{
   RArray<TWFFavSyncListIndex>& indexList = iSyncManager->GetUpdateList();
   for (TInt i = 0; i < indexList.Count(); ++i) {
      TWFFavSyncListIndex listEntry = indexList[i];
      if (listEntry.type == EWFServerList) {
         Favorite* fav = iFavoriteList->at(listEntry.index);
         fav->setSynced(EFalse);
         class isab::ChangeFavoriteMess cfm(fav);
         // Send a message with this as receiver to end up in 
         // GuiProtReceiveMessage function below. Also add the message id 
         // to our array to make sure that we can control that all messages 
         // have returned to us.
         iServerMessageQueue.InsertInOrder(iSender.SendMessage(cfm, this));
      }
   }
   iState |= EWFUpdateToServerDone;
   iState |= EWFWaitingForServerFavReplies;
   if ((iState & EWFAddToServerDone) && (iState & EWFDeleteFromServerDone) && 
       (iState & EWFUpdateToServerDone) && (iServerMessageQueue.Count() == 0)) {
      iState &= ~EWFWaitingForServerFavReplies;
      iState |= EWFServerQueueDone;
   }
   ProcessNextState(); 
}

void CFavoriteSyncHandler::DeleteFromServerL()
{
   RArray<TWFFavSyncListIndex>& indexList = iSyncManager->GetDeleteList();
   for (TInt i = 0; i < indexList.Count(); ++i) {
      TWFFavSyncListIndex listEntry = indexList[i];
      if (listEntry.type == EWFServerList) {
         if (!iSyncManager->IsInDeleteList(listEntry.index, EWFLmsList)) {
            class isab::GenericGuiMess ggm(GuiProtEnums::REMOVE_FAVORITE, 
                                           iFavoriteList->at(listEntry.index)->getID());
            // Send a message with this as receiver to end up in 
            // GuiProtReceiveMessage function below. Also add the message id 
            // to our array to make sure that we can control that all messages 
            // have returned to us.
            iServerMessageQueue.InsertInOrder(iSender.SendMessage(ggm, this));
         }
      }
   }
   iState |= EWFDeleteFromServerDone;
   iState |= EWFWaitingForServerFavReplies;
   if ((iState & EWFAddToServerDone) && (iState & EWFDeleteFromServerDone) && 
       (iState & EWFUpdateToServerDone) && (iServerMessageQueue.Count() == 0)) {
      iState &= ~EWFWaitingForServerFavReplies;
      iState |= EWFServerQueueDone;
   }
   ProcessNextState(); 
}

void CFavoriteSyncHandler::CheckEmptyLmsL()
{
   if (iLMSManager->LandmarkStoreEmptyL(*iLmsId)) {
      iLMSManager->DeleteLandmarkStoreL(*iLmsId);
   }
   iState |= EWFEmptyLmsCheckDone;
   ProcessNextState();
}

