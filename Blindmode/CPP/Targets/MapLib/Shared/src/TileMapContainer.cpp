/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapContainer.h"
#include "TileMapHandler.h"
#include "TileMapUtil.h"
#include "FeatureType.h"

using namespace std;

/// ----- MapStorage -----
MapStorage::MapStorage( bool containsStrings, 
                        TileMapGarbage<TileMap>* garbage )
      : m_containsStrings( containsStrings ), 
        m_mapDesc( NULL ), m_garbage( garbage ) 
{
   // To make armi r2 compiler happy. Otherwise swap might not work.
   m_maps.clear();
   m_params.clear();
   m_emptyMaps.clear();
   m_importances.clear();
}

MapStorage::~MapStorage()
{
#ifndef QUICK_SHUTDOWN   
   for ( uint32 i = 0; i < m_maps.size(); ++i ) {
      m_garbage->addGarbage( m_maps[ i ] );
   }
#endif
}

void
MapStorage::swap( MapStorage& other )
{
      m_params.swap( other.m_params );
      m_maps.swap( other.m_maps );
      m_emptyMaps.swap( other.m_emptyMaps );
      m_importances.swap( other.m_importances );
}

int 
MapStorage::getCoverage( const layerImp_t& imp, 
                         const ParamsNotice& params,
                         const MapProjection& projection,
                         const TileMapFormatDesc& mapDesc ) const
{
   int coverage = MIN_INT32;
   int nbrMapsFound = 0;
   map<layerImp_t, range_t>::const_iterator findit = 
      m_importances.find( imp );

   // Set the (maybe rotated) bounding box)
   MC2BoundingBox projBBox( projection.getBoundingBox() );

   int32 minLat    = projBBox.getMinLat();
   int32 minLon    = projBBox.getMinLon();

   int32 maxLat    = projBBox.getMaxLat();
   int32 maxLon    = projBBox.getMaxLon();
   
   if ( findit != m_importances.end() ) {
      
      int detailLevel = 
         m_params[ (*findit).second.first ].getDetailLevel();

      int startLat, endLat, startLon, endLon;
      if ( detailLevel == params.m_detailLevel ) {
         startLat = params.m_startLatIdx;
         endLat   = params.m_endLatIdx;
         startLon = params.m_startLonIdx;
         endLon   = params.m_endLonIdx;
      } else {
         mapDesc.getTileIndex( imp.first, 
                               detailLevel,
                               minLat,
                               minLon,
                               startLat, 
                               startLon );
         mapDesc.getTileIndex( imp.first, 
                               detailLevel,
                               maxLat,
                               maxLon,
                               endLat, 
                               endLon );
      }
      
      for ( int i = (*findit).second.first; 
            i < (*findit).second.second; ++i ) {
         const TileMapParams& param = m_params[ i ];
         if ( ( ( m_maps[ i ] != NULL ) || ( m_emptyMaps[ i ] ) ) &&
              param.getTileIndexLat() >= startLat &&
              param.getTileIndexLat() <= endLat &&
              param.getTileIndexLon() >= startLon &&
              param.getTileIndexLon() <= endLon ) {
            ++nbrMapsFound;
         }
      }
      coverage = int(nbrMapsFound / 
         float(( endLat - startLat + 1 ) * ( endLon - startLon + 1 )) 
            * MAX_INT16);
      mc2dbg8 << "nbrMapsFound = " << nbrMapsFound <<  ", total nbr maps = "
         << ( endLat - startLat + 1 ) * ( endLon - startLon + 1 )
         << ", coverage = " << coverage << endl;
   }

   return coverage;
}

int
MapStorage::getStrIdx( int index ) const
{
   MC2_ASSERT( (index + ( m_maps.size() >> 1 ) ) < m_maps.size() );
   return index + ( m_maps.size() >> 1);
}

int
MapStorage::getMapIdx( int strIdx ) const
{
   MC2_ASSERT( strIdx - ( m_maps.size() >> 1 ) < m_maps.size() );
   return strIdx - ( m_maps.size() >> 1);
}

