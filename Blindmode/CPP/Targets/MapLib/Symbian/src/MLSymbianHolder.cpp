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

#include "MLSymbianHolder.h"
#include "MLMapInterface.h"
#include "MLConfigurator.h"
#include "MapDrawingInterface.h"
#include "MapMovingInterface.h"
#include "MapLib.h"
#include "SymbianMapLib.h"
#include "SymbianMapControl.h"
#include "Cursor.h"
#include "MC2Coordinate.h"
#include "MC2Point.h"

// To be removed
#include "SymbianTCPConnectionHandler.h"
#include "HttpClientConnection.h"
#include "SharedHttpDBufRequester.h"


namespace {

   /**
    *   Better name, please
    */
   class MLMapLibConfigurator : public MLConfigurator {
   public:
      MLMapLibConfigurator( MapLib* mapLib ) {
         m_mapLib = mapLib;
      }

      const char* setLanguageAsISO639( const char* isoStr ) {
         return m_mapLib->setLanguageAsISO639( isoStr );
      }

      void setDPICorrectionFactor( uint32 factor ) {
         m_mapLib->setDPICorrectionFactor( factor );
      }

      int addDiskCache( const char* path,
                        uint32 maxSize ) {
         return m_mapLib->addDiskCache( path, maxSize );
      }

      int setDiskCacheSize( uint32 nbrBytes ) {
         return m_mapLib->setDiskCacheSize( nbrBytes );
      }

      int clearDiskCache() {
         return m_mapLib->clearDiskCache();
      }
      
   private:
      MapLib* m_mapLib;
   };
   
   /**
    *   This class should be platform independent
    *   but a better name would be nice.
    */
   class MapLibMLMapInterface : public MLMapInterface {
   public:
      MapLibMLMapInterface( MapLib* mapLib,
                            Cursor* cursor ) {
         m_mapLib = mapLib;
         m_cursor = cursor;
         m_lastBitMapID = 0;
      }

      MapMovingInterface* mm() const {
         return m_mapLib->getMapMovingInterface();
      }

      MapDrawingInterface* md() {
         return m_mapLib->getMapDrawingInterface();
      }

      void rr() {
         md()->requestRepaint();
      }

      void setCenter( const MC2Coordinate& center ) {
         mm()->setCenter( center );
         rr();
      }

      void setScale( double scale ) {
         mm()->setScale( scale );
         rr();
      }

      MC2Coordinate getCoord() const {
         MC2Coordinate retval;
         mm()->inverseTransform( retval,
                                 m_cursor->getCursorPos() );
         return retval;
      }

      const char* getServerString() const {
         const ClickInfo& cl =
            mm()->getInfoForFeatureAt( m_cursor->getCursorPos(),
                                       true );
         return cl.getServerString();
      }

      void setGPSPosition( const MC2Coordinate& gpsPos ) {
         setCenter( gpsPos );
      }

      int addUserDefBitmap( const char* bitmap,
                            const MC2Coordinate& coord,
                            const char* info = "" ) {
         return 0;
      }

      int removeUserDefBitmap( int id ) {
         return -1;
      }

      void clearUserDefBitmaps() {
      }

   private:
      MapLib* m_mapLib;
      Cursor* m_cursor;
      uint32 m_lastBitMapID;
   };
   
   class MLSymbianHolderStorage {
   private:
      friend class MLSymbianHolder;

      MLSymbianHolderStorage( CCoeControl& parent,
                              DBufConnection* mapConn ) {
         if ( mapConn == NULL ) {
            mapConn = new SharedHttpDBufRequesterDBufConnection( 
               new HttpClientConnection(
                  "oss-xml.services.wayfinder.com", 80,
                  NULL,  new SymbianTCPConnectionHandler()), "/TMap", "" );
            // FIXME: Remove this when implemented by Indians
         }
         m_control = CSymbianMapControl::NewL( parent, mapConn );
         m_mapInterface = new MapLibMLMapInterface( m_control->getMapLib(),
                                                    m_control );
         m_configurator = new MLMapLibConfigurator( m_control->getMapLib() );
      }

      ~MLSymbianHolderStorage() {
         delete m_mapInterface;
         delete m_configurator;
         delete m_control;
      }
      
      CSymbianMapControl* m_control;
      MLConfigurator* m_configurator;
      MapLibMLMapInterface* m_mapInterface;
      
   };
   
}

MLSymbianHolder*
MLSymbianHolder::NewL( CCoeControl& parent,
                       DBufConnection* mapConn )
{
   return new MLSymbianHolder( parent, mapConn );
}

MLSymbianHolder::MLSymbianHolder( CCoeControl& parent,
                                  DBufConnection* mapConn )
{
   m_storage = new MLSymbianHolderStorage( parent, mapConn );
}

MLSymbianHolder::~MLSymbianHolder()
{
   delete m_storage;
}

CCoeControl*
MLSymbianHolder::getControl() const
{
   return m_storage->m_control;
}

MLConfigurator*
MLSymbianHolder::getConfigurator() const
{
   return m_storage->m_configurator;
}

MLMapInterface*
MLSymbianHolder::getMapInterface() const
{
   return m_storage->m_mapInterface;
}
