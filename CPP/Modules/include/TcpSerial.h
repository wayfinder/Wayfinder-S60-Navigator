/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULES_TcpSerial_H
#define MODULES_TcpSerial_H

#include "arch.h"
#include "Module.h"
#include "Serial.h"
#include "ParameterEnums.h"
#include "Parameter.h"

namespace isab {
   /** Forward declaration. */
   class TcpThread;
   
   /** Class used as a bridge between a tcp stream and a serial stream. 
       Can be used both as a server socket and as a client socket. */
   class TcpSerial
      : public Module,
        public SerialProviderInterface,
        public ParameterConsumerInterface
   {
      /** TcpThread needs to access rootPublic.*/
      friend class TcpThread;
      
   public:
      /** @name Constructors and destructor. */
      //@{
      /** Constructor for a TcpSerial waiting for orders. 
       * @param name the name of the TcpSerial thread, also used as the 
       *             prefix in logging output.
       */
      TcpSerial(const char* name);

      /** Constructor for a TcpSerial acting as a server socket.
       * @param port the port number to listen at. 
       * @param name the name of the TcpSerial thread, also used as the 
       *             prefix in logging output. 
       */
      TcpSerial(int port, const char* name);

      /** Constructor for a TcpSerial acting as a client socket, 
       * connecting to a server somewhere. 
       * @param hostname the host to connect to.
       * @param port the port to connect to.
       * @param name the name of the TcpSerial thread, also used as the 
       *             prefix in logging output.
       */
      TcpSerial(const char *hostname, int port, const char* name);

      /** Virtual destructor. */
      virtual ~TcpSerial();
      //@}

      /** Creates a new SerialProviderPublic object used to reach this 
       * module.
       * @return a new SerialProviderPublic object connected to the queue.
       */
      class SerialProviderPublic * newPublicSerial();

      /** @name From Module. */
      //@{
      virtual void decodedStartupComplete();
      virtual void decodedShutdownPrepare(int16 upperTimeout);
      virtual void decodedExpiredTimer(uint16 timerID);
      //@}

      /** @name From SerialProviderInterface */
      //@{
      /** Handles incoming data relayed from the public interface. 
       * @param length the number of bytes to deal with. 
       * @param data   a pointer to an array of char at least length 
       *               bytes long.
       */
      virtual void decodedSendData(int length, const uint8 *data, uint32 src);

      /** Generic order to control the connection to the peer module.
       * @param ctrl can be CONNECT, DISCONNECT, or QUERY. The action 
       *             performed when CONNECT is sent depends on the method
       *             argument. DISCONNECT ignores the method argument.
       *             QUERY doesn't affect the connection but causes the
       *             module to send a connectionNotify to the sender of
       *             the connectionctrl message.
       * @param method only affects CONNECT requests. The string shall be 
       *               of the form [<host>:]<port>. If only a port is given,
       *               the TcpSerial object will listen at that port. If a
       *               host and a port i given, the TcpSerial will try to
       *               connect to the specified host and port. All 
       *               non-conforming messages will be ignored.
       * @param src the sender of this message.
       */
      virtual void decodedConnectionCtrl(enum ConnectionCtrl ctrl, 
                                         const char *method, uint32 src);
      //@}

      /** @name From ParameterConsumerInterface. */
      //@{
      virtual void decodedParamValue(uint32 paramId, const int32 * data, 
                                     int32 numEntries, uint32 src, uint32 dst);

      virtual void decodedParamValue(uint32 /*paramId*/, const float * /*data*/,
                                     int32 /*numEntries*/, uint32 /*src*/,
                                     uint32 /*dst*/)
      {
         /*This function intentionally left blank*/
      }

      virtual void decodedParamValue(uint32 paramId, const char * const * data,
                                     int32 numEntries, uint32 src, uint32 dst );

      /** Delivers a binary block parameter.*/
      virtual void decodedParamValue(uint32 /*paramId*/, const uint8* /*data*/, 
                                     int /*size*/, uint32 /*src*/, 
                                     uint32 /*dst*/)
      {
         /*This function intentionally left blank*/
      }

      virtual void decodedParamNoValue(uint32 paramId, uint32 src, uint32 dst);
      //@}

      /** 
       * Used by TcpThread to send connectionNotify messages. 
       * @param ctrl The message to send. 
       */
      void connectionNotify(enum ConnectionNotify ctrl);

      class DNSCache& getDNSCache();
#ifdef __SYMBIAN32__
      virtual void Kill(int reason,Thread* originator);
#endif
   protected:
      /** This function is used in this class to send messages to the
       * Module immediatly above us, i.e. closer to the CtrlHub. */
      class SerialConsumerPublic * rootPublic();
      /** Decoder for SerialProvider-messages */
      class SerialProviderDecoder m_providerDecoder;
      /** Sender for parameter messages. */
      class ParameterProviderPublic* m_paramProvider;
      /** Decoder for ParameterConsumer messages. */
      class ParameterConsumerDecoder m_paramDecoder;
      
      /** Decides what should be done with incoming Buffers. */
      virtual MsgBuffer * dispatch(MsgBuffer *buf);
         
      /** The reading thread. Handles all things related to the TCP socket. */
      TcpThread * m_tcpThread;

      /** serial number for tcp threads.*/
      int m_threadNum;
      /** The current state of the TcpSerial connection. */
      ConnectionNotify m_state;

      virtual void treeIsShutdown();
      /**
       * Does the actual sending of connectionNotify messages. Handles
       * some bookkeeping related to this.
       * @param msg The notification message. 
       * @param rsn The reason for the message.
       * @param dst The address of the receiver of the message.  
       */
      void sendConnectionNotify(enum ConnectionNotify msg, 
                                enum ConnectionNotifyReason rsn, uint32 dst);
      /** @name TcpThread-handling functions. */
      //@{
      /** Deletes the tcpthread and sets the pointer to NULL. */
      void closeThread();
      /**
       * Creates a new TcpThread object using the given parameters. 
       * @param port      The port parameter to use in TcpThreads constructor. 
       * @param hostname  The host parameter to use in TcpThreads
       *                  constructor. Defaults to NULL.
       * @param reconnect The reconnect parameter to use in TcpThreads
       *                  constructor. Defaults to 0.
       */
      void newThread(int port, const char* hostname = NULL, int reconnect = 0);
      /**
       * Creates and starts a new TcpThread. The thread is created
       * using the newThread function.
       * @param port      The port parameter to use in TcpThreads constructor. 
       * @param hostname  The host parameter to use in TcpThreads
       *                  constructor. Defaults to NULL.
       * @param reconnect The reconnect parameter to use in TcpThreads
       *                  constructor. Defaults to 0.
       * @return The startStatus of the call to the threads start funciton.
       */
      startStatus startNewThread(int port, const char* hostname = NULL, 
                                 int reconnect = 0);
      //@}

#if (_WIN32_WCE >= 300)
      /**
       * Handle to Connection Manager network connection.
       */ 
      HANDLE m_phConnection;

      /***/
      bool openConnection(const char* hostname);
      void closeConnection();
         
#endif
      /** The timeout time for keeping the connection open.*/
      uint32 m_keepConnectionTimeout;

      /** The timer id for keeping track of connections.
       *  This is used for _two different_ purposes. Under WCE
       *  it is used to keep the physical channel open (GPRS etc).
       *  Under Symbian it is used as a HACK to keep the tcp channel
       *  open even after a close in the hope that this will allow
       *  reuse of the tcp connection. The correct fix is to modify
       *  NSC. */
      uint16 m_connectionTimer;

      int32 m_connectionParam;

      enum ConnectionNotify m_lastSent;

      class DNSCache* m_dnsCache;
   };


} /* namespace isab */

#endif /* MODULES_TcpSerial_H */
