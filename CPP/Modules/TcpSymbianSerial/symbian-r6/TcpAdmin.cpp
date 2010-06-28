/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "TcpSymbianSerial.h"

#include "TcpAdmin.h"
#include "WFTextUtil.h"

#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

namespace {
   _LIT(KTcpAdmin, "TcpAdmin");
   enum TTcpAdminPanic {
      EBadHost         = 0,
      EWrongThread     = 1,
      EAssertIsThread  = 2,
      EAssertNotThread = 3,
   };
}

#define ASSERT_IS_THREAD(thread, panic) ((IsThread(thread))||(User::Panic(KTcpAdmin, panic),0))
#define ASSERT_NOT_THREAD(thread, panic) ((!IsThread(thread))||(User::Panic(KTcpAdmin, panic),0))

CTcpAdmin::CTcpAdmin(class isab::TcpSymbianSerial* aTss, class isab::LogMaster* aLogMaster) : 
   iTss(aTss), iLogMaster(aLogMaster), iIAP(-2)
{
}

CTcpAdmin::~CTcpAdmin()
{
   DBG("~CTcpAdmin");
   //XXX maybe already done?
   iSocketsEngine->Cancel();
   iCritical.Close();
   delete iSocketsEngine;
   delete iGenerator;
   DBG("CTcpAdmin destroyed.");
   delete iLog;
}

void CTcpAdmin::StartL()
{
   if(iLogMaster && !iLog){
      iLog = new (ELeave) isab::Log("TcpAdmin", isab::Log::LOG_ALL, iLogMaster);
   }
   DBG("Thread started.");
   class RThread thisThread;
   iThreadId = thisThread.Id();
   User::LeaveIfError(iCritical.CreateLocal());
   iGenerator = CTcpEventGenerator::NewL(*this, iThreadId, iLog ? iLog->getLogMaster() : NULL);
   iSocketsEngine = CTcpSymbianEngine::NewL(*this, iLog ? iLog->getLogMaster() : NULL);
   DBG("All objects constructed.");
}

void CTcpAdmin::StopL()
{
   DBG("Stopping thread");
   iSocketsEngine->Cancel();
   iCritical.Close();
   DBG("All is canceled and closed.");
}


void CTcpAdmin::ReceiveData(const TDesC8& aData)
{
   DBG("Receiving %d bytes of read data", aData.Length());
   ASSERT_IS_THREAD(iThreadId, EWrongThread);
   iTss->receive(aData.Ptr(), aData.Length());
}

TInt CTcpAdmin::ConnectionControl(enum isab::Module::ConnectionCtrl aControl, 
                                  const char* aMethod)
{
   DBG("ConnectionControl(%d, %s)", int(aControl), aMethod);
   ASSERT_NOT_THREAD(iThreadId, EWrongThread);
   if(!aMethod) User::Panic(_L("Method"), 0);
   //This function should only ever be called from outside the thread
   class CTcpEvent* event = CTcpConnectionEvent::NewLC(aControl, aMethod);
   TInt queued = iGenerator->SendEventL(event); ///XXX leave safe?
   DBG("%d events queued", queued);
   CleanupStack::Pop(event);
   return queued;
   //   return iConnCtrl->Connection(aControl, aMethod);
}

TInt CTcpAdmin::HandleConnectionEventL(class CTcpConnectionEvent& aEvent) 
{
   return SocketsConnectL(aEvent.Action(), aEvent.Method());
}

TInt CTcpAdmin::HandleDataEventL(class CTcpDataEvent& aEvent)
{
   return WriteToTcp(aEvent.ReleaseData());
}

TInt CTcpAdmin::HandleRunError(TInt aError)
{
   //only SocketsConnectL can make us end up here....
   //... and only CTcpSocketsEngine::ConnectL can leave ...
   //... and only with KErrNoMem in Symbian 9.
   ERR("HandleRunError %d", aError);
   ConnectionNotify(isab::Module::DISCONNECTING, 
                    isab::Module::METHOD_ERROR);
   return KErrNone;
}

 
TInt CTcpAdmin::SocketsConnectL(enum isab::Module::ConnectionCtrl aAction,
                                const TDesC8* aMethod)
{
   DBG("SocketsConnectL(%d, ?)", int(aAction));
   ASSERT_IS_THREAD(iThreadId, EWrongThread);
   TInt ret = KErrNone;
   switch(aAction){
   case isab::Module::CONNECT:
      {
         DBG("CONNECT");
         ///if aMethod is NULL we do an allocation we don't need. As
         ///this should never happen it's not really a problem.
         HBufC8* server = HBufC8::NewLC(aMethod ? aMethod->Length() : 4);
         TPtr8 host = server->Des();
         TUint port = 0;
         if(aMethod && WFTextUtil::ParseHost(*aMethod, host, port)){
            DBG("Host succesfully parsed");
            //LeaveIfError(RConnect::Open) in here
            iSocketsEngine->ConnectL(host, port, ConnectionParam(),
                                     iConsiderWLAN); //active request
         } else {
            ret = KErrArgument;
            WARN("Host was not parsed, DISCONNECTING with INTERNAL_ERROR");
            ConnectionNotify(isab::Module::DISCONNECTING, 
                             isab::Module::METHOD_ERROR);
            ///XXX do we want to do this?
            if(iSocketsEngine->IsConnected()){
               WARN("Stopping SocketsEngine with ELookUpFailed.");
               iSocketsEngine->Stop(CTcpSymbianEngine::ELookUpFailed); //active request
            }
         }
         CleanupStack::PopAndDestroy(server);
         DBG("CONNECT DONE");
      }
      break;
   case isab::Module::DISCONNECT:
      DBG("DISCONNECT");
      iSocketsEngine->Stop(); //active request
      break;
   case isab::Module::QUERY:
      DBG("QUERY");
      iSocketsEngine->Query(); 
      iGenerator->SetReady();
      break;
   }
   DBG("SocketsConnectL done, returning %d.", ret);
   return ret;
}

