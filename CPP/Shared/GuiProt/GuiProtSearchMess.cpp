/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "arch.h"
#include "nav2util.h"
#include <algorithm>
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtMapMess.h"

#include "GuiProt/SearchRegion.h"
#include "GuiProt/SearchArea.h"
#include "GuiProt/SearchItem.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/FullSearchItem.h"

namespace isab{
   using namespace std;
   // SearchMess ///////////////////////////////////////////////

   SearchMess::SearchMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_topRegion = buf->readNextUnaligned32bit();
      m_areaID = buf->getNextCharString();
      m_areaQuery = buf->getNextCharString();
      m_itemQuery = buf->getNextCharString();
      m_origLat = buf->readNextUnaligned32bit();
      m_origLon = buf->readNextUnaligned32bit();
   };   

   SearchMess::SearchMess(uint32 topRegion, const char* areaID, 
                          const char* areaQuery, const char* itemQuery,
                          int32 origLat, int32 origLon) :
      GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::SEARCH),
      m_topRegion(topRegion), m_areaID(areaID), 
      m_areaQuery(areaQuery), m_itemQuery(itemQuery),
      m_origLat(origLat), m_origLon(origLon)
   {
   }
   
   SearchMess::~SearchMess()
   {
   }

   void SearchMess::deleteMembers()
   {
   }
 

   void SearchMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNextUnaligned32bit(m_topRegion);
      buf->writeNextCharString(m_areaID);
      buf->writeNextCharString(m_areaQuery);
      buf->writeNextCharString(m_itemQuery);
      buf->writeNextUnaligned32bit(m_origLat);
      buf->writeNextUnaligned32bit(m_origLon);
   }

   uint32 SearchMess::getTopRegion() const
   {
      return m_topRegion;
   }

   const char* SearchMess::getAreaID() const
   {
      return m_areaID;
   }


   const char* SearchMess::getAreaQuery() const
   {
      return m_areaQuery;
   }


   const char* SearchMess::getItemQuery() const
   {
      return m_itemQuery;
   }

   bool SearchMess::areaSelected() const
   {
      return *m_areaID;
   }

   int32 SearchMess::getOrigLat() const
   {
      return m_origLat;
   }

   int32 SearchMess::getOrigLon() const
   {
      return m_origLon;
   }


   // SearchArea helpers ////////////////////////////////////////
   char* idStringFromIdMapCountry(uint32 id, uint32 mapid, uint32 country)
   {
      const size_t len = 3*10 + 2 + 2 + 1;
      char* ret = new char[len];
      snprintf(ret, len - 1, "<%#x,%#x,%#x>", id, mapid, country);
      ret[len-1] = 0;
      return ret;
   }
   
   bool idMapCountryFromIdString(uint32& id, uint32& mapid, uint32& country,
                                 const char* idString)
   {
      return 3 == sscanf(idString, "<%"PRIx32",%"PRIx32",%"PRIx32">", 
                         &id, &mapid, &country);
   }


   // SearchArea //////////////////////////////////////////////
   SearchArea::SearchArea(const char* name, const char* id) :
      m_name(NULL), m_id(NULL)
   {
      m_name = strdup_new(name);
      m_id = strdup_new(id);
   }

   SearchArea::SearchArea(const SearchArea& rhs) :
      m_name(NULL), m_id(NULL)
   {
      m_name = strdup_new(rhs.m_name);
      m_id = strdup_new(rhs.m_id);
   }
   SearchArea::SearchArea(Buffer* buf) : 
      m_name(NULL), m_id(NULL)
   {
      m_name = strdup_new(buf->getNextCharString());
      m_id = strdup_new(buf->getNextCharString());
   }

   SearchArea::~SearchArea()
   {
      delete[] m_name;
      delete[] m_id;
   }

   void SearchArea::serialize(Buffer* buf) const
   {
      buf->writeNextCharString(m_name);
      buf->writeNextCharString(m_id);
   }

   const char* SearchArea::getName() const
   {
      return m_name;
   }

   const char* SearchArea::getID() const
   {
      return m_id;
   }

   bool SearchArea::operator==(const SearchArea& rhs) const
   {
      return this == &rhs || (0 == strcmp(m_name, rhs.m_name) && 
                              0 == strcmp(m_id, rhs.m_id));
   }

   // SearchAreaReplyMess /////////////////////////////////////////
   SearchAreaReplyMess::SearchAreaReplyMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      uint32 num = buf->readNextUnaligned32bit();
      m_areas.reserve(num);
      while(num--){
         m_areas.push_back(new SearchArea(buf));
      }
   }

   SearchAreaReplyMess::SearchAreaReplyMess(const SearchArea*const* areas, 
                                            unsigned num) :
      GuiProtMess(GuiProtEnums::type_and_data,
                  GuiProtEnums::GET_SEARCH_AREAS_REPLY),
      m_areas(areas, areas + num)
   {
   }


   SearchAreaReplyMess::~SearchAreaReplyMess()
   {
   }

   void SearchAreaReplyMess::deleteMembers()
   {
      std::for_each(m_areas.begin(), m_areas.end(), 
                    Delete<const SearchArea*>());
//       for(SearchArea_list::iterator q = m_areas.begin(); q != m_areas.end(); ++q){
//          delete *q;
//       }
   }

   void SearchAreaReplyMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNextUnaligned32bit(m_areas.size());
      for(SearchArea_list::const_iterator q = m_areas.begin(); q != m_areas.end(); ++q){
         (*q)->serialize(buf);
      }
   }



   const SearchArea* SearchAreaReplyMess::operator[](int index) const
   {
      if(index >= 0 && unsigned(index) < m_areas.size()){
         return m_areas[index];
      } else {
         return NULL;
      }
   }

   int SearchAreaReplyMess::size() const
   {
      return m_areas.size();
   }

   //// SearchRegion ////////////////////////////////////////////
   SearchRegion::SearchRegion(uint32 type, const char* id, const char* name) :
      m_regionType(type), m_id(NULL), m_name(NULL)
   {
      m_id = strdup_new(id);
      m_name = strdup_new(name);
   }

   SearchRegion::SearchRegion(Buffer* buf) :  
      m_regionType(0), m_id(NULL), m_name(NULL)
   {
      m_regionType = buf->readNextUnaligned32bit();
      m_id = strdup_new(buf->getNextCharString());
      m_name = strdup_new(buf->getNextCharString());
   }

   SearchRegion::SearchRegion(const SearchRegion& sr) :
      m_regionType(sr.m_regionType), m_id(NULL), m_name(NULL)
   {
      m_id = strdup_new(sr.m_id);
      m_name = strdup_new(sr.m_name);
   }

   const SearchRegion& SearchRegion::operator=(const SearchRegion& rhs){
      if(this != &rhs){
         m_regionType = rhs.m_regionType;
         replaceString(m_id, rhs.m_id);
         replaceString(m_name, rhs.m_name);
      }
      return *this;
   }

   SearchRegion::~SearchRegion()
   {
      delete[] m_id;
      delete[] m_name;
   }

   void SearchRegion::serialize(Buffer* buf) const
   {
      buf->writeNextUnaligned32bit(m_regionType);
      buf->writeNextCharString(m_id);
      buf->writeNextCharString(m_name);
   }

   const char* SearchRegion::getName() const
   {
      return m_name;
   }

   uint32 SearchRegion::getType() const
   {
      return m_regionType;
   }

   const char* SearchRegion::getId() const
   {
      return m_id;
   }

   bool SearchRegion::operator<(const SearchRegion& rhs) const
   {
      int cmp = 0;
      return (m_regionType < rhs.m_regionType) || 
         ((m_regionType == rhs.m_regionType) && 
          (((cmp = strcmp(m_name, rhs.m_name)) < 0) || 
           ((cmp == 0) && (strcmp(m_id,rhs.m_id) < 0))));
   }

   // SearchItem ////////////////////////////////////////////////////////
   SearchItem::SearchItem(Buffer* buf, uint16 version) :
      m_name(NULL), m_id(NULL), m_matchType(0), m_matchSubType(0),
      m_distance(0), m_regionList(0), m_lat(MAX_INT32), m_lon(MAX_INT32)
   {
      m_name    = strdup_new(buf->getNextCharString());
      m_id      = strdup_new(buf->getNextCharString());
      m_matchType = buf->readNextUnaligned16bit();
      m_matchSubType = buf->readNextUnaligned16bit();
      m_distance = buf->readNextUnaligned32bit();
      uint8 num = buf->readNext8bit();
      m_regionList.reserve(num);
      while(num--){
         m_regionList.push_back(new SearchRegion(buf));
      }
      m_lat = buf->readNextUnaligned32bit();
      m_lon = buf->readNextUnaligned32bit();
      m_version = version;
      if (version > 1) {
         m_imageName = strdup_new(buf->getNextCharString());
         m_advert = buf->readNext8bit();
      }
   }

   SearchItem::SearchItem(const char* name, const char* id, uint16 type,
                          uint16 subtype, uint32 distance, 
                          const SearchRegion* const* regions, unsigned num,
                          int32 lat, int32 lon, uint16 version,
                          const char* imageName, uint8 advert) :
      m_name(NULL), m_id(NULL), m_matchType(type), m_matchSubType(subtype),
      m_distance(distance), m_regionList(num, (SearchRegion*)NULL), 
      m_lat(lat), m_lon(lon), m_version(version)
   {
      m_name = strdup_new(name);
      m_id = strdup_new(id);
      std::transform(regions, regions + num, m_regionList.begin(), 
                     Clone<SearchRegion>());
      m_imageName = strdup_new(imageName);
      m_advert = advert;
   }

   SearchItem::SearchItem(const SearchItem& si) :
      m_name(NULL), m_id(NULL), m_matchType(si.m_matchType), 
      m_matchSubType(si.m_matchSubType), m_distance(si.m_distance), 
      m_regionList(0), m_lat(si.m_lat), m_lon(si.m_lon), m_version(si.m_version)
   {
      m_name    = strdup_new(si.m_name);
      m_id      = strdup_new(si.m_id);
      m_regionList.resize(si.m_regionList.size(), (SearchRegion*)NULL);
      std::transform(si.m_regionList.begin(), si.m_regionList.end(), 
                     m_regionList.begin(), Clone<SearchRegion>());
      if (m_version > 1) {
         m_imageName = strdup_new(si.m_imageName);
         m_advert = si.m_advert;
      } 
   }

   const SearchItem& SearchItem::operator=(const SearchItem& rhs)
   {
      if(this != &rhs){
         replaceString(m_name, rhs.m_name);
         replaceString(m_id, rhs.m_id);
         m_matchType = rhs.m_matchType;
         m_matchSubType = rhs.m_matchSubType;
         m_distance = rhs.m_distance;
         for_each(m_regionList.begin(), m_regionList.end(), 
                  Delete<const SearchRegion*>());
         m_regionList.resize(rhs.m_regionList.size(), (SearchRegion*)NULL);
         std::transform(rhs.m_regionList.begin(), rhs.m_regionList.end(), 
                        m_regionList.begin(), Clone<SearchRegion>());
         m_lat = rhs.m_lat;
         m_lon = rhs.m_lon;
         m_version = rhs.m_version;
         replaceString(m_imageName, rhs.m_imageName);
         m_advert = rhs.m_advert;
      }
      return *this;
   }


   SearchItem::~SearchItem()
   {
      delete[] m_name;
      delete[] m_id;
      if (m_version > 1) {
         delete[] m_imageName;
      }
      for_each(m_regionList.begin(), m_regionList.end(), 
               Delete<const SearchRegion*>());
   }

   void SearchItem::serialize(Buffer* buf) const
   {
      buf->writeNextCharString(m_name);
      buf->writeNextCharString(m_id);
      buf->writeNextUnaligned16bit(m_matchType);
      buf->writeNextUnaligned16bit(m_matchSubType);
      buf->writeNextUnaligned32bit(m_distance);
      buf->writeNext8bit(m_regionList.size());
      RegionList::const_iterator q;
      for(q = m_regionList.begin(); q != m_regionList.end(); ++q){
         (*q)->serialize(buf);
      }
      buf->writeNextUnaligned32bit(m_lat);
      buf->writeNextUnaligned32bit(m_lon);
      if (m_version > 1) {
         buf->writeNextCharString(m_imageName);
         buf->writeNext8bit(m_advert);
      }
   }

   const char* SearchItem::getName() const
   {
      return m_name;
   }

   const char* SearchItem::getID() const
   {
      return m_id;
   }

   uint16 SearchItem::getType() const
   {
      return m_matchType;
   }

   uint16 SearchItem::getSubType() const
   {
      return m_matchSubType;
   }

   unsigned int SearchItem::noRegions() const
   {
      return m_regionList.size();
   }

   const SearchRegion* SearchItem::getRegion(unsigned index) const
   {
      if(index < m_regionList.size()){
         return m_regionList[index];
      }
      return NULL;
   }

   //functor for finding SearchRegions with a certain type.
   class FindType : public std::unary_function<const SearchRegion*, bool> {
   public:
      FindType(GuiProtEnums::RegionType type) : m_type(type){}
      result_type operator()(argument_type arg){ 
         return GuiProtEnums::RegionType(arg->getType()) == m_type; 
      }
   private:
      GuiProtEnums::RegionType m_type;
   };

   const SearchRegion* 
   SearchItem::getRegionOfType(GuiProtEnums::RegionType type,
                               const SearchRegion* idx) const
   {
      //find starting point.
      RegionList::const_iterator start = 
         std::find(m_regionList.begin(), m_regionList.end(), idx);
      //didn't find it. use the real start
      if(start == m_regionList.end()){
         start = m_regionList.begin();
      }
      //find the first of the type.
      RegionList::const_iterator ret = 
         std::find_if(start, m_regionList.end(), FindType(type));
      if(ret != m_regionList.end()){
         return *ret; //found it!
      } else {
         return NULL; //no such region
      }  
   }

   uint32 SearchItem::getDistance() const
   {
      return m_distance;
   }

   int32 SearchItem::getLat() const
   {
      return m_lat;
   }

   int32 SearchItem::getLon() const
   {
      return m_lon;
   }
 
   uint16 SearchItem::getVersion() const
   {
      return m_version;
   } 

   const char* SearchItem::getImageName() const
   {
      return m_imageName;
   }

   uint8 SearchItem::getAdvert() const
   {
      return m_advert;
   }

   bool operator==(const SearchItem::RegionList& lhs, 
                   const SearchItem::RegionList& rhs)
   {
      if(&lhs == &rhs) 
         return true;
      if(lhs.size() != rhs.size() )
         return false;
      SearchItem::RegionList::const_iterator l,r;
      for(l = lhs.begin(), r = rhs.begin(); l != lhs.end(); ++l, ++r){
         if(*l != *r)
            return false;
      }
      return true;
   }

   bool SearchItem::operator==(const SearchItem& rhs) const
   {
      return this == &rhs || 
         (0 == strcmp(m_name, rhs.m_name) &&
          0 == strcmp(m_id, rhs.m_name) &&
          m_matchType    == rhs.m_matchType &&
          m_matchSubType == rhs.m_matchSubType &&
          m_distance     == rhs.m_distance && 
          m_regionList   == rhs.m_regionList &&
          m_version      == rhs.m_version);
   }

   SearchItem::RegionList::size_type 
   SearchItem::addRegion(const SearchRegion& sr)
   {
      m_regionList.push_back(new SearchRegion(sr));
      return m_regionList.size();
   }

   // SearchItemReplyMess /////////////////////////////////////////////////

   SearchItemReplyMess::SearchItemReplyMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_totalHits = buf->readNextUnaligned16bit();
      m_startIndex = buf->readNextUnaligned16bit();
      uint32 num = buf->readNextUnaligned32bit();
      m_items.reserve(num);
      while(num--){
         m_items.push_back(new SearchItem(buf));
      }
   }

   SearchItemReplyMess::SearchItemReplyMess(const SearchItem*const* items, 
                                            unsigned num, uint16 totalHits,
                                            uint16 startIndex) :
         GuiProtMess(GuiProtEnums::type_and_data, 
                     GuiProtEnums::GET_SEARCH_ITEMS_REPLY),
         m_items(items, items + num), m_totalHits(totalHits), 
         m_startIndex(startIndex)
   {
   }

   SearchItemReplyMess::~SearchItemReplyMess()
   {
   }

   void SearchItemReplyMess::deleteMembers()
   {
      for_each(m_items.begin(), m_items.end(), Delete<const SearchItem*>());
   }

   void SearchItemReplyMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(m_totalHits);
      buf->writeNextUnaligned16bit(m_startIndex);
      buf->writeNextUnaligned32bit(m_items.size());
      for_each(m_items.begin(), m_items.end(), Serialize(*buf));
   }

   const SearchItem* SearchItemReplyMess::operator[](unsigned index)
   {
      if(index < m_items.size()){
         return m_items[index];
      } else {
         return NULL;
      }
   }

   int SearchItemReplyMess::size() const
   {
      return m_items.size();
   }

   uint16 SearchItemReplyMess::getTotalHits() const
   {
      return m_totalHits;
   }

   uint16 SearchItemReplyMess::getStartIndex() const
   {
      return m_startIndex;
   }


   // AdditionalInfo ////////////////////////////////
   AdditionalInfo::AdditionalInfo(const char* key, const char* value, 
                                  uint32 type) :
      m_type(type), m_key(NULL), m_value(NULL)
   {
      m_key = strdup_new(key);
      m_value = strdup_new(value);
   }
   AdditionalInfo::AdditionalInfo(Buffer* buf) :
      m_type(0), m_key(NULL), m_value(NULL)
   {
      m_type  = buf->readNextUnaligned32bit();
      m_key   = strdup_new(buf->getNextCharString());
      m_value = strdup_new(buf->getNextCharString());
   }

   AdditionalInfo::AdditionalInfo(const AdditionalInfo& ai) :
      m_type(ai.m_type), m_key(NULL), m_value(NULL)
   {
      m_key   = strdup_new(ai.m_key);
      m_value = strdup_new(ai.m_value); 
   }

   AdditionalInfo::~AdditionalInfo()
   {
      delete[] m_key;
      delete[] m_value;
   }

   void AdditionalInfo::serialize(Buffer* buf) const
   {
      buf->writeNextUnaligned32bit(m_type);
      buf->writeNextCharString(m_key);
      buf->writeNextCharString(m_value);
   }

   uint32 AdditionalInfo::getSize() const {
      return strlen( m_key ) + 1 + strlen( m_value ) + 1 + 4 + 3;
   }

   uint32 AdditionalInfo::getType() const
   {
      return m_type;
   }

   const char* AdditionalInfo::getKey() const
   {
      return m_key;
   }

   const char* AdditionalInfo::getValue() const
   {
      return m_value;
   }

   void AdditionalInfo::setValue(const char* value) 
   {
      delete[] m_value;
      m_value = strdup_new(value);
   }

   bool AdditionalInfo::operator==(const AdditionalInfo& rhs) const
   {
      return this == &rhs || (m_type == rhs.m_type && 
                              0 == strcmp(m_key, rhs.m_key) && 
                              0 == strcmp(m_value, rhs.m_value));
   }

   //FullSearchItem ///////////////////////////////////////

   FullSearchItem::FullSearchItem(Buffer* buf) : 
      SearchItem(buf), m_additionalInfo(0), m_alt(MAX_INT32), 
      m_searchModuleStatusFlags(0)
   {
      uint8 num = buf->readNext8bit();
      m_additionalInfo.reserve(num);
      while(num--){
         m_additionalInfo.push_back(new AdditionalInfo(buf));
      }
      m_alt = buf->readNextUnaligned32bit();
      m_searchModuleStatusFlags = buf->readNextUnaligned32bit();
   }

   FullSearchItem::FullSearchItem(const char* name, const char* id, 
                                  uint16 type, 
                                  uint16 subtype, uint32 distance, 
                                  const SearchRegion*const* regions, 
                                  unsigned numRegions, int32 lat, int32 lon, 
                                  int32 alt, uint32 flags, 
                                  const AdditionalInfo*const* infos, 
                                  unsigned numInfo) :
      SearchItem(name, id, type, subtype, distance, regions, numRegions, lat, 
                 lon), m_additionalInfo(numInfo, (AdditionalInfo*)NULL), 
      m_alt(alt), m_searchModuleStatusFlags(flags)
   {
      //XXX doesnt work on msc?
      std::transform(infos, infos + numInfo, m_additionalInfo.begin(), 
                     Clone<AdditionalInfo>());
   }

   FullSearchItem::FullSearchItem(const char* name, const char* id, 
                                  uint16 type, uint16 subtype, int32 lat, 
                                  int32 lon) :
      SearchItem(name, id, type, subtype, 0, NULL, 0, lat, 
                 lon), m_additionalInfo(0, (AdditionalInfo*)NULL), 
      m_alt(0), m_searchModuleStatusFlags(0)
   {
   }      

   FullSearchItem::FullSearchItem(const FullSearchItem& fsi) :
      SearchItem(fsi), m_additionalInfo(0), m_alt(fsi.m_alt), 
      m_searchModuleStatusFlags(fsi.m_searchModuleStatusFlags)
   {
      m_additionalInfo.resize(fsi.m_additionalInfo.size());
      std::transform(fsi.m_additionalInfo.begin(), fsi.m_additionalInfo.end(),
                     m_additionalInfo.begin(), Clone<AdditionalInfo>());
   }

   const FullSearchItem& FullSearchItem::operator=(const FullSearchItem& rhs)
   {
      if(this != &rhs){
         SearchItem::operator=(rhs); //copy SearchItem part
         std::for_each(m_additionalInfo.begin(), m_additionalInfo.end(), 
                       Delete<AdditionalInfo*>());
         m_additionalInfo.resize(rhs.m_additionalInfo.size());
         std::transform(rhs.m_additionalInfo.begin(), 
                        rhs.m_additionalInfo.end(),
                        m_additionalInfo.begin(), Clone<AdditionalInfo>());
         m_searchModuleStatusFlags = rhs.m_searchModuleStatusFlags;
      }
      return *this;
   }

   FullSearchItem::~FullSearchItem()
   {
      for_each(m_additionalInfo.begin(), m_additionalInfo.end(), 
               Delete<AdditionalInfo*>());
   }

   void FullSearchItem::serialize(Buffer* buf) const
   {
      SearchItem::serialize(buf);
      buf->writeNext8bit(m_additionalInfo.size());
      for_each(m_additionalInfo.begin(), m_additionalInfo.end(),
               Serialize(*buf));
      buf->writeNextUnaligned32bit(m_alt);
      buf->writeNextUnaligned32bit(m_searchModuleStatusFlags);

   }

   // no is a bad abreviation for number
   unsigned FullSearchItem::noAdditionalInfo() const
   {
      return m_additionalInfo.size();
   }

   const AdditionalInfo* FullSearchItem::getInfo(unsigned index) const
   {
      if(index < m_additionalInfo.size()){
         return m_additionalInfo[index];
      }
      return NULL;
   }

   FullSearchItem::InfoList::size_type 
   FullSearchItem::addInfo(const AdditionalInfo& ai)
   {
      m_additionalInfo.push_back(new AdditionalInfo(ai));
      return m_additionalInfo.size();
   }

   int32 FullSearchItem::getAlt() const
   {
      return m_alt;
   }

   uint32 FullSearchItem::getSearchModuleStatusFlags() const
   {
      return m_searchModuleStatusFlags;
   }

   // FullSearchDataReplyMess ///////////////////////////
   FullSearchDataReplyMess::FullSearchDataReplyMess(Buffer* buf) :
      GuiProtMess(buf), m_items(0)
   {
      uint16 num = buf->readNextUnaligned16bit();
      m_items.reserve(num);
      while(num--){
         m_items.push_back(new FullSearchItem(buf));
      }
   }

   FullSearchDataReplyMess::FullSearchDataReplyMess(const FullSearchItem *const  * items,
                                                    unsigned num) :
      GuiProtMess(GuiProtEnums::type_and_data, 
                  GuiProtEnums::GET_FULL_SEARCH_DATA_REPLY), 
      m_items(items, items + num)
   {
   }

      FullSearchDataReplyMess::FullSearchDataReplyMess(FullSearchDataReplyMess* mess) :
      GuiProtMess(GuiProtEnums::type_and_data,                                                    GuiProtEnums::GET_FULL_SEARCH_DATA_REPLY)
   {
      uint16 num = mess->size();
      m_items.reserve(num);
      uint16 i = 0;
      while(i < num){
         m_items.push_back(new FullSearchItem(*((*mess)[i])));
         i++;
      }
   }


   FullSearchDataReplyMess::~FullSearchDataReplyMess()
   {
   }

   void FullSearchDataReplyMess::deleteMembers()
   {
      for_each(m_items.begin(), m_items.end(), 
               Delete<const FullSearchItem*>());
   }
  
   void FullSearchDataReplyMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(m_items.size());
      for_each(m_items.begin(), m_items.end(), Serialize(*buf));
   }

   unsigned FullSearchDataReplyMess::size() const
   {
      return m_items.size();
   }

   const FullSearchItem* FullSearchDataReplyMess::operator[](unsigned index) const
   {
      if(index < m_items.size()){
         return m_items[index];
      }
      return NULL;
   }

   // FullSearchDataFromItemIdReplyMess ///////////////////////////
   FullSearchDataFromItemIdReplyMess::FullSearchDataFromItemIdReplyMess(
         const FullSearchItem * const * items,
         unsigned num) : FullSearchDataReplyMess(items, num)
   {
      m_messageType = GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID_REPLY;
   }

   FullSearchDataFromItemIdReplyMess::FullSearchDataFromItemIdReplyMess(
         FullSearchDataFromItemIdReplyMess* mess)
         : FullSearchDataReplyMess(mess)
   {
      m_messageType = GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID_REPLY;
   }

   GetMoreDataMess::GetMoreDataMess(Buffer* buf) :
      GuiProtMess(buf), m_index(MAX_UINT16), m_value(NULL)
   {
      m_index = buf->readNextUnaligned16bit();
      m_value = strdup_new(buf->getNextCharString());
   }

   GetMoreDataMess::GetMoreDataMess(uint16 index, const char* value) :
      GuiProtMess(GuiProtEnums::type_and_data, 
                  GuiProtEnums::GET_MORE_SEARCH_DATA), 
      m_index(index), m_value(value)
   {
   }

   GetMoreDataMess::~GetMoreDataMess()
   {
   }

   void GetMoreDataMess::deleteMembers()
   {
      delete[] const_cast<char*>(m_value);
   }
   
   void GetMoreDataMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNextUnaligned16bit(m_index);
      buf->writeNextCharString(m_value);
   }
   
   uint16 GetMoreDataMess::getIndex() const
   {
      return m_index;
   }

   const char* GetMoreDataMess::getValue() const
   {
      return m_value;
   }


}
