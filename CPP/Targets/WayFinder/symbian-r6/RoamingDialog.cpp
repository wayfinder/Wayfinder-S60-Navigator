/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <wayfinder8_s60v5.rsg>
#include "RoamingDialog.h"

// CTimerCallback
class MRoamingCallback;
class CAknStaticNoteDialog;
CTimerCallback* CTimerCallback::NewL(MTimerCallback& aCallback, TInt aTimeout)
{
  CTimerCallback* self = new (ELeave) CTimerCallback(aCallback);
  CleanupStack::PushL(self);
  self->ConstructL(aTimeout);
  CleanupStack::Pop(self);
  return self;
}

CTimerCallback::CTimerCallback(MTimerCallback& aCallback)
: CTimer(CActive::EPriorityStandard)
, iCallback(aCallback)
{
  CActiveScheduler::Add(this);
}

void CTimerCallback::ConstructL(TInt aTimeout)
{
  CTimer::ConstructL();
  TTimeIntervalMicroSeconds32 KTimeout(aTimeout);
  After(KTimeout);
}

void CTimerCallback::RunL()
{
  iCallback.HandleTimedOut();
}

// CRouteDialog

CRoamingDialog* CRoamingDialog::NewL(TDes& aDataText, MRoamingCallback* aCallback)
{
  CRoamingDialog* self = new (ELeave) CRoamingDialog(aDataText, aCallback);
  return self;
}

CRoamingDialog::~CRoamingDialog()
{
  delete iCallBack;
  iCallBack = NULL;
}

void CRoamingDialog::HandleTimedOut()
{
  iRoamingCallback->HandleRoamingDialogDone(MRoamingCallback::ETimeout);
}

TBool CRoamingDialog::OkToExitL(TInt aButtonId)
{
  if (aButtonId == EAknSoftkeyYes)
    {
      iRoamingCallback->HandleRoamingDialogDone(MRoamingCallback::EAccept);
    }
  else
    {
      iRoamingCallback->HandleRoamingDialogDone(MRoamingCallback::EExit);
    }
  return ETrue;
}

void CRoamingDialog::PostLayoutDynInitL()
{
  if (iTimeout != 0)
    {
      iCallBack = CTimerCallback::NewL(*this, iTimeout);
    }
}

void CRoamingDialog::SetTone(TTone aTone)
{
  iTone = aTone;
}

void CRoamingDialog::SetTimeout(TTimeout aTimeout)
{
  iTimeout = aTimeout;
}

void CRoamingDialog::ExitDialogL()
{
  TryExitL(EAknSoftkeyCancel);
}

CRoamingDialog::CRoamingDialog(TDes& aDataText, MRoamingCallback* aCallback)
: CAknStaticNoteDialog()
, iRoamingCallback(aCallback)
{
  PrepareLC(R_WAYFINDER_ROAMING_DIALOG);
  SetNumberOfBorders(1);
  SetTextL(aDataText);
}

void CRoamingDialog::Start()
{
  RunLD();
}
