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


#ifndef MODULES_MotorolaParser_H
#define MODULES_MotorolaParser_H

namespace isab {

   /** This class stores the state of
    * the NMEA packet reassembly state machine. In addition it maintains
    * some information on the current state of the receiver such as 
    * health and the current constellation of satellites etc.
    */
   class MotorolaParser : public GpsParser{
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
      };

      /** Max size of any data ever received from the gps receiver. */
#ifndef _MSC_VER
      static const int max_nmea_packet_size=120;
#else
      enum {max_nmea_packet_size=120};
#endif

      /** The packet is temporarily stored 
          here after receiveing it       +*/
      char reassmebled_pkt[max_nmea_packet_size+1];

      /** Current position in the reassmebled_pkt */
      char *reassembly_ptr;

      enum nmea_state state;

      bool   m_validSolution;

      /* Count broken packets */
      uint32 unknown_packets;
      uint32 bad_packets;
      uint32 wrong_length;
 

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

      MotorolaParser(SerialProviderPublic *p, 
                 Log * log);
   };
   

} /* namespace isab */

#endif /* MODULES_MotorolaParser_H */
