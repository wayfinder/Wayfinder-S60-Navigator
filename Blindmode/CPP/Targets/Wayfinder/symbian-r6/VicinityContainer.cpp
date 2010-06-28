/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <barsread.h>
#include <aknlists.h>
#include <hal.h>

#include "WayFinderConstants.h"		// view id constants
#include "RsgInclude.h"					// dodona strings and resources
#include "GfxUtility.h"
#include "VicinityView.h"				// parent view
#include "WayFinderAppUi.h"			// parent app
#include "MapView.h"						// TileMapEventListener + MapView
#include "TileMapControl.h"			// CTileMapControl
#include "MapFeatureHolder.h"			// CMapFeatureHolder
#include "MapInfoControl.h"			// CMapInfoControl
#include "VectorMapConnection.h"		// CVectorMapConnection
#include "DataHolder.h"
#include "SettingsData.h"

#include "DistancePrintingPolicy.h"
#include "UserDefinedScaleFeature.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"
#include "DetailParser.h"
#include "MapResourceFactory.h"

#include "VicinityContainer.h"
#include "TimeOutTimer.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include <aknlists.h>  // for avrell style listbox
#include <aknpopup.h>  // for pop up menu
#include "memlog.h"
#include "TraceMacros.h"
#include "Dialogs.h"
#include "WFTextUtil.h"
#include "FormatDirection.h"
#include "nav2util.h"
#include "FavoriteParser.h"
#include "DirectionHelper.h"


#define KEY_GREEN_PHONE     EStdKeyYes
#define LISTBOX_POS TPoint(10, 15)

using namespace isab;

static const TUint32 KStartRetryTimeOut = 1*1000*50;

enum TVicinityControls {
   EVicinityList,
//   EInstructionLabel,
   ENumberControls
};

CVicinityContainer::~CVicinityContainer()
{

   delete iTimer;
   delete iVicinityList;
}

void
CVicinityContainer::SetDetailFetcher(DetailFetcher* detailFetcher)
{
   m_detailFetcher = detailFetcher;
}

int translateCrossing(int numRoads) {
   switch(numRoads) {
      case 2:
         return R_WF_CROSSING_2WAY;
         break;         
      case 3:
         return R_WF_CROSSING_3WAY;
         break;
      case 4:
         return R_WF_CROSSING_4WAY;
         break;
      case 5:
         return R_WF_CROSSING_5WAY;
         break;
      case 6:
         return R_WF_CROSSING_6WAY;
         break;
   }
   
   return R_WF_GUIDE_TEXT_EMPTY;
}

void
CVicinityContainer::ShowVicinityDetails()      {
   if(!iVicinityList)
      return;

   VicinityItem* curSelected = GetSelectedItem();

   if(curSelected)
      curSelected->Accept(this);
}
 

LOCAL_C TInt CallbackText1(TAny* aAny)
{
   ((CVicinityContainer*) aAny)->DoCall();
   return FALSE;
}


void
CVicinityContainer::DetailsReceived(isab::FullSearchItem* details,
                                    bool cachedResult)
{
   DetailParser detailParser(iCoeEnv);
   detailParser.parseDetails(details, 10);
   
   VicinityItem* curSelected = GetSelectedItem();
   
   if(!curSelected)
      return;

   delete[] m_curTelephoneNumber;
   m_curTelephoneNumber = isab::strdup_new(detailParser.getTelephoneNumber());

   HBufC* header =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_POI);
   
   HBufC* message =
      WFTextUtil::AllocLC(detailParser.getParsedDetails());
   

   TCallBack callback1(CallbackText1, this);

   WFDialog::ShowPhoneDialogL( header,
                               message,
                               EFalse,
                               callback1);
   
   CleanupStack::PopAndDestroy(message);
   CleanupStack::PopAndDestroy(header);
}

void
CVicinityContainer::Visit(VicinityItem* item)
{
   if(item->Name() == item->ServerName())
      return;


   if(!m_detailFetcher)
      return;
   
   /* Id is server name, name is regular name. */
   m_detailFetcher->GetDetails(item->ServerName().c_str(),
                               item->Name().c_str(),
                               this);   
}


