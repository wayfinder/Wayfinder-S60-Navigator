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
#include <barsread.h>  // for resource reader
#include <gulcolor.h>  // for color
#include <aknutils.h>  // for fonts
#include <txtfmlyr.h>
#include <aknlayoutfont.h>

#if defined NAV2_CLIENT_SERIES60_V2
#include <aknsdrawutils.h>
#endif

#include "RsgInclude.h"
#include "wficons.mbg"
#include "InfoContainer.h"
#include "InfoView.h"
#include "WayFinderConstants.h"
#include "InfoCompass.h"
#include "memlog.h"
#include "WFTextUtil.h"
#include "PositionDisplay.h"
#include "RectTools.h"

#include "DistancePrintingPolicy.h"
#include "WFLayoutUtils.h"

#define MARGIN_Y           4


#define SPEED_LABEL_POS       TPoint(  8, 60)
#define SPEED_EDWIN_POS       TPoint(  8, 78)

#define ALTITUDE_LABEL_POS    TPoint(  8, 102)
#define ALTITUDE_EDWIN_POS    TPoint(  8, 120)

#define COMPASS_CTRL_POS      TPoint( 96, 84)
#define COMPASS_SIZE          TSize( 75, 75 ) 

#define ANGLE_LABEL_CENTER    TPoint( 100, 136 )

#if defined NAV2_CLIENT_SERIES60_V3
#define COMPASS_IMAGE         EMbmWficonsCompass
#define COMPASS_IMAGE_M       EMbmWficonsCompass_mask
#else
#define COMPASS_IMAGE         EMbmWficonsCompass75
#define COMPASS_IMAGE_M       EMbmWficonsCompass75_mask
#endif

_LIT( K1Number, "  %i" );
_LIT( K2Numbers, " %i" );
_LIT( K3Numbers, "%i" );

// Enumerations
enum TControls
{
   EPositionDisplay,
   ESpeedLabel,
   EKmhLabel,
   ECompassControl,
   EAltitudeLabel,
   EAltitudeHeader,
   EDegreeLabel,
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================
HBufC*
CInfoContainer::GetSpeedLabel() const
{
   switch (iView->GetDistanceMode()) {
      case isab::DistancePrintingPolicy::ModeImperialYards:
      case isab::DistancePrintingPolicy::ModeImperialFeet:
         return iCoeEnv->AllocReadResourceL( R_INFO_MPH_LABEL );
         break;
      case isab::DistancePrintingPolicy::ModeInvalid:
      case isab::DistancePrintingPolicy::ModeMetric:
      default:
         return iCoeEnv->AllocReadResourceL( R_INFO_KMH_LABEL );
         break;
   }
}

class CInfoContainer*
CInfoContainer::NewL(class CInfoView* aParent,
                     const TRect& aRect,
                     const TDesC& aMbmFile,
                     isab::Log* aLog)
{
   class CInfoContainer* self = new (ELeave) CInfoContainer(aLog);
   CleanupStack::PushL(self);
   self->SetMopParent(aParent);
   self->ConstructL(aParent, aRect, aMbmFile);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CInfoContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CInfoContainer::ConstructL(CInfoView* aView,
      const TRect& aRect,
      const TDesC& aMbmFile)
{
   iView = aView;
   CreateWindowL();
   iMbmFile = aMbmFile.Alloc();

/*    TResourceReader reader; */
   HBufC* text;

   TRgb bg(KBackgroundRed, KBackgroundGreen, KBackgroundBlue);
   iPosition = CPositionDisplay::NewL(*this,
         R_CONNECT_CURRENT_POSITION,
         R_INFO_LAT_PLUS_SIGN,
         R_INFO_LAT_MINUS_SIGN,
         R_INFO_LON_PLUS_SIGN,
         R_INFO_LON_MINUS_SIGN,
         bg,
         KRgbDarkGray, KRgbWhite, KRgbBlack);

   
   iLabelSpeed = new (ELeave) CEikLabel;
   LOGNEW(iLabelSpeed, CEikLabel);
   iLabelSpeed->SetContainerWindowL( *this );
   iLabelSpeed->SetFont(AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont));
   text = iCoeEnv->AllocReadResourceL( R_INFO_SPEED_LABEL);
   iLabelSpeed->SetTextL(*text);
   delete text;
 
   text = GetSpeedLabel();
   iLabelKmh = new (ELeave) CEikLabel;
   LOGNEW(iLabelKmh, CEikLabel);
   iLabelKmh->SetContainerWindowL( *this );
   iLabelKmh->SetFont(AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont));
   iLabelKmh->SetTextL(*text);
   delete text;

   iCompass = CInfoCompass::NewL(*this,
                                 TRect( WFLayoutUtils::
                                        CalculatePosUsingMainPane( COMPASS_CTRL_POS ), 
                                        WFLayoutUtils::
                                        CalculateSizeUsingMainPane( COMPASS_SIZE ) ),
                                 *iMbmFile,
                                 COMPASS_IMAGE,
                                 COMPASS_IMAGE_M);

