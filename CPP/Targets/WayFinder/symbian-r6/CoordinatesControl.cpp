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
#include <barsread.h>    // for resource reader
#include <eikbctrl.h>    // for border control
#include <gulcolor.h>    // for color
#include <aknutils.h>    // for fonts
#include <coeinput.h>
#include <txtfmlyr.h>

#include "RsgInclude.h"
#include "WayFinderConstants.h"
#include "CoordinatesControl.h"
#include "WFLayoutUtils.h"
/* #include "IsabTypes.h" */
#include "memlog.h"
#include "wayfinder.loc"
_LIT(KSign, "%c");
_LIT(K3Numbers,   "  %i");
_LIT(K2Numbers,   " %i");
_LIT(K1Numbers,   "%i");
_LIT(K10Numbers,  "0%i");

#define SCREEN_WIDTH_V2  176
#define SCREEN_HEIGHT_V2 144
#define TOP              91
#define LEFT             0
#define BOTTOM           142
#define RIGHT            176      
#define LABELOFFSET      10
#define MAX_EDWIN_HEIGHT 20

// Enumarations
enum TControls
{
   ELat0Edwin,
   ELat1Edwin,
   ELat2Edwin,
   ELat3Edwin,
   ELat4Edwin,
   ELon0Edwin,
   ELon1Edwin,
   ELon2Edwin,
   ELon3Edwin,
   ELon4Edwin,
   ELatLabel,
   EFirstLatLabel=ELatLabel,
   ELat1Label,
   ELat2Label,
   ELat3Label,
   ELat4Label,
   ELonLabel,
   ELon1Label,
   ELon2Label,
   ELon3Label,
   ELon4Label,
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCoordinatesControl::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//

// Constructor
CCoordinatesControl::CCoordinatesControl(
                                          TRgb aBackground, 
                                          TRgb aText,
                                          TBool aBold )
{
   iBackground = aBackground;
   iText = aText;
   iBold = aBold;
}

// Destructor
CCoordinatesControl::~CCoordinatesControl()
{
   for (int i = 0; i < MAX_LABELS; i++) {
      LOGDEL(iLabelLat[i]);
      delete iLabelLat[i];
   }

   LOGDEL(iEdwinLat0);
   delete iEdwinLat0;

   LOGDEL(iEdwinLat1);
   delete iEdwinLat1;

   LOGDEL(iEdwinLat2);
   delete iEdwinLat2;
   
   LOGDEL(iEdwinLat3);
   delete iEdwinLat3;
   
   LOGDEL(iEdwinLat4);
   delete iEdwinLat4;
   
   LOGDEL(iEdwinLon0);
   delete iEdwinLon0;

   LOGDEL(iEdwinLon1);
   delete iEdwinLon1;

   LOGDEL(iEdwinLon2);
   delete iEdwinLon2;

   LOGDEL(iEdwinLon3);
   delete iEdwinLon3;

   LOGDEL(iEdwinLon4);
   delete iEdwinLon4;
}

void CCoordinatesControl::ConstructFromResourceL( TResourceReader& aReader )
{
   TInt top    = aReader.ReadInt16();
   TInt left   = aReader.ReadInt16();
   TInt bottom = aReader.ReadInt16();
   TInt right  = aReader.ReadInt16();

   TInt labelOffset = 10;

   if( iBold )
      labelOffset = 3;

   iLatLabelPos[0] = WFLayoutUtils::CalculatePosUsingMainPane(left+labelOffset, top+5);
   iLatLabelPos[1] = WFLayoutUtils::CalculatePosUsingMainPane(left+76, top+3);
   iLatLabelPos[2] = WFLayoutUtils::CalculatePosUsingMainPane(left+106, top+3);
   iLatLabelPos[3] = WFLayoutUtils::CalculatePosUsingMainPane(left+136, top+5);
   iLatLabelPos[4] = WFLayoutUtils::CalculatePosUsingMainPane(left+170, top+3);
   iLatLabelPos[5] = WFLayoutUtils::CalculatePosUsingMainPane(left+labelOffset, top+30);
   iLatLabelPos[6] = WFLayoutUtils::CalculatePosUsingMainPane(left+76, top+28);
   iLatLabelPos[7] = WFLayoutUtils::CalculatePosUsingMainPane(left+106, top+28);
   iLatLabelPos[8] = WFLayoutUtils::CalculatePosUsingMainPane(left+136, top+30);
   iLatLabelPos[9] = WFLayoutUtils::CalculatePosUsingMainPane(left+170, top+28);

   iLatEdwinPos[0] = WFLayoutUtils::CalculatePosUsingMainPane(left+27,top+0);
   iLatEdwinPos[1] = WFLayoutUtils::CalculatePosUsingMainPane(left+45,top+0);
   iLatEdwinPos[2] = WFLayoutUtils::CalculatePosUsingMainPane(left+81, top+0);
   iLatEdwinPos[3] = WFLayoutUtils::CalculatePosUsingMainPane(left+111, top+0);
   iLatEdwinPos[4] = WFLayoutUtils::CalculatePosUsingMainPane(left+139, top+0);
   iLatEdwinPos[5] = WFLayoutUtils::CalculatePosUsingMainPane(left+27, top+25);
   iLatEdwinPos[6] = WFLayoutUtils::CalculatePosUsingMainPane(left+45, top+25);
   iLatEdwinPos[7] = WFLayoutUtils::CalculatePosUsingMainPane(left+81, top+25);
   iLatEdwinPos[8] = WFLayoutUtils::CalculatePosUsingMainPane(left+111, top+25);
   iLatEdwinPos[9] = WFLayoutUtils::CalculatePosUsingMainPane(left+139, top+25);

//    TSize controlSize( CalculateSize( right-left, bottom-top ) );
// #if defined NAV2_CLIENT_SERIES60_V3
//    TRect rect;
//    AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
//    TSize controlSize(rect.Size());
// #else
//    TSize controlSize( right-left, bottom-top );
// #endif
//    TSize controlSize( CalculateSize( right - left, bottom - top ) );
   TSize controlSize( WFLayoutUtils::CalculateSizeUsingMainPane( right - left, bottom - top ) );
   CCharFormatLayer* FormatLayer = CEikonEnv::NewDefaultCharFormatLayerL();
   
   TCharFormat charFormat;
   TCharFormatMask charFormatMask;

   FormatLayer->Sense( charFormat, charFormatMask );
   charFormat.iFontPresentation.iTextColor = iText;
   charFormatMask.SetAttrib(EAttColor);
   FormatLayer->SetL(charFormat, charFormatMask);

   CCharFormatLayer* FormatLayer2 = FormatLayer->CloneL();
   CCharFormatLayer* FormatLayer3 = FormatLayer->CloneL();
   CCharFormatLayer* FormatLayer4 = FormatLayer->CloneL();
   CCharFormatLayer* FormatLayer5 = FormatLayer->CloneL();
   CCharFormatLayer* FormatLayer6 = FormatLayer->CloneL();
   CCharFormatLayer* FormatLayer7 = FormatLayer->CloneL();
   CCharFormatLayer* FormatLayer8 = FormatLayer->CloneL();
   CCharFormatLayer* FormatLayer9 = FormatLayer->CloneL();
   CCharFormatLayer* FormatLayer10 = FormatLayer->CloneL();

   TResourceReader reader;

   const CFont* font;
   const CFont* labelFont;

#if defined NAV2_CLIENT_SERIES60_V3
   font = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont);
   labelFont = AknLayoutUtils::FontFromId(EAknLogicalFontPrimarySmallFont);
#else
   if( iBold )
      font = LatinBold12();
   else
      font = LatinPlain12();
   labelFont = LatinBold12();
#endif

