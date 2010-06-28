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
#include "WayFinderSettings.h"
#include "CommonWayfinderTargets.h"
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
#include <aknutils.h>
#include "TileMapUtil.h"
#ifndef NAV2_CLIENT_SERIES60_V1
#include <pathinfo.h>
#endif

#include "TileMapUtil.h"
#include "InterpolationHintConsumer.h"

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
#define LOGPOINTER if(iWayfinderAppUi->iLog)(iWayfinderAppUi->iLog)
#include "LogMacros.h"

#include "MapFeatureHolder.h"
#include "nav2util.h"
#include "UTF8Util.h"

/* For CNewDestView::GetRegionTypeName. */
#include "DataHolder.h"
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

#include "MC2Coordinate.h"
#include "ErrorFilter.h"
#include "TimeOutErrorFilter.h"
#include "WFLayoutUtils.h"
#include "RouteID.h"

_LIT( KGifNormalMap,        "map_normal.png" );
_LIT( KGifFullScreenMap,    "map_fullscreen.png" );

// For searching and moving map files
//_LIT(KMapFileTargetDir, "\\system\\data\\wfmaps\\");
_LIT(KMapFileExt, "wfd"); 
_LIT(KMapFileName, "*");

const TInt KDefaultRadius = 800;
const TReal KZoomFactor = 0.4;

//#define SAVE_MAPS_TO_FILE
#define VECTOR_MAP_MAX_ZOOM 14000

/*#define USE_UNENCRYPTED_MAPS */
#define ADD_WAREZ_PRECACHED_MAPS

// Enable this later. The server can send the route maps together with 
// the route reply. Also the switch in WayFinderAppUi must be updated.
#undef  ENABLE_FORCEFED_ROUTES

#include "DistanceBitmap.h"
#include "wficons.mbg"
const TInt cFullScreenOffset = 5;

using namespace isab;
using namespace MapEnums;
using namespace RouteEnums;

// ================= MEMBER FUNCTIONS =======================

CMapView::CMapView(CWayFinderAppUi* aUi, isab::Log* aLog) : 
   CViewBase(aUi),
   iLog(aLog),
   iPreservePositionOnNextActivation(EFalse)
{}
// ---------------------------------------------------------
// CMapView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CMapView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_MAP_VIEW );

   iPrevFinalizedTime = MAX_INT32;
   iPrevFinalizedPeriod = MAX_INT32;
   
   /*
   iOdometerFontCache =
      CDistanceBitmapHelper::NewL( iWayfinderAppUi->iPathManager->GetMbmName(), MbmfileIds );
   */
   iCenter.iX = DefaultVectorMapCenterLon;
   iCenter.iY = DefaultVectorMapCenterLat;
   iCurrentScale = MAX_INT32;

   iCurrentLat = MAX_INT32;
   iCurrentLon = MAX_INT32;
   iDestinationLat = MAX_INT32;
   iDestinationLon = MAX_INT32;
   iRouteDestLat = MAX_INT32;
   iRouteDestLon = MAX_INT32;
   iMapRadius = KDefaultRadius;

   iRequestType = MapEnums::None;
   iRequestCoord.iY = MAX_INT32;
   iRequestCoord.iX = MAX_INT32;

   iFeatureName.Copy( _L("") );

   iMapCacheSize = 10;
   
   iTrackingType = ERotating;

   iExitCount = -1;
   iExitCountMask = -1;

#ifdef _MSC_VER
   iFullScreenMapName.Copy(iWayfinderAppUi->iPathManager->GetWayfinderPath());
   iFullScreenMapName.Append(KGifFullScreenMap);
   iNormalScreenMapName.Copy(iWayfinderAppUi->iPathManager->GetWayfinderPath());
   iNormalScreenMapName.Append(KGifNormalMap);
#else
   iFullScreenMapName.Copy( _L("d:\\system\\temp\\") );
   iFullScreenMapName.Append(KGifFullScreenMap);
   iNormalScreenMapName.Copy( _L("d:\\system\\temp\\") );
   iNormalScreenMapName.Append(KGifNormalMap);
#endif
   iEventGenerator = CMapViewEventGenerator::NewL(*this);
   iDpiCorrFact = WFLayoutUtils::CalculateDpiCorrectionFactor();
   iPrevTimeStampMillis = MAX_INT32;
}

CMapView* CMapView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CMapView* self = new (ELeave) CMapView(aUi, aLog);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CMapView* CMapView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CMapView *self = CMapView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CMapView::~CMapView()
// ?implementation_description
// ---------------------------------------------------------
//
CMapView::~CMapView()
{
   if (iMapControl && iMapControl->getMapLib()) {
      MapLib* mapLib = iMapControl->getMapLib();
      mapLib->clearInterpolationCallback( this );
   }
   
   if( iVectorMapContainer ){
      AppUi()->RemoveFromViewStack( *this, iVectorMapContainer );
      LOGDEL(iVectorMapContainer);
      delete iVectorMapContainer;
      iVectorMapContainer = NULL;
   }

   /*   
   User::Free(iOdometerFontCache);
   iOdometerFontCache = NULL;
   */
   if (iGetDetailsReplyMessage) {
      iGetDetailsReplyMessage->deleteMembers();
      delete iGetDetailsReplyMessage;
   }
   // Should be called before exit, called here to do some clean up
   // if called by a panic shutdown.
   ReleaseMapControls();
   delete m_forcedRoutes;
   m_forcedRoutes = NULL;
   if (iMapFileList) {
      iMapFileList->Reset();  
      delete iMapFileList;
   }
   delete iEventGenerator;
   if (iNextStreet) {
      delete iNextStreet;
      iNextStreet = NULL;
   }
}

void CMapView::ReleaseMapControls()
{
   if (iVectorMapContainer) {
      iVectorMapContainer->SetMapControlsNull();
   }

   if( iMapControl ){
      // Trap added here to prevent CBase-66
      TRAPD(result, delete iMapControl; )
      iMapControl = NULL;
   }

   delete m_mapInfoControl;
   m_mapInfoControl = NULL;
   
   delete iMapFeatureHolder;
   iMapFeatureHolder = NULL;

}

void CMapView::DestroyVectorMapContainer()
{
   // Only called from HandleSystemEventL in AppUi. Needed to be able to close 
   // the application nicely from the framework e.g. when the user closes the 
   // app from the task manager.
   if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() ) {
      AppUi()->RemoveFromViewStack( *this, iVectorMapContainer );
      delete iVectorMapContainer;
      iVectorMapContainer = NULL;
   }
}

void
CMapView::SetVectorMapCoordinates(int32 scale, int32 lat, int32 lon)
{
   iCurrentScale = scale;
   iCenter.iX = lon;
   iCenter.iY = lat;
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      // XXX: REMOVE THIS; ONLY FOR TESTING
      //iVectorMapContainer->SetZoom(iCurrentScale);
      iVectorMapContainer->SetCenter(
            iCenter.iY, /* Remember, latlon is y,x. */
            iCenter.iX);
   }
}

void
CMapView::SaveVectorMapCoordinates(int32 scale, int32 lat, int32 lon)
{
   iCurrentScale = scale;
   iCenter.iX = lon;
   iCenter.iY = lat;

   VectorMapCoordinates* vmc =
      new (ELeave) VectorMapCoordinates(scale, lat, lon);

   if (vmc->Valid()) {
      GeneralParameterMess* gpm = vmc->CreateGeneralParameter();
      iWayfinderAppUi->SendMessageL( gpm );
      gpm->deleteMembers();
      LOGDEL(gpm);
      delete gpm;
   }
   delete vmc;
}

const std::vector<const TileCategory*>*
CMapView::GetPoiCategories()
{
   if (iMapControl) {
      /* Got the mapcontrol. */
      return iMapControl->Handler().getCategories();
   } else {
      return NULL;
   }
}

void
CMapView::ConvertCategoriesParamsToSettings(
      Buffer* buf,
      CSettingsData* aSettingsData)
{
   if (!buf) {
      /* Wrong version, ignore. */
      aSettingsData->iShowCategories = 0;
      return ;
   }
   int16 version = buf->readNextUnaligned16bit();
   if (version != VECTOR_MAP_POI_CATEGORY_VERSION) {
      /* Wrong version, ignore. */
      aSettingsData->iShowCategories = 0;
      return ;
   }

   std::map<int32,std::pair<int8,const char*>*> disabledList;
   if (buf) {
      /* Create the list for the settings in parameter. */

      while (buf->remaining() >= 4) {
         /* Won't allocate new space. */
         const char *tmp = buf->getNextCharString();

         int32 id = buf->readNextUnaligned32bit();
         int8 val = buf->readNext8bit();
         disabledList[id] = new std::pair<int8,const char *>(val,tmp);
      }
   }

   CArrayFix<TInt>& valList = aSettingsData->CheckboxArray();
   CArrayFix<char *>& txtList = aSettingsData->CheckboxTextArray();
   CArrayFix<TInt>& idList = aSettingsData->CheckboxIdArray();

   {
      /* Empty the current settings lists. */
      /* Clean up the arrays. */
      valList.Reset();
      TInt n;
      for (n = 0; n < txtList.Count(); n++) {
         /* Get the char* and delete the data. */
         delete[] txtList[n];
      }
      /* Remove entries. */
      txtList.Reset();
      idList.Reset();
   }

   /* If we get here, we have a number of poi categories. */
   aSettingsData->iShowCategories = 1;

   std::map<int32,std::pair<int8,const char *>*>::iterator it = disabledList.begin();
   while (it != disabledList.end()) {
      std::pair<int8,const char*>* setting = (*it).second;
      int32 val = setting->first;
      /* Need to create new copy for use in array. */
      char *name = strdup_new(setting->second);
      int32 id = (*it).first;

      if (iMapControl) {
         if (val)  {
            iMapControl->Handler().setCategoryEnabled(id, true);
         } else {
            iMapControl->Handler().setCategoryEnabled(id, false);
         }
      }
      valList.AppendL(val);
      txtList.AppendL(name);
      idList.AppendL(id);

      it++;
   }
}

struct TileCategoryLessThan {
   bool operator()(const TileCategory* aLhs, const TileCategory* aRhs)
   {
      HBufC* lhs = WFTextUtil::Alloc(aLhs->getName().c_str());
      HBufC* rhs = WFTextUtil::Alloc(aRhs->getName().c_str());
      if(lhs && rhs){
         bool less = lhs->CompareC(*rhs) < 0;
         delete lhs;
         delete rhs;
         return less;
      } else {
         TPtrC8 lhs_p(reinterpret_cast<const TText8*>(aLhs->getName().c_str()));
         TPtrC8 rhs_p(reinterpret_cast<const TText8*>(aRhs->getName().c_str()));
         return lhs_p.CompareC(rhs_p) < 0;
      }
   }
};

void
CMapView::ConvertCategoriesSettingsToTileCategories(
      Buffer* buf,
      CSettingsData* aSettingsData)
{
   const tc_vector_type_base* tc_vector = GetPoiCategories();

   if (!tc_vector || tc_vector->begin() == tc_vector->end()) {
      /* No maplib list available. */
      ConvertCategoriesParamsToSettings(buf, aSettingsData);
      return;
   }
   /* Got maplib list. */

   if (buf) {
      int16 version = buf->readNextUnaligned16bit();
      if (version != VECTOR_MAP_POI_CATEGORY_VERSION) {
         /* Wrong version, ignore. */
         buf = NULL;
      }
   }

   std::map<int32,int8> disabledList;
   if (buf) {
      /* Create the list for the settings in parameter. */

      while (buf->remaining() >= 4) {
         /* Won't allocate new space. */
         const char *tmp = buf->getNextCharString();
         tmp = tmp;

         int32 id = buf->readNextUnaligned32bit();
         int8 val = buf->readNext8bit();
         disabledList[id] = val;
      }
   }

   CArrayFix<TInt>& valList = aSettingsData->CheckboxArray();
   CArrayFix<char *>& txtList = aSettingsData->CheckboxTextArray();
   CArrayFix<TInt>& idList = aSettingsData->CheckboxIdArray();

   {
      /* Empty the current settings lists. */
      /* Clean up the arrays. */
      valList.Reset();
      TInt n;
      for (n = 0; n < txtList.Count(); n++) {
         /* Get the char* and delete the data. */
         delete[] txtList[n];
      }
      /* Remove entries. */
      txtList.Reset();
      idList.Reset();
   }

   /* If we get here, we have a number of poi categories. */
   aSettingsData->iShowCategories = 1;

   /* For each category, find current information from maplib */
   /* and compare to optional list saved in parameter file. */

   //Sort the vector.
   tc_vector_type_base sorted_tc(tc_vector->begin(), tc_vector->end());
   std::sort(sorted_tc.begin(), sorted_tc.end(), TileCategoryLessThan());

   for(tc_vector_type_base::const_iterator it = sorted_tc.begin();
       it != sorted_tc.end(); ++it) {
      const TileCategory *tc = *it;

      int32 val = tc->isEnabled();
      char *name = strdup_new(tc->getName().c_str());
      int32 id = tc->getID();

      if (buf) {
         /* We're only allowed to change the values in MapLib */
         /* if we have a parameter file buffer which states */
         /* which categories to use. */
         std::map<int32,int8>::iterator dit=disabledList.find(id);
         if (dit == disabledList.end()) {
            /* This category should be enabled. */
            val = 1;
            iMapControl->Handler().setCategoryEnabled(id, true);
         } else if ((*dit).second) {
            /* This category should be enabled. */
            val = 1;
            iMapControl->Handler().setCategoryEnabled(id, true);
         } else {
            /* This category should be disabled. */
            val = 0;
            iMapControl->Handler().setCategoryEnabled(id, false);
         }
      }

      valList.AppendL(val);
      txtList.AppendL(name);
      idList.AppendL(id);
   }
}


