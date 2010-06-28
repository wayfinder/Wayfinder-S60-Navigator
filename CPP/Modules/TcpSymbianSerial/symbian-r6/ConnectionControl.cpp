/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ConnectionControl.h"
#include "TcpAdmin.h"

namespace {
   enum TConnCtrlPan {
      ENoCritical, ///The critical section was supposed to be locked
      ENotActive,  ///The object should be active here
      ENullMethod, ///The method argument was NULL
      ERunEmpty,   ///Run called even though iCommands was empty
      ERunError,   ///Status was not KErrNone in RunL
      EWrongThread,///Function called from wrong thread.
      EDisconnectLeave, ///SocketsConnectL(DISCONNECT) leaved
   };
   _LIT(KConnPanic, "TcpConnCtrl");
}

#define ASSERT_IS_THREAD(thread, panic) ((IsThread(thread))||(User::Panic(KConnPanic, panic),0))
#define ASSERT_NOT_THREAD(thread, panic) ((!IsThread(thread))||(User::Panic(KConnPanic, panic),0))


class CConnectionControl* 
CConnectionControl::NewL(class CTcpAdmin* aAdmin, 
                         const class TThreadId& aThreadId)
{
   class CConnectionControl* self = 
      new (ELeave) CConnectionControl(aAdmin, aThreadId);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

CConnectionControl::CConnectionControl(class CTcpAdmin* aAdmin, 
                                       class TThreadId aThreadId) :
   CActive(EPriorityStandard), iAdmin(aAdmin), iThreadId(aThreadId)
{
}

void CConnectionControl::ConstructL()
{
   iCommands = new (ELeave) CArrayPtrSeg<CConnectionParams>(4);
   User::LeaveIfError(iCritical.CreateLocal());
   CActiveScheduler::Add(this);
   iCritical.Wait(); //must be locked for request to work. 
   Request();
   iCritical.Signal();
}


TBool CConnectionControl::ActivateSelf()
{
   TBool alreadyActivated = (iStatusPtr != NULL);
   if(!alreadyActivated){
      iStatusPtr = &iStatus;
      iStatus = KRequestPending;
      SetActive();
   }
   return alreadyActivated;
}

TBool CConnectionControl::CompleteSelf(TInt aStatus)
{
   TBool wasActive = (iStatusPtr != NULL);
   if(wasActive){
      User::RequestComplete(iStatusPtr, aStatus);
   }
   return wasActive;
}

void CConnectionControl::Request()
{
   __ASSERT_ALWAYS(iCritical.IsBlocked(), 
                   User::Panic(KConnPanic, ENoCritical));
   ASSERT_IS_THREAD(iThreadId, EWrongThread);
   ActivateSelf();
   if(iCommands->Count() > 0){
      CompleteSelf(KErrNone);
   }
}

CConnectionControl::~CConnectionControl()
{
   iCommands->ResetAndDestroy();
   delete iCommands;
}
 
void CConnectionControl::RunL()
{
   iCritical.Wait();
   if(iStatus == KErrNone){
      if(iCommands->Count() > 0){
         class CConnectionParams* params = iCommands->At(0);
         iAdmin->SocketsConnectL(params->iAction, params->iMethod);
         iCommands->Delete(0);
         delete params;
      } else {
         //XXX can this happen?
         //just request again
      }
   } else {
      //XXX can this even happen? We do our own completions, never
      //with anything but KErrNone and KErrCancel, and Cancel should
      //never end up here.

      //just request again
   }
   Request();
   iCritical.Signal();
}

TInt CConnectionControl::RunError(TInt /*aError*/)
{
   ///Possible errors: KErrNoMemory or any return value from RConnection::Open.
   iAdmin->ConnectionNotify(isab::Module::DISCONNECTING, 
                            isab::Module::INTERNAL_ERROR);
   TRAPD(err, iAdmin->SocketsConnectL(isab::Module::DISCONNECT, NULL));
   if(err != KErrNone){
      User::Panic(KConnPanic, EDisconnectLeave);
   }
   return KErrNone;
}

void CConnectionControl::DoCancel()
{
   iCritical.Wait();
   CompleteSelf(KErrCancel);
   iCritical.Signal();
}

void CConnectionControl::ConnectionL(enum isab::Module::ConnectionCtrl aAction,
                                     const char* aMethod)
{
   iCritical.Wait();
   ASSERT_NOT_THREAD(iThreadId, EWrongThread);
   class CConnectionParams* param = CConnectionParams::NewLC(aAction, aMethod);
   iCommands->AppendL(param);
   CleanupStack::Pop(param);
   if(IsActive()){
      class RThread otherThread;
      otherThread.Open(iThreadId);
      otherThread.RequestComplete(iStatusPtr, KErrNone);
      otherThread.Close();
   } else {
      //This is not a problem. If we are not active, we are either
      //canceled or running. The critical region should stop the
      //latter, so maybe we have to look out for the canceled case. But
      //I don't think it's a problem
   }
   iCritical.Signal();
}

TInt CConnectionControl::Connection(enum isab::Module::ConnectionCtrl aAction,
                                    const char* aMethod)
{
   TRAPD(err, ConnectionL(aAction, aMethod));
   return err; //should only be KErrNoMemory or KErrNone.
}

CConnectionControl::CConnectionParams::CConnectionParams(enum isab::Module::ConnectionCtrl aAction) : 
   iAction(aAction)
{
}

void CConnectionControl::CConnectionParams::ConstructL(const char* aMethod)
{
   User::LeaveIfError(Construct(aMethod));
}

TInt CConnectionControl::CConnectionParams::Construct(const char* aMethod)
{
   if(aMethod){
      iMethod = HBufC8::New(strlen(aMethod) + 2);
      if(iMethod){
         iMethod->Des() = reinterpret_cast<const TText8*>(aMethod);
      }
   }
   //successfull if aMethod is NOT set or iMethod was constructed.
   return (!aMethod || iMethod) ? KErrNone : KErrNoMemory;
}


class CConnectionControl::CConnectionParams* 
CConnectionControl::CConnectionParams::NewLC(enum isab::Module::ConnectionCtrl aAction, const char* aMethod)
{
   class CConnectionParams* self = new (ELeave) CConnectionParams(aAction);
   CleanupStack::PushL(self);
   self->ConstructL(aMethod);
   return self;
}

class CConnectionControl::CConnectionParams*
CConnectionControl::CConnectionParams::New(enum isab::Module::ConnectionCtrl aAction, const char* aMethod)
{
   class CConnectionParams* self = new CConnectionParams(aAction);
   if(self){
      if(!self->Construct(aMethod)){
         delete self;
         self = NULL;
      }
   }
   return self;
}


CConnectionControl::CConnectionParams::~CConnectionParams()
{
   delete iMethod;
}
