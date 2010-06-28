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
#include "DetailParser.h"

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

#include "TraceMacros.h"
#include "DetailFetcher.h"
#include "FavoriteParser.h"

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
#include "MapResourceFactory.h"

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

#include "FavoriteFetcher.h"

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
#ifdef NAV2_CLIENT_SERIES60_V3
const TInt MbmfileIds [] = {
   EMbmWficons00,
   EMbmWficons01,
   EMbmWficons02,
   EMbmWficons03,                                   
   EMbmWficons04,
   EMbmWficons05,
   EMbmWficons06,
   EMbmWficons07,
   EMbmWficons08,
   EMbmWficons09,
   EMbmWficonsD,
   EMbmWficonsNumber_dot2,
   EMbmWficonsF,
   EMbmWficonsI,
   EMbmWficonsK,
   EMbmWficonsM,
   EMbmWficonsT,
   EMbmWficonsY,
   EMbmWficonsBlank,
   EMbmWficonsRight_cap,
   EMbmWficonsLeft_cap,
   EMbmWficonsBluebar,
   EMbmWficonsBlank_mask,
   EMbmWficonsRight_cap_mask,
   EMbmWficonsLeft_cap_mask
};

const TReal MbmfileRelations [] = {
   2.14,
   3.33,
   2.14,
   2.14,
   2.14,
   2.14,
   2.14,
   2.31,
   2.14,
   2.14,
   2.0,
   3.75,
   3.0,
   5.0,
   2.0,
   1.58,
   3.0,
   2.31,
   6.0,
   6.0,
   0.21,
   0.86
};

#else
const TInt MbmfileIds [] = {
   EMbmWficonsNumber_0,
   EMbmWficonsNumber_1,
   EMbmWficonsNumber_2,
   EMbmWficonsNumber_3,                                   
   EMbmWficonsNumber_4,
   EMbmWficonsNumber_5,
   EMbmWficonsNumber_6,
   EMbmWficonsNumber_7,
   EMbmWficonsNumber_8,
   EMbmWficonsNumber_9,
   EMbmWficonsNumber_d,
   EMbmWficonsNumber_dot2,
   EMbmWficonsNumber_f,
   EMbmWficonsNumber_i,
   EMbmWficonsNumber_k,
   EMbmWficonsNumber_m2,
   EMbmWficonsNumber_t,
   EMbmWficonsNumber_y,
   EMbmWficonsNumber_infinite,
   EMbmWficonsNumber_border_right,
   EMbmWficonsNumber_border_left,
   EMbmWficonsMap_top_border,
   EMbmWficonsMedium_mask,
   EMbmWficonsNumber_border_right, // This is just for space filling between V3 and other phones.
   EMbmWficonsNumber_border_left,  // This is just for space filling between V3 and other phones.
};
#endif
const TInt cFullScreenOffset = 5;

using namespace isab;
using namespace MapEnums;
using namespace RouteEnums;


// ================= MEMBER FUNCTIONS =======================

CMapView::CMapView(isab::Log* aLog) : iLog(aLog),
                                      m_detailFetcher(0)
{
   

}
// ---------------------------------------------------------
// CMapView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CMapView::ConstructL( CWayFinderAppUi* aWayFinderUI)
{
   BaseConstructL( R_WAYFINDER_MAP_VIEW );
   iWayFinderUI = aWayFinderUI;
   iShowExtendedDetails = false;

   m_detailFetcher = 0;
   
   /*
   iOdometerFontCache =
      CDistanceBitmapHelper::NewL( iWayFinderUI->iPathManager->GetMbmName(), MbmfileIds );
   */
   SetCenter(MAX_INT32, MAX_INT32);
   SetScale(MAX_INT32);

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

   iMapCacheSize = 1;
   
   iTrackingType = ERotating;

#ifdef _MSC_VER
   iFullScreenMapName.Copy(iWayFinderUI->iPathManager->GetWayfinderPath());
   iFullScreenMapName.Append(KGifFullScreenMap);
   iNormalScreenMapName.Copy(iWayFinderUI->iPathManager->GetWayfinderPath());
   iNormalScreenMapName.Append(KGifNormalMap);
#else
   iFullScreenMapName.Copy( _L("d:\\system\\temp\\") );
   iFullScreenMapName.Append(KGifFullScreenMap);
   iNormalScreenMapName.Copy( _L("d:\\system\\temp\\") );
   iNormalScreenMapName.Append(KGifNormalMap);
#endif
   iEventGenerator = CMapViewEventGenerator::NewL(*this);
}

CMapView* CMapView::NewLC(CWayFinderAppUi* aUi,
                          isab::Log* aLog)
{
   CMapView* self = new (ELeave) CMapView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aUi);
   return self;
}

class CMapView* CMapView::NewL(CWayFinderAppUi* aUi,
                               isab::Log* aLog)
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
   TRACE_FUNC1("Entering map view destructor.");
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
}

