/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "TileMapConfig.h"
#include "TileMapTypes.h"

#include "BitBuffer.h"
#include <vector>
#include <map>
#include "MC2SimpleString.h"
#include "LangTypes.h"
#include "MC2Point.h"
#include "TileMapUtil.h"
#include "TileMapFormatDesc.h"

class TilePrimitiveFeature;
class TileMapFormatDesc;
class BitBuffer;
class TileFeature;

// Forward declaration.
class TileMapParams;

/**
 *    This class represents one Tile of TileFeatures.
 *    Can contain strings or features.
 */
class TileMap {
public:
   /// Vector of primitives.
   typedef std::vector<TilePrimitiveFeature> primVect_t;
   /// map of feature id to category id
   typedef std::vector< std::pair< uint32, uint16 > > poiCategoryMap_t;
   TileMap() {
      m_stringBuffer = NULL;
      m_primitives  = NULL;
      m_params      = NULL;
      m_otherTypeOfMapParams = NULL;
      m_mc2Scale = 1; // Set to a nonzero value before tmfd is loaded.
      m_emptyImportances = 0; // No empty importances.
   }

   /// Destructor
   ~TileMap();

#ifdef MC2_SYSTEM
   /**
    *  Save the tilemapdata part to buffer.
    */
   void saveTileMapData( BitBuffer& buf ) const;
 

   /**
    *   Saves the map to the databuffer.
    *    
    *   @param   buf      The buffer to save to.
    *   @return True if the save was ok.
    */
   bool save( BitBuffer& buf ) const;

   /**
    *   Saves the TileMap to the stream.
    *
    *   @param stream The stream to save the object to.
    *   @return True if the save was ok.
    */
   bool save(ostream& stream) const;
#endif

   /**
    *   Returns the receive time for the map or UINT32 if the
    *   map doesn't have one.
    */
   static uint32 readReceiveTime( const TileMapFormatDesc& tmfd,
                                  const TileMapParams& params,
                                  BitBuffer& buf );

   /**
    *   Sets the receive time for the map if it is a map that should
    *   have one.
    */
   static BitBuffer* writeReceiveTime( const TileMapFormatDesc& tmfd,
                                       const MC2SimpleString& params,
                                       BitBuffer* buf );

   /**
    *   Returns true if the map is older than the number of
    *   minutes set in the tilemapformatdesc.
    */
   inline int expired( const TileMapFormatDesc& desc,
                       const TileMapParams& params ) const;
   
   /**
    *   Loads the TileMap from the BitBuffer.
    */
   bool load( BitBuffer& buf,
              const TileMapFormatDesc& desc,
              const MC2SimpleString& paramString,
              int gunzippedAlready = 0 );

   /**
    *   Returns the size of this map when it was read from the
    *   buffer. (Only works for maps loaded from buffers).
    */
   uint32 getBufSize() const;
   
   /**
    *   Creates the primitives of the map.
    */
   bool createPrimitives(const TileMapFormatDesc& desc);

   /**
    *   Returns reference to vector of primitives for
    *   level <code>level</code>.
    */
   inline primVect_t& getPrimitives(const TileMapFormatDesc& desc,
                                    int level) ;

   /**
    *   Returns the params for the corresponding string map.
    *   Returns NULL if this is a string map already.
    *   @param lang The language of the string parameters.
    */
   const TileMapParams* getStringMapParams(LangTypes::language_t lang) const;

   /**
    *   Returns the params for the corresponding data map.
    *   Returns NULL if this is a data map already.
    *   @param lang The language of the string parameters.
    */
   const TileMapParams* getDataMapParams() const;
   
#ifdef MC2_SYSTEM
   void dump( ostream& stream ) const;
#endif

   /// Typedef for iterators
   typedef std::vector<TileFeature>::iterator iterator;
   /// Typedef for const_iterator
   typedef std::vector<TileFeature>::const_iterator const_iterator;

   inline iterator begin(int order = -1);
      
   inline iterator end(int order = -1);
   
   inline const_iterator begin(int order = -1) const;
      
   inline const_iterator end(int order = -1) const;

   inline int nbrFeatures(int order = -1 ) const;

   inline const TileMapCoord& getReferenceCoord() const;
   inline int getMC2Scale() const;