TBool CTcpAdmin::IsConnected()
{
   return iSocketsEngine->IsConnected();
}

void 
CTcpAdmin::ConnectionNotify(enum isab::Module::ConnectionNotify aNotification,
                            enum isab::Module::ConnectionNotifyReason aReason)
{
   DBG("ConnectionNotify(%d,%d)", int(aNotification), int(aReason));
   ASSERT_IS_THREAD(iThreadId, EWrongThread);
   iTss->connectionNotify(aNotification, aReason);
}

TInt CTcpAdmin::SendData(const TUint8* aData, TInt aLength)
{
   DBG("SendData %d bytes", aLength);
   ASSERT_NOT_THREAD(iThreadId, EWrongThread);
   //This function should only ever be called from outside the thread
   //   return iSprocket->SendData(aData, aLength);
   return iGenerator->SendEventL(CTcpDataEvent::NewL(aData, aLength));
}

TInt CTcpAdmin::WriteToTcp(HBufC8* aData)
{
   DBG("WriteToTcp %d bytes", aData->Length());
   ASSERT_IS_THREAD(iThreadId, EWrongThread);
   //this function should only ever be called from inside the thread.
   if(iSocketsEngine->IsConnected()){
      iSocketsEngine->Write(aData);
   } else {
      WARN("Socket not connected, discarding %d bytes of data.", 
           aData->Length());
      delete aData;
      OperationComplete();
   }
   return KErrNone;
}

void CTcpAdmin::BlockEvents()
{
   DBG("BlockEvents!");
   iGenerator->SetReady(EFalse);
}

void CTcpAdmin::OperationComplete()
{
   DBG("OperationComplete: %scorrect thread, generator is %sactive", 
       IsThread(iThreadId) ? "" : "in", iGenerator->IsActive() ? "" : "not ");
   ASSERT_IS_THREAD(iThreadId, EWrongThread);
   iGenerator->SetReady();
}

void CTcpAdmin::Panic(TInt aReason)
{
   ASSERT_IS_THREAD(iThreadId, EWrongThread);
   ERR("PANIC %d", aReason);
   iTss->tcpThreadPanic(aReason);
}

void CTcpAdmin::SetConnectionParam(TInt aIAP)
{
   DBG("SetConnectionParam(%d)", aIAP);
   iCritical.Wait();
   iIAP = aIAP;
   iCritical.Signal();
}

TInt CTcpAdmin::ConnectionParam()
{
   iCritical.Wait();
   TInt iap = iIAP;
   iCritical.Signal();
   return iap;
}

void CTcpAdmin::ConsiderWLAN(TBool aConsider)
{
   iConsiderWLAN = aConsider;
}

// CTcpAdminFactory member functions. 

CTcpAdminFactory::CTcpAdminFactory(class isab::TcpSymbianSerial* aTss) :
   iTss(aTss)
{}

class CTcpAdminFactory* 
CTcpAdminFactory::NewL(class isab::TcpSymbianSerial* aTss, const TDesC& aName,
                       class isab::LogMaster* aLogMaster)
{
   class CTcpAdminFactory* self = new (ELeave) CTcpAdminFactory(aTss);
   CleanupStack::PushL(self);
   self->SetNameL(aName);
   self->iLogMaster = aLogMaster;
   CleanupStack::Pop(self);
   return self;
}

class CTcpAdminFactory* 
CTcpAdminFactory::New(class isab::TcpSymbianSerial* aTss, const TDesC& aName,
                      class isab::LogMaster* aLogMaster)
{
   class CTcpAdminFactory* self = new CTcpAdminFactory(aTss);
   if(self){
      self->iLogMaster = aLogMaster;
      if(!self->SetName(aName)){
         delete self;
         self = NULL;
      }
   }
   return self;
}

class CThreadAdmin* CTcpAdminFactory::CreateAdminL()
{
   iAdmin = new (ELeave) CTcpAdmin(iTss, iLogMaster);
   return iAdmin;
}

class CTcpAdmin* CTcpAdminFactory::CreatedObject() const
{
   return iAdmin;
}
