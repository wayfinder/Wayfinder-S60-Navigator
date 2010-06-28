/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __MAIN_MENU_LIST_CONTAINER_H__
#define __MAIN_MENU_LIST_CONTAINER_H__

#include <eiklbo.h>

/**
 *  CMainMenuListContainer, container control class.
 */
class CMainMenuListContainer : public CCoeControl,
                               public MEikListBoxObserver
{
public: 
   
   /**
    * NewL.
    * Two-phased constructor.
    *
    * @param    aRect                   The rectangle this container will be drawn to.
    * @param    aView                   The view.
    * @param    aMbmImageIds            An array of image IDs.
    * @param    aMbmMaskIds             An array of image mask IDs.
    * @param    aMainMenuCommandIds     An array of commands.
    * @param    aMainMenuFirstLabelIds  An array of texts for first label in listbox.
    * @param    aMainMenuSecondLabelIds An array of texts for second label in listbox.
    * @return   Pointer to the created
    *           instance of CMainMenuListContainer.
    */
   static CMainMenuListContainer* NewL(const TRect& aRect, 
                                       class CMainMenuListView* aView, 
                                       const TInt* aMbmImageIds,
                                       const TInt* aMbmMaskIds,
                                       const TInt* aMainMenuCommandIds,
                                       const TInt* aMainMenuFirstLabelIds,
                                       const TInt* aMainMenuSecondLabelIds);
   
   /**
    * NewLC.
    * Two-phased constructor.
    *
    * @param    aRect                   The rectangle this container will be drawn to.
    * @param    aView                   The view.
    * @param    aMbmImageIds            An array of image IDs.
    * @param    aMbmMaskIds             An array of image mask IDs.
    * @param    aMainMenuCommandIds     An array of commands.
    * @param    aMainMenuFirstLabelIds  An array of texts for first label in listbox.
    * @param    aMainMenuSecondLabelIds An array of texts for second label in listbox.
    * @return Pointer to the created
    *         instance of CMainMenuListContainer.
    */
   static CMainMenuListContainer* NewLC(const TRect& aRect, 
                                        class CMainMenuListView* aView, 
                                        const TInt* aMbmImageIds,
                                        const TInt* aMbmMaskIds,
                                        const TInt* aMainMenuCommandIds,
                                        const TInt* aMainMenuFirstLabelIds,
                                        const TInt* aMainMenuSecondLabelIds);
   
   /**
    * ~CMainMenuListContainer.
    * Destructor.
    */
   virtual ~CMainMenuListContainer();
   
protected: 
   
   /**
    * CMainMenuListContainer.
    * C++ default constructor.
    */
   CMainMenuListContainer();

private: 
   
   /**
    * ConstructL.
    * 2nd phase constructor.
    *
    * @param    aRect                   The rectangle this container will be drawn to.
    * @param    aView                   The view.
    * @param    aMbmImageIds            An array of image IDs.
    * @param    aMbmMaskIds             An array of image mask IDs.
    * @param    aMainMenuCommandIds     An array of commands.
    * @param    aMainMenuFirstLabelIds  An array of texts for first label in listbox.
    * @param    aMainMenuSecondLabelIds An array of texts for second label in listbox.
    */
   void ConstructL(const TRect& aRect, 
                   class CMainMenuListView* aView, 
                   const TInt* aMbmImageIds,
                   const TInt* aMbmMaskIds,
                   const TInt* aMainMenuCommandIds,
                   const TInt* aMainMenuFirstLabelIds,
                   const TInt* aMainMenuSecondLabelIds);
   
public:

   /** 
    * Adds an item to the grid.
    *
    * @param   aBitmapId   ID of the bitmap that should be in the grid.
    * @param   aMaskId     ID of the mask that should be in the grid.
    */
   void AddItemL(TInt aBitmapId, TInt aMaskId, 
                 TInt aFirstLabelId, TInt aSecondLabelId);


   /** 
    * Issues the command of the currently selected item in the grid.
    */
   void ActivateSelection();

   /** 
    * Sets the currently active selection in the grid.
    *
    * @param   aIndex   Index of the item to set as active.
    */
   void SetActiveSelection(TInt aIndex);

   /** 
    * Gets the ID currently selected item in the grid.
    *
    * @return   Index of the currently active selection.
    */
   TInt GetActiveSelection();

public: // From base classes

   /**
    * From MEikListBoxObserver
    * Handles listbox event.
    * @param aListBox Pointer to ListBox object is not used.
    * @param aEventType Type of listbox event.
    */
   void HandleListBoxEventL(CEikListBox* aListBox,
                            TListBoxEvent aEventType);

protected: // Functions from base classes

   /**
    * Needed to draw the background skin.
    */
   TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

private:
   
   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the view size is changed.
    */
   void SizeChanged();

   /** 
    * Framework function called when we switch layour.
    */
   void HandleResourceChange(TInt aType);

   /**
    * From CCoeControl, OfferKeyEventL.
    * Handles key events.
    *
    * @param aKeyEvent the key event.
    * @param aType The type of the event.
    * @return Indicates whether or not the key event was used by
    * this control.
    */
   TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

   /**
    * From CCoeControl,Draw.
    */
   void Draw(const TRect& aRect) const;

   /**
    * Finds the largest possible font for the specified string and available width.
    *
    * @param aTextToFit, the text to fit.
    * @param aMaxWidthInPixels, the available width we have in pixels.
    * @param aPreferredLogicalFontId, the logical font we prefer for the string.
    */
   CFont* FindLargestPossibleFontL(const TDesC& aTextToFit, TInt aMaxWidthInPixels, enum TAknLogicalFontId aPreferredLogicalFontId) const;

   /**
    * Hides the second row in the listbox, used for small screens to avoid
    * scrolling in the list.
    */
   void HideSecondRow(TBool aHide);

private:

   /// The listbox
   class CAknDoubleLargeStyleListBox* iListBox;
   /// The grid view.
   class CMainMenuListView* iView;
   /// Int array of command ids.
   const TInt* iMainMenuCommandIds;
   /// To be able to draw a skinned background
   class CAknsBasicBackgroundControlContext* iBgContext;
};

#endif  // __MAIN_MENU_LIST_CONTAINER_H__

// End of File
