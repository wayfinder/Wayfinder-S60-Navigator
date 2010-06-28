/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "PedestrianPositionControl.h"
#include <gdi.h>
#include <aknsutils.h> 
#include <gulcolor.h> 
#include <aknutils.h> 
#include <w32std.h>

CPedestrianPositionControl* 
CPedestrianPositionControl::NewLC(CCoeControl* aParent,
                                  const TRect& aRect,
                                  const TSize& aCenterSize,
                                  const TRgb& aColor,
                                  const TRgb& aBgColor)
{
   CPedestrianPositionControl* self = new (ELeave) 
      CPedestrianPositionControl(aColor, aBgColor);
   CleanupStack::PushL(self);
   self->ConstructL(aParent, aRect, aCenterSize);
   return self;
}

CPedestrianPositionControl* 
CPedestrianPositionControl::NewL(CCoeControl* aParent,
                                 const TRect& aRect,
                                 const TSize& aCenterSize,
                                 const TRgb& aColor,
                                 const TRgb& aBgColor)  
{
   CPedestrianPositionControl* self = 
      CPedestrianPositionControl::NewLC(aParent, aRect, aCenterSize,
                                        aColor, aBgColor);
   CleanupStack::Pop(self);
   return self;
}

void CPedestrianPositionControl::ConstructL(CCoeControl* aParent,
                                            const TRect& aRect,
                                            const TSize& aCenterSize)
{   
   if (!aParent) {
      // Create a window for this control
      CreateWindowL();
   } else {
      // We got an valid parent, we should not be window owning
      SetContainerWindowL(*aParent);      
   }

   SetRect(aRect);
   //iCenterRect = aRect;
   //iCenterRect.Shrink(aRect.Height()/3, aRect.Width()/3);
   iCenterRect = TRect(aRect.Center(), aCenterSize);
   iCenterRect.Move(-(iCenterRect.Width() >>1 ), 
                    -(iCenterRect.Height() >> 1));

   if (!aParent) {
      // Activate the window, which makes it ready to be drawn
      ActivateL();
   }
}

CPedestrianPositionControl::CPedestrianPositionControl(const TRgb& aColor, 
                                                       const TRgb& aBgColor) :
   iColor(TRgb(aColor.Internal(), aColor.Alpha())),
   iBgColor(TRgb(aBgColor.Internal(), aBgColor.Alpha())),
   iLineSize(TSize(1, 1))
{
}

CPedestrianPositionControl::~CPedestrianPositionControl()
{
}

void CPedestrianPositionControl::PositionChanged()
{
   // Get the complete rect of this control
   TRect rect = Rect();
   TPoint dMove((rect.Width() >> 1) - (iCenterRect.Width() >> 1),
                (rect.Height() >> 1) - (iCenterRect.Height() >> 1));
   iCenterRect.SetRect(TPoint(rect.iTl + dMove), iCenterRect.Size());
}

void CPedestrianPositionControl::SizeChanged()
{
   // Get the complete rect of this control
   TRect rect = Rect();
}

void CPedestrianPositionControl::SetCenter(TPoint aCenter)
{
   TRect rect = Rect();
   TPoint dMove((rect.Width() >> 1), (rect.Height() >> 1));
   this->SetPosition(aCenter - dMove);
}

void CPedestrianPositionControl::Draw(const TRect& /*aRect*/) const
{
   // Get the standard graphics context
   CWindowGc& gc = SystemGc();
   TRect rect(Rect());
   gc.SetClippingRect(rect);

   gc.SetPenColor(iColor);
   gc.SetPenSize(iLineSize);

   gc.SetBrushColor(iBgColor);
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);

   gc.DrawEllipse(rect);

   //rect.Shrink(5, 5);
   //gc.SetBrushStyle(CGraphicsContext::ENullBrush);
   //gc.DrawEllipse(rect);

   gc.SetBrushColor(iColor);
   //gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawEllipse(iCenterRect);
}
