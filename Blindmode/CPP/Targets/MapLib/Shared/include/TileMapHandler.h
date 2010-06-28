/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_MAPHANDLER_H
#define TILE_MAPHANDLER_H

#include "MC2SimpleString.h"
#include "TileMapConfig.h"
#include "TileMapHandlerTypes.h"

#include <map>
#include <set>
#include <vector>
#include <list>
#include <sstream>

#include "TileMapToolkit.h"
#include "MapPlotter.h"
#include "MC2BoundingBox.h"
#include "MapProjection.h"
#include "LangTypes.h"

#include "PixelBox.h"
#include "OverlapDetector.h"
#include "MovementHelper.h"
#include "TileMap.h"
#include "MapMovingInterface.h"
#include "MapDrawingInterface.h"
#include "BitmapMovementHelper.h"

using namespace isab;

class DBufRequester;
class BitBuffer;
class TileMapFormatDesc;
class ClickInfo;
class CoordsArg;
class TileMapUtil;
class TileMap;
class TileMapParams;
class TileMapCoord;
class TilePrimitiveFeature;
class DBufRequestListener;
class TransformMatrix;
class FileDBufRequester;
class RouteID;
class MC2Coordinate;
class MC2Point;
class TileMapTextHandler;
class UserDefinedFeature;
class DirectedPolygon;
class TileMapEventListener;
class TileMapTextSettings;
class TileMapLayerInfoVector;
class TileMapContainer;
class TileCategory;
class TMKTimingInfo;
class TileMapHandlerReleaseChecker;
class TileMapInfoCallback;
class TileMapEvent;
class TileMapHandler;
class Road;
class RoadColletcion;

#include "VicinityItem.h"
#include "Crossings.h"


/**
 *    Class for returning information about clicked items.
 */
class TileMapHandlerClickResult : public ClickInfo {
public:
   TileMapHandlerClickResult() : m_clickedFeature(NULL),
                                 m_selectionOutline(NULL),
                                 m_distance(MAX_UINT32) {}

   /**
    *   Returns the name of the clicked feature, if a
    *   feature in the map has been clicked.
    */
   inline const char* getName() const {
      return m_name.c_str();
   }

   /**
    *   Returns the server string to use when getting more information
    *   about an item when a feature in the map has been clicked.
    */
   inline const char* getServerString() const {
      return m_serverString.c_str();
   }

   /**
    *   Returns the feature clicked, if any. Probably already owned
    *   by the caller of the method. NULL if a feature in the map
    *   was clicked.
    */
   inline UserDefinedFeature* getClickedUserFeature() const {
      return m_clickedFeature;
   }

   /// Returns the outline feature. Must not be deleted.
   inline UserDefinedFeature* getOutlineFeature() const {
      return m_selectionOutline;
   }

   inline int32 getDistance() const {
      return m_distance;
   }
   
   inline bool shouldHighlight() const {
      return getOutlineFeature() != NULL;
   }
private:
   friend class TileMapHandler;
   
   MC2SimpleString m_name;
   MC2SimpleString m_serverString;
   UserDefinedFeature* m_clickedFeature;
   UserDefinedFeature* m_selectionOutline;
   uint32 m_distance;
};


#include "TileMapGarbage.h"

#include "TileFeature.h"

/**
 *    Cross-platform TileMap getter and drawer.
 *    No platform dependand code should be here, only
 *    in the drivers, e.g. XXXMapPlotter.
 */
