/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapClientApp.h"

#include "MapDrawingArea.h"
#include "DBufRequester.h"
#include "MC2Point.h"
#include "TileMapHandler.h"
#include "TileMapParamTypes.h"
#include "RouteID.h"
#include "UserDefinedBitMapFeature.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

//!! TODO !!, check for gcc version and include the "right" header

#include <sstream>

void setInfoLabelDirect( GtkLabel *label, 
                                  ostringstream &labelStr )
{
        // if object had the label property -> set text
   if (label != NULL) {
      gtk_label_set_markup(label, labelStr.str().c_str() );
   }
        else
           // mc2dbg2 << "Label is null!" << endl;
        // clear string
        labelStr.str("");
}

/*
* Get GtkLabel "property" from topLevel widget and
* set label string
*/
void setInfoLabel( GtkWidget *topLevel, 
                                  const char *property,
                                  ostringstream &labelStr )
{
        GtkLabel *label = 
                static_cast<GtkLabel*>(
                g_object_get_data ( G_OBJECT( topLevel ),
                property ) );

   setInfoLabelDirect(label, labelStr);
}

void setStatusLabel( MapClientApp* app, ostringstream& os, int numSpaces = 0) {
   GtkLabel *statusLabel = 
      GTK_LABEL(app->getInfoLabel());

   for(; numSpaces<3; numSpaces++) 
      os << endl;

   setInfoLabelDirect(statusLabel, os);
}


void setInfoLabelDirect(  GtkLabel *label,
                                  MC2SimpleString str)
{
   ostringstream tempStream;
   tempStream << str;

   setInfoLabelDirect(label, tempStream);
}


void updateInfoLabels( GtkWidget *topLevel, TileMapHandler &handler);

//
// GTK+ callbacks
//
//
 
// end application
void destroy_event( GtkWidget *widget,
                    gpointer   data )
{
    gtk_main_quit();
}

gboolean delete_event( GtkWidget *widget,
                       GdkEvent  *event,
                       gpointer   data )
{
   /* If you return FALSE in the "delete_event" signal handler,
    * GTK will emit the "destroy" signal. Returning TRUE means
    * you don't want the window to be destroyed.
    * This is useful for popping up 'are you sure you want to quit?'
    * type dialogs. 
    */

    // Change TRUE to FALSE and the main window 
   // will be destroyed with a "delete_event". 

   return TRUE;
}
void zoomInCallback(GtkToolButton* tool,
                    MapClientApp* app) 
{
   app->zoom( true );
   app->getDrawingArea()->getTileMapHandler()->requestRepaint();
}

void zoomOutCallback(GtkToolButton* tool,
                     MapClientApp* app) 
{
   app->zoom( false );
   app->getDrawingArea()->getTileMapHandler()->requestRepaint();
}

void setRouteIDCallback( GtkWidget *w,
                         MapClientApp* app)
{
   app->setRouteID();
}

int appendStreetNames(ostringstream& ostream, MC2SimpleString m) {
   string s(m.c_str());

   int numLines = 0;
   int wrapIndex = 0;

   for(unsigned int i = 0; i<s.size(); i++, wrapIndex++) {
      if(i > 0 && s[i-1] == '/' && wrapIndex > 26) {
         ostream << endl << "\t";
         numLines++;
         wrapIndex = 0;
      }
      else
         ostream << s[i];
   }

   return numLines;
}

