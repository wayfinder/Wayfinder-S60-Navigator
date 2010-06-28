/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "nav2util.h"
#include "CombinedSearchDataTypes.h"

using namespace isab;

CombinedSearchCategory::CombinedSearchCategory(isab::Buffer *buffer)
{
   m_round = buffer->readNextUnaligned32bit();
   m_headingNo = buffer->readNextUnaligned32bit();
   m_name = strdup_new(buffer->getNextCharString());
   m_topRegionId = buffer->readNextUnaligned32bit();
   m_imageName = strdup_new(buffer->getNextCharString());
   m_advertHitTile = NULL;
   m_normHitTile = NULL;
}

CombinedSearchCategory::CombinedSearchCategory(const CombinedSearchCategory& csc)
{
   m_round = csc.m_round;
   m_headingNo = csc.m_headingNo;
   m_topRegionId = csc.m_topRegionId;
   m_totalNbrHits = csc.m_totalNbrHits;
   m_topHits = csc.m_topHits;
   m_imageName = strdup_new(csc.m_imageName);
   m_name = strdup_new(csc.m_name);
   m_advertHitTile = strdup_new(csc.m_advertHitTile);
   m_normHitTile = strdup_new(csc.m_normHitTile);
   m_typeOfHits = csc.m_typeOfHits;
   m_results.assign(csc.m_results.begin(), csc.m_results.end());
}

CombinedSearchCategory::~CombinedSearchCategory()
{
   clearResults();
   delete[] m_name;
   delete[] m_imageName;
   delete[] m_advertHitTile;
   delete[] m_normHitTile;   
}

void CombinedSearchCategory::clearResults()
{
   m_results.clear();
}

const char* CombinedSearchCategory::getImageName() const
{
   return m_imageName;
}

const char* CombinedSearchCategory::getName() const
{
   return m_name;
}

uint32 CombinedSearchCategory::getRound() const
{
   return m_round;
}

uint32 CombinedSearchCategory::getHeadingNo() const
{
   return m_headingNo;
}

uint32 CombinedSearchCategory::getTopRegionId() const
{
   return m_topRegionId;
}

uint32 CombinedSearchCategory::getTotalNbrHits() const
{
   return m_totalNbrHits;
}

uint32 CombinedSearchCategory::getTopHits() const
{
   return m_topHits;
}

const char* CombinedSearchCategory::getAdvertHitTile() const
{
   return m_advertHitTile;
}

const char* CombinedSearchCategory::getNormHitTile() const
{
   return m_normHitTile;
}

uint32 CombinedSearchCategory::getTypeOfHits() const
{
   return m_typeOfHits;
}

const std::vector<isab::SearchItem>& CombinedSearchCategory::getResults() const
{
   return m_results;
}

void CombinedSearchCategory::setResults( std::vector<isab::SearchItem>& siVec )
{
   clearResults();
   m_results.reserve( siVec.size() );
   m_results.assign( siVec.begin(), siVec.end() );
}

void CombinedSearchCategory::appendResults( std::vector<isab::SearchItem>& siVec )
{
   m_results.insert( m_results.end(), siVec.begin(), siVec.end() );
}

void CombinedSearchCategory::setTotalNbrHits(uint32 totalNbrHits)
{
   m_totalNbrHits = totalNbrHits;
}

void CombinedSearchCategory::setTopHits(uint32 topHits)
{
   m_topHits = topHits;
}

void CombinedSearchCategory::setAdvertHitTile(const char* advertHitTitle)
{
   if (m_advertHitTile) {
      delete[] m_advertHitTile;
   }
   m_advertHitTile = strdup_new(advertHitTitle);
}

void CombinedSearchCategory::setNormHitTitle(const char* normHitTitle)
{
   if (m_normHitTile) {
      delete[] m_normHitTile;
   }
   m_normHitTile = strdup_new(normHitTitle);
}

void CombinedSearchCategory::setTypeOfHits(uint32 typeOfHits)
{
   m_typeOfHits = typeOfHits;
}
