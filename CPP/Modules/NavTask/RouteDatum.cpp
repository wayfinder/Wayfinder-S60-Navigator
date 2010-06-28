/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Log.h"
#include "RouteDatum.h"
#include <stdlib.h>



namespace isab{
   uint16 RouteDatum::getPointId(RouteDatum* start)
   {
      uint16 lll;
      if (!start) {
         /* Wrrorrrr! */
//          err("Wrrroorrr: get_point_id start is null!\n");
         return 0;
      }
      if (start > this) {
         /* Wrrorrrr! */
//          err("Wrrroorrr: get_point_id start beyond this point!\n");
         return 0;
      }
      
      lll = (this - start);
      return lll;
   }

   int16 RouteDatum::copyWaypoint(RouteDatum* dest)
   {
      if (!dest) {
//          err("Got null in RouteDatum::copyWaypoint!\n");
         return 0;
      }
      memcpy(dest, this, sizeof(*this));
      return 1;
   }

   MetaPointOrigo* RouteDatum::getOrigo(/*nav_route_datum_p rp*/)
   {
      RouteDatum *rp = this;
      while (!rp->actionIsStart()) {
         if (rp->actionIsOrigo()) {
            return rp->asMetaPointOrigoP();
         }
         rp--;
      }
      /* Didn't find it? Strange... */
//       err("Danger Will Robinson! No origo backwards!\n");
      return NULL;
   }

   void RouteDatum::fill(const uint8* rawData)
   {
      m_action = u16ntoh(rawData);
      if(m_action & META_MASK){
         switch(m_action){
            case ORIGO:
               this->asMetaPointOrigo().fill(rawData);
               break;
            case SCALE:
               this->asMetaPointScale().fill(rawData);
               break;
            case MINI_POINTS:
               this->asMiniPoints().fill(rawData);
               break;
            case MICRO_DELTA_POINTS:
               this->asMicroDeltaPoints().fill(rawData);
               break;
            case META:              
               this->asMetaPoint().fill(rawData);
               break;
            case TDL:              
               this->asMetaPointTimeDistLeft().fill(rawData);
               break;
            case LANDMARK:
               this->asMetaPointLandmark().fill(rawData);
               break;
            case LANE_INFO:
               this->asMetaPointLaneInfo().fill(rawData);
               break;
            case LANE_DATA:
               this->asMetaPointLaneData().fill(rawData);
               break;
            case SIGNPOST:
               this->asMetaPointSignPost().fill(rawData);
               break;
            default:
//               err("RouteDatum::fill - Unknown META action: %x", m_action);
               break;
         }
      } else {
         this->asRoutePoint().fill(rawData);
      }
   }

   namespace {
      const char* routeAction(RouteDatum::RouteActionType datum)
      {
         switch(datum){
            case RouteDatum::END_POINT:        return "END_POINT";
            case RouteDatum::START_POINT:      return "START_POINT";
            case RouteDatum::AHEAD:            return "AHEAD";
            case RouteDatum::LEFT:             return "LEFT";
            case RouteDatum::RIGHT:            return "RIGHT";
            case RouteDatum::UTURN:            return "UTURN";
            case RouteDatum::STARTAT:          return "STARTAT";
            case RouteDatum::FINALLY:          return "FINALLY";
            case RouteDatum::ENTER_RDBT:       return "ENTER_RDBT";
            case RouteDatum::EXIT_RDBT:        return "EXIT_RDBT";
            case RouteDatum::AHEAD_RDBT:       return "AHEAD_RDBT";
            case RouteDatum::LEFT_RDBT:        return "LEFT_RDBT";
            case RouteDatum::RIGHT_RDBT:       return "RIGHT_RDBT";
            case RouteDatum::EXITAT:           return "EXITAT";
            case RouteDatum::ON:               return "ON";
            case RouteDatum::PARK_CAR:         return "PARK_CAR";
            case RouteDatum::KEEP_LEFT:        return "KEEP_LEFT";
            case RouteDatum::KEEP_RIGHT:       return "KEEP_RIGHT";
            case RouteDatum::START_WITH_UTURN: return "START_WITH_UTURN";
            case RouteDatum::UTURN_RDBT:       return "UTURN_RDBT";
            case RouteDatum::FOLLOW_ROAD:      return "FOLLOW_ROAD";
            case RouteDatum::ENTER_FERRY:      return "ENTER_FERRY";
            case RouteDatum::EXIT_FERRY:       return "EXIT_FERRY";
            case RouteDatum::CHANGE_FERRY:     return "CHANGE_FERRY";
            case RouteDatum::ROAD_END_LEFT:    return "ROAD_END_LEFT";
            case RouteDatum::ROAD_END_RIGHT:   return "ROAD_END_RIGHT";
            case RouteDatum::DELTA:            return "DELTA";
            case RouteDatum::ACTION_MAX:       return "ACTION_MAX";
            default:                           return "UNKNOWN";
         }
      }
   }
   
   
   
