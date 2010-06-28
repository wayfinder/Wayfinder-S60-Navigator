/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if !(defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3)

#include "BtGpsConnector.h"
#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1
# include <cknflopr.h>
#endif
#include <coemain.h>
#include <barsread.h>
#include "SprocketsEngine.h"
#include "WFTextUtil.h"

#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

#if (!defined NO_LOG_OUTPUT) && (defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1)
# define PROGRESS_BAR
#endif

//#define DEBUG_BT_SEARCH

// ------ MGpsConnectionObserver -----

void MGpsConnectionObserver::BluetoothDebug(const TDesC& /*aDbgMsg*/) 
{
}

void MGpsConnectionObserver::BluetoothDeviceChosen(TInt64 /*aAddr*/, 
                                                   const TDesC& /*aName*/)
{
}

void MGpsConnectionObserver::BluetoothDeviceChosen(const class TBTDevAddr&, 
                                                   const TDesC& /*aName*/)
{
}

// ---------- CBtGpsConnector ----------------

CBtGpsConnector::CBtGpsConnector(class MGpsConnectionObserver* aObserver) :
   iObserver(aObserver)
{
}

void CBtGpsConnector::ConstructL(class isab::Nav2::Channel* aChannel)
{
   iBtEngine = CBluetoothEngine::NewL( *this );
   iSprocket = CSprocketsEngine::NewL(this, aChannel);
   iSprocket->ConnectL();
}

void CBtGpsConnector::ConstructL(class MGpsObserver* aObserver)
{
   iBtEngine = CBluetoothEngine::NewL(*this);
   iReceptor = aObserver;
}

