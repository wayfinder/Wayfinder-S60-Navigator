/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PARAMETER_ENUMS_H
#define GUI_PARAMETER_ENUMS_H

namespace isab {

   namespace GuiProtEnums{

      enum ParameterType {
         // Parameters marked with type_and_data below have 
         // specific classes that are subclasses to GuiProtMess,
         // which defines the shape of the data. 

         // GuiProtMess class = GetTopRegionReplyMess.
         paramTopRegionList                        = 0x0,

         // paramServerNameAndPort  The server name and
         //                         port in the same string
         //                         separated by colon.
         paramServerNameAndPort                    = 0x1,

         // int32 volumePercent Value between decimal 0
         //                     and 99 giving apriximately 
         //                     the volume percentage.
         paramSoundVolume                          = 0x2,

         // int32 useSpeaker True if the speaker should be
         //                 used, false otherwise.
         //                 NB! This only affects the main
         //                     speaker(s). Any hansfree
         //                     or similar will still give
         //                     sound.
         // 
         //                     To mute all sound use
         //                     paramSoundVolume = 0 instead.
         paramUseSpeaker                           = 0x3,

         // int32 autoReroute    This makes Nav2 initiate a
         //                     reroute when getting off-
         //                     track, without any interaction
         //                     from the user.
         paramAutoReroute                          = 0x4,

         // int32
         // NavServerComEnums::VehicleType transportationType
         //                     Determines what roads that are
         //                     used when creating a route.
         paramTransportationType                   = 0x5,

         // int32
         // DistancePrintingPolicy::DistanceMode distanceMode
         //                     Determines which units are
         //                     used for presentation of distances.
         paramDistanceMode                         = 0x6,

         // int32
         // NavServerComEnums::TollRoads tollRoads
         //                     Determines if toll roads are
         //                     penalized.
         paramTollRoads                            = 0x7,

         // int32
         // NavServerComEnums::Highways highways
         //                     Determines if highways are
         //                     penalized.
         paramHighways                             = 0x8,

         // int32
         // NavServerComEnums::RouteOptimizedFor timeDist
         //                     Determines if the route module
         //                     strives for shortest time or
         //                     shortest distance.
         paramTimeDist                             = 0x9,

         // type_and_string
         // char* userName      The user name used in the 
         //                     communication with the server.
/*          paramUserName                             = 0xa, */

         // XXX Deprecated XXX
/*          paramCallCenterNumbers                    = 0xb, //Deprecated */

         // int32 maxNbrSearchResults. Value between 0 and 255.
         paramMaxNbrOfSearchResults                = 0xc,

         // int32
         // GuiProtEnums::YesNoAsk strategy.
         //               Determines if destinations are
         //               stored automatically or if the
         //               user is asked what to do.
         paramStoreSMSDestInMyDest                 = 0xd,

         // int32
         // bool automaticRoute
         paramAutomaticRouteOnSMSDest              = 0xe,

         // int32
         // GuiProtEnums::YesNoAsk strategy.
         //               Determines if SMS destinatios
         //               are keept in the inbox or 
         //               something else happends.
         paramKeepSMSDestInInbox                   = 0xf,

         // Currently not sent to GUI.
         paramLanguage                             = 0x10,

         // int32
         // uint8 backlight strategy.
         //               Determines how and when the GUI wil
         //               shutdown the backlight.
         paramBacklightStrategy                    = 0x11,

         // int32
         // GuiProtEnums::YesNoAsk strategy.
         //               Determines if the images should be shown
         //               for left side traffic or not.
/*          paramLeftSideTraffic                      = 0x12, */

         paramWebUsername                         = 0x13,
         paramWebPassword                         = 0x14,

/*          paramUsername                             = 0x15, */
/*          paramPassword                             = 0x16, */

         paramCategoryIds                          = 0x17,
         paramCategoryNames                        = 0x18,
         paramLatestNewsImage                      = 0x19,
         paramCategoryIcons                        = 0x1a,
         paramCategoryIntIds                       = 0x1b,
/*          paramLatestNewsChecksum                   = 0x1a, */

         // XXX Deprecated XXX
/*          paramServerCallCenters                   = 0x1b, //Deprecated */

         paramTimeLeft                             = 0x1c,

