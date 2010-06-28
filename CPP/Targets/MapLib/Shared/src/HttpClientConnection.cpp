/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include<string.h>
#include <ctype.h>
// For sprintf
#include <stdio.h>

#include <algorithm>

#include "HttpClientConnection.h"
#include "HttpClientConnectionListener.h"
#include "SharedBuffer.h"
#include "SharedHttpHeader.h"

class HttpClientConnectionReader {
public:
   /**
    *   Creates a new HttpClientConnectionReader
    */
   HttpClientConnectionReader(HttpClientConnection* connection,
                              HttpClientConnectionListener* listener);

   /// Destroys the internal stuff.
   ~HttpClientConnectionReader();

   /**
    *   Handles that a read is done.
    */
   TCPClientConnection::status_t
           readDone(TCPClientConnection::status_t status,
                    const byte* bytes, int nbrBytes);

   /**
    *
    */
   void setListener(HttpClientConnectionListener* listener ) {
      m_listener = listener;
   }
   
public:

   /// The connection
   HttpClientConnection* m_connection;

   /// The connection listener
   HttpClientConnectionListener* m_listener;
   
   /// The header
   SharedHttpHeader m_header;
   
   enum state_t {
      IDLE           = 0,
      READING_HEADER = 1,
      READING_BODY   = 2,
   } m_state;

   /// The content length.
   int m_contentLength;
   /// The content length left.
   int m_contentLengthLeft;
   /// The current position in the body
   int m_curBodyPos;
   /// Max length to read
   int m_readLength;
   /// Read buffer.
   byte* m_readBuffer;
};

HttpClientConnectionReader::
HttpClientConnectionReader(HttpClientConnection* connection,
                           HttpClientConnectionListener* listener)
{
   m_connection = connection;
   m_listener   = listener;
   m_readLength = 1024;
   m_state      = IDLE;
}

HttpClientConnectionReader::~HttpClientConnectionReader()
{
   setListener(NULL);
   if ( m_connection ) {
      m_connection->setListener(NULL);
   }
}

