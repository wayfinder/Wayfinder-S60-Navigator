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
#include <txtfmlyr.h>

#include "RsgInclude.h"
#include "TwoEdwinControl.h"
#include "memlog.h"

#define SCREEN_WIDTH_V2  176
#define SCREEN_HEIGHT_V2 144
#define MAX_EDWIN_HEIGHT 20

// Enumarations
enum TControls
{
   EFirstLabel,
   EFirstEdwin,
   ESecondLabel,
   ESecondEdwin,
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CTwoEdwinControl::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//

// Constructor
CTwoEdwinControl::CTwoEdwinControl( TInt aLabelOneUid, TInt aLabelTwoUid,
                                    TInt aEdwinOneUid, TInt aEdwinTwoUid )
{
   iLabelOneUid = aLabelOneUid;
   iLabelTwoUid = aLabelTwoUid;
   iEdwinOneUid = aEdwinOneUid;
   iEdwinTwoUid = aEdwinTwoUid;
}

// Destructor
CTwoEdwinControl::~CTwoEdwinControl()
{
//    LOGDEL(iLabelOne);
   delete iLabelOne;
   iLabelOne = NULL;

//    LOGDEL(iEdwinOne);
   delete iEdwinOne;
   iEdwinOne = NULL;

//    LOGDEL(iLabelTwo);
   delete iLabelTwo;
   iLabelTwo = NULL;

//    LOGDEL(iEdwinTwo);
   delete iEdwinTwo;
   iEdwinTwo = NULL;
}

void CTwoEdwinControl::ConstructFromResourceL( TResourceReader& aReader )
{
   TInt top    = aReader.ReadInt16();
   TInt left   = aReader.ReadInt16();
   TInt bottom = aReader.ReadInt16();
   TInt right  = aReader.ReadInt16();

   iFirstLabelPos  = CalculatePositions( left+10, top+8 );
   iFirstEdwinPos  = CalculatePositions( left+10, top+22 );
   iSecondLabelPos = CalculatePositions( left+10, top+50 );
   iSecondEdwinPos = CalculatePositions( left+10, top+64 );
//    iFirstLabelPos = TPoint( left+10, top+8 );
//    iFirstEdwinPos = TPoint( left+10, top+22 );

//    iSecondLabelPos = TPoint( left+10, top+50 );
//    iSecondEdwinPos = TPoint( left+10, top+64);

#if defined NAV2_CLIENT_SERIES60_V3
   const CFont* labelFont = AknLayoutUtils::FontFromId(EAknLogicalFontPrimarySmallFont);
#else
   const CFont* labelFont = LatinPlain12();
#endif

   TSize controlSize( CalculateSize( right-left, bottom-top ) );

   TBuf<KBuf32Length> text;
   TResourceReader reader;

   TRgb background( KRgbWhite );

   CCharFormatLayer* FormatLayer = CEikonEnv::NewDefaultCharFormatLayerL();
   
   TCharFormat charFormat;
   TCharFormatMask charFormatMask;

   FormatLayer->Sense( charFormat, charFormatMask );
   charFormat.iFontPresentation.iTextColor = KRgbBlack;
   charFormatMask.SetAttrib(EAttColor);
   FormatLayer->SetL(charFormat, charFormatMask);

   CCharFormatLayer* FormatLayer2 = FormatLayer->CloneL();

   iCoeEnv->ReadResource( text, iLabelOneUid );
   iLabelOne = new (ELeave) CEikLabel;
   //LOGNEW(iLabelOne, CEikLabel);
   iLabelOne->SetContainerWindowL( *this );
   iLabelOne->SetFont(labelFont);
   iLabelOne->SetTextL(text);
   iLabelOne->OverrideColorL( EColorControlBackground, background );

   iCoeEnv->CreateResourceReaderLC( reader, iEdwinOneUid );
   iEdwinOne = new (ELeave) CEikEdwin;
   //LOGNEW(iEdwinOne, CEikEdwin);
   iEdwinOne->SetContainerWindowL(*this);
   iEdwinOne->ConstructFromResourceL(reader);
   iEdwinOne->SetCharFormatLayer(FormatLayer);
   
   TMargins8 margins = iEdwinOne->Margins();
   margins.iTop += 3;
   margins.iLeft += 3;
   margins.iBottom = 0;
   margins.iRight = 0;

   iEdwinOne->SetBorderViewMargins(margins);
   iEdwinOne->SetBorder(TGulBorder::EShallowSunken);
   iEdwinOne->SetMaximumHeight(GetMaxEdwinHeight());
   CleanupStack::PopAndDestroy();  // Resource reader

   iCoeEnv->ReadResource( text, iLabelTwoUid );
   iLabelTwo = new (ELeave) CEikLabel;
   //LOGNEW(iLabelTwo, CEikLabel);
   iLabelTwo->SetContainerWindowL( *this );
   iLabelTwo->SetFont(labelFont);
   iLabelTwo->SetTextL(text);
   iLabelTwo->OverrideColorL( EColorControlBackground, background );
   
   iCoeEnv->CreateResourceReaderLC(reader, iEdwinTwoUid );
   iEdwinTwo = new (ELeave) CEikEdwin;
   //LOGNEW(iEdwinTwo, CEikEdwin);
   iEdwinTwo->SetContainerWindowL(*this);
   iEdwinTwo->ConstructFromResourceL(reader);
   iEdwinTwo->SetBorderViewMargins(margins);
   iEdwinTwo->SetBorder(TGulBorder::EShallowSunken);
   iEdwinTwo->SetCharFormatLayer(FormatLayer2);
   iEdwinTwo->SetMaximumHeight(GetMaxEdwinHeight());
   CleanupStack::PopAndDestroy();  // Resource reader

   SetSize( controlSize );
   ActivateL();
}

void CTwoEdwinControl::SetFirstEdwinL( TDes* aText )
{
   iEdwinOne->SetTextL( aText );
}

void CTwoEdwinControl::SetSecondEdwinL( TDes* aText )
{
   iEdwinTwo->SetTextL( aText );
}

void CTwoEdwinControl::GetFirstEdwin( TDes* aText )
{
   iEdwinOne->GetText( *aText );
}

void CTwoEdwinControl::GetSecondEdwin( TDes* aText )
{
   iEdwinTwo->GetText( *aText );
}

TBool CTwoEdwinControl::IsFirstFocused()
{
   return iEdwinOne->IsFocused();
}

TBool CTwoEdwinControl::IsSecondFocused()
{
   return iEdwinTwo->IsFocused();
}

void CTwoEdwinControl::SetSecondFocused( TBool aFocused, TDrawNow aDrawNow )
{
   iEdwinTwo->SetFocus( aFocused );
   iEdwinOne->SetFocus( !aFocused );
   if(aDrawNow)
      DrawNow();
}

void CTwoEdwinControl::SetFirstDimmed( TBool aDimmed )
{
   iEdwinOne->SetFocus( EFalse );
   iEdwinOne->SetDimmed( aDimmed );
   iLabelOne->SetDimmed( aDimmed );
}

TBool CTwoEdwinControl::IsFirstDimmed()
{
   return iEdwinOne->IsDimmed();
}

void CTwoEdwinControl::SetSecondDimmed( TBool aDimmed )
{
   iEdwinTwo->SetFocus( EFalse );
   iEdwinTwo->SetDimmed( aDimmed );
   iLabelTwo->SetDimmed( aDimmed );
}

TBool CTwoEdwinControl::IsSecondDimmed()
{
   return iEdwinTwo->IsDimmed();
}

// This function is called by the framework whenever a component in a dialog is 
// about to lose focus. It checks that the data in ithe component is valid.
void CTwoEdwinControl::PrepareForFocusLossL()
{
}

TKeyResponse CTwoEdwinControl::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }
	// Use the arrow keys to move focus between the controls.
	switch (aKeyEvent.iScanCode)
   {
   case EStdKeyDownArrow:
      if ( iEdwinOne->IsFocused() ){
         iEdwinOne->SetFocus(EFalse, EDrawNow);
         iEdwinTwo->SetFocus(ETrue, EDrawNow);
         DrawNow();
      }
      else if (iEdwinTwo->IsFocused()){
         iEdwinTwo->SetFocus(EFalse, EDrawNow);
         break;
      }
      return EKeyWasConsumed;
      break;
   case EStdKeyUpArrow:
      if (iEdwinTwo->IsFocused()){
         iEdwinTwo->SetFocus(EFalse, EDrawNow);
         iEdwinOne->SetFocus(ETrue, EDrawNow);
         DrawNow();
      }
      else if ( iEdwinOne->IsFocused() ){
         iEdwinOne->SetFocus(EFalse, EDrawNow);
         break;
      }
      return EKeyWasConsumed;
      break;
   default:
      if ( iEdwinOne->IsFocused() )
         return iEdwinOne->OfferKeyEventL( aKeyEvent, aType );
      else if ( iEdwinTwo->IsFocused() )
         return iEdwinTwo->OfferKeyEventL( aKeyEvent, aType );
      break;
   }
   return EKeyWasNotConsumed;
}

