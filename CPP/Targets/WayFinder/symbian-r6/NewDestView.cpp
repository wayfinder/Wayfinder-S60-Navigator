/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES
#include <eikmenup.h>
#include <avkon.hrh>
#include <aknselectionlist.h>
#include <aknnotedialog.h>
#include <eikedwin.h>

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include <arch.h>
#include "RouteEnums.h"
#include "MapEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtRouteMess.h"
#include "WayFinderAppUi.h"
#include "nav2util.h"

#include "RsgInclude.h"
#include "wayfinder.hrh"

#include "NewDestView.h"
#include "StartPageView.h"
#include "NewDestContainer.h"
#include "PositionSelectView.h"
#include "MapView.h"
#include "WFTextUtil.h"
#include "WFS60Util.h"

#include "MC2Coordinate.h"

#include "BufferArray.h"
#include "DataHolder.h"

#include "GuiProt/SearchRegion.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/FullSearchItem.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtMapMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/AdditionalInfo.h"

#include "NewDestDialog.h"

#include "Dialogs.h"
#include "PopUpList.h"

#include "debuggingdefines.h"
#define MAX_NUM_SEARCH_HITS      25

#include "SettingsData.h"
#include "CallBackDialog.h"
#include "ExitQueryDlg.h"

using namespace isab;

// ================= MEMBER FUNCTIONS =======================

CNewDestView::CNewDestView(CWayFinderAppUi* aUi, isab::Log* aLog) : 
   CViewBase(aUi),
   iLog(aLog) 
{}
// ---------------------------------------------------------
// CNewDestView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CNewDestView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_NEWDEST_VIEW );
   iSearchString.Zero();
   iHouseNum.Zero();
   WFS60Util::TDesReadResourceL(iCity, R_WAYFINDER_FROM_GPS_TEXT, iCoeEnv);
   WFS60Util::TDesReadResourceL(iCountry, R_WAYFINDER_FROM_GPS_TEXT, iCoeEnv);
   iIsOrigin = EFalse;
   iCategorySelected = EFalse;
   iQueryDialog = NULL;
   iCommand = ENoMessage;
   iNewSearch = ETrue;
   m_inited = EFalse;
   iShowSearchHistory = EFalse;
   iEventGenerator = CSearchViewEventGenerator::NewL(*this);
}

CNewDestView* CNewDestView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CNewDestView* self = new (ELeave) CNewDestView(aUi, aLog);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CNewDestView* CNewDestView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CNewDestView *self = CNewDestView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CNewDestView::~CNewDestView()
// ?implementation_description
// ---------------------------------------------------------
//
CNewDestView::~CNewDestView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }
   LOGDEL(iContainer);

   iApaLsSession.Close();
   delete iContainer;
}

void
CNewDestView::ResetCountryToDefault(TBool forceGPS)
{
   if (forceGPS) {
      /* Initiate to "From GPS". */
      WFS60Util::TDesReadResourceL(iCountry, R_WAYFINDER_FROM_GPS_TEXT,iCoeEnv);
   } else {
      // experimental
      bool updated = false;
      if (!updated) {
         iCoeEnv->ReadResource( iCountry, R_WAYFINDER_SELECT_COUNTRY_TEXT);
      }
   }
   if( iQueryDialog ) {
      iQueryDialog->SetCountry( iCountry );
   }
}

void
CNewDestView::ResetCityToDefault(TBool forceGPS)
{
   if (forceGPS) {
      /* Initiate to "From GPS". */
      WFS60Util::TDesReadResourceL(iCity, R_WAYFINDER_FROM_GPS_TEXT, iCoeEnv);
   } else {
      iCity.Copy( _L("") );
/*       WFS60Util::TDesReadResourceL(iCity, R_WAYFINDER_SELECT_CITY_TEXT,iCoeEnv); */
   }
   if (iQueryDialog) {
      iQueryDialog->SetCity(iCity);
   }
}

// ---------------------------------------------------------
// TUid CNewDestView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
void CNewDestView::SearchL()
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();

   TBool countryOK = EFalse;
   TBool cityOK = EFalse;
   if( iCountry.Length() > 0 && !IsSelectCountry( iCountry )) {
      countryOK = ETrue;
   }
/*    if( iCity.Length() > 0 && !IsSelectCity( iCity )) { */
      cityOK = ETrue;
