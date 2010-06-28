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
#include <eikedwin.h>  // for the editor
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
#include "SREdwin.h"

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
// position of the editor for the screen reader
#define TEXT_INSTRUCTION_POS	TPoint(  0,  0)

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

// position of the editor for the screen reader
#define TEXT_INSTRUCTION_X_LS 0.0
#define TEXT_INSTRUCTION_Y_LS 0.0
#define TEXT_INSTRUCTION_X_PR 0.0
#define TEXT_INSTRUCTION_Y_PR 0.0

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
   ETextInstructionLabel,
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

   InitTTSContainer();    
   GetTalkingLabel()->SetContainerWindowL(*this);

    
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
   iNextTurnPicture->ConstructL(TRect(NEXT_TURN_PICTURE_POS, TSize(24, 24)), 
                                this);

   iDetourPicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iDetourPicture, CGuidePicture);
   iDetourPicture->ConstructL(TRect(DETOUR_PICTURE_POS, TSize(24, 24)), 
                              this);

   iDetourPicture->OpenBitmapFromMbm(iView->GetMbmName(), 
                                     EMbmWficonsSmall_detour);
   iDetourPicture->SetShow(EFalse);

   iSpeedCamPicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iSpeedCamPicture, CGuidePicture);
   iSpeedCamPicture->ConstructL(TRect(SPEEDCAM_PICTURE_POS, TSize(24, 24)), 
                                this);

   iSpeedCamPicture->OpenBitmapFromMbm(iView->GetMbmName(), 
                                       EMbmWficonsSmall_speedcam);
   iSpeedCamPicture->SetShow(EFalse);


   iExitPicture = new (ELeave) CGuidePicture( iLog );
   LOGNEW(iExitPicture, CGuidePicture);
   iExitPicture->ConstructL( TRect( EXIT_PICTURE_POS, TSize( 26, 16 ) ), this );
   iExitPicture->SetShow(EFalse);
#endif

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
      iGuidePicture->CreateIconL(iView->GetMbmName(), aMbmIndex, EMbmWficonsKeep_left_mask);
#else
      iGuidePicture->OpenBitmapFromMbm( iView->GetMbmName(), aMbmIndex );
      iGuidePicture->OpenBitmapMaskFromMbm( iView->GetMbmName(), EMbmWficonsTurn_mask );
#endif
      Window().Invalidate(iGuidePicture->Rect());
   }
   else{
      iGuidePicture->SetShow( EFalse );
      Window().Invalidate(iGuidePicture->Rect());
   }
}

