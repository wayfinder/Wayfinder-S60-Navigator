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

// #if ! (defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_UIQ /*|| defined NAV2_CLIENT_UIQ3*/)
#if defined NAV2_CLIENT_UIQ33 || defined NAV2_CLIENT_SERIES60_V3
#include "NokiaLbs.h"

#include <e32math.h>

#if defined NAV2_CLIENT_UIQ33
# include <lbs/lbsadmin.h>
#endif

#include "TimeOutTimer.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

#ifndef M_PI
# define M_PI      3.14159265358979323846
#endif

#define NMEA_STYLE

//#define BT_GPS_PSY_ID 0x101fd1df

class CModuleStatusReceiver : public CActiveLog
{
   public:
      static class CModuleStatusReceiver* NewL(class MWfNokiaLbsOwner& aMaster,
                                               class CNokiaLbsReader & aReader,
                                               class RPositionServer & aServ, 
                                               TPositionModuleId aModuleId);
      virtual ~CModuleStatusReceiver();

      virtual void DoCancel();
      virtual void RunL();

   TBool IsModuleAvailable() const;

   protected:
      CModuleStatusReceiver(class MWfNokiaLbsOwner & aMaster, 
                            class CNokiaLbsReader & aReader,
                            class RPositionServer & aServ,
                            TPositionModuleId aModuleId);
      void ConstructL();
      TBool NotifyMaster(TPositionModuleStatus::TDeviceStatus aStatus);

   private:
      class MWfNokiaLbsOwner & iMaster;
      class CNokiaLbsReader & iReader;
      class RPositionServer & iServer;
      TPositionModuleStatusEvent iEvent;
      TPositionModuleId iModuleId;
      TBool iIsAvailable;
};


CModuleStatusReceiver::CModuleStatusReceiver(class MWfNokiaLbsOwner & aMaster, 
                                             class CNokiaLbsReader & aReader,
                                             class RPositionServer & aServ,
                                             TPositionModuleId aModuleId) :
   CActiveLog(EPriorityUserInput, "LbsStatus"), iMaster(aMaster), 
   iReader(aReader), iServer(aServ), iModuleId(aModuleId), 
   iIsAvailable(EFalse)
{
}

CModuleStatusReceiver::~CModuleStatusReceiver()
{
   DBG("~CModuleStatusReceiver");
   if(IsActive()){
      DBG("Destroying while active");
      Cancel();
      DBG("Cancel complete");
   }
}

void CModuleStatusReceiver::ConstructL()
{
   CActiveScheduler::Add(this);

   class TPositionModuleStatus status;
   TInt retval = iServer.GetModuleStatus(status, iModuleId);

   if (retval == KErrNotFound) {
      ERR("GetModuleStatus KErrNotFound. Sending ENotFound to master");
      // No good can come of this - do not try any more
      iMaster.NokiaLbsUpdate(MWfNokiaLbsOwner::ENotFound);
      // FIXME - disconnect here?
      return;
   }

   if (retval != KErrNone) {
      ERR("GetModuleStatus returned %d. Sending EUnknownInStatus2 to master", 
          retval);
      // Program error - Leave? Panic?
      iMaster.NokiaLbsUpdate(MWfNokiaLbsOwner::EUnknownInStatus2);
      ERR("Leaving with %d", retval);
      // FIXME - disconnect here?
      User::Leave(retval);
   }

   DBG("Notify master %d", int(status.DeviceStatus()));
   NotifyMaster(status.DeviceStatus());

   iEvent = TPositionModuleStatusEvent(TPositionModuleStatusEvent::EEventDeviceStatus);
   iServer.NotifyModuleStatusEvent(iEvent, iStatus, iModuleId);
   DBG("Active with NotifyModuleStatusEvent");
   SetActive();
}

class CModuleStatusReceiver * 
CModuleStatusReceiver::NewL(class MWfNokiaLbsOwner & aMaster, 
                            class CNokiaLbsReader & aReader,
                            class RPositionServer & aServ,
                            TPositionModuleId aModuleId)
{
   class CModuleStatusReceiver* ret = 
      new (ELeave) CModuleStatusReceiver(aMaster, aReader, aServ, aModuleId);
   CleanupStack::PushL(ret);
   ret->ConstructL();
   CleanupStack::Pop(ret);
   return ret;
}

