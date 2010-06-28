/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "PositionDisplay.h"
#include <e32def.h>         //_LIT
#include <math.h>           //floor
#include <coemain.h>        //CCoeEnv
#if defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES80
# include <cknlogicalfont.h> //TCknLogicalFont
#endif
#include <eiklabel.h>       //CEikLabel
#include <eikenv.h>         //CEikonEnv
#include <stdlib.h>         //abs

#include "WFTextUtil.h"

static const double KPI = 3.14159265358979323846264338327950288419716939937510;

  
void SetCoordinate(TInt32 aCoord, 
                   TInt &aSign, 
                   TInt &aDeg, 
                   TInt &aMin, 
                   TInt &aSec, 
                   TInt &aParts)
{  
   TReal coord = (aCoord / KPI) * 180;
   coord /= 100000000;

   aSign = coord < 0 ? -1 : 1;
   coord *= aSign;

   aDeg = TUint(floor(coord));
   coord = (coord-aDeg)*60;
   aMin = TUint(floor(coord));
   coord = (coord-aMin)*60;
   aSec = TUint(floor(coord));
   coord = (coord-aSec)*100;
   aParts = TUint(floor(coord+0.5));
   if( aParts >= 99 ){
      aParts = 0;
   }
}

void CCoordinateLineDisplay::SetCoordinate(TInt32 aCoord, 
                                           TValueParts& aParts) const
{
   ::SetCoordinate(aCoord, aParts.iSign, aParts.iDeg, aParts.iMin, aParts.iSec,
                   aParts.iParts);
}


CCoordinateLineDisplay::CCoordinateLineDisplay(TInt aMaxVal, 
                                               class TRgb aBackground, 
                                               class TRgb aText) : 
   iMaxVal(aMaxVal), iBgClr(aBackground), iTxtClr(aText)
{
}

void CCoordinateLineDisplay::ConstructL(class CCoeControl& aParent, 
                                        TInt aPosId, TInt aNegId)
{
   SetContainerWindowL(aParent);
   const TInt letters[] = {aNegId, aPosId, aPosId};
   const TInt num = sizeof(letters)/sizeof(*letters);
   TBuf<2> tmp;
   for(TInt a = 0; a < num; ++a){
      iCoeEnv->ReadResource(tmp, letters[a]);
      iDirections.Append(tmp[0]);
   }
   Format(0); //must be after iDirections has been set up.
   ActivateL();
}

class CCoordinateLineDisplay* 
CCoordinateLineDisplay::NewLC(class CCoeControl& aParent, 
                              TInt aPosId, TInt aNegId, TInt aMaxVal,
                              class TRgb aBackground, class TRgb aText)
{
   class CCoordinateLineDisplay* self = 
      new CCoordinateLineDisplay(aMaxVal, aBackground, aText);
   CleanupStack::PushL(self);
   self->ConstructL(aParent, aPosId, aNegId);
   return self;
}

class CCoordinateLineDisplay* 
CCoordinateLineDisplay::NewL(class CCoeControl& aParent,
                             TInt aPosId, TInt aNegId, TInt aMaxVal,
                             class TRgb aBackground, class TRgb aText)
{
   class CCoordinateLineDisplay* self = 
      CCoordinateLineDisplay::NewLC(aParent, aPosId, aNegId, aMaxVal, 
                                    aBackground, aText);
   CleanupStack::Pop(self);
   return self;
}

CCoordinateLineDisplay::~CCoordinateLineDisplay()
{
}

void CCoordinateLineDisplay::Draw(const TRect& aRect) const
{
   TValueParts parts;
   SetCoordinate(iVal, parts);
   const TInt numLetters = sizeof("E 179+59+59+99+") - 1;
   const TInt pixelwidth = Rect().Width() - 16;
   const TInt step = pixelwidth / numLetters;

   CWindowGc& gc = SystemGc();
   gc.SetClippingRect(aRect);
   gc.SetBrushColor(iBgClr);
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   //gc.Clear(Rect()); //paint background color.
   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenColor(iTxtClr);   

#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1
   TCknLogicalFont logicalfont(TCknLogicalFont::EViewLevel2ZoomDefaultFont);
   const class CFont* font = iEikonEnv->Font(logicalfont);
#else
   const class CFont* font = CCoeEnv::Static()->NormalFont();
#endif
   gc.UseFont(font);
   
   TPoint offset(Rect().iTl.iX + (step/2), Rect().iTl.iY + 2);
   TInt baseline = font->AscentInPixels();

   TRect paintRect(offset, TSize(step, Rect().Height() - 4));
   baseline = baseline + ((paintRect.Height() - baseline)/2) - 1;
   for(TInt pos = 0; pos < iNewString.Length(); ++pos){
      gc.DrawText(iNewString.Mid(pos,1), paintRect, baseline, CGraphicsContext::ECenter);
      paintRect.Move(step, 0);
   }
}

