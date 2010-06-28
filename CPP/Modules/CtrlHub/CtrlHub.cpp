/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* The CtrlHub is the central switchboard for the Nav2 system.
 */



#include "arch.h"
#include <math.h>
#include "Buffer.h"
#include "MsgBufferEnums.h"
#include "MsgBuffer.h"
#include "CtrlHub.h"
#include "CtrlHubInternal.h"

#include "LogMacros.h"

namespace isab {


   CtrlHub::CtrlHub()
      : Module("CtrlHub")
   {
      DBG("CtrlHub::CtrlHub\n");
      m_address = 0x00000000;
      /* The netmask is implied /24 in CtrlHub::addModule */
      m_netmask = 0xff000000;
      m_treeNetmask = 0x00000000;
      m_ctrlHubSelfPublic = new CtrlHubPublic(this);
   }

   CtrlHub::~CtrlHub()
   {
      delete m_ctrlHubSelfPublic;
   }

   int CtrlHub::addModule(CtrlHubAttachedPublic *p, int topId)
   {
      uint32 newAddr;

      p->setOwnerModule(this);
      p->connect(new CtrlHubPublic(m_queue));
      if (topId < 0) {
         newAddr = ( uint32(m_numLowerModules+1) | 0x80 ) << 24;
      } else {
         newAddr = uint32(topId) << 24;
      }
      newLowerModule(p, newAddr);
      p->negotiateAddressSpace(m_address, newAddr, m_netmask);
      return 0;
   }

   void CtrlHub::startupComplete()
   {
      int i;
      for (i=0; i < m_numLowerModules; i++) {
         m_lowerModules[i].theInterface->startupComplete();
      }
   }

   void CtrlHub::initiateShutdown()
   {
      m_ctrlHubSelfPublic->startShutdownProcedure();
   }

   void CtrlHub::debugProdMe(uint32 dst, char *msg)
   {
      DBG("CtrlHub::debugProdMe()\n");
      MsgBuffer *buf = new MsgBuffer(dst, 0, MsgBufferEnums::SERIAL_SEND_DATA, 8);
      buf->writeNextCharString(msg);
      m_queue->insert(buf);
   }
   
   MsgBuffer * CtrlHub::dispatch(MsgBuffer *buf)
   {
      //DBG("CtrlHub::dispatch : %i\n", buf->getMsgType());
      buf=m_ctrlHubDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=Module::dispatch(buf);
      return buf;
   }

   void CtrlHub::relayMulticastUp(MsgBuffer *buf)
   {  
      buf->setDestination(buf->getDestination() & ~0x00010000L);
      relayMulticastDown(buf);
      buf = dispatch(buf);
      if (buf) {
         /* Unhandled message. Do not know what to do with this buffer, delete it */
         delete(buf);
      }
   }

   void CtrlHub::decodedStartShutdownProcedure()
   {
      DBG("Got StartShutdownProcedure !!!!");
      decodedShutdownPrepare( MaxShutdownWait );
   }




} /* namespace isab */

