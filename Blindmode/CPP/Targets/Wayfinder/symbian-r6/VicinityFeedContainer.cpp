/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "VicinityFeedContainer.h"
#include <barsread.h>
#include <aknlists.h>
#include <hal.h>


#include "VicinityFeedView.h"

#include "WayFinderConstants.h"		// view id constants
#include "RsgInclude.h"					// dodona strings and resources

#include "VicinityView.h"				// parent view
#include "WayFinderAppUi.h"			// parent app
#include "MapView.h"						// TileMapEventListener + MapView
#include "TileMapControl.h"			// CTileMapControl
#include "MapFeatureHolder.h"			// CMapFeatureHolder
#include "MapInfoControl.h"			// CMapInfoControl
#include "VectorMapConnection.h"		// CVectorMapConnection
#include "DataHolder.h"

#include "DistancePrintingPolicy.h"
#include "UserDefinedScaleFeature.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"
#include "DetailParser.h"
#include "GuideView.h"

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
#include "MapResourceFactory.h"
#include "MC2Direction.h"
#include "DirectionHelper.h"

#define KEY_SOFTKEY_LEFT    EStdKeyDevice0
#define KEY_GREEN_PHONE     EStdKeyYes
#define LISTBOX_POS TPoint(10, 15)
#define UPDATE_INTERVAL 5

const int IDLE_CUTOFF = 200;
const int MAX_DISTANCE = 2000;
const int PASSING_BY_CUTOFF = 20;

using namespace isab;

static const TUint32 KStartRetryTimeOut = 1*1000*50;

enum TVicinityControls {
   EViewLabel = 0,
   ENumberControls
};

CVicinityFeedContainer::~CVicinityFeedContainer()
{
   delete iTimer;
   delete iViewLabel;
}


void
CVicinityFeedContainer::ConstructL(const TRect& aRect, 
                                   CVicinityFeedView * aView,
                                   CGuideView* aGuideView,
                                   CWayFinderAppUi * aWayFinderUI) 
{
   iDirection = 0;
   TRACE_FUNC1("Constructing vic-feed-container");
   iView = aView;
   iWayFinderUI = aWayFinderUI;
   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);

   aGuideView->SharedActivation();
   
   iGuideView = aGuideView;
   
   iUpdateCounter = UPDATE_INTERVAL;
   iInstrLabelHasPriority = EFalse;
   iTimerCancelled = EFalse;

   TRACE_FUNC1("Creating window");
   CreateWindowL();

   SetRect(aRect);

   TRACE_FUNC1("Accessing map resources");
   MapResourceFactory* mrFactory = iWayFinderUI->GetMapResourceFactory();
   mrFactory->initIfNecessary();
   
   iMapLib = mrFactory->getMapControl()->getMapLib();

#if defined NAV2_CLIENT_SERIES60_V3
   const CFont* labelFont = AknLayoutUtils::FontFromId(EAknLogicalFontPrimarySmallFont);
#else
   const CFont* labelFont = LatinPlain12();
#endif


   TRACE_FUNC1("Initializing label");
   iViewLabel = new (ELeave) CEikLabel;
   iViewLabel->OverrideColorL( EColorLabelTextEmphasis, KRgbBlack );
   iViewLabel->OverrideColorL( EColorLabelHighlightFullEmphasis, KRgbWhite );
   iViewLabel->SetFont(labelFont);
   iViewLabel->SetEmphasis( CEikLabel::EFullEmphasis );

   iViewLabel->SetContainerWindowL(* this);
   iViewLabel->SetTextL(_L(""));
   iViewLabel->SetRect(Rect());
   iViewLabel->DrawDeferred();

   TRACE_FUNC1("Activating.");
   ActivateL();
   TRACE_FUNC1("Construct complete.");

///iViewLabel->SetExtent(TPoint(0, 0), TSize(100, 100)); 
   forceNextIdleUpdate();
   iLastUpdatedTime.HomeTime();
   iLastUpdatedTime -= TTimeIntervalSeconds( 24*60*60 );
 
}

void 
CVicinityFeedContainer::TimerExpired()
{
   UpdateText();
}

