/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"


#include "TileMapConfig.h"


#ifdef __SYMBIAN__
   #include <e32math.h>
#endif
#include<stdlib.h>

#include <vector>
#include <set>
#include <algorithm>
#include "MC2SimpleString.h"

#include "TileMapTextHandler.h"

#include "TileMap.h"
#include "TileFeature.h"
#include "TileMapParams.h"
#include "MC2BoundingBox.h"
#include "MC2Point.h"
#include "MapPlotter.h"
#include "MapPlotterTypes.h"
#include "GfxUtility.h"
#include "InsideUtil.h"

#include "MapProjection.h"
#include "TileMapHandler.h"

#define USE_TEXT_HANDLIND
#define MOVE_HORIZONTAL_TEXTS_EVEN_IF_NOT_TRACKING

using namespace std;
   
void
TextResult::setString( STRING* string )
{
   m_string = string;
}

void
TextResult::setFontName( const STRING* fontName )
{
   m_fontName = fontName;
}

void
TextResult::setFontSize( int fontSize )
{
   m_fontSize = fontSize;
}

void
TextResult::setTypeOfString( uint16 type )
{
   m_type = type;
}

void
TextResult::setStringAsRect( const isab::Rectangle& stringAsRect )
{
   m_stringAsRect = stringAsRect;
}

void
TextResult::setFontColor( 
      unsigned int r, 
      unsigned int g, 
      unsigned int b )
{
   m_r = r;
   m_g = g;
   m_b = b;
}

const isab::Rectangle&
TextResult::getStringAsRect( ) const
{
   return m_stringAsRect;
}

const STRING&
TextResult::getString() const
{
   return *m_string;
}

const STRING&
TextResult::getFontName() const
{
   return *m_fontName;
}

int
TextResult::getFontSize() const
{
   return m_fontSize;
}

uint16
TextResult::getTypeOfString() const
{
   return m_type;
}

void
TextResult::getFontColor( 
      unsigned int& r, 
      unsigned int& g, 
      unsigned int& b ) const 
{
   r = m_r;
   g = m_g;
   b = m_b;
}

const vector<TextPlacementNotice>&
TextResult::getTPN() const
{
   return m_stringNotices;
}

vector<TextPlacementNotice>&
TextResult::getTPN()
{
   return m_stringNotices;
}

void
TextResult::addTPN( const TextPlacementNotice& tpn )
{
   m_stringNotices.push_back( tpn );
}

int
TileMapTextHandler::moveToNextString()
{
   // Move to next feature index.
   ++m_currentTextIndexInMap;
   // Stop when we have reached the end iterator.
   while ( m_currentMapIt != m_endMapIt ) {
      // Get the current string map.
      TileMap* stringMap = m_currentMapIt.getStringMap();
      if ( *m_currentMapIt == NULL || 
           stringMap == NULL ) {
         // Either string or feature map is NULL. Move to next map.
         ++m_currentMapIt;
      } else if ( m_currentTextIndexInMap >= 
                  stringMap->getNbrFeaturesWithText() ) {
         // Feature index is out of range. Move to next map.
         m_currentTextIndexInMap = 0;
         ++m_currentMapIt;
      } else {
         // Valid string found.
         return true;
      }
   }
   // If we get here, no string has been found.
   return false;
}


TileMapTextHandler::TileMapTextHandler( isab::MapPlotter* mapPlotter,
                                        const MapProjection& transformMatrix,
                                        const MapsToDrawIt& endMapIt,
                                        TileMapHandler& mapHandler )
      : m_currentMapIt( endMapIt ), m_endMapIt( endMapIt ),
        m_matrix( transformMatrix ),
        m_mapHandler( mapHandler )
{
   m_currentMapIt = m_endMapIt;
   m_mapPlotter = mapPlotter;
   m_state = placingText; 
   isab::Rectangle r;
   m_mapPlotter->getMapSizeDrawingUnits( r );
   m_screenCoords = PixelBox( r );
   m_magicConst = 30;
   m_maxTotalAngle = 90;
   m_maxAngle = 30;
   m_maxTotalCost = 100;
   m_angle = m_matrix.getAngle();
   m_oldScale = 1.0;
}


TileMapTextHandler::~TileMapTextHandler()
{
   reset();   
   for ( map<MC2SimpleString, STRING*>::iterator it = m_fonts.begin();
         it != m_fonts.end();
         ++it ) {
      m_mapPlotter->deleteString( it->second );
   }
}

void
TileMapTextHandler::setTextSettings( const TileMapTextSettings& settings )
{
   m_textSettings = settings;
}

const STRING*
TileMapTextHandler::getPlotterStr( const MC2SimpleString& strFont )
{
   map<MC2SimpleString, STRING*>::const_iterator it =
      m_fonts.find( strFont );
   if ( it != m_fonts.end() ) {
      return it->second;
   } else {
      // Insert
      m_fonts.insert(
         make_pair( strFont,
                    m_mapPlotter->createString( strFont.c_str() ) ) );
      return getPlotterStr( strFont );
   }
}

void
TileMapTextHandler::deleteAndClear( vector<TextResult*>& vect )
{
   int vecSize = vect.size();   
   for ( int i = 0; i < vecSize; ++i ) {
      // Plotter font is cached in TextHandler now.
      //m_mapPlotter->deleteString ( vect[i]->m_fontName );
      m_mapPlotter->deleteString ( vect[i]->m_string );
      delete vect[i];
   }
   vect.clear();
}

void
TileMapTextHandler::newMapArrived( const MapsToDrawIt& /*curIt*/,
                                   const MapsToDrawIt& beginIt,
                                   const MapsToDrawIt& endIt ) 
{
   /** 
    * If the new map is of lower prio than the current one, do nothing.
    * If its of higher prio restart the textplacement.
    */
   if( m_textResultsBeforeNewMapArrived.size() <
         m_textResults.size()){
      m_textResultsBeforeNewMapArrived.swap( m_textResults );
      reset(false);
   }
 

   // FIXME: For now just start over again.
   m_currentMapIt = beginIt;
//   if ( m_currentMapIt < curIt ) {
//      m_currentMapIt = beginIt;
//   } else {
//      m_currentMapIt = curIt;
//   }
   m_endMapIt = endIt;
   
}

inline static int text_type_ok_to_move( uint32 type )
{
   TileMapNameSettings::name_t typ = TileMapNameSettings::name_t(type);
   switch ( typ ) {
      case TileMapNameSettings::on_roundrect:
      case TileMapNameSettings::horizontal:
         return true;
      default:
         return false;
   }
}