   for (int i=0; i < MAX_LABELS; i++) {
      iLabelLat[i] = new (ELeave) CEikLabel;
      LOGNEW(iLabelLat[i], CEikLabel);
      iLabelLat[i]->SetContainerWindowL( *this );
      iLabelLat[i]->SetFont(font);
      iLabelLat[i]->OverrideColorL( EColorControlBackground, iBackground );
   }

   HBufC* text = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_LAT_SHORT);
   iLabelLat[0]->SetTextL(*text);
   CleanupStack::PopAndDestroy(text);

   text = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_LON_SHORT);
   iLabelLat[5]->SetTextL(*text);
   CleanupStack::PopAndDestroy(text);

//    iLabelLat1->SetFont(LatinBold12());
   text = CEikonEnv::Static()->AllocReadResourceLC(R_DEGREE);
   iLabelLat[1]->SetTextL( *text );
   iLabelLat[6]->SetTextL( *text );
   CleanupStack::PopAndDestroy(text);
   
   iLabelLat[2]->SetTextL( _L("'") );
   iLabelLat[3]->SetTextL( _L(".") );
   iLabelLat[4]->SetTextL( _L("\"") );
   
   iLabelLat[7]->SetTextL( _L("'") );
   iLabelLat[8]->SetTextL( _L(".") );
   iLabelLat[9]->SetTextL( _L("\"") );

   iCoeEnv->CreateResourceReaderLC( reader, R_WAYFINDER_LAT0_EDWIN );
   iEdwinLat0 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLat0, CEikEdwin);
   iEdwinLat0->SetContainerWindowL(*this);
   iEdwinLat0->ConstructFromResourceL(reader);
   iEdwinLat0->SetCharFormatLayer(FormatLayer);
   
   TMargins8 margins = iEdwinLat0->Margins();
   margins.iTop += 3;
   margins.iLeft += 3;
   margins.iBottom = 0;
   margins.iRight = 0;

   iEdwinLat0->SetBorderViewMargins(margins);
   iEdwinLat0->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLat0->SetMaximumHeight(20);
   iEdwinLat0->SetMaximumHeight(GetMaxEdwinHeight());
   CleanupStack::PopAndDestroy();  // Resource reader

   iCoeEnv->CreateResourceReaderLC( reader, R_WAYFINDER_LAT3_EDWIN );
   iEdwinLat1 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLat1, CEikEdwin);
   iEdwinLat1->SetContainerWindowL(*this);
   iEdwinLat1->ConstructFromResourceL(reader);
   iEdwinLat1->SetCharFormatLayer(FormatLayer2);
   iEdwinLat1->SetBorderViewMargins(margins);
   iEdwinLat1->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLat1->SetMaximumHeight(20);
   iEdwinLat1->SetMaximumHeight(GetMaxEdwinHeight());
   iEdwinLat1->SetInputCapabilitiesL( TCoeInputCapabilities(
                                      TCoeInputCapabilities::EWesternNumericIntegerPositive ) );
   CleanupStack::PopAndDestroy();  // Resource reader
   
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_LAT2_EDWIN);
   iEdwinLat2 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLat2, CEikEdwin);
   iEdwinLat2->SetContainerWindowL(*this);
   iEdwinLat2->ConstructFromResourceL(reader);
   iEdwinLat2->SetCharFormatLayer(FormatLayer3);
   iEdwinLat2->SetBorderViewMargins(margins);
   iEdwinLat2->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLat2->SetMaximumHeight(20);
   iEdwinLat2->SetMaximumHeight(GetMaxEdwinHeight());
   iEdwinLat2->SetInputCapabilitiesL( TCoeInputCapabilities(
                                      TCoeInputCapabilities::EWesternNumericIntegerPositive ) );
   CleanupStack::PopAndDestroy();  // Resource reader
   
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_LAT2_EDWIN);
   iEdwinLat3 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLat3, CEikEdwin);
   iEdwinLat3->SetContainerWindowL(*this);
   iEdwinLat3->ConstructFromResourceL(reader);
   iEdwinLat3->SetCharFormatLayer(FormatLayer4);
   iEdwinLat3->SetBorderViewMargins(margins);
   iEdwinLat3->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLat3->SetMaximumHeight(20);
   iEdwinLat3->SetMaximumHeight(GetMaxEdwinHeight());
   iEdwinLat3->SetInputCapabilitiesL( TCoeInputCapabilities(
                                      TCoeInputCapabilities::EWesternNumericIntegerPositive ) );
   CleanupStack::PopAndDestroy();  // Resource reader
   
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_LAT2_EDWIN);
   iEdwinLat4 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLat4, CEikEdwin);
   iEdwinLat4->SetContainerWindowL(*this);
   iEdwinLat4->ConstructFromResourceL(reader);
   iEdwinLat4->SetCharFormatLayer(FormatLayer5);
   iEdwinLat4->SetBorderViewMargins(margins);
   iEdwinLat4->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLat4->SetMaximumHeight(20);
   iEdwinLat4->SetMaximumHeight(GetMaxEdwinHeight());
   iEdwinLat4->SetInputCapabilitiesL( TCoeInputCapabilities(
                                      TCoeInputCapabilities::EWesternNumericIntegerPositive ) );
   CleanupStack::PopAndDestroy();  // Resource reader
   
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_LAT0_EDWIN);
   iEdwinLon0 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLon0, CEikEdwin);
   iEdwinLon0->SetContainerWindowL(*this);
   iEdwinLon0->ConstructFromResourceL(reader);
   iEdwinLon0->SetCharFormatLayer(FormatLayer6);
   iEdwinLon0->SetBorderViewMargins(margins);
   iEdwinLon0->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLon0->SetMaximumHeight(20);
   iEdwinLon0->SetMaximumHeight(GetMaxEdwinHeight());
   CleanupStack::PopAndDestroy();  // Resource reader
   
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_LAT3_EDWIN);
   iEdwinLon1 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLon1, CEikEdwin);
   iEdwinLon1->SetContainerWindowL(*this);
   iEdwinLon1->ConstructFromResourceL(reader);
   iEdwinLon1->SetCharFormatLayer(FormatLayer7);
   iEdwinLon1->SetBorderViewMargins(margins);
   iEdwinLon1->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLon1->SetMaximumHeight(20);
   iEdwinLon1->SetMaximumHeight(GetMaxEdwinHeight());
   iEdwinLon1->SetInputCapabilitiesL( TCoeInputCapabilities(
                                      TCoeInputCapabilities::EWesternNumericIntegerPositive ) );
   CleanupStack::PopAndDestroy();  // Resource reader
   
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_LAT2_EDWIN);
   iEdwinLon2 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLon2, CEikEdwin);
   iEdwinLon2->SetContainerWindowL(*this);
   iEdwinLon2->ConstructFromResourceL(reader);
   iEdwinLon2->SetCharFormatLayer(FormatLayer8);
   iEdwinLon2->SetBorderViewMargins(margins);
   iEdwinLon2->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLon2->SetMaximumHeight(20);
   iEdwinLon2->SetMaximumHeight(GetMaxEdwinHeight());
   iEdwinLon2->SetInputCapabilitiesL( TCoeInputCapabilities(
                                      TCoeInputCapabilities::EWesternNumericIntegerPositive ) );
   CleanupStack::PopAndDestroy();  // Resource reader
   
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_LAT2_EDWIN);
   iEdwinLon3 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLon3, CEikEdwin);
   iEdwinLon3->SetContainerWindowL(*this);
   iEdwinLon3->ConstructFromResourceL(reader);
   iEdwinLon3->SetCharFormatLayer(FormatLayer9);
   iEdwinLon3->SetBorderViewMargins(margins);
   iEdwinLon3->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLon3->SetMaximumHeight(20);
   iEdwinLon3->SetMaximumHeight(GetMaxEdwinHeight());
   iEdwinLon3->SetInputCapabilitiesL( TCoeInputCapabilities(
                                      TCoeInputCapabilities::EWesternNumericIntegerPositive ) );
   CleanupStack::PopAndDestroy();  // Resource reader
   
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_LAT2_EDWIN);
   iEdwinLon4 = new (ELeave) CEikEdwin;
   LOGNEW(iEdwinLon4, CEikEdwin);
   iEdwinLon4->SetContainerWindowL(*this);
   iEdwinLon4->ConstructFromResourceL(reader);
   iEdwinLon4->SetCharFormatLayer(FormatLayer10);
   iEdwinLon4->SetBorderViewMargins(margins);
   iEdwinLon4->SetBorder(TGulBorder::EShallowSunken);
