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

#ifdef SYMBIAN_7S //this class is usable in Symbian 7.0s and later.

#include "SimpleConnectionManager.h"
#if defined __WINS__ || defined __WINSCW__
# define NO_MANAGE
#else
# undef NO_MANAGE
#endif

#ifdef SYMBIAN_AVKON
# include <apengineconsts.h>
#endif

CSimpleConnectionManager::CSimpleConnectionManager(TInt32 aIAP) : 
   CActive(EPriorityStandard), iIAP(aIAP)
{
}

void CSimpleConnectionManager::ConstructL()
{
   CActiveScheduler::Add(this);
   StartL();
}

TInt32 CSimpleConnectionManager::Iap() const
{
   return iIAP;
}


class CSimpleConnectionManager* CSimpleConnectionManager::NewL(TInt32 aIAP)
{
   class CSimpleConnectionManager* self = 
      new (ELeave) CSimpleConnectionManager(aIAP);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

CSimpleConnectionManager::~CSimpleConnectionManager()
{
   Cancel();
   if(iState != EIdle){
      iManager.Close();
      iSession.Close();
   }
}

TInt CSimpleConnectionManager::RunError(TInt /*aError*/)
{
   return KErrNone;
}

void CSimpleConnectionManager::RunL()
{
   switch(iState){
   case EIdle: break; //TODO
   case EConnecting:
      if(iStatus == KErrNone){
         iState = EConnected;
         //WhyDoWeDoThisL();
      } else {
         User::Leave(iStatus.Int());
      }
      break;
   case EConnected://never supposed to happen
      break;
   }
}

void CSimpleConnectionManager::DoCancel()
{
}


void CSimpleConnectionManager::StartL()
{
   User::LeaveIfError(iSession.Connect());
   CleanupClosePushL(iSession);
   User::LeaveIfError(iManager.Open(iSession));
   CleanupStack::Pop(&iSession);
   iState = EConnecting;

#ifndef NO_MANAGE
   class TCommDbConnPref iPrefs = TCommDbConnPref();
   if(iIAP >= 0){
      iPrefs.SetIapId(iIAP);
      iPrefs.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
      iManager.Start(iPrefs, iStatus);
   } else if(iIAP == -2){
      iManager.Start(iStatus);
   } else {
      iPrefs.SetDialogPreference(ECommDbDialogPrefPrompt);
      iPrefs.SetDirection(ECommDbConnectionDirectionOutgoing);
#ifdef SYMBIAN_CKON
      iPrefs.SetBearerSet(ECommDbBearerCSD | ECommDbBearerWcdma |
                          ECommDbBearerCdma2000 | ECommDbBearerPSD);
#elif defined SYMBIAN_AVKON
      iPrefs.SetBearerSet( EApBearerTypeGPRS | EApBearerTypeHSCSD );
#elif defined SYMBIAN_QKON
      iPrefs.SetBearerSet( ECommDbBearerCSD | ECommDbBearerWcdma |
                           ECommDbBearerCdma2000 | ECommDbBearerPSD);
#else
# error Set some bearerprefs here
#endif
      iManager.Start(iPrefs, iStatus);
   }
   SetActive();
#endif
   return;
}

#endif
