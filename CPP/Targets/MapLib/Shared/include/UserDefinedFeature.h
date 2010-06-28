/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERDEFINEDFEATURE_H
#define USERDEFINEDFEATURE_H

class MapMovingInterface;

#include "config.h"
#include "ScreenOrWorldCoordinate.h"
#include "MC2SimpleString.h"

#include <vector>

/**
 *   A UserDefinedFeature is a feature that can be added to the map
 *   e.g. by Nav2.
 */
class UserDefinedFeature {
public:

   /// Destructor
   virtual ~UserDefinedFeature() {};
   
   /// Types of features that can be drawn.
   enum user_feature_t {
      /// A filled or unfilled polygon
      directed_poly,
      /// A bitmap stored on the server
      bitmap,
      /// A scale
      scale,
   };

   /// Sets visible on/off.
   void setVisible(int visible);

   /// @return True if visible
   inline int isVisible() const;
   
   /// Sets clickable on/off.
   void setClickable(int clickable);

   /// @return True if clickable
   inline int isClickable() const;

   /// @return True if the feature was drawn last time.
   inline int wasDrawn() const;
   
   /// Set if the features is drawn.
   inline void setDrawn( int drawn );
   
   /**
    *   Sets new center coordinate.
    */
   void setCenter( const ScreenOrWorldCoordinate& centerCoord );

   /**
    *   Gets the center coordinate.
    */
   const ScreenOrWorldCoordinate& getCenter() const;

   /**
    *   Sets new point vector.
    */
   void setPoints( const std::vector<MC2Point>& newPoints );

   /**
    *   Swaps the points from the sent in vector and the internal one.
    */
   void swapPoints( std::vector<MC2Point>& points );
   
   /** 
    *   rescales all points in m_origPoints
    *   according to the m_dpiCorrectionFactor and the aDynamicScale
    *   @param aDynamicScale the dynamic scale we want to rescale the points to.
    */
   void rescalePoints(float aDynamicScale = 1.0);
 
   /** 
    *   rescales all points in m_origPoints
    *   according to the m_dpiCorrectionFactor and the aDynamicScale based
    *   on the coordinates in points.
    *   @param points The points to be scaled.
    *   @param aDynamicScale the dynamic scale we want to rescale the points to.
    *   
    */
   void rescalePoints( const std::vector<MC2Point>& points, float aDynamicScale = 1.0);

   /**
    *   Updates the screen coordinates using the matrix.
    *   (Internal cosLat must be set).
    */    
   void updateScreenCoords(const MapMovingInterface& matrix);
   
   /**
    *   Returns the vector of pixels to draw.
    */
   inline const std::vector<MC2Point>& getScreenCoords() const;
   
   /// Returns the type of this feature.
   inline user_feature_t getType() const { return m_type; }

   /**
    *   Set the name of the feature. 
    */
   void setName( const MC2SimpleString& name );
   
   /**
    *   Get the name of the feature.
    */
   const MC2SimpleString& getName() const;
   
   /**
    *   Return if the feature has a name.
    */
   bool hasName() const;

   /**
    *   True if the feature always should be plottered in 2d.
    */
   bool getAlways2d() const;

   /**
    *   Sets this feature to be plottered in 2d only or in 3d
    *   when being in 3d mode.
    */
   void setAlways2d( bool always2d );

   /**
    * Sets the dpiCorrectionFactor.
    * @param dpiCorrectionFactor The new dpiCorrectionFactor.
    */
   void setDpiCorrectionFactor( int dpiCorrectionFactor );
   
protected:
   /**
    *   Sets the type.
    */
   UserDefinedFeature(user_feature_t type,
                      const std::vector<MC2Point>& points,
                      const ScreenOrWorldCoordinate& center,
                      int dpiCorrectionFactor = 1);
   
  
   /// Type of feature.
   user_feature_t m_type;
   
   /// Vector of original points to draw
   std::vector<MC2Point> m_origPoints;
   /// Vector of points converted into screen points to draw.
   std::vector<MC2Point> m_drawPoints;
   /// Point corresponding to MC2Point(0,0) in the feature.
   ScreenOrWorldCoordinate m_center;
   /// Angle in degrees. Not applicable for all features.
   float m_angleDegs;
   /// True if angle is from north. Not applicable for all features.
   int m_angleIsFromNorth;
   /// True if the feature is visible.
   int m_visible;
   /// True if it should be possible to select the feature by clicking
   int m_clickable;
   /// True if the feature always should be plottered in 2d.
   bool m_always2d;
   /// Set the DPI correction factor.
   int m_dpiCorrectionFactor;
   /// The name of the feature.
   MC2SimpleString m_name;

   /// If the feature was drawn last time.
   int m_drawn;
};

inline const std::vector<MC2Point>&
UserDefinedFeature::getScreenCoords() const
{
   return m_drawPoints;
}

inline int
UserDefinedFeature::isVisible() const
{
   return m_visible;
}

inline int
UserDefinedFeature::isClickable() const
{
   return m_clickable;
}
inline int 
UserDefinedFeature::wasDrawn() const
{
   return m_drawn;
}
   
inline void 
UserDefinedFeature::setDrawn( int drawn )
{
   m_drawn = drawn;
}

#endif



