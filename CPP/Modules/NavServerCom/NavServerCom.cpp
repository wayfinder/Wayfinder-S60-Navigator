/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// This module handles communication with the Navigator Server.
#include "arch.h"
#include "Constants.h"
#include "Nav2Error.h"
#include "TopRegion.h"

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Matches.h"
#include "NavServerComEnums.h"
#include "MapEnums.h"
#include "GuiProt/Favorite.h"
#include "ErrorModule.h"
#include "NavPacket.h"

#include "GuiProt/ServerEnums.h"
#include "NavServerCom.h"

#include "Serial.h"
#include "CtrlHub.h"
#include "RequestList.h"
#include "Buffer.h"
#include "RouteEnums.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "NavTask.h"

#include "ParameterEnums.h"
#include "Parameter.h"
#include "ErrorModule.h"
#include "NavServerCom.h"
#include "NavServerComInternal.h"
#include "NavHandler.h"
#include "ServerParser.h"
#include "HttpServerParser.h"

#include "ParameterBlock.h"
#include "Server.h"

#include "TimeUtils.h"

#include "GetIMEI.h"

#include "MsgBuffer.h"

#include "GlobalData.h"

// defines all log macros such as DBG
#include "LogMacros.h"
#include <stdio.h>
#include "TraceMacros.h"

#ifndef UNDER_CE
# include <unistd.h>
# include <sys/stat.h>
# include <dirent.h>
#endif

#include "OfflineTester.h"

#include "Tracking.h"
#include "TraceMacros.h"

#include "WFID.h"

#define NAV_ROUTE_REPLY_HEADER_SIZE 35
#define USE_IMEI_NUMBER_AS_ID 1

#define VERSION_FILE_NAME "newversion.txt"

/* FIXME - design a proper bit field encoding */
#define NSC_REQ_STATUS_ORIG_FROM_GPS 1

#define IMEIXOR     \
"\x5A\x94\xF3\x31\xF3\x71\xDB\xE3\x15\x58\xAB\xB1\xB5\x4C\x4F\x21"  \
"\xF3\x55\x62\x5A\xC3\x44\xB6\xC4\x21\x93\x00\x3E\x3A\xF7\xDD\x5F"  
#define IMEIXORLEN 32

using namespace std;
using namespace isab::Nav2Error;

namespace isab {
   NavServerCom::NavServerCom(SerialProviderPublic* spp,
                              const char* serialNo,
                              const char* server, uint16 port,
                              uint32 majorRel, uint32 minorRel, 
                              uint32 build, uint32 majorResource,
                              uint32 minorResource, uint32 buildResource,
                              const char* clientType, 
                              const char* clientOptions, bool httpProto,
                              const char* httpUserAgent,
                              const char* httpRequest,
                              const char* httpFixedHost,
                              bool useTracking,
                              uint32 maxnbroutstanding,
                              const char* licenseKeyType,
                              bool wayfinderIDStartUp,
                              vector<HWID*> *hardwareIDs  ) :
      Module("NavServerCom"), m_ser_provider(spp),
      m_serialConsumer(NULL), m_reqID(0), 
      m_pendingList(0),
      m_requestList(8), m_connectState(Module::DISCONNECTING),
      m_navTaskProvider(NULL), m_paramProvider(NULL), 
      m_nscConsumer(NULL), m_paramBlock(NULL), m_webUser(NULL), 
      m_syncRequest(NULL),
      m_sessionState(noParams), m_parser(NULL), m_serverIdx(0),
      m_timeoutTimerID(0), m_timeoutTime(60 * 1000), m_fakeConnectTimer(0),
      m_miscTimerID(0), 
      m_pollOffset(0),
      m_longConnectionTimeout(uint32(-1)), 
      m_shortConnectionTimeout(uint32(-1)),
      m_disconnectStrategy(invalidStrategy), m_lastRequest(NAV_SERVER_INVALID),
      m_lastLat(MAX_INT32), m_lastLon(MAX_INT32), m_lastHeading(0),
      m_lastSpeed(0),
      m_lastCs(invalid),
      m_lastType(NAV_SERVER_INVALID),
      m_lastSent(MAX_UINT32),
      m_lastToSend(MAX_UINT32),
      m_routeType(TIME), 
      m_transportationType(passengerCar), 
      m_expireVector(GuiProtEnums::EXPIRE_VECTOR_LENGTH),
      m_imeiXor( (byte*)IMEIXOR ), m_imeiXorLen( IMEIXORLEN ),
      m_lastRouteAddress( 0 ), m_proxyServerAndPort( NULL ), 
      m_offlineTester(NULL), m_maxnbroutstanding( maxnbroutstanding )
   {
      //Set up connection with my serial provider.
      m_serialConsumer = new SerialConsumerPublic(m_queue);
      spp->setOwnerModule(this);
      spp->connect(m_serialConsumer);
      newLowerModule(spp);
      //Create necessary XYZPublic objects.
      m_navTaskProvider = new NavTaskProviderPublic(this);
      m_paramProvider   = new ParameterProviderPublic(this);
      m_nscConsumer     = new NavServerComConsumerPublic(this);
      //Create helper object.
      if ( httpProto ) {
         m_parser = new HttpServerParser( m_ser_provider, this,
                                          httpUserAgent, httpRequest, 
                                          httpFixedHost );
         m_serverListParam = 
            ParameterEnums::NSC_HttpServerHostname;
         m_serverListChecksumParam = 
            ParameterEnums::NSC_HttpServerListChecksum;
      } else {
         m_parser = new ServerParser( m_ser_provider, this );
         m_serverListParam = 
            ParameterEnums::NSC_ServerHostname;
         m_serverListChecksumParam = 
            ParameterEnums::NSC_ServerListChecksum;
      }

      //Set the license key type.
      m_parser->setLicenseKeyType( licenseKeyType ); 
      
      m_parser->setWayfinderIdStartUp( wayfinderIDStartUp );
      if( wayfinderIDStartUp ){
         // Set the license keys type. 
         for ( vector<HWID*>::iterator iteratorn = hardwareIDs->begin();
               iteratorn!= hardwareIDs->end(); ++iteratorn ){
            m_parser->AddHWID(*iteratorn);
         }
      }

      if ( useTracking ) {
         //TRACE_FUNC1( "Using tracking" );
         DBG( "Using tracking" );
         m_tracking = new Tracking( 
            m_queue, new NavServerComProviderPublic( this ), 
            m_paramProvider );
         m_parser->setTracking( m_tracking );
      } else {
         //TRACE_FUNC1( "No tracking" );
         DBG( "No tracking" );
         m_tracking = NULL;
      }
      m_servers = new std::vector<Server*>;
      if(server != NULL && port > 0){
         m_servers->push_back(new Server(server, port));
         (*m_servers)[0]->persistent = false;
      }
      //server parameters
      m_paramBlock = new ParameterBlock();
      uint32 version[3] = { majorRel, minorRel, build};
      if(majorRel > 0 || minorRel > 0){
         m_paramBlock->setProgramVersion(version);
      }
      if(majorResource > 0 || minorResource > 0){
         version[0] = majorResource;
         version[1] = minorResource;
         version[2] = buildResource;
         m_paramBlock->setResourcesVersion(version);
      }
      if(clientType != NULL){
         m_paramBlock->setClientType(clientType);
      }
      if(clientOptions != NULL){
         m_paramBlock->setClientTypeOptions(clientOptions);
      }
#ifdef MAX_SERVER_PACKET_SIZE
      m_paramBlock->setMaxPacketSize(MAX_SERVER_PACKET_SIZE);
#else 
      m_paramBlock->setMaxPacketSize( 15000 );
#endif
      isabTime now;
      SET(m_seed, now.millis(), now.millis());

      // set up progress message permission vector
#ifdef __linux
# define DEFAULT_PROGRESS true
#else
# define DEFAULT_PROGRESS false
#endif
      std::fill(m_csLookUp, 
                m_csLookUp + sizeof(m_csLookUp)/sizeof(*m_csLookUp), 
                DEFAULT_PROGRESS);
      m_csLookUp[NavServerComEnums::connecting] = true;
      m_csLookUp[NavServerComEnums::connected] = true;
      m_csLookUp[NavServerComEnums::done] = true;
      // These have been added so NavTask knows about connection
      // timeouts as it would otherwise keep rerouting even if
      // the connection is down.
      m_csLookUp[NavServerComEnums::connectionTimedOut] = true;
      m_csLookUp[NavServerComEnums::disconnectionError] = true;
      m_csLookUp[NavServerComEnums::unknownError] = true;

      if(serialNo && (strlen(serialNo) > 0)){
         DBG("Got machine id '%s'.", serialNo);
         m_paramBlock->setUserLicense((uint8*)(serialNo),
                                      strlen(serialNo)); //no \0-byte
      }
   }

   NavServerCom::~NavServerCom()
   {
      while(!m_servers->empty()){
         delete m_servers->back();
         m_servers->pop_back();
      }
      delete m_servers;
      delete[] m_webUser;
      delete [] m_proxyServerAndPort;
      delete m_tracking;
   }


   void NavServerCom::decodedStartupComplete()
   {
      //this srand call is for the call to rand in nextServer.
      isabTime now;
      srand(now.millis());
      Module::decodedStartupComplete();
      // Parameters we need and will subscribe to.
      ParameterEnums::ParamIds params[] = {ParameterEnums::Language,
                                ParameterEnums::NSC_AttachCrossingMaps,
                                ParameterEnums::NSC_CachedConnectionStrategy,
                                ParameterEnums::NSC_CallCenterChecksum,
                                ParameterEnums::NSC_CategoriesChecksum,
                                ParameterEnums::NSC_CrossingMapsImageFormat,
                                ParameterEnums::NSC_CrossingMapsSize,
                                ParameterEnums::NSC_ImageCrossingMaps,
                                ParameterEnums::NSC_LatestNewsChecksum,
                                ParameterEnums::NSC_NavigatorID, 
                                ParameterEnums::NSC_RouteCostType,
                                ParameterEnums::NSC_RouteTollRoads,
                                ParameterEnums::NSC_RouteHighways,
                                ParameterEnums::NSC_ServerListChecksum,
                                ParameterEnums::NSC_SubscriptionLeft,
                                ParameterEnums::NSC_TransactionsLeft,
                                ParameterEnums::NSC_TransportationType,
                                ParameterEnums::NSC_UserAndPasswd, 
                                ParameterEnums::NSC_ExpireVector,
                                ParameterEnums::TopRegionListLegacy,
                                ParameterEnums::TopRegionList,
                                ParameterEnums::UC_MaxNumberOfGuiSearchResults,
                                ParameterEnums::WayfinderType,
                                ParameterEnums::NSC_ServerHostname, 
                                //replaced by UC_MaxNumberOfGuiSearchResults
                                //ParameterEnums::NSC_MaxSearchMatches,
                                ParameterEnums::NSC_LastIMEI,
                                ParameterEnums::NSC_serverAuthBob,
                                ParameterEnums::NSC_serverAuthBobChecksum,
                                ParameterEnums::SelectedAccessPointId2,
      };
      
      if ( m_serverListParam == ParameterEnums::NSC_HttpServerHostname ) {
         // Change to Http Servers
         std::replace( params, params + sizeof(params)/sizeof(*params),
                       ParameterEnums::NSC_ServerHostname, 
                       ParameterEnums::NSC_HttpServerHostname );
         std::replace( params, params + sizeof(params)/sizeof(*params),
                       ParameterEnums::NSC_ServerListChecksum, 
                       ParameterEnums::NSC_HttpServerListChecksum );
      }
      //sort the list to facilitate joining
      std::sort(params, params + sizeof(params)/sizeof(*params));
      //for loop to get parameters and join parameter multicast groups
      uint16 last = 0;
      for(unsigned j = 0; j < sizeof(params)/sizeof(*params); ++j){
         uint16 group = ParameterProviderPublic::paramIdToMulticastGroup(params[j]);
         m_paramProvider->getParam(params[j]);
         if(group != last){
            m_rawRootPublic->manageMulticast(JOIN, group);
         }
         last = group;
      }
      //subscribe to navtask messages.
      m_rawRootPublic->manageMulticast(JOIN, Module::NavTaskMessages);

      //find IMEI number and fill into parameter block
      {
         bool isSet = false;
         int len = 0;
         m_paramBlock->getUserLicense( len, &isSet );
         //Don't GetIMEI if we already have it.
         if(!isSet){
            char* imei = GetIMEI();
            if(imei && (strlen(imei) > 0)){
               DBG("Got machine id.");
               DBG("Got IMEI result: \"%s\".", imei);
               m_paramBlock->setUserLicense((uint8*)(imei),
                                            strlen(imei)); //no \0-byte
               isSet = true;
            }
            delete[] imei;
         }
         if(!isSet){
            sendError(NSC_NO_LICENSE_ID, 0);
            sendError(PANIC_ABORT, 0);
            ERR("Unable to obtain IMEI");
         }
      }
      if ( m_tracking ) {
         m_tracking->decodedStartupComplete();
         // Tracking mentally projected the needs for TrackingClass
         // Is this needed or is getParam enough?
         m_rawRootPublic->manageMulticast(
            JOIN, ParameterEnums::TrackingClass );
      }

      m_offlineTester = new OfflineTester();
   }

   void NavServerCom::decodedShutdownPrepare( int16 upperTimeout )
   {
      //Really shut down.
      m_ser_provider->connectionCtrl(DISCONNECT, "FAKETIMEOUT");
      if ( m_tracking ) m_tracking->decodedShutdownPrepare( upperTimeout );
      updateState(shutdown);
      Module::decodedShutdownPrepare( upperTimeout );
   }


   void NavServerCom::decodedShutdownNow( int16 upperTimeout )
   {
      std::vector<Server*>::iterator s;
      for(s = m_servers->begin(); s != m_servers->end(); ++s){
         DBG("Delete Server %s (%p)", (**s).getHostAndPort(), *s);
         delete *s;
      }
      m_servers->clear();
      delete m_paramBlock;
      m_paramBlock = NULL;
      delete m_navTaskProvider;
      m_navTaskProvider = NULL;
      delete m_paramProvider;
      m_paramProvider = NULL;
      delete m_nscConsumer;
      m_nscConsumer = NULL;
      delete m_parser;
      m_parser = NULL;
      Module::decodedShutdownNow( upperTimeout );
   }

   void NavServerCom::treeIsShutdown()
   {//this function is called by Module after all underlying modules
    //have signalled shutdowncomplete.
      delete m_offlineTester;
      m_offlineTester = NULL;
      Module::treeIsShutdown();
   }

   void NavServerCom::decodedExpiredTimer(uint16 timerID)
   {
      if (timerID == m_timeoutTimerID && reallyTriggerTimout(timerID)) {
         // We add the if-guard reallyTriggerTimout to make sure we 
         // don't trigger the timeout by a race between timer thread 
         // and the dispatch queue.
         WARN("Server communication timeout. ID = %u",timerID);
         if(m_fakeConnectTimer != 0){
            WARN("removing Fake connection timer %u", timerID);
            m_queue->removeTimer(m_fakeConnectTimer);
            m_fakeConnectTimer = 0;
         }
         updateState(timeout);
         return;
      } else if (timerID == m_miscTimerID) {
         // This is just copied behaviour from above if-statement to 
         // not change anything else in the old spaghetti code.
         WARN("Server communication timeout. ID = %u",timerID);
         if(m_fakeConnectTimer != 0){
            WARN("removing Fake connection timer %u", timerID);
            m_queue->removeTimer(m_fakeConnectTimer);
            m_fakeConnectTimer = 0;
         }
         updateState(timeout);
         return;
      } else if(timerID == m_fakeConnectTimer){
         WARN("Fake connection timeout. ID = %u", timerID);
         m_ser_provider->connectionCtrl(DISCONNECT, "FAKETIMEOUT");
         m_queue->removeTimer(m_fakeConnectTimer);
         DBG("Fake connect timer set to %x", MAX_UINT16);
         m_fakeConnectTimer = MAX_UINT16;
      } else if ( m_tracking && m_tracking->isdecodedTimerOk( timerID ) ) {
         m_tracking->decodedExpiredTimer( timerID );
      } else {
         WARN("Unknown timer %u timed out!", timerID);
      }
      Module::decodedExpiredTimer(timerID);
   }

   bool NavServerCom::reallyTriggerTimout(uint16 timerID)
   {
      if (timerID == m_timeoutTimerID) {
         uint32 now = isab::TimeUtils::millis();
         uint32 timeoutDiff = now - m_timeoutTimerTimestamp;
         if (timeoutDiff < (m_timeoutTime/10)){
            // The timeoutTimer was defined less then a tenth of its time 
            // ago, so it can not be the correct expired timer, but the cause 
            // of decodedExpiredTimer is a race between the timer thread
            // and the message queue.
            // So we return false and thus not handle the timeout.
            return false;
         }
      }
      return true;
   }

   void NavServerCom::setTimeoutTimer(uint32 timeout)
   {
      // Remember when we set the timer.
      m_timeoutTimerTimestamp = isab::TimeUtils::millis();
      // Set or define the timer.
      if (m_timeoutTimerID == 0) {
         m_timeoutTimerID = m_queue->defineTimer(timeout);
      } else {
         m_queue->setTimer(m_timeoutTimerID, timeout);
      }
   }

#define eventName(a)                                                        \
((a == invalidEvent) ? "invalidEvent" :                                     \
 ((a == allParamsRead) ? "allParamsRead" :                                  \
  ((a == receivedRequest) ? "receivedRequest" :                             \
   ((a == connectionEstablished) ? "connectionEstablished" :                \
    ((a == parameterReply) ? "parameterReply" :                             \
     ((a == sentRequest) ? "sentRequest" :                                  \
      ((a == receivedServerReply) ? "receivedServerReply" :                 \
       ((a == receivedServerError) ? "receivedServerError" :                \
        ((a == pollReply) ? "pollReply" :                                   \
         ((a == finalPollReply) ? "finalPollReply" :                        \
          ((a == disconnected) ? "disconnected" :                           \
           ((a == parameterChange) ? "parameterChange" :                    \
            ((a == timeout) ? "timeout" :                                   \
             ((a == redirect) ? "redirect" :                                \
             ((a == requestDisconnect) ? "requestDisconnect" :              \
              ((a == unauthorized) ? "unauthorized" :                       \
               ((a == shutdown) ? "shutdown" : "undefined" )))))))))))))))))



#define stateName(a)                                                \
   ((a == noSession) ? "noSession" :                                \
    ((a == waitingForConnect) ? "waitingForConnect" :               \
     ((a == sendingParams) ? "sendingParams" :                      \
      ((a == sendingRequests) ? "sendingRequests" :                 \
       ((a == serverPoll) ? "serverPoll" :                          \
         ((a == waitingForDisconnect) ? "waitingForDisconnect" :    \
          ((a == interrupted) ? "interrupted" :                     \
           ((a == timedOut) ? "timedOut" :                          \
            ((a == noParams) ? "noParams" :                         \
              "undefined")))))))))

