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
#include <arch.h>

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "WayFinderAppUi.h"
#include "WFTextUtil.h"
#include "Dialogs.h"
#include "GuiProt/Favorite.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "ViewBase.h"

#include "EditFavoriteView.h"
#include "EditFavoriteContainer.h"

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32 
// s60v3fp2 and s60v5 only class

CEditFavoriteView::CEditFavoriteView(CWayFinderAppUi* aAppUi) : 
   CViewBase(aAppUi),
   iFav(NULL),
   iWaitingForFavoriteReply(EFalse)
{
}

CEditFavoriteView::~CEditFavoriteView()
{
   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }
   delete iContainer;
   delete iFav;	
}

CEditFavoriteView* CEditFavoriteView::NewL(CWayFinderAppUi* aAppUi)
{
   CEditFavoriteView* self = CEditFavoriteView::NewLC(aAppUi);
   CleanupStack::Pop(self);
   return self;
}

CEditFavoriteView* CEditFavoriteView::NewLC(CWayFinderAppUi* aAppUi)
{
   CEditFavoriteView* self = new (ELeave) CEditFavoriteView(aAppUi);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

void CEditFavoriteView::ConstructL()
{
   BaseConstructL(R_WAYFINDER_EDIT_FAVORITE_VIEW);
}

TUid CEditFavoriteView::Id() const
{
   return TUid::Uid(EWayFinderEditFavoriteView);
}

void CEditFavoriteView::HandleCommandL(TInt aCommand)
{
   switch (aCommand) {
   case EWayFinderSoftkeyFavoriteSave:
      {
         iContainer->SaveFavoriteDataL();
         if (iCustomMessageId == EAddFavorite) {
            AddFavoriteL();
         } else if (iCustomMessageId == EEditFavorite) {
            UpdateFavoriteL();
         }
         iWayfinderAppUi->HandleCommandL(EAknSoftkeyBack);
      }
      break;
   case EAknSoftkeyBack:
      {
         if (iFav) {
            delete iFav;
            iFav = NULL;
         }
         iWayfinderAppUi->HandleCommandL(aCommand);
      }
      break;
   default:
      iWayfinderAppUi->HandleCommandL(aCommand);
      break;
   }
}

void CEditFavoriteView::DoActivateL(const TVwsViewId& aPrevViewId,
                                    TUid aCustomMessageId,
                                    const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

//    CEikButtonGroupContainer* cba = AppUi()->Cba();
//    if (cba != NULL) {
//       cba->MakeVisible(EFalse);
//    }

   HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_SAVE_AS_FAVOURITE_TEXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   if (iContainer == NULL) {
      CEditFavoriteContainer* form = 
         CEditFavoriteContainer::NewL(this, *this);
      form->SetMopParent(this); 
      form->ExecuteLD(R_WAYFINDER_EDIT_FAVORITE_FORM);
      AppUi()->AddToStackL(*this, form);
      iContainer = form;
   }
   if (iFav) {
      iContainer->SetFavoriteDataL(*iFav);
   }
}

void CEditFavoriteView::DoDeactivate()
{
//    CEikButtonGroupContainer* cba = AppUi()->Cba();
//    if (cba != NULL) {
//       cba->MakeVisible(ETrue);
//       cba->DrawDeferred();
//    }

   if (iContainer != NULL) {
      AppUi()->RemoveFromStack(iContainer);
      delete iContainer;
      iContainer = NULL;
   }
//    if (iFav) {
//       delete iFav;
//       iFav = NULL;
//    }
}

void CEditFavoriteView::SetFavoriteDataL(isab::Favorite* aFav)
{
   if (iContainer) {
      iContainer->SetFavoriteDataL(*aFav);
      delete iFav;
      iFav = new isab::Favorite(*aFav);
   } else {
      delete iFav;
      iFav = new isab::Favorite(*aFav);
   }
}

void CEditFavoriteView::UpdateFavoriteL()
{
   if (iFav) {
      // The edited favorite was saved by the edit favorite view / form.
      if (IsWayfinderFavorite(iFav)) {
         // This is a wayfinder favorite.
         isab::Favorite* updFavorite = 
            new Favorite(iFav->getID(), iFav->getLat(), iFav->getLon(),
                         iFav->getName(), iFav->getShortName(),
                         iFav->getDescription(), iFav->getCategory(), 
                         iFav->getMapIconName());


         iWaitingForFavoriteReply = ETrue;
         ChangeFavoriteMess* message = 
            new (ELeave) ChangeFavoriteMess(updFavorite);
         iWayfinderAppUi->SendMessageL(message);
         message->deleteMembers();
         delete message;
      } else {
         // This is a landmark favorite so don't send info about it to Nav2.
         isab::Favorite* updFavorite = new Favorite(*iFav);
         iWayfinderAppUi->UpdateFavoriteInLmsL(*updFavorite);
         delete updFavorite;
         // Maybe we dont want a confirmation when only editing LMS favorites, 
         // in that case, remove the line below.
         ShowConfirmationDialogL();
      }
   }
}

void CEditFavoriteView::AddFavoriteL()
{
   if (iFav) {
      isab::Favorite* addFavorite = new Favorite(*iFav);
      iWaitingForFavoriteReply = ETrue;
      AddFavoriteMess* message = new (ELeave) AddFavoriteMess(addFavorite);
      iWayfinderAppUi->SendMessageL(message);
      message->deleteMembers();
      delete message;
   }
}

TBool CEditFavoriteView::WaitingForFavoriteReply()
{
   return iWaitingForFavoriteReply;
}

void CEditFavoriteView::FavoriteReplyGot()
{
   iWaitingForFavoriteReply = EFalse;
}

void CEditFavoriteView::ShowConfirmationDialogL()
{
   if (iCustomMessageId == EAddFavorite) {
      // Show confirmation dialog when adding a new favorite is done.
      WFDialog::ShowConfirmationL(R_WAYFINDER_FAVORITESAVED_MSG, iCoeEnv);
   } else if (iCustomMessageId == EEditFavorite) {
      // Show confirmation dialog when saving an updated favorite is done.
      WFDialog::ShowConfirmationL(R_WAYFINDER_FAVORITECHANGED_MSG, iCoeEnv);
   }   
}

#endif // NAV2_CLIENT_SERIES60_V5 || NAV2_CLIENT_SERIES60_V32

/**
 * We just leave the static functions since they are needed for s60v3 as well.
 */

TBool CEditFavoriteView::IsWayfinderFavorite(isab::GuiFavorite* aFav)
{
   TUint32 favId = aFav->getID();
   if ((favId != Favorite::INVALID_FAV_ID && 
       (aFav->getLmsID() && strcmp(aFav->getLmsID(), "") == 0)) ||
       (favId >= Favorite::LOWEST_LOCAL_CLIENT_FAV_ID && 
        favId < Favorite::INVALID_FAV_ID)) {
      // This favorite has a server id and an emtpy lms id _OR_ 
      // it has a local client favorite id (meaning it is a favorite that 
      // has just been edited or added in client but not synced with server).
      // So this is a Wayfinder favorite and not a phone landmark.
      return ETrue;
   }
   return EFalse;
}

TBool CEditFavoriteView::IsWayfinderFavorite(isab::Favorite* aFav)
{
   TUint32 favId = aFav->getID();
   if ((favId != Favorite::INVALID_FAV_ID && 
       (aFav->getLmsID() && strcmp(aFav->getLmsID(), "") == 0)) ||
       (favId >= Favorite::LOWEST_LOCAL_CLIENT_FAV_ID && 
        favId < Favorite::INVALID_FAV_ID)) {
      // This favorite has a server id and an emtpy lms id _OR_ 
      // it has a local client favorite id (meaning it is a favorite that 
      // has just been edited or added in client but not synced with server).
      // So this is a Wayfinder favorite and not a phone landmark.
      return ETrue;
   }
   return EFalse;
}
