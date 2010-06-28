/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMBINED_SEARCH_DISPATCHER_H
#define COMBINED_SEARCH_DISPATCHER_H

#include "arch.h"
#include "GuiProt/GuiProtMessageHandler.h"
#include "GuiProt/GuiProtMess.h"
#include "SearchResultObserver.h"
#include "SearchRecord.h"
#include "NParamBlock.h"
#include "CombinedSearchDataHolder.h"
#include "TmapImageHandler.h"

/**
 * Class that handles all combined searches. Triggeres download
 * for missing images. It downloads categories, triggeres search
 * and can request more search hits from the server.
 * @class TmapImageHandler
 */
class CombinedSearchDispatcher : 
   public TmapImageObserver
{
 public:
   /**
    * Enum to keep track of different search request types.
    */
   enum SearchRequestType {
      ENotASearchRequest = 0,
      ERoundOne = 1,
      ERoundTwo = 2,
      EAreaMatch = 3, 
      EMoreHits = 4
   };

 public:

   /**
    * Creates a CombinedSearchDispatcher.
    * @param messageHandler The message handler needed for sending 
    *                       messages to Nav2.
    * @param observer The observer that will receive all the
    *                 search results when handled.
    */
   CombinedSearchDispatcher( GuiProtMessageHandler& messageHandler,
                             MSearchResultObserver& observer,
                             const char* imagePath,
                             const char* imageExt );

   virtual ~CombinedSearchDispatcher();

   /**
    * Send message to get categories, the message
    * will be received by AppUi that will pass it forward
    * to handleCombinedSearchCategoriesGot.
    */
   void requestCategories();

   /**
    * Creates a search message, the search is a area match search.
    * @param sr, the SearchRecord containing information what to search for.
    * @param heading, the id of the heading to perform the area match search for.
    */
   void dispatchAreaMatchSearch(SearchRecord& sr, uint32 heading);

   /**
    * Creates the search message to be sent to Nav2 and further 
    * to the server. Calls combinedSearchDispatchSearchMessage
    * that will send the message twice, one for round one and
    * one for round two.
    * @param searchRecord Structure holding the search conditions.
    * @return The message id.
    */  
   void combinedSearchStart(SearchRecord& sr);

   /**
    * Called from the gui when more hits needs to be
    * requested from the server.
    * @param index, The search items start index, the first item 
    *               to be downloaded.
    * @param heading, The heading that contains the search hits
    *                 to be requested from the server.
    */ 
   void requestMoreHits(uint32 index, uint32 heading);

   /** 
    * Called from message handler (AppUi) when a search failed.
    * @param aSeqId   The message id of the failed request.
    */
   void searchRequestFailed(int32 aSeqId);

   /**
    * Returns the global data holder for combined search, the returned reference
    * is const, hence no changes will be allowed.
    * @return m_dataHolder The global instance of the CombinedSearchDataHolder, the
    *                      return value is const.
    */
   const CombinedSearchDataHolder& getCombinedSearchDataHolder() const; 

   /**
    * Returns the global data holder for combined search.
    * @return m_dataHolder The global instance of the CombinedSearchDataHolder
    */
   CombinedSearchDataHolder& getCombinedSearchDataHolder(); 

   /**
    * Calls CombinedSearchDataHolder::getCombinedSearchCategories
    * @return Stored map containing all downloaded categories.
    */
   const std::map<uint32, CombinedSearchCategory*>& getCombinedSearchCategories();

   /**
    * Calls CombinedSearchDataHolder::getCombinedSearchResults
    * @return Stored vector containing all search hits for the latest search.
    */
   const std::vector<CombinedSearchCategory*>& getCombinedSearchResults();

   /**
    * Calls CombinedSearchDataHolder::getCombinedSearchCategoryResults
    * @return Stored vector containing all category results and the
    *         search results.
    */
   const std::vector<CombinedSearchCategory*>& getCombinedSearchCategoryResults();

   /**
    * Function that will be called by the receiver of NAV_SEARCH_DESC_REPLY
    * reply. Usually this is AppUi. This function needs to check if
    * all images for all categories is downloaded, if not this must
    * be done.
    * @param mess The message containing all the received categories.
    */
   void handleCombinedSearchCategoriesGot(isab::DataGuiMess *mess);

   /**
    * Called from WayfinderAppUi::ReceiveMessage when server 
    * responded to a search request. 
    * @param mess, The DataGuiMess containing all search hits and
    *              the categories.
    */
   void handleCombinedSearchResultsGot(isab::DataGuiMess *mess);

   /**
    * Returns current round for this search.
    * @return Current round.
    */
   int getCurrentRound() const;

   /**
    * Returns the search request type if the sequence id is a 
    * found to be a search request.
    *
    * @param aSeqId   The sequence id to check for.
    * @return True if the sequence id is related to a search.
    */
   CombinedSearchDispatcher::SearchRequestType IsOutstandingSearchRequest(int32 aSeqId);

public: // From TmapImageObserver

   /** 
    * Called by the TmapImageHandler when requested image
    * is downloaded
    * @param imageName, The name of the image just downloaded.
    */
   void ImageReceived(const char* imageName);

   /**
    * Called by the TmapImageHandler when something went
    * wrong with the download.
    * @param imageName, The name of the image that failed to be downloaded.
    */
   void ImageReceivedError(const char* imageName);

private:

   /**
    * Sends a search message to Nav2.
    * @param params NParamBlock that contains the search conditions.
    */
   int32 combinedSearchDispatchSearchMessage(isab::NParamBlock& params);

   /**
    * Iterates through the search results, just for a quick
    * calculation of how many search hits was received.
    * @param mess The DataGuiMess that contains all search hits.
    */
   void calcTotalNbrHits(isab::DataGuiMess* mess);

   /**
    * Checks if a image file is downloaded or not. If not
    * the image name is added to the m_combinedSearchNeededImages
    * and will be downloaded later.
    * @param imageName The name of the image to search for.
    */
   void checkAndAddImageName(const char* imageName);

   /**
    * Dowloads all images that was added into the
    * m_combinedSearchNeededImages vector.
    */
   void downloadNeededImages();

   /**
    * Compares the crc that arrives from the server with the one that we
    * have stored on file.
    */
   bool compareIconCrc( const char* serverCrc );

   /**
    * Stores the server crc to file.
    */
   void saveIconCrc( const char* serverCrc );

   /**
    * Removes all the icon files from disc.
    */
   void clearStoredIcons();

 protected:
   /// Used to send messages to the server
   GuiProtMessageHandler& m_messageHandler;
   /// The observer that gets callbacks on its requests
   MSearchResultObserver& m_observer;
   /// Container that holds all search results etc
   CombinedSearchDataHolder* m_dataHolder;
   /// Handles downloading of images connected to headings
   /// and search results
   TmapImageHandler* m_imageHandler;
   /// Vector holding all images that needs to be downloaded
   std::vector<char*> m_combinedSearchNeededImages;
   /// Path to shared folder holding all images for the combined search
   char* m_imagePath;
   /// Extension of the images to download (s60v3 uses mif, uiq3 uses png)
   char* m_imageExt;
   /// Flag that is true if categories has been downloaded, if so there is no
   /// need to download them again
   bool m_categoriesDownloaded;
   /// If 0, first round. If 1, second round. Images are not triggered to be
   /// downloaded until both rounds are received
   uint16 m_round;
   /// Just a flag so that we know if requester asked for a search or just
   /// for a downloadMorHits. Different callback functions is called depending
   /// on this flag.
   bool m_moreHitsReq;
   /// Tells us if there exist any outstanding requests already sent to the
   /// server. If so, no more searches are allowed until we receive a message
   /// for that search
   bool m_searchInitiated;
   /// Flag indicating if the current search request was a area match search or not.
   bool m_areaMatchSearch;
   /// Map to keep track of sent search requests (for error handling in 
   /// handleRequestFailed).
   std::map<int32, SearchRequestType> m_searchRequests;
};

#endif
