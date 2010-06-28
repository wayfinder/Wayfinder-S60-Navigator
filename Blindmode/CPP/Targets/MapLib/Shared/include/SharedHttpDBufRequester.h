/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SHARED_HTTP_BDUBF_REQUYESTE_ER_H
#define SHARED_HTTP_BDUBF_REQUYESTE_ER_H

#include "config.h"
#include "SharedDBufRequester.h"
#include "DBufConnection.h"

class HttpClientConnection;
class MC2SimpleString;

/**
 *   Class requesting buffers using http. Connects to
 *   ISAB XMLServer.
 */
class SharedHttpDBufRequester : public SharedDBufRequester {
public:
   /**
    *   Creates a new SharedHttpDBufRequest.
    *   @param clientConnetion The connection handler
    *   @param host            Hostname.
    *   @param port            The port.
    *   @param prefix          What to prefix all uri:s with.
    *   @param urlParams       [Optional] The url parameters 
    *                          (for instance containing the session).
    *                          Should begin with a "?".
    */
   SharedHttpDBufRequester(HttpClientConnection* clientConnetion,
                           const char* prefix,
                           const char* urlParams = "");

   /**
    *   Destructor.
    */
   virtual ~SharedHttpDBufRequester();   
};

class SharedHttpDBufRequesterDBufConnection : public DBufConnection {
public:
   /**
    *    Creates a new SharedHttpDBufRequesterDBufConnection
    *    which will add the prefix to the get and posts.
    *    @param urlParams       [Optional] The url parameters 
    *                           (for instance containing the session).
    *                           Should begin with a "?".
    */
   SharedHttpDBufRequesterDBufConnection(HttpClientConnection* clientConnetion,
                                         const MC2SimpleString& prefix,
                                         const MC2SimpleString& urlParams);

   /**
    *    Destructor.
    */
   virtual ~SharedHttpDBufRequesterDBufConnection() {}
   
   /**
    *    Requests one databuffer. The value returned here should
    *    also be used when sending replies to the listeners.
    *    @param descr Buffer to request.
    *    @return -1 if too many requests have been sent already.
    */
   int requestOne(const char* descr);

   /**
    *    Requests many databuffers. (e.g. using post in http).
    *    The value returned here should
    *    also be used when sending replies to the listeners.
    *    @param body Body of data as described in SharedDBufRequester
    *                and ParserThread. (For multiple databufs).
    *    @return -1 if too many requests have been sent already.
    */
   int requestMany(const uint8* buf, int nbrBytes);

   /**
    *    Sets listener.
    */
   void setListener(HttpClientConnectionListener* listener);

private:
   /// Client connection to use.
   HttpClientConnection* m_clientConnection;
   /// prefix to use.
   MC2SimpleString m_prefix;
   /// The urlParams.
   MC2SimpleString m_urlParams;
};

#endif