void
TileMapTextHandler::updateCoordsForTextResults( vector<TextResult*>& vect,
                                                int angleDiff,
                                                bool trackingMode )
{
   for ( vector<TextResult*>::iterator it = vect.begin();
         it != vect.end();
         ++it ) {
      TextResult* tr = *it;
      vector<TextPlacementNotice>& tpn = tr->getTPN();

      if ( ! ( trackingMode ||
               text_type_ok_to_move( tr->getTypeOfString() ) ) ) {
         // Remove the notice
         m_mapPlotter->deleteString( (*it)->m_string );
         delete *it;
         it = vect.erase(it);
         --it;
         continue;
      }
      
      if ( ( tr->getTypeOfString() == TileMapNameSettings::on_roundrect ) &&
           !tpn.empty() ) {
         // Also move the sign. Will only work if the string is in the
         // middle of the sign.
         const isab::Rectangle& rect = tr->getStringAsRect();
         MC2Point tmpPoint(0,0);
         vector<TextPlacementNotice>::iterator jt = tpn.begin();
         m_matrix.transformPointInternalCosLat( tmpPoint,
                                                jt->m_worldCoord );         
         // Set a new rectangle centered around the text.
         tr->setStringAsRect( isab::Rectangle( tmpPoint.getX() -
                                               (rect.getWidth() >> 1),
                                               tmpPoint.getY() -
                                               (rect.getHeight() >> 1),
                                               rect.getWidth(),
                                               rect.getHeight() ) );
      }
      bool upsideDown = false;
      for ( vector<TextPlacementNotice>::iterator jt = tpn.begin();
            jt != tpn.end();
            ++jt ) {
         m_matrix.transformPointInternalCosLat( jt->m_point,
                                                jt->m_worldCoord );
         // Only adjust the angle for the ones on lines.
         if ( tr->getTypeOfString() != TileMapNameSettings::on_line ) {
            continue;
         }

         jt->m_angle += angleDiff;
         while ( jt->m_angle > 360 ) {
            jt->m_angle += 360;
         }
         jt->m_angle %= 360;
         if ( jt->m_angle > 90 && jt->m_angle < 270 ) {
            // Upside down.
            upsideDown = true;
            jt->m_angle += 180;
            jt->m_angle %= 360;
         }
      }
      if ( upsideDown ) {
         // Reverse the text - not working properly, the spaces are wrong.
         int first = 0;
         int last  = tpn.size() - 1;
         while ( first < last ) {
            std::swap( tpn[first].m_worldCoord, tpn[last].m_worldCoord );
            std::swap( tpn[first].m_point,      tpn[last].m_point );
            ++first;
            --last;
         }
      }
   }
}

void 
TileMapTextHandler::mapVectorChanged( const MapsToDrawIt& beginIt,
                                      const MapsToDrawIt& endIt,
                                      int trackingMode )
{
   float matrixScale = m_matrix.getPixelScale();
#ifdef MOVE_HORIZONTAL_TEXTS_EVEN_IF_NOT_TRACKING
   int keep = true;
#else
   int keep = trackingMode;
#endif
   if ( keep && ( matrixScale != m_oldScale ) ) {
      // Check some stuff if the scale has changed.
      // Cannot keep if one of the values are zero. Otherwise we
      // will divide by zero.
      keep = ( matrixScale != 0.0 ) && ( m_oldScale != 0.0 );
      if ( keep ) {
         // Now check how much the scale has changed.
         float scaleQuotient = matrixScale/m_oldScale;
         if ( scaleQuotient < 1.0 ) {
            scaleQuotient = 1.0 / scaleQuotient;
         }
         keep = scaleQuotient < 1.2;
      }
   }
   if ( keep ) {
      // Use the best list and move some texts around.
      if ( m_textResults.size() > m_textResultsBeforeNewMapArrived.size() ) {
         m_textResultsBeforeNewMapArrived.swap( m_textResults );
      }
      // Get the new angle.
      int angleDiff = int( m_matrix.getAngle() - m_angle );
      updateCoordsForTextResults(
         m_textResultsBeforeNewMapArrived, angleDiff, trackingMode );
      updateCoordsForTextResults( m_textResults, angleDiff, trackingMode );
      // Compensate for the angle difference.
      m_angle += angleDiff;
      reset(0);
   } else {
      // New texts are needed.
      reset();
   }   
   m_currentMapIt = beginIt;
   m_endMapIt = endIt;   

}
 
int
TileMapTextHandler::needsToRun()
{

#ifdef USE_TEXT_HANDLIND
   mc2dbg2 << "[TMTH] needsToRun()." << endl;
   mc2dbg4 << "[TMTH] we  have: " << m_textResults.size()
          << " nbr strings to place." << endl;

   if ( m_currentMapIt != m_endMapIt ) {
      return true; 
   } else {
      // Set the angle and scale to the correct values.
      m_angle = m_matrix.getAngle();
      m_oldScale = m_matrix.getPixelScale();
      // Be sure that we use the new text results.
      deleteAndClear( m_textResultsBeforeNewMapArrived );
      return false;
   }
#else
   return false;
#endif

}

int
TileMapTextHandler::doTheNeedful()
{
#ifdef USE_TEXT_HANDLIND

   mc2dbg2 << "[TMTH] -+-+-+-+-+-+- Do the needful. -+-+-+-+-+-+-+-" <<
      endl << endl;
  
   // Make sure nothing has happened since needsToRun was last called.
   if ( ! needsToRun() ) {
      return 0;
   }
   
   // placera lite text. 
   // börja på m_currentMapIndex kartan och i den på 
   // m_currentTextIndexInAllMap
   // som skall vara den text i den aktuella kartan som vi vill processa

   m_state = placingText;
   
   switch (m_state) {
      case ( placingText ): 
         placeSomeText();
         break;

      default:
         break;
   }
#endif
   return 0;
}

void
TileMapTextHandler::reset( int clear_m_textResultsBeforeNewMapArrived )
{
   m_currentTextIndexInMap = 0;

   // Delete the textnotices.
   if( clear_m_textResultsBeforeNewMapArrived ) {
      deleteAndClear( m_textResultsBeforeNewMapArrived );
   }

   deleteAndClear( m_textResults );

   m_overlapDetector.clear();
   m_stringsThatHaveBeenDrawnAlready.clear();
   isab::Rectangle r;
   m_mapPlotter->getMapSizeDrawingUnits( r );
   m_screenCoords = r;
}

