/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef MSG_BUFFER_H
#define MSG_BUFFER_H

#include "Buffer.h"

namespace isab{
   class MsgBuffer : public Buffer {
   public:

   protected:
      /** The destination module of this message */
      uint32 m_destination;

      /** The source of this message */
      uint32 m_source;

      /** Command type See the enum MsgTypes. */
      uint16 m_cmd;

      /** Extra data. Free for use */
      uint16 m_extraData;

   public:
      MsgBuffer(uint32 dest, uint32 src, uint16 msgType, size_t length = 4)
        : Buffer(length), m_destination(dest), m_source(src), m_cmd(msgType),
          m_extraData(0)
      {
      }

      MsgBuffer(const MsgBuffer &m)
        : Buffer(m), m_destination(m.m_destination), m_source(m.m_source),
          m_cmd(m.m_cmd), m_extraData(m.m_extraData)
      {
      }

   private:
      const MsgBuffer& operator=(const MsgBuffer& mbuf);
   public:

      MsgBuffer(Buffer *buf, uint32 dest, uint32 src, uint16 msgType)
        : Buffer(0), m_destination(dest), m_source(src), m_cmd(msgType),
          m_extraData(0)
      {
         //         takeDataAndClear(*buf);
         takeDataAndDelete(buf);
      }

      void setExtraData(uint16 data) {
         m_extraData = data;
      }

      uint16 getExtraData() const {
         return m_extraData;
      }

      /** @return the destination address for this message */
      uint32 getDestination() const{
         return m_destination;
      }

      /** Change the destination for this message.
       * @return the old destination address for this message */
      uint32 setDestination(uint32 newDst){
         uint32 oldDest = m_destination;
         m_destination = newDst;
         return oldDest;
      }

      /** @return the source address for this message */
      uint32 getSource() const 
      {
         return m_source;
      }

      /** @return the message type (see the enum MessageTypes */
      uint16 getMsgType() const
      {
         return m_cmd;
      }

   };

}

#endif
