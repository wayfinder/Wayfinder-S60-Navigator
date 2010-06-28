/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CS_MAIN_VIEW_H
#define CS_MAIN_VIEW_H

#if defined NAV2_CLIENT_SERIES60_V5
# include <akntoolbarobserver.h>
#endif 
#include "WayFinderConstants.h"
#include "ViewBase.h"

#include "CombinedSearchHistoryItem.h"
#include "CountrySelectInterface.h"
#include "SearchRecord.h"


/**
 * Avkon view class for STestFormView. It is register with the view server
 * by the AppUi. It owns the container control.
 * @class	CCSMainView STestFormView.h
 */
class CCSMainView : public CViewBase, 
#if defined NAV2_CLIENT_SERIES60_V5
                    public MAknToolbarObserver,
#endif
                    public MCountrySelectInterface
{
public:
   /**
    * Symbian two-phase constructor.
    * This creates an instance then calls the second-phase constructor
    * without leaving the instance on the cleanup stack.
    * @return new instance of CCSMainView
    */
   static CCSMainView* NewL(class CWayFinderAppUi* aAppUi);

   /**
    * Symbian two-phase constructor.
    * This creates an instance, pushes it on the cleanup stack,
    * then calls the second-phase constructor.
    * @return new instance of CCSMainView
    */
   static CCSMainView* NewLC(class CWayFinderAppUi* aAppUi);

   /** 
    * The view's destructor removes the container from the control
    * stack and destroys it.
    */
   virtual ~CCSMainView();

protected:

   /**
    * First phase of Symbian two-phase construction. Should not contain any
    * code that could leave.
    */
   CCSMainView(class CWayFinderAppUi* aAppUi);

private:
   /**
    * Second-phase constructor for view.  
    * Initialize contents from resource.
    */ 
   void ConstructL();

public:
   struct TFormData {
      TBuf<256> iWhat;
      TBuf<256> iWhere;
      TBuf<256> iCountry;
      TBuf<256> iCategoryId;
      TInt iCountryId;
      TInt iUseFromGps;
      TInt iActiveIndex;
      TInt iSavedCountryId;
      
      TFormData() : iWhat(KEmpty), iWhere(KEmpty), iCountry(KEmpty), 
                    iCategoryId(KEmpty),
                    iCountryId(-1), iUseFromGps(1), iActiveIndex(0) {}

      TFormData(const TDesC& aWhat, const TDesC& aWhere,
                const TDesC& aCountry, TInt aCountryId, 
                const TDesC& aCategoryId,
                TInt aUseFromGps = 1, TInt aActiveIndex = 0) :
         iWhat(aWhat), iWhere(aWhere), 
         iCountry(aCountry), iCategoryId(aCategoryId), 
         iCountryId(aCountryId),
         iUseFromGps(aUseFromGps), iActiveIndex(aActiveIndex), iSavedCountryId(-1) {}
   };

public:
   /**
    * @return The UID for this view
    */
   TUid Id() const;

   /**
    * Handle a command for this view (override)
    * @param aCommand command id to be handled
    */
   void HandleCommandL( TInt aCommand );

   /**
    *
    */
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

   /**
    * Called when receiving an event from the toolbar.
    *
    * @param aCommandId, the id of the button that was pressed.
    */
   void OfferToolbarEventL(TInt aCommmandId);

public:

   /**
    * Updates the button visibility in the button bar.
    */
   void UpdateButtonVisibility();

   /**
    *
    */
   void SetValue(const TDesC& aValue);

   /**
    *
    */
   void SetValue(TInt aIndex);

   /**
    *
    */
   void SwitchViewL(TInt aViewId);

   /**
    *
    */
   void SwitchViewL(TInt aViewId, TInt aCustomMessageId, 
                    const TDesC8 &aCustomMessage);

   /**
    *
    */
   void UpdateGpsButton();
  const CCSMainView::TFormData& GetFormData() const;

protected:
   /**
    *	Handles user actions during activation of the view, 
    *	such as initializing the content.
    */
   void DoActivateL( const TVwsViewId& aPrevViewId,
                     TUid aCustomMessageId,
                     const TDesC8& aCustomMessage );
   /**
    *
    */
   void DoDeactivate();

private:

   void GetSearchHistory();

   void ReceiveSearchHistoryL(const SearchHistoryDeque_t& aSearchHistory);

   void SelectedHistoryItem(TInt index);

   void ResetSearchFieldsL();

   void ResetWhatFieldL();

   void ResetWhereFieldL();

   void ResetCountryFieldL();

   void SearchL();

   /**
    * Sets up the toolbar. Creates buttons and adds to
    * the toolbar if not already added before.
    */
   void InitToolbarL();

public:

   /**
    *
    */
   TBool HasSetCountry();

   /**
    *
    */
   TBool IsPreviousSearchSet();

   /**
    *
    */
   TBool IsGpsConnected();

   /**
    * Returns true if there is a usable GPS signal or
    * the current position has been found by cell id
    */
   TBool IsGpsConnectedOrCellidAvailable();

   /**
    *
    */
   TBool IsGpsAllowed();

   TBool GetButtonState();
   
   SearchRecord& GetSearchRecord();
   
   TInt GetCellCountry();
   /**
    * Lets the view know that a search request has failed.
    * Gives it a chance to prepare the controls for a new search.
    */
   void HandleFailedSearchRequest();
private: 

   class CCSMainContainer* iContainer;
   SearchRecord iSearchRecord;
   TBool iSettingCountry;
   TBool iHasSetCountry;
   TFormData iSavedData;

#if defined NAV2_CLIENT_SERIES60_V5
   class CAknButton* iGpsButton;
#endif
};

#endif // STESTFORMVIEW_H			

