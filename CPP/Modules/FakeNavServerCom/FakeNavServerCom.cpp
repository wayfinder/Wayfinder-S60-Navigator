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
#include "MsgBufferEnums.h"
#include "Module.h"
#include "CtrlHub.h"

#include "NavPacket.h"
#include "GuiProt/ServerEnums.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "NavServerCom.h"

#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "RouteEnums.h"
#include "NavTask.h"
/* #include "ErrorModule.h" */

#include "ParameterEnums.h"
#include "Parameter.h"

#include "nav2util.h"

#include "Faker.h"

/* #include "Constants.h" */
/* #include "Nav2Error.h" */
/* #include "TopRegion.h" */
#include "GlobalData.h"

/* #include "ServerParser.h" */
#include "FakeNavServerCom.h"
/* #include "GuiProt/SearchRegion.h" */
#include "GuiProt/MapClasses.h"

// defines all log macros such as DBG
#include "LogMacros.h"

#define NAV_SERVER_COM_HEADER_SIZE 14
#define NAV_ROUTE_REPLY_HEADER_SIZE 35
#define NAV_CTRL_BUFFER_ROUTE_BYTES 15000

using namespace isab::Nav2Error;

namespace isab {
   FakeNavServerCom::FakeNavServerCom() :
      Module("FakeNavServerCom"),
      m_navTaskProvider(NULL), 
      m_nscConsumer(NULL)
   {
      //Create necessary XYZPublic objects.
      m_navTaskProvider = new NavTaskProviderPublic(this);
      m_nscConsumer     = new NavServerComConsumerPublic(this);
      m_paramProvider   = new ParameterProviderPublic(this);
      m_faker           = new Faker();
      m_faker->setInterface(this);
   }

   FakeNavServerCom::~FakeNavServerCom()
   {
      delete m_navTaskProvider;
      delete m_nscConsumer;
      delete m_paramProvider;
      delete m_faker;
   }

   MsgBuffer* FakeNavServerCom::dispatch(MsgBuffer* buf)
   {
      if(buf) buf = m_comDecoder.dispatch(buf, m_faker);
      if(buf) buf = m_ctrlHubAttachedDecoder.dispatch(buf, this);
      if(buf) buf = m_paramDecoder.dispatch(buf, this);
      if(buf) buf = Module::dispatch(buf);
      return buf;
   }

   void FakeNavServerCom::sendError( const ErrorObj& err, 
                                     uint32 src ) const
   {
      if ( src == 0 ) {
         unsolicitedError( err );
      } else {
         m_nscConsumer->solicitedError( err, src );
      }
   }

   void FakeNavServerCom::decodedStartupComplete()
   {
      Module::decodedStartupComplete();
      // Parameters we need and will subscribe to.
      ParameterEnums::ParamIds params[] = {ParameterEnums::NSC_ServerHostname, 
                                ParameterEnums::NSC_UserAndPasswd, 
                                ParameterEnums::NSC_TransportationType,
                                ParameterEnums::NSC_RouteHighways,
                                ParameterEnums::NSC_RouteTollRoads,
                                ParameterEnums::NSC_RouteCostType};
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
   }

   void FakeNavServerCom::decodedExpiredTimer(uint16 timerID)
   {
      if(m_externalTimers.count(timerID) > 0){
         m_externalTimers[timerID]->timerExpired(timerID);
         m_externalTimers.erase(timerID);
      } else {
         WARN("Unknown timer %u timed out!", timerID);
      }
      Module::decodedExpiredTimer(timerID);
   }

   uint16 FakeNavServerCom::setTimer(int32 timeout, 
                                     class NavServerComTimerCallBack* callback)
   {
      uint16 id = m_queue->defineTimer(timeout);
      if(id != 0xffff){
         m_externalTimers[id] = callback;
      }
      return id;
   }



   void FakeNavServerCom::decodedParamNoValue(uint32 uParam, uint32 /*src*/, 
                                              uint32 /*dst*/)
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
         charval[num++] = "nav-wf-prod-1.services.wayfinder.biz:9655";
         charval[num++] = "nav-wf-prod-2.services.wayfinder.biz:9655";
         {for(int i = 0; i < num; ++i){
            WARN("Server hostname set to default value: %s", charval[i]);
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
      case ParameterEnums::TopRegionList:
         WARN("Top Region Checksum value set to 0x%"PRIx32, intval[0]);
         type = noParam; //dont write to parammodule.
         break;
      case ParameterEnums::NSC_TransportationType:
         intval[num++] = NavServerComEnums::passengerCar;
         WARN("Transportation type set to passenger car.");
         break;
      case ParameterEnums::NSC_RouteCostType:
         intval[num++] = NavServerComEnums::TIME;
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
         intval[num++] = 0;
         WARN("Parameter %#x set to 0", param);
         break;
      case ParameterEnums::NSC_ExpireVector:
         DBG("Expire vector set to 3 x MAX_INT32");
         intval[num++] = MAX_INT32;
         intval[num++] = MAX_INT32;
         intval[num++] = MAX_INT32;
         break;
      default:
         WARN("Unknown parameter %#x", param);
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
} /* namespace isab */




