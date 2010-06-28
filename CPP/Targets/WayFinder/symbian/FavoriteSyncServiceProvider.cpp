/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FavoriteSyncServiceProvider.h"
#include "GuiProt/Favorite.h"
#include "WFTextUtil.h"

CFavoriteSyncServiceProvider* 
CFavoriteSyncServiceProvider::NewLC(MFavoriteSyncObserver& aObserver,
                                    const TDesC& aLmsId)
{
   CFavoriteSyncServiceProvider* self = 
      new (ELeave) CFavoriteSyncServiceProvider(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL(aLmsId);
   return self;
}

CFavoriteSyncServiceProvider* 
CFavoriteSyncServiceProvider::NewL(MFavoriteSyncObserver& aObserver,
                                   const TDesC& aLmsId)
{
   CFavoriteSyncServiceProvider* self = 
      CFavoriteSyncServiceProvider::NewLC(aObserver, aLmsId);
   CleanupStack::Pop(self);
   return self;
}

CFavoriteSyncServiceProvider::~CFavoriteSyncServiceProvider()
{
   Cancel();
   delete[] iLmsId;
}

void CFavoriteSyncServiceProvider::ConstructL(const TDesC& aLmsId)
{
   iLmsId = WFTextUtil::newTDesDupL(aLmsId);
   CActiveScheduler::Add(this);
}

CFavoriteSyncServiceProvider::CFavoriteSyncServiceProvider(MFavoriteSyncObserver& aObserver) : 
   CActive(EPriorityIdle), 
   iObserver(aObserver)
{
}

void CFavoriteSyncServiceProvider::DoSyncL(std::vector<Favorite*>* aSourceVec, 
                                           std::vector<Favorite*>* aDestVec,
                                           TRequestStatus& aStatus)
{
   // Reset all the values and complete to enter RunL and do one iteration.
   iCallerStatus = &aStatus;
   *iCallerStatus = KRequestPending;
   iCurrentIndex = 0;
   iSourceList = aSourceVec;
   iFavList = aDestVec;
   iState = EWFFavSyncCompareFavNext;
   if (iSourceList == NULL || iFavList == NULL) {
      iState = EWFFavSyncError;
   }
   CompleteRequest(KErrNone);
}

void CFavoriteSyncServiceProvider::CompleteRequest(TInt aStatus)
{
   TRequestStatus* status = &iStatus; 
   iStatus = KRequestPending;
   User::RequestComplete(status, aStatus);
   SetActive();
}

void CFavoriteSyncServiceProvider::RunL()
{
   TInt status = iStatus.Int();

   switch (status) {
   case KErrCancel:
      iObserver.SyncError(0);
      break;
   case KErrNone:
      {
         switch (iState) {
         case EWFFavSyncCompareFavNext:
            // Compare the next favorite.
            CompareFavoriteL();
            CompleteRequest(KErrNone);
            break;
         case EWFFavSyncCompareFavDone:
            // We have compared all the favorites now check for deleted ones.
            FindDeletedFavsL();
            CompleteRequest(KErrNone);
            break;
         case EWFFavSyncComplete:
            // We have compared all favorites, let caller know we are done.
            iObserver.SyncComplete();
            User::RequestComplete(iCallerStatus, KErrNone);
            break;
         case EWFFavSyncError:
            iObserver.SyncError(0);
            User::RequestComplete(iCallerStatus, KErrArgument);
            break;
         }
      }
   }
}

void CFavoriteSyncServiceProvider::DoCancel()
{
   if (iCallerStatus) {
      User::RequestComplete(iCallerStatus, KErrCancel);
   }
}

void CFavoriteSyncServiceProvider::CompareFavoriteL()
{
   // Do the crc calc and syncing of one favorite here.
   if (iSourceList->empty()) {
      // If source list is empty we have nothing to sync 
      // or add (only maybe delete).
      iState = EWFFavSyncCompareFavDone;
      return;
   }
   Favorite* sourceFav = iSourceList->at(iCurrentIndex);
   std::vector<Favorite*>::iterator favit = iFavList->begin();
   TInt i = 0;
   while (favit != iFavList->end()) {
      // Go through the list of favorites to find the one with the current 
      // lm id.
      if ((sourceFav->getLmID() == (*favit)->getLmID()) && sourceFav->getLmID() != MAX_UINT32) {
         if (!sourceFav->isMd5sumSet()) {
            // If no md5sum is calculated do it now.
            sourceFav->setMd5sum(NULL);
         }
         if ((memcmp(sourceFav->getMd5sum(), (*favit)->getMd5sum(),
                    Favorite::MAX_SIZE_MD5SUM) != 0)) {
            if (!iObserver.IsInUpdateList(i)) {
               // If the md5sum of the favorite is different and the favorite 
               // does not already exist in the other update list we update it
               // and add it to the update list.
               TUint32 favid = (*favit)->getID();
               delete (*favit);
               (*favit) = new Favorite(*sourceFav);
               if ((*favit)->getID() == Favorite::INVALID_FAV_ID) {
                  (*favit)->setID(favid);
               }
               if (strcmp((*favit)->getLmsID(), "") == 0) {
                  (*favit)->setLmsID(iLmsId);
               }
               iObserver.AddToUpdateList(i);
            } else {
               // If the favorite has been changed in both the lms and the server we need the newly
               // created server id set on the lms favorite since otherwise the server does not 
               // understand which favorite has been changed.
               (*favit)->setID(sourceFav->getID());
            }
         }
         break;
      }
      ++favit;
      ++i;
   }
   if (favit == iFavList->end()) {
      // We didn't find any favorite with the same lm id so we add.
      Favorite* fav = new Favorite(*sourceFav);
      if (strcmp(fav->getLmsID(), "") == 0) {
         fav->setLmsID(iLmsId);
      }
      iFavList->push_back(fav);
      iObserver.AddToAddList(iFavList->size() - 1);
   }
   iCurrentIndex++;
   if (iCurrentIndex >= iSourceList->size()) {
      // If we have gone through all favorites in the source list we're done.
      iState = EWFFavSyncCompareFavDone;
   } else {
      // Otherwise we trigger the syncing of the next favorite.
      iState = EWFFavSyncCompareFavNext;
   }
}

void CFavoriteSyncServiceProvider::FindDeletedFavsL()
{
   std::vector<Favorite*>::iterator favit;
   TInt i = 0;
   for (favit = iFavList->begin(); favit != iFavList->end(); ++favit, ++i) {
      // Go through the favorite list and see if we have more entrys in 
      // this list than in the source list, which would mean that they have 
      // been deleted from the source list.
      std::vector<Favorite*>::const_iterator findit = 
         find_if(iSourceList->begin(), iSourceList->end(),
                 std::bind2nd(FavLmIDCmp(), (*favit)));
      if (findit == iSourceList->end()) { 
         if (!iObserver.IsInAddList(i)) {
            // The favorite was not found in the source list and it was not 
            // tagged as added in the other list (other meaning server list if 
            // we're currently working on the lms list) 
            // so add it to the delete list.
            if (strcmp((*favit)->getLmsID(), "") == 0) {
               (*favit)->setLmsID(iLmsId);
            }
            iObserver.AddToDeleteList(i);
         } 
         if (iObserver.IsInUpdateList(i)) {
            // The favorite is marked as to be updated but it was deleted
            // in other plase (e.g server) and delete should over rule update.
            if (strcmp((*favit)->getLmsID(), "") == 0) {
               (*favit)->setLmsID(iLmsId);
            }
            //iObserver.AddToDeleteList(i);
            iObserver.RemoveFromUpdateList(i);
         }
      }
   }
   iState = EWFFavSyncComplete;
}
