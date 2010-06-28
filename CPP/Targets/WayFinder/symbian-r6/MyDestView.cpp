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
#include <aknviewappui.h>
#include <akntitle.h>
#include <eikmenup.h>
#include <avkon.hrh>
#include <aknnotewrappers.h> 
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>
#include <aknselectionlist.h>
#include <stringloader.h>
#if defined NAV2_CLIENT_SERIES60_V5
# include <akntoolbar.h>
# include <aknbutton.h>
#endif

#include <vector>
#include <arch.h>
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "MapEnums.h"
#include "TimeOutNotify.h"
#include "WayFinderAppUi.h"

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "MyDestView.h"

#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiProtRouteMess.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/FullSearchItem.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/SearchRegion.h"
#include "DistancePrintingPolicy.h"

#include "MapView.h"
#include "PositionSelectView.h"
#include "MyDestContainer.h"

#include "EditFavoriteView.h"

#include "memlog.h"

#include "BufferArray.h"
#include "DataHolder.h"

#include "WFS60Util.h"
#include "Dialogs.h"
#include "SettingsData.h"

#include "MC2Coordinate.h"
#include "PathFinder.h"
#include "WFLMSManagerBase.h"
#include "DeleteHelpers.h"

#include "PathFinder.h"
#include "wficons.mbg"

/* For RApaLsSession */
#include <apgcli.h>

using namespace std;
using namespace isab;

// ================= MEMBER FUNCTIONS =======================

CMyDestView::CMyDestView(CWayFinderAppUi* aUi, isab::Log* aLog) : 
   CViewBase(aUi),
   iLog(aLog) 
{}
// ---------------------------------------------------------
// CMyDestView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CMyDestView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_MYDEST_VIEW );
   iSaving = EFalse;
   iEditing = EFalse;
   iRemoving = EFalse;
   iShowInMap = EFalse;
   iSetOrigin = EFalse;
   iSetDestination = EFalse;
   iSendToFriend = EFalse;
   iShowFavorite = EFalse;
   iFavVec = new std::vector<GuiFavorite*>(50);

#if defined NAV2_CLIENT_SERIES60_V5
   if (Toolbar()) {
      Toolbar()->SetToolbarObserver( this );
   }
#endif
}

CMyDestView* CMyDestView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CMyDestView* self = new (ELeave) CMyDestView(aUi, aLog);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CMyDestView* CMyDestView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CMyDestView *self = CMyDestView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CMyDestView::~CMyDestView()
// ?implementation_description
// ---------------------------------------------------------
//
CMyDestView::~CMyDestView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }
   LOGDEL(iContainer);

   STLUtility::deleteValues(*iFavVec);
   delete iFavVec;
   iApaLsSession.Close();
   delete iContainer;
}

void CMyDestView::HandleFavoritesReply( GetFavoritesReplyMess* aMessage )
{
   TInt mess = -1;
   if( iSaving ){
      iSaving = EFalse;
      mess = R_WAYFINDER_FAVORITESAVED_MSG;
   } else if( iEditing ){
      mess = R_WAYFINDER_FAVORITECHANGED_MSG;
      iEditing = EFalse;
   } else if( iRemoving ){
      mess = R_WAYFINDER_FAVORITEREMOVED_MSG;
      iRemoving = EFalse;
   }
   if (mess > 0) {
      WFDialog::ShowConfirmationL( mess, iCoeEnv);
   }

   if (!iGotLmsFavs) {
      // Take ownership of the vector in aMessage
      iMergeVec = aMessage->getGuiFavorites();
   } else {
      // In this case we have gotten both the nav2 favs and the lms favs.
      // So merge them together to the one favorite vector.      
      std::vector<GuiFavorite*>* tmpVec = aMessage->getGuiFavorites();
      MergeNav2LmsFavVectors(tmpVec);
      // Clear the vector, we dont delete the objects since they are 
      // now pointed at in the iFavVec.
      tmpVec->clear();
      delete tmpVec;
   }
   iGotNav2Favs = ETrue;
   if (iGotLmsFavs && iGotNav2Favs) {
      // If we have gotten both the favorites from nav2 and from the lms,
      // we can now add all favorites to the listbox
      AddFavoritesToListL();
   }
}

void CMyDestView::HandleLmsImportReplyL(std::vector<GuiFavorite*>* aLmsList)
{
   if (!iGotNav2Favs) {
      // Set iMergeVec to point to the same vector as aLmsList points
      // to and then tell the WFLMSManager to give us the ownership of
      // the vector.
      iMergeVec = aLmsList;
      iWayfinderAppUi->DisownGuiLandmarkList();
   } else {
      // In this case we have gotten both the nav2 favs and the lms favs.
      // So merge them together to the one favorite vector.
      MergeNav2LmsFavVectors(aLmsList);
      // Tell the WFLMSManager to give us the ownership of its vector. 
      iWayfinderAppUi->DisownGuiLandmarkList();;
   }
   iGotLmsFavs = ETrue;
   if (iGotLmsFavs && iGotNav2Favs) {
      // If we have gotten both the favorites from nav2 and from the lms,
      // we can now add all favorites to the listbox
      AddFavoritesToListL();
   }
}