         paramSelectedAccessPointId                = 0x1d,
         paramSelectedAccessPointId2               = 0x1e,

         /* Pure GUI parameters. */
         paramSearchStrings                        = 0x1f,
/*          paramSearchCountry                        = 0x20, */
/*          paramPositionSelectData                   = 0x21, */
/*          paramMyDestIndex                          = 0x22, */
/*          paramLastKnownRouteEndPoints              = 0x23, */
         paramLastKnownRouteId                     = 0x24,

         //trial, silver, gold
         paramWayfinderType                        = 0x25,

         // Mute turn sounds during phone calls.
/*          paramMuteTurnSound                        = 0x26, */

         /**
          * Vector map settings.
          */
         paramVectorMapSettings                    = 0x27,
         /**
          * Gui mode setting
          */
/*          paramGuiMode                              = 0x28, */
         /**
          * Show favorites in map
          */
         paramFavoriteShow                         = 0x29,
         /**
          * Connect automatically to GPS?
          */
         paramGPSAutoConnect                       = 0x2a,

         /**
          * Binary blob which contains the tile categories settings.
          */
         paramPoiCategories                        = 0x2b,

         paramDemoMode                             = 0x2c,

         /**
          * Binary blob which contains the positions for the last vectormap.
          */
         paramVectorMapCoordinates                 = 0x2d,

/*          paramChooseHomeServer                     = 0x2e, */
         paramAutoTracking                         = 0x2f,

         /**
          * What symbol to draw at the current position
          */
         paramPositionSymbol                       = 0x30,


         /// The level of client tracking. See TrackingLevel_t enum.
         paramTrackingLevel                        = 0x31,


         /**
          * The PIN(s), list of strings. used to access client tracking 
          * log.
          */
         paramTrackingPIN                          = 0x32,
         ///BTGPS bt-address and bt-name. 
         paramBtGpsAddressAndName                  = 0x33,

         /**
          * Binary blob which contains the tile map layers settings.
          */
         paramMapLayerSettings                     = 0x34,

         /**
          * The User's rights in an uint32 array.
          */
         userRights                                = 0x35,

         /**
          * The UC_LinkLayerKeepAlive setting.
          */
         paramLinkLayerKeepAlive                   = 0x36,

         /**
          * User has accepted User terms.
          */
         paramUserTermsAccepted                    = 0x37,

/*          paramLatestShownNewsChecksum              = 0x38, */

         /**
          * Set how talkative the navigator should be
          */
         paramTurnSoundsLevel                      = 0x39,

         /**
          * Update time for route traffic update.
          */
         userTrafficUpdatePeriod                   = 0x40,

         /**
          * New combined parameter for Username and password
          * (only used for debug)
          */
         paramUserAndPassword                      = 0x41,

         /**
          * paramHttpServerNameAndPort  Http server name and port list
          */
         paramHttpServerNameAndPort                = 0x42,

         /**
          * Latest News Id from server.
          */
         paramShowNewsServerString                 = 0x43,

         /**
          * Last shown Latest News Id in Gui.
          */
         paramShownNewsChecksum                    = 0x44,

         /**
          * Value to indicate that US disclaimer should never be shown.
          * (Even if user has US states in top region list.
          */
         paramNeverShowUSDisclaimer                = 0x45,

         /**
          * Flag saying if user has sent registration sms or not.
          */
         paramRegistrationSmsSent                  = 0x46,

         /**
          * Flag saying if acp is enabled or disabled.
          */ 
         paramMapACPSetting                        = 0x47,

         /**
          * True if automatically checking for updates
          */
         paramCheckForUpdates                      = 0x48,

         /**
          * New version value.
          */ 
         paramNewVersion                           = 0x49,

         /**
          * New version url.
          */ 
         paramNewVersionUrl                        = 0x50,

         ///invalid parameter.
         paramInvalid                              = 0xffff, //MAX_UINT16
      };

      class ParameterTypeHolder {
      public:
         inline ParameterTypeHolder( GuiProtEnums::ParameterType val )
            : m_enum( val ) {}
         inline operator GuiProtEnums::ParameterType() const { return m_enum; }
      private:
         GuiProtEnums::ParameterType m_enum;
      };
   } // namespace GuiProtEnums.

} // namespace isab

#endif