TCPClientConnection::status_t
HttpClientConnectionReader::readDone(TCPClientConnection::status_t status,
                                     const byte* bytes,
                                     int nbrBytes)
{
   mc2dbg << "[HttpClientConnectionReader::readDone called with status = "
          << int(status) << " and nbr bytes = " << nbrBytes
          << endl;
   
   if ( m_state == IDLE ) {
      if ( m_connection->getNbrSent() ) {
         m_state = READING_HEADER;
         m_header.reset();
         m_curBodyPos = 0;
      } else {
         return TCPClientConnection::ERROR;
      }      
   }

   // FIXME: Divide this into more functions.
   if ( status == TCPClientConnection::OK ) {
      int headerBytes = 0;
      while ( nbrBytes > 0 ) {
         int currentRequest = m_connection->getCurrentRequestNbr();
         if ( m_state == READING_HEADER ) {
            mc2dbg << "[HttpClientConnectionReader]: Adding max "
                   << nbrBytes
                   << "bytes to header"
                   << endl;
            headerBytes = m_header.addBytes(bytes, nbrBytes);
            if ( headerBytes < 0 ) {
               m_state = IDLE;
               m_connection->connectionClosed(TCPClientConnection::ERROR);
               return TCPClientConnection::ERROR;
            }
            bytes +=    headerBytes;
            nbrBytes -= headerBytes;
            if ( headerBytes < 0 ) {
               // ERROR
               m_state = IDLE;
               return TCPClientConnection::ERROR;
            }
            if ( m_header.complete() ) {
               mc2dbg << "[HttpClientConnectionReader]: HTTP-status is "
                      << m_header.getStatusCode() << endl;
               // Status is ok.
               m_contentLength = 0;
               const char* contentLengthStr =
                  m_header.getHeaderValue("Content-Length");
               if ( contentLengthStr ) {
                  m_contentLength = atoi(contentLengthStr);
               }
               m_contentLengthLeft = m_contentLength;
               mc2dbg << "[Http]: Content-length = "
                      << m_contentLength << endl;
               // Tell the listener that there are bytes.
               if ( m_listener ) {
                  m_listener->bytesReceived(currentRequest,
                                            m_header.getStatusCode(),
                                            NULL, 0, 0,
                                            m_contentLength);
               }
               m_state = READING_BODY;
               m_curBodyPos = 0;
            }
         }
         mc2dbg << "[HttpClientConnectionReader]: headerBytes = "
                << headerBytes << endl;
         
         if ( m_state == READING_BODY ) {
            if ( nbrBytes ) {
               int nbrBytesToListener = MIN(nbrBytes, m_contentLengthLeft);
               if ( m_listener ) {
                  m_listener->bytesReceived(currentRequest,
                                            m_header.getStatusCode(),
                                            bytes,
                                            m_curBodyPos,
                                            nbrBytesToListener,
                                            m_contentLength);
               }
               m_contentLengthLeft -= nbrBytesToListener;
               m_curBodyPos        += nbrBytesToListener;
               nbrBytes            -= nbrBytesToListener;
               bytes               += nbrBytesToListener;
            }
            mc2dbg << "[HttpClientConnectionReader]: m_contentLengthLeft = "
                   << m_contentLengthLeft << endl;
            if ( (m_contentLength != 0) && (m_contentLengthLeft == 0) ) {
               m_connection->oneRequestComplete();
               if ( m_connection->getNbrSent() == 0 ) {
                  mc2dbg << "[HttpClientConnectionReader]: Should be IDLE now"
                         << endl;
                  m_state = IDLE;
               } else {
                  m_state = READING_HEADER;
                  m_curBodyPos = 0;
                  m_contentLengthLeft = 0;
                  m_contentLength = 0;
                  m_header.reset();
               }
            }
         }
      }
   } else if ( ( status == TCPClientConnection::CLOSED ) &&
               ( m_state == READING_BODY ) &&
               ( m_contentLength == 0) ) {
      const int currentRequest = m_connection->getCurrentRequestNbr();         
      // The server didn't send content length.
      // We're done when the connection is closed.
      if ( m_listener ) {
         m_listener->bytesReceived(currentRequest,
                                   m_header.getStatusCode(),
                                   bytes,
                                   m_curBodyPos,
                                   nbrBytes, m_curBodyPos+nbrBytes);
      }
      m_curBodyPos += nbrBytes;
      bytes += nbrBytes;
      m_connection->oneRequestComplete();
      m_state = READING_HEADER;
      m_curBodyPos = 0;
      m_header.reset();
   } else {
      // Error!
      m_state = IDLE;
      return TCPClientConnection::ERROR;
   }

   // Always check for more data.
   m_connection->readNextRequest();

   return status;
}

HttpClientConnection::
HttpClientConnection(const char* host,
                     unsigned int port,
                     HttpClientConnectionListener* listener,
                     TCPConnectionHandler* connHandler)
#ifdef __SYMBIAN32__
      : m_userAgent("SymbClientConn/0.01"),
#elif  WIN32
      : m_userAgent("WinClientConn/0.01"),
#else
      : m_userAgent("MC2ClientConn/0.01"),
#endif
        m_maxNbrRetries(3)
{
   m_maxNbrGet = 3;
   m_proxyHeaderLines = "";
   setHostAndPort(host, port);
   m_connHandler = connHandler;
   m_connecting  = false;
   m_reading     = false;
   m_writing     = false;
   m_state       = NOT_CONNECTED;

   m_nbrWaiting          = 0;

   m_listener = listener;
   
   m_connReader = new HttpClientConnectionReader(this, listener);

   m_nextID           = 0;
   m_nbrRetries       = 0;
}