void 
CMyDestView::MergeNav2LmsFavVectors(std::vector<isab::GuiFavorite*>* aAddVec)
{
   // Clear the iFavVec and reserve/resize space to the vec.
   STLUtility::deleteValues(*iFavVec);
   iFavVec->resize(iMergeVec->size() + aAddVec->size(), NULL);

   // Put together iMergeVec (holding lms/nav2 favs) and tmpVec 
   // (holding lms/nav2 favs) into iFavVec.
   std::copy(iMergeVec->begin(), iMergeVec->end(), iFavVec->begin());
   std::copy(aAddVec->begin(), aAddVec->end(), 
             (iFavVec->begin() + iMergeVec->size()));
   // Sort the result here in the gui to make sure we have a sorted list 
   // of favorites. This is since it seems that the strCollateUtf8 function 
   // that is used to sort the favorites in GuiFavoriteCmp does not work 
   // with for example swedish characters.
   std::sort(iFavVec->begin(), iFavVec->end(),
             UiGuiFavoriteCmp());
   
   // Clear the vectors, we dont delete their objects since they are are 
   // now pointed at in the iFavVec.
   iMergeVec->clear();
   delete iMergeVec;
   iMergeVec = NULL;
}

void CMyDestView::AddFavoritesToListL()
{
   if (!iContainer) {
      return;
   }
   iContainer->RemoveAllItemsL();

   if( iFavVec->size() > 0 ){
      GuiFavorite* favorite = NULL;
      HBufC* buffer = HBufC::NewLC(256);
      for (std::vector<GuiFavorite*>::const_iterator it = iFavVec->begin(); 
           it != iFavVec->end(); 
           it++) {
         favorite = *it;
         const char *favname = favorite->getName();
         // Clear existing data from the buffer
         buffer->Des().Zero();
         TInt length = strlen( favname );
         if ( length >= buffer->Des().MaxLength() ) {
            // If the length of favname is longer than buffer
            // realloc more memory
            HBufC* tmp = buffer->ReAllocL( length + 8 );
            CleanupStack::Pop( buffer );
            buffer = tmp;
            CleanupStack::PushL( buffer );
         }
         TPtr name = buffer->Des();
         WFTextUtil::char2TDes(name, favname);
         name.PtrZ();
         TPtrC shortName(name);
#if defined NAV2_CLIENT_SERIES60_V5
         // On s60v5 we use a single line listbox so we dont want the tab + 
         // the description we only want the name.
         TInt pos = name.Find(KTab);
         if (pos != KErrNotFound) {
            shortName.Set(name.Left(pos));
         }
#endif
         if (CEditFavoriteView::IsWayfinderFavorite(favorite)) {
            // Wayfinder favorite
            iContainer->AddItemL(shortName, CMyDestContainer::EFavoriteIconIndex);
         } else {
            iContainer->AddItemL(shortName, CMyDestContainer::ELandmarkIconIndex);
         }
         name.Zero();
      }
      CleanupStack::PopAndDestroy( buffer );

      iContainer->ShowPreviewContent(ETrue);
      iContainer->UpdatePreviewContentL();
   } else {
      iContainer->ShowPreviewContent(EFalse);
   }

   // Reset flags
   iGotNav2Favs = EFalse;
   iGotLmsFavs  = EFalse;
}

void CMyDestView::HandleFavoritesAllDataReply( GetFavoritesAllDataReplyMess* /*aMessage*/ )
{
}

void CMyDestView::HandleFavoriteInfoReply( GetFavoriteInfoReplyMess* aMessage )
{
   Favorite* favorite = aMessage->getFavorite();
   if( iShowInMap ){
      ShowInfoInMap(*favorite);
   }
   else if( iSetOrigin ){
      SetAsOrigin( favorite->getLat(), favorite->getLon() );
   } else if( iSetDestination ){
      SetAsDestination( favorite->getLat(), favorite->getLon() );
   } else if (iSendToFriend) {
      SendToFriend(*favorite);
   } else if ( iShowFavorite ) {
      ShowInfoInServiceWindow(*favorite);
   } else {
      Favorite* updFavorite = ShowEditFavoriteDialogL(favorite);
      if (updFavorite) {
         ChangeFavoriteMess* message = new (ELeave) ChangeFavoriteMess(updFavorite);
         LOGNEW(message, ChangeFavoriteMess);
         iWayfinderAppUi->SendMessageL( message );
         iEditing = ETrue;
         message->deleteMembers();
         LOGDEL(message);
         delete message;
      }
   } 
}

