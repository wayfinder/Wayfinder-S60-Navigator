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
#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"

#include "Buffer.h"
#include "MsgBuffer.h"

namespace isab {
   
uint32
SerialConsumerPublic::receiveData(int length, const uint8 *data, uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::SERIAL_RECEIVE_DATA,length);

   //cout << "SerialConsumerPublic::receiveData " << length << endl;
   buf->writeNextByteArray(data, length);
   m_queue->insert(buf);
   return src;
}

uint32
SerialConsumerPublic::receiveData(Buffer *b, uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf=new MsgBuffer(b, dst, src, MsgBufferEnums::SERIAL_RECEIVE_DATA);
   m_queue->insert(buf);
   return src;
}



uint32
SerialProviderPublic::sendData(int length, const uint8 *data, uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::SERIAL_SEND_DATA,length);

   //cout << "SerialProviderPublic::sendData " << length << endl;
   buf->writeNextByteArray(data, length);
   m_queue->insert(buf);
   return src;
}

uint32
SerialProviderPublic::sendData(Buffer *b, uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer *buf=new MsgBuffer(b, dst, src, MsgBufferEnums::SERIAL_SEND_DATA);

   m_queue->insert(buf);
   return src;
}

uint32
SerialProviderPublic::connect(SerialConsumerPublic *consumer)
{
   //cout << "SerialProviderPublic::connect" << endl;
   return lowlevelConnectTo(consumer);
}



class MsgBuffer *
SerialConsumerDecoder::dispatch(class MsgBuffer *buf,
      class SerialConsumerInterface *m )
{
   int length;

   //cout << "SerialConsumerDecoder::dispatch : " << buf->getMsgType() << endl;
   length=buf->getLength();
   switch (buf->getMsgType()) {
      case MsgBufferEnums::SERIAL_RECEIVE_DATA:
         m->decodedReceiveData(length, buf->accessRawData(0), buf->getSource());
         delete buf;
         return NULL;
      default:
         return buf;
   }

}

class MsgBuffer *
SerialProviderDecoder::dispatch(class MsgBuffer *buf,
      class SerialProviderInterface *m )
{
   int length;

   //cout << "SerialProviderDecoder::dispatch : " << buf->getMsgType() << endl;
   length=buf->getLength();
   switch (buf->getMsgType()) {
      case MsgBufferEnums::SERIAL_SEND_DATA:
         m->decodedSendData(length, buf->accessRawData(0), buf->getSource());
         delete buf ;
         return NULL;
      default:
         return buf;
   }

}


} /* namespace isab */
