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
 * Simulation.cpp - Read simulation data from a serial stream or
 *                  from a rom area and decode it. 
 **/

#include "arch.h"
#include <string.h>

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "Simulation.h"

#include "Buffer.h"
#include "MsgBuffer.h"

#include "GlobalData.h"
#include "nav2util.h"
#include "Log.h"

/* RCSINFO(driv_new_sim_master,
*/



#define ESCAPE_CHAR             0xf0
#define ESCAPE_REPLACEMENT_CHAR 0x00
#define TERMINATE_CHAR          0x7f

#define DISPATCHER_PRIO 6
#define INDUCED_SLACK_AFTER_RECONNECT 300

#define TRIMBLE_MAX_SIZE 50
#define DLE 0x10
#define ETX 0x03


#include "LogMacros.h"

/* Data types used by the decoder */
#define MAGIC_FLAG_RECONNECTED 0x00000100

namespace isab {

#ifdef SIM_DATA_FROM_FILE
   Simulation::Simulation(SerialProviderPublic *p, const char* simFile) :
#else
   Simulation::Simulation(SerialProviderPublic *p) :
#endif
      Module("Simulation"), m_provider(p)
   {
      SerialConsumerPublic * m_consumer;

      DBG("Constuctor\n");
      DBG("Simdata from %s", simFile);
      m_shutdown = false;

      /* Start the decoder in a known state */
      decoder.state = decoder_idle;

      /* We are always disconnected upon startup */
      decoder.reconnected = 1;
      decoder.cmdBeingAssembled = NULL;

      /* Set up the trimble decoding system */
      trimble.state = state_idle;
      trimble.base_time = 0;
      elapsed_simulation_time = 0;

      dispatcher.timerId = m_queue->defineTimer();
      dispatcher.next_command = NULL;

      /* Do the connections */
      if(p){
         p->setOwnerModule(this);
         m_consumer=new SerialConsumerPublic(m_queue);
         p->connect(m_consumer);
         newLowerModule(p);
      }

#ifdef SIM_DATA_FROM_FILE
      m_fileSimData = new Buffer(1024 * 100); //100 kB.
      m_alreadyTriedToOpenFile = false;
#if defined(__SYMBIAN32__)
      char fileName[256];
      strcpy(fileName, getGlobalData().m_commondata_base_path);
      if (simFile) {
         strcat(fileName, simFile);
      } else {
         strcat(fileName, "simfile");
      }
      m_simDataFile = strdup_new(fileName);
#else
      m_simDataFile = strdup_new(simFile);
#endif
#endif

   }

   SerialProviderPublic * Simulation::newPublicSerial()
   {
      DBG("newPublicSerial()\n");
      return new SerialProviderPublic(m_queue);
   }
   
   void Simulation::decodedStartupComplete()
   {
      Module::decodedStartupComplete();

#ifdef SIM_DATA_FROM_FILE
      if(m_simDataFile){
         DBG("decodedStartupComplete");
         
         if (m_fileSimData->getWritePos() == 0){
            // read data from file.
            DBG("if (m_fileSimData->getWritePos() == 0");
            
            if (!m_alreadyTriedToOpenFile){
               DBG("if (!m_alreadyTriedToOpenFile)");
               

// #if defined(__SYMBIAN32__)
//                const char* fileName = "\\system\\apps\\wayfinder\\data.sim";
// #else
//                const char* fileName = "data.sim";
// #endif
               const char* fileOpenFlags = "rb";
               FILE* simFile = fopen(m_simDataFile, fileOpenFlags);
               
               if (simFile == NULL){
                  DBG("if (simFile == NULL)");
                  m_alreadyTriedToOpenFile = true;
               }
               else{
                  DBG("if (simFile != NULL)");
                  bool eof = false;
                  const size_t tmpBufSize = 256;
                  uint8 tmpBuf[tmpBufSize]; 
                  while (!eof){
                     size_t readBytes = fread(tmpBuf, sizeof(uint8), 
                                              tmpBufSize, simFile);
                     if (readBytes < tmpBufSize){
                        eof = true;
                     }
                     m_fileSimData->writeNextByteArray(tmpBuf, readBytes);
                  }
                  
                  int result = fclose(simFile);
                  if (result != 0){
                     WARN("Simulation could not close sim data file.");
                  }
               }
               

            }
         }
         
         simDataFromFileToDecoding();
      }
#endif

   }