void 
CVicinityFeedContainer::SizeChanged() 
{
   TRACE_FUNC();
   //if(iViewLabel) {
   //   iViewLabel->SetRect(Rect());
   //}
   

   if (WFLayoutUtils::LandscapeMode()) {

   }
   else {

   }

   TRACE_FUNC();
}

TInt
CVicinityFeedContainer::CountComponentControls() const 
{
   TRACE_FUNC();
   return ENumberControls;
   TRACE_FUNC();
}

CCoeControl * 
CVicinityFeedContainer::ComponentControl(TInt aIndex) const 
{
   TRACE_FUNC1("Entering component control");
   switch(aIndex) 
      {
      case EViewLabel:
         return iViewLabel;
         break;
      default:
         TRACE_FUNC1("Returning NULL-value");
         return NULL;
      }
}

void
CVicinityFeedContainer::Draw(const TRect& aRect) const 
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
CVicinityFeedContainer::HandleControlEventL(CCoeControl * aControl, 
                                        TCoeEvent aEventType) 
{
         //TODO: implement
}

TKeyResponse
CVicinityFeedContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                   TEventCode aType) 
{
   TRACE_FUNC();
   
   //To prevent hanging of application
   if(aKeyEvent.iScanCode == KEY_SOFTKEY_LEFT)
      return EKeyWasConsumed;
   
   if(aType != EEventKey) {
      return EKeyWasNotConsumed;
   }

   return EKeyWasNotConsumed;
   TRACE_FUNC();
}

inline void CVicinityFeedContainer::GetDistance(TUint aDistance, TDes &aText)
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


void CVicinityFeedContainer::UpdateText()
{
   switch(GetFeedState()) {
      case FEED_ROUTE_INFORMATION:
         UpdateRouteFeed();
         break;
      case FEED_VICINITY_INFORMATION:
         UpdateVicinityFeed();
         break;
   }
}


void CVicinityFeedContainer::UpdateVicinityFeed()
{
       
   HBufC* text = HBufC::NewLC(1024);
   TPtr connector = text->Des();

   _LIT(KBreak, "\n");
   _LIT(KSpace, " ");

   HBufC* yourPosition =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_LOCATION);

   HBufC* yourHeading =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_HEADING);

   connector.Append(*yourPosition);
   connector.Append(KBreak);
   
   MC2SimpleString whereAmI = iMapLib->getWhereAmI(" ");
   HBufC* waiBuf = WFTextUtil::AllocLC(whereAmI.c_str());
   connector.Append(*waiBuf);
   CleanupStack::PopAndDestroy(waiBuf);

   connector.Append(KBreak);

   if(iWayFinderUI->getDirectionType() == MC2Direction::DirectionBased) {
      //Append user direction
      connector.Append(*yourHeading);   
      connector.Append(KSpace);   
      AppendDirection(connector, iDirection);   
      connector.Append(KBreak);
   }
   
   connector.Append(KBreak);
   
   if(iWayFinderUI->getFeedOutputFormat() == FeedSettings::FEED_CONCISE) {
      AppendConciseItemList(connector);
   } else {
      AppendVerboseItemList(connector);
   }
   
   ReadTextInstuction(text);
   
   CleanupStack::PopAndDestroy(yourHeading);
   CleanupStack::PopAndDestroy(yourPosition);
   
   CleanupStack::PopAndDestroy(text);
}

void CVicinityFeedContainer::UpdateRouteFeedActive()
{
   TRACE_FUNC1("Updating route feed active");
   HBufC* text = iGuideView->GetCurrentTextInstruction();
   TRACE_FUNC1("Text instruction fetched");

   if(text == NULL)
      return;
   
   ReadTextInstuction(text);
}

bool
CVicinityFeedContainer::AppendPassingVicinityVector(vicVec_t& vicVector,
                                                    TPtr& connector,
                                                    bool passed,
                                    std::set<MC2SimpleString>& namesAdded )
{
   HBufC* passingBy =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_PASSING_BY);
   
   for(vicVec_t::iterator vi = vicVector.begin();
       vi!=vicVector.end();
       vi++)
   {
      VicinityItem* v = *vi;
      
      MC2Direction mv(v->Direction());

      bool inFrontOfUser = mv.InFrontOfUser(iDirection);
      int distanceToObject = v->Distance();
      
      if( inFrontOfUser && distanceToObject < PASSING_BY_CUTOFF )
      {
         if(!passed) 
            connector.Append(*passingBy);
         passed = true;
         AppendPassingVicinityItem(connector, *vi);        
         namesAdded.insert( v->Name() );
      }
   }

   CleanupStack::PopAndDestroy(passingBy);
   
   return passed;
}