//    iEdwinLon4->SetMaximumHeight(20);
   iEdwinLon4->SetMaximumHeight(GetMaxEdwinHeight());
   iEdwinLon4->SetInputCapabilitiesL( TCoeInputCapabilities(
                                      TCoeInputCapabilities::EWesternNumericIntegerPositive ) );
   CleanupStack::PopAndDestroy();  // Resource reader
   
   SetSize( controlSize );
   ActivateL();
}

void
CCoordinatesControl::SetCoordinateEditors( TChar& aSign,
                                    TInt* aDeg,
                                    TInt* aMin,
                                    TInt* aSec,
                                    TInt* aSecFr,
                                    TBool isLat)
{ 
   TBuf<2> textSign;
   TBuf<4> textDeg;
   TBuf<4> textMin;
   TBuf<4> textSec;
   TBuf<4> textSecFr;
   
   // If using Format on s60v3 it gives a USER-EXEC 3
   // panic. By using Append we get rid of this problem.
//    textSign.Format( KSign, aSign );
   textSign.Append(aSign);
   if( *aDeg < 10 ) {
      textDeg.Format( K3Numbers, *aDeg );
   } else if( *aDeg < 100 ) {
      textDeg.Format( K2Numbers, *aDeg );
   } else {
      textDeg.Format( K1Numbers, *aDeg );
   }

   if( *aMin < 10 ) {
      textMin.Format( K2Numbers, *aMin );
   } else {
      textMin.Format( K1Numbers, *aMin );
   }

   if( *aSec < 10 ) {
      textSec.Format( K2Numbers, *aSec );
   } else {
      textSec.Format( K1Numbers, *aSec );
   }

   if( *aSecFr == 0 ) {
      textSecFr.Format( K1Numbers, *aSecFr );
   } else if (*aSecFr <10) {
      textSecFr.Format( K10Numbers, *aSecFr );
   } else {
      textSecFr.Format( K1Numbers, *aSecFr );
   }

   if (isLat) {
      iEdwinLat0->SetTextL( &textSign );
      iEdwinLat1->SetTextL( &textDeg );
      iEdwinLat2->SetTextL( &textMin );
      iEdwinLat3->SetTextL( &textSec );
      iEdwinLat4->SetTextL( &textSecFr );
   } else {
      iEdwinLon0->SetTextL( &textSign );
      iEdwinLon1->SetTextL( &textDeg );
      iEdwinLon2->SetTextL( &textMin );
      iEdwinLon3->SetTextL( &textSec );
      iEdwinLon4->SetTextL( &textSecFr );
   }
}

