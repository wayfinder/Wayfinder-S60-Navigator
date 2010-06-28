/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NEWDESTCONTAINER_H
#define NEWDESTCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <aknlists.h>  // for avrell style listbox
#include "Log.h"
// CONSTANTS
const TInt KNewDestTextBufLength = 128;

class CNewDestView;

// CLASS DECLARATION

/**
 *  CNewDestContainer  container control class.
 *  
 */
class CNewDestContainer : public CCoeControl,
                                 MCoeControlObserver,
                                 MEikListBoxObserver
{
public: // Constructors and destructor
        
   CNewDestContainer(isab::Log* aLog) : iLog(aLog) {}

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( const TRect& aRect, CNewDestView* aView );

   /**
    * Destructor.
    */
   ~CNewDestContainer();

public: // New functions
   void SetTimerTick(TTimeIntervalMicroSeconds32  aTick);
   void StartTimer();
   void StopTimer();
   static TInt Period(TAny* aPtr);
   void DoPeriod();

   /**
    * Add an item to the list.
    * @param aItem The item to add.
    */
   void AddItemL( TUint aId, TDesC &aListItem );

   /**
    * Get the text of the selected item.
    * @param aItem where the text is returned
    * @return true if the item is found.
    *
    */
   TBool GetSelItem( TDes& aItem );

   /**
    * Remove all the items in the list.
    */
   TBool RemoveAllItemsL();

   /**
    * @return true if there are any search results.
    */
   TBool HasResults();
   
   /**
    * @return the index of the selected destiantion.
    */
   TInt GetSelIndex();

   void SetSelected(TInt index);

public: // Functions from base classes

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
   
   // From MEikListBoxObserver

   /**
    * Handles listbox event.
    * @param aListBox Pointer to ListBox object is not used.
    * @param aEventType Type of listbox event.
    */
   void HandleListBoxEventL( CEikListBox* aListBox,
                             TListBoxEvent aEventType );

   /**
    * From CCoeControl.
    * Handles layout awarness.
    */
   void HandleResourceChange(TInt aType);

private: // Functions from base classes

   /**
    * From CoeControl,SizeChanged.
    */
   void SizeChanged();

   /**
    * From CoeControl,CountComponentControls.
    */
   TInt CountComponentControls() const;

   /**
    * From CCoeControl,ComponentControl.
    */
   CCoeControl* ComponentControl(TInt aIndex) const;

   /**
    * From CCoeControl,Draw.
    */
   void Draw(const TRect& aRect) const;

   /**
    * From ?base_class ?member_description
    */
   // event handling section
   // e.g Listbox events
   void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);

private: //data
   CPeriodic* iPeriodicTimer;
   TTimeIntervalMicroSeconds32  iTick;

   /// The containers parent view.
   CNewDestView* iView;

   /// List with the destinations
   CAknDoubleStyleListBox* iListBox;

   /// Array of the destinations ids.
   RArray<TUint> iIds;

   /// Number of destinations in the list
   TInt         iNumberDests;

   isab::Log* iLog;
};

#endif

// End of File
