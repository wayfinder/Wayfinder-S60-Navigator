/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERDEFINEDSCALEFEATURE_H
#define USERDEFINEDSCALEFEATURE_H

#include "config.h"
#include "UserDefinedFeature.h"

namespace isab {
   class MapPlotter;
}
class MapProjection;

/**
 *   Settings for scale. See cpp-file for examples.
 *
 */
class ScaleSettings {
public:

   struct unitInfo_t {

      /**
       *   @param name Name in utf-8. Will not be copied.
       *   @param factorFromMeters Conversion factor from meters.
       *   @param changeValue The scale will start using the unit
       *                      when the number of units reach this value.
       *   @param pluralName  Name to use in plural.
       */
      unitInfo_t( const char* name,
                  float32 factorFromMeters,
                  float32 changeValue,
                  const char* pluralName = NULL ) :
            m_name( name ),
            m_factorFromMeters( factorFromMeters ),
            m_changeValue( changeValue ),
            m_pluralName( pluralName ) {
         if ( pluralName == NULL ) {
            m_pluralName = name;
         }
      }
      
      const char* m_name;
      float32 m_factorFromMeters;
      float32 m_changeValue;
      const char* m_pluralName;
   };
   
   typedef std::vector<unitInfo_t> unitVector_t;

   // These must be sorted. Smallest unit first.
   ScaleSettings( const unitVector_t& otherUnits )
         :  m_units ( otherUnits ) {
   }
   
   const unitVector_t& getUnitVector() const {
      return m_units;
   }

private:
   /// Vector of units to use
   unitVector_t m_units;   
};

class UserDefinedScaleFeature : public UserDefinedFeature {
private:
   
   static ScaleSettings getTestScaleSettings();

public:
   /**
    *   Scale feature that can display a scale symbol.
    */
   UserDefinedScaleFeature( const char* fontName,
                            int fontSize,
                            const ScaleSettings& scaleSettings =
                            getTestScaleSettings(), 
                            int dpiCorrectionFactor = 1 );

   /**
    *   Deletes the bitmap name.
    */
   ~UserDefinedScaleFeature();

   /**
    *   Draws the scale on the supplied plotter.
    */
   void draw( const MapProjection& matrix,
              isab::MapPlotter& plotter,
              uint32 textColor );

   /**
    *   Sets a new point where the scale feature should
    *   be positioned. Currently always bottom-right.
    *   (-1, -1) means bottom right of screen with some margin.
    *   @param point New point.
    */
   void setPoint( const MC2Point& point );

   void setScale( const ScaleSettings& scaleSettings );

   static ScaleSettings getMilesYardsSettings();
   static ScaleSettings getMilesFeetSettings();
   static ScaleSettings getMeterSettings();
private:   
   /**
    *    Chooses a factor from the vector of units
    *    which will be the largest unit where the unit is > 1.
    */
   int32 chooseFactor( int32 origDist, int& unitNumber );
   
   /// The name of the font.
   char* m_fontName;
   /// The size of the font.
   int m_fontSize;
   /// Scale settings
   ScaleSettings m_scaleSettings;
   /// Coordinate (currently bottom right)
   MC2Point m_point;
};

#endif
