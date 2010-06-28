/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMON_WAYFINDER_SETTINGS_H
#define COMMON_WAYFINDER_SETTINGS_H


#include "CommonWayfinderVersion.h"

//Move to a better place
#define ARC_DE_TRIOMPHE 2
#define WASHINGTON_MEMORIAL 3
#define BRANDENBURGER_TOR 4


// To allow config-dependant rsg files..... 
#if defined(RSG_DEFAULT) || defined(MBG_DEFAULT)
# define RELEASE_DEFAULT_BRONZE
#endif



/* The default settings. These are compatible with the old
 * target RELEASE_DEFAULT_BRONZE. For all other targets 
 * some defines needs to be overridden. */

// Wayfinder ID with silent startup (no service window if uin exists).
#define USE_SILENT_STARTUP

// Use a trial system. See also USE_AKM, USE_AKM_???? and
// TRIAL_IS_MAPGUIDE for options also affecting
// the trial system. */
#define BRONZE_TRIAL

// The trials have access to the gps
// Change the undef to a define to prevent this.
#undef TRIAL_IS_MAPGUIDE

// The user can not switch between languages.
// This must only be used for sis files that
// contain only one language. (i.e. 
// WayFinder+CONFIG@EN.sis)
#undef USE_ONLY_ONE_LANGUAGE

#undef DISABLE_LANG_SELECTOR_FILE

// Parameter file trailer name
// A file <PARAM_TRAILER_NAME>_<LANG>.txt is appended to the
// generated paramseed file.
#define PARAM_TRAILER_NAME param_trailer_europe

// Include standard bronze seed file in SIS.
#define INCLUDE_SEED_FILE

// Good to have. Always used.
#define USE_EXCEPTION_HANDLER
//
// Good to have. Always used.
#define USE_TRAP_HARNESS_ON_THREADS

// Undefine to get access to special features for debugging.
#define __RELEASE__

// Use HTTP Protocol
#define HTTP_PROTO
// The Http User-Agent string, default none
#define HTTP_USER_AGENT NULL
// The Http request string, default none (which will give "/nav")
#define HTTP_REQUEST_STRING NULL
// If defined to a string that string will be the content of the http
// Host header
#undef HTTP_HOST_OVERRIDE
// If to use the proxy host and port in the IAP, default no.
#undef USE_IAP_PROXY
// If to use hardcoded proxy host in the IAP, default no. See also port.
#undef HARD_IAP_PROXY_HOST
// If to use hardcoded proxy port in the IAP, default no. Used with
// HARD_IAP_PROXY_HOST
#undef HARD_IAP_PROXY_PORT

// If to use IMSI as licence key type (instead of IMEI).
#undef LICENCE_KEY_TYPE_IMSI

/* Symbian 9 requires that a privacy statement is shown to the user at the first run */
#if defined NAV2_CLIENT_SERIES60_V3
# define SHOW_PRIVACY_STATEMENT
#endif
// Now we don't want the privacy statement.
#undef SHOW_PRIVACY_STATEMENT

// But we want the end user warning message instead.
#define SHOW_END_USER_WARNING_MESSAGE
#undef SHOW_END_USER_WARNING_MESSAGE

// Installation directory and target name on phone for pkg etc.
#define APPLICATION_BASE_DIR_NAME "wayfinder8"

// The name of the application visible in the phone. (AIF etc)
#define APPLICATION_VISIBLE_NAME "Navigator"

// Default center point for vector maps is europe.
#define VECTOR_MAP_CENTER ARC_DE_TRIOMPHE
// currently only for UIQ (ARC_DE_TRIOMPHE)
#define MAPCENTERLAT 85300832
#define MAPCENTERLON 4006333

// The favorite image name for displaying favorites in the map.
#define WF_MAP_FAVORITE_IMAGE_NAME "favorite"
// The small favorite image name for displaying favorites in the map.
#define WF_MAP_FAVORITE_IMAGE_NAME_SMALL "favorite_small"

// Disable remote debugging (debuglog over tcp)
#define DEBUGMACHINE    NULL

// Disable the debug log
#define DEBUGOUTPUT     NULL

// Use new TCP Symbian serial.
#undef USE_TCP_SYMBIAN_SERIAL

// Use a direct connection between the gui and nav2. Unused? FIXME
#define DONT_USE_TCP_TO_GUI 
//
// Use a direct connection between the gui and nav2
#define DONT_USE_TCP_TO_NAV2

// Use bluetooth to connect to the GPS
#define USE_BLUETOOTH

// Use the new script-based syntaxes
#define USE_SYNTAX_SCRIPTS