   void NavServerCom::updateState(enum Event event)
   {
      enum SessionStates lastState = m_sessionState;
      DBG("updateState event: %s (%d)", eventName(event), int(event));
      if(m_shutdownPrepared){
         DBG("Shutdown has been prepared - at least! Abort ASAP!");
         return;
      }
      if(m_sessionState == serverPoll && 
         (event == parameterChange || event == receivedRequest)){
         ///Avoid these events like the bubonic plague. If we reach
         ///stateChangeAction while a poll is in progress the infamous
         ///double poll problem rears its ugly head!
         DBG("updateState state is: serverPoll");
         DBG("return here, no stateChangeAction.");
         return;
      };
      DBG("updateState state was: %s", stateName(m_sessionState));
      switch(event){
      case shutdown:
         if(m_sessionState != noSession && m_sessionState != noParams){
            m_sessionState = waitingForDisconnect;
         }
         break;
      case parameterChange:
         if(m_sessionState == noParams && allNecessaryParamsSet()){
            m_sessionState = noSession;
         }
         if(m_requestList.outstanding() == 0){
            return; //Dont even go to stateAction, may cause double poll
         }
         //fallthough
      case receivedRequest:
         if(m_sessionState == noSession && allNecessaryParamsSet()){
            m_sessionState = waitingForConnect;
            reportProgress(connecting, 
                           MessageType((*(m_requestList.begin())).request));
         } else if ( m_timeoutTimerID != 0 ) {
            // Was idle report Progress
            reportProgress(connected, 
                           MessageType((*(m_requestList.begin())).request));
         }
         break;
      case connectionEstablished:
         if(m_sessionState == waitingForConnect ||
            m_sessionState == noSession && m_requestList.outstanding() > 0){
            m_sessionState = sendingParams;
            reportProgress(connected, 
                           MessageType((*(m_requestList.begin())).request));
         }
         break;
      case parameterReply:
         if((m_sessionState == sendingParams) || 
            (m_sessionState == sendingRequests)){
            if((m_pendingList.outstanding() > 0) || 
               (m_requestList.outstanding() > 0)){
               m_sessionState = sendingRequests;
            } else {
//               m_sessionState = serverPoll;
               /// No need to send poll all the time with 0x0a
               m_sessionState = sendingRequests;
               // Set timeout to good value
               setTimeoutTimer(m_timeoutTime);
               reportProgress( done ); // Done for now
               DBG( "Defining timer %u timeout %"PRIu32, m_timeoutTimerID,
                    m_timeoutTime );
            }
         }
         break;
      case redirect:
         m_requestList.concatenate(m_pendingList, true);
         lastState = sendingParams;
         m_sessionState = waitingForDisconnect;
         break;
      case requestDisconnect:
         // Requested to disconnect. Can typically happen if getting "Connection: Close"
         // and we want to ensure that we do get disconnected.
         m_requestList.concatenate(m_pendingList);
         m_sessionState = waitingForDisconnect;
         break;
      case unauthorized:
         if(m_sessionState != noSession){
            m_sessionState = waitingForDisconnect;
            reportProgress(unknownError);
         }
         break;
      case receivedServerReply:
         if(m_sessionState == sendingRequests && 
            m_requestList.outstanding() == 0 && 
            m_pendingList.outstanding() == 0){
//            m_sessionState = serverPoll;
            /// No need to send poll all the time with 0x0a
            m_sessionState = sendingRequests;
            // Set timeout to good value
            setTimeoutTimer(m_timeoutTime);
            reportProgress( done ); // Done for now
            DBG( "Defining timer %u timeout %"PRIu32, m_timeoutTimerID,
                 m_timeoutTime );
//No, params get sent again            m_sessionState = waitingForDisconnect;
//            reportProgress( done );
         }
         break;
      case receivedServerError:
         if(m_sessionState == sendingRequests || m_sessionState == serverPoll){
            m_sessionState = waitingForDisconnect;
            reportProgress(unknownError);
         }
         break;
      case pollReply:
         if(m_sessionState == serverPoll){
            m_sessionState = serverPoll;
         }
         // Set timeout to good value
         setTimeoutTimer(m_timeoutTime);
         reportProgress( done ); // XXX:?here?
         DBG( "Defining timer %u timeout %"PRIu32, m_timeoutTimerID,
              m_timeoutTime );
         break;
      case finalPollReply:
         if(m_sessionState == serverPoll){
            m_sessionState = waitingForDisconnect;
            reportProgress(done);
         }
         break;
      case timeout:
         DBG("updateState -- timeout");
         if(m_sessionState != noParams && m_sessionState != noSession &&
            m_sessionState != timedOut){
            DBG("updateState -- changeing state to timedOut");
            m_sessionState = timedOut;
            reportProgress(connectionTimedOut);
            //XXX if(m_waitingForConnect){ whatever?
         } else if(m_connectState != CONNECTED &&
                   m_connectState != CONNECTING){
            DBG("updateState -- Changeing state to noSession");
            m_sessionState = noSession;
            DBG("updateState -- Removing timer %u.", m_timeoutTimerID);
            m_queue->removeTimer(m_timeoutTimerID);
            m_timeoutTimerID = 0;
            if(m_fakeConnectTimer != MAX_UINT16){
               DBG("updateState -- Removing fakeconnection timer %u.", 
                   m_fakeConnectTimer);
               m_queue->removeTimer(m_fakeConnectTimer);
               m_fakeConnectTimer = 0;
            }
         } else if(m_sessionState == timedOut){
            DBG("updateState -- timedout -> waitingfordisconnect");
            m_sessionState = waitingForDisconnect;
            DBG("updateState -- Removing timer %u.", m_timeoutTimerID);
            m_queue->removeTimer(m_timeoutTimerID);
            m_timeoutTimerID = 0;
            if(m_fakeConnectTimer != MAX_UINT16){
               DBG("updateState -- Removing fakeconnection timer %u.", 
                   m_fakeConnectTimer);
               m_queue->removeTimer(m_fakeConnectTimer);
               m_fakeConnectTimer = 0;
            }
         } else {
            DBG("updateState -- No state change");
         }
         break;
      case disconnected:
         //This is where things get complicated. If a session is
         //actually in progress when the connection is broken we
         //need to initiate a recovery. This is acheived by entering
         //the 'interrupted' state. Some states can ignore the
         //disconnection, and some can recover on their own.
         if(m_sessionState == waitingForDisconnect || 
            m_sessionState == timedOut || 
            m_sessionState == noSession  || 
            (m_sessionState == sendingParams && 
             m_pendingList.outstanding() == 0) && m_paramSyncs.size() == 0){
            DBG("m_sessionState == %s, m_requestList.outstanding() == %u, m_pendingList.outstanding() == %u", stateName(m_sessionState), m_requestList.outstanding(), m_pendingList.outstanding());
            m_sessionState = noSession;
            reportProgress(done);
         } else if(m_sessionState == noParams){
            DBG("m_sessionState == noParams");
            m_sessionState = noParams;
         } else {
            DBG("m_sessionState == %s", stateName(m_sessionState));
            m_sessionState = interrupted;
         }
         DBG("Nbr of paramsyncs = %u", m_paramSyncs.size());
         break;
      default:
         ERR("updateState: no such event: %u", event);
      };
      DBG("updateState new state: %s", stateName(m_sessionState));
      stateChangeAction(lastState);
   }

   uint32 NavServerCom::calcDataToSend()
   {
      uint32 sum = 0;
      RequestIterator p;
      //DBG("calcDataToSend");
      for(p = m_pendingList.begin(); p != m_pendingList.end(); ++p){
         if ( (*p).data ) {
            sum += static_cast<ServerRequestData*>((*p).data)->getLengthToSend();
         }
         sum += NAV_ROUTE_REPLY_HEADER_SIZE;
      }
      //DBG("data sent (in pending list): %u", sum);
      for(p = m_requestList.begin(); p != m_requestList.end(); ++p){
         if((*p).data){
            sum += 
               static_cast<ServerRequestData*>((*p).data)->getLengthToSend();
            sum += NAV_ROUTE_REPLY_HEADER_SIZE;
         }
      }
      //DBG("sent and to be sent: %u", sum);
      if(!m_paramBlock){
         ERR("No ParameterBlock, where did it go?! Allocating one now");
         m_paramBlock = new ParameterBlock(); //this should never ever happen
      }
      sum += m_paramBlock->getLength() + NAV_ROUTE_REPLY_HEADER_SIZE;
      //DBG("sen + to be sent + param: %u", sum);
      return sum;
   }

   bool NavServerCom::checkAndSetupPendingRequestsToResend() 
   {
      if ( m_pendingList.outstanding() > 0 || 
            m_requestList.outstanding() > 0 ||
            m_paramSyncs.size() > 0 )
      {
         DBG( "Close was expected reconnecting" );

         DBG("m_sessionState == %s, m_requestList.outstanding() == %u, m_pendingList.outstanding() == %u", stateName(m_sessionState), m_requestList.outstanding(), m_pendingList.outstanding());

         // Hmm, send again
         if ( m_pendingList.outstanding() > 0 ) {
            m_requestList.concatenate( m_pendingList );
         }
         if ( m_paramSyncs.size() > 0 ) {
            DBG("Reinsert paramsync request");
            RequestListItem* item = m_requestList.allocate();
            if ( item ){
               item->reqID = m_reqID++;
               DBG("checkAndSetupPendingRequestsToResend: m_reqID=0x%"PRIx8, item->reqID);
               item->request = NAV_SERVER_PARAMETER_REQ;
               DBG("checkAndSetupPendingRequestsToResend: requestType=0x%"PRIx16, item->request);
               item->errorType = NAV_STATUS_OK;
               item->status = 0;
               item->src = m_paramSyncs[0];
               item->data = NULL;
               m_requestList.enqueue(item);
            }
         }
         return true;
      } // End if have requests left to send
      return false;
   }   
   