void
CVicinityContainer::Visit(UDFItem* item) {
   CMapFeature* mf = iFeatureHolder->GetFeature(item->GetFeature());
   
   if (mf) {
      /* Got it. */
      if (mf->Type() == map_feature_favorite) {
         /* Got a favorite. */
         uint32 id = (uint32)mf->Id();
         RequestFavorite(id);
      }
   } else {
      return;
   }
}


void
CVicinityContainer::DetailsReceived(isab::GetFavoriteInfoReplyMess* details)
{
   FavoriteParser f(details);
   f.parseDetails();

   HBufC* tmp = WFTextUtil::AllocLC( f.getParsedData() );

   HBufC* header =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_UDF);
   
   WFDialog::ShowScrollingDialogL( *header, *tmp, EFalse );
   
   CleanupStack::PopAndDestroy(header);
   CleanupStack::PopAndDestroy(tmp);    
}


void
CVicinityContainer::Visit(CrossingItem* item)
{
   Crossing crossing;
   
   if(iWayFinderUI->getDirectionType()==MC2Direction::ClockBased)
      crossing = item->GetCrossingsRelative(iDirection);
   else
      crossing = item->GetCrossingsAbsolute();
   
   int numResourceId = translateCrossing(crossing.size());
   
   HBufC* header =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_CROSSING);
   
   HBufC* crossingType =
      CEikonEnv::Static()->AllocReadResourceLC(numResourceId);
   
   _LIT(KSpace, " ");
   _LIT(KBreak, "\n");
   
   HBufC* text = HBufC::NewLC(512);
   HBufC* degree = iCoeEnv->AllocReadResourceLC(R_DEGREE);
   
   TPtr connector = text->Des();
   connector.Append(*crossingType);
   connector.Append(KBreak);
   connector.Append(KBreak);

   for(Crossing::iterator ci =
          crossing.begin();
       ci!=crossing.end();
       ci++)
   {
      const IntersectingRoad& ir = *ci;
      
      HBufC* tmp = WFTextUtil::AllocLC(ir.name.c_str());
      connector.Append(*tmp);

      CleanupStack::PopAndDestroy(tmp);
      
      connector.Append(KSpace);

      AppendDirection(   connector,
                         ir.direction);
      
      if(std::distance(ci, crossing.end()) > 0)
         connector.Append(KBreak);
   }
   
   CleanupStack::PopAndDestroy(degree);
      
   WFDialog::ShowScrollingDialogL( *header, *text, EFalse );
   
   CleanupStack::PopAndDestroy(text);
   CleanupStack::PopAndDestroy(crossingType);
   CleanupStack::PopAndDestroy(header);
}



void CVicinityContainer::ResetItemPos()
{
   if(!iVicinityList)
      return;

   //if(iVicinityList->Model()->NumberOfItems()>0) {
//   iVicinityList->Model()->SetCurrentItemIndex(0);

   iVicinityList->SetCurrentItemIndex(0);
   iVicinityList->ItemDrawer()->SetItemMarkPosition(0);
   iVicinityList->ItemDrawer()->SetTopItemIndex(0);

          //}
}


