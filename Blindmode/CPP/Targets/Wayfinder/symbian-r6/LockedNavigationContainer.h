/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOCKED_NAVIGATION_CONTAINER
#define LOCKED_NAVIGATION_CONTAINER

#include "DistancePrintingPolicy.h"

class CLockedNavigationView;
class CWayFinderAppUi;
class TRect;

#include <coecntrl.h>
#include "ScalableFonts.h"
#include "TimeOutNotify.h"
#include "WFTextUtil.h"
#include "MC2Coordinate.h"
#include "VicinityItem.h"
#include <vector>
#include "MC2Coordinate.h"
#include "MC2Direction.h"

/**
 *	   Component class for display of distance and heading to a 
 *		specific target. Also displays "Where am I"-information.
 *
 */


class CLockedNavigationContainer : public CCoeControl, 
                                   public MCoeControlObserver,
                                   public MTimeOutNotify
{
public:
   ~CLockedNavigationContainer();

   void
   ConstructL(const TRect& aRect, 
              CLockedNavigationView* aView,
              CWayFinderAppUi * aWayFinderUI);
public: //Functions from base classes
   void SizeChanged();

   TInt CountComponentControls() const;

   CCoeControl* ComponentControl(TInt aIndex) const;

   void Draw(const TRect& aRect) const;

   void HandleControlEventL(CCoeControl * aControl, TCoeEvent aEventType);

   TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,
                               TEventCode aType);

   void UpdateText( HBufC* aTargetName,
                    HBufC* aWhereAmI,
                    TInt distance,
                    MC2Direction targetDirection,
                    MC2Direction userDirection);


   void GetDistance(TUint aDistance, TDes &aText);
   
   TInt GetDistanceModel();
   
   /**
    * From CCoeControl
    * Handles layout awarness.
    */
   void HandleResourceChange(TInt aType);

   /** From MTimeOutNotify. */
   void TimerExpired();
private:
   void AppendDirection(   TPtr& connector,
                           const MC2Direction& direction,
                           int heading = 0);

   CEikLabel * iLocationLabel;
   
   CLockedNavigationView* iView;
   CWayFinderAppUi* iWayFinderUI;
};




#endif
