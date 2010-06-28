/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TcpSocketReader.h"
#include "TcpSocketEngine.h"
#include "SocketUser.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

CTcpSymbianRead::CTcpSymbianRead(class RSocket& aSocket, 
                                  class MTcpSymbianSocketUser& aConsole,
                                  class CTcpSymbianEngine& aEngine) : 
   CActive(EPriorityStandard), iEchoSocket(aSocket), iConsole(aConsole),
   iEngine(aEngine)
{
}

class CTcpSymbianRead* CTcpSymbianRead::NewL(class RSocket& aSocket, 
                                 class MTcpSymbianSocketUser& aConsole,
                                 class CTcpSymbianEngine& aEngine,
                                      class isab::LogMaster* aLogMaster)
{
   class CTcpSymbianRead* self = NewLC(aSocket, aConsole, aEngine, aLogMaster);
   CleanupStack::Pop(self);
   return self;
}

class CTcpSymbianRead* CTcpSymbianRead::NewLC(class RSocket& aSocket, 
                                              class MTcpSymbianSocketUser& aConsole,
                                              class CTcpSymbianEngine& aEngine,
                                              class isab::LogMaster* aLogMaster)
{
   class CTcpSymbianRead* self = 
      new (ELeave) CTcpSymbianRead(aSocket, aConsole, aEngine);
   CleanupStack::PushL(self);
   self->ConstructL(aLogMaster);
   return self;
}

void CTcpSymbianRead::ConstructL(class isab::LogMaster* aLogMaster)
{
   if(aLogMaster){
      iLog = new (ELeave) isab::Log("TcpSymbianRead", isab::Log::LOG_ALL, aLogMaster);
   }
   CActiveScheduler::Add(this);
}

CTcpSymbianRead::~CTcpSymbianRead()
{
   DBG("~CTcpSymbianRead");
   delete iLog;
}

void CTcpSymbianRead::DoCancel()
{
   DBG("DoCancel, Calling iEchoSocket.CancelRead");
   iEchoSocket.CancelRead();
}

void CTcpSymbianRead::RunL()
{
   DBG("RunL, iStatus: %d", iStatus.Int());
   if (iStatus == KErrNone){
      //TRACE_DBG( "KErrNone - iConsole.ReceiveData\n%*s", iBuffer.Length(), iBuffer.Ptr() );
      // Character has been read from socket
      iConsole.ReceiveData(iBuffer);
      IssueRead();
   } else if(iStatus == KErrTimedOut){
      WARN("Read timed out, try again");
      //TRACE_DBG( "KErrTimedOut" );
      IssueRead();
   } else if(iStatus == KErrDisconnected){
      WARN("Disconnected. This should never happen");
      //TRACE_DBG( "Disconnected. This should never happen" );
      iEngine.Stop(CTcpSymbianEngine::EBroken);
   } else if ( iStatus == KErrEof ) {
      DBG("Read encountered EErrEof, connection closed by other.");
      //TRACE_DBG("Read encountered EErrEof, connection closed by other.");
      iEngine.Stop( CTcpSymbianEngine::EClosed );
   } else if ( iStatus == KErrAbort) {
      DBG("Read encountered KErrAbort, connection probably broken");
      //TRACE_DBG("Read encountered KErrAbort, connection probably broken");
      iEngine.Stop( CTcpSymbianEngine::EBroken ); 
#ifdef PANIC_ON_ALL_UNKNOWN_ERRORS
   } else {
      ERR("PANIC KEchoEngine EUnhandledReadStatus(%d)", iStatus.Int());
      User::Panic(KEchoEngine, EUnhandledReadStatus);
#else
   } else {
      DBG("Read encountered error, connection probably broken");
      //TRACE_DBG("Read encountered error, connection probably broken. iStatus: %d", iStatus.Int() );
      iEngine.Stop(CTcpSymbianEngine::EBroken);
#endif
      ///XXX handle the different cases
      // Error: pass it up to user interface
      //      _LIT(KCTcpSymbianReadError,"\nCTcpSymbianRead error");
      //      iConsole.ConnectionNotify(KCTcpSymbianReadError);
   }	
}

void CTcpSymbianRead::IssueRead()
{
   if (!IsActive()){
      iEchoSocket.RecvOneOrMore(iBuffer, 0, iStatus, iReadAmount);
      SetActive();
   } else {
      DBG("IssueRead while active");
   }
}
