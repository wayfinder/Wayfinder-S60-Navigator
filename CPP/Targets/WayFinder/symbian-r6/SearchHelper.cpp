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
#include <e32base.h>
#include "WFTextUtil.h"
#include "DataHolder.h"
#include "WayFinderAppUi.h"
#include "SearchHelper.h"
#include "RsgInclude.h"
/* #include "Nav2UiProtHandler.h" */

#include "nav2util.h"

using namespace isab;

TBool
CSearchHelper::IsCurrentCountryL(const TDesC &countryString )
{
   return WFTextUtil::MatchesResourceStringL(countryString, R_WAYFINDER_FROM_GPS_TEXT);
}
TBool
CSearchHelper::IsCurrentCityL(const TDesC &cityString )
{
   return WFTextUtil::MatchesResourceStringL(cityString, R_WAYFINDER_FROM_GPS_TEXT);
}
TBool
CSearchHelper::IsSelectCountryL(const TDesC &countryString )
{
   return WFTextUtil::MatchesResourceStringL(countryString, R_WAYFINDER_SELECT_COUNTRY_TEXT);
}
TBool
CSearchHelper::IsSelectCityL(const TDesC &cityString )
{
   return WFTextUtil::MatchesResourceStringL(cityString, R_WAYFINDER_SELECT_CITY_TEXT);
}

TBool
CSearchHelper::IsCountryOkL(const TDesC& aCountry)
{
   if( aCountry.Length() > 0 && !IsSelectCountryL( aCountry )) {
      return (ETrue);
   }
   return (EFalse);
}
TBool
CSearchHelper::IsCityOkL(const TDesC& aCity)
{
   if( aCity.Length() > 0 && !IsSelectCityL( aCity )) {
      return (ETrue);
   }
   return (EFalse);
}

void
CSearchHelper::SaveHistoryL(CWayFinderAppUi* aWayFinderUI,
                            const TDesC& searchString,
                            const TDesC& houseNum,
                            const TDesC& searchCity,
                            const TDesC& cityId,
                            const TDesC& searchCountry,
                            TInt countryId)
{
   char *ss = WFTextUtil::TDesCToUtf8L(searchString);
   CleanupStack::PushL(ss);
   char *hn = WFTextUtil::TDesCToUtf8L(houseNum);
   CleanupStack::PushL(ss);
   char *cis;
   if ( IsCurrentCityL(searchCity)) {
      cis = strdup_new("");
   } else {
      cis = WFTextUtil::TDesCToUtf8L(searchCity);
   }
   CleanupStack::PushL(cis);

   /* We cannot save area ID until the search request can handle */
   /* request where the ID may be wrong. */
/*          char *cii = WFTextUtil::TDesCToUtf8L(cityId); */
/*    CleanupStack::PushL(cii); */
   char *cii = ""; /* Placeholder. */

   char *cos;
   if ( IsCurrentCountryL(searchCountry)) {
      cos = strdup_new("");
   } else {
      cos = WFTextUtil::TDesCToUtf8L(searchCountry);
   }
   CleanupStack::PushL(cos);

   /* Allocates a string. */
   char *coi = WFTextUtil::uint32AsStringL(countryId);
   CleanupStack::PushL(coi);

   CDataHolder* dataHolder = aWayFinderUI->GetDataHolder();
   dataHolder->AddSearchHistoryItemL(ss, hn, cis, cii, cos, coi);

   CleanupStack::PopAndDestroy(coi);
   CleanupStack::PopAndDestroy(cos);
/*    CleanupStack::PopAndDestroy(cii); */
   CleanupStack::PopAndDestroy(cis);
   CleanupStack::PopAndDestroy(hn);
   CleanupStack::PopAndDestroy(ss);

   aWayFinderUI->HandleSaveSearchHistoryL( );
}

