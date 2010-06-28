/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef SEARCH_RECORD_H
#define SEARCH_RECORD_H

#include "arch.h"
#include "nav2util.h"
#include <string.h>

using namespace isab;

/**
 * Class that describes one search record for a combined search
 */
class SearchRecord 
{
 public:

   /**
    * Default constructor
    */
   SearchRecord( ) :
      m_regionId(0), 
      m_areaId(NULL),
      m_areaName(NULL), 
      m_searchString(NULL),
      m_categoryId(NULL),
      m_origLat(MAX_INT32), 
      m_origLon(MAX_INT32)
      {}
   
   /**
    * Constructro creating an instance of
    * this class
    * @param regionId, The region id
    * @param areaId, The area id
    * @param areaName, Name of the area
    * @param searchString, The search string for this search
    * @param categoryId, The id of the category
    * @param origLat, Latitude for this search
    * @param origLon, Longtitude for this search
    */
   SearchRecord( unsigned int regionId,
                 const char* areaId,
                 const char* areaName,
                 const char* searchString,
                 const char* categoryId,
                 int32 origLat = MAX_INT32,
                 int32 origLon = MAX_INT32 ) : 
      m_regionId( regionId ), 
      m_origLat( origLat ), 
      m_origLon( origLon ) 
   {
      // Copy, no worry if NULL result will dest
      // will be null if source is NULL
      m_areaId = strdup_new(areaId);
      m_areaName = strdup_new(areaName);
      m_searchString = strdup_new(searchString);
      m_categoryId = strdup_new(categoryId);
   }

   /**
    * Copy constructor, creates a copy of sr.
    * @param sr, the SearchRecord to be copied.
    */
   SearchRecord( const SearchRecord& sr) :
      m_regionId(sr.m_regionId),
      m_origLat( m_origLat ), 
      m_origLon( m_origLon )
   {
      m_areaId = strdup_new(sr.m_areaId);
      m_areaName = strdup_new(sr.m_areaName);
      m_searchString = strdup_new(sr.m_searchString);
      m_categoryId = strdup_new(sr.m_categoryId);
   }

   ~SearchRecord() {
      delete[] m_areaId;
      delete[] m_areaName;
      delete[] m_searchString;
      delete[] m_categoryId;
   }

   /**
    * Simple set function. Sets the area id.
    * @param areaId, the new area id.
    */
   void setAreaId(const char* areaId);

   /**
    * Simple set function. Sets the area name.
    * @param areaName, the new area name.
    */
   void setAreaName(const char* areaName);

   /**
    * Simple set function. Sets the search string.
    * @param searchString, the new search string.
    */
   void setSearchString(const char* searchString);

   /**
    * Simple set function. Sets the category id.
    * @param categoryId, the new category id.
    */
   void setCategoryId(const char* categoryId);

   /**
    * Ovarloading of assignment operator that handles
    * self assignment.
    * @param sr The searchRecord that should be copied.
    * @return The copied searchRecord, this that now contains
    *         the copied values.
    */
   SearchRecord& operator= (const SearchRecord& sr);

   /// Region id for this search
   uint32 m_regionId;
   /// Area id for this search
   char* m_areaId;
   /// Area name of this search
   char* m_areaName;
   /// Search string for this search
   char* m_searchString;
   /// Category id for this search
   char* m_categoryId;
   /// Latitude for this search
   int32 m_origLat;
   /// Longtitude for this search
   int32 m_origLon;
};

inline void SearchRecord::setAreaId(const char* areaId)
{
   if (m_areaId)
      // Delete if existing
      delete[] m_areaId;

   // Copy new area id
   m_areaId = strdup_new(areaId);
}

inline void SearchRecord::setAreaName(const char* areaName)
{
   if (m_areaName)
      // Delete if existing
      delete[] m_areaName;

   // Copy new area id
   m_areaName = strdup_new(areaName);   
}

inline void SearchRecord::setSearchString(const char* searchString)
{
   if (m_searchString) {
      // Delete if no null
      delete[] m_searchString;
   }

   // Copy new searchString
   m_searchString = strdup_new(searchString);
}

inline void SearchRecord::setCategoryId(const char* categoryId)
{
   if (m_categoryId) {
      // Delete if no null
      delete[] m_categoryId;
   }

   // Copy new searchString
   m_categoryId = strdup_new(categoryId);   
}

inline SearchRecord& SearchRecord::operator= (const SearchRecord& sr) 
{
   if( this == &sr ) {
      // If same object, just return this
      return *this;
   }
   // Delete if exisiting
   if(m_areaId) 
      delete[] m_areaId;
   if(m_areaName) 
      delete[] m_areaName;
   if(m_searchString) 
      delete[] m_searchString;
   if(m_categoryId)
      delete[] m_categoryId;

   // Copy all records
   m_regionId = sr.m_regionId; 
   m_areaId = strdup_new(sr.m_areaId);
   m_areaName = strdup_new(sr.m_areaName);
   m_searchString = strdup_new(sr.m_searchString);
   m_categoryId = strdup_new(sr.m_categoryId);

   m_origLat = sr.m_origLat;
   m_origLon = sr.m_origLon;
   return *this;
}
#endif