int
MapStorage::getMapIndex( const TileMapParams& param ) const
{
   int index = -1;
   if ( param.getTileMapType() == TileMapTypes::tileMapStrings &&
        ! m_containsStrings ) {
      return index;
   }
   
   layerImp_t imp( param, *m_mapDesc );

   map<layerImp_t, range_t>::const_iterator findit = 
      m_importances.find( imp );
   if ( findit != m_importances.end() ) {
      for ( int i = (*findit).second.first; i < (*findit).second.second; 
            ++i ) {
         const TileMapParams& curParam = m_params[ i ];

         // Note that we don't care if it's map or strings or language.
         if ( curParam.getTileIndexLat() == param.getTileIndexLat() &&
              curParam.getTileIndexLon() == param.getTileIndexLon() &&
              curParam.getImportanceNbr() == param.getImportanceNbr() &&
              curParam.getDetailLevel() == param.getDetailLevel() &&
              curParam.getLayer() == param.getLayer() ) {
            // Also check route ID.
            const RouteID* curRouteID = curParam.getRouteID();
            const RouteID* routeID    = param.getRouteID();
            
            if ( ( curRouteID == NULL && routeID == NULL ) ||
                 ( curRouteID != NULL && routeID != NULL && 
                   *curRouteID == *routeID ) ) {
               // The param is found!
               index = i;
               // Jump out of the loop.
               break;
            }
         }
      }
   }
   if ( index >= 0 && 
        param.getTileMapType() == TileMapTypes::tileMapStrings ) {
      index += m_maps.size() / 2;
   }
   return index;
}

void
MapStorage::updateMapsFrom( MapStorage& other )
{
   for ( uint32 i = 0; i < m_params.size(); ++i ) {
      int mapIndex = other.getMapIndex( m_params[ i ] );
      if ( mapIndex >= 0 ) {
         MC2_ASSERT( m_maps[ i ] == NULL );
         mc2dbg8 << "updateMapsFrom: Updating map " << mapIndex << " "
                << other.m_maps[ mapIndex ] 
                << " "<< other.m_params[ mapIndex ].getAsString() << endl;
         m_maps[ i ] = other.m_maps[ mapIndex ];
         other.m_maps[ mapIndex ] = NULL;
         m_emptyMaps[ i ] = other.m_emptyMaps[ mapIndex ];
      }
   }
}
   
void 
MapStorage::takeImportance( MapStorage& other, const layerImp_t& imp )
{
   map<layerImp_t, range_t>::iterator otherIt = 
      other.m_importances.find( imp );
   MC2_ASSERT( otherIt != other.m_importances.end() );

   // Remove the current importance and NULL the maps.
   // If the new importance contains equally many or less maps, 
   // then put them where the old importance was.
   // Otherwise, add them at the end of the vector.
   map<layerImp_t, range_t>::iterator curIt = m_importances.find( imp );
   bool reuseOldSpace = false;
   int nbrOldImp = 0;
   int firstOldPos = -1;
   if ( curIt != m_importances.end() ) {
      nbrOldImp = (*curIt).second.second - (*curIt).second.first;
      firstOldPos = (*curIt).second.first;
      clear( &imp );
   }
   
   int nbrNewImp = (*otherIt).second.second - (*otherIt).second.first;
   if ( nbrNewImp <= nbrOldImp ) {
      // Equal or less number of maps as before. 
      // Put them where the old maps were.
      reuseOldSpace = true;
   } 
      
   int newPos;
   if ( ! reuseOldSpace ) {
      // Add them last.
      int oldSize = m_maps.size();
      int newSize = oldSize + nbrNewImp;
      m_maps.resize( newSize, NULL );
      m_params.resize( newSize );
      m_emptyMaps.resize( newSize );
      newPos = oldSize;
   } else {
      // Reuse the old space.
      newPos = firstOldPos;
   }
   
   // Update m_importances
   m_importances[ imp ] = range_t( newPos, newPos + nbrNewImp );
  
   // Add the new maps and params.
   for ( int i = (*otherIt).second.first; i < (*otherIt).second.second;
         ++i ) {
      mc2dbg8 << "TakeImportance: Moving map " << i << " " 
             << other.m_maps[ i ] << " " 
             << other.m_params[ i ].getAsString() << endl; 
      m_maps[ newPos ] = other.m_maps[ i ];
      other.m_maps[ i ] = NULL;
      m_emptyMaps[ newPos ] = other.m_emptyMaps[ i ];
      m_params[ newPos ] = other.m_params[ i ];
      ++newPos;
   }
}
   
void 
MapStorage::updateImportances()
{
   m_importances.clear();
   if ( m_params.empty() ) {
      return;
   }
   
   const TileMapParams& firstParam = m_params.front();
   layerImp_t imp( firstParam, *m_mapDesc );
   range_t range;
   range.first = 0;

   uint32 mapSize;
   if ( m_containsStrings ) {
      // Only the first half of the map vector contains data.
      mapSize = m_maps.size() >> 1;
   } else {
      mapSize = m_maps.size();
   }
   
   for ( uint32 i = 1; i < mapSize; ++i ) {
      const TileMapParams& param = m_params[ i ];
      layerImp_t curImp( param, *m_mapDesc );
      if ( curImp != imp ) {
         // New importance.
         // Add the previous one.
         range.second = i;
         m_importances[ imp ] = range;
         
         // Prepare the next one.
         range.first = i;
         imp = curImp;
      }
   }   
   // Add the previous one.
   range.second = mapSize;
   m_importances[ imp ] = range;
}