inline MC2Coordinate&
TileMapTextHandler::toWorld(MC2Coordinate& dest, const MC2Point& src)
{
   m_matrix.inverseTranformCosLatSupplied( dest, src.getX(), src.getY(),
                                           m_matrix.getCosLat(
                                              m_matrix.getCenter().lat ) );
   return dest;
}

inline int TileMapTextHandler::testAndAddOverlap( const PixelBox& inBox )
{
   return m_overlapDetector.addIfNotOverlapping( inBox );
}

inline int TileMapTextHandler::testAndAddOverlaps( 
      const vector<PixelBox>& inBoxes )
{
   return m_overlapDetector.addIfNotOverlapping( inBoxes );
}


void
TileMapTextHandler::placeSomeText()
{
   STRING* nameOfFeature;
   const TileMap* stringMap;
   TileMap* dataMap;
  
   dataMap = *m_currentMapIt;
   stringMap = m_currentMapIt.getStringMap();
  
   if ( dataMap == NULL || stringMap == NULL || 
        m_currentTextIndexInMap >= stringMap->getNbrFeaturesWithText() ) {
      moveToNextString();
      return;
   }
 
   MC2_ASSERT( dataMap != NULL );
   MC2_ASSERT( stringMap != NULL );

   int featureIndex = stringMap->getFeatureIdxInTextOrder(
         m_currentTextIndexInMap );

   const char* tmpString =
      stringMap->getStringForFeature( featureIndex );
   if ( ( tmpString == NULL ) || ( tmpString[0] == '\0') ) {
      mc2dbg4 << "( tmpString == NULL ) || ( *tmpString == '' )" << endl;

      moveToNextString();
      return;
   }
  
   TileFeature* feature = 
      const_cast<TileFeature*> (dataMap->getFeature( featureIndex ));
   if ( feature == NULL ) {
      moveToNextString();
      return;
   }

   // Get the primitive.
   TilePrimitiveFeature* prim = dataMap->getOnePrimitive( *feature );
   if ( prim == NULL || ( ! prim->isDrawn() ) ) {
      // Primitive NULL or was not drawn. 
      // Then it shouldn't get any text either.
      moveToNextString();
      return;
   }
   
   // Create the screen coordinates.
   const VectorProxy<MC2Point>& pointsInFeature = 
                  m_mapHandler.getScreenCoords( *dataMap, *prim );
 
   // Start allocating stuff since we should now place text.
   

   MC2SimpleString tmpSimpleString( tmpString );
   const MC2SimpleString* theString = &tmpSimpleString;
   nameOfFeature = m_mapPlotter->createString( theString->c_str() );
   
   uint16 type = feature->getType();
  
//   mc2dbg4 << "Name: " << *nameOfFeature << " nbrPoints "
//      << (int)pointsInFeature.size() << endl;
   
   int retval = 0;
   if ( ! pointsInFeature.empty() ) {
      if( m_stringsThatHaveBeenDrawnAlready.find( *theString ) == 
          m_stringsThatHaveBeenDrawnAlready.end() ) {
         switch ( type ) {
            case ( TileFeature::city_centre_2 ):
            case ( TileFeature::city_centre_4 ):
            case ( TileFeature::city_centre_5 ):
            case ( TileFeature::city_centre_7 ):
            case ( TileFeature::city_centre_8 ):
            case ( TileFeature::city_centre_10 ):
            case ( TileFeature::city_centre_11 ):
            case ( TileFeature::city_centre_12 ):
               retval = placeHorizontal( 
                  nameOfFeature,
                  pointsInFeature, type );
            break;
            case ( TileFeature::street_class_0 ):
            case ( TileFeature::street_class_0_level_0 ): {
               bool roundRect = false;
               // Check that at least one char is a digit.
               for ( uint32 i = 0; i < theString->length(); ++i ) {
                  if ( theString->c_str()[ i ] >= '0' &&
                       theString->c_str()[ i ] <= '9' ) {
                     roundRect = true;
                     break;
                  }
               }
               // Cannot be too long string.
               // Otherwise for instance "Vei Merket Mot E18" 
               // gets a roundrect.
               if ( theString->length() > 6 ) {
                  roundRect = false;
               }
               if ( roundRect ) {   
                  // XXX: Skip E-roads if the scalelevel is too high.
                  if ( m_mapHandler.getPixelScale() > 50.0 ) {
                     break;
                  }
                  // Place roundrect.
                  retval = placeOn_RoundRect( 
                     nameOfFeature,
                     pointsInFeature, type );
               } else {
                  // Place on line.
                  retval = placeOn_Line( 
                     nameOfFeature,
                     pointsInFeature, type,
                     m_mapHandler.getPolylineOuterPixelWidth( *prim ) );
               }
            } break;
            case ( TileFeature::street_class_1 ):
            case ( TileFeature::street_class_2 ):
            case ( TileFeature::street_class_3 ):
            case ( TileFeature::street_class_4 ):
            case ( TileFeature::street_class_1_level_0 ):
            case ( TileFeature::street_class_2_level_0 ):
            case ( TileFeature::street_class_3_level_0 ):
            case ( TileFeature::street_class_4_level_0 ): {
               retval = placeOn_Line( 
                  nameOfFeature,
                  pointsInFeature, type,
                  m_mapHandler.getPolylineOuterPixelWidth( *prim ) );
               break;
            } 
            case ( TileFeature::water ):
            case ( TileFeature::park ):
               retval = placeInsidePolygon( 
                  nameOfFeature,
                  pointsInFeature, type );
               break;
            default:            
               break;
         }
         if( retval == 1) {
            mc2dbg4 << "inserting string into " <<
               "stringsThatHaveBeenDrawnAlready" << endl;
            m_stringsThatHaveBeenDrawnAlready.insert( *theString );
         }
      }
   }
  
   if ( retval == 0 ) {
      m_mapPlotter->deleteString( nameOfFeature );
   }
   
   moveToNextString();

   // When removing the code above in favor for the code below, 
   // also change in tilemaphandler drawtext.
/*   const SimpleArg* nameArg =
      static_cast<const SimpleArg*>(
            feature->getArg(TileArgNames::name_type));
mc2dbg << "feature->getArg(TileArgNames::name_type)) " <<
   feature->getArg(TileArgNames::name_type) << endl;
  
   if ( nameArg != NULL ) {
       mc2dbg << "nameArg->getValue(): " << nameArg->getValue() << endl;
      int nameArgVal = nameArg->getValue();
      switch ( nameArgVal ) {
         case TileMapNameSettings::horizontal:
            placeHorizontal( nameOfFeature, pointsInFeature, nameArgVal );
            break;

         case TileMapNameSettings::on_roundrect:
            placeOn_RoundRect( nameOfFeature, pointsInFeature, nameArgVal );
            break;

         case TileMapNameSettings::on_line:
            placeOn_Line( nameOfFeature, pointsInFeature, nameArgVal );
            break;

         case TileMapNameSettings::inside_polygon:
            placeInsidePolygon( nameOfFeature, pointsInFeature, nameArgVal);
            break;

         default:
            moveToNextString();
            break;
      }
   }
   else{
      mc2dbg << "Nameargs == NULL" << endl;
   }
   m_currentTextIndexInMap++;
  */
}

