/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <aknviewappui.h>
#include <eikmenub.h>
#include <avkon.hrh>
#include <akncontext.h>
#include <akntitle.h>
#include <stringloader.h>
#include <barsread.h>
#if defined NAV2_CLIENT_SERIES60_V5
# include <akntoolbar.h>
# include <aknbutton.h>
#endif
#include <arch.h>

#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "WayFinderAppUi.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"
#include "WFTextUtil.h"
#include "PopUpList.h"
#include "DataHolder.h"
#include "Dialogs.h"
#include "MC2Coordinate.h"

#include "CSMainView.h"
#include "CSMainContainer.h"
#include "CombinedSearchDispatcher.h"

CCSMainView::CCSMainView(CWayFinderAppUi* aAppUi) : 
   CViewBase(aAppUi)
{
	
}

CCSMainView::~CCSMainView()
{
   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }
   delete iContainer;
}

CCSMainView* CCSMainView::NewL(CWayFinderAppUi* aAppUi)
{
   CCSMainView* self = CCSMainView::NewLC(aAppUi);
   CleanupStack::Pop( self );
   return self;
}

CCSMainView* CCSMainView::NewLC(CWayFinderAppUi* aAppUi)
{
   CCSMainView* self = new ( ELeave ) CCSMainView(aAppUi);
   CleanupStack::PushL( self );
   self->ConstructL();
   return self;
}

void CCSMainView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_CS_MAIN_VIEW );

#if defined NAV2_CLIENT_SERIES60_V5
   if (Toolbar()) {
      Toolbar()->SetToolbarObserver( this );
   }
#endif
}
	
TUid CCSMainView::Id() const
{
   return TUid::Uid( EWayFinderCSMainView );
}

void CCSMainView::HandleCommandL( TInt aCommand )
{   
   TInt cc = 0;
   CDataHolder* dataHolder = 0;
    HBufC* titleText = 0;
   switch ( aCommand ) {
   case EAknSoftkeyBack:
      iHasSetCountry = EFalse;
      iWayfinderAppUi->HandleCommandL(aCommand);
      break;
   case EWayFinderCmdCSNewSearch:
      iHasSetCountry = EFalse;
      SearchL();
      //iContainer->SetMethod(ETrue);
      break;
   case EWayFinderCmdCSSwitchToCountrySelect:
      iSettingCountry = ETrue;
      iWayfinderAppUi->SetupCSCountryListL();
      iWayfinderAppUi->HandleCommandL(EWayFinderCmdCSSwitchToCountrySelect);
      titleText = StringLoader::LoadLC(R_TITLEPANE_COUNTRY_LIST_TEXT); // CS_CHANGEME
      iWayfinderAppUi->setTitleText(titleText->Des());
      CleanupStack::PopAndDestroy(titleText);
      break;
   case EWayFinderCmdCSSwitchToCategorySelect:
      iSettingCountry = EFalse;
      iWayfinderAppUi->SetupCSCategoryListL();
      iWayfinderAppUi->HandleCommandL(EWayFinderCmdCSSwitchToCategorySelect);
      titleText = StringLoader::LoadLC(R_TITLEPANE_SELECT_CATEGORY_TEXT);
      iWayfinderAppUi->setTitleText(titleText->Des());
      CleanupStack::PopAndDestroy(titleText);
      break;
   case EWayFinderCmdCSSetFromGps:
   	  if (iSavedData.iUseFromGps == 0 && (IsGpsConnected() || iWayfinderAppUi->GetCellCountry() >= 0))
   	  {
   	    iSavedData.iUseFromGps = 1;
      	dataHolder = iWayfinderAppUi->GetDataHolder();
      	if (dataHolder)
      	{
      		cc = iWayfinderAppUi->GetCellCountry();
      		if (cc >= 0)
      		{
      		iSavedData.iCountry.Copy(*dataHolder->GetCountryFromId(iWayfinderAppUi->GetCellCountry()));
      		iSavedData.iCountryId = iWayfinderAppUi->GetCellCountry();
      		}
      	}
		if ((iSavedData.iCountryId == -1) && (iWayfinderAppUi->GetCellCountry() != -1))
     	 {
     	 	iSavedData.iCountryId = iWayfinderAppUi->GetCellCountry();
     	 }
   	  }

      iContainer->SetUpdate(ETrue);
      iContainer->InitSearchFields();
      #if defined NAV2_CLIENT_SERIES60_V5
      if (iGpsButton) {
      if (IsGpsConnected() || iWayfinderAppUi->GetCellCountry() >= 0)
      	{
         iGpsButton->SetCurrentState(1, ETrue);
      	}
      }
	  #endif
      break;
   case EWayFinderCmdCSSearchHistory:
   #if defined NAV2_CLIENT_SERIES60_V5
      if (iGpsButton) {
      iGpsButton->SetCurrentState(0, ETrue);
      }
   #endif
      GetSearchHistory();
      break;
   default:
      iWayfinderAppUi->HandleCommandL(aCommand);
      break;
   }
}