void CCoordinatesControl::SetLatitudeL( TInt32 &aLat )
{
   TChar sign;
   TInt deg;
   TInt min;
   TInt sec;
   TInt secfr;
   SetCoordinate( aLat, sign, deg, min, sec, secfr, ETrue );
   SetCoordinateEditors( sign, &deg, &min, &sec, &secfr, ETrue ); 
}

void CCoordinatesControl::SetLongitudeL( TInt32 &aLon )
{
   TChar sign;
   TInt deg;
   TInt min;
   TInt sec;
   TInt secfr;
   SetCoordinate( aLon, sign, deg, min, sec, secfr, EFalse );
   SetCoordinateEditors( sign, &deg, &min, &sec, &secfr, EFalse );
}

TBool CCoordinatesControl::GetLatitudeL( TInt32 &aLat )
{
   TBuf<2> sign;
   TBuf<4> deg;
   TBuf<4> min;
   TBuf<4> sec;
   TBuf<4> secfr;
   iEdwinLat0->GetText( sign );
   iEdwinLat1->GetText( deg );
   iEdwinLat2->GetText( min );
   iEdwinLat3->GetText( sec );
   iEdwinLat4->GetText( secfr );
   sign.Trim();
   deg.Trim();
   min.Trim();
   sec.Trim();
   secfr.Trim();
   if (sign.Length() <= 0) {
      /* Bad sign. */
      return EFalse;
   }
   if (deg.Length() <= 0) {
      return EFalse;
   }
   if (min.Length() <= 0) {
      return EFalse;
   }
   if (sec.Length() <= 0) {
      return EFalse;
   }
   if (secfr.Length() <= 0) {
      return EFalse;
   }
   TChar tmp = sign[0];
   GetCoordinate(aLat, tmp, TLex16(deg), TLex16(min), TLex16(sec), TLex16(secfr));
   sign[0] = tmp;
   return ETrue;
}

