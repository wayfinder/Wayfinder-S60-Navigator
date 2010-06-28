/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "WinMapControl.h"

/* MCShared includes */
#include "MC2Point.h"
#include "SharedHttpDBufRequester.h"
#include "MemoryDBufRequester.h"
#include "WinTCPConnectionHandler.h"
#include "HttpClientConnection.h"
#include "WinTileMapToolkit.h"
#include "WinMapPlotter.h"
#include "WinBitmap.h"
#include "TileMapHandler.h"
#include "MC2Coordinate.h"
#include "TileMapTextSettings.h"
#include "PixelBox.h"

#include <mbstring.h>
#include <tchar.h>

/* the global logging file */
#include <fstream>
extern ofstream gLogFile;

/* the name of the window class */
static const char* WINCLASSNAME = "MAPWINCLASS";

/* the title of the window */
static const char* WINTITLE = "WinMapClient";

/* private constructor */
WinMapControl::WinMapControl(HWND parent)
: m_hwnd(parent),
  m_plotter(NULL),
  m_toolkit(NULL),
  m_httpConn(NULL),
  m_httpReq(NULL),
  m_memReq(NULL),
  m_connHandler(NULL),
  m_mapHandler(NULL)
{
}

/* second-phase constructor */
bool WinMapControl::construct(LPCREATESTRUCTW cParams, 
                              const char* mapServer, int portNum,
                              const char* sessionId,
                              const char* sessionKey)
{
   /* get the dimensions of the window */
   m_width = cParams->cx;
   m_height = cParams->cy;

   /* create the toolkit */
   m_toolkit = new WinTileMapToolkit(m_hwnd, cParams->hInstance);
   if(m_toolkit == NULL) return(false);

   /* create the plotter */
   m_plotter = isab::WinMapPlotter::allocate(m_width, m_height, 
                                             m_hwnd);
   if(m_plotter == NULL) return(false);
   /* set the blitting position for the plotter */
   m_plotter->setBlitPoint(cParams->x, cParams->y);

   /* create the TCP Connection handler */
   m_connHandler = WinTCPConnectionHandler::allocate(cParams->hInstance);
   if(m_connHandler == NULL) return(false);

   /* create the HTTP client connection */
   m_httpConn = new HttpClientConnection(mapServer, 
                                         portNum,
                                         NULL,
                                         m_connHandler);
   if(m_httpConn == NULL) return(false);

   /* check if session params needed */
   if( (sessionId == NULL) || (sessionKey == NULL) ||
       (sessionId == "") || (sessionKey == "") ) {
      /* create the Internet Requester without session info */
      m_httpReq = new SharedHttpDBufRequester(m_httpConn, "/TMap");
   }
   else {
      /* create the session params */
      char sessionParam[256];
      _mbscpy((unsigned char*)sessionParam, (unsigned char*)"?");
      _mbscat((unsigned char*)sessionParam, (unsigned char*)"si=");
      _mbscat((unsigned char*)sessionParam, (unsigned char*)sessionId);
      _mbscat((unsigned char*)sessionParam, (unsigned char*)"&sk=");
      _mbscat((unsigned char*)sessionParam, (unsigned char*)sessionKey);
      /* create the Internet Requester */
      m_httpReq = new SharedHttpDBufRequester(m_httpConn, "/TMap",
                                              sessionParam);
   }
   if(m_httpReq == NULL) return(false);

   /* create the caching requester */
   m_memReq = new MemoryDBufRequester(m_httpReq, 8*1024*1024);
   if(m_memReq == NULL) {
      /* delete the requester, since it wont get deleted thru 
         the TileMapHandler's destructor */
      delete m_httpReq;
      return(false);
   }

   /* create the map handler */
   m_mapHandler = new TileMapHandler(m_plotter,
                                     m_memReq,
                                     m_toolkit);
   if(m_mapHandler == NULL) return(false);

   /* initiate the first redraw */
   m_mapHandler->repaint(0);

   /* success */
   return(true);
}

