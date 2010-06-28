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


#ifndef MODULE_CtrlHubInternal_H
#define MODULE_CtrlHubInternal_H

#include "Module.h"
#include "Buffer.h"
#include "CtrlHub.h"

namespace isab {

   class CtrlHub : public Module, public CtrlHubInterface {
   public:
      CtrlHub();
      ~CtrlHub();
      int addModule(CtrlHubAttachedPublic *p, int topId = -1 );
      void startupComplete();
      void initiateShutdown();
      void debugProdMe(uint32 dst, char *msg);
      virtual void decodedStartShutdownProcedure();

   protected:
      virtual MsgBuffer * dispatch(MsgBuffer *buf);

      /** Decoder for GpsConsumer-messages */
      CtrlHubDecoder m_ctrlHubDecoder;

      /** Used to send shutdown messages to self */
      CtrlHubPublic * m_ctrlHubSelfPublic;

      //      virtual void decodedManageMulticast(enum MulticastCmd cmd, int16 group);

      virtual void relayMulticastUp(MsgBuffer * buf);


   };

} /* namespace isab */

#endif /* MODULE_CtrlHubInternal_H */
