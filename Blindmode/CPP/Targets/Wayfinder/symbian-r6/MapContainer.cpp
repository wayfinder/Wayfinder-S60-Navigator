/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES
#include "MapContainer.h"

#include "wayfinder.hrh"
#include "WayFinderConstants.h"
#include "GuidePicture.h"
#include "MapView.h"

/* For the bitmap enum. */
#include "wficons.mbg"
#include "RsgInclude.h"

#include "memlog.h"
#include "WayFinderAppUi.h"
#include "debuggingdefines.h"
#include "CellDataDrawer.h"

#include "Dialogs.h"

#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
#include <aknsdrawutils.h>
#endif

const TInt cScreenWidth = 176;
const TInt cNormalScreenHeight = 144;
const TInt cFullScreenHeight = 208;
const TInt cFullScreenOffset = 5;
const TInt cFullMapWidth = 186; // Width + 2*offset
const TInt cFullMapHeight = 218; // Height + 2*offset
/* const TInt cZoomInWidth = 470; */
/* const TInt cZoomInHeight = 510; */
/* const TInt cZoomOutWidth = 282; */
/* const TInt cZoomOutHeight = 306; */


_LIT( KGifPosition, "position.gif" );

// Enumarations
enum TControls
{
   ENormalMap,
   EFullScreenMap,
   EPositionSprite1,
   EPositionSprite2,
   EPositionSprite3,
   EPositionSprite4,
   ETurnPicture,
#ifdef DEBUG_CELL
   ECellData,
#endif
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CMapContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CMapContainer::ConstructL( const TRect& aRect,
                                CMapView* aView,
                                CGuidePicture* aNormalMap,
                                TBool aHasRoute )
{
   CreateWindowL();
   iView = aView;

   iWidth = cScreenWidth;
   iHeight = cNormalScreenHeight;

   iNormalRect = aRect;
   SetRect(aRect);

   if( aNormalMap == NULL ){
      iNormalMap = new (ELeave) CGuidePicture( iLog );
      LOGNEW(iNormalMap, CGuidePicture);
      iNormalMap->ConstructL( TRect( TPoint(0,0), TSize( cScreenWidth, cNormalScreenHeight ) ), this );

      iNormalMap->SetShow( ETrue );
      iNormalMap->SetClear( EFalse );
   } else {
      iNormalMap = aNormalMap;
      iNormalMap->SetPictureContainer( this );
      iNormalMap->SetShow( ETrue );
   }

   iFullScreenMap = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iFullScreenMap, CGuidePicture);
   iFullScreenMap->ConstructL( TRect( TPoint(0,0), TSize( cFullMapWidth, cFullMapHeight ) ), this );

   iFullScreenMap->SetShow( EFalse );
   iFullScreenMap->SetClear( EFalse );

   GuiDataStore* gds = iView->GetGuiDataStore();
   HBufC* wfmbmname = gds->iWayfinderMBMFilename;

   /* Create new position image for 22.5 degree angles. */
   iPositionSprite22 = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iPositionSprite22, CGuidePicture);
   iPositionSprite22->ConstructL( TRect( TPoint(0,0), TSize( 21, 21 ) ), this );
   iPositionSprite22->OpenBitmapFromMbm(*wfmbmname, EMbmWficonsBbb4_r);
   iPositionSprite22->OpenBitmapMaskFromMbm(*wfmbmname, EMbmWficonsBbb4);
   iPositionSprite22->SetShow( EFalse );
   iPositionSprite22->SetClear( EFalse );
   iOldDir22 = 0;

   /* Create new position image for 45 degree angles. */
   iPositionSprite45 = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iPositionSprite45, CGuidePicture);
   iPositionSprite45->ConstructL( TRect( TPoint(0,0), TSize( 21, 21 ) ), this );
   iPositionSprite45->OpenBitmapFromMbm(*wfmbmname, EMbmWficonsBbb1_r);
   iPositionSprite45->OpenBitmapMaskFromMbm(*wfmbmname, EMbmWficonsBbb1);
   iPositionSprite45->SetShow( EFalse );
   iPositionSprite45->SetClear( EFalse );
   iOldDir45 = 0;

   /* Create new position image for 67.5 degree angles. */
   iPositionSprite67 = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iPositionSprite67, CGuidePicture);
   iPositionSprite67->ConstructL( TRect( TPoint(0,0), TSize( 21, 21 ) ), this );
   iPositionSprite67->OpenBitmapFromMbm(*wfmbmname, EMbmWficonsBbb3_r);
   iPositionSprite67->OpenBitmapMaskFromMbm(*wfmbmname, EMbmWficonsBbb3);
   iPositionSprite67->SetShow( EFalse );
   iPositionSprite67->SetClear( EFalse );
   iOldDir67 = 3;

   /* Create new position image for 90 degree angles. */
   iPositionSprite90 = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iPositionSprite90, CGuidePicture);
   iPositionSprite90->ConstructL( TRect( TPoint(0,0), TSize( 21, 21 ) ), this );
   iPositionSprite90->OpenBitmapFromMbm(*wfmbmname, EMbmWficonsBbb2_r);
   iPositionSprite90->OpenBitmapMaskFromMbm(*wfmbmname, EMbmWficonsBbb2);
   iPositionSprite90->SetShow( EFalse );
   iPositionSprite90->SetClear( EFalse );
   iOldDir90 = 0;

