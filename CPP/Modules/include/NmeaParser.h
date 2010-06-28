/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/** This is a helper class for gps receivers with
 * nmea output.
 */


#ifndef MODULES_NmeaParser_H
#define MODULES_NmeaParser_H

#include <vector>

namespace isab {

   /** This class stores the state of
    * the NMEA packet reassembly state machine. In addition it maintains
    * some information on the current state of the receiver such as 
    * health and the current constellation of satellites etc.
    */
   class NmeaParser : public GpsParser{
   protected:
      /*+****************************************************************
       * The TSIP protocol is received using a state machine which passes
       * through these states. At startup state_idle is assumed to be
       * correct. After the first packet the state machine should be
       * in sync with the gps module.
       ****************************************************************+*/
      enum nmea_state {
         state_pre_system_startup,
         state_idle, 
         state_in_packet,
         state_in_packet_after_star,
      };

      /** Max size of any data ever received from the gps receiver. */
#ifndef _MSC_VER
      static const int max_nmea_packet_size=90;
      static const int max_nmea_params=20;
#else
      enum {max_nmea_packet_size=90};
      enum {max_nmea_params=20};
#endif

      /** The packet is temporarily stored 
          here after receiveing it       +*/
      char reassmebled_pkt[max_nmea_packet_size+1];

      /** Current position in the reassmebled_pkt */
      char *reassembly_ptr;

      uint8 checksum_from_pkt;
      uint8 calculated_checksum;
      bool  checksum_present;

      enum nmea_state state;

      /* Cache of GPS information so we can request status without
       * having to be a subscriber...
       */
//      int constellation_no;
//      uint32 old_constellation;
//      uint8  old_solution_mode;
      int16  last_weekno;
      float  last_tow;

      float  last_lat;
      float  last_lon;
      float  last_alt;
      float  last_fix_time;

      float  last_speed;
      float  last_heading;
      float  last_pdop;

      bool   m_validSolution;
      bool   m_doing3d;
      int32  m_satInfoSeqId;

      /* Count broken packets */
      uint32 unknown_packets;
      uint32 bad_packets;
      uint32 wrong_length;

      GpsSatInfoHolder *m_satInfo;
      
      /** Check if pdop inidcates an ivalid solution 
       *
       * @return true if last_pdop > 49
       */
      inline bool pdopTooHigh() {
         return (last_pdop > 49);
      }


      float nmeaTimeToSeconds(char *s);
      int32 nmeaDateToDays(char *s);

      int nmeaReadLatitude(char *s, float &latitude);
      int nmeaReadLongitude(char *s, float &longitude);

      static const int hexToInt(char c);


      /* ********************************************************
       * Setup routines
       * ********************************************************/

      /**
       * Send a packet to the navigator
       *
       * @param buf
       *       Data of the packet. Note that this _includes_ the 
       *       starting dollar sign, but not the checksum asterisk.
       *
       * @return 1 if the message was sent correctly and 0 if not.
       */
      int send_pkt(Buffer *buf);

      /**
       * Send a reset packet to the navigator.
       * 
       * This sends a reset request to the trimble navigator. It is sent
       * after the StartupComplete message has been received.
       */
      void reset_navigator();

      /**
       * Send setup packet(s) to the navigator.
       * 
       * This function is called when a 82 packet is received after a 
       * 45 packet. 
       */
      void send_setup();

      /* **********************************************************************
       * Routines named packet_XXX decode an NMEA packet 
       * named XXX and create a canoical packet from the information.
       * Some of the incoming NMEA packets do not create gps-protocol calls
       * but have other effects. 
       ********************************************************************** */


      /** GGA - Global Positioning System Fix Data **/
      /** Note - only used for altitude due to no quality indication **/
      /**        included in message. **/
      void packet_GGA(int num_params, char *params[]);

      /** RMC - Recomended Minimum Specific */
      void packet_RMC(int num_params, char *params[]);

      /** GSA - GNSS (navstar, gps) DOP and Active Satellites */
      void packet_GSA(int num_params, char *params[]);

      /** GSV - Satellites in view. **/
      /** These sentences describe the sky position of a UPS **/
      /** satellite in view. Typically they're shipped in a **/
      /** group of 2 or 3. **/
      void packet_GSV(int num_params, char *params[]);

      typedef void (NmeaParser::*handlerFunction_t)(int num_params, char *params[]);
      typedef struct {
         char id[8];
         int minParams;
         handlerFunction_t handler;
      } handlerListElement_t;
      static const handlerListElement_t paramHandlers[];

      /**
       * Handle an unknown packet from the NMEA gps navigator.
       * 
       * If an unknown packet is received this function is called
       * to handle the data.
       * 
       * @param packet    the unaltered packet contents
       */
      void packet_unknown(char *packet);

      /**
       * Handle an incoming packet from the NMEA gps navigator.
       * 
       * This routine selects the proper handling function for a 
       * particular packet. The data is chopped into zero
       * terminated parameters.
       */
      void interpret_packet();


      /**
       * Called to start up the NMEA protocol.
       */
      int setup();


   public:

      /**
       * Called by the owner (a GpsTrimble object) once the connection 
       * to the hardware or a simulator is in place.
       */
      virtual void connection_ready();

      /** 
       * Set a new consumer. Called from highlevelConnectTo().
       */
      virtual void setConsumer(GpsConsumerPublic *c);

      /**
       * Receive data from the serial port.
       * 
       * Returns the number of handled packets and -1 on failure. 
       * 0 means no end of packet was found for a valid packet. 
       * This is normal.
       *  
       * If there is an unfinsihed packet when the input buffer ends the
       * current state of the packet reassembly state machine is stored.
       */
      virtual int receiveData(const uint8 *indata, int len);

      /** Unconditionally send the current status to the cosnumer,
       * if one exists.
       */
      virtual void sendStatus();

      NmeaParser(SerialProviderPublic *p, 
                 Log * log);

      virtual ~NmeaParser();
   };
   

} /* namespace isab */

#endif /* MODULES_NmeaParser_H */
