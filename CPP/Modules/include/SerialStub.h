/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERIAL_STUB_H
#define SERIAL_STUB_H

#include <arch.h>
#include "Module.h"
#include "Serial.h"


namespace isab {


   class SerialStub : public Module, public SerialProviderInterface
   {
   public:
      SerialStub();
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
      
   protected:
      /** This function is used in this class to send messages to the
       * Module immediatly above us, i.e. closer to the CtrlHub. */
      SerialConsumerPublic * rootPublic();
      /** Decoder for SerialProvider-messages */
      SerialProviderDecoder m_providerDecoder;
      
      /** Decides what should be done with incoming Buffers. */
      virtual MsgBuffer * dispatch(MsgBuffer *buf);
   };


} /* namespace isab */

#endif