void 
MapStorage::clear( const layerImp_t* imp )
{
   if ( imp == NULL ) {
      // Clear and delete everything.
      for ( uint32 i = 0; i < m_maps.size(); ++i ) {
         mc2dbg8 << "clear all: " << i << " " << m_maps[ i ] << " " 
            << m_params[ i ].getAsString() << endl;
         m_garbage->addGarbage( m_maps[ i ] );
         m_maps[ i ] = NULL;
         m_emptyMaps[ i ] = false;
      }
      m_importances.clear();
   } else {
      // Only clear the specified importance.
      map<layerImp_t, range_t>::iterator it = m_importances.find( *imp );
      if ( it != m_importances.end() ) {
         for ( int i = (*it).second.first; i < (*it).second.second; ++i ) {
            mc2dbg8 << "clear imp: " << imp->first << "," << imp->second 
                   << " " << m_maps[ i ] << " " 
                   << m_params[ i ].getAsString() << endl;
            m_garbage->addGarbage( m_maps[ i ] );
            m_maps[ i ] = NULL;
            m_emptyMaps[ i ] = false;
         }
         m_importances.erase( it );
      }
   }
}

void
MapStorage::setMapDesc( const TileMapFormatDesc* mapDesc )
{
   // Set the new map desc.
   m_mapDesc = mapDesc;

   clear();
   m_maps.clear();
   m_params.clear();
   m_emptyMaps.clear(); 
}

/// ----- TileMapContainer -----

TileMapContainer::TileMapContainer( const MapProjection* projection,
                                    TileMapGarbage<TileMap>* garbage )
      : m_mapsToReq( true, garbage ),   // contains strings
        m_oldMaps( false, garbage ),    // no strings
        m_reserveMaps( false, garbage ), // no strings
        m_projection( projection ),
        m_garbage( garbage ),
        m_highlightMapIt( endMapsToDraw() )
{
   m_mapDesc = NULL;
   m_nbrReserveMaps = 0;
}

      
TileMapContainer::~TileMapContainer()
{

}

void
TileMapContainer::setMapDesc( const TileMapFormatDesc* mapDesc )
{
   m_mapDesc = mapDesc;
   m_mapsToReq.setMapDesc( mapDesc );
   m_oldMaps.setMapDesc( mapDesc );
   m_reserveMaps.setMapDesc( mapDesc );
   // Clear m_mapsToDraw.
   m_mapsToDraw.clear();
   m_highlightMapIt = endMapsToDraw();
   
   m_paramsByLayerNbr.clear();
}

