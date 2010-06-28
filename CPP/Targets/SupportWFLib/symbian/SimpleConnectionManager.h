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

#ifndef SIMPLE_CONNECTION_MANAGER_H
#define SIMPLE_CONNECTION_MANAGER_H

#ifdef SYMBIAN_7S

#include <e32base.h>
#include <commdbconnpref.h>
#include <es_sock.h>

/**
 * Handles GPRS connections. Used to connect to the GPRS network
 * through a selected access point. This will speed up future TCP
 * connections.
 */
class CSimpleConnectionManager : public CActive
{
   /**
    * The index of the access point used for connections. 
    */
   TInt32 iIAP;

   /**
    * The connection manager. 
    */
   class RConnection iManager;
   /**
    * The Socket Server Session.
    */
   class RSocketServ iSession;
   /**
    * Connection preferences. 
    */
   class TCommDbConnPref iPrefs;

   /**
    * Start connection secuence.
    * @param aIAP the access point to connect through. A bit
    *        unnecessary as we already have the access point as a
    *        member variable.
    */
   void StartL();

   /** @name Constructors and destructor. */
   //@{
   /**
    * Constructor. 
    * @param aIAP the access point to connect through.
    */
   CSimpleConnectionManager(TInt32 aIAP);

   /**
    * Second phase constructor. 
    */
   void ConstructL();
public:
   /**
    * Static constructor. 
    * @param aIAP the access point to connect through.
    * @return a new CSimpleConnectionManager object.
    */
   static class CSimpleConnectionManager* NewL(TInt32 aIAP);
   /**
    * Virtual destructor. 
    */
   virtual ~CSimpleConnectionManager();
   //@}

   /**
    * @return the access point used by this CSimpleConnectionManager
    *         object.
    */
   TInt32 Iap() const;
private:
   /** 
    * Stuff done for hysterical reasons. 
    */
   void WhyDoWeDoThisL();
   /** @name From CActive. */
   //@{
   virtual TInt RunError(TInt aError);
protected:
   virtual void RunL();
   virtual void DoCancel();
   //@}
   
   /**
    * Enumerates the different states used during connection. 
    */
   enum TConnectionState{
      /** Connection not yet started. */
      EIdle = 0,
      /** Connection in progress. */
      EConnecting,
      /** Connection active.*/
      EConnected
   };
   /** Holds the current connection state. */
   enum TConnectionState iState;
};
#endif
#endif
