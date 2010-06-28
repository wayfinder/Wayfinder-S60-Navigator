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
#include "Reporter.h"
#include <bttypes.h>

CBtReporter::CBtReporter(class MGpsConnectionObserver* aObserver) : 
   iObserver(aObserver)
{}

void CBtReporter::ConstructL()
{
   iGenerator = CEventGenerator<CBtReporter, CBtEvent*>::NewL(*this);
}

class CBtReporter* CBtReporter::NewL(class MGpsConnectionObserver* aObserver)
{
   class CBtReporter* self = new (ELeave) CBtReporter(aObserver);
   self->ConstructL();
   return self;
}

void CBtReporter::HandleGeneratedEventL(class CBtEvent* aEvent)
{
   aEvent->Report(iObserver);
   delete aEvent;
}

void CBtReporter::BluetoothDebug(const TDesC& aDbgMsg)
{
   iGenerator->SendEvent(CBtEvent::NewDebugL(aDbgMsg));
}

void CBtReporter::BluetoothError(const TDesC& aErrorMsg)
{
   iGenerator->SendEvent(CBtEvent::NewErrorL(aErrorMsg));
}

void CBtReporter::BluetoothError(TInt aErrorMsg)
{
   iGenerator->SendEvent(CBtEvent::NewErrorL(aErrorMsg));
}

void CBtReporter::BluetoothStatus(TBool aOk)
{
   iGenerator->SendEvent(CBtEvent::NewStatusL(aOk));
}

void CBtReporter::BluetoothDeviceChosen(TInt64 aAddr, const TDesC& aName)
{
   iGenerator->SendEvent(CBtEvent::NewDeviceL(aAddr, aName));
}

void CBtReporter::BluetoothDeviceChosen(const class TBTDevAddr& aAddr, 
                                        const TDesC& aName)
{
   iGenerator->SendEvent(CBtEvent::NewDeviceL(aAddr, aName));
}

/// CBtEvent

class CBtEvent* CBtEvent::NewDebugL(const TDesC& aDbgMsg)
{
   return CBtTextDebugEvent::NewL(aDbgMsg);
}

class CBtEvent* CBtEvent::NewErrorL(const TDesC& aErrorMsg)   
{
   return CBtTextErrorEvent::NewL(aErrorMsg);
}

class CBtEvent* CBtEvent::NewErrorL(TInt aErrorMsg)   
{
   return CBtIntErrorEvent::NewL(aErrorMsg);
}

class CBtEvent* CBtEvent::NewStatusL(TBool aOk)   
{
   return CBtStatusEvent::NewL(aOk);
}

class CBtEvent* CBtEvent::NewDeviceL(TInt64 aAddr, const TDesC& aName)
{
   return CBtDeviceEvent<TInt64>::NewL(aAddr, aName);
}

class CBtEvent* CBtEvent::NewDeviceL(const class TBTDevAddr& aAddr, 
                                     const TDesC& aName)
{
   return CBtDeviceEvent<TBTDevAddr>::NewL(aAddr, aName);
}
#endif
