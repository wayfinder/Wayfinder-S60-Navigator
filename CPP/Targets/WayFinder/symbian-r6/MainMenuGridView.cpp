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

#include "WayFinderSettings.h"
#include "WayFinderConstants.h"
#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"

#include "WayFinderAppUi.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"

#include "MainMenuGridView.h"
#include "MainMenuGridContainer.h"

#if defined FULL_GRID_MENU
const TInt MbmImageIds [] = {
   EMbmWficonsRoute_planner, 
   EMbmWficonsSearch,
   EMbmWficonsMap,
   EMbmWficonsGuide_view,
   EMbmWficonsServices,
   EMbmWficonsFavorites,
   EMbmWficonsSettings,
   EMbmWficonsGps_info,
   EMbmWficonsExit
};
const TInt MbmMaskIds [] = {
   EMbmWficonsRoute_planner_mask,
   EMbmWficonsSearch_mask,
   EMbmWficonsMap_mask,
   EMbmWficonsGuide_view_mask,
   EMbmWficonsServices_mask,
   EMbmWficonsFavorites_mask,
   EMbmWficonsSettings_mask,
   EMbmWficonsGps_info_mask,
   EMbmWficonsExit_mask
};
//This int array has to end with -1 so the container knows how 
//many items we should have in the grid.
const TInt MainMenuCommandIds [] = {
   EWayFinderCmdPositionSelect,
   EWayFinderCmdCSMainView,
   EWayFinderCmdMap,
   EWayFinderCmdGuide,
   EWayFinderCmdService,
   EWayFinderCmdMyDest,
   EWayFinderCmdSettings,
   EWayFinderCmdInfoViewGPSConnect, 
   EAknCmdExit,
   -1
};
const TInt MainMenuLabelIds [] = {
   R_MM_ROUTE,
   R_MM_SEARCH,
   R_MM_MAP,
   R_MM_GUIDE,
   R_MM_SERVICES,
   R_MM_FAVORITES,
   R_MM_SETTINGS,
   R_MM_INFO,
   R_MM_EXIT
};
#elif defined EARTH_GRID_MENU
const TInt MbmImageIds [] = {
   EMbmWficonsRoute_planner, 
   EMbmWficonsSearch,
   EMbmWficonsFavorites,
   EMbmWficonsMap,
   EMbmWficonsWeather,
   EMbmWficonsCityguide
};
const TInt MbmMaskIds [] = {
   EMbmWficonsRoute_planner_mask,
   EMbmWficonsSearch_mask,
   EMbmWficonsFavorites_mask,
   EMbmWficonsMap_mask,
   EMbmWficonsWeather_mask,
   EMbmWficonsCityguide_mask
};
//This int array has to end with -1 so the container knows how 
//many items we should have in the grid.
const TInt MainMenuCommandIds [] = {
   EWayFinderCmdPositionSelect,
   EWayFinderCmdCSMainView,
   EWayFinderCmdMyDest,
   EWayFinderCmdMap,
   EWayFinderCmdServiceWeather,
   EWayFinderCmdServiceCityGuide,
   -1
};
const TInt MainMenuLabelIds [] = {
   R_MM_ROUTE,
   R_MM_SEARCH,
   R_MM_FAVORITES,
   R_MM_MAP,
   R_MM_SERVICES_WEATHER,
   R_MM_SERVICES_CITYGUIDE
};
#else
const TInt MbmImageIds [] = {
   EMbmWficonsRoute_planner, 
   EMbmWficonsSearch,
   EMbmWficonsFavorites,
   EMbmWficonsMap,
   EMbmWficonsServices,
   EMbmWficonsSettings
};
const TInt MbmMaskIds [] = {
   EMbmWficonsRoute_planner_mask,
   EMbmWficonsSearch_mask,
   EMbmWficonsFavorites_mask,
   EMbmWficonsMap_mask,
   EMbmWficonsServices_mask,
   EMbmWficonsSettings_mask
};
//This int array has to end with -1 so the container knows how 
//many items we should have in the grid.
const TInt MainMenuCommandIds [] = {
   EWayFinderCmdPositionSelect,
   EWayFinderCmdCSMainView,
   EWayFinderCmdMyDest,
   EWayFinderCmdMap,
   EWayFinderCmdService,
   EWayFinderCmdSettings,
   -1
};
const TInt MainMenuLabelIds [] = {
   R_MM_ROUTE,
   R_MM_SEARCH,
   R_MM_FAVORITES,
   R_MM_MAP,
   R_MM_SERVICES,
   R_MM_SETTINGS
};
#endif