   /**
    *    Snap the supplied coordinate to a pixel in the TileMap.
    *    @param   coord The coordinate.
    */
   void snapCoordToPixel( TileMapCoord& coord ) const;

   /**
    *    Returns the parent feature for the supplied primitive.
    */
   const TileFeature* getParentFeature(
         const TilePrimitiveFeature* prim) const;

   /**
    *   Returns the string for feature number <code>featureNbr</code>
    *   in the corresponding ordinary map (if this is a string map).
    *   NB! The string is in UTF8!
    *   @return NULL if not found.
    */
   const char* getStringForFeature(int featureNbr) const;

   /**
    *   Returns a reference to the coordinates.
    */
   TileMapCoords& coordsRef() { return m_allCoords; }

   /**
    * 
    */
   const TileFeature* getFeature(int featureNbr) const;
   
   /**
    * 
    */
   int getFeatureIdxInTextOrder( int i ) const;
        
   /**
    * 
    */
   int getNbrFeaturesWithText() const;

   /**
    *    @return The CRC. Should be same for both tileMapData and 
    *            tileMapString TileMaps.
    */
   inline uint32 getCRC() const;

   /**
    *    @return If the TileMap contains no data.
    */
   inline bool empty() const;

#ifdef MC2_SYSTEM   
   /**
    *    Set the bitfield containing the empty importances.
    */
   inline void setEmptyImportances( uint32 emptyImp );
#endif

   /**
    *    @return  Bitfield containing the empty importances.
    */
   inline uint32 getEmptyImportances() const;
 
   /**
    *    @return  The supplied feature if it's already a primitive, 
    *             or one of the associated primitives to the feature.
    */
   TilePrimitiveFeature* getOnePrimitive( 
                              TilePrimitiveFeature& feature ) const;
   
#ifdef __unix__   
   /**
    *    @return The approximate size in memory of the object.
    */
   uint32 getMemSize() const {
      
      uint32 tileMapObjectSize = sizeof(TileMap);
      uint32 realCoordSize = m_allCoords.size() * 4 * 2;
      uint32 stringSize = m_strings.size() * 4;
      if ( m_stringBuffer != NULL ) {
         stringSize += m_stringBuffer->getBufferSize();
      }
      stringSize += m_strIdxByFeatureIdx.size() * 4;
      stringSize += m_featureIdxInTextOrder.size() * 4;

      uint32 featureSize = 0;
      for ( uint32 i = 0; i < m_features.size(); ++i ) {
         featureSize += m_features[ i ].getMemSize();
      }

      uint32 primitiveSize = 0;
      if ( m_primitives != NULL )  { 
         for ( int i = 0; i < m_nbrPrimitiveLevels; ++i ) {
            for ( uint32 j = 0; j < m_primitives[ i ].size(); ++j ) {
               primitiveSize += m_primitives[ i ][ j ].getMemSize();
            }
         } 
      }
      
      return tileMapObjectSize + realCoordSize +  
         stringSize + featureSize + primitiveSize;
   }
#endif 
   // Enable when needed
   //   inline const poiCategoryMap_t& getPOICategories() const;

protected:
   
   /// Unimplemented copy constructor.
   TileMap( const TileMap& other);
   /// Unimplemented assignment operator
   const TileMap& operator=(const TileMap& other);
   
#ifdef MC2_SYSTEM   
   /**
    *   Saves the header to the databuffer.
    *    
    *   @param  buf      The buffer to save to.
    *   @return True if the save was ok.
    */
   bool saveHeader( BitBuffer& buf ) const;
#endif

   /**
    *   Loads the header from the databuffer.
    *   @param  buf      The buffer to load from.
    *   @return True if the load was ok.
    *   
    */
   inline bool loadHeader( BitBuffer& buf );
   void loadCategories( BitBuffer& inBuf );
   void saveCategories( BitBuffer& inBuf ) const;
   
   /// The features
   std::vector<TileFeature> m_features;

   /// The VectorProxies of the features
   std::vector<VectorProxy<MC2Point> > m_vectorProxies;
   
