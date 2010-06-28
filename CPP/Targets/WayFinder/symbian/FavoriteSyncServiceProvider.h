/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef C_FAVORITE_SYNC_SERVICE_PROVIDER_H
#define C_FAVORITE_SYNC_SERVICE_PROVIDER_H

#include "arch.h"
#include <vector>

#include "FavoriteSyncObserver.h"

namespace isab {
   class GuiFavorite;
   class Favorite;
}

using namespace isab;

class CFavoriteSyncServiceProvider : public CActive
{
public:
   /**
    * Internal states for the active object.
    */
   enum TWFFavSyncStates {
      EWFFavSyncCompareFavNext,
      EWFFavSyncCompareFavDone,
      EWFFavSyncComplete,
      EWFFavSyncError
   };

   /**
    * Standard NewLC function.
    *
    * @param aObserver   The observer that receives status updates.
    * @param aLmsId      The id of the wayfinder LMS.
    * @return   The newly created instance of CFavoriteSyncServiceProvider.
    */
   static CFavoriteSyncServiceProvider* NewLC(MFavoriteSyncObserver& aObserver,
                                              const TDesC& aLmsId);

   /**
    * Standard NewLC function.
    *
    * @param aObserver   The observer that receives status updates.
    * @param aLmsId      The id of the wayfinder LMS.
    * @return   The newly created instance of CFavoriteSyncServiceProvider.
    */
   static CFavoriteSyncServiceProvider* NewL(MFavoriteSyncObserver& aObserver,
                                             const TDesC& aLmsId);

   /**
    * Destructor
    */
   virtual ~CFavoriteSyncServiceProvider();

   /**
    * Starts the syncing of the two vectors.
    * (Normally this is LMS - Client and then Server - Client.)
    *
    * @aSourceVec   Vector of favorites from LMS or Server to be synced.
    * @aDestVec     Vector of favorites from Client to be updated and synced.
    * @aStatus      Callers request status for the async request.
    */
   void DoSyncL(std::vector<class Favorite*>* aSourceVec, 
                std::vector<class Favorite*>* aDestVec,
                TRequestStatus& aStatus);

protected:
   /**
    * ConstrucL
    */
   void ConstructL(const TDesC& aLmsId);

   /**
    * Constructor
    */
   CFavoriteSyncServiceProvider(MFavoriteSyncObserver& aObserver);

   /**
    * RunL that performs a call to do one iteration of the calculation.
    */
   void RunL();

   /**
    * Called by framework call Cancel().
    */
   void DoCancel();

private:
   /**
    * Function that sets iStatus and creates an
    * outstanding request and then completes it.
    * By doing this RunL can be called the appropriate
    * way.
    *
    * @param aStatus The status that iStatus should contain
    *                when this active object has completed and
    *                RunL is called.
    */
   void CompleteRequest(TInt aStatus);

   /**
    * This function takes current favorite from iSourceList and compares it 
    * to the favorite with the same lmsID in iFavList and updates if necessery 
    * (more information about this step can be found in DoSyncL notes). 
    */
   void CompareFavoriteL();

   /**
    * Compares the two vectors with set_difference to find which favorites 
    * which have been deleted.
    */
   void FindDeletedFavsL();

private:
   /// The request status of our invoker
   TRequestStatus* iCallerStatus;

   /// The source list
   std::vector<class Favorite*>* iSourceList;

   /// The favorite list (dest)
   std::vector<class Favorite*>* iFavList;

   /// The Observer
   MFavoriteSyncObserver& iObserver;

   /// The current index in the vector
   TUint iCurrentIndex;

   /// The internal state
   TInt iState;

   /// WF LMS ID
   char* iLmsId;
};
#endif