void CModuleStatusReceiver::DoCancel()
{
   DBG("DoCancel start, CancelRequest");
   iServer.CancelRequest(EPositionServerNotifyModuleStatusEvent);
   DBG("DoCancel complete");
}

void CModuleStatusReceiver::RunL()
{
   DBG("RunL: iStatus : %d", iStatus.Int());
   if (iStatus == KErrNotFound) {
      ERR("KErrNotFound => ENotFound");
      // No good can come of this - do not try any more
      iMaster.NokiaLbsUpdate(MWfNokiaLbsOwner::ENotFound);
      // FIXME - disconnect here?
      return;
   }

   if (iStatus == KErrCancel) {
      ERR("KErrCancel => EStatusCanceled");
      iMaster.NokiaLbsUpdate(MWfNokiaLbsOwner::EStatusCanceled);
      // FIXME - disconnect here?
      return;
   }

   if (iStatus != KErrNone) {
      ERR("%d => EStatusCanceled", iStatus.Int());
      // Brokenness - ignore silently
      iMaster.NokiaLbsUpdate(MWfNokiaLbsOwner::EStatusCanceled);
      // FIXME - disconnect here?
      return;
   }
   TBool disconnected = EFalse;
   if (iEvent.OccurredEvents() & TPositionModuleStatusEvent::EEventDeviceStatus) {
      TPositionModuleStatus status;
      iEvent.GetModuleStatus(status);
      DBG("NotifyMaster %d", int(status.DeviceStatus()));
      disconnected = NotifyMaster(status.DeviceStatus());
   }
   if (!disconnected) {
      // Reissue the request.
      iEvent = TPositionModuleStatusEvent(TPositionModuleStatusEvent::EEventDeviceStatus);
      iServer.NotifyModuleStatusEvent(iEvent, iStatus, iModuleId);
      DBG("Avtive with NotifyModuleStatusEvent");
      SetActive();
   }
}

TBool CModuleStatusReceiver::IsModuleAvailable() const
{ 
   return iIsAvailable;
}

TBool CModuleStatusReceiver::NotifyMaster(TPositionModuleStatus::TDeviceStatus aStatus)
{
   MWfNokiaLbsOwner::GpsState state;
   TBool disconnectAfterStatus = EFalse;

   switch (aStatus) {
      case TPositionModuleStatus::EDeviceUnknown:
      case TPositionModuleStatus::EDeviceError:
         state = MWfNokiaLbsOwner::EBroken;
         iIsAvailable = EFalse;
         // FIXME - disconnect here?
         break;
      case TPositionModuleStatus::EDeviceDisabled:
         state = MWfNokiaLbsOwner::EUserDisabled;
         iIsAvailable = EFalse;
         if (!iReader.IsInStartUp()) {
            // Only do the disconnect if this happens during normal operation.
            // If we're in startup we dont disconnect since it might be that 
            // the phone just havnt realised that the internal gps is activated.
            // If we're in startup we crash if we disconnect since we have 
            // circular dependencies between this class and CNokiaLbsReader.
            disconnectAfterStatus = ETrue;
         }
         break;
      case TPositionModuleStatus::EDeviceInactive:
      case TPositionModuleStatus::EDeviceInitialising:
         state = MWfNokiaLbsOwner::EStartup;
         iIsAvailable = EFalse;
         break;
      case TPositionModuleStatus::EDeviceStandBy:
      case TPositionModuleStatus::EDeviceReady:
      case TPositionModuleStatus::EDeviceActive:
         state = MWfNokiaLbsOwner::EAvailable;
         iIsAvailable = ETrue;
         break;
      default:
         state = MWfNokiaLbsOwner::EUnknownInStatus;
         // No change of iIsAvailable
         break;
   }

   DBG("LbsUpdate %d", int(state));
   iMaster.NokiaLbsUpdate(state);
   if (disconnectAfterStatus) {
      DBG("Disconnecting after status");
      iReader.DisconnectL();
   }
   return disconnectAfterStatus;
}


// For the simulation only
// #include <oem/epos_tpossimpsysettings.h>
// #include <oem/epos_rpossettingsfile.h>

class CNokiaLbsReader * CNokiaLbsReader::NewL(class MWfNokiaLbsOwner &aAppUi, 
                                              class isab::Nav2::Channel *btCh,
                                              const TDesC& aRequestorInformation)
{
   CNokiaLbsReader* self = CNokiaLbsReader::NewLC( aAppUi, btCh, aRequestorInformation );
   CleanupStack::Pop();
   return self;
}