   //=================================================
   //======= Functions for MetaPointLaneInfo ===========
   void MetaPointLaneInfo::fill(const uint8* rawData)
   {
      flags() = *rawData++;
      nbrLanes() = *rawData++;
      lane1() = (isab::Lane&)*rawData++;
      lane2() = (isab::Lane&)*rawData++;
      lane3() = (isab::Lane&)*rawData++;
      lane4() = (isab::Lane&)*rawData++;
      distance() = s32ntoh(rawData);
   }

   uint8& MetaPointLaneInfo::flags()
   {
      return m_data[0];
   }

   uint8& MetaPointLaneInfo::nbrLanes()
   {
      return m_data[1];
   }

   Lane& MetaPointLaneInfo::getlane( int idx )
   {
      return (isab::Lane&)m_data[2+idx];
   }

   Lane& MetaPointLaneInfo::lane1()
   {
      return (isab::Lane&)m_data[2];
   }

   Lane& MetaPointLaneInfo::lane2()
   {
      return (isab::Lane&)m_data[3];
   }

   Lane& MetaPointLaneInfo::lane3()
   {
      return (isab::Lane&)m_data[4];
   }

   Lane& MetaPointLaneInfo::lane4()
   {
      return (isab::Lane&)m_data[5];
   }

   int32& MetaPointLaneInfo::distance()
   {
      return reinterpret_cast<int32*>(m_data+2)[1];
   }

   bool MetaPointLaneInfo::stopOfLanes()
   {
      return (flags() & 0x1) != 0;
   }

   bool MetaPointLaneInfo::remindOfLanes()
   {
      return (flags() >> 1 & 0x1) != 0;
   }

   //=================================================
   //============= MetaPointLaneData =================
   void MetaPointLaneData::fill(const uint8* rawData)
   {
      lane1() = (isab::Lane&)*rawData++;
      lane2() = (isab::Lane&)*rawData++;
      lane3() = (isab::Lane&)*rawData++;
      lane4() = (isab::Lane&)*rawData++;
      lane5() = (isab::Lane&)*rawData++;
      lane6() = (isab::Lane&)*rawData++;
      lane7() = (isab::Lane&)*rawData++;
      lane8() = (isab::Lane&)*rawData++;
      lane9() = (isab::Lane&)*rawData++;
      lane10() = (isab::Lane&)*rawData++;
   }

   Lane& MetaPointLaneData::getlane( int idx )
   {
      return (isab::Lane&)m_data[idx];
   }

   Lane& MetaPointLaneData::lane1()
   {
      return (isab::Lane&)m_data[0];
   }

   Lane& MetaPointLaneData::lane2()
   {
      return (isab::Lane&)m_data[1];
   }

   Lane& MetaPointLaneData::lane3()
   {
      return (isab::Lane&)m_data[2];
   }

   Lane& MetaPointLaneData::lane4()
   {
      return (isab::Lane&)m_data[3];
   }

   Lane& MetaPointLaneData::lane5()
   {
      return (isab::Lane&)m_data[4];
   }

   Lane& MetaPointLaneData::lane6()
   {
      return (isab::Lane&)m_data[5];
   }

   Lane& MetaPointLaneData::lane7()
   {
      return (isab::Lane&)m_data[6];
   }

   Lane& MetaPointLaneData::lane8()
   {
      return (isab::Lane&)m_data[7];
   }

   Lane& MetaPointLaneData::lane9()
   {
      return (isab::Lane&)m_data[8];
   }

   Lane& MetaPointLaneData::lane10()
   {
      return (isab::Lane&)m_data[9];
   }

   //=========================================================
   //==================== Lane ===============================
   bool Lane::isPreferred()
   {
      return (m_lane >> 7 & 0x1) != 0;
   }

   bool Lane::isAllowed()
   {
      return (m_lane >> 6 & 0x1) == 0;
   }

   Lane::Direction Lane::direction()
   {
      return (Lane::Direction)(m_lane & 0x1F);
   }

   //=================================================
   //======= MetaPointSignPost ===========
   inline void MetaPointSignPost::fill(const uint8* rawData)
   {
      textIndex() = s16ntoh(rawData);
      textColor() = *rawData++;
      backgroundColor() = *rawData++;
      frontColor() = *rawData++;
      distance() = u32ntoh(rawData);
   }
   inline uint16& MetaPointSignPost::textIndex()
   {
      return reinterpret_cast<uint16*>(m_data)[0];
   }
   inline uint8& MetaPointSignPost::textColor()
   {
      return m_data[2];
   }
   inline uint8& MetaPointSignPost::backgroundColor()
   {
      return m_data[3];
   }
   inline uint8& MetaPointSignPost::frontColor()
   {
      return m_data[4];
   }
   int32& MetaPointSignPost::distance()
   {
      return reinterpret_cast<int32*>(m_data+2)[1];
   }

}


