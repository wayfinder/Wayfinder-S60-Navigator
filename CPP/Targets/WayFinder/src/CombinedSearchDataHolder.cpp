/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "CombinedSearchDataHolder.h"
#include "CombinedSearchDataTypes.h"
#include <memory>
#include <algorithm>

#define MAX_SEARCH_HISTORY_ITEMS 20

CombinedSearchDataHolder::CombinedSearchDataHolder() :
   m_advertHeadingTitle(NULL),
   m_normHeadingTitle(NULL)
{
}

CombinedSearchDataHolder::~CombinedSearchDataHolder()
{
   clearCombinedSearchCategories();
   clearCombinedSearchResults();
   clearCombinedSearchCategoryResults();
   clearCombinedSearchHistory();
   clearTopHitResults();
   delete[] m_advertHeadingTitle;
   delete[] m_normHeadingTitle;
}


void CombinedSearchDataHolder::setSearchRecord(const SearchRecord& aSearchRecord)
{
   m_searchRecord = aSearchRecord;
}

SearchRecord& CombinedSearchDataHolder::getSearchRecord()
{
   return m_searchRecord;
}

const std::map<uint32, CombinedSearchCategory *>&
CombinedSearchDataHolder::getCombinedSearchCategories() const
{
   return m_searchCategories;
}

const std::vector <CombinedSearchCategory *>&
CombinedSearchDataHolder::getCombinedSearchResults() const
{
   return m_searchResults;
}

const std::vector <CombinedSearchCategory *>& 
CombinedSearchDataHolder::getCombinedSearchCategoryResults() const
{
   return m_searchCategoryResults;
}

const CombinedSearchCategory*
CombinedSearchDataHolder::getCombinedSearchCategory(uint32 heading) const
{
   for( CSCatVec_t::const_iterator it = m_searchCategoryResults.begin();
        it != m_searchCategoryResults.end(); ++it ) {
      if ( (*it)->getHeadingNo() == heading ) {
         return (*it);
      }
   }
   return NULL;
}

void CombinedSearchDataHolder::clearCombinedSearchCategories()
{
   std::map<uint32, CombinedSearchCategory*>::iterator it = m_searchCategories.begin();
   while( it != m_searchCategories.end() ) {
      delete it->second;
      ++it;
   }
   m_searchCategories.clear();
}

void CombinedSearchDataHolder::clearCombinedSearchResults()
{
   std::vector<CombinedSearchCategory*>::iterator it = m_searchResults.begin();
   while( it != m_searchResults.end() ) {
      delete *it;
      ++it;
   }
   m_searchResults.clear();
}

void CombinedSearchDataHolder::clearCombinedSearchCategoryResults()
{
   std::vector<CombinedSearchCategory*>::iterator it = m_searchCategoryResults.begin();
   while( it != m_searchCategoryResults.end() ) {
      delete *it;
      ++it;
   }
   m_searchCategoryResults.clear();
}

void CombinedSearchDataHolder::clearTopHitResults()
{
   m_topHitResults.clear();
}

void CombinedSearchDataHolder::setCombinedSearchCategories( std::map
                                                            <uint32, CombinedSearchCategory *>& cats ) 
{
   clearCombinedSearchCategories();
   clearCombinedSearchResults();
   clearCombinedSearchCategoryResults();
   clearTopHitResults();
   m_searchCategories.swap( cats );
}

void CombinedSearchDataHolder::setSearchResults( int cat_no, 
                                                 uint32 typeOfHits,
                                                 uint32 totalNbrHits, 
                                                 uint32 topHits,
                                                 const char* advertHitTitle,
                                                 const char* normHitTitle,
                                                 std::vector<isab::SearchItem>& newres )
{
   // Find the category with heading number cat_no
   std::map<uint32, CombinedSearchCategory*>::const_iterator it = m_searchCategories.find( cat_no );
   if( it != m_searchCategories.end() ) {
      // Found a match, push back a newly created CombinedSearchCategory into 
      // the container holding all search results
      m_searchResults.push_back( new CombinedSearchCategory( *it->second ) );
      // Get the newly added CombinedSearchCategory
      CombinedSearchCategory* tmpCat = m_searchResults.back();
      // Set the type of hits for this category
      tmpCat->setTypeOfHits( typeOfHits );
      // Set total number of hits for the new category
      tmpCat->setTotalNbrHits( totalNbrHits );
      // Set number of top hits for this category
      tmpCat->setTopHits( topHits );
      // Set the title for advertisement hits
      tmpCat->setAdvertHitTile( advertHitTitle );
      // Set the title for normal hits
      tmpCat->setNormHitTitle( normHitTitle );
      // Add search results to the new category
      tmpCat->setResults( newres );
   }   
}

void CombinedSearchDataHolder::replaceSearchResults( int cat_no, 
                                                     uint32 typeOfHits,
                                                     uint32 totalNbrHits,
                                                     uint32 topHits,
                                                     const char* advertHitTitle,
                                                     const char* normHitTitle,
                                                     CSSIVec_t& newres) 
{
   CSCatVec_t::const_iterator it;
   for (it = m_searchResults.begin(); it != m_searchResults.end(); ++it) {
      // Iterate through the vector holding all search results
      if ( (*it)->getHeadingNo() == (uint32)cat_no ) {
         // Found the matching heading number, add the searchitem to the heading/category
         (*it)->setResults( newres );
         (*it)->setTotalNbrHits(totalNbrHits);
         (*it)->setTypeOfHits(typeOfHits);
         (*it)->setTopHits(topHits);
         (*it)->setAdvertHitTile(advertHitTitle);
         (*it)->setNormHitTitle(normHitTitle);
         break;
      }
   }
}

