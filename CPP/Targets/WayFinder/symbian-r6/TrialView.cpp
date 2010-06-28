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
#include <avkon.hrh>
#include <eikmenup.h>
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <aknwaitdialog.h>
#include <msvapi.h>

#include <arch.h>
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "TimeOutNotify.h"
/* #include "GuiProt/GuiProtEnums.h" */
#include "WayFinderAppUi.h"
#include "Dialogs.h"
#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"

#include "IAPSearchGui_old.h"
#include "GuiProt/ServerEnums.h"
#include "TrialView.h"
#include "TrialContainer.h" 
#include "ProgressDlg.h"

/* #include "GuiProt/GuiProtMess.h" */
#include "nav2util.h"
#include "memlog.h"
#include "IniFile.h"

using namespace isab;

enum TTrialMode
{
   EShowNothing,
   EShowIAPProgressBar,
   EShowTrialView,
};

// ================= MEMBER FUNCTIONS =======================

CTrialView::CTrialView(class isab::Log* aLog) : 
   iMode(EShowNothing),
   iLog(aLog),
   iAskForIAP(ETrue)
{
}

// ---------------------------------------------------------
// CTrialView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void
CTrialView::ConstructL(class CWayFinderAppUi* aWayFinderUI )
{
   BaseConstructL( R_WAYFINDER_TRIAL_VIEW );
   iWayfinderAppUi = aWayFinderUI;
}

class CTrialView*
CTrialView::NewLC(class CWayFinderAppUi* aUi, class isab::Log* aLog)
{
   class CTrialView* self = new (ELeave) CTrialView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aUi);
   return self;
}

class CTrialView*
CTrialView::NewL(class CWayFinderAppUi* aUi, class isab::Log* aLog)
{
   class CTrialView *self = CTrialView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CTrialView::~CTrialView()
// ?implementation_description
// ---------------------------------------------------------
//
CTrialView::~CTrialView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;

   LOGDEL(iTrialMessage);
   delete iTrialMessage;
}

void CTrialView::SetReRegisterIsUpgrade(TBool aReRegisterIsUpgrade)
{
   iReRegisterIsUpgrade = aReRegisterIsUpgrade;
}

