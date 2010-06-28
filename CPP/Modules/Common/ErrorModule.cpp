/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* #include "ModuleQueue.h" */
#include "Buffer.h"
#include "MsgBuffer.h"
#include "MsgBufferEnums.h"

#include "Module.h"
#include "Nav2Error.h"
#include "ErrorModule.h"

namespace isab {

uint32 ErrorProviderPublic::unsolicitedError( const ErrorObj& err,
                                              uint32 dst )
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::UNSOLICITED_ERROR_MSG, 64 + err.getSize() );

   err.serialize( buf );
   m_queue->insert(buf);
   return src;
}


MsgBuffer*
ErrorProviderDecoder::dispatch(MsgBuffer *buf, ErrorProviderInterface *m )
{
   switch (buf->getMsgType()) {
      case MsgBufferEnums::UNSOLICITED_ERROR_MSG:
         {
            Nav2Error::ErrorNbr theError = 
               Nav2Error::ErrorNbr( buf->readNextUnaligned32bit() );
            m->decodedUnsolicitedError( theError, buf->getSource() );
            delete buf ;
            return NULL;
         } break;
      default:
         return buf;
   } // switch
}


} /* namespace isab */