void outputVicinityList(   vicVec_t& vicItems, 
                           MapClientApp* app)  
{
   TileMapHandler* mapHandler = app->getDrawingArea()->getTileMapHandler();

   MC2Point inarg(0, 0);
   mapHandler->transform(inarg, mapHandler->getCenter());
   
   ostringstream labelStr;
   labelStr.precision(3);
   labelStr << showpoint;
   GtkWidget* topLevel = app->getVicinityArea();
   ostringstream combiner;

   if(app->shouldDetectAlongTheRoad() && app->hasRoad()) {
      labelStr << "Along <b>" << app->getRoadName() << "</b>:";
      labelStr << endl;
   }
   
   int numLines = 0;
   for(unsigned int i = 0; i<5; i++) {
      VicinityItem* vItem = NULL;

      if(i<vicItems.size())
         vItem = vicItems[i];
      
      if(!vItem || vItem->Name().empty()) {
         labelStr << endl << endl;   
         numLines+=2;
         continue;
      }

      labelStr << (i+1) << ".\t";
      labelStr << "<b>";
      numLines+=appendStreetNames(labelStr, vItem->Name());
      labelStr << "</b>";
      labelStr << endl << "\t";
      numLines++;

      labelStr << left;
      //labelStr.width(15);
      combiner << vItem->Distance() << " m, ";
      labelStr << combiner.str();
      combiner.str("");
      //labelStr << "lat:  " << vItem->Coord().lat << " ";
      //labelStr << "lon:  " << vItem->Coord().lon << " ";
      //labelStr << left;
      //labelStr.width(7);
      //combiner << int(vItem->AngleFromNorth() + 0.5f) << " degrees";
      labelStr << combiner.str();
      combiner.str("");
      labelStr << endl;
      numLines++;
   }
   
   for( ; numLines<20; numLines++) {
      labelStr << endl;
   }

   combiner << "Total number found: <b>" << vicItems.size() << "</b>.";
   setStatusLabel(app, combiner);
   //   setInfoLabelDirect( GTK_LABEL(app->getInfoLabel()), combiner);


   const unsigned int reduceFactor = 5;

   unsigned int reduce = vicItems.size();
   if(reduce>reduceFactor)
      reduce = reduceFactor;

   if(!app->shouldHighlightAll()) {
      vicItems.erase( vicItems.begin() + reduce, vicItems.end());
      vicVec_t(vicItems).swap(vicItems);
   }

   mapHandler->markDetectedPoints(vicItems);

   setInfoLabel( topLevel, "vcitem", labelStr );
}


void printCrossingsCallback( GtkWidget *w, MapClientApp* app)
{
   TileMapHandler& mapHandler = 
      *app->getDrawingArea()->getTileMapHandler();

   MC2Point inarg(0, 0);
   mapHandler.transform(inarg, mapHandler.getCenter());

   vicVec_t crossings;

   if(app->shouldDetectAlongTheRoad() && app->hasRoad()) {
      mapHandler.getCrossingsAlongRoad(inarg,
                                       app->getRoadName(),
                                       crossings,
                                       app->shouldHighlightAll());
   } else {
      mapHandler.getCrossingsInVicinity(inarg,
                                        crossings,
                                        app->shouldHighlightAll());
   }
   

   stable_sort( crossings.begin(),
                crossings.end(),
                VicinityUtil::distanceComp );
   
   mc2dbg << "CROSS_SIZE: " << crossings.size() << std::endl;
   
   outputVicinityList(crossings, app);
}

void printUDFsCallback( GtkWidget *w, MapClientApp* app)
{

   TileMapHandler& mapHandler = 
      *app->getDrawingArea()->getTileMapHandler();
        
   MC2Point inarg(0, 0);
   mapHandler.transform(inarg, mapHandler.getCenter());

   vicVec_t dump;
   mapHandler.getUDFsInVicinity(inarg, dump);

   sort(dump.begin(), dump.end(), VicinityUtil::distanceComp);
        
   outputVicinityList(dump, app);
}

void printVicinityCallback( GtkWidget *w, MapClientApp* app)
{

   TileMapHandler& mapHandler = *app->getDrawingArea()->getTileMapHandler();
        
   MC2Point inarg(0, 0);
   mapHandler.transform(inarg, mapHandler.getCenter());

   vicVec_t dump; 
   
   if(app->shouldDetectAlongTheRoad() && app->hasRoad()) {
      mapHandler.getPOIsAndUDFsAlongRoad(inarg, app->getRoadName(), dump);
   } else {
      mapHandler.getPOIsInVicinity(inarg, dump);
   }
   
   sort(dump.begin(), dump.end(), VicinityUtil::distanceComp);
        
   outputVicinityList(dump, app);
}