void
HttpClientConnection::setHostAndPort(const char* host, unsigned int port)
{
   m_host = host;
   m_port = port;
#if defined(__unix__) || defined(__MACH__)
   // Check for proxy environment variable.
   const char* proxy = getenv("http_proxy");
   if ( ( proxy != NULL ) && ( proxy[0] != 0 ) ) {
      char proxyHostName[1024];
      strcpy(proxyHostName, proxy);
      int len = strlen(proxyHostName);
      for ( int i = 0; i < len; ++i ) {
         if( proxyHostName[i] == ':' ) {
            proxyHostName[i] = ' ';
         }
      }
      unsigned int proxyPort = 0;
      if ( sscanf( proxyHostName,
                   "http //%s %u/", proxyHostName, &proxyPort) == 2 ) {
         char urlPrefix[1024];         
         sprintf(urlPrefix, "http://%s:%u", m_host.c_str(), m_port);
         m_urlPrefix = urlPrefix;
         m_host = proxyHostName;
         m_port = proxyPort;
         m_proxyHeaderLines = "Proxy-Connection: Keep-Alive\r\n";
      } else {
         mc2dbg << "proxyHostName = " << proxyHostName << endl;
         mc2dbg << "[HttpCC]: Could not parse proxy "
                << MC2CITE(proxy) << ". Should be of the form "
                << "http://host:port/" << endl;
      }
   }
#endif
}

HttpClientConnection::~HttpClientConnection()
{
   m_connHandler->remove(this);
   // Close connection.
   // Delete buffers.
   delete m_connReader;

   // FIXME: Delete the requests...
}

void
HttpClientConnection::setListener(HttpClientConnectionListener* listener)
{
   m_listener = listener;
   m_connReader->setListener(listener);
   if ( m_listener == NULL ) {
      m_connHandler->remove(this);
   }
}


void
HttpClientConnection::readDone(status_t status,
                               const byte* bytes,
                               int nbrBytes)
{
   m_reading = false;
   m_connReader->readDone(status, bytes, nbrBytes);
}

void
HttpClientConnection::connectionClosed(status_t /*status*/)
{
   m_connecting  = false;
   m_reading     = false;
   m_writing     = false;
   m_connReader->readDone(TCPClientConnection::CLOSED, 0, 0);
   m_state = NOT_CONNECTED;
   // Move back the waiting fors to the requestlist
   std::reverse(m_waitingFor.begin(), m_waitingFor.end());
   for( requestList_t::iterator it = m_waitingFor.begin();
        it != m_waitingFor.end();
        ++it ) {
      mc2dbg << "[HttpClientConnection]: Moving back "
             << (*it).first << endl;
      m_requestsToSend.push_front(*it);
   }
   if ( ! m_currentlyWriting.empty() ) {
      mc2dbg << "[HttpClientConnection]: Moving back "
             << m_currentlyWriting.front().first << " from current" << endl;
      m_requestsToSend.push_front(m_currentlyWriting.front());
   }
   m_currentlyWriting.clear();
   m_waitingFor.clear();
   writeNextRequest();
}

void
HttpClientConnection::connectDone(status_t status)
{
   m_connecting = false;
   
   if ( status == OK ) {
      mc2dbg << "[HttpClientConnection]: Connected! " << endl;
      m_state = CONNECTED;
      // Check if there are requests to be written.
      writeNextRequest();
   } else {
      m_state = NOT_CONNECTED;
      if ( m_nbrRetries >= m_maxNbrRetries ) {
         // Signal the error to the listener
         if ( ! m_waitingFor.empty() ) {
            mc2dbg << "[HttpClientConnection:] Could not connect "
                   << " and m_waitingFor is not empty that is wrong." << endl;
         }
         if ( ! m_currentlyWriting.empty() ) {
            m_waitingFor.push_back(m_currentlyWriting.front());
            m_currentlyWriting.pop_front();         
         } else if ( ! m_requestsToSend.empty() ) {
            m_waitingFor.push_back(m_requestsToSend.front());
            m_requestsToSend.pop_front();
         }
         int reqNbr = getCurrentRequestNbr();
         oneRequestComplete();
         if ( m_listener ) {
            m_listener->bytesReceived(reqNbr, 999,
                                      NULL, -1, -1, -1);
         }
         // Just try once for the rest.
         writeNextRequest();
      } else {
         writeNextRequest();
      }                                             
   }
}

