/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MAPLIB_H
#define MAPLIB_H

#include "config.h"

//    namespace {
   class MapLibInternal;
//    }

namespace isab {
   class MapPlotter;
}

class DBufConnection;
class DBufRequester;
class FileHandler;
class MC2Point;
class MapDrawingInterface;
class MapMovingInterface;
class RouteID;
class TileMapEventListener;
class TileMapHandler;
class TileMapToolkit;
class InterpolationHintConsumer;
class InterpolationCallback;

/**
 *   Interface class for MapLib that should have been created
 *   a long time ago.
 *   <b>All strings are in utf-8 and zero terminated.</b>
 */
class MapLib {
public:

   /// Return value for get cache info.
   class CacheInfo {
   public:
      /// Returns the path of the cache in question.
      virtual const char* getPathUTF8() const = 0;
      /// Returns the user displayable name of the cache
      virtual const char* getNameUTF8() const = 0;
      /// Returns true if the cache seems valid.
      virtual bool isValid() const = 0;
   };

   typedef const CacheInfo* const_cacheinfo_p;
   typedef const const_cacheinfo_p* const_cacheinfo_p_p;
   
   /**
    *    Returns the filenames of the currently active caches.
    *    Should not be deleted and is invalidated when adding/removing
    *    caches.
    *    @param infoArray Is set to a pointer to the internal array.
    *    @return The size of the array.
    */
   int getCacheInfo( const_cacheinfo_p_p& infos );

   /**
    *    Add a cache file to the cache stack.
    *    Will invalidate the CacheInfo.
    *    @param fileName Full path to the file.
    *    @param uin      UIN of the user.
    *    @return -1 if failed.
    */
   int addSingleFileCache( const char* fileName,
                           const char* uin );

   /**
    *    Sets the size of the memory cache.
    */
   void setMemoryCacheSize( uint32 nbrBytes );

   /**
    *    Clears the memory cache.
    */
   void clearMemoryCache();

   /**
    *    Creates the read/write cache with the supplied max size.
    *    @param path    The path to the cache in utf-8.
    *    @param maxSize The maximum size of the cache.
    */
   int addDiskCache( const char* path,
                     uint32 maxSize );

   /**
    *    Sets the max size of the disk cache if it exists.
    *    @param nbrBytes The maximum size of the R/W cache in bytes.
    */
   int setDiskCacheSize( uint32 nbrBytes );

   /**
    *    Clears the R/W disk cache.
    */
   int clearDiskCache();
   
   /**
    *    Adds a multifile read only cache with the specified path.
    *    @param path The path to the directory where the multi file
    *                cache is located.
    *    @param uin  The uin for decryption if downloaded cache. <br /> 
    *                NULL for warez.
    */
   int addMultiFileCache( const char* path,
                          const char* uin );

   /**
    *    Disconnects the cache with the corresponding info.
    *    Currently only works for single file caches.
    *    The CacheInfo can never be used again.
    */
   int disconnectCache( const CacheInfo* info );

   /**
    *    Adds an event listener to be called when new categories
    *    are available and if cache info needs update.
    */
   void addEventListener( TileMapEventListener* listener );

   /**
    *    Removes an event listener.
    */
   void removeEventListener( TileMapEventListener* listener );

   /**
    *    Sets the route id of MapLib.
    */
   void setRouteID( const RouteID& routeID );

   /**
    *    Gets the route id of MapLib.
    */
   const RouteID* getRouteID() const;

   /**
    *    Clears the routeID.
    */
   void clearRouteID();

   /**
    *   Set the language using the specified iso639 string.   
    *   The string can consist of two or three letters and must be
    *   NULL terminated. 
    *   @param isoStr The string, containing two or three letters.
    *   @return   The language as iso639 that was set to MapLib.
    */
   const char* setLanguageAsISO639( const char* isoStr );

