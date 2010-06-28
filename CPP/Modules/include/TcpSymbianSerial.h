/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULES_TcpSymbianSerial_H
#define MODULES_TcpSymbianSerial_H

class CThreadOwner;
class CTcpAdmin;
namespace isab {
   /** Class used as a bridge between a tcp stream and a serial stream. 
       Can be used both as a server socket and as a client socket. */
   class TcpSymbianSerial
       : public Module,
         public SerialProviderInterface,
         public ParameterConsumerInterface
   {
   public:
      /** @name Constructors and destructor */
      //@{
      /** Constructor for a TcpSymbianSerial waiting for orders. 
       * @param name the name of the TcpSymbianSerial thread, also used as the 
       *             prefix in logging output.
       */
      TcpSymbianSerial(const char* name, 
                       int considerWLAN);

      /** Virtual destructor. */
      virtual ~TcpSymbianSerial();
      //@}

      /** 
       * Creates a new SerialProviderPublic object used to reach this
       * module.
       * @return a new SerialProviderPublic object connected to the queue.
       */
      class SerialProviderPublic* newPublicSerial();

      /** @name From Thread. */
      //@{
#ifdef __SYMBIAN32__
      virtual void Kill(int reason, class Thread* originator);
#endif
      /** Decides what should be done with incoming Buffers. */
      virtual class MsgBuffer * dispatch(MsgBuffer *buf);         
      //@}

      /** @name From Module. Reimplemented here. */
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
       *               the TcpSymbianSerial object will listen at that port. If a
       *               host and a port i given, the TcpSymbianSerial will try to
       *               connect to the specified host and port. All 
       *               non-conforming messages will be ignored.
       * @param src the sender of this message.
       */
      virtual void decodedConnectionCtrl(enum ConnectionCtrl ctrl, 
                                         const char *method, uint32 src);
      //@}

      /** @name From ParameterConsumerInterface. */
      //@{
      virtual void decodedParamValue(uint32 paramId,
                                     const int32 * data,
                                     int32 /*numEntries*/,
                                     uint32 /*src*/,
                                     uint32 /*dst*/);

      virtual void decodedParamValue(uint32 /*paramId*/,
                                     const float * /*data*/,
                                     int32 /*numEntries*/,
                                     uint32 /*src*/,
                                     uint32 /*dst*/)
      {/*This function intentionally left blank*/}

      virtual void decodedParamValue(uint32 paramId,
                                     const char * const * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst);

      /** Delivers a binary block parameter.*/
      virtual void decodedParamValue(uint32 /*paramId*/, 
                                     const uint8* /*data*/, 
                                     int /*size*/,
                                     uint32 /*src*/,
                                     uint32 /*dst*/)
      {/*This function intentionally left blank*/}

      virtual void decodedParamNoValue(uint32 paramId,
                                       uint32 src,
                                       uint32 dst);
      //@}

      /** 
       * Receive data from TCP.
       * @param data pointer to the data.
       * @param length the length of the data area.
       * @return true if the data was sent somewhere. 
       */
      bool receive(const uint8* data, int length);
      
      /**
       * Receives connection events from the TCP thread and passes
       * them on to the next module.
       * @param ctrl   the connection event.
       * @param reason the reason for the event. 
       */
      void connectionNotify(enum ConnectionNotify ctrl,
                            enum ConnectionNotifyReason reason);

      void tcpThreadPanic(int reason);


      /** XXX FIXME ONLY FOR DEBUGGING. REMOVE BEFORE RELEASE. */
      void debugProdMe(void);
   private:
      void handleConnect(const char* method, uint32 src);
      void handleDisconnect(const char* method, uint32 src);

   protected:
      /** @name Member variable used for inter-module communication.*/
      //@{
      /** This function is used in this class to send messages to the
       * Module immediatly above us, i.e. closer to the CtrlHub. */
      class SerialConsumerPublic * rootPublic();
      /** Decoder for SerialProvider-messages */
      class SerialProviderDecoder m_providerDecoder;
      class ParameterProviderPublic* m_paramProvider;
      class ParameterConsumerDecoder m_paramDecoder;
      //@}

      /** @name Member variables used to handle the TCP thread. */
      //@{
      /** The reading thread. Handles all things related to the TCP socket. */
      /*      TcpThread * m_tcpThread;*/
      class CThreadOwner* m_threadHolder;
      class CTcpAdmin* m_tcpAdmin;
      //@}

      /** @name Connection state variables. */
      //@{
      /** The current state of the TcpSymbianSerial connection. */
      enum ConnectionNotify m_state;
      enum ConnectionNotify m_lastSent;
      //@}

      void sendConnectionNotify(enum ConnectionNotify, 
                                enum ConnectionNotifyReason, uint32 src);

      void closeThread();
      void newThread(int port, const char* hostname = NULL, int reconnect = 0);
      enum startStatus startNewThread(int port, const char* hostname = NULL, 
                                      int reconnect = 0);
      void resetThread(bool restart = true);

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

      bool m_resetThread;
      bool m_tcpThreadPanicked; 
      bool m_considerWLAN;
   };

} /* namespace isab */

#endif /* MODULES_TcpSymbianSerial_H */
