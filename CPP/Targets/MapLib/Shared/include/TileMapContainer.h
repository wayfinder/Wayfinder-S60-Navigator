/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_MAPCONTAINER_H
#define TILE_MAPCONTAINER_H

#include "TileMap.h"
#include "TileMapParams.h"
#include "TileMapHandlerTypes.h"
#include "LangTypes.h"
#include <list>
#include "RouteID.h"
#include "TileMapFormatDesc.h"
#include "MapProjection.h"
#include "ParamsNotice.h"

#include "TileMapGarbage.h"
#include "DBufRequester.h"

/**
 *    Layer importance notice.
 *    First is layer, second is importance.
 */
struct layerImp_t : public std::pair<int,int>
{
   /**
    *    Constructor.
    *    @param   param    The TileMapParams.
    *    @param   mapDesc  The tmfd.
    */
   inline layerImp_t( const TileMapParams& param,
                      const TileMapFormatDesc& mapDesc );

   inline friend std::ostream& operator<<( std::ostream& ost, const layerImp_t& imp ) {
      return ost << "layerImp_t(" << imp.first << ", " << imp.second << ")";
   }

   
};


// ----------------------- MapStorage -------------------------

/**
 *    MapStorage class.
 */
struct MapStorage {

   /**
    *    Constructor.
    *    @param   containsStrings   If strings are present among the maps.
    *    @param   garbage           The garbage collector for tilemaps.
    */
   MapStorage( bool containsStrings, TileMapGarbage<TileMap>* garbage ); 
   
   /**
    *    Destructor.
    */
   ~MapStorage();
   
   /**
    *    Swap the maps with another MapStorage.
    *    @param   other The other MapStorage.
    */
   void swap( MapStorage& other );

   /**
    *    Get how much of the area specified by the MapProjection 
    *    this MapStorage covers. Full coverage is MAX_INT16, 
    *    no coverage is zero.
    *
    *    @param   imp         The layer and importance.
    *    @param   params      The paramsnotice.
    *    @param   projection  The map projection.
    *    @param   mapDesc     The mapformat description.
    *    @return  The coverage value.
    */
   int getCoverage( const layerImp_t& imp, 
                    const ParamsNotice& params,
                    const MapProjection& projection,
                    const TileMapFormatDesc& mapDesc ) const;
   
   /**
    *    Get the string index from the map index.
    *    @param   index    The map index.
    *    @return  The string index.
    */
   int getStrIdx( int index ) const;

   /**
    *    Get the map index from the string index.
    *    @param   index    The string index.
    *    @return  The map index.
    */
   int getMapIdx( int strIdx ) const;

   /**
    *    Get map index for the param.
    *    @param   param The param.
    *    @return  The map index.
    */
   int getMapIndex( const TileMapParams& param ) const;
   
   /**
    *    Update maps from the specified MapStorage, (i.e. move them to this
    *    MapStorage if they are found in the other).
    *    @param   other The other MapStorage to update from.
    */
   void updateMapsFrom( MapStorage& other );
            
   /**
    *    Take the specified importance from the other MapStorage and 
    *    move it to this MapStorage. Any existing importance will be replaced.
    *    @param   other The other MapStorage.
    *    @param   imp   The importance.
    */
   void takeImportance( MapStorage& other, const layerImp_t& imp );

   /**
    *    Update the importance map.
    */
   void updateImportances();

   /**
    *    Clear data.
    *    @param   imp   [Optional] If specified, only the importance will be
    *                   cleared. Otherwise all data will be cleared.
    */
   void clear( const layerImp_t* imp = NULL );

   /**
    *    Set the tilemapformatdesc.
    *    @param mapDesc The mapdesc.  
    */
   void setMapDesc( const TileMapFormatDesc* mapDesc );
   
   /**
    *    Vector of TileMapParams. Adjacent TileMap found in m_maps.
    */
   std::vector<TileMapParams> m_params;
   