void CCSMainView::OfferToolbarEventL(TInt aCommmandId)
{
   TInt cc = 0;
   TBuf<256> tmp;
   CDataHolder* dataHolder = 0;
   switch (aCommmandId) {
   case EWayFinderCmdButtonSearch:
      HandleCommandL(EWayFinderCmdCSNewSearch);
      break;
   case EWayFinderCmdButtonFromGps:
      if (iSavedData.iUseFromGps == 0 && (IsGpsConnected() || iWayfinderAppUi->GetCellCountry() >= 0)) 
      {
         iSavedData.iUseFromGps = 1;
     	 dataHolder = iWayfinderAppUi->GetDataHolder();
     	 if (dataHolder)
     	 {
     	 cc = iWayfinderAppUi->GetCellCountry();
     	 if (cc >= 0)
     	 	{
     	 	iSavedData.iCountry.Copy(*dataHolder->GetCountryFromId(iWayfinderAppUi->GetCellCountry()));
     	 	iSavedData.iCountryId = iWayfinderAppUi->GetCellCountry();
     	 	}
     	 }
     	 if ((iSavedData.iCountryId == -1) && (iWayfinderAppUi->GetCellCountry() != -1))
     	 {
     	 	iSavedData.iCountryId = iWayfinderAppUi->GetCellCountry();
     	 }
      }
      else 
      {
         iSavedData.iUseFromGps = 0;
      }
      iContainer->SetUpdate(ETrue);
      iContainer->InitSearchFields();
      break;
   case EWayFinderCmdButtonCategorySelect:
      HandleCommandL(EWayFinderCmdCSSwitchToCategorySelect);
      break;
   }
}

void CCSMainView::DoActivateL( const TVwsViewId& aPrevViewId,
                               TUid aCustomMessageId,
                               const TDesC8& aCustomMessage )
{
  CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
  
  // Explicitly hide the arrows in navipane since on s60v5 they tend to 
  // show up in some views otherwise.
  iWayfinderAppUi->setNaviPane(EFalse);
  
  HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_SEARCH_TEXT);
  iWayfinderAppUi->setTitleText(titleText->Des());
  CleanupStack::PopAndDestroy(titleText);
  iSavedData.iUseFromGps = (iSavedData.iUseFromGps && IsGpsConnectedOrCellidAvailable() );
  iSavedData.iCountryId = iSavedData.iSavedCountryId;
  InitToolbarL();
  
  CEikButtonGroupContainer* cba = AppUi()->Cba();
  if ( cba != NULL ) {
    cba->MakeVisible( EFalse );
  }
  
  if ( iContainer == NULL ) {
    if (iSavedData.iCountry.Length() == 0)
      {
        TInt countryID = iWayfinderAppUi->GetCellCountry();
		if (countryID >= 0) {
          CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
          iSavedData.iCountryId = countryID;
          iSavedData.iSavedCountryId = countryID;
          iSavedData.iCountry.Copy(*dataHolder->GetCountryFromId(countryID));
        }
        if (!(countryID >= 0 || IsGpsConnected()))
        {
        	iSavedData.iUseFromGps = 0;
        }
      }
  
    CCSMainContainer* form = CCSMainContainer::NewL( this, *this, iSavedData );
    form->SetUpdate(ETrue);
    form->SetMopParent( this ); 
    form->ExecuteLD( R_WAYFINDER_CS_SEARCH_FORM );
    AppUi()->AddToStackL( *this, form );
    iContainer = form;
  }
}