   void NavServerCom::stateChangeAction(enum SessionStates fromState)
   {
      if(m_sessionState == fromState && m_sessionState == waitingForConnect) {
         // Not really a state change.
         // We can get connect requests from both Nop and paramchange but we do not allow multiple connects so we filter it out here.
         return;
      }

      switch(m_sessionState){
      case noParams:
         break; // do nothing
      case noSession:
         DBG("Removing timer %u", m_timeoutTimerID);
         m_queue->removeTimer(m_timeoutTimerID);
         m_timeoutTimerID = 0;
         DBG("Removing fakeconnect timer %u", m_fakeConnectTimer);
         m_queue->removeTimer(m_fakeConnectTimer);
         m_fakeConnectTimer = 0;
         if(!allNecessaryParamsSet()){
            break;
         }
         // fallthrough
      case waitingForConnect:
         if(m_requestList.outstanding() > 0){
            DBG("%d outstanding requests", m_requestList.outstanding());
            if(unsigned(m_serverIdx) >= m_servers->size()){
               m_serverIdx = findFirstServer(*m_servers, m_seed);
               if(unsigned(m_serverIdx) >= m_servers->size()){
                  setReachable(*m_servers);
                  m_serverIdx = findFirstServer(*m_servers, m_seed);
               }
            } 
            //             if(unsigned(m_serverIdx) < m_servers->size()){
            DBG("NbrOfServers: %u, Idx: %d", m_servers->size(),m_serverIdx);
            const char* server =(*m_servers)[m_serverIdx]->getHostAndPort();
            // Proxy? 
            if ( m_proxyServerAndPort != NULL ) {
               m_parser->setProxy( server );
               server = m_proxyServerAndPort;
            }
            //for the HTTP host header.
            m_parser->setHost(server);
            m_parser->setUrl((*m_servers)[m_serverIdx]->getUrl());
            DBG("Using server '%s'", server);
            m_ser_provider->connectionCtrl(CONNECT, server);

            setTimeoutTimer(m_timeoutTime);
            DBG("Defining timer %u timeout %"PRIu32, m_timeoutTimerID,
                m_timeoutTime);
         }
         // reset poll sequence number.
         m_pollOffset = 0;
         break;
      case sendingParams: {
         if(m_requestList.outstanding() == 0){
            // This is commented out since it causes a DISCONNECT when
            // sending tunnel packets before parameters have been sent.
            
            // // the request that triggered us to connect has been canceled.
            // // Don't cache dialup connection
            // m_ser_provider->connectionCtrl(DISCONNECT, "0");
            // DBG("Removing timer %u", m_timeoutTimerID);
            // m_queue->removeTimer(m_timeoutTimerID);
            // m_timeoutTimerID = 0;
            break;
         }
         DBG("Setting timer %u timeout %"PRIu32,
             m_timeoutTimerID, m_timeoutTime);
         setTimeoutTimer(m_timeoutTime);
         reportProgress(sendingData, NAV_SERVER_PARAMETER_REQ, 
                        0, calcDataToSend());
         DBG("Sending paramblock");
         // Set if secondary server
         bool secondaryServer = false;
         if ( (*m_servers)[m_serverIdx]->group() >= 3 ) {
            // Perhaps a more general way than checking against 3
            secondaryServer = true;
         }
         bool sendUnaccompanied = false;
         if ( m_requestList.outstanding() > 0 &&
              m_requestList.begin()->request == NAV_SERVER_UPGRADE_REQ )
         {
            // Send nothing but upgrade, after upgrade server info is sent
            sendUnaccompanied = true;
         }
         if ( m_requestList.outstanding() > 0 && 
              m_requestList.begin()->request == NAV_REQUEST &&
              m_requestList.begin()->data != NULL )
         {
            ServerRequestData* r = static_cast<ServerRequestData*>( 
               m_requestList.begin()->data );
            if ( NavReplyPacket::getRequestType( 
                    r->getBuffer()->accessRawData( 0 ),
                    r->getBuffer()->getLength() ) == 
                 navRequestType::NAV_TUNNEL_DATA_REQ )
            {
               // Send nothing but tunnel. After tunnel something like upgrade
               // might come.
               sendUnaccompanied = true;  
            }
         }
         // We might send parameters again if PARAMETER_REQ in requestList
         uint8 reqID = m_reqID++;
         if ( m_requestList.begin() != m_requestList.end() &&
              !sendUnaccompanied && m_paramSyncs.size() != 0 &&
              m_requestList.begin()->request == NAV_SERVER_PARAMETER_REQ )
         {
            RequestListItem* req = m_requestList.dequeueFirst();
            reqID = uint8(req->reqID);
            // m_paramSyncs will make a reply
            m_requestList.deallocate( req );
         }
         
         m_sentData = m_parser->sendParameters( reqID, m_paramBlock,
                                                m_paramSyncs.size() != 0,
                                                secondaryServer,
                                                sendUnaccompanied );
         // DBGDUMP("parameterblock sent:", m_paramBlock->accessRawData(0), 
         //         m_paramBlock->getLength());
         reportProgress(sendingData, NAV_SERVER_PARAMETER_REQ, 
                        m_sentData, calcDataToSend());
      }
         //fallthrough
      case sendingRequests:
         while ( m_requestList.outstanding() > 0 && 
                 uint32(m_pendingList.outstanding()) < m_maxnbroutstanding ) 
         {
            DBG("stateChangeAction: m_sessionState = sendingRequests");
            m_sessionState = sendingRequests;
            RequestListItem* req = m_requestList.dequeueFirst();

            if(req->nrpData != NULL){
               // Handle special case for poll requests.
               DBG("nrpData Packet sent with request id 0x%"PRIx8, req->reqID);
               NavRequestPacket* data;
               data = static_cast<NrpDataHolder*>(req->nrpData)->getNrpData();
               // req->reqID is assigned from m_reqID which is an uint8.
               m_sentData += m_parser->sendPacket(data);
               setTimeoutTimer(m_timeoutTime);
               DBG( "Defining timer %u timeout %"PRIu32, m_timeoutTimerID,
                    m_timeoutTime );
               m_pendingList.enqueue(req);
               reportProgress(sendingData, MessageType(req->request),
                              m_sentData, calcDataToSend());
            } else if(req->data != NULL){
               DBG("refreshData: lat %"PRIx32", lon %"PRIx32", head %d",
                   m_lastLat, m_lastLon, m_lastHeading);
               DBG("Content Packet sent with request id 0x%"PRIx8, req->reqID);
               static_cast<ServerRequestData*>(req->data)->
                  refreshData(m_lastLat, m_lastLon, m_lastHeading);
               Buffer* data;
               data = static_cast<ServerRequestData*>(req->data)->getBuffer();
               // req->reqID is assigned from m_reqID which is an uint8.
               m_sentData += m_parser->sendContents(uint8(req->reqID), 
                                                    req->request, data);
               setTimeoutTimer(m_timeoutTime);
               DBG( "Defining timer %u timeout %"PRIu32, m_timeoutTimerID,
                    m_timeoutTime );
               m_pendingList.enqueue(req);
               reportProgress(sendingData, MessageType(req->request),
                              m_sentData, calcDataToSend());
               if(req->request == NAV_SERVER_ROUTE_REQ || 
                  req->request == NAV_SERVER_SEARCH_REQ ||
                  req->request == NAV_SERVER_BINARY_UPLOAD_REQ){
                  //this is from disconnection timeout strategies.
                  m_lastRequest = MessageType(req->request);
               }
            } else if ( req->request == NAV_SERVER_PARAMETER_REQ ) {
               // Param sync (When connected) has empty data
               // Set if secondary server
               bool secondaryServer = false;
               DBG("Param Packet sent with request id 0x%"PRIx8, req->reqID);
               if ( (*m_servers)[m_serverIdx]->group() >= 3 ) {
                  // Perhaps a more general way than checking against 3
                  secondaryServer = true;
               }
               m_sentData += m_parser->sendParameters( 
                  uint8(req->reqID), m_paramBlock,
                  true, secondaryServer, false );
               m_pendingList.enqueue( req );
               setTimeoutTimer(m_timeoutTime);
               DBG( "Defining timer %u timeout %"PRIu32, m_timeoutTimerID,
                    m_timeoutTime );
               reportProgress( sendingData, MessageType(req->request),
                               m_sentData, calcDataToSend() );

            } else {
               m_requestList.deallocate(req);
               DBG("Empty request, ignoring");
            }
         }         
         break;
      case serverPoll:
         m_parser->pollServer(m_reqID++, m_pollOffset);
         break;
      case waitingForDisconnect:
         {
            m_queue->removeTimer(m_timeoutTimerID);
            m_timeoutTimerID = 0;
            if(m_fakeConnectTimer != MAX_UINT16){
               m_queue->removeTimer(m_fakeConnectTimer);
               m_fakeConnectTimer = 0;
            }
            // This part finds the timeout to send to the lower module.
            char timeout[16] = "0";
            if(m_shutdownPrepared){
               DBG("awaiting shutdown, remove all requests.");
               DBG("%d outstanding requests", m_requestList.outstanding());
               //disconnect since we are doing shutdown preparations.
               m_requestList.concatenate(m_pendingList);
               RequestListItem* req = NULL;
               while(NULL != (req = m_requestList.dequeueFirst())){
                  //sendError(NSC_CANCELED_BY_SHUTDOWN, req->src);
                  delete req->data;
                  req->data = NULL;
                  delete req->nrpData;
                  req->nrpData = NULL;
                  m_requestList.deallocate(req);
               }
               DBG("%d outstanding requests", m_requestList.outstanding());
            } else if(fromState == sendingParams){
               //we come here if we got a redirect.
               unsigned redirectTO = 30000U;
               if(unsigned(m_serverIdx) >= m_servers->size()){
                  setReachable(*m_servers);
                  m_serverIdx = findFirstServer(*m_servers, m_seed);
               }
               snprintf(timeout, 16, "%u", redirectTO);
            } else {
               int shift = 0;
               switch(m_lastRequest){
               case NAV_SERVER_ROUTE_REQ:  shift = routeShift;  break;
               case NAV_SERVER_SEARCH_REQ: shift = searchShift; break;
               case NAV_SERVER_BINARY_UPLOAD_REQ: 
                  shift = synchronizeShift; break;
               default: 
                  shift = 32; break;  // anything shifted that far will be 0
               }
               uint32 strategy = 0x03 & (m_disconnectStrategy >> shift);
               switch(strategy){
               case shortTimeout:
                  snprintf(timeout, 16, "%u", m_shortConnectionTimeout);
                  break;
               case longTimeout:
                  snprintf(timeout, 16, "%u", m_longConnectionTimeout);
                  break;
               case disconnectImmediately:
               default:
                  break;
               }
            }
            DBG("DISCONNECT - %s", timeout);
            m_ser_provider->connectionCtrl(DISCONNECT, timeout);
            break;
         }
      case interrupted:
         {
            ERR("NavServerCom session interrupted because %u!", 
                m_disconnectReason);
            // set timer for new connect, just in case.
            setTimeoutTimer(m_timeoutTime);
            DBG( "Defining timer %u timeout %"PRIu32, m_timeoutTimerID,
                 m_timeoutTime );
            m_sessionState = noSession;
            bool closeExpected = m_parser->getCloseExpected();
            m_parser->flush();
            enum ErrorNbr error = NO_ERRORS;
            switch(m_disconnectReason){
            case UNSPECIFIED:
               ERR("UNSPECIFIED disconnect error");//XXX
               //fallthrough
            case BY_PEER:
               error = NSC_SERVER_CONNECTION_BROKEN;
               ERR("NSC_SERVER_CONNECTION_BROKEN");
               if ( closeExpected ) {
                  if (checkAndSetupPendingRequestsToResend()) {
                     goto connect_server;
                  }
               } // End if close was expected
               break;
            case LOOKUP_FAILED:
               error = NSC_SERVER_NOT_FOUND;
               ERR("NSC_SERVER_NOT_FOUND");
               goto set_server_unreachable;
            case TRANSPORT_FAILED:
               error = NSC_TRANSPORT_FAILED;
               ERR("NSC_TRANSPORT_FAILED");
               break;
            case CHANNEL_FAILED:
               error = NSC_SERVER_UNREACHABLE;
               ERR("NSC_SERVER_UNREACHABLE");
               goto set_server_unreachable;
            case NO_RESPONSE:
               error = NSC_SERVER_NOT_RESPONDING;
               ERR("NSC_SERVER_NOT_RESPONDING");
               if(m_pendingList.outstanding() > 0){
                  m_requestList.concatenate(m_pendingList);
               }
               if(m_paramSyncs.size() > 0){
                  DBG("insert paramsync request");
                  RequestListItem* item = m_requestList.allocate();
                  if(item){
                     item->reqID = m_reqID++;
                     item->request = NAV_SERVER_PARAMETER_REQ;
                     item->errorType = NAV_STATUS_OK;
                     item->status = 0;
                     item->src = m_paramSyncs[0];
                     item->data = NULL;
                     m_requestList.enqueue(item);
                  }
               }  
               goto set_server_unreachable;
            case INTERNAL_ERROR:
               error = NSC_TCP_INTERNAL_ERROR;
               ERR("NSC_TCP_INTERNAL_ERROR");
               goto set_server_unreachable;
            case REQUESTED:
               //XXXXXXX send real error here XXXXXX
               WARN("REQUESTED disconnect - shutdown is %sin progress.",
                       m_shutdownPrepared ? "" : "NOT ");
               if (m_fakeConnectTimer == MAX_UINT16) {
                  /* Fake connect timer has triggered. */
                  /* Mark server as unreachable. */
                  error = NSC_FAKE_CONNECT_TIMEOUT;
                  goto set_server_unreachable;
               } else if (m_shutdownPrepared) {
                  error = NSC_CANCELED_BY_SHUTDOWN;
               } else {
                  /* Unknown error, will send "NSC_TCP_INTERNAL_ERROR2" to */
                  /* requesting module. */
                  error = NSC_TCP_INTERNAL_ERROR2;
               }
               break;
            set_server_unreachable:
               DBG("Setting server %s unreachable.", 
                   (*m_servers)[m_serverIdx]->getHostAndPort());
               (*m_servers)[m_serverIdx]->setUnreachable();
               m_serverIdx = nextServer(m_seed, *m_servers, m_serverIdx);
            connect_server:
               if(m_serverIdx >= int32(m_servers->size())){
                  m_serverIdx = findFirstServer(*m_servers, m_seed);
                  if(m_serverIdx >= int32(m_servers->size())){
                     setReachable(*m_servers);
                     DBG("All servers are reachable again.");
                     m_serverIdx = findFirstServer(*m_servers, m_seed);
                  }
               } else {
                  const char* s = (*m_servers)[m_serverIdx]->getHostAndPort();
                  DBG("More servers are available. Using %s", s);
                  m_sessionState = waitingForConnect;
                  // Proxy?
                  if ( m_proxyServerAndPort != NULL ) {
                     m_parser->setProxy( s );
                     m_parser->setUrl((*m_servers)[m_serverIdx]->getUrl());
                     s = m_proxyServerAndPort;
                  }
                  DBG( "Server unreachable setting new server '%s'", s );
                  m_ser_provider->connectionCtrl(CONNECT, s);
               }
               if(m_serverIdx > -1 && 
                  unsigned(m_serverIdx) < m_servers->size()){
                  DBG("New server: %s",
                      (*m_servers)[m_serverIdx]->getHostAndPort());
               } else {
                  reportProgress(disconnectionError);
                  reportProgress(done);
                  WARN("No more servers!");
                  decodedParamNoValue( uint32(m_serverListParam), 0, 0 );
               }
               break;
            case METHOD_ERROR:
               error = NSC_SERVER_UNREACHABLE;
               ERR("%s (%d) causes a METHOD_ERROR. Removing permanently.", 
                   (*m_servers)[m_serverIdx]->getHostAndPort(), m_serverIdx);
               removeServer(*m_servers, m_serverIdx);
               if(!m_servers->empty()){
                  m_serverIdx = findFirstServer(*m_servers, m_seed);
                  if(unsigned(m_serverIdx) >= m_servers->size()){
                     setReachable(*m_servers);
                     m_serverIdx = findFirstServer(*m_servers, m_seed);
                  }
               } else {
                  //simplest way to set the default servers.
                  decodedParamNoValue( uint32(m_serverListParam), 0, 0 );
                  m_serverIdx = -1;
               }
               break;
            }
            DBG("%d requests, %d pending", m_requestList.outstanding(), 
                m_pendingList.outstanding());
            if(m_sessionState != waitingForConnect){
               //remove timer.
               DBG("Removing timeouttimer %u and fakeconnecttimer %u",
                   m_timeoutTimerID, m_fakeConnectTimer);
               m_queue->removeTimer(m_timeoutTimerID);
               m_timeoutTimerID = 0;
               m_queue->removeTimer(m_fakeConnectTimer);
               m_fakeConnectTimer = 0;
               //move pending requests into the waiting list.
               m_requestList.concatenate(m_pendingList);
               RequestListItem* req = NULL;
               while(NULL != (req = m_requestList.dequeueFirst())){
                  sendError(error, req->src);
                  ERR("sendError(0x%x, 0x%08"PRIx32")", error, req->src);
                  delete req->data;
                  req->data = NULL;
                  delete req->nrpData;
                  req->nrpData = NULL;
                  m_requestList.deallocate(req);
               }
               DBG("Switching from state interrupted to state noSession.");
               m_sessionState = noSession;
               reportProgress(done);
            }
         }
         m_disconnectReason = UNSPECIFIED;
         break;
      case timedOut:
         if ( m_pendingList.outstanding() == 0 && 
              m_requestList.outstanding() == 0 )
         {
            // Idle timeout close connection before server does
            if ( m_connectState == CONNECTED ||
                 m_connectState == CONNECTING )
            {
               // Cache dialup connection for 1 minute by default.
               DBG( "Idle connection TIMEOUT, disconnecting." );
               m_ser_provider->connectionCtrl( DISCONNECT, "60000" );
               DBG("TIMEOUT -- sending progress report");
               reportProgress(connectionTimedOut);
            }
         } else {
            ERR("NavServerCom session timed out!");
            DBG("Setting timer %u timeout %u", m_miscTimerID, 100);
            if (m_miscTimerID == 0) {
               m_miscTimerID = m_queue->defineTimer(100);
            } else {
               m_queue->setTimer(m_miscTimerID, 100);
            }
            m_requestList.insertList(m_pendingList);
            if(m_connectState == CONNECTED || m_connectState == CONNECTING){
               DBG("TIMEOUT, sending disconnect, caching dialup conn. 1 min.");
               // Cache dialup connection for 1 minute by default.
               m_ser_provider->connectionCtrl(DISCONNECT, "60000");
            }
            // this should only be done once
            DBG("TIMEOUT -- sending progress report");
            reportProgress(connectionTimedOut);
            reportProgress(done);
            { // scope for req
               RequestListItem *req = NULL;
               while(NULL != (req = m_requestList.dequeueFirst())){
                  ERR("sendError(NSC_SERVER_COMM_TIMEOUT, %#08"PRIx32")", 
                      req->src);
                  const enum ErrorNbr errors[] = { 
                     NSC_SERVER_COMM_TIMEOUT_CONNECTING,
                     NSC_SERVER_COMM_TIMEOUT_CONNECTED,
                     NSC_SERVER_COMM_TIMEOUT_DISCONNECTING,
                     NSC_SERVER_COMM_TIMEOUT_CLEAR,
                     NSC_SERVER_COMM_TIMEOUT_WAITING_FOR_USER,
                  };
                  sendError(errors[m_connectState], req->src);
                  delete req->data;
                  req->data = NULL;
                  delete req->nrpData;
                  req->nrpData = NULL;
                  m_requestList.deallocate(req);
               }
            }
         }
         break;
      default:
         // assume the worst XXXX report error?
         DBG("Removing timer %u", m_timeoutTimerID);
         m_queue->removeTimer(m_timeoutTimerID);
         m_timeoutTimerID =0;
         DBG("Removing fakeconnection timer %u", m_fakeConnectTimer);
         m_queue->removeTimer(m_fakeConnectTimer);
         m_fakeConnectTimer =0;
         ERR("stateChangeAction: No such state: %u", m_sessionState);
         reportProgress(unknownError);
         reportProgress(done);
         //Cache dialup connection for 1 minute.
         m_ser_provider->connectionCtrl(DISCONNECT, "60000");
         m_sessionState = interrupted;
      }
      DBG("End of stateChangeAction");
   }

   void NavServerCom::decodedSetServerParams(const uint8* data, size_t length, 
                                             uint32 /*src*/)
   {
      DBGDUMP("setServerParams -- IGNORING", data, length);
      //XXX this must be fixed if we want to use DisplaySerial.
   }
   