void CVicinityFeedContainer::UpdateRouteFeedIdle()
{
   // Long straight road, include crossings, POIs etc.

   TRACE_FUNC1("Updating route feed when idle");

   MC2SimpleString curRoad = iMapLib->getCurrentRoad();
      
   HBufC* text = HBufC::NewLC(1024);

   HBufC* keepWalking =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_KEEP_WALKING);

   HBufC* crBuf = WFTextUtil::AllocLC(curRoad.c_str());
   
   TPtr connector = text->Des();
   
   _LIT(KBreak, "\n");
   _LIT(KSpace, " ");
   
   vicVec_t crossings;
   iMapLib->getCrossingsAlongCurrentRoad(crossings);
   
   vicVec_t poisAndUdfs;
   iMapLib->getPOIsAndUDFsAlongCurrentRoad(poisAndUdfs);
   
   connector.Append(*keepWalking);
   connector.Append(KSpace);
   connector.Append(*crBuf);
   
   connector.Append(KBreak);
   std::set<MC2SimpleString> crossingNamesAdded;
   std::set<MC2SimpleString> allNamesAdded;

   if(!iWayFinderUI->IsHeadingUsable()) {
      /* Removed due to user confusion. Better to not report anything
         at all. */
      
/*      HBufC* unreliableHeading =
         CEikonEnv::Static()->AllocReadResourceLC(R_WF_UNRELIABLE_HEADING);
      connector.Append(*unreliableHeading);      
      CleanupStack::PopAndDestroy(unreliableHeading);*/
   } else {
      bool passed = false;
      passed = AppendPassingVicinityVector(crossings, connector, passed,
                                           crossingNamesAdded );
      
      allNamesAdded = crossingNamesAdded;
      
      passed = AppendPassingVicinityVector(poisAndUdfs, connector, passed,
                                           allNamesAdded );
   }

   bool itemsUpdated  = (allNamesAdded != m_prevItemNames );
   if ( m_forceIdleNameUpdate ) {
      // Force to treat the item names as updated.
      // This is to ensure that first time entering idle mode
      // we don't have to wait 20 seconds.
      itemsUpdated = true;
      m_forceIdleNameUpdate = false;
   }
  
   bool update = false;

   // The following update logic is used.
   // Update if crossings has changed since last update.
   // Update if anything else has changed and there has been
   // at least 20 seconds since last update.

   if ( itemsUpdated ) {

      // Only update if last update was some time ago.
      TTime now;
      now.HomeTime();

      TTime nextAllowedUpdateTime = 
         iLastUpdatedTime + TTimeIntervalSeconds( 20 );

      if ( now > nextAllowedUpdateTime ) {
         // At least 20 seconds since last update. OK to update now.
         update = true;
      }

      // New updated crossing names should result in a forced update.
      if ( !crossingNamesAdded.empty() && 
           (crossingNamesAdded != m_prevCrossingNames) ) {
         update = true;
      }

   }


   if ( update ) {
     
      // Update when last updated.
      iLastUpdatedTime.HomeTime();
     
      m_prevCrossingNames = crossingNamesAdded;
      m_prevItemNames = allNamesAdded;
     
      iViewLabel->SetTextL(*text);
      iViewLabel->SetFocus(ETrue);
      iViewLabel->DrawDeferred();
   } 
   
   CleanupStack::PopAndDestroy(crBuf);
   CleanupStack::PopAndDestroy(keepWalking);
   CleanupStack::PopAndDestroy(text);  

   //XXX:FIXME PROBABLY LEAKING crossings and poisAndUdfs.
}

void CVicinityFeedContainer::UpdateRouteFeed()
{
   TRACE_FUNC1("Updating route feed");
   switch(GetRouteState())
   {
   case IDLE_ROUTE:
      UpdateRouteFeedIdle();
      break;
   case ACTIVE_ROUTE:
      UpdateRouteFeedActive();
      break;
   }
}