   void Simulation::decodedShutdownPrepare( int16 upperTimeout )
   {
      m_queue->removeTimer(dispatcher.timerId);
      m_shutdown = true;
      Module::decodedShutdownPrepare( upperTimeout );

      delete decoder.cmdBeingAssembled;
      decoder.cmdBeingAssembled = NULL;
#ifdef SIM_DATA_FROM_FILE
      delete m_fileSimData;
#endif
      while (!dispatcher.cmdQueue.empty()) {
         delete dispatcher.cmdQueue.front();
         dispatcher.cmdQueue.pop();
      }
   }

   MsgBuffer * Simulation::dispatch(MsgBuffer *buf)
   {
      DBG("dispatch : %i\n", buf->getMsgType());
      buf=m_consumerDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=m_serialProviderDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=Module::dispatch(buf);
      return buf;
   }

   SerialConsumerPublic * Simulation::rootPublic()
   {  
      return reinterpret_cast<SerialConsumerPublic *>(m_rawRootPublic);
   }

   void Simulation::decodedReceiveData(int length, const uint8 *data, uint32 /*src*/)
   {
      DBG("decodedReceiveData(%i, xxxx)\n", length);
      if (m_shutdown) {
         return;
      }
      decode_data(data, length, 0, 0);
   }

   
   /* *********************************************
    * Dispatch received commands in a timely 
    * fashion.
    * ******************************************* */



   void Simulation::setupForNextCommand()
   {
      bool reconnect=false;
      isabTime execute_at;
      int delay_ms;

      /* We should never _never_ get here with the timer 
       * already running, or expired but unserved. */

      dispatcher.next_command = dispatcher.cmdQueue.front();
      dispatcher.cmdQueue.pop();
      delay_ms = conv_betoh_int32(dispatcher.next_command->buf);

      execute_at = dispatcher.last_exec_time;
      execute_at.add(delay_ms);
      if (execute_at < dispatcher.next_command->rxTime) {
         reconnect = true;
      }

      if (dispatcher.next_command->reconnect) {
         reconnect = true;
      }

      /* We get here after reconnects or slips */
      if (reconnect) {
         INFO("Reconnected in setupForNextCommand\n");
         execute_at.now();
         execute_at.add(INDUCED_SLACK_AFTER_RECONNECT);
         reconnect = 0;
      }

//      os_timer_set_time(self->dispatcher.timer, execute_at);
//      os_timer_start(self->dispatcher.timer);
//      os_semaphore_wait(self->dispatcher.timer_sem);

//      Add code to add data for the next buffer in variables in dispatcher.

      DBG("Should execute at %"PRId32", delay_ms %i, last_exec %"PRId32, 
            execute_at.millis(), delay_ms, 
            dispatcher.last_exec_time.millis() );
      dispatcher.last_exec_time = execute_at;

      m_queue->setTimer(dispatcher.timerId, execute_at);
   }


   void Simulation::decodedExpiredTimer(uint16 timerno)
   {
      float gps_time_now;
      int size;
      uint8 *bufptr;

      if (dispatcher.timerId != timerno) {
         Module::decodedExpiredTimer(timerno);
         return;
      }
      if (m_shutdown) {
         return;
      }
      if (!dispatcher.next_command) {
         ERR("Spurious timer expire\n");
         return;
      }
      /* Skip the delay */
      bufptr = dispatcher.next_command->buf + 4; 
      size = dispatcher.next_command->numBytes - 4;

      switch (dispatcher.next_command->pktType) {
      case 1:
         /* Trimble payload - generic packet */
         DBG("new_sim: trimble payload %i long", size);
         if (rootPublic()) {
            rootPublic()->receiveData(size, bufptr);
         }
         break;

      case 2:
         /* Trimble payload - unbytestuffed packet needing time fixup.
            Used for position/velocity packet (0x4a/0x56). Byte 0 holds
            the offset where the time is to be inserted. */
         gps_time_now = trimble.base_time + elapsed_simulation_time / 1000.0F;
         memcpy(bufptr + bufptr[0], &gps_time_now, 4);
         break;

      default:
         WARN("unknown pkttype %i\n", dispatcher.next_command->pktType);
         break;
      }

      delete(dispatcher.next_command);
      dispatcher.next_command = NULL;

      if (!dispatcher.cmdQueue.empty()) {
         setupForNextCommand();
      }
#ifdef SIM_DATA_FROM_FILE
      else if(m_simDataFile){
         simDataFromFileToDecoding();         
      }
#endif
   }


