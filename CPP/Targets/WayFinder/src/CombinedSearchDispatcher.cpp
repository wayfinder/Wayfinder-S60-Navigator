/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "CombinedSearchDispatcher.h"

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "nav2util.h"
#include "NavServerComProtoVer.h"
#include "NavRequestType.h"
#include "GuiProt/SearchRegion.h"


using namespace isab;

#define MAX_REPLIES_IN_HEADING 100
#define IMG_CRC_FILENAME "imgcrc.dat"

CombinedSearchDispatcher::CombinedSearchDispatcher(GuiProtMessageHandler& messageHandler,
                                                   MSearchResultObserver& observer,
                                                   const char* imagePath,
                                                   const char* imageExt) :
   m_messageHandler( messageHandler ),
   m_observer( observer ),
   m_imageHandler( NULL ),
   m_categoriesDownloaded( false ),
   m_moreHitsReq( false ),
   m_searchInitiated( false ),
   m_areaMatchSearch( false )
{
   m_dataHolder = new CombinedSearchDataHolder();
   m_imagePath = strdup_new(imagePath);
   m_imageExt = strdup_new(imageExt);
   m_searchRequests.clear();
}

CombinedSearchDispatcher::~CombinedSearchDispatcher()
{
   std::vector<char*>::iterator it = m_combinedSearchNeededImages.begin();
   while(it != m_combinedSearchNeededImages.end()) {
      delete[] (*it);
      ++it;
   }
   m_combinedSearchNeededImages.clear();
   delete m_dataHolder;
   delete[] m_imagePath;
   delete[] m_imageExt;
   m_searchRequests.clear();
}

void CombinedSearchDispatcher::requestCategories()
{   
   isab::Buffer *buf = new isab::Buffer(10);
   NParamBlock params;

   Buffer tmpbuf(128);

   std::vector< byte > bbuf;

   params.writeParams( bbuf, NSC_PROTO_VER, false /* NO GZIP */ );
   tmpbuf.writeNextByteArray( &bbuf.front(), bbuf.size() );
   const uint8* data = tmpbuf.accessRawData();
   uint32 size = tmpbuf.getLength();

   buf->writeNextUnaligned16bit( navRequestType::NAV_SEARCH_DESC_REQ );

   class DataGuiMess mess( GuiProtEnums::GUI_TO_NGP_DATA,
                           buf->getLength(), buf->accessRawData(),
                           size, data);

   m_messageHandler.SendMessage(mess);
   delete buf;
}

void CombinedSearchDispatcher::dispatchAreaMatchSearch(SearchRecord& sr, uint32 heading)
{
   if (m_searchInitiated) {
      return;
   }
   m_searchInitiated = true;
   m_searchRequests.clear();

   NParamBlock params;

    // Set up all the params needed for the search
   params.addParam( NParam( 1201, sr.m_areaName ) );
   params.addParam( NParam( 1202, sr.m_areaId ) );
   if ( sr.m_categoryId && ( strlen(sr.m_categoryId) > 0 ) ) {
      // If the user selected a category we send that instead of 
      // a search string.
      params.addParam( NParam( 1204, sr.m_categoryId )  );
   } else {
      // The user has not selected a category to search for so send 
      // the search string that he entered.
      params.addParam( NParam( 1203, sr.m_searchString ) );
   }
   params.addParam( NParam( 1205, sr.m_regionId ) );
   params.addParam( NParam( 5600, uint32( 0 ) ) );
   params.addParam( NParam( 5601, uint32( 1 ) ) );
   params.addParam( NParam( 5602, heading ) );
   params.addParam( NParam( 5603, uint32( MAX_REPLIES_IN_HEADING ) ) );

   NParam& p = params.addParam( NParam( 1000 ) );
   p.addInt32( sr.m_origLat );
   p.addInt32( sr.m_origLon );
   p.addUint16( uint16(-1) );

   m_round = 1;
   m_moreHitsReq = false;
   m_areaMatchSearch = true;
   int32 seqId = combinedSearchDispatchSearchMessage( params );
   m_searchRequests.insert(std::make_pair(seqId, EAreaMatch));
}

