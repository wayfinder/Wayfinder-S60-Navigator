/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef INFOVIEW_H
#define INFOVIEW_H

// INCLUDES
#include <aknview.h>
#include <e32std.h>
#include "Log.h"
#include "TimeOutNotify.h"

// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CInfoContainer;

namespace isab{
   class UpdatePositionMess;
}

// CLASS DECLARATION

/**
*  CInfoView view class.
* 
*/
class CInfoView : public CAknView,
                  public MTimeOutNotify
{
public: // Constructors and destructor
   CInfoView(isab::Log* aLog);
   /**
    * EPOC default constructor.
    */
   void ConstructL( CWayFinderAppUi* aWayFinderUI );

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CInfoView object.
   ///        Note that this object is still on the CleanupStack.
   static class CInfoView* NewLC(CWayFinderAppUi* aUi, isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this functionc returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CInfoView object.
   static class CInfoView* NewL(CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CInfoView();

public: // New functions;

   void SetInputData(int32 lat, int32 lon, TInt heading, TInt speed, TInt alt, TInt aGpsState);

   void ForceRefresh();

   void UpdateGpsState( TInt aGpsState );

   /** From MTimeOutNotify. */
   void TimerExpired();
   
#if 0
   void UpdateDataL( UpdatePositionMess* aPositionMess );
#endif

   void UpdateRouteDataL( TInt32 aDistanceToGoal,
                          TInt32 aEstimatedTimeToGoal,
                          TInt aSpeedLimit );

   TInt GetDistanceMode();

private: // New functions

   void ShowOnMap();

   void AttemptReading();
   
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

   /// If the heading is usable or not.
   bool IsHeadingUsable() const;

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
   void UpdateContainer();
   void ReadContainer();

   /** Timer that periodically checks if the maps are fully downloaded */
   class CTimeOutTimer* iTimer;
   
   /// A pointer to the creating UI.
   CWayFinderAppUi * iWayFinderUI;
   
   /// The controls container.
   CInfoContainer* iContainer;

   TInt32 iDistance;

   TVwsViewId iPrevViewId;

   TUid iCustomMessageId;

   isab::Log * iLog;

	TTime iReferenceTime;
   TInt iLastReadGpsState;
   

   int32 iCurLat;
   int32 iCurLon;
   TInt iCurHeading;
   TInt iCurSpeed;
   TInt iCurAlt;
   TInt iCurGpsState;
   TBool iDisplayList;
};

#endif

// End of File
