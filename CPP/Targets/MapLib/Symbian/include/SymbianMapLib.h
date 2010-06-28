/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYMBIANMAPLIB_KROKOFANT_H
#define SYMBIANMAPLIB_KROKOFANT_H

#include "MapLib.h"

class CCoeControl;
class FileHandler;
namespace isab {
   class MapRenderer;
}
class RFs;
class TileMapToolkit;

/**
 *    Symbian specific MapLib.
 */
class SymbianMapLib : public MapLib {
public:
   /**
    *   Constructor
    *   @param mapConn DBufConnection to get the tiles from
    *   @param control Control to draw the map in.
    *   @param fs      File server session.
    */
   SymbianMapLib( DBufConnection* mapConn,
                  CCoeControl& control,
                  RFs& fs );

   /**
    *   Returns the mapplotter.
    */
   isab::MapRenderer* getSymbianMapPlotter();

protected:

   /// Lazy constructor used by TileMapControl.
   SymbianMapLib( TileMapHandler* handler,
                  RFs& fileServer );

   /**
    *   Creates a SymbianFileHandler.
    *   @see MapLib.
    */
   FileHandler* createFileHandler( const char* filename,
                                   bool readOnly,
                                   bool createFile,
                                   bool initNow );

   /**
    *   Returns "\\"
    */
   const char* getPathSeparator() const;

   /// Function used in startup
   isab::MapRenderer* createPlotter( CCoeControl& parent );

   /// Function used in startup
   TileMapToolkit* createToolkit( CCoeControl& parent );
   
   /// The file server.
   RFs& m_fileServer;

   /// Toolkit
   TileMapToolkit* m_toolkit;

   /// MapRenderer
   isab::MapRenderer* m_mapPlotter;
};

#endif 
