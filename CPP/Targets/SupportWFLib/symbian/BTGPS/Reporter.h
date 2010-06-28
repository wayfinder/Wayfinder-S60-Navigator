/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BT_REPORTER_H
#define BT_REPORTER_H
#include <e32base.h>
#include "BtGpsConnector.h"
#include "EventGenerator.h"
class CBtReporter : public CBase, public MGpsConnectionObserver
{
   /** @name Constructors and destructor. */
   //@{
   /**
    * Constructor.
    * @param aObserver the receiver of all reports. 
    */
   CBtReporter(class MGpsConnectionObserver* aObserver);
public:
   /** Second phase constructor. */
   void ConstructL();
   /**
    * Static constructor. 
    * @param aObserver the receiver of all reports. 
    * @return a new CBtReporter object.
    */
   static class CBtReporter* NewL(class MGpsConnectionObserver* aObserver);
   //@}
   ///@name From MGpsConnectionObserver
   //@{
   virtual void BluetoothDebug(const TDesC& aDbgMsg);
   virtual void BluetoothError(const TDesC& aErrorMsg);
   virtual void BluetoothError(TInt aErrorMsg);
   virtual void BluetoothStatus(TBool aOk);
   virtual void BluetoothDeviceChosen(TInt64 aAddr, const TDesC& aName);
   virtual void BluetoothDeviceChosen(const class TBTDevAddr& aAddr, 
                                      const TDesC& aName);
   virtual void BluetoothConnectionLost(TBool aWillTryReconnect);
   //@}
   ///
   void HandleGeneratedEventL(class CBtEvent* aEvent);

private:
   class MGpsConnectionObserver* iObserver;
   CEventGenerator<CBtReporter, CBtEvent*>* iGenerator;
};

class CBtEvent : public CBase{
public:
   virtual void Report(class MGpsConnectionObserver* aObserver) = 0;
   static class CBtEvent* NewDebugL(const TDesC& aDbgMsg);
   static class CBtEvent* NewErrorL(const TDesC& aErrorMsg);
   static class CBtEvent* NewErrorL(TInt aErrorMsg);
   static class CBtEvent* NewStatusL(TBool aOk);
   static class CBtEvent* NewDeviceL(TInt64 aAddr, const TDesC& aName);
   static class CBtEvent* NewDeviceL(const class TBTDevAddr& aAddr, 
                                     const TDesC& aName);

};

template<class Derived>
class CBtTextEvent : public CBtEvent{
protected:
   HBufC* iText;
   void ConstructL(const TDesC& aText)
   {
      iText = aText.AllocL();
   }
public:
   static Derived* NewBaseL(const TDesC& aText)
   {
      Derived* self = new (ELeave) Derived();
      CleanupStack::PushL(self);
      self->ConstructL(aText);
      CleanupStack::Pop(self);
      return self;
   }
public:
   virtual ~CBtTextEvent()
   {
      delete iText;
   }
};

class CBtTextErrorEvent : public CBtTextEvent<CBtTextErrorEvent>
{
public:
   static class CBtTextErrorEvent* NewL(const TDesC& aText)
   {
      return NewBaseL(aText);
   }
   virtual void Report(class MGpsConnectionObserver* aObserver)
   {
      aObserver->BluetoothError(*iText);
   }
};

class CBtTextDebugEvent : public CBtTextEvent<CBtTextDebugEvent>
{
public:
   static class CBtTextDebugEvent* NewL(const TDesC& aText)
   {
      return NewBaseL(aText);
   }
   virtual void Report(class MGpsConnectionObserver* aObserver)
   {
      aObserver->BluetoothDebug(*iText);
   }
};

class CBtStatusEvent : public CBtEvent
{
   TBool iOk;
   CBtStatusEvent(TBool aOk) : iOk(aOk) {}
public:
   static class CBtStatusEvent* NewL(TInt aOk)
   {
      return new (ELeave) CBtStatusEvent(aOk);
   }
   virtual void Report(class MGpsConnectionObserver* aObserver)
   {
      aObserver->BluetoothStatus(iOk);
   }
};

class CBtIntErrorEvent : public CBtEvent
{
   TInt iError;
   CBtIntErrorEvent(TInt aError) : iError(aError) {}
public:
   static class CBtIntErrorEvent* NewL(TInt aError)
   {
      return new (ELeave) CBtIntErrorEvent(aError);
   }
   virtual void Report(class MGpsConnectionObserver* aObserver)
   {
      aObserver->BluetoothError(iError);
   }
};

template<class T>
class CBtDeviceEvent : public CBtTextEvent<CBtDeviceEvent<T> >
{
   T iAddr;
public:
   static CBtDeviceEvent<T>* NewL(T aAddr, const TDesC& aName)
   {
      CBtDeviceEvent<T>* self = CBtTextEvent<CBtDeviceEvent<T> >::NewBaseL(aName);
      self->iAddr = aAddr;
      return self;
   }
   virtual void Report(class MGpsConnectionObserver* aObserver)
   {
      aObserver->BluetoothDeviceChosen(iAddr, *(this->iText));
   }
};


class CBtConnectionLostEvent : public CBtEvent
{
   TBool iWillTryReconnect;
public:
   static class CBtConnectionLostEvent* NewL(TBool aWillTryReconnect)
   {
      class CBtConnectionLostEvent* self = 
         new (ELeave) CBtConnectionLostEvent();
      self->iWillTryReconnect = aWillTryReconnect;
      return self;
   }
   virtual void Report(class MGpsConnectionObserver* aObserver)
   {
      aObserver->BluetoothConnectionLost(iWillTryReconnect);
   }
};

#endif