// Let Nav2 connect to the Gui bluetooth connection directly. 
// Wayfinder will probably not work without this anymore.
#define DONT_USE_TCP_TO_BLUETOOTH 

// Define to use Nokia Mobile Location Framework instead
// of our own blutooth implementation. Needs USE_BLUETOOTH
// as well.
#undef NOKIALBS
#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES60_V3 || defined NAV2_CLIENT_UIQ33
# if defined __WINS__ || defined SYMBIAN_WINSCW
#  undef NOKIALBS
#  define HIDE_MLFW_OPTION
# else
#  define NOKIALBS
#  undef HIDE_MLFW_OPTION
# endif
#endif

// Disable gps simulation
#define SIMINPUT        NULL,0

// Disable gps simulation
#undef SIMFILE

// Default to NMEA simulation
#define SIM_PROTO NMEA

// Always show the upgrade option
#undef SHOW_UPGRADE_ALWAYS

// Hide the upgrade option in trial
#undef HIDE_UPGRADE_IN_TRIAL

// Hide the upgrade option in trial
#undef HIDE_UPGRADE_IN_SILVER

// Hide the upgrade option in gold
#undef HIDE_UPGRADE_IN_GOLD

// Hide upgrade option for softkeys.
#define HIDE_UPGRADE_IN_CBA

// Hide trial option in Trial / Reregister / Register menu
#undef HIDE_TRIAL_OPTION

// The "ReRegister" option in Trial menu will open upgrade dialog. 
#undef TRIAL_REREGISTER_IS_UPGRADE

// Hide "activate"/"register" menu option in services.
#undef HIDE_SUBMIT_CODE

// Enable the wayfinder service menu (upgrade and stuff).
#define ENABLE_WAYFINDER_SERVICE_MENU

// Enable the wayfinder community menu (share wf and stuff).
#define ENABLE_WAYFINDER_COMMUNITY_MENU

// Hide "Services" option in main menu. Replaced by communicate in S60
// and UIQ and by Latest News in Series 80.
#undef HIDE_SERVICES

// If HIDE_SERVICES is defined, then this setting chooses if
// fourth entry on startpage is share wf or communicate.
#undef SHOW_SHARE_WF_ON_STARTPAGE

// If the below setting is defined, a menu entry is added to Extras,
// which calls the call center number.
#undef SHOW_CALL_SUPPORT_IN_EXTRAS_MENU

// Phone number to call if above setting is defined.
/* #define CALL_SUPPORT_NUMBER "0" */

// Always show the buy extension waplink.
#undef ALWAYS_SHOW_BUYEXTENSION
#define ALWAYS_SHOW_BUYEXTENSION

// Don't show the trial view at all.
#undef HIDE_TRIAL_VIEW

// Don't show bitmap choice
#define HIDE_CHANGE_MAPTYPE

// Default is the non-Ilmari style UI
#undef SEED_ILMARI_DEFAULT_UI

// Tracking is not on by default
#define SEED_AUTO_TRACKING

// Show the Move Wayfinder menu option if MoveWayfinder
// is installed
#define SHOW_MOVEWAYFINDER

// Show Acceptance popup when US-map data is available.
#define SHOW_US_POPUP_WARNING

// No signature set by default (each arch needs to set default)
#undef UNSIGNED_PKG_SIGNATURE
#undef SELF_SIGNED_PKG_SIGNATURE
#undef SYMBIAN_SIGNED_SUBMIT_PKG_SIGNATURE

// Uid numbers to use by default if not overridden later
#if defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_UIQ3)
# define WayFinder_uid1 0x1000007A
# define WayFinder_uid2 0x100039ce
#else
# define WayFinder_uid1 0x10000079
# define WayFinder_uid2 0x100039ce
#endif

#if defined NAV2_CLIENT_SERIES60_V3
# define WayFinder_uid3 0x20016434
# define WayFinder_uid3_string "20016434"
#else
# error "Unsupported architecture"
#endif

// Show question to user about automatic accesspoint search
#undef SHOW_CHOOSE_IAP_SEARCH_METHOD

// Use WLan (Wifi) access points in IAP search
#undef USE_WLAN_IN_IAP_SEARCH

// Define which languages should be available to choose from.
#define ALLOWED_LANGS ""

// Do not show languages which we do not have .r01-resource 
// files for by default. (Override if using the installer)
#undef SHOW_LANGS_WITHOUT_RESOURCES

// Show languages even if there is no voice installed
#define SHOW_LANGS_WITHOUT_VOICE

// Define the Share Wayfinder SMS URL.
#define SHARE_WF_SMS_URL "http://adurl.com/"
#define SHOW_SHARE_WF_SMS