void CMyDestView::Route()
{
   if(iContainer && !iContainer->IsListEmpty()){
      TUint favId = GetSelectedId();
      TUint favIndex = iContainer->GetSelectedIndex();

      GuiFavorite* guiFav = iFavVec->at(iContainer->GetSelectedIndex());
      if (!CEditFavoriteView::IsWayfinderFavorite(guiFav)) {
         // This is a landmark, not a wayfinder favorite
         TUint lmId  = GetSelectedLmId();
         Favorite* favorite = iWayfinderAppUi->GetFavoriteFromLmsL(lmId, guiFav->getLmsID());
         
         if(favorite) {
            HBufC* name = WFTextUtil::AllocL(iFavVec->at(favIndex)->getName());
            char* name_str = WFTextUtil::newTDesDupL(*name);
            iWayfinderAppUi->
               SetPendingRouteMessageAndSendVehicleTypeL(GuiProtEnums::PositionTypePosition,
                                                         "", favorite->getLat(), favorite->getLon(), name_str);
            iWayfinderAppUi->SetCurrentRouteEndPoints( *name,
                                                       favorite->getLat(), favorite->getLon(),
                                                       MAX_INT32, MAX_INT32);
            
            delete name;
            delete[] name_str;
            delete favorite;
         }
      }
      else {
         char *strId = WFTextUtil::uint32AsStringL(favId);
         LOGNEWA(strId, char, strlen(strId) + 1);
         iWayfinderAppUi->
            SetPendingRouteMessageAndSendVehicleTypeL(GuiProtEnums::PositionTypeFavorite,
                                                      strId, MAX_INT32, MAX_INT32, "");
         HBufC* name = WFTextUtil::AllocL(iFavVec->at(favIndex)->getName());
         iWayfinderAppUi->SetCurrentRouteEndPoints( *name,
                                                    MAX_INT32, MAX_INT32,
                                                    MAX_INT32, MAX_INT32);
         
         delete name;
         LOGDELA(strId);
         delete[] strId;
      }
   }
}

void
CMyDestView::AddFavorite(TInt32 aLat, TInt32 aLon, const TDesC& aName,
                         const TDesC& aDescription)
{
   char* newName = WFTextUtil::newTDesDupL(aName);
   char* newDesc;
   if (aDescription.CompareF(KNullDesC)) {
      newDesc = WFTextUtil::newTDesDupL(aDescription);
   } else {
      newDesc = WFTextUtil::newTDesDupL(aName);
   }
   
   // Call AddFavoriteD with a new favorite. AddFavoriteD will handle
   // all the lms stuff
   AddFavoriteD(new Favorite(aLat, aLon, newName, "", newDesc, "", "", false));
}

void 
CMyDestView::AddFavoriteD( Favorite* fav )
{
   iSaving = ETrue;

   AddFavoriteMess* message = 
      new (ELeave) AddFavoriteMess( fav );
   LOGNEW(message, AddFavoriteMess);
   iWayfinderAppUi->SendMessageL( message );
   message->deleteMembers();
   LOGDEL(message);
   delete message;
}

void CMyDestView::AddFavorite()
{
   TBuf<KBuf256Length> name(KEmpty);    // the thumb compiler will not
   TBuf<KBuf256Length> description(KEmpty); // accept the operator= notation.
   TBuf<KBuf256Length> category(KEmpty);
   TBuf<KBuf256Length> alias(KEmpty);
   TPoint currPos = iWayfinderAppUi->GetCurrentPosition();
   int32 lat = currPos.iY;
   int32 lon = currPos.iX;
   if( lat == MAX_INT32 || !iWayfinderAppUi->CanUseGPSForEveryThing())
      lat = 0;
   if( lon == MAX_INT32 || !iWayfinderAppUi->CanUseGPSForEveryThing())
      lon = 0;
   TBool canceled;
   if( iContainer->ShowDialogL( name, description, category, alias, lat, lon,
                                canceled ) ){

      // Call AddFavoriteD with a new favorite. AddFavoriteD will handle
      // all the lms stuff
      AddFavoriteD(CreateFavorite( 0, lat, lon, name, description,
                                   category, alias, "", EFalse, MAX_UINT32,
                                   NULL, NULL));

   }
   else if( !canceled ){
      WFDialog::ShowInformationL( R_WAYFINDER_FAVORITEINCOMPLETE_MSG, iCoeEnv );
   }
}

void CMyDestView::SendToFriend(const Favorite& aFavorite)
{
   iSendToFriend = EFalse;
   Favorite* fav = new Favorite(aFavorite); 
   iWayfinderAppUi->SendFavoriteL(fav);
}

void CMyDestView::EditFavorite()
{
   if(iContainer && !iContainer->IsListEmpty() ){
      TUint favId = GetSelectedId();
      GenericGuiMess* message = 
         new (ELeave) GenericGuiMess( GuiProtEnums::GET_FAVORITE_INFO, 
                                      (uint32)favId );
      LOGNEW(message, GenericGuiMess);
      iWayfinderAppUi->SendMessageL( message );
      message->deleteMembers();
      LOGDEL(message);
      delete message;
   }
}

