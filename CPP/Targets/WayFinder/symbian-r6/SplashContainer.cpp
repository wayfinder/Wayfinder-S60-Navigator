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
#include <eikprogi.h>
#include <eiklabel.h>

#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
#include <aknsdrawutils.h>
#endif

#include "GuiDataStore.h"
#include "SplashContainer.h"
#include "IAPSearchGui_old.h"
#include "SplashView.h"
#include "GuidePicture.h"
#include "ImageHandler.h"

#include "RsgInclude.h"
#include "TraceMacros.h"
#include "WFLayoutUtils.h"
#define NEWS_WAIT_CYCLES 5000

// Enumerations
enum TControls
{
   EStartUpPicture,
   ENumberControls,  
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSplashContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSplashContainer::ConstructL( const TRect& aRect,
      CGuidePicture* aStartupImage,
      CSplashView* aView )
{
   iBgColor = aView->GetNewsBgColor();
   //
   // Create and activate the main window
   //
   CreateWindowL();

   iUseImageHandler = EFalse;

   iStartupImage = aStartupImage;
   if (iStartupImage) {
      iStartupImage->SetShow(ETrue);
   }
   InitComponents(aRect, aView);
}

void CSplashContainer::ConstructL( const TRect& /*aRect*/,
      CImageHandler* aImageHandler,
      CSplashView* aView)

{
   iBgColor = aView->GetNewsBgColor();
   //
   // Create and activate the main window
   //
   CreateWindowL();
   
   iUseImageHandler = ETrue;

   iImageHandler = aImageHandler;
   if (iImageHandler) {
      iImageHandler->SetShow(ETrue);
   }
   TRect rect = WFLayoutUtils::GetMainPaneRect();
   rect.iTl = TPoint(0, 0);
   InitComponents(rect, aView);
}   

void CSplashContainer::InitComponents(const TRect& aRect,
                                      CSplashView* aView)
{
   iNewsRect.iTl = TPoint(0,0);
   iNewsRect.iBr = aRect.iBr;
   iNormalTopLeft = aRect.iTl;

   iView = aView;

   SetRect( iNewsRect );
   ActivateL();

   // Create a periodic timer but don't start it yet
	iPeriodicTimer = CPeriodic::NewL(CActive::EPriorityHigh);
   LOGNEW(iPeriodicTimer, CPeriodic);

   /* Timeout each millisecond. */
   SetTimerTick(1000);
   iNewsCount = NEWS_WAIT_CYCLES;   
}

// Destructor
CSplashContainer::~CSplashContainer()
{
   StopTimer();
   delete iPeriodicTimer;

   /* We don't own iStartupImage. */
   iStartupImage = NULL;
}

// ---------------------------------------------------------
// CSplashContainer::SetTimerTick(TTimeIntervalMicroSeconds32  aTick) 
// Set the Tick value (duration between two screen redraw)
// ---------------------------------------------------------
//
void CSplashContainer::SetTimerTick(TTimeIntervalMicroSeconds32  aTick)
{
   iTick=aTick;
}


// ---------------------------------------------------------
// CSplashContainer::StartTimer() 
// Start the timer (required for animated SplashScreen)
// Optional but useful for static ones.
// ---------------------------------------------------------
//
void CSplashContainer::StartTimer()
{
   //If the timer is not already running, start it
   if (!iPeriodicTimer->IsActive()){
      iPeriodicTimer->Start( iTick, iTick,
            TCallBack(CSplashContainer::Period,this) );
   }
}

// ---------------------------------------------------------
// CSplashContainer::StopTimer() 
// 
// ---------------------------------------------------------
//
void CSplashContainer::StopTimer()
{
   //Stop the timer if it is active
   if (iPeriodicTimer->IsActive()){
      iPeriodicTimer->Cancel();
   }
}

// This function is called by the periodic timer
TInt CSplashContainer::Period(TAny * aPtr)
{
   ((CSplashContainer*)aPtr)->DoPeriod();
   //returning a value of TRUE indicates the callback should be done again
   return TRUE;
}

// ---------------------------------------------------------
// CSplashContainer::DoPeriod() 
// Function that is called periodically by the timer.
// Call the child Tick() treatment and request a screen 
// redraw if needed.
// ---------------------------------------------------------
//
void CSplashContainer::DoPeriod()
{
   /* Check which mode we're in. */
   if (iView->DoShowIAPProgress()) {
      StopTimer();      
      iView->CheckIAP();
   } else if (iView->DoShowLatestNewsImage()) {
      /* Show the latest news image. */
      if (iNewsCount == NEWS_WAIT_CYCLES) {
         if (iStartupImage) {
            iStartupImage->SetShow(ETrue);
         }
         DrawDeferred();
      }
      if (iView->DoShowNews() && iNewsCount >= 0 ){
         iNewsCount--;
         return;
      } else {
         iView->SetShownNewsChecksum();
         StopTimer();
         iView->CheckGoToStart();
      }
   } else {
      if (iStartupImage) {
         iStartupImage->SetShow(ETrue);
      }
      /* Don't show anything. */
      StopTimer();
      iView->CheckGoToStart();
   }
   DrawDeferred();
}

void CSplashContainer::AskForIAPDone()
{
   if (iStartupImage) {
      iStartupImage->SetShow(ETrue);
   }
   DrawDeferred();
}

void CSplashContainer::ReportProgress(TInt /*aVal*/, TInt /*aMax*/)
{
   TBool showNews; 
   if (iView->DoShowIAPProgress()) {
      showNews = EFalse;
   } else {
      showNews = ETrue;
   }
   if (iStartupImage) {
      iStartupImage->SetShow(showNews);
   }
   SizeChanged();
   DrawDeferred();
}

// ---------------------------------------------------------
// CSplashContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSplashContainer::SizeChanged()
{
   DrawDeferred();
}

// ---------------------------------------------------------
// CSplashContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSplashContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CSplashContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSplashContainer::ComponentControl( TInt aIndex ) const
{
   switch ( aIndex )
   {
   case EStartUpPicture:
      if (iUseImageHandler) {
         return iImageHandler;
      } else {
         return iStartupImage;
      }
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CStartPageContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSplashContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/ )
{
   // TODO: Add your control event handler code here
}

// ---------------------------------------------------------
// CSplashContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CSplashContainer::Draw(const TRect& /*aRect*/) const
{
   CWindowGc& gc = SystemGc();
   gc.SetBrushColor(iBgColor);
   // Draw White background
   gc.Clear();
}

void CSplashContainer::ShowStatusPane(TBool aShow)
{
   if(aShow){
      iNewsRect.iTl = iNormalTopLeft;
   } else {
      iNewsRect.iTl = TPoint(0,0);
   }
   SetRect( iNewsRect );
   DrawDeferred();
}

void CSplashContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      TRect rect = WFLayoutUtils::GetMainPaneRect();
      rect.iTl = TPoint(0, 0);
      SetRect(rect);
      if (iUseImageHandler && iImageHandler) {
         iImageHandler->HandleLayoutSwitch(aType);
      }
   }   
}
// End of File  