class CBtGpsConnector* 
CBtGpsConnector::NewLC(class MGpsConnectionObserver* aObserver,
                       class isab::Nav2::Channel* aChannel)
{
   class CBtGpsConnector* self = new (ELeave) CBtGpsConnector(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL(aChannel);
   return self;
}

class CBtGpsConnector* 
CBtGpsConnector::NewLC(class MGpsObserver* aObserver)
{
   class CBtGpsConnector* self = new (ELeave) CBtGpsConnector(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL(aObserver);
   return self;
}


class CBtGpsConnector* 
CBtGpsConnector::NewL(class MGpsConnectionObserver* aObserver,
                      class isab::Nav2::Channel* aChannel)
{
   class CBtGpsConnector* self = CBtGpsConnector::NewLC(aObserver, aChannel);
   CleanupStack::Pop(self);
   return self;
}

class CBtGpsConnector* 
CBtGpsConnector::NewL(class MGpsConnectionObserver* aObserver,
                      struct TBtErrorToResource* aMapping, TInt aMappingSize,
                      class isab::Nav2::Channel* aChannel)
{
   class CBtGpsConnector* self = CBtGpsConnector::NewLC(aObserver, aChannel);
   self->SetErrorMappingL(aMapping, aMappingSize);
   CleanupStack::Pop(self);
   return self;
}

class CBtGpsConnector* 
CBtGpsConnector::NewL(class MGpsConnectionObserver* aObserver,
                      TInt aMappingResource,
                      class isab::Nav2::Channel* aChannel)
{
   class CBtGpsConnector* self = CBtGpsConnector::NewLC(aObserver, aChannel);
   self->SetErrorMappingL(aMappingResource);
   CleanupStack::Pop(self);
   return self;
}


class CBtGpsConnector* 
CBtGpsConnector::NewL(class MGpsObserver* aObserver,
                      struct TBtErrorToResource* aMapping, TInt aMappingSize)
{
   class CBtGpsConnector* self = CBtGpsConnector::NewLC(aObserver);
   self->SetErrorMappingL(aMapping, aMappingSize);
   CleanupStack::Pop(self);
   return self;
}

class CBtGpsConnector* 
CBtGpsConnector::NewL(class MGpsObserver* aObserver,
                      TInt aMappingResource)
{
   class CBtGpsConnector* self = CBtGpsConnector::NewLC(aObserver);
   self->SetErrorMappingL(aMappingResource);
   CleanupStack::Pop(self);
   return self;
}

CBtGpsConnector::~CBtGpsConnector()
{
   DBG("~BtGpsConnector");
   delete iMapping;
   iMapping = NULL;
   delete iBtEngine;
   iBtEngine = NULL;
   delete iSprocket;
   iSprocket = NULL;
   DBG("destructor complete");
   delete iLog;
   iLog = NULL;
}

void CBtGpsConnector::SetErrorMappingL(TInt aMappingResource)
{
   HBufC8* res= CCoeEnv::Static()->AllocReadResourceAsDes8LC(aMappingResource);
   class TResourceReader reader;
   reader.SetBuffer(res);
   TInt mappingSize = reader.ReadInt16();
   struct TBtErrorToResource* mapping = NULL;
   if(mappingSize < 1){
      mapping = new (ELeave) TBtErrorToResource[mappingSize];
      CleanupStack::PushL(mapping);
      for(TInt i = 0; i < mappingSize; ++i){
         struct TBtErrorToResource& curr = mapping[i];
         curr.iError = reader.ReadInt16();
         curr.iResourceString = reader.ReadInt32();
      }
   }
   if(mapping){
      this->SetErrorMappingL(mapping, mappingSize);
      CleanupStack::PopAndDestroy(mapping);
   }
   CleanupStack::PopAndDestroy(res);
}

void 
CBtGpsConnector::SetErrorMappingL(const struct TBtErrorToResource* aMapping,
                                  TInt aNum)
{
   CArrayFix<TBtErrorToResource>* newArray = 
      new (ELeave) CArrayFixSeg<TBtErrorToResource>(aNum);
   CleanupStack::PushL(newArray);
   for(TInt q = 0; q < aNum; ++q){
      newArray->AppendL(aMapping[q]);
   }
   CleanupStack::Pop(newArray);
   {
      CArrayFix<TBtErrorToResource>* tmp = iMapping;
      iMapping = newArray;
      newArray = tmp;
   }
   delete newArray;
}

void CBtGpsConnector::ReleaseL()
{
   if(iSprocket) {
      iSprocket->Release();
   }
   DisconnectL();
}

void CBtGpsConnector::ForwardAddress(const class TBTDevAddr& aAddress, 
                                     const TDesC& aName)
{
   TInt64 address = 0;
   for(TInt a = 0; a < 6; ++a){
      address = (address << 8) + TInt64(aAddress[a] & 0x0ff);
   }
   DBG("BTAddress: "PRIxBTADDR, PRIxBTEXPAND(aAddress));
   iObserver->BluetoothDeviceChosen(address, aName);
   iObserver->BluetoothDeviceChosen(aAddress, aName);
}

void CBtGpsConnector::ReceiveMessageL(const TDesC8& aBytes)
{
   iBtEngine->SendDataL(aBytes);
}

void CBtGpsConnector::ConnectL()
{
#ifdef DEBUG_BT_SEARCH
   _LIT(KConnectWithAddress, "BtGpsConnector::ConnectL");
   iObserver->BluetoothDebug(KConnectWithAddress);
#endif
   DBG("ConnectL");
   iBtEngine->ConnectL();
}

void CBtGpsConnector::DisconnectL()
{
   iBtEngine->DisconnectL();
}

TBool CBtGpsConnector::IsConnected() const
{
   return iBtEngine && iBtEngine->IsConnected();
}

TBool CBtGpsConnector::IsBusy() const
{
   return iBtEngine && iBtEngine->IsBusy();
}

TBool CBtGpsConnector::SetAddress(const char **aData, TInt aNum)
{
   DBG("SetAddress char**, %d entries", aNum);
   if(aNum > 2){
      DBG("Address %s, %s", aData[0], aData[1]);
      char* end = NULL;
      TUint32 high = strtoul(aData[0], &end, 16);
      DBG("high: %08"PRIx32, high);
      //    __ASSERT_DEBUG(end == (aData[0] + strlen(aData[0])), 
      //                   ::Panic(EWayfinderBadBtGpsAddress));
      TUint32 low  = strtoul(aData[1], &end, 16);
      DBG("low: %08"PRIx32, low);
      //    __ASSERT_DEBUG(end == (aData[1] + strlen(aData[1])), 
      //                   ::Panic(EWayfinderBadBtGpsAddress));
      TInt64 address = MakeInt64(high, low);
      SetAddressAndConnectL(address);
      return ETrue;
   }
   return EFalse;
}

namespace {
   unsigned long int strtoul(const TText8 *nptr, char **endptr, int base)
   {
      return ::strtoul(reinterpret_cast<const char*>(nptr), endptr, base);
   }
}

TBool CBtGpsConnector::SetAddress(const class MDesC8Array& aData)
{
   DBG("SetAddress MDesC8Array, %d entries", aData.MdcaCount());
   if(aData.MdcaCount() > 2){
      TBuf8<32> buf = aData.MdcaPoint(0);
      char* end = NULL;
      TUint32 high = strtoul(buf.PtrZ(), &end, 16);
      DBG("high: 0x%08"PRIx32, high);
      buf = aData.MdcaPoint(1);
      TUint32 low = strtoul(buf.PtrZ(), &end, 16);
      TInt64 address = MakeInt64(high, low);
      SetAddressAndConnectL(address);
      return ETrue;
   }
   return EFalse;
}

void CBtGpsConnector::SetAddressAndConnectL(const class TBTDevAddr& aAddress)
{
   if((aAddress != TBTDevAddr()) &&
      (aAddress != iBtEngine->GetDeviceAddress())){
      DBG("Interesting address "PRIxBTADDR, PRIxBTEXPAND(aAddress));
      iBtEngine->SetDeviceAddr(aAddress);
      if(!(IsBusy() || IsConnected())){
         //first connect, search for a minute or so
         iBtEngine->ConnectL(TSdpRepeatSettings(TTimeIntervalMinutes(2), 
                                                TTimeIntervalSeconds(20)));
      }
   }
   
}

void CBtGpsConnector::DisconnectComplete()
{
   if(iReconnecting){
      DBG("DisconnectComplete, iReconnecting set. Starting ConnectL.");
      iReconnecting = EFalse;
      ConnectL();
   }
}


void CBtGpsConnector::ReConnect()
{
   TBool wasConnected = IsConnected();
   DBG("ReConnect, was %sconnected", wasConnected ? "" : "not ");
   DisconnectL(); //disconnect current device. 
   iBtEngine->SetDeviceAddr(TBTDevAddr()); //clear address of btclient
   ForwardAddress(TBTDevAddr(), KNullDesC); //clear address parameter.
   iReconnecting = wasConnected;
   if(! wasConnected){
      DBG("Not connected, starting connection  now.");
      ConnectL();
   }
}

void CBtGpsConnector::SetLogMaster(class isab::LogMaster* aNewMaster)
{
   delete iLog;
   iLog = NULL;
   iLog = new (ELeave) isab::Log("BtGpsConn", isab::Log::LOG_ALL, aNewMaster);
   iBtEngine->SetLogMasterL(aNewMaster);
   iSprocket->SetLogMasterL(aNewMaster, "BtSprocket");
}

void CBtGpsConnector::BtEngineInfo(enum TBtEngineEvent aEvent)
{
   DBG("BtEngineInfo 0x%x", unsigned(aEvent));
   if(iMapping){
      TInt i = 0;
      while(i < iMapping->Count() && iMapping->At(i).iError != aEvent){
         ++i;
      }
      if(i < iMapping->Count()){
         iObserver->BluetoothError(iMapping->At(i).iResourceString);
      }
   }
   switch(aEvent){
   case EBtEngineSelectionFailed:
   case EBtEngineSelectionSdpFailed:
      //DBG("BTError(EBluetoothNewDeviceNotFound, %d)", TInt(aEvent));
      //BTError(EBluetoothNewDeviceNotFound, aEvent);
      break;
   case EBtEngineSocketConnectionFailed:
   case EBtEngineConnectionFailed:
   case EBtEngineRepeatedSearchFailed:
      //DBG("BTError(EBluetoothDeviceNotFound, %d)", TInt(aEvent));
      //BTError(EBluetoothDeviceNotFound, aEvent);
      break;
   case EBtEnginePowerOnFailed:
   case EBtEnginePowerOnCancelled:
      //DBG("BTError(EBluetoothTurnedOff, %d)", TInt(aEvent));
      //BTError(EBluetoothTurnedOff, aEvent);
      break;
   case EBtEngineConnectionLostReadError:
   case EBtEngineConnectionLostHardwareFailed:
   case EBtEngineConnectionLostWriteTimeout:
   case EBtEngineConnectionLostWriteError:
      //DBG("BTError(EBluetoothDisconnected, %d)", TInt(aEvent));
      //BTError(EBluetoothDisconnected, aEvent);
      break;
   case EBtEngineHardwareUnavailable:
      break;
   case EBtEngineConnectionComplete:
      DBG("EConnectionComplete");
      break;
   case EBtEngineDisconnectionComplete:
      DBG("DisconnectComplete()");
      DisconnectComplete();
      break;
   }
#ifdef PROGRESS_BAR
   delete iBar;
   iBar = NULL;
#endif
}

void CBtGpsConnector::BtEngineSearching(TInt aNum, TInt aTotal)
{
#ifdef PROGRESS_BAR
   if(!iBar){
      DBG("Creating floatingprogressbar");
      iBar = CCknFloatingProgressBar::NewL(_L("Searching..."), aTotal);
   }
   DBG("Updating progressbar");
   iBar->UpdateValue(aNum);
#endif
}

void CBtGpsConnector::BtEngineDevice(const class TBTDevAddr& aAddr, 
                                     const TDesC& aName)
{
   TInt64 address = 0;
   for(TInt a = 0; a < 6; ++a){
      address = (address << 8) + TInt64(aAddr[a] & 0x0ff);
   }
   DBG("BTAddress: "PRIxBTADDR, PRIxBTEXPAND(aAddr));
   iObserver->BluetoothDeviceChosen(address, aName);
   iObserver->BluetoothDeviceChosen(aAddr,   aName);
   DBG("BluetoothDeviceChosen called two times.");
}


void CBtGpsConnector::BtEngineData(const TDesC8& aData)
{
   if(iSprocket) {
      iSprocket->Write(aData);
      DBG("Wrote %d bytes to sprocket", aData.Length());
   } 
   if(iReceptor) {
      iReceptor->Write(aData);
      DBG("Wrote %d bytes to receptor", aData.Length());
   }
   /* Notify GUI that Bluetooth is ok. */
   iObserver->BluetoothStatus(ETrue);   
}

void CBtGpsConnector::BtConnectionLost(TBool aWillTryToReconnect)
{
   iObserver->BluetoothConnectionLost(aWillTryToReconnect);
}


#endif