/*    } */
   
   if (cityOK && countryOK) {
      // Defaults use current country and city.
      TUint selCountry = MAX_UINT32;
      TUint selCity = MAX_UINT32;
      char* area = "";
      char* area_id = "";
      TBool currentCity = ETrue;

      if( !IsCurrentCountry( iCountry ) ){
         selCountry = dataHolder->GetSelectedCountryId();
         if (selCountry == MAX_UINT32) {
            /* Country selection problem. */
            ResetCountryToDefault(IsGpsAllowed());
         }
      }

      if( !IsCurrentCity( iCity ) ) {
         currentCity = EFalse;
         selCity = dataHolder->GetSelectedNewCity();
         // Search using city string.
         TInt pos = iCity.Find(_L(","));
         if (pos != KErrNotFound) {
            /* Found a comma. */
            area = WFTextUtil::newTDesDupL( iCity.Left(pos));
         } else {
            /* No comma, use the string as it is. */
            area = WFTextUtil::newTDesDupL( iCity);
         }
         LOGNEWA(area, char, strlen(area) + 1);
         if( selCity > TUint( dataHolder->GetNbrNewCities() ) ||
             !dataHolder->IsDuplicateNewCity( iCity ) ){
            // New search, not a selected string from the list
            // of cities.
            selCity = MAX_UINT32;
            area_id = "";
         } else {
            // Get the id string for the selected city
            // from the dataholder.
            HBufC* tmp = dataHolder->GetNewCityId( selCity );
            area_id = WFTextUtil::newTDesDupL(*tmp);
         }
      }
      char *search_item = NULL;
      // Check if the user selected a category, and use the category
      // id instead as the search string.
      if( iCategorySelected && (dataHolder->GetCategory(iSelectedCategory))->Compare(iSearchString) == 0 ){
         // Yup, the category is unchanged.
         const char *cat_id = dataHolder->GetCategoryId(iSelectedCategory);
         search_item = new (ELeave) char[strlen(cat_id)+2];
         LOGNEWA(search_item, char, strlen(cat_id) + 2);
         search_item[0] = 0x18;
         strcpy(&search_item[1], cat_id);
         /* Ignore house number. */
      } else {
         /* Normal search. Save the data for later use. */
         HBufC* tmp = HBufC::NewLC(iSearchString.Length()+iHouseNum.Length()+1);
         tmp->Des().Copy(iHouseNum);
         if (iHouseNum.Length()>0) {
            tmp->Des().Append(_L(" "));
         }
         tmp->Des().Append(iSearchString);
         search_item = WFTextUtil::newTDesDupL(*tmp);
         LOGNEWA(search_item, char, strlen(search_item) + 1);
         CleanupStack::PopAndDestroy(tmp);

         char *ss = WFTextUtil::newTDesDupL(iSearchString);
         char *housenumber = WFTextUtil::newTDesDupL(iHouseNum);
         char *cis;
         if ( IsCurrentCity(iCity)) {
            cis = strdup_new("");
         } else {
            cis = WFTextUtil::newTDesDupL(iCity);
         }
/*          char *cii = strdup_new(area_id); */
         char *cii = "";
         char *cos;
         if ( IsCurrentCountry(iCountry)) {
            cos = strdup_new("");
         } else {
            cos = WFTextUtil::newTDesDupL(iCountry);
         }
         char *coi = WFTextUtil::uint32AsStringL(selCountry);
         dataHolder->AddSearchHistoryItemL(ss, housenumber, cis, cii, cos, coi);
         delete[] ss;
         delete[] housenumber;
         delete[] cis;
/*          delete[] cii; */
         delete[] cos;
         delete[] coi;
         iWayfinderAppUi->HandleSaveSearchHistoryL( );
      }

      TInt32 lat;
      TInt32 lon = MAX_INT32;

      if (IsCurrentCountry(iCountry) || IsCurrentCity(iCity)) {
         /* Coordinate is needed. */
         lat = MAX_INT32;
      } else {
         /* Latitude is set to MAX_INT32 - 1 to avoid */
         /* coordinate refresh in NavServerCom. */
         /* We don't want the coordinate to be sent to server. */
         /* Results ordered from coordinate. */
         lat = MAX_INT32 - 1;
      }
      /* If we do have a category search, then we should use the */
      /* coordinate from the current origin (if available). */
      /* Send search message to Nav2 */

      SearchMess *message = new (ELeave) SearchMess( selCountry,
                                                     area_id,
                                                     area,
                                                     search_item,
                                                     lat, lon );
      LOGNEW(message, SearchMess);
      iWayfinderAppUi->SendMessageL( message );
      if (iQueryDialog) {
         iQueryDialog->RemoveInputFocus();
      }
      message->deleteMembers();
      LOGDEL(message);
      delete message;
      if (selCity != MAX_UINT32) {
         LOGDELA(area);
         delete[] area;
         LOGDELA(area_id);
         delete[] area_id;
      } else if (!currentCity) {
         LOGDELA(area);
         delete[] area;
      }
      LOGDELA(search_item);
      delete[] search_item;
   } else {
      if( !countryOK ){
         /* Country not selected correctly. */
         WFDialog::ShowInformationL( R_WAYFINDER_SEARCHINCOMPLETE_COUNTRY_MSG );
      } else {
         /* City not selected (input) correctly. (Select city) */
         WFDialog::ShowInformationL( R_WAYFINDER_SEARCHINCOMPLETE_CITY_MSG );
      }
   }
}

void
CNewDestView::RequestNextXHits()
{
   if (iContainer && iContainer->HasResults()) {
      CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
      TInt startIdx = dataHolder->GetSearchStartIndex();

      if ((startIdx+MAX_NUM_SEARCH_HITS)
            < dataHolder->GetSearchTotalHits()) {
         startIdx += MAX_NUM_SEARCH_HITS;
         GenericGuiMess getItems(GuiProtEnums::GET_SEARCH_ITEMS,
               uint16(startIdx));
         iWayfinderAppUi->SendMessageL(&getItems);
      } else {
         /* No more hits. */
      }
   }
}

void
CNewDestView::RequestPrevXHits()
{
   if (iContainer && iContainer->HasResults()) {
      CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
      TInt startIdx = dataHolder->GetSearchStartIndex();
      if (startIdx == 0 ) {
         /* Can't show negative hits. */
      } else {
         startIdx -= MAX_NUM_SEARCH_HITS;
         if (startIdx < 0) {
            startIdx = 0;
         }
         GenericGuiMess getItems(GuiProtEnums::GET_SEARCH_ITEMS,
               uint16(startIdx));
         iWayfinderAppUi->SendMessageL(&getItems);
      }
   }
}

void CNewDestView::RouteL()
{
   if( iContainer && iContainer->HasResults() ){
      CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
      HBufC* hId = dataHolder->GetNewDestId( iContainer->GetSelIndex());
      char *id = WFTextUtil::newTDesDupL(*hId);
      LOGNEWA(id, char, strlen(id) + 1);
      RouteMess* message = new (ELeave) RouteMess(
            GuiProtEnums::PositionTypeSearch,
            id,
            MAX_INT32, MAX_INT32,
            "");
      LOGNEW(message, RouteMess);
      iWayfinderAppUi->SendMessageL( message );
      message->deleteMembers();
      LOGDEL(message);
      delete message;
      LOGDELA(id);
      delete[] id;
      TBuf<KBuf256Length> name;
      name.Copy( dataHolder->GetNewDest(iContainer->GetSelIndex())->Des() );
      iWayfinderAppUi->SetCurrentRouteEndPoints( name,
                                              MAX_INT32, MAX_INT32,
                                              MAX_INT32, MAX_INT32 );
   } else{
      WFDialog::ShowInformationL( R_WAYFINDER_NODESTINATION_MSG, iCoeEnv );
   }
}

void CNewDestView::SetSearchDataL(TBool cityReply)
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   if( iContainer ) {
      iContainer->RemoveAllItemsL();
   }
   if (cityReply) {
      iNewSearch = EFalse;
   }
   if( dataHolder->GetNbrNewDests() > 0 ){
      if( iContainer ){
         for( TInt i=0; i < dataHolder->GetNbrNewDests(); i++ ){
            iContainer->AddItemL( i, *(dataHolder->GetNewDest(i)) );
         }
      }
      CloseQueryDialog();
   } else if( dataHolder->GetNbrNewCities() > 1 && !iNewSearch){
      ShowCitiesL();
   } else {
   }
}

TBool CNewDestView::HasCities()
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();

   return dataHolder->GetNbrNewCities() > 0;
}

TBool CNewDestView::HasSearchData()
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   return dataHolder->GetNbrNewDests() > 0;
}

TBool
CNewDestView::IsPreviousSearchSet()
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   return (dataHolder->GetNumSearchHistoryItems() > 0);
}

TBool
CNewDestView::IsGpsConnected()
{
   return iWayfinderAppUi->IsGpsConnected();
}
TBool
CNewDestView::IsGpsConnectedOrSearching()
{
   return iWayfinderAppUi->IsGpsConnectedOrSearching();
}
TBool
CNewDestView::IsGpsAllowed()
{
   return iWayfinderAppUi->IsGpsAllowed();
}

