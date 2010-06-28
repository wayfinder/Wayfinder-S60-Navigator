/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAV2_CLIENT_SERIES60_V3

#include <gdi.h>
#include <sacls.h>
#include <eikapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include "arch.h"
#include "WayFinderAppUi.h"
#include "CallObserver.h"

#define ILOG_POINTER (aAppUi->iLog)
#include "memlog.h"

CCallObserver* CCallObserver::NewL(class CWayFinderAppUi* aAppUi )
{
   CCallObserver* self = CCallObserver::NewLC( aAppUi );
   CleanupStack::Pop();
   return self;
}

	
CCallObserver* CCallObserver::NewLC(class CWayFinderAppUi* aAppUi )
{
   CCallObserver* self = new (ELeave) CCallObserver();
   LOGNEW(self, CCallObserver);
   CleanupStack::PushL(self);
   self->ConstructL( aAppUi );
   return self;
}


CCallObserver::CCallObserver()
: CActive(CActive::EPriorityStandard)
{
}


CCallObserver::~CCallObserver()
{
   Cancel();
}


void CCallObserver::ConstructL(class CWayFinderAppUi* aAppUi )
{
   iAppUi = aAppUi;
   CActiveScheduler::Add(this);
   User::LeaveIfError( iSysAgent.Connect() );
}

void CCallObserver::StartL()
{
    iSysAgentEvent.SetRequestStatus(iStatus);
    iSysAgentEvent.SetUid(KUidCurrentCall);
    // Keep the events for 20 seconds
    iSysAgent.SetEventBufferEnabled( true, 20 );
    iSysAgent.NotifyOnEvent(iSysAgentEvent);
    SetActive();
}

void CCallObserver::RunL()
{
   // Check the state of the phone instead of the event.
   switch( iSysAgent.GetState( KUidCurrentCall ) )
   {
   case ESACallNone:
// ESACallDisconnecting does not exist in series 60 v1
#ifndef NAV2_CLIENT_SERIES60_V1
   case ESACallDisconnecting:
#endif
      iAppUi->HandlePhoneCallL( EFalse );
      break;
   case ESACallVoice:
   case ESACallRinging:
   case ESACallAlerting:
   case ESACallFax:
      iAppUi->HandlePhoneCallL( ETrue );
      break;
   case ESACallData:
      break;  
   case ESACallAlternating:
      break;
   }
   StartL();
}

void CCallObserver::DoCancel()
{
   iSysAgent.NotifyEventCancel();
}

#endif