void CMyDestView::RemoveFavorite()
{
   if(iContainer && !iContainer->IsListEmpty() ){
      
      HBufC* buf = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_DELETEDESTINATION_MSG );
      bool deleteIt = WFDialog::ShowQueryL( *buf );

      CleanupStack::PopAndDestroy(buf);

      if (deleteIt){
         GuiFavorite* guiFav = iFavVec->at(iContainer->GetSelectedIndex());
         TUint favId = GetSelectedId();
         if (CEditFavoriteView::IsWayfinderFavorite(guiFav)) {
            // This is a wayfinder favorite
            GenericGuiMess* message = 
               new (ELeave) GenericGuiMess( GuiProtEnums::REMOVE_FAVORITE,
                                            (uint32)favId );
            LOGNEW(message, GenericGuiMess);
            iWayfinderAppUi->SendMessageL( message );
            iRemoving = ETrue;
            message->deleteMembers();
            LOGDEL(message);
            delete message;
            // We remove the item from the list the confirmation dialog is
            // shown when the reply is received from nav2.
            RemoveItemL(iContainer->GetSelectedIndex());
         }
         else {
            iWayfinderAppUi->DeleteFavoriteInLmsL(GetSelectedLmId(), 
                                                  guiFav->getLmsID());
            // We remove the item from the list and show a confirmation dialog.
            RemoveItemL(iContainer->GetSelectedIndex());
            WFDialog::ShowConfirmationL(R_WAYFINDER_FAVORITEREMOVED_MSG, iCoeEnv);
         }
      } else{
         //Do not delete, the user changed his mind.
      }
   }
}

void CMyDestView::RemoveItemL(TInt aIndex)
{
   if (aIndex >= 0 && TUint(aIndex) < iFavVec->size()) {
      iContainer->RemoveItemL(aIndex);
      GuiFavorite* fav = iFavVec->at(aIndex);
      iFavVec->erase(iFavVec->begin() + aIndex);
      delete fav;
      iContainer->UpdatePreviewContentL();
      if (iFavVec->size() == 0) {
         iContainer->ShowPreviewContent(EFalse);
      }
   }
}

void CMyDestView::SetStatesFalse()
{
   iSaving = EFalse;
   iEditing = EFalse;
   iRemoving = EFalse;
}

void CMyDestView::SetAsOrigin( TInt32 aLat, TInt32 aLon )
{
   iSetOrigin = EFalse;
   TBuf<KBuf256Length> name;
   if( iContainer->GetSelItem( name ) ){
      char* id = NULL;
      TUint favid = GetSelectedId();
      if (favid != MAX_UINT32) {
         id = WFTextUtil::uint32AsStringL(favid);
      }
//       LOGNEWA(id, char, strlen(id) + 1);
//       if(!id) User::LeaveNoMemory();
//       LOGNEWA(id, char, strlen(id) + 1);
      iWayfinderAppUi->SetOrigin( GuiProtEnums::PositionTypePosition, name, id, 
                                  aLat, aLon );
      LOGDELA(id);
      delete id;
   }
   iContainer->SetFindBoxFocus(EFalse);
   iWayfinderAppUi->push( KPositionSelectViewId );
}

void CMyDestView::SetAsDestination( TInt32 aLat, TInt32 aLon )
{
   iSetDestination = EFalse;
   TBuf<KBuf256Length> name;
   if( iContainer->GetSelItem( name ) ){
      char* id = NULL;
      TUint favid = GetSelectedId();
      if (favid != MAX_UINT32) {
         id = WFTextUtil::uint32AsStringL(favid);
      }
//       LOGNEWA(id, char, 16);
//       if(!id) User::LeaveNoMemory();
//       LOGNEWA(id, char, strlen(id) + 1);
      iWayfinderAppUi->SetDestination(GuiProtEnums::PositionTypeFavorite, name, 
                                      id, aLat, aLon);
      LOGDELA(id);
      delete id;
   }
   iContainer->SetFindBoxFocus(EFalse);
   iWayfinderAppUi->push( KPositionSelectViewId );
}

void CMyDestView::ShowInfoInServiceWindow(const Favorite& aFavorite)
{
   iContainer->SetFindBoxFocus(EFalse);
   iShowFavorite = EFalse;
   Nav2Coordinate nav2Coord(aFavorite.getLat(), aFavorite.getLon());
   MC2Coordinate mc2Coord(nav2Coord);
   iWayfinderAppUi->GotoInfoInServiceViewL(mc2Coord.lat, mc2Coord.lon);
}

void CMyDestView::ShowInfoInMap(const Favorite& aFavorite)
{
   iContainer->SetFindBoxFocus(EFalse);
   iShowInMap = EFalse;
   iWayfinderAppUi->RequestMap(MapEnums::FavoritePosition, aFavorite.getLat(), 
                               aFavorite.getLon(), EShowOnMap);
}

TBool CMyDestView::SettingOrigin()
{
   //return ( iPrevViewId.iViewUid == KPositionSelectViewId ) && iIssOrigin;
   return (iCustomMessageId == ERoutePlannerSetAsOrigin);
}