void CMapView::ReleaseMapControls()
{
   if( iMapControl ){
      // Trap added here to prevent CBase-66
      TRAPD(result, delete iMapControl; )
      iMapControl = NULL;
   }

   delete m_mapInfoControl;
   m_mapInfoControl = NULL;
   
   delete iMapFeatureHolder;
   iMapFeatureHolder = NULL;

   if (iVectorMapContainer) {
      iVectorMapContainer->SetMapControlsNull();
   }
}

void
CMapView::SetDetailFetcher( DetailFetcher* detailFetcher )
{
   m_detailFetcher = detailFetcher;
}


#define VM_COORD_VERSION 2
void
CMapView::SetVectorMapCoordinates(int32 scale, int32 lat, int32 lon)
{
   SetScale(scale);
   SetCenter(lat, lon);
   
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetZoom(iCurrentScale);
      iVectorMapContainer->SetCenter(
            iCenter.iY, /* Remember, latlon is y,x. */
            iCenter.iX);
   }
}

void
CMapView::SaveVectorMapCoordinates(int32 scale, int32 lat, int32 lon)
{
   SetScale(scale);
   SetCenter(lat, lon);

   VectorMapCoordinates* vmc =
      new (ELeave) VectorMapCoordinates(scale, lat, lon);

   if (vmc->Valid()) {
      GeneralParameterMess* gpm = vmc->CreateGeneralParameter();
      iWayFinderUI->SendMessageL( gpm );
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
   iWayFinderUI->SendMessageL( gpm );
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
      if (iMapControl->Handler().updateLayerInfo(*paramVector)) {
         /* Rewrite settings to NAv2. */
         /* Transfer ownership of paramVector... */
         ConvertMapLayerSettingsToNav2Parameter(paramVector, aSettingsData);
         /* Write settings when parameter returns. */
         delete buf;
         return;
      }
   }

   

   /* Vector wasn't changed by maplib, copy relevant settings */
   /* to SettingsData. */

   TileMapLayerInfoVector::iterator it =
      std::find_if(paramVector->begin(), paramVector->end(),
            IsTheOne);

   if (it != paramVector->end()) {
      aSettingsData->iShowTraffic = ETrue;
      aSettingsData->iTrafficTime.Num((*it).getUpdatePeriodMinutes(),
            EDecimal);
      aSettingsData->iTrafficOldTime.Num((*it).getUpdatePeriodMinutes(),
            EDecimal);
      aSettingsData->iTrafficEnabled = (*it).isVisible();
      aSettingsData->iTrafficOldEnabled = (*it).isVisible();
   }
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
      (*it).setVisible(aSettingsData->iTrafficEnabled);
      char* foo = WFTextUtil::newTDesDupL(aSettingsData->iTrafficTime);
      TInt val = atoi(foo);
      delete[] foo;
      (*it).setUpdatePeriodMinutes(val);
   }

   ConvertMapLayerSettingsToNav2Parameter(tmp, aSettingsData);
}

CSettingsData*
CMapView::GetSettingsData()
{
   return iWayFinderUI->iSettingsData;
}

void
CMapView::ToggleSoftKeys(TBool on)
{
   Cba()->MakeVisible(on);
}

TBool
CMapView::DontHandleAsterisk()
{
   return iWayFinderUI->DontHandleAsterisk();
}

void
CMapView::Call()
{
   const FullSearchItem* item = m_detailFetcher->GetLatestMessage();
   
   if (item) {
      /* Find a phone number. */

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

         iWayFinderUI->DialNumberL( temp, EWayFinderCmdMapCall );
      }
   }
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

void CMapView::DetailsReceived(isab::FullSearchItem* details,
                                bool cachedResult)
{
   if (iVectorMapContainer->ShowingInfoText()) {
      HandleCommandL(EWayFinderCmdMapShowInfo);
   } else {
      ShowDetailsL(details);
   }

   delete details;
}

void
CMapView::GetAndShowDetails(const char* aItemId,
                            const char* aItemName)
{
   if(iVectorMapContainer->ShowingInfoText() &&
      !m_detailFetcher->HasMessageCached(aItemId))
   {
      /* Can't get more details if there are none. */
      if (iVectorMapContainer &&
          iVectorMapContainer->ConstructDone()) {
         
         /* Remove text-note. */
         iVectorMapContainer->setInfoText(NULL);
      }
      
      return;
   }

   m_detailFetcher->GetDetails( aItemId, aItemName, this);
}