inline void
TileMapContainer::paramsUpdated( vector<TileMapParams>& newParams,
                                 vector<TileMapParams>& reserveParams ) 
{
   // Clear the old mapstodraw.
   m_mapsToDraw.clear();
   MapStorage prevMapsToReq( true, m_garbage ); // Contains strings.
   prevMapsToReq.setMapDesc( m_mapDesc );
   prevMapsToReq.swap( m_mapsToReq );
   
   m_mapsToReq.m_maps.resize( newParams.size(), NULL );
   m_mapsToReq.m_emptyMaps.resize( newParams.size(), false );
   m_mapsToReq.m_params.swap( newParams );
      
   m_mapsToReq.updateMapsFrom( prevMapsToReq );

   // Delete all the strings in prevMaps, i.e. the second half of the maps.

   MC2_ASSERT( ( prevMapsToReq.m_params.size() & 1 ) == 0 ); // Must be even.
   uint32 strIdxStart = prevMapsToReq.m_maps.size() >> 1;
   for ( uint32 i = strIdxStart; i < prevMapsToReq.m_maps.size(); ++i ) {
      mc2dbg8 << "Deleting string map " << i << " " 
             << prevMapsToReq.m_maps[ i ] << " "
             << prevMapsToReq.m_params[ i ] << endl;
      m_garbage->addGarbage( prevMapsToReq.m_maps[ i ] );
      prevMapsToReq.m_maps[ i ] = NULL;
   }
   prevMapsToReq.m_params.resize( strIdxStart );
   prevMapsToReq.m_maps.resize( strIdxStart );
   prevMapsToReq.m_emptyMaps.resize( strIdxStart );
   prevMapsToReq.m_containsStrings = false;
 
#ifdef NAV2_CLIENT_SERIES60_V1
   // Need to save memory for s60 v1 clients. Don't store any old maps.
   prevMapsToReq.clear();
#endif
   
   // Update reserveParams if we got any new ones.
   if ( ! reserveParams.empty() ) {
      MapStorage prevReserve( false, m_garbage ); // No strings.
      prevReserve.setMapDesc( m_mapDesc );
      prevReserve.swap( m_reserveMaps );

      m_reserveMaps.m_maps.resize( reserveParams.size(), NULL );
      m_reserveMaps.m_emptyMaps.resize( reserveParams.size(), false );
      m_reserveMaps.m_params.swap ( reserveParams );

      m_reserveMaps.updateMapsFrom( prevReserve );
      m_reserveMaps.updateImportances(); 
      prevReserve.clear();
   }    
   
   // First check if we have any tiles in oldMaps 
   // that we can directly use.
   
   m_mapsToReq.updateMapsFrom( m_oldMaps );
   m_mapsToReq.updateImportances(); 

   emptyMapInMapsToReq();
   if ( m_emptyImpByTile.size() > 100 ) {
      m_emptyImpByTile.clear();
   }

   // Get the best storage for each importance.
   for ( map<layerImp_t, range_t>::const_iterator it = 
            m_mapsToReq.m_importances.begin(); 
         it != m_mapsToReq.m_importances.end(); ++it ) {

      const layerImp_t& imp = (*it).first;
      mapStorage_t bestStorage = getBestCoverage( imp,
                                                  m_mapsToReq,
                                                  m_oldMaps,
                                                  m_reserveMaps,
                                                  &prevMapsToReq );
//      bestStorage = mapsToReq_t;
      switch ( bestStorage ) {
         case ( mapsToReq_t ) :
            m_oldMaps.clear( &imp );
            setMapsToDraw( m_mapsToReq, imp );
            break;
         case ( oldMaps_t ) :
            setMapsToDraw( m_oldMaps, imp );
            break;
         case ( reserveMaps_t ) :
            m_oldMaps.clear( &imp );
            setMapsToDraw( m_reserveMaps, imp );
            break;
         case ( prevMaps_t ) :
            // Move the importance to oldMaps, and use oldmaps 
            // when drawing.
            m_oldMaps.takeImportance( prevMapsToReq, imp );
            setMapsToDraw( m_oldMaps, imp );
            break;
      }
   } 
   
   // Clear the maps left in prevMapsToReq.
   prevMapsToReq.clear();

   // Clean up the old maps.
   m_oldMaps.cleanUp();
   
//     mc2log << "At end of paramsUpdated:" << endl;
//     mc2log << "Reserve maps:" << endl;
//     m_reserveMaps.dump();
//     mc2log << "Old maps:" << endl;
//     m_oldMaps.dump();
//     mc2log << "MapsToReq:" << endl;
//     m_mapsToReq.dump();

   countMapsInMapsToReq();
   // Reset the highlight iterator
   m_highlightMapIt = endMapsToDraw();
}

bool
TileMapContainer::updateParams( TileMapFormatDesc& mapFormatDesc,
                                const MapProjection& projection,
                                bool useGzip,
                                LangTypes::language_t lang,
                                const set<int>& layersToDisplay,
                                const RouteID* routeID )
{
   vector<TileMapParams> newParams;
   vector<TileMapParams> reserveParams;
   
   if ( mapFormatDesc.createParams(m_paramsByLayerNbr,
                                   m_reserveParamsNotice,
                                   newParams,
                                   reserveParams,
                                   projection, useGzip,
                                   lang,
                                   layersToDisplay, routeID) ) {
//      // New params were created.      
      paramsUpdated( newParams, reserveParams );
      
      return true; 
   }
   return false;
}


