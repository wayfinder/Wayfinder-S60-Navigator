/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef GUIDEVIEW_H
#define GUIDEVIEW_H

// INCLUDES
#include "ViewBase.h"

#include "WayFinderConstants.h"
#include "Log.h"
#include "RouteEnums.h"
#include "RouteInfo.h"
#include "TurnPictures.h"

// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;

namespace isab{
   class RouteList;
}

// CLASS DECLARATION

/**
*  CGuideView view class.
* 
*/
class CGuideView : public CViewBase
{
public: // Constructors and destructor

   CGuideView(class CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * EPOC default constructor.
    */
   void ConstructL();

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CGuideView object.
   ///        Note that this object is still on the CleanupStack.
   static class CGuideView* NewLC(CWayFinderAppUi* aUi, isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CGuideView object.
   static class CGuideView* NewL(CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CGuideView();

public: // New Functions

   void SetLeftSideTraffic(TBool aLeftSideTraffic);
   /**
    * Handle a update route info message.
    * @param aUpdateMessage a UpdateRouteinfoMessage.
    */
   void UpdateStatus( isab::RouteEnums::OnTrackEnum aStatus,
                      TDesC &aCurrStreet,
                      TDesC &aNextStreet,
                      TUint &aDistance );

   /**
    * Handle a update route info message.
    * @param aAction
    * @param aCrossing
    * @param aCurrStreet
    * @param aNextStreet
    * @param aDistance
    * @param aExit
    * @param aHighway    indicates if we are dealing with highways, which 
    *                    affects what pictures we are showing.
    */
   void UpdateWayPoint( isab::RouteEnums::RouteAction aCurrAction,
                        isab::RouteEnums::RouteAction aNextAction,
                        isab::RouteEnums::RouteCrossing aCurrCrossing,
                        isab::RouteEnums::RouteCrossing aNextCrossing,
                        TDesC &aCurrStreet, TDesC &aNextStreet,
                        TUint &aCurrDistance, TUint &aNextDistance,
                        TInt &aExit, TInt32 aLat, TInt32 aLon,
                        TBool aHighway = EFalse,
                        TInt detour = 0, TInt speedcam = 0);

   void UpdateDistance( TUint aDistance );

   void UpdateCurrentTurn( TInt aTurn );

   void SetCurrentTurn( TInt aTurn );

   TInt GetCurrentTurn();

   void UpdateSpeedcam( TUint aSpeedCam );

   void UpdateDetour( TUint aDetour );

   void GoToTurn( TBool aNextTurn );
   
   void RequestRouteListL();

   void SetRouteList( const isab::RouteList* aRouteList );

   TInt GetDistanceMode();

   TInt GetDistanceModeWithSpace();

   void PictureError( TInt aError );

   TPtrC GetMbmName();

   TBool IsNightMode();

   void GetForegroundColor(TRgb& aRgb);

   void GetNightmodeBackgroundColor(TRgb& aRgb);

   /**
    * Turns night mode on or off depending on aNightMode.
    * @param aNightMode True if nightmode should be switched on
    *                   False if nightmode should be switched off.
    */
   void SetNightModeL(TBool aNightMode);

   /**
    * Returns the mbm index of the current turn picture (aka GuidePicture).
    * Is needed when switching from nightmode to normal mode.
    * @return Index of the current turn picture.
    */
   TInt GetCurrentTurnPicture();

   /**
    * Returns the mmb index of the next turn picture and mask.
    * Is needed when switching from nightmode to normal mode.
    * @param aMbmIndex index of the next turn picture.
    * @param aMaskMbmIndex index of the mask for next turn picture.
    */
   void GetNextTurnPicture(TInt& aMbmIndex, TInt& aMaskMbmIndex);

   void UpdateRouteDataL(TInt32 aDistanceToGoal, TInt32 aEstimatedTimeToGoal);

   //static TPictures GetPicture( RouteAction aAction,
   //                             RouteCrossing aCrossing,
   //                             TUint &aDistance, TBool aHighway );
private:

   void ShowOnMap( TBool aShowRoute );

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
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   class CGuideContainerPreviewController* iContainer;
#else
   class CGuideContainer* iContainer;
#endif

   /// The last shown turn.
   TPictures iCurrentPicture;

   TBuf<KBuf64Length> iCurrentStreet;
   
   TPictures iNextTurn;

   TBuf<KBuf64Length> iNextStreet;

   TInt  iLastDistance;

   isab::Log* iLog;

   TBool iLeftSideTraffic;

   TInt iCurrentTurn;

   TPoint iTurnCoordinate;

   TBool iIsLastTurn;
};

#endif

// End of File