void
CMapView::ShowDetailsL( isab::FullSearchItem* item )
{
   //TODO: aMessage->size() <= 0 check
   if(  !iVectorMapContainer ){
      return;
   }
   
   if(!iVectorMapContainer->ConstructDone() ) {
      return;
   }
   
   if (!iVectorMapContainer->IsInfoTextOn()) {
      /* Info text removed, don't show the new information, the user */
      /* "clicked" it away! */
      return;
   }
   
   DetailParser detailParser(iCoeEnv);
   detailParser.parseDetails(item);
   char* out = detailParser.getParsedDetails();

   if (strlen(out) > 0) {
      if (detailParser.hasSensibleData()) {
         iVectorMapContainer->setInfoText(out,
                                          detailParser.getLeftCba(),
                                          detailParser.getRightCba());
         iVectorMapContainer->UpdateRepaint();
      } else {
         /* No new interesting data. */
         /* Don't change the current note. */
      }
   } else {
      iVectorMapContainer->setInfoText("Empty search details!");
   }
}

void
CMapView::HandleFavoriteInfoReply( GetFavoriteInfoReplyMess* aMessage )
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()){
      FavoriteParser f(aMessage);
      f.parseDetails();
      iVectorMapContainer->setInfoText(f.getParsedData());
   }
}

void
CMapView::FavoriteChanged()
{
   if (iMapFeatureHolder) {
      GenericGuiMess get(GuiProtEnums::GET_FAVORITES_ALL_DATA, uint16(0),
                         uint16(MAX_UINT16));
      iWayFinderUI->SendMessageL(&get);
   }
}

void
CMapView::HandleFavoritesAllDataReply( GetFavoritesAllDataReplyMess* aMessage )
{
   if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      favorite_vector_type* favVector = aMessage->getFavorites();
      if (iMapFeatureHolder)  {
         iMapFeatureHolder->UpdateFavorites(favVector);
         iVectorMapContainer->UpdateRepaint();
      }
   }
}

void
CMapView::NewRouteDownloaded()
{
   m_routeid = iWayFinderUI->GetRouteId();
   SetMapRequestData( MapEnums::OverviewRoute, MAX_INT32, MAX_INT32 );
   if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      iVectorMapContainer->SetRoute( m_routeid );
      iVectorMapContainer->ShowEnd(EFalse, 0, 0);
      iVectorMapContainer->ShowStart(EFalse, 0, 0);
      iVectorMapContainer->ShowPoint(EFalse, 0, 0);
      iVectorMapContainer->UpdateRepaint();
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
   }
}


TPtrC 
CMapView::GetMbmName()
{
   return iWayFinderUI->iPathManager->GetMbmName();
}

const TInt* CMapView::GetMbmfileIds()
{
   return MbmfileIds;
}

TPtrC 
CMapView::GetMapCachePath()
{
   return iWayFinderUI->iPathManager->GetMapCacheBasePath();
}

TPtrC 
CMapView::GetCommonDataPath()
{
   return iWayFinderUI->iPathManager->GetCommonDataPath();
}

#if defined NAV2_CLIENT_SERIES60_V3
const TReal* CMapView::GetMbmFileRelations()
{
   return MbmfileRelations;
}
#endif

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
      if (iWayFinderUI->UseTrackingOnAuto()) {
         SetTracking(gps_on);
      }
   }
}

TBool
CMapView::SetConStatusImage(CFbsBitmap* bmp, CFbsBitmap* mask)
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone() && 
       iVectorMapContainer->IsFullScreen())
   {
      iVectorMapContainer->SetConStatusImage(bmp, mask);
      return ETrue;
   } else {
      return EFalse;
   }
}
void
CMapView::SetGpsStatusImage(CFbsBitmap* bmp, CFbsBitmap* mask)
{
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone() &&
       iVectorMapContainer->IsFullScreen()) {
      iVectorMapContainer->SetGpsStatusImage(bmp, mask);
   } else {
      /* Not necessary. */
      delete bmp;
      delete mask;
   }
}

TBool
CMapView::UseTrackingOnAuto()
{
   return iWayFinderUI->UseTrackingOnAuto();
}

GuiDataStore*
CMapView::GetGuiDataStore()
{
   return iWayFinderUI->GetGuiDataStore();
}

void CMapView::GetTempBaseDirectory( TDes& baseName )
{
   baseName.Copy( _L("d:\\system\\temp\\") );
}

void CMapView::GetBaseDirectory( TDes& baseName )
{
   baseName.Copy(iWayFinderUI->iPathManager->GetWayfinderPath());
}

