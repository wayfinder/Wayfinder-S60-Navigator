/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef HTTP_TUNNEL_GUI_MESS_H
#define HTTP_TUNNEL_GUI_MESS_H

#include <arch.h>

namespace isab
{
   enum http_tunnel_gui_mess_type {
      service_window_legacy      = 0x00,
      service_window             = 0x01,
   };

   /**
    * HttpTunnelGuiMessObserver
    * Interface for objects that wish to use the HttpTunnelGuiMess.
    */
   class HttpTunnelGuiMessObserver
   {
   public:
      /**
       * DataReceived
       * Called when the data in a HttpTunnelGuiMess has been parsed.
       * @param seqId      Sequence id from message
       * @param startByte  Index of first byte in data
       * @param endByte    Index of last byte in data
       * @param total      Total number of bytes in the file
       * @param url        URL
       * @param data       Data pointer
       * @param dataLength Length of the data
       */
      virtual void DataReceived(uint32 seqId,
            uint32 startByte,
            uint32 endByte,
            uint32 total,
            const char* url,
            const char* data,
            uint32 dataLength) = 0;
   };

   /**
    * HttpTunnelGuiMess
    * Convenience routines for encoding and decoding tunnel messages
    * to and from Nav2.
    */
   class HttpTunnelGuiMess
   {
   public:
      /**
       * CreateTunnelMess
       * Creates a tunnel message with the supplied parameters.
       * @param messageType   Always 1
       * @param seqNo         Sequence number of the request
       * @param url           URL to get
       * @param startByte     Index of first byte to get
       * @return a new DataGuiMess
       */
      static class DataGuiMess* CreateTunnelMess(
            int32 messageType,
            uint32 seqNo,
            const char* url,
            uint32 startByte);

      /**
       * ParseTunnelMess
       * Parse a message and do a callback with the appropriate parameters.
       * @param message       DataGuiMess from Nav2
       * @param observer      Object to call when parsing is complete
       * @return type of request (1) or -1 for failure
       */
      static int32 ParseTunnelMess(const DataGuiMess* message,
            HttpTunnelGuiMessObserver* observer);
   };
}

#endif
