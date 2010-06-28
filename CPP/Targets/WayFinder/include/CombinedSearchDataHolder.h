/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef COMBINED_SEARCH_DATA_HOLDER_H
#define COMBINED_SEARCH_DATA_HOLDER_H

#include "arch.h"
#include <vector>
#include <map>
#include <deque>
#include "SearchRecord.h"
#include "GuiProt/SearchItem.h"
#include "CombinedSearchHistoryItem.h"

class CombinedSearchCategory;

typedef std::vector<CombinedSearchCategory*> CSCatVec_t;
typedef std::vector<isab::SearchItem> CSSIVec_t;

/**
 * Global data holder that stores all search data.
 * class CombinedSearchDataHolder
 */
class CombinedSearchDataHolder
{
 public:

   /**
    * Constructor, empty for now
    */
   CombinedSearchDataHolder();

   /**
    * Deletes all member containers and its content
    */
   virtual ~CombinedSearchDataHolder();

   /**
    * Simple set function that assigns m_searchRecord
    * aSearchRecord using copy constructor
    * @param aSearchRecord, The searchrecord to be copied
    */
   void setSearchRecord(const SearchRecord& aSearchRecord);

   /**
    * Simple get function.
    * @return a reference to m_searchRecord
    */
   SearchRecord& getSearchRecord();

   /**
    * Simple get function
    * @return a reference to m_searchCategories
    */
   const std::map<uint32, CombinedSearchCategory *>& getCombinedSearchCategories() const;

   /**
    * Simple get function
    * @return a reference to m_searchResults
    */
   const std::vector <CombinedSearchCategory *>& getCombinedSearchResults() const;

   /**
    * Simple get function
    * @return a reference to m_searchCategoryResults
    */
   const std::vector <CombinedSearchCategory *>& getCombinedSearchCategoryResults() const;

   /**
    * Simple get function that returns the stored CombinedSearchCategory the
    * supplied heading.
    * @param heading, the category to get.
    * @return CombinedSearchCategory, the found category.
    */
   const CombinedSearchCategory* getCombinedSearchCategory(uint32 heading) const;

   /**
    * Clears m_searchCategories
    */
   void clearCombinedSearchCategories();

   /**
    * Clears m_searchResults
    */
   void clearCombinedSearchResults();

   /**
    * Clears m_searchCategoryResults
    */
   void clearCombinedSearchCategoryResults();

   /**
    * Clears m_topHitResults
    */
   void clearTopHitResults();

   /**
    * Sets the m_searchCategories with new categories received from
    * the server. Clears m_searchCategories, m_searchCategoryResults and
    * m_searchResults, then adds all elements from vect to m_searchCategories
    * @param vect, The vector containing all new categories to be added
    */
   void setCombinedSearchCategories( std::map<uint32, CombinedSearchCategory *>& vect );
   
   /**
    * Finds the category with the cat_no in m_searchCategories, if found a new
    * CombinedSearchCategory is created with the m_searchCategories.find(cat_no).second
    * as paramter to its constructor. Total number of hits and corresponding search hits
    * (a vector holding searchItems for this category) is added to the newly created
    * category. 
    * @param cat_no, The category number to add
    * @param typeOfHits, The type of hits contained in this heading, 
    *                    0 for search hits and 1 for area matches.
    * @param totalNbrHits, Total number of hits for this category
    * @param topHits, Number of top hits for this category.
    * @param advertHitTitle, The title for advertisement hits
    * @param normHitTitle, The title of normal hits for this category
    * @param newres, A vector holding all search items to add to this category
    */
   void setSearchResults( int cat_no,
                          uint32 typeOfHits,
                          uint32 totalNbrHits,
                          uint32 topHits,
                          const char* advertHitTitle,
                          const char* normHitTitle,
                          std::vector<isab::SearchItem>& newres );

   /**
    * Finds the category with cat_no, if found the matching CombinedSearchCategory
    * gets a call to appendResults(newRes), which adds the new searchRes vector
    * the category, appends not replaces.
    * @param cat_no, The category number to add results to
    * @param newres, The vector holding all search results to be added to the category
    */
   void appendSearchResults( int cat_no, std::vector<isab::SearchItem>& newres );

   /**
    * Finds the category with the cat_no in m_searchResults, if found we replace
    * existing values to the supplied ones.
    * @param cat_no, The category number to add
    * @param typeOfHits, The type of hits contained in this heading, 
    *                    0 for search hits and 1 for area matches.
    * @param totalNbrHits, Total number of hits for this category
    * @param topHits, Number of top hits for this category.
    * @param advertHitTitle, The title for advertisement hits
    * @param normHitTitle, The title of normal hits for this category
    * @param newres, A vector holding all search items to add to this category
    */
   void replaceSearchResults( int cat_no, 
                              uint32 typeOfHits, 
                              uint32 totalNbrHits,
                              uint32 topHits,
                              const char* advertHitTitle,
                              const char* normHitTitle,
                              CSSIVec_t& newres);


