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

#include "NetworkInfoHandler.h"
#include "NetworkInfoObserver.h"

#include "machine.h"

CNetworkInfoHandler* CNetworkInfoHandler::NewLC(MNetworkInfoObserver* aObserver, 
                                                enum TPriority aPriority)
{
   CNetworkInfoHandler* self = 
      new (ELeave) CNetworkInfoHandler(aObserver, aPriority);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CNetworkInfoHandler* CNetworkInfoHandler::NewL(MNetworkInfoObserver* aObserver, 
                                               enum TPriority aPriority)
{
   CNetworkInfoHandler* self = 
      CNetworkInfoHandler::NewLC(aObserver, aPriority);
   CleanupStack::Pop(self);
   return self;
}

CNetworkInfoHandler::~CNetworkInfoHandler()
{
   this->Cancel();
   delete iTelephony;
}

CNetworkInfoHandler::CNetworkInfoHandler(MNetworkInfoObserver* aObserver, 
                                         enum TPriority aPriority) :
   CActive(aPriority),
   iObserver(aObserver),
   iState(EIdle),
   iIndicatorV1Pckg(iIndicatorV1),
   iSigStrengthV1Pckg(iSigStrengthV1),
   iNetworkRegistrationV1Pckg(iNetworkRegistrationV1),
   iNetworkInfoV1Pckg(iNetworkInfoV1)
{
   iSigStrength = MAX_INT32;
   iRegStatus = CTelephony::ERegistrationUnknown;
   iNetworkMode = CTelephony::ENetworkModeUnknown;
   iMcc = KNullDesC;
   iMnc = KNullDesC;
   iLac = MAX_UINT32;
   iCellId = MAX_UINT32;
   iNetworkAccess = CTelephony::ENetworkAccessUnknown;
   iCdmaSID = KNullDesC;
   iCdmaBID = CTelephony::EBandUnknown;
}

void CNetworkInfoHandler::ConstructL()
{
   CActiveScheduler::Add(this);
   iTelephony = CTelephony::NewL();
}

void CNetworkInfoHandler::FetchAllNetworkInfoL()
{
   iSigStrength = MAX_INT32;
   iRegStatus = CTelephony::ERegistrationUnknown;
   iNetworkMode = CTelephony::ENetworkModeUnknown;
   iMcc = KNullDesC;
   iMnc = KNullDesC;
   iLac = MAX_UINT32;
   iCellId = MAX_UINT32;
   iNetworkAccess = CTelephony::ENetworkAccessUnknown;
   iCdmaSID = KNullDesC;
   iCdmaBID = CTelephony::EBandUnknown;

   if (iState == EIdle) {
      iState = EFetchingSignalIndicator;
      iTelephony->GetIndicator(iStatus, iIndicatorV1Pckg);
      SetActive();
   } else {
      iObserver->NetworkInfoInUse();
   }
}

void CNetworkInfoHandler::GetSignalStrength()
{
   iState = EFetchingSignalStrength;
   iTelephony->GetSignalStrength(iStatus, iSigStrengthV1Pckg);
   SetActive();
}

void CNetworkInfoHandler::GetRegistrationStatus()
{
   iState = EFetchingRegistrationStatus;
   iTelephony->GetNetworkRegistrationStatus(iStatus, iNetworkRegistrationV1Pckg);
   SetActive();
}

void CNetworkInfoHandler::GetNetworkInfo()
{
   iState = EFetchingNetworkInfo;
   iTelephony->GetCurrentNetworkInfo(iStatus, iNetworkInfoV1Pckg);
   SetActive();
}

void CNetworkInfoHandler::FetchingDone()
{
   iState = EIdle;
   iObserver->NetworkInfoDone();
}

void CNetworkInfoHandler::FetchingFailed()
{
   iState = EIdle;
   iObserver->NetworkInfoFailed(iStatus.Int());
}

void CNetworkInfoHandler::ProcessNetworkInfo()
{
   iNetworkMode = iNetworkInfoV1.iMode;
   iMcc = iNetworkInfoV1.iCountryCode;
   iMnc = iNetworkInfoV1.iNetworkId;
   //if (iNetworkInfoV1.iAreaKnown) {
   // iAreaKnown needs to be true for the LAC and CellId to be valid 
   // according to SDK documentation. BUT for iAreaKnown to become 
   // true it takes a few seconds after startup of Wayfinder and from 
   // testing a little bit the the LAC and CellId seems to be correct 
   // even without the if-guard above and then we don't need to wait.
   // So if you get strange values from LAC and CellId enable the if-guard.
   iLac = iNetworkInfoV1.iLocationAreaCode;
   iCellId = iNetworkInfoV1.iCellId;
   //}
   iNetworkAccess = iNetworkInfoV1.iAccess;
   if (iNetworkMode == CTelephony::ENetworkModeAmps || 
       iNetworkMode == CTelephony::ENetworkModeCdma95 || 
       iNetworkMode == CTelephony::ENetworkModeCdma2000) {
      iCdmaSID = iNetworkInfoV1.iCdmaSID;
      iCdmaBID = iNetworkInfoV1.iBandInfo;
   }
}

void CNetworkInfoHandler::RunL()
{
   switch (iStatus.Int()) {
   case KErrNone:
      {
         switch (iState) {
         case EFetchingSignalIndicator:
            if (iIndicatorV1.iCapabilities & CTelephony::KIndNetworkAvailable) {
               // We can detect when a network is present.
               if (iIndicatorV1.iIndicator & CTelephony::KIndNetworkAvailable) {
                  // Network is present, go ahead and fetch info about it.
                  GetSignalStrength();
               } else {
                  // Network is not present.
#ifdef __WINS__  
                  // If wins, pretend with fixed test values
                  iNetworkMode = (CTelephony::TNetworkMode)6;
                  iMcc = _L("240");
                  iMnc = _L("08");
                  iLac = 330;
                  iCellId = 246749;
                  iNetworkAccess = (CTelephony::TNetworkAccess)3;
                  FetchingDone();
#else            	
                  FetchingFailed();
#endif                  
               }
            } else {
               // We do not know whether a network is present.
               FetchingFailed();
            }
            break;
         case EFetchingSignalStrength:
            // Signal strength (in dBm), 0 means the value cannot be measured.
            iSigStrength = iSigStrengthV1.iSignalStrength;
            GetRegistrationStatus();
            break;
         case EFetchingRegistrationStatus:
            // We can check for No Service, Emergency Only, Busy, Roaming, etc.
            iRegStatus = iNetworkRegistrationV1.iRegStatus;
            GetNetworkInfo();
            break; 
         case EFetchingNetworkInfo:
            // We can get cellid, mcc, mnc, lac, etc.
            ProcessNetworkInfo();
            FetchingDone();
            break;
         case EDone:
         case EIdle:
         default:
            //Should not happen, do nothing.
            break;
         }
      }
      break;
   case KErrNotSupported:
      if (iState == EFetchingSignalStrength) {
         // Signal strength retrieval is not supported on this phone, so 
         // continue fetching other information.
         GetRegistrationStatus();
      } else if (iState == EFetchingRegistrationStatus) {
         // Registration status retrieval is not supported on the phone,
         // try to get the network info anyway.
         GetNetworkInfo();
      }
      break;
   case KErrNotFound:
      if (iState == EFetchingSignalStrength) {
         // No signal strength info is available at the moment, so 
         // continue fetching other information.
         GetRegistrationStatus();
      } else if (iState == EFetchingRegistrationStatus) {
         // No registration status is available, try to get the 
         // network info anyway.
         GetNetworkInfo();
      }
      break;
   case KErrCancel:
      FetchingFailed();
      break;
   default:
      FetchingFailed();
      break;
   }
}

void CNetworkInfoHandler::DoCancel()
{
   if (IsActive()) {
      switch (iState) {
      case EFetchingSignalIndicator:
         iTelephony->CancelAsync(CTelephony::EGetIndicatorCancel);
         break;
      case EFetchingSignalStrength:
         iTelephony->CancelAsync(CTelephony::EGetSignalStrengthCancel);
         break;
      case EFetchingRegistrationStatus:
         iTelephony->CancelAsync(CTelephony::EGetNetworkRegistrationStatusCancel);
         break; 
      case EFetchingNetworkInfo:
         iTelephony->CancelAsync(CTelephony::EGetCurrentNetworkInfoCancel);
         break;
      case EDone:
      case EIdle:
      default:
         //Should not happen, do nothing.
         break;
      }
   }
}