void CombinedSearchDispatcher::combinedSearchStart(SearchRecord& sr)
{
   if (m_searchInitiated) {
      // If we already have an outstanding search request dont search again.
      return;
   }
   m_searchInitiated = true;
   m_searchRequests.clear();

   if ( !m_categoriesDownloaded ) {
      requestCategories();
   }

   m_dataHolder->clearCombinedSearchCategoryResults();
   m_dataHolder->clearCombinedSearchResults();

   m_dataHolder->setSearchRecord(sr);
   NParamBlock params;

   // Get the reference to the stored search record
   SearchRecord& searchRecord = m_dataHolder->getSearchRecord();

   // Set up all the params needed for the search
   params.addParam( NParam( 1201, searchRecord.m_areaName ) );
   params.addParam( NParam( 1202, searchRecord.m_areaId ) );
   if ( searchRecord.m_categoryId && 
        ( strlen(searchRecord.m_categoryId) > 0 ) ) {
      // If the user selected a category we send that instead of 
      // a search string.
      params.addParam( NParam( 1204, searchRecord.m_categoryId )  );
   } else {
      // The user has not selected a category to search for so send 
      // the search string that he entered.
      params.addParam( NParam( 1203, searchRecord.m_searchString ) );
   }
   params.addParam( NParam( 1205, searchRecord.m_regionId ) );
   uint32 index = 0;
   params.addParam( NParam( 5600, uint32( 0 ) ) );
   NParam &search_round = params.addParam( NParam( 5601, index ) );
   uint32 heading = MAX_UINT32;
   params.addParam( NParam( 5602, heading ) );
   uint32 nbrReplies = MAX_REPLIES_IN_HEADING;
   params.addParam( NParam( 5603, nbrReplies ) );

   NParam& p = params.addParam( NParam( 1000 ) );

   p.addInt32( searchRecord.m_origLat );
   p.addInt32( searchRecord.m_origLon );
   p.addUint16( uint16(-1) );

   int32 seqId = 0;
   m_round = 0;
   m_moreHitsReq = false;
   m_areaMatchSearch = false;
   seqId = combinedSearchDispatchSearchMessage( params );
   m_searchRequests.insert(std::make_pair(seqId, ERoundOne));
   uint32 round = 1;
   search_round = NParam( 5601, round );
   seqId = combinedSearchDispatchSearchMessage( params );
   m_searchRequests.insert(std::make_pair(seqId, ERoundTwo));
}

void CombinedSearchDispatcher::requestMoreHits( uint32 index, uint32 heading ) 
{
   NParamBlock params;
   // Get the reference to the stored search record
   SearchRecord& searchRecord = m_dataHolder->getSearchRecord();
   
   // Set up all the params needed for the search
   params.addParam( NParam( 1201, searchRecord.m_areaName ) );
   params.addParam( NParam( 1202, searchRecord.m_areaId ) );

   if ( searchRecord.m_categoryId && 
        ( strlen(searchRecord.m_categoryId) > 0 ) ) {
      // If the user selected a category we send that instead of 
      // a search string.
      params.addParam( NParam( 1204, searchRecord.m_categoryId )  );
   } else {
      // The user has not selected a category to search for so send 
      // the search string that he entered.   
      params.addParam( NParam( 1203, searchRecord.m_searchString ) );
   }
   params.addParam( NParam( 1205, searchRecord.m_regionId ) );
   params.addParam( NParam( 5600, index ));
   params.addParam( NParam( 5601, uint32( 0 ) ) );
   params.addParam( NParam( 5602, heading ) );
   uint32 nbrReplies = MAX_REPLIES_IN_HEADING;
   params.addParam( NParam( 5603, nbrReplies ) );
   
   NParam& p = params.addParam( NParam( 1000 ) );
   
   p.addInt32( searchRecord.m_origLat );
   p.addInt32( searchRecord.m_origLon );
   p.addUint16( uint16(-1) );
   
   m_round = 1;
   m_moreHitsReq = true;
   m_areaMatchSearch = false;
   int32 seqId = combinedSearchDispatchSearchMessage( params );
   m_searchRequests.insert(std::make_pair(seqId, EMoreHits));
}