/**
 * Note: The below string cannot be set as a _LIT,
 *       since the macro does not do conversion of
 *       UTF-8, and the compiler does not accept the
 *       iso-8859-1 degree sign.
 */
#define TOTAL_STRING "%c %+ 3u°%+02u'%+02u.%+02u\""

void CCoordinateLineDisplay::Format(TInt aVal) 
{
   class TValueParts parts;
   SetCoordinate(aVal, parts);
   
   HBufC* format = WFTextUtil::AllocLC(TOTAL_STRING);

   iNewString.Format(*format, iDirections[parts.iSign + 1], parts.iDeg, 
                     parts.iMin, parts.iSec, parts.iParts);
   CleanupStack::PopAndDestroy(format);
}

TInt CCoordinateLineDisplay::GetVal() const
{
   return iVal;
}

void CCoordinateLineDisplay::SetVal(TInt aVal)
{
   if(aVal != iVal){
      if(abs(aVal) > iMaxVal){
         aVal = 0;
      }
      iLastString = iNewString; //keep old string
      iVal = aVal;
      Format(iVal);
      if (IsVisible()) {
         Window().Invalidate(Rect());
      }
   }
}



CPositionDisplay::CPositionDisplay(class TRgb aBackground, class TRgb aOutline, 
                                   class TRgb aField, class TRgb aText) :
   iBgClr(aBackground), iOutlineClr(aOutline), iFldClr(aField), iTxtClr(aText)                               
{
}

void CPositionDisplay::ConstructL(class CCoeControl& aParent, TInt aHeaderId,
                                  TInt aLatPlusId, TInt aLatMinusId, 
                                  TInt aLonPlusId, TInt aLonMinusId)
{
   SetContainerWindowL(aParent);

   iHeader = new (ELeave) CEikLabel();
   HBufC* hdr = iCoeEnv->AllocReadResourceLC(aHeaderId);
   iHeader->SetTextL(*hdr);
   CleanupStack::PopAndDestroy(hdr);

   const TInt KPIe8 = 314159265;
   const TInt LAT_MAX = KPIe8 / 2;
   const TInt LON_MAX = KPIe8;
   iLatControl = CCoordinateLineDisplay::NewL(*this, aLatPlusId,
                                              aLatMinusId, LAT_MAX,
                                              iFldClr, iTxtClr);
   iLonControl = CCoordinateLineDisplay::NewL(*this, aLonPlusId,
                                              aLonMinusId, LON_MAX,
                                              iFldClr, iTxtClr);
   ActivateL();
}

class CPositionDisplay* 
CPositionDisplay::NewLC(class CCoeControl& aParent, TInt aHeaderId, 
                        TInt aLatPlusId, TInt aLatMinusId, 
                        TInt aLonPlusId, TInt aLonMinusId,
                        class TRgb aBackground, class TRgb aOutline, 
                        class TRgb aField, class TRgb aText)
{
   class CPositionDisplay* self = 
      new (ELeave) CPositionDisplay(aBackground, aOutline, aField, aText);
   CleanupStack::PushL(self);
   self->ConstructL(aParent, aHeaderId, aLatPlusId, aLatMinusId, 
                    aLonPlusId, aLonMinusId);
   return self;
}

class CPositionDisplay* 
CPositionDisplay::NewL(class CCoeControl& aParent, TInt aHeader, 
                       TInt aLatPlusId, TInt aLatMinusId, 
                       TInt aLonPlusId, TInt aLonMinusId,
                       class TRgb aBackground, class TRgb aOutline, 
                       class TRgb aField, class TRgb aText)
{
   class CPositionDisplay* self = 
      CPositionDisplay::NewLC(aParent, aHeader, aLatPlusId, aLatMinusId, 
                              aLonPlusId, aLonMinusId, aBackground, aOutline, 
                              aField, aText);
   CleanupStack::Pop(self);
   return self;
}

