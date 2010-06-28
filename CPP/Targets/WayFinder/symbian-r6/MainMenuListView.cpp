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

#include <arch.h>
#include <stringloader.h>
#include "MainMenuListView.h"
#include "WayFinderSettings.h"
#include "WayFinderConstants.h"
#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "WayFinderAppUi.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"
#include "MainMenuListContainer.h"

const TInt MbmImageIds [] = {
   EMbmWficonsSearch,
   EMbmWficonsFavorites,
   EMbmWficonsMap
};

const TInt MbmMaskIds [] = {
   EMbmWficonsSearch_mask,
   EMbmWficonsFavorites_mask,
   EMbmWficonsMap_mask
};

//This int array has to end with -1 so the container knows how 
//many items we should have in the grid.
const TInt MainMenuCommandIds [] = {
   EWayFinderCmdCSMainView,
   EWayFinderCmdMyDest,
   EWayFinderCmdMap,
   -1
};

const TInt MainMenuFirstLabelIds [] = {
   R_MM_SEARCH,
   R_MM_FAVORITES,
   R_MM_MAP
};

const TInt MainMenuSecondLabelIds [] = {
   R_MM_SEARCH_SUBTITLE,
   R_MM_FAVORITES_SUBTITLE,
   R_MM_MAP_SUBTITLE
};

CMainMenuListView* CMainMenuListView::NewL(CWayFinderAppUi* aWayFinderAppUi)
{
   CMainMenuListView* self = CMainMenuListView::NewLC(aWayFinderAppUi);
   CleanupStack::Pop(self);
   return self;
}

CMainMenuListView* CMainMenuListView::NewLC(CWayFinderAppUi* aWayFinderAppUi)
{
   CMainMenuListView* self = new (ELeave) CMainMenuListView(aWayFinderAppUi);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CMainMenuListView::CMainMenuListView(CWayFinderAppUi* aWayFinderAppUi) :
   CViewBase(aWayFinderAppUi), iSelectedItem(0)
{
}

void CMainMenuListView::ConstructL()
{
   BaseConstructL(R_WAYFINDER_MAIN_MENU_LIST_VIEW);
}

CMainMenuListView::~CMainMenuListView()
{
   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }
   delete iContainer;
}

TPtrC CMainMenuListView::GetMbmName()
{
   return iWayfinderAppUi->iPathManager->GetMbmName();
}

void CMainMenuListView::SetNaviPaneLabelL(TInt aLabelId)
{
   iWayfinderAppUi->SetMainMenuNaviPaneLabelL(MainMenuFirstLabelIds[aLabelId]);
}

void CMainMenuListView::ResetNaviPaneLabelL()
{
   SetNaviPaneLabelL(iSelectedItem);
}

// void CMainMenuListView::HandleCommandL(const class CWAXParameterContainer& aCont)
// {
//    iWayfinderAppUi->ClearBrowserCache();
//    iWayfinderAppUi->GotoStartViewL();
// }

TUid CMainMenuListView::Id() const
{
   return KMainMenuViewId;
}

void CMainMenuListView::HandleCommandL( TInt aCommand )
{
   switch (aCommand) {
   case EWayFinderCmdStartPageOpen:
      {
         iContainer->ActivateSelection();
         break;
      }
   case EWayFinderPublishMyPosition:
      {
         iWayfinderAppUi->SendTrackingLevel(isab::GuiProtEnums::tracking_level_live);
         break;
      }
   case EWayFinderStopPublishMyPosition:
      {
         iWayfinderAppUi->SendTrackingLevel(0);
         break;
      }
   default:
      {
         iSelectedItem = iContainer->GetActiveSelection();
         AppUi()->HandleCommandL(aCommand);
         break;
      }
   }
}

void CMainMenuListView::HandleClientRectChange()
{
   if (iContainer) {
      iContainer->SetRect(ClientRect());
   }
}

void CMainMenuListView::DynInitMenuPaneL(TInt aResourceId, 
                                         CEikMenuPane* aMenuPane)
{
   AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}

void CMainMenuListView::DoActivateL(const TVwsViewId& aPrevViewId, 
                                    TUid aCustomMessageId,
                                    const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
   iWayfinderAppUi->ActivateMainMenuNaviPaneLabelL();

   HBufC* titleText = StringLoader::LoadLC(R_TITLEPANE_NAVIGATION_TEXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   
   if(!iContainer) { 
      iContainer = 
         CMainMenuListContainer::NewL(ClientRect(), this, 
                                      MbmImageIds, MbmMaskIds, 
                                      MainMenuCommandIds,
                                      MainMenuFirstLabelIds,
                                      MainMenuSecondLabelIds);
      AppUi()->AddToStackL(*this, iContainer);
   }
   iContainer->SetActiveSelection(iSelectedItem);
   SetNaviPaneLabelL(iContainer->GetActiveSelection());

   // check for new versions of software
   iWayfinderAppUi->GenerateEvent(CWayFinderAppUi::EWayfinderEventCheckAndShowUpgradeDialog);
}

void CMainMenuListView::DoDeactivate()
{
   iWayfinderAppUi->DeactivateMainMenuNaviPaneLabelL();

   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }   
   delete iContainer;
   iContainer = NULL;
}

void CMainMenuListView::UpdateActiveSelection()
{
   //iSelectedItem = aSelectedItem;
   if(iContainer) { 
      iContainer->SetActiveSelection(iSelectedItem);
      SetNaviPaneLabelL(iSelectedItem);
   }
}

void CMainMenuListView::UpdateSelectedIndex(TInt aIndex)
{ 
   iSelectedItem = aIndex;
}

TInt CMainMenuListView::GetSelectedIndex()
{
   return iSelectedItem;
}