   /* *********************************************
    * Data stream decoding
    * ******************************************* */

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
   inline void Simulation::append_data(uint8 data)
   {
      /* Not an escape char -> another char for the buffer */
      if (decoder.cmdBeingAssembled->numBytes < MaxMsgSize) {
//         decoder.buf.writeNext8bit(data);
         decoder.cmdBeingAssembled->buf[decoder.cmdBeingAssembled->numBytes]=data;
         decoder.cmdBeingAssembled->numBytes++;
      } else {
         /* Too large packet received */
         WARN("too large packet (type %i)\n", 
               decoder.cmdBeingAssembled->pktType);
         delete(decoder.cmdBeingAssembled);
         decoder.cmdBeingAssembled = NULL;
         decoder.state = decoder_idle;
      }
   }


   /*+*******************************************************
   * Decode simulation data
   *
   * driv_new_sim_self_p self
   *        This simulator instance. Normally only one 
   *        ever exists. 
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
   void
   Simulation::decode_data(const uint8 * data_stream,
                          int32 data_size,
                          int reconnect,
                          int /*blocking*/)
   {
      int32 i;
      uint8 data;
      isabTime now;

      /* Remember a reconnect until a packet is dispatched succesfully */
      decoder.reconnected = decoder.reconnected || reconnect;
      if (reconnect) {
         if (decoder.state == decoder_in_packet ||
             decoder.state == decoder_in_packet_got_escape) {
            delete(decoder.cmdBeingAssembled);
            decoder.cmdBeingAssembled = NULL;
         }
         decoder.state = decoder_idle;
      }

      for (i = 0; i < data_size; i++) {
         data = data_stream[i];
         switch (decoder.state) {

         case decoder_idle:
            /* Hunt for escape to start a new frame */
            if (data == ESCAPE_CHAR) {
               decoder.state = decoder_idle_got_escape;
            }
            break;

         case decoder_idle_got_escape:
            /* Check for proper frame start */
            if (data >= 0x01 && data <= 0x7e) {
               if (decoder.cmdBeingAssembled) {
                  delete decoder.cmdBeingAssembled;
               }
               decoder.cmdBeingAssembled = new struct SimCommand;
               decoder.cmdBeingAssembled->rxTime = now;
               decoder.cmdBeingAssembled->pktType = data;
               decoder.cmdBeingAssembled->numBytes = 0;
               decoder.state = decoder_in_packet;
               break;
            }
            decoder.state = decoder_idle;
            break;

         case decoder_in_packet:
            if (data == ESCAPE_CHAR) {
               decoder.state = decoder_in_packet_got_escape;
               break;
            }
            append_data(data);
            break;

         case decoder_in_packet_got_escape:
            if (data == TERMINATE_CHAR) {
               DBG("got complete packet of type %i, length %i\n",
                      decoder.cmdBeingAssembled->pktType, 
                      decoder.cmdBeingAssembled->numBytes);
               if (decoder.cmdBeingAssembled->numBytes < (4+1) ) {
                  ERR("which was too short (only %i < 5 bytes)\n",
                        decoder.cmdBeingAssembled->numBytes);
               }
               decoder.cmdBeingAssembled->reconnect = (decoder.reconnected != 0);
               decoder.reconnected = 0;
               dispatcher.cmdQueue.push(decoder.cmdBeingAssembled);
               decoder.cmdBeingAssembled = NULL;
               if (dispatcher.next_command == NULL) {
                  /* Was empty before, need to kickstart it */
                  setupForNextCommand();
               }
               decoder.state = decoder_idle;
               break;
            }

            if (data == ESCAPE_REPLACEMENT_CHAR) {
               /* State may be overridden by append_data */
               decoder.state = decoder_in_packet;
               append_data(ESCAPE_CHAR);
               break;
            }
            /* Any other data is a new packet. Ignore it, and 
             * synchronize to the next packet. */
            delete(decoder.cmdBeingAssembled);
            decoder.cmdBeingAssembled = NULL;
            decoder.state = decoder_idle;
            break;
         }
      }
   }




   /* Definitions for the const static data in the class declaration. */
   /** Software version */
   const uint8 Simulation::simulated_pkt45[] = {
      0x10, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x03 };
   /** Helth of receiver */
   const uint8 Simulation::simulated_pkt46[] =
      { 0x10, 0x46, 0x00, 0x00, 0x10, 0x03 };
   /** Differential fix mode */
   const uint8 Simulation::simulated_pkt82[] =
      { 0x10, 0x82, 0x01, 0x10, 0x03 };


   /*+***********************************************************
    * Respond to commands the same way a Trimble ACE-III 
    * would.
    * 
    ***********************************************************+*/
   void
   Simulation::trimble_interpret_packet(uint8 * bufptr,
                                        int /*size*/)
   {
      DBG("trimble_ip: got %x\n", bufptr[0]);
      switch (bufptr[0]) {
      case 0x25:
         /* Soft reset */
         rootPublic()->receiveData(sizeof (simulated_pkt45), simulated_pkt45);
         rootPublic()->receiveData(sizeof (simulated_pkt46), simulated_pkt46);
         rootPublic()->receiveData(sizeof (simulated_pkt82), simulated_pkt82);
         break;

      }
   }

   /*+***********************************************************
    * Decode data sent to a Trimble ACE-III gps receiver.
    * 
    * This function is an almost verbatim copy of 
    * driv_nav_trimble_receive_buf().
    *
    * Returns 0 on failure and 1 on success. Currently sucess is 
    * always returned.
    *
    * This function is called from the serial port protocol. We 
    * remove the DLE escapes and try to reassemble a packet. If
    * there is an unfinsihed packet when the input buffer ends the
    * current state of the packet reassembly state machine is stored
    * in self->state, self->saved_outlen and self->saved_outpos.
    ***********************************************************+*/
   void
   Simulation::decodedSendData(int inlen, const uint8 *data, uint32 /*src*/)
   {
      const uint8 *currinpos;             /* Position in the in buffer */
      char databyte;               /* Byte being examined */

      DBG("decodedSendData(%i, xxxx)\n", inlen);
      currinpos = data;     /* Prepare the input data */

      while (inlen) {
         databyte = *currinpos;
         switch (trimble.state) {
         case state_idle:
            if (databyte == DLE)
               trimble.state = state_after_initial_dle;
            break;
         case state_in_packet_after_dle:
            /* We received a DLE inside a packet. It is either the start of a */
            /* new packet (an error really), the end of the current packet    */
            /* (if it is followed by a ETX) or just an escaped DLE            */
            if (databyte == DLE) {
               if (trimble.outlen >= (signed int)(sizeof (trimble.reassmebled_pkt))) {
                  trimble.state = state_idle;        /* Oversized packet, drop it */
                  ERR("TrimbleDecode: Oversized packet after DLE!\n");
                  break;
               }
               *(trimble.outpos++) = databyte; /* DLE DLE => DLE */
               trimble.outlen++;
               trimble.state = state_in_packet;
               break;
            } else if (databyte == ETX) {
               trimble_interpret_packet(trimble.reassmebled_pkt,
                                        trimble.outlen);
               trimble.state = state_idle;
               break;
            }
            /* Fall through and process as a new packet */
         case state_after_initial_dle:
            switch (databyte) {
            case ETX:             /* DLE ETX => end of packet outside packet */
            case DLE:
               trimble.state = state_idle;   /* DLE DLE outside a packet is nonsense */
               ERR("TrimbleDecode: Bad packet after initial DLE!\n");
               break;              /* We are probably out of sync */
            default:
               trimble.outpos = trimble.reassmebled_pkt;
               *(trimble.outpos++) = databyte;
               trimble.outlen = 1;
               trimble.state = state_in_packet;

            }
            break;                 /* switch (databyte) in state_after_initial_dle */
         case state_in_packet:
            if (databyte == DLE) {
               trimble.state = state_in_packet_after_dle;
               break;
            }
            if (trimble.outlen >= (signed int)(sizeof (trimble.reassmebled_pkt))) {
               trimble.state = state_idle;   /* Oversized packet, drop it */
               ERR("TrimbleDecode: Oversized packet!\n");
               break;
            }
            *(trimble.outpos++) = databyte;
            trimble.outlen++;
            break;
         }
         currinpos++;
         inlen--;
      }

   }


#ifdef SIM_DATA_FROM_FILE
   void Simulation::simDataFromFileToDecoding()
   {
      DBG("simDataFromFileToDecoding.");


      
      if (m_fileSimData->getWritePos() != 0){
         DBG("if (m_fileSimData->getWritePos() != 0)");

         if (m_fileSimData->remaining() == 0){
            DBG("if (m_fileSimData->remaining() == 0)");

            m_fileSimData->setReadPos(0);
         }

         const size_t bufToDecodeAlloc = 512;
         uint8 bufToDecoding[bufToDecodeAlloc];

         size_t bufToDecodingSize = bufToDecodeAlloc;
         if (m_fileSimData->remaining() < bufToDecodingSize){
            DBG("if (m_fileSimData->remaining() < bufToDecodingSize)");

            bufToDecodingSize = m_fileSimData->remaining();
         }
         m_fileSimData->readNextByteArray( bufToDecoding,
                                           bufToDecodingSize);
         
         uint32 src = 0;
         decodedReceiveData(bufToDecodingSize, bufToDecoding, src);

         if (m_fileSimData->remaining() <= 0){
            m_fileSimData->setReadPos(0);
         }
      }
   
   
   }
#endif

} /* namespace isab */