   iLabelDegree = new (ELeave) CEikLabel;
   LOGNEW(iLabelDegree, CEikLabel);
   iLabelDegree->SetContainerWindowL( *this );
   iLabelDegree->SetFont(AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont));
   iLabelDegree->SetTextL( _L("-") );

   iLabelAltitude = new (ELeave) CEikLabel;
   LOGNEW(iLabelAltitude, CEikLabel);
   iLabelAltitude->SetContainerWindowL( *this );
   iLabelAltitude->SetFont(AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont));
   iLabelAltitude->SetTextL( _L("-") );

   iLabelAltitudeHeader = new (ELeave) CEikLabel;
   LOGNEW(iLabelAltitudeHeader, CEikLabel);
   iLabelAltitudeHeader->SetContainerWindowL( *this );
   iLabelAltitudeHeader->SetFont(AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont));
   text = iCoeEnv->AllocReadResourceL( R_WF_GPS_ALTITUDE );
   iLabelAltitudeHeader->SetTextL( *text  );
   delete text;

   SetRect(aRect);
   ActivateL();
}

// Destructor
CInfoContainer::~CInfoContainer()
{
   delete iLabelSpeed;
   delete iCompass;
   delete iLabelKmh;
   delete iLabelDegree;
   delete iLabelAltitudeHeader;
   delete iLabelAltitude;
   delete iMbmFile;
   delete iPosition;
}

void CInfoContainer::SetPosL( TInt32 aLatitude, TInt32 aLongitude )
{
   iPosition->SetPos(aLatitude, aLongitude);
}

void CInfoContainer::SetLatitudeL( TInt32 aLatitude )
{
   iPosition->SetLat( aLatitude );
}

void CInfoContainer::SetLongitudeL( TInt32 aLongitude )
{
   iPosition->SetLon( aLongitude );
}


void CInfoContainer::SetPadding(HBufC* text, TInt num)
{
   if( num < 10 ) {
      text->Des().Copy(_L("  "));
   } else if( num < 100 ) {
      text->Des().Copy(_L(" "));
   } else {
      text->Des().Copy(_L(""));
   }
}

void CInfoContainer::SetAltitude( TInt aAltitude )
{
   isab::DistancePrintingPolicy::DistanceMode mode =
      isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());

   HBufC* tmp;
   aAltitude /= 10;
   char* tmp2 = isab::DistancePrintingPolicy::convertAltitude(aAltitude,
         mode,
         isab::DistancePrintingPolicy::Exact);
   if (tmp2) {
      tmp = WFTextUtil::AllocLC(tmp2);
      delete[] tmp2;
   } else {
      tmp = HBufC::NewLC(16);
      tmp->Des().Copy(_L(" "));
   }
   HBufC* text = HBufC::NewLC(tmp->Length()+8);
   SetPadding(text, aAltitude);
   text->Des().Append(*tmp);

   TRect oldRect = iLabelAltitude->Rect();

   iLabelAltitude->SetTextL(*text);
   PositionOfControlChanged(iLabelAltitude, iLabelAltitude->Position());
   CleanupStack::PopAndDestroy(text);
   CleanupStack::PopAndDestroy(tmp);

   Window().Invalidate(Shrink(oldRect, -3));
   Window().Invalidate(Shrink(iLabelAltitude->Rect(), -3));
}

void CInfoContainer::SetHeading( TInt aHeading )
{
   HBufC *textBuf = HBufC::NewLC(32);
   TPtr text= textBuf->Des();
   if( aHeading > -1 ){
      text.Num(aHeading, EDecimal);
      HBufC* degree = CEikonEnv::Static()->AllocReadResourceLC(R_DEGREE);
      text.Append(*degree);
      CleanupStack::PopAndDestroy(degree);
   } else {
      text.Copy(_L("-"));
   }

   TRect oldRect = iLabelDegree->Rect();

   iLabelDegree->SetTextL( *textBuf );

   TPoint pos = iLabelDegree->Position();
   pos = TPoint (Rect().Width()/2+4 - iLabelDegree->MinimumSize().iWidth/2,
         pos.iY);
   PositionOfControlChanged(iLabelDegree, pos);

   iCompass->SetHeading( aHeading );
   CleanupStack::PopAndDestroy(textBuf);

   Window().Invalidate(Shrink(oldRect, -3));
   Window().Invalidate(Shrink(iLabelDegree->Rect(), -3));
}