void
HttpClientConnection::readNextRequest()
{
   mc2dbg << "HttpClientConnection::readNextRequest" << endl;
   if ( m_state == CONNECTED ) {
      m_connHandler->read(m_connReader->m_readLength, this);
   }
   
}

int
HttpClientConnection::getCurrentRequestNbr() const
{
   if ( m_waitingFor.empty() ) {
      return -1;
   } else {
      return m_waitingFor.front().first;
   }
}

void
HttpClientConnection::writeDone(status_t status)
{
   mc2dbg << "[HttpClientConnection]: writeDone("
          << (int)status << " for "
           << m_currentlyWriting.front().first << endl;

   m_writing = false;

   if ( status == OK  ) {
      // Ok one down
      // Move the request to the waitingfor-list.
      m_waitingFor.push_back(m_currentlyWriting.front());
      m_currentlyWriting.pop_front();
   } else {
      // Connect and send again.
      m_connHandler->connect(m_host.c_str(), m_port, this);
      m_nbrRetries++;
      return;
   }

   if ( status == OK ) {
      m_nbrWaiting++;
      if ( m_state == CONNECTED ) {
         // Request read too.
         readNextRequest();
      }
      
      // Check if we need to resend data.
      writeNextRequest();
   } else {
      m_state = NOT_CONNECTED;
   }
}

void
HttpClientConnection::oneRequestComplete()
{
   if ( ! m_waitingFor.empty() ) {
      mc2dbg << "[HttpClientConnection]: Receieved all for "
             << m_waitingFor.front().first << endl;
      delete m_waitingFor.front().second;
      m_waitingFor.pop_front();
   }
   writeNextRequest();
}

void
HttpClientConnection::writeNextRequest()
{
   mc2dbg << "[HttpClientConnection]: writeNextRequest" << endl;

   // Check if we are connected.
   if ( m_state == NOT_CONNECTED &&
        ( !m_requestsToSend.empty() || ! m_currentlyWriting.empty() ) ) {
      if ( ! m_connecting ) {
         mc2dbg << "[HttpClientConnection]: connecting!" << endl;
         m_nbrRetries++; // VARUN
         m_connHandler->connect(m_host.c_str(), m_port, this);
         //m_nbrRetries++; VARUN : SHIFTING ABOVE RETRY
         m_connecting = true;
      }
      return;
   }
   
   if ( m_writing ) {
      mc2dbg << " [HttpClientConnection]: Writing already - strange"
             << endl;
   }
   
   bool isRetry = true;
   // Check if we are writing and if there are requests to send.
   // Also check if we have to many outstanding requests or not.
   if ( ( m_currentlyWriting.empty() ) &&
        ( ! m_requestsToSend.empty() ) &&
        ( int(m_waitingFor.size()) < m_maxNbrGet ) ) {
      isRetry = false;
      // Remove first element in requests to send.
      m_currentlyWriting.push_back(m_requestsToSend.front());
      m_requestsToSend.pop_front();
   } else {
      mc2dbg << "[HttpClientConnection]: m_currentlyWriting.empty() = "
             << int(m_currentlyWriting.empty()) << endl
             << "m_requestsToSend.empty() = "
             << int(m_requestsToSend.empty()) << endl
             << "m_waitingFor.size() = " << m_waitingFor.size() << endl;
   }
   
   if ( ! m_currentlyWriting.empty() ) {
      mc2dbg << "[HttpClientConnection]: Will write "
             << m_currentlyWriting.front().first << endl;
      m_writing = true;
      SharedBuffer* curBuf = m_currentlyWriting.front().second;
      m_connHandler->write(
                           curBuf->getBufferAddress(),
                           curBuf->getBufferSize(),
                           this);
      if ( isRetry ) {
         m_nbrRetries++;
      }
   }
   
}

