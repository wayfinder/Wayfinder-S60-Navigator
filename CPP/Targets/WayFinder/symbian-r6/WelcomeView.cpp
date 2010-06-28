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
#include <eikmenup.h>
#include <avkon.hrh>
#include <eikmenub.h>
#include <eikcmbut.h>

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include "wayfinder.hrh"
#include "RsgInclude.h"
#include "WelcomeView.h"
#include "WayFinderAppUi.h"
#include "WayFinderConstants.h"
#include "ImageHandler.h"
#include "WFLayoutUtils.h"

// ================= MEMBER FUNCTIONS =======================
CWelcomeView::CWelcomeView(CWayFinderAppUi* aUi, isab::Log* aLog) : 
   CViewBase(aUi),
   iStartupImage(NULL),
   iLog(aLog)
{
}

// ---------------------------------------------------------
// CWelcomeView::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CWelcomeView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_WELCOME_VIEW );
}

class CWelcomeView* CWelcomeView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CWelcomeView* self = new (ELeave) CWelcomeView(aUi, aLog);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CWelcomeView* CWelcomeView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CWelcomeView *self = CWelcomeView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// Destructor
CWelcomeView::~CWelcomeView()
{
}

TBool
CWelcomeView::SetConStatusImage(CFbsBitmap* bmp, CFbsBitmap* mask)
{
   if (iContainer) {
      iContainer->SetConStatusImage(bmp, mask);
      return ETrue;
   } else {
      return EFalse;
   }
}

void
CWelcomeView::SetImage(CGuidePicture* aStartupImage)
{
   iStartupImage = aStartupImage;
   if (iContainer) {
      iContainer->SetImage(iStartupImage);
      iContainer->DrawNow();
   }
}

void
CWelcomeView::SetImage(CImageHandler* aImageHandler)
{
   iImageHandler = aImageHandler;
   if (iContainer) {
      iContainer->SetImage(iImageHandler);
      iContainer->DrawNow();
   }
}

TRgb CWelcomeView::GetNewsBgColor()
{
   return iWayfinderAppUi->GetNewsBgColor();
}

TPoint CWelcomeView::GetNewsConStatusPos() 
{
   return iWayfinderAppUi->GetNewsConStatusPos();
}

// ---------------------------------------------------------
// TUid CWelcomeView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CWelcomeView::Id() const
{
   return KWelcomeViewId;
}

// ---------------------------------------------------------
// CWelcomeView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CWelcomeView::HandleCommandL(TInt aCommand)
{
   switch (aCommand) {
      case EAknSoftkeyOk:
         {
            AppUi()->HandleCommandL(EWayFinderSoftkeyStart);
         }
         break;
      default:
         AppUi()->HandleCommandL( aCommand );
         break;
   }
}

// ---------------------------------------------------------
// CWelcomeView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CWelcomeView::HandleClientRectChange()
{
   if ( iContainer ){
      //iContainer->SetRect( ClientRect() );
      //iContainer->SetRect( WFLayoutUtils::GetFullScreenRect() );
      if (WFLayoutUtils::LandscapeMode()) {
         iContainer->SetRect( WFLayoutUtils::GetMainPaneRect() );
      } else {
         iContainer->SetRect( WFLayoutUtils::GetFullScreenRect() );
      }
   }
}

// ---------------------------------------------------------
// CWelcomeView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CWelcomeView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CWelcomeView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CWelcomeView::DoActivateL( const TVwsViewId& aPrevViewId,
                                TUid aCustomMessageId,
                                const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);

   if (WFLayoutUtils::LandscapeMode()) {
      Cba()->MakeVisible(ETrue);
   } else {
      Cba()->MakeVisible(EFalse);
      iWayfinderAppUi->GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
   }
   if(iWayfinderAppUi->iUrgentShutdown || aCustomMessageId.iUid == EShutdown ){
      /* Urgent shutdown. */
      iWayfinderAppUi->RealShutdown();
   } else if (aCustomMessageId.iUid == EShowNews) {
      Cba()->MakeVisible(ETrue);
      Cba()->MakeCommandVisible(EAknSoftkeyOk, ETrue);
   }
   if(!iContainer){
      iContainer = new (ELeave) CWelcomeContainer(iLog);
      LOGNEW(iContainer, CWelcomeContainer);
      iContainer->SetMopParent(this); 
#if defined NAV2_CLIENT_SERIES60_V3
      //iContainer->ConstructL( ClientRect(), iImageHandler, this );
      if (WFLayoutUtils::LandscapeMode()) {
         iContainer->ConstructL( WFLayoutUtils::GetMainPaneRect(), iImageHandler, this );
         iContainer->SetRect(WFLayoutUtils::GetMainPaneRect());
      } else {
         iContainer->ConstructL( WFLayoutUtils::GetFullScreenRect(), iImageHandler, this );
         iContainer->SetRect(WFLayoutUtils::GetFullScreenRect());
      }
#else
      iContainer->ConstructL( ClientRect(), iStartupImage, this );
#endif
      AppUi()->AddToStackL( *this, iContainer );
   }
/*    iContainer->StartTimer(); */
   // Always hide the upgrade button. NOTE! Upgrade removed.
   /* Cba()->MakeCommandVisible(EWayFinderSoftKeyUpgrade, EFalse); */

}

// ---------------------------------------------------------
// CWelcomeView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CWelcomeView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }
   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

// void
// CWelcomeView::HandleResourceChange(TInt aType)
// {
// //    CCoeControl::HandleResourceChange(aType);
//    iContainer->HandleResourceChange(aType);
//    if (aType == KEikDynamicLayoutVariantSwitch) {
//    }
// }

// End of File  
