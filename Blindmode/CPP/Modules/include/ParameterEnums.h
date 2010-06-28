/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PARAMETER_ENUMS_H
#define PARAMETER_ENUMS_H
#include "Module.h"
/*
 * This file contains the enums used to identify the internal Nav2
 * parameters.
 *
 */

namespace isab {

   namespace ParameterEnums {
      /**
       * The parameters are divided into classes, where each class
       * also is a multicast group.
       */
      enum ParamClasses { 
         /** No class may have a value lower than this. */
         MinAllowedClass   = Module::MaxAllowedMulticastAddress + 1, /* 0x3a */
         /** This class is only used for testing and development. */
         TestShellClass    = MinAllowedClass, /* 0x3a */
         /** This class holds parameters related to the server communication.*/
         NavServerComClass,     /* 0x3b */
         /** This class holds parameters related to the user interface. */
         UiCtrlClass,           /* 0x3c */
         /**
          * This class holds the parameters the Destination module
          * uses to keep track of favorites.
          */
         DestinationsClass,     /* 0x3d */
         /** This class holds geografic information. */
         RegionDataClass,       /* 0x3e */
         /** 
          * This class holds connection parameters for bluetooth and
          * internet.
          */
         ConnectionClass,       /* 0x3f */
         /** This class holds wayfinder account level information. */
         WayfinderClass,        /* 0x40 */
         /** This class holds information regarding tracking and fleet. */
         TrackingClass,         /* 0x41 */
         /** This class holds information regarding stored routes. */
         RouteClass,            /* 0x42 */
         /** This class holds parameters used by NavTask. */
         NavTaskClass,          /* 0x43 */
      };

//#define PARAM(class, mask, value) ((uint32(class) << 16) | (mask) | (value))

      /**
       * The parameter identifiers are 32 bit numbers divided into bit
       * fields:
       *
       * --------------------------------------------
       * | 1 bit     | 7 bits | 1 bit      | 7 bits |
       * | Multicast | Class  | Persistent | Id     |
       * --------------------------------------------
       *
       * If the multicast bit is set, the parameter is multicasted to
       * all subscribers whenever it is changed.
       *
       * The class is a value from the ParamClasses enum.
       *
       * If the peristence bit is set the parameter is saved to the
       * parameter file and read at program startup.
       *
       * This enum holds the values of the individual parameters,
       * including class, multicast, and persistence bits.
       */
      enum ParamIds {
         /** @name The NavServerComClass parameters. */
         //@{
         /** Holds the Navigatorserver's hostname and port as a
          * string: <host>:<port>. Can be a list of such strings.*/
         NSC_ServerHostname               = uint32(NavServerComClass) << 16 | 0x80008000,
         /** The 32bit number that uniquely id's this navigator.*/
         NSC_NavigatorID                  = uint32(NavServerComClass) << 16 | 0x80008001,
         /** A string[2] parameter with user name and password.  Maybe
          * the password should be encrypted? */
         NSC_UserAndPasswd                = uint32(NavServerComClass) << 16 | 0x80008002,
         /** Three 32bit integers describing the strategy and timeouts
          * of cached connections. These are mostly used when using
          * dialup connections.*/
         NSC_CachedConnectionStrategy     = uint32(NavServerComClass) << 16 | 0x80008003,
         /** Remembers the 'Use crossing maps setting'. 
          * An int32 used as a bool*/
         NSC_AttachCrossingMaps           = uint32(NavServerComClass) << 16 | 0x80008004,
         /** Download crossing maps as images. An int32 used as a bool. */
         NSC_ImageCrossingMaps            = uint32(NavServerComClass) << 16 | 0x80008005,
         /** Image format of crossing maps. An int32 with valid values
          * in the range [0, 255]. */
         NSC_CrossingMapsImageFormat      = uint32(NavServerComClass) << 16 | 0x80008006,
         /** Size in pixels of crossing map images. En int32 where the
          * 16 most significant bits are the width and the 16 lsb 
          * represent the height. */
         NSC_CrossingMapsSize             = uint32(NavServerComClass) << 16 | 0x80008007,
         /** Int32 with valid range [0,255].
          * 
          *  NB! This one has no effect on the search requests to the 
          *      server. Use UC_MaxNumberOfGuiSearchResults instead.
          */
         NSC_MaxSearchMatches             = uint32(NavServerComClass) << 16 | 0x80008008,