   /**
    *    Vector of Maps. Adjacent TileMapParams found in m_params.
    */
   std::vector<TileMap*> m_maps;

   /**
    *    Vector of bools indicating if the adjacent tilemap in m_maps
    *    is totally empty and therefore should not be requested
    *    if possible.
    */
   std::vector<bool> m_emptyMaps;
   
   /**
    *    Range of indices. First is begin and second is end.
    */
   typedef std::pair<int, int> range_t;
   
   /**
    *    Map of importances. The key is the importance, and the values is
    *    index range in m_maps and m_params for the importance.
    *    I.e. the importance starts at second.first 
    *    and ends after second.second.
    */
   std::map<layerImp_t, range_t> m_importances;

   /**
    *    Indicates if strings maps are present in this MapStorage.
    */
   bool m_containsStrings;
  
   /**
    *    The tilemapformatdesc.
    */
   const TileMapFormatDesc* m_mapDesc;
  
   /**
    *    The garbage collector of TileMaps.
    */
   TileMapGarbage<TileMap>* m_garbage;
   
   /**
    *    Get the map at index i.
    *    @param   i  The index.
    *    @return  The map at index i. NULL if not found.
    */
   TileMap* getMap( uint32 i ) const {
      if ( i < m_maps.size() ) {
         return m_maps[ i ];
      }
      return NULL;
   }
   
   /**
    *    Clean up.
    */
   void cleanUp() {
      if ( m_importances.empty() ) {
#ifdef MC2_SYSTEM         
         // Make sure all the maps are NULL.
         for ( uint32 i = 0; i < m_maps.size(); ++i ) {
            MC2_ASSERT( m_maps[ i ] == NULL );
         }
#endif         
         m_params.clear();
         m_maps.clear();
      }
   }
   
   /**
    *    Dump everything.
    */
   void dump() const {
      mc2log << "MapStorage: dump" << std::endl;
      for ( std::map<layerImp_t, range_t>::const_iterator it = 
            m_importances.begin(); it != m_importances.end(); ++it ) {
         dump( (*it).first );
      }
      for ( uint32 i = 0; i < m_params.size(); ++i ) {
         mc2log << "Param = " << i << " - " << m_params[ i ].getAsString();
         if ( getMap( i ) != NULL ) {
            mc2log << " not NULL ";
         } else {
            mc2log << " NULL";
         }
         if ( m_emptyMaps[ i ] ) {
            mc2log << " empty." << std::endl;
         } else {
            mc2log << " not empty." << std::endl;
         }
      }
   }
   
   /**
    *    Dump an importance.
    */
   void dump( const layerImp_t& imp ) const {
      std::map<layerImp_t, range_t>::const_iterator findit = 
         m_importances.find( imp );
      if ( findit != m_importances.end() ) {
         mc2log << "MapStorage: dump - layerImp_t " << imp.first << ", "
            << imp.second << " - range " << findit->second.first 
            << " - " << findit->second.second << endl;
         mc2log << "Size of maps = " << m_maps.size() << endl;
      } else {
         mc2log << "MapStorage: dump - layerImp_t " << imp.first << ", "
            << imp.second << " - Not found." << endl;
      }
   }

   /**
    *    @return The map sizes in this storage.
    */
   uint32 getMapSizes() const {
      uint32 totalMapSize = 0;
#ifdef __unix__
      uint32 totalStringSize = 0;
      uint32 totalMemSize = 0;
      for ( uint32 i = 0; i < m_maps.size(); ++i ) {
         if ( m_maps[ i ] != NULL ) {
            totalMemSize += m_maps[ i ]->getMemSize();
            uint32 mapSize = m_maps[ i ]->getBufSize();
            totalMapSize += mapSize;
            if ( mapSize > 5*1024 ) {
               mc2log << "[MS]: map " << m_params[ i ].getAsString()
                    << " consists of " << mapSize << " bytes." << endl;
            }
            if ( m_params[ i ].getTileMapType() == 
                 TileMapTypes::tileMapStrings ) {
               totalStringSize += mapSize;
            }
         }
      }
      mc2log << "[MS]: Storage totally contains " << totalMapSize 
           << " bytes maps." << endl;
      if ( totalStringSize > 0 ) {
         mc2log << "[MS]: of which " << totalStringSize << " was strings."
              << endl;
      }
      mc2log << "[MS]: Estimated size in memory is " << totalMemSize << endl;
#endif            
      return totalMapSize;
   }
      
};


