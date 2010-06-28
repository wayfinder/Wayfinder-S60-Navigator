/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ERROR_MODULE_H
#define ERROR_MODULE_H

#include "Module.h"

namespace isab {


/** This module is only an interface to use when sending 
  * unsolicited error messages. Those sending the errors
  * should keep an ErrorProviderPublic member and those
  * listening to the errors should implement 
  * ErrorProviderInterface.
  */
class ErrorProviderPublic : public ModulePublic {
   public:
      ErrorProviderPublic(Module *m) : 
         ModulePublic(m) { 
            setDefaultDestination(uint32(Module::ErrorMessages) | 
                                  uint32(MsgBufferEnums::ADDR_MULTICAST_UP) );
         };
      /**
       * Send an unsolicited error to whoever listens.
       *
       * @param theError Number of the error.
       * @param dst Use this method to send the error message
       *            to a specific module instead of multicasting it.
       */
      uint32 unsolicitedError( const ErrorObj& err,
                               uint32 dst = MsgBufferEnums::ADDR_DEFAULT );
};




/** An interface that must be implemented by the module(s)
 *  handling the errors.
 */
class ErrorProviderInterface {
   public:
      /**
       * @param theError The unsolicited error message.
       * @param src      Address of the module sending the error.
       */
      virtual void decodedUnsolicitedError( const ErrorObj& err, 
                                            uint32 src ) = 0;
};


/** An object of this class is owned by each module that implements 
 *  the ErrorProviderInterface. This object contains a dispatch()
 *  function that does the actual decoding of an incoming buffer.
 *  It then call the apropriate methods in ErrorProviderInterface.
 */
class ErrorProviderDecoder {
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf, ErrorProviderInterface *m );
};

} /* namespace isab */

#endif /* ERROR_MODULE_H */