int
TileMapTextHandler::placeInsidePolygon(
      STRING* nameOfFeature,
      const VectorProxy<MC2Point>& pointsInFeature,
      uint16 /*type*/ )
{
  // mc2dbg2 << "[TMTH] name: " << *nameOfFeature << endl;

   int max_x = 0; 
   int max_y = 0;
   int min_x = m_screenCoords.getMaxLon();
   int min_y = m_screenCoords.getMaxLat();
   for( int i = 0; i < (int)pointsInFeature.size(); i++ ) {
      int x = pointsInFeature[i].getX();
      int y = pointsInFeature[i].getY();
      if( x > max_x ) { max_x = x; }
      if( y > max_y ) { max_y = y; }
      if( x < min_x ) { min_x = x; }
      if( y < min_y ) { min_y = y; }
   }
    
   if( max_x > m_screenCoords.getMaxLon() || 
       max_y > m_screenCoords.getMaxLat() ||  
       min_x < m_screenCoords.getMinLon() || 
       min_y < m_screenCoords.getMinLat() )
   return 0;
  
   MC2Point posOfString( ( min_x + max_x ) / 2,
                         ( min_y + max_y ) / 2 );
   
   if( !InsideUtil::inside(
            pointsInFeature.begin(),
            pointsInFeature.end(),
            posOfString ) ) {
      return 0;
   }
   
   if( posOfString.getX() == 0 && posOfString.getY() == 0 ){
      return 0;
   }
  
   // Collect and send the results to our textresults vector.
   TextResult* tr = new TextResult();
   tr->setString( nameOfFeature );
   const STRING* font =
      getPlotterStr(m_textSettings.getInsidePolygonFont().first);
   tr->setFontName( font );
   tr->setFontColor( (m_textColorNonRoundRects >> 16 ) & 0xff,
                     (m_textColorNonRoundRects >> 8 ) & 0xff,
                     (m_textColorNonRoundRects >> 0 ) & 0xff );
   tr->setTypeOfString( TileMapNameSettings::inside_polygon );
   tr->setFontSize( m_textSettings.getInsidePolygonFont().second );
   
   m_mapPlotter->setFont( *font, tr->getFontSize() );
   PixelBox stringRectangle =
      m_mapPlotter->getStringAsRectangle( *nameOfFeature, posOfString);

   MC2Coordinate worldCoord;
   
   
   // Add the TextPlacementNotice to the TextResult.
   tr->addTPN( TextPlacementNotice ( posOfString,
                                     // toWorld returns worldCoord
                                     toWorld(worldCoord, posOfString), 
                                     0, -1, 0 ) );

   if( stringRectangle.inside( m_screenCoords ) &&
         testAndAddOverlap( stringRectangle ) ) {
      addTextResult( tr );
   } else {
      delete tr;
      return 0;
  }
   mc2dbg8 << "[TMTH] we now have: " << m_textResults.size() 
      << " nbr of strings to place." << endl;
   return 1;
} 


int
TileMapTextHandler::placeOn_RoundRect(
      STRING* nameOfFeature,
       const VectorProxy<MC2Point>& pointsInFeature,
      uint16 /*type*/ )
{
   
   //mc2dbg2 << "[TMTH] name: " << *nameOfFeature << endl;

   MC2Point posOfString ( pointsInFeature.front() );
   
   if( posOfString.getX() == 0 && posOfString.getY() == 0 ){
      return 0;
   }
   // Collect and send the results to our textresults vector.
   const STRING* font =
      getPlotterStr( m_textSettings.getRoundRectFont().first );
   
   TextResult* tr = new TextResult();
   tr->setString( nameOfFeature );
   tr->setFontName( font );
   tr->setFontColor( 255, 255, 255 );
   tr->setTypeOfString( TileMapNameSettings::on_roundrect );
   tr->setFontSize( m_textSettings.getRoundRectFont().second );  
   
   m_mapPlotter->setFont( *font, tr->getFontSize() );
   isab::Rectangle stringRectangle( 
         m_mapPlotter->getStringAsRectangle(
               (*nameOfFeature), posOfString));
   int border_size = 3;
   int x1 = stringRectangle.getX() - border_size;
   int x2 = stringRectangle.getWidth() + ( border_size * 2 );
   int y1 = stringRectangle.getY() - border_size;
   int y2 = stringRectangle.getHeight() + ( border_size * 2 );
   stringRectangle = isab::Rectangle( x1, y1, x2, y2 );
   tr->setStringAsRect ( stringRectangle );

   MC2Coordinate worldCoord;
   
   TextPlacementNotice tpn ( posOfString,
                             // toWorld returns worldCoord after converting.
                             toWorld(worldCoord, posOfString),
                             0, -1, 0 );

   // Add the TextPlacementNotice to the TextResult.
   tr->addTPN( tpn );

   if( PixelBox( stringRectangle ).inside( m_screenCoords ) &&
         testAndAddOverlap( stringRectangle ) ) { 
           addTextResult( tr );
   } else {
      delete tr;
      return 0;
  }
   mc2dbg8 << "[TMTH] we now have: " << m_textResults.size() 
      << " nbr of strings to place." << endl;
   return 1;
} 

