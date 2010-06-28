/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTP_SERVER_PARSER_H
#define HTTP_SERVER_PARSER_H


namespace isab{

   /**
    * Subclass to ServerParser that sends/receives Http-encapsulated 
    * requests/replies.
    */
   class HttpServerParser : public ServerParser {
      public:
         /**
          * Constructor.
          *
          * @param spp  The SerialProviderPublic used to send 
          *             Messages to the server.
          * @param nsc  The NavServerCom module.
          * @param httpUserAgent The User-Agent string to use.
          * @param freq Frequency of progressMessages. For every 
          *             <b>freq</b> byte uploaded or downloaded a
          *             progressMessage is sent. Default value 500.
          * @param httpRequest The requeststring used in http requests.
          *                    If NULL the requeststring will be "/nav".
          *                    Default is NULL.
          */
         HttpServerParser( SerialProviderPublic* spp, NavServerCom* nsc, 
                           const char* httpUserAgent = NULL,
                           const char* httpRequest = NULL,
                           const char* httpFixedHost = NULL,
                           unsigned freq = 500);

         /**
          * Destructor. 
          */
         virtual ~HttpServerParser();


         /**
          * Deletes the input buffer and resets the state too. 
          */
         virtual void flush();


         /**
          * Handle some data from network.
          *
          * @param data The data to handle.
          * @param len The length of data.
          */
         virtual void decodeServerPacket( const uint8* data, size_t len );


         /**
          * Set proxy mode and the real server for the proxty to connect 
          * to.
          *
          * @param realServererAndPort The server to ask the proxy to use.
          */
         virtual void setProxy( const char* realServererAndPort );

         /**
          * Set URL part for proxty to connect to.
          *
          * @param realUrl The server to ask the proxy to use.
          */
         virtual void setUrl( const char* realUrl );

      /**
       * Set the content of the Host header.
       */
      virtual void setHost( const char* host );

         /**
          * If last reply indicated that close of socket was due to happen.
          */
         virtual bool getCloseExpected() const;


      protected:
         /**
          * Send a Nav packet to SerialProviderPublic.
          *
          * @param pack The packet to send.
          * @return The number of bytes the packet is when sent.
          */
         virtual int sendPacket( const NavRequestPacket* pack );


      private:
         /**
          * The states.
          */
         enum HttpState {
            readingResponseLine = 0,
            readingHeader = 1,
            readingBody = 2,
         } m_httpState;
         

         /**
          * The current line from server being assembled.
          */
         Buffer m_currLine;


         /**
          * The Content-Length from server.
          */
         uint32 m_contentLength;


         /**
          * The number of content bytes read.
          */
         uint32 m_contentRead;


         /**
          * Add a char to m_currLine and return true if eol.
          */
         bool addToCurrLine( uint8 ch );


         /**
          * The server to tell proxy to connect to.
          */
         char* m_realServerAndPort;

         /**
          * URL to add to server string.
          */
         char* m_realUrl;

      /**
       * The content of the Host: header.
       */
      char* m_host;

      /**
       * If this variable is set, the m_host variable may not be
       * changed after construction.
       */
      const bool m_fixedHost;
         
         /**
          * The Http User-Agent string to use.
          */
         char* m_httpUserAgent;

         /**
          * If last reply went ok.
          */
         bool m_closeIsOK;


         /**
          * If the last reply had a connection close.
          */
         bool m_lastReplyHadClose;

         /**
          *
          */
         bool m_readingErrorReply;

      /**
       * Request string to add to the http request. i.e. "/nav".
       */
      char* m_request;
   };


} // End namespace isab


#endif // HTTP_SERVER_PARSER_H

