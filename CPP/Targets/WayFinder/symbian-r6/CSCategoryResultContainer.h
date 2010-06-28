/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CS_CATEGORY_RESULT_CONTAINER_H
#define CS_CATEGORY_RESULT_CONTAINER_H

#include <coecntrl.h>		
#include <akniconutils.h>
#include <gulicon.h>
#include <vector>
#include <eiklbo.h>

#ifdef NAV2_CLIENT_SERIES60_V5
# include <aknlongtapdetector.h>
#endif

#include "EventGenerator.h"

class MEikCommandObserver;		
class CAknSingleGraphicHeadingStyleListBox;
class CEikTextListBox;

/**
 * Container class for STestResultListBox
 * 
 * @class	CCSCategoryResultContainer STestResultListBox.h
 */
class CCSCategoryResultContainer : public CCoeControl,
                                   public MEikListBoxObserver
#ifdef NAV2_CLIENT_SERIES60_V5
                                 , public MAknLongTapDetectorCallBack
#endif
{
public:
   // constructors and destructor
   CCSCategoryResultContainer(class CCSCategoryResultView* aView);


   /**
    * Construct the control (first phase).
    *  Creates an instance and initializes it.
    *  Instance is not left on cleanup stack.
    * @param aRect bounding rectangle
    * @param aParent owning parent, or NULL
    * @param aCommandObserver command observer
    * @return initialized instance of CCSCategoryResultContainer
    */
   static CCSCategoryResultContainer* NewL( const TRect& aRect, 
                                            class CCSCategoryResultView* aView );

   /**
    * Construct the control (first phase).
    *  Creates an instance and initializes it.
    *  Instance is left on cleanup stack.
    * @param aRect The rectangle for this window
    * @param aParent owning parent, or NULL
    * @param aCommandObserver command observer
    * @return new instance of CCSCategoryResultContainer
    */
   static CCSCategoryResultContainer* NewLC( const TRect& aRect, 
                                             class CCSCategoryResultView* aView );

   /**
    * Construct the control (second phase).
    *  Creates a window to contain the controls and activates it.
    * @param aRect bounding rectangle
    * @param aCommandObserver command observer
    * @param aParent owning parent, or NULL
    */ 
   void ConstructL( const TRect& aRect );

   virtual ~CCSCategoryResultContainer();

public:   // from base class CCoeControl
   /**
    * Return the number of controls in the container (override)
    * @return count
    */
   TInt CountComponentControls() const;

   /**
    * Get the control with the given index (override)
    * @param aIndex Control index [0...n) (limited by #CountComponentControls)
    * @return Pointer to control
    */
   CCoeControl* ComponentControl( TInt aIndex ) const;

   /**
    *	Handle key events.
    */				
   TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, 
                                TEventCode aType );

   /** 
    * Observer function from MEikListBoxObserver, here we handle
    * events as double click etc.
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

   /** 
    * Handle global resource changes, such as scalable UI or skin events (override)
    */
   void HandleResourceChange( TInt aType );
	
public: // New functions

   /**
    * Checks if current selected item in the listbox is a top hit or not
    * @return ETrue if current item is a top hit.
    *         EFalse if not a top hit.
    */
   TBool IsTopHit();

protected:   // from base class CCoeControl
   /**
    *	Handle resizing of the container. This implementation will lay out
    *  full-sized controls like list boxes for any screen size, and will layout
    *  labels, editors, etc. to the size they were given in the UI designer.
    *  This code will need to be modified to adjust arbitrary controls to
    *  any screen size.
    */
   void SizeChanged();

private:   // from base class CCoeControl
   /**
    *	Draw container contents.
    */
   void Draw( const TRect& aRect ) const;

private:
   /**
    *	Initialize each control upon creation.
    */
   void InitializeControlsL();