void 
CVicinityContainer::UpdateList() 
{   
   MapLib * mapLib = iMapControl->getMapLib();
   
   if (!mapLib->mapFullyLoaded()) {
      if (!iTimer->IsActive()) {
         // If the map isnt finished downloading then we need to wait,
         // start a timer and retry a bit later.
       iTimer->After(KStartRetryTimeOut);      
      }
      return;
   }

   /* We shouldn't update the display if we don't have reliable GPS data */
   /* No longer true since we allow fixed-point-vicinity information */
   //   if(!iWayFinderUI->IsGpsConnected())
   //    return;
   
   iDisplayList = EFalse; //The user will now have to force an update.
      
   PopulateItemArray();
        
   vicVec_t vicItemList = GetItemArray();
   
   // Get a text array for adding new values in.
   CTextListBoxModel * model = iVicinityList->Model();
   CDesCArray * itemArray = STATIC_CAST(CDesCArray *, model->ItemTextArray());

   // Reset the current list, if not there might be doubles
   itemArray->Reset();   
   iVicinityList->HandleItemRemovalL();
   
   _LIT(KTabulator, "\t");
   _LIT(KSpace, " ");

   for (vicVec_t::iterator i = vicItemList.begin();
        i != vicItemList.end();
        i++) {

      VicinityItem& vicItem = *(*i);
      
      if (i == NULL) {
         continue;
      }

      // Allocate the string that is to be appended to the list
      HBufC* text = HBufC::NewLC(512);
      TPtr connector = text->Des();

      // Add leading tab sign
      connector.Append(KTabulator);
      
      // Add the distance
      TBuf<50> distStr;
      GetDistance(vicItem.Distance(), distStr);
      connector.Append(distStr);

      // Add separating space
      connector.Append(KSpace);

      // Append degrees
      AppendDirection(   connector,
                         vicItem.Direction());
      
      // Add new tab sign
      connector.Append(KTabulator);

      // Append the category
      MC2SimpleString name = vicItem.Name();
      
      // Append the name of the feature
      HBufC* tmp = WFTextUtil::AllocLC(name.c_str());
      connector.Append(*tmp);

      itemArray->AppendL(*text);


      
      // Clean up
      CleanupStack::PopAndDestroy(tmp);
      CleanupStack::PopAndDestroy(text);
   }

   // Generate a redraw of the list
   ResetItemPos();
   iVicinityList->HandleItemAdditionL();

   UpdateReading();
}

void
CVicinityContainer::ConstructL(const TRect& aRect, 
                               CVicinityView * aView,
                               CWayFinderAppUi * aWayFinderUI,
                               CMapView * aMapView) 
{
//   InitTalkingLabel(this);
   
   m_curTelephoneNumber = 0;
   m_detailFetcher = 0;
   iDirection = 0;
   iView = aView;
   iWayFinderUI = aWayFinderUI;
   iMapView = aMapView;
   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);

   iLastUpdateTime.HomeTime();
   
   CreateWindowL();

   MapResourceFactory* mrFactory =
      iWayFinderUI->GetMapResourceFactory();
   
   CVectorMapConnection* vectorMapConnection = NULL;

   mrFactory->initIfNecessary();

   iMapControl = 					mrFactory->getMapControl();
   iFeatureHolder = 		      mrFactory->getFeatureHolder();
   iMapInfoControl = 	 		mrFactory->getMapInfoControl();
   vectorMapConnection = 	   mrFactory->getVectorMapConnection();
   
   iMapInfoControl->SetContainerWindowL(* this);
   iMapControl->SetContainerWindowL(* this);
   
   iDisplayList = ETrue;
   
   iView->RequestFavorites();

   TResourceReader reader;
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_VICINITY_LIST);
   iVicinityList = new (ELeave) CAknDoubleStyleListBox();
   LOGNEW(iVicinityList, CAknDoubleStyleListBox);
   iVicinityList->SetContainerWindowL(* this);
   iVicinityList->ConstructFromResourceL(reader);
   CleanupStack::PopAndDestroy();
   
   iVicinityList->SetRect(Rect()); 


   // InitTalkingLabel(*this);
      
   SetRect(aRect);
   if( iVicinityList ) {
      UpdateList();
   }
   ActivateL();
// iLocationLabel->SetExtent(TPoint(0, 0), TSize(100, 100)); 
}

void 
CVicinityContainer::TriggerWhereAmIL() 
{
   iWayFinderUI->DisplayWhereAmIL();
}


