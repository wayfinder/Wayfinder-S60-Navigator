/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* The meat of the navigation system. This is the algorithm that 
 * actually follows the position of the car.
 */

#define ALLOW_UNSNAPPED_COORDINATES

#include "arch.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Buffer.h"
#include "CtrlHub.h"

#include "Nav2DistanceUtil.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "RouteEnums.h"
#include "NavTask.h"

#include "NavPacket.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "GuiProt/ServerEnums.h"
#include "NavServerCom.h"

#include "ParameterEnums.h"
#include "Parameter.h"
#include "../NavTask/Point.h"
#include "../NavTask/Route.h"

#include "Gps.h"
#include "NavTaskInternal.h"

#include "FastTrig.h"
#include "RouteInfo.h"
#include "Constants.h"
#include "Penalties.h"

#include "GuiCommandHandler.h"

#include "TimeLoggingUtil.h"

#include "nav2util.h"

#include "LogMacros.h"

#include "HintNodeAlgorithms.h"

#define STOP_NAVIGATING_ON_GOAL
#define TESTING_PENALTY_WEIGHTS

#define SIMULATION_DATA_INTERVAL 1000
#define WAIT_FOR_GPS_SIGNAL_INTERVAL (5*1000)
#define FIRST_WAIT_FOR_GPS_SIGNAL (1*1000)
#define NBR_TIMER_RETRIES (50)

// XXX At least one of these needs to be defined
#define USE_OLD_OFFTRACK_DETECTION
#undef USE_NEW_OFFTRACK_DETECTION

//Define this to enable time logging of route requests.
#undef DEBUG_TIME_LOGGING

static const float PDOP_EXCELLENT_TRESH=2;
static const int32 MAX_ALLOWED_PTUI_MINUTES = 1200;

#define INTERPOL_LOG_FILE "c:\\hintnodes.txt"

//#define GPSDBG DBG
#define GPSDBG DROP

//#define RTDBG DBG
#define RTDBG DROP

#define INTERPOLATION_USE_GPS_SPEED
//#define LOG_INTERPOLATION_DATA


namespace isab {

#ifdef TESTING_PENALTY_WEIGHTS
   static const Penalties ontrack_penalty_params = {
         30,        /* angle_penalty           - Penalty in m for 180 degrees wrong direction */
          7,        /* early_in_seg_limit      - 0..7 into the segments we add a penalty      */
          7,        /* early_in_seg_penalty    - Add 7 meters penalty                         */
         20         /* smallest_seg_with_early - unless the segment is shorter than 20 meters */
         };

   static const Penalties offtrack_penalty_params = {
          0,        /* angle_penalty           - Penalty in m for 180 degrees wrong direction */
          7,        /* early_in_seg_limit      - We actually like the early parts of segments when off track */
         -7,        /* early_in_seg_penalty    -                                              */
         10         /* smallest_seg_with_early - Those shorter than 10 meters we dont care about */
         };
#else
   static const Penalties ontrack_penalty_params = {
         15,        /* angle_penalty           - Penalty in m for 180 degrees wrong direction */
          7,        /* early_in_seg_limit      - 0..7 into the segments we add a penalty      */
          7,        /* early_in_seg_penalty    - Add 7 meters penalty                         */
         20         /* smallest_seg_with_early - unless the segment is shorter than 20 meters */
         };

   static const Penalties offtrack_penalty_params = {
          0,        /* angle_penalty           - Penalty in m for 180 degrees wrong direction */
          7,        /* early_in_seg_limit      - We actually like the early parts of segments when off track */
         -7,        /* early_in_seg_penalty    -                                              */
         10         /* smallest_seg_with_early - Those shorter than 10 meters we dont care about */
         };
#endif


   
   NavTask::NavTask(GpsProviderPublic *p) :
      Module("NavTaskthread"), m_gps(p), m_navTaskConsumer(NULL), 
      m_route(new Route(m_log)), m_receiveRoute(new Route(m_log)),
      m_followingRoute(false),
      m_routeValid(false),
      m_simInfoStatus(simulate_off),
      m_simSpeed(2),
      m_routeLastFileName(NULL),
      last_gps_head_valid(false),
      m_lastGpsQuality(QualityMissing), m_awaitingRouteReply(false),
      m_stopNextInternalRoute(false),
      m_autoRerouteFailed(false),
      m_useGPSNavigation(false),
      m_user_ptui(MAX_INT32),
      m_server_ptui(0),
      m_ptuiTimer(0)
   {
      DBG("NavTask::NavTask\n");

#ifdef LOG_INTERPOLATION_DATA
      FILE* fClear = fopen( INTERPOL_LOG_FILE, "w" );
      fclose( fClear );
#endif
      
      m_simTimeoutId    = m_queue->defineTimer();
      m_ptuiTimer       = m_queue->defineTimer();
      m_waitForSignalId = m_queue->defineTimer();

      m_prevLat = MAX_INT32;
      m_prevLon = MAX_INT32;
      
      p->setOwnerModule(this);
      p->connect(new GpsConsumerPublic(m_queue));
      m_last_time = isab::TimeUtils::millis() - 251;

      m_navTaskConsumer = new NavTaskConsumerPublic(this);
      uint32 defaultDest = 
         uint32(Module::NavTaskMessages) | MsgBufferEnums::ADDR_MULTICAST_UP;
      m_navTaskConsumer->setDefaultDestination(defaultDest);

      m_navServerCom = new NavServerComProviderPublic(this);
      m_parameter = new ParameterProviderPublic(this);

      m_guiCommandHandler = new GuiCommandHandler(this);

      // Calc how many times the timer should expire
      // before sending an error to gui.
      m_nbrRetries = 0;

#ifdef DEBUG_TIME_LOGGING
      m_timeLogger = 
         new TimeLoggingUtil("c:\\system\\data\\timelog.txt", true);
      m_timeLogger->AddTimer(1, "TotalRouteReq");
#endif
      newLowerModule(p);
   }


   NavTask::~NavTask()
   {
      delete m_parameter;
      delete m_navServerCom;
      delete m_navTaskConsumer;
      delete m_route;
      delete m_receiveRoute;
      delete m_guiCommandHandler;
#ifdef DEBUG_TIME_LOGGING
      delete m_timeLogger;
#endif
      delete[] m_routeLastFileName;
      m_queue->cancelTimer(m_simTimeoutId);
      m_queue->cancelTimer(m_ptuiTimer);
      m_queue->cancelTimer(m_waitForSignalId);
   }

   class MsgBuffer * NavTask::dispatch(class MsgBuffer *buf)
   {
      //DBG("NavTask::dispatch : %i\n", buf->getMsgType());
      if(buf) buf = m_gpsDecoder.dispatch(buf, this);
      if(buf) buf = m_ctrlHubAttachedDecoder.dispatch(buf, this);
      if(buf) buf = m_navTaskProviderDecoder.dispatch(buf, this);
      if(buf) buf = m_parameterConsumerDecoder.dispatch(buf, this);
      if(buf) buf = m_nscDecoder.dispatch(buf, this);
      if(buf) buf = Module::dispatch(buf);
      return buf;
   }

   class CtrlHubAttachedPublic * NavTask::newPublicCtrlHub()
   {
      DBG("newPublicCtrlHub()\n");
      return new CtrlHubAttachedPublic(m_queue);

   }

   void NavTask::decodedStartupComplete()
   {
      Module::decodedStartupComplete();
      // Parameters we need and will subscribe to.
      ParameterEnums::ParamIds params[] = {
         ParameterEnums::WayfinderType,
         ParameterEnums::R_routeName,
         ParameterEnums::R_lastIndex,
         ParameterEnums::NT_UserTrafficUpdatePeriod,
         ParameterEnums::NT_ServerTrafficUpdatePeriod,
      };
      //sort the list to facilitate joining
      std::sort(params, params + sizeof(params)/sizeof(*params));
      //for loop to get parameters and join parameter multicast groups
      uint16 last = 0;
      for(unsigned j = 0; j < sizeof(params)/sizeof(*params); ++j){
         uint16 group = ParameterProviderPublic::paramIdToMulticastGroup(params[j]);
         m_parameter->getParam(params[j]);
         if(group != last){
            m_rawRootPublic->manageMulticast(JOIN, group);
         }
         last = group;
      }

      m_rawRootPublic->manageMulticast(JOIN, Module::NSCProgressMessages);
   }

   void NavTask::decodedExpiredTimer(uint16 timerid)
   {
      if (timerid == m_simTimeoutId) {
         DBG("Resetting simulator timer and requesting next step.");
         m_queue->setTimer(m_simTimeoutId, SIMULATION_DATA_INTERVAL);
         generateNextSimulatedGpsPosition();
      } else if(timerid == m_ptuiTimer) {
         DBG("PTUI timeout. Reroute for traffic info.");
         if(m_followingRoute && !m_awaitingRouteReply && 
            m_lastPosState.validCoord()){
            DBG("Requesting reroute");
            reRoute(true, NavServerComEnums::traffic_info_update);
         }
         resetPtuiTimer();
      } else if (timerid == m_waitForSignalId) {
         m_nbrRetries++;
         HandleRouteToGpsRequest();
      } else {
         Module::decodedExpiredTimer(timerid);
      }
   }

   void NavTask::decodedConnectionCtrl(enum ConnectionCtrl command, 
                                       const char *method, uint32 /*src*/)
   {
      DBG("Calls connection control of m_gps.");
      m_gps->connectionCtrl(command, method);
   }

   int32 NavTask::getRouteLastUsedIndex()
   {
      //m_parameter->getParam(ParameterEnums::R_lastIndex);
      //Above is not needed since we receive multicasts for variable.
      return m_routeLastUsedIndex;
   }

   char* NavTask::getRouteFileName()
   {
      //m_parameter->getParam(ParameterEnums::R_routeName);
      //Above is not needed since we receive multicasts for variable.
      return m_routeLastFileName;
   }

   void NavTask::setupGpsSimulation()
   {
      m_simInfoStatus |= simulate_on;
      isabTime t; //Broken since long before me!
      //enum GpsStatus sum_status = os_gps_ok_3d;
      //enum GpsStatus sum_status = os_gps_demo_1x;
      m_simSpeed = 2;
      m_simGpsStatus = os_gps_demo_1x;
      uint16 error_conditions = 0;
      float pdop = 1;
      uint32 src = (uint32)MsgBufferEnums::ADDR_DEFAULT;

      m_route->findFirstPoint(m_currentSimGpsPoint);
      m_currentSimGpsPoint.getNextPoint(&m_nextSimGpsPoint);

      //Starting the navigation state machine off track sets it up correctly.
      m_carOnTrack = OffTrack;

      m_distFromPrevWpt = 0;
      m_simulatedSpeed = 0;

      //decodedGpsStatusFunc(t, sum_status, error_conditions, pdop, src);
      decodedGpsStatusFunc(t, (enum GpsStatus)m_simGpsStatus, error_conditions, pdop, src);
      //decodedGpsSatelliteInfoFunc(satInfo, src); //What is satInfo, we dont need it in sim!?
      m_queue->setTimer(m_simTimeoutId, SIMULATION_DATA_INTERVAL);
   }

