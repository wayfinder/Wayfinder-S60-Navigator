/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOCKED_NAVIGATION_VIEW
#define LOCKED_NAVIGATION_VIEW

#include <aknview.h> // for CAknView
#include "wayfinder.hrh"
#include "WayFinderConstants.h"
#include "VicinityItem.h"
#include "Log.h"
#include "VicinityItem.h"
#include "MC2Coordinate.h"

class CWayFinderAppUi;

class CLockedNavigationContainer;

class CLockedNavigationView : public CAknView {
public:
   
	/**
	 * Factory method to create an object of this class.
	 */
   
   static CLockedNavigationView* NewLC(CWayFinderAppUi* aWayFinderUI,
                                       isab::Log* aLog);

	/**
	 * Factory method to create an object of this class.
	 */
   
   static CLockedNavigationView* NewL(CWayFinderAppUi* aWayFinderUI,
                                      isab::Log* aLog );


   void SetTarget(VicinityItem* aTarget);
   ~CLockedNavigationView();
private:
   /**
    * Constructor
    *
    * Constructor is private to ensure the objects are created through
    * the static NewLx methods.
    */
   CLockedNavigationView(isab::Log* aLog);


   /**
    * Second phase constructor called by the factory methods NewLx().
    *
    */

   void ConstructL(CWayFinderAppUi* aWayFinderUI);
public:
   
   void SetInputData(int32 lat,
                     int32 lon,
                     TInt heading,
                     TInt speed,
                     TInt alt,
                     TInt aGpsState);
   
   	/**
	 * Returns the id of this view. Id is defined in WayFinderConstants.h and 
	 * wayfinder.hrh
	 */
   TUid Id() const;

   /**
    * Command handling.
    */
   void HandleCommandL(TInt aCommand);

   /**
    * Changes any resizing or similar changes of th ui.
    */
   void HandleClientRectChange();
   
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
    * Cleanup method needs to be called manually before the destructor is 
    * called by itself.
    */
   void DoDeactivate();


   void ForceRefresh();
private:
   void UpdateState();
   isab::Log * iLog;
   HBufC* iTargetName;
   CLockedNavigationContainer* iContainer;
   
   MC2Coordinate iTargetCoordinate;

   CWayFinderAppUi * iWayFinderUI;
	TTime iReferenceTime;
};


#endif
