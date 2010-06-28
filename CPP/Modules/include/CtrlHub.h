/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* The public interface to the CtrlHub. The CtrlHub is the central 
 * switchboard for the Nav2 system.
 */


#ifndef MODULE_CtrlHub_H
#define MODULE_CtrlHub_H

#include "Module.h"

namespace isab {

/** A class to convert function calls into messages that are 
 * sent from the CtrlHub to a module. That module must implement
 * CtrlHubInterface. */
class CtrlHubPublic : public ModulePublic {
   public:
      CtrlHubPublic(ModuleQueue *peerQueue) : 
         ModulePublic(peerQueue) { };
      CtrlHubPublic(Module *m) : 
         ModulePublic(m) { };

      /** 
       * This is a modified version of the normal posting method used 
       * to shortcut to the right receiver. In the future, when a 
       * common shortcut method is established this may not be 
       * necessary and this function may be removed
       */
      virtual void preformedMessage(class MsgBuffer *buf);

      //
      // CtrlHub: Add mellsage types to be received by the consumer here
      //           These must be added in CtrlHubInterface as well.
      //           Also remember to add them to CtrlHubDecoder.
      // e.g. : virtual int receiveData(int length, char *data);
      virtual int startShutdownProcedure();
};

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * CtrlHubAttachedInterface. */
class CtrlHubAttachedPublic : public ModulePublic {
   public:
      CtrlHubAttachedPublic(ModuleQueue *peerQueue) : 
         ModulePublic(peerQueue) { };
      virtual uint32 connect(CtrlHubPublic *consumer);

      //
      // CtrlHub: Add mellsage types to be received by the provider here
      //           These must be added in CtrlHubAttachedInterface as well.
      //           Also remember to add them to CtrlHubAttachedDecoder.
      // e.g. : virtual int sendData(int length, char *data);
      //
};




/* Next are the interfaces. These should be implemented by the 
 * module that wishes to receive the corresponding messages. 
 * The classes below are pure virtual and correspond in function
 * to "interfaces" in java. */

/** An interface that must be implemented to receive messages in a
 * CtrlHub provider-consumer relationship. This is implemented 
 * in the _upper_ module.
 */
class CtrlHubInterface {
   public:
      //
      // CtrlHub: Add message types to be received by the consumer here.
      //           These must correspond to the ones in the class
      //           CtrlHubPublic.
      // e.g. : virtual void decodedReceiveData(int length, char *data) = 0;
      //
      virtual void decodedStartShutdownProcedure() = 0;
};

/** An interface that must be implemented to receive messages in a
 * CtrlHub provider-consumer relationship. This is implemented 
 * in the _lower_ module.
 */
class CtrlHubAttachedInterface {
   public:
      //
      // CtrlHub: Add message types to be received by the provider here.
      //           These must correspond to the ones in the class
      //           CtrlHubAttachedPublic.
      // e.g. : virtual void decodedSendData(int length, char *data) = 0;
      //
};

/** An object of this class is owned by each module that implements 
 * the CtrlHubInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in CtrlHubInterface.
 */
class CtrlHubDecoder {
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf, CtrlHubInterface *m );
};

/** An object of this class is owned by each module that implements 
 * the CtrlHubAttachedInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in CtrlHubAttachedInterface.
 *
 * @return the buffer if further processing may be done or NULL if 
 *         the buffer was processed and either passed on or 
 *         destroyed and thus no further processing should be done.
 */
class CtrlHubAttachedDecoder {
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf, CtrlHubAttachedInterface *m );
};

} /* namespace isab */

#endif /* MODULE_CtrlHub_H */