int32 CombinedSearchDispatcher::combinedSearchDispatchSearchMessage( NParamBlock& params )
{
   isab::Buffer *buf = new isab::Buffer( 10 );

   Buffer tmpbuf( 128 );

   std::vector< byte > bbuf;

   params.writeParams(bbuf, NSC_PROTO_VER, false /* NO GZIP */);
   tmpbuf.writeNextByteArray( &bbuf.front(), bbuf.size() );
   const uint8* data = tmpbuf.accessRawData();
   uint32 size = tmpbuf.getLength();

   buf->writeNextUnaligned16bit(navRequestType::NAV_COMBINED_SEARCH_REQ);
//    // 2 is the request version (NAV_COMBINED_SEARCH_REQ V.2)
//    buf->writeNext8bit( 2 );

   class DataGuiMess mess( GuiProtEnums::GUI_TO_NGP_DATA,
                           buf->getLength(), buf->accessRawData(),
                           size, data, 2 );

   int32 messageId = m_messageHandler.SendMessage(mess);

   delete buf;

   return messageId;
}

void CombinedSearchDispatcher::handleCombinedSearchCategoriesGot(isab::DataGuiMess *mess)
{
   NParamBlock params(mess->getAdditionalData(), 
                      mess->getAdditionalSize(),
                      NSC_PROTO_VER);

   const NParam *crcParam = params.getParam(28);
   if (crcParam) {
      const char* crc = crcParam->getString();
      // Compare server crc with the stored crc in file.
      if ( !compareIconCrc(crc) ) {
         // Remove all files in csicons dir if crc is not the same.
         clearStoredIcons();
         // Write new crc txt file to csicons dir.
         saveIconCrc( crc );
      }
   }

   const NParam *param = params.getParam(5900);

   isab::NonOwningBuffer *bff = new isab::NonOwningBuffer(const_cast<uint8*>(param->getBuff()),
                                                          param->getLength());

   std::map<uint32, CombinedSearchCategory*> categories;

   while( bff->remaining() >= 10 ) {
      CombinedSearchCategory* tmp = new CombinedSearchCategory( bff );
      categories.insert( std::make_pair( tmp->getHeadingNo(), tmp ) );
   }

   std::vector<char*>::iterator it = m_combinedSearchNeededImages.begin();
   while(it != m_combinedSearchNeededImages.end()) {
      // Delete images
      delete[] (*it);
      ++it;
   }
   m_combinedSearchNeededImages.clear();
   m_combinedSearchNeededImages.reserve( categories.size() );

   std::map<uint32, CombinedSearchCategory*>::const_iterator cit;
   for( cit = categories.begin(); cit != categories.end(); ++cit ) {
      CombinedSearchCategory* cat = cit->second;
      // Check if all images are downloaded, if not download them
      // Check if file already exists.
      const char* imageName = cat->getImageName();
      checkAndAddImageName(imageName);
   }

   // Store the map in the data holder, setCombinedSearchCategories
   // calls swap and takes ownership
   m_dataHolder->setCombinedSearchCategories( categories );
   // Call the observer with SearchCategoriesReceived
   m_observer.SearchCategoriesReceived();
   // Request a download of needed images
   downloadNeededImages();

   delete bff;
}