TileMapContainer::mapStorage_t
TileMapContainer::getBestCoverage( const layerImp_t& imp,
                                   const MapStorage& /*mapsToReq*/,
                                   const MapStorage& /*oldMaps*/,
                                   const MapStorage& /*reserveMaps*/,
                                   const MapStorage* prevMaps /* = NULL */ )
{
   const ParamsNotice& paramNotice = m_paramsByLayerNbr[ imp.first ];

   int mapsToReqCov = m_mapsToReq.getCoverage( imp, 
                                               paramNotice,
                                               *m_projection,
                                               *m_mapDesc );
   if ( fullCoverage( mapsToReqCov ) ) {
      return mapsToReq_t;
   } else {

      int prevMapsToReqCov = 0;
      int oldMapsCov = 0;
      int reserveCov = 0;
      if ( prevMaps != NULL ) {
         prevMapsToReqCov = 
            prevMaps->getCoverage( imp,
                                   paramNotice,
                                   *m_projection,
                                   *m_mapDesc );
      }
      
      if ( fullCoverage( prevMapsToReqCov ) ) {
         return prevMaps_t;
      } else {
         oldMapsCov = m_oldMaps.getCoverage( imp, 
                                             paramNotice,
                                             *m_projection,
                                             *m_mapDesc );
         if ( fullCoverage( oldMapsCov ) ) {
            return oldMaps_t;
         } else {
            reserveCov =  m_reserveMaps.getCoverage( imp, 
                                                     paramNotice,
                                                     *m_projection,
                                                     *m_mapDesc );
            if ( fullCoverage( reserveCov ) ) {
               return reserveMaps_t;
            }
         }
      }
      
      // Have no maps that fully cover the requested area.
      // Reserv maps are out of the game. 
      // They are not used unless they fully cover.
     
//      if ( mapsToReqCov >= prevMapsToReqCov && 
//           mapsToReqCov >= oldMapsCov ) {
//         return mapsToReq_t;
//      } else if ( prevMapsToReqCov >= oldMapsCov ) {
//         return prevMaps_t;
//      } else {
//         return oldMaps_t;
//      }
      
      if ( mapsToReqCov >= prevMapsToReqCov && 
           mapsToReqCov >= oldMapsCov &&
           mapsToReqCov >= reserveCov ) {
         return mapsToReq_t;
      } else if ( prevMapsToReqCov >= oldMapsCov &&
                  prevMapsToReqCov >= reserveCov ) {
         return prevMaps_t;
      } else if ( oldMapsCov >= reserveCov ) {
         return oldMaps_t;
      } else {
         return reserveMaps_t;
      }
   } 
}

void
TileMapContainer::setMapsToDraw( MapStorage& maps, 
                                 const layerImp_t& imp )
{
#if 0
   mc2dbg << "setMapsToDraw: " << imp.first << "," << imp.second;
   if ( &maps == &m_mapsToReq ) {
      mc2dbg << " m_mapsToReq" << endl;
   } else if ( &maps == &m_oldMaps ) {
      mc2dbg << " m_oldMaps" << endl;
   } else if ( &maps == &m_reserveMaps ) {
      mc2dbg << " m_reserveMaps" << endl;
   }
#endif
   
   // Check that there are maps in the importance.
   map<layerImp_t, range_t>::iterator it = maps.m_importances.find( imp );
   if ( it != maps.m_importances.end() && 
        (*it).second.first < (*it).second.second ) {
      // The importance contains data.
      m_mapsToDraw[ imp ] = &maps;
      mc2dbg8 << "setMapsToDraw: Added importance" << endl;
   } else {
      // No data in the importance. Clear this importance.
      m_mapsToDraw.erase( imp );
      mc2dbg8 << "setMapsToDraw: No data in importance!" << endl;
   }
#if 0
   mc2dbg << "setMapsToDraw:" << endl;
   {for ( mapsToDraw_t::iterator it = m_mapsToDraw.begin(); 
         it != m_mapsToDraw.end(); ++it ) {
      mc2dbg << "mapsToDraw: layerImp:" << it->first.first
         << ", " << it->first.second
         << " ";
      if ( it->second == &m_mapsToReq ) {
         mc2dbg << "m_mapsToReq" << endl;
      } else if ( it->second == &m_oldMaps ) {
         mc2dbg << "m_oldMaps" << endl;
      } else if ( it->second == &m_reserveMaps ) {
         mc2dbg << "m_reserveMaps" << endl;
      }
      //it->second->dump( it->first );
   }}
#endif
}
   
bool
TileMapContainer::updateMapsToDraw( const TileMapParams& param,
                                    MapStorage& maps )
{
   // Get the layer and importance.
   layerImp_t imp( param, *m_mapDesc );
   
   mapsToDraw_t::iterator it = m_mapsToDraw.find( imp ); 
  
   if ( it != m_mapsToDraw.end() ) {
      MapStorage* currMapStorage = (*it).second;
      if ( currMapStorage == &m_mapsToReq || 
           currMapStorage == &maps ) {
         return false;
      }
      const ParamsNotice& paramNotice = m_paramsByLayerNbr[ imp.first ];

      if ( fullCoverage(  maps.getCoverage( imp,
                                            paramNotice,
                                            *m_projection,
                                            *m_mapDesc ) ) ) {
         mc2dbg8 << "updateMapsToDraw: Importance " << imp.first << ","
                 << imp.second << " replaced by better one!" 
                 << endl;
         MC2_ASSERT( &maps == &m_mapsToReq );
         setMapsToDraw( maps, imp );
         if ( currMapStorage == &m_oldMaps ) {
            m_oldMaps.cleanUp();
         }
         return true;
      } else {
         // Keep the previous map storage since the new one
         // didn't fully cover.
         return false;
      }

   } else {
      mc2dbg8 << "updateMapsToDraw: Added a whole new importance. " 
         << imp.first << "," << imp.second << endl; 
      // The importance was not present among m_mapsToDraw even.
      setMapsToDraw( maps, imp );
      return true; 
   }
}