   void NavTask::stopGpsSimulation()
   {
      m_queue->stopTimer(m_simTimeoutId);
      uint32 src = (uint32)MsgBufferEnums::ADDR_DEFAULT;
      isabTime t; //Broken since long before me!
      m_simSpeed = 2;
      enum GpsStatus sum_status = os_gps_fatal;
      uint16 error_conditions = 0;
      float pdop = 1;
      float latTemp, lonTemp;
      uint8 tmpAngle, tmpSpeed;

      m_route->findFirstPoint(m_currentSimGpsPoint);
      m_currentSimGpsPoint.getCoordWGS84(latTemp, lonTemp);
      m_currentSimGpsPoint.getAngle(tmpAngle);
      tmpSpeed = m_currentSimGpsPoint.getSpeedLimit() * 10 / 36;

      m_currentSimGpsPosition.latitude          = latTemp;
      m_currentSimGpsPosition.longitude         = lonTemp;
      m_currentSimGpsPosition.altitude          = 74.0;
      m_currentSimGpsPosition.heading           = tmpAngle;
      m_currentSimGpsPosition.speed_over_ground = tmpSpeed;
      m_currentSimGpsPosition.vertical_speed    = 0;
      m_currentSimGpsPosition.weekno_for_fix    = 0;
      m_currentSimGpsPosition.tow_for_fix       = 0;
      //m_currentSimGpsPosition.fix_time        = 0; //Broken since long before me!
      m_currentSimGpsPosition.constellation_no  = 0;
      m_currentSimGpsPosition.latency           = (float)0.7;

      m_simInfoStatus &= ~simulate_on;
      m_simInfoStatus &= ~simulate_paused;
      m_simInfoStatus &= ~simulate_max_speed; 
      m_simInfoStatus &= ~simulate_min_speed;

      decodedGpsPositionVelocityFunc(m_currentSimGpsPosition, src);
      decodedGpsStatusFunc(t, sum_status, error_conditions, pdop, src);
   }

   void NavTask::resetGpsSimulation()
   {
      uint32 src = (uint32)MsgBufferEnums::ADDR_DEFAULT;
      float latTemp, lonTemp;
      uint8 tmpAngle, tmpSpeed;

      m_route->findFirstPoint(m_currentSimGpsPoint);
      m_currentSimGpsPoint.getCoordWGS84(latTemp, lonTemp);
      m_currentSimGpsPoint.getAngle(tmpAngle);
      tmpSpeed = m_currentSimGpsPoint.getSpeedLimit() * 10 / 36;
      m_currentSimGpsPoint.getNextPoint(&m_nextSimGpsPoint);
      m_distFromPrevWpt = 0;
      m_simulatedSpeed = 0;

      m_currentSimGpsPosition.latitude          = latTemp;
      m_currentSimGpsPosition.longitude         = lonTemp;
      m_currentSimGpsPosition.altitude          = 74.0;
      m_currentSimGpsPosition.heading           = tmpAngle;
      m_currentSimGpsPosition.speed_over_ground = tmpSpeed;
      m_currentSimGpsPosition.vertical_speed    = 0;
      m_currentSimGpsPosition.weekno_for_fix    = 0;
      m_currentSimGpsPosition.tow_for_fix       = 0;
      //m_currentSimGpsPosition.fix_time        = 0; //Broken since long before me!
      m_currentSimGpsPosition.constellation_no  = 0;
      m_currentSimGpsPosition.latency           = (float)0.7;

      decodedGpsPositionVelocityFunc(m_currentSimGpsPosition, src);
   }

   void NavTask::setSimStop()
   {
      resetGpsSimulation();
      m_simInfoStatus &= ~simulate_on;
      m_queue->setTimer(m_simTimeoutId, SIMULATION_DATA_INTERVAL);
   }

   void NavTask::setSimRepeat()
   {
      m_simInfoStatus |= simulate_repeat_on;
   }

   void NavTask::stopSimRepeat()
   {
      m_simInfoStatus &= ~simulate_repeat_on;
   }

   void NavTask::setSimPaused()
   {
      uint32 src = (uint32)MsgBufferEnums::ADDR_DEFAULT;
      m_simInfoStatus |= simulate_paused;
      decodedGpsPositionVelocityFunc(m_currentSimGpsPosition, src);
   }

   void NavTask::resumeSimPaused()
   {
      m_simInfoStatus &= ~simulate_paused;
      m_queue->setTimer(m_simTimeoutId, SIMULATION_DATA_INTERVAL);
   }

   void NavTask::incSimSpeed()
   {
      isabTime t;
      uint16 error_conditions = 0;
      float pdop = 1;
      uint32 src = (uint32)MsgBufferEnums::ADDR_DEFAULT;
      if (m_simSpeed > 4) { //could be up to 128x.
         m_simInfoStatus |= simulate_max_speed;
         m_simGpsStatus = os_gps_demo_4x; 
         m_simSpeed = 8;
      }
      else {
         m_simInfoStatus &= ~simulate_min_speed;
         m_simGpsStatus++;
         m_simSpeed <<= 1;
         if (m_simSpeed > 4) {
            m_simInfoStatus |= simulate_max_speed;
         }
      }
      decodedGpsStatusFunc(t, (enum GpsStatus)m_simGpsStatus, error_conditions, pdop, src);
   }

   void NavTask::decSimSpeed()
   {
      isabTime t;
      uint16 error_conditions = 0;
      float pdop = 1;
      uint32 src = (uint32)MsgBufferEnums::ADDR_DEFAULT;
      if (m_simSpeed < 2) {
         m_simInfoStatus |= simulate_min_speed;
         m_simGpsStatus = os_gps_demo_hx;
         m_simSpeed = 1;
      }
      else {
         m_simInfoStatus &= ~simulate_max_speed; 
         m_simGpsStatus--;
         m_simSpeed >>= 1;
         if (m_simSpeed < 2) {
            m_simInfoStatus |= simulate_min_speed;
         }
      }
      decodedGpsStatusFunc(t, (enum GpsStatus)m_simGpsStatus, error_conditions, pdop, src);
   }

   void NavTask::generateNextSimulatedGpsPosition()
   {
      DBG("Simulating so generating a GPS Position.");
      uint32 src = (uint32)MsgBufferEnums::ADDR_DEFAULT;
      float tmpLat, tmpLon;
      int32 latStart, lonStart, latEnd, lonEnd, currentLat, currentLon;
      int32 distBtwWpts, distToTurn;
      uint8 angleStart, angleEnd, angleDiff, currentAngle;
      uint8 speedLimit, maxTurnSpeed;

      if (m_simInfoStatus & simulate_paused) {
         m_queue->stopTimer(m_simTimeoutId);
         decodedGpsPositionVelocityFunc(m_currentSimGpsPosition, src);
      }
      if (!(m_simInfoStatus & simulate_on)) {
         stopGpsSimulation();
      }
      if (m_simInfoStatus & simulate_on) {
         speedLimit = m_currentSimGpsPoint.getSpeedLimit() * 10 / 36;
         maxTurnSpeed = uint8(speedLimit / 2);
         distBtwWpts = m_currentSimGpsPoint.distanceToPoint(&m_nextSimGpsPoint);
         //When the end is reached distBtwWpts is zero which is not good when dividing.
         if (distBtwWpts < 1) {
            distBtwWpts = 1;
         }

         m_currentSimGpsPoint.getAngle(angleStart);
         m_nextSimGpsPoint.getAngle(angleEnd);
         angleDiff = abs(angleEnd - angleStart);
         if (angleDiff > 128) {
            angleDiff = 256 - angleDiff;
         }
         currentAngle = angleStart;

         //We're 50m - 0m before a hard turn, so decrease speed.
         if (((m_distFromPrevWpt + 50) >= distBtwWpts) && (angleDiff > 30)) {
            //Smooth the turn.
            distToTurn = m_distFromPrevWpt;
            distToTurn += (m_simSpeed * m_simulatedSpeed) / 2 * SIMULATION_DATA_INTERVAL / 1000;
            if (distToTurn >= distBtwWpts) {
               currentAngle = angleStart + int8(angleEnd - angleStart) / 3;
               //If we're coming to fast, brake hard to a maximum turn speed. 
               if (m_simulatedSpeed > maxTurnSpeed) {
                  m_simulatedSpeed = maxTurnSpeed;
               }
            }
            m_simulatedSpeed -= 2;
         }
         //This postspones the acceleration after an intersection.
         else if ((m_distFromPrevWpt - 20) < 0) {
            //Do nothing - maintain speed.
         }
         //As long as we're under the speed limit and not close 
         //to a turn we accelerate.
         else if (m_simulatedSpeed < speedLimit) {
            m_simulatedSpeed += 2;
         }
         
         //Make sure that we're always moving at least 4m/s.
         if (m_simulatedSpeed < 4) {
            m_simulatedSpeed = 4;
         }
         //Make sure that we're not going faster than the speed limit.
         if (m_simulatedSpeed > speedLimit) {
            m_simulatedSpeed = speedLimit;
         }

         //We've iterated past a new waypoint so change waypoints.
         if (m_distFromPrevWpt >= distBtwWpts) {
            //Smooth the turn.
            if (angleDiff > 30) {
               currentAngle = angleStart + (int8(angleEnd - angleStart) / 3) * 2;
            }
            m_currentSimGpsPoint = m_nextSimGpsPoint;
            m_currentSimGpsPoint.getNextPoint(&m_nextSimGpsPoint);
            if (m_nextSimGpsPoint.isEnd()) {
               if (m_simInfoStatus & simulate_repeat_on) {
                  m_carOnTrack = OffTrack; //To reset the navigation state machine.
                  resetGpsSimulation();
                  return;
               }
               else {
                  resetGpsSimulation();
                  m_simInfoStatus &= ~simulate_on;
                  return;
               }
            }
            else {
               distBtwWpts = m_currentSimGpsPoint.distanceToPoint(&m_nextSimGpsPoint);
               //When the end is reached distBtwWpts is zero which is not good when dividing.
               if (distBtwWpts < 1) {
                  distBtwWpts = 1;
               }
               m_distFromPrevWpt = 0;
            }
         }
         m_currentSimGpsPoint.getCoord(latStart, lonStart);

         m_nextSimGpsPoint.getCoord(latEnd, lonEnd);

         if (latStart < latEnd) {
            currentLat = ((latEnd - latStart) * m_distFromPrevWpt / distBtwWpts + latStart);
         }
         else {
            currentLat = ((latStart - latEnd) * (distBtwWpts - m_distFromPrevWpt) / 
                           distBtwWpts + latEnd);
         }
         if (lonStart < lonEnd) {
            currentLon = ((lonEnd - lonStart) * m_distFromPrevWpt / distBtwWpts + lonStart);
         }
         else {
            currentLon = ((lonStart - lonEnd) * (distBtwWpts - m_distFromPrevWpt) / 
                           distBtwWpts + lonEnd);
         }

         tmpLat = float(currentLat / 100000000.0);
         tmpLon = float(currentLon / 100000000.0);

         m_distFromPrevWpt += (m_simSpeed * m_simulatedSpeed) / 2 * SIMULATION_DATA_INTERVAL / 1000;

         m_currentSimGpsPosition.latitude          = tmpLat;
         m_currentSimGpsPosition.longitude         = tmpLon;
         m_currentSimGpsPosition.altitude          = 74.0;
         m_currentSimGpsPosition.heading           = currentAngle;
         m_currentSimGpsPosition.speed_over_ground = m_simulatedSpeed;
         m_currentSimGpsPosition.vertical_speed    = 0;
         m_currentSimGpsPosition.weekno_for_fix    = 0;
         m_currentSimGpsPosition.tow_for_fix       = 0;
         //m_currentSimGpsPosition.fix_time        = 0; //Broken since long before me!
         m_currentSimGpsPosition.constellation_no  = 0;
         m_currentSimGpsPosition.latency           = (float)0.7;

         decodedGpsPositionVelocityFunc(m_currentSimGpsPosition, src);
      }
      DBG("Simulating - Done generating GPS Position.");
   }