   /// The primitives
   primVect_t* m_primitives;
   /// The min level of the primitives
   int m_minLevelOfPrimitives;
   /// The number of primitive levels
   int m_nbrPrimitiveLevels;
   
   /// The strings. UTF8
   std::vector<const char*> m_strings;
   /// Buffer containing the strings
   BitBuffer* m_stringBuffer;

   /**
    * String index (in m_strings) for feature index.
    * If the string index is negative then there's no 
    * string for that feature.
    */
   std::vector<int> m_strIdxByFeatureIdx;

   /**
    * The feature indices are sorted in the textplacement order,
    * so that the feature with the most important text is first.
    */
   std::vector<uint32> m_featureIdxInTextOrder;
   // Enable when needed.
   //   poiCategoryMap_t m_poiCategoryMap;

   /// The parameters for the map.
   TileMapParams* m_params;

   /// The parameters for the string type map.
   TileMapParams* m_otherTypeOfMapParams;

   /// The size of the BitBuffer that the map came from
   uint32 m_loadSize;

   /// The reference coordinate.
   TileMapCoord m_referenceCoord;

   /// MC2 scale, i.e. mc2 units per pixel.
   int m_mc2Scale;

   /// The CRC for the tilefeatures.
   uint32 m_crc;

   /// Bitfield containing the empty importances.
   uint32 m_emptyImportances;
   
   /// Coordinates for this map
   TileMapCoords m_allCoords;
   /// The arrival time in minutes
   uint32 m_arrivalTime;

};

// -- Inlined functions

inline TileMap::iterator 
TileMap::begin(int order) 
{
   if ( order < 0 ) {
      return m_features.begin();
   } else {
      return m_features.begin();
   }
}
      
inline TileMap::iterator 
TileMap::end(int order) 
{
   if ( order < 0 ) {
      return m_features.end();
   } else {
      return m_features.end();
   }
}

inline TileMap::const_iterator
TileMap::begin(int order) const
{
   return const_cast<TileMap*>(this)->begin(order);
}

inline TileMap::const_iterator
TileMap::end(int order) const
{
   return const_cast<TileMap*>(this)->end(order);
}

inline int 
TileMap::nbrFeatures(int /*order*/) const 
{
   return m_features.size();
}

inline const TileMapCoord& 
TileMap::getReferenceCoord() const
{
   return m_referenceCoord;
}
   
inline int 
TileMap::getMC2Scale() const
{
   return m_mc2Scale;
}

#ifdef MC2_SYSTEM
inline bool 
TileMap::saveHeader( BitBuffer& buf ) const 
{
   //buf.writeNextBAShort( m_importanceType );
   return true;
}
#endif

inline bool 
TileMap::loadHeader( BitBuffer& /*buf*/ ) 
{
   //m_importanceType = buf.readNextBAShort();
   return true;
}

inline TileMap::primVect_t&
TileMap::getPrimitives(const TileMapFormatDesc& desc,
                       int level)
{
   if ( m_primitives == NULL ) {
      createPrimitives(desc);
   }
   return m_primitives[ level - m_minLevelOfPrimitives ];
}

inline uint32 
TileMap::getCRC() const
{
   return m_crc;
}

inline bool
TileMap::empty() const
{
   return m_features.empty() && m_strIdxByFeatureIdx.empty();
}

#ifdef MC2_SYSTEM
inline void 
TileMap::setEmptyImportances( uint32 emptyImp )
{
   m_emptyImportances = emptyImp;
}
#endif

inline uint32 
TileMap::getEmptyImportances() const
{
   return m_emptyImportances;
}

inline int
TileMap::expired( const TileMapFormatDesc& desc,
                  const TileMapParams& params ) const
{
   if ( m_arrivalTime == MAX_UINT32 ) {
      return false;
   }
   uint32 nbrMinutes = desc.getUpdatePeriodMinutes( params );
   if ( nbrMinutes == 0 ) {
      return false;
   } else {
      
      uint16 curTime = TileMapUtil::currentTimeMillis() / 1000 / 60;
      return ( curTime - m_arrivalTime ) >= nbrMinutes;
   }
}
/*
inline const TileMap::poiCategoryMap_t& 
TileMap::getPOICategories() const {
   return m_poiCategoryMap;
}
*/
#endif // TILE_MAP_H

