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
#include <eikedwin.h>  // for edwin text boxes
#include <eikmfne.h>  
#include <barsread.h>  // for resource reader
#include <gulcolor.h>  // for color
#include <aknutils.h>  // for fonts
#include <txtfmlyr.h>
#include <aknlayoutfont.h>

#include "RsgInclude.h"
#include "wficons.mbg"
#include "RouteContainer.h"
#include "WayFinderConstants.h"
#include "InfoCompass.h"
#include "memlog.h"
#include "RouteView.h"

#include "WFTextUtil.h"

#include "DistancePrintingPolicy.h"

#include "WFLayoutUtils.h"
#include "WayFinderAppUi.h"
#include "RectTools.h"

#include <e32std.h>

#if defined NAV2_CLIENT_SERIES60_V2
# include <aknsdrawutils.h>
#endif

#define SHORT_INFO_LABEL_WIDTH     65
#define MARGIN                5
#define SHORT_INFO_LABEL_LENGTH_IN_CHARS 8
#define SCREEN_WIDTH          176
#define DESTINATION_INFO_LABEL_POS TPoint(MARGIN, 20)
#define DISTANCE_INFO_LABEL_POS    TPoint(MARGIN, 54)
#define ETG_INFO_LABEL_POS         TPoint(MARGIN, 89)
#define ETA_INFO_LABEL_POS         TPoint(MARGIN, 124)
#define SPEED_INFO_LABEL_POS       TPoint(SCREEN_WIDTH-(MARGIN*2+SHORT_INFO_LABEL_WIDTH+2), 54)

#define DESTINATION_LABEL_POS TPoint(MARGIN, 4)
#define DISTANCE_LABEL_POS    TPoint(MARGIN, 38)
#define ETG_LABEL_POS         TPoint(MARGIN, 72)
#define ETA_LABEL_POS         TPoint(MARGIN, 107)
#define SPEED_LABEL_POS       TPoint(SCREEN_WIDTH-(MARGIN*2+SHORT_INFO_LABEL_WIDTH+2), 38)

#define COMPASS_CTRL_POS      TPoint(SCREEN_WIDTH-(MARGIN+SHORT_INFO_LABEL_WIDTH+2+2), 81)
#define COMPASS_SIZE          TSize( 60, 60 ) 

#if defined NAV2_CLIENT_SERIES60_V3
# define COMPASS_IMAGE         EMbmWficonsCompass
# define COMPASS_IMAGE_M       EMbmWficonsCompass_mask
# define COMPASS_IMAGE_NIGHT   EMbmWficonsCompass_red;
# define COMPASS_IMAGE_NIGHT_M EMbmWficonsCompass_red_mask;
#else
# define COMPASS_IMAGE         EMbmWficonsCompass60
# define COMPASS_IMAGE_M       EMbmWficonsCompass60_mask
# define COMPASS_IMAGE_NIGHT   EMbmWficonsCompass60_red;
# define COMPASS_IMAGE_NIGHT_M EMbmWficonsCompass60_mask;
#endif

using namespace isab;

_LIT( K1Number, "  %i" );
_LIT( K2Numbers, " %i" );
_LIT( K3Numbers, "%i" );

_LIT( KDistanceM, "%i m" );
_LIT( KDistanceK, "%i.%i km" );
_LIT( KDistanceKK, "%i km" );

_LIT( KSpeed1, "  %i" );

// Enumarations
enum TControls
{
   ESpeedLabel,
   ESpeedInfoLabel,
   EDestinationLabel,
   EDestinationInfoLabel,
   EDistanceLabel,
   EDistanceInfoLabel,
   EETGLabel,
   EETGInfoLabel,
   EETALabel,
   EETAInfoLabel,
   ECompassControl,
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================


class CRouteContainer*
CRouteContainer::NewL(CRouteView* aParent,
      const TRect& aRect,
      const TDesC& aMbmName,
      isab::Log* aLog,
      class CWayFinderAppUi* aAppUi)
{
   class CRouteContainer* self = new (ELeave) CRouteContainer(aLog);
   CleanupStack::PushL(self);
   self->SetMopParent(aParent);
   self->ConstructL(aParent, aRect, aMbmName, aAppUi);
   CleanupStack::Pop(self);
   return self;
}
// ---------------------------------------------------------
// CRouteContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CRouteContainer::ConstructL(CRouteView* aView,
      const TRect& aRect,
      const TDesC& aMbmName,
      class CWayFinderAppUi* aAppUi)
{
   iView = aView;
   CreateWindowL();
   iMbmName = aMbmName.Alloc();