#ifdef DEBUG_CELL
   iCellDataDrawer = new(ELeave) CCellDataDrawer();
   LOGNEW(iCellDataDrawer, CCellDataDrawer);
   iCellDataDrawer->ConstructL(TRect(TPoint(0,0), TSize( cScreenWidth, cNormalScreenHeight )));
#endif

   iCenterOffsetX = 10;
   iCenterOffsetY = 10;

   iTurnPicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iTurnPicture, CGuidePicture);
   iTurnPicture->ConstructL( TRect( TPoint( 133, 1 ), TSize( 42, 36 ) ), this );
   if( aView->IsGpsAllowed() ){
      iTurnPicture->SetShow( aHasRoute );
   }
   else{
      iTurnPicture->SetShow( EFalse );
   }
   iTurnPicture->SetClear( EFalse );

   iOffset.iX = 0;
   iOffset.iY = 0;
   
   iMaxOffsetX = cScreenWidth-cFullMapWidth;
   iMaxOffsetY = cFullScreenHeight-cFullMapHeight;

   iHeading = 0;

   ActivateL();
}

// Destructor
CMapContainer::~CMapContainer()
{
   //iNormalMap is deleted in the view

   LOGDEL(iFullScreenMap);
   delete iFullScreenMap;

   LOGDEL(iPositionSprite22);
   delete iPositionSprite22;

   LOGDEL(iPositionSprite45);
   delete iPositionSprite45;

   LOGDEL(iPositionSprite90);
   delete iPositionSprite90;

   LOGDEL(iPositionSprite67);
   delete iPositionSprite67;
   
   LOGDEL(iTurnPicture);
   delete iTurnPicture;

#ifdef DEBUG_CELL
   LOGDEL(iCellDataDrawer);
   delete iCellDataDrawer;
#endif
}

TInt CMapContainer::GetStaticOffset()
{
   return cFullScreenOffset;
}

void CMapContainer::GetStaticSize( TUint16 &aX, TUint16 &aY )
{
   aX = cScreenWidth;
   aY = cNormalScreenHeight;
}

CGuidePicture* CMapContainer::GetNormalMapPtr()
{
   return iNormalMap;
}

void CMapContainer::SetMapImage( TDesC &aMapName )
{
   if( iIsFullScreen ){
      iFullScreenMap->OpenGif( aMapName );
   }
   else{
      iNormalMap->OpenGif( aMapName );
   }
   DrawNow();
}

void CMapContainer::SetMapImage( HBufC8* aMapImage )
{
   if( iIsFullScreen ){
      iFullScreenMap->OpenDesc( aMapImage );
   } else{
      iNormalMap->OpenDesc( aMapImage );
   }
   DrawNow();
}

