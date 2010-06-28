/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if !(defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3)
#include "BtThread.h"
#include <coemain.h>
#include <barsread.h>
#include "BtGpsConnector.h"
#include "Reporter.h"
#include "nav2util.h"

_LIT(KBtThread, "BtThread");

CBtThread::CBtThread(class MGpsConnectionObserver* aObserver) : 
   iObserver(aObserver)
{
}

void CBtThread::ConstructL(class isab::Nav2::Channel* aChannel)
{
   iChannel = aChannel;
   iReporter = CBtReporter::NewL(iObserver);
   ResetThreadL();
}

void CBtThread::SetErrorMappingL(TInt aMappingResource)
{
   HBufC8* res= CCoeEnv::Static()->AllocReadResourceAsDes8LC(aMappingResource);
   class TResourceReader reader;
   reader.SetBuffer(res);
   TInt mappingSize = reader.ReadInt16();
   struct TBtErrorToResource* iMapping = NULL;
   if(mappingSize < 1){
      iMapping = new (ELeave) TBtErrorToResource[mappingSize];
      iMapSize = mappingSize;
      for(TInt i = 0; i < mappingSize; ++i){
         struct TBtErrorToResource& curr = iMapping[i];
         curr.iError = reader.ReadInt16();
         curr.iResourceString = reader.ReadInt32();
      }
   }
   CleanupStack::PopAndDestroy(res);
}


void CBtThread::ConstructL(struct TBtErrorToResource* aMapping,
                           TInt aMappingSize,
                           class isab::Nav2::Channel* aChannel)
{
   iChannel = aChannel;
   iReporter = CBtReporter::NewL(iObserver);
   iMapping = new (ELeave) TBtErrorToResource[aMappingSize];
   for(TInt a = 0; a < aMappingSize; ++a){
      iMapping[a] = aMapping[a];
   }
   iMapSize = aMappingSize;
   ResetThreadL();
}

void CBtThread::ConstructL(TInt aMappingResource,
                           class isab::Nav2::Channel* aChannel)
{
   iChannel = aChannel;
   iReporter = CBtReporter::NewL(iObserver);
   SetErrorMappingL(aMappingResource);
   ResetThreadL();
}


