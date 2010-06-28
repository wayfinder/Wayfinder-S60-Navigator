/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEVIEW_H
#define ROUTEVIEW_H

// INCLUDES
#include "ViewBase.h"
#include "Log.h"
#include "RouteEnums.h"
#include "RouteInfo.h"

// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CRouteContainer;

namespace isab{
   class RouteList;
}

// CLASS DECLARATION

/**
 *  CRouteView view class.
 * 
 */
class CRouteView : public CViewBase
{
public: // Constructors and destructor

   CRouteView(class CWayFinderAppUi* aUi, isab::Log* aLog);
   /**
    * EPOC default constructor.
    */
   void ConstructL();

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CRouteView object.
   ///        Note that this object is still on the CleanupStack.
   static class CRouteView* NewLC(CWayFinderAppUi* aUi, isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CRouteView object.
   static class CRouteView* NewL(CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CRouteView();

public: // New Functions

   void SetDestination( const TDesC &aDestination );

   void SetCoordinates( TInt32 aLat, TInt32 aLon );

   void SetInputData(int32 lat, int32 lon, TInt heading, TInt speed, TInt alt);

   void UpdateRouteDataL( TInt32 aDistanceToGoal,
         TInt32 aEstimatedTimeToGoal,
         TInt aAngleToGoal );

   void SetCurrentTurn( TInt aTurn );

   void GoToTurn( TInt aNextTurn );

   void SetRouteList( const isab::RouteList* aRouteList );

   TInt GetDistanceMode();

   TBool IsIronVersion();

   /**
    * Turns night mode on or off depending on aNightMode.
    * @param aNightMode True if nightmode should be switched on
    *                   False if nightmode should be switched off.
    */
   void SetNightModeL(TBool aNightMode);

private:

   /**
    * Display the destination in a map.
    */
   void ShowOnMap();

public: // Functions from base classes

   /**
    * From AknView.
    * @return the ID of the view.
    */
   TUid Id() const;

   /**
    * From AknView, takes care of command handling.
    * @param aCommand command to be handled
    */
   void HandleCommandL(TInt aCommand);

   /**
    * From AknView, handles a change in the view.
    */
   void HandleClientRectChange();

private: // Functions from base classes

   /**
    * From AknView, Called when the option softkey is pressed.
    * @param aResourceId the focused resource.
    * @param aMenuPane the current menu.
    */
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

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
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoDeactivate();

private: // Data

   /// The controls container.
   CRouteContainer* iContainer;

   TBuf<KBuf64Length> iDestination;

   /// The latitude of the viewed destination
   TInt32 iDestinationLat;

   /// The longitude of the viewed destination
   TInt32 iDestinationLon;

   TInt32 iDistance;

   isab::Log* iLog;

   TReal iEta; 
   TInt32 iEtg;
   TInt iGoalHeading;

   TInt iCurrentTurn;
};

#endif

// End of File