   enum NavServerCom::Event NavServerCom::decodeParameterReply(
      Buffer& buffer, uint8 status, ErrorObj& err, uint32 extendedError )
   {
      enum Event retval = parameterReply;
      enum ErrorNbr error = NSC_AUTHORIZATION_FAILED; //just in case ...
      DBGDUMP("decodeParameterReply", buffer.accessRawData(),
              buffer.remaining());
      if ( status & REQUEST_SPECIFIC ) {
         DBG("Request specific status: %#x", status);
         switch(ParamReplyStatus(status)){
         case PARAM_REPLY_WF_TYPE_TOO_HIGH:
            ERR("PARAM_REPLY_WF_TYPE_TOO_HIGH");
            break;
         case PARAM_REPLY_UPDATE_NEEDED:
            DBG("Program update needed.");
            err.setErr( NSC_SERVER_NEW_VERSION );
            sendError( err );
            error = NSC_SERVER_NEW_VERSION;
         case PARAM_REPLY_EXPIRED_USER:
            if(error == NSC_AUTHORIZATION_FAILED){
               error = NSC_EXPIRED_USER;
               ERR("User account has expired. Aborting with error message.");
            }
         case PARAM_REPLY_UNAUTHORIZED_USER:
            {//if protects from fallthrough
               if(status == PARAM_REPLY_UNAUTHORIZED_USER){ 
                  ERR("Unauthorized user. Aborting with error message.");
               }
               retval = unauthorized;
               m_pendingList.concatenate(m_requestList);
               RequestListItem* req = NULL;
               err.setErr( error );
               while(NULL != (req = m_pendingList.dequeueFirst())){
                  sendError( err, req->src );
                  delete req->data;
                  req->data = NULL;
                  delete req->nrpData;
                  req->nrpData = NULL;
                  m_requestList.deallocate(req);
               }
            }
            break;
         case PARAM_REPLY_REDIRECT:
            DBG("Redirecting to alternate server.");
            retval = redirect;
            m_serverIdx = nextServer(m_seed, *m_servers, m_serverIdx);
            break;
         case PARAM_REPLY_UNAUTH_OTHER_HAS_LICENSE :
            DBG("PARAM_REPLY_UNAUTH_OTHER_HAS_LICENSE");
            error = NSC_UNAUTH_OTHER_HAS_LICENSE;
            break;
         case CHANGED_LICENSE_REPLY_OLD_LICENSE_NOT_IN_ACCOUNT :
            DBG("CHANGED_LICENSE_REPLY_OLD_LICENSE_NOT_IN_ACCOUNT");
            error = NSC_OLD_LICENSE_NOT_IN_ACCOUNT;
            break;
         case CHANGED_LICENSE_REPLY_MANY_USERS_WITH_OLD_LICENSE :
            DBG("CHANGED_LICENSE_REPLY_MANY_USERS_WITH_OLD_LICENSE");
            error = NSC_OLD_LICENSE_IN_MANY_ACCOUNTS;
            break;
         case CHANGED_LICENSE_REPLY_MANY_USERS_WITH_NEW_LICENSE :
            DBG("CHANGED_LICENSE_REPLY_MANY_USERS_WITH_NEW_LICENSE");
            error = NSC_NEW_LICENSE_IN_MANY_ACCOUNTS;
            break;
         case CHANGED_LICENSE_REPLY_OLD_LICENSE_IN_OTHER_ACCOUNT :
            DBG("CHANGED_LICENSE_REPLY_OLD_LICENSE_IN_OTHER_ACCOUNT");
            error = NSC_OLD_LICENSE_IN_OTHER_ACCOUNT;
            break;
         case UPGRADE_REPLY_MUST_CHOOSE_REGION :
            DBG("UPGRADE_REPLY_MUST_CHOOSE_REGION");
            error = NSC_UPGRADE_MUST_CHOOSE_REGION;
            break;
         }
      } else if(status != NAV_STATUS_OK){
         DBG("Common status: %#x", status);
         switch(ReplyStatus(status)){
         case NAV_STATUS_NOT_OK: ERR("NAV_STATUS_NOT_OK");
         case NAV_STATUS_PARAM_REQ_NOT_FIRST: 
            ERR("NAV_STATUS_PARAM_REQ_NOT_FIRST");
            error = NSC_SERVER_NOT_OK; 
            break;
         case NAV_STATUS_REQUEST_TIMEOUT: 
            ERR("NAV_STATUS_REQUEST_TIMEOUT");
            error = NSC_SERVER_REQUEST_TIMEOUT;
            break;
         case NAV_STATUS_OUTSIDE_MAP: ERR("NAV_STATUS_OUTSIDE_MAP");
            error = NSC_SERVER_OUTSIDE_MAP;
            break;
         case NAV_STATUS_PROTOVER_NOT_SUPPORTED: 
            ERR("NAV_STATUS_PROTOVER_NOT_SUPPORTED");
            error = NSC_SERVER_PROTOCOL_ERROR;
            break;
         case NAV_STATUS_OUTSIDE_ALLOWED_AREA: 
            ERR("NAV_STATUS_OUTSIDE_ALLOWED_AREA");
            error = NSC_SERVER_UNAUTHORIZED_MAP;
            break;
         case NAV_STATUS_NO_TRANSACTIONS_LEFT: 
            ERR("NAV_STATUS_NO_TRANSACTIONS_LEFT");
            error = NSC_NO_TRANSACTIONS;
            break;
         case NAV_STATUS_UNAUTH_OTHER_HAS_LICENSE: 
            ERR("NAV_STATUS_UNAUTH_OTHER_HAS_LICENSE");
            error = NSC_UNAUTH_OTHER_HAS_LICENSE;
            break;
         case NAV_STATUS_EXTENDED_ERROR :
            ERR("NAV_STATUS_EXTENDED_ERROR");
            error = extenedErrorToNav2Error( extendedError );
            break;
         case NAV_STATUS_OK: 
            ERR("NAV_STATUS_OK");
         case NAV_STATUS_REQUEST_SPECIFIC_MASK: 
            ERR("NAV_STATUS_REQUEST_SPECIFIC_MASK");
            //will never happen
            ERR("Illegal program path!");
         }
         retval = receivedServerError;
         m_pendingList.concatenate(m_requestList);
         RequestListItem* req = NULL;
         err.setErr( error );
         while(NULL != (req = m_pendingList.dequeueFirst())){
            sendError( err, req->src );
            delete req->data;
            req->data = NULL;
            delete req->nrpData;
            req->nrpData = NULL;
            m_requestList.deallocate(req);
         }
      }
      ParameterBlock block(buffer);
      block.output(*m_log);
      //DBGDUMP("Parameterblock", block.accessRawData(0), block.getLength());
      bool isSet = false;
      const char* param = NULL;
      int32 u32param = 0;
      uint16 u16param = 0;
      const uint32* u32aparam = NULL;
      //It looks weird, but I've finally found a use for the comma operator!
      if((param = block.getAlternativeServer(&isSet), isSet)){
         DBG("Got an AlternativeServer parameter: %s", param);
         Buffer buf;
         buf.writeNextCharString(param);
         std::vector<Server*>::iterator q;
         std::vector<Server*> temp;// = new std::vector<Server*>;
         temp.reserve(m_servers->size());
         for(q = m_servers->begin(); q != m_servers->end(); ++q){
            //keep nonpersistent servers in a temp vector. delete the
            //persistent ones.
            if(*q != NULL && !(*q)->persistent){
               temp.push_back(*q);
               (*q)->setGroup(0); //shouldn't be necessary
            } else {
               delete *q;
            }
         }
         m_servers->clear();
         //return the nonpersisten servers to the real vector.
         while(!temp.empty()){
            m_servers->push_back(temp.back());
            temp.pop_back();
         }
         //read server list checksum
         char* end = NULL;
         int32 checksum = strtoul(param, &end, 0);
         if(end != param && *end == '!'){
            m_paramBlock->setServerListChecksum(checksum);
            m_paramProvider->
               setParam( m_serverListChecksumParam, &checksum );
            buf.jumpReadPosTo('!');
            buf.jumpReadPos(1);
         }
         //read in servers from the buffer. ',' separete servers ';'
         //separate server groups.
         int group = m_servers->empty() ? 0 : 1;
         uint8 next = '\0';
         do{
            Server* tmp = Server::factory(buf, group);
            if(tmp){
               m_servers->push_back(tmp);
            }
            next = buf.readNext8bit();
            if(next == ';'){
               next = ',';
               group++;
            }
         }while(next == ',');
         setReachable(*m_servers); //set all servers as reachable
         m_serverIdx = findFirstServer(*m_servers, m_seed); //startserver
         const char** tmpServers = new const char*[m_servers->size()];
         unsigned num = 0;
         Buffer textServers(buf.getLength() + 256);
         // Prepare to send the new server list to the parameter module.
         for(q = m_servers->begin(); q != m_servers->end(); ++q){
            if(!(*q)->persistent){
               DBG("Server %s is not persistent", (*q)->getHostAndPort());
            } else {
               DBG("Server %u: %s", num, (*q)->getHostAndPort());
               (*q)->toString(textServers);
               DBGDUMP("textServers", textServers.accessRawData(0), 
                       textServers.getLength(), 0, 1, 32);
               tmpServers[num++] = textServers.getNextCharString();
               DBG("tmpServer[%u] = '%s'", num-1, tmpServers[num-1]);
            }               
         }
         if(num < 1){
            WARN("No servers left, rerequest from parameters");
            //this is not good, we have no servers left. Better get
            //the old list from the paramer module.
            m_paramProvider->getParam( m_serverListParam );
            m_requestList.concatenate(m_pendingList);
            RequestListItem* req = NULL;
            while(NULL != (req = m_requestList.dequeueFirst())){
               ERR("sendError(NSC_OUT_OF_SERVERS, %#08"PRIx32")", req->src);
               sendError(NSC_OUT_OF_SERVERS, req->src);
               delete req->data;
               req->data = NULL;
               delete req->nrpData;
               req->nrpData = NULL;
               m_requestList.deallocate(req);
            }
         } else {
            DBG("Set %u servers to parameters.", num);
            m_paramProvider->
               setParam( m_serverListParam, tmpServers, num );
         }
         delete[] tmpServers;
      }
      if((param = block.getCallCenterMSISDN(&isSet), isSet)){
         DBG("Got a CallCenterMSISDN parameter: %s - setting", param);
         char* checksum = strdup_new(param);
         char* cc = strchr(checksum, '!');
         std::vector<char*> callCenters;
         callCenters.reserve(10);
         while(cc){
            *cc++ = '\0';
            callCenters.push_back(cc);
            cc = strchr(cc, ',');
         }
         int32 cs = strtoul(checksum, NULL, 0);
         m_paramProvider-> 
            setParam(ParameterEnums::NSC_CallCenterMSISDN, 
                     &callCenters.front(), callCenters.size());
         m_paramProvider->
            setParam(ParameterEnums::NSC_CallCenterChecksum, 
                     &cs);
         m_paramBlock->setCallCenterChecksum(cs);
         delete[] checksum;
      }
      if((u32param = block.getTransactionsLeft(&isSet), isSet)){
         enum TransactionType tt = TransactionType((u32param >> 30) & 0x03);
         u32param &= 0x3fffffff;
         DBG("%"PRIu32" transaction%s left", u32param, tt ? "s" : "DaysLeft");
         switch(tt){
         case transactions:
            m_expireVector[GuiProtEnums::transactionsLeft] = u32param;
            break;
         case daysLeftTransactions:
            m_expireVector[GuiProtEnums::transactionDaysLeft] = u32param;
         }
         DBGDUMP("Setting NSC_ExpireVector to:",
                 (uint8*)&(m_expireVector.front()),
                 m_expireVector.size() * sizeof(u32param), 16, 4, 2);
         m_paramProvider->
            setParam(ParameterEnums::NSC_ExpireVector, 
                     &(m_expireVector.front()), m_expireVector.size());
      }
      if((u16param = block.getSubscriptionLeft(&isSet), isSet)){
         DBG("%u left of subscription", u16param);
         time_t expire = TimeUtils::time(NULL) + u16param * 24 * 60 * 60;
         m_expireVector[GuiProtEnums::expireDay] = expire;
         m_paramProvider->
            setParam(ParameterEnums::NSC_ExpireVector, 
                     &(m_expireVector.front()), m_expireVector.size());
      }
      if((block.getUserLogin(&isSet), isSet) ||
         (block.getUserPassword(&isSet), isSet) || 
         (block.getPublicUserName(&isSet), isSet)){
         const char* up[2] = {NULL, NULL}; //for setting Nav2 parameters
         const char* webPass = NULL;
         const char* webUser = NULL;
         up[0] = block.getUserLogin(&isSet);
         if(!isSet){
            up[0] = NULL;
         }
         webPass = up[1] = block.getUserPassword(&isSet);
         if(!isSet){
            webPass = up[1] = NULL;
         }
         webUser = block.getPublicUserName(&isSet);
         if(!isSet){
            webUser = NULL;
         }
         //new username from server.
         DBG("username: '%s', password: '%s'", up[0], up[1]);
         DBG("WebUser: '%s', WebPass: '%s'", webUser, webPass);
         if(up[0] || up[1]){
            if(up[0]){
               //set the username parameter in the outgoing parameterblock
               m_paramBlock->setUserLogin(up[0]);
            } else if((up[0] = m_paramBlock->getUserLogin(&isSet), !isSet)){
               up[0] = NULL;
            }
            if(up[1]){
               m_paramBlock->setUserPassword(up[1]);
            } else if((up[1] = m_paramBlock->getUserPassword(&isSet), !isSet)){
               up[1] = NULL;
            }
            if(up[0] && up[1]){ //do we have a password?
               //yes we do! set the nav2 parameter.
               DBG("new username: '%s' and password '%s'", up[0], up[1]);
               m_paramProvider->setParam(ParameterEnums::NSC_UserAndPasswd, up,2);
               m_paramProvider->rewriteOrigFile();
            } else {
               //no we don't! Bugger off you sod!
               ERR("Received username '%s', but no password!", param);
            }
         }
         if(webUser && (!m_webUser || !strequ(webUser, m_webUser))){
            DBG("webUser: '%s' m_webUser: '%s'", webUser, m_webUser);
            delete m_webUser;
            m_webUser = strdup_new(webUser);
            //set the webusername parameter
            m_paramProvider->setParam(ParameterEnums::UC_WebUser, 
                                      &webUser);
         }
         if(webPass){
            m_paramProvider->setParam(ParameterEnums::UC_WebPasswd,
                                      &webPass);
         }
      }
      if((u32param = block.getWayfinderType(&isSet), isSet)){
         //WayfinderType differs in client and server.
         DBG("WayfinderType %#"PRIx32, u32param);
#define MATRIX_OF_DOOM_II
#ifdef MATRIX_OF_DOOM_I
         if((u32param == GuiProtEnums::Gold) && 
            (GuiProtEnums::Silver == m_paramBlock->getWayfinderType())){
            m_paramProvider->setParam(ParameterEnums::WayfinderType,
                                      &u32param);
            m_paramBlock->setWayfinderType(u32param);
         } else if(u32param != m_paramBlock->getWayfinderType()){
            retval = unauthorized;
            m_pendingList.concatenate(m_requestList);
            RequestListItem* req = NULL;
            while(NULL != (req = m_pendingList.dequeueFirst())){
               sendError(NSC_AUTHORIZATION_FAILED, req->src);
               delete req->data;
               req->data = NULL;
               delete req->nrpData;
               req->nrpData = NULL;
               m_requestList.deallocate(req);
            }
         }
#elif defined(MATRIX_OF_DOOM_II)
         m_paramProvider->setParam(ParameterEnums::WayfinderType,
                                   &u32param);
         m_paramBlock->setWayfinderType(u32param);
#else
#error No Matrix Of Doom defined.
#endif
      }

      int len = 0;
      const uint8* bparam = NULL;
      if ( (bparam = block.getOldUserLicense( len, &isSet ), isSet) ) {
         // Update last_imei in params
         uint8* lastImei = encryptIMEI( bparam, len );
         m_paramProvider->setParam( ParameterEnums::NSC_LastIMEI,
                                    (const uint8*)lastImei, len );
         delete [] lastImei;
      }
      if ( (bparam = block.getServerAuthBob( len, &isSet ), isSet) ) {
         // Update ServerAuthBob in params
         m_paramProvider->setParam( 
            ParameterEnums::NSC_serverAuthBob,
            (const uint8*)bparam, len );
         m_paramBlock->setServerAuthBob( (const uint8*)bparam, len );
         if ( (u32param = block.getServerAuthBobChecksum( &isSet ), isSet))
         {
            m_paramProvider->setParam( 
               ParameterEnums::NSC_serverAuthBobChecksum,
               &u32param );
            m_paramBlock->setServerAuthBobChecksum( u32param );
         }
      }
      if ( (bparam = block.getCenterPointAndScale( len, &isSet ), isSet) )
      {
         // Update Centerpoint in params
         Buffer *buf = new Buffer( 20 );
         Buffer data( (uint8*)bparam, len, len );
         int32 lat = data.readNextUnaligned32bit();
         int32 lon = data.readNextUnaligned32bit();
         int32 scale = data.readNextUnaligned32bit();
         buf->writeNext32bit( 2 ); /* Version 2 supported here. See SetVectorMapCoordinates i MapView.cpp */
         buf->writeNext32bit( scale );
         buf->writeNext32bit( lon );
         buf->writeNext32bit( lat );
         m_paramProvider->setParam( 
            ParameterEnums::UC_VectorMapCoordinates,
            (const uint8*)buf->accessRawData(), buf->getLength() );
         data.releaseData(); // Don't delete bparam
         delete buf;
      }
      if ( (u32aparam = block.getUserRights( len, &isSet ), isSet) ) {
         DBG("Received new user rights.");
         // Set parameter
         m_paramProvider->setParam( 
            ParameterEnums::NSC_userRights,
            reinterpret_cast<const int32*> (u32aparam), len );
      }
      if ( (param = block.getNewVersion( &isSet ), isSet) ) {
         //Also write to versionfile here!
         if (writeVersionFile(param, VERSION_FILE_NAME) != 0) {
            DBG("Could not write version file.");
         }
         // Set parameter
         m_paramProvider->setParam( 
            ParameterEnums::NSC_newVersion, &param, 1 );
      }
      if ( (param = block.getNewVersionUrl( &isSet ), isSet) ) {
         // Set parameter
         m_paramProvider->setParam( 
            ParameterEnums::NSC_newVersionUrl, &param, 1 );
      }
      if ( (param = block.getLatestNewsId( &isSet ), isSet) ) {
         // Set parameter
         m_paramProvider->setParam( 
            ParameterEnums::NSC_latestNewsId, &param, 1 );
      }
      if ( (param = block.getFavoritesCRC( &isSet ), isSet) ) {
         // Set parameter
         m_paramProvider->setParam( 
            ParameterEnums::NSC_favoritesCRC, &param, 1 );
      }
      if ( (u32param = block.getServerPtui( &isSet ), isSet) ) {
         // Set parameter
         m_paramProvider->setParam( 
            ParameterEnums::NT_ServerTrafficUpdatePeriod, &u32param, 1 );
      }

      m_paramBlock->setNewPasswordSet(NULL);
      m_paramBlock->setDisableNoRegionCheck(false);
      return retval;
   }

   int NavServerCom::writeVersionFile(const char* verInfo, const char *version_file_name)
   {
      char* dirName = NULL;
#if defined(__SYMBIAN32__) || defined(UNDER_CE)
      dirName = strdup_new(getGlobalData().m_data_base_path);
#else
      dirName = strdup_new("./");
#endif
      if (verInfo) {
         char fileName[256];
         strcpy(fileName, dirName);
         strcat(fileName, version_file_name);
         FILE* pSaveFile = fopen(fileName, "w");
         if (pSaveFile != NULL) {
            fprintf(pSaveFile, "%s\n", verInfo);
            fclose(pSaveFile);
            delete[] dirName;
            return 0;
         }
      }
      delete[] dirName;
      return -1;
   }

   enum NavServerCom::Event NavServerCom::decodePollServerReply(Buffer& buffer,
                                                                uint8 status)
   {
      DBG("decodePollServerReply()");
      enum Event retEvent = pollReply;
      int data_length = buffer.getLength() - buffer.getReadPos();
      DBG("data_length == %d", data_length);
      if(status != NAV_STATUS_OK && 
         !(status & NAV_STATUS_REQUEST_SPECIFIC_MASK)){
         ERR("decodedPollServerReply: bad poll server reply 0x%"PRIx8, status);
         retEvent = receivedServerError; //XXX better error handling.
         if(m_syncRequest != NULL){
            DBG("Sync request that could match this poll data");
            uint32 requester = m_syncRequest->src;
            delete m_syncRequest->data;
            m_syncRequest->data = NULL;
            m_requestList.deallocate(m_syncRequest);
            m_syncRequest = NULL;
            sendError(NSC_SERVER_NOT_OK, requester);
         }
      } else{
         if (status == POLL_SERVER_FINAL) {
            DBG("final poll reply");
            retEvent = finalPollReply;
            if (buffer.remaining() <= 0) {
               DBG("no poll data returned");
            }
         }
         //DBGDUMP("Poll data:", buffer.accessRawData(), buffer.remaining());
         while(buffer.remaining() > 0){
            DBG("%"PRIu32" bytes remaining of binary transfer", 
                buffer.remaining());
            BinaryTransferType type = 
               BinaryTransferType(buffer.readNextUnaligned32bit());
            switch(type){
            case CrossingMapData:
               ERR("Unhandled binary type : CrossingMapType");
               break;
            case SyncFavoritesRequest:
               // This is NGP now! Look in Destinationmodule, its all there
               ERR("Unhandled binary type : SyncFavoritesRequest");
               break;
            case MapData:
               ERR("Unhandled binary type : MapData");
               break;
            case TopRegionListData:
               DBG("Received TopRegionList, sending to parameters.");
               {
                  int start = buffer.getReadPos();
                  TopRegionList trList(buffer);
                  int size = buffer.getReadPos() - start;
#if defined NAV2_USE_UTF8
                  m_paramProvider->setParam(ParameterEnums::TopRegionList, 
                        buffer.accessRawData(start), size);
#else
                  m_paramProvider->setParam(ParameterEnums::TopRegionListLegacy, 
                        buffer.accessRawData(start), size);
#endif
                  m_paramBlock->setTopRegionChecksum(trList.getVersion());
                  if(m_log){
                     trList.log(*m_log);
                  }
               }
               break;
            case LogfileData:
               ERR("Unhandled binary type : LogfileData");
               break;
            case CategoriesFile:
               DBG("New categories:");
               {
                  int32 checksum = buffer.readNextUnaligned32bit();
                  uint32 numcat = buffer.readNextUnaligned32bit();
                  char** idcat = new char*[numcat];
                  char** namecat = new char*[numcat];
                  for(unsigned i = 0; i < numcat; ++i){
                     idcat[i] = strdup_new(buffer.getNextCharString());
                     namecat[i] = strdup_new(buffer.getNextCharString());
                  }
                  uint32 readpos = buffer.getReadPos();
                  bool hasicons = buffer.readNextUnaligned32bit() == 3501;
                  char** iconcats = NULL;
                  if( hasicons ){
                     // There are image names in the buffer
                     iconcats = new char*[numcat];
                     for(unsigned i = 0; i < numcat; ++i){
                        iconcats[i] = strdup_new(buffer.getNextCharString());
                     }
                  }
                  else{
                     buffer.setReadPos( readpos );
                  }

                  readpos = buffer.getReadPos();
                  bool hasids = buffer.readNextUnaligned32bit() == 3502;
                  int32* catids = NULL;
                  if( hasids ){
                     // There are category ids in the buffer
                     catids = new int32[numcat];
                     for(unsigned i = 0; i < numcat; ++i){
                        catids[i] = (int32)buffer.readNextUnaligned16bit();
                     }
                  }
                  else{
                     buffer.setReadPos( readpos );
                  }

                  m_paramProvider->setParam(ParameterEnums::CategoryIds, idcat, numcat);
                  m_paramProvider->setParam(ParameterEnums::CategoryNames,namecat,numcat);
                  m_paramProvider->setParam(ParameterEnums::NSC_CategoriesChecksum, 
                                            &checksum);
                  if( hasicons ){
                     m_paramProvider->setParam(ParameterEnums::CategoryIcons,iconcats,numcat);
                  }
                  if( hasids ){
                     m_paramProvider->setParam(ParameterEnums::CategoryIntIds,catids,numcat);
                  }

                  m_paramBlock->setCategories(checksum);
                  for(unsigned j = 0; j < numcat; ++j){
                     delete[] idcat[j];
                     delete[] namecat[j];
                     if( hasicons ){
                        delete[] iconcats[j];
                     }
                  }
                  delete[] idcat;
                  delete[] namecat;
                  delete[] iconcats;
                  delete[] catids;
               }
               break;
            case LatestNewsFile:
               {
                  DBG("New News!");
                  int32 checksum = buffer.readNextUnaligned32bit();
                  uint32 size = buffer.readNextUnaligned32bit();
#ifdef __linux
                  char file[] = "LatestNews.png";
                  FILE* fd = fopen(file, "wb");
                  if(fd){
                     fwrite(buffer.accessRawData(), size, 1, fd);
                     fclose(fd);
                  }
#endif
                  m_nscConsumer->latestNews(checksum, buffer.accessRawData(), 
                                            size);
                  buffer.jumpReadPos(size);
/*                   checksum = 0; */
                  m_paramProvider->setParam(ParameterEnums::NSC_LatestNewsChecksum,
                                            &checksum);
                  m_paramBlock->setLatestNews(checksum);
               }
               break;
            default:
               ERR("Unknown binary transfer type: %0#10x", type);
               break;
            }
         }
         m_pollOffset += data_length;
         DBG("PollOffset == %"PRIu32, m_pollOffset);
      }
      return retEvent;
   }