class CBtThread* CBtThread::NewL(class MGpsConnectionObserver* aObserver,
                                 class isab::Nav2::Channel* aChannel)
{
   class CBtThread* self = new (ELeave) CBtThread(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL(aChannel);
   CleanupStack::Pop(self);
   return self;
}

class CBtThread* CBtThread::NewL(class MGpsConnectionObserver* aObserver,
                                 struct TBtErrorToResource* aMapping,
                                 TInt aMappingSize,
                                 class isab::Nav2::Channel* aChannel)
{
   class CBtThread* self = new (ELeave) CBtThread(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL(aMapping, aMappingSize, aChannel);
   CleanupStack::Pop(self);
   return self;
}

class CBtThread* CBtThread::NewL(class MGpsConnectionObserver* aObserver,
                                 TInt aMappingResource,
                                 class isab::Nav2::Channel* aChannel)
{
   class CBtThread* self = new (ELeave) CBtThread(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL(aMappingResource, aChannel);
   CleanupStack::Pop(self);
   return self;
}

CBtThread::~CBtThread()
{
   ResetThreadL(EFalse);
   delete iChannel;
   delete iReporter;
   delete[] iMapping;
}

void CBtThread::ResetThreadL(TBool aRestart)
{
   delete iOwner;
   iOwner = NULL;
   iAdmin = NULL;
   if(aRestart){
      if(iChannel && iReporter){
         class CBtAdminFactory* factory = 
            CBtAdminFactory::NewLC(iChannel, iReporter, iMapping, iMapSize);
         iOwner = CThreadOwner::NewL(factory);
         iAdmin = factory->Admin();
         SetPositionInterfaceRedirect(iAdmin);
         CleanupStack::PopAndDestroy(factory);
      } else {
         User::Leave(KErrArgument);
      }
   }
}

//========================AdminFactory===================

CBtAdminFactory::CBtAdminFactory(class isab::Nav2::Channel* aChannel, 
                                 class CBtReporter* aReporter, 
                                 struct TBtErrorToResource* aMapping, 
                                 TInt aMapSize) : 
   CThreadAdminFactory(EPriorityRealTime),
   iChannel(aChannel), iReporter(aReporter), 
   iMapping(aMapping), iMapSize(aMapSize)
{}

class CThreadAdmin* CBtAdminFactory::CreateAdminL()
{
   iAdmin = CBtAdmin::NewL(iChannel, iReporter, iMapSize, iMapping);
   return iAdmin;
}

class CBtAdminFactory* 
CBtAdminFactory::NewLC(class isab::Nav2::Channel* aChannel, 
                       class CBtReporter* aReporter, 
                       struct TBtErrorToResource* aMapping, TInt aMapSize)
{
   class CBtAdminFactory* self = 
      new (ELeave) CBtAdminFactory(aChannel, aReporter, aMapping, aMapSize);
   CleanupStack::PushL(self);
   self->SetNameL(KBtThread);
   return self;
}

class CBtAdmin* CBtAdminFactory::Admin()
{
   return iAdmin;
}

CBtAdminFactory::~CBtAdminFactory()
{
}
//=========================Admin========================

CBtAdmin::CBtAdmin(class isab::Nav2::Channel* aChannel, 
                   class CBtReporter* aReporter) : 
   iChannel(aChannel), iReporter(aReporter)
{}

void CBtAdmin::ConstructL()
{
   User::LeaveIfError(iCritical.CreateLocal());
   iGenerator = CPositionEventGenerator::NewL(*this);
}

class CBtAdmin* CBtAdmin::NewL(class isab::Nav2::Channel* aChannel, 
                               class CBtReporter* aReporter,
                               TInt aMapSize,
                               struct TBtErrorToResource* aMapping)
{
   class CBtAdmin* self = new (ELeave) CBtAdmin(aChannel, aReporter);
   CleanupStack::PushL(self);
   self->ConstructL();
   self->iMapping = aMapping;
   self->iMapSize = aMapSize;
   CleanupStack::Pop(self);
   return self;
}

void CBtAdmin::DeleteAddressCacheUnprotected()
{
   if(iAddressCache){
      for(TInt i = 0; i < iAddressSize; ++i){
         delete[] iAddressCache[i];
      }
      delete[] iAddressCache;
      iAddressCache = NULL;
   }
}

void CBtAdmin::DeleteAddressCache()
{
   iCritical.Wait();
   DeleteAddressCacheUnprotected();
   iCritical.Signal();
}

CBtAdmin::~CBtAdmin()
{
   DeleteAddressCache();
   iCritical.Close();
}

void CBtAdmin::StartL()
{
   class RThread thisThread;
   iThreadId = thisThread.Id();
   //   User::LeaveIfError(iCritical.CreateLocal());
   iGpsConnector = CBtGpsConnector::NewL(iReporter, iChannel);
   if(iMapping){
      iGpsConnector->SetErrorMappingL(iMapping, iMapSize);
   }
   SetPositionInterfaceRedirect(iGpsConnector);
}

void CBtAdmin::StopL()
{
   iGpsConnector->DisconnectL();
}

void CBtAdmin::HandleGeneratedEventL(enum TPositionInterfaceOperation aEvent)
{
   switch(aEvent){
   case EPositionInterfaceConnect:
      iGpsConnector->ConnectL();
      break;
   case EPositionInterfaceDisconnect:
      iGpsConnector->DisconnectL();
      break;
   case EPositionInterfaceSetAddress:
      iCritical.Wait();
      if(iAddressCache){
         iGpsConnector->SetAddress(const_cast<const char**>(iAddressCache), 
                                   iAddressSize);
      }
      DeleteAddressCacheUnprotected();
      iCritical.Signal();
      break;
   case EPositionInterfaceReConnect:
      iGpsConnector->ReConnect();
   }
}

void CBtAdmin::ConnectL()
{
   iGenerator->SendEventL(EPositionInterfaceConnect);
}

void CBtAdmin::DisconnectL()
{
   iGenerator->SendEventL(EPositionInterfaceDisconnect);
}

TBool CBtAdmin::SetAddress(const char **aData, TInt aNum)
{
   char** tmpCache = new char*[aNum];
   if(!tmpCache){
      return EFalse; //No leaves allowed
   }
   for(TInt i = 0; i < aNum; ++i){
      tmpCache[i] = NULL;
      tmpCache[i] = isab::strdup_new(aData[i]);
      if(!tmpCache[i]){
         //no leaves allowed
         for(TInt j = 0; j < i; ++j){
            delete[] tmpCache[j];
         }
         delete[] tmpCache;
         tmpCache = NULL;
         return EFalse;
      }
   }
   iCritical.Wait();
   DeleteAddressCacheUnprotected();
   iAddressSize = aNum;
   iAddressCache = tmpCache;
   iCritical.Signal();
   iGenerator->SendEventL(EPositionInterfaceSetAddress);
   return ETrue;
}

void CBtAdmin::ReConnect()
{
   iGenerator->SendEventL(EPositionInterfaceReConnect);
}

#endif
