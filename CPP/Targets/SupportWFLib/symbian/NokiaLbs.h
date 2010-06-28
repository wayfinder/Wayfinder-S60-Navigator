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

#ifndef NokiaLbs_H 
#define NokiaLbs_H

class MWfNokiaLbsOwner {
   public:
      enum GpsState { 
         EUnknownInStatus,    // 0
         EUnknownInStatus2,   // 1
         EUnknownInStatus3,
         EUnknownInPosition,
         EBroken,
         EUserDisabled,       // 5
         EPreStartup,
         EStartup,
         EAvailable,          // 8
         EUnsupported,
         EAccessDenied,       // 10
         ENotFound,
         EUserCanceledBT,     // 12
         EStatusCanceled,
         EDisconnectCalled,
         EPosBufOverflow,     // 15
         ENewModuleId,        // 16
         EGpsBadData,
         EGpsGotFirstPos      // 18
      };

      virtual void NokiaLbsUpdate(GpsState newState) = 0;
};

// #if ! (defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_UIQ /*|| defined NAV2_CLIENT_UIQ3*/)

#if defined NAV2_CLIENT_UIQ33 || defined NAV2_CLIENT_SERIES60_V3

#include <stdio.h>
#include <lbs.h>
#include <lbscommon.h>
#include <lbssatellite.h>
#include "PositionInterface.h"
#include "Nav2.h"
#include "TimeOutNotify.h"
#include "ActiveLog.h"

#undef DELAY_AFTER_SETUP
#ifdef DELAY_AFTER_SETUP
class CTimeOutTimer;
#endif

class CModuleStatusReceiver;

class CNokiaLbsReader : public MPositionInterface,
                        public CActiveLog
#ifdef DELAY_AFTER_SETUP
                      , public MTimeOutNotify
#endif
{
public: // Constructors & Destructor
   ///Static contructor. Leaves if necessary.
   ///@param aAppUi reference to a AppUi class. Used for debuglogging.
   ///@param btCh pointer to a Nav2Channel. This is where all positions 
   ///            are sent.
   ///@return a brand new CNokiaLbsReader object.
   static class CNokiaLbsReader* NewL(class MWfNokiaLbsOwner &aAppUi,
                                      class isab::Nav2::Channel *btCh,
                                      const TDesC& aRequestorInformation);
   ///Static contructor. Leaves if necessary. Pushes the constructed
   ///object onto the CleanupStack.
   ///@param aAppUi reference to a AppUi class. Used for debuglogging.
   ///@param btCh pointer to a Nav2Channel. This is where all positions 
   ///            are sent.
   ///@return a brand new CNokiaLbsReader object, pushed onto the 
   ///        CleanupStack.
   static class CNokiaLbsReader* NewLC(class MWfNokiaLbsOwner &aAppUi,
                                       class isab::Nav2::Channel *btCh,
                                       const TDesC& aRequestorInformation);
   ///Destructor.
   virtual ~CNokiaLbsReader();
   ///Second part of the two phase contruction.
   void ConstructL(const TDesC& aRequestorInformation);

protected: // Constructors & Destructor
   ///Constructor
   ///@param aAppUi reference to a AppUi class. Used for debuglogging.
   ///@param btCh pointer to a Nav2Channel. This is where all positions 
   ///            are sent.
   CNokiaLbsReader(class MWfNokiaLbsOwner &aAppUi, isab::Nav2::Channel *btCh);


public: // new methods
   virtual void ConnectL();
private:
   void ConnectPhase2L();
public:
   virtual void DisconnectL();
   virtual TBool IsConnected() const;
   virtual TBool IsBusy() const;
   virtual TBool IsInStartUp() const;
protected: // from CActive
   
   /**
    * DoCancel
    * cancel any outstanding operation
    */
   void DoCancel();
   
   /**
    * RunL
    * called when operation complete
    */
   void RunL();
   
private: // New methods
   void SendGpsPositionL();
   void SendGpsBadStatusL();
   void MaybeNewModuleId(TPositionModuleId aModuleId);
   
#ifdef DELAY_AFTER_SETUP
   public: // Form TimeOutNotify
   virtual void TimerExpired();
#endif

private:
   /** @name From CActiveLog */
   //@{
   virtual CArrayPtr<CActiveLog>* SubLogArrayLC();
   //@}

private: // Member variables
   class MWfNokiaLbsOwner &iAppUi;
   class RPositionServer m_lbsServer;
   class RPositioner     m_lbs;
   //TPositionCourseInfo m_posInfo;
   class TPositionSatelliteInfo m_posInfo;
   class isab::Nav2::Channel *m_btCh;
   TBool m_connected;
#ifdef DELAY_AFTER_SETUP
   class CTimeOutTimer * iTimer;
#endif
   TPositionModuleId iModuleId;
   class CModuleStatusReceiver * iStatusReceiver;
   TBool iGotFirstPos;
   HBufC* iRequestorInformation;
   TBool m_inStartUp;
} ;


#endif /* NAV2_CLIENT_SERIES90_V1 */
#endif /* NokiaLbs_H */

