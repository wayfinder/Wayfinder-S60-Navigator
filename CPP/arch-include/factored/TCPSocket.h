/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Generic TCP Socket implementation.
 *
 */

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

/* 5 outstanding connect requests are supported in all */
/* TCP implementations. For a busy server on a newer OS */
/* there should not be such a low limit. */
#define DEFAULT_BACKLOG 5

/* How many ports to loop through, trying to find a free one. */
#define MAX_NBR_TRIES_PORTFIND 5

#include <arch.h>
#include "Readable.h"
#include "Writable.h"
#include "Selectable.h"
#ifdef __SYMBIAN32__
# include "Sockets.h"
#elif _MSC_VER
# include <windows.h>
# include <winsock.h>
#endif

namespace isab {
   class Log;

   /**
    * Generic TCP Socket implementation.
    * The TCPSocket class can be read from, written to, and selected upon. 
    * Convenience routines for doing reads/writes with timeouts are
    * also available.
    *
    */
   class TCPSocket : public Readable, public Writable, public Selectable {

      public:
         int32 m_error;

         /**
          * State of the socket.
          */
         enum TCPSocketState {
            /* State unknown (probably just created). */
            UNKNOWN,
            /* Created and bound, but not open yet. */
            BOUND,
            /* Socket open. */
            OPEN,
            /* Socket open for listening. */
            LISTEN,
            /* The socket is connected to another socket */
            CONNECTED,
            /* This socket is closed. */
            CLOSED,
         };


         /**
          *    If the function is not done in 10 minutes, something is 
          *    wrong.
          */
#        define TCPSOCKET_DEFAULT_TIMEOUT 10 * 60 * 1000000

         /**
          *    Also define the infinite timeout-time.
          */
#        define TCPSOCKET_INFINITE_TIMEOUT -1

         /**
          * Constructor.
          *
          * @param backLog       The number of pending connections that the
          *                      kernel should keep before we accept them.
          */
         TCPSocket(int backLog);

         /**
          * Constructor which uses an existing socket.
          */
         TCPSocket(SOCKET sock,
                   int backLog,
                   TCPSocketState state);

         /**
          * Destructor.
          */
         virtual ~TCPSocket();

         /**
          * Bind the socket to a local address.
          *
          * @param hostname      The hostname for the localhost.
          *                      This hostname will decide which
          *                      interface (IP-address) the socket
          *                      should be bound to.
          * @param portNumber    The port that should be bound to
          *                      this socket.
          * @return              1 for success, -1 for temporary failure
          *                      and -2 for permanent error.
          */
         virtual int bind(const char *hostname, uint16 portNumber);

         /**
          * Close the socket in a friendly way, reading out and
          * discarding all data.
          *
          * @return              1 for success, -1 for error.
          */
         virtual int close();

         /**
          * Set up a connection to the Internet through which tcp
          * session can be set up.
          *
          * @param connParam     Select which connection to use. (Internet Access Point)
          * @return <code>None</code> for successfull connection, 
          */
         int setupConnection ( int32 connParam );


         /**
          * This enum defines the different strategies available
          * for choosing a free local port.
          */
         enum TCPPortChoice { GENERIC, FINDFREEPORT, DONTCARE };

         /**
          * Bind and listen on the socket.
          *
          * @param portnumber    The local portnumber to bind to.
          *                      The value zero is not a valid portnumber.
          * @param pc            Port choice algorithm.
          *          The GENERIC case is the normal one, we send in one
          *          portnumber, and if it isn't free, we'll return an error.
          *
          *          The FINDFREEPORT tries to allocate the specified port,
          *          and if that port is not free, the next higher numbered
          *          port, and so on. If a free port is not found in a certain
          *          range, the routine returns an error.
          *
          *          DONTCARE implies that the portnumber is not important and
          *          as long as we get one, everything is allright.
          *          The OS then allocates an "ephemeral" port which is free.
          *          We can still return error if there are no resources available
          *          to open a new port.
          *  @return             The portnumber which was finally chosen,
          *                      or zero on error.
          */
         virtual uint16 listen(uint16 portnumber, TCPPortChoice pc = GENERIC);