int
TileMapTextHandler::placeOn_Line(
      STRING* nameOfFeature,
      const VectorProxy<MC2Point>& proxy_pointsInFeature,
      uint16 type,
      uint32 lineWidth )
{
   vector<MC2Point> pointsInFeature;
   for( int i=0; i < (int)proxy_pointsInFeature.size(); i++ ) {
      pointsInFeature.push_back( proxy_pointsInFeature[ i ] );
   }
   
//   int isInside=0;
//   for( int i=0; i < (int)proxy_pointsInFeature.size(); i++ ) {
//      /*
//       * This clipping is not done in a correct way but it is probably
//       * as good as we need it to be. It will remove any segments
//       * that dosent have any points inside the screen.
//       */
//      int m_screenCoords_pointInside_proxy_pointsInFeature_i_1 = 0;
//      if( i > 1 && 
//            m_screenCoords.pointInside(proxy_pointsInFeature[i-1]) ) {
//         m_screenCoords_pointInside_proxy_pointsInFeature_i_1 = 1;
//      }
//      if( (m_screenCoords.pointInside(proxy_pointsInFeature[i]) ||
//           m_screenCoords.pointInside(proxy_pointsInFeature[i+1])) ||
//          (m_screenCoords.pointInside(proxy_pointsInFeature[i]) ||
//           m_screenCoords_pointInside_proxy_pointsInFeature_i_1)) {
//         pointsInFeature.push_back( proxy_pointsInFeature[ i ] );
//         isInside = 1;
//      } else if( isInside ) {
//         i = (int)proxy_pointsInFeature.size();    
//      }
//   }

   vector<int> segmentWidths;
   int longestSegmentInFeature =
      getLongestSegmentInFeature( pointsInFeature );

   if( longestSegmentInFeature == -1 ) {
      return 0;
   }

   int lsifLength = Pow2DistBetweenPoints( 
         pointsInFeature[ longestSegmentInFeature ],
         pointsInFeature[ longestSegmentInFeature + 1 ] ); 
 
   const STRING* font = getPlotterStr(
      m_textSettings.getLineFont( lineWidth ).first );

   const int fontSize = m_textSettings.getLineFont( lineWidth ).second;
   
   m_mapPlotter->setFont( *font, fontSize );
   
   int stringWidth = getTextWidth(
         nameOfFeature, 0, -1 );
 
   int lengthOfString = m_mapPlotter->getStringLength( *nameOfFeature );
   vector<int> charsOnSegment( (int)pointsInFeature.size() - 1 );
   for( int j=0; j < (int)pointsInFeature.size() - 1; j++) {
      charsOnSegment[ j ] = 0;
   }
 
   stringWidth = Pow2( stringWidth );
   if( lsifLength > stringWidth ) {
      // Trying to place on the longest segment in the feature.
      charsOnSegment[ longestSegmentInFeature ] = lengthOfString;
   } else {
      // XXX:
      //return 0; 
      for( int i = 0; i < (int)pointsInFeature.size() - 1; i++ ) {
         segmentWidths.push_back(
               Pow2DistBetweenPoints(
                  pointsInFeature[ i ],
                  pointsInFeature[ i + 1 ] ) ); 
      }
      
      int bestCostSoFar = INT_MAX;

      for( int segment = 0; 
            segment < (int)segmentWidths.size();         
            segment++ ) {
         vector<int> tempCharsOnSegment;
         tempCharsOnSegment.resize( (int)segmentWidths.size() );

         int nbrOfCharsPossitioned = FillRestOfCharsOnSegment(
               nameOfFeature,
               segmentWidths, 
               tempCharsOnSegment, 
               lengthOfString, 
               segment, 0 );
         
         if( nbrOfCharsPossitioned == lengthOfString ) {
            int cost = Cost( 
                  nameOfFeature, 
                  segmentWidths,
                  tempCharsOnSegment,
                  pointsInFeature ); 
            if( nbrOfCharsPossitioned != lengthOfString ) {
               cost = INT_MAX;
            }
            
            if( cost < bestCostSoFar ) {
               bestCostSoFar = cost;
               tempCharsOnSegment.swap( charsOnSegment );
            } 
         } else {
            segment = (int)segmentWidths.size();
         }
      }
   }

   int firstSegment = -1;
   for( int k = 0; k < (int)charsOnSegment.size(); k++ ) {
      if( firstSegment == -1 && charsOnSegment[ k ] != 0 ) {
         firstSegment = k;
         k = (int)charsOnSegment.size();
      }
   }
   
   if ( firstSegment < 0 ) {
      return 0;
   }
   
   int reverseText = IsTextReversed( pointsInFeature, firstSegment );
   
   return placeOn_Segments( nameOfFeature, font, fontSize,
                            pointsInFeature,
                            type, charsOnSegment,
                            reverseText );
}

int
TileMapTextHandler::getTextWidth( 
      STRING* nameOfFeature,
      int startIndex,
      int nbrOfChars )
{
   if( nbrOfChars == -1 ) {
      nbrOfChars = 
         m_mapPlotter->getStringLength( *nameOfFeature ) - 
         startIndex;
      
   }
   if( nbrOfChars <= 0 ) { return 0; }

   MC2Point p( 0, 0 );
   return m_mapPlotter->getStringAsRectangle(
             *nameOfFeature, p, startIndex, nbrOfChars, 0 ).getWidth();
}

void
TileMapTextHandler::reversePoints(
       vector<MC2Point>& pointsInFeature )
{
   vector<MC2Point> retvec;
   int poifSize = (int)pointsInFeature.size();
   for( int i = 0; i < poifSize; i++ ) {
      retvec.push_back( 
            pointsInFeature.back() );
      pointsInFeature.pop_back();
   }
   retvec.swap( pointsInFeature );
}

int
TileMapTextHandler::IsTextReversed(
         const vector<MC2Point>& pointsInFeature,
         int firstSegment )
{
   const MC2Point &lineBegin = pointsInFeature[ firstSegment ];
   const MC2Point &lineEnd = pointsInFeature[ firstSegment + 1 ];
   int p1x = lineBegin.getX();  
   int p1y = lineBegin.getY();
   int p2x = lineEnd.getX();
   int p2y = lineEnd.getY();

   int16 angle = calcAngleForTextOnLine( p1x, p1y, p2x, p2y );
   if( angle == 9999 ) {
      mc2dbg << "angle == 9999" << endl;
   }
   if( ( angle > -90 ) && ( angle < 90 ) ) {
      return 1;
   }
   return 0;
}