void
CMapView::ConvertMapLayerSettingsToNav2Parameter(
      TileMapLayerInfoVector* aParamVector,
      CSettingsData* aSettingsData)
{
   TileMapLayerInfoVector* tmpVector = aParamVector;
   if (!aParamVector) {
      /* Parameter was unset. Create empty vector. */
      tmpVector = new TileMapLayerInfoVector();
   }
   /* Set parameter from MapLib. */
   if (!iMapControl) {
      /* Maplib not available yet, wait for callback. */
      /* If we get here, we probably have aParamVector == NULL */
      /* or that iMapControl has been deleted between tests. */
   } else {
      if (!iMapControl->Handler().updateLayerInfo(*tmpVector)) {
         /* Vector wasn't changed by maplib, no data yet? */
      } else {
         /* Vector changed. We just write the parameter, and */
         /* change settings data when it arrives back. */
      }
   }
   /* Got vector. Write parameter. */
   int32 len = tmpVector->getSizeInDataBuffer();
   uint8 *data = new uint8[len+16];

   BitBuffer* buf = new BitBuffer(data, len);
   tmpVector->save(*buf);
   delete buf;

   /* Create message to send to Nav2. */
   GeneralParameterMess* gpm = new (ELeave) GeneralParameterMess(
      GuiProtEnums::paramMapLayerSettings, data, len);
   iWayfinderAppUi->SendMessageL( gpm );
   delete gpm;
   delete data;

   delete aSettingsData->iMapLayerSettingsVector;

   aSettingsData->iMapLayerSettingsVector = tmpVector;
}

bool
IsTheOne(const TileMapLayerInfo& item)
{
   return item.isOptionalAndPresent() && item.getID() == 0x3;
}

void
CMapView::ConvertMapLayerSettingsToLayerInfo(uint8 *data,
      int32 len, CSettingsData* aSettingsData)
{
   BitBuffer *buf = new BitBuffer(data, len);

   TileMapLayerInfoVector *paramVector =
      new TileMapLayerInfoVector();
   paramVector->load( *buf );

   if (iMapControl) {
      /* Copy settings to maplib. */
      // This is where we fill the paramvector with data from maplib 
      // the first run when it's empty.
      if (iMapControl->Handler().updateLayerInfo(*paramVector)) {
         // If the updateLayerInfo says that the vector is changed.
         // Rewrite parameters to Nav2.
         if (aSettingsData->iMapLayerSettingsVector->size() == 0) {
            // If the actual MapLayer settings data is empty it is the 
            // first time we enter the map ever. Then we want to copy 
            // the user setting for traffic info into the maplib 
            // instead of using the default value supplied from server.
            TileMapLayerInfoVector::iterator it =
               std::find_if(paramVector->begin(), paramVector->end(),
                            IsTheOne);
            if (it != paramVector->end()) {
               (*it).setVisible(aSettingsData->m_trafficUpdate);
               (*it).setUpdatePeriodMinutes(aSettingsData->m_trafficUpdatePeriod);
            }
         } 

         // Send the GuiProtEnums::paramMapLayerSettings parameter.
         // Transfers ownership of paramVector.
         ConvertMapLayerSettingsToNav2Parameter(paramVector, aSettingsData);
         delete buf;

         // GuiProtEnums::userTrafficUpdatePeriod parameter 
         // needs to be saved as well since the map might have 
         // forced a change of the setting. (and it is now 
         // the same parameter in settings view.)
         TileMapLayerInfoVector::iterator it =
            std::find_if(paramVector->begin(), paramVector->end(),
                         IsTheOne);
         if (it != paramVector->end()) {
            int32 updTime = (*it).getUpdatePeriodMinutes();
            if (!(*it).isVisible()) {
               // Let nav2 know that user has switched this option off.
               // Set the second highest bit, which is used as a flag.
               updTime |= 0x40000000;
            }
            GeneralParameterMess* gpm = new (ELeave) GeneralParameterMess(
               GuiProtEnums::userTrafficUpdatePeriod, updTime);
            iWayfinderAppUi->SendMessageL(gpm);
            delete gpm;
         }

         // Write to settings data when parameter returns.
         return;
      }
   }

   /* Vector wasn't changed by maplib, copy relevant settings */
   /* to SettingsData. */

   TileMapLayerInfoVector::iterator it =
      std::find_if(paramVector->begin(), paramVector->end(),
            IsTheOne);

   if (it != paramVector->end()) {
      //aSettingsData->iShowTraffic = ETrue;
   	  aSettingsData->iShowTraffic = EFalse;
      aSettingsData->m_trafficUpdatePeriod = (*it).getUpdatePeriodMinutes();
      aSettingsData->m_trafficOldUpdatePeriod = (*it).getUpdatePeriodMinutes();
      aSettingsData->m_trafficUpdate = (*it).isVisible();
      aSettingsData->m_trafficOldUpdate = (*it).isVisible();
   }

   SaveACPSettings(iWayfinderAppUi->GetSettingsData()->m_acpSettingEnabled);
   delete aSettingsData->iMapLayerSettingsVector;
   aSettingsData->iMapLayerSettingsVector = paramVector;

   delete buf;
}

void
CMapView::SaveMapLayerSettings(class CSettingsData* aSettingsData)
{
   /* Get settings from settingsdata. */
   TileMapLayerInfoVector* tmp = aSettingsData->iMapLayerSettingsVector;
   aSettingsData->iMapLayerSettingsVector = 0;

   if (!tmp) {
      /* No vector??? Should never happen! */
      return;
   }

   TileMapLayerInfoVector::iterator it =
      std::find_if(tmp->begin(), tmp->end(),
            IsTheOne);

   if (it != tmp->end()) {
      (*it).setVisible(aSettingsData->m_trafficUpdate);
      (*it).setUpdatePeriodMinutes(aSettingsData->m_trafficUpdatePeriod);
   }

   ConvertMapLayerSettingsToNav2Parameter(tmp, aSettingsData);
}

void CMapView::SaveACPSettings(int32 aEnableACP)
{
   if (iMapControl && iMapControl->getMapLib()) {
      iMapControl->getMapLib()->setACPModeEnabled((bool)aEnableACP);
   }
}

#define MAX_CACHED_RESULTS 10
void
CMapView::ReleaseInfoText()
{
   if (iGetDetailsReplyMessage) {
      /* Info text is released, thus the reply message is not current. */
      const FullSearchItem* item = iGetDetailsReplyMessage->operator [](0);
      char *txt;
      txt = strdup_new(item->getID());
      std::pair<char *, FullSearchDataReplyMess*> *pp =
         new std::pair<char *, FullSearchDataReplyMess*>(txt,
               iGetDetailsReplyMessage);

      iGetDetailsReplyCache.push_back(pp);

      if (iGetDetailsReplyCache.size() > MAX_CACHED_RESULTS) {
         /* Need to remove one entry at the front. */
         pp = iGetDetailsReplyCache.front();
         iGetDetailsReplyCache.pop_front();
         delete(pp->first);
         pp->second->deleteMembers();
         delete(pp->second);
         delete pp;
      }
      iGetDetailsReplyMessage = NULL;
   }
}

CSettingsData*
CMapView::GetSettingsData()
{
   return iWayfinderAppUi->GetSettingsData();
}

void
CMapView::ToggleSoftKeys(TBool on)
{
   Cba()->MakeVisible(on);
}

TBool
CMapView::DontHandleAsterisk()
{
   return iWayfinderAppUi->DontHandleAsterisk();
}

void
CMapView::Call()
{
   if (iGetDetailsReplyMessage) {
      /* Find a phone number. */
      const FullSearchItem* item = iGetDetailsReplyMessage->operator [](0);
      char *txt = NULL;

      TInt i=0;
      const AdditionalInfo* info;
      GuiProtEnums::AdditionalInfoType infoType;
      if( item->noAdditionalInfo() > 0 ){
         /* We have additional info. */
         info = item->getInfo(i);
         TBool done = EFalse;
         while( info != NULL  && !done){
            infoType = (GuiProtEnums::AdditionalInfoType)info->getType();
            switch (infoType) {
               case GuiProtEnums::phone_number:
               case GuiProtEnums::mobile_phone:
                  txt = WFTextUtil::stripPhoneNumberL(info->getValue());
                  done = ETrue;
                  break;
               default:
                  break;
            }
            i++;
            info = item->getInfo(i);
         }
      }

      if (txt) {
         /* Got a number. */
         TBuf<128> temp;
         WFTextUtil::char2TDes(temp, txt);

         iWayfinderAppUi->DialNumberL( temp, EWayFinderCmdMapCall );
      }
   }
}

FullSearchDataReplyMess*
CMapView::GetDetailsFindCached( const char* aItemId )
{
   std::deque<std::pair<char *, FullSearchDataReplyMess*> *>::iterator it;

   if (iGetDetailsReplyMessage) {
      const FullSearchItem* item = iGetDetailsReplyMessage->operator [](0);
      if (strcmp(item->getID(), aItemId) == 0 ) {
         /* Same! */
         FullSearchDataReplyMess* tmp = iGetDetailsReplyMessage;
         iGetDetailsReplyMessage = NULL;
         return tmp;
      }
   }
   /* Check the deque. */
   std::pair<char *, FullSearchDataReplyMess*> *pp;
   it = iGetDetailsReplyCache.begin();

   while (it != iGetDetailsReplyCache.end()) {
      pp = *it;
      if (strcmp(pp->first, aItemId) == 0) {
         /* Got it! */
         iGetDetailsReplyCache.erase(it);
         delete pp->first;
         FullSearchDataReplyMess* tmp = pp->second;
         delete pp;
         return tmp;
      }
      it++;
   }
   /* Didn't find it. */
   return NULL;
}

void
CMapView::GetDetails( const char* aItemId, TBool moreDetails )
{
   if (iCurrentGetDetailsName) {
      delete iCurrentGetDetailsName;
   }
   if (iCurrentGetDetailsId) {
      delete[] iCurrentGetDetailsId;
   }

   iCurrentGetDetailsName = WFTextUtil::newTDesDupL(iFeatureName);
   iCurrentGetDetailsId = strdup_new(aItemId);

   /* Check if this id is already cached. */
   FullSearchDataReplyMess *cached_mess =
      GetDetailsFindCached(aItemId);
   if (cached_mess) {
      /* Use this message instead of sending a new request. */
      if (moreDetails) {
         HandleCommandL(EWayFinderCmdMapShowInfo);
      } else {
         ShowDetailsL(cached_mess);
      }
      return;
   }
   if (moreDetails) {
      /* Can't get more details if there are none. */
      if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
         /* Remove text-note. */
         iVectorMapContainer->setInfoText(NULL);
      }
      return;
   }

   GenericGuiMess* message = new (ELeave) GenericGuiMess(
            GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID,
            iCurrentGetDetailsId,
            iCurrentGetDetailsName );
   LOGNEW(message, GenericGuiMess);
   iWayfinderAppUi->SendMessageL( message );
   message->deleteMembers();
   LOGDEL(message);
   delete message;
}


void CMapView::SetInfoText( const TDesC & text, TBool persistant )
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      HBufC8 * tmpDes = HBufC8::NewLC(6 * text.Length() + 1);
      TPtr8 tmpPtr = tmpDes->Des();
      CnvUtfConverter::ConvertFromUnicodeToUtf8(tmpPtr, text);
      iVectorMapContainer->setInfoText((char *)(tmpPtr.PtrZ()), NULL, NULL, persistant);
      CleanupStack::PopAndDestroy(tmpDes);
   }
}

void CMapView::SetInfoText( TInt aResourceId, TBool persistant )
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      HBufC * res = iCoeEnv->AllocReadResourceLC( aResourceId );
      SetInfoText(*res, persistant);
      CleanupStack::PopAndDestroy(res);
   }
}

