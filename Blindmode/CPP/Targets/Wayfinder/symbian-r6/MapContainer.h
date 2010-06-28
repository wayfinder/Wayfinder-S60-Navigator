/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MAPCONTAINER_H
#define MAPCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include "Log.h"
#include "PictureContainer.h"
#include "WayFinderConstants.h"

// FORWARD DECLARATIONS
class CEikLabel;
class CMapView;
class CGuidePicture;
class CCellDataDrawer;
// CLASS DECLARATION


/**
 *  CMapContainer  container control class.
 *  
 */
class CMapContainer : public CCoeControl,
                             MCoeControlObserver,
                             PictureContainer
{
public: // Constructors and destructor
   CMapContainer(isab::Log* aLog) : iLog(aLog) {}

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( const TRect& aRect, CMapView* aView, CGuidePicture* aNormalMap, TBool aHasRoute );

   /**
    * Destructor.
    */
   ~CMapContainer();

public: // New functions

   static TInt GetStaticOffset();

   static void GetStaticSize( TUint16 &aX, TUint16 &aY );

   CGuidePicture* GetNormalMapPtr();

   void SetMapImage( TDesC &aMapName );

   void SetMapImage( HBufC8* aMapImage );

   void SetFullScreen( TBool aFullScreen );

   TBool IsFullScreen();

   TBool SetScreenOffset( TInt &aX, TInt &aY, TInt aOffset );

   void GetMaxOffset( TInt &aX, TInt &aY );

   void SetZoom( TZoomFactor aNewFactor, TZoomFactor aOldFactor, TInt &aX, TInt &aY );

   void GetPixelSize( TUint16 &aX, TUint16 &aY );

   void GetScreenSize( TUint16 &aX, TUint16 &aY );

   void CloseMapFile();

   const TInt GetNormalScreenHeight();

   CCellDataDrawer& getCellDataDrawer() const {return *iCellDataDrawer;}

   /**
    * @return true if the position picture's being rotated.
    */
   TBool SetPixelPosition( TBool aShow, TInt aX, TInt aY,
         TInt aHeading, TInt aRealHeading );

   
   void SetTurnPictureL( TInt aMbmIndex );

   /*void SetTurnPicture( TPictures aTurn,
                        TDesC &aFullPath,
                        TBool aRightTraffic);*/

   void HideTurnPicture();

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
   void HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType );

private: //data

   CEikLabel* iLabel;

   CMapView* iView;

   TRect iNormalRect;

   CGuidePicture* iNormalMap;

   CGuidePicture* iFullScreenMap;

   CGuidePicture* iTurnPicture;

   CGuidePicture* iPositionSprite22;
   CGuidePicture* iPositionSprite45;
   CGuidePicture* iPositionSprite67;
   CGuidePicture* iPositionSprite90;
   CCellDataDrawer* iCellDataDrawer;

   TInt iOldDir22;
   TInt iOldDir45;
   TInt iOldDir67;
   TInt iOldDir90;

   TBool iIsFullScreen;

   TInt iMaxOffsetX;

   TInt iMaxOffsetY;

   TPoint iOffset;

   TInt iWidth;

   TInt iHeight;

   TInt iHeading;

   TInt iCenterOffsetX;
   TInt iCenterOffsetY;

   TPoint iPosition;

   TBuf<KBuf256Length> iPositionName;

   isab::Log* iLog;
};

#endif

// End of File