int
TileMapTextHandler::Cost(
      STRING* nameOfFeature,
      const vector<int>& segmentWidths, 
      const vector<int>& charsOnSegment,
      const vector<MC2Point>& pointsInFeature )
{
   int startIdx = 0;
   int cost = 0;
   int totalAngle = 0;
   int atLeastOneSegmentHaveCharsThatNeedsPlacment = 0;
   // For all segments.
   for( int i = 0; i < (int)segmentWidths.size()-1; i++ ) {
      if( charsOnSegment[ i ] != 0 ) {
         atLeastOneSegmentHaveCharsThatNeedsPlacment = 1;
      }
      
      if( i > 0 && i < (int)segmentWidths.size() - 1 ) {
         if( ( charsOnSegment[ i - 1 ] != 0 ) &&
               ( charsOnSegment[ i ] == 0 ) &&
               ( charsOnSegment[ i + 1 ] != 0 ) ) {
            mc2dbg << "We have atleast one segment that are empty within "
               << "the segments that we are placing text on." << endl;
            return INT_MAX;
         }
         int subStringOnSegWidth = getTextWidth(
               nameOfFeature, 
               startIdx, 
               charsOnSegment[ i ] );
         startIdx += charsOnSegment[ i ];
         if( ( charsOnSegment[ i ] != 0 ) &&
               ( charsOnSegment[ i - 1 ] != 0 ) &&
               ( charsOnSegment[ i + 1 ] != 0 ) ) {
            cost += abs(int( Pow2( subStringOnSegWidth ) - 
                  segmentWidths[ i ] ) );
         }
      }
      if( i < (int)segmentWidths.size() - 1 ) {
         if( ( charsOnSegment[ i ] != 0 ) &&
               ( charsOnSegment[ i + 1 ] != 0 ) ) {
            int p1x = pointsInFeature[ i ].getX();
            int p1y = pointsInFeature[ i ].getY();
            int p2x = pointsInFeature[ i + 1 ].getX();
            int p2y = pointsInFeature[ i + 1 ].getY();
            int a12Cost = calcAngleForTextOnLine( p1x, p1y, p2x, p2y );
            int p3x = pointsInFeature[ i + 2 ].getX();
            int p3y = pointsInFeature[ i + 2 ].getY();
            int a23Cost = calcAngleForTextOnLine( p2x, p2y, p3x, p3y );

            int a13Cost = abs(int( a12Cost - a23Cost ) );
            if ( a13Cost > 180 ) { a13Cost = 360 - a13Cost; }
            totalAngle += a13Cost;
            if ( a13Cost > m_maxAngle || totalAngle > m_maxTotalAngle ) {
               return INT_MAX;
            }
            cost += a13Cost;
         }
      }
      if( cost > m_maxTotalCost ) {
         return INT_MAX;
      }
   }
   if( atLeastOneSegmentHaveCharsThatNeedsPlacment ) {
      return cost;
   } else {
      return INT_MAX;
   }
}

int
TileMapTextHandler::FillRestOfCharsOnSegment(
            STRING* nameOfFeature,
            vector<int>& segmentWidths,
            vector<int>& tempCharsOnSegment,
            int stringLength,
            int startSegment,
            int startChar )
{
   for( int i = startSegment; i < (int)segmentWidths.size(); i++ ) {
      if( startChar >= stringLength ) {
         return startChar;
      }
      tempCharsOnSegment[ i ] = 
         NbrOfCharsThatFitsInSeg(
                nameOfFeature, stringLength,
            segmentWidths[ i ], startChar );
      startChar += tempCharsOnSegment[ i ];
      if( tempCharsOnSegment[ i ] == 0 ) {
         return -1;
      }
   }
   return startChar;
}

int
TileMapTextHandler::NbrOfCharsThatFitsInSeg(
      STRING* nameOfFeature,
      int stringLength,
      int segmentWidth,
      int startChar )
{  
   if( startChar >= stringLength ) {
      return 0;
   }
   if( segmentWidth <= 0 ) {
      return 0;
   }
   int nbrOfCharsUsedSoFar = 0;
   int charsWidth = getTextWidth( nameOfFeature, 
         startChar, nbrOfCharsUsedSoFar );
   
   int firstCharWidth = getTextWidth( nameOfFeature,
         startChar, 1 );
   int lastCharWidth = getTextWidth( nameOfFeature,
         startChar + nbrOfCharsUsedSoFar, 1 );

   if( startChar + nbrOfCharsUsedSoFar >= stringLength ) {
      return stringLength - startChar;
   }

   while( segmentWidth > Pow2( charsWidth + 
            ( firstCharWidth + lastCharWidth ) / 2 ) ) {

      nbrOfCharsUsedSoFar++;
      if( startChar + nbrOfCharsUsedSoFar >= stringLength ) {
         return stringLength - startChar;
      }
      charsWidth = getTextWidth( nameOfFeature, 
         startChar, nbrOfCharsUsedSoFar );
      lastCharWidth = getTextWidth( nameOfFeature,
         startChar + nbrOfCharsUsedSoFar, 1 );
   }
   return nbrOfCharsUsedSoFar;
}

int
TileMapTextHandler::Pow2( int a ) { return a*a; }

int
TileMapTextHandler::Pow2DistBetweenPoints( 
      MC2Point p1, 
      MC2Point p2 )
{
   return Pow2( p1.getX() - p2.getX() ) + 
          Pow2( p1.getY() - p2.getY() );
}