void CCSMainView::InitToolbarL()
{
#if defined NAV2_CLIENT_SERIES60_V5
   if (Toolbar()) {
      CAknButton* button = static_cast<CAknButton*>
         (Toolbar()->ControlOrNull(EWayFinderCmdButtonFromGps));
      if (!button) {
         HBufC* helpText = CEikonEnv::Static()->AllocReadResourceLC(R_WF_TOOLTIP_SEARCHAROUND);
         button = // icon, dimmed, pressed, hower
            CAknButton::NewL(iWayfinderAppUi->iPathManager->GetMbmName(),
                             EMbmWficonsGps_info, EMbmWficonsGps_info_mask,
                             EMbmWficonsGps_info_dimmed, EMbmWficonsGps_info_dimmed_mask,
                             EMbmWficonsGps_info, EMbmWficonsGps_info_mask,
                             EMbmWficonsGps_info, EMbmWficonsGps_info_mask,
                             KNullDesC, *helpText, 0, 0);
         button->SetIconScaleMode(EAspectRatioPreservedAndUnusedSpaceRemoved);
         button->AddStateL(iWayfinderAppUi->iPathManager->GetMbmName(),
                           EMbmWficonsGps_info, EMbmWficonsGps_info_mask,
                           EMbmWficonsGps_info_dimmed, EMbmWficonsGps_info_dimmed_mask,
                           EMbmWficonsGps_info, EMbmWficonsGps_info_mask,
                           EMbmWficonsGps_info, EMbmWficonsGps_info_mask,
                           KNullDesC, *helpText, KAknButtonStateHasLatchedFrame,
                           KAknsIIDNone, KAknsIIDNone, KAknsIIDNone, KAknsIIDNone);
         Toolbar()->AddItemL(button, EAknCtButton, EWayFinderCmdButtonFromGps, 0, 0);
         CleanupStack::PopAndDestroy(helpText);
      }
      iGpsButton = button;
      if (iSavedData.iUseFromGps ) {
         button->SetCurrentState(1, ETrue);
      } else {
         button->SetCurrentState(0, ETrue);
      }
      if ( IsGpsConnectedOrCellidAvailable() ) {
         button->SetDimmed(EFalse);
      } else {
         button->SetDimmed(ETrue);
      }
      button->DrawDeferred();

      button = static_cast<CAknButton*>
         (Toolbar()->ControlOrNull(EWayFinderCmdButtonSearch));
      if (!button) {
         HBufC* helpText = CEikonEnv::Static()->AllocReadResourceLC(R_WF_TOOLTIP_MAKESEARCH);
         button = 
            CAknButton::NewL(iWayfinderAppUi->iPathManager->GetMbmName(),
                             EMbmWficonsSearch, EMbmWficonsSearch_mask,
                             EMbmWficonsSearch, EMbmWficonsSearch_mask,
                             EMbmWficonsSearch, EMbmWficonsSearch_mask,
                             EMbmWficonsSearch, EMbmWficonsSearch_mask,
                             KNullDesC, *helpText, 0, 0);
         button->SetIconScaleMode(EAspectRatioPreservedAndUnusedSpaceRemoved);
         Toolbar()->AddItemL(button, EAknCtButton, EWayFinderCmdButtonSearch, 0, 1);
         CleanupStack::PopAndDestroy(helpText);
      }

      button = static_cast<CAknButton*>
         (Toolbar()->ControlOrNull(EWayFinderCmdButtonCategorySelect));
      if (!button) {
         HBufC* helpText = CEikonEnv::Static()->AllocReadResourceLC(R_WF_TOOLTIP_SELECTCAT);
         button = 
            CAknButton::NewL(iWayfinderAppUi->iPathManager->GetMbmName(),
                             EMbmWficonsCategory_dash, EMbmWficonsCategory_dash_mask,
                             EMbmWficonsCategory_dash, EMbmWficonsCategory_dash_mask,
                             EMbmWficonsCategory_dash, EMbmWficonsCategory_dash_mask,
                             EMbmWficonsCategory_dash, EMbmWficonsCategory_dash_mask,
                             KNullDesC, *helpText, 0, 0);
         button->SetIconScaleMode(EAspectRatioPreservedAndUnusedSpaceRemoved);
         Toolbar()->AddItemL(button, EAknCtButton, EWayFinderCmdButtonCategorySelect, 0, 2);
         CleanupStack::PopAndDestroy(helpText);
      }

   }
#endif
}

void CCSMainView::DoDeactivate()
{
   CEikButtonGroupContainer* cba = AppUi()->Cba();
   if ( cba != NULL ) {
      cba->MakeVisible( ETrue );
      cba->DrawDeferred();
   }
	
   if ( iContainer != NULL ) {
      AppUi()->RemoveFromStack( iContainer );
      delete iContainer;
      iContainer = NULL;
   }
}