void
CMapView::ShowDetailsL( FullSearchDataReplyMess* aMessage )
{
   if( aMessage->size() <= 0 || !iVectorMapContainer ){
      /* Reply size zero! */
      return;
   }
   if(!iVectorMapContainer->ConstructDone() ) {
      return;
   }
   /* Remove old data. */
   /* The data should always be NULL here. */
   if (iGetDetailsReplyMessage) {
      /* NB! This should never happen. */
      iGetDetailsReplyMessage->deleteMembers();
      delete iGetDetailsReplyMessage;
   }
   /* Save the data for later use. */
   iGetDetailsReplyMessage = new FullSearchDataReplyMess(aMessage);
   if (!iVectorMapContainer->IsInfoTextOn()) {
      /* Info text removed, don't show the new information, the user */
      /* "clicked" it away! */
      ReleaseInfoText();
      return;
   }
   const FullSearchItem* item = aMessage->operator [](0);
   std::vector<char *> maybe_show;
   std::vector<char *> show_second;
   std::vector<char *> show_first;
   char *txt;

   bool hasSensibleData = false;

   txt = strdup_new(item->getName());
   show_first.push_back(txt);
   txt = strdup_new("\n");
   show_first.push_back(txt);

   TInt i=0;
   const AdditionalInfo* info;
   GuiProtEnums::AdditionalInfoType infoType;
   if( item->noAdditionalInfo() > 0 ){
      /* We have additional info. */
      info = item->getInfo(i);
      while( info != NULL ){
         infoType = (GuiProtEnums::AdditionalInfoType)info->getType();
         switch (infoType) {
            case GuiProtEnums::dont_show:
            case GuiProtEnums::vis_house_nbr:
            case GuiProtEnums::short_info:
            case GuiProtEnums::image_url:
               break;
            case GuiProtEnums::phone_number:
            case GuiProtEnums::mobile_phone:
            case GuiProtEnums::vis_address:
               txt = strdup_new( info->getValue() );
               show_first.push_back(txt);
               txt = strdup_new("\n");
               show_first.push_back(txt);
               hasSensibleData = true;
               break;
            case GuiProtEnums::open_hours:
            case GuiProtEnums::short_description:
            case GuiProtEnums::vis_full_address:
            case GuiProtEnums::email:
            case GuiProtEnums::url:
               /* Show phone numbers. */
/*                txt = strdup_new(info->getKey()); */
/*                show_second.push_back(txt); */
/*                txt = strdup_new(": "); */
/*                show_second.push_back(txt); */
               txt = strdup_new( info->getValue() );
               show_second.push_back(txt);
               txt = strdup_new("\n");
               show_second.push_back(txt);
               break;
            case GuiProtEnums::wap_url:
            default:
               /* Show only if nothing else is available. */
               txt = strdup_new(info->getKey());
               maybe_show.push_back(txt);
               txt = strdup_new(": ");
               maybe_show.push_back(txt);
               txt = strdup_new( info->getValue() );
               maybe_show.push_back(txt);
               txt = strdup_new("\n");
               maybe_show.push_back(txt);
               break;
         }
         i++;
         info = item->getInfo(i);
      }
   }

   i=0;
   const SearchRegion* region = item->getRegion(i);
   while( region != NULL ){
      HBufC *tmp = CDataHolder::GetRegionTypeName(region);
      if (tmp) {
         txt = WFTextUtil::newTDesDupL(*tmp);
         show_second.push_back(txt);
         txt = strdup_new(": ");
         maybe_show.push_back(txt);
         txt = strdup_new( region->getName() );
         show_second.push_back(txt);
         txt = strdup_new("\n");
         maybe_show.push_back(txt);
         delete tmp;
      }
      i++;
      region = item->getRegion(i);
   }

   /* Concatenate string vectors. */
   if (show_second.size() > 0) {
      show_first.insert(show_first.end(),
            show_second.begin(),
            show_second.end());
      hasSensibleData = true;
   }
   if (maybe_show.size() > 0) {
      show_first.insert(show_first.end(), maybe_show.begin(), maybe_show.end());
      hasSensibleData = true;
   }

   /* Create the string to be presented. */
   TInt num_lines = 0;
   TInt index = 0;
   char *out = new char[1024];
   out[0] = 0;
   char *tmp = NULL;
   std::vector<char *>::iterator it = show_first.begin();

   while (num_lines < 6 && index < 1000 && it != show_first.end() ) {
      tmp = *it;
      if ( tmp && strcmp(tmp, "\n") == 0) {
         num_lines++;
      }
      int len = strlen(tmp);
      if (index + len <= 1000) {
         strcat(out, tmp);
         index += len;
      } else {
         strcat(out, "...");
         index += 3;
      }
      it++;
   }
   if (strlen(out) > 0) {
      if (hasSensibleData) {
#if defined NAV2_USE_UTF8
         iVectorMapContainer->setInfoText(out);
#else
         char* utf8Out = UTF8Util::isoToUtf8L(out);
         iVectorMapContainer->setInfoText(utf8Out);
#endif
         iVectorMapContainer->UpdateRepaint();
      } else {
         /* No new interesting data. */
         /* Don't change the current note. */
      }
   } else {
      iVectorMapContainer->setInfoText("Empty search details!");
   }


   it = show_first.begin();
   while (it != show_first.end()) {
      tmp = *it;
      delete[] tmp;
      it++;
   }
   show_first.clear();
   show_second.clear();
   maybe_show.clear();

   delete[] out;
}

void
CMapView::HandleFavoriteInfoReply( GetFavoriteInfoReplyMess* aMessage )
{
   Favorite* newFavorite = aMessage->getFavorite();

/*    HBufC *tmp = HBufC::NewLC(256); */
/*    tmp->Des().Copy(_L("")); */
/*    tmp->Des().AppendNum(newFavorite->getID(), EHex); */
/*    tmp->Des().Append(_L(" - ")); */
/*    TBuf<128> name; */
/*    WFTextUtil::char2TDes(name, newFavorite->getName()); */
/*    tmp->Des().Append(name); */
/*    name.Copy(_L("Favorite:")); */
/*    WFDialog::ShowScrollingDialogL(name, *tmp, EFalse); */

/*    CleanupStack::PopAndDestroy(tmp); */
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()){

      const char *name = newFavorite->getName();
      const char *desc = newFavorite->getDescription();
      char *dest = new char[strlen(newFavorite->getName())+
                            strlen(newFavorite->getDescription())+4];

      if ((desc == NULL) || (strcmp(desc, "") == 0)){
         /* No description. */
         strcpy(dest, name);
      } else if (strcasecmp(name, desc) == 0){
         /* Same string == special. Replace first "," with "\n". */
         strcpy(dest, name);
         char* commaCharPos = strchr(dest,  ',');
         if (commaCharPos != NULL){
            *commaCharPos = '\n';
            if (*(commaCharPos+1)==' ') {
               /* Replace space with another newline. */
               *(commaCharPos+1) = '\n';
            }
         }
      } else {
         /* Use both. */
         strcpy(dest, name);
         strcat(dest, "\n");
         strcat(dest, desc);
      }

#if defined NAV2_USE_UTF8
      iVectorMapContainer->setInfoText(dest);
#else
      char *foo = UTF8Util::isoToUtf8L(dest);
      iVectorMapContainer->setInfoText(foo);
      delete[] foo;
#endif

      delete[] dest;
   }
}

void
CMapView::RequestFavorite(uint32 id)
{
   GenericGuiMess* message = 
      new (ELeave) GenericGuiMess( GuiProtEnums::GET_FAVORITE_INFO, 
                                   id );
   LOGNEW(message, GenericGuiMess);
   iWayfinderAppUi->SendMessageL( message );
   message->deleteMembers();
   LOGDEL(message);
   delete message;
}

void
CMapView::FavoriteChanged()
{
   if (iMapFeatureHolder) {
      GenericGuiMess get(GuiProtEnums::GET_FAVORITES_ALL_DATA, uint16(0),
                         uint16(MAX_UINT16));
      iWayfinderAppUi->SendMessageL(&get);
   }
}

void
CMapView::HandleFavoritesAllDataReply( GetFavoritesAllDataReplyMess* aMessage )
{
   if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      favorite_vector_type* favVector = aMessage->getFavorites();
      if (iMapFeatureHolder)  {
         iMapFeatureHolder->UpdateFavorites(favVector, 
                                            WF_MAP_FAVORITE_IMAGE_NAME, 
                                            WF_MAP_FAVORITE_IMAGE_NAME_SMALL);
         iVectorMapContainer->UpdateRepaint();
      }
   }
}

void
CMapView::NewRouteDownloaded()
{
   m_routeid = iWayfinderAppUi->GetRouteId();
   SetMapRequestData( MapEnums::OverviewRoute, MAX_INT32, MAX_INT32 );
   if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      iVectorMapContainer->SetRoute( m_routeid );
      iVectorMapContainer->ShowEnd(EFalse, 0, 0);
      iVectorMapContainer->ShowStart(EFalse, 0, 0);
      iVectorMapContainer->ShowPoint(EFalse, 0, 0);
      iVectorMapContainer->UpdateRepaint();
      //Set3dMode(ETrue);
   } else {
      // Not able to fully execute what we want since the VectorMapContainer
      // hasnt yet been created. What we do is that we cache this call
      // and executes it in DoActivateL when VectorMapContainer has been
      // fully created.
      m_ptmfVec.push_back(&CMapView::NewRouteDownloaded);
   }
}

void
CMapView::ClearRoute()
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->ClearRoute();
      iVectorMapContainer->ShowEnd(EFalse, 0, 0);
      iVectorMapContainer->ShowStart(EFalse, 0, 0);
      iVectorMapContainer->UpdateRepaint();
      HandleCommandL(EWayFinderCmdMap);//Route is complete so we reset the view by activating it again.
      Set3dMode(EFalse);
      
      // Delete this, since no more navigation for the moment there
      // is no need to keep this allocated.
      delete iNextStreet;
      iNextStreet = NULL;
   } 
}


TPtrC 
CMapView::GetMbmName()
{
   return iWayfinderAppUi->iPathManager->GetMbmName();
}

TPtrC 
CMapView::GetMapCachePath()
{
   return iWayfinderAppUi->iPathManager->GetMapCacheBasePath();
}

TPtrC 
CMapView::GetCommonDataPath()
{
   return iWayfinderAppUi->iPathManager->GetCommonDataPath();
}

TPtrC 
CMapView::GetWritableAutoMapCachePath()
{
   return iWayfinderAppUi->iPathManager->GetWritableAutoMapCachePath();
}

/*
CDistanceBitmapHelper*
CMapView::GetOdometerFontCache()
{
   return iOdometerFontCache;
}
*/

void
CMapView::GpsStatus(TBool gps_on)
{
   if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() ) {
      if( Get3dMode() ){
         SetTracking( ETrue );
      }
      else if (iWayfinderAppUi->UseTrackingOnAuto()) {
         SetTracking(gps_on);
      }
   }
}

TBool
CMapView::SetConStatusImage(CFbsBitmap* bmp, CFbsBitmap* mask)
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetConStatusImage(bmp, mask);
      return ETrue;
   } else {
      return EFalse;
   }
}

void
CMapView::SetGpsStatusImage(TInt aImgId, TInt aMaskId)
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetGpsStatusImage(aImgId, aMaskId);
   } 
}

void CMapView::MakeGpsIndicatorVisible(bool aVisible) {
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->MakeGpsIndicatorVisible(aVisible);
   } 
}

TBool
CMapView::UseTrackingOnAuto()
{
   return iWayfinderAppUi->UseTrackingOnAuto();
}

GuiDataStore*
CMapView::GetGuiDataStore()
{
   return iWayfinderAppUi->GetGuiDataStore();
}

void CMapView::GetTempBaseDirectory( TDes& baseName )
{
   baseName.Copy( _L("d:\\system\\temp\\") );
}

void CMapView::GetBaseDirectory( TDes& baseName )
{
   baseName.Copy(iWayfinderAppUi->iPathManager->GetWayfinderPath());
}

void CMapView::positionInterpolated( MC2Coordinate newPosition,
                                     double velocityMPS,
                                     double headingDegrees )
{
   unsigned int curTime = TileMapUtil::currentTimeMillis();

   /**
    *   If we have only managed to draw a single position,
    *   and more than half the time between interpolation
    *   data updates have passed, drawing a new frame would
    *   be wasteful.
    */ 

   if (curTime > iPrevFinalizedTime + iPrevFinalizedPeriod / 2
       && iNumPositionInterpolations == 1) {
      return;
   }

   iNumPositionInterpolations++;   

   UpdatePosition( newPosition,
                   static_cast<int>( headingDegrees + 0.5),
                   static_cast<int>( velocityMPS * 3.6 + 0.5 ),
                   true );
}

void CMapView::UpdatePosition( const MC2Coordinate& newPosition,
                               int heading,
                               TInt aSpeed,
                               bool interpolated )
{
   Nav2Coordinate newCoord = newPosition;
   
   iCurrentLat = newCoord.nav2lat;
   iCurrentLon = newCoord.nav2lon;
   
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      
      int zoomScale = GetZoomScale( aSpeed );

      iVectorMapContainer->ShowUserPos( ETrue );
            
      // no need for Cell pos if Gps available
      iVectorMapContainer->SetCellIdIconEnabled(EFalse);
       
     if ( iVectorMapContainer->setGpsPos( newPosition,
                                           heading,
                                           zoomScale,
                                           interpolated) ) {

         if ( iIsTracking ) {
            // Show feature info in the blue note.
            iVectorMapContainer->ShowFeatureInfo();
         }
         iVectorMapContainer->UpdateRepaint();
         
      }
      
      if ( iIsTracking ) {
         if (iWayfinderAppUi->GetSettingsData()->m_backlightStrategy ==
             GuiProtEnums::backlight_on_during_route) {
            iWayfinderAppUi->TurnBackLightOnL();
         }
         // XXX: This was here before. Don't know what it's used for.
         iVectorMapContainer->Connect();
         iGotMapType = ETrue;
      }

   }
}


bool
CMapView::LoadNewInterpolationDataL( isab::UpdatePositionMess* aPositionMess,
                                      TInt aSpeed )
{
   /**
    *  Sometimes, the application will receive the same position message
    *  twice or more times in a row. This will interfere with the interpolation
    *  and thus we check for this condition.
    */
   
   if( aPositionMess->getTimeStampMillis() == iPrevTimeStampMillis ) {
      return false;
   }

   iNumPositionInterpolations = 0;   
   
   iPrevTimeStampMillis = aPositionMess->getTimeStampMillis();
   
   InterpolationHintConsumer* hintConsumer =
      iMapControl->getMapLib()->getInterpolationHintConsumer();

   /**
    *   We start the time period with the current time in the same
    *   format as MapLib uses.
    */
   
   unsigned int timeMillis = TileMapUtil::currentTimeMillis();
   
   hintConsumer->prepareNewData( timeMillis );

   /**
    *  Transfer the new data to our hint consumer.
    */

   const std::vector<HintNode>& hints =
      aPositionMess->getInterpolationHints();
   
   for( unsigned int i = 0; i < hints.size(); i++ ) {
      HintNode curHint = hints[i];
      
      hintConsumer->addDataPoint( Nav2Coordinate( curHint.lat,
                                                  curHint.lon ),
                                  curHint.velocityCmPS );
   }
   
   hintConsumer->finalizeNewData();

   /** 
    *  Update the period so that we can determine if we should
    *  update (draw) or not.
    */
   
   if( iPrevFinalizedTime != MAX_INT32 ) { 
      iPrevFinalizedPeriod =
         TileMapUtil::currentTimeMillis() - iPrevFinalizedTime;
   }

   iPrevFinalizedTime = TileMapUtil::currentTimeMillis();
   
   return true;
}

