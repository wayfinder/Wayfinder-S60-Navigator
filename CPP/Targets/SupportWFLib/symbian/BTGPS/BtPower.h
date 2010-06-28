/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BTPOWER_H
#define BTPOWER_H
#include "ActiveLog.h"
#include "Completer.h"

///Active object that asks the user to activate or deactivate
///Bluetooth if necessary.  The result of the operation is reported to
///another active object through the MCompleter interface.
class CBtPowerChecker : public CActiveLog, public MCompleter
{
   CBtPowerChecker(const class CBtPowerChecker&);
   class CBtPowerChecker& operator=(const class CBtPowerChecker&);
   ///Default constructor.
   CBtPowerChecker();
   ///Second phase constructor. Connects to the Notifier Server
   ///through the <code>iNotifier</code> object. Leaves if the call to
   ///<code>RNotifier::Connect</code> fails.
   void ConstructL();
public:
   virtual ~CBtPowerChecker();
   ///Static constructor.
   ///May leave with KErrNoMem or any error code returned by
   ///<code>RNotifier::Connect</code>. 
   ///@return a new object of type CBtPowerChecker.
   static class CBtPowerChecker* NewL();
   ///Static constructor.
   ///May leave with KErrNoMem or any error code returned by
   ///<code>RNotifier::Connect</code>. 
   ///@return a new object of type CBtPowerChecker, which is still on
   ///        the CleanupStack. 
   static class CBtPowerChecker* NewLC();
   ///Asynchronous request to check if Bluetooth is turned on or off,
   ///and if not ask the user to do so. 
   void PowerCheck(class TRequestStatus* aStatus, TBool aOn);
   void PowerCheck(class TRequestStatus* aStatus);

   void CancelPowerCheck();

   TBool IsPoweredOn();
   virtual void RunL();
   virtual void DoCancel();
private:
   class RNotifier iNotifier;
   TPckgBuf<TBool> iPowerCommand;
   TPckgBuf<TBool> iPowerReply;
};
#endif
