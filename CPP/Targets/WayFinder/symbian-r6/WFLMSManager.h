/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef C_WF_LMS_MANAGER_H
#define C_WF_LMS_MANAGER_H

#include "arch.h"
#include <vector>
#include <badesca.h>
#include <epos_tposlmsortpref.h>

#include "WFLMSManagerBase.h"

class CPosLandmarkDatabase;
class CPosLandmark;

namespace isab {
   class GuiFavorite;
   class Favorite;
}

using namespace isab;

class CWFLMSManager : public CActive, 
                      public MWFLMSManagerBase
{
public:

   /**
    * NewLC, leaves the instance on the cleanupstack
    * 
    * @param aObserver, the observer to receive status when using the 
    *                   async functions
    * @param aWFLmsUri, the uri for the wayfinder landmark store
    * @return A newly created instance of CWFLMSManager, left on the cleanupstack.
    */
   static CWFLMSManager* NewLC(MWFLMSObserver& aObserver, 
                               const TDesC& aWFLmsUri,
                               const char* aImeiCrcHex);

   /**
    * NewL
    * 
    * @param aObserver, the observer to receive status when using the 
    *                   async functions
    * @param aWFLmsUri, the uri for the wayfinder landmark store
    * @return A newly created instance of CWFLMSManager
    */
   static CWFLMSManager* NewL(MWFLMSObserver& aObserver, 
                              const TDesC& aWFLmsUri,
                              const char* aImeiCrcHex);

   virtual ~CWFLMSManager();

public:

   /**
    * Starts initializaton of a database async.
    */
   void StartDbInitializationAsyncL();

   /**
    * Leaves ownership of iGuiLandmarkList to caller.
    */
   void DisownGuiLandmarkList();

   /**
    * Leaves ownership of iLandmarkList to caller.
    */
   void DisownLandmarkList();

   /**
    * Adds a landmark to the wayfinder landmarkstore, converts
    * the supplied favorite to a CPosLandmark.
    *
    * @param aFavorite, the favorite that should be converted
    *                   to a CPosLandmark and added to the database
    * @return the id of the newly added landmark
    */
   TInt AddLandmarkL(const Favorite& aFavorite);

   /**
    * Opens a landmarkstore with the supplied lmsUri, optionally
    * the landmarkstore is created if not already exists.
    *
    * @param aLmsUri, the name of the landmarkstore to be opened
    * @param aCreate, if true a landmarkstore is created if not exists.
    * @return ETrue if the lms was opened and/or created successfully
    */
   TBool OpenLandmarkStoreL(const TDesC& aLmsUri, TBool aCreate = ETrue);   

   /**
    * Deletes the landmark from the landmarkstore.
    *
    * @param aFavorite, the favorite object that holds the lmId and
    *                   the lms id.
    * @return One of the error types in TWFLMSError.
    */
   TInt DeleteLandmarkL(const class Favorite& aFavorite);

   /**
    * Deletes the landmark from the landmarkstore.
    * 
    * @param aId, the lmId of the landmark to be deleted, takes
    *             for granted that the landmark should be deleted
    *             from the wayfinder lms since no lmsUri is specified.
    * @return One of the error types in TWFLMSError.
    */
   TInt DeleteLandmarkL(TUint aId);

   /**
    * Deletes the landmark from the landmarkstore.
    * 
    * @param aId, the lmId of the landmark to be deleted
    * @param aLmsUri, the name of the lms that contains the landmark 
    *                 that shold be deleted.
    * @return One of the error types in TWFLMSError.
    */
   TInt DeleteLandmarkL(TUint aID, const char* aLmsUri);

   /**
    * Deletes a lms with the specified name.
    * 
    * @param aName, the name of the lms to be deleted.
    * @return One of the error types in TWFLMSError.
    */
   void DeleteLandmarkStoreL(const TDesC& aName);

   /**
    * Imports landmarks as Favorites async, calls the observer when done.
    *
    * aFavTypes, specifies what kind of landmarks that should be fetched
    *            from the lms. See TWFLMSFavTypes for more explanation about
    *            this.
    */
   void GetLandmarksAsyncL(enum TWFLMSFavTypes aFavTypes);

   /**
    * Returns landmarks from the lms as Favorites in a vector.
    *
    * aFavTypes, specifies what kind of landmarks that should be fetched
    *            from the lms. See TWFLMSFavTypes for more explanation about
    *            this.
    * @return iLandmarkList, the member vector containing Favorites is returned.
    */
   std::vector<Favorite*>* GetLandmarksL(enum TWFLMSFavTypes aFavTypes);

   /**
    * Returns landmarks from the lms as GuiFavorites in a vector.
    *
    * aFavTypes, specifies what kind of landmarks that should be fetched
    *            from the lms. See TWFLMSFavTypes for more explanation about
    *            this.
    * @return iGuiLandmarkList, the member vector containing GuiFavorites is 
    *                           returned.
    */
   std::vector<GuiFavorite*>* GetGuiLandmarksL(enum TWFLMSFavTypes aFavTypes);

   /**
    * Imports landmarks as GuiFavorites async, calls the observer when done.
    *
    * aFavTypes, specifies what kind of landmarks that should be fetched
    *            from the lms. See TWFLMSFavTypes for more explanation about
    *            this.
    */
   void GetGuiLandmarksAsyncL(enum TWFLMSFavTypes aFavTypes);

   /**
    * Checks if a landmark exists.
    *
    * @param aId, the id of the landmark to search for.
    * @param aLmsUri, the uri of the database to search in.
    * @return ETrue if found
    */
   TBool LandmarkExistsL(TUint32 aId, const TDesC& aLmsUri);

   /**
    * Checks if a landmark exists. Only searches in Wayfinder
    * landmark database. 
    *
    * @param aId, the id of the landmark to search for.
    * @return ETrue if found
    */
   TBool LandmarkExistsL(TUint32 aId);

   /**
    * Checks if a lms with the supplied name exists.
    *
    * @param aName, the name of the lms to search for.
    * @return ETrue if found
    */
   TBool LandmarkStoreExistsL(const TDesC& aName);

   /**
    * Checks if a lms with the supplied name is empty.
    *
    * @param aName, the name of the lms to search for.
    * @return ETrue if empty
    */
   TBool LandmarkStoreEmptyL(const TDesC& aName);

   /**
    * Updates an existing landmark in the landmarkstore.
    *
    * @param aFavorite, the favorite containing all the new values.
    * @return One of the error types in TWFLMSError.
    */
   TInt UpdateLandmarkL(const class Favorite& aFavorite);

   /**
    * Returns the landmark as a Favorite from the lms.
    *
    * @param aLmId, the id of the landmark.
    * @param aLmsUri, the name of the lms where to find the landmark.
    * @return a newly created Favorite based on the values of the found landmark.
    */
   Favorite* GetLandmarkL(TUint32 aLmId, const TDesC& aLmsUri);

   /**
    * Returns the landmark as a Favorite from the lms.
    *
    * @param aLmId, the id of the landmark.
    * @param aLmsUri, the name of the lms where to find the landmark.
    * @return a newly created Favorite based on the values of the found landmark.
    */
   Favorite* GetLandmarkL(TUint32 aLmId, const char* aLmsUri);

   /**
    * Returns the landmark as a Favorite from the Wayfinder lms.
    *
    * @param aLmId, the id of the landmark.
    * @return a newly created Favorite based on the values of the found landmark.
    */
   Favorite* GetLandmarkL(TUint32 aLmId);

   /**
    * Returns the list of Favorites.
    *
    * @return the iLandmarkList, note that the function returns
    *                            a pointer to a member.
    */
   std::vector<Favorite*>* GetLandmarkList();

   /**
    * Returns the list of GuiFavorites.
    *
    * @return the iGuiLandmarkList, note that the function returns
    *                               a pointer to a member.
    */
   std::vector<GuiFavorite*>* GetGuiLandmarkList();

   /**
    *
    */
   TBool IsIdle();

protected: // From CActive
   
   /**
    * Handles the states when async tasks has been completed, 
    * such as reading landmarks async.
    */
   void RunL();

   /**
    * Called by framework call Cancel().
    */
   void DoCancel();

protected:

   /** 
    * Constructor
    *
    * @param aObserver, the MWFLMSObserver that should receive 
    *                   different notifications.
    */ 
   CWFLMSManager(MWFLMSObserver& aObserver);

   /**
    * Constructl
    *
    * @param aWFLmsUri, the name of the wayfinder lms.
    */
   void ConstructL(const TDesC& aWFLmsUri,
                   const char* aImeiCrcHex);

   /**
    * Deletes the landmark from a landmarkstore.
    *
    * @param CPosLandmarkDatabase, the database from where the landmark should
    *                              be deleted.
    * @param aID, the lmId of the landmark to be deleted
    * @return One of the error types in TWFLMSError.
    */
   TInt DeleteLandmarkL(class CPosLandmarkDatabase* db, TUint aID);

   /**
    * Creates a Favorite based that is basically a copy of the
    * supplied CPosLandmark.
    *
    * @param aLm, the landmark to copy.
    * @param aLmsUri, the name of the lms where the landmark
    *                 is stored.
    * @return a newly created Favorite, note that the caller takes ownership 
    *                                   of the created Favorite.
    */
   Favorite* CreateFavoriteL(const CPosLandmark& aLm, const TDesC& aLmsUri);

   /**
    * Creates a Favorite based that is basically a copy of the
    * supplied CPosLandmark.
    *
    * @param aLm, the landmark to copy.
    * @param aLmsUri, the name of the lms where the landmark
    *                 is stored.
    * @return a newly created Favorite, note that the caller takes ownership 
    *                                   of the created Favorite.
    */
   Favorite* CreateFavoriteL(const CPosLandmark& aLm, const char* aLmsUri);

   /**
    * Creates a GuiFavorite based that is basically a copy of the
    * supplied CPosLandmark.
    *
    * @param aLm, the landmark to copy.
    * @param aLmsUri, the name of the lms where the landmark
    *                 is stored.
    * @return a newly created GuiFavorite, note that the caller takes ownership 
    *                                      of the created GuiFavorite.
    */
   GuiFavorite* CreateGuiFavoriteL(const CPosLandmark& aLm, const TDesC& aLmsUri);

   /**
    * Creates a GuiFavorite based that is basically a copy of the
    * supplied CPosLandmark.
    *
    * @param aLm, the landmark to copy.
    * @param aLmsUri, the name of the lms where the landmark
    *                 is stored.
    * @return a newly created GuiFavorite, note that the caller takes ownership 
    *                                      of the created GuiFavorite.
    */
   GuiFavorite* CreateGuiFavoriteL(const CPosLandmark& aLm, const char* aLmsUri);
   
   /**
    * Creates a CPosLandmark based that is basically a copy of the
    * supplied Favorite.
    *
    * @param aFavorite, the favorite to copy.
    * @return a newly created CPosLandmark, note that the caller takes ownership 
    *                                       of the created CPosLandmark.
    */
   CPosLandmark* CreateLandmarkL(const Favorite& aFavorite);

   /**
    * Creates a CPosLandmark based that is basically a copy of the
    * supplied Favorite.
    *
    * @param aFavorite, the favorite to copy.
    * @return a newly created CPosLandmark, note that the caller takes ownership 
    *                                       of the created CPosLandmark and that 
    *                                       the object is still on the cleanupstack.
    */
   CPosLandmark* CreateLandmarkLC(const Favorite& aFavorite);

   /**
    * Fetches a CPosLandmark from the lms and returns it, not that the landmark
    * object is still on the cleanupstack.
    *
    * @param aLmdId, the id of the landmark to fetch.
    * @param aLmsUri, the name of the lms where the landmark is stored.
    * @param aLms, if supplied and not NULL the landmark will be fetched
    *              from this lms.
    */
   CPosLandmark* GetLandmarkLC(TUint32 aLmId, 
                               const char* aLmsUri,
                               CPosLandmarkDatabase* aLms = NULL);

   /**
    * Fetches a CPosLandmark from the lms and returns it, not that the landmark
    * object is still on the cleanupstack.
    *
    * @param aLmdId, the id of the landmark to fetch.
    * @param aLmsUri, the name of the lms where the landmark is stored.
    * @param aLms, if supplied and not NULL the landmark will be fetched
    *              from this lms.
    */
   CPosLandmark* GetLandmarkLC(TUint32 aLmId, 
                               const TDesC& aLmsUri, 
                               CPosLandmarkDatabase* aLms = NULL);

   /**
    * Updates an existing landmark with the values from the supplied
    * favorite. This function does not update the lms with the new
    * values of the landmark. It only updates the supplied landmark.
    *
    * @param aFavorite, the favorite that holds the correct values used
    *                   for updating the landmark.
    * @param aLm, the landmark to be updated. On return this landmark
    *             is updated with the correct values. 
    */
   void UpdateLandmarkL(const Favorite& aFavorite, CPosLandmark& aLm);

   /**
    * Opens the landmark database and performs initialization if needed.
    * If open leaves this function returns NULL.
    *
    * @param aName, the name of the dabase to be opened.
    * @return a pointer to the opened landmark database.
    */
   CPosLandmarkDatabase* OpenLandmarkStoreInternalL(const TDesC& aName);

   /**
    * Opens the landmark database and performs initialization if needed.
    * If open leaves this function returns NULL.
    * Leaves the opened database on the Cleanup stack.
    *
    * @param aName, the name of the dabase to be opened.
    * @return a pointer to the opened landmark database.
    */
   CPosLandmarkDatabase* OpenLandmarkStoreInternalLC(const TDesC& aName);

   /**
    * Returns a lms with the supplied name. First checks if aLmsUri matches
    * the name of the wf lms. If so, we check that iLandmarkStore isnt NULL,
    * if NULL the wf lms is opened and iLandmarkStore is updated.
    * If aLmsUri doesnt match the name of the wf lms we open the correct
    * lms and returns it.
    *
    * Note that if aLmsUri matches the name of the wf lms the return value
    * of this function will point to iLandmarkStore, hence the caller should
    * be careful when deleting the return value. A good idea is to call the
    * template function in the cpp file (DeleteAndNullIfNotSame) with the 
    * returned landmarkstore pointer as first argument and iLandmarkStore as
    * second argument. The function will the delete the first argument if
    * its not the same as iLandmarkStore (second argument).
    *
    * @param aLmsUri, the name of the lms to be returned.
    * @return a pointer to the correct lms.
    */
   CPosLandmarkDatabase* GetLandmarkStoreL(const TDesC& aLmsUri);

   /**
    * Returns a lms with the supplied name. First checks if aLmsUri matches
    * the name of the wf lms. If so, we check that iLandmarkStore isnt NULL,
    * if NULL the wf lms is opened and iLandmarkStore is updated.
    * If aLmsUri doesnt match the name of the wf lms we open the correct
    * lms and returns it.
    *
    * Note that if aLmsUri matches the name of the wf lms the return value
    * of this function will point to iLandmarkStore, hence the caller should
    * be careful when deleting the return value. A good idea is to call the
    * template function in the cpp file (DeleteAndNullIfNotSame) with the 
    * returned landmarkstore pointer as first argument and iLandmarkStore as
    * second argument. The function will the delete the first argument if
    * its not the same as iLandmarkStore (second argument).
    *
    * @param aLmsUri, the name of the lms to be returned.
    * @return a pointer to the correct lms.
    */
   CPosLandmarkDatabase* GetLandmarkStoreL(const char* aLmsUri);

   /**
    * Init function that sets the members that are used in the async 
    * functions.
    *
    * @param aFavTypes, specifies what kind of landmarks that should be fetched
    *                   from the lms. See TWFLMSFavTypes for more explanation 
    *                   about this.
    */
   void InitGetAsyncL(enum TWFLMSFavTypes aFavTypes);

   /**
    * Checks if all landmarkstores has been processed, if not the next
    * lms in line will be opened.
    */
   void OpenLandmarkStoreAsyncL();

   /**
    * Retreives the ids of the landmarks stored in the current
    * database. These ids are stored in the iItemIds array.
    */
   void PopulateItemIdArrayL();
   
   /**
    * Takes a subset of ids from the iItemIds, how many is depending
    * on the desired chunk size to read. The WFLMSOpWrapper is then
    * started asynchronosly to read these landmarks. Will end up in
    * RunL when all landmarks from the subset is read from the database. 
    */
   void ReadSomeLandmarksAsyncL();

   /**
    * Called when the WFLMSOpWrapper is finished with fetching the 
    * subset of landmarks as reqeusted. Iterates through the
    * result and creates Favorites or GuiFavorites depending on the
    * entry point (GetLandmarksAsyncL or GetGuiLandmarksAsyncL) and
    * stores them in one of the vectors (iGuiLandmarkList, iLandmarkList)
    */
   void FetchLandmarksLC();

   /**
    * Checks if all landmarks has been processed in the current lms, if not
    * the next one will be processed.
    */
   void GetLandmarkAsyncL();

   /**
    * Function that sets iStatus and creates an
    * outstanding request and the completes it.
    * By doing this RunL can be called the appropriate
    * way.
    *
    * @param aStatus The status that iStatus should contain
    *                when this active object has completed and
    *                RunL is called.
    */
   void CompleteRequest(TInt aStatus);

private:
   /// Pointer to the wf lms
   class CPosLandmarkDatabase* iLandmarkStore;
   /// Pointer to the current lms, for async functions
   class CPosLandmarkDatabase* iAsyncLms;
   /// An active object wrapping a CPosLmOperation object
   class CWFLMSOpWrapper* iLmOpWrapper;
   /// Defines the sort preferences when fetching landmarks
   TPosLmSortPref iSortPref;
   /// Defines the number of landmarks to read in each chunk
   TInt iNrOfItemsToRead;
   /// Current status of an async operation
   TWFLMSEvent iActiveOperation;
   /// Id containing all landmark ids for a database
   RArray<TPosLmItemId> iItemIds;
};

inline void CWFLMSManager::CompleteRequest(TInt aStatus) 
{
   class TRequestStatus* status = &iStatus; 
   iStatus = KRequestPending;
   SetActive();
   User::RequestComplete(status, aStatus);
}

#endif
