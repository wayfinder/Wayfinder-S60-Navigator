/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/** A module to communicate with a Trimble gps receiver.
 */


#ifndef MODULES_GpsTrimble_H
#define MODULES_GpsTrimble_H

namespace isab {

class GpsTrimble : public Module, public SerialConsumerInterface,
                                  public GpsProviderInterface {

   class TrimbleParser;
   friend class TrimbleParser;


   /** This class stores the state of
    * the TSIP packet reassembly state machine. In addition it maintains
    * some information on the current state of the receiver such as 
    * health and the current constellation of satellites etc.
    */
   class TrimbleParser {
   protected:
      /*+****************************************************************
       * The TSIP protocol is received using a state machine which passes
       * through these states. At startup state_idle is assumed to be
       * correct. After the first packet the state machine should be
       * in sync with the gps module.
       ****************************************************************+*/
      enum driv_nav_trimble_state {
         state_pre_system_startup,
         state_idle, 
         state_in_packet_after_dle,
         state_after_initial_dle,
         state_in_packet
      };

      /** Max size of any data ever received from the gps receiver. */
#ifndef _MSC_VER
      static const int max_tsip_packet_size=176;
#else
      enum {max_tsip_packet_size=176};
#endif

      /** The packet is temporarily stored 
          here after de-DLE:ing it       +*/
      uint8 reassmebled_pkt[max_tsip_packet_size];

      /** Current position in the reassmebled_pkt */
      uint8 *reassembly_ptr;

      enum driv_nav_trimble_state state;

      /* Cache of GPS information so we can request status without
       * having to be a subscriber...
       */
      int constellation_no;
      uint32 old_constellation;
      uint8  old_solution_mode;
      int16  last_weekno;
      float  last_tow;

      float  last_lat;
      float  last_lon;
      float  last_alt;
      float  last_pos_fix_time;
      float  last_vel_north;
      float  last_vel_east;
      float  last_vel_up;
      float  last_vel_fix_time;
      float  last_pdop;

      /* Count broken packets */
      uint32 unknown_packets;
      uint32 bad_packets;
      uint32 wrong_length;
 
                     /** status code from packet 0x46   */
      uint8  status_byte1;
                     /** error code from packet 0x46    */
      uint8  status_byte2;
                     /** Status byte 1 from packet 0x4b */
      uint8  adstatus_byte1;
                     /** Status byte 2 from packet 0x4b */
      uint8  adstatus_byte2;

      /** Set if we have sent a reset cmd and are waiting for the 
          result. See driv_nav_trimble_reset_navigator and the 
          handlers for packets 45 and 82 */
      int navigator_just_reset;      
      /** Is the navigator initialized and ready yet? */
      int initialized_ok;


      /** The serial port that connects us to the Trimble receiver */
      SerialProviderPublic * m_serial;
      /** Where to send the calculated data */
      GpsConsumerPublic    * m_consumer;
      /** The logging facility */
      Log * m_log;



      /** Time support routine */
      static isabTime time_from_gps(int weekno, float tow);

      /* ********************************************************
       * Setup routines
       * ********************************************************/

      /**
       * Send a packet to the navigator
       *
       * @param self  
       *       Pointer back to the trimble_self struct
       *
       * @param data
       *       Data of the packet. Note that this _includes_ the 
       *       packet type byte but not the inital DLE. 
       *
       * @param len
       *       Length of the data packet held in *data.
       *
       * @return 1 if the message was sent correctly and 0 if not.
       */
      int send_pkt(uint8 *data, int len);

      /**
       * Send a clear battery backup packet to the navigator.
       * 
       * This clears the battery backupped memory (almanac, ephemeris
       * current position, mode and communication port setup)
       * Afterwards, it continues as a normal reset.
       */
      void clear_battery_backup();

      /**
       * Send a request differental fix mode (0x62) packet to the navigator.
       */
      void request_fix_mode();

      /**
       * Send a request software version (0x1F) packet to the navigator.
       */
      void request_version();

      /**
       * Send a request signal levels (0x27) packet to the navigator.
       */
      void request_signal_levels();

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

      /**
       * Create and send a os_gps_status packet to the consumer
       * 
       * This routine combines information from several packets 
       * into a more generic status message. Currently data from
       * packets 0x46 (Health of Receiver), 0x4B (Machine Code ID/
       * Additional Status) and 0x6D (All-in-View Satellite 
       * Selection) is used.
       */
      void calculate_status();

      /* **********************************************************************
       * Routines named packet_XX decode a trimble packet 
       * numbered XX (hex) and create a canoical packet from the information.
       * Some of the incoming TSIP packets do not create gps-protocol calls
       * but have other effects. 
       ********************************************************************** */

      void packet_13(uint8 *pktdata, int len);

      /**
       * Handle a 0x41 packet (GPS Time) from the 
       * trimble gps receiver.
       * 
       * This routine is an important part of the time syncronization
       * system. If this packet arrives within a short amount of time
       * after the latest pps signal they are correlated.
       *
       * @param pktdata  Raw packet data - need not be 
       *                 aligned in any way. The leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      Length of input packet in pktdata.
       *                 The header is not counted.
       */
      void packet_41(uint8 *pktdata, int len);

      /**
       * Handle a 0x44 packet (Satellite selection) from the 
       * trimble gps receiver.
       * 
       * If either the mode (2d/3d) or the satellite constellation
       * has changed a new constellation number will be used in 
       * further position/velocity packets. This way a consumer knows
       * that a change in geometry has occured.
       *
       * It is not clear if some position/velocity packets can 
       * be sent using a new constellation before this packet is sent.
       * 
       * In 6-sat mode packet 0x6D is used instead
       *
       * @param pktdata  Raw packet data - need not be 
       *                 aligned in any way. The leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      Length of input packet in pktdata.
       *                 The header is not counted.
       */
      void packet_44(uint8 *pktdata, int len);

      /**
       * Handle a 0x45 packet (Software Version) from the 
       * trimble gps receiver. Indicates a reset has occured.
       * 
       * This packet is only sent after a reset or after a packet 1F
       * (Request Software Versions). Since we never send such a
       * request we know that the navigator has been reset.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_45(uint8 *pktdata, int len);

      /**
       * Handle a 0x46 packet (Health of Receiver) from the 
       * trimble gps receiver.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_46(uint8 *pktdata, int len);

      /**
       * Handle a 0x47 packet (Signal Levels for all Satellites)
       * from the trimble gps receiver.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_47(uint8 *pktdata, int len);

      /**
       * Receive a 0x48 packet (GPS System Message) from the Trimble
       * receiver.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_48(uint8 *pktdata,int len);

      /**
       * Receive a 0x49 packet (Almenac Health Page) from the
       * trimble receiver.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_49(uint8 *pktdata, int len);

      /**
       * Handle a 0x4A packet (Single-precision LLA Position fix) from the 
       * trimble gps receiver.
       * 
       * This packet is automaticallly sent as soon as a new position 
       * fix has been calculated.
       *
       * FIXME - check if GPS or UTC time should be used.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_4A(uint8 *pktdata, int len);

      /**
       * Handle a 0x4B packet (Machine Code ID/Additional Status) from the 
       * trimble gps receiver.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_4B(uint8 *pktdata, int len);

      /**
       * Handle a 0x4C packet (Report Operating Parameters) from the 
       * trimble gps receiver.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_4C(uint8 *pktdata, int len);

      /**
       * Handle a 0x55 packet (I/O options) from the 
       * trimble gps receiver.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_55(uint8 *pktdata,int len);

      /**
       * Handle a 0x56 packet (Single-precision ENU velocity fix) from the 
       * trimble gps receiver.
       * 
       * This packet is automaticallly sent as soon as a new position 
       * fix has been calculated.
       *
       * FIXME - check if GPS or UTC time should be used.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_56(uint8 *pktdata, int len);

      /**
       * Handle a 0x59 packet (Status of Satellite Disable or Ignore Health)
       * from the trimble gps receiver.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_59(uint8 *pktdata, int len);

      /**
       * Handle a 0x6D packet (All-in-View Satellite Selection) from the 
       * trimble gps receiver.
       * 
       * If either the mode (2d/3d) or the satellite constellation
       * has changed a new constellation number will be used in 
       * further position/velocity packets. This way a consumer knows
       * that a change in geometry has occured.
       *
       * It is not clear if some position/velocity packets can 
       * be sent using a new constellation before this packet is sent.
       * 
       * With 4 or fewer satellites packet 0x44 is used instead
       *
       * FIXME:
       * This function relies on the highest numbered satellite 
       * being numbered less than 32.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_6D(uint8 *pktdata, int len);

      /**
       * Handle a 0x70 packet (Filter Control)
       * from the trimble gps receiver. Diagnostic information
       * as these parameters should be set correctly.  
       * 
       * This packet is only sent after a packet 0x70 (which is sent
       * at startup).
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_70(uint8 *pktdata, int len);

      /**
       * Handle a 0x76 packet (High-8-Best-4/High-6 overdetermined mode)
       * from the trimble gps receiver. Current satellite tracking mode.
       * 
       * This packet is only sent after a packet 0x75 (which is sent
       * at startup).
       *
       * This packet is only sent by the SVeeSix CM3 module, not
       * by the ACE-II module.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_76(uint8 *pktdata, int len);

      /**
       * Handle a 0x82 packet (Differential position Fix Mode) from the 
       * trimble gps receiver. Indicates end of startup of the navigator.
       * 
       * This packet is sent at the end of the reset sequence of the 
       * navigator, in response to packet 62 (Request Differential Mode)
       * or after a satellite selection. If it occurs after the start
       * of a reset cycle (indicated by a packet 45) we know it indicates 
       * end of reset for the navigator.
       * We then proceed to set the operating mode of the navigator.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_82(uint8 *pktdata, int len);

      /**
       * Handle a 0xBB packet (Set/Query Receiver Configuration) from the 
       * trimble gps receiver.
       * 
       * This packet is sent as a response to the 0xBB packet we
       * send at restart to set GPS variables.
       *
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_BB(uint8 *pktdata, int len);

      /**
       * Handle a 0xBC packet (Protocol Configuration) from the 
       * trimble gps receiver.
       * 
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_BC(uint8 *pktdata, int len);

      /**
       * Handle an unknown packet from the trimble gps navigator.
       * 
       * If an unknown packet is received this function is called
       * to handle the data.
       * 
       * @param type     Packet type byte
       * @param pktdata  raw packet data - need not be 
       *                 aligned in any way. the leading
       *                 packet type byte and the packet
       *                 header are stripped off. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void packet_unknown(int type, uint8 *pktdata, int len);

      /**
       * Handle an incoming packet from the trimble gps navigator.
       * 
       * This routine selects the proper handling function for a 
       * particular packet. The data passed tot he handler is massaged
       * so that the byte numbers are identical to those in the
       * trimble documentation.
       *
       * @param pktdata  Raw packet data - need not be 
       *                 aligned in any way. All the 
       *                 DLE:s are stripped off but the
       *                 packet type byte is still at
       *                 the head of the data. 
       * @param len      length of input packet in pktdata.
       *                 the header is not counted.
       */
      void interpret_packet(uint8 *pktdata, int pktlen);


      /**
       * Called to start up the trimble protocol.
       */
      int setup();


   public:
      /**
       * Send a request time (0x21) packet to the navigator.
       */
      void request_time();

      /**
       * Called by the owner (a GpsTrimble object) once the connection 
       * to the hardware or a simulator is in place.
       */
      void connection_ready();

      /** 
       * Set a new consumer. Called from highlevelConnectTo().
       */
      void setConsumer(GpsConsumerPublic *c);

      /**
       * Receive data from the serial port.
       * 
       * Returns 1 on failure and 0 on success. Currently sucess is 
       * always returned.
       *  
       * This function is called from the GpsTrimble module. We
       * remove the DLE escapes and try to reassemble a packet. If
       * there is an unfinsihed packet when the input buffer ends the
       * current state of the packet reassembly state machine is stored
       * in state, saved_outlen and saved_outpos.
       */
      int receiveData(const uint8 *indata, int len);

      TrimbleParser(SerialProviderPublic *p, 
                    Log * log);
   };
   
public:
   GpsTrimble(SerialProviderPublic *p);
   virtual ~GpsTrimble();
   /** Creates a new GpsProviderPublic object used to reach this 
   * module.
   * @return a new GpsProviderPublic object connected to the queue.
    */
   GpsProviderPublic * newPublicGps();

protected:
   SerialProviderPublic * m_provider;
   TrimbleParser * m_trimbleParser;
   virtual MsgBuffer * dispatch(MsgBuffer *buf);
   virtual void decodedConnectionCtrl(enum ConnectionCtrl command, 
                              const char *method, uint32 src);
   virtual void decodedReceiveData(int length, const uint8 *data, uint32 src);
   virtual void decodedExpiredTimer(uint16 timerid);
   virtual void decodedResetReceiver(uint32 src);
   virtual void decodedStartupComplete();
   virtual int highlevelConnectTo();
   GpsConsumerPublic * rootPublic();

   /** Decoder for SerialConsumer-messages */
   SerialConsumerDecoder m_consumerDecoder;
   /** Decoder for GpsProvider-messages */
   GpsProviderDecoder m_providerDecoder;

   uint16 m_slowGpsTimerId;


};

} /* namespace isab */

#endif /* MODULES_GpsTrimble_H */