void CMapView::UpdateDataInterpolatedL( UpdatePositionMess* aPositionMess,
                                        TInt aSpeed )
{
   
   /**
    *   All conditions met, thus enabling interpolation.
    */
   MapLib* mapLib = iMapControl->getMapLib();
   mapLib->setInterpolationCallback( this );         
   mapLib->setInterpolationEnabled( true );

   if( LoadNewInterpolationDataL( aPositionMess, aSpeed ) ) {
      mapLib->requestImmediatePositionInterpolation();             
   }
}
void CMapView::UpdateDataL( UpdatePositionMess* aPositionMess,
                            TInt aSpeed, TInt aUnitSpeed )
{
   bool interpolatingPositions = false;
   
#ifdef INTERPOLATE_POSITIONS
   interpolatingPositions = true;
#endif

   MapLib* mapLib = NULL;

   if( iMapControl ) {
      mapLib = iMapControl->getMapLib();
   }
   
   if( mapLib && 
       !aPositionMess->getInterpolationHints().empty() &&
       HasRoute() &&
       interpolatingPositions &&
       iWayfinderAppUi->RouteStatusOnTrack() && 
       !iWayfinderAppUi->InPedestrianMode()  )
   {
      // When in pedestrian mode we dont want interpolated positions.
      UpdateDataInterpolatedL( aPositionMess,
                               aSpeed );
      if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
         iVectorMapContainer->SetSpeedL(aUnitSpeed);
      }
      return;
   } else if( mapLib ) {
      mapLib->setInterpolationEnabled( false );
   }

   // Now we use snap to route calculated positions in map.
   if (HasRoute() && iWayfinderAppUi->RouteStatusOnTrack() && 
       !iWayfinderAppUi->InPedestrianMode()) {
      iCurrentLat = aPositionMess->getRouteLat();
      iCurrentLon = aPositionMess->getRouteLon();
   } else {
      // We're not onTrack or we're pedestrians and dont want snap to route.
      iCurrentLat = aPositionMess->getLat();
      iCurrentLon = aPositionMess->getLon(); 
   }

   //   TInt heading = -1;
   TInt realHeading = -1;
   if (aPositionMess->headingQuality() > QualityUseless) {
      // Now we use snap to route calculated heading in map.
      if (HasRoute() && iWayfinderAppUi->RouteStatusOnTrack() && 
          !iWayfinderAppUi->InPedestrianMode()) {
         realHeading = aPositionMess->getRouteHeading();
      } else {
         // We're not onTrack or we're pedestrians and dont want snap to route.
         realHeading = aPositionMess->getHeading();
      }
   }
   
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone() ) {
      
      TInt heading360 = 0;
      
      if( realHeading > 0 ){
         heading360 = TInt(realHeading * ( 360.0 / 256.0 ) + 0.5);
      }
      if ( iIsTracking ) {
         if (iWayfinderAppUi->GetSettingsData()->m_backlightStrategy ==
               GuiProtEnums::backlight_on_during_route) {
            iWayfinderAppUi->TurnBackLightOnL();
         }
         // XXX: This was here before. Don't know what it's used for.
         iVectorMapContainer->Connect();
         iGotMapType = ETrue;
      }
      
      iVectorMapContainer->SetSpeedL(aUnitSpeed);
      UpdatePosition( Nav2Coordinate( iCurrentLat, iCurrentLon ),
                      (int) heading360,
                      aSpeed,
                      false );
   }   
}


_LIT(KSpeedScale, "Speed: %i Scale: %i");
int CMapView::GetZoomScale( TInt aSpeed )
{
   TInt newScale = -1;
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone() /*&& iIsTracking*/ ){

      TInt currentScale = iVectorMapContainer->GetScale();
      newScale = currentScale;
      TInt targetScale;
      if( aSpeed < 30 ){
         targetScale = 4;
      }
      else if( aSpeed < 55 ){
         targetScale = 4;
      }
      else if( aSpeed < 75 ){
         targetScale = 10;
      }
      else if( aSpeed < 95 ){
         targetScale = 20;
      }
      else{
         targetScale = 20;
      }

      TInt dScale = targetScale - currentScale;
      if( dScale > 3 ){
         newScale = currentScale+2;
      }
      else if( dScale >= 1 ){
         newScale = currentScale+1;
      }
      else if( dScale < -25 ){
         newScale = targetScale;
      }
      else if( dScale < -3 ){
         newScale = currentScale-2;
      }
      else if( dScale <= -1 ){
         newScale = currentScale-1;
      }
   }
   return newScale;
}


void CMapView::HideUserPosition()
{
   if( iVectorMapContainer && iVectorMapContainer->ConstructDone() ) {
      if( !iIsTracking || !iWayfinderAppUi->IsGpsConnected() ){
         // if a cell id position is available use that
         ShowCellIdPositionIfKnown();
         iVectorMapContainer->ShowUserPos( EFalse );
         iVectorMapContainer->UpdateRepaint();
      }
      else {
         iVectorMapContainer->SetCellIdIconEnabled(EFalse);  
      }
   } else {
      m_ptmfVec.push_back(&CMapView::HideUserPosition);
   }
}


void CMapView::SetDestination( TInt32 aLat, TInt32 aLon )
{
   iRouteDestLat = aLat;
   iRouteDestLon = aLon;
}

void CMapView::UpdatePicture( RouteEnums::RouteAction aAction,
                              RouteEnums::RouteCrossing aCrossing,
                              TUint &aDistance, TBool aLeftSideTraffic,
                              TInt aExitCount, TBool aHighway,
                              TInt detour, TInt speedcam)
{
   iCurrentTurn = TTurnPictures::GetPicture( aAction, aCrossing, aDistance, aHighway );
   iLeftSideTraffic = aLeftSideTraffic;
   if (iCurrentTurn == EMultiWayRdb) {
      iExitCount = TTurnPictures::GetExit(aExitCount);
      iExitCountMask = TTurnPictures::GetExitMask(aExitCount);
   } 
   SetTopBorder();
   SetPicture();
   UpdateDistance( aDistance );
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetDetourPicture( detour );
      iVectorMapContainer->SetSpeedCamPicture( speedcam );
      if (iCurrentTurn == EMultiWayRdb) {
         iVectorMapContainer->SetExitCountPictureL(iExitCount, iExitCountMask);
      } else {
         iVectorMapContainer->SetExitCountPictureL(-1, -1);
      }
   }
}

void CMapView::UpdateStatus( OnTrackEnum aStatus, TUint &aDistance )
{
   switch(aStatus)
      {
      case RouteEnums::OffTrack:
         iCurrentTurn = EOffTrack;
         break;
      case RouteEnums::WrongWay:
         iCurrentTurn = EWrongDirection;
         break;
      case RouteEnums::Goal:
         iCurrentTurn = EFinishFlag;
         aDistance = 0;
         break;
      case RouteEnums::OnTrack:
         break;
      }
   SetPicture();
   UpdateDistance( aDistance );
}

void CMapView::UpdateDistance( TUint aDistance )
{
   iLastDistance = aDistance;
   if( iVectorMapContainer && iVectorMapContainer->ConstructDone() ) {
      iVectorMapContainer->SetDistanceL( aDistance );
   }
}
void CMapView::UpdateDetour( TUint aDetour )
{
   if( iVectorMapContainer && iVectorMapContainer->ConstructDone() )
      iVectorMapContainer->SetDetourPicture( aDetour );
}

void CMapView::UpdateSpeedcam( TUint aSpeedcam )
{
   if( iVectorMapContainer && iVectorMapContainer->ConstructDone() )
      iVectorMapContainer->SetSpeedCamPicture( aSpeedcam );
}

void CMapView::UpdateETGL(TInt aETG)
{
  if( iVectorMapContainer && iVectorMapContainer->ConstructDone() )
     iVectorMapContainer->SetETGL(aETG);   
}

void CMapView::UpdateNextStreetL(const TDesC& aNextStreet)
{
   if( iVectorMapContainer && iVectorMapContainer->ConstructDone() ) {
     iVectorMapContainer->SetNextStreetL(aNextStreet);
     delete iNextStreet;
     iNextStreet = NULL;
   } else {
      if (iNextStreet) {
         delete iNextStreet;
         iNextStreet = NULL;
      }
      iNextStreet = aNextStreet.AllocL();
   }
}

TInt
CMapView::GetDistanceMode()
{
   return iWayfinderAppUi->GetDistanceMode();
}
void
CMapView::SetDistanceMode(TInt mode)
{
   if (iMapFeatureHolder && iMapFeatureHolder->iScaleFeature) {
      /* Change the scale feature. */

      DistancePrintingPolicy::DistanceMode d_mode =
         DistancePrintingPolicy::DistanceMode(mode);
      switch (d_mode) {
         case DistancePrintingPolicy::ModeImperialFeet:
            iMapFeatureHolder->iScaleFeature->setScale(
                  UserDefinedScaleFeature::getMilesFeetSettings());
            break;
         case DistancePrintingPolicy::ModeImperialYards:
            iMapFeatureHolder->iScaleFeature->setScale(
                  UserDefinedScaleFeature::getMilesYardsSettings());
            break;
         case DistancePrintingPolicy::ModeMetric:
            /* FALLTHROUGH */
         default:
            iMapFeatureHolder->iScaleFeature->setScale(
                  UserDefinedScaleFeature::getMeterSettings());
            break;
      }
   }
}

void CMapView::SetTopBorder()
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetTopBorder();
   }
}

void CMapView::NextStreetCtrlMakeVisible(TBool aMakeVisible)
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->NextStreetCtrlMakeVisible(aMakeVisible);
   }
}

void CMapView::SetPicture()
{
   TInt mbmIndex = TTurnPictures::GetMediumTurnPicture(iCurrentTurn, 
                                                       iLeftSideTraffic);
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone() &&
       mbmIndex > -1) {
      iVectorMapContainer->SetTurnPictureL(mbmIndex, 
                                           EMbmWficonsSm_turn_blank_mask);

      if (iCurrentTurn == EMultiWayRdb) {
         iVectorMapContainer->SetExitCountPictureL(iExitCount,
                                                   iExitCountMask);
      } else {
         iVectorMapContainer->SetExitCountPictureL(-1, -1);
      }
   }
}

void CMapView::RequestMap( MapSubject aMapType,
                           TInt32 aPosLat,
                           TInt32 aPosLon,
                           TInt32 /*aMaxLat*/, 
                           TInt32 /*aMinLon*/,
                           TInt32 /*aMinLat*/,
                           TInt32 /*aMaxLon*/ )
{
   switch( aMapType )
   {
   case MapEnums::OverviewRoute:
   {
      SetTracking(EFalse);
      RequestRouteVectorMap();
      break;
   }
   case MapEnums::SearchPosition:
   case MapEnums::FavoritePosition:
   case MapEnums::DestinationPosition:
   {
      TPoint pos;
      pos.iY = aPosLat;
      pos.iX = aPosLon;
      SetTracking(EFalse);
      RequestDestinationVectorMap( pos );
      break;
   }
   case MapEnums::UnmarkedPosition:
   {
      TPoint pos;
      pos.iY = aPosLat;
      pos.iX = aPosLon;
      SetTracking(EFalse);
      RequestPositionVectorMap( pos );
      break;
   }
   case MapEnums::UserPosition:
   {
      TPoint pos;
      pos.iY = iCurrentLat;
      pos.iX = iCurrentLon;
/*          SetTracking(EFalse); */
      RequestPositionVectorMap( pos );
      break;
   }
   case MapEnums::Tracking:
   {
      TPoint pos;
      pos.iY = aPosLat;
      pos.iX = aPosLon;
      RequestPositionVectorMap( pos );
      break;
   }
   case MapEnums::None:
      /* No longer used. */
   case MapEnums::Route:
      /* Not used. */
   //default:
      break;
   }
}

void CMapView::HandleMultiVectorMapReply( const DataGuiMess* aMessage )
{
   if ( aMessage->getMessageType() ==
        GuiProtEnums::GET_MULTI_VECTOR_MAP_REPLY ) { 

      if(iWayfinderAppUi->iLog) {
         iWayfinderAppUi->iLog->debug("Received multivectormapreply with id %u", 
                                   aMessage->getMessageID());
      }
      if( iVectorMapConnection ){
         if(iWayfinderAppUi->iLog){
            iWayfinderAppUi->iLog->debug("VectorMapConnection present");
         }
         iVectorMapConnection->MultiMapReply( *aMessage );
      }
   } else {
#ifdef ENABLE_FORCEFED_ROUTES
      // Then it is something from the route.
      // Forcefed map from route reply
      if ( iMapControl ) {
         // Nice. The map control is already there.
         iMapControl->receiveExtraMaps( aMessage->getData(),
                                        aMessage->getSize() );
         delete m_forcedRoutes;
         m_forcedRoutes = NULL;
      } else {
         // Save the buffer for later adding
         delete m_forcedRoutes;
         m_forcedRoutes = new DataGuiMess( aMessage->getMessageType(),
                                           aMessage->getSize(),
                                           aMessage->getData() );
      }
#endif
   }
}

void CMapView::HandleVectorMapErrorReply( uint16 aRequestId )
{
   if( iVectorMapConnection ){
      iVectorMapConnection->MapErrorReply( aRequestId );
   }
}

TBool
CMapView::IsGpsAllowed()
{
   return iWayfinderAppUi->IsGpsAllowed();
}

TBool
CMapView::IsGpsConnected()
{
   return iWayfinderAppUi->IsGpsConnected();
}

TBool
CMapView::IsGpsConnectedOrSearching()
{
   return iWayfinderAppUi->IsGpsConnectedOrSearching();
}

TBool
CMapView::IsReleaseVersion()
{
   return iWayfinderAppUi->IsReleaseVersion();
}

TBool
CMapView::IsIronVersion()
{
   return iWayfinderAppUi->IsIronVersion();
}

TBool CMapView::IsTracking()
{
   return iIsTracking;
}

TBool CMapView::IsSimulating()
{
   return iWayfinderAppUi->IsSimulating();
}


void
CMapView::Debug(TDesC& a)
{
   WFDialog::ShowDebugDialogL(a);
}

TBool CMapView::VectorMaps()
{
   return ETrue;
}


