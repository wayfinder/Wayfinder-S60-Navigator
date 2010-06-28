/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* This is a skeleton version of an interface between two modules. 
 * The lower module (furthest from the CtrlHub) os called the 
 * "provider" and the upper is called "consumer".
 */


#ifndef MODULE_BtManager_H
#define MODULE_BtManager_H

#include "Module.h"
#include "Buffer.h"
#include "BtIfDefinitions.h"

namespace isab {

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * BtManagerConsumerInterface. */
class BtManagerConsumerPublic : public ModulePublic {
   public:
      BtManagerConsumerPublic(Module *m) : 
         ModulePublic(m) { };
      //
      // BtManager: Add mellsage types to be received by the consumer here
      //           These must be added in BtManagerConsumerInterface as well.
      //           Also remember to add them to BtManagerConsumerDecoder.
      // e.g. : virtual uint32 receiveData(int length, char *data, 
      //                                   uint32 dst = MsgBuffer::ADDR_DEFAULT);
      //

      virtual uint32 btFoundDevice(uint8 bda[6], uint32 dst = MsgBuffer::ADDR_DEFAULT);
      virtual uint32 btSerialResult(bool ok, uint8 scn, uint8 bda[6], 
            const TCHAR *service_name, GUID guid, uint32 dst = MsgBuffer::ADDR_DEFAULT);
};

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * BtManagerProviderInterface. */
class BtManagerProviderPublic : public ModulePublic {
   public:
      BtManagerProviderPublic(Module *m) : 
         ModulePublic(m) {
         setDefaultDestination(Module::addrFromId(Module::BtManagerModuleId));
      };
      //
      // BtManager: Add mellsage types to be received by the provider here
      //           These must be added in BtManagerProviderInterface as well.
      //           Also remember to add them to BtManagerProviderDecoder.
      // e.g. : virtual uint32 sendData(int length, char *data,
      //                                uint32 dst = MsgBuffer::ADDR_DEFAULT);
      //

      virtual uint32 btSearchForDevice(uint8 bda[6], uint32 dst = MsgBuffer::ADDR_DEFAULT);
      virtual uint32 btRequestSerial(uint8 bda[6], uint32 dst = MsgBuffer::ADDR_DEFAULT);
};




/* Next are the interfaces. These should be implemented by the 
 * module that wishes to receive the corresponding messages. 
 * The classes below are pure virtual and correspond in function
 * to "interfaces" in java. */

/** An interface that must be implemented to receive messages in a
 * BtManager provider-consumer relationship. This is implemented 
 * in the _upper_ module.
 */
class BtManagerConsumerInterface {
   public:
      //
      // BtManager: Add message types to be received by the consumer here.
      //           These must correspond to the ones in the class
      //           BtManagerConsumerPublic.
      // e.g. : virtual void decodedReceiveData(int length, const char *data, uint32 src) = 0;
      //
      virtual void decodedBtFoundDevice(uint8 bda[6], uint32 dst) = 0;
      virtual void decodedBtSerialResult(bool ok, uint8 scn, uint8 bda[6], 
            TCHAR *service_name, GUID guid, uint32 dst) = 0;
};

/** An interface that must be implemented to receive messages in a
 * BtManager provider-consumer relationship. This is implemented 
 * in the _lower_ module.
 */
class BtManagerProviderInterface {
   public:
      //
      // BtManager: Add message types to be received by the provider here.
      //           These must correspond to the ones in the class
      //           BtManagerProviderPublic.
      // e.g. : virtual void decodedSendData(int length, const char *data, uint32 src) = 0;
      //

      virtual void decodedBtSearchForDevice(uint8 bda[6], uint32 src) = 0;
      virtual void decodedBtRequestSerial(uint8 bda[6], uint32 src) = 0;

};

/** An object of this class is owned by each module that implements 
 * the BtManagerConsumerInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in BtManagerConsumerInterface.
 */
class BtManagerConsumerDecoder {
   public:
      MsgBuffer * dispatch(MsgBuffer *buf, BtManagerConsumerInterface *m );
};

/** An object of this class is owned by each module that implements 
 * the BtManagerProviderInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in BtManagerProviderInterface.
 *
 * @return the buffer if further processing may be done or NULL if 
 *         the buffer was processed and either passed on or 
 *         destroyed and thus no further processing should be done.
 */
class BtManagerProviderDecoder {
   public:
      MsgBuffer * dispatch(MsgBuffer *buf, BtManagerProviderInterface *m );
};

} /* namespace isab */

#endif /* MODULE_BtManager_H */
