/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "WFLMSManager.h"

#include "GuiProt/Favorite.h"

#include <epos_cposlmdatabasemanager.h> 
#include <epos_cposlandmarkdatabase.h> 
#include <epos_cposlandmark.h> 
#include <epos_cposlmitemiterator.h>
#include <lbsposition.h> 
#include <memory>
#include <e32math.h>

#include "WFLMSOpWrapper.h"
#include "WFTextUtil.h"
#include "DeleteHelpers.h"
#include "MC2Coordinate.h"
#include "nav2util.h"

template <class LHS, class RHS>
void DeleteAndNullIfNotSame(LHS*& lhs, RHS*& rhs)
{
   if (lhs != rhs) {
      delete lhs;
   } 
   lhs = NULL;
}

CWFLMSManager* CWFLMSManager::NewLC(MWFLMSObserver& aObserver, 
                                    const TDesC& aWFLmsUri,
                                    const char* aImeiCrcHex)
{
   CWFLMSManager* self = new (ELeave) CWFLMSManager(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL(aWFLmsUri, aImeiCrcHex);
   return self;
}

CWFLMSManager* CWFLMSManager::NewL(MWFLMSObserver& aObserver, 
                                   const TDesC& aWFLmsUri,
                                   const char* aImeiCrcHex)
{
   CWFLMSManager* self = CWFLMSManager::NewLC(aObserver, aWFLmsUri, aImeiCrcHex);
   CleanupStack::Pop(self);
   return self;
}

CWFLMSManager::CWFLMSManager(MWFLMSObserver& aObserver) :
   CActive(CActive::EPriorityStandard),
   MWFLMSManagerBase(aObserver),
   iSortPref(CPosLandmark::ELandmarkName, TPosLmSortPref::EAscending),
   iNrOfItemsToRead(5),
   iActiveOperation(EWFLMSNoOperation)
{
   CActiveScheduler::Add(this);
}

void CWFLMSManager::ConstructL(const TDesC& aWFLmsUri,
                               const char* aImeiCrcHex) 
{
   iWFLmsUri = aWFLmsUri.AllocL();
   iWFLmsUri_char = WFTextUtil::newTDesDupL(*iWFLmsUri);
   iImeiCrcHex = isab::strdup_new(aImeiCrcHex);   
   
   // Open database and initilize it if needed
   OpenLandmarkStoreL(*iWFLmsUri, EFalse);
   iLandmarkList = new std::vector<Favorite*>;
   iGuiLandmarkList = new std::vector<GuiFavorite*>;
   iLmOpWrapper = new (ELeave) CWFLMSOpWrapper();
}

CWFLMSManager::~CWFLMSManager()
{
   DeleteAndNullIfNotSame(iAsyncLms, iLandmarkStore);
   delete iLandmarkStore;
   iLandmarkStore = NULL;
   delete iWFLmsUri;
   delete[] iWFLmsUri_char;
   STLUtility::deleteValues(*iLandmarkList);
   STLUtility::deleteValues(*iGuiLandmarkList);
   delete iLandmarkList;
   delete iGuiLandmarkList;
   delete iLmOpWrapper;
   delete iAsyncLmsList;
   delete[] iImeiCrcHex;
   ReleaseLandmarkResources();
}

void CWFLMSManager::StartDbInitializationAsyncL()
{
   // Create initialize operation
   CPosLmOperation* operation = iLandmarkStore->InitializeL();
   
   iActiveOperation = EWFLMSInitializeDb;
   iStatus = KRequestPending;
   SetActive();
 
   // Start asynchronous initialization
   TBool reportProgress = ETrue;
   iLmOpWrapper->StartOperation(operation, iStatus, reportProgress);
}

void CWFLMSManager::DisownGuiLandmarkList()
{
   iGuiLandmarkList = new std::vector<GuiFavorite*>;
}

void CWFLMSManager::DisownLandmarkList()
{
   iLandmarkList = new std::vector<Favorite*>;
}

TInt CWFLMSManager::AddLandmarkL(const Favorite& aFavorite)
{
   CPosLandmark* lm = CPosLandmark::NewLC();
   TBuf<256> var;
   WFTextUtil::char2TDes(var, aFavorite.getName());
   lm->SetLandmarkNameL(var);
   WFTextUtil::char2TDes(var, aFavorite.getDescription());
   lm->SetLandmarkDescriptionL(var);
   TLocality loc;
   Nav2Coordinate nav2Coord(aFavorite.getLat(), aFavorite.getLon());
   MC2Coordinate mc2Coord(nav2Coord);
   WGS84Coordinate coord(mc2Coord);
   loc.SetCoordinate(coord.latDeg, coord.lonDeg);
   //loc.SetAccuracy(0, 0);
   lm->SetPositionL(loc);
   TInt ret = 0;
   if (!iLandmarkStore) {
      OpenLandmarkStoreL(*iWFLmsUri, ETrue);
   }
   TRAPD(err, ret = iLandmarkStore->AddLandmarkL(*lm));
   if (err != KErrNone) {
      ret = -1;
   }
   CleanupStack::PopAndDestroy(lm);
   return ret;
}

TBool CWFLMSManager::OpenLandmarkStoreL(const TDesC& aLmsUri, TBool aCreate)
{
   if (iLandmarkStore) {
      // Prevent memory leak
      delete iLandmarkStore;
      iLandmarkStore = NULL;
   }
   
   CPosLmDatabaseManager* dbManager = CPosLmDatabaseManager::NewL();
   CleanupStack::PushL(dbManager);
   TBool dbExists = dbManager->DatabaseExistsL(aLmsUri);
   if (!dbExists && aCreate) {
      // Database does not exist and aCreate is true, create the database
      HPosLmDatabaseInfo* dbInfo = HPosLmDatabaseInfo::NewLC(aLmsUri);
      TRAPD(err, dbManager->CreateDatabaseL(*dbInfo));
      if (err == KErrNotSupported || err == KErrArgument) {
         // The protocol specified in the URI is not supported or
         // The URI is incorrect
         CleanupStack::PopAndDestroy(dbInfo);
         CleanupStack::PopAndDestroy(dbManager);
         return EFalse;
      } 
      CleanupStack::PopAndDestroy(dbInfo);
   } else if (!dbExists) {
      // The database does not exist and aCreate is false, do not create the
      // database.
      CleanupStack::PopAndDestroy(dbManager);
      return EFalse;
   }
   CleanupStack::PopAndDestroy(dbManager);
   iLandmarkStore = OpenLandmarkStoreInternalL(aLmsUri);
   // Return true if iLandmarkStore was successfully created
   return iLandmarkStore != NULL;   
}

CPosLandmarkDatabase* CWFLMSManager::OpenLandmarkStoreInternalL(const TDesC& aName)
{
   CPosLandmarkDatabase* db = OpenLandmarkStoreInternalLC(aName);
   if (db) {
      CleanupStack::Pop(db);
   }
   return db;
}

CPosLandmarkDatabase* CWFLMSManager::OpenLandmarkStoreInternalLC(const TDesC& aName)
{
   CPosLandmarkDatabase* db = NULL;
   TRAPD(err, db = CPosLandmarkDatabase::OpenL(aName));
   if (err != KErrNone) {
      // Could not open the database
      return NULL;
   }
   //CleanupStack::PushL(db);
   // Perform initialization. If initialization is not needed, this
   // function will no do anything, SDK says not to have the object on 
   // the cleanup stack.
   ExecuteAndDeleteLD(db->InitializeL());
   CleanupStack::PushL(db);
   return db;
}

TInt CWFLMSManager::DeleteLandmarkL(CPosLandmarkDatabase* db, TUint aID)
{
   if (!db) {
      return EWFLMSLmsOpenError;
   }
   TRAPD(err, db->RemoveLandmarkL(aID)); 
   if (err != KErrNotFound) {
      // The database is read-only since this is the only
      // leave code this function call can leave with
      return EWFLMSAccessDenied;
   }
   return EWFLMSErrNone;
}

TInt CWFLMSManager::DeleteLandmarkL(const Favorite& aFavorite)
{
   if (aFavorite.getLmID() == MAX_UINT32) {
      return EWFLMSLmIdNotValid;
   }
   CPosLandmarkDatabase* db = GetLandmarkStoreL(aFavorite.getLmsID());
   TInt ret = DeleteLandmarkL(db, aFavorite.getLmID());
   DeleteAndNullIfNotSame(db, iLandmarkStore);
   return ret;  
}

TInt CWFLMSManager::DeleteLandmarkL(TUint aID)
{
   if (aID == MAX_UINT32) {
      return EWFLMSLmIdNotValid;
   }
   if (!iLandmarkStore) {
      // WF landmarkstore is not open, try to open it
      iLandmarkStore = OpenLandmarkStoreInternalL(*iWFLmsUri);
   } 
   if (!iLandmarkStore) {
      // The wf landmarkstore does not exist.
      return EWFLMSLmsNotExists;
   }
   TInt ret = DeleteLandmarkL(iLandmarkStore, aID);
   return ret;  
}

TInt CWFLMSManager::DeleteLandmarkL(TUint aID, const char* aLmsUri)
{
   if (aID == MAX_UINT32) {
      return EWFLMSLmIdNotValid;
   }
   CPosLandmarkDatabase* db = GetLandmarkStoreL(aLmsUri);
   TInt ret = DeleteLandmarkL(db, aID);
   DeleteAndNullIfNotSame(db, iLandmarkStore);
   return ret;
}

void CWFLMSManager::DeleteLandmarkStoreL(const TDesC& aName)
{
   CPosLmDatabaseManager* dbManager = CPosLmDatabaseManager::NewL();
   CleanupStack::PushL(dbManager);
   TBool dbExists = EFalse;
   TInt err = 0;
   TRAP(err, dbExists = dbManager->DatabaseExistsL(aName));
   if (err != KErrNone || !dbExists) {
      CleanupStack::PopAndDestroy(dbManager);
      return;
   } 
   if (iLandmarkStore) {
      HBufC* uri = iLandmarkStore->DatabaseUriLC();
      if (uri->FindF(aName) != KErrNotFound) {
         delete iLandmarkStore;
         iLandmarkStore = NULL;
      }
      CleanupStack::PopAndDestroy(uri);
   }
   TRAP(err, dbManager->DeleteDatabaseL(aName));
   CleanupStack::PopAndDestroy(dbManager);
}

std::vector<Favorite*>* CWFLMSManager::GetLandmarksL(enum TWFLMSFavTypes aFavTypes)
{
   // Clear the vector
   STLUtility::deleteValues(*iLandmarkList);
   iLandmarkList->reserve(100);
  
   RArray<TPosLmItemId> landmarks;
   CleanupClosePushL(landmarks);
   CPosLmDatabaseManager* dbManager = CPosLmDatabaseManager::NewL();
   CleanupStack::PushL(dbManager);
   // Get a list of databases
   CDesCArray* lmsList = dbManager->ListDatabasesLC();
   for (TInt i = 0; i < lmsList->Count(); ++i) {
      TPtrC lmsUri = lmsList->MdcaPoint(i);
      if ((aFavTypes == EWFLMSLMFavs && lmsUri == *iWFLmsUri) ||
          (aFavTypes == EWFLMSWFFavs && lmsUri != *iWFLmsUri)) {
         // First expression checks if we want to exclude wayfinder landmarks
         // and the current lms is the wayfinder lms, then continue with the next
         // lms.
         // The second expression checks if we only wants wayfinder landmarks and if
         // the current lms isnt the wayfinder lms, then continue with the next lms.
         continue;
      } 
      CPosLandmarkDatabase* db = OpenLandmarkStoreInternalLC(lmsUri);
      if (!db) {
         continue;
      }
      // Create an iterator to retrieve IDs for all landmarks in the database.
      CPosLmItemIterator* it = db->LandmarkIteratorL(iSortPref);
      CleanupStack::PushL(it);
      // Transfer all IDs to an array
      it->GetItemIdsL(landmarks, 0, it->NumOfItemsL());
      CleanupStack::PopAndDestroy(it);
      // Set partial parameters so that only the landmark name will be retrieved.
      CPosLmPartialReadParameters* part = CPosLmPartialReadParameters::NewLC();
      part->SetRequestedAttributes(CPosLandmark::ELandmarkName | CPosLandmark::EDescription |
                                   CPosLandmark::EPosition | CPosLandmark::EIcon);
      db->SetPartialReadParametersL(*part);
      CleanupStack::PopAndDestroy(part);
      // Tell Landmarks to start collecting the partial landmark data. Note that
      // this will be quicker than reading full landmarks.
      CPosLmOperation* op = db->PreparePartialLandmarksL(landmarks);
      op->ExecuteL();
      CleanupStack::PopAndDestroy(&landmarks);
      CleanupStack::PushL(op);
      // The landmark objects can be retrieved from the operation object. These
      // landmark objects now contains only the names of the landmarks so
      // the heap is used efficiently.
      CArrayPtr<CPosLandmark>* lmList =
         db->TakePreparedPartialLandmarksL(op);
      
      char* lmsUri_ch = WFTextUtil::newTDesDupL(lmsUri);
      for (TInt x = 0; x < lmList->Count(); ++x) {
         // For each landmark in the current landmark store
         iLandmarkList->push_back(CreateFavoriteL(*(lmList->operator[](x)), lmsUri_ch));
      }
      delete[] lmsUri_ch;
      lmList->ResetAndDestroy();      
      delete lmList;
      CleanupStack::PopAndDestroy(op);
      CleanupStack::PopAndDestroy(db);
   }
   CleanupStack::PopAndDestroy(lmsList);
   CleanupStack::PopAndDestroy(dbManager);
   return iLandmarkList;
}

std::vector<GuiFavorite*>* CWFLMSManager::GetGuiLandmarksL(enum TWFLMSFavTypes aFavTypes)
{
   // Clear the vector
   STLUtility::deleteValues(*iGuiLandmarkList);
   iGuiLandmarkList->reserve(100);

   RArray<TPosLmItemId> landmarks;
   CleanupClosePushL(landmarks);
   CPosLmDatabaseManager* dbManager = CPosLmDatabaseManager::NewL();
   CleanupStack::PushL(dbManager);
   // Get a list of databases
   CDesCArray* lmsList = dbManager->ListDatabasesLC();
   for (TInt i = 0; i < lmsList->Count(); ++i) {
      TPtrC lmsUri = lmsList->MdcaPoint(i);
      if ((aFavTypes == EWFLMSLMFavs && lmsUri == *iWFLmsUri) ||
          (aFavTypes == EWFLMSWFFavs && lmsUri != *iWFLmsUri)) {
         // First expression checks if we want to exclude wayfinder landmarks
         // and the current lms is the wayfinder lms, then continue with the next
         // lms.
         // The second expression checks if we only wants wayfinder landmarks and if
         // the current lms isnt the wayfinder lms, then continue with the next lms.
         continue;
      } 
      CPosLandmarkDatabase* db = OpenLandmarkStoreInternalLC(lmsUri);
      if (!db) {
         continue;
      }
      // Create an iterator to retrieve IDs for all landmarks in the database.
      CPosLmItemIterator* it = db->LandmarkIteratorL();
      CleanupStack::PushL(it);
      // Transfer all IDs to an array
      it->GetItemIdsL(landmarks, 0, it->NumOfItemsL());
      CleanupStack::PopAndDestroy(it);
      // Set partial parameters so that only the landmark name will be retrieved.
      CPosLmPartialReadParameters* part = CPosLmPartialReadParameters::NewLC();
      part->SetRequestedAttributes(CPosLandmark::ELandmarkName | CPosLandmark::EDescription |
                                CPosLandmark::EPosition | CPosLandmark::EIcon);
      db->SetPartialReadParametersL(*part);
      CleanupStack::PopAndDestroy(part);
      // Tell Landmarks to start collecting the partial landmark data. Note that
      // this will be quicker than reading full landmarks.
      CPosLmOperation* op = db->PreparePartialLandmarksL(landmarks);
      op->ExecuteL();
      CleanupStack::PopAndDestroy(&landmarks);
      CleanupStack::PushL(op);
      // The landmark objects can be retrieved from the operation object. These
      // landmark objects now contains only the names of the landmarks so
      // the heap is used efficiently.
      CArrayPtr<CPosLandmark>* lmList =
         db->TakePreparedPartialLandmarksL(op);
      
      char* lmsUri_ch = WFTextUtil::newTDesDupL(lmsUri);
      for (TInt x = 0; x < lmList->Count(); ++x) {
         // For each landmark in the current landmark store
         iGuiLandmarkList->push_back(CreateGuiFavoriteL(*(lmList->operator[](x)), lmsUri_ch));
      }
      delete[] lmsUri_ch;
      lmList->ResetAndDestroy();      
      delete lmList;
      CleanupStack::PopAndDestroy(op);
      CleanupStack::PopAndDestroy(db);
   }
   CleanupStack::PopAndDestroy(lmsList);
   CleanupStack::PopAndDestroy(dbManager);
   return iGuiLandmarkList;
}

void CWFLMSManager::GetLandmarksAsyncL(enum TWFLMSFavTypes aFavTypes)
{
   STLUtility::deleteValues(*iLandmarkList);
   iLandmarkList->reserve(100);
   iAsyncGetGuiLandmarks = EFalse;
   InitGetAsyncL(aFavTypes);
}

void CWFLMSManager::GetGuiLandmarksAsyncL(enum TWFLMSFavTypes aFavTypes)
{
   STLUtility::deleteValues(*iGuiLandmarkList);
   iGuiLandmarkList->reserve(100);
   iAsyncGetGuiLandmarks = ETrue;
   InitGetAsyncL(aFavTypes);
}

TBool CWFLMSManager::LandmarkExistsL(TUint32 aId, const TDesC& aLmsUri)
{
   if (!LandmarkStoreExistsL(aLmsUri)) {
      // The database does not exist, cant delete the landmark
      return EFalse;
   }
   CPosLandmarkDatabase* db = NULL;
   TRAPD(err, db = CPosLandmarkDatabase::OpenL(aLmsUri));
   if (err != KErrNone) {
      return EFalse;
   }
   CleanupStack::PushL(db);
   // Remove the landmark, if it does not exist nothing will happen
   db->RemoveLandmarkL(aId);
   CleanupStack::PopAndDestroy(db);
   return ETrue;
}

TBool CWFLMSManager::LandmarkExistsL(TUint32 aId)
{
   return LandmarkExistsL(aId, *iWFLmsUri);
}

TBool CWFLMSManager::LandmarkStoreExistsL(const TDesC& aName)
{
   CPosLmDatabaseManager* dbManager = CPosLmDatabaseManager::NewL();
   CleanupStack::PushL(dbManager);
   TBool dbExists = EFalse;
   TRAPD(err, dbExists = dbManager->DatabaseExistsL(aName));
   if (err != KErrNone) {
      dbExists = EFalse;
   }
   CleanupStack::PopAndDestroy(dbManager);
   return dbExists;
}

TBool CWFLMSManager::LandmarkStoreEmptyL(const TDesC& aName)
{
   CPosLandmarkDatabase* db = OpenLandmarkStoreInternalLC(aName);
   if (!db) {
      return ETrue;
   }
   // Create an iterator to retrieve IDs for all landmarks in the database.
   CPosLmItemIterator* it = db->LandmarkIteratorL();
   CleanupStack::PushL(it);
   // Transfer all IDs to an array
   TInt nbrItems = it->NumOfItemsL();
   CleanupStack::PopAndDestroy(it);
   CleanupStack::PopAndDestroy(db);
   return (nbrItems == 0);
}

TInt CWFLMSManager::UpdateLandmarkL(const Favorite& aFavorite)
{
   CPosLandmarkDatabase* lms = GetLandmarkStoreL(aFavorite.getLmsID());
   if (!lms) {
      return EWFLMSLmsOpenError;
   }
   CPosLandmark* lm = GetLandmarkLC(aFavorite.getLmID(), aFavorite.getLmsID(), lms);
   if (!lm) {
      DeleteAndNullIfNotSame(lms, iLandmarkStore);
      return EWFLMSLmNotFound;
   }
   UpdateLandmarkL(aFavorite, *lm);
   TRAPD(err, lms->UpdateLandmarkL(*lm));
   
   // Call this template function that checks if
   // lms and iLandmarkStore actally isnt the same
   // pointer, if not lms is deleted and set to NULL
   DeleteAndNullIfNotSame(lms, iLandmarkStore);
   CleanupStack::PopAndDestroy(lm);
   if (err != KErrNone) {
      return err;
   }
   return EWFLMSErrNone;
}

Favorite* CWFLMSManager::GetLandmarkL(TUint32 aLmId, const TDesC& aLmsUri)
{
   CPosLandmark* lm = GetLandmarkLC(aLmId, aLmsUri);
   Favorite* fav = CreateFavoriteL(*lm, aLmsUri);
   CleanupStack::PopAndDestroy(lm);
   return fav;
}

Favorite* CWFLMSManager::GetLandmarkL(TUint32 aLmId, const char* aLmsUri)
{
   CPosLandmark* lm = GetLandmarkLC(aLmId, aLmsUri);
   Favorite* fav = CreateFavoriteL(*lm, aLmsUri);
   CleanupStack::PopAndDestroy(lm);
   return fav;
}

Favorite* CWFLMSManager::GetLandmarkL(TUint32 aLmId)
{
   return GetLandmarkL(aLmId, *iWFLmsUri);
}

CPosLandmark* CWFLMSManager::GetLandmarkLC(TUint32 aLmId, 
                                           const TDesC& aLmsUri, 
                                           CPosLandmarkDatabase* aLms)
{
   CPosLandmarkDatabase* lms = NULL;
   CPosLandmark* lm = NULL;
   if (!aLms) {
      // The supplied landmarkstore pointer is NULL,
      lms = GetLandmarkStoreL(aLmsUri);
      if (!lms) {
         // Still null, cant do anything
         return NULL;
      }
      TRAPD(err, lm = lms->ReadLandmarkLC(aLmId); CleanupStack::Pop(lm));
      if (err != KErrNone) {
         // ReadLandmarkLC leaved, probably with KErrNotFound,
         // set the lm to NULL
         lm = NULL;
      }
      // Call this template function that checks if
      // lms and iLandmarkStore actally isnt the same
      // pointer, if not lms is deleted and set to NULL
      DeleteAndNullIfNotSame(lms, iLandmarkStore);
   } else {
      TRAPD(err, lm = aLms->ReadLandmarkLC(aLmId); CleanupStack::Pop(lm));
      if (err != KErrNone) {
         // ReadLandmarkLC leaved, probably with KErrNotFound,
         // set the lm to NULL
         lm = NULL;
      }
   }
   // Note that the newly created landmark is still
   // pushed onto the cleanupstack
   if (lm) {
      CleanupStack::PushL(lm);
   }
   return lm;
}

CPosLandmark* CWFLMSManager::GetLandmarkLC(TUint32 aLmId, 
                                           const char* aLmsUri,
                                           CPosLandmarkDatabase* aLms)
{
   // Convert the supplied char* to a descriptor and
   // use the other GetLandmarkLC that takes the lmsUri as 
   // descriptor to get the landmark
   TBuf<255> lmsUri;
   WFTextUtil::char2TDes(lmsUri, aLmsUri);
   // Note that the newly created landmark is still
   // pushed onto the cleanupstack
   return GetLandmarkLC(aLmId, lmsUri, aLms);
}

std::vector<Favorite*>* CWFLMSManager::GetLandmarkList()
{
   return iLandmarkList;
}

std::vector<GuiFavorite*>* CWFLMSManager::GetGuiLandmarkList()
{
   return iGuiLandmarkList;
}

Favorite* CWFLMSManager::CreateFavoriteL(const CPosLandmark& aLm, const TDesC& aLmsUri) 
{
   char* lmsUri = WFTextUtil::newTDesDupL(aLmsUri);
   Favorite* fav = CreateFavoriteL(aLm, lmsUri);
   delete[] lmsUri;
   return fav;
}

Favorite* CWFLMSManager::CreateFavoriteL(const CPosLandmark& aLm, const char* aLmsUri) 
{
   TInt err = 0;
   TPtrC tmp;
   TRAP(err, aLm.GetLandmarkName(tmp));
   char* name = WFTextUtil::newTDesDupL(tmp);
   TRAP(err, aLm.GetLandmarkDescription(tmp));
   char* desc = WFTextUtil::newTDesDupL(tmp);
   TLocality pos;
   TRAP(err, aLm.GetPosition(pos));
   TReal64 lat = pos.Latitude();
   TReal64 lon = pos.Longitude();
   MC2Coordinate mc2Coord;
   if (Math::IsNaN(lat) || Math::IsNaN(lon)) {
      //pos.SetCoordinate(MAX_INT32, MAX_INT32);
      mc2Coord.lat = MAX_INT32;
      mc2Coord.lon = MAX_INT32;
   } else {
      WGS84Coordinate wgs84Coord(pos.Latitude(), pos.Longitude());
      mc2Coord = wgs84Coord;
   }
   Nav2Coordinate nav2Coord(mc2Coord);
//    uint8* md5sum = Favorite::calcMd5Sum(name, desc, 
//                                         aLm.GetPosition().Latitude(),
//                                         aLm.GetPosition().Longitude());
   uint8* md5sum = NULL;
//    WGS84Coordinate coord(pos.Latitude(), pos.Longitude());
//    MC2Coordinate mc2Coord = coord;
   Favorite* fav = new (ELeave) Favorite(nav2Coord.nav2lat,
                                         nav2Coord.nav2lon,
                                         name, "", desc, "", "", false,
                                         aLm.LandmarkId(), iImeiCrcHex, aLmsUri, md5sum);

   // Transferrs ownership
   return fav;
}

GuiFavorite* CWFLMSManager::CreateGuiFavoriteL(const CPosLandmark& aLm, const TDesC& aLmsUri)
{
   char* lmsUri = WFTextUtil::newTDesDupL(aLmsUri);
   GuiFavorite* fav = CreateGuiFavoriteL(aLm, lmsUri);
   delete[] lmsUri;
   return fav;   
}

GuiFavorite* CWFLMSManager::CreateGuiFavoriteL(const CPosLandmark& aLm, const char* aLmsUri)
{
   // Return a created guiFavorite, the CreateFavoriteL call will return a 
   // allocated Favorite but since we use an auto_ptr we don't need to delete
   // it expicitly
   return std::auto_ptr<Favorite> (CreateFavoriteL(aLm, aLmsUri))->getGuiFavorite();
}

CPosLandmark* CWFLMSManager::CreateLandmarkL(const Favorite& aFavorite)
{
   CPosLandmark* lm = CreateLandmarkLC(aFavorite);
   CleanupStack::Pop(lm);
   // Return the newly created landmark, it is poped from the cleanupstack
   return lm;
}

CPosLandmark* CWFLMSManager::CreateLandmarkLC(const Favorite& aFavorite)
{
   CPosLandmark* lm = CPosLandmark::NewL();
   // Call UpdateLandmarkL to set the correct values in lm
   UpdateLandmarkL(aFavorite, *lm);
   // Return the newly created landmark, not that it is pushed onto the cleanupstack  
   return lm;
}

void CWFLMSManager::UpdateLandmarkL(const Favorite& aFavorite, CPosLandmark& aLm)
{
   TBuf<255> tmp;

   // Set the name
   WFTextUtil::char2TDes(tmp, aFavorite.getName());
   aLm.SetLandmarkNameL(tmp);

   // Set the description
   WFTextUtil::char2TDes(tmp, aFavorite.getDescription());
   aLm.SetLandmarkDescriptionL(tmp);

   // Set lat and lon
   Nav2Coordinate nav2Coord(aFavorite.getLat(), aFavorite.getLon());
   MC2Coordinate mc2Coord(nav2Coord);
   WGS84Coordinate wgs84Coord(mc2Coord);
   aLm.SetPositionL(TLocality(TCoordinate(wgs84Coord.latDeg, wgs84Coord.lonDeg), 0.0));
}

CPosLandmarkDatabase* CWFLMSManager::GetLandmarkStoreL(const TDesC& aLmsUri)
{
   // Check if the supplied lms uri is the wayfinder landmarkstore
   if (aLmsUri == *iWFLmsUri) {
      // Return iLandmarkStore, caller needs to be careful when handling
      // the return value since returning a member
      if (!iLandmarkStore) {
         OpenLandmarkStoreL(aLmsUri, EFalse);
      }
      return iLandmarkStore;
   }
   // Not the wayfinder landmark store, open and return the correct one
   return OpenLandmarkStoreInternalL(aLmsUri);   
}

CPosLandmarkDatabase* CWFLMSManager::GetLandmarkStoreL(const char* aLmsUri) 
{
   if (!aLmsUri) {
      return NULL;
   }
   TBuf<255> lmsUri;
   WFTextUtil::char2TDes(lmsUri, aLmsUri);
   return GetLandmarkStoreL(lmsUri);
}

void CWFLMSManager::InitGetAsyncL(enum TWFLMSFavTypes aFavTypes)
{
   iAsyncLmsListIndex = 0;
   if (!iAsyncLmsList) {
      iAsyncLmsList = new (ELeave) CDesCArrayFlat(10);
   } else {
      iAsyncLmsList->Reset();
   }
   if (aFavTypes == EWFLMSWFFavs) {
      // If we only wants to export from wf lms, append the wf lmsuri
      // to the iLmsList
      iAsyncLmsList->AppendL(*iWFLmsUri);
   } else {
      CPosLmDatabaseManager* dbManager = CPosLmDatabaseManager::NewL();
      CleanupStack::PushL(dbManager);
      // Get all lms from the lms manager
      iAsyncLmsList = dbManager->ListDatabasesLC();
      CleanupStack::Pop(iAsyncLmsList);
      CleanupStack::PopAndDestroy(dbManager);
   }
   if (aFavTypes == EWFLMSLMFavs) {
      // If we wants to export from all lms except from the wf lms, find
      // the wf lmsuri in the list and delete it
      TInt index = -1;
      if (iAsyncLmsList->Find(*iWFLmsUri, index) == 0) {
         iAsyncLmsList->Delete(index);
      }
   }
   iActiveOperation = EWFLMSGetLandmarksAsyncOpen;
   CompleteRequest(KErrNone);
}

void CWFLMSManager::OpenLandmarkStoreAsyncL()
{
   if (iAsyncLms) {
      // Delete the iAsyncLms since we are going to get a new lms below
      DeleteAndNullIfNotSame(iAsyncLms, iLandmarkStore);
      // Reset, new list will be fetched further down
      iAsyncLmList.Reset();
   }
   if (iAsyncLmsListIndex >= iAsyncLmsList->MdcaCount()) {
      // No more landmarks stores to process, we are finished
      iActiveOperation = EWFLMSGetLandmarksAsyncFinished;
      CompleteRequest(KErrNone);
      delete iAsyncLmsList;
      iAsyncLmsList = NULL;
      return;
   }
   // Get the lms that should be exported
   iAsyncLms = GetLandmarkStoreL(iAsyncLmsList->MdcaPoint(iAsyncLmsListIndex));

   if (!iAsyncLms) {
      // No lms
      iObserver.LmsError(EWFLMSLmsNotExists);
      iAsyncLmsListIndex++;
      iActiveOperation = EWFLMSGetLandmarksAsyncOpen;
      CompleteRequest(KErrNone);
      return;
   }

   // Set the partial read parameters
   CPosLmPartialReadParameters* part = CPosLmPartialReadParameters::NewLC();
   part->SetRequestedAttributes(CPosLandmark::ELandmarkName | CPosLandmark::EDescription |
                                CPosLandmark::EPosition | CPosLandmark::EIcon);
   // Set the read parameters to the database
   iAsyncLms->SetPartialReadParametersL(*part);
   CleanupStack::PopAndDestroy(part);

   // Set the index to zero since we are starting over with a new list 
   // of landmark ids
   iAsyncLmListIndex = 0;
   // Generate event so that the next lm in line will be processed
   iActiveOperation = EWFLMSLandmarksAsyncSearch;
   CompleteRequest(KErrNone);
}

void CWFLMSManager::PopulateItemIdArrayL()
{
   // Fetch itemIterator
   CPosLmItemIterator* itemIterator = NULL;
   itemIterator = iAsyncLms->LandmarkIteratorL(iSortPref);
   CleanupStack::PushL(itemIterator);
   itemIterator->Reset();

   // Fetch iItemIds
   TUint nbrOfItems = itemIterator->NumOfItemsL();
   if (nbrOfItems > 0) {
      TInt startIndex = 0;
      iItemIds.Reset();
      itemIterator->GetItemIdsL(iItemIds, startIndex, nbrOfItems);
      iActiveOperation = EWFLMSLandmarksAsyncRead;
      CompleteRequest(KErrNone);
   } else {
      // No matches found
      iAsyncLmsListIndex++;
      iActiveOperation = EWFLMSGetLandmarksAsyncOpen;
      CompleteRequest(KErrNone);
   }
   CleanupStack::PopAndDestroy(itemIterator);
}

void CWFLMSManager::ReadSomeLandmarksAsyncL()
{
    RArray<TPosLmItemId> subSetOfIds;
    CleanupClosePushL(subSetOfIds);

    // Create a subset of the found matches to read
    //    TInt nbrOfMatches = iAsyncLmList.Count();
    TInt nbrOfMatches = iItemIds.Count();
    TInt i = 0;
    while (i < iNrOfItemsToRead && iAsyncLmListIndex < nbrOfMatches) {
       subSetOfIds.Append(iItemIds[iAsyncLmListIndex]);
       iAsyncLmListIndex++;
       i++;
    }

    // Start reading subset of found matches
    iStatus = KRequestPending;
    iActiveOperation = EWFLMSLandmarksAsyncRead;
    CPosLmOperation* operation = iAsyncLms->PreparePartialLandmarksL(subSetOfIds);
    iLmOpWrapper->StartOperation(operation, iStatus);
    SetActive();

    CleanupStack::PopAndDestroy(&subSetOfIds);
}

void CWFLMSManager::FetchLandmarksLC()
{
   // Fetch the partially read landmarks.
   if (!iLmOpWrapper->LmOperationPtr()) {
      return;
   }
   CArrayPtr<CPosLandmark>* result = 
      iAsyncLms->TakePreparedPartialLandmarksL(iLmOpWrapper->LmOperationPtr());
   CleanupStack::PushL(result);
   
   for (TInt i = 0; i < result->Count(); ++i) {
      if (iAsyncGetGuiLandmarks) {         
         // User requested the landmarks as gui favorites
         iGuiLandmarkList->push_back(CreateGuiFavoriteL(*(result->At(i)),  
                                     iAsyncLmsList->MdcaPoint(iAsyncLmsListIndex)));         
      } else {
         // User requested the landmarks as favorites
         iLandmarkList->push_back(CreateFavoriteL(*result->At(i),
                                  iAsyncLmsList->MdcaPoint(iAsyncLmsListIndex)));         
      }
   }
   CleanupStack::PopAndDestroy(result);
}

void CWFLMSManager::RunL()
{
   if (iActiveOperation == EWFLMSInitializeDb) {
      if (iStatus == KPosLmOperationNotComplete) {
         // Report progress
         iStatus = KRequestPending;
         SetActive();
         iLmOpWrapper->ExecuteNextStep(iStatus);
      } else {
         iObserver.DatabaseInitialized();
      }
   } else if (iStatus == KErrNone) {
      if (iActiveOperation == EWFLMSGetLandmarksAsyncOpen) {
         // Start the async work, open a database
         OpenLandmarkStoreAsyncL();
      } else if (iActiveOperation == EWFLMSLandmarksAsyncSearch) {
         // Get the ids of landmarks available in the current database
         PopulateItemIdArrayL();
      } else if (iActiveOperation == EWFLMSLandmarksAsyncRead) {
         FetchLandmarksLC();
         if (iAsyncLmListIndex < iItemIds.Count()) {
            // There are still found landmarks to read
            ReadSomeLandmarksAsyncL();
         } else {
            iAsyncLmsListIndex++;
            iLmOpWrapper->ResetOperationPtr();
            OpenLandmarkStoreAsyncL();
         }
      } else if (iActiveOperation == EWFLMSGetLandmarksAsyncFinished) {
         if (iAsyncGetGuiLandmarks) {
            iObserver.GuiLandmarkImportCompleteL();
         } else {
            iObserver.LandmarkImportCompleteL();
         }
         iActiveOperation = EWFLMSNoOperation;
      }
   } else if (iStatus == KErrNotFound) {
      if (iActiveOperation == EWFLMSLandmarksAsyncSearch) {
         // This means that the current lms is empty,
         // go to the next one.
         iAsyncLmsListIndex++;
         OpenLandmarkStoreAsyncL();
      } else {
         iObserver.LmsError(iStatus.Int());
      }
   } else {
      iObserver.LmsError(iStatus.Int());         
   }
}

void CWFLMSManager::DoCancel()
{
   iLmOpWrapper->Cancel();
   DeleteAndNullIfNotSame(iAsyncLms, iLandmarkStore);
   iActiveOperation = EWFLMSNoOperation;
}

TBool CWFLMSManager::IsIdle()
{
   return ((iActiveOperation == EWFLMSNoOperation) && !IsActive());
}
