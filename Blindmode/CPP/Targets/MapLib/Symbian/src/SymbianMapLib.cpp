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

#include "SymbianMapLib.h"
#include "MapRenderer.h"
#include "SymbianTilemapToolkit.h"

#include "SymbianMFDBufRequester.h"

#include <coemain.h>
#include <gdi.h>
#include <coecntrl.h>

using namespace isab;

MapRenderer*
SymbianMapLib::createPlotter( CCoeControl& parent )
{   
   /* get the devices color deopth */
   int colorDepth = 12;
   
   TDisplayMode nativeMode = 
      CCoeEnv::Static()->ScreenDevice()->DisplayMode();
   switch ( nativeMode ) {
      case EColor16:
         colorDepth = 4;
         break;
      case EColor256:
         colorDepth = 8;
         break;
      case EColor4K:
         colorDepth = 12;
         break;
      case EColor64K:
         colorDepth = 16;
         break;
      default:
         colorDepth = 12;
         break;
   }

   m_mapPlotter = MapRenderer::NewL(parent, 
                                    parent.Rect().Width(),
                                    parent.Rect().Height(), 
                                    colorDepth);

   
   return m_mapPlotter;
}

TileMapToolkit*
SymbianMapLib::createToolkit( CCoeControl& parent )
{
   m_toolkit = new SymbianTileMapToolkit(parent);
   return m_toolkit;
}

MapRenderer*
SymbianMapLib::getSymbianMapPlotter()
{  
   return m_mapPlotter;
}

SymbianMapLib::SymbianMapLib( DBufConnection* mapConn,
                              CCoeControl& control,
                              RFs& fs ) :
      MapLib( createPlotter( control ),
              createToolkit( control ),
              mapConn ),
      m_fileServer( fs )
{
   // Toolkit and plotter will be leaked unless fixed.
}

SymbianMapLib::SymbianMapLib( TileMapHandler* handler,
                              RFs& fileServer ) : MapLib( handler ),
                                                  m_fileServer( fileServer )
{
   m_mapPlotter = NULL;
   // In this case the plotter and toolkit will be deleted by the caller
   // since we know that it is TileMapControl.
}

FileHandler*
SymbianMapLib::createFileHandler( const char* filename,
                                  bool readOnly,
                                  bool createFile,
                                  bool initNow )
{
   return new SymbianFileHandler( m_fileServer,
                                  filename,
                                  createFile,
                                  initNow,
                                  readOnly );
}

const char*
SymbianMapLib::getPathSeparator() const
{
   return "\\";
}