void
CNewDestView::GpsStatus(TBool onoff)
{
   if (onoff) {
      /* GPS connected. */
      /* Change city and country string to "From GPS". */
      ResetCityToDefault(ETrue);
      ResetCountryToDefault(ETrue);
   } else {
      /* GPS disconnected. */
      /* Change city and country strings to "Select city" if */
      /* it is "From GPS". */
      if ( IsCurrentCity(iCity)) {
         ResetCityToDefault(EFalse);
      }
      if ( IsCurrentCountry(iCountry)) {
         ResetCountryToDefault(EFalse);
      }

   }
   if (iContainer) {
      iContainer->DrawNow();
   }
}

void CNewDestView::SetCategorySelected( TBool aSelected )
{
   iCategorySelected = aSelected;
}

_LIT(KListFormat, "\t%S\t\t");

void CNewDestView::ShowCountriesL()
{
   if (iCountryList) {
      /* Already selecting. */
      return;
   }
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   int preallocatedListPositions = MAX(5, dataHolder->GetNbrCountries() + 1);
   iCountryList = new (ELeave) CDesCArrayFlat(preallocatedListPositions);

   TInt countryId = -1;
   if( IsGpsAllowed()){
      HBufC* buf = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_FROM_GPS_TEXT);
      HBufC* tmp = HBufC::NewLC(buf->Length()+10);
      TPtr bar = buf->Des();
      tmp->Des().Format(KListFormat, &bar);
      iCountryList->AppendL( *tmp );
      CleanupStack::PopAndDestroy(tmp);
      CleanupStack::PopAndDestroy(buf);
   }
   int preselection = -1;
   if( dataHolder->GetNbrCountries() > 0 ){
      for( TInt i=0; i < dataHolder->GetNbrCountries(); i++ ){
         if (int(dataHolder->GetCountryId(i)) == countryId) {
            preselection = iCountryList->Count();
         }
         HBufC* foo = dataHolder->GetCountry(i);
         HBufC* tmp = HBufC::NewLC(foo->Length()+10);
         TPtr bar = foo->Des();
         tmp->Des().Format(KListFormat, &bar);
         iCountryList->AppendL( *tmp );
         CleanupStack::PopAndDestroy(tmp);
      }
   }

   if (preselection == -1) {
      // on failure to get cell data country, get last used country
      preselection = dataHolder->GetSelectedCountry(); // none.
   }

   if (preselection < 0) {
      preselection = 0;
   } else if (preselection >= iCountryList->Count()) {
      preselection = iCountryList->Count()-1;
   }

   iLastCountryIndex = preselection;

   ShowCountryListSelectionL();
}



void CNewDestView::ShowCitiesL()
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   CDesCArrayFlat* descArray = new (ELeave) CDesCArrayFlat(5);
   LOGNEW(descArray, CDesCArrayFlat);
   CleanupStack::PushL( descArray );
   if(IsGpsAllowed() ){
      TBuf<KBuf32Length> buf;
      WFS60Util::TDesReadResourceL( buf, R_WAYFINDER_FROM_GPS_TEXT, iCoeEnv);
      descArray->AppendL( buf );
   }
   if( dataHolder->GetNbrNewCities() > 0 ){
      for( TInt i=0; i < dataHolder->GetNbrNewCities(); i++ ){
         descArray->AppendL( *dataHolder->GetNewCity(i) );
      }
   }
   TBool okChoosen;
   TInt cityIdx = PopUpList::ShowPopupListL(R_WAYFINDER_SOFTKEYS_OK_CANCEL,
                                            *descArray, okChoosen );
   if( okChoosen ){
      iCity.Copy( (*descArray)[cityIdx] );
      if ( iQueryDialog ) {
         iQueryDialog->SetCity( iCity );
      }
      if( IsGpsAllowed() )
         cityIdx--;
      dataHolder->SetSelectedNewCity( cityIdx );
/*       if( iSearchString.Length() > 0 ){ */
         /* Got something in the search string, and the user didn't cancel. */
         /* Do the search. */
         SearchL();
/*       } */
   } else {
      iNewSearch = ETrue;
   }
   CleanupStack::PopAndDestroy(descArray);
}

void CNewDestView::ShowCategoriesL( TInt /*aCommand*/ )
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   CDesCArrayFlat* descArray = new (ELeave) CDesCArrayFlat(5);
   LOGNEW(descArray, CDesCArrayFlat);
   CleanupStack::PushL( descArray );
   if( dataHolder->GetNbrCategories() > 0 ){
      for( TInt i=0; i < dataHolder->GetNbrCategories(); i++ ){
         descArray->AppendL( *(dataHolder->GetCategory(i)) );
      }
   }
   TBool okChoosen;
   iSelectedCategory = PopUpList::ShowPopupListL(R_WAYFINDER_SOFTKEYS_OK_CANCEL,
                                                 *descArray, okChoosen );
   if( okChoosen ){
      iSearchString.Copy( (*descArray)[iSelectedCategory] );
      /* Selected a category string. */
      SetCategorySelected(ETrue);
      if( iQueryDialog ){
         /* Creates the iQueryDialog object. */
         iQueryDialog->SetString( iSearchString );
      }
   }
   CleanupStack::PopAndDestroy(descArray);
}

void CNewDestView::ReinitCountriesL( TUint aSelectedCountryId ){
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   bool setId = dataHolder->SetSelectedCountryId( aSelectedCountryId );
   
   if( iQueryDialog ){
      // Update the country field in the query dialog.
      if( setId ){
         TDesC* country = dataHolder->GetCountryFromId( aSelectedCountryId );
         iQueryDialog->SetCountry( *country );
      }
      else{
         // The previously selected country was not present,
         // select "current country".
         ResetCountryToDefault(IsGpsAllowed());
      }
      if(iContainer){
         iContainer->DrawNow();
      }
   }
}

void
CNewDestView::GetSearchHistory()
{

   /* Request the search history from Nav2. */
   /* Actually we will fake it for the moment. :-) */
   /* The results will arrive as a callback to SearchHistory(). */

   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   iShowSearchHistory = ETrue;
   /* XXX TRAPD ??? */
   ReceiveSearchHistoryL(dataHolder->GetSearchHistory());
}

void
CNewDestView::ReceiveSearchHistoryL(SearchHistoryDeque_t* searchHistoryContainer)
{
   /* We've gotten the search history. */
   if (iShowSearchHistory) {
      /* Initialize a list for the user to select from. */
      CDesCArrayFlat* list = new (ELeave) CDesCArrayFlat(20);
      LOGNEW(list, CDesCArrayFlat);

      CleanupStack::PushL( list );
      std::deque<SearchHistoryItem*>::iterator q;
      q = searchHistoryContainer->begin();
      while (q != searchHistoryContainer->end()) {
         TBuf<256> buf;
         char *tmp = (*q)->GetHistoryListEntry();
         WFTextUtil::char2TDes(buf, tmp);
         list->AppendL(buf);
         ++q;
      }
      TBool aOkChosen;
      TInt index = PopUpList::ShowPopupListL(R_WAYFINDER_SOFTKEYS_OK_CANCEL,
                                             *list, aOkChosen );
      SelectedHistoryItem(index);
      CleanupStack::PopAndDestroy(list);

      iShowSearchHistory = EFalse;
   } else {
      /* Not initiated from GetSearchHistory(). */
      /* Ignore. */
   }
}