class TileMapHandler : public MapProjection,
                       public TileMapTimerListener,
                       public TileMapIdleListener,
                       public MapMovingInterface,
                       public MapDrawingInterface {
public:
   
   /**
    *   Creates a new TileMapHandler plotting on the supplied
    *   MapPlotter and requesting from the supplied requester.
    *   @param plotter       The MapPlotter.
    *   @param requester     DBufRequester to use.
    *   @param platform      The toolkit for the platform.
    *   @param eventListener 
    */
   TileMapHandler(MapPlotter* plotter,
                  DBufRequester* requester,
                  TileMapToolkit* platform,
                  TileMapEventListener* eventListener = NULL );

   /**
    *   The destructor.
    */
   virtual ~TileMapHandler();
   
   /**
    *   Tells the TileMapHandler that it is time to redraw
    *   the screen.
    */
   void repaint(uint32 maxTimeMillis);
   
   /**
    *   Called when a timer expires.
    */
   void timerExpired(uint32 id);

   /**
    *   Called when the system is a bit idle.
    */
   void runIdleTask(uint32 id);

   /**
    *   Sets automatic selection highlighting on/off.
    */
   void setHighlight( bool onoff );

   /**
    *   Sets highlight point. MC2Point(-1, -1) means auto-center.
    */
   void setHighlightPoint( const MC2Point& hightlightPoint );

   /**
    *   Sets new text settings in the text handler.
    */
   void setTextSettings( const TileMapTextSettings& settings );
   
   /**
    *   Sets the current route id to show in the map.
    *   @param routeID Route to show.
    */
   void setRouteID(const RouteID& routeID);

   /**
    *   Removes the route id from the TileMapHandler.
    *   @see setRouteID.
    */
   void clearRouteID();

   /**
    *    Set the language for the maps 
    *    using the LangTypes::language_t type.
    */   
   void setLanguage( LangTypes::language_t lang );

   /**
    *    Returns the lanugage.
    */
   LangTypes::language_t getLanguage() const { return m_lang; }
   
   /**
    *    Set the language for the maps using the nav2 lang type.
    */   
   void setNav2Language( uint32 nav2Lang );

   /**
    *   Set the language using the specified iso639 string.   
    *   The string can consist of two or three letters and must be
    *   NULL terminated. 
    *   @param isoStr The string, containing two or three letters.
    *   @return   The language as iso639 that was set to MapLib.
    */
   const char* setLanguageAsISO639( const char* isoStr );

   /**
    *   Returns all information possible at the same time.
    *   More efficient than calling getNameForFeatureAt followed
    *   by getSelectionOutline etc.
    *
    *   @param highlightable  [Optional] If set to true, only
    *                         highlightable features will be
    *                         processed. Default is that
    *                         all feature types are processed.
    *   @param infoCallback   [Optional] Callback that will be called
    *                         when a missing string map is received.
    */
   void getInfoForFeatureAt( TileMapHandlerClickResult& res,
                             const MC2Point& point,
                             bool highlightable = false,
                             TileMapInfoCallback* infoCallback= NULL );
   
   /**
    *   Returns the name for an item at the specified point.
    *   @deprecated Use getInfoForFeatureAt instead.
    *   @param point    The coordinate to look at.
    *   @param distance Set to the distance to the found item in pixels
    *                   if not NULL.
    */
   const char* getNameForFeatureAt(const MC2Point& point,
                                   uint32* distance = NULL );
   
   /**
    *   Puts most probably only one id string to identify the
    *   object at the given coordinate.
    *   @deprecated Use getInfoForFeatureAt instead.
    *   @param strings The strings are put here. Send them to the server.
    *   @return The number of items found.
    */
   int getServerIDStringForFeaturesAt( std::vector<MC2SimpleString>& strings,
                                       const MC2Point& point );

   /**
    *   Set the user defined features that should be drawn after
    *   the map.
    */
   void setUserDefinedFeatures( std::vector<UserDefinedFeature*>* features );

   /**
    *   Returns the poi categories or NULL if not loaded yet.
    *   Must _not_ be saved since the vector can disappear
    *   anytime after you have left your calling method.
    */
   const std::vector<const TileCategory*>* getCategories() const;


   /*   WayFinder Access specific functions */

   /*   Level checking for roads that is put on hold until 
    *   better data can be delivered from the server.
    *
   void setShouldCheckLevels(bool shouldCheckLevels) {
      m_shouldCheckLevels = shouldCheckLevels;
   }

   bool shouldCheckLevels() {
      return m_shouldCheckLevels;
   }
   */

   enum {
      LIMIT_CUTOFF_CROSSINGS = 3200,    //Limit for increasing cutoff
      LIMIT_CUTOFF_CROSSINGS_ALONG_ROAD = 300, //No need to get out of hand
      START_CUTOFF_CROSSINGS = 200,     //Start value for increasing cutoff
      DEFAULT_CUTOFF = 2000,            //Default cutoff for POIs and UDFs
      ROAD_DISTANCE_CUTOFF = 10
   };


   /**
    *   Sometimes we want to the use the information in the map
    *   without actually redrawing it every time something changes.
    *
    *   This method makes this change possible.
    *
    *
    */
   
   void setIsDrawing(bool isDrawing);

   RoadCollection*
   getAllRoads(const MC2Point& centerPos,
               int cutOffMeters);

   
   
   void
   getPOIsAndUDFsAlongRoad( const MC2Point& userPosition,
                            const MC2SimpleString& roadName,
                            vicVec_t& retVec);
   
   void
   getCrossingsAlongRoad( const MC2Point& userPosition,
                          const MC2SimpleString& roadName,
                          vicVec_t& retVec,
                          bool overRideCutOff = false,
                          int cutOff = LIMIT_CUTOFF_CROSSINGS_ALONG_ROAD);

   bool
   isAlongRoad(Road& road, VicinityItem* vItem);
   
   /**
    *   Detects all crossings in the vicinity of userPosition. Will 
    *   look in an increasingly larger area until it has found
    *   at least 15 elements, or when the current cutoff value
    *   exceeds LIMIT_CUTOFF_CROSSINGS. If overRideCutOff is true, it
    *   will attempt to get as many crossings as possible.
    *   
    *   @param  userPosition    Current center point of search.
    *   
    *   @param  retVec          Vector that will contain detected
    *                           crossings.
    *
    *   @param  overRideCutOff  [optional] Ignore cutoff and detect 
    *                           all crossings  within the maximum 
    *                           area allowed by cutoff limit 
    *                           (LIMIT_CUTOFF_CROSSINGS)
    *
    *   @param  overRideCutoff  
    */
   
   void getCrossingsInVicinity( const MC2Point& userPosition,
                                vicVec_t& retVec,
                                bool overRideCutOff = false,
                                int cutoff = LIMIT_CUTOFF_CROSSINGS);

   /**
    *   Child function of above function. Is repeatedly called with
    *   an increasing cutoff value.
    *
    *   @param  cutoffMeters    Current cutoff limit.
    *
    */

   void getCrossingsInVicinityLimited( const MC2Point& userPosition,
                                       const MC2SimpleString& roadName,
                                       vicVec_t& retVec,
                                       int cutoffMeters );

   /**
    *   Will detect all primitives that are roads in tileMap
    *   at the level mapLevel and create create corresponding road 
    *   objects that are stored in a RoadCollection.
    *   
    *   Please note that it is up to the caller to free the memory
    *   allocated for the return value.
    *   
    *   @param  tileMap         The tilemap that contains the
    *                           primitives.
    *   
    *   @param  mapLevel        The internal level of the map to
    *                           look at.
    *   
    *   @param  includeBox      The bounding box that is derived from 
    *                           the current cutoff value. If a road is
    *                           not within this box, it is not considered.
    *                           
    *   @return RoadCollection* A collection of all the roads that were 
    *                           detected, suitable for further processing. 
    *                           Note that this resource must be freed 
    *                           upon completed usage.
    *           
    **/

   RoadCollection* 
   getRoadsInTileMap( TileMap* tileMap,
                      int mapLevel,
                      const MC2BoundingBox& includeBox);

   /*
    *   Helper method for getRoadsInVicinity, creates a
    *   Road object from a TilePrimitiveFeature.
    *   
    *   @param  tileMap         The TileMap that contains the feature.
    *                           Needed due to name lookups etc.
    *   @param  feature         The primitive feature that will be used
    *                           as a basis for the road object.
    *   @return Road            The completed road.
   **/

   Road createRoad( const TileMap* tileMap, 
                    const TilePrimitiveFeature& feature);


   /** 
    *   Returns a road object representing the  road at point if it exists, 
    *   otherwise it will return an empty road object.
    *   
    *   @param  point           Position to look at.
    *   
    *   @return Road            Returns a complete Road object if
    *                           there is a road at point, otherwise
    *                           the object will be empty.
    */
   
   MC2SimpleString getRoadAt(MC2Point point);


   /**
    *   Returns a string giving extended information about a
    *   clicked feature.
    *
    *
    *   @param  point                   The clicked position.
    *
    *   @return MC2SimpleString         The extended information.
    *                                   String will be empty if no
    *                                   feature was clicked upon.
    *
    **/
   
   MC2SimpleString getServerInfo(MC2Point point);
   
   /**
    *   This method will return a vector containing basically as much
    *   information as possible given a specific location. Working
    *   downwards, it continually adds strings describing regions that
    *   encircle the point. For example:
    *   
    *   [ Sweden, Lund, Baravagen ]
    *
    *   @param  point   The point to base the search on. 
    *   
    */

   std::vector<MC2SimpleString> getLayeredPointInfo(MC2Point point);
   


   /**
    *   Returns true if prim overlaps bbox in any way.
    *
    *   @param bbox                     The bounding box to check.
    *   @param prim                     The primitive to investigate.
    *   @param smallestLineDist         The smallest distance from the
    *                                   center of the bounding box to
    *                                   a line (road) for it to be
    *                                   considered as overlapping.
    *   
    */

   bool primitiveOverlaps(const MC2Point& centerPoint, 
                          const TilePrimitiveFeature& prim,
                          int& smallestLineDist);

   /**
    *   Will find all the intersections between the roads in
    *   the RoadCollection roads. 
    *   
    *   @param  retVector       The vicinity item vector that will
    *                           be filled with results.
    *   
    *   @param  roads           The collection of roads that will be 
    *                           checked for intersections.
    *   
    *   @param  userCoord       The user coordinate. Used to determine
    *                           the distance and angle between the
    *                           user and the crossing.
    *
    *   @param  cutoff          The cutoff limit. If an intersection
    *                           is detected that is farther away from
    *                           the users position than this, it will
    *                           not be considered.
    **/

   void findIntersections( vicVec_t& retVector, 
                           RoadCollection& roads, 
                           const MC2Coordinate& userCoord, 
                           int cutoff);

   /**
    *   Will find all the intersections between the roads in
    *   the RoadCollection roads and userRoad. 
    *   
    *   @param  retVector       The vicinity item vector that will
    *                           be filled with results.
    *
    *
    *   @param  userRoad        The road that the user is currently
    *                           on.
    *
    *                           *   
    *   @param  roads           The collection of roads that will be 
    *                           checked for intersections.
    *   
    *   @param  userCoord       The user coordinate. Used to determine
    *                           the distance and angle between the
    *                           user and the crossing.
    *
    *   @param  cutoff          The cutoff limit. If an intersection
    *                           is detected that is farther away from
    *                           the users position than this, it will
    *                           not be considered.
    **/

   
   void findIntersections( vicVec_t& retVector,
                           const MC2SimpleString& roadName,
                           RoadCollection& roads, 
                           const MC2Coordinate& userCoord, 
                           int cutoff);
   /**  
    *   Will detect all UDFs (user defined features) that are within
    *   a distance of DEFAULT_CUTOFF meters.
    * 
    *   @param  userPosition    Position of user, used to determine distances.
    *
    *   @param  retVec          Return value, will contain found UDFs.
    *
    *   @param  cutOff          [optional] Maximum distance to UDFs.
    */

   void getUDFsInVicinity(const MC2Point& userPosition, 
                          vicVec_t& retVec,
                          int cutOff = DEFAULT_CUTOFF);
   
   /**
    *   Detects all POIs (Point Of Interest) in the vicinity of userPosition.
    *   
    *   @param  userPosition    The position of the user.+
    *
    *   @param  retVec          Return value, will contain found POIs.
    *
    *   @param  cutOff          [optional] Maximum distance of POI.
    *
    */

   void getPOIsInVicinity(const MC2Point& userPosition, 
                          vicVec_t& retVec,
                          int cutOff = DEFAULT_CUTOFF);

   /**
    *   Will add all POIs that are in a given level on a tileMap.
    *   
    *   @param  tileMap         The tileMap that contains the POIs.
    *
    *   @param  level           The level to look at.
    *
    *   @param  retVec          Return value, will contain found POIs.
    */

   void addPOIsInTileMapLevel(TileMap* tileMap,
                              int level,
                              const MC2Point& userPosition,
                              int cutoff,
                              vicVec_t& retVec);

   /**
    *   Will evaluate primitive and if it fits the criteria add it as
    *   a vicinity item POI to retVec. 
    *   
    *   @param  tileMap         The tileMap that contains the primitive.
    *
    *   @param  primitive       The primitive.
    *
    *   @param  userPosition    The users position. Used to determine distance
    *                           and angle.
    *
    *   @param cutOff           The maximum distance to the POI.
    *   
    */


   void evaluatePrimitiveAsPOI(TileMap* tileMap,
                               const TilePrimitiveFeature& primitive,
                               const MC2Point& userPosition,
                               vicVec_t& retVec,
                               int cutOff);
 

   /**  
    *   Debug-related functions that are used in the linux
    *   client.
    *
    */

   #ifdef __unix__
   /**
    *  Marks detected points (UDFs, POIs or crossings)
    */
   void markDetectedPoints(vicVec_t& points);
   void setSelectedRoad(MC2SimpleString selectedRoad);
   
   /* 
   *   The detected points (UDFs, POIs, crossings) that should be drawn.
   */
   std::vector<std::vector<MC2Coordinate> >   m_detectedPoints;
   MC2SimpleString m_selectedRoad;
   
   #endif
                         

   /**
    *   Synchronizes the layer info between the local copy
    *   and the copy of the ui.
    *   @return True if the info was updated and may need to be 
    *           re-displayed.
    */
   bool updateLayerInfo( TileMapLayerInfoVector& info );

   /**
    *   Enable or disable a category with the specified id.
    *   Will not work correctly until getCategories returns non-NULL.
    *   @param id      The id of the parameter to disable/enable.
    *   @param enabled True/false.
    *   @return True if the category id was found.
    */
   bool setCategoryEnabled( int id, bool enabled );

   /**
    *   Sets tracking mode on or off.
    *   On (true) means that all of the map is drawn at once, including the
    *   edges of the streets and that the texts are moved with the map
    *   to be updated correctly later.
    */
   //void setTrackingMode( bool on );
   
   /**
    *   Returns the layer id:s and descriptions. If the vector is NULL
    *   it means that the layers and descriptions have not arrived
    *   yet, try again later. The ids are the ids to use when
    *   choosing which layers to display.
    */
   const TileMapLayerInfoVector* getLayerIDsAndDescriptions() const;

   /**
    *   Returns the detail level for the layer.
    *   FOR DEBUG!
    */
   uint32 getDetailLevelForLayer(uint32 layerID) const;

   /**
    *   Returns the total map size for the maps loaded so far.
    *   Only for DEBUG!.
    */
   uint32 getTotalMapSize() const;

   /**
    *   Dups the sizes of the maps to mc2dbg.
    *   Only for DEBUG!. 
    */
   void dumpMapSizes() const;

   /**
    *   Requests that the screen should be repainted soon.
    */
   void requestRepaint();

   /**
    *   Requests that the display should be repainted now.
    */
   void repaintNow() {
      repaint(0);
   }

   /**
    *   Puts the dimensions of the displayed map in the
    *   variables.
    */
   PixelBox getMapSizePixels() const;

   /**
    * Set the position of the copyright string of the map. 
    * @param   pos   The left baseline position of the copyright string.
    */
   void setCopyrightPos( const MC2Point& pos );
   
   /**
    * Set if to show copyright string.
    */
   void showCopyright( bool show );

   template<class VECTOR> void getAllRequesters( VECTOR& dest ) {
      if ( m_requester ) {
         m_requester->getAllRequesters( dest );
      }
   }

   // -- Implementation of MapMovingInterface - all is repeated...
   
   void setMovementMode( bool moving ) {
      setMoving( moving );
   }
   
   /**
    *   Yet another interface to clicking.
    *   Returns a reference to info for the feature at the specified
    *   position on the screen.
    *   @param point    The point on the screen where
    *                   the info should be found.
    *   @param onlyPois True if only poi info should be returned.
    */
   const ClickInfo&
      getInfoForFeatureAt( const MC2Point& point,
                           bool onlyPois,
                           TileMapInfoCallback* infoCallback = NULL );

   inline const MC2Coordinate& getCenter() const {
      return MapProjection::getCenter();
   }

   inline void transform( MC2Point& point,
                          const MC2Coordinate& coord ) const {
      MapProjection::transformPointInternalCosLat( point, coord );
   }

   inline void inverseTransform( MC2Coordinate& coord,
                                 const MC2Point& point ) const {
      MapProjection::inverseTranformUsingCosLat( coord,
                                                 point );
   }

   inline void setCenter( const MC2Coordinate& newCenter ) {
      MapProjection::setCenter( newCenter );
   }

   inline void setPoint(const MC2Coordinate& newCoord,
                        const MC2Point& screenPoint ) {
      MapProjection::setPoint( newCoord, screenPoint );
   }

   inline void move( int deltaX, int deltaY ) {
      MapProjection::move( deltaX, deltaY );
   }
   
   inline void setAngle(double angleDegrees) {
      MapProjection::setAngle( angleDegrees );
   }

   inline void setAngle( double angleDegrees,
                         const MC2Point& rotationPoint ) {
      MapProjection::setAngle( angleDegrees, rotationPoint );
   }
   
   /**
    *   Rotates the display the supplied number of degrees.
    */
   void rotateLeftDeg(int nbrDeg);

   inline double getAngle() const {
      return MapProjection::getAngle();
   }

   inline double setScale( double scale ) {
      return MapProjection::setDPICorrectedScale( scale );
   }
   
   inline double getScale() const {
      return MapProjection::getDPICorrectedScale();
   }

   inline double zoom( double factor ) {
      return MapProjection::zoom( factor );
   }

   inline double zoom( double factor, 
                       const MC2Coordinate& zoomCoord,
                       const MC2Point& zoomPoint ) {
      return MapProjection::zoom( factor, zoomCoord, zoomPoint );
   }

   inline void setPixelBox( const MC2Point& oneCorner,
                            const MC2Point& otherCorner ) {
      MapProjection::setPixelBox( oneCorner, otherCorner );
   }
   
   inline void setWorldBox( const MC2Coordinate& oneCorner,
                            const MC2Coordinate& otherCorner ) {
      MapProjection::setBoundingBox( MC2BoundingBox( oneCorner, 
                                                     otherCorner ) );
   }
      
   bool canHandleScreenAsBitmap() const;
   void moveBitmap(int deltaX, int deltaY);
   void setPointBitmap( const MC2Point& screenPoint );
   void setBitmapDragPoint( const MC2Point& dragPoint );
   void zoomBitmapAtPoint( double factor, 
                           const MC2Point& screenPoint );
   void zoomBitmapAtCenter( double factor );
 
protected:
   /**
    *   Called by tranformmatrix to tell TileMapHandler that
    *   the matrix has been updated.
    */
   void matrixUpdated();

public:

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
    *    Sends a TileMapEvent to all listeners.
    */
   void sendEvent( const TileMapEvent& event );
   
   /* returns true if the specified layer is already visible,
      else false. FIXME: Un-inline this function */
   bool isLayerVisible(int layerNo) {
      return m_layersToDisplay.find( layerNo ) != m_layersToDisplay.end();
   }

   /**
    *   sets the specified layer for display if it is not getting displayed,
    *   else sets it to not be displayed.
    */
   void toggleLayerToDisplay(int layerNo);

   /**
    *   Get's a reference to the screen coords for the feature.
    *   To be used by the textplacement.
    */
   VectorProxy<MC2Point>& getScreenCoords( TilePrimitiveFeature& feature );

   /**
    *    Get the outer pixel width of the polyline.
    */
   uint32 getPolylineOuterPixelWidth( 
                              const TilePrimitiveFeature& prim ) const;

   /**
    *    Set the pixelbox for the progress indicator text.
    */
   void setProgressIndicatorBox( const PixelBox& box );

   /**
    *    @return The DBufRequestListener.
    */
   DBufRequestListener* getDBufRequestListener() const;

   /**
    *    Set timing info to display or null.
    */
   void setTMKTimingInfo( const TMKTimingInfo* info ) {
      m_tmkTimingInfo = info;
   }


   /**
    * Sets the bbox and sets routeID if route param str.
    *
    * @param paramString The tilemap param string.
    */
   void setBBoxFromParamStr( const MC2SimpleString& paramString );


   /**
    *    Get the next point that contains a feature that could be
    *    highlighted.
    *    @param   point [IN] The point to be set to the next highlightable
    *                   feature.
    *    @return  True if a highlightable feature was found and thus
    *             if the point was updated. False otherwise.
    */
   bool getNextHighlightPoint( MC2Point& point );

   /**
    *    Set if the map is moving or not.
    */
   void setMoving( bool moving );

   /**
    *    Set if to detect movement by self, or by someone using the
    *    setMoving method.
    */
   void setDetectMovementBySelf( bool detectSelf );

   /**
    *    @return Percentage of received maps / total number maps.
    */
   uint32 getPercentageMapsReceived() const;

   /**
    * Checks if the map is fully loaded or not, it is fully loaded
    * if getPercentageMapsReceived is 100 and if m_mapFormatDesc is
    * not null.
    * @return true if map is fully loaded.
    *         false if map is not fully loaded.
    */
   bool mapFullyLoaded() const;
   
   int getDistanceMeters(const MC2Point& point,
                         const Road& road,
                         float cosLat);

   int getDistanceMeters(const MC2Point& point,
                         const TilePrimitiveFeature& feat,
                         float cosLat);

   int getDistanceMeters(const MC2Coordinate& point,
                         const MC2Coordinate& lineBegin,
                         const MC2Coordinate& lineEnd,
                         float cosLat);
   
   int getDistanceMeters(const MC2Point& point,
                         const MC2Point& lineBegin,
                         const MC2Point& lineEnd,
                         float cosLat);
   
   /**
    *    Returns the distance from the coordinate coord to the
    *    line described by the coordinates linefirst and
    *    lineSecond.
    */
   static int64 sqDistanceCoordToLine(const TileMapCoord& coord,
                                      const TileMapCoord& lineFirst,
                                      const TileMapCoord& lineSecond,
                                      float cosF );

   /**
    *    Returns lhs if rhs has a north degree less than rhs from centerCoord.
    */

   /*   bool compareAngle(const MC2Coordinate& lhs, 
                     const MC2Coordinate& rhs, 
                     const MC2Coordinate& centerCoord) const;*/
private:

   friend class TileMapHandlerReleaseChecker;

   /// Easiest way to solve this right now
   friend class MapLib;

   /// Easiest way to solve this right now
   friend class MapLibInternal;

   /// For use in MapLib.
   DBufRequester* getInternalRequester() {
      return m_requester;
   } 

   int getFeatureId(TileMap* map,
                    const TilePrimitiveFeature* feature);
   
   #ifdef __unix__
   void plotDebugRoad(TileMap& curMap, TilePrimitiveFeature& curPrim,
                      const MC2BoundingBox& bbox);
   #endif

   /**
    *   Converts a bit map desc from the server to a bitmap name.
    */
   MC2SimpleString bitMapDescToBitMapName(const MC2SimpleString& desc);

   /**
    *   Converts a bit map name to a desc to use when requesting from
    *   the server.
    */
   MC2SimpleString bitMapNameToDesc( const MC2SimpleString& name );

   /**
    *   Gets a bitmap from the internal storage and returns it if
    *   it is there. If it is not there, the function will add it to
    *   the queue of buffers to be fetched from the server and return
    *   NULL.
    */
   isab::BitMap* getOrRequestBitMap( const MC2SimpleString& name );
   
   /**
    *   Requests the desc from cache or by requesting it from
    *   the requester.
    *   @param desc     DataBuffer to send for.
    *   @param reqType  The type of source that the desc may be requested
    *                   from. Default is both cache or internet.
    *   @return True if the buffer was found in the cache.
    */
   bool requestFromCacheOrForReal(
                        const MC2SimpleString& desc,
                        const DBufRequester::request_t& reqType =
                                    DBufRequester::cacheOrInternet );

   /**
    *    Clear coordinates for the feature.
    */
   inline void clearCoords( TilePrimitiveFeature& feat );
   
   /**
    *   Prepares the coordinates of the feature to work with
    *   the current screen.
    */
   inline int prepareCoordinates(TilePrimitiveFeature& feat,
                                 const MC2BoundingBox& bbox);
   
   /**
    *   Plots a feature containing polyline stuff.
    *   @param color     The color in rrggbb.
    *   @param lineWidth Line width in pixels.
    *   @return Number of coordinates drawn.    
    */
   inline int plotPolyLines(const TilePrimitiveFeature& feat,
                            uint32 color,
                            int lineWidth );

   /**
    *   Plots a polygon.
    *   @return Number of coordinates drawn.
    */
   inline int plotPolygon(const TilePrimitiveFeature& feat,
                          uint32 color );
   
   /**
    *   Called by dataBufferReceived when the BitBuffer
    *   contains a tileMap.
    *    @param   removeFromCache   [OUT] Descs that are to be removed
    *                                     from the cache.
    *    @return  True if everything was ok. False if the map contained
    *             unknown features and therefore the mapdesc should be
    *             rerequested.
    */
   inline bool tileMapReceived(const MC2SimpleString& descr,
                               BitBuffer* buffer,
                               std::vector<TileMapParams>& removeFromCache );

   /**
    *   Get which maps to plot.
    *   @param maps     [Out] The maps to plot.
    *   @return   True if the outparameter contains the maps to plot.
    *             False if m_mapVector should be used instead.
    */
   bool getWhichMapsToPlot( std::vector<TileMap*>& maps );

   /**
    *   @return If the feature is within the current scale.
    */
   int checkScale( const TilePrimitiveFeature& feature ) const;

   /**
    *    Draw the progess indicator.
    */
   void drawProgressIndicator();
   
   /**
    *   Plots the maps that we have already.
    */
   void plotWhatWeGot( );

   /**
    *   Plots the maps in a certain bounding box only.
    */
   void plotWhatWeGotInBBox( const MC2BoundingBox& bbox,
                             int skipOutLines,
                             int& nbrFeatures,
                             int& nbrDrawn );

   /**
    *   Draws the debug strings if enabled.
    */
   void drawDebugStrings();
   
   /**
    *   Get the inner pixel width of the polyline. 
    */
   inline uint32 getPolylineInnerPixelWidth( 
                              const TilePrimitiveFeature& prim ) const;
   
   /**
    *   Plots a polygon or line feature.
    *   @return The number of coordinates plotted.
    */
   inline int plotPolygonOrLineFeature( const TilePrimitiveFeature& prim,
                                        int pass,
                                        int moving );

   /**
    *   Converts the coordinates in the CoordArg of primWithCoordArg
    *   to screen coordinates.
    *   @return True if there were coordinates in the feature and they
    *           were inside the bbox.
    */
   inline bool getXYFromCoordArg( 
                         const TilePrimitiveFeature& primWithCoordArg,
                         const MC2BoundingBox& bbox,
                         int& x,
                         int& y ) const;

   /**
    *   Check if the primitive features is forbidden 
    *   (exists in a disabled category).
    *   @return True if the feature is forbidden, false otherwise.
    */
   inline bool forbiddenFeature( const TileMap& curMap,
                                 const TilePrimitiveFeature& prim ) const;
   
   /**
    *   Plots a bitmap feature. Must have one coordinate.
    */
   inline int plotBitMapFeature( TileMap& curMap,
                                 TilePrimitiveFeature& prim,
                                 const MC2BoundingBox& bbox,
                                 int pass,
                                 int moving );
   /**
    *   Plots a circle feature. Must have one coordinate.
    */
   inline int plotCircleFeature( TilePrimitiveFeature& prim,
                                 const MC2BoundingBox& bbox,
                                 int pass,
                                 int moving );

   int plotBBox( const MC2BoundingBox& bbox,
                 const MC2BoundingBox& screenBBox );
   
   /**
    *   Plots a feature.
    */
   inline int plotPrimitive( TileMap& curMap,
                             TilePrimitiveFeature& prim,
                             const MC2BoundingBox& bbox,
                             int pass,
                             int moving );
   
   /**
    *   Returns true if one of the coordinates in coords
    *   is inside the boundingbox.
    */
   inline static bool coordsInside(const CoordsArg& coords,
                                   const MC2BoundingBox& bbox);
     
   /**
    *   Returns true if the map with the given description
    *   is loaded.
    */
   inline bool haveMap(const MC2SimpleString& desc) const;

   /**
    *   Returns a pointer to the map with the given description
    *   or NULL if not loaded yet.
    */
   inline TileMap* getMap(const MC2SimpleString& desc) const;

   /**
    *   Returns true if new parameters have been created.
    *   @param oldParamsUpdated  [Out] If the old params also 
    *                            were updated.
    */
   bool getNewParams( bool& oldParamsUpdated );

   /**
    *   Updates the parameters so that they are ok for the
    *   current display etc.
    */
   void updateParams();

   /**
    *    Debug method. Dumps the params.
    */
   void dumpParams();
   
   /**
    *   Checks the cache for the existance of the descr
    *   and then calls dataBufferReceived if the requested
    *   data was in a cache.
    *   @param descr Descr to request.
    *   @return True if the buffer was in cache.
    */
   bool requestFromCacheAndCreate(const MC2SimpleString& descr);
   
   /**
    *   Requests maxNbr maps or other buffers.
    */
   void requestMaps(int maxNbr);

   // -- Geometry
   


   /**
    *    Returns the minimum distance from a coordinate to
    *    a primitive.
    */
   
   static int64 getMinSQDist(const TileMapCoord& coord,
                             const TilePrimitiveFeature& prim,
                             float cosLat);

   void getClosestPrimCoord(const TileMapCoord& coord,
                            const TilePrimitiveFeature& prim,
                            float cosLat,
                            MC2Coordinate& closestPrimCoord);


   /**
    *    Returns the closest feature and map for the given point.
    *    @param primType   [Optional] Default (MAX_INT32) is that
    *                      all primitive types are processed, 
    *                      but setting the param to a specific primitive
    *                      type (e.g. FeatureType::bitmap)
    *                      will lead to that only that primitive
    *                      type will be processed.
    */
   std::pair<const TileMap*, const TilePrimitiveFeature*>
      getFeatureAt(const MC2Point& points, uint32* distance = NULL,
                   int32 primType = MAX_INT32 );
   
   /**
    *   Get name for feature.
    *
    *   @param theMap         The map.
    *   @param theFeature     The feature to get the name of.
    *   @param infoCallback   [Optional] Callback that will be called
    *                         when the possibly missing string map is 
    *                         received.
    */
   const char* getNameForFeature( 
                               const TileMap& theMap,
                               const TilePrimitiveFeature& theFeature,
                               TileMapInfoCallback* infoCallback = NULL );
   
   /**
    *    Returns the first primitive that is inside a polygon.
    *    Goes through the levels top down.
    */
   std::pair<const TileMap*, const TilePrimitiveFeature*> getFirstInside(
      const TileMapCoord& coord) const;

   /**
    *    Get the bitmap's pixelbox. The method is not forgiving if
    *    submitting a non bitmap feature.
    */
   PixelBox getPixelBoxForBitMap( 
            const TilePrimitiveFeature& bitmap ) const; 
   
   /**
    *    Returns the closest feature or null.
    *    @param primType   MAX_INT32 means that
    *                      all primitive types are processed, 
    *                      but setting the param to a specific primitive
    *                      type (e.g. FeatureType::bitmap)
    *                      will lead to that only that primitive
    *                      type will be processed.
    */   
   inline const TilePrimitiveFeature*
      getClosest(const MC2Point& point,
                 const MC2Coordinate& coord,
                 TileMap::primVect_t::const_iterator begin,
                 TileMap::primVect_t::const_iterator end,
                 int64& mindist,
                 int primType ) const;
  
   /// Plot the copyright string.
   void plotCopyrightString();
   
   // -- Variables

   bool m_isDrawing;
   
   /**
    *   The MapPlotter can plot stuff, e.g. maps.
    */
   MapPlotter* m_plotter;

   /**
    *   The DBufRequester can request BitBuffers from
    *   server or e.g. a file using a string as key.
    */
   DBufRequester* m_requester;
   
   /**
    *    The current platform.
    */
   TileMapToolkit* m_toolkit;

   /** 
    *   The MapFormatDescription.
    */
   TileMapFormatDesc* m_mapFormatDesc;
   
   /**
    *    The current databufferlistener.
    */
   DBufRequestListener* m_dataBufferListener;

   /**
    *    The text handler.
    */
   TileMapTextHandler* m_textHandler;

   /**
    *    The id of the timer of the textHandler.
    */
   uint32 m_textHandlerIdleID;
   
   /**
    *    If == 0 we are not painting.
    */
   int m_painting;

 

   /**
    *   Callback to use when databuffer is received.
    *   Will release the buffer back to the requester.
    *   @param descr       The received descr.
    *   @param dataBuffer  The received dataBuffer.
    *   @param requestMore True if another buffer should be reuqested. 
    */
   void dataBufferReceived(const MC2SimpleString& descr,
                           BitBuffer* dataBuffer,
                           bool requestMore = true);

   /**
    *   Requests a idle object if the text handler wants one
    *   @return True if the text handler wants to run.
    */
   bool requestTextHandlerIdle();

   /// Draws the texts from the TileMapTextHandler.
   void drawTexts();

   /// Draws one UserDefinedFeature and updates the coords of it
   inline void drawOneUserDefinedFeature( UserDefinedFeature& userFeat );
   
   /// Draws the user defined features if there are any.
   void drawUserDefinedFeatures();
   
   /// Plots some debug on the screen.
   void printDebugStrings(const std::vector<MC2SimpleString>& strings);
   
   /// Pointer to the outline feature
   DirectedPolygon* m_outlinePolygon;


   // Determines if the level check in crossings detection should be 
   // disabled. Currently disabled due to faulty map data.
   //bool  m_shouldCheckLevels;

   /**
    *    Time when the stuff was repainted last time.
    */
   uint32 m_lastRepaintTime;
   
   /// Type of the map where we store our bitmaps.
   typedef std::map<MC2SimpleString, isab::BitMap*> bitMapMap_t;
   
   /**
    *   Map containing the bitmaps for e.g. pois.
    *   The leading 'B' has been removed and also the extension.
    */
   bitMapMap_t m_bitMaps;

   /**
    *   Map containing the bitmaps needed.
    *   The leading 'B' has been removed.
    */
   std::set<MC2SimpleString> m_neededBitMaps;
   
   /**
    *    True if gzipped maps should be used.
    */
   bool m_useGzip;

   /**
    *    The id of the repaint timer if a repaint timer
    *    has been requested. Else 0.
    */
   uint32 m_repaintTimerRequested;
   
   /**
    *    The id of the detail repaint timer if a detail repaint timer
    *    has been requested. Else 0. Used for filling in missing details
    *    after a while of non-movement.
    */
   uint32 m_detailRepaintTimerRequested;

   /**
    *    True if there has been time for the detail repaint timer
    *    to run.
    */
   int m_detailRepaintTimerHasBeenRun;

   /**
    *    The id of a timer that will repaint the screen
    *    after the last received map in a batch.
    */
   uint32 m_repaintAfterMapReceptionTimer;

   /// Timer that handles re-requests of maps that have not arrived.
   uint32 m_rerequestTimer;
   /// Time for the re-request timer
   uint32 m_rerequestTimerValue;
   enum {
      /// Minimum value for the re-request timer
      c_minRerequestTimerValue = 7500,
      /// Maximum value for the re-requsst timer.If above, it will be shut down
      c_maxRerequestTimerValue = 5*60*1000,
   };
   
   /// True if the transformmatrix has changed since the last draw.
   int m_matrixChangedSinceDraw;
   
   /**
    *    The current detaillevel.
    */
   int m_detailLevel;

   /// Set of layers to display (e.g. map, route, pois)
   std::set<int> m_layersToDisplay;

   /// The route id or NULL.
   RouteID* m_routeID;

   /// The current language
   LangTypes::language_t m_lang;  

   /// The overlap detector for bitmaps.
   OverlapDetector<PixelBox> m_overlapDetector;

   /// The current scale index.
   int m_scaleIndex;

   /// Random characters to put in request for DXXX
   MC2SimpleString m_randChars;

   /// List of debug printouts, a character in first and a time in second.
   std::list<std::pair<char, uint32> > m_drawTimes;

   /// Pointer to the list of user defined features.
   std::vector<UserDefinedFeature*>* m_userDefinedFeatures;
   
   /// The garbage collector.
   TileMapGarbage<TileMap> m_garbage;

   /// The maps.
   TileMapContainer* m_tileMapCont;

   /// True if tracking mode is on
   int m_trackingModeOn;

   /// Vector of old map descriptions which cannot be deleted when map refer
   std::vector<TileMapFormatDesc*> m_oldFormatDescs;

   /// Set of features that are disabled for drawing.
   std::set<int32> m_disabledParentTypes;

   /// Event listener to inform e.g. when DXXX has arrived
   std::vector<TileMapEventListener*> m_eventListeners;

   /// True if highlight should be shown.
   int m_showHighlight;
   
   /// Highlight point
   MC2Point m_highlightPoint;
   
   /// This class should be able to call the Handler when DB is recv.
   friend class TileMapHandlerDefaultDBListener;
   /// This class needs the m_mapFormatDesc
   friend class TileMapHandlerDBBufRequester;

   /// Screen coords for the currently drawn feature.
   std::vector<MC2Point> m_realScreenCoords;
   VectorProxy<MC2Point> m_screenCoords;

   /// The bitbuffer of the last loaded DXXX ( or null )
   BitBuffer* m_dxxBuffer;
   /// True if the crc for the DXXX has been received
   int m_descCRCReceived;
   /// Server's version of the crc
   uint32 m_serverDescCRC;

   /// The pixel box that the progress indicator text should be drawn into.
   PixelBox m_progressIndicatorBox;

   /// Current timing info from TileMapKeyHandler
   const TMKTimingInfo* m_tmkTimingInfo;

   /// True if a map has arrived since the map was repainted
   bool m_mapArrivedSinceRepaint;
   
   /// True if the outlines were drawn last time
   bool m_outlinesDrawnLastTime;

   /// Screen size the last time the screen was drawn
   isab::Rectangle m_lastScreenSize;
   
   /// Movement helper.
   MovementHelper m_movementHelper;

   /// Copyright string position.
   MC2Point m_copyrightPos;
   
   /// Release checker
   TileMapHandlerReleaseChecker* m_releaseChecker;
   
   /// If to show copyright.
   bool m_showCopyright;

   /// Clickinfo to return.
   TileMapHandlerClickResult* m_clickInfo;

   /**
    *    Keeps track if we requested a string map due to a call of
    *    getNameForFeature.
    * 
    *    first is the string map desc that we are waiting for.
    *    second is the callback class that should be called once this desc
    *    is received. If NULL then we are not waiting for any info strings.
    */
   std::pair<MC2SimpleString, TileMapInfoCallback*> m_waitingForInfoString;

   /**
    *    The bitmap mover helper class.
    */
   BitmapMovementHelper m_bitmapMover;
   
};

#endif // TILE_MAPHANDLER_H