class CNokiaLbsReader* CNokiaLbsReader::NewLC(class MWfNokiaLbsOwner &aAppUi, 
                                              class isab::Nav2::Channel *btCh,
                                              const TDesC& aRequestorInformation)
{
   CNokiaLbsReader* self = new (ELeave) CNokiaLbsReader( aAppUi, btCh );
   CleanupStack::PushL(self);
   self->ConstructL(aRequestorInformation);
   return self;
}

CNokiaLbsReader::CNokiaLbsReader( class MWfNokiaLbsOwner &aAppUi,  
                                  isab::Nav2::Channel *btCh) : 
   CActiveLog(EPriorityStandard, "LbsReader"), iAppUi(aAppUi), 
   m_btCh(btCh), m_connected(EFalse)
#ifdef DELAY_AFTER_SETUP 
   , iTimer(NULL)
#endif
   , iStatusReceiver(NULL), iGotFirstPos(EFalse)
{
}

CNokiaLbsReader::~CNokiaLbsReader()
{
   DBG("~CNokiaLbsReader");
   DisconnectL();  
   //m_btch is not armed, so we don't need to cancel
   // XXX disconnect will result in a Kern-Exec 0 when shutting down 
   // wayfinder, Nav2 will do disconnect in function Nav2::Halt
//    m_btCh->disconnect();
   if (iStatusReceiver) {
      delete iStatusReceiver;
      iStatusReceiver = NULL;
   }
   delete m_btCh;
   delete iRequestorInformation;
   DBG("~CNokiaLbsReader complete.");
}


void CNokiaLbsReader::ConstructL(const TDesC& aRequestorInformation)
{
   if (iRequestorInformation) {
      delete iRequestorInformation;
      iRequestorInformation = NULL;
   }
   iRequestorInformation = aRequestorInformation.AllocL();
   CActiveScheduler::Add(this);
}

TBool CNokiaLbsReader::IsBusy() const
{
   return EFalse;
}

void CNokiaLbsReader::ConnectL()
{
   m_inStartUp = ETrue;
   DBG("ConnectL beginning");
   if (m_connected) {
      // This is done by security, if doing a connect when allready 
      // connected we will get a leave. Since we may need to connect
      // again if user has pressed cancel on a bt search we are allready
      // connected but we need to do this all over again for making
      // mlfw do a bt search.
      DisconnectL();
   }

   User::LeaveIfError(m_lbsServer.Connect());
   //CleanupClosePushL(m_lbsServer);

   User::LeaveIfError(m_lbs.Open(m_lbsServer));

#ifndef NAV2_CLIENT_UIQ33
   /// XXX Temporary until SE sorted this problem out
   User::LeaveIfError(m_lbs.SetRequestor(CRequestor::ERequestorService,
                                         CRequestor::EFormatApplication,
                                         *iRequestorInformation));

   TPositionUpdateOptions opts(TInt64(1*1000*1000), //Want a pos every second 
                               TInt64(60*1000*1000),//May wait indefinitly for it 
                               TInt64(500*1000),     // Do not use cached data 
                               ETrue);              //Accept partial fixes 
#else

   CLbsAdmin* admin = CLbsAdmin::NewL();
   CleanupStack::PushL(admin);
   admin->Set(KLbsSettingRoamingSelfLocate, CLbsAdmin::ESelfLocateOn);
   CleanupStack::PopAndDestroy(admin);

   TPositionUpdateOptions opts(TInt64(1*1000*1000), //Want a pos every second 
                               TInt64(10*1000*1000),//May wait indefinitly for it 
                               TInt64(0),     // Do not use cached data 
                               EFalse);              //Accept partial fixes 
#endif
   
   // Ok if this fails - I think!
   m_lbs.SetUpdateOptions(opts);

#ifdef DELAY_AFTER_SETUP
   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this, NULL);
   iTimer->After(10*1000*1000);
#else
   ConnectPhase2L();
#endif
   DBG("ConstructL done");
   m_inStartUp = EFalse;
}

