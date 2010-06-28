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

#include <eiklabel.h>  // for labels
#include <f32file.h>

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include <arch.h>
#include "RouteEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/GuiProtEnums.h"
#include "WayFinderAppUi.h"

#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "GuideContainer.h"
#include "GuidePicture.h"
#include "GuideProgressBar.h"
#include "GuideView.h"
#include "WFTextUtil.h"

#include "memlog.h"

#include "DistancePrintingPolicy.h"
#include "ImageHandler.h"
#include "WFLayoutUtils.h"
#include "RectTools.h"

#define S60V2_SCREEN_WIDTH    176
#define S60V2_SCREEN_HEIGHT   208
#define IMAGE_PADDING         2
#define NEXTSTREET_LABEL_POS  TPoint( 10,  5)
#define GUIDE_PICTURE_POS     TPoint( 10, 20)
#define EXIT_PICTURE_POS      TPoint( 47, 61)
#define CURRSTREET_LABEL_POS  TPoint( 10,128)
#define PROGRESS_BAR_POS      TPoint(125, 20)
#define DISTANCE_LABEL_POS    TPoint(120,110)
#define NEXT_TURN_PICTURE_POS TPoint( 84, 22)
#define DETOUR_PICTURE_POS    TPoint( 12, 94)
#define SPEEDCAM_PICTURE_POS  TPoint( 84, 94)

#define NEXT_STREET_LABEL_X_LS 0.1
#define NEXT_STREET_LABEL_Y_LS 0.0
#define NEXT_STREET_LABEL_X_PR 0.06
#define NEXT_STREET_LABEL_Y_PR 0.03

#define CURR_STREET_LABEL_X_LS 0.1
#define CURR_STREET_LABEL_Y_LS 0.85
#define CURR_STREET_LABEL_X_PR 0.06
#define CURR_STREET_LABEL_Y_PR 0.89

#define DIST_STREET_LABEL_X_LS 0.68
#define DIST_STREET_LABEL_Y_LS 0.70
#define DIST_STREET_LABEL_X_PR 0.68
#define DIST_STREET_LABEL_Y_PR 0.76

// Enumarations
enum TControls
{
   ENextStreetLabel,
   EGuidePicture,
   EExitPicture,
   ECurrStreetLabel,
   EDistanceLabel,
   EProgressBar,
   ENextTurnPicture,
   EDetourPicture,
   ESpeedCamPicture,
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CGuideContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CGuideContainer::ConstructL( CWayFinderAppUi* appUi,
      const TRect& aRect, CGuideView* aView )
{
   iAppUI = appUi;

   iView = aView;

   CreateWindowL();

   iNextStreetLabel = new (ELeave) CEikLabel;
   LOGNEW(iNextStreetLabel, CEikLabel);
   iNextStreetLabel->SetContainerWindowL( *this );
   iNextStreetLabel->SetBufferReserveLengthL( KBuf256Length );
   iNextStreetLabel->SetTextL( _L("") );

   iCurrStreetLabel = new (ELeave) CEikLabel;
   LOGNEW(iCurrStreetLabel, CEikLabel);
   iCurrStreetLabel->SetContainerWindowL( *this );
   iCurrStreetLabel->SetBufferReserveLengthL( KBuf256Length );
   iCurrStreetLabel->SetTextL( _L("") );

   iDistanceLabel = new (ELeave) CEikLabel;
   LOGNEW(iDistanceLabel, CEikLabel);
   iDistanceLabel->SetContainerWindowL( *this );
   iDistanceLabel->SetTextL( _L("           ") );


#ifdef NAV2_CLIENT_SERIES60_V3
   if (WFLayoutUtils::LandscapeMode()) {
      CalculateLandscapeLayout();
   } else {
      CalculatePortraitLayout();
   }
#else

   // FIXME hfasth, use CImageHandler here to! Then we can migrate
   // v2 and v3.

   iGuidePicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iGuidePicture, CGuidePicture);
   iGuidePicture->ConstructL( TRect( GUIDE_PICTURE_POS, TSize( 100, 100 ) ), this );

   iProgressBar = CGuideProgressBar::NewL(*this, 
                                          TRect(PROGRESS_BAR_POS, TSize(40, 90)), 
                                          iView->GetMbmName(), 
                                          EMbmWficonsGpb_triangle,
                                          EMbmWficonsGpb_triangle_active,
                                          EMbmWficonsGpb_triangle_mask,
                                          EMbmWficonsGpb_rect,
                                          EMbmWficonsGpb_rect_active,
                                          EMbmWficonsGpb_rect_mask);

   iNextTurnPicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iNextTurnPicture, CGuidePicture);
   iNextTurnPicture->ConstructL( TRect( NEXT_TURN_PICTURE_POS, TSize( 24, 24 ) ), this );

   iDetourPicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iDetourPicture, CGuidePicture);
   iDetourPicture->ConstructL( TRect( DETOUR_PICTURE_POS, TSize( 24, 24 ) ), this );

   iDetourPicture->OpenBitmapFromMbm( iView->GetMbmName(), EMbmWficonsSmall_detour );
   iDetourPicture->SetShow(EFalse);

   iSpeedCamPicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iSpeedCamPicture, CGuidePicture);
   iSpeedCamPicture->ConstructL( TRect( SPEEDCAM_PICTURE_POS, TSize( 24, 24 ) ), this );

   iSpeedCamPicture->OpenBitmapFromMbm( iView->GetMbmName(), EMbmWficonsSmall_speedcam );
   iSpeedCamPicture->SetShow(EFalse);


   iExitPicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iExitPicture, CGuidePicture);
   iExitPicture->ConstructL( TRect( EXIT_PICTURE_POS, TSize( 26, 16 ) ), this );
   iExitPicture->SetShow(EFalse);
#endif

   iAppUI->GetBackgroundColor(iR, iG, iB);

   SetRect(aRect);

   ActivateL();
}

// Destructor
CGuideContainer::~CGuideContainer()
{
   LOGDEL(iNextStreetLabel);
   delete iNextStreetLabel;
   iNextStreetLabel = NULL;

   LOGDEL(iGuidePicture);
   delete iGuidePicture;
   iGuidePicture = NULL;

   LOGDEL(iCurrStreetLabel);
   delete iCurrStreetLabel;
   iCurrStreetLabel = NULL;

   LOGDEL(iDistanceLabel);
   delete iDistanceLabel;
   iDistanceLabel = NULL;

   LOGDEL(iProgressBar);
   delete iProgressBar;
   iProgressBar = NULL;
   
   LOGDEL(iNextTurnPicture);
   delete iNextTurnPicture;
   iNextTurnPicture = NULL;

   LOGDEL(iDetourPicture);
   delete iDetourPicture;
   iDetourPicture = NULL;

   LOGDEL(iSpeedCamPicture);
   delete iSpeedCamPicture;
   iSpeedCamPicture = NULL;

   LOGDEL(iExitPicture);
   delete iExitPicture;
   iExitPicture = NULL;
}

void CGuideContainer::SetGuidePictureL( TInt aMbmIndex )
{
   if( aMbmIndex >= 0 ){
      iGuidePicture->SetShow( ETrue );
      iGuidePicture->SetClear( EFalse );
#ifdef NAV2_CLIENT_SERIES60_V3
      TRect oldRect = iGuidePicture->Rect();
      iGuidePicture->CreateIconL(iView->GetMbmName(), aMbmIndex, 
                                 EMbmWficonsKeep_left_mask, 
                                 EAspectRatioPreservedAndUnusedSpaceRemoved);
      //iGuidePicture->NightFilter(iAppUI->IsNightMode());
#else
      iGuidePicture->OpenBitmapFromMbm( iView->GetMbmName(), aMbmIndex );
      iGuidePicture->OpenBitmapMaskFromMbm( iView->GetMbmName(), EMbmWficonsTurn_mask );
#endif
      if (iLayoutChanged) {
         // If a layout switch has been done then we can assume that 
         // the size has changed. If the size has changed there is a 
         // possibility that the aspect ratio of the screen has changed.
         // Since the image rect of the guide picture is being scaled we
         // need to be sure that no left overs from the previous image
         // rect is left on the screen.
         if (oldRect.Width() < iGuidePicture->Rect().Width()) {
            // If the new width is larger than the old one, use the new
            // height when repainting the image.
            oldRect.SetWidth(iGuidePicture->Rect().Width());
         } 
         if (oldRect.Height() < iGuidePicture->Rect().Height()) {
            // If the new height is larger than the old one, use the new
            // height when repainting the image.
            oldRect.SetHeight(iGuidePicture->Rect().Height());  
         }
         iLayoutChanged = EFalse;
         Window().Invalidate(oldRect);
      } else {
         Window().Invalidate(iGuidePicture->Rect());
      } 
   } else {
      iGuidePicture->SetShow( EFalse );
      Window().Invalidate(iGuidePicture->Rect());
   }
}

void CGuideContainer::SetNextTurnPictureL( TInt aMbmIndex , TInt aMbmMaskIndex )
{
   if( aMbmIndex >= 0 ){
      SetImagePos(iNextTurnPicture,
                  iGuidePicture->GetRect(),
                  &TopRight, 
                  -iImagePaddingX,
                  iImagePaddingY);

      iNextTurnPicture->SetShow( ETrue );
      iNextTurnPicture->SetClear( EFalse );
#ifdef NAV2_CLIENT_SERIES60_V3
      iNextTurnPicture->CreateIconL(iView->GetMbmName(), aMbmIndex, aMbmMaskIndex//,
                                    /*EAspectRatioPreservedAndUnusedSpaceRemoved*/);
      //iNextTurnPicture->NightFilter(iAppUI->IsNightMode());
#else
      iNextTurnPicture->OpenBitmapFromMbm( iView->GetMbmName(), aMbmIndex );
      iNextTurnPicture->OpenBitmapMaskFromMbm( iView->GetMbmName(), aMbmMaskIndex );
#endif
      Window().Invalidate(iNextTurnPicture->Rect());
   }
   else{
      iNextTurnPicture->SetShow( EFalse );
      Window().Invalidate(iNextTurnPicture->Rect());
   }
}

