/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef HTTPCLIENTCONNECTIONLISTENER_H
#define HTTPCLIENTCONNECTIONLISTENER_H

#include "config.h"

class HttpClientConnectionListener {
public:
   /**
    *   Tells the listener that there are bytes of the
    *   body
    *   @param requestNbr     The number of the request.
    *   @param httpStatusCode The http status code.
    *   @param bytes          The bytes if any.
    *   @param startOffset    The start offset in the body, not in bytes.
    *   @param nbrBytes       The number of bytes in bytes.
    *   @param contentLength  The content length if known or 0 if unknown.
    *                         -1 if error. Can be used the first time to
    *                         initialize the buffer to store stuff in.
    */
   virtual void bytesReceived(int requestNbr,
                              int httpStatusCode,
                              const byte* bytes,
                              int startOffset,
                              int nbrBytes,
                              int contentLength) = 0;
   
};

#endif