bool CombinedSearchDispatcher::compareIconCrc( const char* serverCrc )
{
   char filename[] = IMG_CRC_FILENAME;
   char* completeName = new char[strlen(m_imagePath) + strlen(filename) + 1];
   strcpy(completeName, m_imagePath);
   strcat(completeName, filename);
   FILE* crcFile = fopen(completeName, "r");
   if (crcFile != NULL) {
      fseek(crcFile, 0, SEEK_END);
      int fileSize = ftell(crcFile);
      rewind(crcFile);
      char* storedCrc = new char[fileSize + 1];
      fread(storedCrc, 1, fileSize, crcFile);
      fclose(crcFile);
      storedCrc[fileSize] = '\0';
      bool equalCrc = (strcmp(storedCrc, serverCrc) == 0);
      delete[] storedCrc;
      delete[] completeName;
      if (equalCrc) {
         return true;
      } else {
         return false;
      }
   } else {
      delete[] completeName;
      return false;
   }
}

void CombinedSearchDispatcher::saveIconCrc( const char* serverCrc )
{
   char filename[] = IMG_CRC_FILENAME;
   char* completeName = new char[strlen(m_imagePath) + strlen(filename) + 1];
   strcpy(completeName, m_imagePath);
   strcat(completeName, filename);
   FILE* crcFile = fopen(completeName, "w");
   if (crcFile != NULL) {
      fwrite(serverCrc, 1, strlen(serverCrc), crcFile);
      fclose(crcFile);
   }
   delete[] completeName;
}

void CombinedSearchDispatcher::clearStoredIcons()
{
   //open image dir
   DIR* pDir = opendir(m_imagePath);
   struct dirent* pDirentStruct;
   struct stat st;
   if (pDir != NULL) {
      while( (pDirentStruct = readdir(pDir)) ) {
         //skip if find . and ..
         if ((strcmp(pDirentStruct->d_name, ".") == 0 ||
              strcmp(pDirentStruct->d_name, "..") == 0)) {
            continue;
         }
         // get the image files
         if (strstr(pDirentStruct->d_name, m_imageExt)) {
            char* filename = pDirentStruct->d_name;
            char* completeName = 
               new char[strlen(m_imagePath) + strlen(filename) + 1];
            strcpy(completeName, m_imagePath);
            strcat(completeName, filename);
            int statret = stat(completeName, &st);
            if (statret != -1 && S_ISDIR(st.st_mode)) {
               // This is a directory not a file, do nothing.
            } else {
               // This is a img file, delete it.
               remove(completeName);
            }
            delete[] completeName;
         }
      }
      closedir(pDir);
   }
}

void CombinedSearchDispatcher::checkAndAddImageName(const char* imageName)
{
   if (imageName && strcmp(imageName, "") != 0 ) {
      char* completeName = new char[strlen(m_imagePath) + strlen(imageName) + 
                                    strlen(m_imageExt) + 1];
      strcpy(completeName, m_imagePath);
      strcat(completeName, imageName);
      strcat(completeName, m_imageExt);
      FILE* imageFile = fopen(completeName, "r");
      if (imageFile == NULL) {
         // m_combinedSearchNeededImages takes ownership over completeName
         bool addImgForDownload = m_combinedSearchNeededImages.empty();
         std::vector<char*>::const_iterator it = m_combinedSearchNeededImages.begin();
         while( it != m_combinedSearchNeededImages.end() ) {
            if ( strstr((*it), completeName) != NULL ) {
               // Found the matching string, dont add it again
               addImgForDownload = false;
               break;
            } else {
               addImgForDownload = true;
            }
            ++it;
         }
         if (addImgForDownload) {
            m_combinedSearchNeededImages.push_back( completeName );
         } else {
            delete[] completeName;
         }
      } else {
         fclose(imageFile);
         delete[] completeName;
      }
   }
}

