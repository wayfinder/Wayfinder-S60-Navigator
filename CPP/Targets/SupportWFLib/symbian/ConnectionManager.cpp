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

#include "arch.h"

#include "IAPConnMgrHandler.h"
#include "ConnectionSettings.h"

#include "IAPConnMgrObserver.h"
#include "ConnectionManager.h"

CConnectionManager::CConnectionManager() : CActive(EPriorityStandard) 
{}

#ifdef USE_RCONNECTION
# ifdef NAV2_CLIENT_SERIES60_V2
#  include <apengineconsts.h>
# endif
# include <commdbconnpref.h>
#elif defined USE_RAGENT
# include <agentclient.h>
#endif

void
CConnectionManager::ConstructL()
{
#ifdef USE_RAGENT
   iAgent = new (ELeave) RGenericAgent();
   iAgent->Open();
#elif defined(USE_RCONNECTION)
   iConnection = new (ELeave) RConnection();
   iSocketServ = new (ELeave) RSocketServ();
#endif
   CActiveScheduler::Add(this);
}

CConnectionManager *
CConnectionManager::NewLC()
{
   class CConnectionManager* self = new (ELeave) CConnectionManager();
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}
class CConnectionManager *
CConnectionManager::NewL()
{
   class CConnectionManager* self = CConnectionManager::NewLC();
   CleanupStack::Pop(self);
   return self;
}

CConnectionManager::~CConnectionManager()
{
#ifdef USE_RCONNECTION
   delete iConnection;
   delete iSocketServ;
#elif defined USE_RAGENT
   if (iAgent && iSettings){
      StopConnectionManager();
   }
   delete iAgent;
   delete iSettings;
#endif
}

void
CConnectionManager::SetIAPConnMgrObserver(class MIAPConnMgrObserver* aObserver)
{
   iObserver = aObserver;
}

void 
CConnectionManager::SetIAP(const class CConnectionSettings& aSetting)
{
   if (iConnectionSetting) {
      delete iConnectionSetting;
   }
   iConnectionSetting = CConnectionSettings::NewL(aSetting);
}

void 
CConnectionManager::Test()
{
/*    if (iObserver) { */
/*       iObserver->ConnMgrSuccess(); */
/*    } */
   StartConnectionManager(iConnectionSetting->GetIAP());
}

void
CConnectionManager::StartConnectionManager(int32 aIAP)
{
   iIAP = aIAP;
   StopConnectionManager();
#ifdef USE_RAGENT
   OpenLinkLayerAgentL();
#elif defined USE_RCONNECTION
# ifdef __WINS__
   if (iObserver) {
      iObserver->ConnMgrSuccess();
   }
# else
   OpenLinkLayerConnectionL();
# endif
#endif
}

void
CConnectionManager::OpenLinkLayerConnectionL()
{
#ifdef USE_RCONNECTION
   User::LeaveIfError(iSocketServ->Connect());
   User::LeaveIfError(iConnection->Open(*iSocketServ));
   TCommDbConnPref prefs = TCommDbConnPref();

   if (iIAP >= 0) {
      prefs.SetIapId(iIAP);
      prefs.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
      iConnection->Start(prefs, iStatus);
   } else if (iIAP == -2) {
      // Debug mode - use system default
      iConnection->Start(iStatus);
   } else {
      // Use Always ask
      prefs.SetDialogPreference(ECommDbDialogPrefPrompt);
      prefs.SetDirection(ECommDbConnectionDirectionOutgoing);
# ifdef NAV2_CLIENT_SERIES60_V2
      prefs.SetBearerSet( EApBearerTypeGPRS | EApBearerTypeHSCSD );
# else //s80, s90
      prefs.SetBearerSet( ECommDbBearerCSD | ECommDbBearerWcdma |
            ECommDbBearerCdma2000 | ECommDbBearerPSD );

# endif
      iConnection->Start(prefs, iStatus);
   }
   SetActive();

#endif
}
void
CConnectionManager::OpenLinkLayerAgentL()
{
#ifdef USE_RAGENT
#ifdef NAV2_CLIENT_SERIES60_V1
   iSettings = CStoreableOverrideSettings::NewL(
         CStoreableOverrideSettings::EParamListPartial,
         EDatabaseTypeIAP);
#else
   //if(iAgent) {

      delete iSettings;
      iSettings = NULL;
      iSettings = CStoreableOverrideSettings::NewL(
                     CStoreableOverrideSettings::EParamListPartial);

      delete iAgent;
      iAgent = NULL;
      iAgent = new (ELeave) RGenericAgent();
      //}

#endif
   class CCommsDbConnectionPrefTableView::TCommDbIapConnectionPref prefs;

   prefs.iBearer.iIapId = iIAP;
   prefs.iRanking = 1;
   prefs.iDirection = ECommDbConnectionDirectionOutgoing;
   prefs.iDialogPref = ECommDbDialogPrefDoNotPrompt;
   prefs.iBearer.iBearerSet = ECommDbBearerGPRS; 
   
   switch(iSettings->SetConnectionPreferenceOverride(prefs)){
   case KErrNone:
        break;
   case KErrNotSupported:
/*         CEikonEnv::Static()->InfoMsg(_L("KErrNotSupported")); */
        break;
   case KErrNotFound:
/*         CEikonEnv::Static()->InfoMsg(_L("KErrNotFound")); */
        break;
   case KErrAlreadyExists:
/*         CEikonEnv::Static()->InfoMsg(_L("KErrAlreadyExists")); */
        break;
   }

   iAgent->Open();
   iAgent->StartOutgoing( *iSettings, iStatus );

   if(!IsActive()){
      SetActive();
   }
#endif
}

void
CConnectionManager::StopConnectionManager()
{
#ifdef USE_RAGENT
   // XXX The iAgent crashes the app with Kern-exec 0 when calling 
   // close on an already closed iAgent. 
   iAgent->Open();
   iAgent->Stop();
   iAgent->Close();

#elif defined USE_RCONNECTION
   iConnection->Close();
   iSocketServ->Close();
#endif
}

void 
CConnectionManager::ConnMgrCancel()
{
   CActive::Cancel();
}

void
CConnectionManager::DoCancel()
{
   StopConnectionManager();
}

void
CConnectionManager::RunL()
{
   switch (iStatus.Int()) {
      case KErrNone:
         /* It's alive! */
         iObserver->ConnMgrSuccess();
         break;
      case KErrAlreadyExists:
         break;
      default:
         /* Not working. */
         iObserver->ConnMgrFailed();
         break;
   }
}