   /**
    * Set the position of the copyright string of the map. 
    * @param   pos   The left baseline position of the copyright string.
    */
   void setCopyrightPos( const MC2Point& pos );
   
   /**
    * Set if to show copyright string.
    */
   void showCopyright( bool show );

   /**
    *    Returns the mapdrawinginteface
    *    Valid during the lifetime of the MapLib.
    */
   MapDrawingInterface* getMapDrawingInterface() const;
   
   /**
    *    Returns the map moving interface.
    *    Valid during the lifetime of the MapLib.
    */
   MapMovingInterface* getMapMovingInterface() const;

//// BEGIN:  TO BE MOVED TO SEPARATE INTERPOLATION INTERFACE
   /**
    *    Returns the interpolation hint conusmer.
    *    Can be used to provide hints to MapLib regarding
    *    where the position should be between real position
    *    updates.
    */ 

   InterpolationHintConsumer* getInterpolationHintConsumer();

   void setInterpolationCallback( InterpolationCallback* callback );

   void clearInterpolationCallback( InterpolationCallback* callback );

   void setInterpolationEnabled( bool enabled );

   void requestImmediatePositionInterpolation();
   
//// END:  TO BE MOVED TO SEPARATE INTERPOLATION INTERFACE
   
   /**
    *    Set the DPI correction factor. The factor 1 corresponds
    *    a value suitable for old phone models, such as Nokia 6600
    *    and similar. For new devices with displays with higher
    *    DPI, the factor should be correspond with how many times
    *    greater the DPI is compared to the reference display
    *    (e.g. Nokia 6600).
    */
   void setDPICorrectionFactor( uint32 factor );

   /**
    * Turns 3d mode on or off.
    */
   void set3dMode( bool on );

   /**
    * Turns outlines on in 3d mode.
    */
   void setOutlinesIn3dEnabled( bool enabled );

   /**
    * True if in 3d mode, false if not.
    */
   bool get3dMode( );

   /**
    * True if in outlines are enabled in 3d mode, false if not.
    */
   bool getOutlinesIn3dEnabled( ) const;


   /**
    * Sets the horizon height that is displayed
    * when map is displayed in 3d. 
    * @param height The height of the horizon.
    */
   void setHorizonHeight( uint32 height );

   /**
    *    Sets night mode on or off. 
    */
   void setNightMode( bool on );

   /**
    *  Get the ACP mode setting. True if ACP enabled, false if not.
    */
   bool isACPModeEnabled() const;

   /**
    * Set the ACP mode setting. True if ACP enabled, false if not.
    */
   void setACPModeEnabled( bool enable );
   
   /**
    *    Returns the currently used TileMapToolkit.
    */
   TileMapToolkit* getToolkit() const;
   
   /**
    *    Destructor.
    */
   virtual ~MapLib();

   /**
    *   Use. Implement.
    */
   virtual FileHandler* createFileHandler( const char* filename,
                                           bool readOnly,
                                           bool createFile,
                                           bool initNow ) = 0;

   /**
    *   Implement.
    *   Return the path separator, e.g. "/" for unix and "\\" for
    *   Symbian/Windows.
    */
   virtual const char* getPathSeparator() const = 0;

   /**
    *   @return If MapLib is initialized.
    */
   bool isInitialized() const;
protected:

   /**
    *   One of the constructors to use when porting MapLib
    *   to another platform. Ownership of the objets will
    *   be taken over by MapLib.
    */
   MapLib( isab::MapPlotter* plotter,
           TileMapToolkit* toolkit,
           DBufConnection* bufferConnection );
   
   /**
    *   One of the constructors to use when porting MapLib
    *   to another platform. Ownership of the objets will
    *   be taken over by MapLib.
    */
   MapLib( isab::MapPlotter* plotter,
           TileMapToolkit* toolkit,
           DBufRequester* bufferRequester );

   /**
    *   Don't use.
    */
   MapLib( TileMapHandler* handler );
   
private:
   MapLibInternal* m_storage;
   
};

#endif