void CombinedSearchDispatcher::calcTotalNbrHits(isab::DataGuiMess* mess)
{
   const std::map<uint32, CombinedSearchCategory*>& cats = m_dataHolder->getCombinedSearchCategories();

   isab::NParamBlock params(mess->getAdditionalData(), mess->getAdditionalSize(), NSC_PROTO_VER);
   int categories = cats.size();
   uint32 headNo = 0;
   for( int a = 0; a < categories; ++a ) {
      // Iterate through all categegories received from NAV_SEARCH_DESC_REPLY
      // Get the heading from the parameter
      const NParam *hdng = params.getParam(5700 + a * 2);
      if( hdng ) {
         isab::NonOwningBuffer* bff = new isab::NonOwningBuffer( const_cast<uint8 *>( hdng->getBuff() ),
                                                                 hdng->getLength() );
         // Store the current read pos, since bff doesnt copy the data 
         // we dont want to break the buffer for later reading.
         uint32 startReadPos = bff->getReadPos();

         // Get the heading, neede for mapping in the data holder         
         headNo = bff->readNext32bit();
         uint32 type = bff->readNext32bit();
         uint32 totalNbrHits = 0;   
         uint32 topHits = 0;
         // Ignore, we just want number of hits
         bff->readNext32bit(); 
         // Get total number or hits
         totalNbrHits = bff->readNext32bit(); 
         // Get the number of top hits for this heading
         topHits = bff->readNext32bit();
         if (m_areaMatchSearch) {
            m_dataHolder->replaceSearchCategoryResults(headNo, type, totalNbrHits, topHits);
         } else {
            // Store the heading and the total number of hits in the data holder
            m_dataHolder->setSearchCategoryResults(headNo, type, totalNbrHits, topHits);
         }
         // Reset the read pos in the buffer
         bff->setReadPos(startReadPos);
         delete bff;
      }
   }
   // Report to the observer, no search hits are actually processed here, only
   // number of hits for each heading is processed and reported. Processing of 
   // search results will be done a bit later.
   if (!m_areaMatchSearch) {
      m_observer.TotalNbrHitsReceived(m_dataHolder->getCombinedSearchCategoryResults());
   }
}