void CMapContainer::SetFullScreen( TBool aFullScreen )
{
   iIsFullScreen = aFullScreen;
   if( aFullScreen ){
      SetExtentToWholeScreen();
      iWidth = cFullMapWidth;
      iHeight = cFullMapHeight;
   }
   else{
      SetRect( iNormalRect );
      iWidth = cScreenWidth;
      iHeight = cNormalScreenHeight;
   }
   iFullScreenMap->SetShow( aFullScreen );
   iNormalMap->SetShow( !aFullScreen );
}

TBool CMapContainer::IsFullScreen()
{
   return iIsFullScreen;
}

TBool CMapContainer::SetScreenOffset( TInt &aX, TInt &aY, TInt aOffset )
{
   TBool outOfBounds = EFalse;
   if( aX > aOffset || aX < (iMaxOffsetX - aOffset) ||
       aY > aOffset || aY < (iMaxOffsetY - aOffset) )
       outOfBounds = ETrue;

   iFullScreenMap->SetScreenOffset( aX, aY );
   iPosition.iX += aX-iOffset.iX;
   iPosition.iY += aY-iOffset.iY;
   iPositionSprite22->SetScreenOffset( iPosition.iX, iPosition.iY);
   iPositionSprite45->SetScreenOffset( iPosition.iX, iPosition.iY);
   iPositionSprite67->SetScreenOffset( iPosition.iX, iPosition.iY);
   iPositionSprite90->SetScreenOffset( iPosition.iX, iPosition.iY);
   iOffset.iX = aX;
   iOffset.iY = aY;
   return outOfBounds;
}

void CMapContainer::GetMaxOffset( TInt &aX, TInt &aY )
{
   aX = iMaxOffsetX;
   aY = iMaxOffsetY;
}

void CMapContainer::GetPixelSize( TUint16 &aX, TUint16 &aY )
{
   aX = iWidth;
   aY = iHeight;
}

void CMapContainer::GetScreenSize( TUint16 &aX, TUint16 &aY )
{
   aX = cScreenWidth;
   if( iIsFullScreen )
      aY = cFullScreenHeight;
   else
      aY = cNormalScreenHeight;
}

void CMapContainer::CloseMapFile()
{
   if( iIsFullScreen ){
      iFullScreenMap->CloseImage();
   }
   else{
      iNormalMap->CloseImage();
   }
}

const TInt CMapContainer::GetNormalScreenHeight()
{
   return cNormalScreenHeight;
}