   void NavServerCom::decodedReceiveData(int len, const uint8 *in, uint32 src)
   {
      //DBGDUMP("Recieved serial data:", in, len);
      src = src; //currently unused
      DBG("Setting timer %u timeout %"PRIu32, m_timeoutTimerID, m_timeoutTime);
      setTimeoutTimer(m_timeoutTime);
      if(m_fakeConnectTimer != 0){
         DBG("removing fakeConnectTimer %u", m_fakeConnectTimer);
         m_queue->removeTimer(m_fakeConnectTimer);
         m_fakeConnectTimer = 0;
      }
      // It is very important that the timer is updated before
      // decodeServerPacket is called as the function may cause the
      // timer to be removed.
      m_parser->decodeServerPacket(in, len);
      DBG("End of decodedReceiveData");
   }

   void NavServerCom::routeReply(uint16 reqID)
   {
      class RequestListItem* req = m_pendingList.dequeue(reqID);
      if(req){
         m_nscConsumer->routeReply(req->src);
         delete req->data;
         req->data = NULL;
         delete req->nrpData;
         req->nrpData = NULL;
         m_requestList.deallocate(req);
      }      
   }

   NavServerCom::Event NavServerCom::unsolicitedMessage(
      Buffer& input, uint16 type, uint8 status, ErrorObj& err,
      uint32 extendedError )
   {
      enum Event retEvent;
      switch (type) {
      case NAV_SERVER_PARAMETER_REPLY:
         DBG("unsolicitedMessage: NAV_SERVER_PARAMETER_REPLY");
         //TRACE_DBG("unsolicitedMessage: NAV_SERVER_PARAMETER_REPLY");
         retEvent = decodeParameterReply( input, status, err, 
                                          extendedError );
         break;
      case NAV_SERVER_POLL_SERVER_REPLY:
         DBG("unsolicitedMessage: NAV_SERVER_POLL_SERVER_REPLY- WE SHOULD NOT BE HERE");
         //retEvent = decodePollServerReply(input, status);
         retEvent = receivedServerReply; // for compilator error fix only.
         // Don't update state here. decodePollServerReply handles it.
         break;
      case NAV_SERVER_MULTI_VECTOR_MAP_REPLY:
         DBG("Unsolicited MultiVector Map Reply.");
         m_nscConsumer->unsolMultiVectorMapReply( 
            input.remaining(), input.accessRawData(), 
            m_lastRouteAddress );
         retEvent = receivedServerReply;
         break;
      case NAV_REPLY : {
         DBG( "NAV_REPLY unsolicited" );
//         TRACE_FUNC1( "Received NAV_REPLY unsolicited" );
         if ( m_tracking ) {
            m_tracking->reply( input.accessRawData(), input.remaining(),
                               0/*src*/ );
         }
         retEvent = receivedServerReply;
      }  break;

      default:
         ERR("unsolicitedMessage: Unknown type=%d, status=%d", type, status);
         retEvent = receivedServerReply;
         break;
      }
      return retEvent;
   }

   enum NavServerCom::Event 
   NavServerCom::solicitedMessage(Buffer& input, uint16 type, 
                                  RequestListItem*& request)
   {
      switch (type) {
      case NAV_SERVER_SEARCH_REPLY:
         {
            decodeSearchReply(input, request->src);
         }
         break;
      case NAV_SERVER_INFO_REPLY:
         DBG("Additional info reply");
         m_nscConsumer->searchInfoReply(input.accessRawData(), 
                                        input.remaining(), request->src);
         break;
      case NAV_SERVER_POLL_SERVER_REPLY:
         DBG("Not really unsolicitedMessage: NAV_SERVER_POLL_SERVER_REPLY - WE SHOULD NOT BE HERE EITHER");
         //decodePollServerReply(input, NAV_STATUS_OK);
         // Don't update state here. decodePollServerReply handles it.
         break;
      case NAV_SERVER_UPGRADE_REPLY:
         DBG("UPGRADE REPLY");
         {
            bool region = !! input.readNext8bit(); //force to bool
            bool key    = !! input.readNext8bit(); //force to bool
            bool phone  = !! input.readNext8bit(); //force to bool
            enum GuiProtEnums::WayfinderType wft = 
               GuiProtEnums::WayfinderType(input.readNext8bit());
            bool name   = !! input.readNext8bit(); //force to bool
            bool email  = !! input.readNext8bit(); //force to bool

            m_nscConsumer->licenseKeyReply(key, phone, region, name, email,
                                           wft, request->src);
            if(region && key && phone && name && email){
               //               int32 gold = GuiProtEnums::Gold;
               m_paramBlock->setWayfinderType(wft);
               int32 paramVal = wft;
               m_paramProvider->setParam(ParameterEnums::WayfinderType,
                                         &paramVal);
            }
         }
         break;
      case NAV_SERVER_MAP_REPLY:
         DBG("Map reply");
         input.setReadPos(NAV_SERVER_COM_HEADER_SIZE);
         DBG("mapReply(%"PRIu32", %p, %0#10"PRIx32")", input.remaining(), 
             input.accessRawData(), request->src);
         DBGDUMP("Map Reply header", input.accessRawData(), 30);
         m_nscConsumer->mapReply(input.remaining(), input.accessRawData(), 
                                 request->src);
         break;
      case NAV_SERVER_GPS_ADDRESS_REPLY:
         DBG("decode where am i rep");
         decodeWhereAmIReply(input, request->src);
         break;
      case NAV_SERVER_GPS_INIT_REPLY:
         DBG("decode gps init reply");
         decodeGpsInitReply(input, request->src);
         break;
      case NAV_SERVER_BINARY_UPLOAD_REPLY:
         DBG("got binary upload reply.");
         m_nscConsumer->binaryUploadReply(request->src);
         //keep request data until poll answer.
         m_syncRequest = request;
         request = NULL;
         break;
      case NAV_SERVER_DEST_REPLY:
         DBG("decode dest rep");
         break;
      case NAV_SERVER_ROUTE_REPLY:
      case NAV_SERVER_PARAMETER_REPLY: {
         ERR("Replies of type %x shouldn't be here!", type);
         // Remove from m_paramSyncs and send reply
         std::deque<uint32>::iterator new_end = std::remove_if(
               m_paramSyncs.begin(), m_paramSyncs.end(),
               std::bind2nd( equal_to<uint32>(), request->src ) );
         m_paramSyncs.erase( new_end, m_paramSyncs.end() );
         bool wftSet = false;
         GuiProtEnums::WayfinderType wft = GuiProtEnums::WayfinderType(
            m_paramBlock->getWayfinderType( &wftSet ) );
         if ( !wftSet ) {
            wft = GuiProtEnums::InvalidWayfinderType;
         }
         m_nscConsumer->paramSyncReply( wft, request->src );
         uint8 status = 0;
         ErrorObj err(isab::Nav2Error::NO_ERRORS);
         uint32 extendedError = 0;
         // Send to everybody as well (like if it was an unsolicitedMessage).
         decodeParameterReply( input, status, err, 
                               extendedError );
      }  break;
      case NAV_SERVER_MESSAGE_REPLY:
         decodeMessageReply(input, request->src);
         break;
      case NAV_SERVER_VECTOR_MAP_REPLY:
         DBG("Vector Map Reply");
         //DBGDUMP("Vector Map Reply", input.accessRawData(), input.remaining());
         m_nscConsumer->vectorMapReply(input.remaining(), 
                                       input.accessRawData(), request->src);
         break;
      case NAV_SERVER_MULTI_VECTOR_MAP_REPLY:
         DBG("MultiVector Map Reply.");
         // DBGDUMP("MultiVector Map Reply", 
         //         input.accessRawData(), input.remaining());
         m_nscConsumer->multiVectorMapReply(input.remaining(), 
                                            input.accessRawData(), 
                                            request->src);
         break;
      case NAV_SERVER_CELL_CONFIRM:
         DBG("CELL_CONFIRM");
         m_nscConsumer->cellConfirm(input.remaining(), input.accessRawData(),
                                    request->src);
         break;
      case NAV_REPLY : {
         DBG( "NAV_REPLY" );
//         TRACE_FUNC1( "Received NAV_REPLY" );
         if ( m_tracking ) {
            m_tracking->reply( input.accessRawData(), input.remaining(),
                               request->src );
         }
         m_nscConsumer->sendReply( 
            input.accessRawData(), input.remaining(),
            request->src );
         } break;

      default:
         ERR("Unknown or unsupported TYPE %#x in decode!", type);
         break;
      }
      return receivedServerReply;
   }

   void NavServerCom::decodeBuffer( 
      Buffer* inBuf, uint8 req_id, uint16 type, 
      uint8 status, ErrorObj& err,uint32 extendedError )
   {
      DBG("decodeBuffer(%p, %#x, %#x, %#x)", inBuf, req_id, type, status);

      if(m_sessionState != timedOut && m_sessionState != interrupted){
         Buffer& input = *inBuf;
         enum Event event;
         // Ok, try to find the correct request. Remember that this is
         // a reply, so the element can't be on the req_list or in the
         // current element pointer.
         RequestListItem* request = m_pendingList.dequeue(uint16(req_id));
         if (!request) {
            DBG("No matching request,");
            // We couldn't find the element on the "sent" list. This
            // could be because the message is a
            // NAV_SERVER_PARAMETER_REPLY or
            // NAV_SERVER_POLL_REPLY. Presently any other message from
            // the server that doesn't have a corresponding element on
            // the sent list is considered to be an error. It's
            // possible that we get NAV_SERVER_GPS_INIT_REQ or simliar
            // here in the future.
            event = unsolicitedMessage( input, type, status, err, 
                                        extendedError );
            if(!m_paramSyncs.empty()){
               bool wftSet = false;
               GuiProtEnums::WayfinderType wft = 
                  GuiProtEnums::WayfinderType(m_paramBlock->getWayfinderType(&wftSet));
               if(!wftSet){
                  wft = GuiProtEnums::InvalidWayfinderType;
               }
               while(!m_paramSyncs.empty()){
                  if ( m_paramSyncs.front() != 0 ) {
                     DBG( "Parameter sync reply to %#"PRIx32". "
                          "Wayfindertype: %d", m_paramSyncs.front(), wft );
                     if ( status != NAV_STATUS_OK ) {
                        event = handleErrorStatus( 
                           ReplyStatus(status), err, 
                           extendedError, m_paramSyncs.front() );
                     } else {
                        m_nscConsumer->paramSyncReply( 
                           wft, m_paramSyncs.front() );
                     }
                  }
                  m_paramSyncs.pop_front();
               }
            }
         } else if (status != NAV_STATUS_OK && 
                    !(status & NAV_STATUS_REQUEST_SPECIFIC_MASK)){
            //reply status not ok. 
            event = handleErrorStatus( ReplyStatus(status), err, 
                                       extendedError, request->src );
         } else if(type == NAV_SERVER_ROUTE_REPLY) { 
            //only route replies with request specific status should come here
            event = handleRouteReplyStatus(RouteReplyStatus(status), input, 
                                           type, request);
         } else if ( type == NAV_SERVER_UPGRADE_REPLY && 
                     (status & NAV_STATUS_REQUEST_SPECIFIC_MASK) ) {
            event = handleErrorStatus( ReplyStatus(status), err, 
                                       extendedError, request->src );
         } else if ( type == NAV_SERVER_POLL_SERVER_REPLY ) {
            DBG("found a matching poll request reply");
            event = decodePollServerReply(input, status);
            // Since we're short-cutting the poll packets now we want it
            // to behave as a normal solicited message.
            event = receivedServerReply;
         } else {
            DBG("found a matching request");
            event = solicitedMessage(input, type, request);
         }
         if(request != NULL){
            delete request->data;
            request->data = NULL;
            delete request->nrpData;
            request->nrpData = NULL;
            m_requestList.deallocate(request);
         }
         updateState(event);
      } else {
         //XXX is this what we want to do?
         WARN("interrupted or timed out. Discarding buffer.");
      }
      DBG("End of decodeBuffer");
   }

   enum NavServerCom::Event
   NavServerCom::handleErrorStatus( enum ReplyStatus status, 
                                    ErrorObj& err,
                                    uint32 extendedError, 
                                    uint32 src )
   {
      enum Event event = receivedServerError;
      enum ErrorNbr error = NSC_SERVER_NOT_OK;
      switch( status ) {
      case NAV_STATUS_NOT_OK:
      case NAV_STATUS_PARAM_REQ_NOT_FIRST:
         error = NSC_SERVER_NOT_OK;                      
         break;
      case NAV_STATUS_REQUEST_TIMEOUT:
         error = NSC_SERVER_REQUEST_TIMEOUT;               
         break;
      case NAV_STATUS_OUTSIDE_MAP:
         error = NSC_SERVER_OUTSIDE_MAP;                   
         break;
      case NAV_STATUS_PROTOVER_NOT_SUPPORTED:
         error = NSC_SERVER_PROTOCOL_ERROR; 
         break;
      case NAV_STATUS_OUTSIDE_ALLOWED_AREA:
         error = NSC_SERVER_UNAUTHORIZED_MAP;              
         break;
      case NAV_STATUS_NO_TRANSACTIONS_LEFT:
         error = NSC_NO_TRANSACTIONS;                      
         break;
      case NAV_STATUS_UNAUTH_OTHER_HAS_LICENSE: 
         ERR("NAV_STATUS_UNAUTH_OTHER_HAS_LICENSE");
         error = NSC_UNAUTH_OTHER_HAS_LICENSE;             
         break;
      case NAV_STATUS_EXTENDED_ERROR :
         error = extenedErrorToNav2Error( extendedError ); 
         break;
      case NAV_STATUS_OK:
      case NAV_STATUS_REQUEST_SPECIFIC_MASK:
         //will never happen
         ERR("Illegal program path!");
         event = invalidEvent;
      }
      if (status & NAV_STATUS_REQUEST_SPECIFIC_MASK) {
         DBG("Request specific status: %#x", status);
         switch(ParamReplyStatus(status)){
            case PARAM_REPLY_WF_TYPE_TOO_HIGH:
               ERR("PARAM_REPLY_WF_TYPE_TOO_HIGH");
               break;
            case PARAM_REPLY_UPDATE_NEEDED:
               DBG("Program update needed.");
               error = NSC_SERVER_NEW_VERSION;
               break;
            case PARAM_REPLY_EXPIRED_USER:
               error = NSC_EXPIRED_USER;
               ERR("User account has expired. Aborting with error message.");
               break;
            case PARAM_REPLY_UNAUTHORIZED_USER:
               error = NSC_AUTHORIZATION_FAILED;
               break;
            case PARAM_REPLY_REDIRECT:
               DBG("Redirecting to alternate server.");
/*                m_serverIdx = nextServer(m_seed, *m_servers, m_serverIdx); */
               break;
            case PARAM_REPLY_UNAUTH_OTHER_HAS_LICENSE :
               DBG("PARAM_REPLY_UNAUTH_OTHER_HAS_LICENSE");
               error = NSC_UNAUTH_OTHER_HAS_LICENSE;
               break;
            case CHANGED_LICENSE_REPLY_OLD_LICENSE_NOT_IN_ACCOUNT :
               DBG("CHANGED_LICENSE_REPLY_OLD_LICENSE_NOT_IN_ACCOUNT");
               error = NSC_OLD_LICENSE_NOT_IN_ACCOUNT;
               break;
            case CHANGED_LICENSE_REPLY_MANY_USERS_WITH_OLD_LICENSE :
               DBG("CHANGED_LICENSE_REPLY_MANY_USERS_WITH_OLD_LICENSE");
               error = NSC_OLD_LICENSE_IN_MANY_ACCOUNTS;
               break;
            case CHANGED_LICENSE_REPLY_MANY_USERS_WITH_NEW_LICENSE :
               DBG("CHANGED_LICENSE_REPLY_MANY_USERS_WITH_NEW_LICENSE");
               error = NSC_NEW_LICENSE_IN_MANY_ACCOUNTS;
               break;
            case CHANGED_LICENSE_REPLY_OLD_LICENSE_IN_OTHER_ACCOUNT :
               DBG("CHANGED_LICENSE_REPLY_OLD_LICENSE_IN_OTHER_ACCOUNT");
               error = NSC_OLD_LICENSE_IN_OTHER_ACCOUNT;
               break;
            case UPGRADE_REPLY_MUST_CHOOSE_REGION :
               DBG("UPGRADE_REPLY_MUST_CHOOSE_REGION");
               error = NSC_UPGRADE_MUST_CHOOSE_REGION;
               break;
         }
      }
      DBG("Sending error %0#10x as reply to request %0#10"PRIx32".",
          error, src);
      err.setErr( error );
      sendError( err, src );
      if(status == NAV_STATUS_PARAM_REQ_NOT_FIRST){
         sendError(NSC_PARAM_REQ_NOT_FIRST);
      }
      //I'm not sure why event is set to receivedServerError here,
      //overriding the assignment to invalidEvent. This was how it was
      //coded before I made it a function of it's own, so I kept it
      //that way.
      event = receivedServerError;
      return event;
   }