void CombinedSearchDispatcher::handleCombinedSearchResultsGot(isab::DataGuiMess *mess)
{
   if (!m_moreHitsReq) {
      // Get number of hits for each heading and report to the observer, processing of 
      // search hits will be done after this.
      calcTotalNbrHits(mess);
   }
   const std::map<uint32, CombinedSearchCategory*>& cats = m_dataHolder->getCombinedSearchCategories();

   // Create the param that will contain the actuall search hits
   isab::NParamBlock params(mess->getAdditionalData(), mess->getAdditionalSize(), NSC_PROTO_VER);

   const NParam* titleParam = params.getParam(6000);
      
   if (titleParam) {
      // Create a buffer that will the two titles, advertisement 
      // title and normal heading title.
      isab::NonOwningBuffer* titleBuf = new isab::NonOwningBuffer( const_cast<uint8 *>( titleParam->getBuff() ),
                                                                   titleParam->getLength() );
      
      m_dataHolder->setAdvertHeadingTitle(titleBuf->getNextCharString());
      m_dataHolder->setNormHeadingTitle(titleBuf->getNextCharString());
      delete titleBuf;
   }

   // Vector that will hold all top hits found in all the categories
   std::vector<isab::SearchItem> topHitItems;
   int categories = cats.size();
   for( int a = 0; a < categories; ++a ) {
      // Iterate through all categegories received from NAV_SEARCH_DESC_REPLY
      // Get the heading from the parameter
      const NParam *hdng = params.getParam(5700 + a * 2);
      // Get the regions for above heading
      const NParam *rgns = params.getParam(5701 + a * 2);
      if( hdng ) {
         // Create a buffer that will contain all heading data.
         isab::NonOwningBuffer* bff = new isab::NonOwningBuffer( const_cast<uint8 *>( hdng->getBuff() ),
                                                                 hdng->getLength() );

         std::vector<isab::SearchItem> items;
         std::vector<isab::SearchRegion *> regions;
          
         if( rgns ) {
            isab::NonOwningBuffer *rgn_bff = new isab::NonOwningBuffer( const_cast<uint8 *>( rgns->getBuff() ),
                                                                        rgns->getLength() );

            while( rgn_bff->remaining() ){
               regions.push_back( new isab::SearchRegion( rgn_bff ) );
            }

            delete rgn_bff;
         }
      
         uint32 headNo = bff->readNext32bit();
         uint32 type = bff->readNext32bit();
         uint32 totalNbrHits = 0;
         uint32 topHits = 0;
         char* advertHitTitle = NULL;
         char* normHitTitle = NULL;
         // Index of first match, skipped since SearchItem does not 
         // a member for storing this
         bff->readNext32bit(); 
         // Get the total number of hits
         totalNbrHits = bff->readNext32bit();
         // Get the number of top hits for this heading
         topHits = bff->readNext32bit();
         // Get the advert title
         advertHitTitle = bff->getNextCharStringAlloc();
         // Get the normal heading title
         normHitTitle = bff->getNextCharStringAlloc();
         int topHitsAdded = 0;
         //          if( type == 0 ) {
         uint16 searchType = 0;
         // Search hit variables
         uint8 subType = 0;
         uint8 advert = 0;
         long lat = 0;
         long lon = 0;
         int distance = -1;
         
         while( bff->remaining() ) {
            if (type == 0) {
               // Type is a searh hit
               // Search type, SearchItemType enum
               searchType = bff->readNext8bit();
               // Sub type, SearchItemsSubType enum
               subType = bff->readNext8bit();
               // Determines if advertisement or not
               advert = bff->readNext8bit();
               // Internal id of the match
            } else if (type == 1) {
               searchType = bff->readNextUnaligned16bit();
            }
            const char *id = bff->getNextCharString();
            if( id ) {
               const char *name = bff->getNextCharString();
               // skip image name since SearchItem does not contain it
               const char *imageName = bff->getNextCharString();  
               if (type == 0) {
                  lat = bff->readNextUnaligned32bit();
                  lon = bff->readNextUnaligned32bit();
               }
               int rgn_cnt = bff->readNext8bit();
               SearchRegion** rgns_temp = 0;
               if( rgn_cnt > 0 ) {
                  rgns_temp = new SearchRegion*[ rgn_cnt ];
                  for( int c = 0; c < rgn_cnt; ++c ) {
                     int index = bff->readNextUnaligned16bit();
                      
                     if( ( (int)regions.size() > index ) && ( index >= 0 ) ) { 
                        rgns_temp[ c ] = regions[ index ];
                     } else {
                        --c;
                        --rgn_cnt;
                     }
                  }
               } else {
                  // Just to make sure we don't send any negative numbers when
                  // creating the searchitem
                  rgn_cnt = 0;
               }
               if (topHitsAdded < (int)topHits) {
                  // Add the top hit to the vector
                  topHitItems.push_back( SearchItem( name, id, searchType, subType, distance, 
                                                     rgns_temp, rgn_cnt, lat, lon, 2, 
                                                     imageName, advert ) );
                  topHitsAdded++;
               }

               items.push_back( SearchItem( name, id, searchType, subType, distance, 
                                            rgns_temp, rgn_cnt, lat, lon, 2, 
                                            imageName, advert ) );

               if (rgns_temp) {
                  delete[] rgns_temp;
               }
               checkAndAddImageName(imageName);
            } else {
               break;
            }
         }
         if (m_moreHitsReq) {
            m_dataHolder->appendSearchResults(headNo, items);
         } else if (m_areaMatchSearch) {
            // Got the results from an area match search, replace existing
            // category.
            m_dataHolder->replaceSearchResults(headNo, 
                                               type, 
                                               totalNbrHits, 
                                               topHits, 
                                               advertHitTitle,
                                               normHitTitle,
                                               items);
         } else {
            m_dataHolder->setSearchResults(headNo, 
                                           type,
                                           totalNbrHits, 
                                           topHits,
                                           advertHitTitle,
                                           normHitTitle,
                                           items);
         }

         // Delete title strings since they have been allocated
         delete advertHitTitle;
         delete normHitTitle;

         std::vector<isab::SearchRegion *>::iterator a = regions.begin();
         std::vector<isab::SearchRegion *>::iterator end = regions.end();
         while( a != end ) {
            delete ( * ( a++ ) );  
         }

         delete bff;
      }
   }
   // Set the top hits to the data holder. 
   m_dataHolder->setTopHitResults( topHitItems );
   
   if (m_round == 1 ){
      m_searchInitiated = false;
      downloadNeededImages();
   }
   m_round = 1;

   // This could be used if we want the gui to wait until both round one and two are finished,
   // if the gui should be updated for each round this should not be used.
   if (m_moreHitsReq) {
      m_observer.MoreSearchResultReceived();
   } else if (m_areaMatchSearch) {
      // Call to the observer that received reply on area match search request.
      m_observer.AreaMatchSearchResultReceived();
   } else {
      m_observer.SearchResultReceived();
   }
}

