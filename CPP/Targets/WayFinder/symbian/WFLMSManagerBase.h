/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef C_WF_LMS_MANAGER_BASE_H
#define C_WF_LMS_MANAGER_BASE_H

#include "arch.h"
#include <vector>
#include <badesca.h>

#include "WFLMSObserver.h"

namespace isab {
   class GuiFavorite;
   class Favorite;
}

using namespace isab;

class MWFLMSManagerBase
{
protected:

   MWFLMSManagerBase(MWFLMSObserver& aObserver) : 
      iObserver(aObserver)
   {}

public:

   virtual ~MWFLMSManagerBase()
   {}

   /**
    * Events for the event generator.
    */
   enum TWFLMSEvent {
      EWFLMSNoOperation,               // No operation ongoing
      EWFLMSGetLandmarksAsyncOpen,     // Generated when a lms should be opened
      EWFLMSGetLandmarksAsyncNext,     // Generated when the next lm should be processed
      EWFLMSLandmarksAsyncSearch,      // Do a search for all landmarks in the current database
      EWFLMSLandmarksAsyncRead,        // Read some landmarks  
      EWFLMSGetLandmarksAsyncFinished, // Generated when the import is finished
      EWFLMSInitializeDb               // Initializing a database
   };

   /**
    * Defines what kind of landmarks that should be imported when user
    * calls any of the GetLandmarks functions
    */
   enum TWFLMSFavTypes {
      EWFLMSWFFavs,    // Only landmarks from our lms (WF landmarks)
      EWFLMSLMFavs,    // All landmars except for WF landmarks
      EWFLMSLMWFFavs   // All landmarks
   };

   /**
    * Error codes for lms import
    */
   enum TWFLMSError {
      EWFLMSErrNone,       // No error
      EWFLMSLmIdNotValid,  // The id of the lm is not valid
      EWFLMSLmsOpenError,  // Error when trying to open the lms
      EWFLMSLmsNotExists,  // The lms does not exist
      EWFLMSLmNotFound,    // Couldnt find the requested lm
      EWFLMSAccessDenied   // Access denied when trying to do something with the lms
   };

public: // Pure virtual functions


   /**
    * Leaves ownership of iGuiLandmarkList to caller.
    */
   virtual void DisownGuiLandmarkList() = 0;

   /**
    * Leaves ownership of iLandmarkList to caller.
    */
   virtual void DisownLandmarkList() = 0;

   /**
    * Adds a landmark to the wayfinder landmarkstore, converts
    * the supplied favorite to a CLmsLandmark.
    *
    * @param aFavorite, the favorite that should be converted
    *                   to a CLmsLandmark and added to the database
    * @return the id of the newly added landmark
    */
   virtual TInt AddLandmarkL(const Favorite& aFavorite) = 0;

   /**
    * Opens a landmarkstore with the supplied lmsUri, optionally
    * the landmarkstore is created if not already exists.
    *
    * @param aLmsUri, the name of the landmarkstore to be opened
    * @param aCreate, if true a landmarkstore is created if not exists.
    * @return ETrue if the lms was opened and/or created successfully
    */
   virtual TBool OpenLandmarkStoreL(const TDesC& aLmsUri, TBool aCreate = ETrue) = 0;

   /**
    * Deletes the landmark from the landmarkstore.
    *
    * @param aFavorite, the favorite object that holds the lmId and
    *                   the lms id.
    * @return One of the error types in TWFLMSError.
    */
   virtual TInt DeleteLandmarkL(const class Favorite& aFavorite) = 0;

   /**
    * Deletes the landmark from the landmarkstore.
    * 
    * @param aId, the lmId of the landmark to be deleted, takes
    *             for granted that the landmark should be deleted
    *             from the wayfinder lms since no lmsUri is specified.
    * @return One of the error types in TWFLMSError.
    */
   virtual TInt DeleteLandmarkL(TUint aId) = 0;

   /**
    * Deletes the landmark from the landmarkstore.
    * 
    * @param aId, the lmId of the landmark to be deleted
    * @param aLmsUri, the name of the lms that contains the landmark 
    *                 that shold be deleted.
    * @return One of the error types in TWFLMSError.
    */
   virtual TInt DeleteLandmarkL(TUint aID, const char* aLmsUri) = 0;

   /**
    * Deletes a lms with the specified name.
    * 
    * @param aName, the name of the lms to be deleted.
    * @return One of the error types in TWFLMSError.
    */
   virtual void DeleteLandmarkStoreL(const TDesC& aName) = 0;

   /**
    * Imports landmarks as Favorites async, calls the observer when done.
    *
    * aFavTypes, specifies what kind of landmarks that should be fetched
    *            from the lms. See TWFLMSFavTypes for more explanation about
    *            this.
    */
   virtual void GetLandmarksAsyncL(enum TWFLMSFavTypes aFavTypes) = 0;