TBool CMyDestView::SettingDestination()
{
   //return ( iPrevViewId.iViewUid == KPositionSelectViewId ) && !iIssOrigin;
   return (iCustomMessageId == ERoutePlannerSetAsDest);
}

TBool CMyDestView::IsEditing()
{
   return iEditing;
}

TBool CMyDestView::IsAdding()
{
   return iSaving;
}

void CMyDestView::ShowConfirmationDialogL()
{
   // For now only used for s60v3 when saving favorite, since editing 
   // confirmation dialog is taken care of by HandleFavoritesReply.
   TInt mess = -1;
   if( iSaving ){
      iSaving = EFalse;
      mess = R_WAYFINDER_FAVORITESAVED_MSG;
   } else if( iEditing ){
      mess = R_WAYFINDER_FAVORITECHANGED_MSG;
      iEditing = EFalse;
   } else if( iRemoving ){
      mess = R_WAYFINDER_FAVORITEREMOVED_MSG;
      iRemoving = EFalse;
   }
   if (mess > 0) {
      WFDialog::ShowConfirmationL( mess, iCoeEnv);
   }
}

TBool CMyDestView::IsGpsAllowed()
{
   return iWayfinderAppUi->IsGpsAllowed();
}

TBool CMyDestView::ValidGpsStrength()
{
   return iWayfinderAppUi->ValidGpsStrength();
}

TPoint CMyDestView::GetCurrentPosition()
{
   return iWayfinderAppUi->GetCurrentPosition();
}

TInt CMyDestView::GetDistanceMode()
{
   DistancePrintingPolicy::DistanceMode mode = 
      DistancePrintingPolicy::DistanceMode(iWayfinderAppUi->GetDistanceMode());
   switch (mode) {
   case DistancePrintingPolicy::ModeImperialYards:
      return DistancePrintingPolicy::ModeImperialYardsSpace;
      break;
   case DistancePrintingPolicy::ModeImperialFeet:
      return DistancePrintingPolicy::ModeImperialFeetSpace;
      break;
   case DistancePrintingPolicy::ModeInvalid:
   case DistancePrintingPolicy::ModeMetric:
   default:
      return DistancePrintingPolicy::ModeMetricSpace;
      break;
   }
}

Favorite* CMyDestView::CreateFavorite( TUint aId, TInt aLat, TInt aLon,
                                       TDesC &aName, TDesC &aDescription,
                                       TDesC &aCategory, TDesC &aAlias,
                                       const char* aIcon, TBool aHasId, uint32 aLmID, 
                                       const char* aImeiCrcHex, const char* aLmsID )
{
   Favorite* newFavorite;
   char* newName = WFTextUtil::newTDesDupL(aName);
   char* newDesc = WFTextUtil::newTDesDupL(aDescription);
   char* newCategory = WFTextUtil::newTDesDupL(aCategory);
   char* newAlias = WFTextUtil::newTDesDupL(aAlias);

   if( aHasId ){
      newFavorite = new (ELeave) Favorite( aId, aLat, aLon, newName, newAlias,
                                           newDesc, newCategory, aIcon, false,
                                           aLmID, aImeiCrcHex, aLmsID);
      LOGNEW(newFavorite, Favorite);
   }
   else{
      newFavorite = new (ELeave) Favorite( aLat, aLon, newName, newAlias, 
                                           newDesc, newCategory, aIcon, false,
                                           aLmID, aImeiCrcHex, aLmsID);
      LOGNEW(newFavorite, Favorite);
   }
   LOGDELA(newAlias);
   delete[] newAlias;
   LOGDELA(newCategory);
   delete[] newCategory;
   LOGDELA(newDesc);
   delete[] newDesc;
   LOGDELA(newName);
   delete[] newName;

   return newFavorite;
}

// ---------------------------------------------------------
// TUid CMyDestView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CMyDestView::Id() const
{
   return KMyDestViewId;
}