void CCSMainView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if ( aResourceId == R_WAYFINDER_NEWDEST_MENU ) {
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, ETrue );
   } else if (aResourceId == R_WAYFINDER_CS_FORM_MENU) {
#if defined NAV2_CLIENT_SERIES60_V5
      // If touch screen hide the search option since we have a search button
      // in the visible toolbar.
      aMenuPane->SetItemDimmed(EWayFinderCmdCSNewSearch, AknLayoutUtils::PenEnabled() );
#endif
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

void CCSMainView::UpdateButtonVisibility()
{
#if defined NAV2_CLIENT_SERIES60_V5
   if (iGpsButton) {
      if (iGpsButton->IsDimmed() && IsGpsConnectedOrCellidAvailable()) {
         iGpsButton->SetDimmed(EFalse);
         iGpsButton->DrawDeferred();
      } else if (!iGpsButton->IsDimmed() && !IsGpsConnectedOrCellidAvailable()) {
         iGpsButton->SetDimmed(ETrue);
         iGpsButton->DrawDeferred();
      }
   }
#endif
}

void CCSMainView::SetValue(const TDesC& aValue) 
{
   // XXX: Do not use this function since there are no 
   //      way to get the correct country id from the
   //      DataHolder using a string.

   if (iSettingCountry) {
      iSavedData.iUseFromGps = 0;
      iHasSetCountry = ETrue;
      if(!iContainer && aValue.Length() > 0) {
         iSavedData.iCountry.Copy(aValue);
      } else {
         iContainer->SetCountry(aValue);
         iContainer->InitSearchFields();
      }
   } else {
      if(!iContainer && aValue.Length() > 0) {
         iSavedData.iWhat.Copy(aValue);
      } else {
         iContainer->SetWhat(aValue);
      }
   }
}

void CCSMainView::SetValue(TInt aIndex) 
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   if (iSettingCountry) {
      // Setting country from SearchableListboxView
      iSavedData.iCountryId = dataHolder->GetCountryId(aIndex);
      if (iSavedData.iCountryId >= 0)
      {
      	iSavedData.iSavedCountryId = iSavedData.iCountryId;
      }
      iSavedData.iUseFromGps = 0;
      iHasSetCountry = ETrue;
      if(!iContainer && aIndex > -1) {
         iSavedData.iCountry.Copy(*dataHolder->GetCountry(aIndex));
      } else if (aIndex > -1) {
         iContainer->SetCountry(*dataHolder->GetCountry(aIndex));
         iContainer->InitSearchFields();
      }
   } else {
      // Setting category from SearchableListboxView
      if(!iContainer && aIndex > -1) {
         iSavedData.iWhat.Copy(*dataHolder->GetCategory(aIndex));
      } else if (aIndex > -1) {
         iContainer->SetWhat(*dataHolder->GetCategory(aIndex));
      }
      const char* categoryId = dataHolder->GetCategoryId(aIndex);
      if (categoryId) {
         WFTextUtil::char2TDes(iSavedData.iCategoryId, categoryId);
      }
   }
}

void CCSMainView::SwitchViewL(TInt aViewId)
{
   iSettingCountry = ETrue;
   HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_COUNTRY_LIST_TEXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   iWayfinderAppUi->SetupCSCountryListL();
   TUid uid = TUid::Uid(aViewId);
   iWayfinderAppUi->push(uid);
}

void CCSMainView::SwitchViewL(TInt aViewId, TInt aCustomMessageId, 
                              const TDesC8& aCustomMessage)
{
   iSettingCountry = ETrue;
   iWayfinderAppUi->SetupCSCountryListL();
   TUid uid = TUid::Uid(aViewId);
   iWayfinderAppUi->push(uid, aCustomMessageId, aCustomMessage);
}

TBool CCSMainView::HasSetCountry()
{
   return iHasSetCountry;
}



TBool CCSMainView::IsPreviousSearchSet()
{
   return (iWayfinderAppUi->GetCSDispatcher()->
           getCombinedSearchDataHolder().getNumSearchHistoryItems() > 0);
}

TBool CCSMainView::IsGpsConnected()
{
   return iWayfinderAppUi->ValidGpsStrength();
}

TBool CCSMainView::IsGpsConnectedOrCellidAvailable()
{
   return ( iWayfinderAppUi->ValidGpsStrength() || iWayfinderAppUi->IsCellPosAvailable() );
}

TBool CCSMainView::IsGpsAllowed()
{
   return iWayfinderAppUi->IsGpsAllowed();
}