gboolean keyEventCallback( GtkWidget* widget,
                           GdkEventKey* event,
                           MapClientApp* app )
{
   char curChar = event->string[0];

   TileMapHandler &mapHandler = *app->getDrawingArea()->getTileMapHandler();
   GtkWidget *topLevel = app->getTopLevelWidget();

   bool request_repaint = false;
   //
   // Zoom map
   //
   if (curChar == '-' || curChar == '+') {

      if (curChar == '-')
         app->zoom( false ); // zoom out
      else
         app->zoom( true ); // zoom in

      request_repaint = true;
   }

   bool handled = true;   

   const int MOVE_DELTA = 10;
   const int NBR_ROTATION_STEPS = 360;
   //
   // Rotate/Move map
   //
   switch (event->keyval) 
      {
      case GDK_Escape:
         gtk_widget_destroy(topLevel);
         break;
      case GDK_Up:
         mapHandler.move( 0, -MOVE_DELTA );
         break;
      case GDK_Left:
         mapHandler.move( -MOVE_DELTA, 0 );
         break;
      case GDK_Down:
         mapHandler.move( 0, MOVE_DELTA );
         break;
      case GDK_Right:
         mapHandler.move( MOVE_DELTA, 0 );
         break;
      case GDK_Page_Down:
         mapHandler.rotateLeftDeg( 360 / NBR_ROTATION_STEPS );
         break;
      case GDK_Page_Up:
         mapHandler.rotateLeftDeg( -360 / NBR_ROTATION_STEPS );
         break;
      case GDK_Home:
         mapHandler.setAngle(0);            
         break;  
      default:
         handled = false;
         break;
   }
   // repaint if we handled a key or requested a repaint
   if (handled || request_repaint)
      mapHandler.requestRepaint();
   else {
      // we did not handle anything here
      // do default handling
      return FALSE; 
   }

   // update information
   updateInfoLabels( topLevel, mapHandler );

   return TRUE;
}


void mapButtonPressEvent( GtkWidget *widget,
                          GdkEventButton* event, 
                          MapClientApp* app )
{
   app->mapClick( static_cast<int>(event->x), 
                  static_cast<int>(event->y), event->button);
}

void updateInfoCallback( GtkWidget *widget,
                         MapClientApp* app ) {
   updateInfoLabels( app->getTopLevelWidget(),
                     *app->getDrawingArea()->getTileMapHandler() );
}

void highlightAllCallback (GtkWidget *widget, MapClientApp* app)
{
   bool state = 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
   app->setHighlightAll(state);
}

void alongRoadCallback (GtkWidget *widget, MapClientApp* app)
{
   bool state = 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

   app->setDetectAlongRoad(state);
}


void disableLevelsCallback (GtkWidget *widget, MapClientApp* app)
{
   //   bool state = 
   // gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

   //app->setShouldDisableLevelCheck(state);
}

void roadNamesCallback (GtkWidget *widget, MapClientApp* app)
{
   bool state = 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
   app->setShowRoadNames(state);
}

// helper function to setup info labels
inline GtkWidget *createInfoLabel( GtkWidget *box,
                                   const char *text,
                                   const char *dataString) 
{
   GtkWidget *label = gtk_label_new( text );
   gtk_box_pack_start( GTK_BOX(box),
                       label,
                       TRUE, TRUE, 0 );
   // attach data to top level
   GtkWidget *topLevel = gtk_widget_get_toplevel( label );
   g_object_set_data( G_OBJECT( topLevel ),
                      dataString, 
                      label );
   gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

   return label;
}

MapClientApp::MapClientApp( DBufRequester * req ):
   m_drawingArea( NULL )
{
   setDetectAlongRoad(false);
   setupMainWindow(req);
   setupUserDefinedFeatures();
   setHighlightAll(false);
   setShowRoadNames(true);
   vicVec_t v;
   outputVicinityList(v , this);
}

MapClientApp::~MapClientApp() 
{
   delete m_drawingArea;
}

void MapClientApp::loop() 
{
   // main loop
   gtk_main();
}

