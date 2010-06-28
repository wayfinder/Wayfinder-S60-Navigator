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

#include "IAPSearchGui_old.h"
#include "GuiDataStore.h"
#include "SplashView.h"
#include "WayFinderAppUi.h"
#include "WayFinderConstants.h"
#include "SettingsData.h"
#include "Dialogs.h"

#include "ConnectionSettings.h"

#include "PathFinder.h"
#include "IniFile.h"
#include "ImageHandler.h"
#include "ProgressDlg.h"
#include "TraceMacros.h"
#include "WFLayoutUtils.h"

enum TSplashMode
{
   EShowNothing,
   EShowIAPProgressBar,
   EShowLatestNewsImage,
};

// ================= MEMBER FUNCTIONS =======================
CSplashView::CSplashView(isab::Log* aLog) : 
   iLog(aLog) 
{}

// ---------------------------------------------------------
// CSplashView::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSplashView::ConstructL( CWayFinderAppUi* aWayFinderUI)
{
   BaseConstructL( R_WAYFINDER_SPLASH_VIEW );
   iMode = EShowNothing;

   iShowNews = ETrue;
   iAskForIAP = ETrue;
   
   iWayFinderUI = aWayFinderUI;
}

class CSplashView* CSplashView::NewLC(CWayFinderAppUi* aWayfinderUI, 
                                      isab::Log* aLog)
{
   CSplashView* self = new (ELeave) CSplashView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aWayfinderUI);
   return self;
}

class CSplashView* CSplashView::NewL(CWayFinderAppUi* aWayfinderUI, 
                                     isab::Log* aLog)
{
   CSplashView* self = CSplashView::NewLC(aWayfinderUI, aLog);
   CleanupStack::Pop(self);
   return self;
}

// Destructor
CSplashView::~CSplashView()
{
}

HBufC*
CSplashView::GetApplicationNameLC()
{
   return iWayFinderUI->GetApplicationNameLC();
}

TRgb CSplashView::GetNewsBgColor()
{
   return iWayFinderUI->GetNewsBgColor();
}

void
CSplashView::SetImage(CGuidePicture* aStartupImage)
{
   iStartupImage = aStartupImage;
}

void
CSplashView::SetImage(CImageHandler* aImageHandler)
{
   iImageHandler = aImageHandler;
}

GuiDataStore*
CSplashView::GetGuiDataStore()
{
   return iWayFinderUI->GetGuiDataStore();
}

void CSplashView::ReportProgress(TInt aVal, TInt aMax, HBufC* aName)
{
   if (DoShowIAPProgress()) {
      TRACE_FUNC();
      if (!iProgressDlg) {
         iProgressDlg = CProgressDlg::NewL();
         iProgressDlg->SetProgressDlgObserver(this);
         iProgressDlg->StartProgressDlgL(R_PROGRESS_DIALOG, 
                                         R_WAYFINDER_IAP_SEARCH1_MSG);
      }
      iProgressDlg->ReportProgress(aVal, aMax, aName);
   } else {
      if (iProgressDlg) {
         delete iProgressDlg;
         iProgressDlg = NULL;
      }
   }
   if ( iContainer ){
      iContainer->ReportProgress(aVal, aMax);
   }
}

void CSplashView::AskForIAP()
{
   iAskForIAP = ETrue;
   if ( iContainer ) {
      iContainer->ReportProgress(0, 100);
   }
}


TBool CSplashView::SearchingForIAP()
{                 
   return iSearchingForIAP;
}                 
void CSplashView::SearchingForIAP(TBool aOn)
{  
   iSearchingForIAP = aOn;
}

void CSplashView::AskForIAPDone()
{
   iAskForIAP = EFalse;
   iSearchingForIAP = EFalse;
   iMode = EShowLatestNewsImage;
   delete iProgressDlg;
   iProgressDlg = NULL;
   if (iContainer) {
      iContainer->ShowStatusPane(EFalse);
      iContainer->AskForIAPDone();
      iContainer->StartTimer();
   }  
   /* Don't do anything. */
   if (iContainer) {
      Cba()->MakeCommandVisible(EAknSoftkeyOk, ETrue);
   }
}

void
CSplashView::CheckIAP()
{
   // TRACE_FUNC();
   if ((iWayFinderUI->GetIAP() < 0) && iContainer && iAskForIAP)  {
      /* Checking for working IAP. */
      iAskForIAP = EFalse;

      /* Show Wayfinder logo. */
      iContainer->ShowStatusPane(EFalse);
      if (!iWayFinderUI->ShowIAPMenu()) {
         iContainer->ShowStatusPane(EFalse);
         iMode = EShowLatestNewsImage;
         iContainer->StartTimer();
      }
   } else if (!iAskForIAP) {
      iSearchingForIAP = EFalse;
      iAskForIAP = EFalse;
      iMode = EShowLatestNewsImage;
      if (iContainer) {
         iContainer->StartTimer();
      }
   }
}

void CSplashView::CheckGoToStart()
{
   iSearchingForIAP = EFalse;
   iMode = EShowLatestNewsImage;
   if (iWayFinderUI->iUrgentShutdown) {
      iWayFinderUI->HandleCommandL( EAknSoftkeyExit );
   } else if (iWayFinderUI->IsGoldVersion() && !iDontTimeout){
      /* Change view to startpage automatically. */
      iWayFinderUI->GotoStartViewL();
   } else if (iDontTimeout) {
      Cba()->MakeCommandVisible(EAknSoftkeyOk, ETrue);
   }
}

