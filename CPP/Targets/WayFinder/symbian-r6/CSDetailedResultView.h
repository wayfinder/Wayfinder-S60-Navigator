/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CS_DETAILED_RESULT_VIEW_H
#define CS_DETAILED_RESULT_VIEW_H

#include "ViewBase.h"
#include <vector>

#include "CombinedSearchDataHolder.h"

/**
 * Avkon view class for STestDetailedResultListBoxView. It is register with the view server
 * by the AppUi. It owns the container control.
 * @class	CCSDetailedResultView STestDetailedResultListBoxView.h
 */
class CCSDetailedResultView : public CViewBase
{
public:
   // constructors and destructor
   CCSDetailedResultView(class CWayFinderAppUi* aWayfinderAppUi);
   static CCSDetailedResultView* NewL(class CWayFinderAppUi* aWayfinderAppUi);
   static CCSDetailedResultView* NewLC(class CWayFinderAppUi* aWayfinderAppUi);
   void ConstructL();
   virtual ~CCSDetailedResultView();

public:
   // from base class CAknView
   TUid Id() const;
   void HandleCommandL( TInt aCommand );
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

protected:
   // from base class CAknView
   void DoActivateL( const TVwsViewId& aPrevViewId,
                     TUid aCustomMessageId,
                     const TDesC8& aCustomMessage );
   void DoDeactivate();

public:

   /**
    * Called when list should be populated, calls AddResultsL
    * in its container.
    * @param aIndex, The index of the chosen heading/category.
    * @param aResults, Vector that contains all search results. Holding
    *                 CombinedSearchCategories which contains search results
    *                 for each heading.
    */
   void AddResultsL(TInt aIndex, const CSCatVec_t& aResults, 
                    TBool aMoreServerHitsReceived = EFalse);

   /**
    * Calls wayfinderappuis member iPathFinder for receiving
    * the path to the icons downloaded by CombinedSearchImageHandler
    * @return The path to downloaded icons.
    */
   TPtrC GetCSIconPath();

   /**
    * Calls wayfinderappuis member iPathFinder for receiving
    * the name and path to the mbm file.
    * @return The path to the mbm file.
    */
   TPtrC GetMbmName();

   /**
    * Gets current distance mode.
    * @return Current distance mode.
    */
   TInt GetDistanceMode();

   /**
    * Returns the global vector holding all the search results.
    * @return The vector containing the searchresults.
    */
   const std::vector<CombinedSearchCategory*>& GetSearchResults();

   /**
    * Gets the the index for the current category.
    * @return index for the current category.
    */
   TInt GetCSCategoryCurrentIndex();

   /**
    * User has pressed next on the last page but there still exists
    * more hits on the server, not yet downloaded. Start downloading
    * these, this will result in a call to 
    * WayFinderAppUi::MoreSearchResultReceived from the CombinedSearchDispatcher
    * when the complementing results are downloaded. This will result in a call
    * from appUi to AddResultsL.
    * @param aLowerBound, The index of the current last index + 1,
    *                     the first item to be fetched.
    * @param aHeadingNo, The heading index to get the search results for.
    */
   void RequestMoreHits(TUint32 aLowerBound, TUint32 aHeadingNo);
   
   /**
    * Resets iOffset and iIndex, if we dont do this, we might start
    * displaying strange indexes when coming back to this view for displaying
    * different search results for a different search.
    */
   void ResetIndexes();

   /**
    * Helps decide wich menu options that should be visible and
    * wich soft keys that should be visible or not.
    * @return Returns ETrue if we are coming from the route planner
    *         and if we wants to set a search result as origin.
    */
   TBool SettingOrigin();

   /**
    * Helps decide wich menu options that should be visible and
    * wich soft keys that should be visible or not.
    * @return Returns ETrue if we are coming from the route planner
    *         and if we wants to set a search result as destination.
    */
   TBool SettingDestination();

   /**
    * Asks wayfinderappui if gps is allowed and if this client
    * is allowed to use a gps for everything.
    * @return True if there is a gps and it can be used.
    *         False if not.
    */
   TBool IsGpsAllowed();

   TBool ValidGpsStrength();

   TPoint GetCurrentPosition();

   /**
    * Returns the searchItem that is located at the index in
    * the global search hit vector.
    * @param aIndex, The index of the searchItem to be fetched.
    *                this index is not the listbox index but the
    *                index returned by GetCalculatedIndex().
    * @return The searchItem object located at aIndex.
    */
   const isab::SearchItem& GetSelectedSearchItem( TInt aIndex );

   /**
    * A static wrapper as a callback to be able to call the real 
    * NavigateToCallback() function from appui
    * by a function pointer to this wrapper. This is used in the gps wizard 
    * to be able to know if/when lbs connected to the gps so it is okay
    * to order the route.
    * This is necessary since we have different objects with different 
    * member functions that can call the gps wizard.
    */
   static void WrapperToNavigateToCallback(void* pt2Object);

private:

   /**
    * Show search hit on map.
    */
   void ShowOnMap();

   /**
    * Asks its container to get the correct index, not the index
    * in the list, but the correct index. With correct index, means
    * if the offset is 1 and the list index is 2, then the correct
    * index is 25 x 1 + list index, which gives us 27.
    * With this index we can get the search item that the user
    * wants to see more information about. We send the position
    * and the id of the search item to appui that then displays
    * the information in the service window.
    */
   void ShowMoreInformation();

   /**
    * Gets the name and desc from the listbox for the
    * selected index. Get the searcItem object for this
    * index 
    */
   void NavigateTo();

   /**
    * NavigateToCallback() function from called form appui via the 
    * static WrapperToNavigateToCallback() function.
    */
   void NavigateToCallback();

   /**
    * Does the same as NavigateTo but instead of calling
    * WayFinderAppUi::NavigateToSearchRes we call
    * WayFinderAppUi::SetOrigin wich calls route planner
    * to set the origin.
    */
   void SetAsOrigin();

   /**
    * Does the same as NavigateTo but instead of calling
    * WayFinderAppUi::NavigateToSearchRes we call
    * WayFinderAppUi::SetDestination wich calls route planner
    * to set the destination.
    */
   void SetAsDestination();
   
   /**
    * Gets the currently selected search item and requests a route to it.
    */
   void Route();

   /**
    * Gets called when user wants to save a search item as a
    * favorite. Gets the selected search item and calls
    * WayFinderAppUi::AddFavoriteFromSearch which
    * stores this search item as a favorite.
    */
   void SaveAsFavorite();

   /**
    * Gets called when user wants to send a search resykt by
    * email or sms. Gets the selected search item and calls
    * WayFinderAppUi::SendFavoriteL which starts the 
    * CCommunicationWizard.
    * Transfer ownership of the newly created favorite.
    */
   void SendAsFavoriteL();

private:
   /// The container holding the listbox
   class CCSDetailedResultContainer* iContainer;
   /// The offset in the list, represents current page   
   TInt iOffset;
   /// Total number of search hits for his search
   TInt iIndex;
};

#endif // STESTDETAILEDRESULTLISTBOXVIEW_H			
