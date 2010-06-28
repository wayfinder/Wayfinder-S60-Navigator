/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPCLIENTAPP_H
#define MAPCLIENTAPP_H

#include <gtk/gtk.h>
#include "MC2SimpleString.h"

class MapDrawingArea;
class DBufRequester;
class Road;
class MC2SimpleString;

class MapClientApp {
public:
   explicit MapClientApp( DBufRequester *req );
   ~MapClientApp();
   void loop();
   /**
    * called when a button is clicked on the map area
    * @param x position of button
    * @param y position of button
    */
   void mapClick( int x, int y, int button );

   GtkWidget *getTopLevelWidget() { return m_window; }
   MapDrawingArea *getDrawingArea() { return m_drawingArea; }
   /// @param inout true if zoom in else zoom out
   void zoom(bool in);
   /// set route id from route id input
   void setRouteID();
   GtkWidget *getVicinityArea();
   GtkWidget *getTable()   {  return m_table; }
   GtkWidget *getInfoLabel() { return m_infoLabel; }

   const MC2SimpleString& getRoadName();

   bool hasRoad();
   void setDetectAlongRoad(bool detectAlongRoad) { m_detectAlongRoad = detectAlongRoad; }
   void setHighlightAll(bool highlightAll) { m_highlightAll = highlightAll; }
   void setShowRoadNames(bool showRoadNames) { m_showRoadNames = showRoadNames; }
   void setShouldDisableLevelCheck(bool disableLevelCheck);

   bool shouldDetectAlongTheRoad() { return m_detectAlongRoad; }
   bool shouldHighlightAll() { return m_highlightAll; }
   bool shouldShowRoadNames() { return m_showRoadNames; }
private:
   void setupUserDefinedFeatures();
   void setupMainWindow( DBufRequester *req );

   bool m_detectAlongRoad;
   bool m_disableLevelCheck;
   bool m_highlightAll;
   bool m_showRoadNames;

   MapDrawingArea *m_drawingArea;
   GtkWidget *m_infoLabel;
   GtkWidget *m_table;
   GtkWidget *m_window;
   GtkWidget *m_vArea;
   MC2SimpleString m_roadName;
};

#endif
