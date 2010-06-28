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

#ifndef NETWORK_INFO_HANDLER_H
#define NETWORK_INFO_HANDLER_H

#include <e32base.h>
#include <etel3rdparty.h>

/**
 * The Network Info handler class.
 * 
 */
class CNetworkInfoHandler : public CActive
{
public:
   /**
    * Enum containing the states of the active object.
    */
   enum TNetworkInfoState {
      EIdle,
      EFetchingSignalIndicator,
      EFetchingSignalStrength,
      EFetchingRegistrationStatus,
      EFetchingNetworkInfo,
      EDone
   };

   /**
    * Symbian static NewLC function.
    *
    * @param aObserver   The observer that want to get notified when
    *                    a async fetching has completed, forwarded to 
    *                    the class constructor.
    * @param aPriotiry   The priority of the active object.
    * @return            A NetworkInfoHandler instance.
    */
   static CNetworkInfoHandler* NewLC(class MNetworkInfoObserver* aObserver, 
                                     enum TPriority aPriority = EPriorityStandard);
   
   /**
    * Symbian static NewL function.
    *
    * @param aObserver   The observer that want to get notified when
    *                    a async fetching has completed, forwarded to 
    *                    the class constructor.
    * @param aPriotiry   The priority of the active object.
    * @return            A NetworkInfoHandler instance.
    */
   static CNetworkInfoHandler* NewL(class MNetworkInfoObserver* aObserver, 
                                    enum TPriority aPriority = EPriorityStandard);

   /**
    * Class Destructor
    */
   virtual ~CNetworkInfoHandler();

protected:
   /**
    * Class constructor
    *
    * @param aObserver   The observer that want to get notified when
    *                    a async fetch has completed.
    * @param aPriotiry   The priority of the active object.
    */
   CNetworkInfoHandler(class MNetworkInfoObserver* aObserver, 
                       enum TPriority aPriority);

private:
   /**
    * Symbian second stage ConstructL.
    */
   void ConstructL();

public:
   /**
    * Starts fetching all network information via a series of async calls 
    * to CTelephony api.
    */
   void FetchAllNetworkInfoL();

private:
   /**
    * Function called when the calls to get all the network info has 
    * completed without errors. Called by RunL as 
    * a result of a previous call to FetchAllNetworkInfoL().
    * Notifies the observer.
    */
   void FetchingDone();

   /**
    * Function called when any call to get network info has 
    * completed with errors. Called by RunL as 
    * a result of a previous call to FetchAllNetworkInfoL().
    * Notifies the observer.
    */
   void FetchingFailed();

   /**
    * Gets the signal strength asynchronously.
    */
   void GetSignalStrength();

   /**
    * Gets the registration status asynchronously.
    */
   void GetRegistrationStatus();

   /**
    * Function that issues the async call to get the network information.
    * When we return in RunL ProcessNetworkInfo and FetchingDone() gets called.
    */
   void GetNetworkInfo();

   /**
    * Collects all the network information available in to member variables.
    */
   void ProcessNetworkInfo();

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
   /// The observer which gets notified when the async request completes.
   class MNetworkInfoObserver* iObserver;

   /// Internal state variable
   enum TNetworkInfoState iState;

   /// The object to get information from the phone.
   class CTelephony* iTelephony;

   /// Contains signal indicator parameters.
   CTelephony::TIndicatorV1 iIndicatorV1;

   /// Package container for iIndicatorV1.
   CTelephony::TIndicatorV1Pckg iIndicatorV1Pckg;

   /// Defines the phone's current signal strength.
   CTelephony::TSignalStrengthV1 iSigStrengthV1;

   /// Package container for iSigStrengthV1.
   CTelephony::TSignalStrengthV1Pckg iSigStrengthV1Pckg;

   /// Defines the current network registration status.
   CTelephony::TNetworkRegistrationV1 iNetworkRegistrationV1;

   /// Package container for iNetworkRegistrationV1.
   CTelephony::TNetworkRegistrationV1Pckg iNetworkRegistrationV1Pckg;

   /// Contains information about the mobile phone network.
   CTelephony::TNetworkInfoV1 iNetworkInfoV1;

   /// Package container for TNetworkInfoV1.
   CTelephony::TNetworkInfoV1Pckg iNetworkInfoV1Pckg;

public:
   /// Signal strength in (dBm)
   TInt32 iSigStrength;

   /// Registration status of the phone (no service, roaming, busy, etc.).
   CTelephony::TRegistrationStatus iRegStatus;

   /// The network mode (GSM, CDMA, etc.).
   CTelephony::TNetworkMode iNetworkMode;

   /// The country code of the network (MCC).
   TBuf<CTelephony::KNetworkCountryCodeSize> iMcc;

   /// The network identity (NID in CDMA and MNC in GSM).
   TBuf<CTelephony::KNetworkIdentitySize> iMnc;

   /// The location area code (LAC), only valid if iNetworkAccess is true.
   TUint iLac;

   /// The Cell identity code (CellId), only valid if iNetworkAccess is true.
   TUint iCellId;

   /// The type of network access on GSM networks (GSM, UMTS).
   CTelephony::TNetworkAccess iNetworkAccess;

   /// The System id of CDMA networks (SID).
   TBuf<CTelephony::KNetworkIdentitySize> iCdmaSID;

   /// The Band id of CDMA networks (BID).
   CTelephony::TNetworkBandInfo iCdmaBID;
};

#endif // NETWORK_INFO_HANDLER_H
