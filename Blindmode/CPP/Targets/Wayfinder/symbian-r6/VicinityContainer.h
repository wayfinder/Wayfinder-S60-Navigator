/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VICINITYCONTAINER_H
#define VICINITYCONTAINER_H

#include "DetailFetcher.h"
#include "FavoriteFetcher.h"

#include <coecntrl.h>
#include "ScalableFonts.h"
#include "TimeOutNotify.h"
#include "WFTextUtil.h"
#include "MC2Coordinate.h"
#include "VicinityItem.h"
#include <vector>

#include "Crossings.h"
#include "CrossingItem.h"

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
//class CDesCArray;

class MapLib;

namespace isab{
   class GetFavoritesAllDataReplyMess;
}

class CVicinityContainer : public CCoeControl, 
                           public MCoeControlObserver,
                           public MTimeOutNotify,
                           public VicinityVisitor,
                           public DetailFetcherCallback,
                           public FavoriteRequester

{
public: // Constructor, Destructor

   ~CVicinityContainer();

   void
   ConstructL(const TRect& aRect, 
              CVicinityView * aView,
              CWayFinderAppUi * aWayFinderUI,
              CMapView * aMapView);

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
   void ForceUpdate();
   
public: //VicinityVisitor functions
   virtual void Visit(VicinityItem* item);
   virtual void Visit(CrossingItem* item);
   virtual void Visit(UDFItem* item);

public:

   virtual void DetailsReceived(isab::GetFavoriteInfoReplyMess* details);
   
   virtual void DetailsReceived(isab::FullSearchItem* details,
                                bool cachedResult);
   
   class MapLib* GetMapLib();
   
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

   /**
    * Handles message from Nav2, updates all favorites.
    * @param aMessage The message that contains info about all 
    *                 the favorites.
    */
   void UpdateFavorites(isab::GetFavoritesAllDataReplyMess* aMessage);

   /**
    * Sets gps pos in TileMapHandler. Also sets the angle/direction
    * and the scale.
    * @param coord New center coordinate.
    * @param direction The new angle.
    * @param scale The new scale.
    */
   void setGpsPos(const MC2Coordinate& coord, 
                  int direction);
   
   /**
    * Returns current scale.
    * @return Current scale.
    */
   int32 GetScale();

   /**
    *  Triggers a TALKS reading of the list
    *
    */  
   
   void UpdateReading();
   
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
   void UpdateList();
   
   void AppendDirection(   TPtr& connector,
                           const MC2Direction& direction);
   

   
   void PopulateItemArray();
   vicVec_t& GetItemArray();
   
   
   void SetControlExtent(class CCoeControl * aControl,
                         const TPoint & aPosition);



   void AddSfdCacheFiles();

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

   /**
    * Gets the text for the clicked item in the list, sets the 
    * instruction label with the information so that Talks will
    * be able to read it.
    */
   void DisplayItemText();


   /* Sets the selected item to be the first. */
   void ResetItemPos();


   /* Determines if a new GPS coordinate is sufficiently far away to warrant a
    * list update */
   
   bool DistantEnough( const MC2Coordinate& newCoord  );
   
private: // member functions
   
   DetailFetcher* m_detailFetcher;
   
   char* m_curTelephoneNumber;
   
   CAknDoubleStyleListBox* iVicinityList;
   CVicinityView* iView;
   
   CMapView* iMapView;

   CWayFinderAppUi* iWayFinderUI;

   CMapFeatureHolder* iFeatureHolder;

   CTileMapControl* iMapControl;

   CMapInfoControl* iMapInfoControl;

   /** Holds all VicinityItems added to the visible list */
   vicVec_t iVicItemList;
   
   int iDirection;
   int iUpdateCounter;

   /** Timer that periodically checks if the maps are fully downloaded */
   class CTimeOutTimer* iTimer;

   TBool iDisplayList;

   MC2Coordinate iLastUpdatePos;
   TTime iLastUpdateTime;
};

#endif
