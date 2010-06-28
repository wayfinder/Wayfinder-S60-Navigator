/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_FEATURE_H
#define TILE_FEATURE_H

#include "TileMapConfig.h"
#include "TileFeatureArg.h"
#include "MC2Point.h"
#include "PixelBox.h"

#include "BitBuffer.h"
#include <map>
#include <vector>
class TileMapFormatDesc;
class MapProjection;
class TileFeatureArg;

class TileMapNameSettings {
public:
   enum name_t {
      on_roundrect,
      on_bitmap,
      on_line,
      above_line,
      under_line,
      horizontal,
      inside_polygon
   };
      
   typedef uint32 color_t;
      
   // Fonts etc.
   const char* m_fontName;
   name_t m_type; // ???
   uint8 m_fontSize;
   color_t m_fontColor;
   color_t m_backgroundColor;
   const char* m_bitmap;
   int8 m_bitmapXOffset;
   int8 m_bitmapYOffset;
};

template<class T> class VectorProxy {
public:

   /**
    *   Creates an empty vector proxy which will probably
    *   crash when used.
    */
   VectorProxy() {}
   
   /**
    *   Creates a new VectorProxy using the real vector
    *   from position startIdx to position startIdx + size. <br/>
    *   <b>BE CAREFUL!</b> There must be enough room for the maximum
    *   size that this vector will have in the other vector.
    *   @param realVector Real vector where the storage is.
    *   @param startIdx   Start index in the real vector. Will never change.
    */
   VectorProxy( std::vector<T>& realVector, int startIdx, int size )
      : m_realVector(&realVector),
        m_startIdx(startIdx),
        m_endIdx(startIdx+size) {}

   typedef typename std::vector<T>::iterator       iterator;
   typedef typename std::vector<T>::const_iterator const_iterator;
   
   typename std::vector<T>::iterator begin() {
      return m_realVector->begin() + m_startIdx;
   }

   typename std::vector<T>::iterator end() {
      return m_realVector->begin() + m_endIdx;
   }
   
   typename std::vector<T>::const_iterator begin() const {
      return m_realVector->begin() + m_startIdx;
   }

   typename std::vector<T>::const_iterator end() const {
      return m_realVector->begin() + m_endIdx;
   }
   
   uint32 size() const {
      return m_endIdx - m_startIdx;
   }

   uint32 empty() const {
      return m_endIdx == m_startIdx;
   }

   void clear() {
      m_endIdx = m_startIdx;
   }

   void push_back(const T& val) {
      MC2_ASSERT( m_realVector->capacity() > m_endIdx );
      (*m_realVector)[m_endIdx++] = val;
   }

   const T& operator[](int i) const {
      MC2_ASSERT( m_realVector->capacity() > m_startIdx+i );
      return (*m_realVector)[m_startIdx+i];
   }
   
   const T& back() const {
      MC2_ASSERT( m_realVector->capacity() > m_endIdx-1 );
      return (*m_realVector)[m_endIdx-1];
   }

   const T& front() const {
      MC2_ASSERT( m_realVector->capacity() > m_startIdx );
      return (*m_realVector)[m_startIdx];
   }
    
private:
   /// The real vector
   std::vector<T>* m_realVector;
   /// The start index of this "vector" in the real vector.
   uint32 m_startIdx;
   /// The end index of this "vector" in the real vector.
   uint32 m_endIdx;
   
   
};

class TileFeature;

/**
 *    Like TileFeature, but it does not delete its arguments.
 *    They belong to the feature.
 */
