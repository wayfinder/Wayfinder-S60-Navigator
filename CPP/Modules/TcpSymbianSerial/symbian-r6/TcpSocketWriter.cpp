/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TcpSocketWriter.h"
#include "CleanupSupport.h"
#include "TcpSocketEngine.h"
#include "TimeOutTimer.h"
#include "SocketUser.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"
					 
//
// CTcpSymbianWrite: active object wrapping comms write requests
//

// Construction functions
CTcpSymbianWrite::CTcpSymbianWrite(class RSocket& aSocket, 
                                   class MTcpSymbianSocketUser& aConsole,
                                   class CTcpSymbianEngine& aEngine) : 
   CActive(EPriorityStandard), iEchoSocket(aSocket), iConsole(aConsole),
   iEngine(aEngine), iTimeOut(KTimeOut), iWriteStatus(EWaiting)
{
}

class CTcpSymbianWrite* 
CTcpSymbianWrite::NewL(class RSocket& aSocket, 
                       class MTcpSymbianSocketUser& aConsole,
                       class CTcpSymbianEngine& aEngine,
                       class isab::LogMaster* aLogMaster)
{
   class CTcpSymbianWrite* self = NewLC(aSocket, aConsole, aEngine, aLogMaster);
   CleanupStack::Pop(self);
   return self;
}

class CTcpSymbianWrite* 
CTcpSymbianWrite::NewLC(class RSocket& aSocket,
                        class MTcpSymbianSocketUser& aConsole,
                        class CTcpSymbianEngine& aEngine,
                        class isab::LogMaster* aLogMaster)
{
   class CTcpSymbianWrite* self = 
      new (ELeave) CTcpSymbianWrite(aSocket, aConsole, aEngine);
   CleanupStack::PushL(self);
   self->ConstructL(aLogMaster);
   return self;
}

void CTcpSymbianWrite::ConstructL(class isab::LogMaster* aLogMaster)
{
   if(aLogMaster){
      iLog = new (ELeave) isab::Log("TcpSymbianWrite", isab::Log::LOG_ALL, aLogMaster);
   }
   iTimer = CTimeOutTimer::NewL(10, *this);
   CActiveScheduler::Add(this);
   DBG("TcpSymbianWrite fully constructed.");
};

CTcpSymbianWrite::~CTcpSymbianWrite()
{
   delete iTimer;
   delete iBuffer;
   DBG("TcpSymbianWrite fully destroyed.");
   delete iLog;
}

void CTcpSymbianWrite::DoCancel()
   // Cancel asychronous write request
{	
   DBG("DoCancel");
   iEchoSocket.CancelWrite();
   iTimer->Cancel();
   DBG("DoCancel complete");
};

void CTcpSymbianWrite::TimerExpired()
   // Implements MTimeOutNotify: called when timeout expired
{
   WARN("TimerExpired");
   Cancel();
   iWriteStatus = ETimedOut;
   class TRequestStatus* p = &iStatus;		// Signal RunL()
   SetActive();
   User::RequestComplete(p, KErrNone); //no system error
   DBG("Completeing self");
}

void CTcpSymbianWrite::RunL()
{
   DBG("RunL, iStatus: %d, iWriteStatus: %d", iStatus.Int(), iWriteStatus);
   if (iStatus == KErrNone){ //normal state handling
      switch(iWriteStatus){
      case ESending:// Buffer has been written to socket
         iTimer->Cancel(); // Cancel TimeOut timer
         iWriteStatus = EWaiting;
         break;
      case ETimedOut:// Request timed out
         WARN("Write timed out and was canceled.");
         iEngine.Stop(); //reason? XXX
         break;
      default:
         ERR("PANIC KEchoEngine EBadWriteState(%d)", EBadWriteState);
         User::Panic(KEchoEngine, EBadWriteState);
         break;
      };
   } else if(iStatus == KErrEof){
      WARN("Write with KErrEof, connection broken?");
      iEngine.Stop(CTcpSymbianEngine::EBroken);
   }else{
      ERR("PANIC KEchoEngine, EUnhandledWriteStatus(%d)",
          EUnhandledWriteStatus);
      User::Panic(KEchoEngine, EUnhandledWriteStatus);
      ///XXXX
   }
   iConsole.OperationComplete();
}

void CTcpSymbianWrite::IssueWrite(HBufC8* aData)
   // Write a data to a stream socket
{
   //replace buffer;
   delete iBuffer;
   iBuffer = NULL;
   iBuffer = aData;

   iEchoSocket.Write(*iBuffer, iStatus);
   // Request timeout
   iTimer->After(iTimeOut);
   SetActive();
   iWriteStatus = ESending;
   DBG("Async write has started.");
};

