/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef DISTANCEPRINTINGPOLICY_H
#define DISTANCEPRINTINGPOLICY_H

#include "arch.h"

namespace isab {

class DistancePrintingPolicy
{
public:
   enum DistanceMode {
      ModeInvalid = 0,
      ModeMetric = 1,
      ModeImperialYards = 2,
      ModeImperialFeet = 3,
      ModeMetricSpace = 4,
      ModeImperialYardsSpace = 5,
      ModeImperialFeetSpace = 6,
   };

   enum DistanceRound {
      Exact = 0,
      Round = 1,
   };
   
   /**
    * Helper class to contain a distance and a unit string separated.
    */
   class DistanceAndUnit {
   public:
      char* m_distance;
      char* m_unit;
      DistanceAndUnit() {
         m_distance = new char[15];
         m_unit = new char[15];
      }
      ~DistanceAndUnit() {
         if(m_distance) { delete m_distance;}
         if(m_unit) { delete m_unit;}
      }
      /**
       * Concatenates the distance and unit into a string with or without a space in between. 
       * @param useSpace true if there should be a space between, false otherwise.
       * @return The new concatenated string. Caller takes ownership.
       */
      char* Concatenated(bool useSpace) {
         char* concatenated = new char[20];
         if(useSpace) {
            sprintf(concatenated, "%s %s", m_distance, m_unit);
         } else {
            sprintf(concatenated, "%s%s", m_distance, m_unit);
         }
         return concatenated;
      }
   };

   /**
    * Rounds the distance into an even multiple of the base.
    * @return The rounded distance.
    */
   static uint32 roundTo(uint32 distance, uint32 base);
   /**
    * Get the decimal digit of the distance divided by 1000.
    * Only one decimal is returned. The routine is intended
    * for use with the get1000() function.
    * @return The decimal digit. (Value 0 - 9)
    */
   static uint32 get1000Decimal(uint32 distance);
   /**
    * Get the integer part of the distance divided by 1000.
    * The result is rounded to nearest 100, ie 999 = 1,
    * but 930 = 0. The routine is intended to be used in
    * combination with get1000Decimal().
    * @return The integer part divided by 1000.
    */
   static uint32 get1000(uint32 distance);
   /**
    * Convert meters into feet.
    * @return feet
    */
   static uint32 meterToFeet(uint32 meters);
   /**
    * Convert meters into yards.
    * @return yards
    */
   static uint32 meterToYards(uint32 meters);
   /**
    * Convert the value into miles and return the value
    * rounded to whole miles.
    * @return Whole miles.
    */
   static uint32 getMilesWhole(uint32 meters);
   /**
    * Convert the distance into miles and return the first
    * decimal rounded to 0.1 miles. The routine is intended
    * to be used in combination with getMilesIntegerPart().
    * @return decimal part of miles.
    */
   static uint32 getMilesDecimal(uint32 meters);
   /**
    * Convert the distance into miles and return the integer
    * part of the value rounded to 0.1 miles. The routine
    * is intended * to be used in combination with getMilesDecimal().
    * @return integer part of miles.
    */
   static uint32 getMilesIntegerPart(uint32 meters);
   /**
    * Convert the value into miles.
    * @return miles as a float.
    */
   static float getMiles(uint32 meters);
   /**
    * Convert the given altitude into a fittingly formatted
    * string using the units indicated by the mode parameter.
    * The round parameter is used to indicate if the distance
    * should be rounded to a natural number (only used for
    * smaller distances, ie 343 feet = 345 feet etc)
    * @return formatted string with distance and unit.
    */
   static char * convertAltitude(uint32 altitude,
         DistancePrintingPolicy::DistanceMode mode,
         DistancePrintingPolicy::DistanceRound round = DistancePrintingPolicy::Exact);
   /**
    * Convert the given distance into a fittingly formatted
    * string using the units indicated by the mode parameter.
    * The round parameter is used to indicate if the distance
    * should be rounded to a natural number (only used for
    * smaller distances, ie 343 feet = 345 feet etc)
    * @return formatted string with distance and unit.
    */
   static char * convertDistance(uint32 distance,
         DistancePrintingPolicy::DistanceMode mode,
         DistancePrintingPolicy::DistanceRound round = DistancePrintingPolicy::Exact);
   /**
    * Convert the given distance into a DistanceAndUnit
    * object using the units indicated by the mode parameter.
    * The round parameter is used to indicate if the distance
    * should be rounded to a natural number (only used for
    * smaller distances, ie 343 feet = 345 feet etc)
    * @return a DistanceAndUnit object containing the distance and unit string.
    */
   static DistanceAndUnit* convertDistanceAndUnit(uint32 distance,
         DistancePrintingPolicy::DistanceMode mode,
         DistancePrintingPolicy::DistanceRound round = DistancePrintingPolicy::Exact);
   /**
    * Convert the given distance into feet and miles.
    * Used by convertDistance().
    * @return formatted string.
    */
   static char * convertDistanceImperialFeet(uint32 distance,
         DistancePrintingPolicy::DistanceRound round,
         bool useSpace = false);
   /**
    * Convert the given distance into feet and miles.
    * Used by convertDistance().
    * @return a DistanceAndUnit object containing the distance and unit string.
    */
   static DistanceAndUnit* convertDistanceAndUnitImperialFeet(uint32 distance,
         DistancePrintingPolicy::DistanceRound round);
   /**
    * Convert the given distance into yards and miles.
    * Used by convertDistance().
    * @return formatted string.
    */
   static char* convertDistanceImperialYards(uint32 distance,
         DistancePrintingPolicy::DistanceRound round,
         bool useSpace = false);
   /**
    * Convert the given distance into yards and miles.
    * Used by convertDistance().
    * @return a DistanceAndUnit object containing the distance and unit string.
    */
   static DistanceAndUnit* convertDistanceAndUnitImperialYards(uint32 distance,
         DistancePrintingPolicy::DistanceRound round);
   /**
    * Convert the given distance into meters and kilometers
    * Used by convertDistance().
    * @return formatted string.
    */
   static char * convertDistanceMetric(uint32 distance,
         DistancePrintingPolicy::DistanceRound round,
         bool useSpace = false);
   /**
    * Convert the given distance into meters and kilometers
    * Used by convertDistance().
    * @return a DistanceAndUnit object containing the distance and unit string.
    */
   static DistanceAndUnit* convertDistanceAndUnitMetric(uint32 distance,
         DistancePrintingPolicy::DistanceRound round);
   /**
    * Convert the given speed (in meters per second)
    * into miles per hour.
    * Used by convertSpeed().
    * @return Speed as float.
    */
   static float convertSpeedMPS2MPH(int32 speed);
   /**
    * Convert the given speed (in meters per second)
    * into kilometers per hour.
    * Used by convertSpeed().
    * @return Speed as float.
    */
   static float convertSpeedMPS2KMH(int32 speed);
   /**
    * Convert the given speed (in meters per second)
    * into the unit specified by the mode parameter.
    * @return Converted speed as float.
    */
   static float convertSpeed(int32 speed,
         DistancePrintingPolicy::DistanceMode mode);

};
} /* namespace isab */

#endif /* DISTANCEPRINTINGPOLICY_H */