void
CNewDestView::SelectedHistoryItem(TInt index)
{
   /* Get correct history item. */
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
   SearchHistoryItem *shi = dataHolder->GetSearchHistoryItem(index);
   if (!shi) {
      /* Failed!?! Don't do anything. */
      return;
   }

   /* Search string. */
   TBuf<256> tmp;
   WFTextUtil::char2TDes(tmp, shi->GetSearchString());
   iSearchString.Copy(tmp);
   if (iQueryDialog) {
      iQueryDialog->SetString(tmp);
   }

   WFTextUtil::char2TDes(tmp, shi->GetHouseNum());
   iHouseNum.Copy(tmp);
   if (iQueryDialog) {
      iQueryDialog->SetHouseNum(tmp);
   }

   /* City */
   if (strlen(shi->GetCityString()) == 0) {
      /* Current city. */
      ResetCityToDefault(IsGpsAllowed() );
   } else {
      /* Use value. */
      WFTextUtil::char2TDes(tmp, shi->GetCityString());
      iCity.Copy(tmp);
      if (iQueryDialog) {
         iQueryDialog->SetCity(iCity);
      }
      if (strlen(shi->GetCityId()) != 0) {
         /* We need to set the city id from history. */
         TBuf<KBuf64Length> tmp2;
         WFTextUtil::char2TDes(tmp2, shi->GetCityId());
         TInt save = dataHolder->GetNbrNewCities();
         dataHolder->AddNewCityL(tmp);
         dataHolder->AddNewCityIdL(tmp2);
         /* Set selected city index to the new element. */
         dataHolder->SetSelectedNewCity(save);
         WFDialog::ShowQueryL(tmp2);
      } else {
         /* Set selected city to unknown. */
         dataHolder->SetSelectedNewCity(MAX_UINT32);

/*          TBuf<KBuf64Length> tmp2; */
/*          tmp2.Copy(_L("No id for search")); */
/*          WFDialog::ShowQueryL(tmp2); */
      }
   }
   /* Country */
   if (strlen(shi->GetCountryString()) == 0) {
      /* No country string (From GPS probably) */
      ResetCountryToDefault(IsGpsAllowed() );
   } else {
      WFTextUtil::char2TDes(tmp, shi->GetCountryString());
      iCountry.Copy(tmp);
      if (iQueryDialog) {
         iQueryDialog->SetCountry(tmp);
      }
      int32 coi = strtol(shi->GetCountryId(), NULL, 10);
      dataHolder->SetSelectedCountryId( coi );
      /* Don't need to set country id, that is looked up */
      /* when the search is performed from the string. */
   }

}

void CNewDestView::ShowQueryDialog( TInt aCommand )
{
   iQueryDialog = CNewDestDialog::NewL( this, iLog );
   iQueryDialog->PrepareLC( R_NEWDEST_QUERY_DIALOG );

   if( IsGpsAllowed() ){
      if( IsSelectCity( iCity ) ) {
         WFS60Util::TDesReadResourceL( iCity,
               R_WAYFINDER_FROM_GPS_TEXT, iCoeEnv);
      }
      m_inited = ETrue;
   } else {
      if (!m_inited) {
         CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
         if (dataHolder->GetNumSearchHistoryItems() > 0) {
            /* Got search history, initiate strings to last search. */
            SearchHistoryItem *shi = dataHolder->GetLastHistoryItem();
            WFTextUtil::char2TDes(iSearchString, shi->GetSearchString());
            WFTextUtil::char2TDes(iHouseNum, shi->GetHouseNum());
            if (strlen(shi->GetCityString())) {
               WFTextUtil::char2TDes(iCity, shi->GetCityString());
            }
            if (strlen(shi->GetCountryString())) {
               WFTextUtil::char2TDes(iCountry, shi->GetCountryString());
               TBuf<32> tmp;
               WFTextUtil::char2TDes(tmp, shi->GetCountryId());
               int32 coi = strtol(shi->GetCountryId(), NULL, 10);
               dataHolder->SetSelectedCountryId( coi );
            }
         }
         m_inited = ETrue;
      }
      if( IsCurrentCity( iCity ) ){
         iCity.Copy(_L(""));
/*          iCoeEnv->ReadResource( iCity, R_WAYFINDER_SELECT_CITY_TEXT ); */
      }
   }
   if (
      (IsGpsAllowed() && IsSelectCountry(iCountry)) ||
      (!(IsGpsAllowed()) && IsCurrentCountry(iCountry)) 
      )
   {
      ResetCountryToDefault(IsGpsAllowed());
   }

   CEikEdwin* edwin = STATIC_CAST( CEikEdwin*, iQueryDialog->ControlOrNull(EWayFinderCtrlNewDestCountryEdwin) );
   if( edwin )
      edwin->SetTextL( &iCountry );
   edwin = STATIC_CAST( CEikEdwin*, iQueryDialog->ControlOrNull(EWayFinderCtrlNewDestHouseNumEdwin) );
   if( edwin ) {
      edwin->SetTextL( &iHouseNum );
   }
   edwin = STATIC_CAST( CEikEdwin*, iQueryDialog->ControlOrNull(EWayFinderCtrlNewDestCityEdwin) );
   if( edwin )
      edwin->SetTextL( &iCity );
   edwin = STATIC_CAST( CEikEdwin*, iQueryDialog->ControlOrNull(EWayFinderCtrlNewDestStringEdwin) );
   if( edwin )
      edwin->SetTextL( &iSearchString );
   iQueryDialog->RunLD();

   if( aCommand != ENoMessage ){
      iCommand = aCommand;
   }

   //if( iCommand == ERoutePlannerSetAsOrigin ){
   if( iCustomMessageId == ERoutePlannerSetAsOrigin ){
      iIsOrigin = ETrue;
   } else {
      iIsOrigin = EFalse;
   }


}

void CNewDestView::CloseQueryDialog()
{
   if( iQueryDialog )
   {
      iQueryDialog->Close(EFalse);
      iQueryDialog = NULL;
   }
}

void CNewDestView::SaveResultL()
{
   if( iContainer && iContainer->HasResults() ){
      HBufC* hId = iWayfinderAppUi->GetDataHolder()->GetNewDestId( iContainer->GetSelIndex());
      char *id = WFTextUtil::newTDesDupL(*hId);
      LOGNEWA(id, char, strlen(id) + 1);

      GenericGuiMess message(GuiProtEnums::ADD_FAVORITE_FROM_SEARCH, id);
      iWayfinderAppUi->SendMessageL( &message );
      message.deleteMembers();
      LOGDELA(id);
      delete[] id;
   }
}

