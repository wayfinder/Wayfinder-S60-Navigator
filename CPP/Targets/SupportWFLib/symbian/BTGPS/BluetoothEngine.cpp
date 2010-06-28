/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "BluetoothEngine.h"

#include "BtSelector.h"
#include "BtGpsConnector.h"
#include "SdpExaminer.h"
#include "BtSocketsEngine.h"
#include "BtPower.h"
#include "SdpRepeater.h"

#include "DiscoveryUtils.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

#define DBG_ACTIVE(p__) \
 DBG(#p__ " is%s present %s", \
      p__ ? "" : " not", \
      p__ ? (p__->IsActive() ? "and active" : "but not active") : "")

#ifdef NAV2_CLIENT_SERIES60_V1
static const TInt KSerialPortUUID = 0x1101;
#endif

_LIT(KBtEngine, "BtEngine");
enum TBtEnginePanics {
   EUndefinedState         =  1,
   ESdpFailedOnKnownDevice =  2,
};

template<class T>
void DeleteAndNull(T*& aPtr)
{
   delete aPtr;
   aPtr = NULL;
}

template<class T>
void SwapAndDelete(T*& aDst, T*& aSrc)
{
   delete aDst;
   aDst = aSrc;
   aSrc = NULL;
}

template<class T>
void SwapAndReset(T& aDst, T& aSrc)
{
   aDst = aSrc;
   aSrc = T();
}

inline TPtrC OrNullDesC(const HBufC* aBuf)
{
   return aBuf ? *aBuf : KNullDesC();
}


CBluetoothEngine::CBluetoothEngine(class MBluetoothEngineObserver& aObserver) :
   CActiveLog(EPriorityStandard, "BtEngine"), iObserver(aObserver)
{
   CActiveScheduler::Add(this);
}

void CBluetoothEngine::ConstructL()
{
}

class CBluetoothEngine* 
CBluetoothEngine::NewL(class MBluetoothEngineObserver& aObserver)
{
   class CBluetoothEngine* self = CBluetoothEngine::NewLC(aObserver);
   CleanupStack::Pop(self);
   return self;
}