TBool CCoordinatesControl::GetLongitudeL( TInt32 &aLon )
{
   TBuf<2> sign;
   TBuf<4> deg;
   TBuf<4> min;
   TBuf<4> sec;
   TBuf<4> secfr;
   iEdwinLon0->GetText( sign );
   iEdwinLon1->GetText( deg );
   iEdwinLon2->GetText( min );
   iEdwinLon3->GetText( sec );
   iEdwinLon4->GetText( secfr );
   sign.Trim();
   deg.Trim();
   min.Trim();
   sec.Trim();
   secfr.Trim();
   if (sign.Length() <= 0) {
      /* Bad sign. */
      return EFalse;
   }
   if (deg.Length() <= 0) {
      return EFalse;
   }
   if (min.Length() <= 0) {
      return EFalse;
   }
   if (sec.Length() <= 0) {
      return EFalse;
   }
   if (secfr.Length() <= 0) {
      return EFalse;
   }
   TChar tmp = sign[0];
   GetCoordinate( aLon, tmp, TLex16(deg), TLex16(min), TLex16(sec), TLex16(secfr) );
   sign[0] = tmp;
   return ETrue;
}

TBool CCoordinatesControl::IsLatFocused()
{
   return iEdwinLat0->IsFocused() || 
          iEdwinLat1->IsFocused() || 
          iEdwinLat2->IsFocused() || 
          iEdwinLat3->IsFocused() || 
          iEdwinLat4->IsFocused();
}

TBool CCoordinatesControl::IsLonFocused()
{
   return iEdwinLon0->IsFocused() || 
          iEdwinLon1->IsFocused() || 
          iEdwinLon2->IsFocused() || 
          iEdwinLon3->IsFocused() || 
          iEdwinLon4->IsFocused();
}

void CCoordinatesControl::SetLatFocus( TBool aFocused, TDrawNow aDrawNow )
{
   iEdwinLat0->SetFocus( aFocused );
   if( aFocused ){
      iEdwinLon0->SetFocus( EFalse );
      iEdwinLon1->SetFocus( EFalse );
      iEdwinLon2->SetFocus( EFalse );
      iEdwinLon3->SetFocus( EFalse );
      iEdwinLon4->SetFocus( EFalse );
      iEdwinLat0->SelectAllL();
   }
   if(aDrawNow) {
      DrawDeferred();
   }
}

void CCoordinatesControl::SetLonFocus( TBool aFocused, TDrawNow aDrawNow )
{
   iEdwinLon0->SetFocus( aFocused );
   if( aFocused ){
      iEdwinLat0->SetFocus( EFalse );
      iEdwinLat1->SetFocus( EFalse );
      iEdwinLat2->SetFocus( EFalse );
      iEdwinLat3->SetFocus( EFalse );
      iEdwinLat4->SetFocus( EFalse );
      iEdwinLon0->SelectAllL();
   }
   if(aDrawNow) {
      DrawDeferred();
   }
}

void CCoordinatesControl::SetCoordinate( TInt32 aCoord, 
                                         TChar &aSign, 
                                         TInt &aDeg, 
                                         TInt &aMin, 
                                         TInt &aSec, 
                                         TInt &aSecFr, 
                                         TBool aIsLat )
{
   TReal coord = (aCoord / K_PI) * 180;
   TBuf<32> tmp;

   coord /= 100000000;

   if( aIsLat ){
      if( coord < 0 ){
         iCoeEnv->ReadResource( tmp, R_INFO_LAT_MINUS_SIGN );
         coord *= -1;
      } else {
         iCoeEnv->ReadResource( tmp, R_INFO_LAT_PLUS_SIGN );
      }
      aSign = tmp[0];
   } else {
      if( coord < 0 ){
         iCoeEnv->ReadResource( tmp, R_INFO_LON_MINUS_SIGN );
         coord *= -1;
      } else {
         iCoeEnv->ReadResource( tmp, R_INFO_LON_PLUS_SIGN );
      }
      aSign = tmp[0];
   }

   aDeg = (TUint)coord;
   coord = (coord-aDeg)*60;
   aMin = (TUint)coord;
   coord = (coord-aMin)*60;
   aSec = (TUint)(coord);
   coord = (coord-aSec)*100;
   aSecFr = (TUint)(coord+0.5);
   if (aSecFr >= 100) {
      aSec++;
      aSecFr -= 100;
   }
}