   void NavTask::decodedGpsSatelliteInfo(GpsSatInfoHolder* satInfo, uint32 src)
   {
      GPSDBG("decodedGpsSatelliteInfo(%p, %"PRIx32")", (void*)satInfo, src);
      if (!(m_simInfoStatus & simulate_on)) {
         decodedGpsSatelliteInfoFunc(satInfo, src);
      }
      GPSDBG("end of GpsSatelliteInfo");
   }

   void NavTask::decodedGpsSatelliteInfoFunc(
         GpsSatInfoHolder* satInfo,
         uint32 /*src*/)
   {
      if (!m_shutdownPrepared) {
         m_navTaskConsumer->satelliteInfo(satInfo);
      }
   }

   void NavTask::decodedGpsPositionVelocity(
         const struct GpsConsumerPublic::GpsPositionVelocityVector &v,
         uint32 src)
   {
      GPSDBG("decodedGpsPositionVelocity(XXX, %"PRIx32")", src);
      if (!(m_simInfoStatus & simulate_on)) {
         decodedGpsPositionVelocityFunc(v, src);
      }
      GPSDBG("end of GpsPositionVelocity");
   }

   void NavTask::decodedGpsPositionVelocityFunc(
         const struct GpsConsumerPublic::GpsPositionVelocityVector &v,
         uint32 /*src*/)
   {
      uint32 curr_time = isab::TimeUtils::millis();
      
      if (curr_time - m_last_time < 250) {
         /* Too near last packet, drop it instead. */
         return;
      }
            
      m_last_time = curr_time;

      last_gps_lat = v.latitude;
      last_gps_lon = v.longitude;
      
      last_gps_alt = v.altitude;
      last_gps_speed = v.speed_over_ground;
      if ( !last_gps_head_valid || last_gps_speed > 0.5) {
         last_gps_head  = v.heading;
         last_gps_head_valid = true;
      }
      m_gpsLatency = v.latency;

      //DBG("Got a gps position! (%f, %f, %f), moving %i at %f m/s - src:0x%08x\n",
      //      v.latitude * 180 / M_PI, v.longitude * 180 / M_PI, v.altitude, 
      //      last_gps_head, last_gps_speed, src);

      m_interpolationHints.clear();
      
      if (m_routeValid && m_followingRoute && m_useGPSNavigation) {
         navigate_gps(); 
      }

      struct PositionState p;
      p.lat=int32(v.latitude * 100000000.0);
      p.lon=int32(v.longitude * 100000000.0);
      p.alt=int32(v.altitude*10);
      p.speed=int(last_gps_speed * 32);
      p.heading=last_gps_head;
      p.gpsQuality=m_lastGpsQuality;
      p.time=v.fix_time;
      p.timeStampMillis = m_last_time;
      p.positionHints = m_interpolationHints;
      m_lastPosState = p;
      if (!m_shutdownPrepared) {
         m_navTaskConsumer->positionState(p);
      }
   }

   void NavTask::decodedGpsStatus(isabTime t,
         enum GpsStatus sum_status,
         uint16 error_conditions,
         float pdop,
         uint32 src)
   {
      /*GPSDBG("decodedGpsStatus(%"PRIx32", %"PRIx32", %#"PRIx16", %f, %#"PRIx32")", 
             t.millis(), int(sum_status), error_conditions, pdop, src);*/
      if (!(m_simInfoStatus & simulate_on)) {
         decodedGpsStatusFunc(t, sum_status, error_conditions, pdop, src);
      }
      GPSDBG("end of GpsStatus");
   }

   void NavTask::decodedGpsStatusFunc(isabTime /*t*/,
         enum GpsStatus sum_status,
         uint16 error_conditions,
         float pdop,
         uint32 /*src*/)
   {
      enum Quality gpsQuality;

      if ( sum_status == os_gps_fatal) {
         gpsQuality = QualityMissing;
      } else if ( (sum_status < os_gps_ok_2d) ||
            (error_conditions != 0)       ) {
         gpsQuality = QualityUseless;
      } else if ( sum_status < os_gps_ok_3d) {
         gpsQuality = QualityPoor;
      } else if ( pdop > PDOP_EXCELLENT_TRESH) {
         gpsQuality = QualityDecent;
      } else if (sum_status == os_gps_demo_1x) {
         gpsQuality = QualityDemo1x;
      } else if (sum_status == os_gps_demo_2x) {
         gpsQuality = QualityDemo2x;
      } else if (sum_status == os_gps_demo_4x) {
         gpsQuality = QualityDemo4x;
      } else if (sum_status == os_gps_demo_hx) {
         gpsQuality = QualityDemohx;
      } else {
         gpsQuality = QualityExcellent;
      }

      /*GPSDBG("GpsStatus: stat=%i, err=%u, pdop=%f, qual=%i", 
            sum_status, error_conditions, pdop, gpsQuality);*/

      if (gpsQuality != m_lastGpsQuality) {
         if (gpsQuality <= QualityUseless) {
            // Send immediatly if the quality is such
            // that it is unlikely we will receive any 
            // position reports anytime soon. This 
            // is true for QualityUseless and 
            // QualityMissing.
            m_lastPosState.gpsQuality = gpsQuality;
            if (!m_shutdownPrepared) {
               m_navTaskConsumer->positionState(m_lastPosState);
            }
         }
         m_lastGpsQuality = gpsQuality;
      }
   }

   void NavTask::decodedGpsTime(isabTime /*t*/,
         float /*tow*/,
         int /*weekno*/,
         float /*utc_offset*/,
         uint32 /*src*/)
   {
   }

   void NavTask::decodedFileOperation(class GuiFileOperation* op, uint32 src)
   {
      m_fileOperationRequester = src;
      const uint16 opType = op->typeId();
      switch(opType)
         {
         case OPERATION_SELECT:
            m_guiCommandHandler->HandleOperationSelect(static_cast<GuiFileOperationSelect*>(op));
            break;
         case OPERATION_SELECT_REPLY:
            m_guiCommandHandler->HandleOperationSelectReply(static_cast<GuiFileOperationSelectReply*>(op));
            break;
         case OPERATION_CONFIRM:
            m_guiCommandHandler->HandleOperationConfirm(static_cast<GuiFileOperationConfirm*>(op));
            break;
         case OPERATION_CONFIRM_REPLY:
            m_guiCommandHandler->HandleOperationConfirmReply(static_cast<GuiFileOperationConfirmReply*>(op));
            break;
         case OPERATION_COMMAND:
            m_guiCommandHandler->HandleOperationCommand(static_cast<GuiFileOperationCommand*>(op));
            break;
         }
   }

   void 
   NavTask::decodedCommand(enum NavTaskProviderPublic::CommandType command,
                           uint32 src)
   {
      switch (command) {
         case NavTaskProviderPublic::Nop:
            break;

         case NavTaskProviderPublic::Reroute:
            m_stopNextInternalRoute = false;
            if (m_awaitingRouteReply) {
               m_navTaskConsumer->solicitedError(Nav2Error::NAVTASK_ALREADY_DOWNLOADING_ROUTE, src);
            } else {
               if (! m_routeValid) {
                  m_navTaskConsumer->solicitedError(Nav2Error::UC_NO_ROUTE, src);
               } else {
                  m_routeRequestorId = src;
                  reRoute(false, NavServerComEnums::user_request);
               }
            }
            break;

         case NavTaskProviderPublic::AbandonRoute:
            m_stopNextInternalRoute = true;
            m_followingRoute = false;
            m_routeValid = false;
            m_navTaskConsumer->invalidateRoute(false, 0,0,0,0,0,0,0,0,0);
            break;

         case NavTaskProviderPublic::ForgetWheelsize:
         case NavTaskProviderPublic::SendStatus:
            // Not implemented yet!
            break;
         case NavTaskProviderPublic::RouteToPositionCancel:
            // Cancel route from position
            //m_cancelWaitForSignal = true;
            m_queue->stopTimer(m_waitForSignalId);
            m_nbrRetries = 0;
            break;
      }
   }

   void NavTask::decodedNTRouteToGps(int32 toLat, int32 toLon, 
                          int32 fromLat, int32 fromLon,
                          uint32 src)
   {
      m_stopNextInternalRoute = false;
      if (m_awaitingRouteReply) {
         m_rawRootPublic->solicitedError(Nav2Error::NAVTASK_ALREADY_DOWNLOADING_ROUTE, src);
      } else {
#ifdef DEBUG_TIME_LOGGING
         m_timeLogger->StartTimer(1);
#endif
         m_routeRequestorId = src;
         DBG("decodedNTRouteToGps(%"PRId32", %"PRId32", %"PRId32", %"PRId32")",
             toLat, toLon, fromLat, fromLon);
         uint16 heading = MAX_UINT16;
         if((fromLat == MAX_INT32) || (fromLon == MAX_INT32)){
            DBG("Route from where we are, set correct heading: %u", 
                last_gps_head);
            heading = last_gps_head;
         }
         // Need to store all the route data in case of bad
         // gps signals.
         m_routeData.toLat = toLat;
         m_routeData.toLon = toLon;
         m_routeData.fromLat = fromLat;
         m_routeData.fromLon = fromLon;
         m_routeData.routeRequestorId = src;
         m_nbrRetries = 0;

         m_nscRouteReqId = m_navServerCom->routeToGps(
            toLat, toLon, heading, fromLat, fromLon, m_routeRequestorId );
         m_awaitingRouteReply = true;
         m_internalRouteRequest = false;
      }
   }


