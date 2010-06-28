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
#include <aknviewappui.h>
#include <aknutils.h>

#include <eiklabel.h>

#include "WelcomeContainer.h"
#include "WelcomeView.h"
#include "RsgInclude.h"
#include "GuidePicture.h"
#include "ImageHandler.h"
#include "WFLayoutUtils.h"
#include "BitmapControl.h"

#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
#include <aknsdrawutils.h>
#endif

#define MAX_WIDTH                176
#define WAIT_LABEL_POS_Y         35

// Enumarations
enum TControls
{
   EStartUpPicture,
   EWaitImage,
   ENumberControls
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CWelcomeContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CWelcomeContainer::ConstructL( const TRect& aRect,
      CGuidePicture* aStartupImage,
      CWelcomeView* aView )
{
   iUseImageHandler = EFalse;

   iBgColor = aView->GetNewsBgColor();
   iConStatusPos = aView->GetNewsConStatusPos();

   iStartupImage = aStartupImage; 
   if (iStartupImage) {
      iStartupImage->SetShow(ETrue);
   }

   SetRect( aRect );

   iView = aView;

   //
   // Create and activate the main window
   CreateWindowL();
   ActivateL();
}

void CWelcomeContainer::ConstructL( const TRect& aRect,
      CImageHandler* aImageHandler,
      CWelcomeView* aView )
{
   iUseImageHandler = ETrue;

   iBgColor = aView->GetNewsBgColor();
   iConStatusPos = aView->GetNewsConStatusPos();

   iImageHandler = aImageHandler;
   if (iImageHandler) {
      // Need to set the parent of the image hander, if not
      // the framework calls to ComponentControls will generate
      // an access violation (Kern-Exec 3) in CImageHandler::Draw()
      // after a number of calls (this goes for s60v3 fp2 phones).
      iImageHandler->SetParent(this);
      iImageHandler->SetShow(ETrue);
   }

   SetRect( aRect );

   iView = aView;

   //
   // Create and activate the main window
   CreateWindowL();
   ActivateL();
}

// Destructor
CWelcomeContainer::~CWelcomeContainer()
{
}

void
CWelcomeContainer::SetConStatusImage(CFbsBitmap* bmp, CFbsBitmap* mask)
{
   TPoint pos = WFLayoutUtils::CalculatePosUsingMainPane(iConStatusPos.iX, 
                                                         iConStatusPos.iY);

   if (bmp) {
      if (!iWaitImage) {
         iWaitImage = CBitmapControl::NewL(this, pos, NULL, NULL);
      }
      iWaitImage->SetBitmap(pos, bmp, mask);
   } else {
      if (iWaitImage) {
         /* Remove image. Need to get rect, since setting the bmp and mask */
         /* to null will not trigger an Invalidate. */
         TRect rect = iWaitImage->Rect();
         iWaitImage->SetBitmap(pos, bmp, mask);
         Window().Invalidate(rect);
      }
   }
}


void
CWelcomeContainer::SetImage(CGuidePicture* aStartupImage)
{
   iUseImageHandler = EFalse;
   iStartupImage = aStartupImage;
   if (iStartupImage) {
      iStartupImage->SetShow(ETrue);
   }
}

void
CWelcomeContainer::SetImage(class CImageHandler* aImageHandler)
{
   iUseImageHandler = ETrue;
   iImageHandler = aImageHandler;
   if (iImageHandler) {
      iImageHandler->SetShow(ETrue);
   }
}

// ---------------------------------------------------------
// CWelcomeContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CWelcomeContainer::SizeChanged()
{
}

// ---------------------------------------------------------
// CWelcomeContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CWelcomeContainer::CountComponentControls() const
{
   if (iWaitImage) {
      return ENumberControls;
   } else {
      return ENumberControls-1;
   }
}

// ---------------------------------------------------------
// CWelcomeContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CWelcomeContainer::ComponentControl( TInt aIndex ) const
{
   switch ( aIndex )
   {
      case EStartUpPicture:
         if (iUseImageHandler) {
            return iImageHandler;
         } else {
            return iStartupImage;
         }
         break;
      case EWaitImage:
         return iWaitImage;
         break;
      default:
         return NULL;
   }
}

// ---------------------------------------------------------
// CStartPageContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CWelcomeContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/ )
{
   // TODO: Add your control event handler code here
}

// ---------------------------------------------------------
// CWelcomeContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
void CWelcomeContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   gc.SetPenColor(iBgColor);
   //gc.SetPenColor(TRgb(255,0,0));
   //gc.SetPenSize(TSize(5,5));
   gc.SetBrushColor(iBgColor);
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   TRect rect_m;

   rect_m = Rect();

   gc.DrawRect(rect_m);
}

void
CWelcomeContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      TRect rect;
      if (WFLayoutUtils::LandscapeMode()) {
         rect = WFLayoutUtils::GetMainPaneRect();
      } else {
         rect = WFLayoutUtils::GetFullScreenRect();
      }
      SetRect(rect);
      if (iUseImageHandler && iImageHandler) {
         iImageHandler->HandleLayoutSwitch(aType);
      }
   }   
}

// End of File  
