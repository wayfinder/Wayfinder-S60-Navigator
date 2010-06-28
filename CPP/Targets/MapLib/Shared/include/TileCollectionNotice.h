/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILECOLLECTIONNOTICE_H 
#define TILECOLLECTIONNOTICE_H

#include <vector>
#include <map>
#include "TileMapParams.h"
#include <algorithm>

class SharedBuffer;

/// The importance range.
struct impRange_t {
   /// Equal operator to find the layer, used by std::find.
   bool operator == ( int layerID ) {
      return m_layerID == layerID;
   }
   
   /// Layer ID.
   int m_layerID;
   /// First importance.
   int m_firstImp;
   /// Last importance. 
   int m_lastImp;
};

/**
 *    Class describing a rectangle of tiles for a specific detaillevel.
 */
class TilesNotice {
public:
   /// The TileMapClientSFDQuery constructs this datastructure. 
   friend class TileMapClientSFDQuery; 
   friend class TilesForAllDetailsNotice;

   /// The size of the offset entry size.
   enum {
      offsetEntrySize = 4
   };

   /// Empty constructor.
   TilesNotice();
   
   /// Assignment operator. 
   TilesNotice& operator = ( const TilesNotice& other );
  
   /// Copy constructor.
   TilesNotice( const TilesNotice& other );
   
   /// Destructor.
   ~TilesNotice();
   
   /// Load from buffer.
   void load( SharedBuffer& buf );
   
   /// Get the file offset for the param.
   inline int getOffset( const TileMapParams& param ) const;

   /**
    *   Get the importance range for this param. 
    *   Note that the layer must be present in this notice!
    */
   inline const impRange_t& getImpRange( const TileMapParams& param ) const;

#ifdef MC2_SYSTEM   
   /// Save into buffer.
   void save( SharedBuffer& buf ) const;

   /// Get the offset after the last one.
   int getNextOffset() const;

   /**
    *    Update the offsets. 
    *    @param   startOffset    The start offset.
    *    @return  The next offset.
    */
   uint32 updateOffset( uint32 startOffset );

   /**
    *    Updates the notice with the param. 
    */
   void updateWithParam( const TileMapParams& param ); 
   
#endif

private:

   /// Offset.
   int m_offset;
   
   int m_startLatIdx;
   int m_endLatIdx;
   int m_startLonIdx;
   int m_endLonIdx;

   /// Nbr of layers which also is the size of m_impRange. 
   uint16 m_nbrLayers;

   /**
    *  Array of importance ranges for each layer.
    *  Note that adding another template to this class will cause
    *  gcc to freak out.
    */
   impRange_t* m_impRange;

};

/**
 *    Class describing all detail levels for tiles covering a rectangle.
 */
class TilesForAllDetailsNotice {
public:
   /// The TileMapClientSFDQuery constructs this datastructure. 
   friend class TileMapClientSFDQuery; 
   friend class TileCollectionNotice;
   
   /// Load from buffer.
   void load( SharedBuffer& buf );
   
#ifdef MC2_SYSTEM
   /// Save into buffer.
   void save( SharedBuffer& buf ) const;
   
   /**
    *    Update the offsets. 
    *    @param   startOffset    The start offset.
    *    @return  The next offset.
    */
   uint32 updateOffset( uint32 startOffset );
#endif

private:
   /// Get the TileNotice for the detail. NULL if no such detail.
   inline TilesNotice* getTilesForDetail( int detail );

   /**
    *    Get the notice for the param. NULL if not found.
    */
   inline const TilesNotice* getNotice( const TileMapParams& param ) const;
  
   /** 
    *    The starting detail level, corresponding to the first index of
    *    m_tilesNotice.
    */
   uint32 m_startDetail;

   /**
    *    The tiles notices. The index in the vector corresponds to 
    *    (detaillevel - m_startDetail).
    */
   std::vector<TilesNotice> m_tilesNotice;

};

/**
 *    Class describing tiles covering a rectangle.
 */
class TileCollectionNotice {

public:
  
