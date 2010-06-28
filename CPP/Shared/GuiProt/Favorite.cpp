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
#include "GuiProt/Favorite.h"
#include "Buffer.h"
#include <string.h>
#include "UTF8Util.h"
#include "GuiProt/AdditionalInfo.h"
#include "crc32.h"
#include "MD5Class.h"

namespace isab {



// GuiFavorite implementation. /////////////////////////

GuiFavorite::GuiFavorite(Buffer* buf){
   m_ID =   buf->readNextUnaligned32bit();
   m_name = buf->getNextCharStringAlloc();
   m_lat = buf->readNextUnaligned32bit();
   m_lon = buf->readNextUnaligned32bit();
   m_lmID = buf->readNextUnaligned32bit();
   m_lmsID = buf->getNextCharStringAlloc();
}

GuiFavorite::GuiFavorite(uint32 id, const char* name, 
                         int32 lat, int32 lon,
                         uint32 lmid, const char* lmsID){
    m_ID = id;
    m_name = strdup_new(name);
    m_lat = lat;
    m_lon = lon;
    m_lmID = lmid;
    m_lmsID = strdup_new(lmsID);
}

GuiFavorite::GuiFavorite(const GuiFavorite& guiFav)
{
   m_ID = guiFav.getID();
   m_name = strdup_new(guiFav.getName());
   m_lat = guiFav.m_lat;
   m_lon = guiFav.m_lon;
   m_lmID = guiFav.getLmID();
   m_lmsID = strdup_new(guiFav.getLmsID());
}

GuiFavorite::~GuiFavorite(){
   delete[] m_name;
   delete[] m_lmsID;
}
void GuiFavorite::serialize(Buffer* buf) const{
   buf->writeNextUnaligned32bit(m_ID);
   buf->writeNextCharString(m_name);
   buf->writeNextUnaligned32bit(m_lat);
   buf->writeNextUnaligned32bit(m_lon);
   buf->writeNextUnaligned32bit(m_lmID);
   buf->writeNextCharString(m_lmsID);
}

uint32 GuiFavorite::getID() const
{
   return m_ID;
}

const char* GuiFavorite::getName() const
{
   return m_name;
}

int32 GuiFavorite::getLat() const
{
   return m_lat;
}

int32 GuiFavorite::getLon() const
{
   return m_lon;
}

uint32 GuiFavorite::getLmID() const
{
   return m_lmID;
}

const char* GuiFavorite::getLmsID() const
{
   return m_lmsID;
}


// Favorite implementation. /////////////////////////

Favorite::Favorite(const class Favorite& aFav) 
{
   initMembers();
   m_ID  = aFav.m_ID;
   m_lat = aFav.m_lat;
   m_lon = aFav.m_lon;
   m_name        = strdup_new(aFav.m_name);
   m_shortName   = strdup_new(aFav.m_shortName);
   m_description = strdup_new(aFav.m_description);
   m_category    = strdup_new(aFav.m_category);
   m_mapIconName = strdup_new(aFav.m_mapIconName);
   m_lmID        = aFav.m_lmID;
   m_lmsID       = strdup_new(aFav.m_lmsID);
   m_md5sum = new uint8[MAX_SIZE_MD5SUM];
   if (aFav.m_md5sum != NULL) {
      memcpy(m_md5sum, aFav.m_md5sum, MAX_SIZE_MD5SUM);
   } else {
      memset(m_md5sum, '0', MAX_SIZE_MD5SUM);
   }
   m_synced = aFav.m_synced;
   m_infos.resize( aFav.m_infos.size() );
   for ( uint32 i = 0 ; i < aFav.m_infos.size() ; ++i ) {
      m_infos[ i ] = new AdditionalInfo( *aFav.m_infos[ i ] );
   }
}

Favorite::Favorite( Buffer* buf, int convertToUTF8, int version )
      : Serializable(buf)
{
   initMembers();

   m_synced = buf->readNext8bit();
   m_ID = buf->readNextUnaligned32bit();
   m_lat = buf->readNextUnaligned32bit();
   m_lon = buf->readNextUnaligned32bit();
   if (convertToUTF8) {
      m_name        = UTF8Util::cleanUtf8(buf->getNextCharString());
      m_shortName   = UTF8Util::cleanUtf8(buf->getNextCharString());
      m_description = UTF8Util::cleanUtf8(buf->getNextCharString());
      m_category    = UTF8Util::cleanUtf8(buf->getNextCharString());
      m_mapIconName = UTF8Util::cleanUtf8(buf->getNextCharString());
   } else {
      m_name        = buf->getNextCharStringAlloc();
      m_shortName   = buf->getNextCharStringAlloc();
      m_description = buf->getNextCharStringAlloc();
      m_category    = buf->getNextCharStringAlloc();
      m_mapIconName = buf->getNextCharStringAlloc();
   }

   if ( version >= 3 ) {
      uint32 nbrInfos = buf->readNextUnaligned32bit();
      m_infos.resize( nbrInfos );
      for ( uint32 i = 0 ; i < nbrInfos ; ++i ) {
         m_infos[ i ] = new AdditionalInfo( buf );
      }
   }

   if (version >= 4) {
      m_lmID = buf->readNextUnaligned32bit();
      m_lmsID = buf->getNextCharStringAlloc();
      m_md5sum = new uint8[MAX_SIZE_MD5SUM];
      buf->readNextByteArray(m_md5sum, MAX_SIZE_MD5SUM);
   } else {
      // Make favorite upgraded to version 4
      m_md5sum = new uint8[MAX_SIZE_MD5SUM];
      memset(m_md5sum, '0', MAX_SIZE_MD5SUM);
   }
}

Favorite::Favorite(uint32 id, int32 lat, int32 lon,
                   const char* name, const char* shortName, 
                   const char* description, const char* category,
                   const char* mapIconName, bool synced,
                   uint32 lmID, const char* imeiCrcHex,
                   const char* lmsID, const uint8* md5sum)
{
   initMembers();   
   
   m_ID  = id;
   m_lat = lat;
   m_lon = lon;
   m_name        = strdup_new(name);
   m_shortName   = strdup_new(shortName);
   m_description = strdup_new(description);
   m_category    = strdup_new(category);
   m_mapIconName = strdup_new(mapIconName);
   m_lmID        = lmID;
   if (imeiCrcHex && lmID != MAX_UINT32) {
      char tmpID[9];
      sprintf(tmpID, "%x", lmID);
      char complete[17];
      strcpy(complete, imeiCrcHex);
      strcat(complete, tmpID);
      AdditionalInfo* info = new AdditionalInfo("lmid", complete, GuiProtEnums::dont_show);
      addAdditionalInfo(info);
   }
   m_lmsID  = strdup_new(lmsID);
   m_md5sum = new uint8[MAX_SIZE_MD5SUM];
   if (md5sum != NULL) {
      memcpy(m_md5sum, md5sum, MAX_SIZE_MD5SUM);
   } else {
      memset(m_md5sum, '0', MAX_SIZE_MD5SUM);
   }

   m_synced = synced;
}

Favorite::Favorite(int32 lat, int32 lon,
                   const char* name, const char* shortName, 
                   const char* description, const char* category,
                   const char* mapIconName, bool synced,
                   uint32 lmID, const char* imeiCrcHex,
                   const char* lmsID, const uint8* md5sum)
{
   initMembers();

   m_lat = lat;
   m_lon = lon;
   m_name        = strdup_new(name);
   m_shortName   = strdup_new(shortName);
   m_description = strdup_new(description);
   m_category    = strdup_new(category);
   m_mapIconName = strdup_new(mapIconName);
   m_lmID        = lmID;    
   if (imeiCrcHex && lmID != MAX_UINT32) {
      char tmpID[9];
      sprintf(tmpID, "%x", lmID);
      char complete[17];
      strcpy(complete, imeiCrcHex);
      strcat(complete, tmpID);
      AdditionalInfo* info = new AdditionalInfo("lmid", complete, GuiProtEnums::dont_show);
      addAdditionalInfo(info);
   }
   m_lmsID       = strdup_new(lmsID);

   m_md5sum = new uint8[MAX_SIZE_MD5SUM];
   if (md5sum != NULL) {
      memcpy(m_md5sum, md5sum, MAX_SIZE_MD5SUM);
   } else {
      memset(m_md5sum, '0', MAX_SIZE_MD5SUM);
   }

   m_synced = synced;
}

void
Favorite::initMembers()
{
   m_ID = uint32(Favorite::INVALID_FAV_ID);
   m_lat = MAX_INT32;
   m_lon = MAX_INT32;
   m_name = NULL;
   m_shortName = NULL;
   m_description = NULL;
   m_category = NULL;
   m_mapIconName = NULL;
   m_lmID = MAX_UINT32;
   m_lmsID = NULL;
   m_md5sum = NULL;

   m_synced = false;
} // initMembers

Favorite::~Favorite()
{
   delete[] m_name;
   delete[] m_shortName;
   delete[] m_description;
   delete[] m_category;
   delete[] m_mapIconName;
   delete[] m_lmsID;
   delete[] m_md5sum;
   for ( InfoVect::const_iterator it = m_infos.begin() ; 
         it != m_infos.end() ; ++it ) 
   {
      delete *it;
   }
} // ~Favorite

void 
Favorite::serialize( Buffer* buf, int version ) const
{
   buf->writeNext8bit(m_synced);

   buf->writeNextUnaligned32bit(m_ID);
   buf->writeNextUnaligned32bit(m_lat);
   buf->writeNextUnaligned32bit(m_lon);
   buf->writeNextCharString(m_name);
   buf->writeNextCharString(m_shortName);
   buf->writeNextCharString(m_description);
   buf->writeNextCharString(m_category);
   buf->writeNextCharString(m_mapIconName);
   if ( version >= 3 ) {
      buf->writeNextUnaligned32bit( m_infos.size() );
      for ( InfoVect::const_iterator it = m_infos.begin() ; 
            it != m_infos.end() ; ++it ) 
      {
         (*it)->serialize( buf );
      }
   }
   if ( version >= 4 ) {
      buf->writeNextUnaligned32bit(m_lmID);
      buf->writeNextCharString(m_lmsID);
      buf->writeNextByteArray(m_md5sum, MAX_SIZE_MD5SUM);
   }
} // serialize

GuiFavorite* 
Favorite::getGuiFavorite() const
{
   // This code determines what to put in the 
   // name field depending on the text in the 
   // description field.
   char* guiFavName = NULL;
   if ((m_description == NULL) || (strcmp(m_description, "") == 0)){
      // The description does not exist.

      guiFavName = strdup_new(m_name);
   }
   else if (strcasecmp(m_name, m_description) == 0){
      // Name and description are equal.
      char tmp[] = ",";
      uint32 pos = strcspn(m_name, tmp);
      if (pos < strlen(m_name)){
         guiFavName = strdup_new(m_name);
         guiFavName[pos] = '\t';
      } else {
         // Name and desc is identical but lacks a comma,
         // create a string that has both name and desc separated
         // by a tab.
         uint32 totalNameLength = strlen(m_name) +
            1 + //tab char.
            strlen(m_description) + 
            1;  //terminating null.
         
         guiFavName = new char[totalNameLength];
         strcpy(guiFavName, m_name);
         strcat(guiFavName, "\t");
         strcat(guiFavName, m_description);
      }
   }
   else{
      // Description exists and differs from name.

      uint32 totalNameLength = strlen(m_name) +
                               1 + //tab char.
                               strlen(m_description) + 
                               1;  //terminating null.

      guiFavName = new char[totalNameLength];
      strcpy(guiFavName, m_name);
      strcat(guiFavName, "\t");
      strcat(guiFavName, m_description);
   }

   GuiFavorite* result = new GuiFavorite(m_ID, guiFavName, m_lat, m_lon, 
                                         m_lmID, m_lmsID);
   delete[] guiFavName;
   return result;
}

uint32
Favorite::getID() const
{
   return m_ID;
}

int32 
Favorite::getLat() const
{
   return m_lat;
}

int32  
Favorite::getLon() const
{
   return m_lon;
}

const char*  
Favorite::getName() const
{
   return m_name ? m_name : "";
}

const char*  
Favorite::getShortName() const
{
   return m_shortName ? m_shortName : "";
}

const char*  
Favorite::getDescription() const
{
   return m_description ? m_description : "";
}

const char*  
Favorite::getCategory() const
{
   return m_category ? m_category : "";
}

const char*  
Favorite::getMapIconName() const
{
   return m_mapIconName ? m_mapIconName : "";
}

uint32
Favorite::getLmID() const
{
   return m_lmID;
}

const char*
Favorite::getLmsID() const
{
   return m_lmsID;
}

const uint8*
Favorite::getMd5sum() const
{
   return m_md5sum;
}

const Favorite::InfoVect&
Favorite::getInfos() const {
   return m_infos;
}

bool
Favorite::hasInfoType( GuiProtEnums::AdditionalInfoType type ) const {
   for ( InfoVect::const_iterator it = m_infos.begin() ; 
         it != m_infos.end() ; ++it ) 
   {
      if ( GuiProtEnums::AdditionalInfoType((*it)->getType()) == type ) {
         return true;
      }
   }
   return false;
}

void  
Favorite::setID( uint32 id )
{
   m_ID = id;
}

void
Favorite::setLat( int32 lat ) {
   m_lat = lat;
}

void
Favorite::setLon( int32 lon ) {
   m_lon = lon;
}

void
Favorite::setName( const char* name ) {
   delete [] m_name;
   m_name = strdup_new( name );
}

void
Favorite::setShortName( const char* sname ) {
   delete [] m_shortName;
   m_shortName = strdup_new( sname );
}

void
Favorite::setDescription( const char* desc ) {
   delete [] m_description;
   m_description = strdup_new( desc );

}

void
Favorite::setCategory( const char* cat ) {
   delete [] m_category;
   m_category = strdup_new(cat  );

}

void
Favorite::setMapIconName( const char* mapIcon ) {
   delete [] m_mapIconName;
   m_mapIconName = strdup_new( mapIcon );

}

void 
Favorite::setLmID( uint32 lmID, const char* imeiCrcHex ) {

   // Convert and concatenate the new imei lmid to set in the
   // infovec
   char tmpID[9];
   sprintf(tmpID, "%x", lmID);
   char complete[17];
   strcpy(complete, imeiCrcHex);
   strcat(complete, tmpID);

   InfoVect::const_iterator it; 
   for (it = m_infos.begin(); it != m_infos.end(); ++it) {
      if (strcmp((*it)->getKey(), "lmid") == 0 &&
          strncmp(imeiCrcHex, (*it)->getValue(), strlen(imeiCrcHex)) == 0) {
         // Found a lmid key, check the value
         // Found a matching imei, check the lmID
         (*it)->setValue(complete);
         break;
      }
   }
   if (it == m_infos.end()) {
      // Didnt find the lmid in the m_infos, add it to the vector.
      AdditionalInfo* info = new AdditionalInfo("lmid", complete, GuiProtEnums::dont_show);
      addAdditionalInfo(info);
   }
   m_lmID = lmID;
}

void 
Favorite::setLmsID( const char* lmsID ) {
   delete[] m_lmsID;
   m_lmsID = strdup_new(lmsID);
}

void 
Favorite::setMd5sum( const uint8* md5sum ) {
   delete[] m_md5sum;
   if (md5sum) {
      m_md5sum = new uint8[MAX_SIZE_MD5SUM];
      memcpy(m_md5sum, md5sum, MAX_SIZE_MD5SUM);
   } else {
      m_md5sum = calcMd5Sum(m_name, m_description, m_lat, m_lon);      
   }      
}

void
Favorite::setSynced( bool sync ) {
   m_synced = sync;
}

bool
Favorite::isSynced() const
{
   return m_synced;
}

void
Favorite::addAdditionalInfo( AdditionalInfo* a ) {
   m_infos.push_back( a );
}

uint32
Favorite::getSize() const {
   uint32 infoSize = 0;
   for ( InfoVect::const_iterator it = m_infos.begin() ; 
         it != m_infos.end() ; ++it )
      {
         infoSize += (*it)->getSize();
      }
   return (1+ 4 * 5 + 256 * 4 + 31 + 4 + 16) /* MAX size for members */ + infoSize;
}

bool 
Favorite::isMd5sumSet() const {
   // Return false if m_md5sum is only zeros
   return !(memcmp(m_md5sum, "0000000000000000", MAX_SIZE_MD5SUM) == 0);
}

namespace {
// Help method for truncateStrings.   
bool truncateString(char* strToTruncate, size_t size)
{
   bool truncate = strlen(strToTruncate) >= size;
   if(truncate){
      strToTruncate[size-1] = '\0';
   }
   return truncate;
   //       char* tmpStr = new char[MAX_SIZE_MAPICONPATH];
   //       strncpy(tmpStr, strToTruncate, size);
   //       tmpStr[size-1] = '\0';
   //       delete strToTruncate;
   //       strToTruncate = tmpStr;
}
}

bool
Favorite::truncateStrings()
{
   struct stringMaxLength {
      char* str;
      size_t max;
   } stringList[] = {{ m_name, MAX_SIZE_NAME },
                     { m_shortName, MAX_SIZE_SHORTNAME },
                     { m_description, MAX_SIZE_DESCRIPTION },
                     { m_category, MAX_SIZE_CATEGORY },
                     { m_mapIconName, MAX_SIZE_MAPICONPATH }};

   bool truncated = false;
   for(size_t i = 0; i < sizeof(stringList)/sizeof(*stringList); ++i){
      truncated = 
         truncateString(stringList[i].str, stringList[i].max) && truncated;
   }
   return truncated;
} // Favorite::truncateStrings

namespace {
// test whether two pointers are both set or both null.
inline bool nullEqual(const void* a, const void* b)
{
   return (a && b) || (!a && !b);
}
}

class Favorite* CloneFavorite(const class Favorite& aFav)
{
   class Favorite* clone = new Favorite(aFav);
   //test that no allocations failed. 
   if(!(nullEqual(aFav.m_name       , clone->m_name       ) &&
        nullEqual(aFav.m_shortName  , clone->m_shortName  ) &&
        nullEqual(aFav.m_description, clone->m_description) &&
        nullEqual(aFav.m_category   , clone->m_category   ) &&
        nullEqual(aFav.m_mapIconName, clone->m_mapIconName) &&
        nullEqual(aFav.m_lmsID      , clone->m_lmsID      ) &&
        nullEqual(aFav.m_md5sum     , clone->m_md5sum     ))){
      delete clone;
      clone = NULL;
   }
   return clone;
}


