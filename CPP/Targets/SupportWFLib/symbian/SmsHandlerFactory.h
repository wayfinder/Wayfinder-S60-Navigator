/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef M_SMS_HANDLER
#define M_SMS_HANDLER

#include <bamdesca.h>
#include <badesca.h>


/**
 * Interface when sending sms. 
 * SmsSent gets called by our sms engines 
 * when sms has been sent.
 */
class MSmsHandler
{
 public:

   virtual ~MSmsHandler();

   virtual void SendSmsL(const MDesCArray& aSmsList,
                         const TDesC& aNumber) = 0;

   virtual void SendSmsL(const TDesC& aSmsText,
                         const TDesC& aNumber) = 0;

   virtual TBool CheckSmsInboxL() = 0;

   virtual CDesCArrayFlat* GetDestinationArray() = 0;

   virtual CDesCArrayFlat* GetSenderArray() = 0;

   virtual void SetSelectedWfSmsL( TInt aIdIdx ) = 0;

   virtual void DeleteSmsL( TInt32 aMsvId ) = 0;
   
   /**
    * Only to be used on wins, sends a sms to the inbox.
    * @param aSmsText, The text in the message.
    * @param aNumber, The number to send to.
    */
   virtual void CreateLocalMessageL(const TDesC& aSmsText,
                                    const TDesC& aNumber) = 0;
};

class SmsHandlerFactory
{
public:
   static MSmsHandler * CreateL(class MSmsHandlerObserver* aObserver);
};

#endif
