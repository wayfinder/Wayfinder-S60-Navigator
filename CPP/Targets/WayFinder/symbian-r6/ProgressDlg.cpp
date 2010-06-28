/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ProgressDlg.h"
#include <eikenv.h>
#include <eiklabel.h>
#include <eikprogi.h>
#include <coecntrl.h>
#include "ProgressDlgObserver.h"
#include "TDesCHolder.h"


CProgressDlg::CProgressDlg() 
{
}

void CProgressDlg::ConstructL()
{
   isRunning = EFalse;
}

class CProgressDlg* CProgressDlg::NewL()
{
   class CProgressDlg* self = new (ELeave) CProgressDlg();
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

CProgressDlg::~CProgressDlg()
{
   if (iAknProgressDlg) {
      iAknProgressDlg->ProcessFinishedL();
      isRunning = EFalse;
   }
}

void CProgressDlg::StartProgressDlgL(TInt aDialogResourceId, TInt aResourceId)
{
   if (!isRunning) {
      iAknProgressDlg = 
         new(ELeave)CAknProgressDialog(REINTERPRET_CAST(CEikDialog**, 
                                                        &iAknProgressDlg),
                                       ETrue);
      TInt res;
      iAknProgressDlg->SetCallback(this);
      res = iAknProgressDlg->ExecuteLD(aDialogResourceId);
      iResourceId = aResourceId;
      isRunning = ETrue;
   }
}

void CProgressDlg::ReportProgress(TInt aVal, TInt aMax, HBufC* aIAPName)
{
   if (!iAknProgressDlg) {
      return;
   }
   CEikProgressInfo* bar = iAknProgressDlg->GetProgressInfoL();
   if(bar) {
      if (aVal > aMax) {
         aVal = aMax;
      }
      if (aVal < 0) {
         aVal = 0;
      }
      _LIT(KSlash, "/");
      _LIT(KColon, ":\n");
      _LIT(KEmpty, "-");
      _LIT(KFormatParams, "%S %d%S%d%S%S");
      HBufC* tmpIAPName;
      if(!aIAPName) {
         tmpIAPName = KEmpty().AllocLC();
      } else {
         tmpIAPName = aIAPName->AllocLC();
      }
      HBufC* downloadText = iCoeEnv->AllocReadResourceLC(iResourceId);
      HBufC* downloadStatus = HBufC::NewLC(downloadText->Length() + 
                                           tmpIAPName->Length() + 10);
      TPtr pDownloadStatus = downloadStatus->Des();
      pDownloadStatus.Format(KFormatParams, downloadText, aVal, &KSlash, 
                             aMax, &KColon, tmpIAPName);
      iAknProgressDlg->SetTextL(*downloadStatus);
      CleanupStack::PopAndDestroy(downloadStatus);  
      CleanupStack::PopAndDestroy(downloadText);
      CleanupStack::PopAndDestroy(tmpIAPName);
      bar->SetFinalValue(aMax);
      bar->SetAndDraw(aVal);
   }
}

void CProgressDlg::DialogDismissedL(TInt aButtonId)
{
   // When in this function we've already "died" so we're not running.
   isRunning = EFalse;
   iAknProgressDlg = NULL;

   if(aButtonId == EAknSoftkeyCancel) {
      if (iProgressDlgObserver) {
         iProgressDlgObserver->ProgressDlgAborted();
      }
   }
}

void CProgressDlg::SetProgressDlgObserver(class MProgressDlgObserver* aProgressDlgObserver)
{
   iProgressDlgObserver = aProgressDlgObserver;
}

void CProgressDlg::StopProgressDlg()
{
   if (isRunning) {
      iAknProgressDlg->ProcessFinishedL();
      isRunning = EFalse;
   }
}