   void NavTask::decodedGetFilteredRouteList(int16 startWpt, int16 numWpts,
         uint32 src)
   {
      if (m_route && m_routeValid) {
         RouteList rl;
         int wptNo;

         Point wpt = m_firstPoint;
         wptNo=0;
         Point nextWpt;

         wpt.getNextWaypointEvenTruncated(&nextWpt);

         // These are used to work around strange waypoints
         uint32 storedAction             = RouteDatum::START_POINT;
         uint8  storedCrossingType          = RouteDatum::NO_CROSSING;
         uint8  storedExitCount             = 0;

         while (!wpt.isEnd() && (numWpts!=0) ) {
            if (startWpt > 0) {
               --startWpt;
            } else {
               float tmplat, tmplon;
               wpt.getCoordWGS84(tmplat, tmplon);
               // DBG("Got coords %f,%f", tmplat / M_PI * 180, tmplon / M_PI * 180);
               RouteInfoParts::RouteListCrossing *rlc = new RouteInfoParts::RouteListCrossing;
               rlc->wptNo=wptNo;
               rlc->toTarget                    = calc_heading_to_target(tmplat, tmplon);
               int32 timeToGoal;
               rlc->distToGoal                  = wpt.distanceToEnd(&timeToGoal);
               rlc->timeToGoal                  = timeToGoal;
               rlc->crossing.lat                = int32(tmplat * 100000000.0);
               rlc->crossing.lon                = int32(tmplon * 100000000.0);
               rlc->crossing.valid=true;
               rlc->crossing.changed=false;
               rlc->crossing.action             = storedAction;
               rlc->crossing.distToNextCrossing = wpt.distanceToPoint(&nextWpt);
               rlc->crossing.crossingType       = storedCrossingType;
               rlc->crossing.exitCount          = storedExitCount;
               FIXME_fixup_roundabout_crossings(rlc->crossing);

               // Set up only one segment for now
               RouteInfoParts::Segment *seg = new RouteInfoParts::Segment;
               seg->valid=true;
               seg->changed=false;
               seg->streetName=wpt.getText();
               seg->speedLimit=wpt.getSpeedLimit();
               seg->flags=wpt.getFlags();
               rlc->segments.push_back(seg);

               // Dump interesting landmarks. Interesting is defined as:
               //    extended and not active before this crossing and active after it (start)
               //    extended and active before this crossing and not active after it (stop)
               //    not extended (only active between this and the next crossing) (start+stop)
               // FIXME - the filtering is not done yet due to the need to accomodate 
               // the guide view in MapGuide.
               wpt.getLandmarks(rlc->rlc_landmarks);

               wpt.getLanes(rlc->rlc_lanes, rlc->crossing.distToNextCrossing);

               wpt.getSignpost(rlc->rlc_signpost, rlc->crossing.distToNextCrossing);

               // Store the RouteListCrossing in the RouteList and move on to the next
               rl.crossings.push_back(rlc);
               if (numWpts > 0) {
                  --numWpts;
               }
            }

            storedAction                = wpt.getWptAction() & RouteDatum::ACTION_MASK;
            storedCrossingType          = wpt.getWptCrossingType();
            storedExitCount             = wpt.getWptExitCount();

            ++wptNo;
            wpt=nextWpt;
            if (!wpt.isEnd()) {
               wpt.getNextWaypointEvenTruncated(&nextWpt);
            }
         }
         if ( numWpts != 0 ) {
            // If we get here we must have exited the loop abode due to hitting the 
            // end node. Fake all data regarding the nextWpt - there is no such thing.
            //float tmplat, tmplon;
            //wpt.getCoordWGS84(tmplat, tmplon);
            // DBG("Got coords %f,%f", tmplat / M_PI * 180, tmplon / M_PI * 180);
            RouteInfoParts::RouteListCrossing *rlc = new RouteInfoParts::RouteListCrossing;
            rlc->wptNo=wptNo;
            rlc->toTarget                    = 0;
            rlc->distToGoal                  = 0;
            rlc->timeToGoal                  = 0;
            /*rlc->crossing.lat                = int32(tmplat * 100000000.0);
            rlc->crossing.lon                = int32(tmplon * 100000000.0);*/
            wpt.getCoord(rlc->crossing.lat, rlc->crossing.lon);
            rlc->crossing.valid=true;
            rlc->crossing.changed=false;
            rlc->crossing.action             = storedAction;    // Could have synthesized these as well.
            rlc->crossing.distToNextCrossing = 0;
            rlc->crossing.crossingType       = storedCrossingType;
            rlc->crossing.exitCount          = storedExitCount;
            rl.crossings.push_back(rlc);
         }
         m_navTaskConsumer->routeList(rl, src);
      } else {
         m_navTaskConsumer->solicitedError(Nav2Error::NAVTASK_NO_ROUTE, src);
      }
   }


   void NavTask::decodedKeepRoute(int64 routeId, uint32 src, uint32 dst)
   {
      DBG("decodedKeepRoute(%#"PRIx32"%08"PRIx32", %#"PRIx32", %#"PRIx32, 
          HIGH(routeId), LOW(routeId), src, dst);
      if(m_awaitingRouteReply){
         m_awaitingRouteReply = false;
         if(m_nscRouteReqId == dst){
            if(routeId == m_route->getRouteId()){
               //keep old route. Make like it's raining.
               DBG("keep old route. Make like it's raining.");
            } else {
               //keep old route but id doesn't match. We have to
               //reroute just in case
               WARN("keep old route but id doesn't match. Reroute!");
               if(!m_awaitingRouteReply) { 
                  reRoute(m_internalRouteRequest, 
                          NavServerComEnums::user_request);
               }
            }
         } else {
            //keepRoute is not connected to current request
            WARN("decodedKeepRoute not matching m_nscRouteReqId (%#"PRIx32").",
                 m_nscRouteReqId);
         }
      } else {
         //not waiting for reply something is baaaad
         ERR("not waiting for reply something is baaaad");
      }
   }

   void NavTask::decodedNewRoute(bool routeOrigFromGPS,
                                 int32 origLat, int32 origLon,
                                 int32 destLat, int32 destLon, 
                                 int packetLength,
                                 uint32 src)
   {
      enum Nav2Error::ErrorNbr retval;
      DBG("decodedNewRoute(%s, %"PRId32", %"PRId32", %"PRId32", %"PRId32", "
          "%d, 0x%08"PRIx32")", routeOrigFromGPS?"true":"false",
          origLat, origLon, destLat, destLon, packetLength, src);
      // FIXME ! Activate error checking
      retval = m_receiveRoute->prepareNewRoute(packetLength, routeOrigFromGPS, 
                                               origLat, origLon, destLat, destLon);
      if (retval != Nav2Error::NO_ERRORS) {
         if (m_internalRouteRequest) {
            unsolicitedError(retval);
         } else {
            m_navTaskConsumer->solicitedError(retval, m_routeRequestorId);
         }
         m_awaitingRouteReply = false;
         if (m_internalRouteRequest) {
            m_autoRerouteFailed = true;
         }
         return;
      }
   }

   void NavTask::decodedRouteChunk(bool failedRoute, int chunkLength, 
                                   const uint8 *chunkData, uint32 /*src*/)
   {
      int result;
      enum Nav2Error::ErrorNbr errval;

      if (m_stopNextInternalRoute) {
         failedRoute = true;
         m_stopNextInternalRoute = false;
      }

      DBG("decodedRouteChunk(%d, %d, XXX)", failedRoute, chunkLength);
      result = m_receiveRoute->routeChunk(failedRoute, chunkLength, 
                                          chunkData, errval);
      if (result < 0) {
         DBG("Route invalid! Error:\n");
         if (errval != Nav2Error::NO_ERRORS) {
            unsolicitedError(errval);
         }
         m_awaitingRouteReply = false;
         if (m_internalRouteRequest) {
            m_autoRerouteFailed = true;
         }
      } else if (result > 0) {
         DBG("Route done and valid\n");
         switchToNewRoute();
      }
   }


   void NavTask::switchToNewRoute()
   {
#ifdef DEBUG_TIME_LOGGING
      m_timeLogger->StopTimer(1);
#endif
      
      Route *tmp;

      tmp = m_route;
      m_route = m_receiveRoute;
      m_receiveRoute = tmp;
      m_followingRoute = true;
      m_routeValid = true;

      /* Reset the simulation */
      //if ((m_simInfoStatus & simulate_on) && !m_internalRouteRequest) {
      //   setSimStop();
      //}
      if (m_simInfoStatus & simulate_on) {
         if (m_internalRouteRequest) {
            resetGpsSimulation();
         } else {
            setSimStop();
         }
      }

      /* Set up the route following state machine to a resonable default. */
      m_route->findFirstPoint(m_firstPoint);
      m_lastCurrSegCrossingNo = m_firstPoint.getCrossingNo();
      m_lastCurrSeg = m_firstPoint;
      m_lastCurrSegValid = false;
      m_carOnTrack = OffTrack;
      m_holdoff_wrong_dir = 0;
      m_holdoff_off_track = 0;
      m_lastShowStartWithUturn = false;
    	m_prev_dist_to_seg = 0;
      m_offTrackPenalty = 0;

      {
         int32 total_dist;
         int32 total_time;
         
         total_dist = m_firstPoint.distanceToEnd(&total_time);
    	   m_prev_dist_to_goal = total_dist;
      }

      if (!m_shutdownPrepared) {
         // For now, assume the route to be followable only when the complete route
         // has been received.
         // FIXME - add a timeout.
         if (!m_awaitingRouteReply) {
            unsolicitedError(Nav2Error::NAVTASK_NSC_OUT_OF_SYNC);
         } else if (!m_internalRouteRequest) {
            m_navTaskConsumer->ntRouteReply(m_route->getRouteId(),
                                            m_routeRequestorId);
            m_navTaskConsumer->invalidateRoute(true,
                                               m_route->getRouteId(),
                                               m_route->getTopLat(),
                                               m_route->getLeftLon(),
                                               m_route->getBottomLat(),
                                               m_route->getRightLon(),
                                               m_route->getOrigLat(),
                                               m_route->getOrigLon(),
                                               m_route->getDestLat(),
                                               m_route->getDestLon()
                                               );
         } else {
            m_navTaskConsumer->invalidateRoute(true,
                                               m_route->getRouteId(),
                                               m_route->getTopLat(),
                                               m_route->getLeftLon(),
                                               m_route->getBottomLat(),
                                               m_route->getRightLon(),
                                               m_route->getOrigLat(),
                                               m_route->getOrigLon(),
                                               m_route->getDestLat(),
                                               m_route->getDestLon()
                                               );
         }
         m_awaitingRouteReply = false;
         m_autoRerouteFailed = false;
      }
   }

   void NavTask::reRoute(bool internal, 
                         enum NavServerComEnums::RerouteReason reason)
   {
      //If we're simulating we dont want to allow user reroutes.
      if ((m_simInfoStatus & simulate_on) && !internal){
         return;
      }
      
      // If the connection is down, we will not allow rerouting
      if(!m_canReroute) {
         return;
      }
      
      m_nscRouteReqId = m_navServerCom->routeToGps(m_route->getDestLat(),
                                                   m_route->getDestLon(),
                                                   last_gps_head, 
                                                   MAX_INT32,
                                                   MAX_INT32,
                                                   m_routeRequestorId,
                                                   m_route->getRouteId(),
                                                   reason);
      DBG("Sent reroute request with id %#"PRIx32, m_nscRouteReqId);
      m_internalRouteRequest = internal;
      m_awaitingRouteReply = true;
   }

   void NavTask::decodedSolicitedError( const ErrorObj& err, 
                                        uint32 src, uint32 dst )
   {
      /* FIXME - permutate the error messages here??? */
      if (m_awaitingRouteReply && dst == m_nscRouteReqId) {
         if ( !m_internalRouteRequest && 
              err.getErr() == Nav2Error::NSC_NO_GPS )
         {
            // Start timer and wait for gps to receive signals.
            m_queue->setTimer(m_waitForSignalId, FIRST_WAIT_FOR_GPS_SIGNAL);
            m_awaitingRouteReply = false;
         } else if (!m_internalRouteRequest) {
            DBG("Error %#x while waiting for external route.",
                unsigned(err.getErr()) );
            m_navTaskConsumer->solicitedError( err, m_routeRequestorId );
            m_awaitingRouteReply = false;
         } else {
            DBG("Error %#x while waiting for internal route.", 
                unsigned(err.getErr()) );
            unsolicitedError( err );
            m_awaitingRouteReply = false;
            reRoute(true, NavServerComEnums::off_track);
         }
      } else {
         WARN("Unknown error %#x to %#"PRIx32" from %#"PRIx32, 
              unsigned(err.getErr()), dst, src );
         /* Unknwon error, what to do??? */
      }
   }

