/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ARCHINCLUDE_ARCH_H
#define ARCHINCLUDE_ARCH_H

/* Archincludes for Symbian */

// Endian issues: BigEndian, LittleEndian, and ByteOrder can be used to order integers to and from network order.

#include <arpa/inet.h>
// Defines NULL unconditionally, must be first
#include <e32def.h>
// Defnies operator new unconditionally, must be included early
#include <e32std.h>
#include <stdlib.h>

/* htons, htonl, ntohs, and ntohl. */
#include <sys/types.h>
// #include <netinet/in.h>

// strlen and mem[cpy|chr]
#include <string.h>

#include "machine.h"
//#include "ArchLog.h"
//#include "ArchFastMath.h"

//#define STR2(a) #a
//#define STR(a) STR2(a)


#define MLIT(name,s) _LIT(name,s)

#ifdef _UNICODE
# define TL(s) L##s
#else
# define TL(s) s
#endif

#define ML(s) TL(s)

// typedefs needed to make exception handler types work.
//Symbian wisely(?) chose to change the TExceptionHandler typedef in version 9.
#if defined NAV2_CLIENT_SERIES60_V3 || defined NAV2_CLIENT_UIQ3
   typedef TExceptionHandler TExceptionHandlerPtr;
#else
   typedef TExceptionHandler* TExceptionHandlerPtr;
#endif

#endif