void CombinedSearchDataHolder::appendSearchResults( int cat_no, std::vector<isab::SearchItem>& newres )
{
   CSCatVec_t::const_iterator it;
   for (it = m_searchResults.begin(); it != m_searchResults.end(); ++it) {
      // Iterate through the vector holding all search results
      if ( (*it)->getHeadingNo() == (uint32)cat_no ) {
         // Found the matching heading number, add the searchitem to the heading/category
         (*it)->appendResults( newres );
         break;
      }
   }
}

void CombinedSearchDataHolder::setSearchCategoryResults( int cat_no,
                                                         uint32 typeOfHits,
                                                         uint32 totalNbrHits,
                                                         uint32 topHits )
{
   // Find the category with heading number cat_no
   std::map<uint32, CombinedSearchCategory*>::const_iterator it = m_searchCategories.find( cat_no );
   if( it != m_searchCategories.end() ) {
      // Found a match, push back a newly created CombinedSearchCategory into 
      // the container       
      m_searchCategoryResults.push_back( new CombinedSearchCategory( *it->second ) );
      // Set total number of hits for the new category
      m_searchCategoryResults.back()->setTotalNbrHits( totalNbrHits );
      // Set the type of hits in this heading
      m_searchCategoryResults.back()->setTypeOfHits( typeOfHits );
      // Set top of hits for the new category
      m_searchCategoryResults.back()->setTopHits( topHits );
   }   
}

void CombinedSearchDataHolder::replaceSearchCategoryResults( int cat_no,
                                                             uint32 typeOfHits,
                                                             uint32 totalNbrHits,
                                                             uint32 topHits)
{
   CSCatVec_t::const_iterator it;
   for (it = m_searchCategoryResults.begin(); it != m_searchCategoryResults.end(); ++it) {
      // Iterate through the vector holding all search results
      if ( (*it)->getHeadingNo() == (uint32)cat_no ) {
         // Found the matching heading number, add the searchitem to the heading/category
         (*it)->setTypeOfHits( typeOfHits );
         (*it)->setTotalNbrHits( totalNbrHits );
         (*it)->setTopHits( topHits );
         break;
      }
   }  
}

void 
CombinedSearchDataHolder::clearCombinedSearchHistory()
{
   std::deque<SearchHistoryItem*>::iterator it = m_searchHistory.begin();
   while( it != m_searchHistory.end() ) {
      delete *it;
      ++it;
   }
   m_searchHistory.clear();
}

const std::deque<SearchHistoryItem*>& 
CombinedSearchDataHolder::getSearchHistory() const
{
   return m_searchHistory;
}

void 
CombinedSearchDataHolder::addSearchHistoryItem(SearchHistoryItem* shItem)
{
   // find an iterator if we have a duplicate entry already
   std::deque<SearchHistoryItem*>::iterator it = 
      std::find_if(m_searchHistory.begin(), m_searchHistory.end(), 
                   std::bind2nd( std::mem_fun(&SearchHistoryItem::isDuplicate),
                                 shItem ) );
   if (it != m_searchHistory.end()) {
      // if we found a duplicate search history item delete it
      delete *it;
      m_searchHistory.erase(it);
   }
   // add / re-add the search history item first in the queue
   m_searchHistory.push_front(shItem);

   if (m_searchHistory.size() > MAX_SEARCH_HISTORY_ITEMS) {
      // remove the last search history item if we have more than allowed
      SearchHistoryItem* tmp = m_searchHistory.back();
      m_searchHistory.pop_back();
      delete tmp;
   }
}

void 
CombinedSearchDataHolder::addSearchHistoryItem(const char* ss, 
                                               const char* cis, 
                                               const char* cos, 
                                               const char* coi, 
                                               const char* cai)
{
   SearchHistoryItem* shi = 
      new SearchHistoryItem(ss, "", cis, "", cos, coi, cai);
   addSearchHistoryItem(shi);
}

SearchHistoryItem* 
CombinedSearchDataHolder::getSearchHistoryItem(int index)
{
   if (index < int(m_searchHistory.size())) {
      return m_searchHistory[index];
   }
   return NULL;
}

uint32 
CombinedSearchDataHolder::getNumSearchHistoryItems()
{
   return m_searchHistory.size();
}

void
CombinedSearchDataHolder::setAdvertHeadingTitle(const char* advertHeadingTitle)
{
   if (m_advertHeadingTitle) {
      delete[] m_advertHeadingTitle;
   }
   m_advertHeadingTitle = strdup_new(advertHeadingTitle);
}

void
CombinedSearchDataHolder::setNormHeadingTitle(const char* normHeadingTitle)
{
   if (m_normHeadingTitle) {
      delete[] m_normHeadingTitle;
   }
   m_normHeadingTitle = strdup_new(normHeadingTitle);
}

const char*
CombinedSearchDataHolder::getAdvertHeadingTitle() const
{
   return m_advertHeadingTitle;
}

const char*
CombinedSearchDataHolder::getNormHeadingTitle() const
{
   return m_normHeadingTitle;
}

void 
CombinedSearchDataHolder::setTopHitResults( CSSIVec_t& siVec )
{
   clearTopHitResults();
   m_topHitResults.reserve( siVec.size() );
   m_topHitResults.assign( siVec.begin(), siVec.end() );
}

const std::vector<isab::SearchItem>& 
CombinedSearchDataHolder::getTopHitResults() const
{
   return m_topHitResults;
}