void CCoordinatesControl::GetCoordinate( TInt32 &aCoord,
                                         TChar &aSign,
                                         TLex16 aDeg,
                                         TLex16 aMin,
                                         TLex16 aSec,
                                         TLex16 aSecFr)
{
   TReal coord;
   TInt32 newCoord;

   TInt sign = 1;
   TInt deg;
   TInt min;
   TInt sec;
   TInt secfr;
   TBuf<32> tmp;

   aDeg.Val( deg );
   aMin.Val( min );
   aSec.Val( sec );
   aSecFr.Val( secfr );

   coord = (TReal)secfr/100;

   coord = (TReal)((coord+sec)/60);
   coord = (TReal)((coord+min)/60);
   coord += (TReal)deg;

   coord = (coord*K_PI)/180;

   iCoeEnv->ReadResource( tmp, R_INFO_LAT_MINUS_SIGN );
   TChar lat_minus = tmp[0];
   iCoeEnv->ReadResource( tmp, R_INFO_LON_MINUS_SIGN );
   TChar lon_minus = tmp[0];
   // FIXME - potentially broken code using upcase on unicode chars...
   aSign.UpperCase();
   lat_minus.UpperCase();
   lon_minus.UpperCase();
   if( aSign == lat_minus || aSign == lon_minus ) {
      sign = -1;
   }

   newCoord = (TInt32)( (coord * 100000000) + 0.5 );

   aCoord = newCoord*sign;
}

TBool
CCoordinatesControl::WrapRight(CEikEdwin* edwin)
{
   return edwin->IsFocused() && edwin->CursorPos() == edwin->TextLength();
}
TBool
CCoordinatesControl::WrapLeft(CEikEdwin* edwin)
{
   return edwin->IsFocused() && edwin->CursorPos() == 0;
}

TKeyResponse
CCoordinatesControl::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
	// Use the arrow keys to move focus between the controls.
   CEikEdwin* iFromWin = NULL;
   CEikEdwin* iToWin = NULL;

   if ( aType == EEventKeyUp ){
	   // Use the arrow keys to move focus between the controls.
      switch( aKeyEvent.iScanCode )
      {
      case EStdKeyRightArrow:
         {

            if ( WrapRight(iEdwinLat0) ) {
               iFromWin=iEdwinLat0;
               iToWin=iEdwinLat1;
            } else if( WrapRight(iEdwinLat1)) {
               iFromWin=iEdwinLat1;
               iToWin=iEdwinLat2;
            } else if( WrapRight(iEdwinLat2)) {
               iFromWin=iEdwinLat2;
               iToWin=iEdwinLat3;
            } else if( WrapRight(iEdwinLat3)) {
               iFromWin=iEdwinLat3;
               iToWin=iEdwinLat4;
            } else if( WrapRight(iEdwinLat4)) {
               iFromWin=iEdwinLat4;
               iToWin=iEdwinLat0;
            } else if ( WrapRight(iEdwinLon0)) {
               iFromWin=iEdwinLon0;
               iToWin=iEdwinLon1;
            } else if( WrapRight(iEdwinLon1)) {
               iFromWin=iEdwinLon1;
               iToWin=iEdwinLon2;
            } else if( WrapRight(iEdwinLon2)) {
               iFromWin=iEdwinLon2;
               iToWin=iEdwinLon3;
            } else if( WrapRight(iEdwinLon3)) {
               iFromWin=iEdwinLon3;
               iToWin=iEdwinLon4;
            } else if( WrapRight(iEdwinLon4)) {
               iFromWin=iEdwinLon4;
               iToWin=iEdwinLon0;
            }
            if (iFromWin && iToWin) {
               iFromWin->SetFocus(EFalse, EDrawNow);
               iToWin->SetFocus(ETrue, EDrawNow);
               iToWin->SelectAllL();
               return EKeyWasConsumed;
            }
            break;
         }
      case EStdKeyLeftArrow:
         {
            if ( WrapLeft(iEdwinLat0)) {
               iFromWin=iEdwinLat0;
               iToWin=iEdwinLat4;
            } else if ( WrapLeft(iEdwinLat1)) {
               iFromWin=iEdwinLat1;
               iToWin=iEdwinLat0;
            } else if ( WrapLeft(iEdwinLat2)) {
               iFromWin=iEdwinLat2;
               iToWin=iEdwinLat1;
            } else if ( WrapLeft(iEdwinLat3)) {
               iFromWin=iEdwinLat3;           
               iToWin=iEdwinLat2;             
            } else if ( WrapLeft(iEdwinLat4)) {
               iFromWin=iEdwinLat4;           
               iToWin=iEdwinLat3;
            } else if ( WrapLeft(iEdwinLon0)) {
               iFromWin=iEdwinLon0;           
               iToWin=iEdwinLon4;             
            } else if ( WrapLeft(iEdwinLon1)) {
               iFromWin=iEdwinLon1;           
               iToWin=iEdwinLon0;             
            } else if ( WrapLeft(iEdwinLon2)) {
               iFromWin=iEdwinLon2;           
               iToWin=iEdwinLon1;             
            } else if ( WrapLeft(iEdwinLon3)) {
               iFromWin=iEdwinLon3;           
               iToWin=iEdwinLon2;             
            } else if ( WrapLeft(iEdwinLon4)) {
               iFromWin=iEdwinLon4;
               iToWin=iEdwinLon3;
            }
            if (iFromWin && iToWin) {
               iFromWin->SetFocus(EFalse, EDrawNow);
               iToWin->SetFocus(ETrue, EDrawNow);
               iToWin->SelectAllL();
               return EKeyWasConsumed;
            }
         }
      }
   }
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }
   if ( iEdwinLat0->IsFocused() ) {
      return iEdwinLat0->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLat1->IsFocused() ) {
      return iEdwinLat1->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLat2->IsFocused() ) {
      return iEdwinLat2->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLat3->IsFocused() ) {
      return iEdwinLat3->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLat4->IsFocused() ) {
      return iEdwinLat4->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLon0->IsFocused() ) {
      return iEdwinLon0->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLon1->IsFocused() ) {
      return iEdwinLon1->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLon2->IsFocused() ) {
      return iEdwinLon2->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLon3->IsFocused() ) {
      return iEdwinLon3->OfferKeyEventL( aKeyEvent, aType );
   } else if ( iEdwinLon4->IsFocused() ) {
      return iEdwinLon4->OfferKeyEventL( aKeyEvent, aType );
   }
   return EKeyWasNotConsumed;
}

