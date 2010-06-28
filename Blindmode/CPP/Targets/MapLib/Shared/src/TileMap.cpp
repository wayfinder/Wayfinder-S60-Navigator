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
#include "TileMap.h"
#include "TileFeature.h"
#include "TileMapParams.h"
#include "TileMapTypes.h"
#include "TileMapFormatDesc.h"
#include "MC2BoundingBox.h"
#include "GfxConstants.h"
#include "TileArgNames.h"
#include "TileMapUtil.h"
#include "TileMapParamTypes.h"

#include "GunzipUtil.h"

#ifdef MC2_SYSTEM
#include "GzipUtil.h"
#include "MathUtility.h"
#endif

using namespace std;


TileMap::~TileMap()
{
   delete m_params;
   
   if ( m_primitives ) {
      delete [] m_primitives;
   }
   delete m_otherTypeOfMapParams;
   delete m_stringBuffer;
}

#ifdef MC2_SYSTEM
void 
TileMap::saveTileMapData( BitBuffer& buf ) const
{
   saveHeader( buf );  

   // Nbr features.
   buf.writeNextBits( m_features.size(), 31 );
   mc2dbg << "[TileMap]: save NbrFeatures = " << hex
      << m_features.size() << dec << endl;
   const TileFeature* prevFeature = NULL;
   for ( vector<TileFeature>::const_iterator it = 
         m_features.begin(); it != m_features.end(); ++it ) {
      it->save( buf, *this, prevFeature );
      // Blargh! Watch out for this.
      prevFeature = &(*it);
   }
}

bool 
TileMap::save( BitBuffer& inbuf ) const
{
   // Make new buffer to that reset will rewind to the start
   // of this map.
   // WARNING! Must skip bytes at the end too!
   BitBuffer buf(inbuf.getBufferAddress() + inbuf.getCurrentOffset(),
                  inbuf.getBufferSize() - inbuf.getCurrentOffset());
   
   TileMapTypes::tileMap_t tileMapType = m_params->getTileMapType();

   bool useGzip = m_params->useGZip();

   if ( useGzip ) {
      // The gunzipped parameters should not lie.
      m_params->setUseGZip(false);
   }
   buf.alignToByte();
   // Write an empty string so that the new client can use the
   // old server.
   //buf.writeNextString(m_params->getAsString().c_str());
   buf.writeNextString("");
   
   if ( tileMapType == TileMapTypes::tileMapData ) {
      saveTileMapData( buf );
   } else if ( tileMapType == TileMapTypes::tileMapStrings ) {
      
      // Save the strings.

      // Calculate the number of bits needed for representing 
      // the string index.
      uint32 nbrBitsStrIdx = MathUtility::getNbrBits( m_strings.size() );
      buf.writeNextBits( nbrBitsStrIdx, 4 );
      
      // Calculate the number of bits needed for representing 
      // the feature index.
      uint32 nbrBitsFeatureIdx = 
         MathUtility::getNbrBits( m_features.size() );
      buf.writeNextBits( nbrBitsFeatureIdx, 4 );
      
      // Write number of features.
      buf.writeNextBits( m_features.size(), nbrBitsFeatureIdx );
      
      // Write number of features with strings.
      buf.writeNextBits( m_featureIdxInTextOrder.size(), 
                         nbrBitsFeatureIdx );
      
      // Write number of strings.
      buf.writeNextBits( m_strings.size(), nbrBitsStrIdx );
      
      // Write pairs of ( feature index, string index ) in 
      // the order as specified by m_featureIdxInTextOrder.
      for ( vector<uint32>::const_iterator it = 
               m_featureIdxInTextOrder.begin(); 
            it != m_featureIdxInTextOrder.end(); ++it ) {
         // Feature index.
         buf.writeNextBits( *it, nbrBitsFeatureIdx );

         // String index.
         buf.writeNextBits( m_strIdxByFeatureIdx[ *it ], nbrBitsStrIdx );
      }

      // Write the strings.
      buf.alignToByte();
      for ( vector<MC2SimpleString>::const_iterator it = m_strings.begin();
            it != m_strings.end(); ++it ) {
         buf.writeNextString( it->c_str() );
      }
   }

   // All is written. Now align to next byte.
   buf.alignToByte();
  
   // Add CRC for the features, if present.
   if ( ! m_features.empty() ) {
      BitBuffer* crcBuf = new BitBuffer( inbuf.getBufferSize() );
      saveTileMapData( *crcBuf );
      crcBuf->alignToByte();
      uint32 crc = MC2CRC32::crc32( crcBuf->getBufferAddress(),
                                    crcBuf->getCurrentOffset() );
      buf.writeNextBALong( crc );
      delete crcBuf;
   }
   
   // Write the empty importances. 16 bits must be enough!
   buf.writeNextBAShort( m_emptyImportances );
 
   const int mapSize = buf.getCurrentOffset();
   // Compress the buffer if gzip should be used.
   if ( useGzip ) {      
      mc2dbg << "[TMap]: Trying gzip" << endl;
      int inLen = mapSize;
      
      // Outbuf should be as large as inbuf
      // The problem is that gzip does not stop if the buffer is too
      // small
      BitBuffer* compBuf = new BitBuffer(inLen*2+1024);
      int outLen =
         GzipUtil::gzip(compBuf->getBufferAddress(),
                        compBuf->getBufferSize(),
                        buf.getBufferAddress(),
                        inLen, 9);
      if ( (outLen > 0) && (outLen < inLen) ) {
         // Ugglehack!!
         buf.reset();
         buf.alignToByte();
         buf.writeNextByteArray(compBuf->getBufferAddress(), outLen);        
         mc2dbg << "[TMap]: Gzip has reduced the size of the map from "
                << inLen << " to " << outLen << " bytes" << endl;
      }
      delete compBuf;
   }

   // Align the buffer to the next byte since we might not have written
   // a complete byte last.
   buf.alignToByte();
   inbuf.readPastBytes(buf.getCurrentOffset());
   inbuf.alignToByte();

   //dump( mc2dbg );
   return true;

}
#endif