int
TileMapTextHandler::placeOn_Segments(
      STRING* nameOfFeature,
      const STRING* font,
      int fontSize,
      const vector<MC2Point>& pointsInFeature,
      uint16 /*type*/,
      vector<int>& charsOnSegment,
      int reverseText )
{
   TextResult* tr = new TextResult();
   m_mapPlotter->setFont( *font, fontSize );
   vector<PixelBox> stringPixelBoxes;
   int textStartIdx = 0; 
   // let the direction of the text be controled by the line
   // drawn by pointsInFeature[ 0 ] and 
   // pointsInFeature[ (int)pointsInFeature.size()-1 ].
   int first=-1;
   int last=-1;
   for( int i = 0; i < (int)charsOnSegment.size(); i++ ) {
      if( first == -1 && charsOnSegment[ i ] != 0 ) {
         first = i;
      }
      if( charsOnSegment[ i ] != 0 ) {
         last = i;
      }
   }
   if( first == last) {
      first = -1;
      last = -1;
   }
   int lengthOfString = m_mapPlotter->getStringLength( *nameOfFeature );
   for( int segmentNbr = 0; 
         segmentNbr < (int)charsOnSegment.size(); 
         segmentNbr++ ) {
      if( charsOnSegment[ segmentNbr ] != 0 ){ 
         MC2Point lineBegin = pointsInFeature[ segmentNbr ];
         MC2Point lineEnd = pointsInFeature[ segmentNbr + 1 ];
        
         int p1x = lineBegin.getX();  
         int p1y = lineBegin.getY();
         int p2x = lineEnd.getX();
         int p2y = lineEnd.getY();

         int16 angle = calcAngleForTextOnLine( p1x, p1y, p2x, p2y );
         if( angle == 9999 ) {
            mc2dbg << "angle == 9999" << endl;
            delete( tr );
            return 0;
         }
         
         // Align al texts on a segment in the same way as the first one.
         if ( reverseText == 0 ) {
            angle += 180;
         }
         // Varun's angle increase ccw.
         angle = -angle;
         if ( angle < 0 ) {
            angle += 360; 
         }
         
         // Collect and send the results to our textresults vector.
         tr->setString( nameOfFeature );
         tr->setFontName( font );
         tr->setFontColor( (m_textColorNonRoundRects >> 16 ) & 0xff,
                           (m_textColorNonRoundRects >> 8 ) & 0xff,
                           (m_textColorNonRoundRects >> 0 ) & 0xff );
         tr->setTypeOfString( TileMapNameSettings::on_line );
         tr->setFontSize( fontSize );
         vector<isab::Rectangle> stringRectangles;
         int posX = 0;
         int posY = 0;
         int textFirstIdx = 0;
         for( int i = 0; i < first; i++ ) {
            textFirstIdx += charsOnSegment[ i ];
         }
         int textLastIdx = 0;
         for( int j = 0; j < last; j++ ) {
            textLastIdx += charsOnSegment[ j ];
         }
         
         // Put text to the left.
         if( segmentNbr == first ) {
            mc2dbg4 << "left" << endl;
            // Compensate for the angle between line 1 and 2.
            double angleDiff = 0;
            if(  segmentNbr + 2 < (int)pointsInFeature.size() ) {
               int fp1x = pointsInFeature[ segmentNbr ].getX();
               int fp1y = pointsInFeature[ segmentNbr ].getY();
               int fp2x = pointsInFeature[ segmentNbr + 1 ].getX();
               int fp2y = pointsInFeature[ segmentNbr + 1 ].getY();
               int fp3x = pointsInFeature[ segmentNbr + 2 ].getX();
               int fp3y = pointsInFeature[ segmentNbr + 2 ].getY();

               angleDiff =
                  calcAngleForTextOnLine( fp1x, fp1y, fp2x, fp2y ) -
                  calcAngleForTextOnLine( fp2x, fp2y, fp3x, fp3y );
               angleDiff = abs(int( (int)angleDiff ) );
               if ( angleDiff > 180 ) { 
                  angleDiff = 360 - angleDiff; 
               }
            } 
            double subTextWidth = getTextWidth( nameOfFeature,
                  textFirstIdx, charsOnSegment[ segmentNbr ] ); 

            // 15 is a magic constant.
            double faktor = ( subTextWidth + ( angleDiff / 
                     m_magicConst ) ) / ( sqrt( (double) 
                        Pow2DistBetweenPoints( lineBegin, lineEnd ) ) * 2.0 );
            posX = (int)( ( lineBegin.getX() * faktor ) + 
                  ( lineEnd.getX() * ( 1 - faktor ) ) + 0.0 );
            posY = (int)( ( lineBegin.getY() * faktor ) + 
                  ( lineEnd.getY() * ( 1 - faktor ) ) + 0.0 );

            // Put text to the right.
         } else if( segmentNbr == last ) {
            mc2dbg4 << "right" << endl;
            double angleDiff = 0;
            int lp1x = pointsInFeature[ segmentNbr - 1 ].getX();
            int lp1y = pointsInFeature[ segmentNbr - 1 ].getY();
            int lp2x = pointsInFeature[ segmentNbr ].getX();
            int lp2y = pointsInFeature[ segmentNbr ].getY();
            int lp3x = pointsInFeature[ segmentNbr + 1 ].getX();
            int lp3y = pointsInFeature[ segmentNbr + 1 ].getY();

            angleDiff =
               calcAngleForTextOnLine( lp1x, lp1y, lp2x, lp2y ) -
               calcAngleForTextOnLine( lp2x, lp2y, lp3x, lp3y );
            angleDiff = abs(int( (int)angleDiff ) );
            if ( angleDiff > 180 ) { 
               angleDiff = 360 - angleDiff; 
            }

            double subTextWidth = getTextWidth( nameOfFeature,
                  textLastIdx, charsOnSegment[ segmentNbr ] ); 

            // 15 is a magic constant.
            double faktor = (subTextWidth + ( angleDiff / 
                     m_magicConst ) ) / ( sqrt( (double) 
               Pow2DistBetweenPoints( lineBegin, lineEnd ) ) * 2.0 );

            posX = (int)( ( lineBegin.getX() * ( 1 - faktor ) ) +
                  ( lineEnd.getX() * faktor ) + 0.0 );
            posY = (int)( ( lineBegin.getY() *  ( 1 - faktor ) ) +
                  ( lineEnd.getY() * faktor ) + 0.0 );

            // Put text in center.
         } else {
            mc2dbg4 << "center" << endl;
            posX = (int)( ( ( lineBegin.getX() +
                   lineEnd.getX() ) / 2 ) + 0.0 );
            posY = (int)( ( ( lineBegin.getY() + 
                   lineEnd.getY() ) / 2 ) + 0.0 );
         }
         MC2Point pos( posX, posY );
         if( reverseText == 0 ) {
               m_mapPlotter->getStringAsRectangles(
                     stringRectangles,
                  *nameOfFeature,
                  pos,
                  textStartIdx, 
                  charsOnSegment[ segmentNbr ],
                  angle );
         } else {
            m_mapPlotter->getStringAsRectangles(
                  stringRectangles,
                  *nameOfFeature,
                  pos,
                  lengthOfString - textStartIdx -
                  charsOnSegment[ segmentNbr ],
                  charsOnSegment[ segmentNbr ],
                  angle );
         }
         for( int k = 0; k < (int)stringRectangles.size(); k++ ) { 
            stringPixelBoxes.push_back( stringRectangles[ k ] );
         }
         
         MC2Point posOfString( posX, posY );
         MC2Coordinate worldCoord;
         toWorld( worldCoord, posOfString );
         
         if( reverseText == 0 ) {
            TextPlacementNotice tpn ( posOfString,
                                      worldCoord,
                                      textStartIdx, 
                                      charsOnSegment[ segmentNbr ], 
                                      angle );
            tr->addTPN( tpn );
         } else {
            TextPlacementNotice tpn = 
               TextPlacementNotice( posOfString,
                                    worldCoord,
                                    lengthOfString - textStartIdx - 
                                    charsOnSegment[ segmentNbr ], 
                                    charsOnSegment[ segmentNbr ],
                    angle );
            tr->addTPN( tpn );
         }
         textStartIdx += charsOnSegment[ segmentNbr ];
      }
      mc2dbg4 << endl;
   }
   int nbrOfstringRectangles_inside_m_screenCoords = 0;
   for( int k = 0; k < (int)stringPixelBoxes.size(); k++ ) {
      if( !( stringPixelBoxes[k].inside(m_screenCoords) ) ) {
         nbrOfstringRectangles_inside_m_screenCoords++;
      }
   }
  
   int halfNbrOfRectangles = (int)stringPixelBoxes.size() / 2;

   // This should be fetched from the MapPlotter for this actual
   // text. Right to left text means "wrong" drawing order, i.e.
   // arabic, hebrew or similar. We only support placing text in
   // the wrong drawing order with one text segment (since ABC DEF
   // -> CBA FED instead of FEDCBA)
   bool leftToRightText = 
      m_mapPlotter->textDirectionLeftToRight( *nameOfFeature );
   

   if( tr->getTPN().size() > 0 &&
       nbrOfstringRectangles_inside_m_screenCoords < 
       halfNbrOfRectangles && 
       testAndAddOverlaps( stringPixelBoxes ) &&
       // OK, if wrong text drawing order and only one string
       (!leftToRightText && tr->getTPN().size() == 1 ||
        // Also OK if normal text drawing order and many strings
        leftToRightText )) {
      addTextResult( tr );
   } else {
      delete tr;
      return 0;
   }
   mc2dbg4 << "[TMTH] we now have: " << m_textResults.size() 
      << " nbr of strings to place." << endl;
   return 1;
} 

