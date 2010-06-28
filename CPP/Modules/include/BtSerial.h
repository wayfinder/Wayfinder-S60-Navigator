/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MODULES_BtSerial_H
#define MODULES_BtSerial_H

#include <arch.h>
#include "Module.h"
#include "Serial.h"
#include "BtManager.h"
#include "BtIfClasses.h"

namespace isab {
   /** Class used as a bridge between a BlueTooth connection and a serial stream. */
   class BtSerial : public Module, public SerialProviderInterface, 
                    public BtManagerConsumerInterface,
                    public RecursiveMutex,
                    CRfCommPort
   {
   public:
      BtSerial(const uint8 bda[6], const char *name = NULL);

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

      virtual void decodedBtFoundDevice(uint8 bda[6], uint32 dst);
      virtual void decodedBtSerialResult(bool ok, uint8 scn, uint8 bda[6], 
            TCHAR *service_name, GUID guid, uint32 dst);


   protected:

      /* Address of requested bluetooth device */
      uint8 m_targetBda[6];

      /* True if in shutdown */
      bool m_shuttingDown;

      enum State {
         ShuttingDown, 
         Idle, 
         Searching, 
         AwaitingSerialResult,
         Connected,
      };

      enum State m_state;

      /** This function is used in this class to send messages to the
       * Module immediatly above us, i.e. closer to the CtrlHub. */
      inline SerialConsumerPublic * rootPublic() {
         return reinterpret_cast<SerialConsumerPublic *>(m_rawRootPublic);
      }
      
      /** Decoder for SerialProvider-messages */
      SerialProviderDecoder m_providerDecoder;
      
      /** Decoder for BtManager-messages */
      BtManagerConsumerDecoder m_btDecoder;
      
      /** Encoder for BtManager-messages */
      BtManagerProviderPublic *m_btMgr;
      
      /** Decides what should be done with incoming Buffers. */
      virtual MsgBuffer * dispatch(MsgBuffer *buf);
         
      virtual void treeIsShutdown();

      virtual void decodedStartupComplete();

      virtual void OnDataReceived(void *p_data, UINT16 len);
      virtual void OnEventReceived(UINT32 event_code);

      void searchForPartner();

   };

} /* namespace isab */

#endif /* MODULES_BtSerial_H */