   /// The TileMapClientSFDQuery constructs this datastructure. 
   friend class TileMapClientSFDQuery; 

   
   /// Load from buffer.
   uint32 load( SharedBuffer& buf );
   
   /// Get the file offset for the param.
   inline int getOffset( const TileMapParams& param ) const;

   /**
    *   Get the importance range for this param. 
    *   NULL if not found.
    */
   const impRange_t* getImpRange( const TileMapParams& param ) const;

#ifdef MC2_SYSTEM      
   /// Save into buffer.
   uint32 save( SharedBuffer& buf ) const;
   
   /**
    *    Update the offsets. 
    *    @param   startOffset    The start offset.
    *    @return  The offset for the start of the buffer data for
    *             this collection.
    */
   uint32 updateOffset( uint32 startOffset );
#endif

private:
   
   /**
    *    Get the notice for the param. NULL if not found.
    */
   inline const TilesNotice* getNotice( const TileMapParams& param ) const;

   /**
    *    Which index in m_tiles to use for which layer id.
    *    Note that several layer ids can use the same index
    *    in m_tiles.
    */
   std::map<int,int> m_indexByLayerID;
   
   /**
    *    Vector of tiles. Index should come from m_indexByLayerID.
    */ 
   std::vector<TilesForAllDetailsNotice> m_tilesForAllDetails;
 
};

// --- Inlines
inline const impRange_t&
TilesNotice::getImpRange( const TileMapParams& param ) const
{
   impRange_t* const endIt = m_impRange + m_nbrLayers;
   const impRange_t* findit = 
      std::find( m_impRange, endIt,
                 param.getLayer() );
   // Note that the layer must be found, or else it's an error.
   MC2_ASSERT( findit != endIt );
   return *findit;
}

inline int 
TilesNotice::getOffset( const TileMapParams& param ) const 
{
   if ( param.getTileIndexLat() < m_startLatIdx ||
        param.getTileIndexLat() > m_endLatIdx ||
        param.getTileIndexLon() < m_startLonIdx ||
        param.getTileIndexLon() > m_endLonIdx ) {
      return -1;
   }

   const impRange_t& impRange = getImpRange( param );
   
   if ( param.getImportanceNbr() < impRange.m_firstImp ||
        param.getImportanceNbr() > impRange.m_lastImp ) {
      // Outside the importance range.
      return -1;
   }
   
   int width = m_endLonIdx - m_startLonIdx + 1;

   return m_offset + 
          ( width * ( param.getTileIndexLat() - m_startLatIdx ) + 
            param.getTileIndexLon() - m_startLonIdx ) * offsetEntrySize;
}

inline TilesNotice* 
TilesForAllDetailsNotice::getTilesForDetail( int detail ) 
{
   int offset = detail - m_startDetail;
   if ( offset >= 0 && offset < (int) m_tilesNotice.size() ) {
      return &m_tilesNotice[ offset ];
   } else {
      return NULL;
   }
}

const TilesNotice*
TilesForAllDetailsNotice::getNotice( const TileMapParams& param ) const
{
   const TilesNotice* notice = 
      const_cast<TilesForAllDetailsNotice*> (this)->
         getTilesForDetail( param.getDetailLevel() );

   return notice;
}

const TilesNotice*
TileCollectionNotice::getNotice( const TileMapParams& param ) const 
{
   int layerID = param.getLayer();
   
   std::map<int,int>::const_iterator it = m_indexByLayerID.find( layerID );
   if ( it == m_indexByLayerID.end() ) {
      return NULL;
   } 
  
   return m_tilesForAllDetails[ (*it).second ].getNotice( param );
}

inline int 
TileCollectionNotice::getOffset( const TileMapParams& param ) const
{
   const TilesNotice* notice = getNotice( param );
   if ( notice == NULL ) {
      return -1;
   } 
   // else
   
   return notice->getOffset( param );   
   
//   mc2dbg8 << "[TCN]: getOffset " << param.getAsString() << " at "
//           << m_tilesForAllDetails[ (*it).second ].getOffset( param )
//           << endl;
}


#endif
