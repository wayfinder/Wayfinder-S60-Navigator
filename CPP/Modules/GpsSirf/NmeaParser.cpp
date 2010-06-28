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

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "Gps.h"
#include "Log.h"
#include "GpsParser.h"
#include "NmeaParser.h"

#include "GpsSatelliteInfo.h"

#define ASSUMED_LATENCY 0.7

namespace isab {


#define KNOTS_TO_METERS_PER_S 0.51444444


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


NmeaParser::NmeaParser(SerialProviderPublic *p, Log *l)
   : GpsParser(p, l)
{
   m_satInfo = 0;
   state = state_pre_system_startup;
}

NmeaParser::~NmeaParser()
{
   delete m_satInfo;
   m_satInfo = 0;
}


void NmeaParser::connection_ready(void)
{
   setup();
}

void NmeaParser::setConsumer(GpsConsumerPublic *c)
{
   m_consumer = c;
}

/** Complete an NMEA packet and send it. Call this function 
 * with a buffer of the format "$GPGLL,12,N,34,E,,456", i.e.
 * with the initial dollar sign but without the trailing 
 * asterisk, checksum and cr/lf.
 *
 * @param input buffer holding the start of the packet.
 *        The pointer is invalid after this function 
 *        returns.
 */
int NmeaParser::send_pkt(Buffer *buf)
{
   int bufLen;
   uint8 csum;

#ifdef NMEA_SHOW_DEBUGTEXTS
  m_log->debug("%s","NmeaParser::send_pkt called\n");
#endif
  buf->setReadPos(1);   /* Skip initial $ */
  bufLen = buf->getLength() - 1;
  csum=0;
  while (bufLen > 0) {
     csum ^= buf->readNext8bit();
     bufLen--;
  }
  buf->setReadPos(0);   /* Prepare for sendData */

  buf->nprintf(5, "*%02x\r\n", csum);

  m_serial->sendData(buf);

  return 1;
}


void NmeaParser::sendStatus()
{
   isabTime t;
   enum GpsStatus sum_status;
   int error_conditions;

#ifdef NMEA_SHOW_DEBUGTEXTS
   m_log->debug("%s","NmeaParser::calculate_status called\n");
#endif
   if ( state < state_idle  ) {
      sum_status=os_gps_fatal;
      error_conditions=0;
   } else {
      if (m_validSolution) {
         if (m_doing3d) {
            sum_status = os_gps_ok_3d;
         } else {
            sum_status = os_gps_ok_2d;
         }
      } else {
         sum_status = os_gps_too_few_sats;
      }

      error_conditions = 0;
   }
   if (m_consumer) {
      m_consumer->gpsStatus(t, sum_status, error_conditions, last_pdop);
   }
#ifdef NMEA_SHOW_DEBUGTEXTS
   m_log->debug("%s","NmeaParser::calculate_status has sent\n");
#endif
}



float NmeaParser::nmeaTimeToSeconds(char *s)
{
   if (strlen(s) < 10) {
      DEBUG_BAD_PACKETS("Invalid time length, %i characters", strlen(s));
      return 0.0;
   }
   return float(readNDigitsInt(s, 2) * 3600 + readNDigitsInt(s+2, 60)) + 
          readFloat(s+4);
}


int32 NmeaParser::nmeaDateToDays(char *s)
{
   int year, month, day;

   if (strlen(s) < 6) {
      DEBUG_BAD_PACKETS("Invalid date length, %i characters", strlen(s));
      return 0;
   }
   year  = readNDigitsInt(s+0, 2);
   month = readNDigitsInt(s+2, 2);
   day   = readNDigitsInt(s+4, 2);

   return 0;
}

int NmeaParser::nmeaReadLatitude(char *s, float &latitude)
{
   if (strlen(s) < 4) {
      DEBUG_BAD_PACKETS("Invalid latitude length, %i characters", strlen(s));
      return 0;
   }
   latitude = (float(readNDigitsInt(s, 2) * 60) + readFloat(s+2)) * float(M_PI / (180 * 60));
   return 1;
}

int NmeaParser::nmeaReadLongitude(char *s, float &longitude)
{
   if (strlen(s) < 4) {
      DEBUG_BAD_PACKETS("Invalid llongitude length, %i characters", strlen(s));
      return 0;
   }
   longitude = (float(readNDigitsInt(s, 3) * 60) + readFloat(s+3)) * float(M_PI / (180 * 60));
   return 1;
}

const int NmeaParser::hexToInt(char c)
{
   if ( c>='0' && c<='9') {
      return c-'0';
   } else if ( c>='a' && c<='f') {
      return c-'a'+10;
   } else if ( c>='A' && c<='F') {
      return c-'A'+10;
   } else {
      return 0;
   }
}


/* **********************************************************************
 * Routines named packet_XXX decode an NMEA packet 
 * named XX (hex) and create a canoical packet from the information.
 * Some of the incoming NMEA packets do not create gps-protocol packets
 * but have other effects. 
 ********************************************************************** */

void NmeaParser::packet_GGA(int /*num_params*/, char *params[])
{
   float altitude;

   if (params[6][0] == '0') {
      m_validSolution = false;
      DEBUG_PKT("packet_GGA - Packet marked no fix");
      return;
   }
   if (pdopTooHigh()) {
      DEBUG_PKT("packet_GGA - pdop too high");
      return;
   }

   altitude = readFloat(params[9]);

   last_alt = altitude;
}

void NmeaParser::packet_GSV(int num_params, char *params[])
{
   /*
    *
    *  GSV - Satellites in view
    *
    *  These sentences describe the sky position of a UPS satellite in view.
    *  Typically they're shipped in a group of 2 or 3.
    *
    *          1 2 3 4 5 6 7     n
    *          | | | | | | |     |
    *   $--GSV,x,x,x,x,x,x,x,...*hh<CR><LF>
    *
    *  Field Number:
    *  1) total number of messages
    *  2) message number
    *  3) satellites in view
    *  4) satellite number
    *  5) elevation in degrees (0-90)
    *  6) azimuth in degrees to true north (0-359)
    *  7) SNR in dB (0-99)
    *  more satellite infos like 4)-7)
    *  n) checksum
    *
    *  Example:
    *  $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
    *  $GPGSV,3,2,11,14,25,170,00,16,57,208,39,18,67,296,40,19,40,246,00*74
    *  $GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D
    *
    */
   /* GSV can come in groups. */
   int32 numMsgs = readInt32(params[1]);
   int32 seqNum = readInt32(params[2]);
   if (seqNum == 1) {
      if (m_satInfo) {
         delete m_satInfo;
      }
      m_satInfo = new GpsSatInfoHolder();
      m_satInfoSeqId = 0;
   }
   if (m_satInfoSeqId + 1 != seqNum) {
      /* Out of order, abort. */
      return;
   } else {
      m_satInfoSeqId = seqNum;
   }
   uint32 numSat = readInt32(params[3]);

   int32 index = 4;

   while (index <= (num_params-3)) {
      int32 id    = readInt32(params[index++]);
      int32 elev  = readInt32(params[index++]);
      int32 azim  = readInt32(params[index++]);
      int32 snr   = readInt32(params[index++]);

      GpsSatInfo* info = new GpsSatInfo( id, elev, azim, snr );
      m_satInfo->addInfo(info);
   }

   if (seqNum == numMsgs) {
      /* Last message in sequence. */
      if (m_satInfo->size() != numSat) {
         DEBUG_PKT("packet_GSV - mismatch in sat num!");
      }
      if (m_consumer) {
         m_consumer->gpsSatInfo(m_satInfo);
      }

      delete m_satInfo;
      m_satInfo = 0;
      m_satInfoSeqId = 0;
   }
}


void NmeaParser::packet_RMC(int /*num_params*/, char *params[])
{
   isabTime t;
   float timeOfDay;
   float latitude, longitude, altitude, tow;
   float speed, heading;
   int32 day;

   if (params[2][0] == 'V') {
      m_validSolution = false;
      DEBUG_PKT("packet_RMC - Packet marked invalid");
      return;
   }
   if (pdopTooHigh()) {
      DEBUG_PKT("packet_RMC - pdop too high");
      return;
   }
   if (params[2][0] != 'A') {
      DEBUG_PKT("packet_RMC - Unknown validity marker");
      return;
   }
   timeOfDay = nmeaTimeToSeconds(params[1]);
   day = nmeaDateToDays(params[9]);

   if (! nmeaReadLatitude(params[3], latitude)) {
      DEBUG_PKT("packet_RMC - Broken latitude");
      return;
   }
   if (params[4][0] == 'S') {
      latitude = -latitude;
   } else if (params[4][0] != 'N') {
      DEBUG_PKT("packet_RMC - Neither N nor S");
      return;
   }

   if (! nmeaReadLongitude(params[5], longitude)) {
      DEBUG_PKT("packet_RMC - Broken longitude");
      return;
   }
   if (params[6][0] == 'W') {
      longitude = -longitude;
   } else if (params[6][0] != 'E') {
      DEBUG_PKT("packet_RMC - Neither E nor W");
      return;
   }

   speed = readFloat(params[7]) * float(KNOTS_TO_METERS_PER_S);
   heading = readFloat(params[8]) * (256.0 / 360.0);

   altitude = 0;
   tow = 0;
   
   last_lat = latitude;
   last_lon = longitude;
   /* XXX Don't reset altitude. */
/*    last_alt = altitude; */
   last_speed = speed;

   // Preserve the last heading if the speed is too low ti give a 
   // reliable heading from the doppler data.
   if (speed > 1) {  // XXX heading filtering should probably not be done here.
      last_heading = heading;
   }

   if (!m_validSolution) {
      m_validSolution = true;
      sendStatus();
   }
   if (m_consumer) {
      GpsConsumerPublic::GpsPositionVelocityVector v;
      v.latitude          = last_lat;
      v.longitude         = last_lon;
      v.altitude          = last_alt;
      v.heading           = (uint8)last_heading;
      v.speed_over_ground = last_speed;
      v.vertical_speed    = 0;
      v.weekno_for_fix    = 0;
      v.tow_for_fix       = tow;
      v.fix_time          = t;
      v.constellation_no  = 0;
      v.latency           = static_cast<float>(ASSUMED_LATENCY);
      m_consumer->gpsPositionVelocity(v);
   }
}

void NmeaParser::packet_GSA(int /*num_params*/, char *params[])
{
   if (params[2][0] == '2') {
      m_doing3d = false;
   } else if (params[2][0] == '3') {
      m_doing3d = true;
   } else {
      m_validSolution = false;
   }

   last_pdop = readFloat(params[15]);

   if (pdopTooHigh()) {
      m_validSolution = false;
   }

   if (m_consumer) {
      sendStatus();
   }
}

const NmeaParser::handlerListElement_t NmeaParser::paramHandlers[] = {
   { "RMC", 11, &NmeaParser::packet_RMC },
   { "GSA", 17, &NmeaParser::packet_GSA },
   { "GGA", 14, &NmeaParser::packet_GGA },
   { "GSV", 3, &NmeaParser::packet_GSV },
   { "", 0, NULL }
};

void NmeaParser::packet_unknown(char *packet)
{
   DEBUG_PKT("Got an unknown packet: %s", packet);
}


void NmeaParser::interpret_packet()
{
   int numParams;
   char *params[max_nmea_params];
   char *s = reassmebled_pkt;
   int i;

   /* basically an strtok implementation */
   params[0]=s;
   numParams=1;
   while ((s < reassembly_ptr) && (numParams < max_nmea_params)) {
      if (*s == ',') {
         *s=0;
         params[numParams]=s+1;
         ++numParams;
      }
      ++s;
   }
   // Zero-terminate the last parameter.
   *reassembly_ptr=0;
   DEBUG_PKT("Decoded %i parameters", numParams);
   DEBUG_PKT("Packet data: %s", reassmebled_pkt);

   /* Try to find a matching handler function 
    * (Do not match on sender, only on type (i.e. skip the first two letters */
   if (strlen(params[0]) < 3) {
      DEBUG_BAD_PACKETS("Too short command name");
      return;
   }
   if (params[0][0] == 'P') {
      DEBUG_PKT("Got proprietary message - ignoring it: %s", params[0]);
      return;
   }
   i=0;
   while (paramHandlers[i].id[0] != 0) {
      if (strcmp(params[0]+2, paramHandlers[i].id) == 0) {
         // Found a handler - check number of parameters
         if (numParams < paramHandlers[i].minParams) {
            DEBUG_BAD_PACKETS("Too few parameters to %s", params[0]);
            return;
         }
         (this->*paramHandlers[i].handler)(numParams, params);
         return;
      }
      ++i;
   }
   DEBUG_PKT("Got unknown message : %s", params[0]);
}


int NmeaParser::receiveData(const uint8 *indata, int len)
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
        if (databyte=='$') {
          state=state_in_packet;
          reassembly_ptr = reassmebled_pkt;
          calculated_checksum = 0;
        }
        break;
      case state_in_packet_after_star:
        /* Calculate the checksum */
        if (databyte == '\r' || databyte == '\n') {
           // Checksum done, handle the packet
           ++numDecodedPackets;
           if ( !checksum_present || (checksum_from_pkt == calculated_checksum) ) {
              interpret_packet();
           } else {
              DEBUG_BAD_PACKETS("Broken nmea checksum, %02x!=%02x\n", checksum_from_pkt, calculated_checksum);
           }
           state=state_idle;
        } else {
           checksum_from_pkt = (checksum_from_pkt << 4) + hexToInt(databyte);
           checksum_present=1;
        }
        break;
      case state_in_packet:
        if (databyte=='*') {
           state=state_in_packet_after_star;
           checksum_from_pkt = 0;
           checksum_present=0;
          break;
        }
        if ((reassembly_ptr - reassmebled_pkt) >= 
            int(sizeof(reassmebled_pkt)) ) {
          state=state_idle;  /* Oversized packet, drop it */
          DEBUG_BAD_PACKETS("%s","Oversized packet!\n");
          break;
        }
        *(reassembly_ptr++)=databyte;
        calculated_checksum ^= databyte;
        break;
    case state_pre_system_startup:
       m_log->error("receiveData: state should never be pre_system_startup");
       return 0; //XXX dom something meningful here.
    }
    indata++;
    len--;
  }
  
  return numDecodedPackets;
}


void NmeaParser::reset_navigator()
{
}

int NmeaParser::setup()
{
   /* Clear lots of status variables. */
   last_lat = 0;
   last_lon = 0;
   last_heading = 0;
   last_speed = 0;
   last_pdop = 0;
   last_alt = 0;
   unknown_packets = 0;
   bad_packets = 0;
   wrong_length = 0;

   delete m_satInfo;
   m_satInfo = 0;
   m_satInfoSeqId = 0;

   state=state_idle;

   m_validSolution = false;
   m_doing3d = false;

   /* Initiate global to invalid value. */
   last_weekno = -1;
   last_tow    = -1;

   reset_navigator();

   return 0;    /* Return success */
}



} /* namespace isab */

