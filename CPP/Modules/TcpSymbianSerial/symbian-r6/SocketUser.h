/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPSYMBIANSOCKETUSER_H
#define TCPSYMBIANSOCKETUSER_H

#include <e32std.h>
#include "Module.h"

/**
 * This interface should be implemented to receive events from
 * CTcpSymbianEngine, CTcpSymbianRead, and CTcpSymbianWrite. 
 */
class MTcpSymbianSocketUser
{
public:
   /**
    * Receives data read froma TCP socket by a CTcpSymbianRead. 
    * @param aData the read data. 
    */
   virtual void ReceiveData(const TDesC8& aData) = 0;
   /**
    * Called by CTcpSymbianEngine whenever the TCP socket connection
    * state changes.
    * @param aEvent  what happend.
    * @param aReason why it happend.
    */
   virtual 
   void ConnectionNotify(enum isab::Module::ConnectionNotify aEvent,
                         enum isab::Module::ConnectionNotifyReason aReason)=0;

   /**
    * Called by CTcpSymbianEngine whenever the engine needs to
    * complete some opration of it's own, such as cleanup after a
    * broken connection.
    */
   virtual void BlockEvents() = 0;
   /**
    * Called by CTcpSymbianWrite each time a write completes. Signals
    * that CTcpSymbianWrite is ready for a new write operation.
    */
   virtual void OperationComplete() = 0;
   virtual void Panic(TInt aReason) = 0;
};
#endif