void
TileMapContainer::addToEmptyImpTable( const TileMapParams& params, 
                                      uint32 emptyImportances )
{
   m_emptyImpByTile[ params ] = emptyImportances;
}
 
bool
TileMapContainer::emptyMapInMapsToReq()
{

   bool emptyMapFound = false; 
   for ( uint32 i = 0; i < m_mapsToReq.m_params.size(); ++i ) {
      if ( ! m_mapsToReq.m_emptyMaps[ i ] ) {
         const TileMapParams& param = m_mapsToReq.m_params[ i ];
         map<Tile, uint32>::const_iterator findIt = 
            m_emptyImpByTile.find( param );
         if ( findIt != m_emptyImpByTile.end() && 
              ( (*findIt).second & 
                ( 1 << param.getImportanceNbr() ) ) ) {
            mc2log << "Found empty map " << param.getAsString() << endl;
//#ifndef __unix__            
#if 0
            FILE* fil = fopen("E:\\emptylog.txt", "a");
            if ( fil ) {
               fprintf(fil, "Found empty map %s\n", param.getAsString().c_str());
               fclose(fil);
            }
#endif            
            emptyMapFound = true;
            m_mapsToReq.m_emptyMaps[ i ] = true;
         }
      }
   }

   return emptyMapFound;
}

bool
TileMapContainer::isEssentialMap( int pos )
{
   if ( m_mapsToReq.m_emptyMaps[ pos ] ) {
      return false;
   }

   const TileMapParams& params = m_mapsToReq.m_params[ pos ];
   if ( ! m_mapDesc->checkFetching( params ) ) {
      // Should only be requested from cache.
      return false;
   } else {
      // Should be requested from either cache or internet.
      return true;
   }  
}

void
TileMapContainer::countMapsInMapsToReq() 
{
   m_mapsToReqCount.first = 0;
   m_mapsToReqCount.second = 0;
   for ( MIt it = beginMapsToReq(); it != endMapsToReq(); ++it ) {
      uint32 pos = std::distance( beginMapsToReq(), it );
      if ( isEssentialMap( pos ) ) {
         if ( *it != NULL ) {
            // Nbr maps found.
            ++m_mapsToReqCount.first;
         }
         // Total nbr maps.
         ++m_mapsToReqCount.second;
      }
   }
}

uint32
TileMapContainer::getPercentageMapsReceived() const 
{
   if ( m_mapsToReqCount.second == 0 ) {
      return 100;
   }

   mc2dbg << "[TMC]: Number of maps received: " << m_mapsToReqCount.first
          << "/" << m_mapsToReqCount.second << endl;
   
   // Return the percentage of maps that are received in m_mapsToReq.
   return ( m_mapsToReqCount.first * 100 ) / m_mapsToReqCount.second;
}

void
TileMapContainer::updateNbrMapsReceived( const MapStorage& mapStorage, 
                                         int index, bool increase )
{
   // Only update for m_mapsToReq
   if ( &mapStorage == &m_mapsToReq ) {

      if ( isEssentialMap( index ) ) { 
         if ( increase ) {
            ++m_mapsToReqCount.first;
         } else {
            --m_mapsToReqCount.first;
         }
      }
   }
}