   void NavTask::decodedParamNoValue(uint32 paramId, 
                                     uint32 /*src*/, uint32 /*dst*/)
   {
      enum ParameterEnums::ParamIds param = ParameterEnums::ParamIds(paramId);
      enum paramType { noParam, intParam, charParam } type = intParam;
      const char* charval[4] = {NULL}; //inc if necessary
      int32 intval[4] = {0};
      int num = 0;
      WARN("Received a response for an empty parameter %#x", param);
      switch(param) {
      case ParameterEnums::R_routeName:
         type = charParam;
         charval[num++] = "Default.rut";
         break;
      case ParameterEnums::R_lastIndex:
      case ParameterEnums::NT_ServerTrafficUpdatePeriod:
         //use default type intParam
         intval[num++] = 0; //one value 0.
         break;
      case ParameterEnums::NT_UserTrafficUpdatePeriod:
         //use default type intParam
         intval[num++] = 30 | 0x00000000; 
         break;
      default:
         type = noParam;
      }

      switch(type){
      case intParam:
         m_parameter->setParam(param, intval, num);
         break;
      case charParam:
         m_parameter->setParam(param, charval, num);
         break;
      default: 
         break;
      }
   }

   void NavTask::decodedParamValue(uint32 paramId, const int32 * data, 
                                   int32 numEntries,
                                   uint32 /*src*/, uint32 /*dst*/)
   {
      switch(paramId){
      case ParameterEnums::UC_AutoReroute:
         if ( (m_carOnTrack == OffTrack) &&
              (!m_awaitingRouteReply)    &&
              (!m_autoRerouteFailed)     &&
              (numEntries >= 1)          &&
              (data[0] != 0) )
         {
            reRoute(true, NavServerComEnums::off_track);
         }
         break;
      case ParameterEnums::WayfinderType:
         if ((data[0] == GuiProtEnums::Silver) || 
             (data[0] == GuiProtEnums::Iron)) {
            m_useGPSNavigation = false;
         } else {
            m_useGPSNavigation = true;
         }
         break;
      case ParameterEnums::R_lastIndex:
         m_routeLastUsedIndex = data[0];
         break;
      case ParameterEnums::NT_UserTrafficUpdatePeriod:
         setUserPtui(data[0]);
         break;
      case ParameterEnums::NT_ServerTrafficUpdatePeriod:
         setServerPtui(data[0]);
         break;
      default:
         WARN("Unexpected int parameter 0x%08"PRIx32, paramId);
      }
   }

   void NavTask::decodedParamValue(uint32 paramId, const float * /*data*/, 
                                   int32 /*numEntries*/,
                                   uint32 /*src*/, uint32 /*dst*/)
   {
      WARN("Unexpected float parameter 0x%08"PRIx32, paramId);
   }

   void NavTask::decodedParamValue(uint32 paramId, const char * const * data, 
                                   int32 /*numEntries*/,
                                   uint32 /*src*/, uint32 /*dst*/)
   {
      if (paramId == ParameterEnums::R_routeName) {
         if (m_routeLastFileName) {
            delete[] m_routeLastFileName;
            m_routeLastFileName = NULL;
         }
         m_routeLastFileName = strdup_new(data[0]);
      } else {
         WARN("Unexpected string parameter 0x%08"PRIx32, paramId);
      }
   }

   void NavTask::decodedParamValue(uint32 paramId, const uint8* /*data*/, int /*size*/,
                          uint32 /*src*/, uint32 /*dst*/)
   {
      WARN("Unexpected blob parameter 0x%08"PRIx32, paramId);
   }

   //
   // Calculate the direction to the target
   //
   uint8 NavTask::calc_heading_to_target(float lat, float lon)
   {
      uint8 heading_to_target;
      float destLat = m_route->getDestLat()/100000000.0;
      float destLon = m_route->getDestLon()/100000000.0;

      float latdiff = destLat - lat;
      float londiff = destLon - lon;
      // DBG("latdiff=%f, londiff=%f", latdiff, londiff);
      if (londiff >  Constants::pi)   londiff -= Constants::pi;
      if (londiff >  Constants::pi)   londiff -= Constants::pi;
      if (londiff < -Constants::pi)   londiff += Constants::pi;
      if (londiff < -Constants::pi)   londiff += Constants::pi;
      float horiz_scale = londiff * 0.5 * ( cos(lat) + cos(destLat));
      // DBG("latdiff=%f, londiff=%f, horiz_scale=%f", latdiff, londiff, horiz_scale);
      // Danger Will Robinson - possible divide by zero?
      heading_to_target = FastTrig::angleTheta2float(horiz_scale, latdiff);
      // DBG("Found heading to the target: %i", heading_to_target);
      return heading_to_target;
   }

   void NavTask::FIXME_fixup_roundabout_crossings(RouteInfoParts::Crossing & c)
   {
      if (c.action == RouteDatum::EXIT_RDBT_8) {
         c.action = RouteDatum::EXIT_RDBT;
         c.exitCount += 8;
      } else if (c.action == RouteDatum::EXIT_RDBT_16) {
         c.action = RouteDatum::EXIT_RDBT;
         c.exitCount += 16;
      }
   }


   bool NavTask::offTrackDetection( int dist_to_seg, int seg_angle, uint32 dist_to_goal )
   {
      int diff = m_prev_dist_to_goal - dist_to_goal;

      int d_offset_par = std::max(0,diff);
      int d_offset_per = dist_to_seg - m_prev_dist_to_seg;
      int norm_der = (1000*d_offset_per)/(d_offset_par+1);

      m_prev_dist_to_goal = dist_to_goal;
      m_prev_dist_to_seg = dist_to_seg;

      int angle_diff = abs(seg_angle - last_gps_head );

      if (angle_diff > 128) {
         angle_diff = 256 - angle_diff;
      }

      //check if route has been reached i.e. we are aligned and close enough
      if (dist_to_seg < MAX_DISTANCE_FOR_SNAP && 
          angle_diff < MAX_ANGLE_ERROR_FOR_SNAP) {
         // Route has been reached!!
         // On track..
         // FIXME: Signal that the route has been reached
         // and allow snap to route. (Use case: driving on parking lot
         // before reaching the route should not snap to route.)
      }

      // require MAX_SEGMENT_DISTANCE_FOR_ONTRACK off route 
      // to off track if position simulated
      /*if (iCurFromSimulator && dist_to_seg > MAX_SEGMENT_DISTANCE_FOR_ONTRACK) {
         return true;
      }*/
	    	
	   // We are very far from the route, off track immediately
      if (dist_to_seg > MAX_DISTANCE_FOR_ON_TRACK) {
         m_offTrackPenalty += MAX_OFF_TRACK_PENALTY;
         return true;
      }

      /** Basic check to see if we should continue with track detection
       * 
       *  1) We are driving faster than MIN_SPEED_FOR_OFFTRACK knots. We need 
       *     to have some minimum speed to get precision in the positions and 
       *     avoid drifting.
       *  2) We have moved at least MIN_PER_DIFF meters further away from the 
       *     route since last position. 
       *  3) The position has not moved more than MAX_PER_DIFF meters further 
       *     away from the route since last position. This is unrealistic unless 
       *     diverging from the route very fast and works as a guard against large
       *     position disturbances.
       */
      if (last_gps_speed >= MIN_SPEED_FOR_OFFTRACK && d_offset_per >= MIN_PER_DIFF) {
         
         if (d_offset_per >= MAX_PER_DIFF) {
            // risk that this is due to large position disturbance
            // return false, but don't reset iOffTrackPenalty
            return false;
         }

         /**
          * Check the angle error to see if we are driving in 
          * the wrong direction
          */
         
          if (angle_diff > MIN_ANGLE_ERROR_FOR_OFF_TRACK) {
            // we are driving in the wrong direction, give large penalty
            
            m_offTrackPenalty += MAX_PENALTY_PER_POSITION;
            
            return true;
          } else {
            // make "normal" off track detection

            // check if we have reached the minimal off track
            // threshold, MIN_OFF_TRACK_PENALTY
            if (norm_der >= MIN_OFF_TRACK_PENALTY) {
               // add norm_der to the penalty
               
               if (dist_to_seg <= MIN_DIST_FOR_EXTRA_PENALTY) {
                  // we are closer than MIN_DIST_FOR_EXTRA_PENALTY to 
                  // the route, add normal penalty
                  m_offTrackPenalty += std::min((int)MAX_PENALTY_PER_POSITION,(int)norm_der);
               } else {
                  // we are further than MIN_DIST_FOR_EXTRA_PENALTY meters from the route
                  // increase the penalty by 100% for every 2 meters further away
                  m_offTrackPenalty += std::min((int)MAX_PENALTY_PER_POSITION,
                        (int)(norm_der*(dist_to_seg-MIN_DIST_FOR_EXTRA_PENALTY)/2));
               }
               
               return true;
             } else {
               // We found a position which did not reach the threshold for 
               // off track. Reset the penalty since we require consecutive bad 
               // positions for off track
               m_offTrackPenalty = 0;
               return false;
             }
          }
       } else {
          // we did not pass the basic check, reset penalty and return false
         
          m_offTrackPenalty = 0;
          
          return false;
      }
   }


   /** 
    * Performs the calculation of the snapped position.
    */
   void NavTask::calculateSnappedPosition( int32 x1, int32 y1,
                                           int32 x2, int32 y2,
                                           int32 xP, int32 yP,
                                           int32 &snap_lat,
                                           int32 &snap_lon )
   {
      snap_lon = xP;
      snap_lat = yP;

      int v12x = x2 - x1;
      int v12y = y2 - y1;
      int v1Px = xP - x1;
      int v1Py = yP - y1;

      int v12l;

      int v12dotv1P;

      int v12lsqr = v12x*v12x + v12y*v12y;

      if( v12lsqr == 0 ){
         // (x1,y1) == (x2,y2)
         return;
      }

      v12l = (int)sqrt(v12lsqr);

      v12dotv1P = v12x * v1Px + v12y * v1Py;

      if (v12dotv1P < 0) {
         // L = P1
         snap_lon = x1;
         snap_lat = y1;
      } else if (v12dotv1P > v12lsqr) {
         // L = P2
         snap_lon = x2;
         snap_lat = y2;
      } else {
         // L is on the segment
         int v1Ll = v12dotv1P / v12l;
         snap_lon = x1 + (x2-x1)*v1Ll / v12l;
         snap_lat = y1 + (y2-y1)*v1Ll / v12l;
      }
   }