// ----------------------- ToDrawIterator -------------------------

/**
 *    Iterator of maps/params to draw.
 */
class ToDrawIterator {
public:
   typedef std::map<layerImp_t, MapStorage*> mapsToDraw_t;
//   typedef TileMapContainer::mapsToDraw_t mapsToDraw_t;
   
   /**
    *   Creates a new ToDrawIterator at either the beginning or end.
    *   @param mapsToDraw  Ptr to mapsToDraw.
    *   @param begin       If the iterator should be the beginning 
    *                      or end.
    */
   ToDrawIterator( const mapsToDraw_t* mapsToDraw, bool begin ) 
         : m_mapsToDraw( mapsToDraw ) {
      if ( begin ) {
         m_mapsToDrawIt = m_mapsToDraw->begin();
         if ( m_mapsToDrawIt == m_mapsToDraw->end() ) {
            m_idx = 0; // end()
         } else {
            m_curRange = getRange();
            m_idx = m_curRange.first;
         }
      } else {
         // end()
         m_mapsToDrawIt = mapsToDraw->end();
         m_idx = 0;
      }
   }

   /**
    *   Creates a new ToDrawIterator according to the params.
    *   @param mapsToDraw  Ptr to mapsToDraw.
    *   @param it          MapsToDraw iterator, i.e. which importance
    *                      that is active.
    *   @param index       The index of the map at the specified importance.
    */
   ToDrawIterator( const mapsToDraw_t* mapsToDraw, 
                   mapsToDraw_t::const_iterator& it,
                   int index ) 
         : m_mapsToDraw( mapsToDraw ),
           m_mapsToDrawIt( it ),
           m_idx( index ) {}

   /**
    *   Returns true if the iterator is equal to another iterator.
    */
   bool operator==(const ToDrawIterator& other) const {
      MC2_ASSERT( m_mapsToDraw == other.m_mapsToDraw );
      return ( other.m_mapsToDrawIt == m_mapsToDrawIt ) &&
             ( other.m_idx == m_idx );
   }

   /**
    *   Returns false if the iterator is equal to another iterator.
    */
   bool operator!=(const ToDrawIterator& other) const {
      return ! (other == *this);
   }
   
   /**
    *   Prefix ++operator. Please use most often.
    */
   const ToDrawIterator& operator++() {
      ++m_idx;
      if ( m_idx >= m_curRange.second ) {
         ++m_mapsToDrawIt;
         if ( m_mapsToDrawIt != m_mapsToDraw->end() ) {
            m_curRange = getRange();
            m_idx = m_curRange.first;
         } else {
            m_idx = 0; // end()
         }
      }
      return *this;
   }

   /**
    *   Postfix operator. Use more seldomly than ++operator, please.
    */
   ToDrawIterator operator++(int) {
      ToDrawIterator tmp(*this);
      ++(*this);
      return tmp;
   }
  
   /**
    *    @return  The stringmap associated with this iterator.
    *             NULL if no stringmap is available.
    */
   TileMap* getStringMap() const {
      const MapStorage* mapStorage = (*m_mapsToDrawIt).second;
      if ( mapStorage->m_containsStrings ) {
         int strIdx = mapStorage->getStrIdx( m_idx );
         return mapStorage->m_maps[ strIdx ];
      } else {
         return NULL;
      }
   }
   
   /**
    *    Operator <, to be implemented.
    */
//   bool operator < ( const ToDrawIterator& other ) {
//      MC2_ASSERT( m_mapsToDraw == other.m_mapsToDraw );
//   }
   
protected:

