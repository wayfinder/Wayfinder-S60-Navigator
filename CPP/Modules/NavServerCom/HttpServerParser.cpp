/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavPacket.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "MsgBufferEnums.h"
#include "Module.h"
#include "ErrorModule.h"
#include "GuiProt/ServerEnums.h"
#include "NavServerCom.h"

#include "Serial.h"
#include "CtrlHub.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "RouteEnums.h"
#include "NavTask.h"
#include "RequestList.h"
#include "NavServerComInternal.h"

#include "NavHandler.h"
#include "ServerParser.h"

#include "HttpServerParser.h"

#include "NParamBlock.h"
#include "GlobalData.h"
#include <ctype.h>

#include "LogMacros.h"
#include "nav2util.h"
#include "TraceMacros.h"

using namespace std;

namespace isab{
   using namespace NavServerComEnums;

   HttpServerParser::HttpServerParser( class SerialProviderPublic* spp,
                                       class NavServerCom* nsc,
                                       const char* httpUserAgent,
                                       const char* httpRequest,
                                       const char* httpFixedHost,
                                       unsigned freq)
         : ServerParser( spp, nsc, freq ), 
           m_httpState( readingResponseLine ), m_contentLength( 0 ),
           m_contentRead( 0 ), m_realServerAndPort( NULL ), m_realUrl(NULL),
           m_host( NULL), m_fixedHost( httpFixedHost != NULL),
           m_httpUserAgent( strdup_new( httpUserAgent ) ),
           m_closeIsOK( false ), m_lastReplyHadClose( false ), m_readingErrorReply( false ),
           m_request( NULL )
   {
      if(!httpRequest){
         m_request = strdup_new("/nav");
      } else {
         m_request = strdup_new(httpRequest);
      } 
      if(httpFixedHost){
         m_host = strdup_new( httpFixedHost );
      } else {
         m_host = strdup_new( "a" );
      }
   }


   HttpServerParser::~HttpServerParser() {
      delete [] m_realUrl;
      delete [] m_realServerAndPort;
      delete [] m_httpUserAgent;
      delete [] m_request;
   }


   void
   HttpServerParser::flush() {
      m_httpState = readingResponseLine;
      m_currLine.clear();
      m_contentLength = 0;
      m_contentRead = 0;
      m_lastReplyHadClose = false;
      m_closeIsOK = false;
      m_readingErrorReply = false;

      // Flush ServerParser too
      ServerParser::flush();
   }


   void
   HttpServerParser::decodeServerPacket( const uint8* data, size_t len ) {
      DBG( "Http::decodeServerPacket %u", len );
      //TRACE_DBG( "Got %d", len );
      size_t inLen = 0;
      while ( len > inLen ) { // May change state
         switch( m_httpState ) {
            case readingResponseLine : 
//               DBG( "Http::decode readingResponseLine %u (%c)", inLen,
//                    data[ inLen ] );
               if ( addToCurrLine( data[ inLen++ ] ) ) {
                  DBG( "Http::decode EOL %s", (const char*)m_currLine.accessRawData( 0) );
                  //TRACE_DBG( "Http::decode EOL %s", (const char*)m_currLine.accessRawData( 0) );
                  m_lastReplyHadClose = false;
                  m_closeIsOK = false;
                  // If HTTP 200 else not ok
                  // End with "TTP/1.1 200 OK"
                  
                  if ( (m_currLine.getLength() >= 12 && 
                        strncmp( (const char*)m_currLine.accessRawData( 0),
                                 "HTTP/1.0 200", 12 ) == 0 ) ||
                       (m_currLine.getLength() >= 15 && 
                        strncmp( (const char*)m_currLine.accessRawData( 
                                    m_currLine.getLength() - 15 ),
                                 "TTP/1.0 200 OK", 14 ) == 0) ||
                       (m_currLine.getLength() >= 12  &&
                        strncmp( (const char*)m_currLine.accessRawData( 0),
                                 "HTTP/1.1 200", 12 ) == 0 ) ||
                       (m_currLine.getLength() >= 15 && 
                        strncmp( (const char*)m_currLine.accessRawData( 
                                    m_currLine.getLength() - 15 ),
                                 "TTP/1.1 200 OK", 14 ) == 0)
                       )
                  {
                     DBG( "Http::decode 200 OK!" );
                     m_httpState = readingHeader;
                     m_currLine.clear();
                  } else if ( (m_currLine.getLength() >= 9 && 
                               strncmp( (const char*)m_currLine.accessRawData( 0 ),
                                        "HTTP/1.0 ", 9 ) == 0 ) ||
                              (m_currLine.getLength() >= 9  &&
                               strncmp( (const char*)m_currLine.accessRawData( 0 ),
                                        "HTTP/1.1 ", 9 ) == 0 ) )
                  {
                     // A reply but not 200
                     m_readingErrorReply = true;
                     m_httpState = readingHeader;
                     m_currLine.clear();
                  } else {
                     ERR( "Http::decode NOT 200!! len %"PRIu32" cmp %d", 
                          m_currLine.getLength(), 
                          strncmp( (char*)m_currLine.accessRawData( 0 ),
                                   "HTTP/1.1 200", 12 ) );
                     // Error
                     flush();
                  }
               } // Else next char
               break;
            case readingHeader : 
//               DBG( "Http::decode readingHeader %u", inLen );
               if ( addToCurrLine( data[ inLen++ ] ) ) {
                  DBG( "Http::decode Header field %s", (const char*)m_currLine.accessRawData( 0) );
                  //TRACE_DBG( "%s", (const char*)m_currLine.accessRawData( 0) );
                  if ( m_currLine.getLength() == 1 ) { // Terminating byte
                     // EOH
                     m_httpState = readingBody;
                     // XXX: Not true but if we get 200 OK we can retry
                     //      with this server if it closes socket after 
                     //      this reply
                     m_closeIsOK = true;
                  } else if ( m_currLine.getLength() >= 16 && 
                       strncasecmp( (const char*)m_currLine.accessRawData( 0 ),
                                "Content-Length: ", 16 ) == 0 )
                  {
                     m_currLine.writeNext8bit( 0 );
                     m_contentLength = strtoul( 
                        (const char*)m_currLine.accessRawData( 16 ), 
                        NULL, 10 );
                  } else if ( m_currLine.getLength() >= 17 && 
                              strncasecmp((const char*)
                                          m_currLine.accessRawData( 0 ),
                                          "Connection: Close", 17 ) == 0 )
                  {
                     m_lastReplyHadClose = true;
                  }
                  m_currLine.clear();
               }
               break;
            case readingBody : 
//               DBG( "Http::decode readingBody %u", inLen );
               uint32 nbrBytes = len - inLen;
               if ( nbrBytes > (m_contentLength-m_contentRead) ) {
                  nbrBytes = (m_contentLength-m_contentRead);
               }
               m_contentRead += nbrBytes;
               if ( !m_readingErrorReply ) {
                  ServerParser::decodeServerPacket( data + inLen, nbrBytes );
               }
               inLen += nbrBytes;
               bool closeIsOK = m_closeIsOK;
               if ( m_parseState == find_stx ||
                    m_contentRead >= m_contentLength ) 
               {
                  if ( m_lastReplyHadClose ) {
                     // Disconnect connection since we got Connection: Close.
                     //TRACE_DBG("Disconnecting since got Connection: Close");
                     m_navServerCom->forceDisconnect(); 
                  }
                  
                  flush(); // Sets state and clears data
               }
               m_closeIsOK = closeIsOK; // Keep after flush()
               // flush resets m_readingErrorReply too
               break;
         }
      }
      
   }