void CMapView::UpdateDataL( UpdatePositionMess* aPositionMess,
                            TInt aSpeed )
{
   iCurrentLat = aPositionMess->getLat();
   iCurrentLon = aPositionMess->getLon();
   //   TInt heading = -1;
   TInt realHeading = -1;
   if( aPositionMess->headingQuality() > QualityUseless ){
      realHeading = aPositionMess->getHeading();
   }
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      TInt heading360 = 0;
      if( realHeading > 0 ){
         heading360 = TInt(realHeading * ( 360.0 / 256.0 ) + 0.5);
      }
      
      int zoomScale = GetZoomScale( aSpeed );

      iVectorMapContainer->ShowUserPos( ETrue );
      if ( iVectorMapContainer->setGpsPos( 
               Nav2Coordinate( iCurrentLat, iCurrentLon ),
               (int) heading360,
               zoomScale ) ) {
         if ( iIsTracking ) {
            // Show feature info in the blue note.
            iVectorMapContainer->ShowFeatureInfo();
         }
         iVectorMapContainer->UpdateRepaint();
         
      }
      if ( iIsTracking ) {
         if (iWayFinderUI->iSettingsData->iBacklightStrategy ==
               GuiProtEnums::backlight_on_during_route) {
            User::ResetInactivityTime();
         }
         // XXX: This was here before. Don't know what it's used for.
         iVectorMapContainer->Connect();
         iGotMapType = ETrue;
      }

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
         targetScale = 2;
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
         targetScale = 40;
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
      if( !iIsTracking ){
         iVectorMapContainer->ShowUserPos( EFalse );
         iVectorMapContainer->UpdateRepaint();
      }
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
                              TBool aHighway,
                              TInt detour, TInt speedcam)
{
   iCurrentTurn = TTurnPictures::GetPicture( aAction, aCrossing, aDistance, aHighway );
   iLeftSideTraffic = aLeftSideTraffic;
   SetTopBorder();
   SetPicture();
   UpdateDistance( aDistance );
   if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetDetourPicture( detour );
      iVectorMapContainer->SetSpeedCamPicture( speedcam );
   }
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