         /**
          * Use the socket to connect to the specified host and portnumber.
          *
          * @param hostname      Host to connect to.
          *                      The hostname may either be a regular hostname
          *                      or an ip number in string form.
          * @param portnumber    Port to connect to.
          * @return <code>1</code> for successfull connection, 
          *         <code>NoLookup</code> when DNS lookup failed,
          *         <code>Refused</code> for refused connection, and 
          *         <code>Timeout</code> for connection timeout.
          *         <code>Unable</code> for lack of system resources.
          */

         virtual int connect(const char *hostname, int portnumber);

         /**
          * Use the socket to connect to the specified host and portnumber.
          *
          * @param hostname      Host ip to connect to, specified as an uint32.
          * @param portnumber    Port to connect to.
          * @return              1 if connect was successful,
          *                      -1 if the hostname could not be looked up,
          *                      or -2 if the connect failed.
          */
         virtual int connect(uint32 hostip, uint16 hostport);

         /**
          * Read at most length bytes from the socket to buffer.
          * This function will block if there is no data to be
          * read from the socket and the socket is set to blocking.
          * The return value could be zero if the socket has been closed.
          * Observe that the length parameter is of type ssize_t,
          * not size_t as in posix read. We only can return a ssize_t.
          *
          * @param buffer        Buffer to read bytes into.
          * @param length        Maximum number of bytes to read.
          * @return              The number of bytes read from the
          *                      socket, or -1 on error.
          */
         virtual ssize_t read(uint8 *buffer, ssize_t length);

#ifdef __SYMBIAN32__
         /**
          * Read at most length bytes from the socket to buffer.
          * This function will block if there is no data to be
          * read from the socket and the socket is set to blocking.
          * The return value could be zero if the socket has been closed.
          * Observe that the length parameter is of type ssize_t,
          * not size_t as in posix read. We only can return a ssize_t.
          *
          * @param buffer        Buffer to read bytes into.
          * @param length        Maximum number of bytes to read.
          * @param micros        Wait time in microseconds.
          * @return              The number of bytes read from the
          *                      socket, or -1 on error, or -2 on timeout.
          */
         virtual ssize_t read( uint8 *buffer, ssize_t length, 
                               uint32 micros );
#endif
         
         /**
          * Write length bytes from buffer to the socket.
          * This function will block if there isn't enough
          * resources to cache the data and the socket is set
          * to blocking.
          *
          * @param buffer        Buffer with the data to be written
          *                      to the socket.
          * @param length        Number of bytes to read.
          * @return              Number of bytes written, or -1 on error.
          */
         virtual ssize_t write(const uint8 *buffer, ssize_t length);

         /**
          * Accept a connection on the socket.
          * This function will block, unless the socket is
          * set to blocking.
          * A new socket will be created and returned,
          * and it is the responsibility of the caller to
          * deallocate that socket.
          *
          * @return              New TCPSocket object representing the
          *                      new connection. (The old socket may
          *                      be reused immediately)
          */
         virtual TCPSocket* accept();

         /**
          * Accept as above, but timeout after a specified time.
          * @param micros        Timeout in microseconds.
          * @return              New TCPSocket object representing the
          *                      new connection. (The old socket may
          *                      be reused immediately)
          */
         virtual TCPSocket* accept(uint32 micros);

         /**
          * Get the address (IP address packed in an uint32) of our peer.
          * @param IP            Reference to uint32 which will be updated
          *                      with the peers packed IP address.
          * @param port          Reference to the variable which will hold
          *                      the peer portnumber after the call.
          * @return              True if the address could be gotten, false
          *                      otherwise.
          */
         bool getPeerName(uint32& IP, uint16& port);

         /**
          * Return the underlying socket object.
          * @return              The socket filedescriptor.
          */
         SOCKET getSOCKET() const;

         SOCKET getFd() const {
            return getSOCKET();
         }

         /**
          * Get the current state of the TCPSocket.
          * @return              The current state of the socket.
          */
         TCPSocketState getState() const;

      /** Cancels any outstanding operations, causing them to fail in
          proper ways. */
      void cancelAll();

      /** 
       * Gets the current IP used. The current IP is set on each
       * successfull call to connect and may be invalidated bay any
       * subsequent call to connect, successfull or not.
       * @return the IP address in host order. 
       */
      uint32 getIP() const;

      protected:

         /**
          * Internal read function. Will reinitiate read after an
          * EINTR error (which seems to be common when running with
          * JTC threads)
          * @param buffer           Buffer to read into.
          * @param length           Maximum number of bytes to read.
          * @return                 Actual number of bytes written.
          */
         ssize_t protectedRead(uint8 *buffer, ssize_t length);

         /**
          * Internal write function. Will reinitiate write after
          * an EINTR error (which seems to be common when running with
          * JTC threads)
          * @param buffer           Buffer with data to write.
          * @param size             Number of bytes to write.
          * @return                 Actual number of bytes written.
          */
         ssize_t protectedWrite(const uint8* buffer, ssize_t size);

         /**
          * Set the socket to be blocking or nonblocking.
          * @param blocking         True to set to blocking, false to set
          *                         to nonblocking.
          * @return                 True if successful, false on failure.
          */
         bool setBlocking(bool blocking);

         /**
          * Set the "pipe" signal handler to ignore.
          * Used to circumvent that signal when writing to a
          * closed or full socket.
          */
         void ignorePipe();

         /**
          * Set the "pipe" signal handler to default.
          */
         void setPipeDefault();

         /**
          * For Windows TCP.
          */
         int c_tcp_proto_nbr;

         /**
          * Creates a new socket and sets it up.
          *
          */
         virtual bool create();

      private:

         /**
          * TCP portnumber.
          */
         int    m_portNumber;

         /**
          * TCP socket variable.
          */
         SOCKET m_socket;

         /**
          * The maximum number of pending connections (not "accept()ed" yet)
          * allowed to this socket.
          */
         int m_backlog;

         /**
          * Describes if the socket is blocking or nonblocking.
          */
         bool m_blocking;

         /**
          * Current state of the socket.
          */
         TCPSocketState m_currentState;

         /**
          * Initialize member variables.
          * @param sock          Socket filedescriptor.
          * @param backlog       Number of pending connects that the
          *                      kernel should allow before we accept.
          */
         void init(SOCKET sock, int backlog);

         /**
          * Initialize member variables.
          * @return
          */
         int setupSocket();

   public:
      enum ConnectError {
         None     =  0, ///No error
         NoLookup = -1, ///DNS lookup failed.
         Refused  = -2, ///Connection was refused by server
         Timeout  = -3, ///Never reached server, timeout.
         Unable   = -4, ///Out of system resources.
         Carrier  = -5, ///Unable to set up the carrier (GPRS/CSD etc)
      };
   private:

      /**
       * Store the IP here in host order. Sometimes we want to know
       * the IP of a host we have connected to.
       */
      uint32 m_ip;
#ifdef __SYMBIAN32__
         bool getHostAddr( TInetAddr& addr, const char *hostname );

      /**
       * @return <code>None</code> for successfull connection, 
       *         <code>Refused</code> for refused connection, and 
       *         <code>Timeout</code> for connection timeout.
       *         <code>Unable</code> for lack of system resources.
       */
         int doConnect( TInetAddr& addr );

         /**
          * Creates a blank socket to be used in accept.
          */
         bool createBlank( );

      int m_maxRead;

      TBuf8<1024> m_tBuf;
      TRequestStatus m_readStatus;
      TSockXfrLength m_readLength;


#else
         bool getHostAddr(struct sockaddr_in *sin, const char *hostname);

      /**
       * @return <code>None</code> for successfull connection, 
       *         <code>Refused</code> for refused connection, and 
       *         <code>Timeout</code> for connection timeout.
       *         <code>Unable</code> for lack of system resources.
       */
         int doConnect(struct sockaddr_in *sin);
#endif

#ifdef __SYMBIAN32__
         /**
          * The socket session.
          */
         RSocketServ *m_session;
# if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3) || defined(NAV2_CLIENT_SERIES60_V3)
         /**
          * The RConnection Connection Manager object
          */
         RConnection *m_connMgr;
# elif defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
         RGenericAgent *m_netAgent;
# endif 
#elif _MSC_VER
         /**
          *   Initializes winsock.
          */
         static bool initWinsock();
         
         /**
          *  Contains details of the Winsock implementation
          *  Don't know what that means, but it's what the docu says.
          *  Must be initialized once.
          */
         static WSADATA c_wsaData;

         /**
          *   True if winsock was inited ok.
          */
         static bool c_winsockOK;
#endif
   };

} /* namespace isab */

#endif /* TCPSOCKET_H */