   enum NavServerCom::Event 
   NavServerCom::handleRouteReplyStatus(enum RouteReplyStatus rrs,
                                        class Buffer& input, 
                                        uint16 type, 
                                        class RequestListItem* request)
   {
      //only route replies with request specific status should come here
      enum Event event = receivedServerError; //default is error.
      WARN("Bad route reply. status: %#x", unsigned(rrs));
      enum ErrorNbr err = NO_ERRORS;
      switch(rrs){
      case ROUTE_REPLY_NO_ROUTE_FOUND:
         err = NSC_SERVER_NO_ROUTE_FOUND;                    break;
      case ROUTE_REPLY_TOO_FAR_FOR_VEHICLE:
         err = NSC_SERVER_ROUTE_TOO_LONG;                    break;
      case ROUTE_REPLY_PROBLEM_WITH_ORIGIN:
         err = NSC_SERVER_BAD_ORIGIN;                        break;
      case ROUTE_REPLY_PROBLEM_WITH_DEST:
         err = NSC_SERVER_BAD_DESTINATION;                   break;
      case ROUTE_REPLY_NO_AUTO_DEST:
         err = NSC_SERVER_NO_HOTDEST;                        break;
      case ROUTE_REPLY_NO_ORIGIN:
         ERR("Bad route request, no origin!");               break;
      case ROUTE_REPLY_NO_DESTINATION:
         ERR("Bad route request, no destination");           break;
      case ROUTE_REPLY_NO_ROUTE_CHANGE:
         ERR("Keep using old route, not meant to be here!"); break;
      }
      if(err != NO_ERRORS){
         sendError(err, request->src);
      }
      return event;
   }
   
   MsgBuffer* NavServerCom::dispatch(MsgBuffer* buf)
   {
      if (!m_shutdownPrepared) {
         if(buf) buf = m_ser_consumer_decoder.dispatch(buf, this);
         if(buf) buf = m_comDecoder.dispatch(buf, this);
         if(buf) buf = m_navTaskDecoder.dispatch(buf, this);
         if(buf) buf = m_paramDecoder.dispatch(buf, this);
      }
      if(buf) buf = m_ctrlHubAttachedDecoder.dispatch(buf, this);
      if(buf) buf = Module::dispatch(buf);
      return buf;
   }

   void NavServerCom::decodedConnectionNotify(enum ConnectionNotify state, 
                                              enum ConnectionNotifyReason rsn,
                                              uint32 src)
   {
      src = src; //currently unused.
      m_connectState = state;
      switch(state){
      case CONNECTING:
         if(m_fakeConnectTimer == MAX_UINT16){
            DBG("CONNECTING while waiting on DISCONNECT FAKETIMEOUT"
                " - disregard");
         } else {
            DBG("CONNECTING to server");
            setTimeoutTimer(m_timeoutTime);
         }
         break;
      case CONNECTED:
         if(m_fakeConnectTimer == MAX_UINT16){
            DBG("CONNECTED while waiting in DISCONNECT FAKETIMEOUT "
                "- disregard");
         } else {
            DBG("CONNECTED to server");
            if(m_fakeConnectTimer == 0){
               m_fakeConnectTimer = m_queue->defineTimer(30*1000);
               DBG("Setting fake connect timer %u to %u",
                   m_fakeConnectTimer, 30*1000);
            }
            updateState(connectionEstablished);
         }
         break;
      case DISCONNECTING:
         DBG("DISCONNECTING, reason %d",rsn);
         if(rsn == NO_RESPONSE && m_fakeConnectTimer == MAX_UINT16){
            DBG("DISCONNECT FAKETIMEOUT. Fakeconnect timer set to 0");
            m_fakeConnectTimer = 0;
         }
         m_disconnectReason = rsn;
         updateState(disconnected);
         break;
      case CLEAR:
         // Sending disconnected events even when reporting CLEAR is
         // necessary for the timeout mechanism?
         DBG("CLEAR from server (CLEAR), rsn %d", rsn);
         m_disconnectReason = rsn;
         break;
      case WAITING_FOR_USER:
         if (m_miscTimerID == 0) {
            m_miscTimerID = m_queue->defineTimer(-400000);
         } else {
            m_queue->setTimer(m_miscTimerID, -400000);
         }
      }
   }

   CtrlHubAttachedPublic * NavServerCom::newPublicCtrlHub()
   {
      DBG("newPublicCtrlHub()\n");
      return new CtrlHubAttachedPublic(m_queue);
   }

   RequestListItem* NavServerCom::simpleRequest(uint16 in_request, 
                                                Buffer* data, 
                                                RefreshLocation* refresh,
                                                uint32 src)
   {
      DBG("simpleRequest(0x%"PRIx16", %p, %#"PRIx32")", in_request, data, src);
      if (m_offlineTester && m_offlineTester->OffLineMode()){ 
         ERR("sendError(NSC_FLIGHT_MODE, %#"PRIx32")", src);
         sendError(NSC_FLIGHT_MODE, src);
         delete data;
         return NULL;
      }
      if(m_offlineTester && !m_offlineTester->NetworkAvailable()){ 
         ERR("sendError(NSC_NO_NETWORK_AVAILABLE, %#"PRIx32")", src);
         sendError(NSC_NO_NETWORK_AVAILABLE, src);
         delete data;
         return NULL;
      }
      DBG("OnLine and have network");
      DBG("m_requestList.outstanding() == %u, m_pendingList.outstanding() == %u", m_requestList.outstanding(), m_pendingList.outstanding());
      RequestListItem* elem = NULL;
      /* Allocate a request element. */
//      bool isSet = false;
      if(m_shutdownPrepared){
         //sendError(NSC_CANCELED_BY_SHUTDOWN, src);
         delete refresh;
         if( in_request != NAV_SERVER_PARAMETER_REQ ) {
            delete data;
         }
/*
      }else if((m_paramBlock->getUserLogin(&isSet), !isSet) || 
               (0 == strcmp(m_paramBlock->getUserLogin(), "") &&
                m_paramBlock->getWayfinderType() != GuiProtEnums::Trial &&
                m_paramBlock->getWayfinderType() != GuiProtEnums::Silver)){
         //no username in parameter file.
         sendError(NSC_NO_USERNAME, src);
         delete refresh;
         if( in_request != NAV_SERVER_PARAMETER_REQ ) {
            delete data;
         } // Don't delete m_paramBlock
*/
      } else {
         elem = m_requestList.allocate();
         if (!elem) {
            // this is very serious as it implies that we ran out of
            // dynamic memory. The only way out is really to bug out and
            // come back another day.
            ERR("simpleRequest failed to allocate a RequestListItem.");
            ERR("sendError(NSC_REQUEST_ALLOC_FAILED");
            delete refresh;
            if( in_request != NAV_SERVER_PARAMETER_REQ ) {
               delete data;
            } // Don't delete m_paramBlock
            sendError(NSC_REQUEST_ALLOC_FAILED);
         } else {
            class ServerRequestData* srd = NULL;
            /* Initialize data. */

            elem->reqID = m_reqID++;
            DBG("simpleRequest: m_reqID=0x%"PRIx8, elem->reqID);
            DBG("simpleRequest: requestType=0x%"PRIx16, in_request);
            elem->request = in_request;
            elem->errorType = NAV_STATUS_OK;
            elem->status = 0;
            elem->src = src;
            if(in_request != NAV_SERVER_PARAMETER_REQ){
               elem->data = srd = new ServerRequestData(data);
            } else {
               DBG("This is a parameterblock. data and src set to 0");
               elem->data = NULL;
            }
            
            /* Insert request element into the request list. */
            if (m_requestList.enqueue(elem)) {
               if(refresh){
                  srd->addRefresh(refresh);
               }
               updateState(receivedRequest); // all is well
            } else {
               // Insertion fail. This should be impossible. 
               ERR("Impossible error, simpleRequest failed for %x",in_request);
               // HM. If we get here the elem or list is broken, 
               // so don't try to deallocate the element. 
               // Memory leakage is the least of our problems here. 
               delete refresh;
               delete data;
               elem->data = NULL;
               elem = NULL;
               ERR("sendError(NSC_BEYOND_SALVATION)");
               sendError(NSC_BEYOND_SALVATION);
            }
         }
      }
      return elem;
   }

   RequestListItem* NavServerCom::addNrpRequest(NavRequestPacket* nrpData)
   {
      DBG("addNrpRequest(0x%"PRIx16", %p, %#"PRIx32")", 
          NAV_SERVER_POLL_SERVER_REQ, nrpData);
      if (m_offlineTester && m_offlineTester->OffLineMode()){ 
         ERR("sendError(NSC_FLIGHT_MODE, %#"PRIx32")", 0);
         sendError(NSC_FLIGHT_MODE);
         delete nrpData;
         return NULL;
      }
      if(m_offlineTester && !m_offlineTester->NetworkAvailable()){ 
         ERR("sendError(NSC_NO_NETWORK_AVAILABLE, %#"PRIx32")", 0);
         sendError(NSC_NO_NETWORK_AVAILABLE);
         delete nrpData;
         return NULL;
      }
      DBG("OnLine and have network");
      DBG("m_requestList.outstanding() == %u, m_pendingList.outstanding() == %u", m_requestList.outstanding(), m_pendingList.outstanding());
      RequestListItem* elem = NULL;
      /* Allocate a request element. */
      if(m_shutdownPrepared){
         //sendError(NSC_CANCELED_BY_SHUTDOWN, src);
         delete nrpData;
      } else {
         elem = m_requestList.allocate();
         if (!elem) {
            // this is very serious as it implies that we ran out of
            // dynamic memory. The only way out is really to bug out and
            // come back another day.
            ERR("addNrpRequest failed to allocate a RequestListItem.");
            ERR("sendError(NSC_REQUEST_ALLOC_FAILED");
            delete nrpData;
            // Don't delete m_paramBlock
            sendError(NSC_REQUEST_ALLOC_FAILED);
         } else {
            /* Initialize data. */
            elem->reqID = m_reqID++;
            DBG("addNrpRequest: m_reqID=0x%"PRIx8, elem->reqID);
            nrpData->setReqID(elem->reqID);
            elem->request = NAV_SERVER_POLL_SERVER_REQ;
            DBG("addNrpRequest: requestType=0x%"PRIx16, elem->request);
            elem->errorType = NAV_STATUS_OK;
            elem->status = 0;
            elem->src = 0;
            DBG("This is a special poll packet. data and src set to 0");
            elem->data = NULL;
            elem->src = 0;
            elem->nrpData = new NrpDataHolder(nrpData);

            /* Insert request element into the request list. */
            if (m_requestList.enqueue(elem)) {
               updateState(receivedRequest); // all is well
            } else {
               // Insertion fail. This should be impossible. 
               ERR("Impossible error, addNrpRequest failed for %x",NAV_SERVER_POLL_SERVER_REQ);
               // HM. If we get here the elem or list is corrupt,
               // so don't try to deallocate the element. 
               // Memory leakage is the least of our problems here.
               delete nrpData;
               elem->data = NULL;
               elem->nrpData = NULL;
               elem = NULL;
               ERR("sendError(NSC_BEYOND_SALVATION)");
               sendError(NSC_BEYOND_SALVATION);
            }
         }
      }
      return elem;
   }

   void NavServerCom::decodeMessageReply(Buffer& buf, uint32 src)
   {
      int length = buf.readNextUnaligned32bit();
      char* data = new char[length];
      buf.readNextByteArray((uint8*)data, length);
      m_nscConsumer->messageReply(length, data, src);
      delete[] data;
   }

   void NavServerCom::decodeWhereAmIReply(Buffer& buf, uint32 src)
   {
      const char* country    = buf.getNextCharString();
      const char* municipal  = buf.getNextCharString();
      const char* city       = buf.getNextCharString();
      const char* district   = buf.getNextCharString();
      const char* streetname = buf.getNextCharString();
      int32 lat = buf.readNextUnaligned32bit();
      int32 lon = buf.readNextUnaligned32bit();
      DBG("country: %s, municicpal: %s, city: %s, district: %s, "
            "lat: %"PRId32", lon %"PRId32, country, municipal, city,
          district, lat, lon);
      m_nscConsumer->whereAmIReply(country, municipal, city, district, 
                                   streetname, lat, lon, src);
   }

   /** Huh! shouldn't ever be needed. */
   void NavServerCom::decodeGpsInitReply(Buffer& buf, uint32 src)
   {
      src = src;       //currently unused
      buf.getLength(); //currently unused
      ERR("decodeGpsInitReply -- XXX -- %s:%d", __FILE__, __LINE__);
   }

   void NavServerCom::decodeSearchReply(Buffer& buffer, uint32 src)
   {
      DBG("Sending search reply data to 0x%"PRIx32, src);
      DBGDUMP("Search Reply Data", buffer.accessRawData(), buffer.remaining());
      m_nscConsumer->searchReply(buffer.accessRawData(), buffer.remaining(), 
                                 src);
   }

   void NavServerCom::sendError( const ErrorObj& err, uint32 src ) const {
      if ( src == 0 ) {
         unsolicitedError( err, MsgBufferEnums::ADDR_DEFAULT );
      } else {
         if ( m_tracking ) {
             m_tracking->solicitedError( err, src );
         }
         m_nscConsumer->solicitedError( err, src );
      }
   }

   bool NavServerCom::validCoords(int32 lat, int32 lon, uint32 src) const
   {
      if(onTheWorld(lat, lon)){
         DBG("lat: %"PRId32" lon %"PRId32" are valid coords.", lat, lon);
         return true;
      }
      if(!((lat == MAX_INT32) && (lon == MAX_INT32))){
         DBG("lat: %"PRId32" lon %"PRId32" are NOT valid coords.", lat, lon);
         ERR("sendError(NSC_OUT_OF_THIS_WORLD, %#"PRIx32")", src);
         sendError(NSC_OUT_OF_THIS_WORLD, src);
         return false;
      }
      if(!onTheWorld(m_lastLat, m_lastLon)){
         DBG("GPS coordinates %"PRId32",%"PRId32" are not OK",
             m_lastLat, m_lastLon);
         ERR("sendError(NSC_NO_GPS, %#"PRIx32")", src);
         sendError(NSC_NO_GPS, src);
         return false;
      }
      DBG("Coordinates are ok!");
      return true;
   }

   void NavServerCom::decodedWhereAmI(int32 lat, int32 lon, uint32 src)
   {
      DBG("decodedWhereAmI(%"PRId32", %"PRId32", %#"PRIx32")", lat, lon, src);
      if(validCoords(lat, lon, src)){
         Buffer *data = new Buffer(8);
         data->writeNextUnaligned32bit(lat);
         data->writeNextUnaligned32bit(lon);
         bool isSet = false;
         uint32 lang = m_paramBlock->getLanguageOfClient(&isSet);
         if(!isSet){
            lang = ENGLISH;
         }
         data->writeNextUnaligned32bit(lang);
         data->writeNext16bit(0);
         simpleRequest(NAV_SERVER_GPS_ADDRESS_REQ, data, 
                       RefreshLocation::NewIf(lat, lon, 0, 4), src);
      }
   }

   void NavServerCom::decodedSearchRequest(const uint8* data, int length,
                                           int32 lat, int32 lon, uint8 hdg,
                                           uint32 country, const char* city,
                                           uint32 src, 
                                           int latOffset, int lonOffset,
                                           int hdgOffset)
   {
      DBG("decodedSearchRequest(lat: %"PRId32", lon: %"PRId32", "
          "hdg: %"PRIu8", cntr: %0#"PRIx32", city: %s, src: %0#"PRIx32")",
          lat, lon, hdg, country, city, src);
      if(((country != MAX_UINT32)
/*                && (strlen(city) > 0) */
               ) ||
         validCoords(lat, lon, src)){
         Buffer* search = new Buffer(length);
         search->writeNextByteArray(data, length);
         simpleRequest(NAV_SERVER_SEARCH_REQ, search, 
                       new RefreshLocation(latOffset, lonOffset, hdgOffset), 
                       src);
      }//error handling elsewere

   }

   void NavServerCom::decodedSearchInfoRequest(int len, const uint8* data,
                                               uint32 src)
   {
      DBG("decodedSearchInfoRequest(%d, %p, %#"PRIx32")", len, data, src);
      Buffer* request = new Buffer(len);
      request->writeNextByteArray(data, len);
      simpleRequest(NAV_SERVER_INFO_REQ, request, src);
   }
   
   void NavServerCom::decodedVerifyLicenseKey(const char* key, 
                                              const char* phone,
                                              const char* name,
                                              const char* email,
                                              const char* optional,
                                              uint32 region, uint32 src)
   {
      DBG("decodedVerifyLicenseKey(%s, %s, %#"PRIx32")", key, phone, region);
      //set the DisableNoRegionCheck parameter
      m_paramBlock->setDisableNoRegionCheck(true);
      //encode message
      Buffer* request = new Buffer(40);
      request->writeNextUnaligned32bit(region);
      request->writeNextCharString(key);
      request->writeNextCharString(phone);
      request->writeNextCharString(name);
      request->writeNextCharString(email);
      request->writeNextCharString(optional);
      //enqueue message 
      simpleRequest(NAV_SERVER_UPGRADE_REQ, request, src);
   }

   void NavServerCom::decodedMessageRequest(const uint8* data, unsigned length,
                                            uint32 src)
   {
      DBG("decodedMessageRequest(%p, %u, %#"PRIx32")", data, length, src);
      Buffer* messReq = new Buffer(length);
      messReq->writeNextByteArray(data, length);
      simpleRequest(NAV_SERVER_MESSAGE_REQ, messReq, src);
   }


