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

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "IAPConnMgrHandler.h"

#if defined (NAV2_CLIENT_UIQ) || defined(NAV2_CLIENT_SERIES60_V1)
# define USE_RAGENT
#elif defined (NAV2_CLIENT_SERIES60_V2) || defined (NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_SERIES90_V1) || defined (NAV2_CLIENT_UIQ3)
# define USE_RCONNECTION
#elif
# error Please choose connection manager, RConnection or RAgent!
#endif

class CConnectionManager : public CActive, public MIAPConnMgrHandler
{
   /** @name Constructors and destructor.*/
   //@{
   /** Default constructor. */
   CConnectionManager();
   /** Second phase constructor. */
   void ConstructL();

public:
   /**
    * Static constructor.
    * @return a new CConnectionManager object that has been pushed
    *         onto the CleanupStack.
    */
   static class CConnectionManager *NewLC();
   /**
    * Static constructor.
    * @return a new CConnectionManager object.
    */
   static class CConnectionManager *NewL();
   /** Virtual destructor. */
   virtual ~CConnectionManager();
   //@}
public:
   /** @name MIAPConnMgrHandler */
   //@{
   virtual void SetIAPConnMgrObserver(class MIAPConnMgrObserver* aObserver);
   virtual void SetIAP(const class CConnectionSettings& aSetting);
   virtual void Test();
   virtual void ConnMgrCancel();
   //@}

   void StartConnectionManager(TInt32 aIAPid);
   void StopConnectionManager();
   void OpenLinkLayerConnectionL();
   void OpenLinkLayerAgentL();

public: 
   /** @name CActive */
   //@{
   virtual void DoCancel();
   virtual void RunL();
   //@}
private:
   class MIAPConnMgrObserver* iObserver;
   class CConnectionSettings* iConnectionSetting;
#ifdef USE_RAGENT
   class CStoreableOverrideSettings *iSettings;
   class RGenericAgent* iAgent;
#endif
#ifdef USE_RCONNECTION
   class RConnection* iConnection;
   class RSocketServ* iSocketServ;
#endif

   TInt32 iIAP;
};



#endif