         /** String containing the phobe number to the call center. */
         // XXX Deprecated XXX
         NSC_CallCenterMSISDN             = uint32(NavServerComClass) << 16 | 0x80008009, //Deprecated

         /** Language to use in server communications. int32 */
         //          NSC_Language, //renamed to Language in RegionDataClass
         /** Type of transportation for route, e.g. pedestrian, car,... int32*/
         NSC_TransportationType           = uint32(NavServerComClass) << 16 | 0x8000800a,
         /** Optimize route by time, distance, ... int32*/
         NSC_RouteCostType                = uint32(NavServerComClass) << 16 | 0x8000800b,
         /** @name Legacy parameters. */
         //@{
         /** Legacy parameter. Telephone number to call the Navigator Server.*/
         OLD_DataPhoneNbr                 = uint32(NavServerComClass) << 16 | 0x8000800c,
         /** Legacy parameter? Telephone number to callcenter. */
         OLD_VoicePhoneNbr                = uint32(NavServerComClass) << 16 | 0x8000800d,
         //@}

         NSC_CategoriesChecksum           = uint32(NavServerComClass) << 16 | 0x8000800e,
         NSC_LatestNewsChecksum           = uint32(NavServerComClass) << 16 | 0x8000800f,
         /** deprecated */
         NSC_TransactionsLeft             = uint32(NavServerComClass) << 16 | 0x80008010,
         /** deprecated */
         NSC_SubscriptionLeft             = uint32(NavServerComClass) << 16 | 0x80008011,

         /** Deprecated */
         NSC_CallCenterChecksum           = uint32(NavServerComClass) << 16 | 0x80008012, //Deprecated

         /** Sent to the server to identify the client type */
         NSC_ClientType                   = uint32(NavServerComClass) << 16 | 0x80008013,
         /** Sent to the server ??? */
         NSC_ClientOptions                = uint32(NavServerComClass) << 16 | 0x80008014,

         /** Uint32 server list checksum. */
         NSC_ServerListChecksum           = uint32(NavServerComClass) << 16 | 0x80008015,

         /** Avoid or do not avoid toll roads. */
         NSC_RouteTollRoads               = uint32(NavServerComClass) << 16 | 0x80008016,
         
         /**
          * This is a fishy parameter. It holds all kinds of account
          * resource limits. The GuiProtEnums::ExpireVectorIndex lets
          * you access the different items in a named way.
          */
         NSC_ExpireVector                 = uint32(NavServerComClass) << 16 | 0x80008017,
         /** The route request to the server can indicate the approximate
          * length of the route expressed in the time it is expected to 
          * take to drive. This allows traffic disturbances etc to change
          * the route after the initial reroute.
          */
         NSC_RequestedRouteTime           = uint32(NavServerComClass) << 16 | 0x80008018,

         /** Avoid or do not avoid highways. */
         NSC_RouteHighways                = uint32(NavServerComClass) << 16 | 0x80008019,

         /** 
          * Last IMEI used in successful server communication. 
          * Binary block. 
          */
         NSC_LastIMEI                     = uint32(NavServerComClass) << 16 | 0x8000801a,

         /** 
          * Bob stored by server in client to authenticate user on 
          * secondary server. Binary block.
          */
         NSC_serverAuthBob                = uint32(NavServerComClass) << 16 | 0x8000801b,

         /**
          * Checksum for server auth bob. Uint32.
          */
         NSC_serverAuthBobChecksum        = uint32(NavServerComClass) << 16 | 0x8000801c,

         /**
          * The HTTP talking server's hostname and port as a
          * string: <host>:<port>. Can be a list of such strings.
          */
         NSC_HttpServerHostname           = uint32(NavServerComClass) << 16 | 0x8000801d,
         /** Uint32 Http server list checksum. */
         NSC_HttpServerListChecksum       = uint32(NavServerComClass) << 16 | 0x8000801e,
         /**
          * Uint32 array with the user's rights, see 
          * GuiProtEnums::userRights.
          */
         NSC_userRights                   = uint32(NavServerComClass) << 16 | 0x8000801f,

         /**
          * String with the new program,resource,mlfw version from server.
          */
         NSC_newVersion                   = uint32(NavServerComClass) << 16 | 0x80008020,
         //@}

