/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES
#include <coemain.h>

#include <arch.h>

#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/SearchRegion.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtSearchMess.h"

#include "RsgInclude.h"
#include "wayfinder.hrh"

#include "BufferArray.h"
#include "memlog.h"
#include "DataHolder.h"
#include "nav2util.h"
#include "WFTextUtil.h"

using namespace isab;

// ================= MEMBER FUNCTIONS =======================
//

// ----------------------------------------------------
// CDataHolder::CDataHolder()
// Constructor
// Frees reserved resources
// ----------------------------------------------------
//
CDataHolder::CDataHolder(isab::Log* aLog) : iLog(aLog)
{
}

// ----------------------------------------------------------
// CDataHolder::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CDataHolder::ConstructL()
{

/*    iCountries = NULL; */
   iSelectedCountry = MAX_UINT32;
/*    iCategories = NULL; */
/*    iCategoryIds = NULL; */
/*    iCategoryIdNum = 0; */
/*    iSelectedCategory = 0; */
/*    iNewCities = NULL; */
/*    iNewCityIds = NULL; */
   iSelectedNewCity = MAX_INT32;
/*    iNewDests = NULL; */
/*    iNewDestIds = NULL; */
/*    iSearchTotalHits = 0; */
/*    iSearchStartIndex = 0; */
/*    iLastAlt = 0; */
   iLastSpeed = -1;
   iLastHeading = -1;
   iLastLat = MAX_INT32;
   iLastLon = MAX_INT32;
/*    iDistanceToGoal = 0; */
/*    iEstimatedTimeToGoal = 0; */
/*    iSpeedLimit = 0; */

   InitNewCities();
}

// ----------------------------------------------------
// CDataHolder::~CDataHolder()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CDataHolder::~CDataHolder()
{
   LOGDEL(iCountries);
   delete iCountries;
   LOGDEL(iCategories);
   delete iCategories;
   if (iCategoryIds != NULL) {
      for (int32 i = 0; i < iCategoryIdNum; i++) {
         LOGDELA(iCategoryIds[i]);
         delete[] iCategoryIds[i];
      }
      LOGDELA(iCategoryIds);
      delete[] iCategoryIds;
      iCategoryIds = NULL;
      iCategoryIdNum = 0;
   }
   LOGDEL(iNewCities);
   delete iNewCities;
   LOGDEL(iNewCityIds);
   delete iNewCityIds;
   LOGDEL(iNewDests);
   delete iNewDests;
   LOGDEL(iNewDestIds);
   delete iNewDestIds;
}

void CDataHolder::InitCountries()
{
   if( iCountries == NULL ){
      iCountries = new (ELeave) CBufferArray(iLog);
      LOGNEW(iCountries, CBufferArray);
   }
   iCountries->InitArray();
   iCountryIds.Reset();
   iSelectedCountry = MAX_UINT32;
}

TInt CDataHolder::GetNbrCountries()
{
   if( iCountries == NULL )
      return 0;
   else
      return iCountries->GetNbrElements();
}

HBufC* CDataHolder::GetCountry( TInt aIdx )
{
   return iCountries->GetElementAt( aIdx );
}

TUint CDataHolder::GetCountryId( TInt aIdx )
{
   return iCountryIds[ aIdx ];
}

HBufC* CDataHolder::GetCountryFromId( TUint aCountryId )
{
   HBufC* result = NULL;

   TInt index = iCountryIds.Find(aCountryId);
   if( index != KErrNotFound ){
      result = GetCountry( index );
   }
   return result;
}

void CDataHolder::AddCountryL( TDesC &aCountry, TUint aId )
{
   iCountries->AddElementL( aCountry , iLog);
   iCountryIds.Append( aId );
}

void CDataHolder::RemoveCountry( TInt aIdx )
{
   iCountries->RemoveElement( aIdx );
}

void CDataHolder::SetSelectedCountry( TInt aIdx )
{
   iSelectedCountry = aIdx;
}

TUint CDataHolder::GetSelectedCountry()
{
   return iSelectedCountry;
}

TUint CDataHolder::GetSelectedCountryId()
{
   if( iSelectedCountry == MAX_UINT32 || 
       TUint(iCountryIds.Count()) <= iSelectedCountry ){
      return MAX_UINT32;
   } else {
      return iCountryIds[iSelectedCountry];
   }
}

bool CDataHolder::SetSelectedCountryId( TInt aCountryId )
{
   bool result = false;

   TInt index = iCountryIds.Find(aCountryId);
   if( index != KErrNotFound ){
      SetSelectedCountry( index );
      result = true;
   }
   else{
      // Could not find the country, select current country.
      SetSelectedCountry(0);
   }
   return result;
}