   iAppUi = aAppUi;

   HBufC* tmp = HBufC::NewL(2);
   tmp->Des().Copy(_L("  "));
   HBufC* text;

   text = iCoeEnv->AllocReadResourceLC( R_DESTINATION_DESTINATION_LABEL );
   iDestinationLabel = new (ELeave) CEikLabel;
   LOGNEW(iDestinationLabel, CEikLabel);
   SetUpLabel(iDestinationLabel, text, *this);
   CleanupStack::PopAndDestroy( text );
   
   iDestinationInfoLabel = new (ELeave) CEikLabel;
   LOGNEW(iDestinationInfoLabel, CEikLabel);
   SetUpLabel(iDestinationInfoLabel, tmp, *this);

   text = iCoeEnv->AllocReadResourceLC( R_DESTINATION_DISTANCE_LABEL );
   iDistanceLabel = new (ELeave) CEikLabel;
   LOGNEW(iDistanceLabel, CEikLabel);
   SetUpLabel(iDistanceLabel, text, *this);
   CleanupStack::PopAndDestroy(text);

   iDistanceInfoLabel = new (ELeave) CEikLabel;
   LOGNEW(iDistanceInfoLabel, CEikLabel);
   SetUpLabel(iDistanceInfoLabel, tmp, *this);

   text = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_DESTINATION_ETA_LABEL );
   iETALabel = new (ELeave) CEikLabel;
   LOGNEW(iETALabel, CEikLabel);
   SetUpLabel(iETALabel, text, *this);
   CleanupStack::PopAndDestroy(text);
   text = iCoeEnv->AllocReadResourceLC( R_WAYFINDER_DESTINATION_ETG_LABEL );

   iETGLabel = new (ELeave) CEikLabel;
   LOGNEW(iETGLabel, CEikLabel);
   SetUpLabel(iETGLabel, text, *this);
   CleanupStack::PopAndDestroy(text);

#undef _SHOW_DURATION_
#ifdef _SHOW_DURATION_
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_DESTINATION_ETG_DURATION );
   iDurationETG = new (ELeave) CEikDurationEditor;
   LOGNEW(iDurationETG, CEikDurationEditor);
   iDurationETG->SetContainerWindowL(*this);
   iDurationETG->ConstructFromResourceL(reader);
   iDurationETG->SetBorder(TGulBorder::ENone);
   TTimeIntervalSeconds duration(0);
   iDurationETG->SetDuration(duration);
#else
   iETGInfoLabel = new (ELeave) CEikLabel;
   LOGNEW(iETGInfoLabel, CEikLabel);
   SetUpLabel(iETGInfoLabel, tmp, *this);
#endif
   iETAInfoLabel = new (ELeave) CEikLabel;
   LOGNEW(iETAEdwin, CEikLabel);
   SetUpLabel(iETAInfoLabel, tmp, *this);

   text = iCoeEnv->AllocReadResourceLC( R_INFO_SPEED_LABEL );
   iSpeedLabel = new (ELeave) CEikLabel;
   LOGNEW(iSpeedLabel, CEikLabel);
   SetUpLabel(iSpeedLabel, text, *this);
   CleanupStack::PopAndDestroy(text);

   iSpeedInfoLabel = new (ELeave) CEikLabel;
   LOGNEW(iSpeedInfoLabel, CEikLabel);
   SetUpLabel(iSpeedInfoLabel, tmp, *this);
  
   if (iView->IsIronVersion()) {
      iSpeedLabel->MakeVisible(EFalse);
      iSpeedInfoLabel->MakeVisible(EFalse);
   }

   SetNightModeL(iAppUi->IsNightMode());
//    iAppUi->GetBackgroundColor(iR, iG, iB);

   SetRect(aRect);
   ActivateL();
}