/* aHeading is no longer set!!! */
TBool CMapContainer::SetPixelPosition( TBool aShow, TInt aX, TInt aY,
                                       TInt /* aHeading */, TInt aRealHeading )
{
   TBool rotated = EFalse;
   TBool rotated2 = EFalse;
   if( aShow ) {
      CGuidePicture* curImage;
      /* Make sure that the angle is between 0 and 255. */
      if (aRealHeading < 0) {
         aRealHeading += 256;
      }
#define USE_POSITION_IMAGES
#ifdef USE_POSITION_IMAGES
      /* Divide by 16 to get values in the range 0 to 15. */
      /* Add 8 to the base value to get true rounded values. */
      /* This means that the value 0 in newdir means an angle */
      /* of 8-16 degrees, 1 in newdir means 16-24, ... */
      /* and 15 in newdir means 248-8. */
      TInt newdir = ((aRealHeading+8)%256) / 16;
      TInt* olddir;
      switch (newdir % 4) {
         case 0:
            /* Even 64 degree angles. (0, 64, 128, 192) */
            curImage = iPositionSprite90;
            iPositionSprite22->SetShow( EFalse );
            iPositionSprite45->SetShow( EFalse );
            iPositionSprite67->SetShow( EFalse );
            iPositionSprite90->SetShow( ETrue );
            olddir = &iOldDir90;
            break;
         case 1:
            /* 16 degree angles. (16, 80, 144, 208) */
            curImage = iPositionSprite22;
            iPositionSprite22->SetShow( ETrue );
            iPositionSprite45->SetShow( EFalse );
            iPositionSprite67->SetShow( EFalse );
            iPositionSprite90->SetShow( EFalse );
            olddir = &iOldDir22;
            break;
         case 2:
            /* 32 degree angles. (32, 96, 160, 224) */
            curImage = iPositionSprite45;
            iPositionSprite22->SetShow( EFalse );
            iPositionSprite45->SetShow( ETrue );
            iPositionSprite67->SetShow( EFalse );
            iPositionSprite90->SetShow( EFalse );
            olddir = &iOldDir45;
            break;
         case 3:
            /* 48 degree angles. (48, 112, 176, 240) */
            curImage = iPositionSprite67;
            iPositionSprite22->SetShow( EFalse );
            iPositionSprite45->SetShow( EFalse );
            iPositionSprite67->SetShow( ETrue );
            iPositionSprite90->SetShow( EFalse );
            olddir = &iOldDir67;
            break;
         default:
            iPositionSprite22->SetShow( ETrue );
            iPositionSprite45->SetShow( ETrue );
            iPositionSprite67->SetShow( ETrue );
            iPositionSprite90->SetShow( ETrue );
            return EFalse;
            break;
      }
      /* Check rotation. */
      TInt newdir2 = newdir / 4;
      CGuidePicture::TImageRotateAngle rotation = CGuidePicture::ENoRotation;
      TInt foo = newdir2-(*olddir);
      if (foo < 0) {
         /* Negative rotation. */
         /* Convert the rotation in counter clockwise direction */
         /* to the appropriate number of rotations in the */
         /* clockwise direction. */
         foo += 4;
      }
      switch (foo) {
         case 0:
            /* No rotation needed. */
            break;
         case 1:
            /* Need rotation 90 degrees. */
            rotation = CGuidePicture::E90Degrees;
            break;
         case 2:
            /* Need rotation 180 degrees. */
            rotation = CGuidePicture::E180Degrees;
            break;
         case 3:
            /* Need rotation 270 degrees. */
            rotation = CGuidePicture::E270Degrees;
            break;
         default:
            /* This is an impossible value! */
            return EFalse;
            break;
      }
      if (curImage->IsReady() && rotation != CGuidePicture::ENoRotation) {
         /* Need to rotate. */
         rotated2 = curImage->RotateImageClockwiseL( rotation );
         *olddir = newdir2;
      }

      iPosition.iX = aX;
      iPosition.iY = iHeight-aY;
      iPosition.iX -= iCenterOffsetX;
      iPosition.iY -= iCenterOffsetY;
      iPositionSprite22->SetScreenOffset( iPosition.iX, iPosition.iY);
      iPositionSprite45->SetScreenOffset( iPosition.iX, iPosition.iY);
      iPositionSprite67->SetScreenOffset( iPosition.iX, iPosition.iY);
      iPositionSprite90->SetScreenOffset( iPosition.iX, iPosition.iY);
   } else {
      iPositionSprite22->SetShow( EFalse );
      iPositionSprite45->SetShow( EFalse );
      iPositionSprite67->SetShow( EFalse );
      iPositionSprite90->SetShow( EFalse );
   }
#else
   /* Draw the position indicator as a vector triangle. */

#endif
   return rotated;
}

void CMapContainer::SetTurnPictureL( TInt aMbmIndex )
{
   if( aMbmIndex >= 0 ){
      /*HBufC* mbmName = iView->GetMbmName();
      CleanupStack::PushL(mbmName);*/
      iTurnPicture->SetShow( ETrue );
      iTurnPicture->OpenBitmapFromMbm( iView->GetMbmName(), aMbmIndex );
      //CleanupStack::PopAndDestroy (mbmName);
   }
   else{
      iTurnPicture->SetShow( EFalse );
   }
}