void CTwoEdwinControl::FocusChanged( TDrawNow aDrawNow )
{
   if ( IsFocused() ){
      iEdwinOne->SetFocus(ETrue, EDrawNow);
   }
	else{
      if (iEdwinOne->IsFocused())
         iEdwinOne->SetFocus(EFalse, EDrawNow);
      else
         iEdwinTwo->SetFocus(EFalse, EDrawNow);
   }
   if(aDrawNow)
      DrawNow();
}

// ---------------------------------------------------------
// CTwoEdwinControl::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CTwoEdwinControl::SizeChanged()
{
#ifdef NAV2_CLIENT_SERIES60_V3
   const CFont* editorFont = CEikonEnv::Static()->DenseFont();
   if( iLabelOne ){
      iLabelOne->SetExtent( iFirstLabelPos, iLabelOne->MinimumSize() );
   }
   if( iEdwinOne ){
      iEdwinOne->SetExtent( iFirstEdwinPos, GetSize( iEdwinOne->MinimumSize(),
                                                     editorFont, 
                                                     iEdwinOne->LayoutWidth() ) );
   }
   if( iLabelTwo ){
      iLabelTwo->SetExtent( iSecondLabelPos, iLabelTwo->MinimumSize() );
   }
   if( iEdwinTwo ){
      iEdwinTwo->SetExtent( iSecondEdwinPos, GetSize( iEdwinTwo->MinimumSize(),
                                                      editorFont, 
                                                      iEdwinTwo->LayoutWidth() ) );
   }  
#else
   if( iLabelOne ){
      iLabelOne->SetExtent( iFirstLabelPos, iLabelOne->MinimumSize() );
   }
   if( iEdwinOne ){
      iEdwinOne->SetExtent( iFirstEdwinPos, iEdwinOne->MinimumSize() );
   }
   if( iLabelTwo ){
      iLabelTwo->SetExtent( iSecondLabelPos, iLabelTwo->MinimumSize() );
   }
   if( iEdwinTwo ){
      iEdwinTwo->SetExtent( iSecondEdwinPos, iEdwinTwo->MinimumSize() );
   }
#endif
}