// Destructor
CRouteContainer::~CRouteContainer()
{  
   delete iCompass;
   delete iETGLabel;
   delete iETALabel;
   delete iDistanceLabel;
   delete iDestinationLabel;
   delete iSpeedLabel;
   delete iETGInfoLabel;
   delete iETAInfoLabel;
   delete iDurationETG;
   delete iDistanceInfoLabel;
   delete iDestinationInfoLabel;
   delete iSpeedInfoLabel;
   delete iMbmName;
}


void CRouteContainer::SetHeading( TInt aHeading )
{
   iCompass->SetHeading( aHeading );
}

void CRouteContainer::SetSpeed( TInt aSpeed )
{
   TBuf<64> text( _L("-") );
   if( aSpeed > -1 ){
      if( aSpeed < 10 ) {
         text.Format( K1Number, aSpeed );
      } else if( aSpeed < 100 ) {
         text.Format( K2Numbers, aSpeed );
      } else {
         text.Format( K3Numbers, aSpeed );
      }
      HBufC* tmp;
      switch (iView->GetDistanceMode()) {
         case DistancePrintingPolicy::ModeImperialYards:
         case DistancePrintingPolicy::ModeImperialFeet:
            tmp = iCoeEnv->AllocReadResourceLC( R_INFO_MPH_LABEL );
            break;
         case DistancePrintingPolicy::ModeInvalid:
         case DistancePrintingPolicy::ModeMetric:
         default:
            tmp = iCoeEnv->AllocReadResourceLC( R_INFO_KMH_LABEL );
            break;
      }
      text.Append(*tmp);
      CleanupStack::PopAndDestroy(tmp);
   }
   TRect oldRect = iSpeedInfoLabel->Rect();

   iSpeedInfoLabel->SetTextL( text );

   PositionOfControlChanged(iSpeedInfoLabel, iSpeedInfoLabel->Position());
   
   Window().Invalidate(Shrink(oldRect, -3));
   Window().Invalidate(Shrink(iSpeedInfoLabel->Rect(), -3));
}

void CRouteContainer::SetDestination( TDesC &aDestination )
{
   TBuf<KBuf64Length> destination;
   destination.Zero();

   if (aDestination.Length() > destination.MaxLength()-2) {
      destination.Copy( aDestination.Ptr(), destination.MaxLength()-2 );
      destination.PtrZ();
   } else {
      destination.Copy(aDestination);
   }

   TRect oldRect = iDestinationInfoLabel->Rect();   

   iDestinationInfoLabel->SetTextL( destination );

   PositionOfControlChanged(iDestinationInfoLabel, iDestinationInfoLabel->Position());
   
   Window().Invalidate(Shrink(oldRect, -3));
   Window().Invalidate(Shrink(iDestinationInfoLabel->Rect(), -3));
}

void CRouteContainer::SetDistanceToGoal( TInt32 aDistance )
{
   TBuf<32> text(_L(""));
   if( aDistance == MAX_INT32 || aDistance <= 0 ){
      //Invalid distance.
      text.Copy(_L(" "));
   } else {
      DistancePrintingPolicy::DistanceMode mode =
         DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());

      char* tmp2 = DistancePrintingPolicy::convertDistance(aDistance, mode);
      if (tmp2) {
         WFTextUtil::char2TDes(text, tmp2);
         delete[] tmp2;
      } else {
         text.Copy(_L(" "));
      }
   }

   TRect oldRect = iDistanceInfoLabel->Rect();   

   iDistanceInfoLabel->SetTextL( text );

   PositionOfControlChanged(iDistanceInfoLabel, iDistanceInfoLabel->Position());
   
   Window().Invalidate(Shrink(oldRect, -3));
   Window().Invalidate(Shrink(iDistanceInfoLabel->Rect(), -3));
}

