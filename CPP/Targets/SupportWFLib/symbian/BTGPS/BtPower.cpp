/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAV2_CLIENT_UIQ
#include "BtPower.h"
#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

/** Notifier id for power mode setting s80 and s90. */
static const TInt32 KBTPowerStateNotifierUid = 0x101F808E;
/** Notifier id for power mode setting s60. */
static const TInt32 KPowerModeSettingNotifierUid = 0x100059E2;
#if   defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES80
static const TInt32 KBTPowerNotifier = KBTPowerStateNotifierUid;
#elif defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V2
static const TInt32 KBTPowerNotifier = KPowerModeSettingNotifierUid;
#else
static const TInt32 KBTPowerNotifier = KPowerModeSettingNotifierUid;
#endif

TUid Uid(TInt32 aId)
{
   TUid uid = {aId};
   return uid;
}

CBtPowerChecker::CBtPowerChecker() : 
   CActiveLog(EPriorityStandard, "BtPower")
{
   iPowerCommand() = ETrue;
   iPowerReply() = EFalse;
}

void CBtPowerChecker::ConstructL()
{
   User::LeaveIfError(iNotifier.Connect());
   CActiveScheduler::Add(this);
}

class CBtPowerChecker* CBtPowerChecker::NewL()
{
   class CBtPowerChecker* self = CBtPowerChecker::NewLC();
   CleanupStack::Pop(self);
   return self;
}

class CBtPowerChecker* CBtPowerChecker::NewLC()
{
   class CBtPowerChecker* self = new (ELeave) CBtPowerChecker();
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CBtPowerChecker::~CBtPowerChecker()
{
   if(IsActive()){
      Cancel();
   }
   iNotifier.Close();
}

void CBtPowerChecker::RunL()
{
   DBG("RunL iStatus == %d", iStatus.Int());
   Complete(iStatus);
   iNotifier.CancelNotifier(Uid(KBTPowerNotifier));
}

void CBtPowerChecker::DoCancel()
{
   DBG("DoCancel()");
   Complete(KErrCancel);
   iNotifier.CancelNotifier(Uid(KBTPowerNotifier));
}

void CBtPowerChecker::PowerCheck(class TRequestStatus* aStatus, TBool aOn)
{
   iPowerCommand() = aOn;
   Activate(aStatus);
   iNotifier.StartNotifierAndGetResponse(iStatus, Uid(KBTPowerNotifier),
                                         iPowerCommand, iPowerReply);
   DBG("Active with powercheck though RNotifier.");
   SetActive();
}

void CBtPowerChecker::PowerCheck(class TRequestStatus* aStatus)
{
   PowerCheck(aStatus, ETrue);
}

void CBtPowerChecker::CancelPowerCheck()
{
   DBG("CancelPowerCheck");
   Complete(KErrCancel);
   if(IsActive()){
      Cancel();
   }
}

TBool CBtPowerChecker::IsPoweredOn()
{
   return iPowerReply();
}

#endif