         /** @name The TestShellClass parameters. */
         //@{
         TS_Param1 = uint32(TestShellClass | 0x8000) << 16,
         TS_Param2,
         TS_Param3 = uint32(TestShellClass         ) << 16,
         TS_Param4,   /* Dummy, never used! */
         //@}

         /** @name The UiCtrlClass parameters. */
         //@{
         /**
          * This parameter is sent in the search requests to the server and 
          * determines how many search results that are returned at most.
          */
         UC_MaxNumberOfGuiSearchResults = uint32(UiCtrlClass)   << 16 | 0x80008000,
         /**
          * Number in aproximately percentage, i.e. 0 to 99. Determines how loud 
          * the GUI plays the sounds.
          */
         UC_SoundVolume                 = uint32(UiCtrlClass)   << 16 | 0x80008001,
         /**
          * Bool. Determines whether the main speaker of the GUI device is used 
          * or not. If this prarameter is set to false, any headset speaker
          * or similar may still be used.
          */
         UC_UseMainSpeaker              = uint32(UiCtrlClass)   << 16 | 0x80008002,
         /**
          * Bool. Determines if a reroute request is sent directly when getting 
          * of track, without any user action.
          */
         UC_AutoReroute                 = uint32(UiCtrlClass)   << 16 | 0x80008003,
         /**
          * Char* array. Numbers possible to choose from when initiating a 
          * call to the callcenter.
          *
          * For now at most 8 numbers should be stored in this one.
          */
         // XXX Deprecated XXX 
         UC_CallCenterNumbers           = uint32(UiCtrlClass)   << 16 | 0x80008004, //Deprecated

         /**
          * Enum GuiProtEnums::YesNoAsk. Determines if the GUI should store
          * destinations arriving in SMS in My Destinations automatically, 
          * automatically throw them away or ask the user what to do.
          */
         UC_StoreSMSDestInMyDest        = uint32(UiCtrlClass)   << 16 | 0x80008005,

         /**
          * Bool. Determines if the GUI should make a route request automatically 
          * when a destination SMS arrives.
          */
         UC_AutomaticRouteOnSMSDest     = uint32(UiCtrlClass)   << 16 | 0x80008006,

         /**
          * Enum GuiProtEnums::YesNoAsk. Determines what the GUI should do
          * with destination SMSs in the SMS inbox of the device.
          */
         UC_KeepSMSDestInInbox        = uint32(UiCtrlClass)     << 16 | 0x80008007,
         /**
          * uint8 strategy. Determines if and when the GUI should turn
          * of the backlight to save power.
          */
         UC_BacklightStrategy         = uint32(UiCtrlClass)     << 16 | 0x80008008,

         /**
          * Enum GuiProtEnums::YesNoAsk. Determines if the GUI should
          * use left side traffic pictures.
          */
         UC_LeftSideTraffic           = uint32(UiCtrlClass)     << 16 | 0x80008009,
         /**
          * Search strings for GUI.
          * Last city string and search string.
          * Will later be search history...
          */
         UC_GUISearchStrings          = uint32(UiCtrlClass)     << 16 | 0x8000800a,
         /**
          * Search country for GUI.
          * Last country id and name
          * Will later be part of search history...
          */
         UC_GUISearchCountryBlob      = uint32(UiCtrlClass)     << 16 | 0x8000800b,
         /**
          * Position Select Data for GUI.
          * Data on origin and destination items.
          */
         UC_GUIPositionSelectDataBlob = uint32(UiCtrlClass)     << 16 | 0x8000800c,
         /**
          * MyDest view current index for GUI.
          * Simple parameter for current index in MyDest list.
          */
         UC_GUIMyDestCurrentSelectedIndex = uint32(UiCtrlClass) << 16 | 0x8000800d,
         /**
          * GUI last known destination.
          * String and coordinates.
          */
         UC_GUILastKnownRouteEndPoints= uint32(UiCtrlClass)     << 16 | 0x8000800e,
         /**
          * GUI last known route
          */
         UC_GUILastKnownRouteId       = uint32(UiCtrlClass)     << 16 | 0x8000800f,

         /** Username for webpage. */
         UC_WebUser                   = uint32(UiCtrlClass)     << 16 | 0x80008010,
         /** Password for webpage. non persistent */
         UC_WebPasswd                 = uint32(UiCtrlClass)     << 16 | 0x80000002,

