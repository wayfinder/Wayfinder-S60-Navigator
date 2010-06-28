/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPAREA_H
#define MAPAREA_H

#include "config.h"

#include <map>

#include <gtk/gtk.h>

#include "TileMapToolkit.h"
#include "TileMapEventListener.h"
#include "GtkTileMapToolkit.h"

#include <vector>

class DBufRequester;
class TileMapHandler;
class MultiFileDBufRequester;
class MC2SimpleString;
class MapLib;

/**
 *    Objects of this class describes the DrawingArea where the map is drawn.
 *    The objects will also handle mouse-click etc. in the map.
 *
 */
class MapDrawingArea : public GtkTileMapToolkit,
                       public TileMapEventListener {
public:

   /**
    *    Create a new MapDrawingArea.
    */
   MapDrawingArea(DBufRequester* mapRequester);

   /**
    *    Destructor.
    */
   virtual ~MapDrawingArea();


   void handleTileMapEvent( const TileMapEvent& event );

   /**
    *    Turns out that most of the events are sent to the
    *    MapDisplayWindow which will need this then.
    */
   TileMapHandler* getTileMapHandler();

   /**
    *
    */
   //!! TODO !!
   /*
   void setLabelToPutClickedNameIn( Gtk::Entry* clickLabel ) {
      m_clickedName = clickLabel;
   }
   */
   /**
    *   Sets cache size if possible.
    */
   void setMaxCacheSize( uint32 newSize );
   GtkWidget *widget() { return m_widget; }
protected:
   /**
    *    Get all dirs directly below basepath containing the 
    *    specified prefix. For instance wf-wmap-.
    */
   static void getDirs( const char* basepath,
                        const char* dirprefix,
                        vector<MC2SimpleString>& foundDirs );

   /**
    *    The implementation of the realize-method. This is called when the
    *    window is created.
    */
   void realize(GtkWidget *widget);

   /**
    *    This method draws the map in the window. Called when the window
    *    needs to be redrawn.
    *    @param  e The event with expose information (not used).
    */
   virtual void expose(GdkEventExpose* e);

   /**
    *    Called when a timer expires.
    */
   //   gint on_timeout(int timer_number);

   /**
    *    Called when a timer expires.
    */
   //   gint on_idle(int idle_number);
      
   /**
    *    This method handels mouse-clicks.
    *    @param  e   The event with information about the mouse-click 
    *                (screen coordinates, time etc.).
    *    @return ???
    */
   gint button_press_event_impl (GdkEventButton* e);

private:
   /**
    * handles GtkDrawingArea expose event
    */
   static void expose_event( GtkWidget* widget, GdkEventExpose* event, 
                             MapDrawingArea* data );
   /**
    * handles realization of the GtkDrawingArea widget
    */
   static void realize_event( GtkWidget* widget, MapDrawingArea* area );
   static void button_press_event( GtkWidget* widget, GdkEventButton* event, 
                                   MapDrawingArea* area );

   /**
    * Widget of the drawing area
    */
   GtkWidget *m_widget;

   /**
    *    The TileMapHandler.
    */
   TileMapHandler * m_tileMapHandler;

   /**
    *    The MapLib
    */
   MapLib* m_mapLib;
      
   /**
    *    Only used by the TileMapHandler.
    */
   DBufRequester* m_mapRequester;

   MultiFileDBufRequester* m_multiFileReq;

   /*
   typedef std::map<guint, CallbackInfo> DrawingMap_t; 
   static DrawingMap_t m_drawingAreas;
   */
};


#endif