   void NavTask::navigate_gps()
   {
      int32 dist_to_seg=MAX_INT32;
      Point::ClosestSegmentResult closest;
      Point first_point, second_point;
      enum OnTrackEnum lastCarOnTrack;

      //DBG("navigate_gps()\n");

      if (!m_route) {
         WARN("navigate_gps called with no m_route\n");
         unsolicitedError(Nav2Error::NAVTASK_INTERNAL_ERROR);
         return;
      }
      if (!m_followingRoute) {
         WARN("navigate_gps called with no active route\n");
         unsolicitedError(Nav2Error::NAVTASK_INTERNAL_ERROR);
         return;
      }


      if (last_gps_speed < 1.0 && m_lastCurrSegValid) {
         return;
      }

      lastCarOnTrack = m_carOnTrack;

      int32 snap_lat = MAX_INT32;
      int32 snap_lon = MAX_INT32;

      switch (m_carOnTrack) {
         case OnTrack:
            dist_to_seg = m_lastCurrSeg.getClosestSegment(
                  last_gps_lat, last_gps_lon,
                  last_gps_head,
                  500,
                  &ontrack_penalty_params,
                  first_point,
                  second_point,
                  closest);
            /* If we find no suitable minimaps ahead of us we
            *           * go off track to try all of them */
            if (dist_to_seg >= 0) {
               /* Found a segment - accept it */
               uint8 snap_dir = last_gps_head;
               if( dist_to_seg > 0 ){
                  int32 first_lat, first_lon;
                  int32 second_lat, second_lon;
                  //if the current position is not on the segement,
                  //calculate a position on the segement used to snap to route.
                  first_point.getCoord( first_lat, first_lon );
                  second_point.getCoord( second_lat, second_lon );
                  calculateSnappedPosition( first_lon, first_lat,
                                            second_lon, second_lat,
                                            (int32)(last_gps_lon*100000000.0),
                                            (int32)(last_gps_lat*100000000.0),
                                            snap_lat, snap_lon );
                  snap_dir = closest.segData.segmentAngle;
               }
               //Set the snap to route coord.
               m_navTaskConsumer->sendRouteCoord(snap_lat, snap_lon, snap_dir);
               break;
            } else if (dist_to_seg == -2) {
               INFO("  nav_gps: Current GPS position outside all forward minimaps");
               m_carOnTrack = OffTrack;
               /* Fall through */
            } else {
               ERR("  nav_gps: internal inconsistency in getClosestSegment detected");
               unsolicitedError(Nav2Error::NAVTASK_CONFUSED);
               m_followingRoute = false;
               m_routeValid = false;
               m_navTaskConsumer->invalidateRoute(false, 0,0,0,0,0,0,0,0,0);
               /* FIXME - send route status here as well? We probably should notify 
                * receivers of oldStyleRouteInfo that we are no longer following a route. */
               return;
            }
            /* Fall through */
         case OnTrackWrongWay:
            /* We do not use any hysteresis when driving the wrong way for now.
             * Fall through and treat the same way as off track. */
         case OffTrack:
            dist_to_seg = m_firstPoint.getClosestSegment(
                  last_gps_lat, last_gps_lon, 
                  last_gps_head,
                  0, 
                  &offtrack_penalty_params,
                  first_point,
                  second_point,
                  closest);
            if (dist_to_seg == -2) {
               INFO("  nav_gps: Current GPS position outside all minimaps");
               m_followingRoute = false;
               // Only declare the route totally invalid if it was based on a 
               // gps origin. Perhaps we should not do this even then.
               if (m_route->wasRouteOrigGPS()) {
                  unsolicitedError(Nav2Error::NAVTASK_FAR_AWAY);
                  m_navTaskConsumer->invalidateRoute(false, 0,0,0,0,0,0,0,0,0);
                  m_routeValid = false;
               }
               return;
            } else if (dist_to_seg < 0) {
               ERR("  nav_gps: internal inconsistency in getClosestSegment (call 2) detected");
               unsolicitedError(Nav2Error::NAVTASK_CONFUSED);
               m_followingRoute = false;
               m_routeValid = false;
               m_navTaskConsumer->invalidateRoute(false, 0, 0,0,0,0,0,0,0,0);
               return;
            }
            break;
         default:
            ERR("nav_gps: unknown state");
            unsolicitedError(Nav2Error::NAVTASK_INTERNAL_ERROR);
            return;
      }
      GPSDBG("Closest seg: %d at %"PRId32"m", 
             closest.seg.getPointId(), dist_to_seg);

      Point nextWpt;
      Point nextSeg;
      int32 dist_left_to_waypoint;
      int8 after_turn; 
      int8 theta_diff;
      int8 to_track_turn = 0;
      int8 to_on_track_turn = 0;
      uint32 dist_to_goal;
      bool reachedGoal = false;
      bool standstill;
      bool showStartWithUturn = false;
      int32 etg_nextseg_to_nextwpt, etg_nextwpt_to_end;
      int32 time_to_goal = 0;

      uint32 phoneHomeDist = m_route->getPhoneHomeDistance();
      uint32 truncatedDist = m_route->getTruncatedDistance();

      closest.seg.getNextWaypointEvenTruncated(&nextWpt);
      closest.seg.getNextPoint(&nextSeg);
      /* FIXME - XXX - check errors from distanceToPoint() */
      dist_left_to_waypoint = closest.segData.distanceLeftOnSegment + 
                              nextSeg.distanceToPoint(&nextWpt, &etg_nextseg_to_nextwpt);
      RTDBG("NAV: DISTANCE TO WPT = %"PRId32"\n", dist_left_to_waypoint);
      dist_to_goal=dist_left_to_waypoint+nextWpt.distanceToEnd(&etg_nextwpt_to_end);
      RTDBG("NAV: TOTAL DISTANCE TO GOAL = %"PRId32"\n", dist_to_goal);

      if (phoneHomeDist) {
         RTDBG("NAV: Left until trunc: %"PRId32", "
               "left until phone home: %"PRId32"", 
               dist_to_goal - truncatedDist, 
               dist_to_goal - (truncatedDist + phoneHomeDist));
      }

      // Hack for ramps. Ramps are digitized at the physical divider. This is 
      // non-intuitive to the driver. The turning instruction for ramps is
      // moved 100m earlier to improve this.
      if (closest.seg.isWptExitRamp()) {
         dist_left_to_waypoint -= 100;
         if (dist_left_to_waypoint < 0) {
            dist_left_to_waypoint = 0;
         }
      }

      time_to_goal = 
         int32(closest.segData.distanceLeftOnSegment*3.6/closest.seg.getSpeedLimit()) + 
         etg_nextseg_to_nextwpt + etg_nextwpt_to_end;
      RTDBG("NAV: ETG = %"PRId32, time_to_goal);


      /* This calculation of the time to the next turn is based on the following
       * assumptions. If the car is less than s1 meters from the turn, assume 
       * a linear deaccelration (in time, not realistic, but good enough) from
       * the current speed (vcurr) to vturn at the turn. If vcurr>vturn then assume
       * constant speed to the corner.
       *
       * At distances to the turn s>s1 assume a constant speed vcurr until s=s1 and 
       * then make the same assumptions as above.
       *
       * vcrnr = vmax/3 (one third of the stipulated speed limit).
       * s1 = 20m
       *
       * This does not take into account the slowdowns due to corners in the road
       * that are not waypoints. The time derived in this way is _only_ valid 
       * close to the waypoint and is an _lower_ limit to the time needed to reach
       * the waypoint. 
       */
      float time_left_to_waypoint = 0;
      {
         int s1 = 20;  /* start slowing down here */
         float vmax = closest.seg.getSpeedLimit() / 3.6;
         float vcurr = last_gps_speed;
         float vcrnr_min = vmax/3.0;
         float vcrnr;

         if (vcurr > vcrnr_min) {
            uint8 segment1Angle, segment2Angle;
            int8 heading_diff;
            segment1Angle = closest.segData.segmentAngle;
            if (0 == nextSeg.getAngle(segment2Angle)) {
               heading_diff = (int8)segment2Angle - (int8)segment1Angle;
               if (heading_diff == MIN_INT8) heading_diff = MAX_INT8;
               if (heading_diff < 0) heading_diff = -heading_diff;
            } else {
               heading_diff=0;
            }
            /* FIXME - apply a better algorithm, something smooth and not this step
             * function. */
            if (heading_diff > 50) {
               vcrnr = vcrnr_min;
            } else {
               vcrnr = vmax;
            }
            //DBG("heading_diff=%i, %i, %i", heading_diff, segment1Angle, segment2Angle);
         } else {
            /* Driving slower than what we think the corner should be - use
             * current speed instead. */
            vcrnr = vcurr;
         }


         /* If vcurr < 1.0m/s then we are close to standstill and will never reach the
          * waypoint. Good enough I guess. */
         if (vcurr >= 1.0) {
            standstill = false;
            int dist_with_deacceleration = (s1 < dist_left_to_waypoint) ? s1 : dist_left_to_waypoint;
            /* Simplified algorithm below... Maybe good enough? */
            time_left_to_waypoint = dist_with_deacceleration / ((vcrnr + vcurr)/2.0);
            if (dist_left_to_waypoint > s1) {
               //DBG("before add: %f", time_left_to_waypoint);
               time_left_to_waypoint += (dist_left_to_waypoint - s1) / vcurr;
            }
            //DBG("time_left_to_waypoint=%f, vcurr=%f, vcrnr=%f, vcrnr_min=%f, vmax=%f, "
            //    "dist_with_deacceleration=%i", 
            //     time_left_to_waypoint, vcurr, vcrnr, vcrnr_min, vmax, dist_with_deacceleration);
         } else {
            standstill = true;
         }
      }


      // If we are able to show start with uturn, allow a much longer off track distance
      // on the first segment.
      bool onFirstSegAndStartWithUturn = closest.seg.isFirstSegAndStartWithUturn();
      int32 max_dist_until_offtrack = MAX_DIST;
      if (onFirstSegAndStartWithUturn) {
         max_dist_until_offtrack += 100;
      }

#if defined USE_OLD_OFFTRACK_DETECTION
      /******* Legacy code **********/
      // ON TRACK !?!?!?
      if (dist_to_seg < max_dist_until_offtrack ) {
         // Not so  fast... we must turn car into correct heading of segment
         if ( m_carOnTrack == OffTrack) {
            //DBG("Smoothly back to ontrack");

            // how much do we need to turn
            after_turn = closest.segData.segmentAngle - last_gps_head;
            //DBG("Smoothly back turn: %i",after_turn);

            if (after_turn < 30 && after_turn > -30 ) {
               // car turned enough, go ON_TRACK
               m_carOnTrack = OnTrack;
               // this may be overwritten below if we are close to crossing
//               input_data->sound = BACK_ON_TRACK_SOUND; FIXME
               DBG("Back ontrack!!!");
               // make sure to be lazy on notifying wrong dir
               m_holdoff_wrong_dir = 0;
            } else {
               // else we are close to track but need to turn the car more
               // in correct direction
               m_carOnTrack = OffTrack;
               DBG("Stay offtrack!!!");
            }
         } else {
            // Close to a viable segment and was not off track. This means that
            // any mode except off track will change to on track as soon as 
            // we get close to the route.
            m_carOnTrack = OnTrack;
         }
      } else {
         m_carOnTrack = OffTrack;
      }

      // Not so fast to off track...
      if (m_carOnTrack == OnTrack) {
         m_holdoff_off_track = 0;
      } else if (m_carOnTrack == OffTrack) {
         m_holdoff_off_track++;

         if (m_holdoff_off_track < OFF_TRACK_HOLDOFF_VAL) {
            // pretend we are on track a while
            m_carOnTrack = OnTrack; 
         } else if (m_holdoff_off_track == OFF_TRACK_HOLDOFF_VAL) {
            // sound instance when we go off-track
            // really not needed
//            input_data->sound = OFF_TRACK_SOUND;   FIXME
//            input_data->route_status = OFF_TRACK;  FIXME
            RTDBG("NAV: Offtrack sound!!!");
         } else {
            // really not needed
            m_carOnTrack = OffTrack;
            // fix the value to avoid wrap-around
            m_holdoff_off_track = OFF_TRACK_HOLDOFF_VAL + 1;
         }
      }
#endif

#if defined USE_NEW_OFFTRACK_DETECTION
      // Faster off track detection.
      if( m_carOnTrack == OnTrack ) {
         // Perform the off track detection.
         // The method will update m_offTrackPenalty.
          offTrackDetection( dist_to_seg, closest.segData.segmentAngle, dist_to_goal );
      }

      if (m_offTrackPenalty >= MAX_OFF_TRACK_PENALTY ) {
         m_carOnTrack = OffTrack;
      } else {
         m_carOnTrack = OnTrack;
      }
#endif



      // ON TRACK !?!?!?
      if (m_carOnTrack == OnTrack) {

//         dist_to_waypoint = distanceLeftOnSegment +
//            nav_route_distance_point_to_point(&nextPoint,&nextWpt);

//         dist_to_curr_waypoint=
//            input_data->dist_to_curr_waypoint_last_gps=dist_left_to_waypoint;

         if ( phoneHomeDist && (dist_to_goal < (phoneHomeDist + truncatedDist)) ) {
            if (!m_awaitingRouteReply && !m_autoRerouteFailed) {
               reRoute(true, NavServerComEnums::truncated_route);
            }
         }

         if ( dist_to_goal < unsigned(GOAL_DIST)) {
            /* The below flag determines if we should stop navigating when */
            /* we reach the goal. */
            //But we never stop navigation when we're simulating.
            if (!(m_simInfoStatus & simulate_on)) {
#ifdef STOP_NAVIGATING_ON_GOAL
               m_followingRoute = false;
               m_navTaskConsumer->invalidateRoute(false, 0,0,0,0,0, 0,0,0,0);
#endif 
               reachedGoal = true;
            }
         }

         // check for wrong direction
         theta_diff = (int8)last_gps_head - closest.segData.segmentAngle;
         if (theta_diff != -128) {
            theta_diff = theta_diff < 0 ? -theta_diff: theta_diff;
         } else {
            // for avoiding --128 since it still is -128
            theta_diff = 127;
         }

         RTDBG("NAV: Theta-diff: %i\n", theta_diff);
         // almost 90 DEG
         if ( theta_diff > 106 ) {
            RTDBG("NAV: WRONG_DIRECTION TENTATIVE\n");
            m_holdoff_wrong_dir++;
            if (m_holdoff_wrong_dir < WRONG_DIR_HOLDOFF_VAL) {
               /* pretend we are on track a while */
               /* GPS positions may temporary jump */
               m_carOnTrack = OnTrack; 
            } else if (m_holdoff_wrong_dir == WRONG_DIR_HOLDOFF_VAL) {
               /* sound instance when we go off-track */
//               input_data->sound = WRONG_DIR_SOUND;
               m_carOnTrack = OnTrackWrongWay;
               RTDBG("NAV: WRONG_DIR! (sound)\n");
            } else {
               /* fix the value to avoid wrap-around */
               RTDBG("NAV: WRONG_DIR!\n");
               m_carOnTrack = OnTrackWrongWay;
               m_holdoff_wrong_dir = WRONG_DIR_HOLDOFF_VAL + 1;
            }
            // Check if on the first segment and we have a "start with uturn" flag
            // in the START-marker route datum
            if (onFirstSegAndStartWithUturn) {
               m_carOnTrack = OnTrack;
               showStartWithUturn = true;
            }
         } else {
            m_holdoff_wrong_dir = 0;
         }

      } else { //m_carOnTrack != OnTrack

         //RTDBG("NAV: Offtrack\n");

         //nav_print_point(&nextPoint);
         m_carOnTrack = OffTrack;

         // Add the distance to the closest point on the segment
         dist_left_to_waypoint += closest.segData.realDistanceToSegment;
         dist_to_goal += closest.segData.realDistanceToSegment;

         // How much do we need to turn to get back to track
         to_track_turn = closest.segData.angleToSegment - (int8)last_gps_head;

         //RTDBG("NAV: TURN TOWARDS angleToSeg: %i seg_angle: %i to_track_turn: %i\n", 
         //      closest.segData.angleToSegment,
         //      closest.segData.segmentAngle,
         //      to_track_turn);

         // Turn when turning back on track
         to_on_track_turn = closest.segData.segmentAngle - closest.segData.angleToSegment;

         //RTDBG("NAV: TURN TO GET BACK TO TRACK: %i\n",
         //      to_on_track_turn);
      }

      

      /* **************** End of old, unchecked code ******************* */



      //
      // Check if we went off track and want to reroute
      // Reroute if going from OnTrack or WrongWay to
      // OffTrack. (Another bunch of conditions need to
      // be fulfilled as well).
      //
      if ( (m_carOnTrack == OffTrack) && 
           ( (lastCarOnTrack == OnTrack) ||
             (lastCarOnTrack == OnTrackWrongWay) ) ) {
         m_parameter->getParam(ParameterEnums::UC_AutoReroute);
      }

      //
      // Calculate the direction to the target
      //
      uint8 heading_to_target = calc_heading_to_target(last_gps_lat, last_gps_lon);
      
      //
      // Send information to the consumer
      //
      GPSDBG("Send information to the consumer");
      RouteInfo ri;
      Point nextnextWpt;
      bool existsNextNext;
      bool currWptChanged;
      bool currSegChanged;

      currWptChanged = !m_lastCurrSegValid || 
                       (m_lastCurrSegCrossingNo != closest.seg.getCrossingNo()) ||
                       (showStartWithUturn != m_lastShowStartWithUturn);

      currSegChanged = !m_lastCurrSegValid                                  ||
                       (m_lastCurrSegText  != closest.seg.getText())        ||
                       (m_lastCurrSegSpeed != closest.seg.getSpeedLimit())  ||
                       (m_lastCurrSegFlags != closest.seg.getFlags())
                       ;
      m_lastCurrSegText  = closest.seg.getText();
      m_lastCurrSegSpeed = closest.seg.getSpeedLimit();
      m_lastCurrSegFlags = closest.seg.getFlags();

      if (nextWpt.isEnd()) {
         existsNextNext = false;
      } else {
         nextWpt.getNextWaypointEvenTruncated(&nextnextWpt);
         existsNextNext = true;
      }
      if (reachedGoal) {
         ri.onTrackStatus = Goal;
      } else {
         switch (m_carOnTrack) {
         case OnTrack:
            ri.onTrackStatus = RouteEnums::OnTrack; break;
         case OnTrackWrongWay:
            ri.onTrackStatus = RouteEnums::WrongWay; break;
         case OffTrack:
            ri.onTrackStatus = RouteEnums::OffTrack; break;
         }
      }

      if (!showStartWithUturn) {
         ri.distToWpt     = dist_left_to_waypoint;
      } else {
         ri.distToWpt     = 0;
      }
      ri.distToGoal    = dist_to_goal;
      ri.distToTrack   = dist_to_seg;
      ri.toOnTrackTurn = to_on_track_turn;
      ri.toTrackTurn   = to_track_turn;
      if ( !standstill ) {
         ri.timeToWpt  = uint16(time_left_to_waypoint * 10.0);
      } else {
         ri.timeToWpt  =  MAX_UINT16;
      }
      ri.timeToGoal    = time_to_goal;
      ri.latency       = uint16(m_gpsLatency * 10.0);
      ri.speed         = uint16(last_gps_speed);
      ri.overSpeed     = (last_gps_speed * 3.6) > closest.seg.getSpeedLimit();

      ri.toTarget      = heading_to_target;                      

      // Get the waypoints coordinates
      nextWpt.getCoord( ri.lat, ri.lon );

      ri.crossingNo = closest.seg.getCrossingNo();
      //DBG("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF crossingNo=%i", ri.crossingNo);

      // Landmark information
      closest.seg.getLandmarks(ri.rip_landmarks);

      // Lanes information
      closest.seg.getLanes(ri.rip_lanes, dist_left_to_waypoint);

      // Signpost information
      closest.seg.getSignpost(ri.rip_signpost, dist_left_to_waypoint);

      /* Fill in information on the segment the car is on */
      ri.currSeg.valid      = true;
      ri.currSeg.changed    = currWptChanged || currSegChanged;
      ri.currSeg.streetName    = closest.seg.getText();
      ri.currSeg.speedLimit    = closest.seg.getSpeedLimit();
      ri.currSeg.flags         = closest.seg.getFlags();

      /* Alt.attrib not implemented yet */
      ri.distToAltAttribSegment = -1;
      ri.altAttribSegment.valid      = false;
      ri.altAttribSegment.changed    = false;
      ri.altAttribSegment.streetName = "";
      ri.altAttribSegment.speedLimit = 0;
      ri.altAttribSegment.flags      = 0;
      
      /* Fill in information on the crossing the car is heading towards */
      // FIXME - XXX - Make each crossing into a function taking the 
      // appropriate waypoints as parameters. Break out common code.
      ri.currCrossing.valid              = true;
      ri.currCrossing.changed            = currWptChanged;
      if (!showStartWithUturn) {
         ri.currCrossing.action = closest.seg.getWptAction() & RouteDatum::ACTION_MASK;
      } else {
         ri.currCrossing.action = RouteDatum::UTURN;
      }
      if (existsNextNext) {
         ri.currCrossing.distToNextCrossing = nextWpt.distanceToPoint(&nextnextWpt); 
      } else {
         ri.currCrossing.distToNextCrossing = 0;
      }
      ri.currCrossing.crossingType = closest.seg.getWptCrossingType();
      ri.currCrossing.exitCount = closest.seg.getWptExitCount();
      FIXME_fixup_roundabout_crossings(ri.currCrossing);
      {
         nextWpt.getCoord(ri.currCrossing.lat, ri.currCrossing.lon);
      }

      /* Fill in information on the segment after the first crossing */
      if (!nextWpt.isEnd()) {
         ri.nextSeg1.valid      = true;
         ri.nextSeg1.changed    = currWptChanged;
         ri.nextSeg1.streetName = nextWpt.getText();
         ri.nextSeg1.speedLimit = nextWpt.getSpeedLimit();
         ri.nextSeg1.flags      = nextWpt.getFlags();
      } else {
         ri.nextSeg1.valid       = false;
         ri.nextSeg1.changed    = currWptChanged;
         ri.nextSeg1.streetName = "";
         ri.nextSeg1.speedLimit = 0;
         ri.nextSeg1.flags      = 0;
      }

      if (!nextWpt.isEnd()) {
         ri.nextCrossing1.valid = true;
         ri.nextCrossing1.changed = currWptChanged;
         ri.nextCrossing1.action = nextWpt.getWptAction() & RouteDatum::ACTION_MASK;
         ri.nextCrossing1.distToNextCrossing = 0;                                    // FIXME - fill this in 
         ri.nextCrossing1.crossingType = nextWpt.getWptCrossingType();
         ri.nextCrossing1.exitCount = nextWpt.getWptExitCount();
         {
            nextWpt.getCoord(ri.nextCrossing1.lat, ri.nextCrossing1.lon);
         }
      } else {
         ri.nextCrossing1.valid = false;
         ri.nextCrossing1.changed = currWptChanged;
         ri.nextCrossing1.action = 0;
         ri.nextCrossing1.distToNextCrossing = 0;
         ri.nextCrossing1.crossingType = 0;
         ri.nextCrossing1.exitCount = 0;
         ri.nextCrossing1.lat = 0;
         ri.nextCrossing1.lon = 0;
      }
      FIXME_fixup_roundabout_crossings(ri.nextCrossing1);

      if (existsNextNext && !nextnextWpt.isEnd()) {
         ri.nextSeg2.valid      = true;
         ri.nextSeg2.changed    = currWptChanged;
         ri.nextSeg2.streetName = nextnextWpt.getText();
         ri.nextSeg2.speedLimit = nextnextWpt.getSpeedLimit();
         ri.nextSeg2.flags      = nextnextWpt.getFlags();
      } else {
         ri.nextSeg2.valid      = false;
         ri.nextSeg2.changed    = currWptChanged;
         ri.nextSeg2.streetName = "";
         ri.nextSeg2.speedLimit = 0;
         ri.nextSeg2.flags      = 0;
      }
      
      // DBG("ri.onTrackStatus=%i", ri.onTrackStatus);
      
      ri.simInfoStatus = m_simInfoStatus;

      if (!m_shutdownPrepared) {
         m_navTaskConsumer->routeInfo(ri);
#ifndef NO_LOG_OUTPUT
         //ri.log(m_log);
#endif
         GPSDBG("Routeinfo sent to consumer");
      } else {
         DBG("Routinfo not sent due to shutdown.");
      }


      /* Remember the new solution */
      m_lastCurrSeg=closest.seg;
      m_lastCurrSegValid = true;
      m_lastCurrSegCrossingNo=closest.seg.getCrossingNo();
      m_lastShowStartWithUturn = showStartWithUturn;

      std::vector<HintNode> routeNodes;
      
      if( snap_lat == MAX_INT32 || snap_lon == MAX_INT32 ) {
      #ifdef ALLOW_UNSNAPPED_COORDINATES
         routeNodes =
            prepareInterpolationVector( (int32)(last_gps_lat*100000000.0),
                                        (int32)(last_gps_lon*100000000.0) );
      #endif
      } else {
         routeNodes = prepareInterpolationVector(snap_lat, snap_lon);
      }

      if( !routeNodes.empty() ) {
         
#ifdef LOG_INTERPOLATION_DATA
         dumpHintNodeVector( m_last_time, routeNodes );
#endif
         
         HintNodeAlgorithms::calculateWithStaticSpeed( routeNodes );
         
         m_interpolationHints = routeNodes;
      }
   }