void CInfoContainer::SetSpeed( TInt aSpeed )
{
   HBufC* textBuf = HBufC::NewLC(32);
   textBuf->Des().Copy(KNullDesC);
   if( aSpeed < 0 ){
      aSpeed = 0;
   }
   SetPadding(textBuf, aSpeed);
   textBuf->Des().AppendNum(aSpeed);

   HBufC *tmp = GetSpeedLabel();
   textBuf->Des().Append(*tmp);
   delete tmp;

   TRect oldRect = iLabelKmh->Rect();

   iLabelKmh->SetTextL( *textBuf );

   PositionOfControlChanged(iLabelKmh, iLabelKmh->Position());
   CleanupStack::PopAndDestroy(textBuf);

   Window().Invalidate(Shrink(oldRect, -3));
   Window().Invalidate(Shrink(iLabelKmh->Rect(), -3));
}

void CInfoContainer::SetSpeedLimit( TInt /*aLimit*/ )
{
}

// ---------------------------------------------------------
// CInfoContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CInfoContainer::SizeChanged()
{
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
   TInt numParts = 9;
   TInt topParts = 4;
   TInt bottomParts = numParts - topParts;

   TInt partHeight = viewRect.Height() / numParts;                                     
   TInt topHeight = partHeight * topParts;
   TInt bottomHeight = partHeight * bottomParts;

   if(iPosition){
      TSize coordSize = TSize(viewRect.Width(), topHeight);
      TRect coordRect = Shrink(TRect(viewRect.iTl, coordSize), 0);
      iPosition->SetRect(coordRect);                            
   }                                                            

   TInt bottomStart = topHeight;
   TInt xMargin = 8 + viewRect.iTl.iX;

   TInt bottomUnits = 4;
   TInt bottomUnitsHeight = bottomHeight;
   TInt bottomUnitSize = bottomUnitsHeight / bottomUnits;

   TPoint pos(xMargin, bottomStart);
   TSize size(viewRect.Width(), bottomUnitSize);

   if( iLabelSpeed ){
      PositionOfControlChanged(iLabelSpeed, pos);
      pos.iY += size.iHeight;
   }
   if( iLabelKmh ){
      PositionOfControlChanged(iLabelKmh, pos);
      pos.iY += size.iHeight;
   }
   if( iLabelAltitudeHeader ){
      PositionOfControlChanged(iLabelAltitudeHeader, pos);
      pos.iY += size.iHeight;
   }
   if( iLabelAltitude ){
      PositionOfControlChanged(iLabelAltitude, pos);
      /*       pos.iY += size.iHeight; */
   }

   if( iLabelDegree ){
      pos = TPoint (Rect().Width()/2+4 - iLabelDegree->MinimumSize().iWidth/2,
                    pos.iY);
      PositionOfControlChanged(iLabelDegree, pos);
   }
}

void
CInfoContainer::PositionOfControlChanged(CCoeControl *control, TPoint pos)
{
   const CAknLayoutFont* font = AknLayoutUtils::LayoutFontFromId(EAknLogicalFontPrimaryFont);
   TSize size = control->MinimumSize();
   size.iHeight = font->TextPaneHeight();
   control->SetExtent(pos, size);
}

// ---------------------------------------------------------
// CInfoContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CInfoContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CInfoContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CInfoContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case EPositionDisplay:
      return iPosition;
   case ESpeedLabel:
      return iLabelSpeed;
   case EKmhLabel:
      return iLabelKmh;
   case ECompassControl:
      return iCompass;
   case EAltitudeLabel:
      return iLabelAltitude;
   case EAltitudeHeader:
      return iLabelAltitudeHeader;
   case EDegreeLabel:
      return iLabelDegree;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CInfoContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CInfoContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();

   gc.SetClippingRect(aRect);
   gc.SetBrushColor(TRgb(KBackgroundRed, KBackgroundGreen, KBackgroundBlue));
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   
   gc.Clear(aRect);

   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenColor(KRgbDarkGray);
   gc.SetBrushColor(KRgbWhite);

   //the labels that will have bubbles.
   class CCoeControl* bubbles[] = {iLabelAltitude, iLabelKmh, iLabelDegree };
   size_t num = sizeof(bubbles)/sizeof(*bubbles);
   for(size_t a = 0; a < num; ++a){
      TRect rect = bubbles[a]->Rect();
      if(rect.Intersects(aRect)){
         gc.DrawRoundRect(Shrink(rect, TSize(-3,-1)), TSize(8,8));
      } 
   }

}

// ---------------------------------------------------------
// CInfoContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CInfoContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                          TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

void CInfoContainer::HandleResourceChange(TInt aType)
{
   if (aType == KEikDynamicLayoutVariantSwitch) {
      iCompass->SetCompassSizeAndPos(TRect(WFLayoutUtils::CalculatePosUsingMainPane(COMPASS_CTRL_POS),
                                           WFLayoutUtils::CalculateSizeUsingMainPane(COMPASS_SIZE)));
      SetRect(WFLayoutUtils::GetMainPaneRect());      
   }
}

// End of File  
