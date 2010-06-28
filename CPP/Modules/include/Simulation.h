/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/** A module to decode simulation data and feed it to the respective
 * protocols.
 */


#ifndef MODULES_Simulation_H
#define MODULES_Simulation_H

#include <list>
#include <queue>

/// Makes the simulation module read simulation data from a file.
#define SIM_DATA_FROM_FILE

namespace isab {

   class Simulation : public Module, public SerialConsumerInterface,
                                     public SerialProviderInterface {
   public:
#ifdef SIM_DATA_FROM_FILE
      Simulation(SerialProviderPublic *p, const char* simFile = NULL);
#else
      Simulation(SerialProviderPublic *p);
#endif
      ~Simulation(){
         delete decoder.cmdBeingAssembled;
#ifdef SIM_DATA_FROM_FILE
         delete[] m_simDataFile;
#endif
      }

      /** Creates a new SerialProviderPublic object used to reach this
       * module. This gets connected to the trimble decoder.
       * @return a new SerialProviderPublic object connected to the queue.
       */
      SerialProviderPublic * newPublicSerial();

#ifndef _MSC_VER
      static const int MaxMsgSize = 50;
#else
      enum { MaxMsgSize = 50 } ;
#endif

   protected:
      /* Maximum size of various messages. Arbitarily chosen to be large enough
       * for all messages to/from the trimble */
      SerialProviderPublic * m_provider;
      virtual MsgBuffer * dispatch(MsgBuffer *buf);
      virtual void decodedReceiveData(int length, const uint8 *data, uint32 src);
      virtual void decodedSendData(int inlen, const uint8 *data, uint32 src);
      SerialConsumerPublic * rootPublic();
      virtual void decodedShutdownPrepare(int16 upperTimeout);
      virtual void decodedStartupComplete();

      /** Set after shutdownPrepare() */
      bool m_shutdown;

      /** Decoder for SerialConsumer-messages */
      SerialConsumerDecoder m_consumerDecoder;

      /** Decoder for SerialProvider-messages */
      SerialProviderDecoder m_serialProviderDecoder;

#ifdef SIM_DATA_FROM_FILE
   /**
    * Reads a file with sim data and stores it in a buffer.
    * 
    *
    */
   void simDataFromFileToDecoding();

   Buffer* m_fileSimData;
   bool m_alreadyTriedToOpenFile;
      char* m_simDataFile;
#endif

      enum driv_new_sim_states {
         decoder_idle,
         decoder_idle_got_escape,
         decoder_in_packet,
         decoder_in_packet_got_escape
      };

      struct SimCommand {
         isabTime rxTime;
         int8 pktType;
         bool reconnect;
         uint8 buf[MaxMsgSize];
         int numBytes;
      };

      struct driv_new_sim_decoder {
         enum driv_new_sim_states state;
         struct SimCommand *cmdBeingAssembled;
         uint8 reconnected;
      };

      enum driv_new_sim_trimble_state {
         state_idle, state_in_packet_after_dle,
         state_after_initial_dle,
         state_in_packet
      };

      struct driv_new_sim_trimble_emulator {
         float base_time;
         uint8 reassmebled_pkt[MaxMsgSize];
         uint8 *outpos;
         int outlen;
         enum driv_new_sim_trimble_state state;
      };

      struct driv_new_sim_dispatcher {
         struct SimCommand *next_command;
         std::queue< struct SimCommand *> cmdQueue;
         isabTime last_exec_time;
         uint16 timerId;
      };

      struct driv_new_sim_decoder decoder;
      struct driv_new_sim_dispatcher dispatcher;
      struct driv_new_sim_trimble_emulator trimble;
      uint32 elapsed_simulation_time;

      /* Standard replies used by the trimble smulator. The actual data
       * is supplied in Simulation.cpp. */
      /** Software version */
      static const uint8 simulated_pkt45[];
      /** Helth of receiver */
      static const uint8 simulated_pkt46[];
      /** Differential fix mode */
      static const uint8 simulated_pkt82[];



      /*+*******************************************************
      * Setup the timers etc for the next command. 
      *
      * This function should only ever be called when no
      * packet is awaiting execution.
      ********************************************************+*/
      void setupForNextCommand();

      /*+*******************************************************
      * Actually dispatch the simulated data to the various
      * protocol drivers. These can either loop the data 
      * locally or send it out to another navigator.  
      *
      * This functions draws the actual data from the list 
      * prepared by decode_data().
      * The list is already ordered by the simulation source.
      ********************************************************+*/
      void decodedExpiredTimer(uint16 timerno);

      /*+*******************************************************
      * Insert data byte into reassembly buffer (helper)
      *
      * This small function breaks out some common code for the 
      * decoder_in_packet and decoder_in_packet_got_escape
      * states. It is expecet that this function is inlined.
      *
      * Append one data byte to the reassembly buffer while
      * checking for too large packets.
      ********************************************************+*/
      void append_data(uint8 data);

      /*+*******************************************************
      * Decode simulation data
      *
      * uint8 data_stream
      *        The raw stream of bytes. See "ISos Simulation 
      *        Protocol v 1.01" for more information.
      *
      * int   data_size
      *        Number of bytes in data_stream
      *
      * int blocking
      *        Zero to return when unable to process more data
      *        or non-zero to block until all data is processed.
      *
      * This function operates in two modes: blocking and 
      * non-blocking. Non-blocking operation is strongly 
      * recommended for serial connections while blocking
      * operation normally is used when reading from rom/ram.
      *
      * Details: a buffer is held in self->decoder.buf whenever
      * the state is not one of the idle ones (idle or 
      * idle_got_escape).
      ********************************************************+*/
      void decode_data(
               const uint8 * data_stream,
               int32 data_size,
               int reconnect,
               int blocking);

      void trimble_interpret_packet(uint8 *bufptr, int size);

   };

} /* namespace isab */

#endif /* MODULES_GpsTrimble_H */