void CNokiaLbsReader::ConnectPhase2L()
{
   DBG("ConnectPhase2L start");
   iGotFirstPos = EFalse;
   m_lbs.NotifyPositionUpdate(m_posInfo, iStatus);
   DBG("Active with NotifyPositionUpdate");
   SetActive();
   m_connected = ETrue;
   DBG("Lbsupdate EPreStartup");
   iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EPreStartup);

   if (iStatusReceiver) {
      delete iStatusReceiver;
      iStatusReceiver = NULL;
   }

   if (KErrNone == m_lbsServer.GetDefaultModuleId( iModuleId )) {
      iStatusReceiver = CModuleStatusReceiver::NewL(iAppUi, *this, m_lbsServer, iModuleId );
      UpdateLogMastersL();
   }
}

#ifdef DELAY_AFTER_SETUP
void CNokiaLbsReader::TimerExpired()
{
   ConnectPhase2L();
//   delete iTimer;
}
#endif

void CNokiaLbsReader::DisconnectL()
{
   //DBG("Disconnect() this:%p", (void*)this);
   m_connected = EFalse;
   iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EDisconnectCalled);
//    if (iStatusReceiver) {
//       delete iStatusReceiver;
//       iStatusReceiver = NULL;
//    }
   if (iStatusReceiver && iStatusReceiver->IsActive()) {
      iStatusReceiver->Cancel();
   }
   iGotFirstPos = EFalse;
   if (IsActive()) {
      Cancel();
   }
   m_lbs.Close();
   m_lbsServer.Close();
   //DBG("Disconnect done");
}

TBool CNokiaLbsReader::IsConnected() const
{
   return m_connected;
}

TBool CNokiaLbsReader::IsInStartUp() const
{
   return m_inStartUp;
}

// from CActive
void CNokiaLbsReader::DoCancel()
{
   //DBG("DoCancel");
   m_lbs.CancelRequest(EPositionerNotifyPositionUpdate);
}

// from CActive
void CNokiaLbsReader::RunL()
{
   DBG("RunL %d", iStatus.Int());

   if (iStatus == KErrNone) {
      // Got valid position
//      iAppUi.ShowDebugDialogL("None");
      MaybeNewModuleId(m_posInfo.ModuleId());
      if (iStatusReceiver && iStatusReceiver->IsModuleAvailable()) {
         SendGpsPositionL();
      }
   } else if (iStatus == KPositionQualityLoss) {
//      iAppUi.ShowDebugDialogL("QualityLoss");
      if (iStatusReceiver && iStatusReceiver->IsModuleAvailable()) {
         SendGpsBadStatusL();
      }
   } else if (iStatus == KPositionPartialUpdate) {
//      iAppUi.ShowDebugDialogL("PartialUpdate");
      MaybeNewModuleId(m_posInfo.ModuleId());
      if (iStatusReceiver && iStatusReceiver->IsModuleAvailable()) {
         SendGpsPositionL();
      }
   } else if (iStatus == KErrCancel) {
//      iAppUi.ShowDebugDialogL("Cancel");
      // Do not restart everything
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EUserCanceledBT);
      DisconnectL();
      return;
   } else if (iStatus == KErrTimedOut) {
//      iAppUi.ShowDebugDialogL("TimedOut");
      // Assume not an error, restart the query
   } else if (iStatus == KErrArgument) {
//      iAppUi.ShowDebugDialogL("Argument");
      // The requested informtaion package is not supported by this module
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EUnsupported);
      DisconnectL();
      return;
   } else if (iStatus == KErrAccessDenied) {
//      iAppUi.ShowDebugDialogL("AccessDenied");
      // User prevented us from getting a position
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EAccessDenied);
      DisconnectL();
      return;
   } else if (iStatus == KErrPositionBufferOverflow) {
//      iAppUi.ShowDebugDialogL("BufferOverflow");
      // This can only happen with HGenericPositionInfo.
      // Abort silently
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EPosBufOverflow);
      DisconnectL();
      return;
   } else if (iStatus == KErrUnknown) {
//      iAppUi.ShowDebugDialogL("KErrUnknown");
      // This actually happens with the BT GPS when corrupt nmea data is received.
      // Restart the query.
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EGpsBadData);
   } else if (iStatus == KErrNotFound) {
      // The lbs was not found (e.g. turned off) report it to observer.
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::ENotFound);
      DisconnectL();
      return;
   } else {
//      iAppUi.ShowDebugDialogL("UnknownError");
      // Unhandled error - what to do? Abort for now without
      // restarting the active object.
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EUnknownInPosition);
      DisconnectL();
      int a=1;
      if (a) return;
   }