TInt CSearchHelper::SearchL(CWayFinderAppUi* aWayFinderUI,
                            const TDesC& searchString,
                            const TDesC& houseNum,
                            const TDesC& searchCity,
                            const TDesC& searchCountry,
                            TInt searchCategory,
                            TBool categorySelected)
{
   CDataHolder* dataHolder = aWayFinderUI->GetDataHolder();

   // Defaults use current country and city.
   TUint selCountry = MAX_UINT32;
   TUint selCity = MAX_UINT32;

   if( !IsCurrentCountryL( searchCountry ) ){
      selCountry = dataHolder->GetSelectedCountryId();
      if (selCountry == MAX_UINT32) {
         /* Country selection problem. */
         return ECountrySelectionProblem;
         /* XXX */
/*             ResetCountryToDefault(aWayFinderUI->IsGpsAllowed()); */
      }
   }

   HBufC* searchCityString = NULL;
   HBufC* searchCityId = NULL;

   if(IsCurrentCityL(searchCity)) {
      searchCityString = KNullDesC().AllocLC();
      searchCityId = KNullDesC().AllocLC();
   } else {
      selCity = dataHolder->GetSelectedNewCity();
      // Search using city string.
      TInt pos = searchCity.Find(_L(","));
      if (pos != KErrNotFound) {
         /* Found a comma. */
         searchCityString = searchCity.Left(pos).AllocL();
      } else {
         /* No comma, use the string as it is. */
         searchCityString = searchCity.AllocL();
      }
      if( selCity > TUint( dataHolder->GetNbrNewCities() ) ||
          !dataHolder->IsDuplicateNewCity( searchCity ) ){
         // New search, not a selected string from the list
         // of cities.
         selCity = MAX_UINT32;
         searchCityId = KNullDesC().AllocLC();
      } else {
         // Get the id string for the selected city
         // from the dataholder.
         searchCityId = dataHolder->GetNewCityId(selCity)->AllocLC();
      }
   }

   /*** Set coordinates correctly. ***/
   TInt32 lat;
   TInt32 lon = MAX_INT32;

   if (IsCurrentCountryL(searchCountry) || IsCurrentCityL(searchCity)) {
      /* Coordinate is needed. */
      lat = MAX_INT32;
   } else {
      /* Latitude is set to MAX_INT32 - 1 to avoid */
      /* coordinate refresh in NavServerCom. */
      /* We don't want the coordinate to be sent to server. */
      /* Results ordered from coordinate. */
      lat = MAX_INT32 - 1;
   }
   /* If we do have a category search, then we should use the */
   /* coordinate from the current origin (if available). */

   /*** Send search message to Nav2 ***/

   if (categorySelected &&
      (searchString.Compare(*(dataHolder->GetCategory(searchCategory))) == 0)){
      /* Category search, don't save in history. */

      const char *cat_id = dataHolder->GetCategoryId(searchCategory);
      char *searchCategoryString = new (ELeave) char[strlen(cat_id)+2];
      searchCategoryString[0] = 0x18;
      strcpy(&searchCategoryString[1], cat_id);

      uih->SendCategorySearchMessL(aWayFinderUI,
            searchCategoryString,
            *searchCityString,
            *searchCityId,
            selCountry, lat, lon);
      delete[] searchCategoryString;
   } else {
      HBufC* tmp = HBufC::NewLC(searchString.Length()+2+houseNum.Length());
      tmp->Des().Copy(searchString);
      if (houseNum.Length() > 0) {
         tmp->Des().Append(_L(" "));
         tmp->Des().Append(houseNum);
      }
      /* Normal search. Save the data for later use. */
      SaveHistoryL(aWayFinderUI,
            searchString,
            houseNum,
            searchCity,
            *searchCityId,
            searchCountry,
            selCountry);

      uih->SendSearchMessL(aWayFinderUI,
            *tmp,
            *searchCityString,
            *searchCityId,
            selCountry, lat, lon);
      CleanupStack::PopAndDestroy(tmp);
   }

   CleanupStack::PopAndDestroy(searchCityId);
   CleanupStack::PopAndDestroy(searchCityString);

   return ENoProblem;
}

