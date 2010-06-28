/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GUIDECONTAINER_H
#define GUIDECONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include "Log.h"
#include "PictureContainer.h"
#include "WayFinderConstants.h"

// FORWARD DECLARATIONS
class CEikLabel;
class CGuideView;
class CGuidePicture;
class CGuideProgressBar;
class CWayFinderAppUi;

// CLASS DECLARATION

/**
 *  CGuideContainer  container control class.
 *  
 */
class CGuideContainer : public CCoeControl,
                               MCoeControlObserver,
                               PictureContainer
{
public: // Constructors and destructor

   CGuideContainer(isab::Log* aLog) : iLog(aLog) {}
        
   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL(CWayFinderAppUi* appUI,
            const TRect& aRect, CGuideView* aView );

   /**
    * Destructor.
    */
   ~CGuideContainer();

public: // New functions

   void SetGuidePictureL( TInt aMbmIndex );

   void SetNextTurnPictureL( TInt aMbmIndex , TInt aMbmMaskIndex );

   void SetDetourPicture(TInt on);
   void SetSpeedCamPicture(TInt on);

   void SetDistanceL( TUint aDistance );

   void SetExitL( TInt aMbmIndex, TInt aMbmMaskIndex = -1 );

   void SetStreetsL( TDesC &aCurrName, TDesC &aNextName );

   /**
    * Turns night mode on or off depending on aNightMode.
    * @param aNightMode True if nightmode should be switched on
    *                   False if nightmode should be switched off.
    */
   void SetNightModeL(TBool aNightMode);

private: // New Functions

   void GetRoundedDistance( TUint aDistance, TUint &aNewDistance );
   void SetLabelExtent(class CEikLabel* aLabel, 
                       const TPoint &aPosition, 
                       TBool aExtendToSceenWidth = EFalse);
   void SetLabelExtent(class CEikLabel* aLabel, 
                       float aWidth,
                       float aHeight, 
                       TBool aExtendToSceenWidth = EFalse);
   void CreatePictureL(class CImageHandler*& aImage, const TRect& aRect, 
                       const TPoint aPoint, TInt aWidth, TInt aHeight);
   void CalculateLandscapeLayout();
   void CalculatePortraitLayout();
//    void CreateTrafficIcons(TBool aLayoutSwitch = EFalse);
   void SetImage(class CImageHandler*& aImage, 
                 const TRect& aBaseRect,
                 TRect (*ptrFunc) (const class TRect& aRect, const class TSize& aSize),
                 float aXPos, float aYPos,
                 TInt aDx = 0, TInt aDy = 0);

   /**
    * Sets the position of an image that should be positioned
    * within a larger rect.
    * @param aImage, The image to be positioned.
    * @param aBaseRect, The rect that the image should be positioned within.
    * @param TRect (*ptrFunc), A pointer to a function that handles positioning within
    *                          a rect.
    * @param aDx, The padding in X line.
    * @param aDy, The padding in Y line.
    */
   void SetImagePos(class CImageHandler*& aImage, 
                    const TRect& aBaseRect,
                    TRect (*ptrFunc) (const class TRect& aRect, const class TSize& aSize),
                    TInt aDx = 0, TInt aDy = 0);

   void RefreshPictures();

public: // Functions from base classes

   /**
    * From PictureContainer
    */
   void PictureError( TInt aError );

   void ScalingDone();

   /**
    * From CoeControl.
    */
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

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
   CCoeControl* ComponentControl( TInt aIndex ) const;

   /**
    * From CCoeControl,Draw.
    */
   void Draw( const TRect& aRect ) const;

   /**
    * From ?base_class ?member_description
    */
   // event handling section
   // e.g Listbox events
   void HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType );

   /**
    * From CCoeControl
    * Handles layout awarness.
    */
   void HandleResourceChange(TInt aType);
private: //data

   CGuideView* iView;

   CEikLabel* iCurrStreetLabel;
   
   CEikLabel* iNextStreetLabel;

   CEikLabel* iDistanceLabel;

#ifdef NAV2_CLIENT_SERIES60_V3
   CImageHandler* iGuidePicture;
   CImageHandler* iNextTurnPicture;
   CImageHandler* iDetourPicture;
   CImageHandler* iSpeedCamPicture;
   CImageHandler* iExitPicture;
#else
   CGuidePicture* iGuidePicture;
   CGuidePicture* iNextTurnPicture;
   CGuidePicture* iDetourPicture;
   CGuidePicture* iSpeedCamPicture;
   CGuidePicture* iExitPicture;
#endif

   CGuideProgressBar* iProgressBar;

   CWayFinderAppUi* iAppUI;

   isab::Log* iLog;

   TBool iGuidePictureLoaded;
   TBool iLayoutChanged;

   TInt iR, iG, iB;
   TInt iImagePaddingX;
   TInt iImagePaddingY;
};

#endif

// End of File
