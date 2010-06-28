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
#include "BufferArray.h"
#include "memlog.h"
#include "nav2util.h"
#include "SearchHistory.h"

SearchHistoryItem::SearchHistoryItem(const char *ss, const char *housenum,
      const char *cis, const char *cii,
      const char *cos, const char *coi)
{
   m_ss = isab::strdup_new(ss); 
   m_housenum = isab::strdup_new(housenum); 
   m_cis = isab::strdup_new(cis); 
   m_cii = isab::strdup_new(cii); 
   m_cos = isab::strdup_new(cos); 
   m_coi = isab::strdup_new(coi); 
}
SearchHistoryItem::~SearchHistoryItem()
{
   delete[] m_ss;
   delete[] m_housenum;
   delete[] m_cis;
   delete[] m_cii;
   delete[] m_cos;
   delete[] m_coi;
}

char *
SearchHistoryItem::GetHistoryListEntry()
{
   char *tmp;
   int length = strlen(m_ss) + 2 +
      strlen(m_housenum) + 2 +
      strlen(m_cis) + 2 +
      strlen(m_cos) + 1;
   tmp = new char[length];
   strcpy(tmp, m_ss);
   if (strlen(m_housenum)) {
      strcat(tmp, " ");
      strcat(tmp, m_housenum);
   }
   if (strlen(m_cis)) {
      strcat(tmp, ", ");
      strcat(tmp, m_cis);
   }
   if (strlen(m_cos)) {
      strcat(tmp, ", ");
      strcat(tmp, m_cos);
   }

   return tmp;
}

bool SearchHistoryItem::isDuplicate(const SearchHistoryItem *shi)
{
   if (strcmp(shi->m_ss, m_ss) ||
         strcmp(shi->m_housenum, m_housenum) ||
         strcmp(shi->m_cis, m_cis) ||
         strcmp(shi->m_cii, m_cii) ||
         strcmp(shi->m_cos, m_cos) ||
         strcmp(shi->m_coi, m_coi)) {
      /* Different. */
      return false;
   }
   return true;
}
