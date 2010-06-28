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
#ifndef NAV2_CLIENT_UIQ
#include "SdpExaminer.h"
#include <bt_sock.h>
#include "SdpAttributeParser.h"
#include "BtHciErrors.h"
#include "Log.h"

#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

static const TInt KServiceClass = 0x1101;    //  SerialPort
_LIT(KSdpExaminer, "SdpExaminer");
enum TSdpExaminerPanics {
   ENoAddressSet   =  0,
   EBadAttributeID =  1,
};

#define ASSERT_ALWAYS(c,p) \
      __ASSERT_ALWAYS((c), User::Panic(KSdpExaminer, (p)))
#define ASSERT_DEBUG(c,p)  \
      __ASSERT_DEBUG((c),  User::Panic(KSdpExaminer, (p)))


CSdpExaminer::CSdpExaminer(const class TBTDevAddr& aAddress) : 
   CActiveLog(EPriorityStandard, "SdpExaminer"), iAddress(aAddress)
{
}

void CSdpExaminer::ConstructL()
{
   CActiveScheduler::Add(this);
}

CSdpExaminer::~CSdpExaminer()
{
   DBG("~CSdpExaminer");
   if(IsActive()){
      Cancel();
   }
   delete iAgent;
   delete iPattern;
   DBG("destructor done");
}

class CSdpExaminer* CSdpExaminer::NewL(const class TBTDevAddr& aAddress)
{
   class CSdpExaminer* self = CSdpExaminer::NewLC(aAddress);
   CleanupStack::Pop(self);
   return self;
}