/* allocator */
WinMapControl* WinMapControl::allocate(HWND parent, LPCREATESTRUCTW cParams,
                                       const char* mapServer, int portNum,
                                       const char* sessionId,
                                       const char* sessionKey)
{
   /* create a new object */
   WinMapControl* newObj = new WinMapControl(parent);
   if(newObj == NULL) return(NULL);

   /* do second-phase */
   if(!newObj->construct(cParams, mapServer, portNum, sessionId, sessionKey)) {
      /* error while second phase construction */
      delete newObj;
      return(NULL);
   }

   /* success, return the allocated object */
   return(newObj);
}

/* destructor */
WinMapControl::~WinMapControl()
{
   delete m_mapHandler;
   delete m_httpConn;
   delete m_connHandler;
   delete m_toolkit;
   delete m_plotter;
   gLogFile << "WINCONTROL : Deleted!" << endl;
}

/* gets the handle to the internal window */
HWND WinMapControl::getWindow() const
{
   return(m_hwnd);
}

/* displays the map drawing buffer on the specified DC */
void WinMapControl::blitTo(HDC destDC, int32 dX, int32 dY)
{
   m_plotter->blitTo(destDC, dX, dY);
   return;
}

/* moves the map */
void WinMapControl::moveMap(int32 dx, int32 dy)
{
   /* tell the map Handler to move the map */
   m_mapHandler->move(dx, dy);
   /* request a repaint */
   doRepaint();
   return;
}

/* rotates the map */
void WinMapControl::rotateMap(int32 angle)
{
   /* tell the map Handler to rotate the map to the left */
   m_mapHandler->rotateLeftDeg(angle);
   /* request a repaint */
   doRepaint();
   return;
}

/* zooms the map */
void WinMapControl::zoomMap(float32 factor)
{
   /* tell the map Handler to zoom the map */
   m_mapHandler->zoom(factor);
   /* request a repaint */
   doRepaint();
   return;
}

/* resets the map rotation to zero degrees */
void WinMapControl::resetRotation()
{
   /* set the rotation to zero */
   m_mapHandler->setAngle(0);
   /* request a repaint */
   doRepaint();
   return;
}

/* request a repaint from the handler */
void WinMapControl::doRepaint()
{
   /* request a repaint */
   m_mapHandler->repaint(20);
   return;
}

/* resizes the control to the specified dimensions
   returns true on success, false on error */
bool WinMapControl::resize(int32 nW, int32 nH)
{
   if(m_plotter->resize(nW, nH)) {
      /* success, do a repaint */
      doRepaint();
      return( true );
   }
   /* resize didn't work */
   return( false );
}

/* gets the name of the feature at the specified location */
const char* WinMapControl::getFeatureName(const MC2Point& pos)
{
   return( m_mapHandler->getNameForFeatureAt(pos) );
}

/* gets the lat\lon location of the feature at the specified point */
void WinMapControl::getFeatureLocation(const MC2Point& pos, 
                                       MC2Coordinate& outLoc)
{
   /* get the feature location from the TileMapHandler */
   m_mapHandler->inverseTranformUsingCosLat(outLoc, pos);
   return;
}

/* gets the screen position of the feature at the specified coordinate */
void WinMapControl::getFeaturePosition(const MC2Coordinate& loc, 
                                       MC2Point& outPos)
{
   /* get the feature position from the TileMapHandler */
   m_mapHandler->transformPointUsingCosLat(outPos.getX(), outPos.getY(), loc);
   return;
}

/* gets all info for the feature at the specified point */
void WinMapControl::getFeatureInfo(const MC2Point& pos,
                                   TileMapHandlerClickResult& result)
{
   m_mapHandler->getInfoForFeatureAt( result, pos );
   return;
}

void
WinMapControl::setTextSettings( const TileMapTextSettings& settings )
{
   m_mapHandler->setTextSettings( settings );
}

void 
WinMapControl::setHighlight( bool highlight )
{
   // The crosshair looks the same, highlighted or not highlighted.
}

void
WinMapControl::setCursorPos( const MC2Point& pos )
{
   // The cursor is always in the center...
}

MC2Point 
WinMapControl::getCursorPos() const
{
   return MC2Point( m_width >> 1,
                    m_height >> 1 );
}

void 
WinMapControl::setCursorVisible( bool visible )
{
   // No cursor.
}

void 
WinMapControl::getCursorBox( PixelBox& box ) const
{
   box = PixelBox( MC2Point( 0, 0), 
                   MC2Point( m_width, m_height ) );

}