inline const TRect BoundingRect(TRect aFirst, const TRect& aSecond)
{
   aFirst.BoundingRect(aSecond);
   return aFirst;
}

inline TBool Contains(class TRect aContainer, class TRect aContainee)
{
   return aContainer.Contains(aContainee.iTl) && 
      aContainer.Contains(aContainee.iBr);
}

void CPositionDisplay::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   gc.SetClippingRect(aRect);
   gc.SetBrushColor(iBgClr);
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);

   //gc.Clear(aRect); //paint background color.

   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenColor(iOutlineClr);
   gc.SetBrushColor(iFldClr);

   if(Contains(iLatControl->Rect(), aRect) || 
      Contains(iLonControl->Rect(), aRect)){
      return;
   }
   TRect bubbles[] = {BoundingRect(iLatControl->Rect(), iLonControl->Rect())};
   const TInt num = sizeof(bubbles)/sizeof(*bubbles);
   for(TInt i = 0; i < num; ++i){
      bubbles[i].Shrink(-2,-2);
      gc.DrawRoundRect(bubbles[i], TSize(8,8));
   }
}

CPositionDisplay::~CPositionDisplay()
{
   delete iHeader;
   delete iLatControl;
   delete iLonControl;
}

TInt CPositionDisplay::CountComponentControls() const
{
   return 3;//TODO: something better here; array, enum
}

class CCoeControl* CPositionDisplay::ComponentControl(TInt aIndex) const
{
   switch(aIndex){ //TODO: something better here; array, enum
   case 0: return iHeader;
   case 1: return iLatControl;
   case 2: return iLonControl;
   }
   return NULL; //TODO: Invariant, panic
}

TInt CPositionDisplay::Indent() const
{
   return iIndent;
}

void CPositionDisplay::SetIndent(TInt aIndent)
{
   if(aIndent != iIndent){
      iIndent = aIndent;
      SizeChanged();
      if (IsVisible()) {
         Window().Invalidate(Rect());
      }
   }
}

void CPositionDisplay::SizeChanged()
{
   TSize headerShrink(7, 2);
   TSize valueShrink(12, 2);
   TInt partHeight = Rect().Height() / 3;
   TSize partSize(Rect().Width(), partHeight);
   TRect rect(Rect().iTl, partSize);

   TRect shrunk = rect;
   shrunk.Shrink( headerShrink);
   iHeader->SetRect(shrunk);

   rect.Move(0,partHeight);
   shrunk = rect;
   shrunk.Shrink( valueShrink);
   iLatControl->SetRect(shrunk);

   rect.Move(0, partHeight - 2);
   shrunk = rect;
   shrunk.Shrink( valueShrink);
   iLonControl->SetRect(shrunk);
}

void CPositionDisplay::MakeVisible(TBool aVisible)
{
   CCoeControl::MakeVisible(aVisible);
   for(TInt i = 0; i < CountComponentControls(); ++i){
      ComponentControl(i)->MakeVisible(aVisible);
   }
}

TInt CPositionDisplay::GetLat() const
{
   return iLatControl->GetVal();
}

TInt CPositionDisplay::GetLon() const
{
   return iLonControl->GetVal();
}

TPoint CPositionDisplay::GetPos() const
{
   return TPoint(GetLat(), GetLon());
}

void CPositionDisplay::GetPos(TInt& aLat, TInt& aLon) const
{
   aLat = GetLat();
   aLon = GetLon();
}

void CPositionDisplay::SetLat(TInt aLat)
{
   iLatControl->SetVal(aLat);
}

void CPositionDisplay::SetLon(TInt aLon)
{
   iLonControl->SetVal(aLon);
}

void CPositionDisplay::SetPos(TInt aLat, TInt aLon)
{
   iLatControl->SetVal(aLat);
   iLonControl->SetVal(aLon);   
}

void CPositionDisplay::SetPos(TPoint aPos)
{
   SetPos(aPos.iX, aPos.iY);
}