void
CGuideContainer::SetDetourPicture(TInt on)
{
   SetImagePos(iDetourPicture,
               iGuidePicture->GetRect(),
               &LowerRight, 
               iImagePaddingX,
               -iImagePaddingY);

   iDetourPicture->SetShow(on == 1);
   Window().Invalidate(iDetourPicture->Rect());
}

void
CGuideContainer::SetSpeedCamPicture(TInt on)
{
   SetImagePos(iSpeedCamPicture,
               iGuidePicture->GetRect(),
               &LowerRight, 
               -iImagePaddingX,
               -iImagePaddingY);

   iSpeedCamPicture->SetShow(on == 1);
   Window().Invalidate(iSpeedCamPicture->Rect());
}

void CGuideContainer::SetExitL( TInt aMbmIndex, TInt aMbmMaskIndex )
{
   if( aMbmIndex >= 0 ){
      iExitPicture->SetShow( ETrue );
#ifdef NAV2_CLIENT_SERIES60_V3
      iExitPicture->CreateIconL(iView->GetMbmName(), 
                                aMbmIndex, 
                                aMbmMaskIndex, 
                                EAspectRatioPreservedAndUnusedSpaceRemoved,
                                ETrue);
      
      //iExitPicture->NightFilter(iAppUI->IsNightMode());
#else
      aMbmMaskIndex = aMbmMaskIndex;
      iExitPicture->OpenBitmapFromMbm( iView->GetMbmName(), aMbmIndex );
#endif
      iExitPicture->SetImageRect(Center(iGuidePicture->GetRect(), 
                                        TRect(TPoint(), iExitPicture->GetSize())));

      Window().Invalidate(iExitPicture->Rect());
   }
   else{
      iExitPicture->SetShow( EFalse );
      Window().Invalidate(iExitPicture->Rect());
   }
}

void CGuideContainer::RefreshPictures()
{
   // Refreshes all the positions of the images, it
   // first recalcs the size of the next turn picture since
   // when changed layout this image will have different
   // size as it had before. Since all the other images (except
   // the progress bar) is positioned within next turn pictures
   // rect, the will be positioned wrong if there positions wont
   // be based on a correct rect.
   SetGuidePictureL(iView->GetCurrentTurnPicture());
   if (iNextTurnPicture && iNextTurnPicture->GetShow()) {
      SetImagePos(iNextTurnPicture,
                  iGuidePicture->GetRect(),
                  &TopRight, 
                  -iImagePaddingX,
                  iImagePaddingY);
   }
   if (iSpeedCamPicture && iSpeedCamPicture->GetShow()) {
      SetImagePos(iSpeedCamPicture,
                  iGuidePicture->GetRect(),
                  &LowerRight, 
                  -iImagePaddingX,
                  -iImagePaddingY);     
   }
   if (iExitPicture && iExitPicture->GetShow()) {
      iExitPicture->SetImageRect(Center(iGuidePicture->GetRect(), 
                                        TRect(TPoint(), iExitPicture->GetSize())));
   }
}

void CGuideContainer::SetDistanceL( TUint aDistance )
{
   TBuf<KBuf32Length> distance;
   TBuf<KBuf32Length> temp;
   isab::DistancePrintingPolicy::DistanceMode mode =
      isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());

   char* tmp2 = isab::DistancePrintingPolicy::convertDistance(aDistance, mode, isab::DistancePrintingPolicy::Round);

   TInt foo = 0;
   distance.Copy(_L(""));
   if (tmp2) {
      WFTextUtil::char2TDes(temp, tmp2);
      foo = strlen(tmp2)/2;
      delete[] tmp2;
   } else {
      distance.Copy(_L("  "));
      foo = 4;
   }
   while (foo < 4) {
      distance.Append(_L(" "));
      foo++;
   }
   distance.Append( temp );
   distance.Append( _L("    "));

   if (*(iDistanceLabel->Text()) != distance) {
      iDistanceLabel->SetTextL( distance );
#ifdef NAV2_CLIENT_SERIES60_V3
      if (WFLayoutUtils::LandscapeMode()) {
         SetLabelExtent(iDistanceLabel, 
                        DIST_STREET_LABEL_X_LS, 
                        DIST_STREET_LABEL_Y_LS);
      } else {
         SetLabelExtent(iDistanceLabel, DISTANCE_LABEL_POS);
      }
#else
      iDistanceLabel->SetExtent( DISTANCE_LABEL_POS, iDistanceLabel->MinimumSize() );
#endif
      iDistanceLabel->DrawDeferred();
   }
   iProgressBar->SetDistance( aDistance );
}

