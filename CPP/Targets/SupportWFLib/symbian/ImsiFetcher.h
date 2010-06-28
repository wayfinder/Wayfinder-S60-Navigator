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

#ifndef IMSIFETCHER_H
#define IMSIFETCHER_H

# include <e32base.h>
# include <stdlib.h>

# include <etel3rdparty.h>

/**
 *    Symbian 9 class for retreiving the IMSI number.
 *    Upon completion of the asynchronous call, 
 *    a call back method to the ImsiRequester will be called.
 */
template <class ImsiRequester>
class CImsiFetcher : public CActive
{
   ///Constructor.
   ///@param aWayfinderAppUi pointer to WayfinderAppUi. Used for
   ///                       callbacks.
   ///@param aPriority       the priority of the active object
   ///                       request issued through GetIMSI. Defaults
   ///                       to EPriorityStandard.
   CImsiFetcher(ImsiRequester& aImsiRequester,
                enum TPriority aPriority = EPriorityStandard)
      : CActive(aPriority), iImsiRequester(aImsiRequester)
   {
   }

   ///Second phase constructor. Must be called before GetIMSI();
   void ConstructL()
   {
      iPkg = new (ELeave) CTelephony::TSubscriberIdV1Pckg(iV1);
      iTelephony = CTelephony::NewL();
      CActiveScheduler::Add(this);
   }
public:
   ///Static constructor.

   ///Completes both stages of construction so that the object is
   ///ready for a call to GetIMSI. the aActivate argument gioves the
   ///user an option to call GetIMSI as a part of the construction.
   ///@param aWayfinderAppUi pointer to WayfinderAppUi. Used for
   ///                       callbacks.
   ///@param aActivate       if ETrue, GetIMSI is called on the object 
   ///                       before it is returned. Defaults to EFalse.
   ///@param aPriority       the priority of the active object
   ///                       request issued through GetIMSI. Defaults
   ///                       to EPriorityStandard.
   ///@return a new, completely constructed CImsiFetcher object.
   static 
      CImsiFetcher<ImsiRequester>* NewL(ImsiRequester& aImsiRequester,
            TBool aActivate = EFalse,
            enum TPriority aPriority = EPriorityStandard)
   {
      CImsiFetcher<ImsiRequester>* self =
         new (ELeave) CImsiFetcher(aImsiRequester, aPriority);
      CleanupStack::PushL(self);
      self->ConstructL();
      if(aActivate){
         self->GetIMSI();
      }
      CleanupStack::Pop(self);
      return self;
   }

   ///Destruction
   ~CImsiFetcher()
   {
      Cancel();   // if any request outstanding, calls DoCancel() to cleanup
      delete iTelephony;
      delete iPkg;
   }

   ///Issue request: retrieve IMSI
   void GetIMSI()
   {
      iTelephony->GetSubscriberId(iStatus, *iPkg );
      SetActive();
   }

   ///Cancel request.
   void DoCancel()
   {
      if(IsActive()){
         iTelephony->CancelAsync(CTelephony::EGetSubscriberIdCancel);
      }
   } 

   ///Service completed request
   void RunL()
   {
      if(iStatus == KErrNone){ 
         iImsiRequester.ImsiNumberSet((*iPkg)().iSubscriberId);
      }
   }

   ///Returns the found IMSI number. Note that if the asynchronous
   ///request has not yet completed the content of the descriptor is
   ///most likely KNullDesC.
   ///@retern a descriptor holding the IMSI number.
   const TDesC& IMSI() const
   {
      return iV1.iSubscriberId;
   }


private:
   ///Telephony object. Provides APIs for fetching IMSI number.
   class CTelephony* iTelephony; 
   ///Data object that holds imsi number, phone model, and phone
   ///manufacturer.
   CTelephony::TSubscriberIdV1 iV1;
   ///Descriptor adaptor for iV1;
   CTelephony::TSubscriberIdV1Pckg* iPkg;
   ///Requester to return IMSI number to.
   ImsiRequester& iImsiRequester;
};
#endif