void CNewDestView::ShowOnMap()
{
   if (iContainer) {
      CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
      TPoint pos = dataHolder->GetNewDestPos(iContainer->GetSelIndex());
      iWayfinderAppUi->RequestMap( MapEnums::SearchPosition, pos.iX, pos.iY, 
                                   EShowOnMap );
//       iWayfinderAppUi->RequestMap( MapEnums::SearchPosition, pos.iX, pos.iY, 
//                                    GetViewState() );
   }
}

void CNewDestView::GetDetails()
{
   if( iContainer && iContainer->HasResults() ){
      GenericGuiMess* message = new (ELeave) GenericGuiMess(
            GuiProtEnums::GET_FULL_SEARCH_DATA, 
            (uint16)iContainer->GetSelIndex(),
            (uint16)iContainer->GetSelIndex() );
      LOGNEW(message, GenericGuiMess);
      iWayfinderAppUi->SendMessageL( message );
      message->deleteMembers();
      LOGDEL(message);
      delete message;
   }
}

HBufC *
CNewDestView::GetRegionTypeName(const SearchRegion* region)
{
   HBufC *aName;
   if (!region) {
      return NULL;
   }
   TInt aResourceId = 0;

   switch(region->getType()){
   case GuiProtEnums::streetNumber:
      aResourceId = R_TEXT_SEARCH_DETAIL_01;
      break;
   case GuiProtEnums::address:
#ifdef USE_ADDRESS_FROM_SEARCH_REPLY
      aResourceId = R_TEXT_SEARCH_DETAIL_02;
#else
      return NULL;
#endif
      break;
   case GuiProtEnums::cityPart:
      aResourceId = R_TEXT_SEARCH_DETAIL_03;
      break;
   case GuiProtEnums::city:
      aResourceId = R_TEXT_SEARCH_DETAIL_04;
      break;
   case GuiProtEnums::municipal:
      aResourceId = R_TEXT_SEARCH_DETAIL_05;
      break;
   case GuiProtEnums::country:
      aResourceId = R_TEXT_SEARCH_DETAIL_06;
      break;
   case GuiProtEnums::state:
      aResourceId = R_TEXT_SEARCH_DETAIL_07;
      break;
   case GuiProtEnums::zipcode:
      aResourceId = R_TEXT_SEARCH_DETAIL_08;
      break;
   default:
      aResourceId = R_TEXT_SEARCH_DETAIL_09;
      break;
   }
   aName = CCoeEnv::Static()->AllocReadResourceL( aResourceId );
   return aName;
}

const AdditionalInfo*
GetSearchInfo(const FullSearchItem* item, TInt index)
{
   const AdditionalInfo* info = NULL;
   for (TInt i=0; i <= index; i++) {
      info = item->getInfo(i);
      if( (GuiProtEnums::AdditionalInfoType)info->getType()
            == GuiProtEnums::dont_show ){
         index++;
      }
   }
   return info;
}

void CNewDestView::ShowDetailsL( FullSearchDataReplyMess* aMessage, TInt aCommand )
{
   if( aMessage->size() > 0 ) {
      HBufC* name;
      HBufC* tmp;
      Favorite::InfoVect delinfos;
      avect_t infos;

      const FullSearchItem* item = aMessage->operator [](0);
      _LIT( KEmptyStr, "" );
      _LIT( KabStr, "\t" );

      // Name
      if ( item->getName() != NULL ) {
         /* Insert Name key value first. */
         name = iCoeEnv->AllocReadResourceLC( R_TEXT_SEARCH_DETAIL_11 );
         tmp = WFTextUtil::SearchAndReplaceL( name->Des(), 
                                              KabStr(), KEmptyStr() );
         CleanupStack::PushL( tmp );
         char* cname = WFTextUtil::newTDesDupLC( *tmp );
         delinfos.push_back( new AdditionalInfo( 
                                cname,
                                item->getName(),
                                GuiProtEnums::text ) );
         infos.push_back( delinfos.back() );
         CleanupStack::PopAndDestroy( cname );
         CleanupStack::PopAndDestroy( tmp );
         CleanupStack::PopAndDestroy( name );
      }

      uint32 i = 0;
      // Infos
      for ( i = 0 ; i < item->noAdditionalInfo() ; ++i ) {
         if ( item->getInfo( i ) != NULL ) {
            infos.push_back( item->getInfo( i ) );
         }
      }

      // Regions
      for ( i = 0 ; i < item->noRegions() ; ++i ) {
         const SearchRegion* region = item->getRegion( i );
         HBufC* name = GetRegionTypeName( region );
         if ( name != NULL && region->getName() != NULL ) {
            tmp = WFTextUtil::SearchAndReplaceL( name->Des(), 
                                                 KabStr(), KEmptyStr() );
            CleanupStack::PushL( tmp );
            char* cname = WFTextUtil::newTDesDupLC( *tmp );
            delinfos.push_back( new AdditionalInfo( 
                                   cname,
                                   region->getName(),
                                   GuiProtEnums::text ) );
            infos.push_back( delinfos.back() );
            CleanupStack::PopAndDestroy( cname );
            CleanupStack::PopAndDestroy( tmp );
         }
         delete name;
      }

      // Max size
      Buffer b;
      item->serialize( &b );


      // Show it
      CNewDestView::ShowInfoDetailsL( infos, 
                                      iApaLsSession,
                                      iWayfinderAppUi,
                                      this,
                                      aCommand,
                                      b.getLength() );

      for ( Favorite::InfoVect::const_iterator it = delinfos.begin() ;
            it != delinfos.end() ; ++it )
      {
         delete *it;
      }

   } // End if message has size > 0
}