void MapClientApp::mapClick( int x, int y, int button ) 
{
   mc2dbg<<"[MapClient] MapClicked("<<x<<", "<<y<<", "<<button<<")"<<endl; 

   TileMapHandler *tileMapHandler = 
      m_drawingArea->getTileMapHandler();
   const char* str = 
      tileMapHandler->getNameForFeatureAt( MC2Point( x, y ) );

   vector<MC2SimpleString> strings;

   tileMapHandler->getServerIDStringForFeaturesAt(
      strings,
      MC2Point( x, y ) );

   m_roadName = tileMapHandler->getRoadAt(MC2Point( x, y ));
   
   tileMapHandler->setSelectedRoad(m_roadName);
   
   // get "street_entry_field" property
   // which contains the entry field that holds
   // information about clicked street
   GtkLabel *textField = GTK_LABEL(getInfoLabel());

   vector<MC2SimpleString> layers =
      tileMapHandler->getLayeredPointInfo( MC2Point( x, y ) );
   
   ostringstream result;

   if (textField == NULL) {
      return;
   }

   int numNewLines = 1;
   std::vector<MC2SimpleString>::size_type index = 1;
   
   if (layers.size() == 0 && !str) {
      result << "<b> No name found!</b>";
   }
   else {
      result << "<b> Where am I:</b>";
      result << endl << "   ";
      numNewLines++;
      mc2dbg << "[MapArea]: String is " << str << endl;

      for(vector<MC2SimpleString>::iterator vsi =
             layers.begin();
          vsi!=layers.end();
          vsi++, index++)
      {
         MC2SimpleString curString(*vsi);
         result << curString;

         if(index == layers.size());
            //result << endl;
         else if(index%3==0) {
            result << endl << "   ";
            numNewLines++;            
         }
         else
            result << " / ";
         
      }   
   }

   result << endl;
   
   MC2SimpleString extendedInformation =
      tileMapHandler->getServerInfo( MC2Point( x, y ) );

   if(extendedInformation.empty()) {
      result << " <b>No extended information could be detected.</b>";
   }    else {
      result << " <b>Extended information: </b>" << endl;
      result << "   " << extendedInformation << endl;
      numNewLines++;
   }
   
   

   
   for( ; numNewLines<20; numNewLines++) {
      result << endl;
   }

   GtkWidget* topLevel = getVicinityArea();
   setInfoLabel( topLevel, "vcitem", result );
}

void MapClientApp::setupUserDefinedFeatures()
{
   TileMapHandler& mapHandler = *getDrawingArea()->getTileMapHandler();

   MC2Coordinate coord(663399930, 154995706);

   ScreenOrWorldCoordinate scm(coord);
   UserDefinedBitMapFeature* ubm = new UserDefinedBitMapFeature(scm, "favorite");
   ubm->setVisible(true);
   ubm->setName("Test av UDF");

   vector<UserDefinedFeature*>* features = new vector<UserDefinedFeature*>; 
   features->push_back(ubm);
   mapHandler.setUserDefinedFeatures(features);
}


void MapClientApp::setShouldDisableLevelCheck(bool disableLevelCheck) {
   //m_drawingArea->getTileMapHandler()->setShouldCheckLevels(!disableLevelCheck);
}


