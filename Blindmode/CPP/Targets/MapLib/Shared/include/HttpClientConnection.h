/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef HTTP_CLIENT_CONNECTUION_H
#define HTTP_CLIENT_CONNECTUION_H

#include "config.h"

#include "MC2SimpleString.h"

#include <list>

class HttpClientConnectionListener;
class SharedBuffer;
class TCPConnectionHandler;

/**
 *   Class describing a asynchronous connection using TCP.
 */
class TCPClientConnection {
public:
   /**
    *   Enum describing the status of the different calls.
    */
   enum status_t {
      /// All is done.
      OK                =  0,
      /// Some error occured.
      ERROR             = -1,
      /// A timout occured
      TIMEOUT           = -2,
      /// An operation of the same kind has been requested already.
      ALREADY_REQUESTED = -3,
      /// The connection was closed
      CLOSED            = -4,
   };
   
   /**
    *   To be called when the connection is established or has
    *   failed.
    */
   virtual void connectDone(status_t status) = 0;

   /**
    *   To be called when the last read is done.
    *   @param status The status of the read.
    */
   virtual void readDone(status_t status,
                         const byte* bytes,
                         int nbrBytes) = 0;

   /**
    *   Informs the ClientConnection that the last write is completed.
    *   @param status The status of the write.
    */
   virtual void writeDone(status_t status) = 0;

   /**
    *   Informs the ClientConnection that the connection is closed.
    */
   virtual void connectionClosed(status_t status) = 0;

   /**
    *   Informs the ClientConnection that the timer with id
    *   timerID has expired.
    */
   virtual void timerExpired(int timerID) = 0;
};


class HttpClientConnectionReader;
class HttpClientConnectionWriter;

/**
 *   Class that handles a HttpConnection to one host and port.
 */
class HttpClientConnection : public TCPClientConnection {
public:
   friend class HttpClientConnectionReader;
   /**
    *   State of the 
    */
   enum state_t {
      NOT_CONNECTED  = 0,
      CONNECTED      = 1,
   } m_state;
    
   /**
    *   Creates a new HttpClientConnection to the supplied
    *   host and port.
    */
   HttpClientConnection(const char* host, unsigned int port,
                        HttpClientConnectionListener* listener,
                        TCPConnectionHandler* connectionHandler);

   /**
    *   Sets listener.
    */
   void setListener(HttpClientConnectionListener* listener);
   
   /**
    *   Deletes the data associated with the connection.
    *   Closes connection.
    */
   virtual ~HttpClientConnection();

   /**
    *   Sends a get-request for the supplied filespec, if there
    *   is enough place in the queue.
    *   @param filespec1 File from server.
    *   @param filespec2 Will be concatenated to filespec1 if not NULL.
    *   @param urlParams [Optional] The url parameters 
    *                    (for instance containing the session).
    *                    Should begin with a "?".
    *   @return A positive number is a request id and a negative number
    *           means that the queue is full.
    */
   int get(const char* filespec1,
           const char* filespec2 = NULL,
           const char* filespec3 = NULL,
           const char* urlParams = "");

   /**
    *   Sends a post-request for the supplied filespec.
    *   @param urlParams [Optional] The url parameters 
    *                    (for instance containing the session).
    *                    Should begin with a "?".
    */
   int post(const char* filespecPart1,
            const char* filespecPart2,
            const char* mimeType,
            const SharedBuffer& body,
            const char* urlParams = "");
   
   /**
    *   To be called when the connection is established or has
    *   failed.
    */
   void connectDone(status_t status);

   /**
    *   To be called when a read is done.
    *   
    */
   void readDone(status_t status,
                 const byte* bytes,
                 int nbrBytes);

   /**
    *   Informs the ClientConnection that the last write is completed.
    */
   void writeDone(status_t status);
   
   /**
    *   Informs the ClientConnection that the connection is closed.
    */
   void connectionClosed(status_t status);

   /**
    *   Informs the HttpClientConnection that the timeout
    *   with id timerID has expired.
    */
   void timerExpired(int timerID);

   /**
    *   Returns the number of requests to send.
    */
   int getNbrSent() const { return m_waitingFor.size(); }
   
   /**
    *   Writes a new request
    */
   void oneRequestComplete();

   /**
    *   Returns the number of the current request we are
    *   waiting for.
    */
   int getCurrentRequestNbr() const;