TBool CSplashView::DoShowLatestNewsImage()
{
   return (iMode == EShowLatestNewsImage);
}
TBool CSplashView::DoShowIAPProgress()
{
   return (iMode == EShowIAPProgressBar);
}
TBool CSplashView::DoShowNews()
{
   if( !iShowNews &&
       iWayFinderUI->iIniFile->shownNewsChecksum !=
       iWayFinderUI->iIniFile->latestNewsChecksum
       ){
      return ETrue;
   } else {
      return iShowNews;
   }
}

void
CSplashView::SetShownNewsChecksum()
{
   iWayFinderUI->iIniFile->SetShownNewsChecksum();
}

TPtrC16
CSplashView::ProgramPath()
{
   return iWayFinderUI->iPathManager->GetWayfinderPath();
}

void CSplashView::PictureError( TInt /*aError*/ )
{
}

// ---------------------------------------------------------
// TUid CSplashView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CSplashView::Id() const
{
   return KSplashViewId;
}

// ---------------------------------------------------------
// CSplashView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSplashView::HandleCommandL(TInt aCommand)
{
   switch ( aCommand )
   {
   case EAknSoftkeyOk:
      {
         AppUi()->HandleCommandL( EWayFinderSoftkeyStart);
/*          iWayFinderUI->iIniFile->shownNewsChecksum =  */
/*             iWayFinderUI->iIniFile->latestNewsChecksum; */
/*          iWayFinderUI->iIniFile->Save(); */
         iWayFinderUI->iIniFile->SetShownNewsChecksum();
      }
      break;
/*    case EWayFinderSoftKeyUpgrade: */
/*       { */
/*          if ( iWayFinderUI->ShowUpgradeInfoCba()) { */
/*             iShowUpgrade = ETrue; */
/*             AppUi()->HandleCommandL( EWayFinderSoftkeyStart ); */
/*          } */
/*       } */
      break;
   default:
      AppUi()->HandleCommandL( aCommand );
   }
}

// ---------------------------------------------------------
// CSplashView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSplashView::HandleClientRectChange()
{
   if ( iContainer ){
#if defined NAV2_CLIENT_SERIES60_V3
      TRect rect = WFLayoutUtils::GetMainPaneRect();
      rect.iTl = TPoint(0, 0);
      iContainer->SetRect(rect);
#else
      iContainer->SetRect( ClientRect() );
#endif
   }
}

// ---------------------------------------------------------
// CSplashView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CSplashView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CSplashView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CSplashView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
                                  TUid aCustomMessageId,
                                  const TDesC8& /*aCustomMessage*/)
{
   iWayFinderUI->GetNavigationDecorator()->MakeScrollButtonVisible( EFalse );
   /* New policy 040712: Never show activate softkey. */
   // Cba()->MakeCommandVisible(EWayFinderSoftKeyUpgrade, EFalse);
   if( !iWayFinderUI->iUrgentShutdown && aCustomMessageId.iUid != EShutdown ){
      iShowNews = ETrue;
      if(aCustomMessageId.iUid == EShowNews) {
         /* Force news showing, the user wanted it. */
         iDontTimeout = ETrue;
         iMode = EShowLatestNewsImage;
      } else if (iWayFinderUI->GetIAP() < 0) {
         /* We need to look for a working IAP. */
         iMode = EShowIAPProgressBar;
         iDontTimeout = EFalse;
         // Always hide Activate CBAbutton.
      } else {
         /* Working IAP, show news as usual. */
         iMode = EShowLatestNewsImage;
         iDontTimeout = EFalse;
         if (iWayFinderUI->IsGoldVersion() ){
            /* Gold users are not forced to watch the latest news always. */
            iShowNews = EFalse;
         }
      }
   } else {
      /* Urgent shutdown. */
      iWayFinderUI->iUrgentShutdown = ETrue;
      iMode = EShowNothing;
   }
   if(!iContainer){
      /* Restart the container. */
      iContainer = new (ELeave) CSplashContainer(iLog);
      LOGNEW(iContainer, CSplashContainer);
      iContainer->SetMopParent(this);
#if defined NAV2_CLIENT_SERIES60_V3
      TRect rect = WFLayoutUtils::GetMainPaneRect();
      rect.iTl = TPoint(0, 0);
      iContainer->ConstructL( rect, iImageHandler, this );
#else
      iContainer->ConstructL( ClientRect(), iStartupImage, this );
#endif
      AppUi()->AddToStackL( *this, iContainer );
      iContainer->StartTimer();
   }
   Cba()->MakeCommandVisible(EAknSoftkeyOk, ETrue);
}

// ---------------------------------------------------------
// CSplashView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSplashView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }
   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
   if( iShowUpgrade ){
      iWayFinderUI->ShowUpgradeInfoL( CWayFinderAppUi::EUpgrade );
   }
   iShowUpgrade = EFalse;
}

void CSplashView::ProgressDlgAborted()
{
   // XXX could pop up a dialog here that tells
   // the user that he can´t abort the search!
}

// End of File  