void assignItem(VicinityItem*& vicItem,
                vicVec_t& parentVector,
                int& numLeft)
{
   vicItem = parentVector.front();
   vicVec_t temp;
   
   std::copy(parentVector.begin() + 1,
             parentVector.end(),
             std::back_inserter(temp));

   parentVector = temp;
   numLeft = parentVector.size();
}

void CVicinityFeedContainer::AppendConciseItemList(TPtr& connector)
{
   vicVec_t closestCrossings;
   iMapLib->getCrossingsInVicinity(closestCrossings, MAX_DISTANCE);

   vicVec_t closestPOIs;
   iMapLib->getPOIsInVicinity(closestPOIs, MAX_DISTANCE);

   vicVec_t closestUDFS;
   iMapLib->getUDFsInVicinity(closestUDFS, MAX_DISTANCE);
   
   int numPois = closestPOIs.size();
   int numCrossings = closestCrossings.size();
   int numUdfs = closestUDFS.size();

   VicinityItem* item1 = NULL;
   VicinityItem* item2 = NULL;
   VicinityItem* item3 = NULL;

   if(numCrossings>0) {
      assignItem(item1, closestCrossings, numCrossings);
   } else if(numPois>0) {
      assignItem(item1, closestPOIs, numPois);
   } else if(numUdfs>0) {
      assignItem(item1, closestUDFS, numUdfs);
   }
   
   if(item1) {
      AppendVicinityItem(connector, item1);
   } else {
      return;
   }

   if(numUdfs>0) {
      assignItem(item2, closestUDFS, numUdfs);
   } else if(numPois>0) {
      assignItem(item2, closestPOIs, numPois);
   } else if(numCrossings>0) {
      assignItem(item2, closestCrossings, numCrossings);
   }

   if(item2) {
      AppendVicinityItem(connector, item2);
   } else {
      return;
   }

   if(numPois>0) {
      assignItem(item3, closestPOIs, numPois);
   } else if(numUdfs>0) {
      assignItem(item3, closestUDFS, numUdfs);
   } else if(numCrossings>0) {
      assignItem(item3, closestCrossings, numCrossings);
   }   

   if(item3) {
      AppendVicinityItem(connector, item3);
   } else {
      return;
   }
}

void CVicinityFeedContainer::AppendVerboseItemList(TPtr& connector)
{
   const unsigned int MAX_NUM_ITEMS = 4;

   vicVec_t closest;
   iMapLib->getAllInVicinity(closest);


   for(unsigned int i = 0; i<MAX_NUM_ITEMS && i<closest.size(); i++) {
      AppendVicinityItem(connector, closest[i]);
   }   
}

HBufC*
CVicinityFeedContainer::GetVicinityItemType(VicinityItem* vicItem) {
   CEikonEnv* resourceLoader = CEikonEnv::Static();

   HBufC* header = NULL;
   
   switch(vicItem->Type()) {
      case VicinityItem::VI_POI:
         header = resourceLoader->AllocReadResourceLC(R_WF_POI);
         break;
      case VicinityItem::VI_UDF:
         header = resourceLoader->AllocReadResourceLC(R_WF_UDF);
         break;
      case VicinityItem::VI_CROSSING:
         header = resourceLoader->AllocReadResourceLC(R_WF_CROSSING);         
         break;
      case VicinityItem::VI_ALL:
         break;
   }

   return header;
}


void
CVicinityFeedContainer::AppendPassingVicinityItem(TPtr& connector,
                                                  VicinityItem* vicItem)
{
   _LIT(KSpace, " ");
   _LIT(KBreak, "\n");

   MC2SimpleString name = vicItem->Name();
   MC2SimpleString category = vicItem->CategoryName();

   // "Getingev / Kung Oskars V \n"
   HBufC* nameBuffer = WFTextUtil::AllocLC(name.c_str());
   HBufC* categoryBuffer = NULL; // "Crossing "

   connector.Append(KSpace);
   connector.Append(*nameBuffer);
   connector.Append(KBreak);
      
   if( !category.empty() ) {
      categoryBuffer = WFTextUtil::AllocLC(category.c_str());
   } else {
      categoryBuffer = GetVicinityItemType(vicItem);
   }
   
   connector.Append(*categoryBuffer);
   connector.Append(KSpace);
   
   // " 10 meters"
   AppendDistance(connector, vicItem->Distance());
   connector.Append(KBreak);

   CleanupStack::PopAndDestroy(categoryBuffer);
   CleanupStack::PopAndDestroy(nameBuffer);
}