int
TileMapContainer::addMap(const TileMapParams& params,
                         BitBuffer& buffer, 
                         MapStorage& mapStorage,
                         vector<TileMapParams>& removeFromCache,
                         bool& cancelRequestedMaps,
                         bool replaceExistingMaps )
{
   
   cancelRequestedMaps = false;
   int index = mapStorage.getMapIndex( params );
   if ( index >= 0 ) {

      if ( mapStorage.m_maps[ index ] != NULL ) {
         if ( ! replaceExistingMaps ) {
            // The map was already found and we should not replace
            // existing maps, so return from method.
            return -1;
         }
         m_garbage->addGarbage( mapStorage.m_maps[ index ] );
         // NULL out the map since it's deleted, and maybe the 
         // replacing map could not be loaded.
         mapStorage.m_maps[ index ] = NULL;

         // Decrease number maps received.
         updateNbrMapsReceived( mapStorage, index, false );
      }

      TileMap* curMap = new TileMap;
      if ( ! curMap->load( buffer, *m_mapDesc, params ) ) {
         // Could not load map! 
         mc2log << "TMC::addMap - Loading of tilemap " << params 
                << " failed. New DXX is needed!" << endl;
         // Also remove the buffer from the cache, so that it will 
         // request a new one in case the server sent a corrupt map 
         // for some reason.
         removeFromCache.push_back( params );
         delete curMap;
         return -2;  // -2 means that map was not properly decoded.
      }

      // Check if the map is expired and remove it from the cache.
      if ( curMap->expired( *m_mapDesc, params ) ) {
         removeFromCache.push_back( params );
      }
      
      // Check CRC.
      if ( mapStorage.m_containsStrings ) {
         int otherIdx = 0;
         if ( params.getTileMapType() == TileMapTypes::tileMapStrings ) {
            // Check corresponding feature map.
            otherIdx = mapStorage.getMapIdx( index );
         } else {
            // Check corresponding string map.
            otherIdx = mapStorage.getStrIdx( index );
         }
         if ( mapStorage.m_maps[ otherIdx ] != NULL &&
              mapStorage.m_maps[ otherIdx ]->getCRC() !=
              curMap->getCRC() ) {
            // CRC differs! Throw away both maps.
            // Remove these maps from the cache also.
            removeFromCache.push_back( params );
            removeFromCache.push_back( mapStorage.m_params[ otherIdx ] );
            mc2log << "[TMC]: CRC differs for " << params << " "
                   << " : " << curMap->getCRC() << " - "
                   << mapStorage.m_maps[ otherIdx ]->getCRC()
                   << endl;
            m_garbage->addGarbage( curMap );
            m_garbage->addGarbage( mapStorage.m_maps[ otherIdx ] );
            mapStorage.m_maps[ otherIdx ] = NULL;
            m_garbage->addGarbage( mapStorage.m_maps[ index ] );
            mapStorage.m_maps[ index ] = NULL;
            // Decrease number maps received.
            updateNbrMapsReceived( mapStorage, otherIdx, false );
            return -1;
         }
      }

      // Check if any empty importances is present for this tile.
      if ( curMap->getEmptyImportances() != 0 ) {
         // Update empty imp table.
         addToEmptyImpTable( params, curMap->getEmptyImportances() );
         if ( emptyMapInMapsToReq() ) {
            // Cancel requested maps, since new empty map found in
            // maps to be requested.
            cancelRequestedMaps = true;
         }
      }
      
      mapStorage.m_maps[ index ] = curMap;
      mc2dbg8 << "addMap " << index << " " << params
              << " - " << mapStorage.m_params[ index ] << endl;
      
      // Update the map count for the maps to request.
      if ( cancelRequestedMaps ) {
         countMapsInMapsToReq();
      } else {
         // Increase number maps received.
         updateNbrMapsReceived( mapStorage, index, true );
      }

      if ( params.getTileMapType() == TileMapTypes::tileMapData &&
           &mapStorage == &m_mapsToReq ) {
         if ( updateMapsToDraw( params, mapStorage ) ) {
            // Reset the highlight iterator.
            m_highlightMapIt = endMapsToDraw();
         }
      }
   }
   return index;
}

MapsToDrawIt
TileMapContainer::addMap(const MC2SimpleString& params,
                         BitBuffer& buffer,
                         vector<TileMapParams>& removeFromCache,
                         bool& mapOK,
                         bool& cancelRequestedMaps )
{
   mapOK = true;
   TileMapParams tileMapParams( params );
   
   MapStorage* mapStorage = NULL;
   int index = addMap( tileMapParams, buffer, m_reserveMaps, 
                       removeFromCache, cancelRequestedMaps, false );
   if ( index >= 0 ) {
      mapStorage = &m_reserveMaps;
   } else if ( index == -1 ) { // Not found in the mapstorage. Try another.
      index = addMap( tileMapParams, buffer, m_mapsToReq, 
                      removeFromCache, cancelRequestedMaps );
      if ( index >= 0 ) {
         mapStorage = &m_mapsToReq;
      }
   } 
   if ( index == -2 ) {
      // The features of the map were not decoded properly.
      // Signal that a new DXX is needed.
      mapOK = false;
      // Return from method.
      return endMapsToDraw();
   }

   if ( mapStorage == NULL ) {
      mc2log << "[TMC] addMap - Refused to add " << params << endl;
      // Men fråga inte efter den hela tiden då!
      return endMapsToDraw();
   } else {
      // Text placement must restart.
      mc2log << "[TMC]: exiting addMap" << endl;
      return beginMapsToDraw();
   }

#if 0
   // This will not be needed until the textplacement uses it.
   if ( mapStorage != NULL ) {
      
      // Return a mapsToDraw iterator in case this map will be among
      // the mapstodraw.
      layerImp_t imp( tileMapParams, *m_mapDesc );
      mapsToDraw_t::const_iterator it = m_mapsToDraw.find( imp );
      if ( it != m_mapsToDraw.end() &&
           (*it).second == mapStorage ) {
         if ( tileMapParams.getTileMapType() == 
              TileMapTypes::tileMapStrings ) {
            // Convert index from string index to map index. 
            index -= (mapStorage->m_maps.size() >> 1);
            MC2_ASSERT( index >= 0 );
         }
        return MapsToDrawIt( &m_mapsToDraw, it, index );
      }
   }
   return endMapsToDraw();
#endif
}