int
TileMapTextHandler::placeHorizontal(
      STRING* nameOfFeature,
      const VectorProxy<MC2Point>& pointsInFeature,
      uint16 /*type*/ )
{
   mc2dbg4 << "[TMTH] Placing Horizontal" << endl;
   //mc2dbg4 << "[TMTH] name: " << *nameOfFeature << endl;
   
   // For city centers use the first point in the feature.
   MC2Point posOfString ( pointsInFeature.front().getX(), 
                          pointsInFeature.front().getY() + 
                           c_heightDisplacmentOftextOverCityCenter );
   
   TextResult* tr = new TextResult();

   const STRING* font = getPlotterStr(
      m_textSettings.getHorizontalFont().first );
   
   tr->setString( nameOfFeature );
   tr->setFontName( font );
   tr->setFontColor( (m_textColorNonRoundRects >> 16 ) & 0xff,
                     (m_textColorNonRoundRects >> 8 ) & 0xff,
                     (m_textColorNonRoundRects >> 0 ) & 0xff );
   tr->setTypeOfString( TileMapNameSettings::horizontal );
   tr->setFontSize( m_textSettings.getHorizontalFont().second );
   m_mapPlotter->setFont( *font, tr->getFontSize() );

   isab::Rectangle stringRectangle( m_mapPlotter->getStringAsRectangle( 
            (*nameOfFeature), posOfString));

   MC2Coordinate worldCoord;
   
   TextPlacementNotice tpn ( posOfString,
                             toWorld(worldCoord, posOfString),
                             0, -1, 0 );
   
   // Add the TextPlacementNotice to the TextResult.
   tr->addTPN( tpn );
   if( PixelBox( stringRectangle ).inside( m_screenCoords ) && 
         testAndAddOverlap( stringRectangle ) ) {
      addTextResult( tr );
   } else {
      delete( tr );
      return 0;
   }
   return 1;
}

int16 
TileMapTextHandler::calcAngleForTextOnLine( 
      int p1x, 
      int p1y, 
      int p2x, 
      int p2y )
{
   double xDiff = p1x - p2x;
   double yDiff = p1y - p2y;
   
   if ( xDiff == 0 && yDiff == 0 ) {
      // x and y diff is 0 means no angle.
      return 9999;
   }
  
   // Calculate at what angle the line p1-p2 is.
#ifdef __SYMBIAN__
   TReal atanRes;
   Math::ATan( atanRes, yDiff, xDiff );
#else
   double atanRes = atan2( yDiff, xDiff );
#endif
   int16 angle = (int16)( ( atanRes / M_PI ) * 180 );
   return angle;
}
 
const vector<TextResult*>& 
TileMapTextHandler::getTextResults() 
{
   if( m_textResultsBeforeNewMapArrived.size() <
         m_textResults.size()){
      return m_textResults;
   }
   return m_textResultsBeforeNewMapArrived;
}

const  
int TileMapTextHandler::getLongestSegmentInFeature( 
      const vector<MC2Point>& feature ) 
{
   double length = 0;
   int index = 0;
   const int featureSize = feature.size();
   if( featureSize >= 2 ) {      
      for ( int i = 0; i < featureSize - 1; ++i ) {
         // Calc the (²) distance between the endpoints of
         // the current segment in the feature.
         int xDiff = feature[ i ].getX() - feature[ i + 1 ].getX();
         int yDiff = feature[ i ].getY() - feature[ i + 1 ].getY();
         double current_length = (double)(Pow2( xDiff ) + Pow2( yDiff ));
         // If the length of this segment is longer than any that we 
         // have tested before, remember this one.
         if( current_length > length ) { 
            length = current_length;
            index = i;
         }
      }
   } else {
      return -1;
   }
   return index;
}


#ifdef MC2_SYSTEM
void TileMapTextHandler::getOverlapVector( 
      vector<PixelBox>& in_boxes )
{
   m_overlapDetector.getBoxes( in_boxes );
}
#endif

void 
TileMapTextHandler::addTextResult( TextResult* tr )
{
   m_textResults.push_back( tr );
}

void 
TileMapTextHandler::setTextColor( uint32 color )
{
   m_textColorNonRoundRects = color;
}