void CCoordinatesControl::FocusChanged( TDrawNow aDrawNow )
{
   if ( IsFocused() ){
      iEdwinLat0->SetFocus(ETrue, EDrawNow);
      iEdwinLat0->SelectAllL();
   } else {
      iEdwinLat0->SetFocus(EFalse, EDrawNow);
      iEdwinLat1->SetFocus(EFalse, EDrawNow);
      iEdwinLat2->SetFocus(EFalse, EDrawNow);
      iEdwinLat3->SetFocus(EFalse, EDrawNow);
      iEdwinLat4->SetFocus(EFalse, EDrawNow);
      iEdwinLon0->SetFocus(EFalse, EDrawNow);
      iEdwinLon1->SetFocus(EFalse, EDrawNow);
      iEdwinLon2->SetFocus(EFalse, EDrawNow);
      iEdwinLon3->SetFocus(EFalse, EDrawNow);
      iEdwinLon4->SetFocus(EFalse, EDrawNow);
   }
   if(aDrawNow) {
      DrawDeferred();
   }
}

// ---------------------------------------------------------
// CCoordinatesControl::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CCoordinatesControl::SizeChanged()
{
   if ( iLabelLat[0] ) {

      for (int i = 0; i < MAX_LABELS; i++) {
         iLabelLat[i]->SetExtent(iLatLabelPos[i], iLabelLat[i]->MinimumSize());
      }

      /** Get the system font */
      const CFont* editorFont = CEikonEnv::Static()->DenseFont();
      
      iEdwinLat0->SetExtent(iLatEdwinPos[0], GetSize(iEdwinLat0->MinimumSize(), editorFont,  1 ) );
      iEdwinLat1->SetExtent(iLatEdwinPos[1], GetSize(iEdwinLat1->MinimumSize(), editorFont,  3 ) );
      iEdwinLat2->SetExtent(iLatEdwinPos[2], GetSize(iEdwinLat2->MinimumSize(), editorFont,  2 ) );
      iEdwinLat3->SetExtent(iLatEdwinPos[3], GetSize(iEdwinLat3->MinimumSize(), editorFont,  2 ) );
      iEdwinLat4->SetExtent(iLatEdwinPos[4], GetSize(iEdwinLat4->MinimumSize(), editorFont,  2 ) );
      iEdwinLon0->SetExtent(iLatEdwinPos[5], GetSize(iEdwinLon0->MinimumSize(), editorFont, 1 ) );
      iEdwinLon1->SetExtent(iLatEdwinPos[6], GetSize(iEdwinLon1->MinimumSize(), editorFont, 3 ) );
      iEdwinLon2->SetExtent(iLatEdwinPos[7], GetSize(iEdwinLon2->MinimumSize(), editorFont, 2 ) );
      iEdwinLon3->SetExtent(iLatEdwinPos[8], GetSize(iEdwinLon3->MinimumSize(), editorFont, 2 ) );
      iEdwinLon4->SetExtent(iLatEdwinPos[9], GetSize(iEdwinLon4->MinimumSize(), editorFont, 2 ) );
   }
}