void CRouteContainer::SetETG( TInt32 aTime )
{
#ifdef _SHOW_DURATION_
   iDurationETG->SetDuration( aTime );
#else
   {
      TBuf<16> text;
      TBuf<16> appendedText;
      text.Copy( _L("") );
      TInt32 myTime = aTime;

      if (myTime <= 0) {
         // Nothing.
      } else if (myTime < 30) {
         // Seconds!
         text.AppendNum( myTime, EDecimal );
         iCoeEnv->ReadResource( appendedText, R_TEXT_ROUTEV_ETG_SEC );
         text.Append( appendedText );
      } else if (myTime < 3600) {
         // Minutes, rounded up.
         myTime += 30;
         myTime /= 60;
         text.AppendNum( myTime, EDecimal );
         iCoeEnv->ReadResource( appendedText, R_TEXT_ROUTEV_ETG_MIN );
         text.Append( appendedText );
      } else if (myTime < 36000) {
         myTime += 30;
         myTime /= 60;
         TInt32 houres = myTime/60;
         myTime -= houres*60;
         text.AppendNum( houres, EDecimal );
#ifdef _DECIMAL_MINUTES_
         // Hours, with decimal.
         myTime = (myTime/60)*10;
         text.Append( _L(".") );
         text.AppendNum( myTime, EDecimal );
         iCoeEnv->ReadResource( appendedText, R_TEXT_ROUTEV_ETG_HOURS );
         text.Append( appendedText );
#else
         // Hours, with minutes.
         iCoeEnv->ReadResource( appendedText, R_TEXT_ROUTEV_ETG_HOURS );
         text.Append( appendedText );
         text.AppendNum( myTime, EDecimal );
         iCoeEnv->ReadResource( appendedText, R_TEXT_ROUTEV_ETG_MIN );
         text.Append( appendedText );
#endif
      } else if (myTime < 172800) {
         // Hours, no decimals.
         myTime /= 3600;
         text.AppendNum( myTime, EDecimal );
         iCoeEnv->ReadResource( appendedText, R_TEXT_ROUTEV_ETG_HOURS );
         text.Append( appendedText );
      } else if (myTime < 604800) {
         // Days, with decimal.
         myTime /= 8640;
         text.AppendNum( (myTime/10), EDecimal );
         text.Append( _L(".") );
         text.AppendNum( (myTime%10), EDecimal );
         iCoeEnv->ReadResource( appendedText, R_TEXT_ROUTEV_ETG_DAYS );
         text.Append( appendedText );
      } else {
         iCoeEnv->ReadResource( appendedText, R_TEXT_ROUTEV_ETG_WEEKS );
         text.Append( appendedText );
      }

      TRect oldRect = iETGInfoLabel->Rect();

      iETGInfoLabel->SetTextL( text );
      PositionOfControlChanged(iETGInfoLabel, iETGInfoLabel->Position());
      
      Window().Invalidate(Shrink(oldRect, -3));
      Window().Invalidate(Shrink(iETGInfoLabel->Rect(), -3));
   }
#endif
   TBuf<30> text;
   text.Copy( _L("") );

   if (aTime <= 0) {
      // Nothing.
   } else {
      TTime now;
      now.HomeTime();

      now += (TTimeIntervalSeconds)aTime;

      _LIT(KDateString,"%-B%:0%J%:1%T%:3%+B");
      now.FormatL(text, KDateString);
   }

   TRect oldRect = iETAInfoLabel->Rect();

   iETAInfoLabel->SetTextL(text);

   PositionOfControlChanged(iETAInfoLabel, iETAInfoLabel->Position());
   
   Window().Invalidate(Shrink(oldRect, -3));
   Window().Invalidate(Shrink(iETAInfoLabel->Rect(), -3));
}

