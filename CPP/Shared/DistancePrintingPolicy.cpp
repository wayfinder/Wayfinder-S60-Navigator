/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <arch.h>
#include <stdio.h>
#include <math.h>
#include "DistancePrintingPolicy.h"

namespace isab {

   namespace {
      inline uint32 ToUint32(float from){
#ifdef __WINS__
         return uint32(floor(from));
#else 
         return uint32(from);
#endif
      }
      inline uint32 ToUint32(double from){
#ifdef __WINS__
         return uint32(floor(from));
#else 
         return uint32(from);
#endif
      }
      
   }
uint32
DistancePrintingPolicy::roundTo(uint32 distance, uint32 base)
{
   return ((distance + base/2 ) / base) * base;
}
uint32
DistancePrintingPolicy::get1000Decimal(uint32 distance)
{
   return ((roundTo(distance, 100)/100) % 10);
}
uint32
DistancePrintingPolicy::get1000(uint32 distance)
{
   return ((roundTo(distance, 100)/1000));
}
uint32
DistancePrintingPolicy::meterToFeet(uint32 meters)
{
   return ToUint32(meters*3.2808399);
}
uint32
DistancePrintingPolicy::meterToYards(uint32 meters)
{
   return ToUint32(meters*1.0936133);
}
uint32
DistancePrintingPolicy::getMilesWhole(uint32 meters)
{
   return ToUint32(getMiles(meters)+0.5);
}
uint32
DistancePrintingPolicy::getMilesIntegerPart(uint32 meters)
{
   return ToUint32(getMiles(meters)+0.05);
}
uint32
DistancePrintingPolicy::getMilesDecimal(uint32 meters)
{
   return ToUint32((getMiles(meters)+0.05)*10)%10;
}
float
DistancePrintingPolicy::getMiles(uint32 meters)
{
   return (meters/1609.344);
}
char *
DistancePrintingPolicy::convertDistanceMetric(uint32 distance,
      DistancePrintingPolicy::DistanceRound round,
      bool useSpace)
{
   DistanceAndUnit* distanceAndUnit = convertDistanceAndUnitMetric(distance, round);
   char* concatenated = distanceAndUnit->Concatenated(useSpace);
   delete distanceAndUnit;
   return concatenated;
}

DistancePrintingPolicy::DistanceAndUnit*
DistancePrintingPolicy::convertDistanceAndUnitMetric(uint32 distance,
      DistancePrintingPolicy::DistanceRound round)
{
   uint32 tmpDist = distance;
   uint32 tmpDist2 = 0;
   DistanceAndUnit* distanceAndUnit = new DistanceAndUnit();
   strcpy(distanceAndUnit->m_unit, "m");

   if (distance > 10000) {
      /* Can use kilometers instead. */
      tmpDist = DistancePrintingPolicy::roundTo(distance, 1000);
      tmpDist /= 1000;
      strcpy(distanceAndUnit->m_unit, "km");
      sprintf(distanceAndUnit->m_distance, "%"PRIu32, tmpDist);
   } else if (distance > 1000) {
      /* Can use kilometers instead. */
      tmpDist = DistancePrintingPolicy::get1000(distance);
      tmpDist2 = DistancePrintingPolicy::get1000Decimal(distance);
      strcpy(distanceAndUnit->m_unit, "km");

      sprintf(distanceAndUnit->m_distance, "%"PRIu32".%"PRIu32, tmpDist, tmpDist2);
   } else {
      /* Single meters. */
      if (round == DistancePrintingPolicy::Round) {
         if (tmpDist < 50) {
            /* Don't do anything. */
         } else if (tmpDist < 200) {
            /* Round to 5meters. */
            tmpDist = DistancePrintingPolicy::roundTo(tmpDist, 5);
         } else {
            /* Round to 10 meters. */
            tmpDist = DistancePrintingPolicy::roundTo(tmpDist, 10);
         }
      }
      sprintf(distanceAndUnit->m_distance, "%"PRIu32, tmpDist);
   }

   return distanceAndUnit;
}

char *
DistancePrintingPolicy::convertDistanceImperialYards(uint32 distance,
      DistancePrintingPolicy::DistanceRound round,
      bool useSpace)
{
   DistanceAndUnit* distanceAndUnit = convertDistanceAndUnitImperialYards(distance, round);
   char* concatenated = distanceAndUnit->Concatenated(useSpace);
   delete distanceAndUnit;
   return concatenated;
}

DistancePrintingPolicy::DistanceAndUnit*
DistancePrintingPolicy::convertDistanceAndUnitImperialYards(uint32 distance,
      DistancePrintingPolicy::DistanceRound round)
{
   float miles = DistancePrintingPolicy::getMiles(distance);
   uint32 tmpDist;
   uint32 tmpDist2;
   DistanceAndUnit* distanceAndUnit = new DistanceAndUnit();
   if (miles > 10) {
      /* Can use 10:s of miles instead. */
      tmpDist = ToUint32(miles+0.5);
      strcpy(distanceAndUnit->m_unit, "mi");
      sprintf(distanceAndUnit->m_distance, "%"PRIu32, tmpDist);
   } else if (miles > 0.5) {
      /* Can use miles with decimal instead. */
      tmpDist = ToUint32(miles+0.05);
      tmpDist2 = DistancePrintingPolicy::getMilesDecimal(distance);
      strcpy(distanceAndUnit->m_unit, "mi");
      sprintf(distanceAndUnit->m_distance, "%"PRIu32".%"PRIu32, tmpDist, tmpDist2);
   } else {
      /* Single yards. */
      tmpDist = DistancePrintingPolicy::meterToYards(distance);
      if (round == DistancePrintingPolicy::Round) {
         if (tmpDist < 50) {
            /* Don't do anything. */
         } else if (tmpDist < 200) {
            /* Round to 5 yards. */
            tmpDist = DistancePrintingPolicy::roundTo(tmpDist, 5);
         } else {
            /* Round to 10 yards. */
            tmpDist = DistancePrintingPolicy::roundTo(tmpDist, 10);
         }
      }
      strcpy(distanceAndUnit->m_unit, "y");
      sprintf(distanceAndUnit->m_distance, "%"PRIu32, tmpDist);
   }

   return distanceAndUnit;
}

char *
DistancePrintingPolicy::convertDistanceImperialFeet(uint32 distance,
      DistancePrintingPolicy::DistanceRound round,
      bool useSpace)
{
   DistanceAndUnit* distanceAndUnit = convertDistanceAndUnitImperialFeet(distance, round);
   char* concatenated = distanceAndUnit->Concatenated(useSpace);
   delete distanceAndUnit;
   return concatenated;
}

DistancePrintingPolicy::DistanceAndUnit*
DistancePrintingPolicy::convertDistanceAndUnitImperialFeet(uint32 distance,
      DistancePrintingPolicy::DistanceRound round)
{
   float miles = DistancePrintingPolicy::getMiles(distance);
   uint32 tmpDist;
   uint32 tmpDist2;
   DistanceAndUnit* distanceAndUnit = new DistanceAndUnit();

   if (miles > 10) {
      /* Can use 10:s of miles instead. */
      tmpDist = ToUint32(miles+0.5);
      strcpy(distanceAndUnit->m_unit, "mi");
      sprintf(distanceAndUnit->m_distance, "%"PRIu32, tmpDist);
   } else if (miles > 0.15) {
      /* Can use miles with decimal instead. */
      tmpDist = ToUint32(miles+0.05);
      tmpDist2 = DistancePrintingPolicy::getMilesDecimal(distance);
      strcpy(distanceAndUnit->m_unit, "mi");
      sprintf(distanceAndUnit->m_distance, "%"PRIu32".%"PRIu32, tmpDist, tmpDist2);
   } else {
      /* Single feet. */
      tmpDist = DistancePrintingPolicy::meterToFeet(distance);
      if (round == DistancePrintingPolicy::Round) {
         if (tmpDist < 100) {
            /* Don't do anything. */
         } else if (tmpDist < 200) {
            /* Round to 5 feet. */
            tmpDist = DistancePrintingPolicy::roundTo(tmpDist, 5);
         } else if (tmpDist < 400) {
            /* Round to 10 feet. */
            tmpDist = DistancePrintingPolicy::roundTo(tmpDist, 10);
         } else if (tmpDist < 600) {
            /* Round to 25 feet. */
            tmpDist = DistancePrintingPolicy::roundTo(tmpDist, 25);
         } else {
            /* Round to 50 feet. */
            tmpDist = DistancePrintingPolicy::roundTo(tmpDist, 50);
         }
      }
      strcpy(distanceAndUnit->m_unit, "ft");
      sprintf(distanceAndUnit->m_distance, "%"PRIu32, tmpDist);
   }

   return distanceAndUnit;
}

char *
DistancePrintingPolicy::convertAltitude(uint32 altitude,
      DistancePrintingPolicy::DistanceMode mode,
      DistancePrintingPolicy::DistanceRound round)
{
   switch (mode) {
      case DistancePrintingPolicy::ModeImperialYards:
      case DistancePrintingPolicy::ModeImperialFeet:
         return convertDistanceImperialFeet(altitude, round);
         break;                    
      case DistancePrintingPolicy::ModeInvalid:
      case DistancePrintingPolicy::ModeMetric:
      default:
         return convertDistanceMetric(altitude, round);
         break;
   }
}
char *
DistancePrintingPolicy::convertDistance(uint32 distance,
      DistancePrintingPolicy::DistanceMode mode,
      DistancePrintingPolicy::DistanceRound round)
{
   switch (mode) {
      case DistancePrintingPolicy::ModeImperialYards:
         return convertDistanceImperialYards(distance, round);
         break;
      case DistancePrintingPolicy::ModeImperialYardsSpace:
         return convertDistanceImperialYards(distance, round, true);
         break;
      case DistancePrintingPolicy::ModeImperialFeet:
         return convertDistanceImperialFeet(distance, round);
         break;
      case DistancePrintingPolicy::ModeImperialFeetSpace:
         return convertDistanceImperialFeet(distance, round, true);
         break;
      case DistancePrintingPolicy::ModeMetricSpace:
         return convertDistanceMetric(distance, round, true);
         break;
      case DistancePrintingPolicy::ModeInvalid:
      case DistancePrintingPolicy::ModeMetric:
      default:
         return convertDistanceMetric(distance, round);
         break;
   }
}

DistancePrintingPolicy::DistanceAndUnit*
DistancePrintingPolicy::convertDistanceAndUnit(uint32 distance,
      DistancePrintingPolicy::DistanceMode mode,
      DistancePrintingPolicy::DistanceRound round)
{
   switch (mode) {
      case DistancePrintingPolicy::ModeImperialYards:
      case DistancePrintingPolicy::ModeImperialYardsSpace:
         return convertDistanceAndUnitImperialYards(distance, round);
         break;
      case DistancePrintingPolicy::ModeImperialFeet:
      case DistancePrintingPolicy::ModeImperialFeetSpace:
         return convertDistanceAndUnitImperialFeet(distance, round);
         break;
      case DistancePrintingPolicy::ModeMetricSpace:
      case DistancePrintingPolicy::ModeInvalid:
      case DistancePrintingPolicy::ModeMetric:
      default:
         return convertDistanceAndUnitMetric(distance, round);
         break;
   }
}

float
DistancePrintingPolicy::convertSpeedMPS2MPH(int32 speed)
{
   return (speed*2.2369363);
}
float
DistancePrintingPolicy::convertSpeedMPS2KMH(int32 speed)
{
   return (speed*3.6);
}
float
DistancePrintingPolicy::convertSpeed(int32 speed,
      DistancePrintingPolicy::DistanceMode mode)
{
   switch (mode) {
      case DistancePrintingPolicy::ModeImperialYards:
      case DistancePrintingPolicy::ModeImperialFeet:
         return DistancePrintingPolicy::convertSpeedMPS2MPH(speed);
         break;
      case DistancePrintingPolicy::ModeInvalid:
      case DistancePrintingPolicy::ModeMetric:
      default:
         return DistancePrintingPolicy::convertSpeedMPS2KMH(speed);
         break;
   }
}


}