   /**
    * Finds the category with the cat_no in m_searchCategories, if found a new
    * CombinedSearchCategory is created with the m_searchCategories.find(cat_no).second
    * as paramter to its constructor. Total number of hits is added to the newly created
    * category. 
    * @param cat_no, The category number to add
    * @param typeOfHits, The type of hits contained in this heading, 
    *                    0 for search hits and 1 for area matches.
    * @param totalNbrHits, Total number of hits for this category
    * @param topHits, Number of top hits for this category
    */
   void setSearchCategoryResults( int cat_no,
                                  uint32 typeOfHits,
                                  uint32 totalNbrHits,
                                  uint32 topHits );

   /**
    * Finds the category with the cat_no in m_searchCategoryResults, if found 
    * the existing values will be replaces with the supplied once.
    * @param cat_no, The category number to add
    * @param typeOfHits, The type of hits contained in this heading, 
    *                    0 for search hits and 1 for area matches.
    * @param totalNbrHits, Total number of hits for this category
    * @param topHits, Number of top hits for this category
    */
   void replaceSearchCategoryResults( int cat_no,
                                      uint32 typeOfHits,
                                      uint32 totalNbrHits,
                                      uint32 topHits);

   /**
    * Simple get function
    * @return a reference to searchHistory
    */
   const std::deque<SearchHistoryItem*>& getSearchHistory() const;

   /**
    * Clears the search history
    */
   void clearCombinedSearchHistory();

   /**
    * Adds a searchHistoryItem to m_searchHistory
    * @shItem, The SearchHistoryItem to be added to m_searchHistory
    */
   void addSearchHistoryItem(SearchHistoryItem* shItem);

   /**
    * Creates a SearchHistoryItem with the parameters and
    * then calls addSearchHistoryItem(SearchHistoryItem)
    * @param ss, The search string 
    * @param cis, The city string
    * @param cos, The country string
    * @param coi, The country id
    * @param cai, The category id
    */
   void addSearchHistoryItem(const char* ss, const char* cis, 
                             const char* cos, const char* coi, 
                             const char* cai);

   /**
    * Simple get function that returns a SearchHistoryItem at 
    * a certain index in the m_searchHistory
    * @param index, The index of the SearchHistoryItem in m_searchHistory
    * @return a pointer to the SearchHistoryItem at the specified index
    */
   SearchHistoryItem* getSearchHistoryItem(int index);

   /**
    * Gets the number of stored SearchHistoryItems in m_searchHistory
    * @return number of items in m_searchHistory
    */
   uint32 getNumSearchHistoryItems();

   /**
    * Sets m_advertHeadingTitle
    * @param advertHeadingTitle, the title for the advertisement hits in heading view
    */
   void setAdvertHeadingTitle(const char* advertHeadingTitle);

   /**
    * Sets m_normHeadingTitle
    * @param normHeadingTitle, the title for the normal headings in heading view
    */
   void setNormHeadingTitle(const char* normHeadingTitle);

   /**
    * Simpel get function
    * @return The title for advertisement hits in heading view (m_advertHeadingTitle)
    */
   const char* getAdvertHeadingTitle() const;

   /**
    * Simpel get function
    * @return The title for normal headings in heading view (m_normHeadingTitle)
    */
   const char* getNormHeadingTitle() const;

   /**
    * Assigns the content in siVec to m_topHitResults.
    * @param siVec, Vector containing all tophits for the search.
    */
   void setTopHitResults( CSSIVec_t& siVec );

   /**
    * Simple get function.
    * @return m_topHitResults, vector containing all top hits for this search.
    */
   const CSSIVec_t& getTopHitResults() const;

 private:
   /// Map holding all received categories from the server, CombinedSearchDispatcher
   /// sets this by calling setCombinedSearchCategories when received categories reply
   /// from the navRequestType::NAV_SEARCH_DESC_REQ request. This map does not
   /// contain any search results, only categories/headings
   std::map<uint32, CombinedSearchCategory*> m_searchCategories;
   /// Vector holding all categories which contains one or more hits. Adds a newly
   /// created CombinedSearchCategory and sets it total number of hits and
   /// the corresponding search results.
   std::vector<CombinedSearchCategory*> m_searchResults;
   /// Vector holding all categories which contains one or more hits. Adds a newly
   /// created CombinedSearchCategory and sets it total number of hits.
   /// The map does not contain any searchItems, only the categories that
   /// has any hits and the total number of hits for each category
   std::vector<CombinedSearchCategory*> m_searchCategoryResults;
   /// Vector containing all the top hits for this search.
   CSSIVec_t m_topHitResults;
   /// Member that holds the data for the current search
   SearchRecord m_searchRecord;
   /// Holding the 20 latest searches done
   std::deque<SearchHistoryItem*> m_searchHistory;
   /// Title in heading view for advertisement hits
   char* m_advertHeadingTitle;
   /// Title in heading view for normal headings
   char* m_normHeadingTitle;
};

#endif