void CGuideContainer::SetStreetsL( TDesC &aCurrName, TDesC &aNextName )
{
   if (iCurrStreetLabel) {
      iCurrStreetLabel->SetTextL( aCurrName );
      if (WFLayoutUtils::LandscapeMode()) {
         SetLabelExtent(iCurrStreetLabel, 
                        CURR_STREET_LABEL_X_LS, 
                        CURR_STREET_LABEL_Y_LS, 
                        ETrue);
      } else {
         SetLabelExtent(iCurrStreetLabel, CURRSTREET_LABEL_POS, ETrue);
      }
   }
   if (iNextStreetLabel) {
      iNextStreetLabel->SetTextL( aNextName );
      if (WFLayoutUtils::LandscapeMode()) {
         SetLabelExtent(iNextStreetLabel,  
                        NEXT_STREET_LABEL_X_LS, 
                        NEXT_STREET_LABEL_Y_LS, 
                        ETrue);
      } else {
         SetLabelExtent(iNextStreetLabel, NEXTSTREET_LABEL_POS, ETrue);
      }
   }
}

void CGuideContainer::GetRoundedDistance( TUint aDistance, TUint &aNewDistance )
{
   aNewDistance = aDistance;
   TInt distDif;
   if( aDistance > 0 && aDistance > 50 ){
      if( aDistance < 200 ){
         distDif = aDistance%5;
         if( distDif < 3 ){
            aNewDistance -= distDif;
         }
         else{
            distDif = 5-distDif;
            aNewDistance += distDif;
         }
      }
      else if( aDistance < 1000 ){
         distDif = aDistance%10;
         if( distDif < 5 ){
            aNewDistance -= distDif;
         }
         else{
            distDif = 10-distDif;
            aNewDistance += distDif;
         }
      }
      else{
         distDif = aDistance%100;
         if( distDif < 50 ){
            aNewDistance -= distDif;
         }
         else{
            distDif = 100-distDif;
            aNewDistance += distDif;
         }
      }
   }
}

void CGuideContainer::PictureError( TInt aError )
{
   iView->PictureError( aError );
}

void CGuideContainer::ScalingDone()
{
   //DrawNow();
   DrawDeferred();
}

// ----------------------------------------------------------------------------
// TKeyResponse CGuideContainer::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
TKeyResponse CGuideContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                               TEventCode aType )
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }

   switch ( aKeyEvent.iScanCode ) 
   {
   // Switches tab.
   case EStdKeyUpArrow:
      if( !(iAppUI->IsGpsAllowed()) &&
               iAppUI->HasRoute() ){
         iView->GoToTurn( ETrue );
         return EKeyWasConsumed;
      }
      break;
   case EStdKeyDownArrow:
      if( !(iAppUI->IsGpsAllowed()) &&
            iAppUI->HasRoute() ){
         iView->GoToTurn( EFalse );
         return EKeyWasConsumed;
      }
      break;
   case EStdKeyDevice3:
      iView->HandleCommandL( EWayFinderCmdGuideReroute );
      return EKeyWasConsumed;
      break;
      // No hash for you, for now anyway
//    case EStdKeyHash:
//       iView->HandleCommandL( EWayFinderCmdMap ) ;
//       return EKeyWasConsumed;
//       break;
   }
   return EKeyWasNotConsumed;
}

// ---------------------------------------------------------
// CGuideContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CGuideContainer::SizeChanged()
{
   if (WFLayoutUtils::LandscapeMode()) {
      if( iNextStreetLabel )
         SetLabelExtent(iNextStreetLabel,  
                        NEXT_STREET_LABEL_X_LS, 
                        NEXT_STREET_LABEL_Y_LS, 
                        ETrue);
      if( iCurrStreetLabel )
         SetLabelExtent(iCurrStreetLabel, 
                        CURR_STREET_LABEL_X_LS, 
                        CURR_STREET_LABEL_Y_LS, 
                        ETrue);
      if( iDistanceLabel )
         SetLabelExtent(iDistanceLabel, 
                        DIST_STREET_LABEL_X_LS, 
                        DIST_STREET_LABEL_Y_LS);
   } else {
      if( iNextStreetLabel )
         SetLabelExtent(iNextStreetLabel, NEXTSTREET_LABEL_POS, ETrue);
      if( iCurrStreetLabel )
         SetLabelExtent(iCurrStreetLabel, CURRSTREET_LABEL_POS, ETrue);
      if( iDistanceLabel )
         SetLabelExtent(iDistanceLabel, DISTANCE_LABEL_POS);
   }
}

// ---------------------------------------------------------
// CGuideContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CGuideContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CGuideContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CGuideContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case ENextStreetLabel:
      return iNextStreetLabel;
   case EGuidePicture:
      return iGuidePicture;
   case EExitPicture:
      return iExitPicture;
   case ECurrStreetLabel:
      return iCurrStreetLabel;
   case EDistanceLabel:
      return iDistanceLabel;
   case EProgressBar:
      return iProgressBar;
   case ENextTurnPicture:
      return iNextTurnPicture;
   case EDetourPicture:
      return iDetourPicture;
   case ESpeedCamPicture:
      return iSpeedCamPicture;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CGuideContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CGuideContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   // TODO: Add your drawing code here
   // example code...
   gc.SetPenStyle(CGraphicsContext::ENullPen);
//    gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
   gc.SetBrushColor(TRgb(iR, iG, iB));
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);

#undef DRAW_IMAGE_RECTS_DBG
   //#define DRAW_IMAGE_RECTS_DBG