   namespace {
      void RemovePort80( char* hostAndPort )
      {
         uint32 hostLen = strlen( hostAndPort );
         if ( hostLen > 3 ){
            //starts at the ':' if it is there.
            char* portpart = hostAndPort + (hostLen - 3);
            if(strequ(portpart, ":80")){
               *portpart = '\0';
            }
         }
      }

      void ReplaceAndRemovePort80(char** dstVar, const char* newHost)
      {
         delete[] *dstVar;
         *dstVar = NULL;
         *dstVar = strdup_new(newHost);
         RemovePort80(*dstVar);
      }
   }

   void
   HttpServerParser::setUrl( const char* realUrl ) {
      DBG( "HttpServerParser::setUrl %s", realUrl );
      if ( realUrl != NULL ) {
         // Set m_realUrl
         ReplaceAndRemovePort80(&m_realUrl, realUrl);
      }
   }

   void
   HttpServerParser::setProxy( const char* realServererAndPort ) {
      DBG( "HttpServerParser::setProxy %s", realServererAndPort );
      // Set m_realServerAndPort
      ReplaceAndRemovePort80(&m_realServerAndPort, realServererAndPort);
   }

   void
   HttpServerParser::setHost( const char* host )
   {
      DBG("HttpServerParser::setHost %s", host);
      if(!m_fixedHost){
         ReplaceAndRemovePort80(&m_host, host);
      }
   }

   bool
   HttpServerParser::getCloseExpected() const {
      return m_closeIsOK;
   }

   namespace {
      void AppendText(vector<uint8>& dst, const char* first, 
                      const char* second = NULL, const char* third = NULL)
      {
         const char* texts[] = {first, second, third};
         const size_t num = sizeof(texts)/sizeof(*texts);
         for(size_t i = 0; i < num && texts[i] != NULL; ++i){
            const char* text = texts[i];
            dst.insert( dst.end(), text, text + strlen(text) );
         }
      }
   }


   int
   HttpServerParser::sendPacket( const NavRequestPacket* pack ) {
      vector< uint8 > buff;
      vector< uint8 > body;
      addPacketToBuff( body, pack );
      const char* eol = "\r\n";
      // Start Http header 
      const char* startHttpHeader = "POST ";
      AppendText(buff, startHttpHeader);
      if ( m_realServerAndPort != NULL ) {
         // Add stuff for proxy
         const char* service = "http://";
         AppendText( buff, service, m_realServerAndPort );

      }
      if (m_realUrl != NULL) {
         AppendText( buff, m_realUrl );
      }
      AppendText( buff, m_request );

      const char* httpAndHost = " HTTP/1.1\r\nHost: ";
      AppendText( buff, httpAndHost );
      if( m_realServerAndPort != NULL ){
         AppendText( buff, m_realServerAndPort );
      } else {
         AppendText( buff, m_host );
      }
      AppendText( buff, eol );

      if ( m_httpUserAgent != NULL ) {
         const char* uaf = "User-Agent: ";
         AppendText( buff, uaf, m_httpUserAgent, eol );
      }

      const char* contentlf = "Content-Length: ";
      //print numbytes in tmp
      char tmp[22];
      unsigned int s = body.size();
      sprintf( tmp, "%u\r\n\r\n", s );
      //append content length, with number
      AppendText( buff, contentlf, tmp);
      // and the body.
      buff.insert( buff.end(), body.begin(), body.end() );

      m_serialProvider->sendData( buff.size(), &buff.front() );
      return buff.size();
   }


   bool
   HttpServerParser::addToCurrLine( uint8 ch ) {
      if ( ch == '\r' ) {
         // Don't add
         return false;
      } else if ( ch == '\n' ) {
         m_currLine.writeNext8bit( '\0' );
         return true;
      } else {
         m_currLine.writeNext8bit( ch );
         return false;
      }

//      return false; // We never reach this
   }


} // End namespace isab