   void NavTask::dumpHintNodeVector( uint32 timeMillis,
                                     const std::vector<HintNode>& nodes )
   {
      FILE * saveFile;
      saveFile = fopen( INTERPOL_LOG_FILE , "a");
      fprintf( saveFile, "@ %d\n", timeMillis );

      for( std::vector<HintNode>::const_iterator hI = nodes.begin();
           hI != nodes.end();
           hI++ )
      {
         const HintNode& h = *hI;

         fprintf( saveFile, "# %d %d %d %c\n",
                  h.lat,
                  h.lon,
                  h.velocityCmPS,
                  hI == nodes.begin() ? 'G' : 'R' );
      }

      fclose( saveFile );
   }

   float NavTask::getCalculatedSpeed() {
      
      /**
       *   We don't want to use the internal speed of the GPS, so
       *   we calculate it using the previous positions.
       */
      
      int32 curTimeMillis = isab::TimeUtils::millis();
      int32 realLat = (int32)( last_gps_lat * 100000000.0 );
      int32 realLon = (int32)( last_gps_lon * 100000000.0 );
      
      if( m_prevLat == MAX_INT32 && m_prevLon == MAX_INT32 ) {
         m_prevLat = realLat;
         m_prevLon = realLon;
         m_prevTimeMillis = curTimeMillis;
         return 0.0f;
      }

      /**
       *   Get the distance travelled since the last GPS update.
       */
      
      float64 distanceCoveredCm =
         static_cast<float>( 
            Nav2DistanceUtil::calcDistanceCentimeters( m_prevLat,
                                                       m_prevLon,
                                                       realLat,
                                                       realLon ) );
      /**
       *  Get the elapsed time since the last GPS update.
       */
      
      float timeDiffSeconds =
         (curTimeMillis - m_prevTimeMillis ) / 1000.0f;

      /**
       *   Get the speed as the quota between the two.
       */ 

      float curSpeedCmPS = distanceCoveredCm / timeDiffSeconds;

      m_prevLat = realLat;
      m_prevLon = realLon;

      m_prevTimeMillis = curTimeMillis;
      
      return curSpeedCmPS;
   }