#ifdef DRAW_IMAGE_RECTS_DBG
   if (iGuidePicture) {
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor(TRgb(0, 0, 0));
      gc.DrawRect(iGuidePicture->Rect());
   }
   if (iNextTurnPicture) {
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor(TRgb(0, 255, 0));
      gc.DrawRect(iNextTurnPicture->Rect());
   }
   if (iDetourPicture) {
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor(TRgb(0, 255, 0));
      gc.DrawRect(iDetourPicture->Rect());
   }
   if (iSpeedCamPicture) {
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor(TRgb(255, 0, 0));
      gc.DrawRect(iSpeedCamPicture->Rect());      
   }
#endif
}

// ---------------------------------------------------------
// CGuideContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CGuideContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

void CGuideContainer::SetLabelExtent(class CEikLabel* aLabel, 
                                     const TPoint &aPosition, 
                                     TBool aExtendToSceenWidth)
{
   // All the sizen for V3 are relative points
   // based on the positions for V2
#ifdef NAV2_CLIENT_SERIES60_V3
   TSize size = aLabel->MinimumSize();
   // We need to increase the height otherwise we wont see
   // a complete letter of type g/p etc.
   size.iHeight = aLabel->Font()->HeightInPixels() + 
      aLabel->Font()->DescentInPixels();
   if (aExtendToSceenWidth) {
      size.iWidth = WFLayoutUtils::GetMainPaneRect().Width();
   }
   aLabel->SetExtent(WFLayoutUtils::
                     CalculatePosUsingMainPane(aPosition, 
                                               WFLayoutUtils::WideScreen()), size);
   aLabel->DrawDeferred();
#else
   // v2 does not use the size that is sent in, it has
   // allways used MinimumSize and I dont want to change 
   // this now.
   aLabel->SetExtent(aPosition, aLabel->MinimumSize());
#endif
}

void CGuideContainer::SetLabelExtent(class CEikLabel* aLabel, 
                                     float aWidth,
                                     float aHeight, 
                                     TBool aExtendToSceenWidth)
{
   // All the sizen for V3 are relative points
   // based on the positions for V2
   TSize size = aLabel->MinimumSize();
#if defined NAV2_CLIENT_SERIES60_V3
   // We need to increase the height otherwise we wont see
   // a complete letter of type g/p etc.
   size.iHeight = aLabel->Font()->HeightInPixels() + 
      aLabel->Font()->DescentInPixels();
#endif
   if (aExtendToSceenWidth) {
      size.iWidth = WFLayoutUtils::GetMainPaneRect().Width();
   }
   aLabel->SetExtent(WFLayoutUtils::
                     CalculatePosUsingMainPaneFloat(aWidth, aHeight, 
                                                    WFLayoutUtils::WideScreen()), size);
   aLabel->DrawDeferred();
}


void CGuideContainer::CreatePictureL(class CImageHandler*& aImage, const TRect& /*aRect*/, 
                                     const TPoint aPoint, TInt aWidth, TInt aHeight)
{
   TSize size;
   if (aWidth == aHeight) {
      // The width and height are equal, we are calculating the size based
      // on the width of the image and the width of the screen so that the
      // image size allways becomes a square.
      size = TSize(WFLayoutUtils::CalculateSizeUsingWidth(aWidth, 
                                                          WFLayoutUtils::WideScreen()));
   } else {
      // The widht are not equal to the height, the image rect should 
      // have the shape of a rectangle.
      size = TSize(WFLayoutUtils::CalculateSizeUsingMainPane(aWidth, aHeight, 
                                                             WFLayoutUtils::WideScreen()));
   }
   if (aImage) {
      aImage->SetImageRect(TRect(WFLayoutUtils::
                                 CalculatePosUsingMainPane(aPoint, 
                                                           WFLayoutUtils::WideScreen()), size));
   } else {
      aImage = CImageHandler::NewL(TRect(WFLayoutUtils::
                                         CalculatePosUsingMainPane(aPoint, 
                                                                   WFLayoutUtils::WideScreen()),
                                         size));
   }
}

// void CGuideContainer::SetPicture(class CImageHandler*& aImage, )
                                     
void CGuideContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);

   if (aType == KEikDynamicLayoutVariantSwitch) {
      iLayoutChanged = ETrue;
      if (WFLayoutUtils::LandscapeMode()) {
         // We are in landscape mode!
         CalculateLandscapeLayout();
      } else {
         // We are in portratit mode!
         CalculatePortraitLayout();
      }
      RefreshPictures();
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
}

