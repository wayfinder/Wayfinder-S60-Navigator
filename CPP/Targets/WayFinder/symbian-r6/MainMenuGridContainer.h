/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __MAINMENUGRIDCONTAINER_H__
#define __MAINMENUGRIDCONTAINER_H__

#include <akngridview.h>
#include <eiklbo.h>

/**
 *  CMainMenuGridContainer  container control class.
 *
 */
class CMainMenuGridContainer : public CCoeControl,
                               public MCoeControlObserver,
                               public MEikListBoxObserver
{
public: 
   
   /**
    * NewL.
    * Two-phased constructor.
    *
    * @param    aRect                 The rectangle this container will be drawn to.
    * @param    aView                 The view.
    * @param    aMbmImageIds          An array of image IDs.
    * @param    aMbmMaskIds           An array of image mask IDs.
    * @param    aMainMenuCommandIds   An array of commands.
    * @return   Pointer to the created
    *           instance of CMainMenuGridContainer.
    */
   static CMainMenuGridContainer* NewL(const TRect& aRect, 
                                       class CMainMenuGridView* aView, 
                                       const TInt* aMbmImageIds,
                                       const TInt* aMbmMaskIds,
                                       const TInt* aMainMenuCommandIds);
   
   /**
    * NewLC.
    * Two-phased constructor.
    *
    * @param    aRect                 The rectangle this container will be drawn to.
    * @param    aView                 The view.
    * @param    aMbmImageIds          An array of image IDs.
    * @param    aMbmMaskIds           An array of image mask IDs.
    * @param    aMainMenuCommandIds   An array of commands.
    * @return Pointer to the created
    *         instance of CMainMenuGridContainer.
    */
   static CMainMenuGridContainer* NewLC(const TRect& aRect, 
                                        class CMainMenuGridView* aView, 
                                        const TInt* aMbmImageIds,
                                        const TInt* aMbmMaskIds,
                                        const TInt* aMainMenuCommandIds);
   
   /**
    * ~CMainMenuGridContainer.
    * Destructor.
    */
   virtual ~CMainMenuGridContainer();
   
protected: 
   
   /**
    * CMainMenuGridContainer.
    * C++ default constructor.
    */
   CMainMenuGridContainer();

private: 
   
   /**
    * ConstructL.
    * 2nd phase constructor.
    *
    * @param    aRect                 The rectangle this container will be drawn to.
    * @param    aView                 The view.
    * @param    aMbmImageIds          An array of image IDs.
    * @param    aMbmMaskIds           An array of image mask IDs.
    * @param    aMainMenuCommandIds   An array of commands.
    */
   void ConstructL(const TRect& aRect, 
                   class CMainMenuGridView* aView, 
                   const TInt* aMbmImageIds,
                   const TInt* aMbmMaskIds,
                   const TInt* aMainMenuCommandIds);
   
public:

   /** 
    * Adds an item to the grid.
    *
    * @param   aBitmapId   ID of the bitmap that should be in the grid.
    * @param   aMaskId     ID of the mask that should be in the grid.
    */
   void AddItemL(TInt aBitmapId, TInt aMaskId, TInt aIndex = -1);

   /** 
    * Removes an item from the grid.
    *
    * @param   aIndex   Index of the item to remove.
    */
   void DeleteItemL(TInt aIndex);

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

private:

   void MakeGridL(const TRect& aRect);

   /** 
    * Initializes the grid.
    */
   void SetupGridItemsL();

   /** 
    * Sets the grid layout.
    */
   void SetGridLayoutL();

   /** 
    * Sets the grid graphic style.
    */
   void SetGridGraphicStyleL();

   /** 
    * Calculates the icon sizes in the grid for scalable UI.
    */
   void SetIconSizes();

   /** 
    * Helper function that updates the grid layout.
    */
   void UpdateGridLayoutL();

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
    * From CoeControl, CountComponentControls.
    * Returns the number of components owned by this container.
    *
    * @return Number of component controls
    */
   TInt CountComponentControls() const;
   
   /**
    * From CCoeControl, ComponentControl.
    * Returns pointer to a particular control.
    *
    * @param aIndex Index of wanted control
    * @return Pointer to component control
    */
   CCoeControl* ComponentControl(TInt aIndex) const;
   
   /**
    * From CCoeControl, Draw.
    * Draws this container to the screen.
    *
    * @param aRect The region of the control to be redrawn.
    */
   void Draw(const TRect& aRect) const;

   /**
    * From MCoeControlObserver, HandleControlEventL
    * Handle event from observed control.
    *
    * @param aControl The control that sent the event.
    * @param aEventType The event type.
    */
   void HandleControlEventL(CCoeControl* aControl, TCoeEvent aEventType);

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

private:

   /**
    * iGrid, grid.
    * Owned by CMainMenuGridContainer object.
    */
   class CAknGrid* iGrid;

   /**
    * iGridM, grid model.
    * Not owned by CMainMenuGridContainer object.
    */
   class CAknGridM* iGridM;

   /**
    * iGridType, the specific grid type
    */
   TInt iGridType;

   /**
    * The grid view.
    */
   class CMainMenuGridView* iView;

   /**
    * Int array of image ids.
    */
   const TInt* iMbmImageIds;

   /**
    * Int array of mask ids.
    */
   const TInt* iMbmMaskIds;

   /**
    * Int array of command ids.
    */
   const TInt* iMainMenuCommandIds;

   /**
    * iVerticalScrollingType, scrolling type
    */
   CAknGridView::TScrollingType iVerticalScrollingType;
   
   /**
    * iHorizontalScrollingType, scrolling type
    */
   CAknGridView::TScrollingType iHorizontalScrollingType;

   /**
    * iVerticalOrientation, params used by SetGridLayoutL().
    */
   TBool iVerticalOrientation;
   
   /**
    * iLeftToRight, fill order status
    */
   TBool iLeftToRight;
   
   /**
    * iTopToBottom, fill order status
    */
   TBool iTopToBottom;

   /**
    * iNumOfColumns, number of grid columns
    */
   TInt iNumOfColumns;
   
   /**
    * iNumOfRows, number of grid rows
    */
   TInt iNumOfRows;

   /**
    * iCellSize, size of grid cells
    */
   TSize iCellSize;

   /**
    * iIconSize, size of grid icons
    */
   TRect iIconRect;

};

#endif  // __MAINMENUGRIDCONTAINER_H__

// End of File