void
HttpClientConnection::timerExpired(int timerID)
{
   mc2dbg << "[HttpClientConnection]: timerExpired = " << timerID
          << endl;
}

int
HttpClientConnection::get(const char* filespec1,
                          const char* filespec2,
                          const char* filespec3,
                          const char* urlParams)
{
   if ( filespec2 == NULL ) {
      filespec2 = "";
   }
   if ( filespec3 == NULL ) {
      filespec3 = "";
   }
                              
   
   mc2dbg << "[HttpClientConnection]: get " << filespec1 << filespec2
          << endl;
   // Check number of 
   if ( m_requestsToSend.size() >= (uint32)m_maxNbrGet ) {
      mc2dbg << "[HttpClientConnection]: To many pending requests" << endl;
      return -1;
   }
   
   
   static const int maxStr = 4024;
   char* tmpRequest = new char[maxStr];

   memset( tmpRequest, 0, maxStr );

   snprintf(tmpRequest, maxStr, 
           "GET %s%s%s%s%s HTTP/1.0\r\n"
           "User-Agent: %s\r\n"
           "Host: %s:%u\r\n"
           "%s" // ProxyHeaderLines
           "Connection: Keep-Alive""\r\n"
           "\r\n",
           m_urlPrefix.c_str(),
           filespec1, filespec2, filespec3, urlParams,
           m_userAgent.c_str(), m_host.c_str(), m_port, m_proxyHeaderLines);
 
   // Should never happen, but ... zero terminate
   tmpRequest[maxStr-1] = '\0';

   m_nextID++;
   if ( m_nextID < 0 ) {
      // If we have sent _very_ many requests, this can wrap.
      m_nextID = 0;
   }

   int bufLength = strlen(tmpRequest);
   m_requestsToSend.push_back(
      std::make_pair(m_nextID,
                new SharedBuffer((uint8*)tmpRequest,
                                 bufLength)));
   // tmpRequest is now handled by the bitbuffer.
      
   // Writes the next request if possible.
   writeNextRequest();
   return m_nextID;
}

int
HttpClientConnection::post(const char* filespecPart1,
                           const char* filespecPart2,
                           const char* /*mimeType*/,
                           const SharedBuffer& body,
                           const char* urlParams)
{
   static const int maxStr = 4024;
   char* tmpRequest = new char[maxStr];
   int contentLength = body.getBufferSize();

   memset( tmpRequest, 0, maxStr );

   snprintf(tmpRequest, maxStr, 
           "POST %s%s%s%s HTTP/1.0\r\n"
           "User-Agent: %s\r\n"
           "Host: %s:%u\r\n"
           "%s" // ProxyHeaderLines
           "Connection: Keep-Alive\r\n"
           "Content-Length: %d\r\n"
           "\r\n",
           m_urlPrefix.c_str(),
           filespecPart1, filespecPart2, urlParams,
           m_userAgent.c_str(),
           m_host.c_str(), m_port,
           m_proxyHeaderLines,
           contentLength);

   int tmpReqLen = strlen(tmpRequest);
   
   m_nextID++;
   if ( m_nextID < 0 ) {
      // If we have sent _very_ many requests, this can wrap.
      m_nextID = 0;
   }

   // Copy the stuff into a new buffer.
   SharedBuffer* tmpBuf =
      new SharedBuffer(tmpReqLen + body.getBufferSize());   
   tmpBuf->writeNextByteArray((const uint8*)tmpRequest, tmpReqLen);
   tmpBuf->writeNextByteArray( body.getBufferAddress(), body.getBufferSize());
   m_requestsToSend.push_back(std::make_pair(m_nextID,
                                        tmpBuf));
   delete [] tmpRequest;

   writeNextRequest();
   
   return m_nextID;   
}


