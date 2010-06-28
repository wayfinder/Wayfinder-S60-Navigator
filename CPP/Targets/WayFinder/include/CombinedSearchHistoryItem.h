/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMBINED_SEARCH_HISTORY_ITEM_H
#define COMBINED_SEARCH_HISTORY_ITEM_H

#include <deque>

#define SEARCH_HISTORY_VERSION_NUMBER 6

class SearchHistoryItem;

typedef std::deque<SearchHistoryItem*> SearchHistoryDeque_t;

class SearchHistoryItem 
{
public:

   SearchHistoryItem(const char* ss, const char* housenum,
                     const char* cis, const char* cii,
                     const char* cos, const char* coi,
                     const char* cai);
   ~SearchHistoryItem();

   bool operator==(const SearchHistoryItem& rhs) const;
   bool isDuplicate(const SearchHistoryItem *shi) const;

   char* GetSearchString()  { return m_ss; }
   char* GetHouseNum()      { return m_housenum; }
   char* GetCityString()    { return m_cis; }
   char* GetCityId()        { return m_cii; }
   char* GetCountryString() { return m_cos; }
   char* GetCountryId()     { return m_coi; }
   int  GetIntCountryId()   { return strtol(m_coi, NULL, 10); }
   char* GetCategoryId()    { return m_cai; }
   char* GetHistoryListEntry();

private:
   // Search string
   char* m_ss;

   // House number (not used in combined search)
   char* m_housenum;

   // City string
   char* m_cis;

   // City Id (not used in combined search)
   char* m_cii;

   // Country string
   char* m_cos;

   // Country Id
   char* m_coi;

   // Category Id (new for combined search history (ver.6))
   char* m_cai; 
};

#endif /* SEARCH_HISTORY_H */