// If we want the globe component.
#undef USE_GLOBE

// Define if we want a full grid menu.
#undef FULL_GRID_MENU

// If we want to hide the guide and intinerary views.
#undef HIDE_GUIDE_ITINERARY

//Allow the user to view map and poi information in services window.
#undef HIDE_SERVICES_INFO

// Debug sms, ends SMS on each "error"
#undef SEND_DEBUG_SMS_ON_ERRORS
#define SEND_DEBUG_SMS_NUMBER ""

#undef SEND_REGISTRATION_SMS

// News screen background color
#define NEWS_BACKGROUND_RED 255
#define NEWS_BACKGROUND_GREEN 255
#define NEWS_BACKGROUND_BLUE 255

// News screen constatus position
#define NEWS_CONSTATUS_POS_X 1
#define NEWS_CONSTATUS_POS_Y 1
// News screen constatus position for uiq since it doesnt use layoututils.
#define NEWS_CONSTATUS_UIQ_POS_X 5
#define NEWS_CONSTATUS_UIQ_POS_Y 5

// Check for file to disable hardcoded proxy.
#define TEST_FOR_NO_PROXY_FILE

#define WAYFINDER_MAJOR_LANG_RESOURCE_MINIMUM 4
#ifdef USE_NEW_SLAVE_AUDIO
# define WAYFINDER_MINOR_LANG_RESOURCE_MINIMUM 11
#else
# define WAYFINDER_MINOR_LANG_RESOURCE_MINIMUM 7
#endif


// activation dialog with code and phone number. Normal for Wayfinder.
#define ACTIVATE_WITH_CODE_AND_MSISDN
// activation code with name and email. Normal for Earth.
#undef ACTIVATE_WITH_NAME_AND_EMAIL

// Show question to user about buy extension when an expired account
// error is received.
#define SHOW_BUY_EXTENSION_ON_EXPIRED_USER

// The file to show while waiting for a page to load in service window.
#define WAYFINDER_PLEASE_WAIT_URL "file:///help/please_wait.XX.html"

// Default is to have "Locator/Tracking/Positioning" on as it is only shown
// If server sends POSITIONING right bit.
#define USE_TRACKING

// minimum time between errors. errors that is reported before
// this time has passed will not be displayed. 
#define MIN_TIME_BETWEEN_SHOWN_ERRORS (10*60*1000)
// Time which some errors needs to be repeated within otherwise they
// will not be shown to user.
#define ERROR_MAX_REPEAT_TIME (15*1000)

/* The architecture based part of the client type. Used in the 
 * specific targets below. */
#if defined NAV2_CLIENT_SERIES60_V5
# define NAV2_CLIENT_TYPE_ARCH_BASE "wf-s-60-v5"
#elif defined NAV2_CLIENT_SERIES60_V3
# define NAV2_CLIENT_TYPE_ARCH_BASE "wf-s-60-v3" 
#else
# error "Unsupported architecture"
#endif

#if defined NAV2_CLIENT_SERIES60_V3
# define MAP_CACHE 2
#else
# error "Unsupported architecture"
#endif

#define WAYFINDER_AUDIO_DIR_NAME "wfvoice8"
/* Default audio format */
#if defined NAV2_CLIENT_SERIES60_V3
// Exchange prefix to Vorbis if Vorbis is wanted
// must be compiled in though.
# define WAYFINDER_AUDIO_FORMAT_PREFIX "Vorbis"
# define USE_NEW_SLAVE_AUDIO
#else
# error "Unsupported architecture"
#endif

#define MacroConcat2Helper(a,b) a##b
#define MacroConcat2(a,b) MacroConcat2Helper(a,b)

#define MacroConcat3Helper(a,b,c) a##b##c
#define MacroConcat3(a,b,c) MacroConcat3Helper(a,b,c)

#define MacroConcat4Helper(a,b,c,d) a##b##c##d
#define MacroConcat4(a,b,c,d) MacroConcat4Helper(a,b,c,d)

#define MacroConcat5Helper(a,b,c,d,e) a##b##c##d##e
#define MacroConcat5(a,b,c,d,e) MacroConcat5Helper(a,b,c,d,e)

#define MacroConcat8Helper(a,b,c,d,e,f,g,h) a##b##c##d##e##f##g##h
#define MacroConcat8(a,b,c,d,e,f,g,h) MacroConcat8Helper(a,b,c,d,e,f,g,h)

#define MacroStrHelper(a) #a
#define MacroStr(a) MacroStrHelper(a)

#endif //COMMON_WAYFINDER_SETTINGS_H