MapsToDrawIt 
TileMapContainer::beginMapsToDraw() const { 
   return MapsToDrawIt( &m_mapsToDraw, true );
}

MapsToDrawIt 
TileMapContainer::endMapsToDraw() const { 
   return MapsToDrawIt( &m_mapsToDraw, false );
}

ParamsToDrawIt 
TileMapContainer::beginParamsToDraw() const { 
   return ParamsToDrawIt( &m_mapsToDraw, true );
}

ParamsToDrawIt 
TileMapContainer::endParamsToDraw() const { 
   return ParamsToDrawIt( &m_mapsToDraw, false );
}

void 
TileMapContainer::dumpMapsToDraw() 
{
#ifdef __unix
   mc2log << "TMC: dumpMapsToDraw" << endl;
   for ( mapsToDraw_t::iterator it = m_mapsToDraw.begin(); 
         it != m_mapsToDraw.end(); ++it ) {
      mc2log << "layerImp:" << it->first.first << ", " << it->first.second
         << " ";
      if ( it->second == &m_mapsToReq ) {
         mc2log << "m_mapsToReq" << endl;
      } else if ( it->second == &m_oldMaps ) {
         mc2log << "m_oldMaps" << endl;
      } else if ( it->second == &m_reserveMaps ) {
         mc2log << "m_reserveMaps" << endl;
      }
      it->second->dump( it->first );
   }
   mc2log << "Iterating:" << endl;
   for( MapsToDrawIt mIt = beginMapsToDraw();
         mIt != endMapsToDraw(); ++mIt ) {
      mc2log << (*ParamsToDrawIt( mIt )).getAsString();
      if ( *mIt != NULL ) {
         mc2log << " not NULL" << endl;
      } else {
         mc2log << " NULL" << endl;
      }
   }
#endif
}
      
uint32
TileMapContainer::getTotalMapSize() const
{
   // Maps to req.
   mc2log << "[TMC]: Maps to req:" << endl;
   uint32 mapsToReqSize = m_mapsToReq.getMapSizes();
   mc2log << "[TMC]: Old maps:" << endl;
   m_oldMaps.getMapSizes();
   mc2log << "[TMC]: Reserve maps:" << endl;
   m_reserveMaps.getMapSizes();
   
   return mapsToReqSize;
}

bool
TileMapContainer::reserveMapsComplete() const
{
   return m_reserveMapsComplete;
}
     
bool
TileMapContainer::updateHighlightFeatureIt()
{
   bool retVal = true;
   if ( m_highlightMapIt != endMapsToDraw() ) {
      // If the map is not NULL, then update the feature iterator.
      if ( *m_highlightMapIt != NULL ) {
         m_highlightFeatureIt = (*m_highlightMapIt)->begin();
      }
   } else {
      // Stop.
      retVal = false;
   }
   return retVal; 
}

const TilePrimitiveFeature* 
TileMapContainer::getNextHighlightFeatureOrNULL()
{

   // If to continue. 
   bool cont = true;
   
   // Rewind the iterators if necessary.
   if ( m_highlightMapIt == endMapsToDraw() ) {
      // Rewind.
      m_highlightMapIt = beginMapsToDraw();
      
      // Update the feature iterator.
      cont = updateHighlightFeatureIt();
   }

   while ( cont ) {
      // The current tilemap.
      const TileMap* theMap = *m_highlightMapIt;
      if ( theMap != NULL ) {
         while ( m_highlightFeatureIt != theMap->end() ) {
            TileFeature& feature = 
               const_cast<TileFeature&> (*m_highlightFeatureIt);
            const TilePrimitiveFeature* prim = 
               theMap->getOnePrimitive( feature );

            if ( (prim != NULL ) && ( prim->getType() == 
                   FeatureType::bitmap ) &&
                  prim->isDrawn() ) {
               // Return the feature and update the iterator.
               ++m_highlightFeatureIt;
               return prim;
            } else {
               // Continue with next feature.
               ++m_highlightFeatureIt;
            }
         }
      }
      
      // Continue with next map.
      ++m_highlightMapIt;
      
      // Update the feature iterator.
      cont = updateHighlightFeatureIt();
   } 

   return NULL;
}

const TilePrimitiveFeature* 
TileMapContainer::getNextHighlightFeature()
{
   const TilePrimitiveFeature* prim = getNextHighlightFeatureOrNULL();
   if ( prim == NULL ) {
      // Try again. In case we reached the end of the features we need 
      // to this.
      prim = getNextHighlightFeatureOrNULL();
   }
   return prim;
}



