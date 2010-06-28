/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _GUIDE_CONTAINER_PREVIEW_CONTROLLER_H_
#define _GUIDE_CONTAINER_PREVIEW_CONTROLLER_H_

#include <coecntrl.h>

/**
 * CGuideContainerPreviewController container control class. 
 */
class CGuideContainerPreviewController : public CCoeControl
{
public:
   /**
    * NewL.
    * Two-phased constructor.
    * Create a CGuideContainerPreviewController object.
    *
    * @param aRect The rectangle this view will be drawn to.
    * @param aView The view that creates this container.
    * @return a pointer to the instance of CGuideContainerPreviewController.
    */
   static CGuideContainerPreviewController* NewL(const TRect& aRect, 
                                                 class CGuideView& aView);

   /**
    * NewLC.
    * Two-phased constructor.
    * Create a CGuideContainerPreviewController object.
    *
    * @param aRect Rectangle this view will be drawn to.
    * @param aView The view that creates this container.
    * @return A pointer to the instance of CGuideContainerPreviewController.
    */
   static CGuideContainerPreviewController* NewLC(const TRect& aRect, 
                                                  class CGuideView& aView);

   /**
    * ~CGuideContainerPreviewController
    * Virtual Destructor.
    */
   virtual ~CGuideContainerPreviewController();

protected:
   /**
    * CGuideContainerPreviewController.
    * C++ default constructor.
    */
   CGuideContainerPreviewController(class CGuideView& aView);

private:
   /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CGuideContainerPreviewController object.
    *
    * @param aRect The initial rect for the content.
    */
   void ConstructL(const TRect& aRect);

protected: // Functions from base classes
   /**
    * Needed to draw the background skin.
    */
   TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the view size is changed.
    */
   virtual void SizeChanged();

   /**
    * From CCoeControl
    * Handles layout awarness.
    */
   virtual void HandleResourceChange(TInt aType);

   /**
    * Gets called by framework if control's focus changes.
    */
   void FocusChanged(TDrawNow aDrawNow);

private: // Functions from base classes
   /**
    * From CCoeControl
    * Draws screen.
    */
   void Draw( const TRect& aRect ) const;

public: // Public functions
   /**
    * Hides or shows the preview content.
    */
   void ShowPreviewContent(TBool aShow);

   /**
    * Updates the guide picture.
    */
   void SetGuidePictureL(TInt aMbmIndex);

   /**
    * Updates the distance left.
    */
   void SetDistanceL(TUint aDistance);

   /**
    * Updates the next turn image.
    */
   void SetNextTurnPictureL(TInt aMbmIndex, TInt aMbmMaskIndex);

   /**
    * Shows or hides the detour picture.
    */
   void SetDetourPicture(TInt on);

   /**
    * Shows or hides the speed cam picture.
    */
   void SetSpeedCamPicture(TInt on);

   /**
    * Updates the round-about exit count image.
    */
   void SetExitL(TInt aMbmIndex, TInt aMbmMaskIndex = -1);

   /**
    * Updates the current and next street names.
    */
   void SetStreetsL(const TDesC& aCurrName, const TDesC& aNextName);

   /**
    * Handles updates from the route data when navigating.
    */
   void UpdateRouteDataL(TInt32 aDistanceToGoal, TInt32 aEstimatedTimeToGoal);

   /**
    * Turns night mode on or off depending on aNightMode.
    * @param aNightMode True if nightmode should be switched on
    *                   False if nightmode should be switched off.
    */
   void SetNightModeL(TBool aNightMode);

private:
   /**
    * Converts and sets the distance to goal.
    */
   void SetDistanceToGoalL(TInt32 aDistanceToGoal);

   /**
    * Converts and sets the estimated time to goal.
    */
   void SetEtgL(TInt32 aEstimatedTimeToGoal);

   /**
    * Converts and sets the estiamted time of arrival.
    */
   void SetEtaL(TInt32 aEstimatedTimeToGoal);

private:
   /// The Guide View.
   class CGuideView& iView;

   /// To be able to draw a skinned background
   class CAknsBasicBackgroundControlContext* iBgContext;

   /// Label printing the distance to next route turn/action.
   class CEikLabel* iDistanceLabel;

   /// The guide image showing next turn/action.
   class CImageHandler* iGuidePicture;

   /// The next turn image.
   class CImageHandler* iNextTurnPicture;

   /// The next turn image.
   class CImageHandler* iSpeedCamPicture;

   /// The next turn image.
   class CImageHandler* iDetourPicture;

   /// The round about exit count image.
   class CImageHandler* iExitPicture;

#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   /// The content shown in the popup controller
   class CGuidePreviewPopUpContent* iPreviewContent;
   /// The popup controller displayed at the top of the listbox
   class CAknPreviewPopUpController* iPreviewController;
#endif

};

#endif //_GUIDE_CONTAINER_PREVIEW_CONTROLLER_H_