//   iAppUi.ShowDebugDialogL("Restart");
   m_lbs.NotifyPositionUpdate(m_posInfo, iStatus);
   DBG("active with notifypositionupdate ");
   SetActive();
   //DBG("RunL ends.");
}

void CNokiaLbsReader::MaybeNewModuleId(TPositionModuleId aModuleId)
{
   if (aModuleId != iModuleId) {
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::ENewModuleId);
      iModuleId = aModuleId;
      delete iStatusReceiver;
      iStatusReceiver = NULL;
   }
   if (! iStatusReceiver ) {
      iStatusReceiver = CModuleStatusReceiver::NewL(iAppUi, *this, m_lbsServer, iModuleId );
      UpdateLogMastersL();
   }
}

void CNokiaLbsReader::SendGpsPositionL()
{
   class TPosition pos;
   class TCourse course;

   m_posInfo.GetPosition(pos);
   m_posInfo.GetCourse(course);

   if (Math::IsNaN(pos.Latitude()) ||
       Math::IsNaN(pos.Longitude()) ||
       Math::IsNaN(course.Speed())  ||
       Math::IsNaN(course.Heading())) {
      //Partial update - send dummy data
      DBG("SendGpsBadStatusL");
      SendGpsBadStatusL();
      return;
   }

   // Notify the master if this is the first gps position. 
   if ( ! iGotFirstPos ) {
      iGotFirstPos = ETrue;
      DBG("First position, Lbsupdate EGpsGotFirstPos");
      iAppUi.NokiaLbsUpdate(MWfNokiaLbsOwner::EGpsGotFirstPos);
   }
   
#ifdef MOTOROLA_STYLE
   int lat = int( pos.Latitude()  * 6000000.0);
   int lon = int( pos.Longitude() * 6000000.0);
   int alt = 0;
   int spd = int(course.Speed() * 1000.0) /* 2*1000 */;
   int head = int(course.Heading() * 1000.0) /* 0 */;
   int quality = 5;   // No idea what this is supposed to mean

   char msg[100];
   sprintf(msg,"^Start%020i;%010i;%010i;%010i;%010i;%010i;%010iEND",0,quality,lat,lon,alt,spd,head);
   m_btCh->writeData((uint8*)(msg), strlen(msg)+1);
#elif defined NMEA_STYLE
   class TDateTime time = pos.Time().DateTime();
   TBool has3D = !(Math::IsNaN(pos.Altitude()));
   TBool north = ETrue, east = ETrue;
   TReal64 lat = pos.Latitude();
   if(lat < 0){
      north = EFalse;
      lat = -lat;
   }
   TReal64 lon = pos.Longitude();
   if(lon < 0){
      east = EFalse;
      lon = -lon;
   }
   TReal32 spd = course.Speed();
   TReal32 hdg = course.Heading();

   TReal32 pdop = 10.0;
   if ( ! Math::IsNaN(m_posInfo.HorizontalDoP())) {
      pdop = m_posInfo.HorizontalDoP();
   }
   
   TInt year = time.Year() - ((time.Year()/100)*100);
                              // + 100*time.Month() + 10000*(time.Day()+1);
   TInt32 deglat, deglon;
   Math::Int(deglat, lat);
   Math::Int(deglon, lon);

# if 1
   char msgbuf[384];
   char* msgGPGSA = msgbuf;

   TReal64 minlatfloat, minlonfloat;
   minlatfloat = (lat-deglat)*60;
   minlonfloat = (lon-deglon)*60;

   TInt32 minlat, minlon;
   Math::Int(minlat, minlatfloat);
   Math::Int(minlon, minlonfloat);

   TInt32 minlatfrac, minlonfrac;
   Math::Frac(minlatfloat, minlatfloat);
   Math::Frac(minlonfloat, minlonfloat);
   Math::Int(minlatfrac, minlatfloat*10000);
   Math::Int(minlonfrac, minlonfloat*10000);

   snprintf(msgGPGSA, 128, "$GPGSA;M;%c;;;;;;;;;;;;;%f;;*\n\r", has3D ? '3' : '2', pdop);
   char from[] = ",;";
   char to[] = ".,";
   size_t i = 0;
   for(i = 0; i < strlen(from); ++i){
      char* pos = strchr(msgGPGSA, int(from[i]));
      while(pos != NULL){
         *pos = to[i];
         pos = strchr(pos, from[i]);
      }
   }

   char* msgGPRMC = msgbuf + strlen(msgGPGSA);


   //   m_btCh->writeData((uint8*)(msgGPGSA), strlen(msgGPGSA)+1);


   snprintf(msgGPRMC,128,
            "$GPRMC;%02u%02u%02u;%c;%02u%02u.%04u;%c;%03u%02u.%04u;%c;%f;%f;%02u%02u%02u;;*\n\r",
            //"$GPRMC;%02u%02u%02u;%c;%02u%09.6f;%c;%03u%09.6f;%c;%f;%f;%02u%02u%02u;;*\n\r",
            //"$GPRMC,%u%u%u,A,%02u%09.6f,%c,%03u%09.6f,%c,%f,%u%u%u,,*\n\r", 
            time.Hour(), time.Minute(), time.Second(),
            'A',
            deglat, minlat, minlatfrac, north ? 'N' : 'S', 
            deglon, minlon, minlonfrac, east ? 'E' : 'W', 
            1.9438445*spd, hdg, 
            time.Day()+1, time.Month()+1, year);
   //char from[] = ",;";
   //char to[] = ".,";
   for(i = 0; i < strlen(from); ++i){
      char* pos = strchr(msgGPRMC, int(from[i]));
      while(pos != NULL){
         *pos = to[i];
         pos = strchr(pos, from[i]);
      }
   }

   char* msgGPGGA = msgbuf + strlen(msgbuf);//strlen(msgGPGSA) + strlen(msgGPRMC);


   //   m_btCh->writeData((uint8*)(msgGPGSA), strlen(msgGPGSA)+1);
   snprintf(msgGPGGA,128,
            "$GPGGA;%02u%02u%02u;%02u%02u.%04u;%c;%03u%02u.%04u;%c;1;04;1.0;%f;M;;;;0000*\n\r",
            time.Hour(), time.Minute(), time.Second(),
            deglat, minlat, minlatfrac, north ? 'N' : 'S', 
            deglon, minlon, minlonfrac, east ? 'E' : 'W', 
            pos.Altitude());

   for(i = 0; i < strlen(from); ++i){
      char* pos = strchr(msgGPGGA, int(from[i]));
      while(pos != NULL){
         *pos = to[i];
         pos = strchr(pos, from[i]);
      }
   }

   DBG("Writing %d bytes to btCh", strlen(msgbuf));
   m_btCh->writeData((uint8*)(msgbuf), strlen(msgbuf));
   DBG("data written");
# else
   TBuf8<128> msg;


   //
   // Send GSA
   //
   TRealFormat conv_form_dop;
   conv_form_dop.iPlaces=1;
   conv_form_dop.iPoint='.';
   conv_form_dop.iTriLen=0;
   conv_form_dop.iType=KRealFormatFixed | KDoNotUseTriads;
   conv_form_dop.iWidth=KDefaultRealWidth;

   snprintf(msg, 128, "$GPGSA;M;%c;;;;;;;;;;;;;%f;;*\n\r", has3D ? '3' : '2', m_posInfo.HorizontalDoP());
   msg.Copy( _L8("$GPGSA,M,") );
   if (has3D) {
      msg.Append( _L8("3"));
   } else {
      msg.Append( _L8("2"));
   }
   msg.Append( _L8(",,,,,,,,,,,,,"));
   msg.AppendNum( m_posInfo.HorizontalDoP(), conv_form_dop);
   msg.Append( _L8(",,*\n\r"));

   m_btCh->writeData(msg.PtrZ(), msg.Length()+1);

   //
   // Send RMC
   //
   TReal64 minlatfloat, minlonfloat;
   minlatfloat = (lat-deglat)*60;
   minlonfloat = (lon-deglon)*60;

   TInt32 minlat, minlon;
   Math::Int(minlat, minlatfloat);
   Math::Int(minlon, minlonfloat);

   TInt32 minlatfrac, minlonfrac;
   Math::Frac(minlatfloat, minlatfloat);
   Math::Frac(minlonfloat, minlonfloat);
   Math::Int(minlatfrac, minlatfloat*10000);
   Math::Int(minlonfrac, minlonfloat*10000);

   msg.Copy( _L8("$GPRMC;") );
   msg.AppendNumFixedWidth(time.Hour()  , EDecimal, 2);
   msg.AppendNumFixedWidth(time.Minute(), EDecimal, 2);
   msg.AppendNumFixedWidth(time.Second(), EDecimal, 2);

   msg.Append(_L8(",A,"));

   msg.AppendNumFixedWidth(deglat, EDecimal, 2);
   msg.AppendNumFixedWidth(minlat, EDecimal, 2);
   msg.Append(_L8("."));
   msg.AppendNumFixedWidth(minlatfrac, EDecimal, 4);
   msg.Append(_L8(","));
   if (north) {
      msg.Append(_L8("N"));
   } else {
      msg.Append(_L8("S"));
   }
   msg.Append(_L8(","));

   msg.AppendNumFixedWidth(deglon, EDecimal, 3);
   msg.AppendNumFixedWidth(minlon, EDecimal, 2);
   msg.Append(_L8("."));
   msg.AppendNumFixedWidth(minlonfrac, EDecimal, 4);
   msg.Append(_L8(","));
   if (east) {
      msg.Append(_L8("E"));
   } else {
      msg.Append(_L8("W"));
   }
   msg.Append(_L8(","));

   TRealFormat conv_form;
   conv_form.iPlaces=2;
   conv_form.iPoint='.';
   conv_form.iTriLen=0;
   conv_form.iType=KRealFormatFixed | KDoNotUseTriads;
   conv_form.iWidth=KDefaultRealWidth;

   msg.AppendNum(1.9438445*spd, conv_form);
   msg.Append(_L8(","));
   msg.AppendNum(hdg, conv_form);
   msg.Append(_L8(","));

   msg.AppendNumFixedWidth(time.Day()+1   , EDecimal, 2);
   msg.AppendNumFixedWidth(time.Month()+1 , EDecimal, 2);
   msg.AppendNumFixedWidth(year           , EDecimal, 2);
   msg.Append(_L8(",,*\n\r"));

   m_btCh->writeData(msg.PtrZ(), msg.Length()+1);
# endif
#else
# error No GPS style defined
#endif
}

