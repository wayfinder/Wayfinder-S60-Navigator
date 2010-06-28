/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STARTPAGECONTAINER_H
#define STARTPAGECONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <eiklbo.h>

#include "StartPageView.h"

// CLASS DECLARATION
class CAknSingleLargeStyleListBox;

/**
*  CStartPageContainer  container control class.
*  
*/
class CStartPageContainer : public CCoeControl,
                            MCoeControlObserver,
                            MEikListBoxObserver
{

public: // Constructors and destructor

   CStartPageContainer(isab::Log* aLog, TInt aMode) : 
      iLog(aLog), iMode(aMode)
   {}

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( const TRect& aRect, 
                    CStartPageView* aView );

   /**
    * Destructor.
    */
   ~CStartPageContainer();

public: // New functions

   /**
    * Change to the selected page.
    */
   void GoToSelection();

/*    void SetList(TBool onoff); */

public: // Functions from base classes
   
   // From MEikListBoxObserver

   /**
    * Handles listbox event.
    * @param aListBox Pointer to ListBox object is not used.
    * @param aEventType Type of listbox event.
    */
   void HandleListBoxEventL( CEikListBox* aListBox,
                             TListBoxEvent aEventType );

private:
#ifdef NAV2_CLIENT_SERIES60_V3
   void AddToIconList(CArrayPtr<CGulIcon>* aIcons, 
                      const TDesC& aFilename, 
                      TInt aIconId, 
                      TInt aIconMaskId);
#endif

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

   /**
    * From CCoeControl : Handling key event.
    * @param aKeyEvent : Information of inputted key. 
    * @param aType : EEventKeyDown | EEventKey | EEventKeyUp
    * @return EKeyWasConsumed if keyevent is used.
    *         Otherwise, return EKeyWasNotConsumed. 
    */
   TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType);

   /** 
    * From CCoeControl, HandleResourcChange
    * Gets called when switching layout.
    */
   void HandleResourceChange(TInt aType);

private: //data

   /// The parent view
   CStartPageView* iView;

   /// The links list.
   CAknSingleLargeStyleListBox* iListBox;

   isab::Log* iLog;

   const TInt iMode;
};

#endif

// End of File