// ---------------------------------------------------------
// CTwoEdwinControl::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CTwoEdwinControl::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CTwoEdwinControl::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CTwoEdwinControl::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case EFirstLabel:
      return iLabelOne;
   case EFirstEdwin:
      return iEdwinOne;
   case ESecondLabel:
      return iLabelTwo;
   case ESecondEdwin:
      return iEdwinTwo;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CTwoEdwinControl::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CTwoEdwinControl::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();

   gc.SetPenStyle(CGraphicsContext::ENullPen);
   //gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
   gc.SetBrushColor( KRgbWhite );
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);
   
   gc.SetBrushStyle(CGraphicsContext::ENullBrush);
   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenColor( KRgbBlack );

   TPoint p1, p2;

   p1 = aRect.iTl;
   p1.iX += 6;
   p1.iY = 0;
   p2 = p1;
   p2.iY = aRect.iBr.iY;
   gc.DrawLine( p1, p2 );

   TRect rect = aRect;
   rect.iTl.iY += 2;
   rect.iTl.iX += 2;
   rect.iBr.iX -= 1;
   TInt height = rect.Height()/2;
   
   if ( iEdwinOne->IsFocused() ){
      rect.iBr.iY -= height;
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
   else if( iEdwinTwo->IsFocused() ){
      rect.iTl.iY += height;
      gc.DrawRect(rect);
      gc.SetPenColor( KRgbWhite );
      p1.iY = rect.iTl.iY+1;
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

// ---------------------------------------------------------
// CTwoEdwinControl::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CTwoEdwinControl::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

TPoint CTwoEdwinControl::CalculatePositions(TInt aX, TInt aY)
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
   return TPoint (((TInt)((float)rect.Width() * ((float)aX / SCREEN_WIDTH_V2))), 
                  ((TInt)((float)rect.Height() * ((float)aY / SCREEN_HEIGHT_V2))));
#else
   return TPoint(aX, aY);
#endif
}

TSize CTwoEdwinControl::CalculateSize(TInt aWidth, TInt aHeight)
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
   return TSize(((TInt)((float)rect.Width() * ((float)aWidth / SCREEN_WIDTH_V2))), 
                ((TInt)((float)rect.Height() * ((float)aHeight / SCREEN_HEIGHT_V2))));
#else
   return TSize(aWidth, aHeight);
#endif
}

TInt CTwoEdwinControl::GetMaxEdwinHeight()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
   return ((TInt)((float)rect.Height() * ((float)MAX_EDWIN_HEIGHT / SCREEN_HEIGHT_V2)));
#else
   return MAX_EDWIN_HEIGHT;
#endif
}

TSize CTwoEdwinControl::GetSize(TSize aSize, const CFont *aFont, TInt aNbrChars)
{
   return TSize(aSize.iWidth + aFont->MaxCharWidthInPixels() * aNbrChars,
                aSize.iHeight );
}


// End of File  