uint32
TileMap::readReceiveTime( const TileMapFormatDesc& tmfd,
                          const TileMapParams& params,
                          BitBuffer& buf )
{
   if ( tmfd.canExpire( params ) ) {
      return buf.readNextBAShort();
   } else {
      return MAX_UINT32;
   }
}

BitBuffer*
TileMap::writeReceiveTime( const TileMapFormatDesc& tmfd,
                           const MC2SimpleString& params,
                           BitBuffer* buf )
{
   // Don't write anything in a NULL buffer
   if ( buf == NULL ) {
      return buf;
   }
   // Only applicable for tiles.
   if ( TileMapParamTypes::getParamType( params ) !=
        TileMapParamTypes::TILE ) {
      return buf;
   }
   if ( ! tmfd.canExpire( params ) ) {
      // Cannot expire - return the same buffer.
      return buf;
   }
   // Can expire - add two bytes at the beginning of the buffer.
   BitBuffer* newBuf = new BitBuffer( buf->getBufferSize() + 2 );
   newBuf->writeNextBAShort( TileMapUtil::currentTimeMillis() / 1000 / 60 );
   newBuf->writeNextByteArray( buf->getBufferAddress(),
                               buf->getBufferSize() );
   newBuf->reset();
   delete buf;
   return newBuf;
}