   /**
    * Returns landmarks from the lms as Favorites in a vector.
    *
    * aFavTypes, specifies what kind of landmarks that should be fetched
    *            from the lms. See TWFLMSFavTypes for more explanation about
    *            this.
    * @return iLandmarkList, the member vector containing Favorites is returned.
    */
   virtual std::vector<Favorite*>* GetLandmarksL(enum TWFLMSFavTypes aFavTypes) = 0;

   /**
    * Returns landmarks from the lms as GuiFavorites in a vector.
    *
    * aFavTypes, specifies what kind of landmarks that should be fetched
    *            from the lms. See TWFLMSFavTypes for more explanation about
    *            this.
    * @return iGuiLandmarkList, the member vector containing GuiFavorites is 
    *                           returned.
    */
   virtual std::vector<GuiFavorite*>* GetGuiLandmarksL(enum TWFLMSFavTypes aFavTypes) = 0;

   /**
    * Imports landmarks as GuiFavorites async, calls the observer when done.
    *
    * aFavTypes, specifies what kind of landmarks that should be fetched
    *            from the lms. See TWFLMSFavTypes for more explanation about
    *            this.
    */
   virtual void GetGuiLandmarksAsyncL(enum TWFLMSFavTypes aFavTypes) = 0;

   /**
    * Checks if a landmark exists.
    *
    * @param aId, the id of the landmark to search for.
    * @param aLmsUri, the uri of the database to search in.
    * @return ETrue if found
    */
   virtual TBool LandmarkExistsL(TUint32 aId, const TDesC& aLmsUri) = 0;

   /**
    * Checks if a landmark exists. Only searches in Wayfinder
    * landmark database. 
    *
    * @param aId, the id of the landmark to search for.
    * @return ETrue if found
    */
   virtual TBool LandmarkExistsL(TUint32 aId) = 0;

   /**
    * Checks if a lms with the supplied name exists.
    *
    * @param aName, the name of the lms to search for.
    * @return ETrue if found
    */
   virtual TBool LandmarkStoreExistsL(const TDesC& aName) = 0;

   /**
    * Checks if a lms with the supplied name is empty.
    *
    * @param aName, the name of the lms to search for.
    * @return ETrue if empty
    */
   virtual TBool LandmarkStoreEmptyL(const TDesC& aName) = 0;

   /**
    * Updates an existing landmark in the landmarkstore.
    *
    * @param aFavorite, the favorite containing all the new values.
    * @return One of the error types in TWFLMSError.
    */
   virtual TInt UpdateLandmarkL(const class Favorite& aFavorite) = 0;

   /**
    * Returns the landmark as a Favorite from the lms.
    *
    * @param aLmId, the id of the landmark.
    * @param aLmsUri, the name of the lms where to find the landmark.
    * @return a newly created Favorite based on the values of the found landmark.
    */
   virtual Favorite* GetLandmarkL(TUint32 aLmId, const TDesC& aLmsUri) = 0;

   /**
    * Returns the landmark as a Favorite from the lms.
    *
    * @param aLmId, the id of the landmark.
    * @param aLmsUri, the name of the lms where to find the landmark.
    * @return a newly created Favorite based on the values of the found landmark.
    */
   virtual Favorite* GetLandmarkL(TUint32 aLmId, const char* aLmsUri) = 0;

   /**
    * Returns the list of Favorites.
    *
    * @return the iLandmarkList, note that the function returns
    *                            a pointer to a member.
    */
   virtual std::vector<Favorite*>* GetLandmarkList() = 0;

   /**
    * Returns the list of GuiFavorites.
    *
    * @return the iGuiLandmarkList, note that the function returns
    *                               a pointer to a member.
    */
   virtual std::vector<GuiFavorite*>* GetGuiLandmarkList() = 0;

protected:
   /// Observer receiving notifications when calling the async functions
   class MWFLMSObserver& iObserver;
   /// Vector containing Favorites exported from the lms
   std::vector<Favorite*>* iLandmarkList;
   /// Vector containing GuiFavorites exported from the lms
   std::vector<GuiFavorite*>* iGuiLandmarkList;
   /// Name of the wf lms
   HBufC* iWFLmsUri;
   /// Name of the wf lms as a char, only to minimise nbr of convertions 
   /// between descriptors and char*
   char* iWFLmsUri_char;
   /// The imei, crc and the converted to hex in a char*
   char* iImeiCrcHex;
   /// Iterator for async functions keeping track of the lms to process
   TInt iAsyncLmsListIndex;
   /// Iterator for async functions keeping track of the lm to process
   TInt iAsyncLmListIndex;
   /// List that holds all landmarks stores to import, for async functions
   CDesCArray* iAsyncLmsList;
   /// Array that holds lm indexes for landmarks to import, for async functions
   RArray<TUint> iAsyncLmList;
   /// Flag keeping track of what to import, GuiFavorites or Favorites, 
   /// for async functions
   TBool iAsyncGetGuiLandmarks;
};
#endif