void CMapView::SetMapRequestData( MapEnums::MapSubject aMapType,
                                  TInt32 aPosLat, TInt32 aPosLon )
{
   iRequestType = aMapType;
   iRequestCoord.iY = aPosLat;
   iRequestCoord.iX = aPosLon;
}


void CMapView::SetMapFeatureName( const unsigned char* aName )
{
   const int length = strlen((const char*)aName);
   MapUtility::ConvertUTF8ToUnicode(aName, iFeatureName, length);

   /*TBuf8<128> UTFBuf;
     int numleft = 0;
     // initialize variables
     // check the length of the string
     int textlen = strlen((char*)aName);
     // copy the text into the buffer
     if( textlen < UTFBuf.MaxLength() ){
     UTFBuf.Copy(aName, textlen);
     }
     else{
     UTFBuf.Copy(aName, UTFBuf.MaxLength());
     }
     // do the conversion
     numleft = CnvUtfConverter::ConvertToUnicodeFromUtf8(iFeatureName, UTFBuf);*/
   
   //WFTextUtil::char2TDes( iFeatureName, aName );
}


void CMapView::GetMapFeatureName( TDes &aName )
{
   aName.Copy( iFeatureName );
}


TBool CMapView::SettingOrigin()
{
   return (iSetOrigin);
}


TBool CMapView::SettingDestination()
{
   return ( iSetDestination );
}


void CMapView::SetUseVectorMaps( TBool /* aUse */ )
{
/*    iUseVectorMaps = aUse; */
}


void CMapView::SetMapCacheSize( TInt aSize )
{
   iMapCacheSize = aSize;
   if( iMapControl && iMapControl->getMapLib() ) {
      if( iMapCacheSize == 0 ){
         ClearMapCache();
      }
      iMapControl->getMapLib()->setDiskCacheSize( iMapCacheSize*1024*1024 );
   }
}


TInt CMapView::GetMapCacheSize()
{
   return iMapCacheSize;
}


void CMapView::ClearMapCache()
{
   if( iMapControl && iMapControl->getMapLib() ) {
      iMapControl->getMapLib()->clearDiskCache();
   }
}


void
CMapView::TrackingStatusChanged()
{
   if( iVectorMapContainer && iVectorMapContainer->ConstructDone() ) {
      bool interpolating = false;
      
#ifdef INTERPOLATE_POSITIONS
      interpolating = true;
#endif
      
      iVectorMapContainer->setTrackingMode( iIsTracking,
                                            iTrackingType,
                                            interpolating );

      iVectorMapContainer->UpdateRepaint();
   }
}

void CMapView::RouteToMapDestinationCoordL() 
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
     TPoint position;
     iVectorMapContainer->GetLongPressOrNormalCoordinate( position );
     iWayfinderAppUi->
       SetPendingRouteMessageAndSendVehicleTypeL(
          GuiProtEnums::PositionTypePosition,
          "", position.iY, position.iX, "");
   }
}


void CMapView::SetTracking(TBool aOnOff)
{
   if( iIsTracking != aOnOff ){
      if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() && 
           iWayfinderAppUi->CanUseGPSForEveryThing() ) {
         iIsTracking = aOnOff;
         iTrackRequestCounter = 0;
         TrackingStatusChanged();
      }
   }
}


void CMapView::SetTrackingType( TrackingType aType )
{
   iTrackingType = aType;
   TrackingStatusChanged();
}


TrackingType CMapView::GetTrackingType()
{
   return iTrackingType;
}

/* void  */
/* CMapView::ShowVectorMapWaitSymbol( bool start ) */
/* { */
/*    if ( iVectorMapContainer != NULL ) {  */
/*       iVectorMapContainer->ShowVectorMapWaitSymbol( start ); */
/*    } */
/* } */

TBool CMapView::RequestRouteVectorMap()
{
   TBool mapRequested = EFalse;
   if( HasRoute() &&
       iWayfinderAppUi->GetRouteId() != 0 ) {
      m_routeid = iWayfinderAppUi->GetRouteId();
      if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
         iVectorMapContainer->SetRoute( m_routeid );
         TPoint tl;
         TPoint br;
         iWayfinderAppUi->GetRouteBoundingBox( tl, br );
         if( tl.iY != MAX_INT32 && tl.iX != MAX_INT32 &&
             br.iY != MAX_INT32 && br.iX != MAX_INT32 ){
            iGotMapType = ETrue;
            TInt dLat = (TInt)((tl.iY - br.iY)*0.05);
            TInt dLon = (TInt)((br.iX - tl.iX)*0.05);
            tl.iY += dLat;
            tl.iX -= dLon;
            br.iY -= dLat;
            br.iX += dLon;
            iVectorMapContainer->RequestRouteMap( tl, br );
            iVectorMapContainer->Connect();
            mapRequested = ETrue;
         }
         iVectorMapContainer->UpdateRepaint();
      }
   }
   return mapRequested;
}


TBool CMapView::RequestCurrPosVectorMap()
{
   TBool mapRequested = EFalse;
   TPoint pos = iWayfinderAppUi->GetCurrentPosition();
   if (iWayfinderAppUi->CanUseGPSForEveryThing() &&
       pos.iY != MAX_INT32 && pos.iX != MAX_INT32) {
      /* Postion is ok. */
      iVectorMapContainer->RequestPositionMap( pos );
      iCurrentScale = iVectorMapContainer->GetScale();
      iVectorMapContainer->Connect();
      iVectorMapContainer->UpdateRepaint();
      iGotMapType = ETrue;
      mapRequested = ETrue;
   }
   return mapRequested;
}


TBool CMapView::RequestPositionVectorMap( TPoint aPos )
{
   TBool mapRequested = EFalse;
   if( aPos.iY != MAX_INT32 && aPos.iX != MAX_INT32) {
      /* Postion is ok. */
      iVectorMapContainer->RequestPositionMap( aPos );
      iCurrentScale = iVectorMapContainer->GetScale();
      iVectorMapContainer->Connect();
      iVectorMapContainer->UpdateRepaint();
      iGotMapType = ETrue;
      mapRequested = ETrue;
   }
   return mapRequested;
}


TBool CMapView::RequestOriginVectorMap()
{
   TBool mapRequested = EFalse;
   if (iWayfinderAppUi->IsOriginSet()) {
      /* Got origin*/
      TPoint origin = iWayfinderAppUi->GetOrigin();
      if (origin.iY != MAX_INT32 && origin.iX != MAX_INT32) {
         // Dont set the point if it is invalid.
         iVectorMapContainer->ShowPoint( ETrue, origin.iY, origin.iX );
         iVectorMapContainer->RequestPositionMap( origin );
      }
      iCurrentScale = iVectorMapContainer->GetScale();
      iVectorMapContainer->Connect();
      iVectorMapContainer->UpdateRepaint();
      iGotMapType = ETrue;
      mapRequested = ETrue;
   }
   return mapRequested;
}


TBool CMapView::RequestDestinationVectorMap( TPoint aDest )
{
   TBool mapRequested = EFalse;
   if( aDest.iY != MAX_INT32 && aDest.iX != MAX_INT32 ){
      /* Got destination */
      iVectorMapContainer->ShowPoint( ETrue, aDest.iY, aDest.iX );
      iVectorMapContainer->RequestMarkedPositionMap( aDest );
      iVectorMapContainer->Connect();
      iVectorMapContainer->UpdateRepaint();
      iGotMapType = ETrue;
      mapRequested = ETrue;
   }
   return mapRequested;
}

void
CMapView::handleTileMapEvent(const class TileMapEvent &event)
{
   switch (event.getType()) {
   case TileMapEvent::NEW_CATEGORIES_AVAILABLE: {
      GeneralParameterMess *gen_mess = new (ELeave) GeneralParameterMess(
                                         GuiProtEnums::paramPoiCategories);
      LOGNEW(gen_mess, GeneralParameterMess);
      iWayfinderAppUi->SendMessageL( gen_mess );
      LOGDEL(gen_mess);
      delete gen_mess;
      
      gen_mess = new (ELeave) GeneralParameterMess(
                   GuiProtEnums::paramMapLayerSettings);
      LOGNEW(gen_mess, GeneralParameterMess);
      iWayfinderAppUi->SendMessageL( gen_mess );
      LOGDEL(gen_mess);
      delete gen_mess;
      break;
   }
   case TileMapEvent::UPDATE_CACHE_INFO:
      if (iMapControl) {
         MapLib* mapLib = iMapControl->getMapLib();
         if (mapLib) {
            iNbrCacheFiles =
               mapLib->getCacheInfo(iCacheInfo);
         }
      }
      break;
   default:
      break;
   }
}

void
CMapView::DialogDismissedL( TInt /*aButtonId*/ )
{
}


Favorite* CMapView::CreateFavorite( TUint aId, TInt aLat, TInt aLon,
                                    TDesC &aName, TDesC &aDescription,
                                    const char* aIcon, TBool aHasId, uint32 aLmID, 
                                    const char* aImeiCrcHex, const char* aLmsID )
{
   Favorite* newFavorite;

   char *newName;

   if (aName.Length() > 0) {
      newName = WFTextUtil::newTDesDupL(aName);
   } else {
      HBufC* tmp;
      tmp = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_UNKNOWN_TEXT);
      newName = WFTextUtil::newTDesDupL(*tmp);
      CleanupStack::PopAndDestroy(tmp);
   }


   char* newDesc = WFTextUtil::newTDesDupL(aDescription);
   char* newCategory = "";
   char* newAlias = "";

   if( aHasId ){
      newFavorite = new (ELeave) Favorite( aId, aLat, aLon, newName, newAlias,
                                           newDesc, newCategory, aIcon, false);
      LOGNEW(newFavorite, Favorite);
   }
   else{
      newFavorite = new (ELeave) Favorite( aLat, aLon, newName, newAlias, 
                                           newDesc, newCategory, aIcon, false);
      LOGNEW(newFavorite, Favorite);
   }
   LOGDELA(newDesc);
   delete[] newDesc;
   LOGDELA(newName);
   delete[] newName;

   return newFavorite;
}


_LIT(KIcon, "icon");

// ---------------------------------------------------------
// TUid CMapView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CMapView::Id() const
{
   return KMapViewId;
}

