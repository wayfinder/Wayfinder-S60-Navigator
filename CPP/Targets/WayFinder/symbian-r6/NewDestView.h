/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NEWDESTVIEW_H
#define NEWDESTVIEW_H

// INCLUDES
#include "ViewBase.h"
#include "WayFinderConstants.h"
#include "Log.h"
#include "CombinedSearchHistoryItem.h"
#include "EventGenerator.h"
#include <vector>

// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CNewDestContainer;
class CDataHolder;
class CNewDestDialog;
/* For RApaLsSession */
#include <apgcli.h>

namespace isab{
   class FullSearchDataReplyMess;
   class SearchRegion;
   class AdditionalInfo;
}

// CLASS DECLARATION
class GuiDataStore;

/**
 *  CNewDestView view class.
 * 
 */
class CNewDestView : public CViewBase
{
public: // Constructors and destructor

   CNewDestView(class CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * EPOC default constructor.
    */
   void ConstructL();

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CNewDestView object.
   ///        Note that this object is still on the CleanupStack.
   static class CNewDestView* NewLC(CWayFinderAppUi* aUi, isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CNewDestView object.
   static class CNewDestView* NewL(CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CNewDestView();

public: // New Functions

   /**
    * Search for a destination.
    */
   void SearchL();

   void RequestNextXHits();
   void RequestPrevXHits();

   /**
    * Route to a selected destination.
    */
   void RouteL();

   /**
    * Display the search result.
    */
   void SetSearchDataL(TBool cityReply = EFalse);

   /**
    * Call this method after the top region list has been updated.
    *
    * @param activeViewId The id of the currently active view.
    * @param aSelectedCountryId The id of the country that was 
    *                           selected before the newtop-regíon 
    *                           list was downloaded.
    */
   void ReinitCountriesL( TUint aSelectedCountryId );

   TBool HasCities();

   TBool HasSearchData();

   TBool IsPreviousSearchSet();

   TBool IsGpsConnected();
   TBool IsGpsConnectedOrSearching();
   TBool IsGpsAllowed();
   TBool IsFullVersion();
   void GpsStatus(TBool onoff);

   void SetCategorySelected( TBool aSelected );

   /**
    * Shows a menu with the available countries.
    */
   void ShowCountriesL();

   /**
    * Shows a menu with previous city hits.
    */
   void ShowCitiesL();

   /**
    * Show a menu with available search categories.
    */
   void ShowCategoriesL( TInt aCommand );

   /**
    * Shows a menu with previous search strings.
    */
   void ShowStringsL();

   /**
    * Shows a pop up list with items for a text box.
    * @param aCommand decied wich list to show.
    *
    * @param aOkChoosen Outparameter If this one is 
    *                   not NULL, the variable pointed
    *                   at is set to true if ok was 
    *                   choosen and false otherwise.
    *
    *                   The outparameter is needed 
    *                   because MAX_UINT32 is a valid
    *                   value for countries.
    */
   void ShowQueryDialog( TInt aCommand );

   void CloseQueryDialog();

   void ResetCountryToDefault(TBool forceGPS = EFalse);
   void ResetCityToDefault(TBool forceGPS = EFalse);
   /**
    * Sets the data of the previous search in the 
    * text fields.
    */
   void PreviousSearchL();

   /**
    * Request the search history.
    */
   void GetSearchHistory();
   /**
    * Receive the search history.
    */
   void ReceiveSearchHistoryL(SearchHistoryDeque_t* searchHistoryContainer);

   /**
    * Set the search parameters to the data stored in the search
    * history for the selected index.
    */
   void SelectedHistoryItem(TInt index);
   /**
    * Add a search result to my destinations.
    */
   void SaveResultL();

   /**
    * Show the selected destination on a map.
    */
   void ShowOnMap();

   /**
    * Request detailed information about a search result.
    */
   void GetDetails();

   /**
    * Get the stringname of the region type.
    * Allocates a heap descriptor!
    */
   static HBufC *GetRegionTypeName(const isab::SearchRegion* region);

   /**
    * Show the details of the selected match.
    */
   void ShowDetailsL( isab::FullSearchDataReplyMess* aMessage,
                      TInt aCommand = EWayFinderCmdNewDestCall);

   typedef std::vector<const isab::AdditionalInfo*> avect_t;
   /**
    * Show the details.
    */
   static void ShowInfoDetailsL( 
      const avect_t& infos,
      RApaLsSession iApaLsSession,
      CWayFinderAppUi* iWayFinderUI,
      CAknView* view,
      TInt aCommand = EWayFinderCmdNewDestCall,
      TInt maxLength = 256 );

   /**
    * Returns the source string stripped of everything except
    * digits and leading '+'.
    */
   char *CNewDestView::StripPhoneNumber(const char *src);

   void SetSearchStrings(const TDesC &aCountry, const TDesC &aCity, const TDesC &aHouseNum, const TDesC &aString );

   void SetAsOrigin( TInt32 aLat, TInt32 aLon );

   void SetAsDestination( TInt32 aLat, TInt32 aLon );

   TBool SettingOrigin();

   TBool SettingDestination();

   void HandleDisableCategory();

   void RemoveInputFocus();

   void ShowNewSearchHelpL();

   void FavoriteSaved();

   GuiDataStore* GetGuiDataStore();

   void TimerTick();

   TBool iSelectingCountry;

private: //New Functions

   /**
    * Tells whether the city string should be used for
    * current city or not.
    *
    * @param cityString The string to check.
    */
   TBool MatchesResourceString( TDesC &string, TInt resourceId);

   TBool IsCurrentCity(TDesC &cityString);
   TBool IsCurrentCountry(TDesC &countryString);
   
   TBool IsSelectCity(TDesC &cityString);
   TBool IsSelectCountry(TDesC &countryString);

public: /* Callback */

   enum TSearchEvent {
      ESearchEventCountryListDialogOk = 0,
      ESearchEventCountryListDialogCancel = 1,
   };
   
   void GenerateEvent(enum TSearchEvent aEvent);
   void HandleGeneratedEventL(enum TSearchEvent aEvent);

   void ShowCountryListSelectionL();
   void CountrySelectionOkL();
   void CountrySelectionCancelL();

public: // Functions from base classes

   /**
    * From AknView.
    * @return the ID of the view.
    */
   TUid Id() const;

   /**
    * From AknView, takes care of command handling.
    * @param aCommand command to be handled
    */
   void HandleCommandL(TInt aCommand);

   /**
    * From AknView, handles a change in the view.
    */
   void HandleClientRectChange();

   /**
    * From AknView, Called when the option softkey is pressed.
    * @param aResourceId the focused resource.
    * @param aMenuPane the current menu.
    */
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

private: // Functions from base classes

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoActivateL( const TVwsViewId& aPrevViewId,
                     TUid aCustomMessageId,
                     const TDesC8& aCustomMessage );

   /**
    * From AknView, Called when the view is deactivated.
    */
   void DoDeactivate();

private: // Data
   
   /// The controls container.
   CNewDestContainer* iContainer;

   /// The current search string.
   TBuf<256> iSearchString;
   TBuf<256> iCity;
   TBuf<256> iHouseNum;
   TBuf<256> iCountry;

   TInt iCommand;

   TBool iIsOrigin;

   TBool iCategorySelected;
   TInt iSelectedCategory;

   CNewDestDialog* iQueryDialog;

   isab::Log* iLog;

   TBool m_inited;
   TBool iShowSearchHistory;
   TBool iNewSearch;

   RApaLsSession iApaLsSession;

   TInt iSelectedCountryItem;
   CDesCArray* iCountryList;
   TInt iLastCountryIndex;
   
   typedef CEventGenerator<CNewDestView, enum TSearchEvent>
      CSearchViewEventGenerator;
   CSearchViewEventGenerator* iEventGenerator;
};

#endif

// End of File