public:
   enum TCSEvent {
      ECSAddCategories
   };

   enum TControls {
      EListBox,
      ELastControl
   };

   void GenerateEvent(enum TCSEvent aEvent);

   void HandleGeneratedEventL(enum TCSEvent aEvent);

   void AddListBoxItemL( TInt aHeading, const char* aMainText, const char* aImageName );

   void InsertListBoxItemL(TInt aHeading, const char* aMainText, 
                           const char* aImageName, TInt aIndex);

   /**
    * Get the array of selected item indices, with respect to the list model.
    * The array is sorted in ascending order.
    * The array should be destroyed with two calls to CleanupStack::PopAndDestroy(),
    * the first with no argument (referring to the internal resource) and the
    * second with the array pointer.
    * @return newly allocated array, which is left on the cleanup stack;
    *	or NULL for empty list. 
    */
   RArray< TInt >* GetSelectedListBoxItemsLC( CEikTextListBox* aListBox );

   /**
    * Delete the selected item or items from the list box.
    */
   void DeleteSelectedListBoxItemsL( CEikTextListBox* aListBox );

   /**
    *	Get the listbox.
    */
   CAknSingleGraphicHeadingStyleListBox* ListBox();

   /**
    *	Create a list box item with the given column values.
    */
   void CreateListBoxItemL( TDes& aBuffer, 
                            TInt aIconIndex,
                            const TDesC& aHeadingText,
                            const TDesC& aMainText );

   HBufC* CreateListBoxItemL( TInt aHeading,
                              const char* aMainText,
                              const char* aImageName );

   /**
    *	This routine loads and scales a bitmap or icon.
    *
    *	@param aFileName the MBM or MIF filename
    *	@param aBitmapId the bitmap id
    *	@param aMaskId the mask id or -1 for none
    *	@param aSize the TSize for the icon, or NULL to use real size
    *	@param aScaleMode one of the EAspectRatio* enums when scaling
    *
    */
   CGulIcon* LoadAndScaleIconL( const TDesC& aFileName );
   
   /** 
    *	Handle commands relating to markable lists.
    */
   TBool HandleMarkableListCommandL( TInt aCommand );

   /**
    * Does the actual work of adding categories.
    *
    * @param aCategoryVec   The vector of categories to be added to listbox
    */
   void AddCategoryResultsL( const std::vector<class CombinedSearchCategory*>& categories );

   /**
    * Returns the correct index for categories, comensated for top hits.
    * @return The correct index, compensated for top hits.
    */
   TInt GetSelectedIndex();

   /**
    * Returns the current index in the listbox.
    * @return Current index in the listbox.
    */
   TInt GetCurrentIndex();

   void SetSelectedIndex(TInt aIndex);

private: 
   class CCSCategoryResultView* iView;
   CAknSingleGraphicHeadingStyleListBox* iListBox;

   /// Eventgenerator sending and receiving events. 
   typedef CEventGenerator<CCSCategoryResultContainer, enum TCSEvent> CSEventGenerator;
   CSEventGenerator* iEventGenerator;

   CDesCArray* iList;
   CArrayPtr<CGulIcon>* iIcons;
   /// Flag telling caller that we are already busy adding categories. The request
   /// will be performed when ready.
   TBool iBusyAddingCategories;
   /// Flag telling us if we have outstanding requests for adding categories.
   TBool iOutstandingRequest;
   /// The offset in the list for top hits. The top hits are not located at the
   /// same level in the category vector, hence we need to adjust for the indexes
   /// in the listbox to match the indexes in the category vector.
   TInt iTopHitsOffset;

#ifdef NAV2_CLIENT_SERIES60_V5
   /// Handles longtap events, calls HandleLongTapEventL when a longtap event is detected
   class CAknLongTapDetector* iLongTapDetector;
   /// Handles the context sensitiv menu that will be launched on longtap detection
   class CAknStylusPopUpMenu* iPopUpMenu;
#endif
   /// Bool flag for knowing when the popup menu has been launched.
   TBool iPopUpMenuDisplayed;
};
				
#endif // STESTRESULTLISTBOX_H
