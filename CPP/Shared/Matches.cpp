/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Matches.h"

namespace isab{
   const char* SuperMatch::setName(const char* name){
      if(m_name){
         delete[] m_name;
         m_name = NULL;
      }
      if(name){
         size_t len = strlen(name) + 1;
         char* tmp = new char[len];
         memcpy(tmp, name, len);
         m_name = tmp;
      }
      return m_name;
   }

   SuperMatch::~SuperMatch()
   {
      setName(NULL);
      if(m_nbrRegions > 0){
         for(unsigned i = 0; i < m_nbrRegions; ++i){
            delete m_regions[i];
         }
         delete[] m_regions;
         m_nbrRegions = 0;
         m_regions = NULL;
      }
      delete[] m_itemIDstring;
   }

   SuperMatch::SuperMatch(Buffer& buf) :
      m_mapID(0), m_itemID(0), m_itemIDstring(NULL), m_type(0), 
      m_countryID(0), m_nbrRegions(0), m_name(NULL), m_regions(NULL)
   {
      m_mapID      = buf.readNextUnaligned32bit();
      m_itemID     = buf.readNextUnaligned32bit();
      m_type       = buf.readNextUnaligned32bit();
      m_countryID  = buf.readNextUnaligned32bit();
      m_nbrRegions = buf.readNextUnaligned32bit();
      m_itemIDstring = new char[12];
      snprintf(m_itemIDstring, 12, "%"PRIu32, m_itemID);
   }

   SuperMatch::SuperMatch(uint32 mapID, uint32 itemID, 
                          uint32 type, uint32 topRegion, 
                          const OverviewMatch* oms, int noOms,
                          const char* name) :
      m_mapID(mapID), m_itemID(itemID), m_itemIDstring(NULL), m_type(type), 
      m_countryID(topRegion), m_nbrRegions(noOms), m_name(NULL), 
      m_regions(NULL)
   {
      setName(name);
      if(m_nbrRegions > 0){
         m_regions = new OverviewMatch*[m_nbrRegions];
         for(unsigned i = 0; i < m_nbrRegions; ++i){
            m_regions[i] = new OverviewMatch(oms[i]);
         }
      }
      m_itemIDstring = new char[12];
      snprintf(m_itemIDstring, 12, "%"PRIu32, m_itemID);
   }

   SuperMatch::SuperMatch(const SuperMatch& sm) :
      m_mapID(sm.m_mapID), m_itemID(sm.m_itemID), m_itemIDstring(NULL),
      m_type(sm.m_type), m_countryID(sm.m_countryID), 
      m_nbrRegions(sm.m_nbrRegions), m_name(NULL), m_regions(NULL)
   {
      setName(sm.m_name);
      if(m_nbrRegions > 0){
         m_regions = new OverviewMatch*[m_nbrRegions];
         for(unsigned i = 0; i < m_nbrRegions; ++i){
            m_regions[i] = new OverviewMatch(*(sm.m_regions[i]));
         }
      }
      m_itemIDstring = new char[12];
      snprintf(m_itemIDstring, 12, "%"PRIu32, m_itemID);
   }

#define STRLEN_PLUS_COMMA_AND_SPACE(arg) (arg ? strlen(arg) + 2 : 0)

   void SuperMatch::pack()
   {
      if(m_nbrRegions > 0){
         OverviewMatch* bua = NULL;
         Buffer packedText(256);
         packedText.nprintf(256, "%s", m_name);
#ifdef __linux
         std::cerr << "pack:::   " << m_name << std::endl;
#endif
         for(unsigned j = 0; j < m_nbrRegions; ++j){
#ifdef __linux
            std::cerr << "pack::: " << j << ' ' 
                      << m_regions[j]->getName() << std::endl;
#endif
            packedText.jumpWritePos(-1); //erase \0-byte

            if(m_regions[j]->isMunicipal()){
               const char* buaname = NULL;
               if(isBuiltUpArea()){
                  buaname = getName();
               } else if(bua){
                  buaname = bua->getName();
               }
               if(!buaname || !strcaseequ(m_regions[j]->getName(), buaname)){
                  packedText.nprintf(256, ", (%s)", m_regions[j]->getName());
               } else {
                  packedText.jumpWritePos(1);
               }
            } else {
               packedText.nprintf(256, ", %s", m_regions[j]->getName());
            }
            if(m_regions[j]->isBuiltUpArea()){
               delete bua;
               bua = m_regions[j];
            } else {
               delete m_regions[j];
            }
         }
         delete bua; //not deleted with others.
         delete[] m_name;
         packedText.setReadPos(0);
         m_name = const_cast<char*>(packedText.getNextCharString());
         packedText.releaseData();
         m_nbrRegions = 0;
      }
      delete[] m_regions;
   }

   OverviewMatch::OverviewMatch(Buffer& buf) :
      SuperMatch(buf)
   {
      setName(buf.getNextCharString());
      if(m_nbrRegions > 0){
         m_regions = new OverviewMatch*[m_nbrRegions];
         for(unsigned i = 0; i < m_nbrRegions; ++i){
            m_regions[i] = new OverviewMatch(buf);
         }
      }
   }

   OverviewMatch::~OverviewMatch()
   {
      setName(NULL);
      if(m_nbrRegions > 0){
         for(unsigned i = 0; i < m_nbrRegions; ++i){
            delete m_regions[i];
         }
         delete[] m_regions;
         m_nbrRegions = 0;
         m_regions = NULL;
      }
   }

   const OverviewMatch& OverviewMatch::operator=(const OverviewMatch& rhs) 
   {
      if(this != &rhs){
         m_mapID = rhs.m_mapID;
         m_itemID = rhs.m_itemID;
         m_type = rhs.m_type;
         m_countryID = rhs.m_countryID;
         m_nbrRegions = rhs.m_nbrRegions;
         m_name = strdup_new(rhs.m_name);
         m_regions = new OverviewMatch*[m_nbrRegions];
         for(unsigned i = 0; i < m_nbrRegions; ++i){
            m_regions[i] = new OverviewMatch(*(rhs.m_regions[i]));
         }
      }
      return *this;
   }

   Match::Match(Buffer& buf) :
      SuperMatch(buf), m_lat(0), m_lon(0)
   {
      m_lat = buf.readNextUnaligned32bit();
      m_lon = buf.readNextUnaligned32bit();
      setName(buf.getNextCharString());
      if(m_nbrRegions > 0){
         m_regions = new OverviewMatch*[m_nbrRegions];
         for(unsigned i = 0; i < m_nbrRegions; ++i){
            m_regions[i] = new OverviewMatch(buf);
         }
      }
   }

   Match::~Match()
   {
      if(m_nbrRegions > 0){
         for(unsigned i = 0; i < m_nbrRegions; ++i){
            delete m_regions[i];
         }
         delete[] m_regions;
         m_nbrRegions = 0;
         m_regions = NULL;
      }
      setName(NULL);
   }


}