// ---------------------------------------------------------
// CRouteContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CRouteContainer::SizeChanged()
{
   // Position out the different components
   TRect viewRect = Rect();
   if (WFLayoutUtils::WideScreen()) {      
      // If wide screen, center the view rect.
      TRect rect = WFLayoutUtils::GetMainPaneRect(ETrue, 60);
      viewRect.iBr.iX = rect.iBr.iX;
      viewRect.iTl.iX = rect.iTl.iX;
   }

   if(iCompass){
      iCompass->SetRect(LowerRight(viewRect, iCompass->MinimumSize()));
   }

   // Number of lines with components that should be drawn to the screen.
   TInt numParts = 8;

   // The height of each component
   TInt partHeight = viewRect.Height() / numParts;                                     

   // Left margin
   TInt xMargin = 8 + viewRect.iTl.iX;

   // The position of a component, starts with the one at the top.
   TPoint pos(xMargin, viewRect.iTl.iY);

   // Size of a component
   TSize size(viewRect.Width(), partHeight);

   // Position out each component, start with the one at the top.
   // Increase pos for each line.
   if( iDestinationLabel ){
      PositionOfControlChanged(iDestinationLabel, pos);
      pos.iY += size.iHeight;
   }
   if( iDestinationInfoLabel ){
      PositionOfControlChanged(iDestinationInfoLabel, pos);
      pos.iY += size.iHeight;
   }
   if( iSpeedLabel ){
      // Speed label should be positioned at the same height as the 
      // DestinationInfoLabel but its x coord will be 55% of the screen
      // width
      PositionOfControlChanged(iSpeedLabel, 
                               TPoint(TInt(viewRect.Width() * 0.55), pos.iY));
   }
   if( iDistanceLabel ){
      PositionOfControlChanged(iDistanceLabel, pos);
      pos.iY += size.iHeight;
   }
   if( iSpeedInfoLabel ){
      // Speed info label should be positioned at the same height as the 
      // DestinationLabel but its x coord will be 55% of the screen
      // width
      TInt xPos;
      if(iSpeedLabel) {
         xPos = iSpeedLabel->Position().iX;
      } else {
         xPos = TInt(viewRect.Width() * 0.55);
      }
      PositionOfControlChanged(iSpeedInfoLabel, 
                               TPoint(xPos, pos.iY));
   }
   if( iDistanceInfoLabel ){
      PositionOfControlChanged(iDistanceInfoLabel, pos);
      pos.iY += size.iHeight;
   }
   if (iETGLabel) {
      PositionOfControlChanged(iETGLabel, pos);
      pos.iY += size.iHeight;
   }
   if (iETGInfoLabel) {
      PositionOfControlChanged(iETGInfoLabel, pos);
      pos.iY += size.iHeight;
   }
   if (iETALabel) {
      PositionOfControlChanged(iETALabel, pos);
      pos.iY += size.iHeight;
   }
   if (iETAInfoLabel) {
      PositionOfControlChanged(iETAInfoLabel, pos);
      pos.iY += size.iHeight;
   }
}

void
CRouteContainer::PositionOfControlChanged(CCoeControl *control, TPoint pos)
{
   const CAknLayoutFont* font = AknLayoutUtils::LayoutFontFromId(EAknLogicalFontPrimaryFont);
   TSize size = control->MinimumSize();
   size.iHeight = font->TextPaneHeight();
   control->SetExtent(pos, size);
}

// ---------------------------------------------------------
// CRouteContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CRouteContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CRouteContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CRouteContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case ESpeedLabel:
      return iSpeedLabel;
   case ESpeedInfoLabel:
      return iSpeedInfoLabel;
   case EETALabel:
      return iETALabel;
   case EETAInfoLabel:
      return iETAInfoLabel;
   case EETGLabel:
      return iETGLabel;
   case EETGInfoLabel:
#ifdef _SHOW_DURATION_
      return iDurationETG;
#else
      return iETGInfoLabel;
#endif
   case ECompassControl:
      return iCompass;
   case EDistanceLabel:
      return iDistanceLabel;
   case EDistanceInfoLabel:
      return iDistanceInfoLabel;
   case EDestinationLabel:
      return iDestinationLabel;
   case EDestinationInfoLabel:
      return iDestinationInfoLabel;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CRouteContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CRouteContainer::Draw(const TRect& aRect) const
{ 
   CWindowGc& gc = SystemGc();
   gc.SetPenStyle(CGraphicsContext::ENullPen);
//    gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) ); 
   gc.SetBrushColor( TRgb( iR, iG, iB ) ); 
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);

   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenColor(iBubblePenColor);
   gc.SetBrushColor(iBubbleBgColor);

   //the labels that will have bubbles.
   class CCoeControl* bubbles[] = { iDestinationInfoLabel,
                                    iDistanceInfoLabel,
                                    iETGInfoLabel,
                                    iETAInfoLabel,
                                    iSpeedInfoLabel };

   size_t num = sizeof(bubbles)/sizeof(*bubbles);
   for(size_t a = 0; a < num; ++a){
      TRect rect = bubbles[a]->Rect();
      if(rect.Intersects(aRect)){
         gc.DrawRoundRect(Shrink(rect, TSize(-3,-1)), TSize(8,8));
      } 
   }
}

