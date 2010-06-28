/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VICINITY_FEED_CONTAINER_H
#define VICINITY_FEED_CONTAINER_H

#include "DetailFetcher.h"
#include "FavoriteFetcher.h"

#include <coecntrl.h>
#include "ScalableFonts.h"
#include "TimeOutNotify.h"
#include "WFTextUtil.h"
#include "MC2Coordinate.h"
#include "VicinityItem.h"
#include <vector>
#include <set>

#include "Crossings.h"
#include "CrossingItem.h"
#include "MC2SimpleString.h"

// FORWARD DECLARATION
class CAknDoubleStyleListBox;
class CVicinityView;
class CWayFinderAppUi;
class CMapView;
class CTileMapControl;
class CMapFeatureHolder;
class CMapInfoControl;
class CVectorMapConnection;
class CEikLabel;
class CVicinityView;
class CVicinityFeedView;
class CGuideView;

//class CDesCArray;

class MapLib;

namespace isab{
   class GetFavoritesAllDataReplyMess;
}

class CVicinityFeedContainer : public CCoeControl, 
                               public MCoeControlObserver,
                               public MTimeOutNotify

{
public: // Constructor, Destructor
   enum FeedState { FEED_ROUTE_INFORMATION,
                    FEED_VICINITY_INFORMATION   };

   enum RouteState { IDLE_ROUTE, ACTIVE_ROUTE  };
   
   ~CVicinityFeedContainer();

   void
   ConstructL(const TRect& aRect, 
              CVicinityFeedView * aView,
              CGuideView* aGuideView,
              CWayFinderAppUi * aWayFinderUI);

public: // functions from base classes
   void SizeChanged();

   TInt CountComponentControls() const;

   CCoeControl* ComponentControl(TInt aIndex) const;

   void Draw(const TRect& aRect) const;

   void HandleControlEventL(CCoeControl * aControl, TCoeEvent aEventType);

   TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,
                               TEventCode aType);
   /**
    * From CCoeControl
    * Handles layout awarness.
    */
   void HandleResourceChange(TInt aType);

   /** From MTimeOutNotify. */
   void TimerExpired();

public:
   enum ZoomScale{
      EScaleMin   = 1,
      EScale1     = 2,
      EScale2     = 4,
      EScale3     = 10,
      EScale4     = 20,
      EScale5     = 50,
      EScale6     = 100,
      EScale7     = 500,
      EScale8     = 1000,
      EScale9     = 2000,
      EScale10    = 5000,
      EScale11    = 10000,
      EScale12    = 14000,
      EScaleGlobe = 100000
   };


   FeedState GetFeedState();

   /**
    * Handles message from Nav2, updates all favorites.
    * @param aMessage The message that contains info about all 
    *                 the favorites.
    */
   void UpdateFavorites(isab::GetFavoritesAllDataReplyMess* aMessage);


   TBool UpdateGpsPos(const MC2Coordinate& coord, 
                     int direction);


   /**
    * Returns current scale.
    * @return Current scale.
    */
   int32 GetScale();

   /**
    * Sets the center position of the map.
    * @param aLat, New center coords lat pos.
    * @param aLon, New center coords lon pos.
    */
   void SetCenter(TInt32 aLat, TInt32 aLon);

   /**
    * Sets the current scale to use in the map.
    * @param aScale The new scale to be used.
    */
   void SetScale(TInt aScale);

   /**
    * Checks if the list is empty or if a invalid selected index.
    * @return ETrue if the list isnt empty and the selected index is valid (greater than -1)
    *         EFalse if the list is empty and the selected index is invalid (less than 0)
    */
   TBool IsListEmpty();

   /**
    * Gets the item for the selected index in the list.
    * 
    * @return VicinityItem If the selected index is a valid index.
    *         NULL         if the selected index isnt a valid index.
    */
   class VicinityItem* GetSelectedItem();

   /**
    * Sets the text instruction to the invicible label that
    * talks reads.
    * @param aInstruction The instruction that Talks should read.
    */
   
//   void SetTextInstructionL(const TDesC &aInstruction);

   /**
    * Calls the SetTextInstruction with information about the
    * current location.
    */
   void TriggerWhereAmIL();


   /**   Shows detailed information about the currently selected
    *    vicinity item.
    */
   void ShowVicinityDetails();

   void SetDetailFetcher(DetailFetcher* detailFetcher);
   void DoCall();
private: // helper functions
   void AppendVicinityItem(TPtr& connector,
                           VicinityItem* vicItem);


   RouteState GetRouteState();
   
   HBufC* GetVicinityItemType(VicinityItem* vicItem);
      
   void AppendDirection(   TPtr& connector,
                           const MC2Direction& direction);

   void AppendDistance(TPtr& connector,
                       TUint distance);
   
   TInt GetLogicalFont(enum TScalableFonts fontId, class TFontSpec& fontSpec);

   /**
    * Gets the correct distance string containing the correct metrics unit
    * @param aDistance The distance to be formated.
    * @param aText Will contain the formated text.
    */
   void GetDistance( TUint aDistance, TDes &aText );

   /**
    * Returns the index of the selected item.
    * @return The index of the selected item.
    */
   TInt GetSelectedIndex();

   /**
    * If the timer is active it is cancelled, the TimerExpired will 
    * get called by the timer object. If it isn't active TimerExpired
    * will get called by this function.
    */
   void CancelTimer();


   void UpdateText();
   
   
private: // member functions
   bool AppendPassingVicinityVector(vicVec_t& vicVector,
                                    TPtr& connector,
                                    bool passed,
                                    std::set<MC2SimpleString>& namesAdded );
      
   void AppendPassingVicinityItem(TPtr& connector,
                                  VicinityItem* vicItem);
   
   void ReadTextInstuction(HBufC* text);
   
   void AppendConciseItemList(TPtr& connector);
   void AppendVerboseItemList(TPtr& connector);

   void UpdateVicinityFeed();
   
   void UpdateRouteFeed();
   
   void UpdateRouteFeedIdle();
   
   void UpdateRouteFeedActive();

   /// Force an update the next time idle route text is active.
   void forceNextIdleUpdate();
   
   DetailFetcher* m_detailFetcher;


   CEikLabel * iViewLabel;
//   CEikLabel * iLocationLabel;
   CVicinityFeedView* iView;

   CGuideView* iGuideView;
   
   CWayFinderAppUi* iWayFinderUI;

   /** Current center position in the map */
   TPoint iCenter;

   /** The current scale in the map */
   int32 iScale;

   MC2Direction iDirection;
   
   int iUpdateCounter;

   /** Timer that periodically checks if the maps are fully downloaded */
   class CTimeOutTimer* iTimer;

   /** Flag that is set to true when the instruction label has focus */
   TBool iInstrLabelHasPriority;

   /** True when the timer is cancelled */
   TBool iTimerCancelled;
   
   /// When the route info in feed was last updated.
   TTime iLastUpdatedTime;

   class MapLib* iMapLib;

   std::set<MC2SimpleString> m_prevCrossingNames;
   std::set<MC2SimpleString> m_prevItemNames;
   bool m_forceIdleNameUpdate;
};

#endif