   // Cmp implementation. (used with Favorite)//////////////////

   FavoriteCmp::FavoriteCmp(GuiProtEnums::SortingType sortOrder) : 
      m_sortingOrder(sortOrder)
      {
      }

bool
FavoriteCmp::operator()(const class Favorite* first, 
                        const class Favorite* second) const
{
   /* This method should always return true if the first element is 
    * less than the second. I.e. if both are equal, false should be
    * returned, just like when the second is less than the first.
    */
   
   bool result = false;
   const char* emptyStr = "";
   const char* firstStr = first->getName();
   const char* secondStr = second->getName();
   if (firstStr == NULL){
      firstStr = emptyStr;
   }
   if (secondStr == NULL){
      secondStr = emptyStr;
   }

   switch (m_sortingOrder){
   case GuiProtEnums::newSort:
      result = ( UTF8Util::strCollateUtf8(firstStr, secondStr) < 0 );
      break;
   case GuiProtEnums::alphabeticalOnName:
   default:
      // This one uses alphabetical order for default.
      result = ( strcasecmp(firstStr, secondStr) < 0 );
   } // switch
   return result;
}

   GuiFavoriteCmp::GuiFavoriteCmp(GuiProtEnums::SortingType sortOrder) : 
      m_sortingOrder(sortOrder)
      {
      }

bool
GuiFavoriteCmp::operator()(const class GuiFavorite* first, 
                           const class GuiFavorite* second) const
{
   /* This method should always return true if the first element is 
    * less than the second. I.e. if both are equal, false should be
    * returned, just like when the second is less than the first.
    */
   
   bool result = false;
   const char* emptyStr = "";
   const char* firstStr = first->getName();
   const char* secondStr = second->getName();
   if (firstStr == NULL){
      firstStr = emptyStr;
   }
   if (secondStr == NULL){
      secondStr = emptyStr;
   }

   switch (m_sortingOrder){
   case GuiProtEnums::newSort:
      result = ( UTF8Util::strCollateUtf8(firstStr, secondStr) < 0 );
      break;
   case GuiProtEnums::alphabeticalOnName:
   default:
      // This one uses alphabetical order for default.
      result = ( strcasecmp(firstStr, secondStr) < 0 );
   } // switch
   return result;
}

FavoriteInfoCmp::FavoriteInfoCmp() {
   uint32 prio = 0;
   m_prios.insert( std::make_pair( GuiProtEnums::phone_number,  prio++ ) );
   m_prios.insert( std::make_pair( GuiProtEnums::mobile_phone,  prio++ ) );
   m_prios.insert( std::make_pair( GuiProtEnums::short_info,    prio++ ) );
   //    m_prios.insert( std::make_pair( GuiProtEnums::vis_full_address, prio++ ) );
   //    m_prios.insert( std::make_pair( GuiProtEnums::vis_address,   prio++ ) );
   //    m_prios.insert( std::make_pair( GuiProtEnums::vis_house_nbr, prio++ ) );
   //    m_prios.insert( std::make_pair( GuiProtEnums::,     prio++ ) );
   m_prios.insert( std::make_pair( GuiProtEnums::url,           prio++ ) );
}


bool
FavoriteInfoCmp::operator()( 
                            const class isab::AdditionalInfo* first,
                            const class isab::AdditionalInfo* second ) const
{
   // List of prioritized types first
   // If either is in priority list then prio compare
   // else compare types
   prioMap::const_iterator fit = m_prios.find( first->getType() );
   prioMap::const_iterator sit = m_prios.find( second->getType() );
   if (  fit != m_prios.end() || sit != m_prios.end() ) {
      if ( fit != m_prios.end() && sit != m_prios.end() ) {
         return (*fit).second < (*sit).second;
      } else if ( fit != m_prios.end() ) {
         return true; // First is prioritized
      } else {
         return false;
      }
   } else {
      return first->getType() < second->getType();
   }
}

uint8* 
Favorite::calcMd5Sum(const char* name, const char* desc, uint32 lat, uint32 lon) 
{
   int32 nameSize = strlen(name);
   int32 descSize = strlen(desc);
   char strLat[16];
   int32 latSize = sprintf(strLat, "%d", lat);
   char strLon[16];
   int32 lonSize = sprintf(strLon, "%d", lon);
   uint8* data = new uint8[nameSize + descSize + latSize + lonSize];
   memcpy(data, name, nameSize);
   memcpy(data + nameSize, desc, descSize);
   memcpy(data + nameSize + descSize, strLat, latSize);
   memcpy(data + nameSize + descSize + latSize, strLon, lonSize);
   MD5 md5;
   uint8* md5sum = md5.GetRawMd5sum(data, nameSize + descSize + latSize + lonSize);
   delete[] data;
   // Transfers ownership
   return md5sum;
}

} // namespace isab
