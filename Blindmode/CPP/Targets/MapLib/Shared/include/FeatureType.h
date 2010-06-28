/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FEATURE_TYPE
#define FEATURE_TYPE

namespace FeatureType {
   /**
    *   Primitives have negative values here and the non-primitives
    *   should not be in the code.
    */
   enum tileFeature_t {
      // Erroneous id
      nonexisting = -5,
      /// Circle primitive.
      circle = -4,
      /// Bitmap primitive.
      bitmap = -3,
      /// Builtin primitive for polygons (filled)
      polygon = -2,
      /// Builtin primitive for lines
      line    = -1,
      /// Forbidden numbers!!! These should not be hard-coded.
      street_class_0 = 0,
      street_class_1 = 1,
      street_class_2 = 2,
      street_class_3 = 3,
      street_class_4 = 4,
      street_class_0_level_0 = 5,
      street_class_1_level_0 = 6,
      street_class_2_level_0 = 7,
      street_class_3_level_0 = 8,
      street_class_4_level_0 = 9,
      poi = 10,
      water = 11,
      park = 12,
      land = 13,
      building_area = 14,
      building_outline = 15,
      bua = 16,
      railway = 17,
      ocean = 18,
      route = 19,
      route_origin = 20,
      route_destination = 21,
      route_park_car = 22,
      // Here comes the POI:s.
      city_centre_2 = 23, 
      city_centre_4 = 24, 
      city_centre_5 = 25, 
      city_centre_7 = 26, 
      city_centre_8 = 27, 
      city_centre_10 = 28, 
      city_centre_11 = 29, 
      city_centre_12 = 30, 
      atm = 31,
      golf_course = 32,
      ice_skating_rink = 33,
      marina = 34,
      vehicle_repair_facility = 35,
      bowling_centre = 36,
      bank = 37,
      casino = 38,
      city_hall = 39,
      commuter_railstation = 40,
      courthouse = 41,
      historical_monument = 42,
      museum = 43, 
      nightlife = 44,
      post_office = 45,
      recreation_facility = 46,
      rent_a_car_facility = 47,
      rest_area = 48,
      ski_resort = 49,
      sports_activity = 50,
      theatre = 51,
      tourist_attraction = 52,
      university = 53,
      winery = 54,
      parking_garage = 55,
      park_and_ride = 56,
      open_parking_area = 57,
      amusement_park = 58,
      library = 59,
      school = 60, 
      grocery_store = 61, 
      petrol_station = 62,
      tram_station = 63,
      ferry_terminal = 64,
      cinema = 65,
      bus_station = 66,
      railway_station = 67,
      airport = 68,
      restaurant = 69,
      hotel = 70,
      tourist_office = 71,
      police_station = 72,
      hospital = 73,
      toll_road = 74,
      nbr_tile_features // Last one
   };
}

#endif