class TilePrimitiveFeature {
private:
   /// Creates TileMaps in the server
   friend class TileMapCreator;
   /// Expands features in client.
   friend class TileMapFormatDesc;
   /// Creates Features in the server
   friend class WritableTileMap;
public:
   /**
    *   Primitives have negative values here and the non-primitives
    *   should not be in the code.
    */
   enum tileFeature_t {
      /// Circle primitive.
      circle = -4,
      /// Bitmap primitive.
      bitmap = -3,
      /// Builtin primitive for polygons (filled)
      polygon = -2,
      /// Builtin primitive for lines
      line    = -1,
      /// Forbidden numbers!!! These should not be hard-coded.
      street_class_0 = 0,
      street_class_1 = 1,
      street_class_2 = 2,
      street_class_3 = 3,
      street_class_4 = 4,
      street_class_0_level_0 = 5,
      street_class_1_level_0 = 6,
      street_class_2_level_0 = 7,
      street_class_3_level_0 = 8,
      street_class_4_level_0 = 9,
      poi = 10,
      water = 11,
      park = 12,
      land = 13,
      building_area = 14,
      building_outline = 15,
      bua = 16,
      railway = 17,
      ocean = 18,
      route = 19,
      route_origin = 20,
      route_destination = 21,
      route_park_car = 22,
      // Here comes the POI:s.
      city_centre_2 = 23, 
      city_centre_4 = 24, 
      city_centre_5 = 25, 
      city_centre_7 = 26, 
      city_centre_8 = 27, 
      city_centre_10 = 28, 
      city_centre_11 = 29, 
      city_centre_12 = 30, 
      atm = 31,
      golf_course = 32,
      ice_skating_rink = 33,
      marina = 34,
      vehicle_repair_facility = 35,
      bowling_centre = 36,
      bank = 37,
      casino = 38,
      city_hall = 39,
      commuter_railstation = 40,
      courthouse = 41,
      historical_monument = 42,
      museum = 43, 
      nightlife = 44,
      post_office = 45,
      recreation_facility = 46,
      rent_a_car_facility = 47,
      rest_area = 48,
      ski_resort = 49,
      sports_activity = 50,
      theatre = 51,
      tourist_attraction = 52,
      church = 53,
      winery = 54,
      parking_garage = 55,
      park_and_ride = 56,
      open_parking_area = 57,
      amusement_park = 58,
      library = 59,
      school = 60, 
      grocery_store = 61, 
      petrol_station = 62,
      tram_station = 63,
      ferry_terminal = 64,
      cinema = 65,
      bus_station = 66,
      railway_station = 67,
      airport = 68,
      restaurant = 69,
      hotel = 70,
      tourist_office = 71,
      police_station = 72,
      hospital = 73,
      toll_road = 74,
      // The forgotten ones.
      island = 75,
      university = 76,
      border = 77,
      wlan = 78,
      custom_poi_2 = 79,
      custom_poi_4 = 80,
      custom_poi_7 = 81,
      custom_poi_10 = 82,
      custom_poi_15 = 83,
      custom_poi_20 = 84,
      custom_poi_30 = 85,
      petrol_station_with_price = 86,
      /// Unkown traffic
      traffic_default = 87,
      /// Traffic known to be roadwork
      roadwork        = 88,
      /// Speed camera
      speed_camera    = 89,
      /// Pannon shop
      pannon_shop     = 90,
      /// Speed trap
      speed_trap      = 91,
      /// airport item
      airport_ground = 92,
      /// A1 shop
      a1_shop     = 93,
      mosque = 94,
      synagogue = 95,
      telekom_austria_shop = 96,
      o2_shop = 97,
      turkish_hospital = 98,
      subway_station = 99,
      cafe = 100,
      hindu_temple = 101,
      buddhist_site = 102,
      oehv_hotel     = 103,
      simobil_shop     = 104,
      /// User defined speed camera
      user_defined_speed_camera    = 105,
      street_class_0_ramp = 106,
      street_class_1_ramp = 107,
      street_class_2_ramp = 108,
      /**
       * Contains some cartographic types, that are considered to be
       * green areas, similar to park items, where you can sit down
       * and have "picnic"
       */
      cartographic_green_area = 109,
      /**
       * Most cartographic types = "all other types", that are not included
       * in any specific cartographic tile feature type (like the green area)
       */
      cartographic_ground = 110,
      forest = 111,
      aircraftroad = 112,

      special_custom_poi_2 = 113,
      special_custom_poi_4 = 114,
      special_custom_poi_7 = 115,
      special_custom_poi_10 = 116,
      special_custom_poi_15 = 117,
      special_custom_poi_20 = 118,
      special_custom_poi_30 = 119,

      shop = 120,

      nbr_tile_features // Last one
   };

   /**
    *    Creates a new TilePrimitiveFeature and sets the type
    *    to the supplied one.
    */
   TilePrimitiveFeature( int32 type = MAX_INT32 ) :
      m_type(type),
      m_parentFeature(-1),
      m_drawn( false )
      {}

#ifdef __VC32__
   /**
    *    Try to outsmart the Visual C++ compiler which cannot handle
    *    vector(vector&).
    */
   TilePrimitiveFeature( const TilePrimitiveFeature& other )
      : m_type(other.m_type), 
        m_parentFeature(other.m_parentFeature),
        m_drawn(other.m_drawn) {
      // This is the replacement for the vector copy constructor which
      // VC++ cannot handle.
      m_args.insert(m_args.end(), other.m_args.begin(), other.m_args.end());
   }
#endif
      
   
   typedef std::vector<TileFeatureArg*>::iterator arg_iterator;
   typedef std::vector<TileFeatureArg*>::const_iterator const_arg_iterator;

