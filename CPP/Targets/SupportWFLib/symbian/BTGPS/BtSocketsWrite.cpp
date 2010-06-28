/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#include "BtSocketsWrite.h"
#include <badesca.h>
#include "TimeOutTimer.h"
#include "BtSocketsPeer.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

static const TInt KTimeOut = 30000000; // 30 seconds time-out
_LIT(KPanicSocketsEngineWrite, "BtSocketWrite");
enum TBtSocketsWritePanics {
   ESocketsBadStatus =  0,
};

class CBtSocketsWrite* CBtSocketsWrite::NewL(class MBtSocketsPeer& aConsole, 
                                             class RSocket& aSocket)
{
   class CBtSocketsWrite* self = CBtSocketsWrite::NewLC(aConsole, aSocket);
   CleanupStack::Pop();
   return self;
}

	
class CBtSocketsWrite* CBtSocketsWrite::NewLC(class MBtSocketsPeer& aConsole,
                                              class RSocket& aSocket)
{
   class CBtSocketsWrite* self = 
      new (ELeave) CBtSocketsWrite(aConsole, aSocket);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}


CBtSocketsWrite::CBtSocketsWrite(class MBtSocketsPeer& aConsole,
                                 class RSocket& aSocket) :
   CActiveLog(EPriorityStandard, "BtSocketsWrite"), 
   iSocket(aSocket), iConsole(aConsole),    
   iWriteStatus(EWaiting)
{
}

CBtSocketsWrite::~CBtSocketsWrite()
{
   if(IsActive()){
      DBG("Have to cancel in destructor....");
      Cancel();
      DBG("...destructor cancel complete.");
   }
   delete iTimer;
   iTimer = NULL;
   delete iDataQueue;
}

void CBtSocketsWrite::DoCancel()
{
   DBG("DoCancel");
   // Cancel asychronous write request
   if(iWriteStatus == ESending){
      iSocket.CancelWrite();
   }
   iTimer->Cancel();
   DBG("DoCancel complete");
}

void CBtSocketsWrite::ConstructL()
{
   CActiveScheduler::Add(this);
   iDataQueue = new (ELeave) CDesC8ArraySeg(4);

   iTimer = CTimeOutTimer::NewL(10, *this);
}

void CBtSocketsWrite::TimerExpired()
{
   DBG("Timer has expired, canceling...");
   Cancel();
   DBG("...timeout cancel complete");
   iWriteStatus = ECommsFailed;
   //TBuf<64> message(_L("Error sending data to server, please retry"));
   iConsole.ErrorNotify(EBtSocketsWriteTimeout, KErrTimedOut);
}

void CBtSocketsWrite::RunL()
{
   // Active object request complete handler
   if (iStatus == KErrNone){
      switch(iWriteStatus){
      // Character has been written to socket
      case ESending:
         SendNextPacket();
         break;
      default:
         ERR("RunL KErrNone with status %d", int(iWriteStatus));
         User::Panic(KPanicSocketsEngineWrite, ESocketsBadStatus);
         break;
      };
   }
   else{
      ERR("Error %d", iStatus.Int());
      // Error: pass it up to user interface
      iTimer->Cancel();
      //TBuf<64> message(_L("Error sending data to server, please retry"));
      iConsole.ErrorNotify( EBtSocketsWriteError, iStatus.Int());
      iWriteStatus = ECommsFailed;
   }
}

void CBtSocketsWrite::IssueWriteL(const TDesC8& aData)
{
   if (iWriteStatus == ECommsFailed){
      ERR("Write issued on bad object");
      User::Leave(KErrNotReady);
   }

   TPtrC8 data = aData.Left(KMaxTInt);
   while(data.Length() > 0){
      iDataQueue->AppendL(data.Left(KMaxMessageLength));
      if(data.Length() > KMaxMessageLength){
         data.Set(data.Mid(KMaxMessageLength));
      } else {
         data.Set(KNullDesC8);
      }
   }

   DBG("After append of %d bytes, the queue is %d packets long", 
       aData.Length(), iDataQueue->MdcaCount());

   SendNextPacket();
}

void CBtSocketsWrite::SendNextPacket()
{
   if(!IsActive()){
      iTimer->Cancel(); // Cancel TimeOut timer
      iWriteStatus = EWaiting;
      
      if (iDataQueue->Count() > 0){
         // Move data from transfer buffer to actual write buffer
         iWriteBuffer = iDataQueue->MdcaPoint(0);
         iDataQueue->Delete(0);
         
         // Initiate actual write
         iSocket.Write(iWriteBuffer, iStatus);
         iWriteStatus = ESending;
         SetActive();
         
         // Request timeout
         iTimer->After(KTimeOut);
      }
   }
}