void CDataHolder::InitCategories()
{
   if (iCategoryIds != NULL) {
      for (int32 i = 0; i < iCategoryIdNum; i++) {
         LOGDELA(iCategoryIds[i]);
         delete[] iCategoryIds[i];
      }
      LOGDELA(iCategoryIds);
      delete[] iCategoryIds;
      iCategoryIds = NULL;
      iCategoryIdNum = 0;
   }
   if( iCategories == NULL ){
      iCategories = new (ELeave) CBufferArray(iLog);
      LOGNEW(iCategories, CBufferArray);
   }
   iCategories->InitArray();
}

TInt CDataHolder::GetNbrCategories()
{
   if( iCategories == NULL )
      return 0;
   else
      return iCategories->GetNbrElements();
}

HBufC*
CDataHolder::GetCategory( TInt aIdx )
{
   return iCategories->GetElementAt( aIdx );
}
const char*
CDataHolder::GetCategoryId( TInt aIdx )
{
   if ((aIdx >= iCategoryIdNum) || (aIdx < 0)) {
      return NULL;
   }
   return iCategoryIds[aIdx];
}

void
CDataHolder::SetCategoryIds( int32 numEntries, const char ** data)
{
   if (iCategoryIds != NULL) {
      for (int32 i = 0; i < iCategoryIdNum; i++) {
         LOGDELA(iCategoryIds[i]);
         delete[] iCategoryIds[i];
      }
      LOGDELA(iCategoryIds);
      delete[] iCategoryIds;
      iCategoryIds = NULL;
   }
   iCategoryIdNum = 0;

   iCategoryIds = new (ELeave) char*[numEntries];
   LOGNEWA(iCategoryIds, char*, numEntries);
   for (int32 i = 0; i < numEntries; i++) {
      iCategoryIds[i] = isab::strdup_new(data[i]);
      LOGNEWA(iCategoryIds[i], char, strlen(iCategoryIds[i]) + 1);
   }
   iCategoryIdNum = numEntries;
}
void
CDataHolder::SetCategoryStrings( int32 numEntries, const char** data)
{
   if( iCategories == NULL ){
      iCategories = new (ELeave) CBufferArray(iLog);
      LOGNEW(iCategories, CBufferArray);
   }
   iCategories->InitArray();
   iSelectedCategory = 0;
   TBuf<256> temp;
   for  (int32 i = 0; i < numEntries; i++) {
      WFTextUtil::char2TDes(temp, data[i]);
      iCategories->AddElementL(temp, iLog);
   }  
}

void CDataHolder::InitNewCities()
{
   if( iNewCities == NULL ){
      iNewCities = new (ELeave) CBufferArray(iLog);
      LOGNEW(iNewCities, CBufferArray);
   }
   iNewCities->InitArray();
   iSelectedNewCity = MAX_INT32;
   if( iNewCityIds == NULL ){
      iNewCityIds = new (ELeave) CBufferArray(iLog);
      LOGNEW(iNewCityIds, CBufferArray);
   }
   iNewCityIds->InitArray();
}

TInt CDataHolder::GetNbrNewCities()
{
   if( iNewCities == NULL )
      return 0;
   else
      return iNewCities->GetNbrElements();
}

HBufC* CDataHolder::GetNewCity( TInt aIdx )
{
   return iNewCities->GetElementAt( aIdx );
}

HBufC* CDataHolder::GetNewCityId( TInt aIdx )
{
   return iNewCityIds->GetElementAt( aIdx );
}

TInt CDataHolder::AddNewCityL( TDesC &aCity )
{
   TInt save = iNewCities->GetNbrElements();
   iNewCities->AddElementL( aCity , iLog);
   return save;
}

TInt CDataHolder::AddNewCityIdL( TDesC &Id )
{
   TInt save = iNewCities->GetNbrElements();
   iNewCityIds->AddElementL( Id , iLog);
   return save;
}

void CDataHolder::RemoveNewCity( TInt aIdx )
{
   iNewCities->RemoveElement( aIdx );
   iNewCityIds->RemoveElement( aIdx );
}

TBool CDataHolder::IsDuplicateNewCity(const TDesC &aCity )
{
   if( iNewCities == NULL )
      return EFalse;
   else
      return iNewCities->IsDuplicate( aCity );
}

void CDataHolder::SetSelectedNewCity( TUint aIdx )
{
   iSelectedNewCity = aIdx;
}

TUint CDataHolder::GetSelectedNewCity()
{
   return iSelectedNewCity;
}

void CDataHolder::InitNewDests()
{
   if( iNewDests == NULL ){
      iNewDests = new (ELeave) CBufferArray(iLog);
      LOGNEW(iNewDests, CBufferArray);
   }
   iNewDests->InitArray();

   if( iNewDestIds == NULL ){
      iNewDestIds = new (ELeave) CBufferArray(iLog);
      LOGNEW(iNewDestIds, CBufferArray);
   }  
   iNewDestIds->InitArray();
   iNewDestPositions.clear();
   iSearchTotalHits = 0;
   iSearchStartIndex = 0;
}


