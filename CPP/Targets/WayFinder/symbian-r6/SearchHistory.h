/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_HISTORY_H
#define SEARCH_HISTORY_H

#include <deque>

#define SEARCH_HISTORY_VERSION_NUMBER 5

class SearchHistoryItem {
public:

   SearchHistoryItem(const char *ss, const char *housenum,
         const char *cis, const char *cii,
         const char *cos, const char *coi);
   ~SearchHistoryItem();
   bool isDuplicate(const SearchHistoryItem *shi);

   char *GetSearchString()  { return m_ss; }
   char *GetHouseNum()      { return m_housenum; }
   char *GetCityString()    { return m_cis; }
   char *GetCityId()        { return m_cii; }
   char *GetCountryString() { return m_cos; }
   char *GetCountryId()     { return m_coi; }
   int  GetIntCountryId()   { return strtol(m_coi, NULL, 10); }
   char *GetHistoryListEntry();

private:
   char *m_ss;
   char *m_housenum;
   char *m_cis;
   char *m_cii;
   char *m_cos;
   char *m_coi;
};

typedef std::deque<SearchHistoryItem*>     SearchHistoryContainer;


#endif /* SEARCH_HISTORY_H */
