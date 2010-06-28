/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/* This is the interface between a gps module and a module that needs
 * to receive gps positions etc.
 */

#ifndef MODULE_Gps_H
#define MODULE_Gps_H

#include <vector>

namespace isab {

enum GpsStatus {
  os_gps_fatal=-1, 
  os_gps_not_init=0, 
  os_gps_no_time,  
  os_gps_no_almanac,
  os_gps_too_few_sats, 
  os_gps_ok_2d, 
  os_gps_ok_3d, 
  os_gps_demo_hx, 
  os_gps_demo_1x,
  os_gps_demo_2x, 
  os_gps_demo_4x
};

#define OS_GPS_ERROR_ANTENNA 0x01   /*+ No antenna found +*/
#define OS_GPS_ERROR_BATTERY 0x02   /*+ Lost battery backed memory +*/
#define OS_GPS_ERROR_PDOP    0x04   /*+ Too high pdod prevents positioning +*/
#define OS_GPS_ERROR_CONFIG  0x08   /*+ Bad configuration of the gps receiver +*/
#define OS_GPS_ERROR_MISSING 0x10   /*+ Receiver is missing or not turned on +*/

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * GpsConsumerInterface. */
class GpsConsumerPublic : public ModulePublic {
   public:
      GpsConsumerPublic(ModuleQueue *peerQueue) : 
         ModulePublic(peerQueue) { };

      struct GpsPositionVelocityVector {
         float latitude;
         float longitude;
         float altitude;
         uint8 heading;
         float speed_over_ground;
         float vertical_speed;
         int16 weekno_for_fix;
         float tow_for_fix;
         isabTime fix_time;
         int16 constellation_no;
         float latency;
      };

      virtual uint32 gpsPositionVelocity(
            const struct GpsPositionVelocityVector &v,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

      virtual uint32 gpsStatus(
            isabTime t,
            enum GpsStatus sum_status,
            uint16 error_conditions,
            float pdop,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

      virtual uint32 gpsSatInfo(
            class GpsSatInfoHolder* info,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

      virtual uint32 gpsTime(
            isabTime t, 
            float tow,
            int weekno,
            float utc_offset,
            uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
};

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * GpsProviderInterface. */
class GpsProviderPublic : public ModulePublic {
   public:
      GpsProviderPublic(ModuleQueue *peerQueue) : 
         ModulePublic(peerQueue) { };
      virtual uint32 connect(GpsConsumerPublic *consumer);
      virtual uint32 resetReceiver(uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
};




/* Next are the interfaces. These should be implemented by the 
 * module that wishes to receive the corresponding messages. 
 * The classes below are pure virtual and correspond in function
 * to "interfaces" in java. */

/** An interface that must be implemented to receive messages in a
 * Gps provider-consumer relationship. This is implemented 
 * in the _upper_ module.
 */
class GpsConsumerInterface {
   public:
      virtual void decodedGpsPositionVelocity(
                          const struct GpsConsumerPublic::GpsPositionVelocityVector &v,
                          uint32 src) = 0;
      virtual void decodedGpsStatus(isabTime t, 
                           enum GpsStatus sum_status,
                           uint16 error_conditions,
                           float pdop,
                           uint32 src) = 0;
      virtual void decodedGpsSatelliteInfo(
                           class GpsSatInfoHolder* satInfo,
                           uint32 src) = 0;
      virtual void decodedGpsTime(isabTime t, 
                          float tow,
                          int weekno,
                          float utc_offset,
                          uint32 src) = 0;
};

/** An interface that must be implemented to receive messages in a
 * Gps provider-consumer relationship. This is implemented 
 * in the _lower_ module.
 */
class GpsProviderInterface {
   public:
      virtual void decodedResetReceiver(uint32 src) = 0;
};

/** An object of this class is owned by each module that implements 
 * the GpsConsumerInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in GpsConsumerInterface.
 */
class GpsConsumerDecoder {
   public:
      MsgBuffer * dispatch(MsgBuffer *buf, GpsConsumerInterface *m );
};

/** An object of this class is owned by each module that implements 
 * the GpsProviderInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in GpsProviderInterface.
 *
 * @return the buffer if further processing may be done or NULL if 
 *         the buffer was processed and either passed on or 
 *         destroyed and thus no further processing should be done.
 */
class GpsProviderDecoder {
   public:
      MsgBuffer * dispatch(MsgBuffer *buf, GpsProviderInterface *m );
};

} /* namespace isab */

#endif /* MODULE_Gps_H */
