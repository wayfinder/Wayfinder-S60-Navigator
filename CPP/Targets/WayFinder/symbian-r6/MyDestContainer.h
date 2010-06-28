/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MYDESTCONTAINER_H
#define MYDESTCONTAINER_H

// INCLUDES
#include <coecntrl.h>  // for coe control
#include <eiklbo.h>    // for listbox observer
#include <e32std.h>

#ifdef NAV2_CLIENT_SERIES60_V5
# include <aknlongtapdetector.h>
#endif

#include "WayFinderConstants.h"
#include "Log.h"
// CONSTANTS
const TInt KMyDestViewInitialId = 0;
const TInt KMyDestTextBufLength = 1024;
// for findbox.
const TInt KMyDestFindBoxTextLength(20);

class CMyDestView;
class CAknDoubleGraphicStyleListBox;
class CAknSearchField;

// CLASS DECLARATION

/**
 *  CMyDestContainer  container control class.
 *  
 *  XXX To get a solid listbox and findbox we need to have a 
 *  CAknColumnListBox* iListBox since if we use a double style 
 *  CAknDoubleStyleListBox* it will crash on the component control
 *  when it return the listbox. The crash happens after calling the 
 *  HandleFixedFindSizeChanged to get the correct layout in the view
 *  This means we can only get one line per favorite in the list. XXX
 * 
 */
class CMyDestContainer : public CCoeControl,
                                MCoeControlObserver,
                                MEikListBoxObserver
#ifdef NAV2_CLIENT_SERIES60_V5
                       , public MAknLongTapDetectorCallBack
#endif
{
public: // Constructors and destructor

   CMyDestContainer(isab::Log* aLog);
        
   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( const TRect& aRect, CMyDestView* aView );

   /**
    * Destructor.
    */
   ~CMyDestContainer();

   enum TFavIconEnum {
      EFavoriteIconIndex = 0,
      ELandmarkIconIndex = 1,
      ETotalNbrIcons = 2
   };

public: // New functions

   /**
    * Hack to make sure the findbox/selectlist does not crash with 
    * eikcoctl 63 the second time you set focus to true or offer 
    * any key event to the find box in a multiview application.
    * (see above comment on what really needs to be done)
    */
   void SetFindBoxFocus(TBool aFocus);

   /**
    * @return true if the listbox is empty.
    */
   TBool IsListEmpty();

   /**
    * @return the index of the selected favorite.
    */
   TInt GetSelectedIndex();

   /**
    * Set how many favorites there are favorite.
    */
   void SetNumberFavorites( TInt ANbrFavs );

   /**
    * Display a dialog for creating or editing destinations.
    */
   TBool ShowDialogL( TDes &aName, TDes &aDescription, 
                      TDes &aCategory, TDes &aAlias,
                      TInt32 &aLat, TInt32 &aLon, TBool &aCanceled );

   /**
    * Add an item to the list. 
    *
    * @param aListItem The item to add.
    */
   void AddItemL( const TDesC& aListItem, enum TFavIconEnum aIconIndex );

   /**
    * Removes an item from the list.
    *
    * @param aIndex, The item to delete
    */
   void RemoveItemL(TInt aIndex);

   /**
    * Return the currently selected item.
    */
   TBool GetSelItem( TDes &aItem );
   
   /**
    * Remove all the items.
    */
   void RemoveAllItemsL();

   /**
    * Displays a pop up list with different choices, e.g navigat to.
    */
   void ShowPopupListL();

   /**
    * Set the correct texts and image in the preview pop up.
    */
   void UpdatePreviewContentL();

   /**
    * Show or hide the preview pop up controll.
    */
   void ShowPreviewContent(TBool aShow);

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

   /// The containers parent view.
   CMyDestView* iView;

   /// List with the destinations
#if defined NAV2_CLIENT_SERIES60_V5
   class CAknSingleGraphicStyleListBox* iListBox;
#else
   class CAknDoubleGraphicStyleListBox* iListBox;
#endif

   /// The search box for the list
   CAknSearchField* iFindBox;

   /// The number of destinations in the list.
   TInt iNumberDests;

   /// Array containing all loaded icons to be displayed in the list
   CArrayPtr<CGulIcon>* iIcons;

   isab::Log* iLog;

#if defined NAV2_CLIENT_SERIES60_V5
   /// The content shown in the popup controller
   class CPreviewPopUpContent* iPreviewContent;
   /// The popup controller displayed at the top of the listbox
   class CAknPreviewPopUpController* iPreviewController;
   /// Handles longtap events, calls HandleLongTapEventL when a longtap event is detected
   class CAknLongTapDetector* iLongTapDetector;
   /// Handles the context sensitiv menu that will be launched on longtap detection
   class CAknStylusPopUpMenu* iPopUpMenu;
#endif

   /// To be able to draw a skinned background
   class CAknsBasicBackgroundControlContext* iBgContext;
   /// Bool flag for knowing when the popup menu has been launched.
   TBool iPopUpMenuDisplayed;

   /// Bool flag to know if the preview controller is shown or not.
   TBool iShowingPreviewController;
};

#endif

// End of File