void
CVicinityContainer::PopulateItemArray()
{
   MapLib * mapLib = iMapControl->getMapLib();
   iVicItemList.clear();
   iVicItemList.reserve(20);

   if(iView->Id() == KVicinityViewAllId) {
      mapLib->getAllInVicinity(iVicItemList);
   }  else if(iView->Id() == KVicinityViewPOIId) {
      mapLib->getPOIsInVicinity(iVicItemList);
   }  else if(iView->Id() == KVicinityViewFavouriteId) {
      mapLib->getUDFsInVicinity(iVicItemList);
   }  else if(iView->Id() == KVicinityViewCrossingId) {
      mapLib->getCrossingsInVicinity(iVicItemList);
   }  else {
      mapLib->getAllInVicinity(iVicItemList);
   }
}

vicVec_t&
CVicinityContainer::GetItemArray()
{
   return iVicItemList;
}


void 
CVicinityContainer::SizeChanged() 
{
   if(iVicinityList) {
      iVicinityList->SetRect(Rect());
   }
}

TInt
CVicinityContainer::CountComponentControls() const 
{
   return ENumberControls;
}

CCoeControl * 
CVicinityContainer::ComponentControl(TInt aIndex) const 
{
   TRACE_FUNC();
   switch(aIndex) 
      {
         //case ELocationLabel:
         //return iLocationLabel;
//         break;
      case EVicinityList:
         return iVicinityList;
         break;
      default:
         return NULL;
      }
   TRACE_FUNC();
}

void
CVicinityContainer::Draw(const TRect& aRect) const 
{
   TRACE_FUNC();
   CWindowGc& gc = SystemGc();
   
   gc.SetPenStyle(CGraphicsContext::ENullPen);
   gc.SetBrushColor(TRgb(KBackgroundRed, KBackgroundGreen, KBackgroundBlue));
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);
   
   TRACE_FUNC();
}

void 
CVicinityContainer::HandleControlEventL(CCoeControl * aControl, 
                                        TCoeEvent aEventType) 
{
         //TODO: implement
}

TKeyResponse
CVicinityContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                   TEventCode aType) 
{

   if(aType == EEventKeyDown) {
      if(aKeyEvent.iScanCode == KEY_GREEN_PHONE)
      {
         DoCall();
         return EKeyWasConsumed;
      }
   }
   
   TRACE_FUNC();
   if(aType != EEventKey) {
      return EKeyWasNotConsumed;
   }
   if(iVicinityList) {
      switch(aKeyEvent.iScanCode) 
         {
         case '9':
            ForceUpdate();
            break;
         case EStdKeyUpArrow:
         case EStdKeyDownArrow:
            iVicinityList->SetFocus(ETrue);
            return iVicinityList->OfferKeyEventL( aKeyEvent, aType );
         case EStdKeyDevice3:
            ShowVicinityDetails();
            return EKeyWasConsumed;
         default:
            return EKeyWasNotConsumed;
         }
   } else {
      return EKeyWasNotConsumed;
   }

   return EKeyWasNotConsumed;
   TRACE_FUNC();
}

void
CVicinityContainer::SetControlExtent(class CCoeControl * aControl,
                                     const TPoint & aPosition) 
{
   TRACE_FUNC();
   // calculate the minimal size needed for the control
   TSize minSize = aControl->MinimumSize();
   
#ifdef NAV2_CLIENT_SERIES60_V3
   // We need to increase the height otherwise the descenders of g or p 
   // will not be seen.
   minSize.iHeight += 3;
   
   // All positions are relative. They are based on the positions of s60v2
   aControl->SetExtent(WFLayoutUtils::CalculatePosUsingMainPane(aPosition),
                       minSize);
   
   aControl->DrawDeferred();
#else
   aControl->SetExtent(aPosition, minSize);
#endif
   TRACE_FUNC();
}


void CVicinityContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);

   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());

      iVicinityList->SetRect(Rect());
   }
}

void CVicinityContainer::TimerExpired()  
{
   if( iVicinityList ) {
      UpdateList();
      UpdateReading();
   }
}

void
CVicinityContainer::UpdateFavorites(GetFavoritesAllDataReplyMess* aMessage)
{
   if(iFeatureHolder) {
      favorite_vector_type* favVector = aMessage->getFavorites();
      iFeatureHolder->UpdateFavorites(favVector);
      iMapControl->Handler().setUserDefinedFeatures(iFeatureHolder->GetUDFVector());
   }    
}

