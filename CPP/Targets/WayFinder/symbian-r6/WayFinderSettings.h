/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WAYFINDER_SETTINGS_H
#define WAYFINDER_SETTINGS_H


#include "CommonWayfinderSettings.h"
#include "CommonWayfinderTargets.h"

#define WayFinder_cap_str LocalServices+NetworkServices+ProtServ+ReadDeviceData+ReadUserData+SwEvent+WriteDeviceData+WriteUserData+Location
#define WayFinder_cap_num 0x0003f130u

/* Use self signed packages unless selected explicitly by target. */
#ifndef UNSIGNED_PKG_SIGNATURE
# ifndef SYMBIAN_SIGNED_SUBMIT_PKG_SIGNATURE
#  define SELF_SIGNED_PKG_SIGNATURE
# endif
#endif

#ifdef NAV2_CLIENT_SERIES60_V3
#undef SHOW_MOVEWAYFINDER
#define DISABLE_IAP_SEARCH
#endif

#include "CommonWayfinderTrailer.h"

#endif /* WAYFINDER_SETTINGS_H */
