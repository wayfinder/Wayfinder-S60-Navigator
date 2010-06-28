/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BtSocketsEngine.h"
#include "BtSocketsRead.h"
#include "BtSocketsWrite.h"
#include "Log.h"

#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

_LIT(KRfComm, "RFCOMM");
_LIT(KBtSocketsEngine, "BtSockEngine");
enum TBtSocketsEnginePanics {
   EDestroyedWhileDisconnecting = 0,
};

CBtSocketsEngine::CBtSocketsEngine(class MBtSocketsPeer& aPeer) : 
   CActiveLog(EPriorityStandard, "BtSocketsEngine"), 
   iState(ENotConnected), iPeer(aPeer)
{
}

void CBtSocketsEngine::ConstructL(const class TBTDevAddr& aAddress, TInt aPort)
{
   CActiveScheduler::Add(this);
   User::LeaveIfError(iSocketServ.Connect());

   iSocketsRead = CBtSocketsRead::NewL(iPeer, iSocket);
   iSocketsWrite = CBtSocketsWrite::NewL(iPeer, iSocket);
   UpdateLogMastersL();

   DBG("Initialized with address "PRIxBTADDR" and port %d", 
       PRIxBTEXPAND(aAddress), aPort);

   iAddress.SetBTAddr(aAddress);
   iAddress.SetPort(aPort);
}

class CBtSocketsEngine* CBtSocketsEngine::NewL(class MBtSocketsPeer& aPeer,
                                               const class TBTDevAddr& aAddr,
                                               TInt aPort)
{
   class CBtSocketsEngine* self = CBtSocketsEngine::NewLC(aPeer, aAddr, aPort);
   CleanupStack::Pop(self);
   return self;
}

class CBtSocketsEngine* CBtSocketsEngine::NewLC(class MBtSocketsPeer& aPeer,
                                                const class TBTDevAddr& aAddr,
                                                TInt aPort)
{
   class CBtSocketsEngine* self = new (ELeave) CBtSocketsEngine(aPeer);
   CleanupStack::PushL(self);
   self->ConstructL(aAddr, aPort);
   return self;
}

CBtSocketsEngine::~CBtSocketsEngine()
{
   if(IsActive() && iState == EConnecting){
      DBG("Destructing while connecting");
      Cancel();
   } else if(iState == EConnected){
      DBG("Destructing while connected");
      Disconnect();
      DBG("Disconnect done");
   } else if(iState == EDisconnecting){
      DBG("Destructing while disconnecting");
      //XXX is this right?
      User::Panic(KBtSocketsEngine, EDestroyedWhileDisconnecting);
      DBG("Disconnect done");
   }
   delete iSocketsWrite;
   delete iSocketsRead;
   iSocketServ.Close();
   DBG("SocketsEngine destroyed.");
}

void CBtSocketsEngine::CancelAll()
{
   if(iState == EConnecting){
      CancelConnect();
   } else if(iState == EDisconnecting){
      CancelDisconnect();
   } else {
      Complete(KErrCancel); //should be safe....
   }
}

void CBtSocketsEngine::ConnectL(class TRequestStatus* aStatus)
{
   DBG("ConnectL");
   iState = EConnecting;
   User::LeaveIfError( iSocket.Open( iSocketServ, KRfComm ) );
   iSocket.Connect( iAddress, iStatus );
   SetActive();
   Activate(aStatus);
}

void CBtSocketsEngine::CancelConnect()
{
   if(iState == EConnecting){
      Complete(KErrCancel);
      iSocket.CancelConnect();
   } else {
      WARN("Unable to cancelconnect, not connecting");
   }
}

TBool CBtSocketsEngine::DisconnectInternal(class TRequestStatus& aStatus)
{
   DBG("Disconnect common");
   TBool ret = EFalse;
   if((ret = IsConnected())){
      iState = EDisconnecting;
      iSocketsRead->CancelRead();
      iSocketsRead->Cancel();
      iSocketsWrite->Cancel();
      iSocket.Shutdown(RSocket::EImmediate, aStatus);
   }
   DBG("Was %sconnected", ret ? "" : "not ");
   return ret;
}   

void CBtSocketsEngine::Disconnect()
{
   DBG("Disconnect synchronous");
   class TRequestStatus status = KRequestPending;
   if(DisconnectInternal(status)){
      DBG("Shutdown and then waitforrrequest.");
      User::WaitForRequest(status);
      DBG("I'm done waiting!");
      iState = ENotConnected;
      iSocket.Close();
   }      
}

//things get to complicated with asynchronous disconnects. 
void CBtSocketsEngine::Disconnect(class TRequestStatus* aStatus)
{
   DBG("Disconnect %p", aStatus);
   TBool disconnecting = DisconnectInternal(iStatus);
   Activate(aStatus); //handles NULL pointer as NOP.
   SetActive();
   
   if(disconnecting){
      DBG("Set as active, will RunL when disconnected.");
   } else {
      DBG("Not connected, complete immediately with KErrNone");
      //complete self since we do not know if the caller has SetActive
      //yet.
      class TRequestStatus* status = &iStatus;
      User::RequestComplete(status, KErrNone); //XXX other status code?
   }
}

void CBtSocketsEngine::CancelDisconnect()
{
   if(iState == EDisconnecting){
      Complete(KErrCancel);
      WARN("Cancel while disconnecting. Complete request with KErrCancel, "
           "but do nothing with the socket");
   } else {
      WARN("Unable to CancelDisconnect, not disconnecting");
   }
}

void CBtSocketsEngine::WriteL(const TDesC8& aData)
{
   iSocketsWrite->IssueWriteL(aData);
}

void CBtSocketsEngine::Read()
{
   iSocketsRead->Start();
}

TBool CBtSocketsEngine::IsConnected() const
{
   return iState == EConnected;
}

void CBtSocketsEngine::DoCancel()
{
   Complete(KErrCancel);
   if(iState == EConnecting){
      DBG("DoCancel while connecting");
      iSocket.CancelConnect();
   }
   if(iState == EDisconnecting){
      DBG("DoCancel while disconnecting");
      //XXX we don't want to cancel this op, but we probably have to
      //do something.
   }
}

void CBtSocketsEngine::RunL()
{
   Complete(iStatus); //complete waiting object with same status
   if(iStatus == KErrNone){
      if(iState == EConnecting){
         DBG("Connect complete");
         iState = EConnected;
         Read();
      } else if(iState == EDisconnecting){
         DBG("Disconnect complete");
         iState = ENotConnected;
         iSocket.Close();
         DBG("SocketsReader is %sactive, SocketsWriter is %sactive",
             iSocketsRead->IsActive()  ? "" : "not ",
             iSocketsWrite->IsActive() ? "" : "not ");
      } else {
         DBG("Disconnect while not connected.");
      }
   } else {
      DBG("Op failed %d, setting state to ENotConnected", iStatus.Int());
      iState = ENotConnected;
   }
}

CArrayPtr<CActiveLog>* CBtSocketsEngine::SubLogArrayLC()
{
   CArrayPtr<CActiveLog>* logs = new (ELeave) CArrayPtrFlat<CActiveLog>(2);
   CleanupStack::PushL(logs);
   logs->AppendL(iSocketsWrite);
   logs->AppendL(iSocketsRead);
   return logs;
}

