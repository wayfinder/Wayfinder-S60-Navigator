/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BTSELECTOR_H
#define BTSELECTOR_H
#include "ActiveLog.h"
#include <btextnotifiers.h>
#include "Completer.h"
#include "SdpAttributeNotifier.h"

/** Class that selects a bt device using the RNotifier framework. */
class CBtSelector : public CActiveLog, 
                    public MCompleter
{
   CBtSelector(const class CBtSelector&);
   const class CBtSelector& operator=(const CBtSelector&);
   /** @name Constructors and destructor. */
   //@{
   /** Default constructor. */
   CBtSelector();
   /** Second phase constructor. */
   void ConstructL();
public:
   /** Virtual destructor. */
   virtual ~CBtSelector();
   /**
    * Static constructor. 
    * @return a new object of class CBtSelector.
    */
   static class CBtSelector* NewL();
   /**
    * Static constructor.
    * @return a new object of class CBtSelector, still on the
    *         cleanupstack.
    */
   static class CBtSelector* NewLC();
   //@}

   /**
    * Start a new asynchronous selection process. 
    * @param aStatus pointer to the TRequestStatus object that will
    *                receive the completion notice.
    * @param aClass  the service class that the selected device must
    *                support.
    */
   void SelectDeviceL(class TRequestStatus* aStatus, const class TUUID& aClass);
   void SelectSerialDeviceL(class TRequestStatus* aStatus);

   void CancelSelection();
   /** 
    * @name Functions that query the selected device about name and
    * address.
    */
   //@}
   /**
    * Gets the name of the selected device. 
    * @return the name. 
    */
   const TDesC& Name();
   /**
    * Gets the bluetooth address of the selected device. 
    * @return the address.
    */
   class TBTDevAddr Address();
   //@}

   ///@name From CActive.
   //@{
   virtual void RunL();
   virtual void DoCancel();
   //@}
   void CompleteSelf(TInt aCompletionCode);
private:
   /** XXX what do we need this for?*/
   class TUUID iClass;
   /** The notifier framework is used for the selection dialog.*/
   class RNotifier iNotifier;
   /** Will contain info about the selected device. */
   TBTDeviceResponseParamsPckg iResponse;
   /** Filter for the selection dialog. */
   TBTDeviceSelectionParamsPckg iSelectionFilter;
};

#endif
