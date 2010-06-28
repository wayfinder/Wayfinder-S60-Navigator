/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "S60WaitDialog.h"
#include <eikenv.h>         //CEikEnv
#include <eikdialg.h>
#include <aknwaitdialog.h>

#include "S60WaitDialogObserver.h"

CWaitDlg::CWaitDlg(class MWaitDialogObserver* aObserver) : 
   iObserver(aObserver),
   iDoStopDialog(EFalse),
   iCancelButtonId(EEikBidCancel)
{
}

CWaitDlg::~CWaitDlg()
{
   //delete iAknWaitDlg;
}

void CWaitDlg::ConstructL()
{
   isRunning = EFalse;
}

CWaitDlg* CWaitDlg::NewL(class MWaitDialogObserver* aObserver)
{
   class CWaitDlg* self = new(ELeave) CWaitDlg(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

void CWaitDlg::StartWaitDlg(TInt aDlgResourceId, TInt  aTextResourceId,
                            TInt aCancelButtonId)
{
   if (!isRunning) {
      iDoStopDialog = EFalse;
      iAknWaitDlg = new(ELeave)CAknWaitDialog(REINTERPRET_CAST(CEikDialog**, 
                                                               &iAknWaitDlg),
                                              EFalse);
      iAknWaitDlg->SetCallback(this);
      iAknWaitDlg->SetTextWrapping(ETrue);
      if(aTextResourceId > 0) {
         HBufC* waitText = CEikonEnv::Static()->AllocReadResourceLC(aTextResourceId);
         iAknWaitDlg->SetTextL(*waitText);
         CleanupStack::PopAndDestroy(waitText);
      }
      if (aCancelButtonId != MAX_INT32) {
         iCancelButtonId = aCancelButtonId;
      }
      iAknWaitDlg->ExecuteLD(aDlgResourceId);
      isRunning = ETrue;
   }
}

void CWaitDlg::DialogDismissedL(TInt aButtonId)
{
   // When in this function we've already "died" so we're not running.
   isRunning = EFalse;

   if (aButtonId == iCancelButtonId) {
      // Cancel was pressed by user so report to observer.
      iObserver->CancelPressed();
   } else if (iDoStopDialog) {
      // The dialog was killed externally so let it die, here do nothing,
      // but make sure we don't end up in else since it was not framework.
   } else {
      // This was not our cancel button that got pressed in the 
      // the dialog (we're here probably because a background/foreground
      // event or switch caused by user or screensaver).
      iObserver->DialogDismissedFromFramework();
   }
}

void CWaitDlg::StopWaitDlg()
{
   if(isRunning) {
      // Flag to know in DialogDismissedL that we want to close the dialog 
      // externally (from caller).
      iDoStopDialog = ETrue; 
      // Stop the dialog.
      iAknWaitDlg->ProcessFinishedL();
      isRunning = EFalse;
   }
}

TBool CWaitDlg::IsRunning() 
{
   return isRunning;	
}
