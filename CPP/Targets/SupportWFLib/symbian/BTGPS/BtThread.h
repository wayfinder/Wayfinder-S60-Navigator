/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BTTHREAD_H
#define BTTHREAD_H
#include "ThreadOwner.h"
#include <e32base.h>           //CActive
#include "Nav2.h"              //isab::Nav2::Channel
#include "PositionInterface.h" //MPositionInterfaceRedirect
//#include "BtGpsConnector.h"    //MGpsConnectionObserver
#include "EventGenerator.h"    //CEventGenerator

class CBtAdmin : public CThreadAdmin, public MPositionInterfaceRedirect
{
   CBtAdmin(class isab::Nav2::Channel* aChannel, class CBtReporter* aReporter);
   void ConstructL();
   ///Deletes the address cache. NOT PROTECTED BY iCritical. MUST BE
   ///CALLED INSIDE Wait/Signal PAIR.
   void DeleteAddressCacheUnprotected();
   ///Deletes the address cache. Protected by iCritical. MUST NOT BE
   ///CALLED INSEIDE A Wait/Signal PAIR.
   void DeleteAddressCache();

public:
   static class CBtAdmin* NewL(class isab::Nav2::Channel* aChannel, 
                               class CBtReporter* aReporter,
                               TInt aMapSize = 0,
                               struct TBtErrorToResource* aMapping = NULL);
   virtual ~CBtAdmin();
   virtual void StartL();
   virtual void StopL();

   ///@name From MPositionInterface
   ///These are the functions from MPositionInterface that cannot be
   ///handled by MPositionInterfaceRedirect since they need to cross
   ///thread borders.
   //@{
   virtual void ConnectL();
   virtual void DisconnectL();
   virtual TBool SetAddress(const char **aData, TInt aNum);
   virtual void ReConnect();
   //@}

   enum TPositionInterfaceOperation{
      EPositionInterfaceConnect,
      EPositionInterfaceDisconnect,
      EPositionInterfaceSetAddress,
      EPositionInterfaceReConnect,
   };

   void HandleGeneratedEventL(enum TPositionInterfaceOperation aEvent);

private:
   ///For communicating with Nav2. Just caching the pointer. 
   class isab::Nav2::Channel* iChannel;
   ///Cross-thread MGpsConnectionObserver interface. Not actively used
   ///by this object, just holds a pointer to it.
   class CBtReporter* iReporter;
   ///The id of the parent thread. What is it good for? Absolutely nothing!
   class TThreadId iThreadId;
   ///GpsConnector object. Does the actual work. 
   class CBtGpsConnector* iGpsConnector;
   ///Cache for any bluetooth adresses. 
   char** iAddressCache;
   ///Size of the iAddressCache
   TInt iAddressSize;
   typedef CEventGenerator<CBtAdmin, TPositionInterfaceOperation> CPositionEventGenerator;
   CPositionEventGenerator* iGenerator;
   class RCriticalSection iCritical;
   struct TBtErrorToResource* iMapping;
   TInt iMapSize;
};

class CBtAdminFactory : public CThreadAdminFactory
{
   CBtAdminFactory(class isab::Nav2::Channel* aChannel,
                   class CBtReporter* aReporter, 
                   struct TBtErrorToResource* aMapping, TInt aMapSize);
public:
   virtual class CThreadAdmin* CreateAdminL();
   static class CBtAdminFactory* NewLC(class isab::Nav2::Channel* aChannel, 
                                       class CBtReporter* aReporter, 
                                       struct TBtErrorToResource* aMapping, 
                                       TInt aMapSize);
   class CBtAdmin* Admin();
   virtual ~CBtAdminFactory();
private:
   class isab::Nav2::Channel* iChannel;
   class CBtReporter* iReporter;
   class CBtAdmin* iAdmin;
   struct TBtErrorToResource* iMapping;
   TInt iMapSize;
};


class CBtThread : public CBase, public MPositionInterfaceRedirect
{
   CBtThread(class MGpsConnectionObserver* aObserver);
   void SetErrorMappingL(TInt aMappingResource);
   void ConstructL(class isab::Nav2::Channel* aChannel);
   void ConstructL(struct TBtErrorToResource* aMapping, TInt aMappingSize, 
                   class isab::Nav2::Channel* aChannel);
   void ConstructL(TInt aMappingResource,
                   class isab::Nav2::Channel* aChannel);

   void ResetThreadL(TBool aRestart = ETrue);
public:
   static class CBtThread* NewL(class MGpsConnectionObserver* aObserver,
                                class isab::Nav2::Channel* aChannel);
   static class CBtThread* NewL(class MGpsConnectionObserver* aObserver,
                                struct TBtErrorToResource* aMapping,
                                TInt aMappingSize,
                                class isab::Nav2::Channel* aChannel);
   static class CBtThread* NewL(class MGpsConnectionObserver* aObserver,
                                TInt aMappingResource,
                                class isab::Nav2::Channel* aChannel);

   virtual ~CBtThread();

private: 
   class MGpsConnectionObserver* iObserver;
   class isab::Nav2::Channel* iChannel;
   class CThreadOwner* iOwner;
   class CBtReporter* iReporter;
   class CBtAdmin* iAdmin;
   struct TBtErrorToResource* iMapping;
   TInt iMapSize;
};
#endif
