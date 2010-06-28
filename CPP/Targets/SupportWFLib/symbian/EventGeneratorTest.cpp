/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * NOTE!
 *
 * This file is used to test the CEventGenerator template. It should
 * never be included in any product. It should be safe to always
 * compile this file since the linker will remove it.
 */
#include "EventGenerator.h"

enum Test1{ a,b,c};

class CEventGeneratorTester : public CBase
{
   enum Test2 { e,f,g};
   typedef CEventGeneratorTester ME;
   CEventGenerator<ME>* iGenInt;
   CEventGenerator<ME, enum Test1>* iGenTest1;
   CEventGenerator<ME, enum Test2>* iGenTest2;
   
   void ConstructL()
   {
      iGenInt = CEventGenerator<ME>::NewL(*this);
      iGenTest1 = CEventGenerator<ME,enum Test1>::NewL(*this);
      iGenTest2 = CEventGenerator<ME,enum Test2>::NewL(*this);
      iGenInt->SendEventL(3);
      iGenInt->SendEventL(a);
      iGenInt->SendEventL(e);
      iGenInt->SendEventL(1,TTimeIntervalSeconds(2));
      iGenInt->SendEventL(a,TTimeIntervalMicroSeconds32(2));
      iGenInt->SendEventL(e,TTimeIntervalMicroSeconds32(0));
   }
public:
   void HandleGeneratedEventL(TInt)
   {
   }
   void HandleGeneratedEventL(enum Test1)
   {
   }
   void HandleGeneratedEventL(enum Test2)
   {
   }

};


//template CEventGenerator<CEventGeneratorTester, TInt>;
template class CEventGenerator<CEventGeneratorTester, TChar>;