// ---------------------------------------------------------
// CMyDestView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CMyDestView::HandleCommandL(TInt aCommand)
{
   switch ( aCommand )
   {
   case EWayFinderCmdMyDestNavigate:
      {
         //Route(); //Used to route before going between route planner.
        if (iWayfinderAppUi->
            CheckAndDoGpsConnectionL((void*) this, 
                                     CMyDestView::WrapperToNavigateToCallback)) {
          // We're connected or connecting to a gps so go ahead.
          iWayfinderAppUi->SetFromGps();
          // Used to go to the Plan route view
		  // HandleCommandL(EWayFinderCmdMyDestDestination);
          Route();
        }
        break;
      }
   case EWayFinderCmdMyDestSync:
      {
         if (!iWayfinderAppUi->IsFavLmsSyncRunning() && 
             !iWayfinderAppUi->IsLmsManagerOpRunning()) {
            iWayfinderAppUi->RequestFavoritesL( ETrue );
         }
         break;
      }
   case EWayFinderCmdMyDestAdd:
      {
         AddFavorite();
         break;
      }
   case EWayFinderCmdMyDestEdit:
      {
         UpdateFavoriteL();
         break;
      }
   case EWayFinderCmdMyDestRemove:
      {
         RemoveFavorite();
         break;
      }
   case EWayFinderCmdMyDestShow:
      {
         iShowInMap = ETrue;
         HandleFavoriteOperationsL();
         break;
      }
   case EWayFinderCmdMyDestViewDetails:
      {
         iShowFavorite = ETrue;
         HandleFavoriteOperationsL();
         break;
      }
   case EWayFinderCmdMyDestOrigin:
      {
         iSetOrigin = ETrue;
         HandleFavoriteOperationsL();
         break;
      }
   case EWayFinderCmdMyDestDestination:
      {
         iSetDestination = ETrue;
         HandleFavoriteOperationsL();
         break;
      }
   case EWayFinderCmdMyDestSend:
      {
         iSendToFriend = ETrue;
         HandleFavoriteOperationsL();
         break;
      }
   case EWayFinderCmdHelp:
      {
         if (iContainer) {
            iContainer->SetFindBoxFocus(EFalse);
         }
         iWayfinderAppUi->ShowHelpL();
      }
      break;
   default:
      {
         AppUi()->HandleCommandL( aCommand );
         break;
      }
   }
}

void CMyDestView::OfferToolbarEventL(TInt aCommmandId)
{
   switch (aCommmandId) {
   case EWayFinderCmdMyDestEditButton:
      HandleCommandL(EWayFinderCmdMyDestEdit);
      break;
   case EWayFinderCmdMyDestRemoveButton:
      HandleCommandL(EWayFinderCmdMyDestRemove);
      break;
   case EWayFinderCmdMyDestViewDetailsButton:
      HandleCommandL(EWayFinderCmdMyDestViewDetails);
      break;
   }
}

// ---------------------------------------------------------
// CMyDestView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CMyDestView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

void CMyDestView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_MYDEST_MENU ){
      if ( iContainer->IsListEmpty() ){
         // No items stored, remove all menu options. 
         aMenuPane->SetItemDimmed( EWayFinderCmdMyDestNavigate, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdMyDestShow, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdMyDestOrigin, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdMyDestDestination, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdMyDestSend, ETrue);
      } else {
         // We have items.
         if (iCustomMessageId == ERoutePlannerSetAsOrigin ||
             iCustomMessageId == ERoutePlannerSetAsDest) {
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestNavigate, ETrue);
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestShow, EFalse);
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestViewDetails, EFalse);
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestSend, ETrue);
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestManage, ETrue);
            if (iCustomMessageId == ERoutePlannerSetAsOrigin) {
               aMenuPane->SetItemDimmed( EWayFinderCmdMyDestOrigin, EFalse);
               aMenuPane->SetItemDimmed( EWayFinderCmdMyDestDestination, ETrue);
            }
            if (iCustomMessageId == ERoutePlannerSetAsDest) {
               aMenuPane->SetItemDimmed( EWayFinderCmdMyDestOrigin, ETrue);
               aMenuPane->SetItemDimmed( EWayFinderCmdMyDestDestination, EFalse);
            }
         } else {
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestOrigin, ETrue);
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestDestination, ETrue);

            if(iWayfinderAppUi->CanUseGPSForEveryThing() && 
               !iWayfinderAppUi->IsIronVersion()) {
               // Not iron and the user has the correct rights
               aMenuPane->SetItemDimmed(EWayFinderCmdMyDestNavigate, EFalse);               
            } else {
               // Either iron or simply not the correct rights
               aMenuPane->SetItemDimmed(EWayFinderCmdMyDestNavigate, ETrue);               
            }

            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestShow, EFalse);
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestViewDetails, EFalse);
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestSend, EFalse);
            aMenuPane->SetItemDimmed( EWayFinderCmdMyDestManage, EFalse);
         }
      }
   } else if (aResourceId==R_WF_MANAGE_FAVORITE_MENU) {
      if ( iContainer->IsListEmpty() ){
         aMenuPane->SetItemDimmed( EWayFinderCmdMyDestRemove, ETrue);
         aMenuPane->SetItemDimmed( EWayFinderCmdMyDestEdit, ETrue);
      }
   }

   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

void CMyDestView::SetFindBoxFocus(TBool aFocus)
{
   if (iContainer) {
      iContainer->SetFindBoxFocus(aFocus);
   }
}

