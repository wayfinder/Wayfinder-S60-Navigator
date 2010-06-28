/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef SEARCHABLE_LIST_BOX_CONTAINER_H
#define SEARCHABLE_LIST_BOX_CONTAINER_H

#include <coecntrl.h>		
#include <eiklbo.h>

class MEikCommandObserver;		
class CAknSingleStyleListBox;
class CEikTextListBox;
class CAknSearchField;

/**
 * Container class for STestListBox
 * 
 * @class	CSearchableListBoxContainer STestListBox.h
 */
class CSearchableListBoxContainer : public CCoeControl,
                                    public MEikListBoxObserver	
{
public:

   /**
    * Construct the control (first phase).
    *  Creates an instance and initializes it.
    *  Instance is not left on cleanup stack.
    * @param aRect bounding rectangle
    * @param aView the parent view
    * @param aCommandObserver command observer
    * @return initialized instance of CSearchableListBoxContainer
    */
   static CSearchableListBoxContainer* NewL( const TRect& aRect, 
                                             class CSearchableListBoxView& aView, 
                                             MEikCommandObserver* aCommandObserver );

   /**
    * Construct the control (first phase).
    *  Creates an instance and initializes it.
    *  Instance is left on cleanup stack.
    * @param aRect The rectangle for this window
    * @param aView the parent view
    * @param aCommandObserver command observer
    * @return new instance of CSearchableListBoxContainer
    */
   static CSearchableListBoxContainer* NewLC( const TRect& aRect, 
                                              class CSearchableListBoxView& aView, 
                                              MEikCommandObserver* aCommandObserver );
   /** 
    * Destroy child controls.
    */
   virtual ~CSearchableListBoxContainer();

protected:
   /**
    * First phase of Symbian two-phase construction. Should not 
    * contain any code that could leave.
    */
   CSearchableListBoxContainer(class CSearchableListBoxView& aView);

private:
   /**
    * Construct the control (second phase).
    *  Creates a window to contain the controls and activates it.
    * @param aRect bounding rectangle
    * @param aCommandObserver command observer
    */ 
   void ConstructL( const TRect& aRect, 
                    MEikCommandObserver* aCommandObserver );

public:
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
    * Handle global resource changes, such as scalable UI or skin events
    */
   void HandleResourceChange( TInt aType );
	
protected:
   /**
    *  Handle resizing of the container. This implementation will lay out
    *  full-sized controls like list boxes for any screen size, and will layout
    *  labels, editors, etc. to the size they were given in the UI designer.
    *  This code will need to be modified to adjust arbitrary controls to
    *  any screen size.
    */	
   void SizeChanged();

private:
   /**
    *	Draw container contents.
    */	
   void Draw( const TRect& aRect ) const;

   /**
    *	Initialize each control upon creation.
    */	
   void InitializeControlsL();

   /**
    * Layout components as specified in the UI Designer
    */
   void LayoutControls();

public: 
   void AddListBoxItemL(const TDesC& aString);

   /**
    *	Add a list box item to a list.
    */
   void AddFormattedListBoxItemL(const TDesC& aString);

   /**
    *	Add a list box array to list.
    */
   void AddFormattedListBoxItemsL( MDesCArray* aArray );

   /**
    * Get the array of selected item indices, with respect to the list model.
    * The array is sorted in ascending order.
    * The array should be destroyed with two calls to CleanupStack::PopAndDestroy(),
    * the first with no argument (referring to the internal resource) and the
    * second with the array pointer.
    * @return newly allocated array, which is left on the cleanup stack;
    *	or NULL for empty list. 
    */
   static RArray< TInt >* GetSelectedListBoxItemsLC( CEikTextListBox* aListBox );

   /**
    * Delete the selected item or items from the list box.
    */
   static void DeleteSelectedListBoxItemsL( CEikTextListBox* aListBox );

   /**
    *	Get the listbox.
    */
   CAknSingleStyleListBox* ListBox();

   /**
    * Return the number of items in the listbox (-1 if no items).
    */
   TInt CountListBoxItems();

   /**
    *	Create a list box item with the given column values.
    */
   void CreateListBoxItemL( TDes& aBuffer, 
                                   const TDesC& aMainText );

   void SetSelectionL(TInt aIndex);
   void AddListBoxResourceArrayItemL( TInt aResourceId );

   /**
    *	Set up the list's icon array.
    */
   void SetupListBoxIconsL();

   /** 
    *	Handle commands relating to markable lists.
    */
   TBool HandleMarkableListCommandL( TInt aCommand );

   /**
    *
    */
   const TPtrC GetValue();

   /**
    *
    */
   TInt GetSelectedId();

protected: 
   /** 
    * Override of the HandleListBoxEventL virtual function
    */
   void HandleListBoxEventL( CEikListBox* aListBox,
                             TListBoxEvent aEventType);

public: 
   enum TControls {
      EListBox,
      EListBoxSearchField,
      ELastControl
   };

   enum TListBoxImages {
      EListBoxFirstUserImageIndex		
   };
	
private:
   class CSearchableListBoxView& iView;

   CAknSingleStyleListBox* iListBox;
   CAknSearchField* iListBoxSearchField;

   CCoeControl* iFocusControl;
   MEikCommandObserver* iCommandObserver;
};
				
#endif // STESTLISTBOX_H