// ---------------------------------------------------------
// CRouteContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CRouteContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

void CRouteContainer::SetUpLabel(class CEikLabel* aLabel,
                                 HBufC* aText, 
                                 class CCoeControl& aContainerWindow)
{
   aLabel->SetContainerWindowL(aContainerWindow);
#ifdef NAV2_CLIENT_SERIES60_V3
   aLabel->SetFont(AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont));
#else
   aLabel->SetFont(LatinBold12());
#endif
   if (aText) {
      aLabel->SetTextL(*aText);
   }
}

void CRouteContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      TBool wideScreen = WFLayoutUtils::WideScreen();
      iCompass->SetCompassSizeAndPos(TRect(WFLayoutUtils::CalculatePosUsingMainPane(COMPASS_CTRL_POS, 
                                                                                    wideScreen),
                                           WFLayoutUtils::CalculateSizeUsingMainPane(COMPASS_SIZE, 
                                                                                     wideScreen)));
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
}

void CRouteContainer::SetNightModeL(TBool aNightMode)
{
   iAppUi->GetBackgroundColor(iR, iG, iB);
   TRgb textColor;
   TRgb northArrowColor;
   TRgb southArrowColor;
   TInt compassImage;
   TInt compassImageMask;
   if (aNightMode) {
      textColor        = KRgbRed;
      iBubblePenColor  = TRgb(0x30, 0x30, 0x30);
      iBubbleBgColor   = TRgb(0x30, 0x30, 0x30);
      compassImage     = COMPASS_IMAGE_NIGHT;
      compassImageMask = COMPASS_IMAGE_NIGHT_M;
      northArrowColor  = KRgbGray;
      southArrowColor  = KRgbRed;
   } else {
      textColor        = KRgbBlack;
      iBubblePenColor  = KRgbDarkGray;
      iBubbleBgColor   = KRgbWhite;
      compassImage     = COMPASS_IMAGE;
      compassImageMask = COMPASS_IMAGE_M;
      northArrowColor  = TRgb(255,255,255),
      southArrowColor  = TRgb(100,100,255);
   }
   iDestinationLabel->OverrideColorL(EColorLabelText, textColor);
   iDistanceLabel->OverrideColorL(EColorLabelText, textColor);
   iETALabel->OverrideColorL(EColorLabelText, textColor);
   iETGLabel->OverrideColorL(EColorLabelText, textColor);
   iSpeedLabel->OverrideColorL(EColorLabelText, textColor);

   iDestinationInfoLabel->OverrideColorL(EColorLabelText, textColor);
   iDistanceInfoLabel->OverrideColorL(EColorLabelText, textColor);
   iETAInfoLabel->OverrideColorL(EColorLabelText, textColor);
   iETGInfoLabel->OverrideColorL(EColorLabelText, textColor);
   iSpeedInfoLabel->OverrideColorL(EColorLabelText, textColor);

   TBool wideScreen = WFLayoutUtils::WideScreen();

   if (!iCompass) {
      iCompass = CInfoCompass::NewL(*this,
                                    TRect(WFLayoutUtils::
                                          CalculatePosUsingMainPane(COMPASS_CTRL_POS,
                                                                    wideScreen),
                                          WFLayoutUtils::
                                          CalculateSizeUsingMainPane(COMPASS_SIZE,
                                                                     wideScreen)),
                                    *iMbmName,
                                    compassImage,
                                    compassImageMask,
                                    EFalse,
                                    northArrowColor,
                                    southArrowColor);
   } else {
      iCompass->SetNewImage(TRect(WFLayoutUtils::
                                  CalculatePosUsingMainPane(COMPASS_CTRL_POS,
                                                            wideScreen),
                                  WFLayoutUtils::
                                  CalculateSizeUsingMainPane(COMPASS_SIZE,
                                                             wideScreen)),
                            *iMbmName,
                            compassImage,
                            compassImageMask,
                            EFalse,
                            northArrowColor,
                            southArrowColor);
   }
}

// End of File  