// ---------------------------------------------------------
// CMapView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CMapView::HandleCommandL(TInt aCommand)
{   
   switch ( aCommand )
      {
      case EAknSoftkeyExit:
         {
            iWayfinderAppUi->HandleCommandL(aCommand);
         }
         break;
      case EAknSoftkeyBack:
         {
            if (iWayfinderAppUi->IsIronVersion()) {
               // if earth there is no 3d to worry about.
               AppUi()->HandleCommandL( aCommand );
            } else {
               if (iCustomMessageId == EShowMapFromItineraryView) {
                  // This happens when itinerary view selects show turn or 
                  // show route from options meny.
                  AppUi()->HandleCommandL( aCommand );
               } else if (MapAsCarNavigation() && !Get3dMode()) {
                  Set3dMode(ETrue);
               } else if (MapAsGeneralNavigation() && HasRoute()) {
                  iWayfinderAppUi->QueryQuitNavigationL();
               } else {
                  AppUi()->HandleCommandL( aCommand );
               }
            }
            break;
         }
      case EWayFinderCmdMapZoomIn:
         {
            if( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
               SetTracking(EFalse);
               iVectorMapContainer->ZoomIn();
               iVectorMapContainer->UpdateRepaint();
            }
            break;
         }
      case EWayFinderCmdMapZoomOut:
         {
            if( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
               SetTracking(EFalse);
               iVectorMapContainer->ZoomOut();
               iVectorMapContainer->UpdateRepaint();
            }
            break;
         }
      case EWayFinderCmdMapMoveUp:
      case EWayFinderCmdMapMoveDown:
      case EWayFinderCmdMapMoveLeft:
      case EWayFinderCmdMapMoveRight:
         {
            break;
         }
      case EWayFinderCmdMapFullScreen:
         {
            //SetFullScreen();
            break;
         }
      case EWayFinderCmdMapViewRoute:
         {
            RequestMap( MapEnums::OverviewRoute );
            break;
         }
      case EWayFinderCmdMapViewCurrent:
         {
            RequestMap( MapEnums::UserPosition );
            break;
         }
      case EWayFinderCmdMapViewOverview:
         {
            if( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
               SetTracking(EFalse);
               iVectorMapContainer->ZoomToOverview();
               iVectorMapContainer->UpdateRepaint();
            }
            break;
         }
      case EWayFinderCmdMapViewDestination:
         {
            if( HasRoute() && iRouteDestLat != MAX_INT32
                && iRouteDestLon != MAX_INT32 ){
               RequestMap( MapEnums::UnmarkedPosition,
                           iRouteDestLat, iRouteDestLon );
            }
            else if( iWayfinderAppUi->IsDestinationSet() ){
               TPoint t1 = iWayfinderAppUi->GetDestination();
               RequestMap( MapEnums::DestinationPosition, t1.iY, t1.iX);
            }
            else{
               /* No route and no destination set? */
               /* Impossible, we can't get this command then. */
            }
            break;
         }
      case EWayFinderCmdMapViewOrigin:
         {
            if (iWayfinderAppUi->IsOriginSet()) {
               TPoint orig = iWayfinderAppUi->GetOrigin();
               RequestMap( MapEnums::UnmarkedPosition, orig.iY, orig.iX );
            }
            else{
               /* No origin set? */
               /* Impossible, we can't get this command then. */
            }
            break;
         }
      case EWayFinderCmdMapViewTrackingOn:
      case EWayFinderCmdMapTracking:
         {
            // if there is no GPS position & there is a cell id move the map
            if (iCurrentLat == MAX_INT32 || iCurrentLon == MAX_INT32 ) {
               // Is there a cell id position
               Nav2Coordinate  navCoOrd(MAX_INT32,MAX_INT32);
               TUint cellRadius = 0;
               if ( iWayfinderAppUi->GetCellPosIfAvailable(navCoOrd, cellRadius) ) {
                  // Get the Cell Position information
                  TPoint cellPos( navCoOrd.nav2lon, navCoOrd.nav2lat );
                   			
                  // Position the map according to the cell infoe
                  // and try to adjust the zoom according to the cell size
                  iVectorMapContainer->RequestPositionAndZoomMap( cellPos, ConvertCellRadiusToMapScale(cellRadius) );

                  // Display the Cell Id icon 
                  ShowCellIdPosition(navCoOrd, cellRadius);
               }
            }
         	
         	// start the GPS tracking
            SetTracking(ETrue);
            break;
         }
      case EWayFinderCmdMapViewTrackingOff:
         {
            SetTracking(EFalse);
            break;
         }
      case EWayFinderCmdMapVoice:
      case EWayFinderCmdMapReroute:
         {
            if (iWayfinderAppUi->IsGpsAllowed() ) {
               AppUi()->HandleCommandL( EWayFinderCmdMapReroute );
            } else {
               AppUi()->HandleCommandL( EWayFinderCmdMapVoice );
            }
            break;
         }
      case EWayFinderCmdMapRouteTo:
         {
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               /*
               TPoint position;
               iVectorMapContainer->GetCoordinate( position );
               iVectorMapContainer->GetFeatureName();
               iWayfinderAppUi->RouteToCoordinateL( position.iY, position.iX, iFeatureName );
               */
               if (iWayfinderAppUi->
                   CheckAndDoGpsConnectionL((void*) this, 
                                            CMapView::WrapperToNavigateToCallback)) {
                  iWayfinderAppUi->SetFromGps();
                  RouteToMapDestinationCoordL();
                  //HandleCommandL(EWayFinderCmdMapSetDestination);
               }
            }
            break;
         }
      case EWayFinderCmdMapShowInfo:
         {
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               MC2Coordinate position;
               iVectorMapContainer->GetLongPressOrNormalCoordinate(position);
               HBufC* serverString = iVectorMapContainer->GetServerStringL();
               iWayfinderAppUi->GotoInfoInServiceViewL(position.lat, position.lon, serverString);
               delete serverString;
            }
            break;
         }
      case EWayFinderCmdMapSetDestination:
         {
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               iWayfinderAppUi->HandleCommandL(EWayFinderCmdPositionSelect);
               TPoint position;
               iVectorMapContainer->GetLongPressOrNormalCoordinate( position );
               iVectorMapContainer->GetFeatureName();
               
               HBufC* unknownCompare = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_UNKNOWN_TEXT);
               if (!iFeatureName.Compare(unknownCompare->Des()))
               {
               iFeatureName.Copy(iCoeEnv->AllocReadResourceL(R_POSITIONSELECT_FROM_MAP_TEXT)->Left(64));
               }
               HBufC* txtBuf = iFeatureName.AllocLC();
               iWayfinderAppUi->SetDestination( GuiProtEnums::PositionTypePosition,
                                             *txtBuf, NULL, position.iY, position.iX );
               CleanupStack::PopAndDestroy(txtBuf);
               CleanupStack::PopAndDestroy(unknownCompare);
               //iWayfinderAppUi->PositionSelectRoute();
               iSetDestination = EFalse;
               iVectorMapContainer->ShowEnd(ETrue, position.iY, position.iX);
               iVectorMapContainer->UpdateRepaint();
            }
            break;
         }
      case EWayFinderCmdMapSetOrigin:
         {
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               iWayfinderAppUi->HandleCommandL(EWayFinderCmdPositionSelect);
               TPoint position;
               iVectorMapContainer->GetLongPressOrNormalCoordinate( position );
               iVectorMapContainer->GetFeatureName();
               
               HBufC* unknownCompare = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_UNKNOWN_TEXT);
               if (!iFeatureName.Compare(unknownCompare->Des()))
               {
               iFeatureName.Copy(iCoeEnv->AllocReadResourceL(R_POSITIONSELECT_FROM_MAP_TEXT)->Left(64));
               }
               HBufC* txtBuf = iFeatureName.AllocLC();
               iWayfinderAppUi->SetOrigin( GuiProtEnums::PositionTypePosition,
                                        *txtBuf, "", position.iY, position.iX );
               CleanupStack::PopAndDestroy(txtBuf);
               CleanupStack::PopAndDestroy(unknownCompare);
               iWayfinderAppUi->PositionSelectRoute();
               iSetOrigin = EFalse;
               iVectorMapContainer->ShowStart(ETrue, position.iY, position.iX);
               iVectorMapContainer->UpdateRepaint();
            }
            break;
         }
      case EWayFinderCmdMapSaveFavorite:
         if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
/*             iVectorMapContainer->SetDistanceL(999999999); */
/*             break; */
            TPoint position;
            iVectorMapContainer->GetLongPressOrNormalCoordinate( position );
            iVectorMapContainer->GetFeatureName();
            Favorite* fav = CreateFavorite( 0, position.iY, position.iX,
                                            iFeatureName, iFeatureName, 
                                            "", EFalse, MAX_UINT32,
                                            NULL, NULL );

            iWayfinderAppUi->AddFavoriteFromMapL(fav);
         }
         break;
      case EWayFinderCmdConnectSendThisPosition:
         if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
            TPoint *position = new (ELeave) TPoint;
            /* position will transfer to Connect Dialog's ownership! */

            iVectorMapContainer->GetCoordinate( *position );
         }
         break;
      case EWayFinderCmdGuide:
         {
            if (HasRoute()) {
               iWayfinderAppUi->push( KGuideViewId );
            } else {
               /* Don't move to Guide view if we have no route. */
            }
            break;
         }
      case EWayFinderCmdDestination:
         {
            if (HasRoute()) {
               iWayfinderAppUi->push( KDestinationViewId );
            } else {
               /* Don't move to Destination view if we have no route. */
            }
            break;
         }
      case EWayFinderCmdMapViewShowInstalledMaps:
         ShowInstalledMaps();
         break;
      case EWayFinderCmdMapViewUpdateMapFiles:
         SearchForMapFiles();
         break;
      case EWayFinderCmdMapViewDeleteMapFiles:
         DeleteMapFiles();
         break;
      case EWayFinderCmdActivate3d:
      case EWayFinderCmdActivate2d:
         Set3dMode( !Get3dMode() );
         break;
      case EWayFinderCmdSend:
         iWayfinderAppUi->SendMapPositionL();
         break;
      default:
         {
            AppUi()->HandleCommandL( aCommand );
            break;
         }
      }
   if(iVectorMapContainer) {
      iVectorMapContainer->ResetLongPressPositionFlag();
   }
}

// ---------------------------------------------------------
// CMapView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CMapView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_MAP_MENU ){
      // Reroute always hidden, except for when in pedestrian navigation.
      aMenuPane->SetItemDimmed(EWayFinderCmdMapReroute, ETrue);

      if (iCustomMessageId == ERoutePlannerSetAsOrigin ||
          iCustomMessageId == ERoutePlannerSetAsDest) {

         // Ordinary map menu itmes
         aMenuPane->SetItemDimmed(EWayFinderCmdActivate3d, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdActivate2d, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapRouteTo, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapZoomIn, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapZoomOut, ETrue);
         //aMenuPane->SetItemDimmed(EWayFinderCmdMapFiles, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSaveFavorite, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdSend, ETrue);

         // Navigation map menu items
         aMenuPane->SetItemDimmed(EWayFinderCmdGuide, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdActivate3d, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdActivate2d, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOn, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOff, ETrue);

         // Show information should be visible in this state as well unless
         // HideServicesInfo is true.
         aMenuPane->SetItemDimmed(EWayFinderCmdMapShowInfo, 
                                  iWayfinderAppUi->HideServicesInfo());
         if (iCustomMessageId == ERoutePlannerSetAsOrigin) {
            // Set as origin, hide command for set dest and unhide 
            // command for set origin.
            aMenuPane->SetItemDimmed(EWayFinderCmdMapSetOrigin, EFalse);
            aMenuPane->SetItemDimmed(EWayFinderCmdMapSetDestination, ETrue);
         }
         if (iCustomMessageId == ERoutePlannerSetAsDest) {
            // Set as dest, hide command for set origin and unhide 
            // command for set dest.
            aMenuPane->SetItemDimmed(EWayFinderCmdMapSetOrigin, ETrue);
            aMenuPane->SetItemDimmed(EWayFinderCmdMapSetDestination, EFalse);
         }
      } else if (MapAsGeneralNavigation() || 
                 (iCustomMessageId == EShowMapFromItineraryView)) {
         // Menu options for non navigation map should be hidden
         aMenuPane->SetItemDimmed(EWayFinderCmdMapRouteTo, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSaveFavorite, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapShowInfo, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdSend, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapZoomIn, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapZoomOut, ETrue);
         //aMenuPane->SetItemDimmed(EWayFinderCmdMapFiles, ETrue);

         // If we're in 3d we don't need the tracking meny option.
         if (Get3dMode()) {
            aMenuPane->SetItemDimmed(EWayFinderCmdMapTracking, ETrue);
         } else {
         	// dim the option if there is no valid GPS or Cell id position
            aMenuPane->SetItemDimmed(EWayFinderCmdMapTracking, 
                                     !(iWayfinderAppUi->IsGpsConnected() && 
                                       iWayfinderAppUi->ValidGpsStrength()) 
                                       || iWayfinderAppUi->IsCellPosAvailable() );
         }

         // Menu options when coming from route planner should be hidden
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSetOrigin, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSetDestination, ETrue);

         // Menu options for navigation map should be visible
         aMenuPane->SetItemDimmed(EWayFinderCmdGuide, EFalse);
         if (MapAsPedestrianNavigation()) {
            // In pedestrian mode we dont allow 3d.
            aMenuPane->SetItemDimmed(EWayFinderCmdActivate3d, ETrue);
            aMenuPane->SetItemDimmed(EWayFinderCmdActivate2d, ETrue);
            // But we show reroute command.
            aMenuPane->SetItemDimmed(EWayFinderCmdMapReroute, EFalse);
         } else {
            aMenuPane->SetItemDimmed(EWayFinderCmdActivate3d, Get3dMode());
            aMenuPane->SetItemDimmed(EWayFinderCmdActivate2d, !Get3dMode());
         }
         aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOn, 
                                  iWayfinderAppUi->IsNightMode()); 
         aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOff, 
                                  !iWayfinderAppUi->IsNightMode()); 

      } else if (iCustomMessageId == EMapAsOverview || 
                 iCustomMessageId == EShowOnMap) {
         // If using the map as overview, the options for navigation
         // map should be visible
         aMenuPane->SetItemDimmed(EWayFinderCmdGuide, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdActivate3d, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdActivate2d, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOn, ETrue); 
         aMenuPane->SetItemDimmed(EWayFinderCmdNightModeOff, ETrue);
         
         // Also hide the options for setting dest and origin
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSetOrigin, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSetDestination, ETrue);
         
         // Menu options for non navigation map should be visible
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSaveFavorite, EFalse);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapTracking, 
                                  !((iWayfinderAppUi->IsGpsConnected() && 
                                    iWayfinderAppUi->ValidGpsStrength())
                                    || iWayfinderAppUi->IsCellPosAvailable() ));
                                    
         aMenuPane->SetItemDimmed(EWayFinderCmdSend, EFalse);
         //aMenuPane->SetItemDimmed(EWayFinderCmdMapFiles, EFalse);
         // Display show info if not defined to hide the service window
         aMenuPane->SetItemDimmed(EWayFinderCmdMapShowInfo, 
                                  iWayfinderAppUi->HideServicesInfo());
         // Display route to unless user don't have the right for it.
         aMenuPane->SetItemDimmed(EWayFinderCmdMapRouteTo, 
                                  !iWayfinderAppUi->CanUseGPSForEveryThing());

#ifdef NAV2_CLIENT_SERIES60_V5
         // If 5th edition, hide zoom in/out if pen is enabled
         aMenuPane->SetItemDimmed(EWayFinderCmdMapZoomIn, AknLayoutUtils::PenEnabled());
         aMenuPane->SetItemDimmed(EWayFinderCmdMapZoomOut, AknLayoutUtils::PenEnabled());
#else
         aMenuPane->SetItemDimmed(EWayFinderCmdMapZoomIn, EFalse);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapZoomOut, EFalse);
#endif
         
         if (IsIronVersion()) {
            // earth should have a limited menu.
            aMenuPane->SetItemDimmed(EWayFinderCmdMapRouteTo, 
                                     !iWayfinderAppUi->EarthAllowRoute());
         }
      }
   } else {
      AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
   }
}

void
CMapView::tryToAddVectorPrecache()
{
   if ( iVectorMapContainer == NULL ) {
      return;
   }
   if ( iMapControl == NULL ) {
      // No mapcontrol
      return;
   }
   if ( iMapControl->getMapLib() == NULL ) {
      // No maplib
      return;
   }
   
   if ( GetSettingsData() == NULL ) {
      return;
   }
   if ( GetSettingsData()->iUsername.Length() == 0 ) {
      // No user name yet.
      return;
   }

   // Add the non-warez stuff first.
   if ( ! m_precachedCachesAdded ) {
      m_precachedCachesAdded = true;
      MapLibSymbianUtil::insertPrecachedMapsL(
         CEikonEnv::Static()->FsSession(),
         *iMapControl->getMapLib(),
         GetSettingsData()->iUsername,
         false );
      
#ifdef ADD_WAREZ_PRECACHED_MAPS
      // Add the warez cache.
      MapLibSymbianUtil::insertPrecachedMapsL(
         CEikonEnv::Static()->FsSession(),
         *iMapControl->getMapLib(),
         GetSettingsData()->iUsername,
         true );      
#endif
   }
}