TBool CCSMainView::GetButtonState()
{
#if defined NAV2_CLIENT_SERIES60_V5
if (iGpsButton)
	{
		if (iGpsButton->IsDimmed())
		{
		return ETrue;
		}
   }
#endif
   return EFalse;
}

void CCSMainView::UpdateGpsButton()
{
#if defined NAV2_CLIENT_SERIES60_V5
   if (iGpsButton && !iGpsButton->IsDimmed()) {
      if (iSavedData.iUseFromGps) {
         iGpsButton->SetCurrentState(1, ETrue);
      } else {
         iGpsButton->SetCurrentState(0, ETrue);
      }
   }
#endif
}

void CCSMainView::GetSearchHistory()
{
   /* Request the search history from Nav2. */
   /* Actually we will fake it for the moment. :-) */
   /* The results will arrive as a callback to SearchHistory(). */
   ReceiveSearchHistoryL(iWayfinderAppUi->GetCSDispatcher()->
                         getCombinedSearchDataHolder().getSearchHistory());
}

void CCSMainView::ReceiveSearchHistoryL(const SearchHistoryDeque_t& aSearchHistory)
{
   /* We've gotten the search history. */
   /* Initialize a list for the user to select from. */
   CDesCArrayFlat* histList = new (ELeave) CDesCArrayFlat(20);
   CleanupStack::PushL( histList );

   SearchHistoryDeque_t::const_iterator it;
   it = aSearchHistory.begin();
   while (it != aSearchHistory.end()) {
      TBuf<256> buf;
      char *tmp = (*it)->GetHistoryListEntry();
      WFTextUtil::char2TDes(buf, tmp);
      histList->AppendL(buf);
      ++it;
   }
   TBool aOkChosen;
   TInt index = PopUpList::ShowPopupListL( R_WAYFINDER_SOFTKEYS_OK_CANCEL,
                                           *histList, aOkChosen );
   SelectedHistoryItem(index);
   CleanupStack::PopAndDestroy(histList);   
}

void CCSMainView::SelectedHistoryItem(TInt index)
{
   SearchHistoryItem *shi = iWayfinderAppUi->GetCSDispatcher()->
      getCombinedSearchDataHolder().getSearchHistoryItem(index);

   if (!shi) {
      // Failed, Don't do anything.
      return;
   }

   iSavedData.iUseFromGps = 0;

   TBuf<256> tmp;

   // Search string (what).
   if (strlen(shi->GetSearchString()) == 0) {
      //No what string
      ResetWhatFieldL();
   } else {
      WFTextUtil::char2TDes(tmp, shi->GetSearchString());
      if (iContainer) {
         iContainer->SetWhat(tmp);
      } else {
         iSavedData.iWhat.Copy(tmp);
      }
   }
   // Category Id (added to search history for combined search).
   if (strlen(shi->GetCategoryId()) == 0) {
      // No categoryid string
      iSavedData.iCategoryId.Copy(KEmpty);
   } else {
      WFTextUtil::char2TDes(tmp, shi->GetCategoryId());
      iSavedData.iCategoryId.Copy(tmp);
   }
   // City (where).
   if (strlen(shi->GetCityString()) == 0) {
      // No city string
      ResetWhereFieldL();
   } else {
      WFTextUtil::char2TDes(tmp, shi->GetCityString());
      if (iContainer) {
         iContainer->SetWhere(tmp);
      } else {
         iSavedData.iWhere.Copy(tmp);
      }
   }
   // Country.
   if (strlen(shi->GetCountryString()) == 0) {
      // No country string
      ResetCountryFieldL();
   } else {
      WFTextUtil::char2TDes(tmp, shi->GetCountryString());
      if (iContainer) {
         iContainer->SetCountry(tmp);
      } else {
         iSavedData.iCountry.Copy(tmp);
      }
      iSavedData.iCountryId = shi->GetIntCountryId();
   }
}

const CCSMainView::TFormData& CCSMainView::GetFormData() const
{
  return iSavedData;
}
void CCSMainView::ResetSearchFieldsL()
{
   ResetWhatFieldL();
   ResetWhereFieldL();
   ResetCountryFieldL();
}

void CCSMainView::ResetWhatFieldL()
{
   if (iContainer) {
      iContainer->SetWhat(KEmpty);
   } else {
      iSavedData.iWhat.Copy(KEmpty);
   }
   iSavedData.iCategoryId.Copy(KEmpty);
}

