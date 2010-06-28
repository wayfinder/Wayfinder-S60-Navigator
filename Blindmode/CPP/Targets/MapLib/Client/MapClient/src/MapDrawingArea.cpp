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

#include "MapDrawingArea.h"
#include "GtkMapPlotter.h"
#include "TileMapHandler.h"
#include "MemoryDBufRequester.h"
#include "FileDBufRequester.h"

#include "GtkMFDBufRequester.h"

#include "MC2Point.h"

#include "WFDRMUtil.h"

#include <iostream>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>

#include "TileMapEvent.h"

#include "MC2String.h"
#include "MemTracker.h"
#include "GtkMapLib.h"


MapDrawingArea::MapDrawingArea(DBufRequester* mapRequester):
   m_widget( gtk_drawing_area_new() ),
   m_tileMapHandler(NULL),
   m_mapRequester(mapRequester)
{

   gtk_widget_add_events( m_widget, 
                          GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

   // request size of the window
   gtk_widget_set_size_request(m_widget, 200, 200);

   // setup callbacks
   g_signal_connect_after( G_OBJECT( m_widget ), "realize",
                           G_CALLBACK( MapDrawingArea::realize_event ), 
                           this );
   g_signal_connect( G_OBJECT( m_widget ), "expose_event",
                     G_CALLBACK( MapDrawingArea::expose_event ), 
                     this );

}

MapDrawingArea::~MapDrawingArea()
{
   delete m_mapLib;
}

void
MapDrawingArea::handleTileMapEvent( const TileMapEvent& event )
{
   mc2log << "[MDA]: TileMapEvent of type "
          << int(event.getType() ) << endl;
}

void
MapDrawingArea::getDirs( const char* basepath,
                         const char* dirprefix,
                         vector<MC2SimpleString>& foundDirs )
{

   MC2String findCmd = MC2String("find ") + 
      MC2String(basepath) + MC2String(" -maxdepth 1 -name \"") + 
      MC2String(dirprefix) + MC2String("*\"");

   FILE* findFile = popen( findCmd.c_str(), "r" );
   char tmpStr[256];
   if ( findFile ) {
      int retVal = 0;

      do {
         retVal = fscanf( findFile, "%s\n", tmpStr );
         if ( retVal >= 0 ) {
            foundDirs.push_back( MC2SimpleString( tmpStr ) );
         }
      } while ( retVal >= 0 );
      
      pclose( findFile );
   }
}

static void findFiles( const char* basepath,
                       const char* suffix,
                       vector<MC2SimpleString>& foundNames )
{

   MC2String findCmd = MC2String("find ") + 
      MC2String(basepath) + MC2String(" -maxdepth 1 -iname \"*") +
      suffix + "\"";

   FILE* findFile = popen( findCmd.c_str(), "r" );
   char tmpStr[256];
   if ( findFile ) {
      int retVal = 0;

      do {
         retVal = fscanf( findFile, "%s\n", tmpStr );
         if ( retVal >= 0 ) {
            foundNames.push_back( MC2SimpleString( tmpStr ) );
         }
      } while ( retVal >= 0 );
      
      pclose( findFile );
   }
}

void 
MapDrawingArea::realize(GtkWidget *widget) 
{

   // FIXME: This code could be somewhere else
   const char* user = getenv("LOGNAME");
   if ( user == NULL ) {
      user = getenv("USER");
      if ( user == NULL ) {
         user = getenv("USERNAME");
      }
   }
  
   vector<MC2SimpleString> precacheDirs;
   
   char* tmpPath = NULL; 
   
   if ( user ) {
      tmpPath = new char[1024];
      snprintf(tmpPath, 1024, "/tmp/%s", user);
      // Get the precached dirs from i.e. /tmp/martins etc.
      getDirs( tmpPath, "wf-wmap-", precacheDirs );
      // Ugle
      mkdir(tmpPath, S_IRWXU | S_IRUSR | S_IWUSR);
      sprintf(tmpPath, "/tmp/%s/tilemaps", user);
      mkdir(tmpPath, S_IRWXU | S_IRUSR | S_IWUSR);
   } else {
      mc2dbg << error << "[MDA]: No username found - no cache." << endl;
   }


   m_tileMapHandler = new TileMapHandler( new GtkMapPlotter( widget->window ),
                                          m_mapRequester,
                                          this,
                                          this );

   // Create the MapLib which i will be the new Interface to MapLib
   m_mapLib = new GtkMapLib( m_tileMapHandler );

   // Set size of memory cache
   m_mapLib->setMemoryCacheSize( 1024 );
   
   // Add the read/write disk cache
   int disk_ret = m_mapLib->addDiskCache( tmpPath, 1024 * 1024 );
   mc2dbg << "[MapDrawingArea]: m_mapLib->addDiskCache returned "
          << disk_ret << endl;

   // Add the multifile precached.
   for ( uint32 i = 0; i < precacheDirs.size(); ++i ) {
      mc2log << "Adding precached dir " <<  precacheDirs[ i ] << endl;
      m_mapLib->addMultiFileCache( precacheDirs[i].c_str(),
                                   NULL ); // Warez!
   }

   m_mapLib->setDiskCacheSize( 10*1024*1024 );

   // Find the sfds on temp
   if ( user ) {
      MC2String path = MC2String("/tmp/") + user + "/";
      vector<MC2SimpleString> filenames;
      findFiles( path.c_str(), ".wfd", filenames );

      for ( vector<MC2SimpleString>::const_iterator it = filenames.begin();
            it != filenames.end();
            ++it ) {
         
         // Add SingleFileDBufRequesters through MapLib
         m_mapLib->addSingleFileCache( (*it).c_str(),
                                       "" // UID
                                       );
      }
   }
         

   MapLib::const_cacheinfo_p_p info;
   int nbr = m_mapLib->getCacheInfo( info );
   mc2dbg << "CacheInfo" << endl;
   mc2dbg << "----------------------------------------------------" << endl;
   for ( int i = 0; i < nbr; ++i ) {
      mc2dbg << i << ":" << info[i]->getPathUTF8() << ":"
             << info[i]->getNameUTF8() << endl;
   }   
   mc2dbg << "----------------------------------------------------" << endl;

#if 0
   // Testing to disconnect the cache.
   m_mapLib->disconnectCache( info[0] );

   nbr = m_mapLib->getCacheInfo( info );
   mc2dbg << "CacheInfo" << endl;
   mc2dbg << "----------------------------------------------------" << endl;
   for ( int i = 0; i < nbr; ++i ) {
      mc2dbg << i << ":" << info[i]->getPathUTF8() << ":"
             << info[i]->getNameUTF8() << endl;
   }   
   mc2dbg << "----------------------------------------------------" << endl;
#endif
      
   m_tileMapHandler->showCopyright( true );
   m_tileMapHandler->setCopyrightPos( MC2Point( 0, 10 ) );
   
   delete [] tmpPath;

   m_tileMapHandler->repaint(0);
}

void
MapDrawingArea::setMaxCacheSize( uint32 newSize )
{
   if ( m_multiFileReq ) {
      mc2log << "[MDA]: Setting new cache size = " << newSize << endl;
      m_multiFileReq->setMaxSize( newSize );
   }
}

void
MapDrawingArea::expose(GdkEventExpose*)
{
   mc2dbg << "[MDA]: expose_event_impl " << endl;
   m_tileMapHandler->repaint(0);
}


TileMapHandler*
MapDrawingArea::getTileMapHandler()
{
   return m_tileMapHandler;
}

void MapDrawingArea::realize_event( GtkWidget* widget, 
                                    MapDrawingArea* area ) {
   assert( area );
   // redirect to map drawing area 
   area->realize(widget);
}

void MapDrawingArea::expose_event( GtkWidget* widget, 
                                   GdkEventExpose* event, MapDrawingArea* area ) {
   assert( area );
   // redirect to map drawing area   
   area->expose(event);
}

