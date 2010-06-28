/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TcpEvent.h"

CTcpEvent::CTcpEvent()
{
}


class CTcpDataEvent* CTcpDataEvent::NewL(const TDesC8& aData) 
{
   class CTcpDataEvent* self = new (ELeave) CTcpDataEvent();
   CleanupStack::PushL(self);
   self->iData = aData.AllocL();
   CleanupStack::Pop(self);
   return self;
}

class CTcpDataEvent* CTcpDataEvent::NewL(const TUint8* aData, TInt aLength) 
{
   return NewL(TPtrC8(aData, aLength));
};

TInt CTcpDataEvent::ProcessL(class MTcpEventHandler& aHandler) 
{
   return aHandler.HandleDataEventL(*this);
}

const TDesC8& CTcpDataEvent::Data() const
{
   return *iData;
}

HBufC8* CTcpDataEvent::ReleaseData() 
{
   HBufC8* tmp = iData;
   iData = NULL;
   return tmp;
}


CTcpConnectionEvent::CTcpConnectionEvent(enum isab::Module::ConnectionCtrl aControl,
                                         const TDesC8& aMethod) :
   iControl(aControl), iMethod(aMethod)
{
}
                                        
class CTcpConnectionEvent* CTcpConnectionEvent::NewL(enum isab::Module::ConnectionCtrl aControl, 
                                                     const char* aMethod) 
{
   return NewL(aControl, reinterpret_cast<const TText8*>(aMethod));
};

class CTcpConnectionEvent* CTcpConnectionEvent::NewLC(enum isab::Module::ConnectionCtrl aControl, 
                                                      const char* aMethod) 
{
   CTcpConnectionEvent* self = NewL(aControl, reinterpret_cast<const TText8*>(aMethod));
   CleanupStack::PushL(self);
   return self;
};

class CTcpConnectionEvent* CTcpConnectionEvent::NewL(enum isab::Module::ConnectionCtrl aControl, 
                                                     const TText8* aMethod) 
{
   return new (ELeave) CTcpConnectionEvent(aControl, (aMethod ? TPtrC8(aMethod) : TPtrC8(KNullDesC8)));
};

TInt CTcpConnectionEvent::ProcessL(class MTcpEventHandler& aHandler) 
{
   return aHandler.HandleConnectionEventL(*this);
};

enum isab::Module::ConnectionCtrl CTcpConnectionEvent::Action() const 
{
   return iControl;
}

const TDesC8* CTcpConnectionEvent::Method() const
{
   return iMethod.Length() ? &iMethod: NULL;
}