/*void CMapContainer::SetTurnPicture( TPictures aTurn,
                                    TDesC &aFullPath,
                                    TBool aRightTraffic )
{   
   TBuf<256> aFileName ( aFullPath );

   iTurnPicture->SetShow( ETrue );
   switch( aTurn )
   {
   case E3WayLeft:
   case E3WayTeeLeft:
   case E4WayLeft:
   case ELeftArrow:
   case ERdbLeft:
      aFileName.Append(KGifSmallLeftArrow);
      break;
   case E3WayRight:
   case E3WayTeeRight:
   case E4WayRight:
   case ERdbRight:
   case ERightArrow:
      aFileName.Append(KGifSmallRightArrow);
      break;
   case E4WayStraight:
   case EHighWayStraight:
   case EStraight:
   case EStraightArrow:
   case ERdbStraight:
      aFileName.Append(KGifSmallStraightArrow);
      break;
   case EEnterHighWay:
   case EEnterMainRoad:
      if( aRightTraffic ){
         aFileName.Append(KGifSmallKeepLeft);
      }
      else{
         aFileName.Append(KGifSmallKeepRight);
      }
      break;
   case EExitHighWayLeft:
   case EExitMainRoadLeft:
   case EKeepLeft:
      aFileName.Append(KGifSmallKeepLeft);
      break;
   case EExitHighWay:
   case EExitMainRoad:
      if( aRightTraffic ){
         aFileName.Append(KGifSmallKeepRight);
      }
      else{
         aFileName.Append(KGifSmallKeepLeft);
      }
      break;
   case EExitHighWayRight:
   case EExitMainRoadRight:
   case EKeepRight:
      aFileName.Append(KGifSmallKeepRight);
      break;
   case EFerry:
      aFileName.Append(KGifSmallFerry);
      break;
   case EFinishArrow:
   case EFinishFlag:
      aFileName.Append(KGifSmallFlag);
      break;
   case EMultiWayRdb:
      if (aRightTraffic) {
         aFileName.Append(KGifSmallMultiWayRdb);
      } else {
         aFileName.Append(KGifSmallMultiWayRdbLeft);
      }
      break;
   case EPark:
      aFileName.Append(KGifSmallPark);
      break;
   case ERdbUTurn:
   case EUTurn:
      if (aRightTraffic) {
         aFileName.Append(KGifSmallUTurn);
      } else {
         aFileName.Append(KGifSmallUTurnLeft);
      }
      break;
   default:
      iTurnPicture->SetShow( EFalse );
      break;
   }
   iTurnPicture->OpenGif(aFileName);
}*/

void CMapContainer::HideTurnPicture()
{
   iTurnPicture->SetShow( EFalse );
}

void CMapContainer::PictureError( TInt aError )
{
   iView->PictureError( aError );
}

void CMapContainer::ScalingDone()
{
   DrawNow();
}

// ----------------------------------------------------------------------------
// TKeyResponse CGuideContainer::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
TKeyResponse CMapContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                            TEventCode aType )
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }
   if ( aKeyEvent.iScanCode == EStdKeyDevice3 ){
      iView->HandleCommandL( EWayFinderCmdMapReroute );
      return EKeyWasConsumed;
   }
   else{
      if( iIsFullScreen ){
         switch (aKeyEvent.iScanCode)
         {
         case EStdKeyUpArrow:
            iView->HandleCommandL(EWayFinderCmdMapMoveUp);
            break;
         case EStdKeyDownArrow:
            iView->HandleCommandL(EWayFinderCmdMapMoveDown);
            break;
         case EStdKeyLeftArrow:
            iView->HandleCommandL(EWayFinderCmdMapMoveLeft);
            break;
         case EStdKeyRightArrow:
            iView->HandleCommandL(EWayFinderCmdMapMoveRight);
            break;
         }
         return EKeyWasConsumed;
      }
      else
         return EKeyWasNotConsumed;
   }
}

// ---------------------------------------------------------
// CMapContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CMapContainer::SizeChanged()
{
}

// ---------------------------------------------------------
// CMapContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CMapContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CMapContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CMapContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case ENormalMap:
      return iNormalMap;
   case EFullScreenMap:
      return iFullScreenMap;
   case EPositionSprite1:
      return iPositionSprite22;
   case EPositionSprite2:
      return iPositionSprite45;
   case EPositionSprite3:
      return iPositionSprite67;
   case EPositionSprite4:
      return iPositionSprite90;
   case ETurnPicture:
      return iTurnPicture;
#ifdef DEBUG_CELL
   case ECellData:
      return iCellDataDrawer;
#endif
   default:
      Assert(false);
      return NULL;
   }
}

// ---------------------------------------------------------
// CMapContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CMapContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();

   gc.SetPenStyle(CGraphicsContext::ENullPen);
   gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );             
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);

}

// ---------------------------------------------------------
// CMapContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CMapContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

// End of File  
