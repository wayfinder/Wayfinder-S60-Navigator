/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpTunnelGuiMess.h"
#include "GuiProt/GuiProtMess.h"
#include "Buffer.h"
#include "NParam.h"
#include "NParamBlock.h"
#include "NavServerComProtoVer.h"
#include <vector>

namespace isab {

#define MAX_PACKET_LENGTH     (60*1024)

class DataGuiMess*
HttpTunnelGuiMess::CreateTunnelMess(int32 messageType,
   uint32 seqNo,
   const char* url,
   uint32 startByte
   )
{
   isab::Buffer *buf = new isab::Buffer(10);
   isab::Buffer *buf2 = new isab::Buffer(128);

   buf2->writeNextUnaligned32bit(messageType);
   buf2->writeNextCharString(url);
   buf2->writeNextUnaligned32bit(seqNo);
   buf2->writeNextUnaligned32bit(startByte + 0);
   buf2->writeNextUnaligned32bit(startByte + MAX_PACKET_LENGTH-1);

   /* To activate test, make sure that GUI_TO_NGP_DATA */
   /* is handled in WAXHandler. */
/* #define TEST_NGP_MESS */
#ifdef TEST_NGP_MESS
   NParamBlock params;
   NParam& p = params.addParam( NParam( 5400 ) );
   p.addByteArray(buf2->accessRawData(), buf2->getLength());

   NParam& p2 = params.addParam( NParam( 5401 ) );
   p2.addByteArray(buf->accessRawData(), buf->getLength());

   Buffer tmpbuf(128);

   std::vector< byte > bbuf;

   params.writeParams(bbuf, NSC_PROTO_VER, false /* NO GZIP */);
   tmpbuf.writeNextByteArray( &bbuf.front(), bbuf.size() );
   const uint8* data = tmpbuf.accessRawData();
   uint32 size = tmpbuf.getLength();

   buf->writeNextUnaligned16bit(0x42);

   class DataGuiMess* mess = new DataGuiMess(
         GuiProtEnums::GUI_TO_NGP_DATA,
         buf->getLength(), buf->accessRawData(),
         size, data);

#else
   class DataGuiMess* mess = new DataGuiMess(
         GuiProtEnums::GUI_TUNNEL_DATA,
         buf->getLength(), buf->accessRawData(),
         buf2->getLength(), buf2->accessRawData());
#endif

   buf->releaseData();
   delete buf;
   buf2->releaseData();
   delete buf2;
   return mess;
}

int32
HttpTunnelGuiMess::ParseTunnelMess(const DataGuiMess* message, HttpTunnelGuiMessObserver* observer)
{
#ifdef TEST_NGP_MESS
   NParamBlock params(message->getAdditionalData(),
      message->getAdditionalSize(), NSC_PROTO_VER);


   const NParam *a = params.getParam(5500);
   const NParam *b = params.getParam(5501);

   if (!a) {
      return -1;
   }

   const uint8* data = b->getBuff();
   uint32 size = b->getLength();

   Buffer* buf = new Buffer(const_cast<uint8 *>(a->getBuff()),
         a->getLength(), a->getLength());
#else
   Buffer* buf = new Buffer(
         const_cast<uint8 *>
         (message->getAdditionalData()),
         message->getAdditionalSize(), message->getAdditionalSize());

   const uint8* data = message->getData();
   uint32 size = message->getSize();
#endif
   int32 type = -1;
   if (buf->remaining()>=4) {
      type = buf->readNextUnaligned32bit();
   }
   switch (type) {
      case -1:
      case 0:
         /* Legacy. */
         break;
      case 1:
         {  
            const char *url = buf->getNextCharString();
            uint32 reqId = buf->readNextUnaligned32bit();
            uint32 from = buf->readNextUnaligned32bit();
            uint32 to = buf->readNextUnaligned32bit();
            uint32 total = buf->readNextUnaligned32bit();

            observer->DataReceived(reqId,
                  from,
                  to,
                  total,
                  url,
                  (const char*)data,
                  size);
         }
      default:
         /* Unknown. */
         break;
   }

   return type;
}

}
