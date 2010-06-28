/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WINMAPCONTROL_H
#define WINMAPCONTROL_H

/* forward declarations */
class WinTCPConnectionHandler;
class HttpClientConnection;
class WinTileMapToolkit;
class WinBitmap;
class MC2Point;
class TileMapHandler;
class TileMapHandlerClickResult;
class SharedHttpDBufRequester;
class MemoryDBufRequester;
class MC2Coordinate;
class TileMapTextSettings;
class PixelBox;

#include "config.h"
#include "WinMapPlotter.h"
#include <windows.h>
#include "Cursor.h"


/* Handles the connection to the TMap Server 
   and the drawing of the Map */
class WinMapControl : public Cursor
{
   /** data **/
   private:
      /* the main window */
      HWND m_hwnd;

      /* the width and height of the control */
      int32 m_width, m_height;

      /* the map plotter */
      isab::WinMapPlotter* m_plotter;

      /* the platform toolkit */
      WinTileMapToolkit* m_toolkit;

      /* the TCP Connection handler */
      WinTCPConnectionHandler* m_connHandler;

      /* the HTTP Connection handler */
      HttpClientConnection* m_httpConn;

      /* the HTTP requester */
      SharedHttpDBufRequester* m_httpReq;

      /* the memory caching requester */
      MemoryDBufRequester* m_memReq;

      /* the Tile Map Handler */
      TileMapHandler* m_mapHandler;

   /** methods **/
   private:
      /* private constructor */
      WinMapControl(HWND parent);

      /* second-phase constructor */
      bool construct(LPCREATESTRUCTW cParams,
                     const char* mapServer, int portNum,
                     const char* sessionId,
                     const char* sessionKey);

   public:
      /* allocator */
      static WinMapControl* allocate(HWND parent, LPCREATESTRUCTW cParams,
                                     const char* mapServer, int portNum,
                                     const char* sessionId,
                                     const char* sessionKey);

      /* destructor */
      ~WinMapControl();

      /* gets the handle to the internal window */
      HWND getWindow() const;

      /* displays the map-drawing buffer on the specified DC */
      void blitTo(HDC destDC, int32 dx, int32 dy);

      /* moves the map */
      void moveMap(int32 dx, int32 dy);

      /* rotates the map */
      void rotateMap(int32 angle);

      /* zooms the map */
      void zoomMap(float32 factor);

      /* resets the map rotation to zero degrees */
      void resetRotation();

      /* request a repaint from the handler */
      void doRepaint();

      /* resizes the control to the specified dimensions
         returns true on success, false on error */
      bool resize(int32 nW, int32 nH);

      /* gets the name of the feature at the specified location */
      const char* getFeatureName(const MC2Point& pos);

      /* gets the lat\lon location of the feature at the specified point */
      void getFeatureLocation(const MC2Point& pos, MC2Coordinate& outLoc);

      /* gets the screen position of the feature at the specified coordinate */
      void getFeaturePosition(const MC2Coordinate& loc, MC2Point& outPos);

      void setBlitPoint(int x, int y)
      {
         m_plotter->setBlitPoint(x, y);
         return;
      }

      /* gets the width of the control */
      int32 width() const
      {
         isab::Rectangle rect;
         m_plotter->getMapSizePixels(rect);
         return( rect.getWidth() );
      }

      /* gets the height of the control */
      int32 height() const
      {
         isab::Rectangle rect;
         m_plotter->getMapSizePixels(rect);
         return( rect.getHeight() );
      }

      /* gets a reference to the internal TileMapHandler */
      TileMapHandler& getHandler() const
      {
         return(*m_mapHandler);
      }

      /* gets a reference to the internal MapPlotter */
      isab::WinMapPlotter& getPlotter() const
      {
         return(*m_plotter);
      }

      /* gets all info for the feature at the specified point */
      void getFeatureInfo(const MC2Point& pos,
                          TileMapHandlerClickResult& result);

      /**
       *   Sets the text settings ( fonts ) for text placement
       *   in maps.
       */
      void setTextSettings( const TileMapTextSettings& settings );

      /**
       *    Get the toolkit.
       */
      WinTileMapToolkit* getToolkit() {
         return m_toolkit;
      }

      /**
       *    Implements abstract method in Cursor.
       *    Currently doesn't do anything.
       */
      void setHighlight( bool highlight );

      /**
       *    Implements abstract method in Cursor.
       *    Currently doesn't do anything.
       */
      void setCursorPos( const MC2Point& pos );
      
      /**
       *    Implements abstract method in Cursor.
       *    Get the current cursor position.
       *    @return  The cursor position.
       */
      MC2Point getCursorPos() const;
         
      /**
       *    Implements abstract method in Cursor.
       *    Set if the cursor should be visible or hidden.
       *    @param   visible  If true, the cursor is visible, 
       *                      otherwise hidden.
       */
      void setCursorVisible( bool visible );

      /**
       *    Implements abstract method in Cursor.
       *    Get the box of the cursor.
       *    @param   box   PixelBox that will be set to the current
       *                   box of the cursor.
       */
      void getCursorBox( PixelBox& box ) const;


};

#endif