         /** Mute turn sounds when phone call is detected.*/
         UC_MuteTurnSounds            = uint32(UiCtrlClass)     << 16 | 0x80008011,
         /** Which units are to be used in distances. */
         UC_DistanceMode              = uint32(UiCtrlClass)     << 16 | 0x80008012,
         /** Array of vector map settings. */
         UC_VectorMapSettings         = uint32(UiCtrlClass)     << 16 | 0x80008013,
         /** Gui mode setting (Classic or Map as main view) */
         UC_GuiMode                   = uint32(UiCtrlClass)     << 16 | 0x80008014,
         UC_FavoriteShow              = uint32(UiCtrlClass)     << 16 | 0x80008015,
         UC_GPSAutoConnect            = uint32(UiCtrlClass)     << 16 | 0x80008016,
         UC_PoiCategories             = uint32(UiCtrlClass)     << 16 | 0x80008017,
         UC_VectorMapCoordinates      = uint32(UiCtrlClass)     << 16 | 0x80008018,
         UC_ChooseHomeServer          = uint32(UiCtrlClass)     << 16 | 0x80008019,
         UC_AutoTracking              = uint32(UiCtrlClass)     << 16 | 0x8000801a,
         
         /** Setting for which symbol to show as position */
         UC_PositionSymbolType        = uint32(UiCtrlClass)     << 16 | 0x8000801b,

         /** Setting for Maplayer settings. */
         UC_MapLayerSettings          = uint32(UiCtrlClass)     << 16 | 0x8000801c,
         /**
          * Boolean parameter that tells the UI whether the Internet
          * Link Layer (GPRS) should be kept alive or not.
          */
         UC_LinkLayerKeepAlive        = uint32(UiCtrlClass)     << 16 | 0x8000801d,
         /**
          * Boolean parameter that tells the UI whether the 
          * user terms has been accepted.
          */
         UC_UserTermsAccepted         = uint32(UiCtrlClass)     << 16 | 0x8000801e,

         /**
          * Integer parameter that tells the UI which latest news
          * has been shown.
          */
         UC_LatestShownNewsChecksum   = uint32(UiCtrlClass)     << 16 | 0x8000801f,

         /**
          * Set how talkative the navigator should be.
          */
         UC_TurnSoundsLevel           = uint32(UiCtrlClass)     << 16 | 0x80008020,

          /**
          * Integer parameter that tells the UI which latest direction
          * type to use (clock based or direction based)
          */
         UC_DirectionType              = uint32(UiCtrlClass)    << 16 | 0x80008021,

         /**
          * Set how verbose the vicinity feed should be.
          */
         UC_FeedOutputFormat           = uint32(UiCtrlClass)    << 16 | 0x80008022,

         /**
          * Set how long vicinity feed should wait between updates
          */

         UC_FeedWaitPeriod             = uint32(UiCtrlClass)    << 16 | 0x80008023,

         /**
          * Set how long vicinity feed should wait between updates
          */

         UC_LockedNavWaitPeriod        = uint32(UiCtrlClass)    << 16 | 0x80008024,

         /**
          * Set minimum usable walking speed
          */

         UC_GpsMinWalkingSpeed         = uint32(UiCtrlClass)    << 16 | 0x80008025,

         /**
          * Set update period for the vicinity lists.
          */ 
         
         UC_VicinityListUpdatePeriod   = uint32(UiCtrlClass)    << 16 | 0x80008026,
         
         //@}

         /** @name The DestinationClass parameters. */
         //@{
         /**
          * Blob for storing favorites in the client device.
          * Deprecated parameter, use Dest_StoredFavoritesVer2
          */
         Dest_StoredFavorites =       uint32(DestinationsClass) << 16 | 0x00008000,
         /**
          * Enum (GuiProtEnums::SortingType) deciding sorting of favorites.
          */
         Dest_FavoritesSortingOrder = uint32(DestinationsClass) << 16 | 0x00008001,

         /**
          * Blob of locally deleted favorites.
          */
         Dest_DeletedFavorites = uint32(DestinationsClass) << 16 | 0x00008002,

