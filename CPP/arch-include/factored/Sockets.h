/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_SOCKETS_H
#define MC2_SOCKETS_H


#ifdef __linux
   #include <sys/socket.h>
   #include <sys/time.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <netdb.h>
   #include <unistd.h>
   #include <sys/types.h>

   #include <fcntl.h>
   #include <signal.h>

   #ifndef _WIN32
      typedef  int   SOCKET;
      #ifndef SOCKET_ERROR
         #define SOCKET_ERROR -1
      #endif
   #endif
#endif

#ifdef __SVR4
   #include <limits.h>
   #include <sys/socket.h>
   #include <sys/time.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <netdb.h>
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/param.h>
   #include <fcntl.h>
   #include <signal.h>

   #ifndef _WIN32
      typedef  int SOCKET;
      #ifndef SOCKET_ERROR
         #define SOCKET_ERROR -1
      #endif
   #endif

   // MAX() and MIN() is declared in <sys/param.h> on linux 
   #define MIN(a,b) (((a)<(b))?(a):(b))
   #define MAX(a,b) (((a)>(b))?(a):(b))
   // and in Utility in mc2
   //#include "Utility.h"

#endif


#if defined(_WIN32) && !defined(__SYMBIAN32__)
   #include <winsock.h>
   #define socklen_t int 
   #ifndef SOCKET_ERROR
      #define SOCKET_ERROR INVALID_SOCKET
   #endif
   // unsigned int
#endif

#ifdef __SYMBIAN32__
//   #include <es_prot.h>
   #include <es_sock.h>
   #include <in_sock.h>
   #if defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
      #include <agentclient.h>
   #endif

#if 1
   #define ssize_t TInt
   typedef RSocket SOCKET;

#else
   typedef  int SOCKET;
   typedef  long ssize_t;
   typedef  long socklen_t;
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <sys/time.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <netdb.h>
   #include <unistd.h>

   #include <fcntl.h>
   #include <signal.h>
   #include <possock.h>

   #ifndef SOCKET_ERROR
      #define SOCKET_ERROR -1
   #endif
#endif

#endif // __SYMBIAN32__

// Define the maximum length of a host name
#ifndef MAXHOSTNAMELEN
   #define MAXHOSTNAMELEN 64
#endif



#endif