   void NavServerCom::decodedRouteToGps(int32 toLat, int32 toLon, 
                                        int32 fromLat, int32 fromLon, 
                                        uint16 heading, 
                                        uint32 routeRequestorId,
                                        int64 oldRouteId, uint8 rerouteReason,
                                        uint32 src)
   {
      DBG("decodedRouteToGps(%"PRId32", %"PRId32", %"PRId32", %"PRId32", "
          "%"PRIu16", %"PRIu8", %0#"PRIx32")", 
          toLat, toLon, fromLat, fromLon, heading, rerouteReason, src);
      m_lastRouteAddress = routeRequestorId;
      if(onTheWorld(toLat, toLon) || // target coords ok
         (toLat == MAX_INT32 && toLon == MAX_INT32)){ // route to hotdest
         if(validCoords(fromLat, fromLon, src)){ //send error if bad coords
            Buffer *buf = new Buffer(32);
            int latOffset = buf->getWritePos();
            buf->writeNextUnaligned32bit(fromLat);
            int lonOffset = buf->getWritePos();
            buf->writeNextUnaligned32bit(fromLon);
            buf->writeNextUnaligned32bit(toLat);
            buf->writeNextUnaligned32bit(toLon);
            buf->writeNextUnaligned32bit(MAX_UINT32); //time to trunc
            int hdgOffset = buf->getWritePos() + 1;
            buf->writeNextUnaligned16bit(heading);
            buf->writeNextUnaligned16bit(m_lastSpeed); //currentspeed
            buf->writeNext8bit(0xff & full); //content
            buf->writeNext8bit(0xff & m_transportationType);
            buf->writeNext8bit( (0x3f & m_routeType)
                     | (m_routeTollRoads ? 0x80 : 0)
                     | (m_routeHighways  ? 0x40 : 0) );
            buf->writeNextUnaligned64bit(oldRouteId);
            buf->writeNext8bit(rerouteReason);
            RequestListItem* req = 
               simpleRequest(NAV_SERVER_ROUTE_REQ, buf, 
                             RefreshLocation::NewIf(fromLat, fromLon, 
                                                    latOffset, lonOffset, 
                                                    hdgOffset), 
                             src);
            if(req){ 
               req->status |= NSC_REQ_STATUS_ORIG_FROM_GPS;
            }
         }
      } else {
         //target coords bad
         ERR("sendError(NSC_OUT_OF_THIS_WORLD, %#"PRIx32")", src);
         sendError(NSC_OUT_OF_THIS_WORLD, src);
      }
   }
   
   void NavServerCom::decodedMapRequest(int len, const uint8* data, 
                                        uint32 src, int latOffset, 
                                        int lonOffset, int hdgOffset, 
                                        int speedOffset)
   {
      DBG("decodedMapRequest(%d, %p, %0#10"PRIx32")", len, data, src);
      Buffer *mapRequest = new Buffer(len);
      mapRequest->writeNextByteArray(data, len);
      int32 lat, lon = lat = MAX_INT32;
      uint8 hdg = 0;
      uint16 spd = 0;
      if(latOffset > 0){
         mapRequest->setReadPos(latOffset);
         lat = mapRequest->readNextUnaligned32bit();
         if(lat != MAX_INT32){
            latOffset = -1;
         }
      } 
      if(lonOffset > 0){
         mapRequest->setReadPos(lonOffset);
         lon = mapRequest->readNextUnaligned32bit();
         if(lon != MAX_INT32){
            lonOffset = -1;
         }
      }
      if(hdgOffset > 0){
         mapRequest->setReadPos(hdgOffset);
         hdg = mapRequest->readNext8bit();
         if(hdg != MAX_UINT8){
            hdgOffset = -1;
         }
      }
      if(speedOffset > 0){
         //speed refresh is not handled by the general refresh mechanism
         mapRequest->setReadPos(speedOffset);
         spd = mapRequest->readNextUnaligned16bit();
         if(spd == MAX_UINT16){
            int pos = mapRequest->setWritePos(speedOffset);
            mapRequest->writeNextUnaligned16bit(m_lastSpeed);
            mapRequest->setWritePos(pos);
         }
      }
      mapRequest->setReadPos(0);
      bool refresh = (latOffset + lonOffset + hdgOffset) > -3;
      if(refresh && !onTheWorld(m_lastLat, m_lastLon)){
         ERR("sendError(NSC_NO_GPS, %#"PRIx32")", src);
         sendError(NSC_NO_GPS, src);
         delete mapRequest;
      } else {         
         simpleRequest(NAV_SERVER_MAP_REQ, mapRequest,
                       RefreshLocation::NewIf(refresh, latOffset, lonOffset, 
                                              hdgOffset), 
                       src);
      }
   }

   void NavServerCom::decodedVectorMapRequest(int len, const uint8* data, 
                                              uint32 src)
   {
      DBG("decodedVectorMapRequest(%d, %p, %0#10"PRIx32")", len, data, src);
      Buffer *mapRequest = new Buffer(len);
      mapRequest->writeNextByteArray(data, len);
      simpleRequest(NAV_SERVER_VECTOR_MAP_REQ, mapRequest, src);
   }
   void NavServerCom::decodedMultiVectorMapRequest(int len, const uint8* data, 
                                                   uint32 src)
   {
      DBG("decodedMultiVectorMapRequest(%d, %p, %0#10"PRIx32")",
          len, data, src);
      Buffer *mapRequest = new Buffer(len);
      mapRequest->writeNextByteArray(data, len);
      simpleRequest(NAV_SERVER_MULTI_VECTOR_MAP_REQ, mapRequest, src);
   }

   void NavServerCom::decodedCellReport(const uint8* data, uint32 length, 
                                        uint32 src)
   {
      DBG("decodedCellReport(%p, %"PRIu32", %0#10"PRIx32")", data,length, src);
      Buffer *cellReport = new Buffer(length);
      cellReport->writeNextByteArray(data, length);
      simpleRequest(NAV_SERVER_CELL_REPORT, cellReport, src);
   }

   void NavServerCom::decodedBinaryUpload(int len, const uint8 * data, 
                                          uint32 src)
   {
      DBG("decodedBinaryUpload(%d, %p, %0#10"PRIx32")",len, data, src);
      //DBGDUMP("Binary upload data:", data, len);
      Buffer *uploadData = new Buffer(len);
      uploadData->writeNextByteArray(data, len);
      simpleRequest(NAV_SERVER_BINARY_UPLOAD_REQ, uploadData, src);
   }

   void NavServerCom::keepRoute(uint8 req_id, int64 routeId, uint32 ptui)
   {
      DBG("keepRoute(%#"PRIx8", %#"PRIx32"%08"PRIx32", %"PRIu32,
          req_id, HIGH(routeId), LOW(routeId), ptui);
      setServerPtui(ptui);
      class RequestListItem* req = m_pendingList.dequeue(uint16(req_id));
      uint32 dst = m_navTaskProvider->addrOrDefault(req ? req->src : 0);
      DBG("keepRoute sent to %#"PRIx32, dst);
      m_navTaskProvider->keepRoute(routeId, dst);
                   
      routeReply(req_id);  
      delete req->data;
      req->data = NULL;
      delete req->nrpData;
      req->nrpData = NULL;
      m_requestList.deallocate(req);
      updateState(receivedServerReply);
   }

   void NavServerCom::newRoute(uint8 req_id, int packlen, uint32 ptui)
   {
      setServerPtui(ptui);
      class RequestListItem* req = m_pendingList.dequeue(uint16(req_id));
      if(req != NULL){ 
         class Buffer* reqData;
         reqData = static_cast<ServerRequestData*>(req->data)->getBuffer();
         reqData->setReadPos(0);
         int32 fromLat = reqData->readNextUnaligned32bit();
         int32 fromLon = reqData->readNextUnaligned32bit();
         int32 toLat   = reqData->readNextUnaligned32bit();
         int32 toLon   = reqData->readNextUnaligned32bit();
         bool routeOrigFromGPS = 
            (0 != (req->status & NSC_REQ_STATUS_ORIG_FROM_GPS));
         DBG("Sending newRoute: from (lat,lon)=(%"PRId32",%"PRId32"), "
             "to (lat,lon)=(%"PRId32",%"PRId32"), len:%d",
             fromLat, fromLon, toLat, toLon, packlen );
         m_navTaskProvider->newRoute(routeOrigFromGPS, fromLat, fromLon, toLat,
                                     toLon, packlen, 
                                     m_navTaskProvider->addrOrDefault(req->src));
         m_pendingList.enqueue(req);
      } else {
         //XXX the route request has been canceled.
      }
   }
   
   void NavServerCom::setServerPtui(uint32 ptui)
   {
      if(ptui != MAX_UINT32){
         int32 tmp = ptui;
         DBG("Setting server PTUI to %"PRId32"(%#"PRIx32")", tmp, uint32(tmp));
         m_paramProvider->setParam(ParameterEnums::NT_ServerTrafficUpdatePeriod, 
                                   &tmp);
      }
   }

   void NavServerCom::sendChunk(uint8 req_id, Buffer* chunk, bool last)
   {
      class RequestListItem* req = m_pendingList.dequeue(uint16(req_id));
      uint32 dst = m_navTaskProvider->addrOrDefault(req ? req->src : 0);

      m_navTaskProvider->routeChunk(false, chunk->getLength(), 
                                    chunk->accessRawData(0), dst);
      if(last){
         routeReply(req_id);
         updateState(receivedServerReply);
      }
   }

   bool NavServerCom::allNecessaryParamsSet()
   {
      DBG("allNecessaryParamsSet, m_paramBlock: %p", m_paramBlock);
      bool ret = true;
      int len = 0;
      if(m_paramBlock == NULL){
         ERR("Damn! Where did that ParameterBlock run off to?");
         m_paramBlock = new ParameterBlock(); //this should never ever happen!
      }
      ret = ret && !m_servers->empty();
      if(ret) m_paramBlock->getUserLogin(&ret);
      if(ret) m_paramBlock->getUserPassword(&ret);
      ret = ret && ((m_longConnectionTimeout != uint32(-1)) && 
                    (m_shortConnectionTimeout != uint32(-1)) && 
                    (m_disconnectStrategy != invalidStrategy));
#if defined(USE_IMEI_NUMBER_AS_ID) && defined(__SYMBIAN32__) && !defined(_WIN32)
      //only when we can get the IMEI number.
      if(ret){
         m_paramBlock->getUserLicense( len, &ret );
      }
#endif
      // Check if gotten last_imei here
      if ( ret ) m_paramBlock->getOldUserLicense( len, &ret );
      if ( ret ) m_paramBlock->getServerAuthBobChecksum( &ret );

      DBG( "allNecessaryParamsSet ret %d", ret );
      return ret;
   }


   uint8* NavServerCom::decryptIMEI( const uint8* key, int keyLen ) const {
      uint8* res = new uint8[ keyLen + 1 ];
      uint32 xorPos = 0;
      for ( int i = 0 ; i < keyLen ; ++i ) {
         res[ i ] = key[ i ] ^ m_imeiXor[ xorPos ];
         xorPos++;
         if ( xorPos >= m_imeiXorLen ) {
            xorPos = 0;
         }
      }
      res[ keyLen ] = '\0'; // Easier to debug
      return res;
   }


   uint8* NavServerCom::encryptIMEI( const uint8* key, int keyLen ) const {
      // XOR is simple
      return decryptIMEI( key, keyLen );
   }


   void NavServerCom::decodedParamNoValue(uint32 uParam, uint32 /*src*/,uint32 /*dst*/)
   {
      ParameterEnums::ParamIds param = ParameterEnums::ParamIds(uParam);
      enum paramType { noParam, intParam, charParam } type = intParam;
      const char* charval[4] = {NULL}; //inc if necessary
      int32 intval[4] = {0};
      int num = 0;
      WARN("Received a response for an empty parameter %#x", param);
      switch(param){
      case ParameterEnums::NSC_ServerHostname:
         type = charParam;
         charval[num++] = "oss-nav.services.wayfinder.com:80";
         charval[num++] = "oss-nav.services.wayfinder.com:80";
         {for(int i = 0; i < num; ++i){
            WARN("Server hostname set to default value: %s", charval[i]);
         }}
         break;
      case ParameterEnums::NSC_HttpServerHostname :
         type = charParam;
         charval[ num++ ] = "oss-nav.services.wayfinder.com:80";
         charval[ num++ ] = "oss-nav.services.wayfinder.com:80";
         charval[ num++ ] = "oss-nav.services.wayfinder.com:80";
         {for( int i = 0 ; i < num; ++i ) {
            WARN( "HttpServer hostname set to default value: %s", 
                  charval[ i ] );
         }}
         break;
#ifdef USE_OLD_PARAMS
      case ParameterEnums::NSC_NavigatorID:
         intval[num++] = 0x0BEEF;
         WARN("Navigator id set to default value: %#010x", intval[0]);
         break; 
#endif
      case ParameterEnums::NSC_UserAndPasswd:         
         //#define NO_DEFAULT_USER
#ifndef NO_DEFAULT_USER
# define DEFAULT_USER_PASSWD ""
         type = charParam;
#else
# define DEFAULT_USER_PASSWD ""
         ERR("FATAL: No username or password!");
         sendError(NSC_NO_USERNAME, 0);
         type = noParam;
#endif
         charval[0] = charval[1] = DEFAULT_USER_PASSWD;
         num = 2;
         WARN("Username set to default value '%s'", charval[0]);
         WARN("Password set to default value '%s'", charval[1]);
// #ifndef NO_DEFAULT_USER
//          decodedParamValue(param, charval, num, src, dst);
// #endif
         break;
      case ParameterEnums::NSC_CachedConnectionStrategy:
         intval[num++] = DisconnectStrategy((shortTimeout << routeShift)  | 
                                            (longTimeout << searchShift) |
                                            (longTimeout << synchronizeShift));
         intval[num++] = 45 * 1000;
         intval[num++] = 15 * 1000;
         WARN("Disconnection strategy set to %#"PRIx32"", intval[0]);
         WARN("Long connection timeout set to %"PRIu32"", intval[1]);
         WARN("Short connection timeout set to %"PRIu32"",intval[2]); 
         break;
#ifdef USE_OLD_PARAMS
      case ParameterEnums::NSC_AttachCrossingMaps:
         intval[num++] = 0;
         WARN("AttachCrossingMaps set to %s.", intval[0] ? "true": "false");
         break;
      case ParameterEnums::NSC_ImageCrossingMaps:
         intval[num++] = 1;
         WARN("ImageCrossingMaps set to %s.", intval[0] ? "true": "false");
         break;
      case ParameterEnums::NSC_CrossingMapsImageFormat:
         intval[num++] = 0;
         WARN("CrossingMapsImageFormat set to %"PRId32".",intval[0]);
         break;
      case ParameterEnums::NSC_CrossingMapsSize:
         intval[num++] = (80 << 16) | 80;
         WARN("CrossingMapsSize set to %"PRId32" by %"PRId32
                 " pixels: %#010"PRIx32, intval[0] >> 16, 
                 intval[0] & 0x0ffff, intval[0]);
         break;
      case ParameterEnums::NSC_MaxSearchMatches:
         intval[num++] = 12;
         WARN("MaxSearchMatches set to %"PRId32".",intval[0]);
         break;
#endif
      case ParameterEnums::TopRegionListLegacy:
/*          m_paramBlock->setTopRegionChecksum(MAX_UINT32); */
         WARN("Top Region Legacy unset.");
         type = noParam; //dont write to parammodule.
         break;
      case ParameterEnums::TopRegionList:
         m_paramBlock->setTopRegionChecksum(MAX_UINT32);
         WARN("Top Region Checksum value set to 0x%"PRIx32, intval[0]);
         type = noParam; //dont write to parammodule.
         break;
      case ParameterEnums::NSC_TransportationType:
         intval[num++] = passengerCar;
         WARN("Transportation type set to passenger car.");
         break;
      case ParameterEnums::NSC_RouteCostType:
         intval[num++] = NavServerComEnums::TIME_WITH_DISTURBANCES;
         WARN("Route cost set to TIME:");
         break;
      case ParameterEnums::NSC_RouteTollRoads:
         intval[num++] = NavServerComEnums::TollRoadsAllow;
         WARN("Route toll roads set to Allow");
         break;
      case ParameterEnums::NSC_RouteHighways:
         intval[num++] = NavServerComEnums::HighwaysAllow;
         WARN("Route highways set to Allow");
         break;
      case ParameterEnums::NSC_CategoriesChecksum:
      case ParameterEnums::NSC_LatestNewsChecksum:
      case ParameterEnums::NSC_CallCenterChecksum:
      case ParameterEnums::NSC_serverAuthBobChecksum:
         intval[num++] = 0;
         WARN("Parameter %#x set to 0", param);
         break;
      case ParameterEnums::NSC_ExpireVector:
         DBG("Expire vector set to 3 x MAX_INT32");
         intval[num++] = MAX_INT32;
         intval[num++] = MAX_INT32;
         intval[num++] = MAX_INT32;
         break;
      case ParameterEnums::NSC_LastIMEI : {
         DBG( "No last IMEI" );
         m_paramBlock->setOldUserLicense( (uint8*)"", 1 );
         // Set it to current IMEI!
         bool isSet = false;
         int len = 0;
         const uint8* bparam = m_paramBlock->getUserLicense( len, &isSet );
         if ( isSet ) { // Should always be set
            uint8* lastImei = encryptIMEI( bparam, len );
            m_paramProvider->setParam( 
               ParameterEnums::NSC_LastIMEI,
               (const uint8*)lastImei, len );
            delete [] lastImei;
         }
         // Have set it not int/char
         type = noParam;
         } break;
      case ParameterEnums::SelectedAccessPointId2 : {
         // Ok, that is all right with me that this is empty
         type = noParam;
      } break;

      default:
         if ( m_tracking && m_tracking->isdecodedParamNoValueOk( param ) ) {
            m_tracking->decodedParamNoValue( param );
         } else {
            WARN( "Unknown parameter %#x", param );
         }
         type = noParam;
      }
      switch(type){
      case intParam:
         m_paramProvider->setParam(param, intval, num);
         break;
      case charParam:
         m_paramProvider->setParam(param, charval, num);
         break;
      default: 
         break;
      }
   }

