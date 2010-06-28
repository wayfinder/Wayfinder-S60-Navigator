/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#include "BtSocketsRead.h"
#include "BtSocketsPeer.h"
#include "BtHciErrors.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

#define MILLISECONDS(ms) ((ms)*1000)
#define CENTISECONDS(cs) MILLISECONDS((cs)*10)
#define DECISECONDS(ds) CENTISECONDS((ds)*10)
#define SECONDS(s) DECISECONDS((s)*10)
#define KReadPeriod CENTISECONDS(15)

class CBtSocketsRead* 
CBtSocketsRead::NewL(class MBtSocketsPeer& aPeer, 
                     class RSocket& aSocket)
{
   class CBtSocketsRead* self = CBtSocketsRead::NewLC(aPeer, aSocket);
   CleanupStack::Pop();
   return self;
}

	
class CBtSocketsRead* 
CBtSocketsRead::NewLC(class MBtSocketsPeer& aPeer, 
                      class RSocket& aSocket)
{
   class CBtSocketsRead* self = new (ELeave) CBtSocketsRead(aPeer, aSocket);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}


CBtSocketsRead::CBtSocketsRead(class MBtSocketsPeer& aPeer,
                               class RSocket& aSocket) : 
   CActiveLog(EPriorityStandard, "BtSocketsRead"), 
   iSocket(aSocket),
   iSocketPeer(aPeer)
{
}

void CBtSocketsRead::ConstructL()
{
   User::LeaveIfError(iTimer.CreateLocal());
   CActiveScheduler::Add(this);
}

CBtSocketsRead::~CBtSocketsRead()
{
   if(IsActive()){
      DBG("Have to cancel in destructor....");
      Cancel();
      DBG("...destructor cancel complete.");
   }
   iTimer.Close();
}

void CBtSocketsRead::CancelRead()
{
   DBG("CancelRead");
   iCancel = ETrue;
}

void CBtSocketsRead::DoCancel()
{
   // Cancel asychronous read request
   DBG("DoCancel");
   if(iReaderState == EBtSockRead){
      iSocket.CancelRead();
   } else {
      iTimer.Cancel();
   }
   DBG("DoCancel complete");
}

void CBtSocketsRead::RunL()
{
   // Active object request complete handler
   if (iStatus == KErrNone){
      if(iCancel) {
         DBG("iCancel set, will stop reading now.");
         // XXX Stop reading
         iSocket.CancelRead(); //XXX do we have an active read here?
         iTimer.Cancel();
         iSocketPeer.ConnectionCancelled();
      } else {
         if(iReaderState == EBtSockRead){
            DBG("Read %d bytes", iBuffer.Length());
            // Character has been read from socket
            iSocketPeer.ReceiveMessageL( iBuffer );
            IssueTimer();
         } else { //iReaderState == EBtSockWait
            IssueRead();
         }
      }
   } else if(iStatus == KErrDisconnected){
      DBG("Socket disconnected.");
      // notify parent
      iSocketPeer.ConnectionLost();
   } else if(iStatus == KHCIHardwareFail) {
      DBG("Socket disconnected, probably no phone battery left.");
      iSocketPeer.ErrorNotify(EBtSocketsReadHardwareFail, iStatus.Int());
      //since it seems the KHCIHardwareFail error can occur at other
      //times than low battery we will try to reconnect on this error
      //as well for now.
      iSocketPeer.ConnectionLost(/*EFalse*/); 
   } else {
      ERR("iStatus == %d", iStatus.Int());
      // Error: pass it up to user interface
      iSocketPeer.ErrorNotify(EBtSocketsReadError, iStatus.Int());
      iSocketPeer.ConnectionLost();
   }	
}


void CBtSocketsRead::IssueRead()
{
   if (!IsActive()){
      iSocket.RecvOneOrMore( iBuffer, 0, iStatus, iReadLength );
      SetActive();
      iReaderState = EBtSockRead;
      DBG("RecvOneOrMore activated");
   } else {
      WARN("Read issued while active!");
   }
}

void CBtSocketsRead::IssueTimer()
{
   if(!IsActive()){
      DBG("Starting timer %u", unsigned(KReadPeriod));
      iTimer.After(iStatus, KReadPeriod);
      SetActive();
      iReaderState = EBtSockWait;
   } else {
      WARN("Timer issued while active!");
   }
}

void CBtSocketsRead::Start()
{
   // Initiate a new read from socket into iBuffer
   IssueRead();
}
 