void
CVicinityFeedContainer::AppendVicinityItem(TPtr& connector,
                                           VicinityItem* vicItem)
{
   _LIT(KSpace, " ");
   _LIT(KBreak, "\n");
   
   MC2SimpleString name = vicItem->Name();

   MC2SimpleString category = vicItem->CategoryName();
   
   // Append the name of the feature
   HBufC* nameBuffer = WFTextUtil::AllocLC(name.c_str());
   connector.Append(*nameBuffer);
   CleanupStack::PopAndDestroy(nameBuffer);
   connector.Append(KBreak);

   HBufC* categoryBuffer = NULL;
      
   if( !category.empty() ) {
      categoryBuffer = WFTextUtil::AllocLC(category.c_str());
   } else {
      categoryBuffer = GetVicinityItemType(vicItem);
   }

   connector.Append(*categoryBuffer);
   CleanupStack::PopAndDestroy(categoryBuffer);
   connector.Append(KSpace);

   AppendDistance(connector, vicItem->Distance());
   connector.Append(KSpace);
   AppendDirection(connector, vicItem->Direction());
   connector.Append(KBreak);
}

TBool
CVicinityFeedContainer::UpdateGpsPos(const MC2Coordinate& coord, 
                              int direction)
{
   TRACE_FUNC1("Updating GPS pos");
   iDirection = MC2Direction(direction);

   if (!iMapLib->mapFullyLoaded()) {
      return EFalse;
   }

   TRACE_FUNC1("Updating text");
   UpdateText();
   return ETrue;
}

int32
CVicinityFeedContainer::GetScale()
{
   return iScale;
}

void
CVicinityFeedContainer::SetScale(TInt aScale)
{
   iScale = aScale;
//   iMapControl->Handler().setScale(aScale);
}

void CVicinityFeedContainer::HandleResourceChange(TInt aType)
{ 
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
}


void CVicinityFeedContainer::AppendDistance(TPtr& connector,
                                            TUint distance)
{
   using namespace isab;
   TBuf<50> distStr;

   typedef DistancePrintingPolicy DPB;
   
   DPB::DistanceMode mode =
      DPB::DistanceMode(iWayFinderUI->GetDistanceMode());
   
   // Get the formated string with the correct metrics unit
   char* tmp = DPB::convertDistance(distance,
                                    mode, 
                                    DPB::Round);
   
   if (tmp) {
      WFTextUtil::char2TDes(distStr, tmp);
      delete[] tmp;
      connector.Append(distStr);
   } else {
      connector.Append(_L(" "));
   }
}

void
CVicinityFeedContainer::AppendDirection( TPtr& connector,
                                         const MC2Direction& direction)
{
   DirectionHelper::AppendDirection( connector,
                                     direction,
                                     iDirection.GetAngle(),
                                     iWayFinderUI->IsHeadingUsable(),
                                     iWayFinderUI->getDirectionType() );
}

CVicinityFeedContainer::FeedState CVicinityFeedContainer::GetFeedState()
{
   if(iWayFinderUI->HasRoute())
      return FEED_ROUTE_INFORMATION;
   else
      return FEED_VICINITY_INFORMATION;
   
}

CVicinityFeedContainer::RouteState CVicinityFeedContainer::GetRouteState()
{
   if( iGuideView->GetCurrentDistance() > IDLE_CUTOFF  ) {
      return IDLE_ROUTE;
   } else {
      forceNextIdleUpdate();
      return ACTIVE_ROUTE;
   }
}

void CVicinityFeedContainer::ReadTextInstuction(HBufC* text)
{
   TRACE_FUNC1("Reading text");
   iViewLabel->SetTextL(*text);
   iViewLabel->SetFocus(ETrue);
   iViewLabel->DrawDeferred();
   TRACE_FUNC1("Text read");
}

void CVicinityFeedContainer::forceNextIdleUpdate()
{
   m_forceIdleNameUpdate = true; 
}
// End of File


