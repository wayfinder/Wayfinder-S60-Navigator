/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PROT_MESSAGE_SENDER_H
#define GUI_PROT_MESSAGE_SENDER_H

#include <arch.h>

namespace isab {
   class GuiProtMess;
}

class MMessageSender {
public:
   /**
    * Legacy interface.
    */
   virtual int32 SendMessageL(isab::GuiProtMess &mess) = 0;
};

class GuiProtMessageSender {
public:
   /**
    * Send message and perform a callback when the matching
    * reply arrives back.
    */
   virtual int32 SendMessage(isab::GuiProtMess &mess,
         class GuiProtMessageReceiver* callback) = 0;
   /**
    * Send message but don't remember the request id.
    */
   virtual int32 SendMessage(isab::GuiProtMess &mess) = 0;
   /**
    * Disown the currently outstanding messages.
    * Called by objects that know they are going to be
    * destructed.
    * May be called even if the object does not have any
    * outstanding messages at the moment.
    * 
    * Removes any message with the specified callback.
    * These messages will not be handled by the GuiProtMessageHandler.
    */
   virtual void DisownMessage(GuiProtMessageReceiver* callback) = 0;
};

class GuiProtMessageReceiver {
public:
   /**
    * Callback interface, this method is called when a
    * message with matching request id has been received.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess) = 0;
};


#endif
