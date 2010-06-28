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
#include <aknview.h>

#include "GuideContainer.h"
#include "WayFinderConstants.h"
#include "Log.h"
#include "RouteEnums.h"
#include "RouteInfo.h"
#include "TurnPictures.h"
#include "DistancePrintingPolicy.h"

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
class CGuideView : public CAknView
{

private:

   class DistInfo {
   public:
      int32 sayAtDistance;
      int32 abortTooShortDistance;
      int32 abortTooFarDistance;
      bool  supress;
      DistInfo() : sayAtDistance(-1), abortTooShortDistance(-1), 
                   abortTooFarDistance(-1), supress(false)
      { }
      DistInfo(int32 a, int32 b, int32 c, bool d) : 
         sayAtDistance(a), abortTooShortDistance(b), abortTooFarDistance(c), supress(d)
      { }
   };

public: // Constructors and destructor

   CGuideView(isab::Log* aLog);

   /**
    * EPOC default constructor.
    */
   void ConstructL( CWayFinderAppUi* aWayFinderUI );

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


   HBufC* GetCurrentTextInstruction();
   
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


   /* Some stuff needs to be activated from Vicinity feed aswell */
   void SharedActivation();
   
   void CreateContainer();
   
   TInt GetCurrentDistance();
   
   void UpdateDistance( TUint aDistance, TBool aSkipTTS = FALSE );

   void UpdateCurrentTurn( TInt aTurn );

   void SetCurrentTurn( TInt aTurn );

   TInt GetCurrentTurn();

   void UpdateSpeedcam( TUint aSpeedCam );

   void UpdateDetour( TUint aDetour );

   void GoToTurn( TBool aNextTurn );
   
   void RequestRouteListL();

   void SetRouteList( const isab::RouteList* aRouteList );

   TInt GetDistanceMode();

   void PictureError( TInt aError );

   TPtrC GetMbmName();

   //static TPictures GetPicture( RouteAction aAction,
   //                             RouteCrossing aCrossing,
   //                             TUint &aDistance, TBool aHighway );
private:
   
   void ShowOnMap( TBool aShowRoute );

   void InitDistanceTableMetric();
        
   void InitDistanceTableFeetMiles();
        
   void InitDistanceTableYardsMiles();

   TInt ShouldUpdateNavInstruction(TInt aDistance);

   bool GetCurrentDistInfo(TInt aDistance, DistInfo& aDistInfo);

   void InitDistVector();

   void ResetSupressedNavInst();


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

   const void GetRouteActionInitialString(isab::RouteEnums::RouteAction &aAction, 
                                          TBool &aPedestrianMode,
                                          TBuf<128> &buffer);

   const void GetRouteActionString(isab::RouteEnums::RouteAction &aAction,
                                   TUint aDistance,
                                   TBuf<128> &buffer);

   const void GetRouteActionPrePositionString(isab::RouteEnums::RouteAction &aAction,
                                              TBuf<128> &buffer);

   TBool 
   CombinePhrasesWithDistance(TUint &aDistance,
                                          HBufC * aInstruction);

   TBool
   GetRouteDescriptionL(isab::RouteEnums::RouteAction &aAction,
                        TUint &aDistance,
                        TInt aExit,
                        TInt aDistanceMode,
                        TBool aPedestrianMode);

   void GetTurnInstruction(TUint aDistance, HBufC* aInstruction);

   void AlertOffTrack();

private: // Data
   HBufC* iCurrentInstruction;
   
   /// A pointer to the creating UI.
   CWayFinderAppUi*   iWayFinderUI;
   
   /// The controls container.   
   CGuideContainer* iContainer;

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

   TTime iReferenceTime;

   TBool iChangeTurn;

   // TEXT INSTRUCTION for BLIND MODE
   //
   // All of the following variables are needed to update the text instruction 
   // without constructing it each time the distance changes.

   /**
    * This buffer holds the initial phrase for the instruction. This is 
    * something like: "You are on " to say the actual position.
    *
    * At the moment the initial phrase is not in dodona therefore it is still 
    * unused.
    */
   TBuf<128> iInitialPhrase;

   /**
    * Here the current street name is hold. 
    */
   TDesC * iCurrStreetName;

   /**
    * Distance mode. kmh or mph?
    */
   TInt iDistanceMode;

   /**
    * This buffer hold the actual instruction. This means if you should walk, 
    * drive or turn around.
    */
   TBuf<128> iInstructionPhrase; // Walk or Drive or something else
	
   /**
    * The action needs to be stored too. It is needed cause the instruction 
    * looks different if it is a start od end instruction. In these cases 
    * the phrases are combined in a different way.
    */
   isab::RouteEnums::RouteAction iAction;
	
   /**
    * The instruction that will follow.
    */
   TBuf<128> iActionPhrase; 
	
   /**
    * And the preposition pointing on the next street.
    */
   TBuf<128> iPreposition;
	
   /**
    * Name of the street you will enter with the next instruction.
    */
   TDesC * iNextStreetName;

   std::vector<DistInfo> iDistVector;

   isab::DistancePrintingPolicy::DistanceMode iCurrentUnits;

};

#endif

// End of File
