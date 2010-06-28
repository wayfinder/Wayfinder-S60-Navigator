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
#include "SocketsRead.h"
#include "SymbianTcpConnection.h"

class CNewSocketsRead* 
CNewSocketsRead::NewL(class SymbianTcpConnection& aAppUI, 
                      class RSocket& aSocket)
{
   class CNewSocketsRead* self = CNewSocketsRead::NewLC(aAppUI, aSocket);
   CleanupStack::Pop();
   return self;
}

	
class CNewSocketsRead* 
CNewSocketsRead::NewLC(class SymbianTcpConnection& aAppUI, 
                       class RSocket& aSocket)
{
   class CNewSocketsRead* self = new (ELeave) CNewSocketsRead(aAppUI, aSocket);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}


CNewSocketsRead::CNewSocketsRead(class SymbianTcpConnection& aAppUI, 
                                 class RSocket& aSocket) : 
   CActive(EPriorityStandard),
   iSocket(aSocket),
   iAppUI(aAppUI)
{
}


CNewSocketsRead::~CNewSocketsRead()
{
   Cancel();
}

void CNewSocketsRead::CancelRead()
{
   iCancel = ETrue;
}


void CNewSocketsRead::ConstructL()
{
   iCancel = EFalse;
   CActiveScheduler::Add(this);
}

void CNewSocketsRead::DoCancel()
{
   // Cancel asychronous read request
   iSocket.CancelRead();
}

void CNewSocketsRead::RunL()
{
   // Active object request complete handler
   if (iStatus == KErrNone){
      if(iCancel) {
         // XXX Stop reading
         iSocket.CancelRead();
         iAppUI.ConnectionCancelled();
      } else {
         // Character has been read from socket
         iAppUI.ReceiveMessageL( iBuffer, iBuffer.Length() );
         IssueRead();
      }
   } else {
      // Error: pass it up to user interface
      //TBuf<64> message(_L("Error while reading data from server, please retry"));
      iAppUI.ErrorNotify( socket_errors_errorreading_msg, iStatus.Int());
   }	
}


void CNewSocketsRead::IssueRead()
{
   if (!IsActive()){
      iSocket.RecvOneOrMore( iBuffer, 0, iStatus, iReadLength );
      SetActive();
   }
}

void CNewSocketsRead::Start()
{
   // Initiate a new read from socket into iBuffer
   IssueRead();
}
