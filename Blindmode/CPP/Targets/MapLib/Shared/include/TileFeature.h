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

#include "FeatureType.h"

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

