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

#ifndef NETWORK_REGISTRATION_NOTIFIER_H
#define NETWORK_REGISTRATION_NOTIFIER_H

#include <e32base.h>
#include <etel3rdparty.h>

class CNetworkRegistrationNotifier : public CActive
{
public:
   static CNetworkRegistrationNotifier* NewLC(
               class MNetworkRegistrationNotifierObserver* aObserver,
               enum TPriority aPriority = EPriorityStandard);

   static CNetworkRegistrationNotifier* NewL(
               class MNetworkRegistrationNotifierObserver* aObserver,
               enum TPriority aPriority = EPriorityStandard);
   /**
    * Class Destructor
    */
   virtual ~CNetworkRegistrationNotifier();

   /**
    * This will start the actual observing.
    */
   void Start();

   /**
    * returns the registration status
    * @return The network registration status.
    */
   CTelephony::TRegistrationStatus RegistrationStatus();
protected:
   /**
    * Class constructor
    * @param aPriotiry   The priority of the active object.
    */
   CNetworkRegistrationNotifier(class MNetworkRegistrationNotifierObserver* aObserver,
                                      enum TPriority aPriority);

private:
   /**
    * Symbian second stage ConstructL.
    */
   void ConstructL();

protected: // From Base classes
   /**
    * CActive RunL function.
    */
   virtual void RunL();

   /**
    * Called by framework call Cancel().
    */
   virtual void DoCancel();

private:
   /// The observer we will notify when a change happens.
   class MNetworkRegistrationNotifierObserver* iObserver;

   /// The object to get information from the phone.
   CTelephony* iTelephony;

   /// The result from the network registration change will end up here.
   CTelephony::TNetworkRegistrationV1 iNetworkRegistrationV1;

   /// Package container for iNetworkRegistrationV1.
   CTelephony::TNetworkRegistrationV1Pckg iNetworkRegistrationV1Pckg;

   /// Registration status of the phone (no service, roaming, busy, etc.).
   CTelephony::TRegistrationStatus iRegStatus;
};

#endif // NETWORK_REGISTRATION_NOTIFIER_H