void CNewDestView::ShowInfoDetailsL( const avect_t& infos, 
                                     RApaLsSession iApaLsSession,
                                     CWayFinderAppUi* iWayfinderAppUi,
                                     CAknView* view,
                                     TInt aCommand,
                                     TInt maxLength ) 
{
   TBool done = EFalse;
   TInt selection = 0;
   while ( !done ) {
      //TBuf<maxLength> text;
      HBufC* text = HBufC::NewLC( maxLength );
      //TBuf<maxLength> temp;
      HBufC* temp = HBufC::NewLC( maxLength );
      CDesCArrayFlat* array = new (ELeave) CDesCArrayFlat(4);
      LOGNEW(array, CDesCArrayFlat);
      CleanupStack::PushL( array );
      TInt firstInfo = array->MdcaCount();
      TInt num_add_info = infos.size();
      const AdditionalInfo* info;
      GuiProtEnums::AdditionalInfoType infoType;

      for ( avect_t::const_iterator it = infos.begin() ; it != infos.end() ;
            ++it )
      {
         if ( (*it)->getType() != GuiProtEnums::dont_show ){
            WFTextUtil::char2HBufC( text, (*it)->getKey() );
            //if the key is address (or whatever language
            //equivalent), don't show this AdditionalInfo.
            text->Des().Insert( 0, KTab );
            text->Des().Append( KTab );
            WFTextUtil::char2HBufC( temp, (*it)->getValue() );
            text->Des().Append( *temp );
            array->AppendL( *text );
         } else {
            /* XXX Problem with additional info getting later. */
            /* XXX We should make sure later that the additional */
            /* XXX info we're getting isn't a "don't show". */
            num_add_info--;
         }
      }

      CleanupStack::Pop( array );

      CAknSelectionListDialog *
         dialog = CAknSelectionListDialog::NewL( selection, array, 
                                                 R_NEWDEST_DETAILS_MENUBAR, 
                                                 view );
      dialog->SetupFind( CAknSelectionListDialog::ENoFind );
      if ( dialog->ExecuteLD( R_NEWDEST_DETAILS_DIALOG ) ) {
         //If the joystick is pressed

         if ( infos.size() > 0 &&
              selection >= firstInfo &&
              selection < (firstInfo + num_add_info)){
            info = infos[ selection-firstInfo ];
            if ( info ) {
               infoType = (GuiProtEnums::AdditionalInfoType)info->getType();
               switch( infoType )
               {
                  case GuiProtEnums::phone_number:
                  case GuiProtEnums::mobile_phone:
                  {
                     char *tmp2 = WFTextUtil::stripPhoneNumberL(info->getValue());
                     WFTextUtil::char2HBufC( temp, tmp2);
                     delete tmp2;
                     /* Shouldn't dial directly, let user edit */
                     /* phone number first. BUT NOOOO, Nokia doesn't */
                     /* want that. */
/*                            iWayfinderAppUi->EditAndCallNumberL(temp, */
/*                                  aCommand ); */
/*                           iWayfinderAppUi->DialNumberL(temp,
                             aCommand );*/
                     /* Now back to our way */
                     iWayfinderAppUi->EditAndCallNumberL( *temp, 
                                                       aCommand );
                  }
                  break;
                  case GuiProtEnums::url:
                  case GuiProtEnums::wap_url:
                  {
                     HBufC* tmp2 = HBufC::NewLC(strlen(info->getValue())+4);
                     HBufC* tmp3 = HBufC::NewLC(strlen(info->getValue())+4);
                     tmp2->Des().Copy(_L("4 "));
                     WFTextUtil::char2HBufC( tmp3, info->getValue() );
                     tmp2->Des().Append(*tmp3);


                     WFS60Util::LaunchWapBrowserL(*tmp2, iApaLsSession);

                     CleanupStack::PopAndDestroy(tmp3);
                     CleanupStack::PopAndDestroy(tmp2);
                  }
                  break;
                  case GuiProtEnums::email:
                  {
                     HBufC* tmp2 = HBufC::NewLC(strlen(info->getKey())+4);
                     HBufC* tmp3 = HBufC::NewLC(strlen(info->getValue())+4);
                     WFTextUtil::char2HBufC( tmp2, info->getKey() );
                     /*                      tmp2->Des().Copy(temp); */
                     WFTextUtil::char2HBufC( tmp3, info->getValue() );

                     WFDialog::ShowScrollingDialogL(*tmp2, *tmp3, EFalse);

                     CleanupStack::PopAndDestroy(tmp3);
                     CleanupStack::PopAndDestroy(tmp2);
                  }
                  break;
                  default:
                  {
                     HBufC* tmp2 = HBufC::NewLC(strlen(info->getKey())+4);
                     HBufC* tmp3 = HBufC::NewLC(strlen(info->getValue())+4);
                     WFTextUtil::char2HBufC( tmp2, info->getKey() );
                     WFTextUtil::char2HBufC( tmp3, info->getValue() );

                     WFDialog::ShowScrollingDialogL(*tmp2, *tmp3, EFalse);

                     CleanupStack::PopAndDestroy(tmp3);
                     CleanupStack::PopAndDestroy(tmp2);
                  }
                  break;
               }
            }
         }/* Not additional info field. */
      } else { // Cancel from dialog
         done = ETrue;
      }

      CleanupStack::PopAndDestroy( temp );
      CleanupStack::PopAndDestroy( text );
   } // End while !done
}

void 
CNewDestView::SetSearchStrings(const TDesC &aCountry, const TDesC &aCity, const TDesC& aHouseNum, const TDesC &aString )
{
   iCountry.Copy( aCountry );
   iHouseNum.Copy( aHouseNum );
   iCity.Copy( aCity );
   iSearchString.Copy( aString );
}

void
CNewDestView::SetAsOrigin( TInt32 aLat, TInt32 aLon )
{
   TBuf<KBuf256Length> name;
   if (iContainer) {
      if( iContainer->GetSelItem( name ) ){
         CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
         HBufC* hId = dataHolder->GetNewDestId( iContainer->GetSelIndex());
         char *id = WFTextUtil::newTDesDupL(*hId);
         LOGNEWA(id, char, strlen(id) + 1);
         iWayfinderAppUi->SetOrigin( GuiProtEnums::PositionTypePosition, name, id, aLat, aLon );
         LOGDELA(id);
         delete[] id;
      }
      iWayfinderAppUi->push( KPositionSelectViewId );
   }
}

void
CNewDestView::SetAsDestination( TInt32 aLat, TInt32 aLon )
{
   TBuf<KBuf256Length> name;
   if (iContainer) {
      if( iContainer->GetSelItem( name ) ){
         CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
         HBufC* hId = dataHolder->GetNewDestId( iContainer->GetSelIndex());
         char *id = WFTextUtil::newTDesDupL(*hId);
         LOGNEWA(id, char, strlen(id) + 1);
         iWayfinderAppUi->SetDestination(GuiProtEnums::PositionTypeSearch,
               name, id, aLat, aLon );
         LOGDELA(id);
         delete[] id;
      }
      iWayfinderAppUi->push( KPositionSelectViewId );
   }
}

TBool
CNewDestView::SettingOrigin()
{
   return (iCustomMessageId == ERoutePlannerSetAsOrigin);
}

TBool CNewDestView::SettingDestination()
{
   return (iCustomMessageId == ERoutePlannerSetAsDest);
}

void CNewDestView::HandleDisableCategory()
{
   iCategorySelected = EFalse;
}

void CNewDestView::RemoveInputFocus()
{
   if( iQueryDialog ){
      CEikEdwin* edwin = STATIC_CAST(CEikEdwin*,iQueryDialog->ControlOrNull(EWayFinderCtrlNewDestCityEdwin));
      if( edwin ) {
         edwin->SetFocus( EFalse );
      }
      edwin = STATIC_CAST(CEikEdwin*,iQueryDialog->ControlOrNull(EWayFinderCtrlNewDestHouseNumEdwin));
      if( edwin ) {
         edwin->SetFocus( EFalse );
      }
      edwin = STATIC_CAST(CEikEdwin*,iQueryDialog->ControlOrNull(EWayFinderCtrlNewDestStringEdwin));
      if( edwin ) {
         edwin->SetFocus( EFalse );
      }
   }
}