// ---------------------------------------------------------
// CMyDestView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CMyDestView::DoActivateL( const TVwsViewId& aPrevViewId,
                               TUid aCustomMessageId,
                               const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_FAVOURITES_TEXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   
   // Explicitly hide the arrows in navipane since on s60v5 they tend to 
   // show up in some views otherwise.
   iWayfinderAppUi->setNaviPane(EFalse);

   // The toolbar commented out for now to get more space for items in list.
   //InitToolbarL();
   
   if ( !iContainer ){
      iContainer = new (ELeave) CMyDestContainer(iLog);
      LOGNEW(iContainer, CMyDestContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect(), this );
      AppUi()->AddToStackL( *this, iContainer );

      if (!iWayfinderAppUi->IsFavLmsSyncRunning() && 
          !iWayfinderAppUi->IsLmsManagerOpRunning()) {
         // We're not actively syncing right now (its done) so it is safe 
         // to request all favorites and landmarks for viewing in the list.
         iWayfinderAppUi->RequestFavoritesL( EFalse );
         iWayfinderAppUi->GetGuiLandmarksAsyncL(MWFLMSManagerBase::EWFLMSLMFavs);
      }
   }

   if (iCustomMessageId == ERoutePlannerSetAsOrigin ||
       iCustomMessageId == ERoutePlannerSetAsDest) {
   }
}