class CSdpExaminer* CSdpExaminer::NewLC(const class TBTDevAddr& aAddress)
{
   class CSdpExaminer* self = new (ELeave) CSdpExaminer(aAddress);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

void CSdpExaminer::FindSerialPortL(class TRequestStatus* aStatus)
{
   delete iAgent;
   iAgent = NULL;
   delete iPattern;
   iPattern = NULL;

   DBG("Find the serial port of "PRIxBTADDR, PRIxBTEXPAND(iAddress));
   DBG("Compare to "PRIxBTADDR, PRIxBTEXPAND(TBTDevAddr()));
   DBG("Comparison %d", int(iAddress != TBTDevAddr()));
   ASSERT_ALWAYS(iAddress != TBTDevAddr(), ENoAddressSet);

   iAgent = CSdpAgent::NewL(*this, iAddress);
   iPattern = CSdpSearchPattern::NewL();
   iPattern->AddL(KServiceClass); //always serial port.
   iAgent->SetRecordFilterL(*iPattern);
   iAgent->NextRecordRequestL(); //leaves with KErrNotSupported in emulator

   DBG("Activate %p", aStatus);
   Activate(aStatus);
   iStatus = KRequestPending;
   iStatusPtr = &iStatus;
   SetActive();
   DBG("Has been set as active.");
}

void CSdpExaminer::CancelFind()
{
   Complete(KErrCancel);
   iCanceled = ETrue;
}


TInt CSdpExaminer::Port() const
{
   return iRfCommPort;
}

void CSdpExaminer::RunL()
{
   if(iCanceled){
      DBG("Cancelled");
      Complete(KErrCancel);
   } else {
      DBG("Anything but Canceled: %d", int(iStatus.Int()));
      switch(iStatus.Int()){
      case KErrL2CAPRequestTimeout: //?
      case KHCIPageTimedOut:        //device not in range ?
         iStatus = KErrTimedOut;
         break;
      case KHCIHardwareFail:  //?
         iStatus = KErrHardwareNotAvailable;
      }
      Complete(iStatus);
   }
}

void CSdpExaminer::DoCancel()
{
   DBG("DoCancel");
   iCanceled = ETrue;
   Complete(KErrCancel);
   CompleteSelf(KErrCancel);
}

void CSdpExaminer::CompleteSelf(TInt aCompletionCode)
{
   if(IsActive()){
      DBG("CompleteSelf %d", int(aCompletionCode));
      User::RequestComplete(iStatusPtr, aCompletionCode);
   }
}

void CSdpExaminer::NextRecordRequestComplete(TInt aError, 
                                             TSdpServRecordHandle aHandle, 
                                             TInt aTotalRecordsCount)
{
   DBG("RecReqCompl(%d, %"PRIx32", %d)", aError, aHandle, aTotalRecordsCount);
   if(iCanceled){
      CompleteSelf(KErrCancel);
   } else if(aError == KErrEof){
      //end of records.
      CompleteSelf(KErrNotFound);
   } else if(aError == KErrNone){
      //received record
      if(aTotalRecordsCount == 0){
         CompleteSelf(KErrNotFound);
      } else {
         TRAPD(err, 
               iAgent->AttributeRequestL(aHandle, 
                                         KSdpAttrIdProtocolDescriptorList));
         if(err != KErrNone){
            CompleteSelf(err);
         }
      }
   } else {
      //bad
      CompleteSelf(aError);
   }
}

static const TInt KRfcommChannel = 1;

static const struct TSdpAttributeParser::SSdpAttributeNode 
KSerialPortProtocolArray[] = {
   { TSdpAttributeParser::ECheckType, ETypeDES },
      { TSdpAttributeParser::ECheckType, ETypeDES },
         { TSdpAttributeParser::ECheckValue, ETypeUUID, KL2CAP },
      { TSdpAttributeParser::ECheckEnd },
      { TSdpAttributeParser::ECheckType, ETypeDES },
         { TSdpAttributeParser::ECheckValue, ETypeUUID, KRFCOMM },
         { TSdpAttributeParser::EReadValue, ETypeUint, KRfcommChannel },
      { TSdpAttributeParser::ECheckEnd },
   { TSdpAttributeParser::ECheckEnd },
   { TSdpAttributeParser::EFinished }
};

static const unsigned KProtocolArraySize = 
sizeof(KSerialPortProtocolArray)/sizeof(*KSerialPortProtocolArray);

void CSdpExaminer::AttributeRequestResult(TSdpServRecordHandle aHandle, 
                                          TSdpAttributeID aAttrID, 
                                          class CSdpAttrValue* aAttrValue)
{
   DBG("AttrReqRes(%"PRIx32", %"PRIx16", %p)", aHandle, aAttrID, aAttrValue);
   if(!iCanceled){
      ASSERT_ALWAYS(aAttrID == KSdpAttrIdProtocolDescriptorList, 
                    EBadAttributeID);
      
      if(!iFoundRfCommPort){ 
         //port not found, we need to parse
         class TSdpAttributeParser parser(KSerialPortProtocolArray,
                                          KProtocolArraySize, *this);
         // Validate the attribute value, and extract the RFCOMM channel
         TInt ret = parser.Parse(*aAttrValue);
         if(ret == KErrNone && parser.ParseComplete()){
            //parse was complete and error free!
            iFoundRfCommPort = ETrue;
         }
      }
   }
   //next attribute requested automagically!
}

void CSdpExaminer::AttributeRequestComplete(TSdpServRecordHandle aHandle,
                                            TInt aError)
{
   DBG("AttrReqCompl(%"PRIx32", %d)", aHandle, aError);
   if(iCanceled){
      CompleteSelf(KErrCancel);
   } else if(aError != KErrNone){
      CompleteSelf(aError);
   } else if(iFoundRfCommPort){
      //done, no need to search any more.
      CompleteSelf(KErrNone);
   } else {
      //keep searching
      TRAPD(err, iAgent->NextRecordRequestL());
      if(err != KErrNone){
         CompleteSelf(err);
      }
   }
}

void CSdpExaminer::FoundElementL(TInt aKey, class CSdpAttrValue& aValue)
{
   if(aKey == KRfcommChannel){ //id from the list declared above
      DBG("RfCommPort %d found", aValue.Uint());
      iRfCommPort = aValue.Uint();
   }
}

#endif
