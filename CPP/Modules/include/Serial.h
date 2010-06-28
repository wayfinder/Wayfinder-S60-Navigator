/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULE_Serial_H
#define MODULE_Serial_H
#include "Module.h"
namespace isab {

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * SerialConsumerInterface. */
class SerialConsumerPublic
   : public ModulePublic
{
   public:
      SerialConsumerPublic(class ModuleQueue *peerQueue) : 
         ModulePublic(peerQueue) { };
      virtual uint32 receiveData(int length, const uint8 *data, 
                                 uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      /** @param buf Warning, the buffer pointer is invalid after this call */
      virtual uint32 receiveData(class Buffer *b, 
                                 uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
};

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * SerialProviderInterface. */
class SerialProviderPublic
   : public ModulePublic
{
   public:
      SerialProviderPublic(class ModuleQueue *peerQueue) : 
         ModulePublic(peerQueue) { };
      virtual uint32 connect(SerialConsumerPublic *consumer);
      virtual uint32 sendData(int length, const uint8 *data, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      /** @param buf Warning, the buffer pointer is invalid after this call */
      virtual uint32 sendData(class Buffer *b, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
};



/* Next are the interfaces. These should be implemented by the 
 * module that wishes to receive the corresponding messages */

/** An interface that must be implemented to receive messages in a
 * serial provider-consumer relationship. This is implemented 
 * in the _upper_ module.
 */
class SerialConsumerInterface
{
   public:
      virtual void decodedReceiveData(int length,
            const uint8 *data,
            uint32 src) = 0;
};

/** An interface that must be implemented to receive messages in a
 * serial provider-consumer relationship. This is implemented 
 * in the _lower_ module.
 */
class SerialProviderInterface
{
   public:
      virtual void decodedSendData(int length,
            const uint8 *data,
            uint32 src) = 0;
};

/** An object of this class is owned by each module that implements 
 * the SerialConsumerInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in SerialConsumerInterface.
 */
class SerialConsumerDecoder
{
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf,
            class SerialConsumerInterface *m );
};

/** An object of this class is owned by each module that implements 
 * the SerialProviderInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in SerialProviderInterface.
 */
class SerialProviderDecoder
{
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf,
            class SerialProviderInterface *m );
};

} /* namespace isab */

#endif /* MODULE_Serial_H */