   typedef MapStorage::range_t range_t;
   
   /**
    *    Get the map index range for this importance.
    */
   range_t getRange() {
      return (*m_mapsToDrawIt).second->m_importances.find( 
                  (*m_mapsToDrawIt).first )->second;
   }

   /// The MapsToDraw
   const mapsToDraw_t* m_mapsToDraw;

   /// The importance iterator.
   mapsToDraw_t::const_iterator m_mapsToDrawIt;
   
   /// The index in the importance.
   int m_idx;
  
   /// The map index range for the current importance. 
   range_t m_curRange;
};

// Forward.
class ParamsToDrawIt;

/**
 *    MapsToDraw iterator.   
 */
class MapsToDrawIt : public ToDrawIterator {
public:
   /**
    *   Creates a new MapsToDrawIt at either the beginning or end.
    *   @param mapsToDraw  Ptr to mapsToDraw.
    *   @param begin       If the iterator should be the beginning 
    *                      or end.
    */
   MapsToDrawIt( const mapsToDraw_t* mapsToDraw, bool begin )
      : ToDrawIterator( mapsToDraw, begin ) {}

   /**
    *   Creates a new MapsToDrawIt according to the params.
    *   @param mapsToDraw  Ptr to mapsToDraw.
    *   @param it          MapsToDraw iterator, i.e. which importance
    *                      that is active.
    *   @param index       The index of the map at the specified importance.
    */
   MapsToDrawIt( const mapsToDraw_t* mapsToDraw, 
                 mapsToDraw_t::const_iterator& it,
                 int index ) : ToDrawIterator( mapsToDraw, it, index ) {}
           
   /**
    *    Copy constructor. 
    */
   MapsToDrawIt( const ToDrawIterator& toDrawIt ) 
      : ToDrawIterator( toDrawIt ) {}

   /**
    *    Operator *
    *    @return  The TileMap.
    */
   TileMap* operator *() const {
      return (*m_mapsToDrawIt).second->m_maps[ m_idx ];
   }
};

/**
 *    MapsToDraw iterator.   
 */
class ParamsToDrawIt : public ToDrawIterator {
public:
   /**
    *   Creates a new ParamsToDrawIt at either the beginning or end.
    *   @param mapsToDraw  Ptr to mapsToDraw.
    *   @param begin       If the iterator should be the beginning 
    *                      or end.
    */
   ParamsToDrawIt( const mapsToDraw_t* mapsToDraw, bool begin ) 
      : ToDrawIterator( mapsToDraw, begin ) {}

   /**
    *   Creates a new ParamsToDrawIt according to the params.
    *   @param mapsToDraw  Ptr to mapsToDraw.
    *   @param it          MapsToDraw iterator, i.e. which importance
    *                      that is active.
    *   @param index       The index of the map at the specified importance.
    */
   ParamsToDrawIt( const ToDrawIterator& toDrawIt ) 
      : ToDrawIterator( toDrawIt ) {}

   /**
    *    Operator *
    *    @return  The TileMapParams.
    */
   TileMapParams& operator *() const {
      return (*m_mapsToDrawIt).second->m_params[ m_idx ];
   }
};



// ----------------------- TileMapContainer -------------------------
//class MapsToDrawIt;
//class ParamsToDrawIt;

/**
 *    Contains all tilemaps.
 */
class TileMapContainer {
      
   public:

      /// Friend.
      friend class TileMapFormatDesc;
      typedef MapStorage::range_t range_t;
      
      /**
       *    Constructor.
       *    @param   projection  The projection.
       *    @param   garbage     Ptr to TileMap* garbage collector.
       */
      TileMapContainer( const MapProjection* projection,
                        TileMapGarbage<TileMap>* garbage );

      /**
       *    Destructor.
       */
      ~TileMapContainer();
     