// ---------------------------------------------------------
// CMapView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CMapView::DoActivateL( const TVwsViewId& aPrevViewId,
                            TUid aCustomMessageId,
                            const TDesC8& aCustomMessage)
{
   iActivated = EFalse;
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
   
   if( !iVectorMapConnection ){
      if( !iWayfinderAppUi->CheckMem(600*1024) ){
         WFDialog::ShowScrollingDialogL( iCoeEnv, R_WAYFINDER_MEMORY_LOW_MSG,
                                         R_WAYFINDER_MEMORY_LOW_EXIT_MSG, ETrue);
         // Switch to main view.
         iWayfinderAppUi->ShutdownNow();
         return;
      }
   }

   TBool hasRoute   = HasRoute();
   iSetOrigin       = (iCustomMessageId == ERoutePlannerSetAsOrigin );
   iSetDestination  = (iCustomMessageId == ERoutePlannerSetAsDest);

   // If we're in pedestrian mode we always want north oriented map.
   if (MapAsPedestrianNavigation()) {
      SetTrackingType(ENorthUp);
   } else {
      if(hasRoute) {
         SetTrackingType(ERotating);
      }
      else {
         SetTrackingType(ENorthUp);
      }
   }
   if( !iVectorMapContainer ){
      iVectorMapContainer = new (ELeave) CVectorMapContainer( this, iLog );
      LOGNEW(iVectorMapContainer, CVectorMapContainer);
      iVectorMapContainer->SetMopParent(this);

      TBool first = ( iMapFeatureHolder == NULL );

      TRect vectorMapRect;
      // Get location of cba buttons.
      AknLayoutUtils::TAknCbaLocation cbaLocation = 
         AknLayoutUtils::CbaLocation();
      if (WFLayoutUtils::LandscapeMode() && !WFLayoutUtils::IsTouchScreen() && 
          (cbaLocation != AknLayoutUtils::EAknCbaLocationBottom)) {
         // If we're in landscape mode, not touch screen and we dont have 
         // the CBAs in the bottom of the screen (like e71) we have to
         // let the top and bottom pane be visible. 
         vectorMapRect = WFLayoutUtils::GetMainPaneRect();
      } else {
         // Otherwise the only system pane we show is the CBA buttons.
         vectorMapRect = WFLayoutUtils::GetFullScreenMinusCBARect();
      }

      iVectorMapContainer->ConstructL(vectorMapRect,
                                      iWayfinderAppUi,
                                      &iMapControl,
                                      &iMapFeatureHolder,
                                      &m_mapInfoControl,
                                      &iVectorMapConnection,
                                      iWayfinderAppUi->iPathManager
                                      ->GetLangResourcePath());
      /*
      if (iCurrentScale != MAX_INT32) {
         iVectorMapContainer->SetZoom(iCurrentScale);
         iVectorMapContainer->SetCenter(iCenter.iY,
                                        iCenter.iX);
      }
      */
      // Try to add the vector map cache. If the username is not set, then
      // the WayFinderAppUi will try to set them when a username arrives.
      tryToAddVectorPrecache();      

#ifdef ENABLE_FORCEFED_ROUTES
      if ( m_forcedRoutes && ( iMapControl != NULL ) ) {
         iMapControl->receiveExtraMaps( m_forcedRoutes->getData(),
                                        m_forcedRoutes->getSize() );
         delete m_forcedRoutes;
         m_forcedRoutes = NULL;
      }
#endif
      
      AppUi()->AddToStackL( *this, iVectorMapContainer );

      if( hasRoute && iWayfinderAppUi->GetRouteId() != 0 ){
         m_routeid = iWayfinderAppUi->GetRouteId();
         iVectorMapContainer->SetRoute( m_routeid );
      }
      /* Check if current position is usable. */
      Nav2Coordinate  nav2Coord(MAX_INT32, MAX_INT32);
      TUint cellRadius = 0;
      if( iRequestType != MapEnums::None ){
         RequestMap(iRequestType, iRequestCoord.iY, iRequestCoord.iX );
      } else if (iWayfinderAppUi->IsGpsAllowed() && !iPreservePositionOnNextActivation && RequestCurrPosVectorMap() ){
         // Make sure we dont hit the if statement below (which reset the pos).
         first = EFalse;
         SetTracking( ETrue );
      } else if( iWayfinderAppUi->GetRouteId() != m_routeid ){
         RequestRouteVectorMap();
      } else if (iWayfinderAppUi->GetCellPosIfAvailable(nav2Coord, cellRadius) && 
                 !iPreservePositionOnNextActivation) {
         // We have a cell id position
         // Get the Cell Position information
         TPoint cellPos(nav2Coord.nav2lon, nav2Coord.nav2lat);
         // Position the map according to the cell infoe
         // and try to adjust the zoom according to the cell size
         iVectorMapContainer->RequestPositionAndZoomMap(cellPos, 
             ConvertCellRadiusToMapScale(cellRadius));
         // Display the Cell Id icon 
         ShowCellIdPosition(nav2Coord, cellRadius);
         // Make sure we dont hit the if statement below (which reset the pos).
         first = EFalse;
         // start the GPS tracking
         SetTracking(ETrue);
      } else if( !iGotMapType ){
         /* No map has been loaded. */
         RequestOriginVectorMap();
      }
      if(iPreservePositionOnNextActivation) {
         iPreservePositionOnNextActivation = EFalse;
      }
      if (first && iRequestType == MapEnums::None) {
         if (iCurrentScale != MAX_INT32) {
            /* Set scale. */
            iVectorMapContainer->SetZoom(iCurrentScale);
            if (HasRoute()) {
               TPoint orig = iWayfinderAppUi->GetOrigin();
               iVectorMapContainer->SetCenter(orig.iY, orig.iX);
            } else {
               iVectorMapContainer->SetCenter(iCenter.iY, /* Remember, latlon is y,x. */
                                              iCenter.iX);
            } 
         }  else {
            iVectorMapContainer->SetCenter(DefaultVectorMapCenterLat, DefaultVectorMapCenterLon);
         }
      }
      /* Make sure that the GPS symbol is visible if necessary. */
      TInt imgId;
      TInt maskId;
      iWayfinderAppUi->GetGpsStatusImageId(imgId, maskId);
      SetGpsStatusImage(imgId, maskId);
      MakeGpsIndicatorVisible(!iWayfinderAppUi->ValidGpsStrength());

      iVectorMapContainer->UpdateRepaint();
      iVectorMapContainer->updateCursorVisibility();
   }
   if( hasRoute && iWayfinderAppUi->CanUseGPSForEveryThing()) {
      SetTopBorder();
      NextStreetCtrlMakeVisible(ETrue);
      if (iNextStreet) {
         UpdateNextStreetL(*iNextStreet);
      }
      SetPicture();
      UpdateDistance( iLastDistance );
   } 
   // Update the visibility of the pedestrian mode indicator.
   iVectorMapContainer->UpdatePedestrianModeIndicator();

   // Get the cacheInfo when loading the view, updating
   // the cache info is done when we are told to do
   // so in handleTileMapEvent.
   if (iMapControl) {
      MapLib* mapLib = iMapControl->getMapLib();
      if (mapLib) {
         iNbrCacheFiles =
            mapLib->getCacheInfo(
               iCacheInfo);
      }
   }
   SetNightModeL( iWayfinderAppUi->IsNightMode() );

   if (MapAsGeneralNavigation() && 
       iCustomMessageId != aCustomMessageId.iUid) {
      // The map view was activated in a non normal way, probably whe screen saver
      // has been deactivated or when having visited phone main menu.
      Set3dMode(Get3dMode());
   } else if (iCustomMessageId == EShowMapFromItineraryView) {
      // This happens when itinerary view selects show turn or 
      // show route from options meny.
      Set3dMode(EFalse);
      SetTracking(EFalse);
   } else {
      // If we don't have GPS here, that means we used to have it but lost
      // it before the route was calculated. Request the map with the last
      // known position, otherwise the center will be the center of the
      // route (box).
      if(hasRoute && 
            (!iWayfinderAppUi->IsGpsConnected() 
             || !iWayfinderAppUi->ValidGpsStrength())) {
         int32 posLat = iCurrentLat;
         int32 posLon = iCurrentLon;
         if((posLat == MAX_INT32) && (posLon == MAX_INT32)) {
            // In case the current position has not yet been set,
            // we use the origin of the route since it is unlikely that
            // the user has managed to move far from it.
         	TPoint origin = iWayfinderAppUi->GetOrigin();
         	posLat = origin.iY;
         	posLon = origin.iX;
         }
         // Call UpdatePosition to make sure that the zoom and angle
         // is correctly set.
         UpdatePosition(Nav2Coordinate(posLat, posLon),
                        0,
                        0,
                        false);
      }      
      Set3dMode(MapAsCarNavigation());
   }

   if (MapAsGeneralNavigation()) {
      SetTracking(ETrue);
      iWayfinderAppUi->ResetDestinationL();
      iVectorMapContainer->StopKeyHandler();
   }
   // Go through the vector containing the cached function calls that 
   // hasnt been fully executed due to VectorMapContainer not yet being
   // created.
   std::for_each(m_ptmfVec.begin(), m_ptmfVec.end(), makeCallback(*this));
   m_ptmfVec.clear();
   
   // Make sure the position of the user shows up when it should, i.e.
   // even in navigation mode when the user does not have a GPS position.
   if(hasRoute && 
         (!iWayfinderAppUi->IsGpsConnected() 
          || !iWayfinderAppUi->ValidGpsStrength())) {
      iVectorMapContainer->ShowUserPos(ETrue);
      iVectorMapContainer->UpdateRepaint();
   }
   else {
      iVectorMapContainer->ShowUserPos(EFalse);
      iVectorMapContainer->UpdateRepaint();
   }
   
   iActivated = ETrue;
}

// ---------------------------------------------------------
//
void CMapView::DoDeactivate()
{
   if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      iVectorMapContainer->SetNightModeL( EFalse );
      if (iMapControl) {
         iMapControl->SetFocus( EFalse );
         iVectorMapContainer->GetCoordinate(iCenter);
         iCurrentScale = iVectorMapContainer->GetScale();
         
         SaveVectorMapCoordinates(iCurrentScale,
                                  iCenter.iY, /* Remember, LatLon is Y,X */
                                  iCenter.iX);
      }
      AppUi()->RemoveFromViewStack( *this, iVectorMapContainer );

      LOGDEL(iVectorMapContainer);
      delete iVectorMapContainer;
      iVectorMapContainer = NULL;
   }

   iRequestType = MapEnums::None;
   iRequestCoord.iY = MAX_INT32;
   iRequestCoord.iX = MAX_INT32;

   // Reset, otherwise if phone is in offline mode, the
   // error wont be shown directly next time user enters
   // mapview.
   iWayfinderAppUi->iErrorFilter->resetParameters(); 
   iWayfinderAppUi->iTimeOutErrorFilter->resetParameters();
   iActivated = EFalse;
}

void
CMapView::GenerateEvent(enum TMapViewEvent aEvent)
{        
   if (iEventGenerator) {
      iEventGenerator->SendEventL(aEvent);
   }
}           

void
CMapView::HandleGeneratedEventL(enum TMapViewEvent aEvent)
{
   switch (aEvent) {
   case EMapViewEventDeleteMapFilesOk:
      DeleteMapFileControlName();
      break;
   case EMapViewEventDeleteMapFilesCancel:
      break;
   case EMapViewEventNoAction:
      break;
   }
}

void CMapView::ShowInstalledMaps()
{
   if (iNbrCacheFiles > 0) {
      if (iMapFileList) {
         iMapFileList->Reset();
         delete iMapFileList;
      }
      iMapFileList = new (ELeave) CDesCArrayFlat(iNbrCacheFiles);
      for (TInt i = 0; i < iNbrCacheFiles; i++) {
         HBufC* tmpFileName = WFTextUtil::Utf8AllocLC(iCacheInfo[i]->
                                                      getNameUTF8());
         iMapFileList->AppendL(*tmpFileName);
         CleanupStack::PopAndDestroy(tmpFileName);
      }
      typedef TCallBackEvent<CMapView, TMapViewEvent> cb_t;
      typedef CCallBackListDialog<cb_t> cbd_t;
      cbd_t::RunDlgLD(cb_t(this,
                           EMapViewEventNoAction,
                           EMapViewEventNoAction),
                      iSelectedMapFile,
                      R_MAP_SHOW_INSTALLED_MAPS,
                      iMapFileList,
                      R_WAYFINDER_CALLBACK_SINGLE_LIST_QUERY_OK);
   } else {
      // No cache files to delete, notify the user.
      typedef TCallBackEvent<CMapView, TMapViewEvent> cb_t;
      typedef CCallBackDialog<cb_t> cbd_t;
      cbd_t::RunDlgLD(cb_t(this,
                           EMapViewEventNoAction,
                           EMapViewEventNoAction),
                      R_MAP_INFO_TITLE,
                      R_MAP_NO_MAPS_AVAILABLE,
                      R_NON_BLOCKING_INFO_MESSAGE);
   }   
}