void CCSMainView::ResetWhereFieldL()
{
   if (iContainer) {
      iContainer->SetWhere(KEmpty);
   } else {
      iSavedData.iWhere.Copy(KEmpty);
   }
}

void CCSMainView::ResetCountryFieldL()
{
   if (iContainer) {
      iContainer->SetCountry(KEmpty);
   } else {
      iSavedData.iCountry.Copy(KEmpty);
   }
}

void CCSMainView::SearchL()
{
   iContainer->SaveDataL();
   iContainer->DisableEditorIndicators();

   // if GPS or Cell Info data is available
   if ( iSavedData.iUseFromGps && IsGpsConnectedOrCellidAvailable() && /*iWayfinderAppUi->GetCellCountry() == */iSavedData.iCountryId == -1){
      // user has selected to search from GPS
      TInt32 lat, lon;
      TUint cellRadius = 0;
      TInt heading, speed, alt;
      iWayfinderAppUi->GetDataHolder()->GetInputData(lat,lon,
                                                     heading, speed, alt);
                                                     
      // if there is no lat and long from GPS check for cell pos values
      if (lat == MAX_INT32 || lon == MAX_INT32) {
         Nav2Coordinate navCoOrd(MAX_INT32,MAX_INT32);
         if ( iWayfinderAppUi->GetCellPosIfAvailable(navCoOrd, cellRadius) ) {
         	lat = navCoOrd.nav2lat;
         	lon = navCoOrd.nav2lon;
         }
      }
      
      if (lat == MAX_INT32 || lon == MAX_INT32) {
         // user wants to search from gps but the gps haven't reported any 
         // valid coords yet
         iWayfinderAppUi->ShowErrorDialogL(0x0200, R_WAYFINDER_NO_GPS_POS_MSG);
      } else {
         // got valid coords from the gps so perfrom the search based on this
         if (iSavedData.iWhat.Length() > 0) {
            char* searchString = WFTextUtil::newTDesDupL(iSavedData.iWhat);
            char* categoryId = WFTextUtil::newTDesDupL(iSavedData.iCategoryId);
            SearchRecord sr(MAX_UINT32, "", "", searchString, 
                            categoryId, lat, lon);
            iSearchRecord = sr;
            delete[] categoryId;
            delete[] searchString;

            iWayfinderAppUi->HandleCommandL(EWayFinderCmdCSNewSearch);
         } else {
            // The user still needs to specify what to search for.
            WFDialog::ShowInformationL( R_WAYFINDER_NOHITS_MSG );
         }
      }
   } else {
      // user has entered searchfields to search from (not from GPS)
      if (iSavedData.iCountry.Length() > 0 &&
          (iSavedData.iWhat.Length() > 0 || iSavedData.iWhere.Length() > 0)) {
         TInt countryId = iSavedData.iCountryId;
         char* areaName = WFTextUtil::newTDesDupL(iSavedData.iWhere);
         char* searchString = WFTextUtil::newTDesDupL(iSavedData.iWhat);
         char* categoryId = WFTextUtil::newTDesDupL(iSavedData.iCategoryId);
         char* countryName = WFTextUtil::newTDesDupL(iSavedData.iCountry);
         char* charCountryId = WFTextUtil::uint32AsStringL(iSavedData.iCountryId);
         iWayfinderAppUi->GetCSDispatcher()->
            getCombinedSearchDataHolder().addSearchHistoryItem(searchString, 
                                                               areaName, 
                                                               countryName, 
                                                               charCountryId, 
                                                               categoryId);

         SearchRecord sr(countryId, "", areaName, searchString, categoryId);
         iSearchRecord = sr;
         delete[] areaName;
         delete[] searchString;
         delete[] categoryId;
         delete[] countryName;
         delete[] charCountryId;

         iWayfinderAppUi->HandleSaveSearchHistoryL();
         iWayfinderAppUi->HandleCommandL(EWayFinderCmdCSNewSearch);
      } else {
         // The user has not given enough information to search for.
         // Display a dialog about this.
         WFDialog::ShowInformationL( R_WAYFINDER_NOHITS_MSG );
         iContainer->EnableEditorIndicators();
      }
   }
}

SearchRecord& CCSMainView::GetSearchRecord()
{
   return iSearchRecord;
}

TInt CCSMainView::GetCellCountry()
{
	return iWayfinderAppUi->GetCellCountry();	
}

void CCSMainView::HandleFailedSearchRequest()
{
   if(iContainer) {
      iContainer->EnableEditorIndicators();
   }
}

