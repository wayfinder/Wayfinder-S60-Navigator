/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FavoriteSyncManager.h"
#include "FavoriteSyncServiceProvider.h"
#include "GuiProt/Favorite.h"

CFavoriteSyncManager* 
CFavoriteSyncManager::NewLC(MFavoriteSyncManagerObserver& aObserver,
                            const TDesC& aLmsId)
{
   CFavoriteSyncManager* self = 
      new (ELeave) CFavoriteSyncManager(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL(aLmsId);
   return self;
}

CFavoriteSyncManager* 
CFavoriteSyncManager::NewL(MFavoriteSyncManagerObserver& aObserver,
                           const TDesC& aLmsId)
{
   CFavoriteSyncManager* self = 
      CFavoriteSyncManager::NewLC(aObserver, aLmsId);
   CleanupStack::Pop(self);
   return self;
}

CFavoriteSyncManager::~CFavoriteSyncManager()
{
   Cancel();
   delete iServiceProvider;
   iAddedIndexList.Reset();
   iUpdatedIndexList.Reset();
   iDeletedIndexList.Reset();
}

void CFavoriteSyncManager::ConstructL(const TDesC& aLmsId)
{
   CActiveScheduler::Add(this);
   iServiceProvider = CFavoriteSyncServiceProvider::NewL(*this, aLmsId);
}

CFavoriteSyncManager::CFavoriteSyncManager(MFavoriteSyncManagerObserver& aObserver) :
   CActive(EPriorityIdle), 
   iObserver(aObserver)
{
}

void CFavoriteSyncManager::DoSyncL(std::vector<Favorite*>* aSourceVec, 
                                   std::vector<Favorite*>* aFavVec, 
                                   TWFFavSyncListType aListType)
{
   // Call the asynchronous service provider that does the work,
   // when the provider is done it returns in RunL.
   iListType = aListType;
   if (aFavVec != NULL) {
      iFavList = aFavVec;
   }
   iServiceProvider->DoSyncL(aSourceVec, iFavList, iStatus);
   SetActive();
}

void CFavoriteSyncManager::DoNoWFLMSSyncL(std::vector<Favorite*>*& aServerFavVec, 
                                          std::vector<Favorite*>*& aFavVec)
{
   std::vector<Favorite*>* tmpVec = aFavVec;
   aFavVec = aServerFavVec;
   aServerFavVec = tmpVec;
   
   for (TUint i = 0; i < aFavVec->size(); ++i) {
      AddToAddList(i, EWFLmsList);
   }
   iObserver.SyncDoneL(0);
}

void CFavoriteSyncManager::RunL()
{
   TInt status = iStatus.Int();

   switch (status) {
   case KErrCancel:
      iObserver.SyncError(0);
      break;
   case KErrArgument:
      iObserver.SyncError(0);
      break;
   case KErrNone:
      iObserver.SyncDoneL(0);
      break;
   }
}

void CFavoriteSyncManager::DoCancel()
{
   iServiceProvider->Cancel();
}

RArray<TWFFavSyncListIndex>& CFavoriteSyncManager::GetAddList()
{
   return iAddedIndexList;
}

RArray<TWFFavSyncListIndex>& CFavoriteSyncManager::GetDeleteList()
{
   return iDeletedIndexList;
}

RArray<TWFFavSyncListIndex>& CFavoriteSyncManager::GetUpdateList()
{
   return iUpdatedIndexList;
}

void CFavoriteSyncManager::AddToDeleteList(TInt aIndex)
{
   iDeletedIndexList.AppendL(TWFFavSyncListIndex(iListType, aIndex));
}

void CFavoriteSyncManager::AddToUpdateList(TInt aIndex)
{
   iUpdatedIndexList.AppendL(TWFFavSyncListIndex(iListType, aIndex));
}

void CFavoriteSyncManager::AddToAddList(TInt aIndex)
{
   iAddedIndexList.AppendL(TWFFavSyncListIndex(iListType, aIndex));
   if (iListType == EWFLmsList) {
      // If we're currently working with the lms list we also want to 
      // add this index to the update server list since this favorite will 
      // need to be updated on the server as well.
      iUpdatedIndexList.AppendL(TWFFavSyncListIndex(EWFServerList, aIndex));
   }
}

void CFavoriteSyncManager::AddToAddList(TInt aIndex, TWFFavSyncListType aListType)
{
   iAddedIndexList.AppendL(TWFFavSyncListIndex(aListType, aIndex));
   if (aListType == EWFLmsList) {
      // If we're currently working with the lms list we also want to 
      // add this index to the update server list since this favorite will 
      // need to be updated on the server as well.
      iUpdatedIndexList.AppendL(TWFFavSyncListIndex(EWFServerList, aIndex));
   }
}

TBool CFavoriteSyncManager::IsInUpdateList(TInt aIndex)
{
   if (iListType == EWFLmsList) {
      // If we're currently working with the lms list we want to see if the 
      // index exists in the server list.
      TWFFavSyncListIndex tmp(EWFServerList, aIndex);
      return (FindInListL(iUpdatedIndexList, tmp) != KErrNotFound);
   } else {
      // Else we're working on the server list and then we want to see if the 
      // index exists in the lms list.
      TWFFavSyncListIndex tmp(EWFLmsList, aIndex);
      return (FindInListL(iUpdatedIndexList, tmp) != KErrNotFound);
   }
}

TBool CFavoriteSyncManager::IsInAddList(TInt aIndex)
{
   if (iListType == EWFLmsList) {
      // If we're currently working with the lms list we want to see if the 
      // index exists in the server list.
      TWFFavSyncListIndex tmp(EWFServerList, aIndex);
      return (FindInListL(iAddedIndexList, tmp) != KErrNotFound);
   } else {
      // Else we're working on the server list and then we want to see if the 
      // index exists in the lms list.
      TWFFavSyncListIndex tmp(EWFLmsList, aIndex);
      return (FindInListL(iAddedIndexList, tmp) != KErrNotFound);
   } 
}

TBool CFavoriteSyncManager::IsInDeleteList(TInt aIndex, TWFFavSyncListType aListType)
{
   TWFFavSyncListIndex tmp(aListType, aIndex);
   return (FindInListL(iDeletedIndexList, tmp) != KErrNotFound);
}

void CFavoriteSyncManager::RemoveFromUpdateList(TInt aIndex)
{
   TWFFavSyncListType listType = EWFLmsList;
   if (iListType == EWFLmsList) {
      // If we're currently working with the lms list we want to see if the 
      // index exists in the server list.
      listType = EWFServerList;
    } 
   TWFFavSyncListIndex tmp(listType, aIndex);
   TInt index = FindInListL(iUpdatedIndexList, tmp);
   if (index != KErrNotFound) {
      iUpdatedIndexList.Remove(index);
   }
}

void CFavoriteSyncManager::SyncError(TInt aError)
{
}

void CFavoriteSyncManager::SyncComplete()
{
}

TInt CFavoriteSyncManager::FindInListL(const RArray<TWFFavSyncListIndex>& aList, 
                                       const TWFFavSyncListIndex& aItem)
{
   for (TInt i = 0; i < aList.Count(); ++i) {
      if (aList[i] == aItem) {
         return i;
      }
   }
   return KErrNotFound;
}
