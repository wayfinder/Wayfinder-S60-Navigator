/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERIAL_MODULE_H
#define SERIAL_MODULE_H
#ifndef __SYMBIAN32__

#include <arch.h>
#include "Module.h"
#include "Serial.h"
#include "Mutex.h"

#ifndef _MSC_VER
#include <termios.h>
#else
#define CRTSCTS 0
#define CLOCAL 0
#define CREAD 0
#define tcflag_t int
#define speed_t int
#endif

namespace isab {

   class SerialThread;


   class SerialModule : public Module, public SerialProviderInterface
   {
   public:
      /**
       *
       */
      SerialModule(int portnumber,
                   int speed,
                   tcflag_t andflags = ~CRTSCTS,
                   tcflag_t orflags = CLOCAL|CREAD,
                   bool blocking = true);


      /** Creates a new SerialProviderPublic object used to reach this 
       * module.
       * @return a new SerialProviderPublic object connected to the queue.
       */
      SerialProviderPublic * newPublicSerial();

      
      /** Handles incoming data relayed from the public interface. 
       * @param length the number of bytes to deal with. 
       * @param data   a pointer to an array of char at least length 
       *               bytes long.
       */
      virtual void decodedSendData(int length, const uint8 *data, uint32 src);

      virtual void decodedConnectionCtrl(enum ConnectionCtrl ctrl, 
                                         const char *method, 
                                         uint32 src);
      int read();

      enum ReadResult {
         INTERRUPTED = -2,  /** The call was interrupted by a signal */
         IO_ERROR = -3,
         FD_ERROR = -4,
         CONNECTED = -5,
         NOT_CONNECTED =-6,/**The device isn't connected or was disconnected.*/
         CONNECTING = -7,   /** The device is still trying to connect */
         RING = -8,         /** Someone is trying to connect.         */
         NO_CONNECTION = -9, /* The connection was terminated */
      } ;

      SerialConsumerPublic * rawRootPublic();
      
   protected:
      /** This function is used in this class to send messages to the
       * Module immediatly above us, i.e. closer to the CtrlHub. */
      SerialConsumerPublic * rootPublic();

      int open();
      int close();
      bool lookupSpeed(int bitrate, speed_t &speed_code);      
      /** Decoder for SerialProvider-messages */
      SerialProviderDecoder m_providerDecoder;
      
      /** Decides what should be done with incoming Buffers. */
      virtual MsgBuffer * dispatch(MsgBuffer *buf);
         
      SerialConsumerPublic* m_serialConsumer;
      
      /**
       */
      int m_portnumber;
      int m_fd;
      int m_speed;
      tcflag_t m_andflags;
      tcflag_t m_orflags; 
      bool m_blockingio;

      enum state_t {
         ERRORE = 0,
         CLOSED,
         OPEN_FOR_WRITE,
         OPEN_READ_WRITE,
      };

      state_t m_state;
#ifdef _MSC_VER
      HANDLE m_hPort;
      uint32 m_fileFlags;
      DCB m_defaultPortDCB;
#endif

      typedef struct speed_pair_t  {
         int speed;
         speed_t speed_code;
      } ;

      static const speed_pair_t speed_table[];
      SerialThread *m_serialthread;

   };

   class SerialThread : public Thread
   {
   public:
      SerialThread(SerialConsumerPublic* scp, SerialModule* sm) : 
         Thread("SerialThread"), m_scp(scp), m_sm(sm), m_stop(false)
      {
      }
      
      virtual void run()
      {
         while (!stop()) {
            m_sm->read();
         }
      }

      virtual void terminate()
      {
         m_mutex.lock();
         m_stop = true;
         m_mutex.unlock();
      }

      virtual ~SerialThread()
      {
         terminate();
         while(isAlive()) {
            join();
         }
      }

   private:
      bool stop(){
         bool ret;
         m_mutex.lock();
         ret = m_stop;
         m_mutex.unlock();
         return ret;
      }

      SerialConsumerPublic* m_scp;
      SerialModule* m_sm;
      bool m_stop;
      Mutex m_mutex;

   };


} /* namespace isab */

#endif
#endif