bool 
TileMap::load( BitBuffer& inbuf,
               const TileMapFormatDesc& desc,
               const MC2SimpleString& paramString,
               int gunzippedAlready )
{
   BitBuffer buf1(inbuf.getBufferAddress() + inbuf.getCurrentOffset(),
                  inbuf.getBufferSize() - inbuf.getCurrentOffset());

   uint32 origOffset = inbuf.getCurrentOffset();

   if( ! gunzippedAlready ) {
      m_arrivalTime = readReceiveTime( desc, paramString, buf1 );
   }
   
   BitBuffer buf( buf1.getBufferAddress() + buf1.getCurrentOffset(),
                  buf1.getBufferSize() - buf1.getCurrentOffset() );
   
   // FIXNE: Ugglehack! use readNextByte and reset instead. 
   if ( buf.getCurrentOffsetAddress()[0] == 0x1f &&
        buf.getCurrentOffsetAddress()[1] == 0x8b ) {
      // Gzipped !
      int origLength =
         GunzipUtil::origLength(buf.getBufferAddress(),
                                buf.getBufferSize());
      if ( origLength < 0 ) {
         mc2dbg << "[TileMap]: origLength < 0" << endl;
      }
      BitBuffer gunzippedBuf(origLength);
      int res =
         GunzipUtil::gunzip(gunzippedBuf.getBufferAddress(),
                            gunzippedBuf.getBufferSize(),
                            buf.getBufferAddress(),
                            buf.getBufferSize());
      mc2dbg << "[TileMap]: gunzipres = " << res << endl;
      inbuf.readPastBytes(res);
      bool retVal = load(gunzippedBuf, desc, paramString, 1);
      if ( retVal ) {
         m_loadSize = origLength; // Return the unzipped size.
      } else {
         m_loadSize = 0;
      }
      return retVal;
   }
   // FIXME: Clean up!!
   delete m_params;
   buf.alignToByte();
   // It used to read the params string here but since it took
   // too much room it is supplied when loading instead.
   // Read a string so that it is possible to use and old server.
   buf.readNextString();
   m_params = new TileMapParams( paramString );

   // Check if the layer exists.
   if ( ! desc.hasLayerID( m_params->getLayer() ) ) {
      MC2_ASSERT( false );
      return false;
   }
   
   // Set the reference coordinate and mc2 scale to the map.
   m_mc2Scale = 
      int( desc.getCoordAndScaleForTile( 
                                    m_params->getLayer(),
                                    m_params->getDetailLevel(),
                                    m_params->getTileIndexLat(),
                                    m_params->getTileIndexLon(),
                                    m_referenceCoord ) *
            GfxConstants::METER_TO_MC2SCALE );
   // Adjust the reference coord so it is a multiple of the mc2scale.
   snapCoordToPixel( m_referenceCoord );
   
   TileMapTypes::tileMap_t tileMapType = m_params->getTileMapType();

   int nbrOneCoordArgs = 0;
   uint32 xnbrFeatures = 0;
   if ( tileMapType == TileMapTypes::tileMapData ) {
      loadHeader( buf );

      // Load nbr features.
      xnbrFeatures = buf.readNextBits(31);
      m_features.resize( xnbrFeatures );
      m_vectorProxies.resize( xnbrFeatures );
      mc2dbg << "[TileMap]: load NbrFeatures = " << hex
             << xnbrFeatures << dec << endl;
      TilePrimitiveFeature* prevFeature = NULL;
      for ( uint32 i = 0; i < xnbrFeatures; ++i ) {
         // Read type and create tilemapfeature of correct
         // dynamic type.
         if ( ! TileFeature::createFromStream( m_features[i],
                                               buf, desc, *this, prevFeature ) ) {
            return false;
         }
         TileFeature* feature = &(m_features[i]);
         
         feature->setFeatureNbr( i );
         // Set empty coordinate "vector"
         if ( feature->getArg( TileArgNames::coord ) ) {
            ++nbrOneCoordArgs;
         }
         prevFeature = feature;
      }
   } else if ( tileMapType == TileMapTypes::tileMapStrings ) {      
      // Load the strings.

      // Read the number of bits needed for representing 
      // the string index.
      uint32 nbrBitsStrIdx = buf.readNextBits( 4 );
     
      // Read the number of bits needed for representing 
      // the feature index.
      uint32 nbrBitsFeatureIdx = buf.readNextBits( 4 );
      
       // Read the number of features.
      uint32 nbrFeatures = buf.readNextBits( nbrBitsFeatureIdx );
      
     // Read the number of features with strings.
      uint32 nbrStringFeatures = buf.readNextBits( nbrBitsFeatureIdx );
      
      // Read the number of strings.
      uint32 nbrStrings = buf.readNextBits( nbrBitsStrIdx );
      
      // Resize the vectors.
      m_featureIdxInTextOrder.resize( nbrStringFeatures );
      m_strings.resize( nbrStrings );
      // Default is -1, i.e. that the feature has no strings.
      m_strIdxByFeatureIdx.resize( nbrFeatures, -1 );
      
      // Read pairs of ( feature index, string index ) in 
      // the order as specified by m_featureIdxInTextOrder.
      
      for ( uint32 i = 0; i < nbrStringFeatures; ++i ) {
         // Feature index.
         uint32 featureIdx = buf.readNextBits( nbrBitsFeatureIdx );

         // String index.
         uint32 strIdx = buf.readNextBits( nbrBitsStrIdx );

         // Add to m_strIdxByFeatureIdx.
         m_strIdxByFeatureIdx[ featureIdx ] = strIdx;

         // Add to m_featureIdxInTextOrder.
         m_featureIdxInTextOrder[ i ] = featureIdx;
      }
      
      // Read the strings.
      buf.alignToByte();

      uint32 startPos = buf.getCurrentOffset();
      {for ( uint32 i = 0; i < nbrStrings; ++i ) {
         // FIXME: We could also let the strings point into the
         //        buffer! Spooky.
         // Ok. We will do that.
         buf.readNextString() ;
      }}
      uint32 endPos = buf.getCurrentOffset();
      m_stringBuffer = new BitBuffer( endPos - startPos );
      memcpy( m_stringBuffer->getBufferAddress(),
              buf.getBufferAddress() + startPos, endPos-startPos );
      {for ( uint32 i = 0; i < nbrStrings; ++i ) {
         m_strings[i] = m_stringBuffer->readNextString();
      }}
   }
   buf.alignToByte();

   // Read CRC if features are present.
   m_crc = MAX_UINT32;
   if ( ! empty() ) {
      // Make sure there is room for CRC in the buffer.
      if ( ( buf.getBufferSize() - buf.getCurrentOffset() ) >= 4 ) {
         m_crc = buf.readNextBALong(); // CRC
         mc2dbg8 << "CRC set to " << m_crc << endl;
      }
   }

   // Read the empty importances. 16 bits must be enough!
   if ( ( buf.getBufferSize() - buf.getCurrentOffset() ) >= 2 ) {
      m_emptyImportances = buf.readNextBAShort();
   }
   
   inbuf.readPastBytes(buf.getCurrentOffset());
   
   m_loadSize = inbuf.getCurrentOffset() - origOffset;

   //dump(mc2dbg);

   mc2log << info << "[TMAp]: Nbr coords = "
          << ( m_allCoords.size() + nbrOneCoordArgs ) << endl;
   
   return true;
}

