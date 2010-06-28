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
#include "MapView.h"
#include <avkon.hrh>
#include <eikdef.h>
#include <eikmenup.h>
#include <f32file.h>

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>
#include <utf.h>
#include <akndialog.h> 
#include <aknprogressdialog.h>
#ifndef NAV2_CLIENT_SERIES60_V1
#include <pathinfo.h>
#endif

#include "arch.h"
#include "WayFinderConstants.h"
#include "Constants.h"
#include "RouteEnums.h"
#include "Quality.h"
#include "TimeOutNotify.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiParameterEnums.h"
#include "WayFinderAppUi.h"
#include "GuideView.h"
#include "WFTextUtil.h"
#include "MapLibSymbianUtil.h"

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "memlog.h"

#include "MapEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtMapMess.h"
#include "GuiProt/MapClasses.h"
#include "GuiProt/Favorite.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/VectorMapCoordinates.h"
#include "CellDataDrawer.h"

#include "NavServerComEnums.h"
#include "SettingsData.h"
#include "Dialogs.h"
#include "debuggingdefines.h"

//For vector maps
#include "userbitmap.h"
#include "VectorMapContainer.h"
#include "VectorMapConnection.h"
#include "TileMapControl.h"

#include "MapUtility.h"

#include "MapInfoControl.h"

#include "Log.h"
#define LOGPOINTER if(iWayFinderUI->iLog)(iWayFinderUI->iLog)
#include "LogMacros.h"

#include "MapFeatureHolder.h"
#include "nav2util.h"
#include "UTF8Util.h"

/* For CNewDestView::GetRegionTypeName. */
#include "NewDestView.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/FullSearchItem.h"
#include "GuiProt/SearchItem.h"
#include "GuiProt/SearchRegion.h"
#include "WFTextUtil.h"

#include "DistancePrintingPolicy.h"
#include "UserDefinedScaleFeature.h"

#include "MC2SimpleString.h"
#include "TileCategory.h"
#include "Buffer.h"

#include "NewDestView.h"
#include "AnimatorFrame.h"
#include "ContextPaneAnimator.h"
#include "TileMapLayerInfo.h"
#include "TileMapHandler.h"

#include "MapLib.h"
#include "TileMapEvent.h"
#include "DriveList.h"
#include "TDesCHolder.h"
#include "PathFinder.h"
#include "CallBackDialog.h"

#include "MapMover.h"

#include "WAXParameterContainer.h"
#include "WAXParameter.h"
#include "WAXParameterTools.h"
#include "MC2Coordinate.h"
#include "ErrorFilter.h"

#include "DistanceBitmap.h"
#include "wficons.mbg"

#include "ConcreteTileMapHandler.h"

using namespace isab;
using namespace MapEnums;
using namespace RouteEnums;


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CMapView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void 
CConcreteTileMapHandler::ConstructL( 
		CWayFinderAppUi* aWayFinderUI 
) {
   iWayFinderUI = aWayFinderUI;
}

CConcreteTileMapHandler * 
CConcreteTileMapHandler::NewLC(
		CWayFinderAppUi * aUi 
) {
   CConcreteTileMapHandler * self = new (ELeave) CConcreteTileMapHandler();
   CleanupStack::PushL(self);
   self->ConstructL(aUi);
   return self;
}

CConcreteTileMapHandler * 
CConcreteTileMapHandler::NewL(
		CWayFinderAppUi * aUi 
) {
   CConcreteTileMapHandler * self = CConcreteTileMapHandler::NewLC(aUi);
   CleanupStack::Pop(self);
   return self;
}

void
CConcreteTileMapHandler::handleTileMapEvent(
		const class TileMapEvent &event
) {
   switch (event.getType()) {
   case TileMapEvent::NEW_CATEGORIES_AVAILABLE: {
      GeneralParameterMess *gen_mess = new (ELeave) GeneralParameterMess(
                                         GuiProtEnums::paramPoiCategories);
      LOGNEW(gen_mess, GeneralParameterMess);
      iWayFinderUI->SendMessageL( gen_mess );
      LOGDEL(gen_mess);
      delete gen_mess;
      
      gen_mess = new (ELeave) GeneralParameterMess(
                   GuiProtEnums::paramMapLayerSettings);
      LOGNEW(gen_mess, GeneralParameterMess);
      iWayFinderUI->SendMessageL( gen_mess );
      LOGDEL(gen_mess);
      delete gen_mess;
      break;
   }
   case TileMapEvent::UPDATE_CACHE_INFO:
      if (iMapControl) {
         MapLib* mapLib = iMapControl->getMapLib();
         if (mapLib) {
            * iNbrCacheFiles =
               mapLib->getCacheInfo(iCacheInfo);
         }
      }
      break;
   }
}

// End of File