CMainMenuGridView* CMainMenuGridView::NewL(CWayFinderAppUi* aWayFinderAppUi)
{
   CMainMenuGridView* self = CMainMenuGridView::NewLC(aWayFinderAppUi);
   CleanupStack::Pop(self);
   return self;
}

CMainMenuGridView* CMainMenuGridView::NewLC(CWayFinderAppUi* aWayFinderAppUi)
{
   CMainMenuGridView* self = new (ELeave) CMainMenuGridView(aWayFinderAppUi);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CMainMenuGridView::CMainMenuGridView(CWayFinderAppUi* aWayFinderAppUi) :
   CViewBase(aWayFinderAppUi)
{
   iSelectedItem = 4;
}

void CMainMenuGridView::ConstructL()
{
   BaseConstructL(R_WAYFINDER_MAINMENU_VIEW);
   TInt nbrItems = 0;
   while (MainMenuCommandIds[nbrItems] != -1) {
      nbrItems++;
   }
   if (nbrItems <= 4) {
      iSelectedItem = 0;
   } else if (nbrItems <= 6) {
      if (WFLayoutUtils::LandscapeMode()) {
         iSelectedItem = 1;
      } else {
         iSelectedItem = 0;
      }
   } else {
      iSelectedItem = 4;
   }
}

CMainMenuGridView::~CMainMenuGridView()
{
   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }
   delete iContainer;
}

TPtrC CMainMenuGridView::GetMbmName()
{
   return iWayfinderAppUi->iPathManager->GetMbmName();
}

void CMainMenuGridView::SetNaviPaneLabelL(TInt aLabelId)
{
   iWayfinderAppUi->SetMainMenuNaviPaneLabelL(MainMenuLabelIds[aLabelId]);
}

void CMainMenuGridView::ResetNaviPaneLabelL()
{
   SetNaviPaneLabelL(iSelectedItem);
}

void CMainMenuGridView::HandleCommandL(const class CWAXParameterContainer& aCont)
{
   iWayfinderAppUi->ClearBrowserCache();
   iWayfinderAppUi->GotoStartViewL();
}

TUid CMainMenuGridView::Id() const
{
   return KMainMenuGridViewId;
}

void CMainMenuGridView::HandleCommandL( TInt aCommand )
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

void CMainMenuGridView::HandleClientRectChange()
{
   if (iContainer) {
      iContainer->SetRect(ClientRect());
   }
}

void CMainMenuGridView::DynInitMenuPaneL(TInt aResourceId, 
                                         CEikMenuPane* aMenuPane)
{
   AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}

void CMainMenuGridView::ReInitContainer()
{
   iSelectedItem = iContainer->GetActiveSelection();

   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }   
   delete iContainer;
   iContainer = NULL;

   iContainer = 
      CMainMenuGridContainer::NewL(ClientRect(), this, 
                                   MbmImageIds, MbmMaskIds, 
                                   MainMenuCommandIds);
   iContainer->SetMopParent(this);
   

   iContainer->SetActiveSelection(iSelectedItem);
   SetNaviPaneLabelL(iSelectedItem);
   AppUi()->AddToStackL(*this, iContainer);
}

void CMainMenuGridView::DoActivateL(const TVwsViewId& aPrevViewId, 
                                    TUid aCustomMessageId,
                                    const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
   iWayfinderAppUi->ActivateMainMenuNaviPaneLabelL();

   if(!iContainer) { 
      iContainer = 
         CMainMenuGridContainer::NewL(ClientRect(), this, 
                                      MbmImageIds, MbmMaskIds, 
                                      MainMenuCommandIds);
      iContainer->SetMopParent(this);
      AppUi()->AddToStackL(*this, iContainer);
   }
   iContainer->SetActiveSelection(iSelectedItem);
   SetNaviPaneLabelL(iContainer->GetActiveSelection());
}

void CMainMenuGridView::DoDeactivate()
{
   iWayfinderAppUi->DeactivateMainMenuNaviPaneLabelL();

   if (iContainer) {
      AppUi()->RemoveFromViewStack(*this, iContainer);
   }   
   delete iContainer;
   iContainer = NULL;
}

void CMainMenuGridView::UpdateActiveSelection()
{
   //iSelectedItem = aSelectedItem;
   if(iContainer) { 
      iContainer->SetActiveSelection(iSelectedItem);
      SetNaviPaneLabelL(iSelectedItem);
   }
}

void CMainMenuGridView::UpdateSelectedIndex(TInt aIndex)
{ 
   iSelectedItem = aIndex;
}

TInt CMainMenuGridView::GetSelectedIndex()
{
   return iSelectedItem;
}

// End of File

