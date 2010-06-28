/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SDP_EXAMINER_H
#define SDP_EXAMINER_H
#include "ActiveLog.h"
#include <btsdp.h>
#include <bttypes.h>
#include "Completer.h"
#include "SdpAttributeNotifier.h"

class CSdpExaminer : public CActiveLog,
                     public MCompleter, 
                     public MSdpAgentNotifier,
                     public MSdpAttributeNotifier
{
   /** @name Constructors and Destructor. */
   //@{
   CSdpExaminer(const class TBTDevAddr& aAddress);
   void ConstructL();
public:
   virtual ~CSdpExaminer();
   static class CSdpExaminer* NewL(const class TBTDevAddr& aAddress);
   static class CSdpExaminer* NewLC(const class TBTDevAddr& aAddress);
   //@}
   void FindSerialPortL(class TRequestStatus* aStatus);
   void CancelFind();
   TInt Port() const;
private:
   /** @name From CActive. */
   //@{
   virtual void RunL();
   virtual void DoCancel();
   //@}
   void CompleteSelf(TInt aCompletionCode);
   
   /** @name From MSdpAgentNotifier. */
   //@{
   virtual void NextRecordRequestComplete(TInt aError, 
                                          TSdpServRecordHandle aHandle, 
                                          TInt aTotalRecordsCount);
   virtual void AttributeRequestResult(TSdpServRecordHandle aHandle, 
                                       TSdpAttributeID aAttrID, 
                                       class CSdpAttrValue* aAttrValue);
   
   virtual void AttributeRequestComplete(TSdpServRecordHandle, TInt aError);
   //@}
   /** @name From MSdpAttributeNotifier. */
   //@{
   virtual void FoundElementL(TInt aKey, class CSdpAttrValue& aValue);
   //@}
   class TRequestStatus* iStatusPtr;
   class TBTDevAddr iAddress;
   class CSdpAgent* iAgent;
   class CSdpSearchPattern* iPattern;
   TBool iCanceled;
   /**
    * The RfComm port that should be used to connect to the serial
    * port on the remote device. 
    */
   TUint iRfCommPort;
   /** Indicates whether the iRfCommPort value is valid. */
   TBool iFoundRfCommPort;
};
#endif