TInt
CMapView::GetDistanceMode()
{
   return iWayFinderUI->GetDistanceMode();
}
void
CMapView::SetDistanceMode(TInt mode)
{
   if (iMapFeatureHolder) {
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

void CMapView::SetPicture()
{
   TInt mbmIndex = TTurnPictures::GetMediumTurnPicture( iCurrentTurn, iLeftSideTraffic );
   if( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      iVectorMapContainer->SetTurnPictureL( mbmIndex );
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

      if(iWayFinderUI->iLog) {
         iWayFinderUI->iLog->debug("Received multivectormapreply with id %u", 
                                   aMessage->getMessageID());
      }
      if( iVectorMapConnection ){
         if(iWayFinderUI->iLog){
            iWayFinderUI->iLog->debug("VectorMapConnection present");
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
   return iWayFinderUI->IsGpsAllowed();
}

TBool
CMapView::IsGpsConnected()
{
   return iWayFinderUI->IsGpsConnected();
}

TBool
CMapView::IsReleaseVersion()
{
   return iWayFinderUI->IsReleaseVersion();
}

TBool
CMapView::IsIronVersion()
{
   return iWayFinderUI->IsIronVersion();
}

TBool CMapView::IsTracking()
{
   return iIsTracking;
}

TBool CMapView::IsSimulating()
{
   return iWayFinderUI->IsSimulating();
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


void CMapView::FetchCurrentMapFeatureName() 
{
   const unsigned char* aName =
      iVectorMapContainer->GetFeatureName();

   SetMapFeatureName(aName);
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
      iVectorMapContainer->setTrackingMode( iIsTracking, iTrackingType );
      iVectorMapContainer->UpdateRepaint();
   }
}

void CMapView::SetTracking(TBool aOnOff)
{
   if( iIsTracking != aOnOff ){
      if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() && 
           iWayFinderUI->CanUseGPSForEveryThing() ) {
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
   if( iWayFinderUI->HasRoute() &&
       iWayFinderUI->GetRouteId() != 0 ) {
      m_routeid = iWayFinderUI->GetRouteId();
      if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
         iVectorMapContainer->SetRoute( m_routeid );
         TPoint tl;
         TPoint br;
         iWayFinderUI->GetRouteBoundingBox( tl, br );
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
   TPoint pos = iWayFinderUI->GetCurrentPosition();
   if (iWayFinderUI->CanUseGPSForEveryThing() &&
       pos.iY != MAX_INT32 && pos.iX != MAX_INT32) {
      /* Postion is ok. */
      iVectorMapContainer->RequestPositionMap( pos );
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
   if (iWayFinderUI->IsOriginSet()) {
      /* Got origin*/
      TPoint origin = iWayFinderUI->GetOrigin();
      iVectorMapContainer->ShowPoint( ETrue, origin.iY, origin.iX );
      iVectorMapContainer->RequestPositionMap( origin );
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

void CMapView::SetFullScreen()
{
   if(iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
      iVectorMapContainer->SetFullScreen(ETrue);
      if (iMapControl) {
         iMapControl->SetFocus( ETrue );
      }
      iVectorMapContainer->UpdateRepaint();
   }
}

void
CMapView::handleTileMapEvent(const class TileMapEvent &event)
{
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
            iNbrCacheFiles =
               mapLib->getCacheInfo(iCacheInfo);
         }
      }
      break;
   }
}

void
CMapView::DialogDismissedL( TInt /*aButtonId*/ )
{
}


Favorite* CMapView::CreateFavorite( TUint aId, TInt aLat, TInt aLon,
                                    TDesC &aName, TDesC &aDescription,
                                    const char* aIcon, TBool aHasId )
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

void
CMapView::HandleCommandL(const class CWAXParameterContainer& aCont)
{
/*    WAXParameter* icon = aCont.GetParameter(KIcon); */

   Nav2Coordinate* coord = WAXParameterTools::GetCoordinate(aCont);
   if (!coord) {
      /* No coordinates. */
      return;
   }

   iWayFinderUI->RequestMap(MapEnums::UnmarkedPosition, coord->nav2lat, coord->nav2lon);

   TInt32 zoom = WAXParameterTools::GetZoomScale(aCont);
   if (zoom > 0) {
      SetScale(zoom);
   }
   delete coord;
}


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
#if 0
      case EWayFinderCmdHelp:
         {
/*             if (iVectorMapContainer) { */
/*                iVectorMapContainer->SetDistanceL(-5); */
/*             } */
            TInt resourceId1, resourceId2, resourceId3, resourceId4;
            if (IsGpsAllowed()) {
               resourceId1 = R_MAP_HELP_GPS_1;
               resourceId4 = R_MAP_HELP_GPS_2;
            } else {
               resourceId1 = R_MAP_HELP_NO_GPS_1;
               resourceId4 = 0;
            }
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               resourceId2 = R_MAP_HELP_COMMON_1_MAP;
               resourceId3 = R_MAP_HELP_COMMON_2_MAP;
            } 
            else {
               resourceId2 = 0;
               resourceId3 = 0;
            }
            iWayFinderUI->ShowHelpL(resourceId1, resourceId2, resourceId3, EFalse, resourceId4);
            break;
         }
#endif
      case EAknSoftkeyExit:
         {
            if (WFDialog::ShowQueryL(R_WAYFINDER_CONFIRM_EXIT_MSG, iCoeEnv)) {
               iWayFinderUI->ShutdownNow();
            }
         }
         break;
      case EWayFinderSoftkeyStart:
      case EAknSoftkeyBack:
         {
            if( iCustomMessage == ESetOrigin ||
                iCustomMessage == ESetDestination ) {
               AppUi()->HandleCommandL( EWayFinderCmdPositionSelect );
            } else {
               AppUi()->HandleCommandL( aCommand );
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
            SetFullScreen();
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
            if( iWayFinderUI->HasRoute() && iRouteDestLat != MAX_INT32
                && iRouteDestLon != MAX_INT32 ){
               RequestMap( MapEnums::UnmarkedPosition,
                           iRouteDestLat, iRouteDestLon );
            }
            else if( iWayFinderUI->IsDestinationSet() ){
               TPoint t1 = iWayFinderUI->GetDestination();
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
            if (iWayFinderUI->IsOriginSet()) {
               TPoint orig = iWayFinderUI->GetOrigin();
               RequestMap( MapEnums::UnmarkedPosition, orig.iY, orig.iX );
            }
            else{
               /* No origin set? */
               /* Impossible, we can't get this command then. */
            }
            break;
         }
      case EWayFinderCmdMapViewTrackingOn:
         {
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
            if (iWayFinderUI->IsGpsAllowed() ) {
               AppUi()->HandleCommandL( EWayFinderCmdMapReroute );
            } else {
               AppUi()->HandleCommandL( EWayFinderCmdMapVoice );
            }
            break;
         }
      case EWayFinderCmdMapRouteTo:
         {
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               TPoint position;
               iVectorMapContainer->GetCoordinate( position );
               //iFeatureName will now hold feature name
               FetchCurrentMapFeatureName();
               iWayFinderUI->RouteToCoordinateL( position.iY, position.iX, iFeatureName );
            }
            break;
         }
      case EWayFinderCmdMapShowInfo:
         {
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               MC2Coordinate position;
               iVectorMapContainer->GetCoordinate(position);
               HBufC* serverString = iVectorMapContainer->GetServerStringL();
               iWayFinderUI->GotoInfoInServiceViewL(position.lat, position.lon, serverString);
               delete serverString;
            }
            break;
         }
      case EWayFinderCmdMapSetDestination:
         {
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               TPoint position;
               iVectorMapContainer->GetCoordinate( position );
               HBufC* tmp;
               tmp = iCoeEnv->AllocReadResourceLC( R_POSITIONSELECT_FROM_MAP_TEXT );
               iWayFinderUI->SetDestination( GuiProtEnums::PositionTypePosition,
                                             *tmp, NULL, position.iY, position.iX );
               CleanupStack::PopAndDestroy(tmp);
               iWayFinderUI->PositionSelectRoute();
               iSetDestination = EFalse;
               iVectorMapContainer->ShowEnd(ETrue, position.iY, position.iX);
               iVectorMapContainer->UpdateRepaint();
            }
            break;
         }
      case EWayFinderCmdMapSetOrigin:
         {
            if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
               TPoint position;
               HBufC* tmp;
               tmp = iCoeEnv->AllocReadResourceLC( R_POSITIONSELECT_FROM_MAP_TEXT );

               iVectorMapContainer->GetCoordinate( position );
               iWayFinderUI->SetOrigin( GuiProtEnums::PositionTypePosition,
                                        *tmp, "", position.iY, position.iX );
               CleanupStack::PopAndDestroy(tmp);
               iWayFinderUI->PositionSelectRoute();
               iSetOrigin = EFalse;
               iVectorMapContainer->ShowStart(ETrue, position.iY, position.iX);
               iVectorMapContainer->UpdateRepaint();
            }
            break;
         }
#if 0
      case EWayFinderCmdMapRoute:
         {
            iWayFinderUI->PositionSelectRoute();

            break;
         }
#endif
      case EWayFinderCmdMapSaveFavorite:
         if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
/*             iVectorMapContainer->SetDistanceL(999999999); */
/*             break; */
            TPoint position;
            iVectorMapContainer->GetCoordinate( position );
            FetchCurrentMapFeatureName();
            AddFavoriteMess* message = 
               new (ELeave) AddFavoriteMess( CreateFavorite( 0,
                                                             position.iY,
                                                             position.iX,
                                                             iFeatureName,
                                                             iFeatureName, 
                                                             "", EFalse ) );
            LOGNEW(message, AddFavoriteMess);
            iWayFinderUI->SendMessageL( message );
            message->deleteMembers();
            LOGDEL(message);
            delete message;
         }
         break;
      case EWayFinderCmdConnectSendCurrentPosition:
         if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
            TPoint *position = new (ELeave) TPoint;
            /* position will transfer to Connect Dialog's ownership! */

            if (IsTracking() &&
                iWayFinderUI->IsGpsAllowed() &&
                iCurrentLat != MAX_INT32 &&
                iCurrentLon != MAX_INT32 ){
               (*position) = iWayFinderUI->GetCurrentPosition();
            } else {
               iVectorMapContainer->GetCoordinate( *position );
            }
#ifndef NAV2_CLIENT_SERIES60_V3
            iWayFinderUI->ShowConnectDialog(NULL,
                                            CConnectDialog::DefaultTransmitMode,
                                            CConnectDialog::Position,
                                            CConnectDialog::DefaultMessageFormat,
                                            position);
#endif
         }
         break;
      case EWayFinderCmdConnectSendThisPosition:
         if (iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
            TPoint *position = new (ELeave) TPoint;
            /* position will transfer to Connect Dialog's ownership! */

            iVectorMapContainer->GetCoordinate( *position );
#ifndef NAV2_CLIENT_SERIES60_V3
            iWayFinderUI->ShowConnectDialog(NULL,
                                            CConnectDialog::DefaultTransmitMode,
                                            CConnectDialog::Position,
                                            CConnectDialog::DefaultMessageFormat,
                                            position);
#endif
         }
         break;
      case EWayFinderCmdStartPageUpgradeInfo:
      case EWayFinderCmdStartPageUpgradeInfo2:
         {
            iWayFinderUI->ShowUpgradeInfoL(CWayFinderAppUi::EUpgrade, Id());
            break;
         }
      case EWayFinderCmdGuide:
         {
            if (iWayFinderUI->HasRoute()) {
               AppUi()->HandleCommandL( aCommand );
            } else {
               /* Don't move to Guide view if we have no route. */
            }
            break;
         }
      case EWayFinderCmdDestination:
         {
            if (iWayFinderUI->HasRoute()) {
               AppUi()->HandleCommandL( aCommand );
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
      default:
         {
            AppUi()->HandleCommandL( aCommand );
            break;
         }
      }
}

// ---------------------------------------------------------
// CMapView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CMapView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   if( aResourceId == R_WAYFINDER_MAP_MENU ){
      if (IsIronVersion()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdRouteSubMenu, ETrue );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdRouteSubMenu, EFalse );
      }
      if (iWayFinderUI->HideServicesInfo()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdMapShowInfo, ETrue );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdMapShowInfo, EFalse );
      }
      aMenuPane->SetItemDimmed( EWayFinderCmdMapReroute, ETrue );
      aMenuPane->SetItemDimmed( EWayFinderCmdMapRouteTo, ETrue);
      if( iWayFinderUI->IsGpsConnected()) {
         if ( iWayFinderUI->CanUseGPSForEveryThing()) {
            aMenuPane->SetItemDimmed( EWayFinderCmdMapRouteTo, EFalse);
            if (iWayFinderUI->HasRoute()) {
               aMenuPane->SetItemDimmed( EWayFinderCmdMapReroute, EFalse );
            }
         }
      }

      if(iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
         if (iVectorMapContainer->IsFullScreen() ){
            aMenuPane->SetItemDimmed(EWayFinderCmdMapFullScreen, ETrue );
         } else {
            aMenuPane->SetItemDimmed(EWayFinderCmdMapFullScreen, EFalse );
            aMenuPane->SetItemDimmed(EWayFinderCmdMapRouteTo, ETrue);
            aMenuPane->SetItemDimmed(EWayFinderCmdMapSetOrigin, ETrue);
            aMenuPane->SetItemDimmed(EWayFinderCmdMapSetDestination, ETrue);
            aMenuPane->SetItemDimmed(EWayFinderCmdMapSaveFavorite, ETrue);
         }
      }
      if (iIsTracking) {
         aMenuPane->SetItemDimmed(EWayFinderCmdMapRouteTo, ETrue );
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSetOrigin, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSetDestination, ETrue);
         aMenuPane->SetItemDimmed(EWayFinderCmdMapSaveFavorite, ETrue);
         if (!iWayFinderUI->HasRoute()) {
#if !defined NAV2_CLIENT_SERIES60_V3
            aMenuPane->SetItemDimmed( EWayFinderCmdSend, ETrue);
#endif
         }
      }
   } else if( aResourceId == R_WAYFINDER_SEND_MENU ){
      if(iVectorMapContainer && iVectorMapContainer->ConstructDone()) {
         if (!iVectorMapContainer->IsFullScreen() || iIsTracking){
            aMenuPane->SetItemDimmed(
                  EWayFinderCmdConnectSendThisPosition, ETrue);
         }
      }
      aMenuPane->SetItemDimmed( EWayfinderCmdDestinationSendTo, ETrue);
      if (!iWayFinderUI->HasRoute()) {
         aMenuPane->SetItemDimmed( EWayfinderCmdGuideSendTo, ETrue);
      }

   } else if( aResourceId == R_WAYFINDER_MAP_ZOOM_MENU ){
      if( iWayFinderUI->HasRoute() ){
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewRoute, EFalse );
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewDestination, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewRoute, ETrue );
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewDestination, ETrue );
      }
      if( iWayFinderUI->IsGpsAllowed()){
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewCurrent, EFalse );
      } else{
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewCurrent, ETrue );
      }
      if ( iWayFinderUI->IsOriginSet() ) {
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewOrigin, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewOrigin, ETrue );
      }
      if ( iWayFinderUI->IsDestinationSet() ) {
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewDestination, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdMapViewDestination, ETrue );
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
                            const TDesC8& /*aCustomMessage*/)
{
   if( !iVectorMapConnection ){
      if( !iWayFinderUI->CheckMem(600*1024) ){
         WFDialog::ShowScrollingDialogL( iCoeEnv, R_WAYFINDER_MEMORY_LOW_MSG,
                                         R_WAYFINDER_MEMORY_LOW_EXIT_MSG, ETrue);
         // Switch to main view.
         iWayFinderUI->ShutdownNow();
         return;
      }
   }

   iCustomMessage = aCustomMessageId.iUid;

   iWayFinderUI->GetNavigationDecorator()->MakeScrollButtonVisible( ETrue );
   TBool hasRoute = iWayFinderUI->HasRoute();
   iSetOrigin = (aCustomMessageId.iUid == ESetOrigin );
   iSetDestination = (aCustomMessageId.iUid == ESetDestination);
   if( !iVectorMapContainer ){
      iVectorMapContainer = new (ELeave) CVectorMapContainer( this, iLog );
      LOGNEW(iVectorMapContainer, CVectorMapContainer);
      iVectorMapContainer->SetMopParent(this);

      TBool first = ( iMapFeatureHolder == NULL );
      
      InitResources(); //make sure that the arguments are initialized

      int savedScale = GetScale();
      
      iVectorMapContainer->ConstructL( ClientRect(), 
                                       iWayFinderUI,
                                       iMapControl,
                                       iMapFeatureHolder,
                                       m_mapInfoControl,
                                       iVectorMapConnection,
                                       iWayFinderUI->iPathManager
                                       ->GetLangResourcePath());
      
      if (iCurrentScale != MAX_INT32) {
         iVectorMapContainer->SetZoom(savedScale);
         iVectorMapContainer->SetCenter(iCenter.iY,
                                        iCenter.iX);
      }
      
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

      if( hasRoute && iWayFinderUI->GetRouteId() != 0 ){
         m_routeid = iWayFinderUI->GetRouteId();
         iVectorMapContainer->SetRoute( m_routeid );
      }
      /* Check if current position is usable. */
      if( iRequestType != MapEnums::None ){
         RequestMap(iRequestType, iRequestCoord.iY, iRequestCoord.iX );
      } else if (iWayFinderUI->IsGpsAllowed() &&
                 RequestCurrPosVectorMap() ){
         if (iWayFinderUI->UseTrackingOnAuto()) {
            SetTracking( ETrue );
         }
      } else if( iWayFinderUI->GetRouteId() != m_routeid ){
         RequestRouteVectorMap();
      } else if( !iGotMapType ){
         /* No map has been loaded. */
         RequestOriginVectorMap();
      }
      SetFullScreen();
      if (first && iRequestType == MapEnums::None) {
         if (iCurrentScale != MAX_INT32) {
            /* Set scale. */
            iVectorMapContainer->SetZoom(iCurrentScale);
            iVectorMapContainer->SetCenter(
                  iCenter.iY, /* Remember, latlon is y,x. */
                  iCenter.iX);
         }  else {
            iVectorMapContainer->SetCenter(
                  DefaultVectorMapCenterLat, DefaultVectorMapCenterLon);
         }
      }
      /* Make sure that the GPS symbol is visible if necessary. */
      CAnimatorFrame* temp = new CAnimatorFrame();
      temp->CopyFrameL(iWayFinderUI->GetGpsStatusImage());
      SetGpsStatusImage(temp->GetBitmap(), temp->GetMask());
      temp->DisownBitmaps();
      delete temp;

      iVectorMapContainer->UpdateRepaint();
      iVectorMapContainer->updateCursorVisibility();

   }
   if( hasRoute && iWayFinderUI->IsGpsAllowed()) {
      SetTopBorder();
      SetPicture();
      UpdateDistance( iLastDistance );
   }

   // Get the cacheInfo when loading the view, updating
   // the cache info is done when we are told to do
   // so in handleTileMapEvent.
   if (iMapControl) {
      MapLib* mapLib = iMapControl->getMapLib();

      mapLib->enableDrawing();
      
      if (mapLib) {
         iNbrCacheFiles = mapLib->getCacheInfo(iCacheInfo);
      }
   }
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

void CMapView::SearchForMapFiles()
{
#ifndef NAV2_CLIENT_SERIES60_V1
   _LIT(KDriveSpec, ":\\");
   if (iMapControl) {
      MapLib* mapLib = iMapControl->getMapLib();
      if (mapLib) {
         mapLib->addEventListener(this);
      }
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
                                     iWayFinderUI->iPathManager->
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

void CMapView::SetScale(int32 aScale)
{
   int ap = 22;
   ap++;
   iCurrentScale = aScale;
}

TPoint CMapView::GetCenter() 
{
   return iCenter;
}

// ---------------------------------------------------------
//
void CMapView::DoDeactivate()
{
   if ( iVectorMapContainer && iVectorMapContainer->ConstructDone() ){
      if (iMapControl) {
         iMapControl->SetFocus( EFalse );
    
         TPoint tempCoord;
         iVectorMapContainer->GetCoordinate(tempCoord);
         SetCenter(tempCoord);
         SetScale(iVectorMapContainer->GetScale());
         
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
   iWayFinderUI->iErrorFilter->resetParameters();   
}


void CMapView::SetCenter(TPoint center)
{
   int ap = 22;
   ap++;
   iCenter = center;
}

void CMapView::SetCenter(int32 aLat, int32 aLon)
{
   int ap = 22;
   ap++;
   
   iCenter.iY = aLat;
   iCenter.iX = aLon;
}

void
CMapView::HandleResourceChangeL(TInt /*aType*/)
{
//    if (aType == KEikDynamicLayoutVariantSwitch) {
//       iVectorMapContainer->HandleResourceChange(aType);      
//    }
}


void CMapView::InitResources()
{
   MapResourceFactory* mrFactory = iWayFinderUI->GetMapResourceFactory();   
   mrFactory->initIfNecessary();

   iMapFeatureHolder = mrFactory->getFeatureHolder();
   iVectorMapConnection = mrFactory->getVectorMapConnection();
   iMapControl = mrFactory->getMapControl();
   m_mapInfoControl = mrFactory->getMapInfoControl();
}

void CMapView::DetailsReceived(isab::GetFavoriteInfoReplyMess* details)
{
   HandleFavoriteInfoReply(details);
}

class CTileMapControl* CMapView::GetMapControl()
{
   return iMapControl;
}

class CMapFeatureHolder* CMapView::GetFeatureHolder()
{
   return iMapFeatureHolder;
}

class CMapInfoControl* CMapView::GetMapInfoControl()
{
   return m_mapInfoControl;
}

class CVectorMapConnection* CMapView::GetVectorMapConnection()
{
   return iVectorMapConnection;
}

// End of File