      /**
       *    Set tilemapformatdesc.
       *    @param   mapDesc  The tilemapformatdesc.
       */
      void setMapDesc( const TileMapFormatDesc* mapDesc );

      /**
       *    Iterator of TileMaps.
       */
      typedef std::vector<TileMap*>::const_iterator MIt;

      /**
       *    Iterator of TileMapParams.
       */
      typedef std::vector<TileMapParams>::const_iterator PIt;
      
      /**
       *    @return  Beginning of maps to request.
       */
      MIt beginMapsToReq() const { return m_mapsToReq.m_maps.begin(); }
      
      /**
       *    @return  After end of maps to request.
       */
      MIt endMapsToReq() const { return m_mapsToReq.m_maps.end(); }
     
      /**
       *    @return  Beginning of params to request.
       */
      PIt beginParamsToReq() const { return m_mapsToReq.m_params.begin(); }
      
      /**
       *    @return  After end of params to request.
       */
      PIt endParamsToReq() const { return m_mapsToReq.m_params.end(); }
    
      /**
       *    Check if the maps-to-req iterator is pointing to a map
       *    that we really want to request, i.e. that it is not a
       *    known empty tilemap.
       *    @param   pos         The position in m_mapsToReq of the map.
       *    @param   requestType [OUT] From which source the map can
       *                         be requested (i.e. from cache only or
       *                         also from internet).
       *    @return  True if the map should be requested.
       */
      inline bool toRequestMap( uint32 pos,
                         DBufRequester::request_t& requestType ) const;

      /**
       *    Check if the maps-to-req iterator is pointing to a map
       *    that we really want to request, i.e. that it is not a
       *    known empty tilemap.
       *    @param   it          The maps-to-req iterator.
       *    @param   requestType [OUT] From which source the map can
       *                         be requested (i.e. from cache only or
       *                         also from internet).
       *    @return  True if the map should be requested.
       */
      inline bool toRequestMap( const MIt& it, 
                                DBufRequester::request_t& requestType ) const;
    
      /**
       *    @return If the map at specified position in m_mapsToReq 
       *            is an essential map, i.e. if it should be requested
       *            from internet if not already present.
       */
      bool isEssentialMap( int pos );
 
      /**
       *    @return Percentage of received maps / total number maps.
       */
      uint32 getPercentageMapsReceived() const; 
      
      /**
       *    @return  Beginning of reserve maps to request.
       */
      MIt beginReserveMapsToReq() const { 
         return m_reserveMaps.m_maps.begin(); }
      
      /**
       *    @return  After end of reserve maps to request.
       */
      MIt endReserveMapsToReq() const { 
         return m_reserveMaps.m_maps.end(); }
     
      /**
       *    @return  Beginning of reserve params to request.
       */
      PIt beginReserveParamsToReq() const { 
         return m_reserveMaps.m_params.begin(); }
      
      /**
       *    @return  After end of reserve params to request.
       */
      PIt endReserveParamsToReq() const { 
         return m_reserveMaps.m_params.end(); }
     
      /**
       *   Returns a pointer to the map with the given description
       *   or NULL if not loaded yet.
       */
      inline TileMap* getMap(const MC2SimpleString& desc) const;
   
     
      /**
       *    Try to add a map to a mapstorage.
       *    @param   params      The params describing the map.
       *    @param   buffer      The map buffer.
       *    @param   mapStorage  The MapStorage to add the map to.
       *    @param   removeFromCache   [OUT] Descs that are to be removed
       *                                     from the cache.
       *    @param   cancelRequestedMaps  [OUT] If a new empty tilemap
       *                                  was found among the maps to be
       *                                  requested, and therefore
       *                                  the currently requested maps
       *                                  should be canceled.
       *    @param   replaceExistingMaps  [Optional] If set to false,
       *                                  existing maps will not be 
       *                                  replaced.
       *    @return The index of the added map in mapStorage.
       *            -1, if it was not added, 
       *            -2, if it should have been added but the 
       *            some features could not be decoded due
       *            to a too old mapformatdesc.
       */
      int addMap( const TileMapParams& params,
            BitBuffer& buffer, 
            MapStorage& mapStorage,
            std::vector<TileMapParams>& removeFromCache,
            bool& cancelRequestedMaps,
            bool replaceExistingMaps = true );

