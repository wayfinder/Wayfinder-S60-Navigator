/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "SocketConstants.h"
#include "SocketsWrite.h"
#include "TimeOutTimer.h"
#include "sockets.pan"
#include "SymbianTcpConnection.h"

static const TInt KTimeOut = 30000000; // 30 seconds time-out

CNewSocketsWrite* CNewSocketsWrite::NewL(SymbianTcpConnection& aConsole, RSocket& aSocket)
{
   CNewSocketsWrite* self = CNewSocketsWrite::NewLC(aConsole, aSocket);
   CleanupStack::Pop();
   return self;
}

	
CNewSocketsWrite* CNewSocketsWrite::NewLC(SymbianTcpConnection& aConsole, RSocket& aSocket)
{
   CNewSocketsWrite* self = new (ELeave) CNewSocketsWrite(aConsole, aSocket);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}


CNewSocketsWrite::CNewSocketsWrite(SymbianTcpConnection& aConsole, RSocket& aSocket)
: CActive(EPriorityStandard),
  iSocket(aSocket),
  iConsole(aConsole)
{
}


CNewSocketsWrite::~CNewSocketsWrite()
{
   Cancel();
   delete iTimer;
   iTimer = NULL;
}

void CNewSocketsWrite::DoCancel()
{	
   // Cancel asychronous write request
   iSocket.CancelWrite();
   iTimer->Cancel();
}

void CNewSocketsWrite::ConstructL()
{
   CActiveScheduler::Add(this);

   iTimeOut = KTimeOut; 
   iTimer = CTimeOutTimer::NewL(10, *this);
   iWriteStatus = EWaiting;
}

void CNewSocketsWrite::TimerExpired()
{
   Cancel();
   iWriteStatus = ECommsFailed;
   //TBuf<64> message(_L("Error sending data to server, please retry"));
   iConsole.ErrorNotify( socket_errors_errorsending_msg, KErrTimedOut);
}

void CNewSocketsWrite::RunL()
{
   // Active object request complete handler
   if (iStatus == KErrNone){
      switch(iWriteStatus)
      {
      // Character has been written to socket
      case ESending:
         SendNextPacket();
         break;
      default:
         User::Panic(KPanicSocketsEngineWrite, ESocketsBadStatus);
         break;
      };
   }
   else{
      // Error: pass it up to user interface
      iTimer->Cancel();
      //TBuf<64> message(_L("Error sending data to server, please retry"));
      iConsole.ErrorNotify( socket_errors_errorsending_msg, iStatus.Int());
      iWriteStatus = ECommsFailed;
   }
}

void CNewSocketsWrite::IssueWriteL(const TDesC8& aData)
{
   if ((iWriteStatus != EWaiting) && (iWriteStatus != ESending)){
      User::Leave(KErrNotReady);
   }

   // Write data to a stream socket
   if ((aData.Length() + iTransferBuffer.Length()) > iTransferBuffer.MaxLength()){
      // Not enough space in buffer
      User::Leave(KErrOverflow);
   }

   // Add new data to buffer
   iTransferBuffer.Append(aData);

   if (!IsActive()){
      SendNextPacket();
   }
}

void CNewSocketsWrite::SendNextPacket()
{
   iTimer->Cancel(); // Cancel TimeOut timer
   iWriteStatus = EWaiting;

   if (iTransferBuffer.Length() > 0){
      // Move data from transfer buffer to actual write buffer
      iWriteBuffer = iTransferBuffer;
      iTransferBuffer.Zero();
      iSocket.Write(iWriteBuffer, iStatus); // Initiate actual write

      // Request timeout
      iTimer->After(iTimeOut);
      SetActive();
      iWriteStatus = ESending;
   }
}