void CNewDestView::ShowNewSearchHelpL()
{
   iWayfinderAppUi->HandleCommandL(EWayFinderCmdHelp);
}

void CNewDestView::FavoriteSaved()
{
   WFDialog::ShowConfirmationL( R_WAYFINDER_FAVORITESUPDATED_MSG );
}

TBool
CNewDestView::MatchesResourceString( TDesC &string, TInt resourceId)
{
   HBufC *tmp;
   tmp = iCoeEnv->AllocReadResourceL( resourceId );
   return ( string.Find(*tmp) == 0 );
}

TBool CNewDestView::IsCurrentCountry( TDesC &countryString )
{
   return MatchesResourceString(countryString, R_WAYFINDER_FROM_GPS_TEXT);
}
TBool CNewDestView::IsCurrentCity( TDesC &cityString )
{
   return MatchesResourceString(cityString, R_WAYFINDER_FROM_GPS_TEXT);
}

TBool CNewDestView::IsSelectCountry( TDesC &countryString )
{
   return MatchesResourceString(countryString, R_WAYFINDER_SELECT_COUNTRY_TEXT);
}
TBool CNewDestView::IsSelectCity( TDesC& /*cityString*/ )
{
   return EFalse;
/*    return MatchesResourceString(cityString, R_WAYFINDER_SELECT_CITY_TEXT); */
}

GuiDataStore*
CNewDestView::GetGuiDataStore()
{
   return iWayfinderAppUi->GetGuiDataStore();
}

void
CNewDestView::TimerTick()
{
   iContainer->StopTimer();
   /* A tick of the timer. */
   if (iNewSearch) {
      /* Popup new search dialog. */
      ShowQueryDialog( iCommand );
   } else {
      /* Do nothing. */
      if(!iContainer->HasResults()) {
         ShowQueryDialog( iCommand );
         if (iQueryDialog) {
            iQueryDialog->SetString(iSearchString);
            iQueryDialog->SetCity(iCity);
            iQueryDialog->SetHouseNum(iHouseNum);
            iQueryDialog->SetCountry(iCountry);
         } 
      }
   }
}


void
CNewDestView::ShowCountryListSelectionL()
{
//    if (iQueryDialog) {
//       iQueryDialog->RemoveInputFocus();
//    }
   // Show popup.
   iSelectingCountry = ETrue;
   typedef TCallBackEvent<CNewDestView, TSearchEvent> cb_t;
   typedef CCallBackSelectionListDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
                        ESearchEventCountryListDialogOk,
                        ESearchEventCountryListDialogCancel),
                   iSelectedCountryItem,
                   iCountryList,
                   R_WAYFINDER_CALLBACK_SELECT_LIST_QUERY,
                   iLastCountryIndex);
}

void
CNewDestView::GenerateEvent(enum TSearchEvent aEvent)
{        
   if (iEventGenerator) {
      iEventGenerator->SendEventL(aEvent);
   }
}           

void
CNewDestView::HandleGeneratedEventL(enum TSearchEvent aEvent)
{
   switch(aEvent){
      case ESearchEventCountryListDialogOk:
         {
            CountrySelectionOkL();

         } break;
      case ESearchEventCountryListDialogCancel:
         {
            CountrySelectionCancelL();
         } break;
   }
   iSelectingCountry = EFalse;
}


void
CNewDestView::CountrySelectionOkL()
{
   CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();

   if(IsGpsAllowed()) {
      iSelectedCountryItem--;
   }

   if (iSelectedCountryItem == -1) {
      /* From GPS. */
      HBufC* frmGPS = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_FROM_GPS_TEXT);
      iCountry.Copy(*frmGPS);
      if (iQueryDialog) {
         iQueryDialog->SetCountry(*frmGPS);
         iQueryDialog->RemoveInputFocus();
      }
      CleanupStack::PopAndDestroy(frmGPS);
   } else {
      HBufC* tmp = dataHolder->GetCountry(iSelectedCountryItem);
      iCountry.Copy(*tmp);
      if (iQueryDialog) {
         iQueryDialog->SetCountry(*tmp);
         iQueryDialog->RemoveInputFocus();
      }
   }

   if (int(dataHolder->GetSelectedCountry()) != iSelectedCountryItem) {
      /* Set selected city to unknown. */
      dataHolder->SetSelectedNewCity(MAX_UINT32);
   }
   dataHolder->SetSelectedCountry( iSelectedCountryItem );
   iLastCountryIndex = iSelectedCountryItem;

   iCountryList->Reset();
   delete iCountryList;
   iCountryList = NULL;
}

void
CNewDestView::CountrySelectionCancelL()
{
   /* Do nothing. */
   iCountryList->Reset();
   delete iCountryList;
   iCountryList = NULL;
}


