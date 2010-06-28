/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef NAV2_CLIENT_TYPE_STRING
# error No client type string defined for the selected taget in WayFinderSettings.h or an invalid target configuration was specified.
#endif

#ifndef NAV2_CLIENT_OPTIONS_STRING
# define NAV2_CLIENT_OPTIONS_STRING "none"
#endif


#ifndef TESTSHELL
# define TESTSHELL DEBUGMACHINE,0
#endif
#ifndef SIMINPUT
# define SIMINPUT  DEBUGMACHINE,4448
#endif

#define GUIHOST   "127.0.0.1"
#define GUIPORT   11112

#define BTCPORT   11115

#undef APPLICATION_ARCH_NAME
#if defined NAV2_CLIENT_SERIES60_V1
# define APPLICATION_ARCH_NAME "_s60v1"
#elif defined NAV2_CLIENT_SERIES60_V2
# define APPLICATION_ARCH_NAME "_s60v2"
#elif defined NAV2_CLIENT_SERIES60_V3
# if defined NAV2_CLIENT_SERIES60_V5
// Since s60v5 target also defines s60v3.
#  define APPLICATION_ARCH_NAME "_s60v5"
# else 
#  define APPLICATION_ARCH_NAME "_s60v3"
# endif
#elif defined NAV2_CLIENT_SERIES80
# define APPLICATION_ARCH_NAME "_s80"
#elif defined NAV2_CLIENT_SERIES90_V1
# define APPLICATION_ARCH_NAME "_s90"
#elif defined NAV2_CLIENT_UIQ
# define APPLICATION_ARCH_NAME "_uiq"
#elif defined NAV2_CLIENT_UIQ3
# define APPLICATION_ARCH_NAME "_uiq3"
#else
# error "Unsupported target architecture for this target."
#endif
#undef APPLICATION_DIR_NAME
#define APPLICATION_DIR_NAME MacroConcat2(APPLICATION_BASE_DIR_NAME,APPLICATION_ARCH_NAME)

#if defined BTGPS || defined BTGPS_THREAD
# define BTGPS_COMMON
#endif

#ifndef APPLICATION_SHORT_NAME
# define APPLICATION_SHORT_NAME APPLICATION_VISIBLE_NAME
#endif

#ifndef APPLICATION_HANDLER_VISIBLE_NAME
# define APPLICATION_HANDLER_VISIBLE_NAME APPLICATION_VISIBLE_NAME
#endif

#ifndef APPLICATION_HANDLER_VISIBLE_NAME_PKG
# define APPLICATION_HANDLER_VISIBLE_NAME_PKG APPLICATION_HANDLER_VISIBLE_NAME
#endif

#ifndef BIO_NAME_PREFIX
# define BIO_NAME_PREFIX APPLICATION_BASE_DIR_NAME
#endif

