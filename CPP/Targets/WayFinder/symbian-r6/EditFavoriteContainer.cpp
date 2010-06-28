/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
// s60v3fp2 and s60v5 only class

#include <avkon.hrh>
#include <avkon.rsg>
#include <eikmenup.h>
#include <aknappui.h>
#include <eikcmobs.h>
#include <barsread.h>
#include <stringloader.h>
#include <gdi.h>
#include <eikedwin.h>
#include <eikenv.h>
#include <eikmfne.h>
#include <lbsposition.h>
#include <e32math.h>

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "WFTextUtil.h"
#include "WFLayoutUtils.h"
#include "GuiProt/Favorite.h"
#include "MC2Coordinate.h"

#include "EditFavoriteContainer.h"

CEditFavoriteContainer::CEditFavoriteContainer(MEikCommandObserver* aCommandObserver,
                                               CEditFavoriteView& aView) : 
   iView(aView), 
   iFav(NULL)
{
   iCommandObserver = aCommandObserver;
}

CEditFavoriteContainer* 
CEditFavoriteContainer::NewL(MEikCommandObserver* aCommandObserver,
                             CEditFavoriteView& aView)
{
   CEditFavoriteContainer* self = 
      CEditFavoriteContainer::NewLC(aCommandObserver, aView);
   CleanupStack::Pop(self);
   return self;
}

CEditFavoriteContainer* 
CEditFavoriteContainer::NewLC(MEikCommandObserver* aCommandObserver,
                              CEditFavoriteView& aView)
{
   CEditFavoriteContainer* self = 
      new (ELeave) CEditFavoriteContainer(aCommandObserver, aView);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

void CEditFavoriteContainer::ConstructL()
{
   CAknForm::ConstructL();
}

CEditFavoriteContainer::~CEditFavoriteContainer()
{	
}
		
TKeyResponse 
CEditFavoriteContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                       TEventCode aType)
{
   return CAknForm::OfferKeyEventL(aKeyEvent, aType);
}

TBool CEditFavoriteContainer::QuerySaveChangesL()
{
   SaveFormDataL();
   return ETrue;
}

TBool CEditFavoriteContainer::SaveFormDataL()
{
   if (iFav) {
      TBuf<KBuf256Length> buf;
      iNameEdwin->GetText(buf);
      char* text = WFTextUtil::newTDesDupL(buf);
      iFav->setName(text);
      delete[] text;

      iDescEdwin->GetText(buf);
      text = WFTextUtil::newTDesDupL(buf);
      iFav->setDescription(text);
      delete[] text;

      TPosition pos;
      iLatEditor->Get(pos);
      iLonEditor->Get(pos);
      TReal64 lat = pos.Latitude();
      TReal64 lon = pos.Longitude();
      MC2Coordinate mc2Coord;
      if (Math::IsNaN(lat) || Math::IsNaN(lon)) {
         mc2Coord.lat = MAX_INT32;
         mc2Coord.lon = MAX_INT32;
      } else {
         WGS84Coordinate wgs84Coord(pos.Latitude(), pos.Longitude());
         mc2Coord = wgs84Coord;
      }
      Nav2Coordinate nav2Coord(mc2Coord);

      iFav->setLat(nav2Coord.nav2lat);
      iFav->setLon(nav2Coord.nav2lon);
   }
   return ETrue;
}

void CEditFavoriteContainer::DoNotSaveFormDataL()
{
   LoadFromDataL();
}

void CEditFavoriteContainer::LoadFromDataL()
{
   if (iFav) {
      TBuf<KBuf256Length> name;
      WFTextUtil::char2TDes(name, iFav->getName());
      
      TBuf<KBuf256Length> desc;
      WFTextUtil::char2TDes(desc, iFav->getDescription());
      
      SetEdwinTextL(EWayFinderEditFavoriteName, &name);
      SetEdwinTextL(EWayFinderEditFavoriteDesc, &desc);

      Nav2Coordinate nav2Coord(iFav->getLat(), iFav->getLon());
      //MC2Coordinate mc2Coord(iFav->getLat(), iFav->getLon());
      MC2Coordinate mc2Coord(nav2Coord);
      WGS84Coordinate wgs84Coord(mc2Coord);
      TTime time;
      time.HomeTime();
      TPosition pos(TLocality(TCoordinate(wgs84Coord.latDeg, 
                                          wgs84Coord.lonDeg), 0.0), time);
      iLatEditor->Set(pos);
      iLonEditor->Set(pos);
   }
}

void CEditFavoriteContainer::HandleResourceChange(TInt aType)
{
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
   CCoeControl::HandleResourceChange(aType);
}

void CEditFavoriteContainer::PreLayoutDynInitL()
{
   iNameEdwin = static_cast<CEikEdwin*>
      (ControlOrNull(EWayFinderEditFavoriteName));

   iDescEdwin = static_cast<CEikEdwin*>
      (ControlOrNull(EWayFinderEditFavoriteDesc));

   iLatEditor = static_cast<CAknLocationEditor*>
      (ControlOrNull(EWayFinderEditFavoriteLat));

   iLonEditor = static_cast<CAknLocationEditor*>
      (ControlOrNull(EWayFinderEditFavoriteLon));

   LoadFromDataL();
}
			
void CEditFavoriteContainer::DynInitMenuPaneL(TInt aResourceId, 
                                              CEikMenuPane* aMenuPane)
{
   CAknForm::DynInitMenuPaneL( aResourceId, aMenuPane );
	
   if (aResourceId == R_AVKON_FORM_MENUPANE) {
      aMenuPane->SetItemDimmed( EAknFormCmdAdd, ETrue );
      aMenuPane->SetItemDimmed( EAknFormCmdLabel, ETrue );
      aMenuPane->SetItemDimmed( EAknFormCmdDelete, ETrue );
      aMenuPane->SetItemDimmed( EAknFormCmdEdit, ETrue );
      aMenuPane->SetItemDimmed( EAknFormCmdSave, ETrue );
   }
}

TBool CEditFavoriteContainer::OkToExitL(TInt aButtonId)
{
   if (aButtonId == EAknSoftkeyBack) {
      if (CAknForm::OkToExitL(aButtonId)) {
         iView.HandleCommandL(aButtonId);
         // The dialog (form) may never die by itself since it will 
         // give a cone 44 panic if it is deleted without beeing 
         // removed from view stack (e.g. dodeactivate).
         return EFalse;
         //iAvkonAppUi->ProcessCommandL( EEikCmdExit );
      } else {
         return EFalse;
      }
   } 
   // If it's not back softkey, handle the command and don't exit (for the 
   // same reason as above), handle command needs to pop the view.
   CAknForm::OkToExitL(aButtonId);
   iView.HandleCommandL(aButtonId);
   return EFalse;
}

void CEditFavoriteContainer::SetFavoriteDataL(isab::Favorite& aFav)
{
   iFav = &aFav;
   LoadFromDataL();
}

void CEditFavoriteContainer::SaveFavoriteDataL()
{
   SaveFormDataL();
}

#endif // NAV2_CLIENT_SERIES60_V5 || NAV2_CLIENT_SERIES60_V32