// ---------------------------------------------------------
// TUid CNewDestView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CNewDestView::Id() const
{
   return KNewDestViewId;
}
// ---------------------------------------------------------
// CNewDestView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CNewDestView::HandleCommandL(TInt aCommand)
{
   switch ( aCommand )
   {
   case EAknSoftkeyBack:
      {
         ShowQueryDialog( ENoMessage );
         iNewSearch = ETrue;
         break;
      }
   case EWayFinderCmdNewDestSearch:
      {
         SearchL();
         break;
      }
   case EWayFinderCmdNewDestCountry:
      {
         ShowCountriesL();
         break;
      }
   case EWayFinderCmdNewDestCity:
      {
         ShowCitiesL();
         break;
      }
   case EWayFinderCmdNewDestCategory:
      {
         ShowCategoriesL( ENoMessage );
         break;
      }
   case EWayFinderCmdNewDestString:
      {
         GetSearchHistory();
         break;
      }
   case EWayFinderCmdNewDestSave:
      {
         SaveResultL();
         break;
      }
   case EWayFinderCmdNewDestShow:
      {
         ShowOnMap();
         break;
      }
   case EWayFinderCmdNewDestNextXHits:
      {
         RequestNextXHits();
         break;
      }
   case EWayFinderCmdNewDestPrevXHits:
      {
         RequestPrevXHits();
         break;
      }
   case EWayFinderCmdNewDestNavigate:
      {
         if (iWayfinderAppUi->IsGpsAllowed()) {
            //RouteL(); //Used to route before going between route planner.
            iWayfinderAppUi->SetFromGps();
            HandleCommandL(EWayFinderCmdNewDestDestination);
         } else {
            /* If no GPS is connected, don't do anything. */
         }
         break;
      }
   case EWayFinderCmdNewDestShowInfo:
      {
         if (iContainer) {
            CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
            TPoint pos = dataHolder->GetNewDestPos(iContainer->GetSelIndex());
            Nav2Coordinate nav2Coord(pos.iX, pos.iY);
            MC2Coordinate position(nav2Coord);
            iWayfinderAppUi->GotoInfoInServiceViewL(position.lat, position.lon);
         }
         break;
      }
   case EWayFinderCmdNewDestOrigin:
      {
         if (iContainer) {
            CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
            TPoint pos = dataHolder->GetNewDestPos(iContainer->GetSelIndex());
            SetAsOrigin( pos.iX, pos.iY);
         }
         break;
      }
   case EWayFinderCmdNewDestDestination:
      {
         if (iContainer) {
            CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
            TPoint pos = dataHolder->GetNewDestPos(iContainer->GetSelIndex());
            SetAsDestination( pos.iX, pos.iY);
         }
         break;
      }
   case EWayFinderCmdNewDestNewSearch:
      {
         ShowQueryDialog( ENoMessage );
         break;
      }
   case EWayFinderCmdNewDest2:
      {
         CloseQueryDialog();
         if( !iContainer ) {
            AppUi()->HandleCommandL( aCommand );
         }
         break;
      }
   case EWayFinderCmdNewDestQueryBack:
      {
         /* NOTE!!! This event may be the */
         /* ultimate effect of calling CloseQueryDialog! */
         iQueryDialog = NULL;

         iWayfinderAppUi->pop();
         break;
      }
   case EWayFinderCmdNewDestInfo:
      {
         CloseQueryDialog();
         AppUi()->HandleCommandL( aCommand );
         break;
      }
   case EWayFinderCmdDebugAction:
      {
         TBuf<64> temp( _L("+4646235268") );
         iWayfinderAppUi->EditAndCallNumberL( temp, EWayFinderCmdNewDestCall );
         break;
      }
   case EAknCmdExit:
   case EAknSoftkeyExit:
//       if (0 == WFDialog::ShowQueryL(R_WAYFINDER_CONFIRM_EXIT_MSG, iCoeEnv)) {
//       HBufC* buf;
//       buf = CCoeEnv::Static()->AllocReadResourceLC(R_WAYFINDER_CONFIRM_EXIT_MSG);
//       if (0 == CExitQueryDlg::RunDlgLD(*buf)) {
//          CleanupStack::PopAndDestroy(buf);
//          /* Don't fall through to below exit. */
//          return ;
//       }
//       CleanupStack::PopAndDestroy(buf);
      iWayfinderAppUi->HandleCommandL(aCommand);
      break;
      /* FALLTHROUGH */
   case EEikCmdExit:
   case EWayFinderExitAppl:
      {
         iWayfinderAppUi->ShutdownNow();
      }
      break;
   default:
      {
         AppUi()->HandleCommandL( aCommand );
         break;
      }
   }
}

// ---------------------------------------------------------
// CNewDestView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CNewDestView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}
// ---------------------------------------------------------
// CNewDestView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CNewDestView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_NEWDEST_MENU ){
      /* Dim everything, un-dim it below. */
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSave, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSendTo, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShow, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNextXHits, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestPrevXHits, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestOrigin, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestDestination, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, ETrue );

      if ( iContainer && iContainer->HasResults() ){
         /* There are results in the list. */

         if (iCustomMessageId == ERoutePlannerSetAsOrigin ||
             iCustomMessageId == ERoutePlannerSetAsDest) {
            if (iCustomMessageId == ERoutePlannerSetAsOrigin) {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestOrigin, EFalse );
            }
            if (iCustomMessageId == ERoutePlannerSetAsDest) {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestDestination, EFalse );
            }
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShow, EFalse );
            
            CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
            if ((dataHolder->GetSearchStartIndex() + MAX_NUM_SEARCH_HITS) >=
                dataHolder->GetSearchTotalHits()) {
               /* No more hits, can't get next X. */
            } else {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNextXHits, EFalse );
            }
            if (dataHolder->GetSearchStartIndex() > 0) {
               /* Not at beginning. */
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestPrevXHits, EFalse );
            }
         } else {
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSave, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestSendTo, EFalse );
            aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShow, EFalse );
            
            if (iWayfinderAppUi->HideServicesInfo()) {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, ETrue );
            } else {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestShowInfo, EFalse );
            }
            
            if( IsGpsAllowed() ) {
               /* We have GPS. */
               /* Add "Calculate Route" option. */
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNavigate, EFalse );
               /* Remove the "set as ..." options. */
               /* ...but show them at the bottom of the menu. */
/*                aMenuPane->SetItemDimmed(EWayFinderCmdNewDestOrigin, EFalse ); */
/*                aMenuPane->SetItemDimmed(EWayFinderCmdNewDestDestination, EFalse); */
            } else {
               /* No GPS available. Don't show "Calculate Route" option. */
               /* ...but show the set as start/destination at the top of menu. */
               aMenuPane->SetItemDimmed(EWayFinderCmdNewDestOrigin, EFalse );
               aMenuPane->SetItemDimmed(EWayFinderCmdNewDestDestination, EFalse );
            }
            CDataHolder* dataHolder = iWayfinderAppUi->GetDataHolder();
            if ((dataHolder->GetSearchStartIndex() + MAX_NUM_SEARCH_HITS) >=
                dataHolder->GetSearchTotalHits()) {
               /* No more hits, can't get next X. */
            } else {
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestNextXHits, EFalse );
            }
            if (dataHolder->GetSearchStartIndex() > 0) {
               /* Not at beginning. */
               aMenuPane->SetItemDimmed( EWayFinderCmdNewDestPrevXHits, EFalse );
            }
         }
      }
   }
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CNewDestView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CNewDestView::DoActivateL( const TVwsViewId& aPrevViewId,
                                TUid aCustomMessageId,
                                const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   if (aCustomMessageId.iUid == EKeepOldStateMessage) {
      //Do nothing
      iNewSearch = EFalse;
   } else if (iCustomMessageId == ENewSearch) {
      iNewSearch = ETrue;
      iCommand = ENoMessage;
   } else if (iCustomMessageId == ERoutePlannerSetAsOrigin ||
              iCustomMessageId == ERoutePlannerSetAsDest) {
      iNewSearch = ETrue;
      iCommand = iCustomMessageId;
   } else {
      iNewSearch = EFalse;
      iCommand = ENoMessage;
   }


   if ( !iContainer ){
      iContainer = new (ELeave) CNewDestContainer(iLog);
      LOGNEW(iContainer, CNewDestContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect(), this );
      AppUi()->AddToStackL( *this, iContainer );
      iContainer->StartTimer();

   }
   SetSearchDataL();
}

// ---------------------------------------------------------
// CNewDestView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CNewDestView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }
   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
   iQueryDialog = NULL;
}


// End of File
