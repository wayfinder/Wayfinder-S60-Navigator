/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TCPSYMBIANREAD_H
#define TCPSYMBIANREAD_H

#include <e32cons.h>
#include <in_sock.h>
#include <nifman.h>
// #include "TimeOutTimer.h"
// #include "TimeOutNotify.h"
#include "Module.h" //needs enums for MTcpSymbianSocketUser

namespace isab {
   class Log;
   class LogMaster;
}

// CTcpSymbianRead: active object wrapping comms read requests
class CTcpSymbianRead : public CActive
{
private:
   CTcpSymbianRead(class RSocket& aSocket, 
                   class MTcpSymbianSocketUser& aConsole,
                   class CTcpSymbianEngine& aEngine);
   void ConstructL(class isab::LogMaster* aLogMaster);
public:
   static class CTcpSymbianRead* NewL(class RSocket& aSocket, 
                                      class MTcpSymbianSocketUser& aConsole,
                                      class CTcpSymbianEngine& aEngine,
                                      class isab::LogMaster* aLogMaster);
   static class CTcpSymbianRead* NewLC(class RSocket& aSocket, 
                                       class MTcpSymbianSocketUser& aConsole,
                                       class CTcpSymbianEngine& aEngine,
                                       class isab::LogMaster* aLogMaster);
   virtual ~CTcpSymbianRead();
   void IssueRead();
   void IssueRecvFrom(TInetAddr &aAddr);

   //Implemented functions from CActive
   void DoCancel();
   void RunL();	
	
private:
   class RSocket& iEchoSocket;
   class MTcpSymbianSocketUser& iConsole;
   class CTcpSymbianEngine& iEngine;
   TBuf8<4096> iBuffer;
   TSockXfrLength iReadAmount; //typedef to TPckgBuf<TInt>
   class isab::Log* iLog;
};

#endif