   std::vector<HintNode> NavTask::prepareInterpolationVector( int32 lat,
                                                              int32 lon )
   {   
      std::vector<HintNode> ret;
      
      float curSpeedCmPS = 0.0f;
      
#ifndef INTERPOLATION_USE_GPS_SPEED
      /**
       *   Calculate the speed using previous and current position.
       */
      curSpeedCmPs = getCalculatedSpeed();

      /**
       *   If no speed could be calculated, return an empty set.
       */
      
      if ( curSpeedCmPS == 0.0f ) {
         return ret;
      }
#else
      /**
       *   Just use the internal speed of the gps.
       */
      
      curSpeedCmPS = last_gps_speed * 100.0f;
#endif

      /**
       *   The first node of the vector is always the current
       *   GPS position and it's velocity
       */
      
      HintNode gpsHint;
      gpsHint.lat = lat;
      gpsHint.lon = lon;
      
      gpsHint.velocityCmPS = static_cast<int32>( curSpeedCmPS );

      ret.push_back( gpsHint );
         
      Point currentPoint = m_lastCurrSeg;
      Point nextPoint;
      
      // We don't want the first point since it
      // lies behind the current position.
      
      currentPoint.getNextPoint( &nextPoint );
      currentPoint = nextPoint;
      uint32 hintDistanceCm = 0;
      static const uint32 minHintDistanceCm = 300 * 100;
      HintNode prevHint;
      while( currentPoint.getNextPoint( &nextPoint ) &&
             hintDistanceCm < minHintDistanceCm )
      {
         HintNode hint;
         currentPoint.getCoord( hint.lat, hint.lon );
         
         ret.push_back( hint );
         if ( prevHint.hasValidPosition() ) {
            hintDistanceCm +=
               Nav2DistanceUtil::calcDistanceCentimeters( prevHint.lat,
                                                          prevHint.lon,
                                                          hint.lat,
                                                          hint.lon );
         }

         currentPoint = nextPoint;

         prevHint = hint;
      }

      return ret;
   }

   void NavTask::resetPtuiTimer()
   {
      int32 user_ptui = (0xbfffffff & m_user_ptui);
      int32 interval =  /* why doesn't the max function work? */
         (m_server_ptui > user_ptui) ? m_server_ptui : user_ptui;
      DBG("resetPtuiTimer:: user %"PRId32" min : server %"PRId32" min :"
          " timer %"PRId32" min", m_user_ptui, m_server_ptui, interval);
      if(user_ptui <= 0 || m_server_ptui <= 0 || !m_followingRoute || 
         interval > MAX_ALLOWED_PTUI_MINUTES){
         DBG("Stopping timer %"PRIu16, m_ptuiTimer);
         m_queue->stopTimer(m_ptuiTimer);
      } else {
         //convert to milliseconds.
         uint32 unterval = uint32(interval) * 60 * 1000;
         m_queue->setTimer(m_ptuiTimer, unterval);
      }
   }

   int32 NavTask::setUserPtuiParameter(int32 user_ptui, int32 server_ptui)
   {
      int32 calc_user_ptui = user_ptui;
      if((0x00ffffff & server_ptui) > (0x00ffffff & user_ptui)){
         calc_user_ptui = ((0xff000000 & user_ptui) | 
                           (0x00ffffff & server_ptui));
      }
      DBG("user_ptui: %"PRId32" (%#"PRIx32"), "
          "server_ptui: %"PRId32" (%#"PRIx32"), "
          "calc_user_ptui: %"PRId32" (%#"PRIx32")", 
          user_ptui, (0x00ffffff & user_ptui),
          server_ptui, (0x00ffffff & server_ptui),
          calc_user_ptui, calc_user_ptui);

      if ((m_user_ptui != calc_user_ptui) || (server_ptui > user_ptui)) {
         m_user_ptui = calc_user_ptui;
         DBG("User Ptui set to %"PRId32" == %"PRIx32, 
             m_user_ptui, uint32(m_user_ptui));
         m_parameter->setParam(ParameterEnums::NT_UserTrafficUpdatePeriod,
                               &calc_user_ptui);
      }
      if (!(0x40000000 & calc_user_ptui)) {
         resetPtuiTimer();
      }
      return calc_user_ptui;
   }

   void NavTask::setServerPtui(int32 ptui)
   {
      DBG("setServerPtui(%"PRId32")", ptui);
      m_server_ptui = ptui;
      setUserPtuiParameter(m_user_ptui, m_server_ptui);
   }

   void NavTask::setUserPtui(int32 ptui)
   {
      DBG("setUserPtui(%"PRId32")", ptui);
      setUserPtuiParameter(ptui, m_server_ptui);
   }

   void NavTask::HandleRouteToGpsRequest()
   {
      m_awaitingRouteReply = false;
      if (m_lastGpsQuality != QualityMissing && m_lastGpsQuality != QualityUseless) {
         // We have a gps signal good enough for "navigate to".
         m_nscRouteReqId = m_navServerCom->routeToGps(m_routeData.toLat, 
                                                      m_routeData.toLon, 
                                                      last_gps_head, 
                                                      m_routeData.fromLat, 
                                                      m_routeData.fromLon, 
                                                      m_routeData.routeRequestorId );
         m_awaitingRouteReply = true;
         m_internalRouteRequest = false;

      } else if (m_nbrRetries == 1) {
         // We have waited one timer run and have not yet
         // received any good signals. Send a solicited error
         // to the ui and continue to wait for gps signal.
         // Send a solicited error
         m_navTaskConsumer->solicitedError(Nav2Error::NSC_NO_GPS_WARN, m_routeData.routeRequestorId); 
         m_queue->setTimer(m_waitForSignalId, WAIT_FOR_GPS_SIGNAL_INTERVAL);
      } /*else if (m_nbrRetries == NBR_TIMER_RETRIES) {
         // We have tried max nbr of time wihout any success in
         // getting a good gps signal, it's time to send an
         // unsolicited error to the ui.
         unsolicitedError(Nav2Error::NSC_NO_GPS_ERR);
         } */
      else {
         // More times to try before giving up.
         m_queue->setTimer(m_waitForSignalId, WAIT_FOR_GPS_SIGNAL_INTERVAL);
      }
   }
   
   void NavTask::decodedProgressMessage(isab::NavServerComEnums::ComStatus status, 
                               GuiProtEnums::ServerActionType type,
                               uint32 done, uint32 of,
                               uint32 src, uint32 dst)
   {
      switch(status)
      {
      	case NavServerComEnums::connected:
      	case NavServerComEnums::sendingData:
      	case NavServerComEnums::downloadingData:
      	{
      	   m_canReroute = true;
      	   break;
      	}
      	case NavServerComEnums::disconnectionError:
      	case NavServerComEnums::unknownError:
      	case NavServerComEnums::connectionTimedOut:
      	{
      	   m_canReroute = false;
      	   break;
      	}
      	case NavServerComEnums::done:
      	case NavServerComEnums::invalid:
      	case NavServerComEnums::connecting:
      	{
      	   // We don't change the status of the connection in these cases.
      	   break;
      	}
      	default:
      	{
      	   break;
      	}
      }
   }

} /* namespace isab */