   inline const_arg_iterator arg_begin() const;

   inline const_arg_iterator arg_end() const;

   inline arg_iterator arg_begin();

   inline arg_iterator arg_end();

   /**
    * @return  The number of args.
    */
   inline uint32 getNbrArgs() const;

   inline int32 getType() const;
   
   /// Get the type to use when drawing the feature.
   inline int32 getDrawType() const;

   //inline int16 getNameType() const;
   
   /**
    *    Gets the argument with the supplied name.
    *    Linear search for now.
    */
   inline TileFeatureArg* getArg( TileArgNames::tileArgName_t name );
   
   /**
    *    Gets the argument with the supplied name.
    *    Linear search for now.
    */
   inline const TileFeatureArg* getArg( TileArgNames::tileArgName_t name )
      const;

   /**
    *    Sets the parent feature number for the primitive feature
    *    or the feature number for the real features.
    */ 
   inline void setFeatureNbr(int i);
   
   /**
    *    Returns the parent feature number for the primitive features
    *    or the feature number for the complex features.
    */
   inline int getFeatureNbr() const;

   /**
    *    Load method. 
    *    
    *    @param   buf         The buffer to save to.
    *    @param   desc        The tilemapformat description.
    *    @param   tileMap     The tile map.
    *    @param   prevFeature The previous feature in the map.
    *                         NULL if no previous feature exists.
    *    @return  If the feature was loaded properly or not.
    */
   static bool createFromStream(
      TilePrimitiveFeature& target,
      BitBuffer& buf,
      const TileMapFormatDesc& desc,
      TileMap& tileMap,
      const TilePrimitiveFeature* prevFeature );
 
   /**
    *    @return True if the feature really is a primitive 
    *            (since complex features also inherit from
    *             TilePrimitiveFeature).
    */
   inline bool isPrimitive() const;
  
   /**
    *    Set if the feature was drawn or not.
    */
   inline void setDrawn( bool drawn );
   
   /**
    *    Get if the feature was drawn or not.
    */
   inline bool isDrawn() const;
   
#ifdef __unix__   
   /**
    *    @return The approximate size in memory of the object.
    */
   uint32 getMemSize() const {
      
      uint32 tileFeatureObjectSize = sizeof(TilePrimitiveFeature);
      return tileFeatureObjectSize;
   }
#endif 
 
protected:   
   /// Sets the type of the feature.
   inline void setType( int32 type ) { m_type = type; }
   
   /**
    *    Internal load method. 
    *    
    *    @param   buf         The buffer to save to.
    *    @param   tileMap     The tile map.
    *    @param   prevFeature The previous feature in the map.
    *                         NULL if no previous feature of the same
    *                         type as this one.
    */
   bool internalLoad( BitBuffer& buf, TileMap& tileMap,
                      const TilePrimitiveFeature* prevFeature );
      
   /**
    *   Be careful!! Does not copy the arg!
    */
   inline void addArg(TileFeatureArg* arg);
   
   /// The arguments of the feature.
   std::vector<TileFeatureArg*> m_args;
   
   /// The type of the fature.
   int16 m_type;   

   /// The feature from which this feature is generated (for prims in TMH).
   int16 m_parentFeature : 15;

   /// If the feature was drawn before.
   bool m_drawn : 1;
   
   /// FIXME: Better way to do this, please.
   friend class TileMapHandler;
   friend class TileMap;   
};

class TileFeature : public TilePrimitiveFeature {

   friend class DummyTileMapFormatDesc;
   
public:
   /// Creates TileMaps in the server
   friend class TileMapCreator;
   /// Sets screen coordinates when loading.
   friend class TileMap;

   
   /**
    *   Deletes all the arguments of the feature.
    *   Note that this destructor isn't virtual, since nothing
    *   inherits from TileFeature.
    */
   ~TileFeature();

   TileFeature(const TileFeature& other);