         /**
          * Blob for storing favorites in the client device.
          * Superceds Dest_StoredFavorites
          * Deprecated parameter, use Dest_StoredFavoritesVer3
          */
         Dest_StoredFavoritesVer2 = uint32(DestinationsClass) << 16 | 0x00008003,
         /**
          * Uint32 array for supported special favorites types.
          * See GuiProtEnums::AdditionalInfoType.
          */
         Dest_SupportedSpecialFavorites = uint32(DestinationsClass) << 16 | 0x00008004,
         /**

          * Blob for storing favorites in the client device.
          * Superceds Dest_StoredFavorites2
          */
         Dest_StoredFavoritesVer3 = uint32(DestinationsClass) << 16 | 0x00008005,
         //@}
         
         /** @name The RegionDataClass parameters. */
         //@{
         /** Hold the top region list, serialized to a binary block. Legacy (non UTF8) */
         TopRegionListLegacy = (RegionDataClass << 16) | 0x80008000,
         /** Language to use in server communications. int32 */
         Language       = (RegionDataClass << 16) | 0x80008001,
         /** The ids of stored categories */
         CategoryIds    = (RegionDataClass << 16) | 0x80008002,
         /** The names of stored categories.*/
         CategoryNames  = (RegionDataClass << 16) | 0x80008003,
         /** Hold the top region list, serialized to a binary block.*/
         TopRegionList       = (RegionDataClass << 16) | 0x80008004,
         //@}

         /** @name The ConnectionClass parameters. */
         //@{
         /** 
          * The last used GPS bluetooth address. Don't now if this has
          * ever been used. 
          */
         BT_GPS_Address = uint32(ConnectionClass) << 16 | 0x00008000,
         /**
          * Three strings, address split into high and low 32 bits (as
          * a hexadecimal number starting with 0x) and name of the
          * last used btgps.
          * Ex: 0x00001234,0x56789abc,Filur
          */
         BtGpsAddressAndName = (ConnectionClass << 16) | 0x00008001,
         /**
          * uint32. The user selected IAP or 0 to always ask.
          */
         SelectedAccessPointId = uint32(ConnectionClass) << 16 | 0x80008001,
         /**
          * uint32 changed to string 20050218 by Daniel. 
          * The IAP for this session.
          * Non persistent parameter.
          */
         SelectedAccessPointId2  = uint32(ConnectionClass) << 16 | 0x80000002,
         
         /**
          * Real accesspoint.
          * This contains all access points keyed by IMEI number.
          */
         SelectedAccessPointIdReal = uint32(ConnectionClass) << 16 | 0x80008003,
         //@}

         /** @name The WayfinderClass parameters. */
         //@{
         /**
          *  This parameter should hold the wayfinder type (trial,
          *  silver, gold, iron).
          */
         WayfinderType = uint32(WayfinderClass) << 16 | 0x80008000,
         //@}

         /** @name The TrackingClass parameters. */
         //@{
         /** uint32 The Client Tracking level see TrackingLevel_t */
         TR_trackLevel          = uint32(TrackingClass) << 16 | 0x80008000,


         /** BLOB The stored tracking data not yet sent to server. */
         TR_trackdata           = uint32(TrackingClass) << 16 | 0x80008001,
         

         /**
          * The PIN(s) for Client Tracking.
          * First uint32 CRC. Then an
          * array of uint32,string,string (id, PIN, Comment). If highest
          * bit set in id PIN is deleted. If id == 0 then locally added
          * PIN not yet on server.
          */
         TR_trackPIN            = uint32(TrackingClass) << 16 | 0x80008002,
         //@}

         /** @name The RouteClass parameters. */
         //@{
         /** uint32 that holds the last selected route in the file list. */
         R_lastIndex            = uint32(RouteClass) << 16 | 0x80008000,

         /** String containing the last entered route name. */
         R_routeName            = uint32(RouteClass) << 16 | 0x80008001,
         //@}

         /** @name The NavTaskClass parameters. */
         //@{
         /**
          * Integer parameter that sets the interval period of traffic
          * info reroutes. This parameter is user controlled.  The
          * period is measured in minutes.
          */
         NT_UserTrafficUpdatePeriod   = uint32(NavTaskClass) << 16 | 0x80008000,
         /**
          * Integer parameter that sets the interval period of traffic
          * info reroutes. This parameter is server controlled.  The
          * period is measured in minutes.
          */
         NT_ServerTrafficUpdatePeriod = uint32(NavTaskClass) << 16 | 0x80008001,
         //@}
      };
   }
} /* namespace isab */
#endif