uint32
TileMap::getBufSize() const
{
   return m_loadSize;
}

bool
TileMap::createPrimitives(const TileMapFormatDesc& desc)
{
   if ( m_primitives != NULL ) {
      return false;
   }


   m_minLevelOfPrimitives = desc.getMinLevel();
   m_nbrPrimitiveLevels = desc.getMaxLevel() - desc.getMinLevel() + 1;
   m_primitives = new primVect_t[ m_nbrPrimitiveLevels ];   
   
   // Not implemented yet.
   vector<int> nbrFeaturesPerLevel;
   desc.countNbrFeaturesPerLevel( nbrFeaturesPerLevel, m_features );

   for( uint32 j = 0; j < nbrFeaturesPerLevel.size(); ++j ) {
      m_primitives[j].reserve( nbrFeaturesPerLevel[j] );
   }

   vector<int> nbrAddedFeaturesPerLevel;
   nbrAddedFeaturesPerLevel.resize( nbrFeaturesPerLevel.size() );
   
   int nbrFeatures = m_features.size();
   vector<TilePrimitiveFeature*> primitives;
   for ( int i = 0; i < nbrFeatures; ++i ) {
      desc.getFeaturesForFeature(primitives,
                                 &m_features[i]);
      for( vector<TilePrimitiveFeature*>::iterator jt = primitives.begin();
           jt != primitives.end();
           ++jt ) {
         int level = 13;
         TilePrimitiveFeature* curPrim = *jt;
         const SimpleArg* levelArg =
            static_cast<const SimpleArg*>
            (curPrim->getArg(TileArgNames::level));
         if ( levelArg != NULL ) {
            level = levelArg->getValue();
         }
         // Copy the primitive into the vector. This may or may not be
         // very good, but let's try it.
         m_primitives[ level - m_minLevelOfPrimitives ].push_back(*curPrim);
//              [nbrAddedFeaturesPerLevel[level - m_minLevelOfPrimitives]++] =
//              *curPrim;
         delete curPrim;

      }
      primitives.clear();
   }

   // Go through the primivitives and update the m_parentFeature for 
   // the complex features so that it points down to one of the 
   // primitives.
   uint32 primNbr = 0;
   for ( int l = 0; l < m_nbrPrimitiveLevels; ++l ) {
      for ( uint32 i = 0; i < m_primitives[ l ].size(); ++i ) {
         TilePrimitiveFeature& curPrim = m_primitives[ l ][ i ];
         m_features[ curPrim.getFeatureNbr() ].setFeatureNbr( primNbr++ );
      }
   }
   
#ifdef MC2_SYSTEM
   for ( int k = 0; k < m_nbrPrimitiveLevels; ++k ) {
      int vectorSize = m_primitives[k].size();
      int counted = nbrFeaturesPerLevel[k];
      MC2_ASSERT( vectorSize == counted );
   }
#endif
   return true;
}


#ifdef MC2_SYSTEM
void 
TileMap::dump( ostream& stream ) const {
  for ( vector<TileFeature>::const_iterator it = 
            m_features.begin(); it != m_features.end(); ++it ) {
      (it)->dump( stream );
   }
   stream << "Nbr strings " << m_strings.size() << endl;
   uint32 i = 0;
   for ( vector<MC2SimpleString>::const_iterator kt = m_strings.begin();
         kt != m_strings.end(); ++kt ) {
      stream << "  String " << i << " " << *kt << endl;
      ++i;
   }
   stream << "String index by feature index." << endl;
   for ( uint32 i = 0; i < m_strIdxByFeatureIdx.size(); ++i ) {
      stream << " feat index " << i << ", str index "
             << m_strIdxByFeatureIdx[ i ] << endl;
   }
   stream << "Feature index in text order." << endl;
   for ( uint32 i = 0; i < m_featureIdxInTextOrder.size(); ++i ) {
      stream << m_featureIdxInTextOrder[ i ] << endl;
   }
   
   stream << "TileMap:" << endl;
   stream << "Nbr features " << m_features.size() << endl;
}
#endif