void CNokiaLbsReader::SendGpsBadStatusL()
{
#ifdef MOTOROLA_STYLE
   int quality = 0;   // No position estimate
   char msg[100];
   sprintf(msg,"^Start%020i;%010i;%010i;%010i;%010i;%010i;%010iEND",0,quality,0,0,0,0,0);
   m_btCh->writeData((uint8*)(msg), strlen(msg)+1);
#elif defined NMEA_STYLE
   char msg[128] = {0};
   snprintf(msg, 128, "$GPGSA,M,2,,,,,,,,,,,,,%f,,*\n\r", 50.0);
   m_btCh->writeData((uint8*)(msg), strlen(msg));
   snprintf(msg,128,
            "$GPRMC;%02u%02u%02u;%c;0000.0000;N;00000.0000;E;%f;%f;%02u%02u%02u;;*\n\r",
            //time.Hour(), time.Minute(), time.Second(),
            //'A',
            //deglat, (lat - deglat)*60, north ? 'N' : 'S', 
            //deglon, (lon - deglon)*60,  east ? 'E' : 'W', 
            //1.9438445*spd, hdg, 
            //time.Day()+1, time.Month()+1, year);
            0, 0, 0,
            'V',
            0.0, 0.0, 
            0, 0, 0);
   char from[] = ",;";
   char to[] = ".,";
   for(size_t i = 0; i < strlen(from); ++i){
      char* pos = strchr(msg, int(from[i]));
      while(pos != NULL){
         *pos = to[i];
         pos = strchr(pos, from[i]);
      }
   }
   DBG("Writing %d bytes of bad data to btCh", strlen(msg)+1);
   m_btCh->writeData((uint8*)(msg), strlen(msg));
#else
# error No GPS style defined
#endif
}

CArrayPtr<CActiveLog>* CNokiaLbsReader::SubLogArrayLC()
{
   CArrayPtr<CActiveLog>* array = new (ELeave) CArrayPtrFlat<CActiveLog>(1);
   CleanupStack::PushL(array);
   array->AppendL(iStatusReceiver);
   return array;
}

#endif