// ---------------------------------------------------------
// CMyDestView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CMyDestView::DoDeactivate()
{
   if ( iContainer ){
      iContainer->SetFindBoxFocus(EFalse);
      iListIndex = iContainer->GetSelectedIndex();
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

TBool CMyDestView::IsIronVersion() 
{
   return iWayfinderAppUi->IsIronVersion();
}

TPtrC CMyDestView::GetMbmName()
{
   return iWayfinderAppUi->iPathManager->GetMbmName();
}

void CMyDestView::HandleFavoriteOperationsL()
{
   if( iContainer && !iContainer->IsListEmpty() ){
      TUint lmId  = GetSelectedLmId();
      GuiFavorite* guiFav = iFavVec->at(iContainer->GetSelectedIndex());
      if (!CEditFavoriteView::IsWayfinderFavorite(guiFav)) {
         // This is a landmark, not a wayfinder favorite
         Favorite* favorite = iWayfinderAppUi->GetFavoriteFromLmsL(lmId, guiFav->getLmsID());
         if (iSetOrigin) {
            SetAsOrigin(favorite->getLat(), favorite->getLon());
         } else if (iSetDestination) {
            SetAsDestination(favorite->getLat(), favorite->getLon());            
         } else if (iShowFavorite) {
            ShowInfoInServiceWindow(*favorite);
         } else if (iShowInMap) {
            ShowInfoInMap(*favorite);
         } else if (iSendToFriend) {
            SendToFriend(*favorite);
         }
         delete favorite;
      } else {
         // This is a wayfinder favorite, call EditFavorite that will request
         // information about the favorite. 
         EditFavorite();         
      }
   }
}

void CMyDestView::UpdateFavoriteL()
{
   if( iContainer && !iContainer->IsListEmpty() ){
      TUint lmId  = GetSelectedLmId();
      GuiFavorite* guiFav = iFavVec->at(iContainer->GetSelectedIndex());
      if (!CEditFavoriteView::IsWayfinderFavorite(guiFav)) {
         // This is a landmark, not a wayfinder favorite
         Favorite* favorite = iWayfinderAppUi->GetFavoriteFromLmsL(lmId, guiFav->getLmsID());
         Favorite* updFavorite = ShowEditFavoriteDialogL(favorite);
         delete favorite;
         if (updFavorite) {
            // Update the favorite to the lms, since it isnt a wayfinder favorite
            // the change is not reported to Nav2
            iWayfinderAppUi->UpdateFavoriteInLmsL(*updFavorite);
            iWayfinderAppUi->RequestFavoritesL( EFalse );
            iWayfinderAppUi->GetGuiLandmarksAsyncL(MWFLMSManagerBase::EWFLMSLMFavs);
            WFDialog::ShowConfirmationL(R_WAYFINDER_FAVORITECHANGED_MSG, iCoeEnv);
         }
         delete updFavorite;
      } else {
         // This is a wayfinder favorite, call EditFavorite that will request
         // information about the favorite. 
         EditFavorite();         
      }
   }
}

Favorite* CMyDestView::ShowEditFavoriteDialogL(Favorite* aFav)
{
   Favorite* fav = NULL;
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iWayfinderAppUi->LaunchEditFavoriteL(aFav, EFalse);
#else
   TBuf<KBuf256Length> name;
   WFTextUtil::char2TDes(name, aFav->getName());
         
   TBuf<KBuf256Length> description;
   WFTextUtil::char2TDes(description, aFav->getDescription());
         
   TBuf<KBuf256Length> category;
   WFTextUtil::char2TDes(category, aFav->getCategory());
         
   TBuf<KBuf256Length> alias;
   WFTextUtil::char2TDes(alias, aFav->getShortName());
         
   int32 lat = aFav->getLat();
   int32 lon = aFav->getLon();

   TBool canceled;
   if( iContainer->ShowDialogL( name, description, category, 
                                alias, lat, lon, canceled ) ) {
      char* lmsId = strdup_new(aFav->getLmsID());
      fav = CreateFavorite(aFav->getID(), lat, lon,
                           name, description, category, alias,
                           aFav->getMapIconName(), ETrue,
                           aFav->getLmID(), NULL, lmsId);
      delete[] lmsId;


   } else {
      // User cancelled so don't show anything.
      //WFDialog::ShowInformationL( R_WAYFINDER_FAVORITECHANGED_MSG, iCoeEnv );
   }
#endif
   return fav;
}

TUint CMyDestView::GetSelectedId()
{
   if (iContainer) {
      iListIndex = iContainer->GetSelectedIndex();
   }
   if (iListIndex >= 0) {
      return iFavVec->at(iListIndex)->getID();
   } else {
      return MAX_UINT32;
   }
}

TUint CMyDestView::GetSelectedLmId()
{
   if (iContainer) {
      iListIndex = iContainer->GetSelectedIndex();
   }
   if (iListIndex >= 0) {
      return iFavVec->at(iListIndex)->getLmID();
   } else {
      return MAX_UINT32;
   }
}

GuiFavorite* CMyDestView::GetSelectedGuiFav()
{
   if (iContainer) {
      iListIndex = iContainer->GetSelectedIndex();
   }
   if (iListIndex >= 0) {
      return iFavVec->at(iListIndex);
   } else {
      return NULL;
   }
}

void CMyDestView::NavigateToCallback()
{
   iWayfinderAppUi->SetFromGps();
   // Used to go to the Plan route view
   // HandleCommandL(EWayFinderCmdMyDestDestination);
   Route();
}

void CMyDestView::WrapperToNavigateToCallback(void* pt2Object)
{
   CMyDestView* self = (CMyDestView*) pt2Object;
   self->NavigateToCallback();
}

void CMyDestView::InitToolbarL()
{
#if defined NAV2_CLIENT_SERIES60_V5
   if (Toolbar()) {
      CAknButton* button = static_cast<CAknButton*>
         (Toolbar()->ControlOrNull(EWayFinderCmdMyDestViewDetailsButton));
      if (!button) {
         HBufC* helpText = CEikonEnv::Static()->AllocReadResourceLC(R_MYDEST_SHOW_INFO);
         button = CAknButton::NewL(iWayfinderAppUi->iPathManager->GetMbmName(),
                                   EMbmWficonsPage_navigation_info, EMbmWficonsPage_navigation_info_mask,
                                   EMbmWficonsPage_navigation_info, EMbmWficonsPage_navigation_info_mask,
                                   EMbmWficonsPage_navigation_info, EMbmWficonsPage_navigation_info_mask,
                                   EMbmWficonsPage_navigation_info, EMbmWficonsPage_navigation_info_mask,
                                   KNullDesC, *helpText, 0, 0);
         Toolbar()->AddItemL(button, EAknCtButton, EWayFinderCmdMyDestViewDetailsButton, 0, 0);
         CleanupStack::PopAndDestroy(helpText);         
      }

      button = static_cast<CAknButton*>(Toolbar()->ControlOrNull(EWayFinderCmdMyDestEditButton));
      if (!button) {
         HBufC* helpText = CEikonEnv::Static()->AllocReadResourceLC(R_MYDEST_EDIT_ITEM);
         button = CAknButton::NewL(iWayfinderAppUi->iPathManager->GetMbmName(),
                                   EMbmWficonsEdit_pencil, EMbmWficonsEdit_pencil_mask,
                                   EMbmWficonsEdit_pencil, EMbmWficonsEdit_pencil_mask,
                                   EMbmWficonsEdit_pencil, EMbmWficonsEdit_pencil_mask,
                                   EMbmWficonsEdit_pencil, EMbmWficonsEdit_pencil_mask,
                                   KNullDesC, *helpText, 0, 0);
         Toolbar()->AddItemL(button, EAknCtButton, EWayFinderCmdMyDestEditButton, 0, 1);
         CleanupStack::PopAndDestroy(helpText);
      }

      button = static_cast<CAknButton*>
         (Toolbar()->ControlOrNull(EWayFinderCmdMyDestRemoveButton));
      if (!button) {
         HBufC* helpText = CEikonEnv::Static()->AllocReadResourceLC(R_MYDEST_REMOVE_ITEM);
         button = CAknButton::NewL(iWayfinderAppUi->iPathManager->GetMbmName(),
                                   EMbmWficonsDelete_trashcan, EMbmWficonsDelete_trashcan_mask,
                                   EMbmWficonsDelete_trashcan, EMbmWficonsDelete_trashcan_mask,
                                   EMbmWficonsDelete_trashcan, EMbmWficonsDelete_trashcan_mask,
                                   EMbmWficonsDelete_trashcan, EMbmWficonsDelete_trashcan_mask,
                                   KNullDesC, *helpText, 0, 0);
         Toolbar()->AddItemL(button, EAknCtButton, EWayFinderCmdMyDestRemoveButton, 0, 2);
         CleanupStack::PopAndDestroy(helpText);         
      }

   }
#endif
}

// End of File
