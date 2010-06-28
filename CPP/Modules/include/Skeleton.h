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


#ifndef MODULE_Skeleton_H
#define MODULE_Skeleton_H

#include "Module.h"
#include "Buffer.h"

namespace isab {

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * SkeletonConsumerInterface. */
class SkeletonConsumerPublic : public ModulePublic {
   public:
      SkeletonConsumerPublic(ModuleQueue *peerQueue) : 
         ModulePublic(peerQueue) { };
      //
      // Skeleton: Add mellsage types to be received by the consumer here
      //           These must be added in SkeletonConsumerInterface as well.
      //           Also remember to add them to SkeletonConsumerDecoder.
      // e.g. : virtual uint32 receiveData(int length, char *data, 
      //                                   uint32 dst = MsgBuffer::ADDR_DEFAULT);
      //
};

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * SkeletonProviderInterface. */
class SkeletonProviderPublic : public ModulePublic {
   public:
      SkeletonProviderPublic(ModuleQueue *peerQueue) : 
         ModulePublic(peerQueue) { };
      virtual uint32 connect(SkeletonConsumerPublic *consumer);
      //
      // Skeleton: Add mellsage types to be received by the provider here
      //           These must be added in SkeletonProviderInterface as well.
      //           Also remember to add them to SkeletonProviderDecoder.
      // e.g. : virtual uint32 sendData(int length, char *data,
      //                                uint32 dst = MsgBuffer::ADDR_DEFAULT);
      //
};




/* Next are the interfaces. These should be implemented by the 
 * module that wishes to receive the corresponding messages. 
 * The classes below are pure virtual and correspond in function
 * to "interfaces" in java. */

/** An interface that must be implemented to receive messages in a
 * Skeleton provider-consumer relationship. This is implemented 
 * in the _upper_ module.
 */
class SkeletonConsumerInterface {
   public:
      //
      // Skeleton: Add message types to be received by the consumer here.
      //           These must correspond to the ones in the class
      //           SkeletonConsumerPublic.
      // e.g. : virtual void decodedReceiveData(int length, const char *data, uint32 src) = 0;
      //
};

/** An interface that must be implemented to receive messages in a
 * Skeleton provider-consumer relationship. This is implemented 
 * in the _lower_ module.
 */
class SkeletonProviderInterface {
   public:
      //
      // Skeleton: Add message types to be received by the provider here.
      //           These must correspond to the ones in the class
      //           SkeletonProviderPublic.
      // e.g. : virtual void decodedSendData(int length, const char *data, uint32 src) = 0;
      //
};

/** An object of this class is owned by each module that implements 
 * the SkeletonConsumerInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in SkeletonConsumerInterface.
 */
class SkeletonConsumerDecoder {
   public:
      MsgBuffer * dispatch(MsgBuffer *buf, SkeletonConsumerInterface *m );
};

/** An object of this class is owned by each module that implements 
 * the SkeletonProviderInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in SkeletonProviderInterface.
 *
 * @return the buffer if further processing may be done or NULL if 
 *         the buffer was processed and either passed on or 
 *         destroyed and thus no further processing should be done.
 */
class SkeletonProviderDecoder {
   public:
      MsgBuffer * dispatch(MsgBuffer *buf, SkeletonProviderInterface *m );
};

} /* namespace isab */

#endif /* MODULE_Skeleton_H */
