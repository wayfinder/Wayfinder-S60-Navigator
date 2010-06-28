/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 


#ifndef COMMON_WAYFINDER_TARGETS_H
#define COMMON_WAYFINDER_TARGETS_H

/***************************************
 * Useful defines
 * ************************************/
// # define FULL_GRID_MENU
// # undef APPLICATION_VISIBLE_NAME
// # define APPLICATION_VISIBLE_NAME "WF Debug"
// # undef APPLICATION_HANDLER_VISIBLE_NAME
// # define APPLICATION_HANDLER_VISIBLE_NAME APPLICATION_VISIBLE_NAME
// # undef APPLICATION_SHORT_NAME
// # define APPLICATION_SHORT_NAME "WF Dbg"
// # define SEND_DEBUG_SMS_ON_ERRORS
// # undef SEND_DEBUG_SMS_NUMBER
// # define SEND_DEBUG_SMS_NUMBER "+467XXXXXXXXX"
// # define USE_CELL_INFO_IN_SMS
// # undef ALLOWED_LANGS
// # define ALLOWED_LANGS "XX EN DE ES SV"
// # define SHOW_IMEI
// # define ENABLE_ROUTE_SIMULATION
// # define ENABLE_DEBUG_OPTIONS
// # undef SHOW_US_POPUP_WARNING
// # undef SEND_REGISTRATION_SMS
// # undef __RELEASE__
// # undef USE_ONLY_ONE_LANGUAGE
// # define SHOW_BUY_EXTENSION_ON_EXPIRED_USER
/* # define HIDE_TRIAL_VIEW */
// #define INTERPOLATE_POSITIONS
// #define ENABLE_3D_OUTLINES


#ifdef RELEASE_DEFAULT_BRONZE_SUPER_SS
# define RELEASE_DEFAULT_BRONZE_SUPER
# define RELEASE_DEFAULT_BRONZE_SS
#endif

#ifdef RELEASE_DEFAULT_BRONZE_SUPER_SSE
# define RELEASE_DEFAULT_BRONZE_SUPER
# define RELEASE_DEFAULT_BRONZE_SSE
#endif

#ifdef RELEASE_DEFAULT_BRONZE_NA_SUPER_SSE
# define RELEASE_DEFAULT_BRONZE_NA_SUPER
# define RELEASE_DEFAULT_BRONZE_NA_SSE
#endif

#ifdef RELEASE_DEFAULT_BRONZE_NA_SUPER_SS
# define RELEASE_DEFAULT_BRONZE_NA_SUPER
# define RELEASE_DEFAULT_BRONZE_NA_SS
#endif

/* *****************
 * Default targets
 * *****************/

#ifdef RELEASE_DEFAULT_BRONZE_SS
# define SYMBIAN_SIGNED_SUBMIT_PKG_SIGNATURE
# define RELEASE_DEFAULT_BRONZE
#endif

#ifdef RELEASE_DEFAULT_BRONZE_SSE
# define SYMBIAN_SIGNED_EXPRESS_PKG_SIGNATURE
# define RELEASE_DEFAULT_BRONZE
#endif

#ifdef RELEASE_DEFAULT_BRONZE_SUPER
# define SUPERSIS
# define RELEASE_DEFAULT_BRONZE
#endif

#ifdef RELEASE_DEFAULT_BRONZE_SUPER_DEMO
# define SUPERSIS
# define RELEASE_DEFAULT_BRONZE_DEMO
#endif

#ifdef RELEASE_DEFAULT_BRONZE_COMBO
# define DISABLE_LANG_SELECTOR_FILE
# define RELEASE_DEFAULT_BRONZE
#endif

/* NA */

#ifdef RELEASE_DEFAULT_BRONZE_NA_SS
# define SYMBIAN_SIGNED_SUBMIT_PKG_SIGNATURE
# define RELEASE_DEFAULT_BRONZE_NA
#endif

#ifdef RELEASE_DEFAULT_BRONZE_NA_SSE
# define SYMBIAN_SIGNED_EXPRESS_PKG_SIGNATURE
# define RELEASE_DEFAULT_BRONZE_NA
#endif

#ifdef RELEASE_DEFAULT_BRONZE_NA_COMBO
# define DISABLE_LANG_SELECTOR_FILE
# define RELEASE_DEFAULT_BRONZE_NA
#endif

#ifdef RELEASE_DEFAULT_BRONZE_NA_SUPER
# define SUPERSIS
# define RELEASE_DEFAULT_BRONZE_NA
#endif

/* ****************
 * Master targets
 * ****************/

#ifdef RELEASE_DEFAULT_BRONZE_DEMO
# ifndef APPLICATION_HANDLER_VISIBLE_NAME
#  define APPLICATION_HANDLER_VISIBLE_NAME APPLICATION_VISIBLE_NAME
# endif
# ifndef APPLICATION_SHORT_NAME
#  define APPLICATION_SHORT_NAME "Navigator"
# endif
# ifndef NAV2_CLIENT_TYPE_STRING
#  define NAV2_CLIENT_TYPE_STRING NAV2_CLIENT_TYPE_ARCH_BASE "-demo"
# endif
# define SHOW_CHOOSE_IAP_SEARCH_METHOD
// Limit network pipelining to 4 messages
# define MAXNBROUTSTANDING 5
# define NOKIALBS
#endif

#ifdef RELEASE_DEFAULT_BRONZE_NA_DEMO
# ifndef APPLICATION_HANDLER_VISIBLE_NAME
#  define APPLICATION_HANDLER_VISIBLE_NAME APPLICATION_VISIBLE_NAME
# endif
# ifndef APPLICATION_SHORT_NAME
#  define APPLICATION_SHORT_NAME "Navigator"
# endif
# ifndef NAV2_CLIENT_TYPE_STRING
#  define NAV2_CLIENT_TYPE_STRING NAV2_CLIENT_TYPE_ARCH_BASE "-demo"
# endif
# define SETTING_FACTORED_OVERRIDE_US
#endif

/* ************* */

#endif
