/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * Attach to a serial connection and decode Nmea gps data
*/

#include "arch.h"
#include <math.h>

#ifndef M_PI
#define M_PI      3.14159265358979323846
#endif

#define PASTE2(x,y) x##y
#define PASTE(x,y) PASTE2(x,y)
#define M_PI_F (PASTE(M_PI,f))

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "Gps.h"
#include "Log.h"
#include "GpsParser.h"
#include "MotorolaParser.h"

#define ASSUMED_LATENCY 1.7

namespace isab {


/*+ Define to handle and show packets only interesting while debugging
    the protocol/navigator interaction. +*/
#define NMEA_SHOW_DEBUGPACKETS
/*+ Define to output a lot of info on received packets on the debug port +*/
#define NMEA_SHOW_DEBUGTEXTS

#define NMEA_DEBUG_PAD_PACKETS

/*+ Show a lot of information on _all_ received frames */
#undef DRIV_NMEA_DEBUG_ALL_INPUT

#ifdef NMEA_DEBUG_PAD_PACKETS
# define DEBUG_BAD_PACKETS m_log->debug
#else
# define DEBUG_BAD_PACKETS m_log->dropIt
#endif

#ifdef NMEA_SHOW_DEBUGTEXTS
# define DEBUG_PKT m_log->debug
#else
# define DEBUG_PKT m_log->dropIt
#endif


/* ********************************************************
 * Setup routines
 * ********************************************************/


MotorolaParser::MotorolaParser(SerialProviderPublic *p, Log *l)
   : GpsParser(p, l)
{
   state = state_pre_system_startup;
}


void MotorolaParser::connection_ready(void)
{
   setup();
}

void MotorolaParser::setConsumer(GpsConsumerPublic *c)
{
   m_consumer = c;
}


void MotorolaParser::sendStatus()
{
   isabTime t;
   enum GpsStatus sum_status;
   int error_conditions;

#ifdef NMEA_SHOW_DEBUGTEXTS
   m_log->debug("%s","MotorolaParser::calculate_status called\n");
#endif
   if ( state < state_idle  ) {
      sum_status=os_gps_fatal;
      error_conditions=0;
   } else {
      if (m_validSolution) {
         //if (m_doing3d) {
         //   sum_status = os_gps_ok_3d;
         //} else {
            sum_status = os_gps_ok_2d;
         //}
      } else {
         sum_status = os_gps_too_few_sats;
      }

      error_conditions = 0;
   }
   if (m_consumer) {
      m_consumer->gpsStatus(t, sum_status, error_conditions, 0);
   }
#ifdef NMEA_SHOW_DEBUGTEXTS
   m_log->debug("%s","MotorolaParser::calculate_status has sent\n");
#endif
}




void MotorolaParser::interpret_packet()
{
   isabTime t;

   if ( (reassembly_ptr - reassmebled_pkt) < 94) {
      m_log->debug("Too short packet, ignoring");
      return;
   }

   const float scaledPi = ( M_PI / 180.0 / 6000000.0);
   int32 quality = readNDigitsInt(reassmebled_pkt + 26, 10);
   float lat = readNDigitsInt(reassmebled_pkt + 37, 10) * scaledPi;
   float lon = readNDigitsInt(reassmebled_pkt + 48, 10) * scaledPi;
   float alt = readNDigitsInt(reassmebled_pkt + 59, 10);
   float speed = readNDigitsInt(reassmebled_pkt + 70, 10) / 1000.0;
   float heading = readNDigitsInt(reassmebled_pkt + 81, 10) * scaledPi;

   bool oldValidSolution = m_validSolution;
   if (!m_validSolution) {
      m_validSolution = true;
   }
   if (quality == 0) {
      m_validSolution = false;
   }
   if (m_validSolution != oldValidSolution) {
      sendStatus();
   }
   if (m_consumer && m_validSolution) {
      GpsConsumerPublic::GpsPositionVelocityVector v;
      v.latitude          = lat;
      v.longitude         = lon;
      v.altitude          = alt;
      v.heading           = (uint8)(heading * (256.0 / (M_PI * 2)));
      v.speed_over_ground = speed;
      v.vertical_speed    = 0;
      v.weekno_for_fix    = 0;
      v.tow_for_fix       = 0; /* FIXME */
      v.fix_time          = t;
      v.constellation_no  = 0;
      v.latency           = static_cast<float>(ASSUMED_LATENCY);
      m_consumer->gpsPositionVelocity(v);
   }
}


int MotorolaParser::receiveData(const uint8 *indata, int len)
{
  char databyte;  /* Byte being examined */
  int numDecodedPackets = 0;

  if (state == state_pre_system_startup) {
#ifdef DRIV_NMEA_DEBUG_ALL_INPUT
     m_log->debug("Dropped data since not initialized: %i bytes", len);
#endif
     return -1;
  }

  while (len) {
    databyte=*indata;
#ifdef DRIV_NMEA_DEBUG_ALL_INPUT
    m_log->debug("rd: 0x%08x, 0x%02x", int(state), databyte);
#endif
    switch (state)  {
      case state_idle:
        if (databyte=='^') {
          state=state_in_packet;
          reassembly_ptr = reassmebled_pkt;
        }
        break;
      case state_in_packet:
        if (databyte == '\r' || databyte == '\n' || databyte == 0 ) {
           // Checksum done, handle the packet
           ++numDecodedPackets;
           interpret_packet();
           state=state_idle;
        }
        if ((reassembly_ptr - reassmebled_pkt) >= 
            int(sizeof(reassmebled_pkt)) ) {
          state=state_idle;  /* Oversized packet, drop it */
          DEBUG_BAD_PACKETS("%s","Oversized packet!\n");
          break;
        }
        *(reassembly_ptr++)=databyte;
        break;
    case state_pre_system_startup:
       m_log->error("receiveData: state should never be pre_system_startup");
       return 0; //XXX do something meningful here.
    }
    indata++;
    len--;
  }
  
  return numDecodedPackets;
}


int MotorolaParser::setup()
{
  /* Clear lots of status variables. */
  //last_lat = 0;
  //last_lon = 0;
  //last_pdop = 0;
  unknown_packets = 0;
  bad_packets = 0;
  wrong_length = 0;

  state=state_idle;

  m_validSolution = false;
                            
  return 0;    /* Return success */
}



} /* namespace isab */

