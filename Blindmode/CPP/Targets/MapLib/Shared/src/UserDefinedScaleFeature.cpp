/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "config.h"

#include "UserDefinedScaleFeature.h"

#include "MC2BoundingBox.h"

#include <math.h>
#include <stdio.h>
#include "TileMapUtil.h"
#include "MapPlotter.h"
#include "GfxConstants.h"
#include "MapProjection.h"

using namespace std;

ScaleSettings
UserDefinedScaleFeature::getMilesYardsSettings()
{
   // These must be sorted. Smallest unit first.
   ScaleSettings::unitVector_t meterVect;
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "yds", 0.9144, 100.0 ) );
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "mile", 1609.344, 1.0,
                                               "miles" ) );
                                               

   return ScaleSettings( meterVect );
}

ScaleSettings
UserDefinedScaleFeature::getMilesFeetSettings()
{
   // These must be sorted. Smallest unit first.
   ScaleSettings::unitVector_t meterVect;
//     meterVect.push_back( ScaleSettings::unitVector_t::value_type( "m", 1.0 ) );
//     meterVect.push_back(
//        ScaleSettings::unitVector_t::value_type( "km", 1000.0 ) );
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "ft", 0.3048, 1.0 ) );
//     meterVect.push_back(
//        ScaleSettings::unitVector_t::value_type( "yds", 0.9144, 100.0 ) );
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "mile", 1609.344, 1.0,
                                               "miles" ) );
                                               

   return ScaleSettings( meterVect );
}

ScaleSettings
UserDefinedScaleFeature::getMeterSettings()
{
   // These must be sorted. Smallest unit first.
   ScaleSettings::unitVector_t meterVect;
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "m", 1.0, 1.0 ) );
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "km", 1000.0, 1.0 ) );

   return ScaleSettings( meterVect );
}

ScaleSettings
UserDefinedScaleFeature::getTestScaleSettings()
{
   // These must be sorted. Smallest unit first.
   ScaleSettings::unitVector_t meterVect;
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "aln", 0.594 , 1.0, "alnar" ) );
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "fjärdingsväg",
                                               2672.0 , 1.0, "fjärdingsvägar"
                                               ) );
   meterVect.push_back(
      ScaleSettings::unitVector_t::value_type( "gam. mil",
                                               10688.0 , 1.0 ) );
   
   return ScaleSettings( meterVect );
}

UserDefinedScaleFeature::
UserDefinedScaleFeature( const char* fontName,
                         int fontSize,
                         const ScaleSettings& scaleSettings,
                         int dpiCorrectionFactor) :
      UserDefinedFeature( UserDefinedFeature::scale,
                          vector<MC2Point>(1, MC2Point(0,0)),
                          MC2Point(0,0),
                          dpiCorrectionFactor),
      m_scaleSettings( scaleSettings ),
      m_point ( -1, -1 )
{
   m_fontName = TileMapUtil::newStrDup( fontName );
   m_fontSize = fontSize;
   setClickable( false );
}

void
UserDefinedScaleFeature::setScale( const ScaleSettings& scaleSettings )
{
   m_scaleSettings = scaleSettings;
}

void
UserDefinedScaleFeature::setPoint( const MC2Point& point )
{
   m_point = point;
}

UserDefinedScaleFeature::~UserDefinedScaleFeature()
{
   delete [] m_fontName;
}

static inline void
makeDrawItemParameters( uint16 width, uint16 height,
                        MC2BoundingBox& bbox,
                        float32& xFactor, 
                        float32& yFactor )
{
   bbox.updateCosLat();
   xFactor = float32(width - 1) / bbox.getLonDiff();
   yFactor = float32(height - 1) / bbox.getHeight();
   
   // Make sure that the image will not have strange proportions.
   float32 factor = float32(bbox.getHeight()) / bbox.getWidth()
      * (width - 1) / (height - 1);
   if (factor < 1) {
      // Compensate for that the image is wider than it is high
      yFactor *= factor;
   } else {
      // Compensate for that the image is higher than it is wide
      xFactor /= factor;
   }
}

//  static int getLinearX( int32 mc2Lon, MC2BoundingBox& bbox, 
//                          float64 xFactor, uint16 width ) 
//  {
//     int32 coord = int32(rint((mc2Lon - bbox.getMinLon()) * xFactor));
         
//     if (coord > (width*2))
//        coord = width*2;
//     else if (coord < (-width*2))
//        coord = -width*2 ;
               
