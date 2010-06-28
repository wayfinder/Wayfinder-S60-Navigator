/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef C_FAVORITE_SYNC_MANAGER_H
#define C_FAVORITE_SYNC_MANAGER_H

#include "arch.h"
#include "FavoriteSyncObserver.h"
#include "FavoriteSyncServiceProvider.h"
#include "FavoriteSyncManagerObserver.h"
#include <vector>

namespace isab {
   class Favorite;
}

using namespace isab;

class CFavoriteSyncManager : public CActive,
                             public MFavoriteSyncObserver
{
public:
   /**
    * Standard NewLC function.
    *
    * @param aObserver   The observer that receives status updates.
    * @param aLmsId      The id of the wayfinder LMS.
    * @return   The newly created instance of CFavoriteSyncManager.
    */
   static CFavoriteSyncManager* NewLC(MFavoriteSyncManagerObserver& aObserver,
                                      const TDesC& aLmsId);
   
   /**
    * Standard NewL function.
    *
    * @param aObserver   The observer that receives status updates.
    * @param aLmsId      The id of the wayfinder LMS.
    * @return   The newly created instance of CFavoriteSyncManager.
    */
   static CFavoriteSyncManager* NewL(MFavoriteSyncManagerObserver& aObserver,
                                     const TDesC& aLmsId);

   /**
    * Destructor
    */
   virtual ~CFavoriteSyncManager();

   /**
    * aSourceVec contains the favorites to be compared against aFavVec. The aFavVec
    * will only be used to compare against the aSourceVec and to keep intermediate
    * versions of client favorites.
    * 
    * First normal operations is to feed the DoSyncL with lms favorites as first
    * argument and client favorites as second argument and then feed it with server
    * favorites as first arguments and once again the now updated client favorites as
    * second argument.
    * 
    * If client favorites are empty we should first sync the server favorites against
    * the client favorites and then lms favorites against client favorites.
    * 
    * If there is a difference between a favorite in aSourceVec compared to the
    * favorite in aFavVec, the favorite in aFavVec will be updated with the info, crc
    * and the index of the favorite in aFavVec will be stored in iUpdatedIndexList.
    * 
    * If there is a Favorite in aSourceVec that doesn't exist in aFavVec, the
    * favorite will be appended to aFavVec and the index of the favorite in aFavVec
    * will be stored in iAddedIndexList.
    * 
    * If there is a favorite in aFavVec that doesn't exist in aSourceVec
    * (set_difference()) this means that the favorite has been deleted in the lms and
    * the index from aFavVec should be stored in iDeletedIndexList.
    * 
    * When above tasks is done for the lms and client vector, the observer should be
    * notified that the vectors has been compared. The observer can then decide if it
    * wants the sync manager to handle the update, add and delete of favorites or if
    * it should sync server client vectors.
    * 
    * If requested the three lists should be iterated and favorites should be updated
    * (add/remove), added and removed according to the indexes in the lists.
    */
   void DoSyncL(std::vector<Favorite*>* aSourceVec, 
                std::vector<Favorite*>* aFavVec, 
                TWFFavSyncListType aSyncType);

   /**
    * Called when the wf lms doesnt exist at all. What it does is to
    * create the wf lms and add all server favs to the lms. It then
    * updates all the favorites to server with a lm id.
    *
    * @param aServerFavVec, a vector containing the server favorites.
    * @param aFavVec, a vector containing the client favorites.
    */
   void DoNoWFLMSSyncL(std::vector<Favorite*>*& aServerFavVec, 
                       std::vector<Favorite*>*& aFavVec);

   /**
    * Returns the list of favorite indexs to add.
    *
    * @returns list of favorite ids and where to add them.
    */
   RArray<TWFFavSyncListIndex>& GetAddList();

   /**
    * Returns the list of favorite indexs to delete.
    *
    * @returns list of favorite ids and where to delete them.
    */
   RArray<TWFFavSyncListIndex>& GetDeleteList();

   /**
    * Returns the list of favorite indexs to update.
    *
    * @returns list of favorite ids and where to update them.
    */
   RArray<TWFFavSyncListIndex>& GetUpdateList();

   /**
    * Adds an index to the delete list.
    *
    * @param aIndex   The index to add.
    */
   void AddToDeleteList(TInt aIndex);

   /**
    * Adds an index to the update list.
    *
    * @param aIndex   The index to add.
    */
   void AddToUpdateList(TInt aIndex);

   /**
    * Adds an index to the add list.
    *
    * @param aIndex   The index to add.
    */
   void AddToAddList(TInt aIndex);

   /**
    * Adds an item to the iAddedIndexList with the given parameters.
    * 
    * @param aIndex, the index of the element, TWFFavSyncListIndex::index.
    * @paran aListType, the type of the element, TWFFavSyncListIndex::type.
    */
   void AddToAddList(TInt aIndex, TWFFavSyncListType aListType);

   /**
    * Checks if an index is in the list.
    *
    * @param aIndex   The index to check for.
    * @return         True if the index is in the list.
    */
   TBool IsInUpdateList(TInt aIndex);

   /**
    * Checks if an index is in the list.
    *
    * @param aIndex   The index to check for.
    * @return         True if the index is in the list.
    */
   TBool IsInAddList(TInt aIndex);

   /**
    * Checks if an index is in the list given a specific list type.
    *
    * @param aIndex, the index to look for.
    * @param aListType, the type to look for.
    * @return True if the index is in the list.
    */
   TBool IsInDeleteList(TInt aIndex, TWFFavSyncListType aListType);

   /**
    * Removes TWFFavSyncListIndex from the iUpdatedIndexList.
    * If iState == EWFLmsList the search is made for types that
    * has listType set to EWFServerList and vice vers.
    *
    * @param aIndex, the index to remove.
    */
   void RemoveFromUpdateList(TInt aIndex);

   /**
    * Status report from the MFavoriteSyncObserver, called from the async prov.
    */
   void SyncError(TInt aError);

   /**
    * Status report from the MFavoriteSyncObserver, called from the async prov.
    */
   void SyncComplete();

protected:
   /**
    * ConstrucL
    *
    * @param aLmsId      The id of the wayfinder LMS.
    */
   void ConstructL(const TDesC& aLmsId);

   /**
    * Constructor
    *
    * @param aObserver   The observer that receives status updates.
    */
   CFavoriteSyncManager(MFavoriteSyncManagerObserver& aObserver);

   /**
    * RunL that handles the async service provider.
    */
   void RunL();

   /**
    * Called by framework call Cancel().
    */
   void DoCancel();

   /**
    * Searches a list for an element. 
    * The list does not need to be  sorted. 
    * The function uses the  == operator to compare the objects.
    *
    * @param aList, the list to search in.
    * @param aItem, the item to search for.
    * @return If found, the index of the matching item.
    *         If not found, KErrNotFound will be returned.
    */
   TInt FindInListL(const RArray<TWFFavSyncListIndex>& aList, 
                    const TWFFavSyncListIndex& aItem);


private:
   /// The source list
   std::vector<Favorite*>* iSourceList;

   /// The favorite list (dest)
   std::vector<Favorite*>* iFavList;

   /// The indexes in the iFavList that will be updated
   RArray<TWFFavSyncListIndex> iUpdatedIndexList;

   /// The indexes in the iFavList that will be added
   RArray<TWFFavSyncListIndex> iAddedIndexList;

   /// The indexes in the iFavList that will be deleted
   RArray<TWFFavSyncListIndex> iDeletedIndexList;

   /// The Observer
   MFavoriteSyncManagerObserver& iObserver;

   /// The async service provider that does the actual work
   class CFavoriteSyncServiceProvider* iServiceProvider;

   /// What list type (lms/server) are we currently working on
   TWFFavSyncListType iListType;

};
#endif
