/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "NetworkRegistrationNotifier.h"
#include "NetworkRegistrationNotifierObserver.h"

#include "machine.h"

CNetworkRegistrationNotifier* 
CNetworkRegistrationNotifier::NewLC(
                                 MNetworkRegistrationNotifierObserver* aObserver,
                                 enum TPriority aPriority)
{
   CNetworkRegistrationNotifier* self = 
      new (ELeave) CNetworkRegistrationNotifier(aObserver, aPriority);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}
CNetworkRegistrationNotifier* 
CNetworkRegistrationNotifier::NewL(
                                 MNetworkRegistrationNotifierObserver* aObserver,
                                 enum TPriority aPriority)
{
   CNetworkRegistrationNotifier* self = 
      CNetworkRegistrationNotifier::NewLC(aObserver, aPriority);
   CleanupStack::Pop(self);
   return self;
}

CNetworkRegistrationNotifier::~CNetworkRegistrationNotifier()
{
   this->Cancel();
   delete iTelephony;
}

void CNetworkRegistrationNotifier::Start()
{
   if(iTelephony && !IsActive()) {
      iTelephony->NotifyChange(iStatus, CTelephony::ENetworkRegistrationStatusChange, iNetworkRegistrationV1Pckg);
      SetActive();
   }
}

CTelephony::TRegistrationStatus 
CNetworkRegistrationNotifier::RegistrationStatus()
{
   return iRegStatus;
}

CNetworkRegistrationNotifier::CNetworkRegistrationNotifier(
                                  MNetworkRegistrationNotifierObserver* aObserver,
                                  enum TPriority aPriority) :
   CActive(aPriority), iObserver(aObserver), 
   iNetworkRegistrationV1Pckg(iNetworkRegistrationV1),
   iRegStatus(CTelephony::ERegistrationUnknown)
{
}

void CNetworkRegistrationNotifier::ConstructL()
{
   CActiveScheduler::Add(this);
   iTelephony = CTelephony::NewL();
}

void CNetworkRegistrationNotifier::RunL()
{
   if(iStatus.Int() == KErrCancel) {
      return;
   }
   if(iStatus.Int() == KErrNone) {
      iRegStatus = iNetworkRegistrationV1.iRegStatus;
      iObserver->NetworkRegistrationStatuschanged();
      iTelephony->NotifyChange(iStatus, CTelephony::ENetworkRegistrationStatusChange, iNetworkRegistrationV1Pckg);
      SetActive();
   }
   else {
      iRegStatus = CTelephony::ERegistrationUnknown;
      iObserver->NetworkRegistrationStatuschanged();
   }
}

void CNetworkRegistrationNotifier::DoCancel()
{
   if (IsActive()) {
      iTelephony->CancelAsync(CTelephony::ENetworkRegistrationStatusChangeCancel);
   }
}
