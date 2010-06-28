/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* The CtrlHub interface is really two parts - a method to encode and pass 
 * messages to the actual CtrlHub to register/deregister multicastaddresses
 * etc, and also a message routing facility.
 */


#include "Buffer.h"
#include "MsgBufferEnums.h"
#include "MsgBuffer.h"

#include "ModuleQueue.h"
#include "CtrlHub.h"

namespace isab {
   
void CtrlHubPublic::preformedMessage(MsgBuffer *buf)
{
//   cout << "CtrlHubPublic::preformedMessage " << endl;
//   delete buf;
   ModulePublic::preformedMessage(buf);
}

int CtrlHubPublic::startShutdownProcedure()
{
   uint32 src = m_owner->getRequestId();
   MsgBuffer * cmdbuf = new MsgBuffer(uint32(0), src, 
         MsgBufferEnums::CTRLHUB_START_SHUTDOWN_PROCEDURE, 0);
   m_queue->insert(cmdbuf);
   return src;
}

uint32 CtrlHubAttachedPublic::connect(CtrlHubPublic *consumer)
{
//   cout << "CtrlHubAttachedPublic::connect" << endl;
   return lowlevelConnectTo(consumer);
}



MsgBuffer * CtrlHubDecoder::dispatch(MsgBuffer *buf, CtrlHubInterface *m )
{
   int length;

   //cout << "CtrlHubDecoder::dispatch : " << buf->getMsgType() << endl;
   length=buf->getLength();
   switch (buf->getMsgType()) {
      case MsgBufferEnums::CTRLHUB_START_SHUTDOWN_PROCEDURE:
         {
            m->decodedStartShutdownProcedure();
            break;
         }
      default:
         return buf;
   }
   delete buf;
   return NULL;
}

MsgBuffer * CtrlHubAttachedDecoder::dispatch(MsgBuffer *buf, CtrlHubAttachedInterface* /*m*/ )
{
   int length;

   //cout << "CtrlHubAttachedDecoder::dispatch : " << buf->getMsgType() << endl;
   length=buf->getLength();
//   switch (buf->getMsgType()) {
//      default:
//         return buf;
//   }
   return buf;

}


} /* namespace isab */
