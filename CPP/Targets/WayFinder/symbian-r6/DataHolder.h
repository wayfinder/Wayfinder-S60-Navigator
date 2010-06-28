/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATAHOLDER_H
#define DATAHOLDER_H

// INCLUDES
#include "BufferArray.h"
#include <deque>

// CLASS DECLARATION
class CBufferArray;
class CBufferElement;
class isab::Log;

namespace isab{
   class SearchRegion;
}

/**
* Message handling class.
* 
*/
class CDataHolder : public CBase
{

public: // // Constructors and destructor

   CDataHolder(isab::Log* aLog);

   /**
    * EPOC default constructor.
    */
   void ConstructL();

   /**
    * Destructor.
    */
   ~CDataHolder();

public: // New functions

   void InitCountries();

   TInt GetNbrCountries();

   HBufC* GetCountry( TInt aIdx );

   TUint GetCountryId( TInt aIdx );

   /**
    * Returns the country string using the id instead of the
    * index. If the id is not present, NULL is returned.
    *
    * @param aCountryId The id of the country to retreive
    *                   the name of, not to be mixed up with
    *                   country index.
    */
   HBufC* GetCountryFromId( TUint aCountryId );

   void AddCountryL( TDesC &aCountry, TUint aId );

   void RemoveCountry( TInt aIdx );

   void SetSelectedCountry( TInt aIdx );

   /**
    * @return The index of the selected country.
    */
   TUint GetSelectedCountry();

   /**
    * Set the selection to the item with the  id specified 
    * in countryId
    *
    * @param aCountryId The id of the country to select, not
    *                   to be mixed up with index in the list.
    *
    * @return Returns true if the country id was present in the 
    *         list, otherwise false.
    */
   bool SetSelectedCountryId( TInt aCountryId );

   TUint GetSelectedCountryId();

   void InitCategories();

   TInt GetNbrCategories();

   HBufC* GetCategory( TInt aIdx );

   void SetCategoryIds( int32 numEntries, const char** data);
   void SetCategoryStrings( int32 numEntries, const char** data);
   const char* GetCategoryId( TInt aIdx );

   void InitNewCities();

   TInt GetNbrNewCities();

   HBufC* GetNewCity( TInt aIdx );
   HBufC* GetNewCityId( TInt aIdx );

   TInt AddNewCityL( TDesC &aCity );
   TInt AddNewCityIdL( TDesC &Id );

   void RemoveNewCity( TInt aIdx );

   TBool IsDuplicateNewCity(const TDesC &aCity );

   void SetSelectedNewCity( TUint aIdx );

   TUint GetSelectedNewCity();

   void InitNewDests();

   void SetSearchTotalHits(TInt aTotalHits);
   void SetSearchStartIndex(TInt aStartIndex);

   TInt GetSearchTotalHits(void);
   TInt GetSearchStartIndex(void);

   TInt GetNbrNewDests();

   HBufC* GetNewDestId( TInt aIdx );
   HBufC* GetNewDest( TInt aIdx );
   TPoint GetNewDestPos( TInt aIdx);

   void AddNewDestL( TDesC &aDest );
   void AddNewDestIdL( TDesC &aDest );
   void AddNewDestPosL( TInt32 lat, TInt32 lon );

   void RemoveNewDest( TInt aIdx );

   /**
    * Get the stringname of the region type.
    * Allocates a heap descriptor!
    * (used to be in NewDestView but now that is replaced by combined search)
    */
   static HBufC* GetRegionTypeName(const isab::SearchRegion* region);

   void SetInputData(TInt32 lat, TInt32 lon,
         TInt heading, TInt speed, TInt alt);
   void GetInputData(TInt32& lat, TInt32& lon,
         TInt& heading, TInt& speed, TInt& alt);

   void SetRouteData( TInt32 DTG, TInt32 ETG, TInt speedLimit);
   void GetRouteData( TInt32& DTG, TInt32& ETG, TInt& speedLimit);

private: //Data

   CBufferArray* iCountries;
   RArray<TUint> iCountryIds;


   CBufferArray* iCategories;
   char**        iCategoryIds;
   TInt          iSelectedCategory;
   int32         iCategoryIdNum;

   CBufferArray* iNewCities;
   CBufferArray* iNewCityIds;

   CBufferArray* iNewDests;
   CBufferArray* iNewDestIds;

   typedef std::deque<TPoint> PositionContainer; 
   typedef PositionContainer::iterator PositionIterator;
   PositionContainer iNewDestPositions; 

   TInt          iSearchTotalHits;
   TInt          iSearchStartIndex;

   TUint         iSelectedCountry;

   TUint         iSelectedNewCity;

   TInt          iLastAlt;
   TInt          iLastSpeed;
   TInt          iLastHeading;
   TInt32        iLastLat;
   TInt32        iLastLon;

   TInt32        iDistanceToGoal;
   TInt32        iEstimatedTimeToGoal;
   TInt          iSpeedLimit;

   isab::Log*     iLog;

   TUint          iCurrentCountryId;
   HBufC*         iCurrentCityId;
   HBufC*         iCurrentCountryString;
   HBufC*         iCurrentCityString;
   HBufC*         iCurrentSearchString;

};

#endif

// End of File