TInt CDataHolder::GetNbrNewDests()
{
   if( iNewDests == NULL )
      return 0;
   else
      return iNewDests->GetNbrElements();
}

HBufC* CDataHolder::GetNewDestId( TInt aIdx )
{
   return iNewDestIds->GetElementAt( aIdx );
}

HBufC* CDataHolder::GetNewDest( TInt aIdx )
{
   return iNewDests->GetElementAt( aIdx );
}

TPoint CDataHolder::GetNewDestPos(TInt aIdx)
{
   size_t idx = aIdx;
   if(idx < iNewDestPositions.size()){
      return iNewDestPositions[aIdx];
   } 
   return TPoint(MAX_INT32, MAX_INT32);
}

void CDataHolder::AddNewDestL( TDesC &aDest )
{
   iNewDests->AddElementL( aDest , iLog);
}

void CDataHolder::AddNewDestIdL( TDesC &aDest )
{
   iNewDestIds->AddElementL( aDest , iLog);
}

void CDataHolder::AddNewDestPosL( TInt32 lat, TInt32 lon )
{
   iNewDestPositions.push_back(TPoint(lat, lon));
}

void CDataHolder::RemoveNewDest( TInt aIdx )
{
   iNewDests->RemoveElement( aIdx );
   iNewDestIds->RemoveElement( aIdx );
   if(size_t(aIdx) < iNewDestPositions.size()){
      PositionIterator p = iNewDestPositions.begin();
      std::advance(p, aIdx);
      iNewDestPositions.erase(p);
   }
}

HBufC* CDataHolder::GetRegionTypeName(const SearchRegion* region)
{
   HBufC *aName;
   if (!region) {
      return NULL;
   }
   TInt aResourceId = 0;

   switch(region->getType()){
   case GuiProtEnums::streetNumber:
      aResourceId = R_TEXT_SEARCH_DETAIL_01;
      break;
   case GuiProtEnums::address:
#ifdef USE_ADDRESS_FROM_SEARCH_REPLY
      aResourceId = R_TEXT_SEARCH_DETAIL_02;
#else
      return NULL;
#endif
      break;
   case GuiProtEnums::cityPart:
      aResourceId = R_TEXT_SEARCH_DETAIL_03;
      break;
   case GuiProtEnums::city:
      aResourceId = R_TEXT_SEARCH_DETAIL_04;
      break;
   case GuiProtEnums::municipal:
      aResourceId = R_TEXT_SEARCH_DETAIL_05;
      break;
   case GuiProtEnums::country:
      aResourceId = R_TEXT_SEARCH_DETAIL_06;
      break;
   case GuiProtEnums::state:
      aResourceId = R_TEXT_SEARCH_DETAIL_07;
      break;
   case GuiProtEnums::zipcode:
      aResourceId = R_TEXT_SEARCH_DETAIL_08;
      break;
   default:
      aResourceId = R_TEXT_SEARCH_DETAIL_09;
      break;
   }
   aName = CCoeEnv::Static()->AllocReadResourceL( aResourceId );
   return aName;
}

void
CDataHolder::SetInputData( TInt32 lat, TInt32 lon,
      TInt heading, TInt speed, TInt alt )
{
   iLastLat = lat;
   iLastLon = lon;
   iLastAlt = alt;
   if (heading > 0) {
      iLastHeading = heading;
   }
   if (speed > 0) {
      iLastSpeed = speed;
   }
}

void
CDataHolder::GetInputData(TInt32& lat, TInt32& lon,
      TInt& heading, TInt& speed, TInt& alt )
{
   lat = iLastLat;
   lon = iLastLon;
   heading = iLastHeading;
   speed = iLastSpeed;
   alt = iLastAlt;
}

void
CDataHolder::SetRouteData( TInt32 DTG, TInt32 ETG, TInt speedLimit)
{
   iDistanceToGoal = DTG;
   iEstimatedTimeToGoal = ETG;
   iSpeedLimit = speedLimit;
}

void
CDataHolder::GetRouteData( TInt32& DTG, TInt32& ETG, TInt& speedLimit)
{
   DTG = iDistanceToGoal;
   ETG = iEstimatedTimeToGoal;
   speedLimit = iSpeedLimit;
}

void
CDataHolder::SetSearchTotalHits(TInt aTotalHits)
{
   iSearchTotalHits = aTotalHits;
}
void
CDataHolder::SetSearchStartIndex(TInt aStartIndex)
{
   iSearchStartIndex = aStartIndex;
}

TInt
CDataHolder::GetSearchTotalHits()
{
   return iSearchTotalHits;
}
TInt
CDataHolder::GetSearchStartIndex()
{
   return iSearchStartIndex;
}

// End of File  
