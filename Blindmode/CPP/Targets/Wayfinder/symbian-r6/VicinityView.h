/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VICINITYVIEW_H
#define VICINITYVIEW_H

// INCLUDES
#include <aknview.h> // for CAknView

#include "Log.h"
#include "wayfinder.hrh"
#include "VicinityContainer.h"
#include "WayFinderConstants.h"
#include "VicinityItem.h"
#include "MC2Coordinate.h"

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CMapView;
class CTileMapControl;
class CMapFeatureHolder;
class CMapInfoControl;
class CVectorMapConnection;

namespace isab{
   class GetFavoritesAllDataReplyMess;
   class UpdatePositionMess;
}

class DetailFetcher;
class FavoriteFetcher;

/**
 * CVicinityView class
 *
 * This class is the interface to the information calculated from the map 
 * data to present them to visually impaired persons
 */
class CVicinityView : public CAknView {
public: // Constructors and deconstructors
	/**
	 * Factory method to create an object of this class.
	 */
   static class CVicinityView* NewLC(CWayFinderAppUi * aWayFinderUI,
                                     CMapView * aMapView,
                                     isab::Log * aLog,
                                     TUid aViewId);
   
   /**
    * Factory method to create an object of this class.
    */
   static class CVicinityView* NewL(CWayFinderAppUi * aWayFinderUI,
                                    CMapView * aMapView,
                                    isab::Log * aLog,
                                    TUid aViewId);
   
   /**
    * Constructor
    *
    * Constructor is private to ensure the objects are created through the 
    * static NewLx methods.
    */
   CVicinityView(isab::Log * aLog);

   /**
    * Second phase constructor called by the factory methods NewLx().
    *
    * This method is called through the NewLx methods.
    */
   void ConstructL(CWayFinderAppUi * aWayFinderUI,
                   CMapView * aMapView,
                   TUid aViewId);

   /**
    * Destructor
    */
   virtual ~CVicinityView();

public: // functions from base classes
	
	/**
	 * Returns the id of this view. Id is defined in WayFinderConstants.h and 
	 * wayfinder.hrh
	 */
   TUid Id() const;


   /**
    *  	Forces a refresh of the vicinity list.
    *
    *
    * */
   
   void ForceRefresh();
   
   /**
    * Command handling.
    */
   void HandleCommandL(TInt aCommand);

   /**
    * Changes any resizing or similar changes of th ui.
    */
   void HandleClientRectChange();

   /**
    * Method to handle all setting that needs a finished object creation.
    */
   void DoActivateL(const TVwsViewId& aPrevViewId,
                    TUid aCustomMessageId,
                    const TDesC8& aCustomMessage);

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
   void NavigateTo();
private: // Member variables
   DetailFetcher* m_detailFetcher;
   FavoriteFetcher* m_favoriteFetcher;
   
   CVicinityContainer * iContainer;

   CWayFinderAppUi * iWayFinderUI;

   MC2Coordinate iGpsPos;
   TInt iHeading360;
   
   CMapView * iMapView;

   TUid iVicinityId;

   TPoint iCenter;

   int32 iCurrentScale;
	
   isab::Log * iLog;
};

#endif
