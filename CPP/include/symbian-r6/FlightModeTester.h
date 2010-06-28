/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef FLIGHT_MODE_TESTER_H
#define FLIGHT_MODE_TESTER_H

#include <e32base.h>
#ifdef NAV2_CLIENT_UIQ3
# include <etel3rdparty.h>
#endif
#include "TraceMacros.h"

template <class FlightModeRequester>
class CFlightModeTester : public CActive
{
 public:
   static CFlightModeTester* NewL(FlightModeRequester& aRequester,
                                  TBool aActivate = EFalse,
                                  enum TPriority aPriority = EPriorityStandard)
   {
      CFlightModeTester<FlightModeRequester>* self =
         new (ELeave) CFlightModeTester(aRequester, aPriority);
      CleanupStack::PushL(self);
      self->ConstructL();
      if(aActivate){
         self->CheckFlightModeOn();
      }
      CleanupStack::Pop(self);
      return self;
   }

   void CheckFlightModeOn()
   {
#ifdef NAV2_CLIENT_UIQ3
      iTelephony->GetFlightMode(iStatus, iFlightModeV1Pckg);
#endif
      SetActive();
#ifndef NAV2_CLIENT_UIQ3
      class TRequestStatus* status = &iStatus; 
      iStatus = KRequestPending;
      User::RequestComplete(status, KErrNone);
#endif
   }
   
 private:
   CFlightModeTester(FlightModeRequester& aRequester,
                     enum TPriority aPriority = EPriorityStandard)
      : CActive(aPriority), iRequester(aRequester)
#ifdef NAV2_CLIENT_UIQ3
      ,iFlightModeV1Pckg(iFlightModeV1) 
#endif
   {
   } 

   void ConstructL()
   {
#ifdef NAV2_CLIENT_UIQ3
      iTelephony = CTelephony::NewL();
#endif
      CActiveScheduler::Add(this);      
   }
   
   void RunL()
   {
      if (iStatus == KErrNone) {
#ifdef NAV2_CLIENT_UIQ3
         CTelephony::TFlightModeStatus flightMode = iFlightModeV1.iFlightModeStatus;
         iRequester.FlightModeOn(flightMode == CTelephony::EFlightModeOn);
#else
         iRequester.FlightModeOn(EFalse);
#endif
      } else {
         iRequester.FlightModeStateFailed(iStatus.Int());
      }
   }

   void DoCancel()
   {
#ifdef NAV2_CLIENT_UIQ3
      iTelephony->CancelAsync(CTelephony::EGetFlightModeCancel);
#endif
   }

 private:
   ///Requester to return flight mode state to.
   FlightModeRequester& iRequester;
#ifdef NAV2_CLIENT_UIQ3
   class CTelephony* iTelephony;
   CTelephony::TFlightModeV1 iFlightModeV1;
   CTelephony::TFlightModeV1Pckg iFlightModeV1Pckg;
#endif

};

#endif
