/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __MAINMENUGRIDVIEW_H__
#define __MAINMENUGRIDVIEW_H__

#include "ViewBase.h"
#include "WayfinderCommandCallback.h"

/**
*  CMainMenuGridView view class.
*
*/
class CMainMenuGridView : public CViewBase, 
                          public WayfinderCommandCallback
{
public: 
   
   /**
    * NewL.
    * Two-phased constructor.
    * Construct a CMainMenuGridView using two phase construction,
    * and return a pointer to the created object
    *
    * @param    aWayFinderAppUi   The AppUi.
    * @return   A pointer to the created
    *           instance of CMainMenuGridView.
    */
   static CMainMenuGridView* NewL(class CWayFinderAppUi* aWayFinderAppUi);
   
   /**
    * NewLC.
    * Two-phased constructor.
    * Construct a CMainMenuGridView using two phase construction,
    * and return a pointer to the created object
    *
    * @param    aWayFinderAppUi   The AppUi.
    * @return   A pointer to the created
    *           instance of CMainMenuGridView.
    */
   static CMainMenuGridView* NewLC(class CWayFinderAppUi* aWayFinderAppUi);
   
   /**
    * ~CMainMenuGridView.
    * Destructor.
    */
   virtual ~CMainMenuGridView();
   
protected:
   
   /**
    * CMainMenuGridContainer.
    * C++ default constructor.
    */
   CMainMenuGridView(class CWayFinderAppUi* aWayFinderAppUi);
   
private:
   
   /**
    * ConstructL.
    * 2nd phase constructor.
    *
    * @param    aWayFinderAppUi   The AppUi.
    */
   void ConstructL();
   
public:

   /** 
    * Returns the Mbm image file path.
    *
    * @return   Mbm image file name.
    */
   TPtrC GetMbmName();

   /**
    * Sets the label in the navigation pane of the phone.
    *
    * @param    aLabelId   A resource id to the text that should be set.
    */
   void SetNaviPaneLabelL(TInt aLabelId);

   /**
    * Resets the navi pane label with the current selection.
    */
   void ResetNaviPaneLabelL();

   /**
    * Reinitializes the container. We need to do this when switching 
    * layout on s60v3 since the grid control does not work correctly 
    * with the scalable UI.
    */
   void ReInitContainer();

   /**
    * From WayfinderCommandCallback
    * Handles commands from the WAX Browser component.
    *
    * @param   aCont   Parameter container.
    */
   void HandleCommandL(const class CWAXParameterContainer& aCont);

   void UpdateActiveSelection();

   void UpdateSelectedIndex(TInt aIndex);

   TInt GetSelectedIndex();

public: 
   
   /**
    * From CAknView
    * Returns the ID of view.
    *
    * @return The ID of view.
    */
   TUid Id() const;
   
   /**
    * From CAknView, HandleCommandL.
    * Handles the commands.
    * @param aCommand Command to be handled.
    */
   void HandleCommandL(TInt aCommand);
   
   /**
    * From CAknView, HandleClientRectChange.
    * Handles the size of the clientrect.
    */
   void HandleClientRectChange();
   
private:

   /**
    * From CAknView, Called when the option softkey is pressed.
    *
    * @param aResourceId the focused resource.
    * @param aMenuPane the current menu.
    */
   void DynInitMenuPaneL(TInt aResourceId, 
                         class CEikMenuPane* aMenuPane);

   /**
    * From CAknView, DoActivateL.
    * Creates the Container class object.
    * @param aPrevViewId Specified TVwsViewId.
    * @param aCustomMessageId Specified TUid.
    * @param aCustomMessage Specified custom message.
    */
   void DoActivateL(const TVwsViewId& aPrevViewId, 
                    TUid aCustomMessageId,
                    const TDesC8& aCustomMessage);
   
   /**
    * From CAknView, DoDeactivate.
    * Deletes the Container class object.
    */
   void DoDeactivate();
   
private:

   /**
    * iContainer, container for this view
    * Owned by CMainMenuGridView object.
    */
   class CMainMenuGridContainer* iContainer;

   /**
    * Currently or last selected item.
    */
   TInt iSelectedItem;

};

#endif  // __MAINMENUGRIDVIEW_H__

// End of File