inline void CVicinityContainer::GetDistance(TUint aDistance, TDes &aText)
{
   isab::DistancePrintingPolicy::DistanceMode mode =
      isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());
   
   // Get the formated string with the correct metrics unit
   char* tmp = isab::
      DistancePrintingPolicy::convertDistance(aDistance,
                                              mode, 
                                              isab::DistancePrintingPolicy::Round);
   if (tmp) {
      WFTextUtil::char2TDes(aText, tmp);
      delete[] tmp;
   } else {
      aText.Copy(_L(" "));
   }
}

bool CVicinityContainer::DistantEnough( const MC2Coordinate& newCoord )
{
   const float allowedDistanceSquared = 10.0f * 10.0f;
   
   float cosLat = GfxUtility::getCosLat( iLastUpdatePos.lat );

   float distSquared =  GfxUtility::squareP2Pdistance_linear(newCoord,
                                                             iLastUpdatePos,
                                                             cosLat);

   return distSquared > allowedDistanceSquared;
}

void
CVicinityContainer::setGpsPos(const MC2Coordinate& coord, 
                              int direction)
{
   TInt timeCap = iWayFinderUI->iSettingsData->iVicinityListPeriod;
   
   iDirection = direction;

   if( IsListEmpty() ) {
      UpdateList();
      return;
   }
   
   if( !iLastUpdatePos.isValid() ) {
      iLastUpdatePos = coord;
      return;
   }

   TTime currentTime;
   currentTime.HomeTime();
   TTimeIntervalSeconds interval;
   TInt res = currentTime.SecondsFrom(iLastUpdateTime, interval);

   if( (res == KErrNone) && (interval.Int() > timeCap) && timeCap != 0 ) {         
      if( DistantEnough( coord ) ) {
         iLastUpdateTime.HomeTime();
         iLastUpdatePos = coord;
         
         if( iVicinityList ) {
            UpdateList();
            UpdateReading();            
         }
      }
   }
}

TBool
CVicinityContainer::IsListEmpty()
{
   CTextListBoxModel * model = iVicinityList->Model();
   CDesCArray * itemArray = STATIC_CAST(CDesCArray *, model->ItemTextArray());
   return ( itemArray->MdcaCount() == 0 || ( GetSelectedIndex() == -1 ) );
}

TInt 
CVicinityContainer::GetSelectedIndex()
{
   return iVicinityList->CurrentItemIndex();
}

class VicinityItem* 
CVicinityContainer::GetSelectedItem()
{
   TInt index = GetSelectedIndex();
   vicVec_t& vicItemList =
      GetItemArray();
   
   if (!vicItemList.empty() && index < TInt(vicItemList.size())) {
      return vicItemList[index];
   }
   return NULL;
}

void 
CVicinityContainer::CancelTimer()
{
   if(iTimer->IsActive()) {
      iTimer->Cancel(); 
   } else {
      TimerExpired();
   }   
}

class MapLib*
CVicinityContainer::GetMapLib()
{
   return iMapControl->getMapLib();
}

void
CVicinityContainer::AppendDirection(   TPtr& connector,
                                       const MC2Direction& direction)
{
   DirectionHelper::AppendDirection( connector,
                                     direction,
                                     iDirection,
                                     iWayFinderUI->IsHeadingUsable(),
                                     iWayFinderUI->getDirectionType() );
}

void CVicinityContainer::DoCall()
{
   TBuf<128> temp;
   WFTextUtil::char2TDes(temp, m_curTelephoneNumber);
   
   iWayFinderUI->DialNumberL( temp, 0);      
}

void CVicinityContainer::ForceUpdate()
{
   if( iVicinityList ) {
      UpdateList();
      UpdateReading();
   }
}


void CVicinityContainer::UpdateReading()
{
   iVicinityList->SetFocus(ETrue);
   iVicinityList->DrawDeferred();
}


// End of File
