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

  (module) driv/nav/driv_nav_trimble

  Attach to a serial connection and decode data from a trimble TSIP
  gps receiver.

  Routines named driv_nav_trimble_packet_XX decode a trimble packet 
  numbered XX (hex) and create a canoical packet from the information.
  Some of the incoming TSIP packets do not create gps-protocol packets
  but have other effects. 
*/

#include "arch.h"
#include <math.h>

#include "FastTrig.h"

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "Gps.h"
#include "Log.h"
#include "GpsTrimble.h"

#define ASSUMED_LATENCY 0.1

namespace isab {

isabTime GpsTrimble::TrimbleParser::time_from_gps(int /*weekno*/, float /*tow*/)
{
   isabTime t;
   return t;
}



/*+ Define to always request the signal levels after a 
    position (4A) packet +*/
#undef DRIV_NAV_TRIMBLE_ALWAYS_REQUEST_SIGNAL_LEVELS

#define DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS

#define DLE 0x10
#define ETX 0x03


/* ********************************************************
 * Setup routines
 * ********************************************************/


GpsTrimble::TrimbleParser::TrimbleParser(SerialProviderPublic *p, Log *l)
   : m_serial(p), m_consumer(NULL), m_log(l)
{
   state = state_pre_system_startup;
}


void GpsTrimble::TrimbleParser::connection_ready(void)
{
   setup();
}

void GpsTrimble::TrimbleParser::setConsumer(GpsConsumerPublic *c)
{
   m_consumer = c;
}

/* FIXME - We do not check if the output buffer overflows */
int GpsTrimble::TrimbleParser::send_pkt(uint8 *data, int len)
{
  uint8 encapsulatedData[max_tsip_packet_size];
  uint8 *pktdata;  /* Insertion pointer into data area of buf */

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_send_pkt called\n");
#endif
  pktdata=encapsulatedData;
  *pktdata++=DLE;    /* Start of packet. The packet type is 
                                   the first byte of the data parameter */
  while (len--) {
    if (*data == DLE) {    /* Duplicate DLE to escape it */
      *pktdata++ = *data;
    }
    *pktdata++ = *data++;
  }
  *pktdata++ = DLE;    /* Signal end of packet */
  *pktdata++ = ETX;
  m_serial->sendData(pktdata-encapsulatedData, encapsulatedData);

  return 1;
}

void GpsTrimble::TrimbleParser::clear_battery_backup()
{
  uint8 data[20];
  uint8 *ptr;

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_clear_battery_backup called\n");
#endif
  ptr=data;
  *ptr++=0x1e;      /* Clear battery backup. */
  *ptr++=0x4b;      /* Safety catch. */
  send_pkt(data,ptr-data);
}

void GpsTrimble::TrimbleParser::request_fix_mode()
{
  uint8 data[10];
  uint8 *ptr;

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_request_fix_mode called\n");
#endif
  ptr=data;
  *ptr++=0x62;  /* packet type */
  *ptr++=0xff;
  send_pkt(data,ptr-data);
}

void GpsTrimble::TrimbleParser::request_time()
{
  uint8 data[10];
  uint8 *ptr;

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_request_time called\n");
#endif
  ptr=data;
  *ptr++=0x21;  /* packet type */
  send_pkt(data,ptr-data);
}

void GpsTrimble::TrimbleParser::request_version()
{
  uint8 data[10];
  uint8 *ptr;

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_request_version called\n");
#endif
  ptr=data;
  *ptr++=0x1f;  /* packet type */
  send_pkt(data,ptr-data);
}

void GpsTrimble::TrimbleParser::request_signal_levels()
{
  uint8 data[10];
  uint8 *ptr;

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_request_signal_levels called\n");
#endif
  ptr=data;
  *ptr++=0x27;  /* packet type */
  send_pkt(data,ptr-data);
}

void GpsTrimble::TrimbleParser::reset_navigator()
{
  uint8 data[20];
  uint8 *ptr;

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_reset_navigator called\n");
#endif
  ptr=data;
  *ptr++=0x25;
  send_pkt(data,ptr-data);
}

void GpsTrimble::TrimbleParser::send_setup()
{
  uint8 data[42];
  uint8 *ptr;
  float tmpfloat;

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_send_setup called\n");
#endif
  ptr=data;
  *ptr++=0xBC;      /* Set port characteristics of port 2. */
  *ptr++=0x01;      /* Port ID (for port 2). */
  *ptr++=0x07;      /* Receive Baudrate 9600. */
  *ptr++=0x07;      /* Transmit Baudrate 9600. */
  *ptr++=0x03;      /* 8-bits data. */
  *ptr++=0x00;      /* No parity. */
  *ptr++=0x00;      /* 1 Stop bit. */
  *ptr++=0x00;      /* No flow control. */
  *ptr++=0x08;      /* Receive mode RTCM SC-104. */
  *ptr++=0x00;      /* No transmit protocol. (Disabled) */
  *ptr++=0x00;      /* Reserved. */
  send_pkt(data, ptr-data);

  /* Send "Set I/O Options" */
  ptr=data;
  *ptr++=0x35;  /* packet type */
  *ptr++=0x0e;  /* single-precision LLA, no superpackets, wgs-84 */
  *ptr++=0x02;  /* enu velocity */
  *ptr++=0x01;  /* utc, asap, not syncronized measurements, no minimize proj */
  *ptr++=0x00;  /* no raw, no doppler smoothed codephase, signal strength in AMU units */
  send_pkt(data, ptr-data);

  ptr=data;
  *ptr++=0x6E;      /* Set Synchronized measurement parameters. */
  *ptr++=0x01;
  *ptr++=0x00;      /* Disable synchronized packets. */
  *ptr++=0x01;
  send_pkt(data, ptr-data);

  ptr=data;
  *ptr++=0x8E;      /* Set receiver configuration. */
  *ptr++=0x15;      /* Set Datum index field. */
  *ptr++=0x0;
  *ptr++=0x0;       /* Choose Datum Index 0, WGS-84. */
  send_pkt(data, ptr-data);

  /* 0x39 Enable/Disable satellite ignore health not sent. */

  /* Send 0xBB, "Set/Query Navigation Configuration" */
  ptr=data;
  *ptr++=0xBB;      /* Set navigation configuration. */
  *ptr++=0x00;      /* Set configuration. */
  *ptr++=0x00;      /* Automatic 2D/3D detection. */
  *ptr++=0x02;      /* Automatic detection of DGPS. */
  *ptr++=0x01;      /* Dynamics Code is Land. (Sea, Air...) */
  *ptr++=0x00;      /* Reserved. */
  /* Set elevation mask. */
  tmpfloat = 0.0873F;  /* 5 degrees in radians. */
  conv_htobe_float(tmpfloat, ptr);
  ptr+=4;
  /* Set AMU mask. (Minimum Signal Level for Fixes (default 2.0).
     Set to 6.0 to prevent signals bouncing off walls but also
     signals attenuated by trees etc */
  tmpfloat = 2.0;
  conv_htobe_float(tmpfloat, ptr);
  ptr+=4;
  /* DOP mask. */
  tmpfloat = 12.0;
  conv_htobe_float(tmpfloat, ptr);
  ptr+=4;
  /* DOP switch */
  tmpfloat = 5.0;
  conv_htobe_float(tmpfloat, ptr);
  ptr+=4;
  *ptr++=30;  /* 21 */
  while (ptr-data < 41) {
    *ptr++=0;
  }
  send_pkt(data, ptr-data);

  /* Packet 0x71 and 0x73 not sent due to bad documentation.               *
   * Hopefully, they are not needed, since the defaults should be correct. *
   * Packet 70 is not very well documented either, but we send it to       *
   * be a bit more certain.                                                */
  ptr=data;
  /* Set Filter Control */
  *ptr++=0x70;
  *ptr++=0x01;  /* Position Filter On */
  *ptr++=0x00;  /* Velocity Filter Off */
  *ptr++=0x01;  /* Altitude Filter On */
  *ptr++=0x00;  /* Reserved */
  send_pkt(data, ptr-data);

  /* Send "Query DGPS Operating Mode and Status" */
  /* This packet is used by the code to identify */
  /* the end of a reset cycle, and provokes a type */
  /* 0x82 packet to be sent in response. */
  /* The 0x62 packet is no longer used to change any */
  /* options in the navigator in the ACE-II model. */
  ptr=data;
  /* packet type */
  *ptr++=0x62;
  /* Request report packet 0x82. */
  *ptr++=0xFF;
  send_pkt(data, ptr-data);


  /* Send "Request Signal Levels". */
  /* This prompts a 0x47 packet from the navigator. */
/*   ptr=data; */
/*   *ptr++=0x27; */
/*   send_pkt(data, ptr-data); */

#ifdef DRIV_NAV_TRIMBLE_SVEESIX_CM3
  /*+********************************************************
  * The following packets are only effective on the
  * old Trimble GPS navigator. Since no old navigators
  * are in use, these are left in the code but ifdef:d.
  *********************************************************+*/
  /* Send "Set Auxilirary Configuration" */
  ptr=data;
  *ptr++=0x8E;      /* packet type */
  *ptr++=0x03;      /* leave dynamics code unchanged */
  *ptr++=0x0B;      /* Transmit Baudrate 9600. */
  *ptr++=0x0B;      /* Receive Baudrate 9600. */
  *ptr++=0x13;      /* No parity, 8bits data. */
  *ptr++=0x07;      /* 1 Stop bit. */
  *ptr++=0x01;      /* Transmit mode off. */
  *ptr++=0x01;      /* Receive mode RTCM SC-104. */
  send_pkt(data, ptr-data);

  /* Send "Set DGPS Options" */
  ptr=data;
  *ptr++=0x62;  /* packet type */
  *ptr++=0x02;  /* Auto DGPS */
  send_pkt(data, ptr-data);


  /* Send "Set Operating Parameters" */
  ptr=data;
  *ptr++=0x2c;    /* packet type */
  *ptr++=0;       /* leave dynamics code unchanged */
  tmpfloat=-1;
  conv_htobe_float(tmpfloat,ptr);  /* Leave elev.mask unchanged */
  ptr+=4;
  conv_htobe_float(tmpfloat,ptr);  /* Leave signal level mask unchanged */
  ptr+=4;
  conv_htobe_float(tmpfloat,ptr);  /* Leave PDOP mask unchanged */
  ptr+=4;
  conv_htobe_float(tmpfloat,ptr);  /* Leave PDOP switch unchanged */
  ptr+=4;
  send_pkt(data, ptr-data);

  /* Send "Set High-8 best 4 or high-6 (overdetermined) mode" */
  ptr=data;
  *ptr++=0x75;    /* packet type */
  *ptr++=0x1;     /* Set High-6 (overdetermined) mode. */
  send_pkt(data, ptr-data);
#endif

}


void GpsTrimble::TrimbleParser::calculate_status()
{
   isabTime t;
   enum GpsStatus sum_status = os_gps_not_init;
   uint16 error_conditions;

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","driv_nav_trimble_calculate_status called\n");
#endif
   if ( !initialized_ok ) {
     sum_status=os_gps_not_init;
     error_conditions=0;
   } else {
     switch (status_byte1) {
       case 0x00: 
         if ((old_solution_mode & 0x07) == 4)
           sum_status=os_gps_ok_3d;
         else
           sum_status=os_gps_ok_2d;
         break;
       case 0x01: sum_status=os_gps_no_time;      break;
       case 0x02: sum_status=os_gps_no_time;      break; /* FIXME */
       case 0x03: 
       case 0x08: 
       case 0x09: 
       case 0x0a: 
         if (adstatus_byte1 & 0x08)
           sum_status=os_gps_no_almanac;
         else
           sum_status=os_gps_too_few_sats;
         break;
       case 0x0b: sum_status=os_gps_too_few_sats; break;
       case 0x0c: sum_status=os_gps_too_few_sats; break; /* FIXME */

     }
     /* We ignore the battery backup status here. It reflects the status at
      * boot, not the current status. *
      *          ((self->status_byte2   &0x01)?OS_GPS_ERROR_BATTERY:0) 
      *          ((self->adstatus_byte1 &0x02)?OS_GPS_ERROR_BATTERY:0)
      */
     error_conditions = 
                 ((status_byte2   &0x10)?OS_GPS_ERROR_ANTENNA:0) | 
                 ((status_byte1  ==0x03)?OS_GPS_ERROR_PDOP   :0) |
                 ((status_byte1  ==0x0c)?OS_GPS_ERROR_CONFIG :0) ;
   }
   if (m_consumer) {
      m_consumer->gpsStatus(t, sum_status, error_conditions, last_pdop);
   }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
   m_log->debug("%s","driv_nav_trimble_calculate_status has sent\n");
#endif
}




/* **********************************************************************
 * Routines named driv_nav_trimble_packet_XX decode a trimble packet 
 * numbered XX (hex) and create a canoical packet from the information.
 * Some of the incoming TSIP packets do not create gps-protocol packets
 * but have other effects. 
 ********************************************************************** */

void GpsTrimble::TrimbleParser::packet_13(uint8 *pktdata, int len)
{
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
  int i = 0;
#endif

  if (len < 1) {
    /* Reject too short packet. */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x13 packet\n");
#endif
    return;
  }
  bad_packets++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
  m_log->debug("Error packet gotten - Bad packet id: %02x  "
               "Bad packets: %i   Unknown packets: %i   "
               "Wrong lengths: %i\n", pktdata[0], (int)bad_packets,
               (int)unknown_packets, (int)wrong_length);
#endif
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
  while (i < len) {
    m_log->debug("  %3i : %02x\n", i, pktdata[i]);
  }
#endif
}

void GpsTrimble::TrimbleParser::packet_41(uint8 *pktdata, int len)
{
   isabTime t;
  int weekno;
  float tow,utc_offset;
  
#ifdef DRIV_NAV_TRIMBLE_SHOW_TIME_PKTS
  int i = 0;
#endif

  if (len!= 11) {   /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Broken time packet received\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 41 (GPS time)\n");
#endif
  
#ifdef DRIV_NAV_TRIMBLE_SHOW_TIME_PKTS
  while (i <= len) {
    m_log->debug("  %3i : %02x\n", i, pktdata[i]);
    i++;
  }
#endif

  tow        = conv_betoh_float(&pktdata[0]);
  weekno     = conv_betoh_int16(&pktdata[4]);
  utc_offset = conv_betoh_float(&pktdata[6]);

  if (m_consumer) {
     m_consumer->gpsTime(t, tow, weekno, utc_offset);
  }

  return;
}

void GpsTrimble::TrimbleParser::packet_44(uint8 *pktdata, int len)
{
  uint8        new_mode;
  uint32       new_con;
  int          i,num_sats=0;

  if (len< 21) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x44 packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 44 (Satellite selection)\n");
#endif
  
  new_mode=pktdata[0];
  new_con=0;
  for (i=1;i<=4;i++)                     /* Compute the satellite bitmap */
    if ((pktdata[i]>0) && (pktdata[i]<=32)) {
      new_con |= 1L << (pktdata[i]-1);
      num_sats++;
    }

  last_pdop = conv_betoh_float(&pktdata[5]);

  #ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
    m_log->debug("  New mode: %i, new constellation: %08x\n",
          new_mode, new_con);
  #endif

  if ((new_mode!=old_solution_mode) || 
      (new_con !=old_constellation))     {
    old_constellation=new_con;    /* There was a change, */
    old_solution_mode=new_mode;   /* increment the serialno of the */
    constellation_no++;           /* constellations */
    #ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
      m_log->debug("  This was a new constellation\n");
    #endif

  }

  calculate_status();
}

void GpsTrimble::TrimbleParser::packet_45(uint8* /*pktdata*/, int len)
{
  if (len < 10) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x45 packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
  m_log->debug("%s","Got a 45 (Software version)\n");
#endif /* DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS */
  navigator_just_reset=1;
  initialized_ok=0;

  return;
}

void GpsTrimble::TrimbleParser::packet_46(uint8 *pktdata, int len)
{
  if (len< 2) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x46 packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 46 (Health of Receiver):\n");

  if (status_byte1 == pktdata[0]) {
    /* No change. */
  } else {
    switch (pktdata[0]) {
      case 0x00:
        /* Doing Position Fixes. */
        m_log->debug("  Doing Position Fixes.\n");
        break;
      case 0x01:
        /* Don't have GPS time yet. */
        m_log->debug("  Don't have GPS time yet.\n");
        break;
      case 0x02:
        /* Need Initialization. */
        m_log->debug("  Need Initialization.\n");
        break;
      case 0x03:
        /* PDOP is too high. */
        m_log->debug("  PDOP is too high.\n");
        break;
      case 0x08:
        /* No usable satellites. */
        m_log->debug("  No usable satellites.\n");
        break;
      case 0x09:
        /* Only 1 usable satellite. */
        m_log->debug("  Only 1 usable satellite.\n");
        break;
      case 0x0a:
        /* Only 2 usable satellites. */
        m_log->debug("  Only 2 usable satellites.\n");
        break;
      case 0x0b:
        /* Only 3 usable satellites. */
        m_log->debug("  Only 3 usable satellites.\n");
        break;
      case 0x0c:
        /* Satellite selected unusable. */
        m_log->debug("  Satellite selected unusable.\n");
        break;
      default:
        /* Error, undefined value! */
        m_log->debug("  Error, undefined value: %i\n", pktdata[0]);
        break;
    }
  }
  if (status_byte2 == pktdata[1]) {
    /* No change. */
  } else {
    m_log->debug("  Battery Back");
    if (pktdata[1] & 0x1) {
      /* Battery Backed Ram not available at start-up. */
      m_log->debug("  Battery Backed Ram not available at start-up.\n");
    } else {
      /* Battery Backup OK. */
      m_log->debug("  Battery Backup OK.\n");
    }
    m_log->debug("  Antenna feedline: ");
    if (pktdata[1] & 0x10) {
      /* Antenna feedline fault. (Short or Open) */
      m_log->debug("  Antenna feedline fault. (Short or Open).\n");
    } else {
      /* Antenna feedline OK. */
      m_log->debug("  OK.\n");
    }
  }
#endif

  if (status_byte1 != pktdata[0] || status_byte2 != pktdata[1]) {
    /* Something changed - take action?. */
    status_byte1 = pktdata[0];
    status_byte2 = pktdata[1];
    calculate_status();
  }

}

void GpsTrimble::TrimbleParser::packet_47(uint8 *pktdata, int len)
{
  float tempfloat;
  int i;
  uint8 count;
  uint8 temp;

  if (len < 2) {
    /* Reject too small packet. */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x47 packet\n");
#endif
    return;
  }

  count = pktdata[0];
  if (count > 8) {
    /* Error */
    m_log->debug("47 - Error, too many sat/level pairs: %i\n", count);
    return;
  }

  m_log->debug("Satellite levels:%i\n", count);

  i = 1;
  while (count) {
    temp = pktdata[i];
    i+=1;

    tempfloat = conv_betoh_float(&pktdata[i]);
    m_log->debug("  Sat: %i, Level:%i\n", temp, int(100000*tempfloat));
    i+=4;
    count--;
  }
}


#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
void GpsTrimble::TrimbleParser::packet_48(uint8 *pktdata,int len)
{
  m_log->debug("%s","Got a 48:\n  ");
  pktdata[len]=0;
  m_log->debug("%s\n", pktdata);

  return;
}
#endif /* DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS */

void GpsTrimble::TrimbleParser::packet_49(uint8 *pktdata, int len)
{
  int i;

  if (len < 32) {
    /* Reject too short packet. */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x49 packet\n");
#endif
    return;
  }

  if (pktdata[31] == 0) {
    m_log->debug("%s","No health almenac page yet.\n");
  } else {
    i = 0;
    m_log->debug("Almenac health:\n");
    while (i < 32) {
       m_log->debug("  %i : %i\n", i+1, pktdata[i]);
      i++;
    }
  }
}

void GpsTrimble::TrimbleParser::packet_4A(uint8 *pktdata, int len)
{
  float tow;
  float latitude, longitude, altitude;

  if (len< 20) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x4A packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 4A (LLA position)\n");
#endif
  
#ifdef DRIV_NAV_TRIMBLE_ALWAYS_REQUEST_SIGNAL_LEVELS
  driv_nav_trimble_request_signal_levels(self);
#endif

  tow=conv_betoh_float(&pktdata[16]);
  /* The tow has wrapped -> we are in the next week. Allow 500 s
   * fudge. Packets _can_ arrive out of order. Time packets 
   * arrive before position fixes with the same timestamp e.g. */
  if (last_tow>(tow+500)) {
    last_weekno++;
  }

  latitude  = conv_betoh_float(&pktdata[0]);
  longitude = conv_betoh_float(&pktdata[4]);
  altitude  = conv_betoh_float(&pktdata[8]);

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("  lat: %f, long: %f, alt: %f, tow: %f\n",
        latitude * 180 / M_PI, longitude * 180 / M_PI, altitude, tow);
#endif


  last_lat = latitude;
  last_lon = longitude;
  last_alt = altitude;
  last_pos_fix_time = tow;

  return;
}

/* #ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS */
void GpsTrimble::TrimbleParser::packet_4B(uint8 *pktdata, int len)
{
  if (len< 3) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x4B packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 4B (Machine Code ID/Additional Status)\n");
#endif
  
  if (pktdata[1] & 1) {
    m_log->debug("Real-time Clock not available on power-up\n");
  }
  if (pktdata[1] & 0x08) {
    m_log->debug("Almanac in receiver not complete and current!\n");
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("Machine ID: %02x, Status1: %02x, Status2: %02x\n",
        pktdata[0], pktdata[1], pktdata[2] );
#endif

  adstatus_byte1=pktdata[1];
  adstatus_byte2=pktdata[2];

  calculate_status();
}

void GpsTrimble::TrimbleParser::packet_4C(uint8 *pktdata, int len)
{
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  float tmpfloat;
#endif

  if (len< 3) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x4C packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 4C (Report Operating Parameters)\n");
#endif

  pktdata = pktdata; // To remove warnings
  
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("  Dynamics code:%02x\n", pktdata[0]);

  tmpfloat = conv_betoh_float(&pktdata[1]);
  m_log->debug("  Elevation angle mask:%02x\n", int(tmpfloat*180/M_PI));

  tmpfloat =conv_betoh_float(&pktdata[5]);
  m_log->debug("  Signal level mask:%02x\n",int(tmpfloat));

  tmpfloat =conv_betoh_float(&pktdata[9]);
  m_log->debug("  PDOP mask:%02x\n", int(tmpfloat));

  tmpfloat =conv_betoh_float(&pktdata[13]);
  m_log->debug("  PDOP switch:%02x\n", int(tmpfloat));
#endif

}

void GpsTrimble::TrimbleParser::packet_55(uint8 *pktdata,int len) 
{

  if (len < 3) {
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x55 packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("Got a 55 (I/O options): %02x %02x %02x %02x\n",
        pktdata[0]<<24, 
        pktdata[1]<<16,
        pktdata[2]<< 8,
        pktdata[3] );
#else
   pktdata = pktdata; // To remove warnings
#endif

  return;
}
/* #endif */
/* DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS */


void GpsTrimble::TrimbleParser::packet_56(uint8 *pktdata, int len)
{
   isabTime t;
  float tow;
  float east, north, up;

  if (len< 20) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x56 packet\n");
#endif
    return;
  }

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 56 (ENU velocity):\n");
#endif

  tow=conv_betoh_float(&pktdata[16]);
  /* The tow has wrapped -> we are in the next week */
  if (last_tow>tow+500) {
    last_weekno++;
  }

  east  = conv_betoh_float(&pktdata[0]);
  north = conv_betoh_float(&pktdata[4]);
  up    = conv_betoh_float(&pktdata[8]);
  if (m_consumer) {
     GpsConsumerPublic::GpsPositionVelocityVector v;
     v.latitude          = last_lat;
     v.longitude         = last_lon;
     v.altitude          = last_alt;
     v.heading           = FastTrig::angleTheta2float(east, north);
     v.speed_over_ground = float(sqrt(east * east + north * north));
     v.vertical_speed    = up;
     v.weekno_for_fix    = last_weekno;
     v.tow_for_fix       = tow;
     v.fix_time          = t;
     v.constellation_no  = constellation_no;
     v.latency           = float(ASSUMED_LATENCY);
     m_consumer->gpsPositionVelocity(v);
  }
  last_vel_north = north;
  last_vel_east  = east;
  last_vel_up    = up;
  last_vel_fix_time  = tow;

  return;
}


#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
void GpsTrimble::TrimbleParser::packet_59(uint8 *pktdata, int len)
{
  uint8 i;
/*   uint8 *yes, *no; */

  if (len < 32) {
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x59 packet\n");
#endif
    /* Ignore a too short message */
    return;
  }

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 59 (Status of Satellite Disable or Ignore Health):\n"); 
#endif

  if (pktdata[0] == 3) {
    /* The following data determines wether the navigator */
    /* may select the satellite in question. */
/*     yes = " enabled\n"; */
/*     no = " disabled\n"; */
  } else if (pktdata[0]) {
    /* The following data determines wether the navigator */
    /* ignores data from the satellite in question. */
/*     yes = " not ignored\n"; */
/*     no = " ignored\n"; */
  } else {
    /* Error */
/*     os_debug_puts("59 - Undefined value: "); */
/*     os_debug_print_int(pktdata[0]); */
/*     os_debug_puts("\n"); */
/*     yes = " yes undef\n"; */
/*     no = " no undef\n"; */
  }
  i = 1;
  while (i <= 32) {
/*     os_debug_puts("Sat "); */
/*     os_debug_print_int(i); */
    if (pktdata[i] == 0) {
      /* Default, heeded or enabled. */
/*       os_debug_puts(yes); */
    } else {
      /* Ignored or disabled. */
/*       os_debug_puts(no); */
    }
    i++;
  }
}
#endif


void GpsTrimble::TrimbleParser::packet_6D(uint8 *pktdata, int len)
{
  uint8        new_mode;
  uint32       new_con;
  int          i,num_sats;

  if (len < 16) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x6D packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("%s","Got a 6D (Satellite selection)\n");
#endif
  
  new_mode=pktdata[0];
  new_con=0;
  num_sats=pktdata[0]>>4;
  if (len < (16+num_sats)) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x6D packet (2)\n");
#endif
    return;
  }
  for (i=0;i<num_sats;i++) {
    /* Compute the satellite bitmap */
    if ((pktdata[17+i]>0) && (pktdata[17+i]<=32)) {
      new_con|=1L << (pktdata[17+i]-1);
    }
  }

  #ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
    m_log->debug("New mode: %i, new constellation: %i\n", new_mode, new_con);
  #endif

  last_pdop = conv_betoh_float(&pktdata[1]);

  if ((new_mode != old_solution_mode) || 
      (new_con  != old_constellation))
  {
    old_constellation=new_con;  /* If there was any change, */
    old_solution_mode=new_mode; /* increment the serialno of the */
    constellation_no++;         /* constellations */
    #ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
      m_log->debug("  which is a new constellation.\n");
    #endif

  }
  calculate_status();
}

#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
void GpsTrimble::TrimbleParser::packet_70(uint8 *pktdata, int len)
{
  if (len != 5) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x70 packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("Got a 70 (Report Filter Control) : len=%i, "
               "pvf %s, sf %s, af %s\n", 
               len,
               pktdata[0] ? "on":"off",
               pktdata[1] ? "on":"off",
               pktdata[2] ? "on":"off"
               );
#endif
}
#endif

#ifdef DRIV_NAV_TRIMBLE_SVEESIX_CM3
void GpsTrimble::TrimbleParser::packet_76(uint8 *pktdata, int len)
{
  uint8 mode;

  if (len < 2) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x76 packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
  m_log->debug("%s","Got a 76 (High-8-Best-4/High-6)\n");
#endif /* DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS */

  mode = pktdata[0];

  if (!mode) {
    m_log->debug("Mode is High-8-Best-4\n");
  } else {
    m_log->debug("Mode is High-6 overdetermined\n");
  }

  return;
}
#endif

void GpsTrimble::TrimbleParser::packet_82(uint8 *pktdata, int len)
{
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  char *debugstr;
#endif

  if (len < 1) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0x82 packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  switch (pktdata[0]) {
    case 0:
      /* Differential Mode off. */
      debugstr="Differential Mode off";
      break;
    case 1:
      /* Differential Mode on. */
      debugstr="Differential Mode on";
      break;
    case 2:
      /* Auto DGPS, DGPS off. */
      debugstr="Auto DGPS, DGPS off";
      break;
    case 3:
      /* Auto DGPS, DGPS on. */
      debugstr="Auto DGPS, DGPS on";
      break;
    default:
      /* Error, this should not happen. */
      debugstr="Unknown";
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
      m_log->debug("This 82 is out of range: %i\n", pktdata[0]);
#endif
      break;
  }
  m_log->debug("Got a 82 - %s.\n", debugstr);
#else
  pktdata = pktdata; //To remove warnings
#endif

  /* This packet is the last one sent after a reset. If it was preceeded 
     by a 45 it means we have completed the reset and the navigator must be
     initialized */
  if (navigator_just_reset) {
    navigator_just_reset=0;
    send_setup();
    initialized_ok=1;
    /* initialized_ok no longer masking true status. */
    /* Immediatly tell the consumer of our status */
    calculate_status();
  }

  return;
}

void GpsTrimble::TrimbleParser::packet_BB(uint8 *pktdata, int len)
{

  if (len < 39) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0xBB packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("Got a BB - mode: %02x, Dimension: %i, DGPS mode: %i, Dynamics: %i\n", 
        pktdata[0], 
        pktdata[1], 
        pktdata[2], 
        pktdata[3] );

  m_log->debug("  BB-masks: Elevation mask: %i, AMU mask: %i, DOP mask: %i,"
               " DOP switch: %i, DGPS age limit: %i\n",
        int(conv_betoh_float(&pktdata[5]) * 100000),
        int(conv_betoh_float(&pktdata[9]) * 100000),
        int(conv_betoh_float(&pktdata[13]) * 100000),
        int(conv_betoh_float(&pktdata[17]) * 100000),
        pktdata[21] );
#else
  pktdata = pktdata; //To remove warnings
#endif
}

void GpsTrimble::TrimbleParser::packet_BC(uint8 *pktdata, int len)
{
  if (len < 9) {
    /* Ignore a too short message */
    wrong_length++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
    m_log->debug("%s","Got short 0xBC packet\n");
#endif
    return;
  }
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGTEXTS
  m_log->debug("Got a BC - Port:%i, Input Baud Rate:%i, "
        "Output Baud Rate:%i, Data bits:%i, Parity:%i, "
        "Stop bits:%i, Flow control:%i, Input proto:%i, "
        "Output proto:%i\n",
        pktdata[0],
        pktdata[1],
        pktdata[2],
        pktdata[3],
        pktdata[4],
        pktdata[5],
        pktdata[6],
        pktdata[7],
        pktdata[8]
        );
#else
  pktdata = pktdata; //To remove warnings
#endif

  return;
}

void GpsTrimble::TrimbleParser::packet_unknown(int type, uint8* /*pktdata*/, int len)
{
  unknown_packets++;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
  m_log->debug("Unknown packet gotten - Packet id: %02x, "
        "length: %i\n", type, len);
  m_log->debug("Unknown packets: %i, Bad packets: %i, Wrong Packets: %i\n",
        (int)unknown_packets, (int)bad_packets, (int)wrong_length);
#endif
  return;
}



void GpsTrimble::TrimbleParser::interpret_packet(uint8 *pktdata, int pktlen)
{
  int type;

  /* Extract and strip off the packet type code. This makes the indices */
  /* into the packet the same as in the trimble documentation           */
  type=*pktdata++;

  /* We have a (hopefully) valid packet. See if it is one we know
     and care about, otherwise it is ignored */
  switch ( type ) {
    case 0x13: packet_13(pktdata,pktlen); break;
    case 0x41: packet_41(pktdata,pktlen); break;
    case 0x44: packet_44(pktdata,pktlen); break;
    case 0x45: packet_45(pktdata,pktlen); break;
    case 0x46: packet_46(pktdata,pktlen); break;
    case 0x47: packet_47(pktdata,pktlen); break;
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
    case 0x48: packet_48(pktdata,pktlen); break;
#endif
    case 0x49: packet_49(pktdata,pktlen); break;
    case 0x4A: packet_4A(pktdata,pktlen); break;

    case 0x4B: packet_4B(pktdata,pktlen); break;
    case 0x4C: packet_4C(pktdata,pktlen); break;
    case 0x55: packet_55(pktdata,pktlen); break;
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
#endif

    case 0x56: packet_56(pktdata,pktlen); break;
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
    case 0x59: packet_59(pktdata,pktlen); break;
#endif
    case 0x6D: packet_6D(pktdata,pktlen); break;
#ifdef DRIV_NAV_TRIMBLE_SHOW_DEBUGPACKETS
    case 0x70: packet_70(pktdata,pktlen); break;
#endif
#ifdef DRIV_NAV_TRIMBLE_SVEESIX_CM3
    case 0x76: packet_76(pktdata,pktlen); break;
#endif
    case 0x82: packet_82(pktdata,pktlen); break;
    case 0xBB: packet_BB(pktdata,pktlen); break;
    case 0xBC: packet_BC(pktdata,pktlen); break;
    default: packet_unknown(type,pktdata,pktlen); break;
  }
    
}


int GpsTrimble::TrimbleParser::receiveData(const uint8 *indata, int len)
{
  uint8 databyte;  /* Byte being examined */

  if (state == state_pre_system_startup) {
// #define DRIV_NAV_TRIMBLE_DEBUG_ALL_INPUT
#ifdef DRIV_NAV_TRIMBLE_DEBUG_ALL_INPUT
     m_log->debug("Dropped data since not initialized: %i bytes", len);
#endif
     return 0;
  }

  while (len) {
    databyte=*indata;
#ifdef DRIV_NAV_TRIMBLE_DEBUG_ALL_INPUT
    m_log->debug("rd: 0x%08x, 0x%02x", int(state), databyte);
#endif
    switch (state)  {
      case state_idle:
        if (databyte==DLE)
          state=state_after_initial_dle;
        break;
      case state_in_packet_after_dle:
        /* We received a DLE inside a packet. It is either the start of a */
        /* new packet (an error really), the end of the current packet    */
        /* (if it is followed by a ETX) or just an escaped DLE            */
        if (databyte==DLE) {
          if ((reassembly_ptr - reassmebled_pkt) >= 
              int(sizeof(reassmebled_pkt)) ) {
            state=state_idle;  /* Oversized packet, drop it */
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
            m_log->debug("%s","Oversized packet after DLE!\n");
#endif
            break;      /* FIXME - log something? */
          }
          *(reassembly_ptr++)=databyte;  /* DLE DLE => DLE */
          state = state_in_packet;
          break;
        } else if (databyte==ETX) {
          interpret_packet(reassmebled_pkt,
                           reassembly_ptr - reassmebled_pkt);
          state=state_idle;
          break;
        }
        /* Fall through and process as a new packet */
      case state_after_initial_dle:
        switch (databyte) {
          case ETX:    /* DLE ETX => end of packet outside packet */
          case DLE:
            state=state_idle;  /* DLE DLE outside a packet is nonsense */
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
            m_log->debug("%s","Bad packet after init DLE!\n");
#endif
            break;         /* We are probably out of sync */
          default:
            reassembly_ptr=reassmebled_pkt;
            *(reassembly_ptr++)=databyte;
            state=state_in_packet;
            
        }
        break;  /* switch (databyte) in state_after_initial_dle */
      case state_in_packet:
        if (databyte==DLE) {
          state=state_in_packet_after_dle;
          break;
        }
        if ((reassembly_ptr - reassmebled_pkt) >= 
            int(sizeof(reassmebled_pkt)) ) {
          state=state_idle;  /* Oversized packet, drop it */
#ifdef DRIV_NAV_TRIMBLE_DEBUG_BAD_PACKETS
          m_log->debug("%s","Oversized packet!\n");
#endif
          break;      /* FIXME - log something? */
        }
        *(reassembly_ptr++)=databyte;
        break;
    case state_pre_system_startup:
       m_log->error("receiveData: state_pre_system_startup in switch. "
                    "Should be impossible.");
       ///XXX - do setup?
       return 0;
    }
    indata++;
    len--;
  }
  
  return 0;
}

int GpsTrimble::TrimbleParser::setup()
{
  /* Clear lots of status variables. */
  last_lat = 0;
  last_lon = 0;
  last_vel_north = 0;
  last_vel_east = 0;
  last_vel_up = 0;
  last_pos_fix_time = 0;
  last_vel_fix_time = 0;
  last_pdop = 0;
  unknown_packets = 0;
  bad_packets = 0;
  wrong_length = 0;
  status_byte1 = 0x01;
  status_byte2 = 0;
  initialized_ok = 0;
  old_solution_mode = 0;

  state=state_idle;
                            
  /* Set up the state for the receive state machine */
  navigator_just_reset=0;
  
  /* Initiate global to invalid value. */
  last_weekno = -1;
  last_tow    = -1;

  reset_navigator();

  return 0;    /* Return success */
}

} /* namespace isab */