void CGuideContainer::CalculateLandscapeLayout()
{
#ifdef NAV2_CLIENT_SERIES60_V3
   // Landscape mode!

   // Clear all labeltexts before setting the new extent,
   // by doing this we will get rid of the text in the
   // old positions.
   ActivateGc();
   CWindowGc& gc = SystemGc();
   gc.Clear(iCurrStreetLabel->Rect());
   SetLabelExtent(iCurrStreetLabel, 
                  CURR_STREET_LABEL_X_LS, 
                  CURR_STREET_LABEL_Y_LS, 
                  ETrue);
   gc.Clear(iNextStreetLabel->Rect());
   SetLabelExtent(iNextStreetLabel, 
                  NEXT_STREET_LABEL_X_LS, 
                  NEXT_STREET_LABEL_Y_LS, 
                  ETrue);
   gc.Clear(iDistanceLabel->Rect());
   SetLabelExtent(iDistanceLabel, 
                  DIST_STREET_LABEL_X_LS, 
                  DIST_STREET_LABEL_Y_LS);
   DeactivateGc();
   TBool wideScreen = WFLayoutUtils::WideScreen();
   if (iGuidePicture) {
      // Recalculate
      iGuidePicture->SetImageRect(TRect(WFLayoutUtils::
                                        CalculatePosUsingMainPaneFloat(0.1, 0.15,
                                                                       wideScreen),
                                        WFLayoutUtils::
                                        CalculateSizeUsingMainPaneFloat(0.45, 0.70,
                                                                        wideScreen)));
   } else {
      iGuidePicture = 
         CImageHandler::NewL(TRect(WFLayoutUtils::
                                   CalculatePosUsingMainPaneFloat(0.1, 0.15,
                                                                  wideScreen),
                                   WFLayoutUtils::
                                   CalculateSizeUsingMainPaneFloat(0.45, 0.70,
                                                                   wideScreen)));
   }

   if (iProgressBar) {
      iProgressBar->SetSizeAndPosition(TRect(WFLayoutUtils::CalculatePosUsingMainPaneFloat(0.7, 0.15, wideScreen),
                                             WFLayoutUtils::CalculateSizeUsingMainPaneFloat(0.20, 0.6, wideScreen)));
   } else {

      TInt progressTriangleIndex;
      TInt progressRectIndex;

      if (iAppUI->IsNightMode()) {
         progressTriangleIndex = EMbmWficonsProgress_triangle_night;
         progressRectIndex = EMbmWficonsProgress_bar_night;
      } else {
         progressTriangleIndex = EMbmWficonsProgress_triangle_blue;
         progressRectIndex = EMbmWficonsProgress_bar_blue;         
      }

      iProgressBar = 
         CGuideProgressBar::NewL(*this, 
                                 TRect( WFLayoutUtils::
                                        CalculatePosUsingMainPaneFloat(0.7, 0.15,
                                                                       wideScreen), 
                                        WFLayoutUtils::
                                        CalculateSizeUsingMainPaneFloat(0.20, 0.6,
                                                                        wideScreen)), 
                                 iView->GetMbmName(),  
                                 EMbmWficonsProgress_triangle_grey,
                                 progressTriangleIndex,
                                 EMbmWficonsProgress_triangle_grey_mask,
                                 EMbmWficonsProgress_bar_grey,
                                 progressRectIndex,
                                 EMbmWficonsProgress_bar_grey_mask);
   }

   // Calculate the padding
   iImagePaddingX = WFLayoutUtils::CalculateXValue(IMAGE_PADDING, wideScreen);
   iImagePaddingY = WFLayoutUtils::CalculateYValueUsingMainPane(IMAGE_PADDING);

   SetImage(iNextTurnPicture,
            iGuidePicture->GetRect(),
            &TopRight, 0.24, 0.24, 
            -iImagePaddingX,
            iImagePaddingY);
   
   SetImage(iDetourPicture,
            iGuidePicture->GetRect(),
            &LowerLeft, 0.24, 0.24, 
            iImagePaddingX,
            -iImagePaddingY);
   
   SetImage(iSpeedCamPicture,
            iGuidePicture->GetRect(),
            &LowerRight, 0.24, 0.24, 
            -iImagePaddingX,
            -iImagePaddingY);

   iDetourPicture->CreateIconL(iView->GetMbmName(), 
                               EMbmWficonsDetour_square, 
                               EMbmWficonsDetour_square_mask,
                               EAspectRatioPreservedAndUnusedSpaceRemoved);
   iSpeedCamPicture->CreateIconL(iView->GetMbmName(), 
                                 EMbmWficonsSpeedcamera_square, 
                                 EMbmWficonsSpeedcamera_square,
                                 EAspectRatioPreservedAndUnusedSpaceRemoved);
   
   TRect exitRect = Center(iGuidePicture->GetRect(), 
                           TRect(TPoint(), 
                                 WFLayoutUtils::
                                 CalculateSizeUsingSize(iGuidePicture->GetSize(), 
                                                        0.1, 0.16, wideScreen)));
   if (iExitPicture) {
      iExitPicture->SetImageRect(exitRect, 
                                 EAspectRatioPreservedAndUnusedSpaceRemoved, 
                                 ETrue);
   } else {
      iExitPicture = CImageHandler::NewL(exitRect);
   }

#endif
}