class CBluetoothEngine* 
CBluetoothEngine::NewLC(class MBluetoothEngineObserver& aObserver)
{
   class CBluetoothEngine* self = new (ELeave) CBluetoothEngine(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CBluetoothEngine::~CBluetoothEngine()
{
   DBG("Destructor");
   if(IsActive()){
      DBG("Destroyed while active.");
      Cancel();
      DBG("Cancel done");
   }
   delete iMaybeDeviceName;
   delete iDeviceName;
   delete iDelayedSelector;
   delete iSdpExaminer;
   delete iSocketsEngine;
   delete iPower;
   DBG("Destructor done");
}

TBool CBluetoothEngine::IsConnected()
{
   return iSocketsEngine && iSocketsEngine->IsConnected();
}

TBool CBluetoothEngine::IsBusy()
{
   //XXX is this enough?
   return IsActive();
}

void CBluetoothEngine::GetDeviceName(TDes& aName) const
{
   aName = iDeviceName ? *iDeviceName : KNullDesC();
}

void CBluetoothEngine::GetDeviceAddr(TDes8& aAddr) const
{
   aAddr = iDeviceAddr.Des();
}

const class TBTDevAddr CBluetoothEngine::GetDeviceAddress() const
{
   return iDeviceAddr;
}

void CBluetoothEngine::SetDeviceAddr(const class TBTDevAddr& aAddr)
{
   DBG("DeviceAddress set to "PRIxBTADDR, PRIxBTEXPAND(aAddr));
   iDeviceAddr = aAddr;
}

TInt CBluetoothEngine::ConnectL(const class TSdpRepeatSettings& aRepeats)
{
   if(IsBusy() && iRepeater){
      DBG("IsBusy with SdpRepeater.");
      DBG("Our address: "PRIxBTADDR", Sdp's address: "PRIxBTADDR, 
          PRIxBTEXPAND(iDeviceAddr), PRIxBTEXPAND(iRepeater->GetAddress()));
      Cancel();
      DBG("Not busy anymore, and no SdpRepeater either");
   }
      
   if(!IsBusy() && !IsConnected()){
      iRepeatSettings = aRepeats;
      if(iDeviceAddr == TBTDevAddr()){
         DBG("Connect to new device, start with selectnotifier");
         //no known bt address, start the selection of a new device. 
         StartBtSelectorL();
      } else if(IsS60Symbian7s()){
         StartSdpRepeaterL();
      } else {
         //TODO: check if bt is on or not so we can restore that state on
         //TODO: exit.
         //we know which device we want. Make sure BT is turned on.
         DBG("Connect to known device, start with power check.");
         iPower = CBtPowerChecker::NewL();
         UpdateLogMastersL();
         iPower->PowerCheck(&iStatus, ETrue);
         SetActive();
      }
      return KErrNone;
   }
   WARN("ConnectL returns KErrInUse since it is busy or connected!");
   LogComponents();
   return KErrInUse;
}

void CBluetoothEngine::DisconnectL()
{
   if(!IsBusy()){
      if(iSocketsEngine && iSocketsEngine->IsConnected()){
         DBG("Starting disconnect");
         iSocketsEngine->Disconnect(&iStatus);
         SetActive();
      }
   }
}

TInt CBluetoothEngine::SendDataL(const TDesC8& aData)
{
   if(IsConnected()){
      iSocketsEngine->WriteL(aData);
   } else {
      ERR("Trying to write data when no device is connected.");
      return KErrNotReady;
   }
   return KErrNone;
}

void CBluetoothEngine::DoCancel()
{
   WARN("I have no idea what to do here!");
   WARN("iStatus: 0x%x", unsigned(iStatus.Int()));
   if(iDelayedSelector){
      DBG_ACTIVE(iDelayedSelector);
      iDelayedSelector->CancelDelayedCall();
      DeleteAndNull(iDelayedSelector);
      DBG("CBtSelector destroyed.");
   } else if(iSdpExaminer){
      DBG_ACTIVE(iSdpExaminer);
      iSdpExaminer->CancelFind();
      DeleteAndNull(iSdpExaminer);
      DBG("CSdpExaminer destroyed");
   } else if(iSocketsEngine){
      DBG_ACTIVE(iSocketsEngine);
      if(iSocketsEngine->IsActive()){
         iSocketsEngine->CancelAll();
      } 
      DeleteAndNull(iSocketsEngine);
      DBG("CBTSocketsEngine destroyed");
   } else if(iPower){
      DBG_ACTIVE(iPower);
      iPower->CancelPowerCheck();
      DeleteAndNull(iPower);
      DBG("CBtPower destroyed");
   } else if(iRepeater){
      DBG_ACTIVE(iRepeater);
      iRepeater->CancelFind();
      DeleteAndNull(iRepeater);
      DBG("CSdpRepeater destroyed.");
   }
}

void CBluetoothEngine::StartBtSelectorL()
{
   class CBtSelector* selector = CBtSelector::NewL();
   UpdateLogMastersL();
#ifdef NAV2_CLIENT_SERIES60_V2
   DBG("Starting delayed selector");
   iDelayedSelector = CActiveDelay<CBtSelector>::NewL(selector, 
                                                      TTimeIntervalSeconds(5));
#else
   DBG("Starting selector now.");
   iDelayedSelector = CActiveDelay<CBtSelector>::NewL(selector, 
                                                      TTimeIntervalSeconds(0));
#endif
   iDelayedSelector->Start(&CBtSelector::SelectSerialDeviceL, 
                           &CBtSelector::CancelSelection, &iStatus);
   SetActive();
   iRepeatSettings = TSdpRepeatSettings();
}

void CBluetoothEngine::StartSdpExaminerL(const class TBTDevAddr& aAddress)
{
   DBG("Start Sdp for: "PRIxBTADDR, PRIxBTEXPAND(aAddress));

   iSdpExaminer = CSdpExaminer::NewL(aAddress);
   UpdateLogMastersL();
   iSdpExaminer->FindSerialPortL(&iStatus);
   SetActive();
}

void CBluetoothEngine::StartSocketsEngineL(const class TBTDevAddr& aAddr,
                                           TInt aPort)
{
   iSocketsEngine = CBtSocketsEngine::NewL(*this, aAddr, aPort);
   UpdateLogMastersL();
   iSocketsEngine->ConnectL(&iStatus);
   SetActive();
}

void CBluetoothEngine::StartSdpRepeaterL()
{
   iRepeater = CSdpRepeater::NewL(*this, iDeviceAddr);
   UpdateLogMastersL();
   iRepeater->SetRepeatSettings(iRepeatSettings);
   iRepeatSettings = TSdpRepeatSettings();
   iRepeater->FindSerialPortL(&iStatus);
   SetActive();
}

void CBluetoothEngine::RunL()
{
   if(iDelayedSelector){
      class CBtSelector* selector = iDelayedSelector->Other();
      if(iStatus == KErrNone){
         DBG("CBtSelector done OK, user has selected a device.");
         //the user has selected a device. 
         //XXX need to handle any leaves gracefully.
         iMaybeDeviceAddr = selector->Address();
         DeleteAndNull(iMaybeDeviceName);
         iMaybeDeviceName = selector->Name().Alloc();

         StartSdpExaminerL(iMaybeDeviceAddr);
      } else if(iStatus == KErrHardwareNotAvailable){
         DBG("CBtSelector returned KErrHardwareNotAvailable");
         //error message.
         iObserver.BtEngineInfo(EBtEngineSelectionFailed);
      } else if(iStatus == KErrCancel){
         DBG("CBtSelector KErrCancel-ed.");
         //probably user cancel. no error message. 
      } else {
         DBG("CBtSelector not OK, BTError(EB...NotFound, %d)", iStatus.Int());
         //error message.
         iObserver.BtEngineInfo(EBtEngineSelectionFailed);
      }
      delete selector;
      DeleteAndNull(iDelayedSelector);
   } else if(iSdpExaminer){
      if(iStatus == KErrNone){
         if(iDeviceAddr == TBTDevAddr()){
            DBG("CSdpSelector has a serial port on a new device.");
            //The user has selected a new device, and a serial port has
            //been found on that device. It's time to connect.
            StartSocketsEngineL(iMaybeDeviceAddr, iSdpExaminer->Port());
         } else {
            //this case shouldn't happen anymore as connections to
            //known devices are handled through CSdpRepeater.
            DBG("CSdpSelector has found the serial port on a known device.");
            StartSocketsEngineL(iDeviceAddr, iSdpExaminer->Port());
         }
      } else if(iStatus == KErrHardwareNotAvailable){
         DBG("Low battery or flight mode.");
         iObserver.BtEngineInfo(EBtEngineHardwareUnavailable);
      } else if(iStatus == KErrTimedOut){
         DBG("SpdExaminer timeout."); //L2CAP or HCI
         iObserver.BtEngineInfo(EBtEngineHardwareUnavailable);
      } else {
         WARN("SdpExaminer op failed: %d", iStatus.Int());
         if(iDeviceAddr == TBTDevAddr()){
            iObserver.BtEngineInfo(EBtEngineSelectionSdpFailed);
         } else {
            //this should never happen
            User::Panic(KBtEngine, ESdpFailedOnKnownDevice);
         }
      }
      DeleteAndNull(iSdpExaminer);
   } else if(iRepeater){
      if(iStatus == KErrNone){
         DBG("CSdpRepeater has found the serial port on a known device.");
         StartSocketsEngineL(iDeviceAddr, iRepeater->Port());
      } else if(iStatus == KErrHardwareNotAvailable){
         DBG("Low battery or flight mode.");
         iObserver.BtEngineInfo(EBtEngineHardwareUnavailable);
      } else {
         DBG("CSdpRepeater has run out of retries.");
         iObserver.BtEngineInfo(EBtEngineRepeatedSearchFailed);
      }
      DeleteAndNull(iRepeater);
   } else if(iPower){
      if(iStatus == KErrNone){
         if(iPower->IsPoweredOn()){
            //we have power!
            DBG("We have power!");
            StartSdpRepeaterL();
         } else {
            WARN("We have no power!");
            iObserver.BtEngineInfo(EBtEnginePowerOnCancelled); 
         }
      } else {
         ERR("CBtPower op failed.");
         iObserver.BtEngineInfo(EBtEnginePowerOnFailed);
      }
      DeleteAndNull(iPower);
   } else if(iSocketsEngine){
      if(iStatus == KErrNone){
         if(iSocketsEngine->IsConnected()){
            //connection is complete. Reading has started, we are ready
            //to write.
            DBG("SocketsEngine connection complete.");
            if(iDeviceAddr == TBTDevAddr()){
               DBG("Setting device address "PRIxBTADDR" and name.", 
                   PRIxBTEXPAND(iMaybeDeviceAddr));
               SwapAndReset(iDeviceAddr,  iMaybeDeviceAddr);
               SwapAndDelete(iDeviceName, iMaybeDeviceName);
            } else {
               //XXX No need to do anything?
            }
            iObserver.BtEngineInfo(EBtEngineConnectionComplete);
            iObserver.BtEngineDevice(iDeviceAddr, OrNullDesC(iDeviceName));
         } else {
            //disconnection complete
            DBG("SocketsEngine disconnect complete.");
            DeleteAndNull(iSocketsEngine);
            iObserver.BtEngineInfo(EBtEngineDisconnectionComplete);
         }
         //XXX what?
      } else {
         ERR("SocketsEngine operation failed: %d", iStatus.Int());
         DeleteAndNull(iSocketsEngine);
         iObserver.BtEngineInfo(EBtEngineSocketConnectionFailed);
      }
   } else {
      ERR("Unhandled state");
      User::Panic(KBtEngine, EUndefinedState);
   }
}

TInt CBluetoothEngine::RunError( TInt aError )
{
   ERR("RunL left with code %d", aError);
   if(iRepeater){
      if(aError == KErrNotSupported){
         DeleteAndNull(iRepeater);
         iObserver.BtEngineInfo(EBtEngineHardwareUnavailable); //XXX
         aError = KErrNone; //cancel out the error dialog
      }
   }
   return aError;
}

void CBluetoothEngine::SdpRepeatInfo(TInt aNum, TInt aTotal)
{
   iObserver.BtEngineSearching(aNum, aTotal);
}

void CBluetoothEngine::ConnectionLost(TBool aReconnect)
{
   WARN("The socket was disconnected!");
   DeleteAndNull(iSocketsEngine);
   if(aReconnect){
      iRepeatSettings = TSdpRepeatSettings(TTimeIntervalSeconds(30));
      StartSdpRepeaterL();
   } else {
      //XXX do something here?
   }
}

void CBluetoothEngine::ConnectionCancelled()
{
   DBG("ConnectionCancelled");
}

void CBluetoothEngine::ReceiveMessageL(const class TDesC8& aData)
{
   //DBG("Received %d bytes of data", aData.Length());
   //iReciver.ReceiveBTData(aData); //receiveddata
   DBG("Read %d bytes from socket", aData.Length());
   iObserver.BtEngineData(aData);
}

void CBluetoothEngine::ErrorNotify(enum TBtSocketsErrors aError, 
                                   TInt aStatus)
{
   WARN("ErrorNotify(%d, %d)", int(aError), aStatus);
   switch(aError){
   case EBtSocketsReadError:
      iObserver.BtEngineInfo(EBtEngineConnectionLostReadError); //???
      break;
   case EBtSocketsReadHardwareFail:
      iObserver.BtEngineInfo(EBtEngineConnectionLostHardwareFailed); //???
      break;
   case EBtSocketsWriteTimeout:
      iObserver.BtEngineInfo(EBtEngineConnectionLostWriteTimeout); //???
      break;
   case EBtSocketsWriteError:
      iObserver.BtEngineInfo(EBtEngineConnectionLostWriteError); //???
      break;
   }
}

CArrayPtr<CActiveLog>* CBluetoothEngine::SubLogArrayLC()
{
   CArrayPtr<CActiveLog>* array = new (ELeave) CArrayPtrFlat<CActiveLog>(4);
   CleanupStack::PushL(array);
   class CActiveLog* logs[] = 
      { iDelayedSelector ? iDelayedSelector->Other() : NULL, 
        iSdpExaminer, iPower, iSocketsEngine, iRepeater };
   array->AppendL(logs, sizeof(logs)/sizeof(*logs));
   return array;
}

void CBluetoothEngine::LogComponents()
{
#ifndef NO_LOG_OUTPUT
   DBG("BluetoothEngine subcomponents:");
   DBG_ACTIVE(this);
   DBG_ACTIVE(iDelayedSelector);
   DBG_ACTIVE(iSdpExaminer);
   DBG_ACTIVE(iPower);
   DBG_ACTIVE(iSocketsEngine);
   DBG_ACTIVE(iRepeater);
   DBG("All cubcomponents accounted for.");
#endif
}
