/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POSITIONSELECTCONTAINER_H
#define POSITIONSELECTCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <eiklbo.h>

#ifdef NAV2_CLIENT_SERIES60_V5
# include <aknlongtapdetector.h>
#endif

#include "PositionSelectView.h"
#include "Log.h"

// CLASS DECLARATION
class CAknDoubleLargeStyleListBox;

/**
*  CPositionSelectContainer  container control class.
*  
*/
class CPositionSelectContainer : public CCoeControl,
                                 MCoeControlObserver,
                                 MEikListBoxObserver
#ifdef NAV2_CLIENT_SERIES60_V5
                               , public MAknLongTapDetectorCallBack
#endif
{

public: // Constructors and destructor

   CPositionSelectContainer(isab::Log* aLog) : iLog(aLog) {}

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( const TRect& aRect, 
                    CPositionSelectView* aView );

   /**
    * Destructor.
    */
   ~CPositionSelectContainer();

public: // New functions

   void ShowPopupListL();

   /**
    * Displays a stylus popup menu with options of how the
    * user wants to set origin, dest etc
    *
    * @param aLocation, the top left corner of the popup menu.
    */
   void ShowStylusPopUpMenuL(const TPoint aLocation);

   /**
    * Change to the selected page.
    */
   void SetOrigin( TDesC &aName );

   /**
    * Change to the selected page.
    */
   void SetDestination( TDesC &aName );

   void SetSelected(TInt index);

   /**
    * @return true if the origin is selected.
    */
   TBool IsOriginSelected();

   TInt GetActiveSelection();

   /**
    * Display a CAknQueryListDialog that lets the user decide for what
    * transportation mode the route should be calculated.
    *
    * @return The selected index, if user clicks ok.
    *         TTransportationMode::EInvalidVehicleType if user clicks cancel.
    */
   TInt ShowRouteAsQueryDialogL();

private:

   void AddItemL( TDesC &aListItem, TBool aIsDestination );

#ifdef NAV2_CLIENT_SERIES60_V3
   void AddToIconList(CArrayPtr<CGulIcon>* aIcons, 
                      const TDesC& aFilename, 
                      TInt aIconId, 
                      TInt aIconMaskId);
#endif

public: // Functions from base classes
   
   // From MEikListBoxObserver

   /**
    * Handles listbox event.
    * @param aListBox Pointer to ListBox object is not used.
    * @param aEventType Type of listbox event.
    */
   void HandleListBoxEventL( CEikListBox* aListBox,
                             TListBoxEvent aEventType );

   /**
    * Gets called by framework when a tap event has occured.
    * We are notifying the iLongTapDetector (only if the current item
    * is a top hit) that a tap has occured, if it is a long tap 
    * our HandleLongTapEventL will get called by the iLongTapDetector. 
    */
   void HandlePointerEventL( const TPointerEvent& aPointerEvent );

   /**
    * Gets called by CAknLongTapDetector (iLongTapDetector) when
    * a long tap has occured. We then launch a CAknStylusPopUpMenu
    * enabling the user to select navigate to, show information etc.
    */
   void HandleLongTapEventL( const TPoint& aPenEventLocation, 
                             const TPoint& aPenEventScreenLocation );

   void HandleResourceChange(TInt aType);

protected: // Functions from base classes

   /**
    * Needed to draw the background skin.
    */
   TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

private: // Functions from base classes

   /**
    * From CoeControl,SizeChanged.
    */
   void SizeChanged();

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

#ifdef NAV2_CLIENT_SERIES60_V5
   /**
    * Calculate the size and positions for the two buttons.
    * 
    * @param aFirstButton, the button that will be positioned at
    *                      the bottom of the screen.
    * @param aSecondButton, this button will be positioned above
    *                       aFirstButton.
    * @return The complete rect for the buttons including the padding
    *         above and below the buttons.
    *                       
    */
   TRect PositionButtonsL(class CAknButton* aFirstButton, 
                          class CAknButton* aSecondButton);

  /**
   * Calculates the size and position for one button. Used when showing
   * a fairly large button at the bottom of the screen, as opposed to the
   * two-button version where both buttons are a bit smaller in size.
   *
   * @param aButton The button to position.
   * @return The complete rect of the button, including padding around it.
   */
   TRect PositionButtonL(class CAknButton* aButton);

   /**
    * Creates the buttons to be used in this container and calls
    * PositionButtonsL to position them out.
    * 
    * @return The complete rect for the buttons including the padding
    *         above and below the buttons.
    */
   TRect CreateAndPositionButtonsL();


#endif

private: //data

   /// The parent view
   CPositionSelectView* iView;

   /// The links list.
   CAknDoubleLargeStyleListBox* iListBox;

   isab::Log* iLog;

#ifdef NAV2_CLIENT_SERIES60_V5
   /// Handles longtap events, calls HandleLongTapEventL when a longtap event is detected
   class CAknLongTapDetector* iLongTapDetector;
   /// Handles the context sensitiv menu that will be launched on longtap detection
   class CAknStylusPopUpMenu* iPopUpMenu;
   /// Button that calcs a route for car mode
   class CAknButton* iCarButton;
   /// Button that calcs a route for pedestrian mode
   //class CAknButton* iPedButton;
   /// Label displaying title above the car and pedestrian button
   //class CEikLabel* iTitleLabel;
#endif
   /// Bool flag for knowing when the popup menu has been launched.
   TBool iPopUpMenuDisplayed;
   /// Stores the latest position of a button up event, this is due to in
   /// HandleListBoxEventL (EEventItemDoubleClicked) the Stylus popup menu 
   /// should be displayed, this however needs a position of where to display it.
   TPoint iLastPointerPos;
   /// To be able to draw a skinned background
   class CAknsBasicBackgroundControlContext* iBgContext;
};

#endif

// End of File