      /**
       *    Adds the map.
       *    @param   params      The params describing the map.    
       *    @param   buffer      The map buffer.
       *    @param   removeFromCache   [OUT] Descs that are to be removed
       *                                     from the cache.
       *    @param   mapOK       [OUT] True if all features was decoded
       *                               properly. False if some were not,
       *                               and therefore the mapdesc should
       *                               be updated.
       *    @param   cancelRequestedMaps  [OUT] If a new empty tilemap
       *                                  was found among the maps to be
       *                                  requested, and therefore
       *                                  the currently requested maps
       *                                  should be canceled.
       *    @return  MapsToDrawIt that is endMapsToDraw() if the added map
       *             is not among the drawn maps, or else the appropriate
       *             MapsToDrawIt to the map.
       */
      MapsToDrawIt addMap( const MC2SimpleString& params, 
                           BitBuffer& buffer,
                           std::vector<TileMapParams>& removeFromCache,
                           bool& mapOK,
                           bool& cancelRequestedMaps );

      /**
       *   Update the params.
       *
       *   @param mapFormatDesc  The tilemapformatdesc.
       *   @param projection     The mapprojection.
       *   @param useGzip        If to use gzip.
       *   @param lang           The requested language for the strings.
       *   @param layersToDisplay   The layers.
       *   @param routeID           [Optional] The route id if the route 
       *                            should be present.
       *                            
       *   Returns true if new parameters have been created.
       */
      bool updateParams( TileMapFormatDesc& mapFormatDesc,
                         const MapProjection& projection,
                         bool useGzip,
                         LangTypes::language_t lang,
                         const std::set<int>& layersToDisplay,
                         const RouteID* routeID = NULL );

      /**
       *    To be called when the params have been updated.
       *    @param   newParams      The new params to request.
       *    @param   reserveParams  The new reserve params.
       */
      void paramsUpdated( std::vector<TileMapParams>& newParams,
                          std::vector<TileMapParams>& reserveParams );
  
      /**
       *    Enum indicating the available instances of MapStorage.
       */
      enum mapStorage_t {
         mapsToReq_t, 
         oldMaps_t,
         reserveMaps_t,
         prevMaps_t
      };

      /**
       *    Get the MapStorage that has the best coverage for the 
       *    importance.
       *
       *    @param   imp            The importance.
       *    @param   mapsToReq      The maps to request storage.
       *    @param   oldMaps        The old map storage.
       *    @param   reserveMaps    The reserve map (i.e. overview map) 
       *                            storage.
       *    @param   prevMaps       The previous map storage.
       *
       *    @return  The best mapstorage.
       */
      mapStorage_t getBestCoverage( const layerImp_t& imp,
                                    const MapStorage& mapsToReq,
                                    const MapStorage& oldMaps,
                                    const MapStorage& reserveMaps,
                                    const MapStorage* prevMaps = NULL );

      /**
       *    Set which mapstorage to use for drawing an importance.
       *    @param   maps     The map storage.
       *    @param   imp      The importance.
       */
      void setMapsToDraw( MapStorage& maps, 
                          const layerImp_t& imp );
   
      /**
       *    Update the maps to draw since the specified map has been
       *    added to the mapstorage.
       *    @param   param    The params of the added map.
       *    @param   maps     The map storage that it was added to.
       */
      bool updateMapsToDraw( const TileMapParams& param,
                             MapStorage& maps );
   
      /**   
       *    Count the number of received maps in m_mapsToReq
       *    as well as total number of maps.
       */
      void countMapsInMapsToReq();
      
      /**
       *    @return  True if full coverage.
       */
      static bool fullCoverage( int coverage ) { 
         return coverage == MAX_INT16; }