void CGuideContainer::CalculatePortraitLayout() 
{
#ifdef NAV2_CLIENT_SERIES60_V3
   ActivateGc();
   CWindowGc& gc = SystemGc();
   gc.Clear(iCurrStreetLabel->Rect());
   SetLabelExtent(iCurrStreetLabel, CURRSTREET_LABEL_POS, ETrue);
   gc.Clear(iNextStreetLabel->Rect());
   SetLabelExtent(iNextStreetLabel, NEXTSTREET_LABEL_POS, ETrue);
   gc.Clear(iDistanceLabel->Rect());
   SetLabelExtent(iDistanceLabel, DISTANCE_LABEL_POS);
   DeactivateGc();
   TBool wideScreen = WFLayoutUtils::WideScreen();
   if (iProgressBar) {
      iProgressBar->SetSizeAndPosition(TRect( WFLayoutUtils::
                                              CalculatePosUsingMainPane( PROGRESS_BAR_POS,
                                                                         wideScreen), 
                                              WFLayoutUtils::
                                              CalculateSizeUsingMainPane( 40, 90,
                                                                          wideScreen) ) );
   } else {

      TInt progressTriangleIndex;
      TInt progressRectIndex;

      if (iAppUI->IsNightMode()) {
         progressTriangleIndex = EMbmWficonsProgress_triangle_night;
         progressRectIndex = EMbmWficonsProgress_bar_night;
      } else {
         progressTriangleIndex = EMbmWficonsProgress_triangle_blue;
         progressRectIndex = EMbmWficonsProgress_bar_blue;         
      }

      iProgressBar = 
         CGuideProgressBar::NewL(*this, 
                                 TRect( WFLayoutUtils::
                                        CalculatePosUsingMainPane( PROGRESS_BAR_POS, 
                                                                   wideScreen ), 
                                        WFLayoutUtils::
                                        CalculateSizeUsingMainPane( 40, 90, wideScreen ) ), 
                                 iView->GetMbmName(),  
                                 EMbmWficonsProgress_triangle_grey,
                                 progressTriangleIndex,
                                 EMbmWficonsProgress_triangle_grey_mask,
                                 EMbmWficonsProgress_bar_grey,
                                 progressRectIndex,
                                 EMbmWficonsProgress_bar_grey_mask);
   }
   
   TRect rect = WFLayoutUtils::GetMainPaneRect();
   CreatePictureL(iGuidePicture, rect, GUIDE_PICTURE_POS, 100, 100);

   iImagePaddingX = WFLayoutUtils::CalculateXValue(IMAGE_PADDING);
   iImagePaddingY = WFLayoutUtils::CalculateYValueUsingMainPane(IMAGE_PADDING);

   SetImage(iNextTurnPicture,
            iGuidePicture->GetRect(),
            &TopRight, 0.24, 0.24, 
            -iImagePaddingX,
            iImagePaddingY);
   
   SetImage(iDetourPicture,
            iGuidePicture->GetRect(),
            &LowerLeft, 0.24, 0.24, 
            iImagePaddingX,
            -iImagePaddingY);

   SetImage(iSpeedCamPicture, 
            iGuidePicture->GetRect(),
            &LowerRight, 0.24, 0.24, 
            -iImagePaddingX,
            -iImagePaddingY);
   
   iDetourPicture->CreateIconL(iView->GetMbmName(), 
                               EMbmWficonsDetour_square, 
                               EMbmWficonsDetour_square_mask,
                               EAspectRatioPreservedAndUnusedSpaceRemoved);
   iSpeedCamPicture->CreateIconL(iView->GetMbmName(), 
                                 EMbmWficonsSpeedcamera_square, 
                                 EMbmWficonsSpeedcamera_square,
                                 EAspectRatioPreservedAndUnusedSpaceRemoved);

   TRect exitRect = Center(iGuidePicture->GetRect(), 
                           TRect(TPoint(), 
                                 WFLayoutUtils::
                                 CalculateSizeUsingSize(iGuidePicture->GetSize(), 
                                                        0.26, 0.16, wideScreen)));

   if (iExitPicture) {
      iExitPicture->SetImageRect(exitRect);
   } else {
      iExitPicture = CImageHandler::NewL(exitRect);
   }

#endif
}

void CGuideContainer::SetImage(class CImageHandler*& aImage, 
                               const TRect& aBaseRect,
                               TRect (*ptrFunc) (const class TRect& aRect, const class TSize& aSize),
                               float aXPos, float aYPos,
                               TInt aDx, TInt aDy)
{
   TBool wideScreen = WFLayoutUtils::WideScreen();
   TRect rect = (*ptrFunc)(aBaseRect,
         TSize(WFLayoutUtils::CalculateXValue(aBaseRect.Width(), aXPos, wideScreen),
               WFLayoutUtils::CalculateYValue(aBaseRect.Height(), aYPos)));
   rect.Move(aDx, aDy);
   if (aImage) {
      aImage->SetImageRect(rect);
   } else {
      aImage = CImageHandler::NewL(rect);
   }
}

void CGuideContainer::SetImagePos(class CImageHandler*& aImage, 
                                  const TRect& aBaseRect,
                                  TRect (*ptrFunc) (const class TRect& aRect, const class TSize& aSize),
                                  TInt aDx, TInt aDy)
{
   if(!aImage) {
      return;
   }
   TRect rect = (*ptrFunc)(aBaseRect, aImage->GetSize());
   rect.Move(aDx, aDy);
   aImage->SetImageRect(rect);
}

