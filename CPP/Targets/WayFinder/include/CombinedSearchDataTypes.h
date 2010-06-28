/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef COMBINED_SEARCH_DATATYPES_H
#define COMBINED_SEARCH_DATATYPES_H

#include "Buffer.h"
#include "NParamBlock.h"

#include "GuiProt/SearchItem.h"

/**
 * Class that is a search category. 
 * @class CombinedSearchCategory
 */
class CombinedSearchCategory
{
public:

   /**
    * Constructor that creates a new CombinedSearchCategory
    * from a isab::Buffer
    * @param buffer, The buffer containing all information about
    *                the CombinedSearchCategory to create
    */
   CombinedSearchCategory( isab::Buffer *buffer );

   /**
    * Copy constructor that creates a new CombinedSearchCategory
    * from a CombinedSearchCategory
    * @param CombinedSearchCategory, The category to copy
    */
   CombinedSearchCategory( const CombinedSearchCategory& );

   /**
    * Clears all existing results in m_results and
    * copies the SearchItems from siVec into m_results
    * @param siVec, The vector containing the new SearhItems to be
    *               inserted into m_results
    */
   void setResults( std::vector<isab::SearchItem>& siVec );

   /**
    * Appends the items in siVec into m_results.
    * @param siVec, The vector containing the new SearhItems to be
    *               appended to m_results
    */
   void appendResults( std::vector<isab::SearchItem>& siVec );

   /**
    * Simple get function
    * @return a reference to m_results
    */
   const std::vector<isab::SearchItem>& getResults() const;

   /**
    * Cleares m_results
    */
   void clearResults();

   // destructor
   ~CombinedSearchCategory();

   /**
    * Simple get function
    * @return a pointer to m_imageName
    */
   const char* getImageName() const;

   /**
    * Simple get function
    * @return a pointer to m_name
    */
   const char* getName() const;

   /**
    * Simple get function
    * @return the round for this category (m_round)
    */
   uint32 getRound() const;

   /**
    * Simple get function
    * @return the heading number for this category (m_headingNo)
    */
   uint32 getHeadingNo() const;

   /**
    * Simple get function
    * @return the top region id for this category (m_topRegionId)
    */
   uint32 getTopRegionId() const;

   /**
    * Simple get function
    * @return the total number of hits for this category (m_totalNbrHits)
    */
   uint32 getTotalNbrHits() const;

   /**
    * Simple get function
    * @return the number of top hits for this category (m_topHits)
    */
   uint32 getTopHits() const;

   /**
    * Simple get function
    * @return the title for advertisement hits (m_advertHitTile)
    */
   const char* getAdvertHitTile() const;

   /**
    * Simple get function
    * @return the title for normal hits (m_normHitTitle)
    */
   const char* getNormHitTile() const;

   /**
    * Simple get function
    * @return the type of hits for this category. If 0 the category
    *         contains search results. If 1 the category contains
    *         area matches.
    */
   uint32 getTypeOfHits() const;

   /**
    * Simple set function that sets total number of hits for this category
    * @param totalNbrHits, The total number of hits to be set for this category
    */
   void setTotalNbrHits(uint32 totalNbrHits);

   /**
    * Simple set function that sets number of top hits for this category
    * @param topHits, The number of top hits to be set for this category
    */
   void setTopHits(uint32 topHits);

   /**
    * Simple set function that sets the title for advertisement hits for this category
    * @param advertHitTitle, The title of advertisement hits for this category
    */
   void setAdvertHitTile(const char* advertHitTitle);

   /**
    * Simple set function that sets the title for normal hits for this category
    * @param normHitTitle, The title of normal hits for this category
    */
   void setNormHitTitle(const char* normHitTitle);

   /**
    * Simple set function that sets the type of hits for this category
    * @param typeOfHits, the type of hits for this category. 0 if the category
    *                    contains search results. 1 if the category contains area matches.
    */
   void setTypeOfHits(uint32 typeOfHits);

private:

   /// The round this category belongs to
   uint32 m_round;
   /// The heading number for this category
   uint32 m_headingNo;
   /// Total number of hits for this category
   uint32 m_totalNbrHits;
   /// Number of top hits for this category
   uint32 m_topHits;
   /// Type of hits in this category, 0 for search hit, 1 for area match
   uint32 m_typeOfHits;
   /// Top region id for this category
   unsigned int m_topRegionId;
   /// Name of the image of this category
   char* m_imageName;
   /// Name of this category
   char* m_name;
   /// Title for the advertisement hits
   char* m_advertHitTile;
   /// Title for normal hits
   char* m_normHitTile;
   /// Vector holding all search hits for this category
   std::vector<isab::SearchItem> m_results;
};

#endif 	// COMBINED_SEARCH_DATATYPES_H