// ---------------------------------------------------------
// CCoordinatesControl::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CCoordinatesControl::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CCoordinatesControl::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CCoordinatesControl::ComponentControl(TInt aIndex) const
{
   if ((aIndex >= EFirstLatLabel) && (aIndex < MAX_LABELS+EFirstLatLabel)) {
      return iLabelLat[aIndex-MAX_LABELS];
   }
   switch ( aIndex )
   {
   case ELat0Edwin:
      return iEdwinLat0;
   case ELat1Edwin:
      return iEdwinLat1;
   case ELat2Edwin:
      return iEdwinLat2;
   case ELat3Edwin:
      return iEdwinLat3;
   case ELat4Edwin:
      return iEdwinLat4;
   case ELon0Edwin:
      return iEdwinLon0;
   case ELon1Edwin:
      return iEdwinLon1;
   case ELon2Edwin:
      return iEdwinLon2;
   case ELon3Edwin:
      return iEdwinLon3;
   case ELon4Edwin:
      return iEdwinLon4;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CCoordinatesControl::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CCoordinatesControl::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();

   gc.SetPenStyle(CGraphicsContext::ENullPen);
   gc.SetBrushColor( iBackground );
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);

   if( !iBold ){   
      gc.SetPenStyle(CGraphicsContext::ESolidPen);
      gc.SetPenColor( KRgbBlack );
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);

      TPoint p1, p2;
      p1 = aRect.iBr;
      p2 = p1;
      p2.iX = aRect.iTl.iX + 6;
      gc.DrawLine( p2, p1 );
      p1 = p2;
      p1.iY = aRect.iTl.iY;
      gc.DrawLine( p1, p2 );

      TRect rect = aRect;
      rect.iTl.iX += 2;
      rect.iBr.iX -= 1;
      rect.iBr.iY -= 2;
      TInt height = rect.Height()/2;
   
      if ( iEdwinLat0->IsFocused() ||
           iEdwinLat1->IsFocused() ||
           iEdwinLat2->IsFocused() ||
           iEdwinLat3->IsFocused() ||
           iEdwinLat4->IsFocused() ){
         rect.iBr.iY -= (height);
         gc.DrawRect(rect);
         gc.SetPenColor( KRgbWhite );
         p2.iY = rect.iBr.iY-1;
         gc.DrawLine( p1, p2 );
         gc.SetPenColor( KRgbDarkGray );
         p1 = rect.iTl;
         p1.iX += 1;
         p1.iY += 1;
         p2 = p1;
         p2.iX = rect.iBr.iX-1;
         gc.DrawLine( p1, p2 );
         p2 = p1;
         p2.iY = rect.iBr.iY-1;
         gc.DrawLine( p1, p2 );
      }
      else 
      if ( iEdwinLon0->IsFocused() ||
           iEdwinLon1->IsFocused() ||
           iEdwinLon2->IsFocused() ||
           iEdwinLon3->IsFocused() ||
           iEdwinLon4->IsFocused() ){
         rect.iTl.iY += (height);
         gc.DrawRect(rect);
         gc.SetPenColor( KRgbWhite );
         p1.iY = rect.iTl.iY+1;
         p2.iY = rect.iBr.iY-1;
         gc.DrawLine( p1, p2 );
         gc.SetPenColor( KRgbDarkGray );
         p1 = rect.iTl;
         p1.iX += 1;
         p1.iY += 1;
         p2 = p1;
         p2.iX = rect.iBr.iX-1;
         gc.DrawLine( p1, p2 );
         p2 = p1;
         p2.iY = rect.iBr.iY-1;
         gc.DrawLine( p1, p2 );
      }
   }
}

// ---------------------------------------------------------
// CCoordinatesControl::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CCoordinatesControl::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

// TPoint CCoordinatesControl::CalculatePositions(TInt aX, TInt aY)
// {
// #if defined NAV2_CLIENT_SERIES60_V3
//    TRect rect;
//    AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
//    return TPoint (((TInt)((float)rect.Width() * ((float)aX / SCREEN_WIDTH_V2))), 
//                   ((TInt)((float)rect.Height() * ((float)aY / SCREEN_HEIGHT_V2))));
// #else
//    return TPoint(aX, aY);
// #endif
// }

TSize CCoordinatesControl::GetSize(TSize aSize, const CFont *aFont, TInt aNbrChars)
{
   return TSize(aSize.iWidth + aFont->MaxCharWidthInPixels() * aNbrChars,
                aSize.iHeight);
}

TInt CCoordinatesControl::GetMaxEdwinHeight()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
   return ((TInt)((float)rect.Height() * ((float)MAX_EDWIN_HEIGHT / SCREEN_HEIGHT_V2)));
#else
   return MAX_EDWIN_HEIGHT;
#endif
}

// TSize CCoordinatesControl::CalculateSize(TInt aWidth, TInt aHeight)
// {
// #if defined NAV2_CLIENT_SERIES60_V3
//    TRect rect;
//    AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
//    return TSize(((TInt)((float)rect.Width() * ((float)aWidth / SCREEN_WIDTH_V2))), 
//                 ((TInt)((float)rect.Height() * ((float)aHeight / SCREEN_HEIGHT_V2))));
// #else
//    return TSize(aWidth, aHeight);
// #endif
// }
// End of File  