      /**
       *    Update number maps that has been received for m_mapsToReq.
       *    Will only count non empty maps.
       *    @param   mapStorage  The map storage.
       *    @param   index       The index of the map in map storage.
       *    @param   increase    If to increase or decrease the map count.
       */
      void updateNbrMapsReceived( const MapStorage& mapStorage, 
                                  int index, bool increase );

      /**
       *    Get the next feature that is highlightable.
       *
       *    @return  The next feature that is highlightable, or
       *             NULL if no highlightable feature is present.
       */
      const TilePrimitiveFeature* getNextHighlightFeature();

   private:      
      /**
       *    Help method to getNextHighlightFeatureOrNULL().
       *    Will update the highlight feature iterator if necessary.
       *    @return If we havn't reached the end of the 
       *            highlight map iterator.
       */
      bool updateHighlightFeatureIt();

      /**
       *    Get the next highlight feature, or NULL if the end
       *    of features was reached. In that case this method
       *    should be called again. getNextHighlightFeature()
       *    does just that.
       *    
       *    @return  The next feature that is highlightable,
       *             or NULL because there are no highlightable
       *             features or NULL because the end of the 
       *             features has been reached.
       */
      const TilePrimitiveFeature* getNextHighlightFeatureOrNULL();
   public:

      /**
       *    Pair containing the number of received maps in m_mapsToReq
       *    as well as the total number of maps 
       *    (including ones not received yet).
       *    first is received maps.
       *    second is total number maps.
       */
      std::pair<int,int> m_mapsToReqCount;
      
      /**
       *    The maps that should be requested and used.
       */
      MapStorage m_mapsToReq;

      /**
       *    Old maps, at another detaillevel that can be used if m_mapsToReq
       *    is not fully loaded yet.
       */
      MapStorage m_oldMaps;

      /**
       *    Reserve maps to use as country if nothing else is available.
       *    Contains very rough country polygons.
       */   
      MapStorage m_reserveMaps;

      /**
       *    Map of paramnotices indexed by layer nbr.
       */
      std::map<uint32, ParamsNotice> m_paramsByLayerNbr;

      /**
       *    The reserve params notice.
       */
      ParamsNotice m_reserveParamsNotice;

      /**
       *    The tilemapformatdescription.
       */
      const TileMapFormatDesc* m_mapDesc;

      /**
       *    The map projection.
       */
      const MapProjection* m_projection;

      /**
       *    The garbage collector of TileMap*s.
       */
      TileMapGarbage<TileMap>* m_garbage;
     
      /**
       *    Maps to draw type. Contains the map storages to use 
       *    for each importance to be drawn.
       */
      typedef ToDrawIterator::mapsToDraw_t mapsToDraw_t;
      
      /**
       *    Which maps to draw for each importance.
       */
      mapsToDraw_t m_mapsToDraw;
     
      /**
       *    @return  Iterator to the beginning of the maps to draw.
       */
      MapsToDrawIt beginMapsToDraw() const;
      
      /**
       *    @return  Iterator to after the end of the maps to draw.
       */
      MapsToDrawIt endMapsToDraw() const;

      /**
       *    @return  Iterator to the beginning of the maps to draw params.
       */
      ParamsToDrawIt beginParamsToDraw() const;
      
      /**
       *    @return  Iterator to after the end of the maps to draw params.
       */
      ParamsToDrawIt endParamsToDraw() const;

      /**
       *    Dump the maps to draw.
       */
      void dumpMapsToDraw();      

      /**
       *    @return The loaded map sizes.
       */
      uint32 getTotalMapSize() const;

      /**
       *    @return  If the reservemaps are complete.
       */
      bool reserveMapsComplete() const;

      /**
       *    If the reservemaps are complete.
       */
      bool m_reserveMapsComplete;

      /**
       *    Number of reservemaps.
       */
      uint32 m_nbrReserveMaps;