   /**
    *   Sets hostname and port.
    */
   void setHostAndPort(const char* host, unsigned int port);
    
   
protected:

   /**
    *   Tries to write a request to the socket.
    */
   void writeNextRequest();

   /**
    *   Tries to read a request from the socket.
    */
   void readNextRequest();
   
private:
   /**
    *   Hostname.
    */
   MC2SimpleString m_host;
   
   /**
    *   Number of the port to use for the connection.
    */
   unsigned int m_port;

   /**
    *   User agent string to send to the server.
    */
   MC2SimpleString m_userAgent;

   /**
    *   Prefix all the url:s with this string.
    *   To be used when using proxy.
    */
   MC2SimpleString m_urlPrefix;
   
   /**
    *   Maximum number of requests to have at a certain time.
    */
   int m_maxNbrGet;

   /**
    *   Type of list to keep the requests in.
    */
   typedef std::list<std::pair<int, SharedBuffer*> > requestList_t;
   
   /**
    *   The outgoing requests.
    */
   requestList_t m_requestsToSend;

   /**
    *   The requests we are waiting for.
    */
   requestList_t m_waitingFor;

   /**
    *   List containing zero or one requests that are currently
    *   written.
    */
   requestList_t m_currentlyWriting;

   /**
    *   Next id for the requests.
    */
   int m_nextID;
   
   /**
    *   The connection handler.
    */
   TCPConnectionHandler* m_connHandler;

   /**
    *   True if the connection is connecting.
    */
   bool m_connecting;

   /**
    *   True if there is a pending read.
    */
   bool m_reading;

   /**
    *   True if there is a pending write.
    */ 
   bool m_writing;

   /**
    *   The number of written requests for which
    *   we haven't got any answers yet.
    */
   int m_nbrWaiting;

   /**
    *   The number of failed attempts in a row.
    */
   int m_nbrRetries;

   /**
    *   Handles all reads from the connection.
    */
   HttpClientConnectionReader* m_connReader;
   
   /**
    *   The listener.
    */
   HttpClientConnectionListener* m_listener;

   /**
    *   The maximum number of retries.
    */
   const int m_maxNbrRetries;

   /**
    *   Header lines that should only be used when using proxy.
    */
   const char* m_proxyHeaderLines;
   
};


class TCPConnectionHandler {
public:   
   /**
    *   Tells the underlying layer to connect to the
    *   supplied host and port. When connected, connected
    *   should be called.
    *   Should call ccon->connectDone when done.
    *   Max one connect per ClientConnection is allowed simultaneously.
    */
   virtual void connect(const char* host, unsigned int port,
                        TCPClientConnection* ccon) = 0;

   /**
    *   Tells the underlying layer to disconnect the
    *   current connection.
    */
   virtual void disconnect(TCPClientConnection* ccon) = 0;

   /**
    *   Tells the underlying layer to write the supplied
    *   bytes. The underlying layer must keep the order
    *   of the write requests.
    *   Should call ccon->writeDone when done.
    *   Max one write per ClientConnection is allowed.
    *   Not allowed if not connected.
    */
   virtual void write(const byte* bytes, int length,
                      TCPClientConnection* ccon) = 0;
      
   /**
    *   Tells the underlying layer to start receiving reads
    *   with up to maxLength bytes from the 
    *   socket. Max <code>length</code> bytes.
    *   Should call the ccon->readDone when done.
    *   Max one read per ClientConnection is allowed.
    *   Not allowed if not connected.
    */
   virtual void read(int maxLength,
                     TCPClientConnection* ccon) = 0;

   /**
    *   Removes the connection from the internal list of
    *   connections in the TCPConnectionHandler.
    */
   virtual void remove(TCPClientConnection* conn) = 0;

   /**
    *   Requests a timeout for the connection conn.
    *   After the timeout, the connectionhandler should call
    *   timerExpired on the connection.
    *   @param nbrMillis The number of milliseconds to wait before
    *                    sending the timeout.
    *   @param conn      The connection that should have the timeout.
    *   @return ID of the timeout.
    */
   virtual int requestTimer(uint32 nbrMillis,
                            TCPClientConnection* conn) = 0;

   /**
    *   Cancels the timer with id timerID.
    */
   virtual void cancelTimer(int timerID) = 0;
};

#endif
