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

#include "BubbleLabel.h"
#include <coemain.h>
#include <w32std.h>
#include <eiklabel.h>
#include "RectTools.h"

CBubbleLabel::CBubbleLabel(TInt aTextLines) : 
   iTextLines(aTextLines)
{
}

void CBubbleLabel::ConstructL(class CCoeControl& aParent, TInt aHeader)
{
   SetContainerWindowL(aParent);

   iHeader = new (ELeave) CEikLabel();
   iHeader->SetContainerWindowL(*this);
   SetHeaderL(aHeader);

   iValue = new (ELeave) CEikLabel();
   iValue->SetContainerWindowL(*this);
   iValue->SetAlignment(EHRightVTop);
   iValue->SetTextL(KNullDesC);

   SetComponentsToInheritVisibility();
   ActivateL();
}

class CBubbleLabel* CBubbleLabel::NewL(class CCoeControl& aParent, 
                                       TInt aHeader, TInt aTextLines)
{
   class CBubbleLabel* self = new (ELeave) CBubbleLabel(aTextLines);
   CleanupStack::PushL(self);
   self->ConstructL(aParent, aHeader);
   CleanupStack::Pop(self);
   return self;
}

CBubbleLabel::~CBubbleLabel()
{
   delete iHeader;
   delete iValue;
}

void CBubbleLabel::SetHeaderL(TInt aResourceId)
{
   HBufC* tmptxt = iCoeEnv->AllocReadResourceLC(aResourceId);
   SetHeaderL(*tmptxt);
   CleanupStack::PopAndDestroy(tmptxt);
}

void CBubbleLabel::SetHeaderL(const TDesC& aText)
{
   iHeader->SetTextL(aText);
   if(IsVisible()){
      Window().Invalidate(iHeader->Rect());
   }
}

void CBubbleLabel::SetTextL(TInt aResourceId)
{
   HBufC* tmptxt = iCoeEnv->AllocReadResourceLC(aResourceId);
   SetTextL(*tmptxt);
   CleanupStack::PopAndDestroy(tmptxt);
}

void CBubbleLabel::SetTextL(const TDesC& aText)
{
   iValue->SetTextL(aText);
   if(IsVisible()){
      Window().Invalidate(iValue->Rect());
   }
}

TInt CBubbleLabel::CountComponentControls() const
{
   return 2;
}

class CCoeControl* CBubbleLabel::ComponentControl(TInt aIndex) const
{
   switch(aIndex){
   case 0: return iHeader;
   case 1: return iValue;
   }
   return NULL;
}

void CBubbleLabel::SizeChanged()
{
   TInt lineHeight = Rect().Height() / (iTextLines + 1);
   TRect headerRect = SetHeight(Rect(), lineHeight);
   iHeader->SetRect(Shrink(headerRect, TSize(4,2)));
   iValue->SetRect(Shrink(SetHeight(FlipDown(headerRect), 
                                    lineHeight * iTextLines), 
                          TSize(8,2)));
}

void CBubbleLabel::Draw(const TRect& aRect) const
{
   class CWindowGc& gc = SystemGc();
   gc.SetClippingRect(aRect);

   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   gc.SetPenColor(KRgbDarkGray);
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.SetBrushColor(KRgbWhite);

   gc.DrawRoundRect(Shrink(iValue->Rect(), TSize(-2,-2)), TSize(8,8));
}