//     return (coord);
//  }


static inline int32
getLon( int x, MC2BoundingBox& bbox, 
        float32 xFactor, uint16 /*width*/ )
{
   return int32( rint( ( (x / xFactor) + bbox.getMinLon() ) ) );
}

int32
UserDefinedScaleFeature::chooseFactor( int32 origDist, int& unitNumber )
{
   float32 dist = origDist;
   const ScaleSettings::unitVector_t& unitVect =
      m_scaleSettings.getUnitVector();
   for ( int i = unitVect.size() - 1; i >= 0; --i ) {
      float32 distByFactor = dist / unitVect[i].m_factorFromMeters;
      if ( distByFactor >= unitVect[i].m_changeValue ) {
         unitNumber = i;
         dist = distByFactor; 
         return int32(dist);
      }
   }
   unitNumber = 0;
   return int32( origDist );
}


void
UserDefinedScaleFeature::draw( const MapProjection& matrix,
                               isab::MapPlotter& plotter )
{
   if ( ! isVisible() ) {
      return;
   }
   MC2BoundingBox bbox = matrix.getBoundingBox();
   // Shamelessly copied from GDImageDraw
   MC2Point screenSize = matrix.getScreenSize();
   uint16 width  = screenSize.getX();
   uint16 height = screenSize.getY();
   
   const float32 scalePartOfWidth = 0.40;
   int bottomMargin = 8;
   int sideMargin = 8;
   int barWidth = 5;
   int textMargin = 2;
   float32 fontSize = 10.0;
   if ( width > 250 ) { // > 250 pxl
      fontSize = 10.0;
      bottomMargin = sideMargin = 6;
   } else if ( width > 150 ) { // 250-151 pxl
      fontSize = 8.0;
      bottomMargin = sideMargin = 4;
      barWidth = 4;
   } else { // less than 150 pxl
      fontSize = 6.0;
      bottomMargin = sideMargin = 2;
      textMargin = 1;
      barWidth = 3;
   }

   int rightPosX = width - sideMargin;
   int posY = height - bottomMargin;
   int leftPosX = int( rint( rightPosX - scalePartOfWidth*width ) );
   float32 xFactor = 0;
   float32 yFactor = 0;
   makeDrawItemParameters( width, height,
                           bbox, xFactor, yFactor );
   
   int32 rightLon = getLon( rightPosX, bbox, xFactor, width );
   int32 leftLon = getLon( leftPosX, bbox, xFactor, width );

   int32 distance = int32( (rightPosX - leftPosX ) * matrix.getPixelScale() *
                           bbox.getCosLat() );

   // Get unit from unit vector.
   int unitIdx;
   distance = chooseFactor( distance, unitIdx );
      
//  int32( rint( ( (rightLon - leftLon)*
//                                     GfxConstants::MC2SCALE_TO_METER*
//                                   bbox.getCosLat() ) ) );
   char distStr[128];
   uint32 white = 0xffffff;
   uint32 black = 0x000000;

   //** Make distance nice length
   // First number zeros
   int32 logDist = 10;
   while( distance > logDist ) {
      logDist *= 10;
   }
   logDist /= 10;
   // Then some fine tuning
   int32 niceDist = 0;
   // Should be even ^2 but 1 is allowed too
   if ( 2*logDist < distance ) {
      niceDist = 2*logDist;
   } else {
      niceDist = logDist;
   }
   while ( niceDist + 2*logDist < distance ) {
      niceDist += 2*logDist;
   }

   //** Nice distance as string
   distStr[ 0 ] = '\0';
   sprintf(distStr, "%u %s",
           (unsigned int)niceDist,
           (niceDist == 1) ?
           m_scaleSettings.getUnitVector()[unitIdx].m_name :
           m_scaleSettings.getUnitVector()[unitIdx].m_pluralName );
      

   // Convert the nice distance back to meters
   niceDist = int32(
      niceDist *
      m_scaleSettings.getUnitVector()[unitIdx].m_factorFromMeters );
   
      
   //** Update leftLon and leftPosX for new nice distance
   leftLon = (rightLon - int32 ( rint( 
      niceDist*GfxConstants::METER_TO_MC2SCALE / bbox.getCosLat() ) ) );

   // Move the stuff to the correct position
   if ( m_point != MC2Point( -1, -1 ) ) {
      rightPosX = m_point.getX();
      posY      = m_point.getY();
   }
   leftPosX = rightPosX - int32( niceDist / matrix.getPixelScale() );

   
   
   
   //** Draw scalebar with four parts
   float32 partWidth = (rightPosX - leftPosX) / 4.0;

   
   
   // Fill all with white
   plotter.setLineWidth( 1 );
   plotter.setPenColor( white );
   plotter.setFillColor( white );
   plotter.drawRect( true, isab::Rectangle( leftPosX,
                                            posY - barWidth,
                                            rightPosX - leftPosX,
                                            barWidth ) );
   
//     gdImageFilledRectangle( m_image, leftPosX, posY - barWidth,
//                             rightPosX, posY, white );
   
   // Black border
   plotter.setPenColor( black );
   plotter.drawRect( false, isab::Rectangle( leftPosX,
                                             posY - barWidth,
                                             rightPosX - leftPosX,
                                             barWidth ) );
//     gdImageRectangle( m_image, leftPosX, posY - barWidth,
//                       rightPosX, posY, white );
   // Leftmost black part
   plotter.setFillColor( black );
   plotter.drawRect( true, isab::Rectangle( leftPosX,
                                            posY - barWidth,
                                            int( rint( partWidth ) ), 
                                            barWidth ) );
   
//     gdImageFilledRectangle( m_image, leftPosX, posY - barWidth,
//                             leftPosX + int( rint( partWidth ) ), 
//                             posY, black );
   // Second black part
   plotter.setFillColor( black );
   {
      int x = leftPosX + int( rint( 2*partWidth ) );
      plotter.drawRect(
         true,
         isab::Rectangle(x,
                         posY - barWidth,
                         leftPosX + int( rint( 3*partWidth ) ) - x,
                         barWidth ) );
   }
   
//     gdImageFilledRectangle( m_image, 
//                             leftPosX + int( rint( 2*partWidth ) ), 
//                             posY - barWidth,
//                             leftPosX + int( rint( 3*partWidth ) ), 
//                             posY, black );

   int barTextSpacing = 2;
   bool textBox = false;

   //** Draw distance string in own square text box
//     char font[512];
//     getFontPath( font, fontName );
//   int textArea[ 8 ];

   // Set the font
   {
      STRING* plotterFont = plotter.createString( m_fontName );   
      plotter.setFont( *plotterFont, m_fontSize );
      plotter.deleteString( plotterFont );
   }

   // Get size of distStr
//     char* err = gdImageStringFT( NULL, textArea, black, font,
//                                  fontSize, 0.0, leftPosX, posY, distStr );
   STRING* str = plotter.createString( distStr );
   isab::Rectangle strRect =
      plotter.getStringAsRectangle( *str, MC2Point(0,0) );
   
   // Use textArea for textbox
//     int textWidth = textArea[ 4 ] - textArea[ 6 ];
//     int textHeight = textArea[ 1 ] - textArea[ 7 ];
   int textWidth = strRect.getWidth();
   int textHeight = strRect.getHeight();
   
   // Put textbox right aligned over scale
   //int textXCenter = rightPosX - int( rint( 2*partWidth ) );
   int textXCenter = rightPosX - (textWidth >> 1);
   int textYBottom = posY - barWidth - barTextSpacing - textMargin;
   // Make sure that text fits in image
   if ( textXCenter + textWidth/2 + textMargin + sideMargin >= width ) {
      textXCenter = width - textWidth/2 - textMargin - sideMargin;
   }
   
   if ( textBox ) {
      plotter.setFillColor( white );
      plotter.drawRect(
         true,
         isab::Rectangle( (textXCenter - textWidth/2 - textMargin ),
                          (textYBottom - textHeight - textMargin),
                          textMargin << 1,
                          textMargin << 1 ) );
//        gdImageFilledRectangle( m_image, 
//                                (textXCenter - textWidth/2 - textMargin ),
//                                (textYBottom - textHeight - textMargin),
//                                (textXCenter + textWidth/2 + textMargin),
//                                (textYBottom + textMargin), 
//                                white );
   }
   
//     int x = textXCenter - textWidth/2;
//     int y = textYBottom;
   int x = textXCenter;
   int y = textYBottom - (textHeight >> 1);
   // Finally draw distStr
   plotter.setPenColor( black );
   plotter.drawText( *str, MC2Point( x, y) );
//     gdImageStringFT( m_image, textArea, black, font,
//                      fontSize, 0.0, x, y, distStr );
   plotter.deleteString( str );
}