void 
TileMap::snapCoordToPixel( TileMapCoord& coord ) const
{
   coord.setCoord( 
         m_mc2Scale * (coord.getLat() / m_mc2Scale),
         m_mc2Scale * (coord.getLon() / m_mc2Scale) );
}

const TileFeature*
TileMap::getParentFeature(const TilePrimitiveFeature* prim) const
{
   int parentIdx = prim->getFeatureNbr();
   if ( parentIdx < 0 || parentIdx > (int)m_features.size() ) {
      return NULL;
   } else {
      return &m_features[parentIdx];
   }
}

const TileMapParams*
TileMap::getStringMapParams(LangTypes::language_t lang) const
{
   if ( m_params->getTileMapType() == TileMapTypes::tileMapStrings ) {
      return NULL;
   } else {
      if ( m_otherTypeOfMapParams != NULL ) {
         return m_otherTypeOfMapParams;
      }
      // Pretend that this function is const.
      TileMap* non_const_this = const_cast<TileMap*>(this);      
      non_const_this->m_otherTypeOfMapParams = new TileMapParams(*m_params);
      non_const_this->m_otherTypeOfMapParams->
         setTileMapType(TileMapTypes::tileMapStrings);
      non_const_this->m_otherTypeOfMapParams->
         setLanguageType(lang);
      return m_otherTypeOfMapParams;
   }
}

const TileMapParams*
TileMap::getDataMapParams() const
{
   if ( m_params->getTileMapType() == TileMapTypes::tileMapData ) {
      return NULL;
   } else {
      if ( m_otherTypeOfMapParams != NULL ) {
         return m_otherTypeOfMapParams;
      }
      // Pretend that this function is const.
      TileMap* non_const_this = const_cast<TileMap*>(this);
      delete non_const_this->m_otherTypeOfMapParams;
      non_const_this->m_otherTypeOfMapParams = new TileMapParams(*m_params);
      non_const_this->m_otherTypeOfMapParams->
         setTileMapType(TileMapTypes::tileMapData);
      non_const_this->m_otherTypeOfMapParams->
         setLanguageType(LangTypes::swedish);
      return m_otherTypeOfMapParams;
   }
}

const char*
TileMap::getStringForFeature(int featureNbr) const
{
   if ( m_strings.empty() || featureNbr >= (int)m_strIdxByFeatureIdx.size()) {
      return NULL;
   } else {
      int strIdx = m_strIdxByFeatureIdx[featureNbr];
      if ( strIdx < 0 ) {
         return NULL;
      } else {
         return m_strings[strIdx];
      }
   }
}

const TileFeature*
TileMap::getFeature(int featureNbr) const
{
   if ( m_features.empty() || 
         featureNbr >= (int)m_features.size() ) {
      return NULL;
   } else {
      return &m_features[ featureNbr ];
   }
}

int 
TileMap::getFeatureIdxInTextOrder( int i ) const
{
   return m_featureIdxInTextOrder[ i ];
}

int 
TileMap::getNbrFeaturesWithText() const
{
   return m_featureIdxInTextOrder.size();
}

TilePrimitiveFeature* 
TileMap::getOnePrimitive( TilePrimitiveFeature& feature ) const
{
   // Make sure the primitives are created first.
   if ( m_primitives == NULL ) {
      return NULL;
   }
   
   if ( feature.isPrimitive() ) {
      // Was already a primitive, so return itself.
      return &feature;
   } else {
      // Complex feature. Use featurenbr to find the primitive.
      uint32 primNbr = feature.getFeatureNbr();
      int curLevel = 0;
      
      while ( ( curLevel < m_nbrPrimitiveLevels ) && 
              ( primNbr >= m_primitives[ curLevel ].size() ) ) {
         primNbr -= m_primitives[ curLevel ].size();
         ++curLevel;
      }
      MC2_ASSERT( curLevel < m_nbrPrimitiveLevels );
      return &m_primitives[ curLevel ][ primNbr ];
   }
}

TileMapParams* TileMap::getParams() const
{
   return m_params;
}