void CGuideContainer::SetNextTurnPictureL( TInt aMbmIndex , TInt aMbmMaskIndex )
{
   if( aMbmIndex >= 0 ){
      iNextTurnPicture->SetShow( ETrue );
      iNextTurnPicture->SetClear( EFalse );
#ifdef NAV2_CLIENT_SERIES60_V3
      iNextTurnPicture->CreateIconL(iView->GetMbmName(), 
                                    aMbmIndex, 
                                    aMbmMaskIndex);
#else
      iNextTurnPicture->OpenBitmapFromMbm(iView->GetMbmName(), aMbmIndex);
      iNextTurnPicture->OpenBitmapMaskFromMbm(iView->GetMbmName(), 
                                              aMbmMaskIndex);
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
   iDetourPicture->SetShow(on == 1);
   Window().Invalidate(iDetourPicture->Rect());
}

void
CGuideContainer::SetSpeedCamPicture(TInt on) 
{
   iSpeedCamPicture->SetShow(on == 1);
   Window().Invalidate(iSpeedCamPicture->Rect());
}

void CGuideContainer::SetExitL(TInt aMbmIndex) 
{
   if( aMbmIndex >= 0 ){
      iExitPicture->SetShow( ETrue );
#ifdef NAV2_CLIENT_SERIES60_V3
      iExitPicture->CreateIconL(iView->GetMbmName(), aMbmIndex);
#else
      iExitPicture->OpenBitmapFromMbm( iView->GetMbmName(), aMbmIndex );
#endif
      Window().Invalidate(iExitPicture->Rect());
   }else{
      iExitPicture->SetShow( EFalse );
      Window().Invalidate(iExitPicture->Rect());
   }
}

void CGuideContainer::SetDistanceL(TUint aDistance) 
{
   if (iAppUI->GetTransportationMode() != isab::NavServerComEnums::pedestrian) {
      TBuf<KBuf32Length> distance;
      TBuf<KBuf32Length> temp;
      isab::DistancePrintingPolicy::DistanceMode mode =
         isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());

      char* tmp2 = isab::DistancePrintingPolicy::
         convertDistance(aDistance, 
                         mode, 
                         isab::DistancePrintingPolicy::Round);

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

void CGuideContainer::SetTextInstructionL(const TDesC &aInstruction) {
   ReadTextInstructionL(aInstruction);
}

void CGuideContainer::GetRoundedDistance(TUint aDistance, 
                                         TUint &aNewDistance) 
{
   aNewDistance = aDistance;
   TInt distDif;
   if( aDistance > 0 && aDistance > 50 ){
      if( aDistance < 200 ){
         distDif = aDistance%5;
         if( distDif < 3 ){
            aNewDistance -= distDif;
         } else{
            distDif = 5-distDif;
            aNewDistance += distDif;
         }
      }
      else if( aDistance < 1000 ){
         distDif = aDistance%10;
         if( distDif < 5 ){
            aNewDistance -= distDif;
         } else{
            distDif = 10-distDif;
            aNewDistance += distDif;
         }
      } else{
         distDif = aDistance%100;
         if( distDif < 50 ){
            aNewDistance -= distDif;
         } else{
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
   CEikLabel* instructionLabel = GetTalkingLabel();
   
   if (aType == EEventKeyDown) {
      this->SetFocus(ETrue);
   } 
//    } else if (aType == EEventKeyUp) {
//       iInstructionLabel->SetFocus(ETrue);
//    }
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }

   switch ( aKeyEvent.iScanCode ) 
   {
   case EStdKeyLeftArrow:
   case EStdKeyRightArrow:
      instructionLabel->SetFocus(ETrue);
      return EKeyWasNotConsumed;
      break;
   // Switches tab.
   case EStdKeyUpArrow:
      if( !(iAppUI->IsGpsAllowed()) &&
          iAppUI->HasRoute() ){
         // update here
         iView->GoToTurn( ETrue );
         instructionLabel->SetFocus(ETrue);
         return EKeyWasConsumed;
      }
      break;
   case EStdKeyDownArrow:
      if( !(iAppUI->IsGpsAllowed()) &&
          iAppUI->HasRoute() ){
         // update here
         iView->GoToTurn( EFalse );
         instructionLabel->SetFocus(ETrue);
         return EKeyWasConsumed;
      }
      break;
   case EStdKeyDevice3:
      iView->HandleCommandL( EWayFinderCmdGuideReroute );
      instructionLabel->SetFocus(ETrue);
      return EKeyWasConsumed;
      break;
   case EStdKeyHash:
      iView->HandleCommandL( EWayFinderCmdMap ) ;
      instructionLabel->SetFocus(ETrue);
      return EKeyWasConsumed;
      break;
   default:
      instructionLabel->SetFocus(ETrue);
      return EKeyWasNotConsumed;
      break;
   }

//    //TODO: Test "return EKeyWasConsumed;"
   return EKeyWasNotConsumed;
}

// ---------------------------------------------------------
// CGuideContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CGuideContainer::SizeChanged()
{
   TTSContainer::SizeChanged();
   
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
   case ETextInstructionLabel:
      return GetTalkingLabel();
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
   gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);
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
   size.iHeight += 3;
   if (aExtendToSceenWidth) {
      size.iWidth = WFLayoutUtils::GetMainPaneRect().Width();
   }
//    aLabel->SetExtent(pos, size);
   aLabel->SetExtent(WFLayoutUtils::CalculatePosUsingMainPane(aPosition), size);
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
   size.iHeight += 3;
#endif
   if (aExtendToSceenWidth) {
      size.iWidth = WFLayoutUtils::GetMainPaneRect().Width();
   }
//    aLabel->SetExtent(pos, size);
   aLabel->SetExtent(WFLayoutUtils::CalculatePosUsingMainPaneFloat(aWidth, aHeight), size);
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
      size = TSize(WFLayoutUtils::CalculateSizeUsingWidth(aWidth));
   } else {
      // The widht are not equal to the height, the image rect should 
      // have the shape of a rectangle.
      size = TSize(WFLayoutUtils::CalculateSizeUsingMainPane(aWidth, aHeight));
   }
   if (aImage) {
      aImage->SetImageRect(TRect(WFLayoutUtils::CalculatePosUsingMainPane(aPoint), size));
   } else {
      aImage = CImageHandler::NewL(TRect(WFLayoutUtils::CalculatePosUsingMainPane(aPoint), size));
   }
}

// void CGuideContainer::SetPicture(class CImageHandler*& aImage, )
                                     
void CGuideContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);

   if (aType == KEikDynamicLayoutVariantSwitch) {
      if (WFLayoutUtils::LandscapeMode()) {
         // We are in landscape mode!
         CalculateLandscapeLayout();
      } else {
         // We are in portratit mode!
         CalculatePortraitLayout();
      }
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
   // Resets the Edwin control for the screen reader
   CEikLabel* instructionLabel = GetTalkingLabel();
   gc.Clear(instructionLabel->Rect());
   SetLabelExtent(instructionLabel, 
                  TEXT_INSTRUCTION_X_LS, 
                  TEXT_INSTRUCTION_Y_LS);
	
   DeactivateGc();
   if (iGuidePicture) {
      // Recalculate
      iGuidePicture->SetImageRect(TRect(WFLayoutUtils::CalculatePosUsingMainPaneFloat(0.1, 0.15),
                                        WFLayoutUtils::CalculateSizeUsingMainPaneFloat(0.45, 0.70)));
   } else {
      iGuidePicture = 
         CImageHandler::NewL(TRect(WFLayoutUtils::CalculatePosUsingMainPaneFloat(0.1, 0.15),
                                   WFLayoutUtils::CalculateSizeUsingMainPaneFloat(0.45, 0.70)));
   }
   if (iProgressBar) {
      iProgressBar->SetSizeAndPosition(TRect(WFLayoutUtils::CalculatePosUsingMainPaneFloat(0.7, 0.15),
                                             WFLayoutUtils::CalculateSizeUsingMainPaneFloat(0.20, 0.6)));
   } else {
      iProgressBar = 
         CGuideProgressBar::NewL(*this, 
                                 TRect( WFLayoutUtils::CalculatePosUsingMainPaneFloat(0.7, 0.15), 
                                        WFLayoutUtils::CalculateSizeUsingMainPaneFloat(0.20, 0.6)), 
                                 iView->GetMbmName(),  
                                 EMbmWficonsProgress_triangle_grey,
                                 EMbmWficonsProgress_triangle_blue,
                                 EMbmWficonsProgress_triangle_grey_mask,
                                 EMbmWficonsProgress_bar_grey,
                                 EMbmWficonsProgress_bar_blue,
                                 EMbmWficonsProgress_bar_grey_mask);
   }

   SetImage(iNextTurnPicture,
            iGuidePicture->GetRect(),
            &TopRight, 0.24, 0.24, 
            -WFLayoutUtils::CalculateXValue(IMAGE_PADDING),
            WFLayoutUtils::CalculateYValueUsingMainPane(IMAGE_PADDING));
   
   SetImage(iDetourPicture,
            iGuidePicture->GetRect(),
            &LowerLeft, 0.24, 0.24, 
            WFLayoutUtils::CalculateXValue(IMAGE_PADDING),
            -WFLayoutUtils::CalculateYValueUsingMainPane(IMAGE_PADDING));
   
   SetImage(iSpeedCamPicture,
            iGuidePicture->GetRect(),
            &LowerRight, 0.24, 0.24, 
            -WFLayoutUtils::CalculateXValue(IMAGE_PADDING),
            -WFLayoutUtils::CalculateYValueUsingMainPane(IMAGE_PADDING));

   iDetourPicture->SetShow(EFalse);
   iSpeedCamPicture->SetShow(EFalse);

   iDetourPicture->CreateIconL(iView->GetMbmName(), 
                               EMbmWficonsDetour_square, 
                               EMbmWficonsDetour_square_mask);
   iSpeedCamPicture->CreateIconL(iView->GetMbmName(), 
                                 EMbmWficonsSpeedcamera_square, 
                                 EMbmWficonsSpeedcamera_square);
   
   TRect exitRect = Center(iGuidePicture->GetRect(), 
                           TRect(TPoint(), 
                                 WFLayoutUtils::
                                 CalculateSizeUsingSize(iGuidePicture->GetSize(), 
                                                        0.26, 0.16)));
   if (iExitPicture) {
      iExitPicture->SetImageRect(exitRect);
   } else {
      iExitPicture = CImageHandler::NewL(exitRect);
   }
   iExitPicture->SetShow(EFalse);
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

   CEikLabel* instructionLabel = GetTalkingLabel();
   
   // Resets the Edwin control for the screen reader
   gc.Clear(instructionLabel->Rect());
   SetLabelExtent(instructionLabel, TEXT_INSTRUCTION_POS);
	
   DeactivateGc();
   if (iProgressBar) {
      iProgressBar->SetSizeAndPosition(TRect( WFLayoutUtils::
                                              CalculatePosUsingMainPane( PROGRESS_BAR_POS ), 
                                              WFLayoutUtils::
                                              CalculateSizeUsingMainPane( 40, 90 ) ) );
   } else {
      iProgressBar = 
         CGuideProgressBar::NewL(*this, 
                                 TRect( WFLayoutUtils::CalculatePosUsingMainPane( PROGRESS_BAR_POS ), 
                                        WFLayoutUtils::CalculateSizeUsingMainPane( 40, 90 ) ), 
                                 iView->GetMbmName(),  
                                 EMbmWficonsProgress_triangle_grey,
                                 EMbmWficonsProgress_triangle_blue,
                                 EMbmWficonsProgress_triangle_grey_mask,
                                 EMbmWficonsProgress_bar_grey,
                                 EMbmWficonsProgress_bar_blue,
                                 EMbmWficonsProgress_bar_grey_mask);
   }
   
   TRect rect = WFLayoutUtils::GetMainPaneRect();
   CreatePictureL(iGuidePicture, rect, GUIDE_PICTURE_POS, 100, 100);
   //    CreatePictureL(iNextTurnPicture, rect, NEXT_TURN_PICTURE_POS, 24, 24);
   //    CreatePictureL(iDetourPicture, rect, DETOUR_PICTURE_POS, 24, 24);
   SetImage(iNextTurnPicture,
            iGuidePicture->GetRect(),
            &TopRight, 0.24, 0.24, 
            -WFLayoutUtils::CalculateXValue(IMAGE_PADDING),
            WFLayoutUtils::CalculateYValueUsingMainPane(IMAGE_PADDING));
   
   SetImage(iDetourPicture,
            iGuidePicture->GetRect(),
            &LowerLeft, 0.24, 0.24, 
            WFLayoutUtils::CalculateXValue(IMAGE_PADDING),
            -WFLayoutUtils::CalculateYValueUsingMainPane(IMAGE_PADDING));

   SetImage(iSpeedCamPicture, 
            iGuidePicture->GetRect(),
            &LowerRight, 0.24, 0.24, 
            -WFLayoutUtils::CalculateXValue(IMAGE_PADDING),
            -WFLayoutUtils::CalculateYValueUsingMainPane(IMAGE_PADDING));

   iDetourPicture->SetShow(EFalse);
   iSpeedCamPicture->SetShow(EFalse);
   
   iDetourPicture->CreateIconL(iView->GetMbmName(), 
                               EMbmWficonsDetour_square, 
                               EMbmWficonsDetour_square_mask);
   iSpeedCamPicture->CreateIconL(iView->GetMbmName(), 
                                 EMbmWficonsSpeedcamera_square, 
                                 EMbmWficonsSpeedcamera_square);

   TRect exitRect = Center(iGuidePicture->GetRect(), 
                           TRect(TPoint(), 
                                 WFLayoutUtils::
                                 CalculateSizeUsingSize(iGuidePicture->GetSize(), 
                                                        0.26, 0.16)));

   if (iExitPicture) {
      iExitPicture->SetImageRect(exitRect);
   } else {
      iExitPicture = CImageHandler::NewL(exitRect);
   }
   iExitPicture->SetShow(EFalse);

   // XXX Temporary removed until the above lines are tested.
   // CreatePictureL(iExitPicture, rect, EXIT_PICTURE_POS, 26, 16);

#endif
}

void CGuideContainer::SetImage(class CImageHandler*& aImage, 
                               const TRect& aBaseRect,
                               TRect (*ptrFunc) (const class TRect& aRect, const class TSize& aSize),
                               float aXPos, float aYPos,
                               TInt aDx, TInt aDy)
{
   TRect rect = (*ptrFunc)(aBaseRect,
         TSize(WFLayoutUtils::CalculateXValue(aBaseRect.Width(), aXPos),
               WFLayoutUtils::CalculateYValue(aBaseRect.Height(), aYPos)));
   rect.Move(aDx, aDy);
   if (aImage) {
      aImage->SetImageRect(rect);
   } else {
      aImage = CImageHandler::NewL(rect);
   }
}

void CGuideContainer::SetEdwinExtent(
		class CEikEdwin * aEdwin, 
		float aX, 
		float aY
) {
	//All the sizes for V3 are relative points based on the positions for V2
	TSize size = Rect().Size();
	aEdwin->SetExtent(
		WFLayoutUtils::CalculatePosUsingMainPaneFloat(aX, aY),
		size
	);
	aEdwin->DrawDeferred();
}
// End of File  