   void NavServerCom::decodedParamValue(uint32 param, const int32* data, 
                                        int32 numEntries, uint32 src, 
                                        uint32 dst)
   {
      typedef ParameterBlock PB;
      switch(param){
      case ParameterEnums::NSC_NavigatorID:
         DBG("Param received : navigator id: %#"PRIx32"", *data);
         m_paramBlock->setNavID(*data);
         break;
      case ParameterEnums::NSC_CachedConnectionStrategy:
         if(numEntries != 3){
            ERR("Param CachedConnectionStrategy: received %"PRId32" entries. "
                "Should be 3", numEntries);
            //set to default, easliy done through decodedParamNoValue
            decodedParamNoValue(param, src, dst);
         } else {
            DBG("Param received: CachedConnectionStrategy "
                "%#0"PRIx32" %"PRId32" %"PRId32,
                data[0], data[1], data[2]);
            m_shortConnectionTimeout = data[2];
            m_longConnectionTimeout = data[1];
            m_disconnectStrategy = DisconnectStrategy(data[0]);
         }
         break;
      case ParameterEnums::NSC_AttachCrossingMaps:
         m_paramBlock->setAttachCrossingMaps(*data != 0);
         break;
      case ParameterEnums::NSC_ImageCrossingMaps:
         m_paramBlock->setImageCrossingMaps(*data != 0);
         break;
      case ParameterEnums::NSC_CrossingMapsImageFormat:
         m_paramBlock->setCrossingMapsImageFormat(*data & 0x0ff);
         break;
      case ParameterEnums::NSC_CrossingMapsSize:
         m_paramBlock->setCrossingMapsImageSize(*data); 
         break;
      case ParameterEnums::UC_MaxNumberOfGuiSearchResults:
         m_paramBlock->setMaxSearchMatches(*data & 0x0ff);
         break;
      case ParameterEnums::Language:
         DBG("Received Language(%#"PRIx32"): %"PRId32, param, *data);
         m_paramBlock->setLanguageOfClient(*data);
         break;
      case ParameterEnums::NSC_TransportationType:
         m_transportationType = *data;
         break;
      case ParameterEnums::NSC_RouteCostType:
         m_routeType = *data;
         break;
      case ParameterEnums::NSC_RouteTollRoads:
         m_routeTollRoads = *data;
         break;
      case ParameterEnums::NSC_RouteHighways:
         m_routeHighways = *data;
         break;
      case ParameterEnums::NSC_ExpireVector:
         m_expireVector.resize(numEntries);
         std::copy(data, data + numEntries, m_expireVector.begin());
         break;
      case ParameterEnums::NSC_CategoriesChecksum:
         m_paramBlock->setCategories(*data);
         break;
      case ParameterEnums::NSC_LatestNewsChecksum:
         m_paramBlock->setLatestNews(*data);
         break;
      case ParameterEnums::NSC_MaxSearchMatches:
         WARN("NSC_MaxSearchMatches are no longer used.");
         return;
      case ParameterEnums::NSC_CallCenterChecksum:
         DBG("NSC_CallCenterChecksum: %#"PRIx32, *data);
         m_paramBlock->setCallCenterChecksum(*data);
         break;
      case ParameterEnums::NSC_ServerListChecksum:
      case ParameterEnums::NSC_HttpServerListChecksum:
         //case m_serverListChecksumParam:
         m_paramBlock->setServerListChecksum(*data);
         break;
      case ParameterEnums::WayfinderType:
         DBG("WayfinderType: %s%s%s%s", 
             *data == GuiProtEnums::InvalidWayfinderType ? "INVALID" : "",
             *data == GuiProtEnums::Trial ? "TRIAL" : "",
             *data == GuiProtEnums::Silver ? "SILVER" : "",
             *data == GuiProtEnums::Gold ? "GOLD" : "");
         m_paramBlock->setWayfinderType(*data);
         break;
      case ParameterEnums::NSC_serverAuthBobChecksum:
         m_paramBlock->setServerAuthBobChecksum( *data );
         break;
      case ParameterEnums::SelectedAccessPointId2 : {
         // Nothing I use in this int
      } break;

      default:
         if ( m_tracking && m_tracking->isdecodedParamValueOk( param ) ) {
            m_tracking->decodedParamValue( param, data, numEntries );
         } else {
            WARN("Received unknown int parameter %#"PRIx32, param);
         }
         return;
      }
      updateState(parameterChange);
   }

   void NavServerCom::decodedParamValue(uint32 param, const float* /*data*/, 
                          int32 /*numEntries*/, uint32 /*src*/, uint32 /*dst*/)
   {
//      switch(param){
//      default:
         WARN("Received unknown float parameter %#"PRIx32, param);
         return;
//      }
//      updateState(parameterChange);
   }
   
   void NavServerCom::decodedParamValue(uint32 param, const char*const* data, 
                          int32 numEntries, uint32 /*src*/, uint32 /*dst*/)
   {
      switch(param){
      case ParameterEnums::NSC_ServerHostname:
      case ParameterEnums::NSC_HttpServerHostname:
         //case m_serverListParam:
         {
            DBG("Param received server name %s", *data);

         std::vector<Server*>::iterator q;
         std::vector<Server*>* temp = new std::vector<Server*>;
         temp->reserve(m_servers->size());
         for(q = m_servers->begin(); q != m_servers->end(); ++q){
            //keep nonpersistent servers.
            if(*q != NULL && !(*q)->persistent){
               temp->push_back(*q);
               (*q)->setGroup(0); //shouldn't be necessary
            } else {
               delete *q;
            }
         }
         m_servers->clear();
         m_servers->reserve(temp->size() + numEntries);
         while(!temp->empty()){
            m_servers->push_back(temp->back());
            temp->pop_back();
         }
         delete temp;
         int lastGroup = 1;
         if(!m_servers->empty()){
            setReachable(*m_servers);
            m_serverIdx = findFirstServer(*m_servers, m_seed);
            lastGroup = max(m_servers->back()->group(), lastGroup);
         }
         for(int i = 0; i < numEntries; ++i){
            Server* server = new Server(data[i]);
            if(server->group() == -1){
               server->setGroup(lastGroup);
            }
            m_servers->push_back(server);
         }
         m_serverIdx = findFirstServer(*m_servers, m_seed);
            for(unsigned j = 0; j < m_servers->size(); ++j){
               DBG("server %u(%d): %s", j, (*m_servers)[j]->group(),
                   (*m_servers)[j]->getHostAndPort());
            } 
            DBG("Use server '%s'",(*m_servers)[m_serverIdx]->getHostAndPort());
         }
         break;
      case ParameterEnums::SelectedAccessPointId2 : {
         // Ok, check for proxy setting
         // XXX:
         // if proxy them use it as server and set server as proxy-server
         // in serverparser
         // A m_proxyserver (host:port string) member? (so we keep servers
         // even if set twise) and the [XXX]Server methods should check for
         // m_proxyserver.
         // possibly a Server object too.
         // If not proxy setting unset in serverparser.
         DBG( "Got SelectedAccessPointId2 %s", data[0] );
         delete [] m_proxyServerAndPort;
         m_proxyServerAndPort = NULL;
         char* cpos = strchr( data[0], ',' );
         if ( cpos != NULL ) {
            // Have comma
            m_proxyServerAndPort = strdup_new( cpos + 1 );
         }
      } break;
      case ParameterEnums::NSC_UserAndPasswd:
         DBG("Param received username: %s and passwd: %s", data[0], data[1]);
         m_paramBlock->setUserLogin(data[0]);
         m_paramBlock->setUserPassword(data[1]);
         break;
      case ParameterEnums::UC_WebPasswd:
         DBG("Param received web passwd: '%s'", *data);
         {
            bool isSet;
            const char* pass = m_paramBlock->getUserPassword(&isSet);
            if(isSet && !strequ(*data, pass)){
               //only if the new password differs from the old.
               m_paramBlock->setNewPasswordSet(*data);
            }
         }
         break;
      default:
         WARN( "Received unknown string parameter %#"PRIx32, param );
         return;
      }
      updateState(parameterChange);
      if(param == ParameterEnums::UC_WebPasswd){
         bool isSet;
         if(m_paramBlock->getNewPasswordSet(&isSet), isSet){
            decodedParamSync(0);
         }
      }         
   }

   void NavServerCom::decodedParamValue(uint32 paramId, const uint8* data, 
                                        int size, uint32 /*src*/, uint32 /*dst*/)
   {
      switch(paramId){
      case ParameterEnums::TopRegionListLegacy:
         {
            DBG("Param received - Legacy (non utf-8) TopRegionList");
            Buffer buf(size);
            buf.writeNextByteArray(data, size);

#if defined (NAV2_USE_UTF8)
            TopRegionList regions(buf, 1); /* Convert! */
            m_paramBlock->setTopRegionChecksum(regions.getVersion());
            /* Move legacy regions to UTF-8 TopRegionList. */
            int new_size = regions.serialize(buf);

            /* Erase legacy param. */
            m_paramProvider->clearParam(ParameterEnums::TopRegionListLegacy);
            m_paramProvider->setParam(ParameterEnums::TopRegionList,
                  buf.accessRawData(),
                  new_size);
#else
            TopRegionList regions(buf);
            m_paramBlock->setTopRegionChecksum(regions.getVersion());
#endif
         }
         break;
      case ParameterEnums::TopRegionList:
         {
#if defined (NAV2_USE_UTF8)
            DBG("Param received TopRegionList");
            Buffer buf(size);
            buf.writeNextByteArray(data, size);
            TopRegionList regions(buf);
            m_paramBlock->setTopRegionChecksum(regions.getVersion());
#else
            /* Downgrading not supported. */
            m_paramProvider->clearParam(ParameterEnums::TopRegionList);
#endif
         }
         break;
      case ParameterEnums::NSC_LastIMEI : {
         // Store for later check.
         // Unectrypt IMEI
         uint8* lastImei = decryptIMEI( data, size );
         DBG( "Param received last IMEI: '%s'", lastImei );
         m_paramBlock->setOldUserLicense( lastImei, size );
         delete [] lastImei;
         } break;
      case ParameterEnums::NSC_serverAuthBob : {
         // Set in paramblock. Send with auth if secondary server
         // See case sendingParams: in stateChangeAction
         m_paramBlock->setServerAuthBob( data, size );
      } break;
      default:
         if ( m_tracking && m_tracking->isdecodedParamValueOk( paramId ) ) 
         {
            m_tracking->decodedParamValue( paramId, data, size );
         } else {
            WARN( "Received unknown Binary Block parameter %#"PRIx32, paramId );
         }
         return;
      }
      updateState(parameterChange);
   }
   
   void NavServerCom::reportProgress(ComStatus cs, MessageType type,
                                     uint32 sent, uint32 toSend)
   {
      DBG("reportProgress(%d, %#x, %"PRIu32", %"PRIu32")",
          cs, type, sent, toSend);
      if(m_csLookUp[cs]){
         if (m_sessionState == timedOut && (cs == downloadingData)){
            return;
         }
         if(m_lastCs != cs || m_lastType != type || 
            m_lastSent != sent || m_lastToSend != toSend){
            GuiProtEnums::ServerActionType actionType = 
               GuiProtEnums::InvalidActionType;
            switch(type){
            case NAV_SERVER_INVALID:
            case NAV_SERVER_PICK_ME_UP_REQ:
            case NAV_SERVER_PICK_ME_UP_REPLY:
            case NAV_SERVER_PICK_UP_REQ:
            case NAV_SERVER_PICK_UP_REPLY:
            case NAV_SERVER_GPS_ADDRESS_REQ:
            case NAV_SERVER_GPS_ADDRESS_REPLY:
            case NAV_SERVER_GPS_POS_REQ:
            case NAV_SERVER_GPS_POS_REPLY:
            case NAV_SERVER_ALARM_REQ:
            case NAV_SERVER_ALARM_REPLY:
            case NAV_SERVER_MESSAGE_REQ:
            case NAV_SERVER_MESSAGE_REPLY:
            case NAV_SERVER_GPS_INIT_REQ:
            case NAV_SERVER_GPS_INIT_REPLY:
            case NAV_SERVER_POLL_SERVER_REQ:
            case NAV_SERVER_POLL_SERVER_REPLY:
            case NAV_REQUEST :
            case NAV_REPLY :
               break;
            case NAV_SERVER_PARAMETER_REQ:
            case NAV_SERVER_PARAMETER_REPLY:
               actionType = GuiProtEnums::SettingPasswd;
               break;
            case NAV_SERVER_INFO_REQ:
            case NAV_SERVER_INFO_REPLY:
               actionType = GuiProtEnums::RetrievingInfo;
               break;
            case NAV_SERVER_UPGRADE_REQ:
            case NAV_SERVER_UPGRADE_REPLY:
               actionType = GuiProtEnums::Upgrading;
               break;
            case NAV_SERVER_ROUTE_REQ:
            case NAV_SERVER_ROUTE_REPLY:
               actionType = GuiProtEnums::CreatingRoute;
               break;
            case NAV_SERVER_SEARCH_REQ:
            case NAV_SERVER_SEARCH_REPLY:
               actionType = GuiProtEnums::PerformingSearch;
               break;
            case NAV_SERVER_DEST_REQ:
            case NAV_SERVER_DEST_REPLY:
            case NAV_SERVER_BINARY_UPLOAD_REQ:
            case NAV_SERVER_BINARY_UPLOAD_REPLY:
               actionType = GuiProtEnums::Synchronizing;
               break;
            case NAV_SERVER_MAP_REQ:
            case NAV_SERVER_MAP_REPLY:
               actionType = GuiProtEnums::DownloadingMap;
               break;
            case NAV_SERVER_VECTOR_MAP_REQ:
            case NAV_SERVER_VECTOR_MAP_REPLY:
            case NAV_SERVER_MULTI_VECTOR_MAP_REQ:
            case NAV_SERVER_MULTI_VECTOR_MAP_REPLY:
               actionType = GuiProtEnums::VectorMap;
               break;
            case NAV_SERVER_CELL_REPORT:
            case NAV_SERVER_CELL_CONFIRM:
               actionType = GuiProtEnums::CellInfoReport;
               break;
            }
            if(m_nscConsumer){
               m_nscConsumer->progressMessage(cs, actionType, sent, toSend);
            }
         }
         m_lastCs = cs;
         m_lastType = type;
         m_lastSent = sent;
         m_lastToSend = toSend;
      }
   }

   void NavServerCom::decodedCancelRequest(uint32 originalSrc, 
                                           uint32 netmask, uint32 src)
   {
      DBG("decodedCancelRequest(0x%8"PRIx32", 0x%8"PRIx32", 0x%8"PRIx32")", 
            originalSrc, netmask, src);
      if(originalSrc != MAX_UINT32 &&
         (netmask & originalSrc) == (netmask & src)){
         RequestListItem *item = m_requestList.dequeue(originalSrc);
         if(item == NULL){
            item = m_pendingList.dequeue(originalSrc);
         }
         if(item != NULL){
            ERR("sendError(NSC_CANCELED_BY_REQUEST, %#"PRIx32")", item->src);
            sendError(NSC_CANCELED_BY_REQUEST, item->src);
            delete item->data;
            item->data = NULL;
            delete item->nrpData;
            item->nrpData = NULL;
            m_requestList.deallocate(item);
         }
      } else if(originalSrc == MAX_UINT32){
         RequestList* lists[] = {&m_requestList, &m_pendingList};
         for(int i = 0; i < 2; ++i){
            RequestListItem * item = NULL;
            while(NULL != (item = lists[i]->dequeueFirst())){
               ERR("sendError(NSC_CANCELED_BY_REQUEST, %#"PRIx32")",item->src);
               sendError(NSC_CANCELED_BY_REQUEST, item->src);
               delete item->data;
               item->data = NULL;
               delete item->nrpData;
               item->nrpData = NULL;
               m_requestList.deallocate(item);
            }
         }
      }
   }

   void NavServerCom::decodedPositionState(const struct PositionState &p, 
                                           uint32 /*src*/)
   {
      if (GuiProtEnums::Silver != m_paramBlock->getWayfinderType()) {
         m_lastLat = p.lat;
         m_lastLon = p.lon;
         m_lastHeading = p.heading;
         m_lastSpeed = p.speed;
      }
      if ( m_tracking ) m_tracking->decodedPositionState( p );
   }
   
   void NavServerCom::setRouteCoordinate( int32 /*lat*/, int32 /*lon*/, int8 /*dir*/ )
   {
      //XXX Mabe we'll need to do something here...
   }
   
   void NavServerCom::decodedRequestReflash(uint32 /*src*/)
   {
      //XXX Danger Will Robinson! What will a reflash do!
      m_paramBlock->setReflashMe(true);
   }

   void NavServerCom::ServerRequestData::refreshData(int32 lat, int32 lon,
                                                     uint8 heading)
   {
      if(m_refresh){
         int pos = m_requestBuffer->getWritePos();
         if(m_refresh->latOffset >= 0){
            m_requestBuffer->setWritePos(m_refresh->latOffset);
            m_requestBuffer->writeNextUnaligned32bit(lat);
         }
         if(m_refresh->lonOffset >= 0){
            m_requestBuffer->setWritePos(m_refresh->lonOffset);
            m_requestBuffer->writeNextUnaligned32bit(lon);
         }
         if(m_refresh->headingOffset >= 0){
            m_requestBuffer->setWritePos(m_refresh->headingOffset);
            m_requestBuffer->writeNext8bit(heading);
         }
         m_requestBuffer->setWritePos(pos);
      }
   }

   void NavServerCom::decodedParamSync(uint32 src)
   {
      if(src != 0){
         DBG("decodedParamSync(%#"PRIx32")", src);
         m_paramSyncs.push_back(src);
      }
      simpleRequest(NAV_SERVER_PARAMETER_REQ, m_paramBlock, src);
   }

   void NavServerCom::decodedSendRequest( const class MsgBuffer* buf )
   {
      simpleRequest( NAV_REQUEST, new Buffer( *buf ), buf->getSource() ); 
   }

   enum Nav2Error::ErrorNbr NavServerCom::extenedErrorToNav2Error( 
      uint32 extendedError ) const 
   {
      switch ( extendedError ) {
         case 0x17001 :
            return NSC_NO_ROUTE_RIGHTS;
         default:
            return NSC_SERVER_NOT_OK;
      }
   }
   
   void NavServerCom::forceDisconnect()
   {
      // Send the requestDisconnect event.
      // Requested to disconnect. Can typically happen if getting "Connection: Close"
      // and we want to ensure that we do get disconnected.
      
      updateState(requestDisconnect);
   }
}