void MapClientApp::setupMainWindow( DBufRequester *req ) 
{
//
   // Setup top level window
   //
   m_window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
   gtk_window_set_title(GTK_WINDOW( m_window ), "Wayfinder Access Debug Tool");
   
   // Setup signal for top level window
   // handle delete_event, given by the window manager
   g_signal_connect( G_OBJECT( m_window ), "delete_event",
                     G_CALLBACK( delete_event ), NULL );
    
   // handle destroy which occurs when we call gtk_widget_destroy or
   // if the "delete_event" callback returns false
   g_signal_connect( G_OBJECT( m_window ), "destroy",
                     G_CALLBACK( destroy_event ), NULL );
   
   g_signal_connect( G_OBJECT( m_window ), "key_press_event",
                     G_CALLBACK( keyEventCallback ), this );


   //
   // Create following layout with
   //  a - map view
   //  b - results
   //  c - result info
   //  d - buttons
   //   ________
   //  |    |   |
   //  |    | b |
   //  |    | _ |
   //  |    |   |
   //  | a  | c |
   //  |    | _ |
   //  |    |   |
   //  |    | d |
   //  |____|___|

   GtkWidget *m_table = gtk_table_new( 10, 16, false);
   gtk_container_add( GTK_CONTAINER( m_window ), m_table);


   /* Setup vicinity list. */
   m_vArea  = gtk_frame_new ("Results");

   GtkWidget *vcitem = gtk_label_new("1. AAAAAAAAAAAAAAAAAAAAAAAA 123456");
   gtk_container_add (GTK_CONTAINER (m_vArea), vcitem);
   GtkWidget *topLevel = gtk_widget_get_toplevel( vcitem );
   g_object_set_data( G_OBJECT( topLevel),
                      "vcitem", 
                      vcitem );
   gtk_label_set_line_wrap (GTK_LABEL (vcitem), TRUE);
   gtk_widget_set_size_request(vcitem, 250, 260);

   /* Setup the map */
   m_drawingArea = new MapDrawingArea( req );

   /* Setup the information label that displays status. */
   GtkWidget* infoLabelFrame  = gtk_frame_new ("Status");
   m_infoLabel = gtk_label_new("Ready.");

   gtk_widget_set_size_request(m_infoLabel, 
                               250, 
                               60);

   gtk_widget_set_size_request(infoLabelFrame, 
                               260, 
                               60);

   gtk_label_set_line_wrap (GTK_LABEL (m_infoLabel), 
                            TRUE);

   // GtkWidget * optionsBox = 
   //  gtk_vbox_new (false, 0);


   gtk_container_add( GTK_CONTAINER(infoLabelFrame),
                      m_infoLabel);

   /* Create the check-buttons */
   GtkWidget * optionsParent = 
      gtk_vbox_new (true, 5);   

   GtkWidget * optionsBox1 = 
      gtk_hbox_new (true, 5);

   GtkWidget * optionsBox2 = 
      gtk_hbox_new (true, 5);

   GtkWidget *showAll = 
      gtk_check_button_new_with_label ( "Highlight all"  );

   GtkWidget *showRoads = 
      gtk_check_button_new_with_label ( "Road names"  );

   GtkWidget *disableLevels =
      gtk_check_button_new_with_label ( "Level checks"  );

   GtkWidget *alongRoad =
      gtk_check_button_new_with_label ( "Along the road"  );
   
   gtk_box_pack_start( GTK_BOX( optionsBox1 ), 
                       showAll,
                       true,
                       true,
                       0);

   gtk_box_pack_start( GTK_BOX( optionsBox1 ), 
                       showRoads,
                       true,
                       true,
                       0);
   
   gtk_box_pack_start( GTK_BOX( optionsBox2 ), 
                       disableLevels,
                       true,
                       true,
                       0);

   gtk_box_pack_start( GTK_BOX( optionsBox2 ), 
                       alongRoad,
                       true,
                       true,
                       0);

   gtk_box_pack_start( GTK_BOX( optionsParent ), 
                       optionsBox1,
                       true,
                       true,
                       0);

   gtk_box_pack_start( GTK_BOX( optionsParent ), 
                       optionsBox2,
                       true,
                       true,
                       0);

   GtkWidget* options  = gtk_frame_new ("Options");

   gtk_widget_set_size_request(optionsBox1, 215, 70);
   gtk_widget_set_size_request(options, 260, 70);
   gtk_container_add (GTK_CONTAINER (options), optionsParent);


   /* Create the buttons. */
   GtkWidget *udfsButton = 
      gtk_button_new_with_label("Detect UDFs");

   GtkWidget *vicinityButton = 
      gtk_button_new_with_label("Detect POIs");

   GtkWidget *crossingsButton = 
      gtk_button_new_with_label("Detect Crossings");

   GtkWidget *quitbutton = 
      gtk_button_new_with_label("Quit");


   g_signal_connect( G_OBJECT( m_drawingArea->widget() ), 
                     "button_press_event",
                     G_CALLBACK( mapButtonPressEvent ), 
                     this );

   g_signal_connect( G_OBJECT( showAll ), 
                     "clicked",
                     G_CALLBACK( highlightAllCallback ),
                     this );

   g_signal_connect( G_OBJECT( showRoads ),
                     "clicked",
                     G_CALLBACK( roadNamesCallback ),
                     this );

   g_signal_connect( G_OBJECT( udfsButton ), 
                     "clicked",
                     G_CALLBACK( printUDFsCallback ),
                     this );

   g_signal_connect( G_OBJECT( disableLevels ), 
                     "clicked",
                     G_CALLBACK( disableLevelsCallback ),
                     this );


   g_signal_connect( G_OBJECT( alongRoad ), 
                     "clicked",
                     G_CALLBACK( alongRoadCallback ),
                     this );

   g_signal_connect( G_OBJECT( vicinityButton ), 
                     "clicked",
                     G_CALLBACK( printVicinityCallback ),
                     this );


   g_signal_connect( G_OBJECT( crossingsButton ), 
                     "clicked",
                     G_CALLBACK( printCrossingsCallback ),
                     this );

   // This will cause the window to be destroyed by calling
   // gtk_widget_destroy(window) when "clicked".  Again, the destroy
   // signal could come from here, or the window manager.
   g_signal_connect_swapped( G_OBJECT( quitbutton ), 
                             "clicked",
                             G_CALLBACK( gtk_widget_destroy ),
                             G_OBJECT( m_window ) );

   gtk_table_attach( GTK_TABLE(m_table), 
                     m_drawingArea->widget(), 
                     0, 7, 0, 17, 
                     (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 
                     (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 
                     0, 0);
   

   gtk_table_attach( GTK_TABLE(m_table),
                     m_vArea, 8, 9, 0, 6,
                     (GtkAttachOptions)(0),
                     (GtkAttachOptions)(GTK_FILL),
                     0, 0);
   
   gtk_table_attach( GTK_TABLE(m_table),
                    infoLabelFrame, 8, 9, 6, 7,
                    (GtkAttachOptions)(0),
                    (GtkAttachOptions)(GTK_FILL),
                    0, 0);

   gtk_table_attach( GTK_TABLE(m_table),
                    options, 8, 9, 7, 8,
                    (GtkAttachOptions)(GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions)(GTK_FILL),
                    0, 0);

   gtk_table_attach( GTK_TABLE(m_table),
                     udfsButton, 8, 9, 13, 14,
                     (GtkAttachOptions)(GTK_SHRINK | GTK_FILL),
                     (GtkAttachOptions)(GTK_FILL),
                     0, 0);

   gtk_table_attach( GTK_TABLE(m_table),
                  vicinityButton, 8, 9, 14, 15,
                  (GtkAttachOptions)(GTK_SHRINK | GTK_FILL),
                  (GtkAttachOptions)(GTK_FILL),
                  0, 0);

   gtk_table_attach( GTK_TABLE(m_table),
                  crossingsButton, 8, 9, 15, 16,
                  (GtkAttachOptions)(GTK_SHRINK | GTK_FILL),
                  (GtkAttachOptions)(GTK_FILL),
                  0, 0);

   gtk_table_attach( GTK_TABLE(m_table),
                  quitbutton, 8, 9, 16, 17,
                  (GtkAttachOptions)(GTK_SHRINK | GTK_FILL),
                  (GtkAttachOptions)(GTK_FILL),
                  0, 0);

   gtk_table_set_row_spacings( GTK_TABLE(m_table), 5);
   gtk_table_set_col_spacings( GTK_TABLE(m_table), 0);
   gtk_table_set_col_spacing ( GTK_TABLE(m_table), 7, 5);
   
   // Sets the border width of the window. 
   gtk_container_set_border_width(GTK_CONTAINER( m_window ), 5 );

   // show all widgets
   gtk_window_set_position(GTK_WINDOW ( m_window ), GTK_WIN_POS_CENTER);
   gtk_widget_show_all( m_window );
   gtk_window_resize(GTK_WINDOW ( m_window ), 820, 530);

   gtk_widget_set_sensitive(GTK_WIDGET(disableLevels), false);
   
   for(int i = 0; i<6; i++)
      zoom(true);
}

void MapClientApp::zoom(bool in) 
{
   const float level = 1.1;
   if (in) 
      m_drawingArea->getTileMapHandler()->zoom( 1.0 / level );
   else
      m_drawingArea->getTileMapHandler()->zoom( level );
}

void MapClientApp::setRouteID() 
{

   // Set the route id of the tilemap handler.
   mc2dbg << "[MDW]: set route ID" << endl;
   GtkEntry *entry = 
      static_cast<GtkEntry*>(
             g_object_get_data ( G_OBJECT( m_window ),
                                 "route_id_entry" ) );
   // we did not find any route id entry widget
   if (entry == NULL)
      return;
   const char *tmp = gtk_entry_get_text( entry );
   std::string routeIDStr(tmp ? tmp : "");

   TileMapHandler &mapHandler = *m_drawingArea->getTileMapHandler();

   if ( routeIDStr.empty() ) {
      mapHandler.clearRouteID();
   } else {
      if ( routeIDStr.find("_") != string::npos ) {
         RouteID routeID( routeIDStr.c_str() );
         mapHandler.setRouteID(routeID);
      } else if ( routeIDStr[0] == '[' ) {
         // Bounding box
         string maxLat = "";
         int pos = 1;
         while ( routeIDStr[++pos] != ',' ) {
            maxLat += routeIDStr[pos];
         }
         string minLon;
         while ( routeIDStr[++pos] != ')' ) {
            minLon += routeIDStr[pos];
         }
         string minLat;
         pos += 2;
         while ( routeIDStr[++pos] != ',' ) {
            minLat += routeIDStr[pos];
         }
         string maxLon;
         while ( routeIDStr[++pos] != ')' ) {
            maxLon += routeIDStr[pos];
         }
         int32 maxLatInt = atoi( maxLat.c_str() );
         int32 minLonInt = atoi( minLon.c_str() );
         int32 minLatInt = atoi( minLat.c_str() );
         int32 maxLonInt = atoi( maxLon.c_str() );
         MC2BoundingBox newBox( maxLatInt, minLonInt,
                                minLatInt, maxLonInt ) ;
         mc2log << "[MDW]: Setting new bbox = "
                << newBox << endl;
         mapHandler.setBoundingBox( newBox );
         mapHandler.requestRepaint();
      } else if ( TileMapParamTypes::getParamType( 
                  routeIDStr.c_str() ) == 
                  TileMapParamTypes::TILE )
      {
         // TileMap
         mapHandler.setBBoxFromParamStr( 
            MC2SimpleString( routeIDStr.c_str() ) );
         mapHandler.requestRepaint();
      } else {
         // Set max cache size instead
         char* tmpPtr = NULL;
         uint32 newSize = strtoul( routeIDStr.c_str(), 
                                   &tmpPtr, 0 );
         if ( tmpPtr != routeIDStr.c_str() && 
              *tmpPtr == '\0' ) {

            mc2log << "[MDW]: Setting max size for cache to "
                   << newSize << endl;

            m_drawingArea->setMaxCacheSize( newSize );

         } else {
            mc2log << warn << "[MDW]: Unknown cache size \""
                   << routeIDStr << "\"" << endl;
         }
      }
   }

}

GtkWidget * MapClientApp::getVicinityArea()
{
	return m_vArea;
}


/**
 * update all information labels
 */
void updateInfoLabels( GtkWidget *topLevel, TileMapHandler &handler) 
{

   ostringstream labelStr;

   labelStr << handler.getBoundingBox();
   setInfoLabel( topLevel, "bounding_box_label", labelStr );

   labelStr << "Scale: " << handler.getScale();
   setInfoLabel( topLevel, "scale_label", labelStr );

   labelStr << "Angle: " << handler.getAngle();
   setInfoLabel( topLevel, "angle_label", labelStr );

   //labelStr << "MapSize: " << handler.getTotalMapSize();
   //setInfoLabel( topLevel, "mapsize_label", labelStr );

    // fill in detail level
   const TileMapLayerInfoVector* layers =
      handler.getLayerIDsAndDescriptions();
   if ( layers != NULL ) {
     for( TileMapLayerInfoVector::const_iterator it = layers->begin();
          it != layers->end();
          ++it ) {

        if ( it != layers->begin() )
           labelStr<<endl;

        labelStr << boolalpha << 
           it->getName() << ":" <<
           it->getID()<< ": detail=" <<
           handler.getDetailLevelForLayer( it->getID() )<< ",on=" <<
           it->isVisible();
     }
   }
   setInfoLabel( topLevel, "detail_level_label", labelStr );
}

bool MapClientApp::hasRoad()
{
   return !getRoadName().empty();
}

const MC2SimpleString& MapClientApp::getRoadName()
{
   return m_roadName;
}