   /// Half-working copy-contructoir. Can be removed when all stuff is in vect
   const TileFeature& operator=(const TileFeature& other);
#ifdef MC2_SYSTEM   
   /**
    *    Save method. 
    *    
    *    @param   buf         The buffer to save to.
    *    @param   tileMap     The tile map.
    *    @param   prevFeature The previous feature in the map.
    *                         NULL if no previous feature exists.
    *    @return  If the saving went well.
    */
   bool save( BitBuffer& buf, const TileMap& tileMap,
              const TileFeature* prevFeature ) const;
   
   void dump( ostream& stream ) const; 
#endif   
   
#ifdef __unix__   
   /**
    *    @return The approximate size in memory of the object.
    */
   uint32 getMemSize() const {
      
      uint32 tileFeatureObjectSize = sizeof(TilePrimitiveFeature);
      uint32 argSize = 0;
      for ( uint32 i = 0; i < m_args.size(); ++i ) {
         argSize += m_args[ i ]->getMemSize();
      }
      return tileFeatureObjectSize + argSize;
   }
#endif 

        
protected:
   
   friend class std::vector<TileFeature>;
   
   /**
    *   Creates the screen coords and the pixbox.
    */
   TileFeature( int32 type = MAX_INT32 );


};

class WritableTileFeature : public TileFeature {
public:

   WritableTileFeature( int32 type ) : TileFeature( type ) {}
   
   inline bool setArg( TileArgNames::tileArgName_t name, uint32 value );
         
   inline void setArgs( const std::vector<TileFeatureArg*>& args );


      
protected:
};
 
// Implementation of inlined methods

inline TilePrimitiveFeature::const_arg_iterator 
TilePrimitiveFeature::arg_begin() const
{
   return m_args.begin();
}

inline TilePrimitiveFeature::const_arg_iterator 
TilePrimitiveFeature::arg_end() const
{
   return m_args.end();
}

inline TilePrimitiveFeature::arg_iterator 
TilePrimitiveFeature::arg_begin()
{
   return m_args.begin();
}

inline TilePrimitiveFeature::arg_iterator 
TilePrimitiveFeature::arg_end()
{
   return m_args.end();
}

inline uint32
TilePrimitiveFeature::getNbrArgs() const
{
   return m_args.size();
}

inline int32
TilePrimitiveFeature::getType() const { 
   return m_type; 
}

inline int32
TilePrimitiveFeature::getDrawType() const {
   // A little bit of hack to improve performance.
   // We know that the first feature type to have this
   // real_feature_type was special_custom_poi_2, so 
   // if the feature type is below that value, then we don't make
   // the real check.
   if ( m_type < TileFeature::special_custom_poi_2 ) {
      return m_type;
   }

   // Make the slow check.
   const SimpleArg* realType = TileFeatureArg::simpleArgCast( getArg( TileArgNames::real_feature_type ) );
   if ( realType != NULL ) {
      return realType->getValue();
   } else {
      return m_type; 
   }
}

inline void 
TilePrimitiveFeature::addArg(TileFeatureArg* arg) 
{ 
   m_args.push_back(arg); 
}

inline void
TilePrimitiveFeature::setFeatureNbr(int i)
{
   m_parentFeature = i;
}
   
inline int
TilePrimitiveFeature::getFeatureNbr() const
{
   return m_parentFeature;
}


inline TileFeatureArg* 
TilePrimitiveFeature::getArg( TileArgNames::tileArgName_t name )
{
   for ( arg_iterator it = arg_begin();
         it != arg_end();
         ++it ) {
      if ( (*it)->getName() == name ) {
         return *it;
      }
   }
   return NULL;
}

inline const TileFeatureArg*
TilePrimitiveFeature::getArg( TileArgNames::tileArgName_t name ) const
{
   return const_cast<TilePrimitiveFeature*>(this)->getArg(name);
}
   
inline bool 
TilePrimitiveFeature::isPrimitive() const
{
   return m_type < 0;
}

inline void 
TilePrimitiveFeature::setDrawn( bool drawn )
{
   m_drawn = drawn;
}
   
inline bool 
TilePrimitiveFeature::isDrawn() const
{
   return m_drawn;
}

inline bool 
WritableTileFeature::setArg( TileArgNames::tileArgName_t name, uint32 value ) 
{
   SimpleArg* arg = static_cast<SimpleArg*> ( getArg( name ) );
   if ( arg != NULL ) {
      arg->setValue( value ); 
      return true;
   }
   return false;
}
         
inline void 
WritableTileFeature::setArgs( const std::vector<TileFeatureArg*>& args ) 
{
   // Set the params.
   m_args = args;
}

#endif // TILE_FEATURE_H