void CMapView::SearchForMapFiles(TBool aDoSilentMapFilesSearch)
{
   iDoSilentMapFilesSearch = aDoSilentMapFilesSearch;
#ifndef NAV2_CLIENT_SERIES60_V1
   _LIT(KDriveSpec, ":\\");
   if (iMapControl) {
      MapLib* mapLib = iMapControl->getMapLib();
      if (mapLib) {
         mapLib->addEventListener(this);
      }
   } else {
      return;
   }

   if (!iMapFileFinder) {
         iMapFileFinder = CMapFinder::NewL(this, iMapControl->getMapLib());
   }
   CDesCArrayFlat* dirList = new CDesCArrayFlat(15); 
   class CDriveList* driveList = CDriveList::NewLC();
   driveList->GenerateDriveListL();
   for (TInt i = 0; i < driveList->Count(); i++) {
      HBufC* tmpBuf = AllocLC(driveList->Get(i) + 
                              KDriveSpec);
      dirList->AppendL(*tmpBuf);
      CleanupStack::PopAndDestroy(tmpBuf);
   }
   CleanupStack::PopAndDestroy(driveList);
   AddSearchDir(dirList, PathInfo::PhoneMemoryRootPath(),
                PathInfo::GamesPath());
   AddSearchDir(dirList, PathInfo::PhoneMemoryRootPath(),
                PathInfo::InstallsPath());
   AddSearchDir(dirList, PathInfo::PhoneMemoryRootPath(),
                PathInfo::OthersPath());
   AddSearchDir(dirList, PathInfo::PhoneMemoryRootPath(),
                PathInfo::VideosPath());
   AddSearchDir(dirList, PathInfo::PhoneMemoryRootPath(),
                PathInfo::ImagesPath());
   AddSearchDir(dirList, PathInfo::MemoryCardRootPath(),
                PathInfo::GamesPath());
   AddSearchDir(dirList, PathInfo::MemoryCardRootPath(),
                PathInfo::InstallsPath());
   AddSearchDir(dirList, PathInfo::MemoryCardRootPath(),
                PathInfo::OthersPath());
   AddSearchDir(dirList, PathInfo::MemoryCardRootPath(),
                PathInfo::VideosPath());
   AddSearchDir(dirList, PathInfo::MemoryCardRootPath(),
                PathInfo::ImagesPath());

   iMapFileFinder->FindAndMoveFilesL(*dirList, 
                                     iWayfinderAppUi->iPathManager->
                                     GetMapCacheBasePath(),
                                     KMapFileName, 
                                     KMapFileExt);
   dirList->Reset();
   delete dirList;
   dirList = NULL;
#endif
}

void
CMapView::AddSearchDir(CDesCArray* aDirList, 
                       const TDesC& aBasPath,
                       const TDesC& aSecondLevelPath)
{
   HBufC* buf = AllocLC(aBasPath + aSecondLevelPath);
   aDirList->AppendL(*buf);
   CleanupStack::PopAndDestroy(buf);
}

void 
CMapView::DeleteMapFiles()
{
   if (iNbrCacheFiles > 0) {
      if (iMapFileList) {
         iMapFileList->Reset();
         delete iMapFileList;
      }
      iMapFileList = new (ELeave) CDesCArrayFlat(iNbrCacheFiles);
      for (TInt i = 0; i < iNbrCacheFiles; i++) {
         HBufC* tmpFileName = WFTextUtil::Utf8AllocLC(iCacheInfo[i]->
                                                      getNameUTF8());
         iMapFileList->AppendL(*tmpFileName);
         CleanupStack::PopAndDestroy(tmpFileName);
      }
      typedef TCallBackEvent<CMapView, TMapViewEvent> cb_t;
      typedef CCallBackListDialog<cb_t> cbd_t;
      cbd_t::RunDlgLD(cb_t(this,
                           EMapViewEventDeleteMapFilesOk,
                           EMapViewEventDeleteMapFilesCancel),
                      iSelectedMapFile,
                      R_MAP_DELETE_FILE_TITLE,
                      iMapFileList,
                      R_WAYFINDER_CALLBACK_SINGLE_LIST_QUERY);
   } else {
      // No cache files to delete, notify the user.
      typedef TCallBackEvent<CMapView, TMapViewEvent> cb_t;
      typedef CCallBackDialog<cb_t> cbd_t;
      cbd_t::RunDlgLD(cb_t(this,
                           EMapViewEventNoAction,
                           EMapViewEventNoAction),
                      R_MAP_INFO_TITLE,
                      R_MAP_NO_MAPS_AVAILABLE,
                      R_NON_BLOCKING_INFO_MESSAGE);
   }
}

void
CMapView::DeleteMapFileControlName()
{
   HBufC* tmpName = WFTextUtil::Utf8AllocLC(iCacheInfo[iSelectedMapFile]->
                                            getNameUTF8());
   TBool found = ETrue;
   if (!(found = (iMapFileList->MdcaPoint(iSelectedMapFile) == *tmpName))) {
      // The indexes in iMapFileList and in iCacheInfo does
      // not match each other, we have to search to find
      // the correct path.
      //found = EFalse;
      TPtrC selectedName = iMapFileList->MdcaPoint(iSelectedMapFile);
      for (TInt i = 0; i < iNbrCacheFiles; i++) {
         // Iterate through the lists and 
         HBufC* name = WFTextUtil::Utf8AllocLC(iCacheInfo[i]->
                                               getNameUTF8());
         if (selectedName == *name) {
            // Name matches.
            iSelectedMapFile = i;
            found = ETrue;
         }
         CleanupStack::PopAndDestroy(name);
      }
   }
   if (found) {
      // We have found a path to delete.
      if (!iMapFileFinder) {
         iMapFileFinder = CMapFinder::NewL(this, iMapControl->getMapLib());
      }
      iMapFileFinder->DeleteFileL(iCacheInfo[iSelectedMapFile]);
   } else {
      // Something strange happened.
      // XXX Do somethin smart.
   }
   CleanupStack::PopAndDestroy(tmpName);
}

void 
CMapView::FindAndMoveFilesCompleted(TInt aNbrFoundFiles)
{
   iWayfinderAppUi->StopWaitDlg();
   if (iDoSilentMapFilesSearch) {
      iDoSilentMapFilesSearch = EFalse;
      return;
   }
   _LIT(KSpace, " ");
   HBufC* text;
   if (aNbrFoundFiles < 1) {
      text = CEikonEnv::Static()->
         AllocReadResourceL(R_MAP_NO_MAPS_AVAILABLE);
   } else if(aNbrFoundFiles < 2) {
      HBufC* tmpText = CEikonEnv::Static()->
         AllocReadResourceLC(R_MAP_NEW_MAP_INSTALLED);         
      text = HBufC::NewL(tmpText->Length() + 10);
      text->Des().Num(aNbrFoundFiles);
      text->Des().Append(KSpace);
      text->Des().Append(*tmpText);
      CleanupStack::PopAndDestroy(tmpText);
   } else {
      HBufC* tmpText = CEikonEnv::Static()->
         AllocReadResourceLC(R_MAP_NEW_MAPS_INSTALLED);
      text = HBufC::NewL(tmpText->Length() + 10);
      text->Des().Num(aNbrFoundFiles);
      text->Des().Append(KSpace);
      text->Des().Append(*tmpText);
      CleanupStack::PopAndDestroy(tmpText);
   }
   typedef TCallBackEvent<CMapView, TMapViewEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
                        EMapViewEventNoAction,
                        EMapViewEventNoAction),
                   R_MAP_SEARCH_FILES_RESULT_TITLE,
                   text,
                   R_NON_BLOCKING_INFO_MESSAGE);
}

void CMapView::NbrMapFilesFound(TInt aCount)
{
   if (aCount > 0) {
      iWayfinderAppUi->StartWaitDlg(this, R_MAP_SHOW_INSTALLED_MAPS);
   }
}

void
CMapView::DeleteFileCompleted(TInt aResult)
{
   TInt title;
   TInt text;
   if (aResult != KErrNone) {
      title = R_MAP_DELETE_MAP_FILE_FAILED_TITLE;
      text = R_MAP_DELETE_MAP_FILE_FAILED_TEXT;
   } else {
      title = R_MAP_INFO_TITLE;
      text = R_MAP_DELETE_FILE_SUCCESS;
   }
   typedef TCallBackEvent<CMapView, TMapViewEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
                        EMapViewEventNoAction,
                        EMapViewEventNoAction),
                   title,
                   text,
                   R_NON_BLOCKING_INFO_MESSAGE);
}

void 
CMapView::CopyFileCompleted(TInt /*aResult*/)
{
}

void 
CMapView::MapFileFinderBusy()
{
}

void 
CMapView::RequestAborted()
{
}

int32 CMapView::GetScale() 
{
   return iCurrentScale;
}

TPoint CMapView::GetCenter() 
{
   return iCenter;
}

void
CMapView::HandleResourceChangeL(TInt /*aType*/)
{
//    if (aType == KEikDynamicLayoutVariantSwitch) {
//       iVectorMapContainer->HandleResourceChange(aType);      
//    }
}

void 
CMapView::SetNightModeL(TBool aNightMode)
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetNightModeL(aNightMode);
      
      if( iWayfinderAppUi->IsGpsConnected() ){
         SetPicture();
         /* Forces a distance update */
         UpdateDistance(0);
         UpdateDistance(iLastDistance);
      }
   }
}

TBool CMapView::IsNightMode()
{
   return iWayfinderAppUi->IsNightMode();
}

void CMapView::Set3dMode(TBool aOn)
{
   if (!iWayfinderAppUi->IsIronVersion()) {
      // if it is iron (earth) no 3d mode is allowed
      if (iMapControl) {
         iMapControl->getMapLib()->set3dMode(aOn);
         iVectorMapContainer->Set3dMode( aOn );
      }
      if( aOn ){
         SetTracking( ETrue );
      }
   }
}

TBool CMapView::Get3dMode()
{
   if (iMapControl) {
      return iMapControl->getMapLib()->get3dMode();
   } else {
      return EFalse;
   }
}

TBool CMapView::MapAsGeneralNavigation()
{
   return (iCustomMessageId == EMapAsNavigation ||
           iCustomMessageId == EToggleRingCycle);
}

TBool CMapView::MapAsCarNavigation()
{
   return (!iWayfinderAppUi->InPedestrianMode() && 
           (iCustomMessageId == EMapAsNavigation ||
            iCustomMessageId == EToggleRingCycle));
}

TBool CMapView::MapAsPedestrianNavigation()
{
   return (HasRoute() && iWayfinderAppUi->InPedestrianMode() && 
           (iCustomMessageId == EMapAsNavigation ||
            iCustomMessageId == EToggleRingCycle));
}

TBool CMapView::HasRoute() 
{
   return iWayfinderAppUi->HasRoute();
}

void CMapView::GetCoordinate(TPoint& aRealCoord)
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->GetCoordinate(aRealCoord);
   } else {
      aRealCoord.iY = 0;
      aRealCoord.iX = 0;
   }
}

TInt& CMapView::GetDpiCorrFact()
{
   return iDpiCorrFact;
}

NavServerComEnums::languageCode CMapView::GetLanguageCode()
{
   return iWayfinderAppUi->GetLanguageCode();
}

void CMapView::CancelPressed() 
{
   iMapFileFinder->CancelOperation();
}

void CMapView::DialogDismissedFromFramework()
{
   // Empty for now.  
}

void CMapView::SetRouteIdL(TInt64 aRouteId)
{
   m_routeid = aRouteId;
   if (iMapControl && iMapControl->getMapLib()) {
      // By accessing the MapLib directly when the VectorMapContainer
      // isnt fully created (which is usually not is when this function)
      // is called the map is already updated with the removed red line
      // when user enters the map. That is not the case when needed
      // to cache the request and calling the VectorMapContainer::SetRoute.
      iMapControl->getMapLib()->setRouteID(RouteID(aRouteId));
   } else {
      m_ptmfVec.push_back(&CMapView::SetRouteIdL);
   }
}


void CMapView::SetRouteIdL()
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetRoute(m_routeid);
   } else if (iMapControl && iMapControl->getMapLib()) {
      iMapControl->getMapLib()->setRouteID(RouteID(m_routeid));      
   } else {
      // Not much to do!
   }
}

void CMapView::HandleRouteTooLongL()
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetRoute(0);
      iVectorMapContainer->UpdateRepaint();
      Set3dMode(EFalse);
   } else {
      m_ptmfVec.push_back(&CMapView::HandleRouteTooLongL);
   }
}

void CMapView::NavigateToCallback()
{
   iWayfinderAppUi->SetFromGps();
   //HandleCommandL(EWayFinderCmdMapSetDestination);
   RouteToMapDestinationCoordL();
}

void CMapView::WrapperToNavigateToCallback(void* pt2Object)
{
   CMapView* self = (CMapView*) pt2Object;
   self->NavigateToCallback();
}

TUint CMapView::ConvertCellRadiusToMapScale(TUint aRadius)
{
   // if the Cell radius is less than 2000m (2k) 
   // then 1k scale will show circle
   if ( aRadius < 2000 ) {
	   return CVectorMapContainer::EScale4;
   }
   // cell size < 5k scale 2k
   else if ( aRadius < 5000 ) {
	   return CVectorMapContainer::EScale5;
   }
   // cell size < 10k scale 5k
   else if ( aRadius < 10000 ) {
	   return CVectorMapContainer::EScale6;
   }
   // cell size < 20k scale 10k
   else if ( aRadius < 20000 ) {
	   return CVectorMapContainer::EScale7;
   }
   else {	
   // default to 20k scale 
       return CVectorMapContainer::EScale8;
   }
}

void CMapView::ShowCellIdPositionIfKnown()
{
   Nav2Coordinate  navCoOrd(MAX_INT32,MAX_INT32);
   TUint cellRadius = 0;

   // Is there a cell id position
   if ( iWayfinderAppUi->GetCellPosIfAvailable(navCoOrd, cellRadius) ) {
      // Display the Cell Id icon 
      ShowCellIdPosition(navCoOrd, cellRadius);
   }
   else {
      if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
         iVectorMapContainer->SetCellIdIconEnabled(EFalse);
      }
   }
}

void CMapView::ShowCellIdPosition(const Nav2Coordinate& aNavCoOrd, const TUint aCellRadius)
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      // Display the Cell Id icon 
      MC2Coordinate mc2CoOrd( aNavCoOrd );
      iVectorMapContainer->UpdateCellIdIconDimensions(aCellRadius);
      iVectorMapContainer->UpdateCellIdPosition(mc2CoOrd);
      iVectorMapContainer->SetCellIdIconEnabled(ETrue);  
   }
}

void CMapView::PreservePositionOnNextActivation()
{
   iPreservePositionOnNextActivation = ETrue;
}

// End of File

