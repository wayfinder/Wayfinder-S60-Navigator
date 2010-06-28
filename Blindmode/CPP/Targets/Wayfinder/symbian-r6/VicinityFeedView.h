/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VICINITY_FEED_VIEW
#define VICINITY_FEED_VIEW


// INCLUDES
#include <aknview.h> // for CAknView

#include "Log.h"
#include "wayfinder.hrh"
#include "VicinityContainer.h"
#include "WayFinderConstants.h"
#include "VicinityItem.h"

// FORWARD DECLARATIONS
class CWayFinderAppUi;

class CMapView;
class CTileMapControl;
class CMapFeatureHolder;
class CMapInfoControl;
class CVectorMapConnection;
class CVicinityFeedContainer;
class CGuideView;

namespace isab{
   class GetFavoritesAllDataReplyMess;
   class UpdatePositionMess;
   class Log;
}

/**
 * CVicinityFeed class
 *
 * This class continually presents the surroundings to the
 * visually impaired user.
 */

class CVicinityFeedView : public CAknView {
public: // Constructors and deconstructors
	/**
	 * Factory method to create an object of this class.
	 */
   static class CVicinityFeedView* NewLC(CWayFinderAppUi * aWayFinderUI,
                                         CGuideView* aGuideView,
                                         isab::Log * aLog);
   
   /**
    * Factory method to create an object of this class.
    */
   static class CVicinityFeedView* NewL(CWayFinderAppUi * aWayFinderUI,
                                        CGuideView* aGuideView,
                                        isab::Log * aLog);
   
   /**
    * Constructor
    *
    * Constructor is private to ensure the objects are created through the 
    * static NewLx methods.
    */
   CVicinityFeedView(isab::Log * aLog);
        
   /**
    * Second phase constructor called by the factory methods NewLx().
    *
    * This method is called through the NewLx methods.
    */
   void ConstructL(CWayFinderAppUi * aWayFinderUI,
                   CGuideView* aGuideView);

   /**
    * Destructor
    */
   ~CVicinityFeedView();

public: // functions from base classes
	
	/**
	 * Returns the id of this view. Id is defined in WayFinderConstants.h and 
	 * wayfinder.hrh
	 */
   TUid Id() const;

   void SetInputData(int32 lat,
                     int32 lon,
                     TInt heading,
                     TInt speed,
                     TInt alt,
                     TInt aGpsState);

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoActivateL( const TVwsViewId& aPrevViewId,
                     TUid aCustomMessageId,
                     const TDesC8& aCustomMessage);
   
   
   /**
    * Command handling.
    */
   void HandleCommandL(TInt aCommand);

   /**
    * Changes any resizing or similar changes of th ui.
    */
   void HandleClientRectChange();

   /**
    * Cleanup method needs to be called manually before the destructor is 
    * called by itself.
    */
   void DoDeactivate();

public: // New functions

   /**
    * Handles message from Nav2, updates all favorites.
    * @param aMessage The message that contains info about all 
    *                 the favorites.
    */
   void HandleFavoritesAllDataReply( isab::GetFavoritesAllDataReplyMess* aMessage );

   /**
    * Returns the current distance mode, feet, meters, yards etc.
    * @return The current used metrics system.
    */
   TInt GetDistanceMode();

   /**
    * Updates the position data in VicinityContainer based on 
    * the current gps data.
    * @param aPositionMess The message holding all the position info.
    */
   void UpdateDataL(isab::UpdatePositionMess* aPositionMess,
                    TInt aSpeed);

   void RequestFavorites();

   void ForceRefresh();
   
   void SetDetailFetcher(DetailFetcher* detailFetcher);
   void SetFavoriteFetcher(FavoriteFetcher* favoriteFetcher);
   
private: // Functions from base classes

   /**
    * From AknView, Called when the option softkey is pressed.
    * @param aResourceId the focused resource.
    * @param aMenuPane the current menu.
    */
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

private: // New functions

   int GetZoomScale(TInt aSpeed);
private: // Member variables
   DetailFetcher* m_detailFetcher;
   FavoriteFetcher* m_favoriteFetcher;
   
   CGuideView* iGuideView;
   
   TInt32 iPrevLat;
   TInt32 iPrevLon;
   
   TTime iReferenceTime;
   
   CVicinityFeedContainer * iContainer;

   CWayFinderAppUi * iWayFinderUI;
   
   TUid iVicinityId;

   TPoint iCenter;

   int32 iCurrentScale;

   isab::Log * iLog;
};
#endif
