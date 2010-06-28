/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CS_CATEGORY_RESULT_VIEW_H
#define CS_CATEGORY_RESULT_VIEW_H

#include "ViewBase.h"
#include "EventGenerator.h"

enum TCSState {
   EAreaMatchSelected,
   EAreaMatchCancelled
};


class CWayFinderAppUi;

/**
 * Avkon view class for STestResultListBoxView. It is register with the view server
 * by the AppUi. It owns the container control.
 * @class	CCSCategoryResultView STestResultListBoxView.h
 */
class CCSCategoryResultView : public CViewBase
{
public:
   // constructors and destructor
   CCSCategoryResultView(CWayFinderAppUi* aWayfinderAppUi);
   static CCSCategoryResultView* NewL(CWayFinderAppUi* aWayfinderAppUi);
   static CCSCategoryResultView* NewLC(CWayFinderAppUi* aWayfinderAppUi);        
   void ConstructL();
   virtual ~CCSCategoryResultView();

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
    * Returns the path to the directory holding all images
    * for the combined search.
    * @return A string containing the correct path.
    */
   TPtrC GetCSIconPath();

   /**
    * Returns the the name of the mbm file.
    * @return The name of the mbm file.
    */
   TPtrC GetMbmName();

   /**
    * Adds all the categories from the search.
    */
   void AddCategories();

   /**
    * Adds all the categories from the search.
    *
    * @param aCategoryVec   The vector of categories to be added to listbox
    */
   void AddCategories(const std::vector<class CombinedSearchCategory*>& categories);

   /**
    * Called from the event generator when we're ready to add more categories.
    */
   void RetryAddCategories();

   /**
    * Returns the currently selected index from the listbox, compensates
    * for top hits.
    *
    * @return The currently selected index
    */
   TInt GetCurrentIndex();

   /**
    * Checks if gps is connected and allowed to use.
    * @return ETrue if connected and allowed to use.
    *         EFalse if not connected or not allowed to use.
    */
   TBool IsGpsAllowed();

   /**
    * Checks if user came from the route planner and
    * is going to set the search res as origin.
    * @return ETrue if coming from route planner and setting origin.
    */
   TBool SettingOrigin();

   /**
    * Checks if user came from the route planner and
    * is going to set the search res as destination.
    * @return ETrue if coming from route planner and setting destination.
    */
   TBool SettingDestination();

   /** 
    * Generates av event.
    * @param aEvent The event to be generated.
    */
   void GenerateEvent(enum TCSState aEvent);

   /**
    * Handles the generated event.
    * @param eEvent The event to be handled.
    */
   void HandleGeneratedEventL(enum TCSState aEvent);

   /**
    * Called when an area match search result is received.
    * Updates the category that made the area match search and
    * switches to detailed result view if aSwitchToResultView is true.
    * @param aSwitchToResultView, if true we automatically switches to
    *                             CCSDetailedResultView.
    */
   void AreaMatchSearchResultReceived(TBool aSwitchToResultView);

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
    * Checks to see if the selected category is empty or not.
    * @return ETrue if the selected category is empty.
    */
   TBool SelectedCategoryEmpty();

   /**
    * Returns the type of hits in the selected category.
    * @return, 0 if the selected category contains search hits
    *          1 if containing area matches.
    */
   TInt TypeOfHitsInSelectedCategory();

   /**
    * Displayes a listbox containing all the area matches received
    * in the search result for the selected category.
    */
   void DisplayAreaMatchesListL();

   /**
    * Show search hit on map.
    */
   void ShowOnMap();

   /**
    * Shows more information about this top hit. We send the position
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
   /// The container containing the listbox.
   class CCSCategoryResultContainer* iContainer;
   /// The current index, compensated for top hits.
   /// This index will match the vector in the
   /// CombinedSearchDataHolder holding all the 
   /// categories.
   TInt iCurrIndex;
   /// Holds the current selected index in the listbox.
   TInt iCurrentListBoxIndex;
   /// Typedef for the eventgenerator
   typedef CEventGenerator<CCSCategoryResultView, enum TCSState> CSCatGenerator;
   /// Event generator, used to generate event 
   CSCatGenerator* iEventGenerator;
   /// Keeps track of the index from which the area match search was triggered.
   TInt iAreaMatchHeadingIndex;
   /// The selected index from the selection list dialog
   TInt iSelectedIndex;
};

#endif // STESTRESULTLISTBOXVIEW_H			