void
CTrialView::ReportProgress(TInt aVal, TInt aMax, HBufC* aName)
{
   if (DoShowIAPProgress()) {
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

void CTrialView::AskForIAP()
{
   iAskForIAP = ETrue;
   if ( iContainer ) {
      iContainer->ReportProgress(0, 100);
   }
}

TBool
CTrialView::SearchingForIAP()
{
   return iSearchingForIAP;
}

void
CTrialView::SearchingForIAP(TBool aOn)
{
   iSearchingForIAP = aOn;
}

void
CTrialView::AskForIAPDone()
{
   iAskForIAP = EFalse;
   iSearchingForIAP = EFalse;
   iMode = EShowTrialView;
   delete iProgressDlg;
   iProgressDlg = NULL;
   if (DoShowTrialView()) {
      if (iContainer) {
         Cba()->MakeCommandVisible(EAknSoftkeyOptions, ETrue);
         Cba()->MakeCommandVisible(EAknSoftkeyExit, ETrue);
         iContainer->AskForIAPDone();
      }
   } else {
      StartTrial();
      iMode = EShowNothing;
   }
}

void
CTrialView::CheckGoToStart()
{
   iMode = EShowTrialView;
   iSearchingForIAP = EFalse;
   iAskForIAP = EFalse;
   if (!DoShowTrialView()) {
      StartTrial();
   }
}

TBool
CTrialView::ShowTrialEntryInTrialView()
{
   return iWayfinderAppUi->ShowTrialEntryInTrialView();
}

void
CTrialView::CheckIAP()
{
   if ((iWayfinderAppUi->GetIAP() < 0) && iContainer && iAskForIAP)  {
      /* Checking for working IAP. */
      iAskForIAP = EFalse;

      /* Show Wayfinder logo. */
      if (!iWayfinderAppUi->ShowIAPMenu()) {
         iSearchingForIAP = EFalse;
         iMode = EShowTrialView;
      }
   } else if (iWayfinderAppUi->GetIAP() > -1 && iContainer) {
      iSearchingForIAP = EFalse;
      iAskForIAP = EFalse;
      iMode = EShowTrialView;
   }
}

TBool
CTrialView::DoShowTrialView()
{
   if (iMode == EShowTrialView && iWayfinderAppUi->ShowTrialView()) {
      return ETrue;
   }
   return EFalse;
}
TBool
CTrialView::DoShowIAPProgress()
{
   if (iMode == EShowIAPProgressBar) {
      return ETrue;
   }
   return EFalse;
}

void
CTrialView::ReRegisterComplete(isab::GuiProtEnums::WayfinderType type,
                               TBool verboseErrors)
{
   iInProgress = EFalse;
   if(type != GuiProtEnums::InvalidWayfinderType){
      //all types except invalid will set the type in appui. 
      iWayfinderAppUi->iIniFile->setWayfinderType(type);
   }
   ShowReRegisterResult(type, verboseErrors); //show result
   switch(type){
   case GuiProtEnums::Trial:
      break;
   case GuiProtEnums::Silver:
      /* Regard as already registered. */
      iWayfinderAppUi->iIniFile->sendtSilverSms = 1;
      iWayfinderAppUi->ForceGotoSplashView();
      break;
   case GuiProtEnums::Gold:
      /* Regard as already registered. */
      iWayfinderAppUi->iIniFile->sendtSilverSms = 1;
      iWayfinderAppUi->ForceGotoSplashView();
      break;
   case GuiProtEnums::Iron:
      iWayfinderAppUi->ForceGotoSplashView();
      break;
   case GuiProtEnums::InvalidWayfinderType: //error
      break;
   }
}

void
CTrialView::ReRegisterFailed()
{
   iInProgress=EFalse;
   //if(iWaitNote){
   //   iWaitNote->ProcessFinishedL();
   //}
}

void
CTrialView::ShowReRegisterResult(enum GuiProtEnums::WayfinderType aType,
                                 TBool verboseError)
{
   TInt header = R_STARTMENU_TRIAL_HEADER;
   TInt text   = R_STARTMENU_TRIAL_TEXT;
   switch(aType){
   case GuiProtEnums::Trial:
      //already correct
      if(!verboseError){
         //don't show dialog
         header = text = 0;
      }
      break;
   case GuiProtEnums::Silver:
      header = R_STARTMENU_SILVER_HEADER;
      text   = R_STARTMENU_SILVER_TEXT;
      break;
   case GuiProtEnums::Gold:
      header = R_STARTMENU_GOLD_HEADER;
      text   = R_STARTMENU_GOLD_TEXT;
      break;
   case GuiProtEnums::Iron:
      header = R_STARTMENU_IRON_HEADER;
      text   = R_STARTMENU_IRON_TEXT;
      break;
   case GuiProtEnums::InvalidWayfinderType:
      header = text = 0;
   }
   if(header != 0){
      WFDialog::ShowScrollingDialogL(header, text, EFalse);
   }
}

void
CTrialView::HandleExpireVectorL(const TInt32* data, TInt32 numEntries)
{
   TInt formatResource = 0;
   TInt32 num = 0;
   if((GuiProtEnums::transactionsLeft < numEntries) && 
      (data[GuiProtEnums::transactionsLeft] != MAX_INT32)){
      formatResource = R_FORMAT_TRIAL_TRANSACTIONSLEFT;
      num = data[GuiProtEnums::transactionsLeft];
   } else if((GuiProtEnums::transactionDaysLeft < numEntries) &&
             (data[GuiProtEnums::transactionDaysLeft] != MAX_INT32)){
      formatResource = R_FORMAT_TRIAL_TRANSACTIONDAYSLEFT;
      num = data[GuiProtEnums::transactionDaysLeft];
   } else if((GuiProtEnums::expireDay < numEntries) &&
             (data[GuiProtEnums::expireDay] != MAX_INT32)){
      formatResource = R_FORMAT_TRIAL_DAYSLEFT;
      num = data[GuiProtEnums::expireDay];
   }
   if(formatResource != 0){
      HBufC* format = iCoeEnv->AllocReadResourceLC(formatResource);
      if(!iTrialMessage){
         iTrialMessage = HBufC::NewL(64);
      }
      iTrialMessage->Des().Format(*format, num);
      CleanupStack::PopAndDestroy(format);

//       TBuf<KBuf64Length> buf;
//       iCoeEnv->ReadResource( buf, formatResource );
//       iTrialMessage = new (ELeave) TBuf<KBuf64Length>(64);
//       iTrialMessage->Format(TRefByValue<const TDesC16>(buf), num);
   }
   if(iTrialMessage && iContainer){
      iContainer->SetTrialMessageL(*iTrialMessage);
   }
}


void CTrialView::StartTrial()
{
   if (!iSearchingForIAP) {
// #ifndef NAV2_CLIENT_SERIES60_V3
//       iWayfinderAppUi->iIniFile->doOnce(0, &IniFile::sendtSilverSms, 
//          MemFunPtr<CWayFinderAppUi, TBool>(iWayfinderAppUi, &CWayFinderAppUi::CheckSilverSMSQueryL));
// #endif
      iWayfinderAppUi->ForceGotoSplashView();
   }
}

// ---------------------------------------------------------
// TUid CTrialView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid
CTrialView::Id() const
{
   return KTrialViewId;
}

// ---------------------------------------------------------
// CTrialView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void
CTrialView::HandleCommandL(TInt aCommand)
{
   if(iReRegisterIsUpgrade && aCommand == EWayFinderCmdTrialReRegister){
      aCommand = EWayFinderCmdTrialUpgrade;
   }
   switch ( aCommand ){
   case EAknSoftkeyOk:
      iEikonEnv->InfoMsg( _L("view1 ok") );
      break;
   case EAknSoftkeyExit:
      if (!iSearchingForIAP) {
         AppUi()->HandleCommandL(EAknSoftkeyExit);
      }
      break;
   case EWayFinderCmdTrialOpen:
      if (!iSearchingForIAP) {
         iContainer->GoToSelection();
      }
      break;
   case EWayFinderCmdTrialRegister:
      if (!iSearchingForIAP) {
         if (iInProgress) {
            WFDialog::ShowScrollingErrorDialogL( R_CONNECT_DIALOG_BUSY );
            return;
         }
         iWayfinderAppUi->ShowUpgradeInfoL( CWayFinderAppUi::EActivate );
      }
      break;
   case EWayFinderCmdTrialReRegister:
      if (!iSearchingForIAP) {
         if (iInProgress) {
            WFDialog::ShowScrollingErrorDialogL( R_CONNECT_DIALOG_BUSY );
            return;
         }
         
         iWayfinderAppUi->SendSyncParameters();
         
         //iWaitNote = new(ELeave)CAknWaitDialog(reinterpret_cast<CEikDialog**>(&iWaitNote));
         //iWaitNote->SetTone(CAknNoteDialog::EConfirmationTone);
         iInProgress=ETrue;
         //CAknWaitNoteWrapper inherites privately from CBase, so
         //it can't be pushed onto the cleanupstack. How do we
         //insure that it's properly destroyed?
         //iWaitNote->ExecuteLD(R_STARTMENU_WAITNOTE);
         //iWaitNote = NULL;
      }
      break;
   case EWayFinderCmdTrialUpgrade:
      if(!iSearchingForIAP){
         if (iInProgress) {
            WFDialog::ShowScrollingErrorDialogL( R_CONNECT_DIALOG_BUSY );
            return;
         }
         iWayfinderAppUi->ShowUpgradeInfoL( CWayFinderAppUi::EUpgrade );
      } 
      break;
   case EWayFinderCmdTrialTrial:
      StartTrial();
      break;
   default:
      AppUi()->HandleCommandL( aCommand );
      break;
   }
}

// ---------------------------------------------------------
// CTrialView::HandleClientRectChange()
// ---------------------------------------------------------
//
void
CTrialView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CTrialView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void
CTrialView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CTrialView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void
CTrialView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
                         TUid /*aCustomMessageId*/,
                         const TDesC8& /*aCustomMessage*/)
{
   if( !iWayfinderAppUi->iUrgentShutdown ){
      if (iWayfinderAppUi->GetIAP() < 0) {
         /* We need to look for a working IAP. */
         iMode = EShowIAPProgressBar;
      } else {
         /* Working IAP, show Trial menu as usual. */
         iMode = EShowTrialView;
      }
   } else {
      /* Urgent shutdown. */
      iMode = EShowNothing;
   }
   if (!iContainer){
      iContainer = new (ELeave) CTrialContainer(iLog);
      LOGNEW(iContainer, CTrialContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect(), this );
      if(iTrialMessage){
         iContainer->SetTrialMessageL(*iTrialMessage);
      }
      AppUi()->AddToStackL( *this, iContainer );
      iContainer->StartTimer();
   }
}

// ---------------------------------------------------------
// CTrialView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void
CTrialView::DoDeactivate()
{
   iInProgress=EFalse;
   //if(iWaitNote){
   //   iWaitNote->ProcessFinishedL();
   //}
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

void CTrialView::ProgressDlgAborted()
{
   // XXX could pop up a dialog here that tells
   // the user that he can´t abort the search!
}

// End of File

