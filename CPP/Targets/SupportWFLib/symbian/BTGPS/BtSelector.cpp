/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//this file is not used in UIQ.
#if !defined(NAV2_CLIENT_UIQ) && !defined(NAV2_CLIENT_UIQ3)

#include "BtSelector.h"
#include <bt_sock.h>
#include "SdpAttributeParser.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

const TInt KServiceClass = 0x1101;    //  SerialPort
#ifdef NAV2_CLIENT_SERIES60_V1
static const TInt KSerialPortUUID = 0x1101;
#endif

_LIT(KBtSelector, "BtSelector");
enum TBtSelectorPanic {
   EUnhandledCompletionStatus =  0,
   EBadAttributeID            =  1,
};

#define ASSERT_ALWAYS(c,p) __ASSERT_ALWAYS((c), User::Panic(KBtSelector, (p)));
#define ASSERT_DEBUG(c,p)  __ASSERT_DEBUG((c), User::Panic(KBtSelector, (p)));



CBtSelector::CBtSelector() : 
   CActiveLog(EPriorityStandard, "BtSelector")
{
}

void CBtSelector::ConstructL()
{
   CActiveScheduler::Add(this);
}

CBtSelector::~CBtSelector()
{
   DBG("~BtSelector");
}

class CBtSelector* CBtSelector::NewL()
{
   class CBtSelector* self = CBtSelector::NewLC();
   CleanupStack::Pop(self);
   return self;
}

class CBtSelector* CBtSelector::NewLC()
{
   class CBtSelector* self = new (ELeave) CBtSelector();
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

void CBtSelector::RunL()
{
   DBG("RunL iStatus: %d", iStatus.Int());
   DBGDUMP("iResponse", iResponse.Ptr(), iResponse.Length());
   iNotifier.Close();
   Complete(iStatus);
}

void CBtSelector::DoCancel()
{
   DBG("DoCancel");
   Complete(KErrCancel);
   iNotifier.CancelNotifier(KDeviceSelectionNotifierUid);
}

void CBtSelector::CompleteSelf(TInt aCompletionCode)
{
   class TRequestStatus* status = &iStatus;
   User::RequestComplete(status, aCompletionCode);
}

void CBtSelector::SelectSerialDeviceL(class TRequestStatus* aStatus)
{
   SelectDeviceL(aStatus, KSerialPortUUID);
}

void CBtSelector::SelectDeviceL(class TRequestStatus* aStatus, 
                                const class TUUID& aClass)
{
   DBG("SelectDeviceL");
   //XXX use the class somehow.
   iClass = aClass; //Save for later...
#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1
   enum TBTMajorServiceClass serviceClass = EMajorServiceObjectTransfer;
   class TBTDeviceClass deviceClass((TUint16)serviceClass, 0, 0);
   class TBTDeviceSelectionParams parameters;
   parameters.SetDeviceClass(deviceClass);
   
   iSelectionFilter = TBTDeviceSelectionParamsPckg(parameters);
   iSelectionFilter().SetUUID(TUUID(0x00000001, 0x00001000, 
                                    0x80000002, 0xEE000002));
#elif defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
   iSelectionFilter().SetUUID(KServiceClass);
#else
# error Unsupported architechture.
#endif
   TInt err = iNotifier.Connect();
   DBG("RNotifier::Connect returned %d", err);
   User::LeaveIfError(err);
   Activate(aStatus);
   iNotifier.StartNotifierAndGetResponse( iStatus, KDeviceSelectionNotifierUid,
                                          iSelectionFilter, iResponse );
   SetActive();
   DBG("Selector started.");
}

void CBtSelector::CancelSelection()
{
   DBG("CancelSelection");
   Complete(KErrCancel);
   Cancel();
}

const TDesC& CBtSelector::Name() 
{
   if(iResponse().IsValidDeviceName()){
      return iResponse().DeviceName();
   } else {
      return KNullDesC;
   }
}

class TBTDevAddr CBtSelector::Address() 
{
   if(iResponse().IsValidBDAddr()){
      return iResponse().BDAddr();
   } else {
      return TBTDevAddr();
   }
}

#endif