void CGuideContainer::SetNightModeL(TBool aNightMode)
{
#ifdef NAV2_CLIENT_SERIES60_V3
   iAppUI->GetBackgroundColor(iR, iG, iB);

   CGuideProgressBar* tmpBar;
   CGuideProgressBar* tmpBar2 = iProgressBar;

   TRect rect;
   // Calculate the size of the progressbar
   if (WFLayoutUtils::LandscapeMode(*iAppUI)) {
      rect.SetRect(WFLayoutUtils::
                   CalculatePosUsingMainPaneFloat(0.7, 0.15, 
                                                  WFLayoutUtils::WideScreen()), 
                   WFLayoutUtils::
                   CalculateSizeUsingMainPaneFloat(0.20, 0.6, 
                                                   WFLayoutUtils::WideScreen())); 
   } else {
      rect.SetRect(WFLayoutUtils::CalculatePosUsingMainPane(PROGRESS_BAR_POS, 
                                                            WFLayoutUtils::WideScreen()), 
                   WFLayoutUtils::CalculateSizeUsingMainPane(40, 90, 
                                                             WFLayoutUtils::WideScreen()));
   }

   if (aNightMode) {
      tmpBar = 
         CGuideProgressBar::NewL(*this, 
                                 rect, 
                                 iView->GetMbmName(),  
                                 EMbmWficonsProgress_triangle_grey,
                                 EMbmWficonsProgress_triangle_night,
                                 EMbmWficonsProgress_triangle_grey_mask,
                                 EMbmWficonsProgress_bar_grey,
                                 EMbmWficonsProgress_bar_night,
                                 EMbmWficonsProgress_bar_grey_mask);

      TRgb nightColor(KForegroundTextColorNightRed,
                      KForegroundTextColorNightGreen,
                      KForegroundTextColorNightBlue);
      iCurrStreetLabel->OverrideColorL(EColorLabelText, nightColor);
      iNextStreetLabel->OverrideColorL(EColorLabelText, nightColor);
      iDistanceLabel->OverrideColorL(EColorLabelText, nightColor);
      /*
      //NightFilter disabled for the pictures.
      //Enable the NightFilter calls for the images in the following functions
      // if you enable it here: SetGuidePictureL, SetNextTurnPictureL and SetExitL.
      if (iGuidePicture) {
         iGuidePicture->NightFilter(iAppUI->IsNightMode());
      }
      if (iNextTurnPicture){
         iNextTurnPicture->NightFilter(iAppUI->IsNightMode());
      }
      if (iDetourPicture){
         iDetourPicture->NightFilter(iAppUI->IsNightMode());
      }
      if (iSpeedCamPicture){
         iSpeedCamPicture->NightFilter(iAppUI->IsNightMode());
      }
      if (iExitPicture){
         iExitPicture->NightFilter(iAppUI->IsNightMode());
      }
      */
   } else {
      tmpBar = 
         CGuideProgressBar::NewL(*this, 
                                 rect, 
                                 iView->GetMbmName(),  
                                 EMbmWficonsProgress_triangle_grey,
                                 EMbmWficonsProgress_triangle_blue,
                                 EMbmWficonsProgress_triangle_grey_mask,
                                 EMbmWficonsProgress_bar_grey,
                                 EMbmWficonsProgress_bar_blue,
                                 EMbmWficonsProgress_bar_grey_mask);

      iCurrStreetLabel->OverrideColorL(EColorLabelText, TRgb(0, 0, 0));
      iNextStreetLabel->OverrideColorL(EColorLabelText, TRgb(0, 0, 0));
      iDistanceLabel->OverrideColorL(EColorLabelText, TRgb(0, 0, 0));  
      /*
      iGuidePicture->TurnNightModeOff(iView->GetMbmName(), 
                                      iView->GetCurrentTurnPicture(),
                                      EMbmWficonsKeep_left_mask);
      TInt imageIndex, maskIndex;
      iView->GetNextTurnPicture(imageIndex, maskIndex);
      iNextTurnPicture->TurnNightModeOff(iView->GetMbmName(),
                                         imageIndex,
                                         maskIndex);
      
      iDetourPicture->TurnNightModeOff(iView->GetMbmName(), 
                                       EMbmWficonsDetour_square, 
                                       EMbmWficonsDetour_square_mask);

      iSpeedCamPicture->TurnNightModeOff(iView->GetMbmName(), 
                                         EMbmWficonsSpeedcamera_square, 
                                         EMbmWficonsSpeedcamera_square);
      
      if (iNextTurnPicture->GetShow()) {
         Window().Invalidate(iNextTurnPicture->Rect());         
      }
      if (iGuidePicture->GetShow()) {
         Window().Invalidate(iGuidePicture->Rect());         
      }
      if (iDetourPicture->GetShow()) {
         Window().Invalidate(iDetourPicture->Rect());         
      } 
      if (iSpeedCamPicture->GetShow()) {
         Window().Invalidate(iSpeedCamPicture->Rect());         
      }
      */
   }

   tmpBar->SetDistance(iProgressBar->GetDistance());
   iProgressBar = tmpBar;
   delete tmpBar2;
#endif
}

// End of File  