      /**
       *    Highlight map iterator.
       */
      MapsToDrawIt m_highlightMapIt;
      
      /**
       *    Highlight feature iterator. 
       *    Refers to the map in m_higlightMapIt.
       */
      TileMap::const_iterator m_highlightFeatureIt;
      
      /**
       *    Notice class representing a tile, i.e. latitude and longitude
       *    index and detaillevel.
       */
      class Tile {
         public:
            /// Construct a Tile object from TileMapParams.
            Tile( const TileMapParams& params ) : 
               m_layer( params.getLayer() ),
               m_latIdx( params.getTileIndexLat() ),
               m_lonIdx( params.getTileIndexLon() ),
               m_detailLevel( params.getDetailLevel() ) {
               if ( params.getRouteID() ) {
                  m_routeID = *params.getRouteID();
               }
            }

            /// Less than operator.
            inline bool operator < ( const Tile& other ) const {
               if ( m_latIdx != other.m_latIdx ) {
                  return m_latIdx < other.m_latIdx;
               } else if ( m_lonIdx != other.m_lonIdx ) {
                  return m_lonIdx < other.m_lonIdx;
               } else if ( m_detailLevel != other.m_detailLevel ) {
                  return m_detailLevel < other.m_detailLevel;
               } else if ( m_layer != other.m_layer ) {
                  return m_layer < other.m_layer;
               } else {
                  return m_routeID < other.m_routeID;
               }
            }

         private:
            /// Route ID
            RouteID m_routeID;
            /// Layer
            int m_layer;
            /// Lat index
            uint16 m_latIdx;
            /// Lon index
            uint16 m_lonIdx;
            /// Detail level
            uint8 m_detailLevel;          
      };
      
      std::map<Tile, uint32> m_emptyImpByTile; 
      
      bool emptyMapInMapsToReq();

      void addToEmptyImpTable( const TileMapParams& params, 
                               uint32 emptyImportances );
      
};


// --- Implementation of inlined methods ---

inline 
layerImp_t::layerImp_t( const TileMapParams& param,
                        const TileMapFormatDesc& mapDesc )
{
   this->first = mapDesc.getLayerNbrFromID( param.getLayer() );
   const TileImportanceNotice* notice = 
      mapDesc.getImportanceNbr( param );
   this->second = notice->getType();
   if ( notice->getThreshold() < MAX_UINT16 ) {
      this->second |= (notice->getThreshold() << 16 );
   }
}


inline TileMap*
TileMapContainer::getMap(const MC2SimpleString& params) const
{  
   return m_mapsToReq.getMap( m_mapsToReq.getMapIndex( params ) );
}

inline bool 
TileMapContainer::toRequestMap( uint32 pos,
                                DBufRequester::request_t& requestType ) const
{
   const TileMap* curMap = m_mapsToReq.m_maps[ pos ];   
   if ( curMap == NULL ) {
      bool empty = m_mapsToReq.m_emptyMaps[ pos ];
      if ( empty ) {
         return false;
      }
      const TileMapParams& params = m_mapsToReq.m_params[ pos ];
      if ( ! m_mapDesc->checkFetching( params ) ) {
         // Only request POI strings from cache.
         requestType = DBufRequester::onlyCache;
      } else {
         requestType = DBufRequester::cacheOrInternet;
      }  
      return true;
   } else {
      const TileMapParams& params = m_mapsToReq.m_params[ pos ];
      // Get the expiry time from the formatdesc
      requestType = DBufRequester::cacheOrInternet; // Should be internet
      return curMap->expired( *m_mapDesc, params );
   }
   
   return true;
}

inline bool 
TileMapContainer::toRequestMap( const MIt& it,
                                DBufRequester::request_t& requestType ) const
{
   uint32 pos = std::distance( beginMapsToReq(), it );
   MC2_ASSERT( pos < m_mapsToReq.m_maps.size() );
   return toRequestMap( pos, requestType );
}


#endif 
