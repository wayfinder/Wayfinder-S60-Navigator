/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PROT_HELPER_ENUMS_H
#define GUI_PROT_HELPER_ENUMS_H

namespace isab {

#define MIN_that_can(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX_that_can(a, b)  (((a) > (b)) ? (a) : (b))

   namespace GuiProtEnums {

      enum SortingType {
         alphabeticalOnName                      = 0x00,
         distance                                = 0x01,
         newSort                                 = 0x02,

         invalidSortingType                      = 0xffff, //MAX_UINT16
      }; // enum SortingType

      enum YesNoAsk {
         yes                                     = 0x00,
         no                                      = 0x01,
         ask                                     = 0x02,
         invalidYesNoAsk                         = 0x03,
      }; // enum SMSInboxStrategy

      enum BacklightStrategy {
         backlight_always_off                    = 0x00,
         backlight_on_during_route               = 0x01,
         backlight_always_on                     = 0x02,
         backlight_near_action                   = 0x03,
         backlight_invalid                       = 0xff,
      };

      enum TurnSoundsLevel {
         turnsound_mute                         = 0x00,
         turnsound_min                          = 0x01,
         turnsound_less                         = 0x02,
         turnsound_normal                       = 0x03,
         turnsound_more                         = 0x04,
         turnsound_max                          = 0x05,
         turnsound_invalid                      = 0xff
      };

      enum ParamType {
         paramTypeInvalid  =  0,
         paramTypeInt32    =  1,
         paramTypeFloat    =  2,
         paramTypeString   =  3,
         paramTypeBinary   =  4,
      };

      enum RegionType{
         invalid      = 0x0,
         streetNumber = 0x1,
         address      = 0x2,
         cityPart     = 0x3,
         city         = 0x4,
         municipal    = 0x5,
         county       = 0x6,
         state        = 0x7,
         country      = 0x8,
         zipcode      = 0x9,
         zipArea      = 0xa,
      };

      enum AdditionalInfoType{
         dont_show    = 0x00,
         dontShow     = 0x00, /* Compat. */
         text         = 0x01,
         url          = 0x02,
         wap_url      = 0x03,
         email        = 0x04,
         phone_number = 0x05,
         mobile_phone = 0x06,
         fax_number   = 0x07,
         contact_info = 0x08,
         short_info   = 0x09,
         vis_address  = 0x0a,
         vis_house_nbr= 0x0b,
         vis_zip_code = 0x0c,
         vis_complete_zip=0x0d,
         Vis_zip_area = 0x0e,
         vis_full_address=0x0f,
         brandname    = 0x10,
         short_description=0x11,
         long_description=0x12,
         citypart     = 0x13,
         ad_info_state= 0x14,
         neighborhood = 0x15,
         open_hours   = 0x16,
         nearest_train= 0x17,
         start_date   = 0x18,
         end_date     = 0x19,
         start_time   = 0x1a,
         end_time     = 0x1b,
         accommodation_type=0x1c,
         check_in     = 0x1d,
         check_out    = 0x1e,
         nbr_of_rooms = 0x1f,
         single_room_from=0x20,
         double_room_from=0x21,
         triple_room_from=0x22,
         suite_from   = 0x23,
         extra_bed_from=0x24,
         weekend_rate = 0x25,
         nonhotel_cost= 0x26,
         breakfast    = 0x27,
         hotel_services=0x28,
         credit_card  = 0x29,
         special_feature=0x2a,
         conferences  = 0x2b,
         average_cost = 0x2c,
         booking_advisable=0x2d,
         admission_charge=0x2e,
         home_delivery= 0x2f,
         disabled_access=0x30,
         takeaway_available=0x31,
         allowed_to_bring_alcohol=0x32,
         type_food    = 0x33,
         decor        = 0x34,
         image_url    = 0x35,
         supplier     = 0x36,
         owner        = 0x37,
         price_petrol_superplus = 0x38,
         price_petrol_super = 0x39,
         price_petrol_normal = 0x3a,
         price_diesel = 0x3b,
         price_biodiesel = 0x3c,
         free_of_charge = 0x3d,
         tracking_data = 0x3e,
         post_address = 0x3f,
         post_zip_area = 0x40,
         post_zip_code = 0x41,
         open_for_season = 0x42,
         ski_mountain_min_max_height = 0x43,
         snow_depth_valley_mountain = 0x44,
         snow_quality = 0x45,
         lifts_open_total = 0x46,
         ski_slopes_open_total = 0x47,
         cross_country_skiing_km = 0x48,
         glacier_area = 0x49,
         last_snowfall = 0x4a,
         special_flag = 0x4b,
         
         more         = 0xff
      };


      enum DemoMode {
         NoDemo, 
         NoServerDemo, 
         OneRouteDemo,
         PlaybackDemo
      };

      enum ShowFavoriteInMap {
         ShowFavoriteInMapAlways                =  0x00,
         ShowFavoriteInMapCityLevel             =  0x01,
         ShowFavoriteInMapNever                 =  0x02,
      };
   }


   class YesNoAskHolder {
   public:
      inline YesNoAskHolder( GuiProtEnums::YesNoAsk val ) : m_enum( val ) {}
      inline operator GuiProtEnums::YesNoAsk() const { return m_enum; }
   private:
      GuiProtEnums::YesNoAsk m_enum;
   };
}

/**
 * This enum contains indexes into the settings vector in a
 * paramVectorMapSetting message. This enum is deliberately left
 * outside the isab and GuiProtEnums namespaces.
 */
enum vmap_set_positions {
   /** Settings vector version. */
   vmap_set_version_pos       = 0,
   /** cache size */
   vmap_set_cache_pos         = 1, 
   /** vector or image*/
   vmap_set_maptype_pos       = 2,
   /** north or heading*/
   vmap_set_orientation_pos   = 3,
   /** show fav in scale*/
   vmap_set_favorite_show_pos = 4,
   /** go to guide or map when route arrives.*/
   vmap_set_guide_mode_pos    = 5,
   /** map or classic */
   vmap_set_gui_mode_pos      = 6,
   /** size of settings vector. */
   vector_map_settings_num    = 7,
};

/**
 * This enum contains the valid values of the vmap_set_guide_mode_pos
 * position in the paramVectorMapSetting parameter.
 */
enum preferred_guide_mode {
   /** Go to guide when a new route arrives. */
   preferred_guide            = 0, 
   /** Go to map when a new route arrives. */
   preferred_map              = 1, 
   /**
    * Use the most recently used navigation view when a new route
    * arrives.
    */
   preferred_automatic = 2
};

#endif