void CombinedSearchDispatcher::downloadNeededImages()
{
   if( m_combinedSearchNeededImages.size() ) {
      if( m_imageHandler ) {
         delete m_imageHandler;
      }
      m_imageHandler = new TmapImageHandler(m_messageHandler, *this);

      // Iterate through the vector and download all the needed images.
      std::vector<char*>::const_iterator it = m_combinedSearchNeededImages.begin();
      while( it != m_combinedSearchNeededImages.end() ) {
         m_imageHandler->FetchImage((*it));
         ++it;
      }
   } else {
      m_categoriesDownloaded = true;
      m_observer.RequestedImagesDownloaded();
   }
}

void CombinedSearchDispatcher::ImageReceived(const char* imageName)
{
   std::vector<char*>::iterator it = m_combinedSearchNeededImages.begin();
   while( it != m_combinedSearchNeededImages.end() ) {
      if (strstr((*it), imageName) != NULL) {
         // Found the matching string, erase since we got it from the server
         delete[] (*it);
         m_combinedSearchNeededImages.erase(it);
         break;
      } 
      ++it;
   }
   if (m_combinedSearchNeededImages.size() == 0) {
      // All images downloaded, begin search.
      delete m_imageHandler;
      m_imageHandler = NULL;
      m_categoriesDownloaded = true;
      m_observer.RequestedImagesDownloaded();
   }
}

void CombinedSearchDispatcher::ImageReceivedError(const char* imageName)
{
   // No error handling for the moment, if the image failed to be dowloaded
   // we will display the fall back image.
   ImageReceived(imageName);
}

void CombinedSearchDispatcher::searchRequestFailed(int32 aSeqId)
{
   // Message receiver got a request_failed
   m_searchInitiated = false;

   std::map<int32, SearchRequestType>::iterator it = 
      m_searchRequests.find(aSeqId);
   m_searchRequests.erase(it);
}

const CombinedSearchDataHolder&
CombinedSearchDispatcher::getCombinedSearchDataHolder() const
{
   return *m_dataHolder;
}

CombinedSearchDataHolder&
CombinedSearchDispatcher::getCombinedSearchDataHolder() 
{
   return *m_dataHolder;
}

const std::map<uint32, CombinedSearchCategory*>&
CombinedSearchDispatcher::getCombinedSearchCategories() 
{
   return m_dataHolder->getCombinedSearchCategories();
}

const std::vector<CombinedSearchCategory*>&
CombinedSearchDispatcher::getCombinedSearchResults() 
{
   return m_dataHolder->getCombinedSearchResults();
}

const std::vector<CombinedSearchCategory*>&
CombinedSearchDispatcher::getCombinedSearchCategoryResults()
{
   return m_dataHolder->getCombinedSearchCategoryResults();
}

int CombinedSearchDispatcher::getCurrentRound() const
{
   return m_round;
}

CombinedSearchDispatcher::SearchRequestType 
CombinedSearchDispatcher::IsOutstandingSearchRequest(int32 aSeqId)
{
   std::map<int32, SearchRequestType>::iterator it = 
      m_searchRequests.find(aSeqId);
   if (it == m_searchRequests.end() ) {
      // The seqence id was not an outstanding search requests.
      return ENotASearchRequest;
   }
   return it->second;
}
