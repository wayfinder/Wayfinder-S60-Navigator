/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TILEMAPKEYHANDLER_H
#define TILEMAPKEYHANDLER_H

#include "config.h"

#include "PixelBox.h"
#include "MC2SimpleString.h"

// Define AUTOMOVE_ON to activate the code for auto movement.
#undef  AUTOMOVE_ON

class MapMovingInterface;
class ClickInfo;
class MapDrawingInterface;
class TileMapPeriodicTimer;
class TileMapToolkit;
class TileMapHandler;
class TMKDestination;
class TMKAutoMoveList;
class Cursor;
class TileMapInfoCallback;
class TileMapHandlerClickResult;

template <class T> class TileMapTimerListenerTemplate;

class TileMapCursorInterface {
public:
   virtual void setStaticCursor( bool staticCursor ) = 0;
   virtual void centerMapAtCursor() = 0;
   virtual void centerCursor() = 0;
   virtual void resetZoomCoord() = 0;
   virtual void setMapBox( const PixelBox& mapBox ) = 0;
};

class TileMapKeyHandlerCallback {
public:
   virtual ~TileMapKeyHandlerCallback() {}
   virtual bool keyHandlerCallback() = 0;   
};

class TileMapKeyHandler : public TileMapCursorInterface {
public:
   
   /// Symbolic key names.
   enum key_t {
      MOVE_UP_KEY,
      MOVE_DOWN_KEY,
      MOVE_LEFT_KEY,
      MOVE_RIGHT_KEY,
      MOVE_CURSOR_UP_KEY,
      MOVE_CURSOR_DOWN_KEY,
      MOVE_CURSOR_LEFT_KEY,
      MOVE_CURSOR_RIGHT_KEY,
      ZOOM_IN_KEY,
      ZOOM_OUT_KEY,
      AUTO_ZOOM_KEY,
      ROTATE_LEFT_KEY,
      ROTATE_RIGHT_KEY,
      RESET_ROTATION_KEY,
      HIGHLIGHT_NEXT_FEATURE_KEY,
      CENTER_MAP_AT_CURSOR_KEY,
      MOVE_UP_RIGHT_KEY,
      MOVE_DOWN_RIGHT_KEY,
      MOVE_UP_LEFT_KEY,
      MOVE_DOWN_LEFT_KEY,
#ifdef AUTOMOVE_ON
      /// Test button for the slow test. (Load maps etc.)
      SLOW_TEST_KEY,
      /// Test button for the full speed test
      FAST_TEST_KEY,
#endif
      /// Drag the map, using a pointer device.
      DRAG_TO,
      ZOOM_IN_ONE_STEP_KEY,
      ZOOM_OUT_ONE_STEP_KEY,
      /// Send this in if nothing should be done.
      NO_KEY,
   };

   /// Kind of keypress
   enum kind_of_press_t {
      /// The key was pressed
      KEY_UP_EVENT,
      /// The key was released
      KEY_DOWN_EVENT,
      /// The key was repeated
      KEY_REPEAT_EVENT,
      /// Unknown type of event
      KEY_UNKNOWN_EVENT,
   };
   
   /**
    *   Creates a new TileMapKeyHandler with the supplied
    *   tileMapHandler.
    *   @param tileMapHandler Needed to be able to set zoom etc.
    *   @param toolkit        Needed for the timers.
    *   @param callback       Will be called before each repaint if not NULL.
    *   @param infoCallback   Will be called when new strings that was needed
    *                         when showing info for a feature has arrived.
    */
   TileMapKeyHandler( MapMovingInterface* mapMovingInterface,
                      MapDrawingInterface* mapDrawingInterface,
                      TileMapToolkit* toolkit,
                      Cursor* cursor,
                      TileMapKeyHandlerCallback* callback = NULL,
                      TileMapInfoCallback* infoCallback = NULL );

   /**
    *
    */
   virtual ~TileMapKeyHandler();

   /**
    *   Handles the press of a key.
    *   @param key      The key which was pressed.
    *   @param upordown Type of event.
    *   @param info     [OUT] The pointer is set to point at the name
    *                   of the feature that the cursor is pointing at,
    *                   or NULL if there is no such feature (with a name).
    *   @param dragToPoint [Optional] Only necessary to supply if a
    *                      DRAG_TO key event is made. Should contain
    *                      the current position of the pointer device.
    *   @param movement [Optional,OUT] If specified, this *bool will be
    *                   set to if the keyhandling has inflicted some kind
    *                   of movement on the map.
    *   @return True if the key was consumed.
    */
   bool handleKeyEvent( key_t key,
                        kind_of_press_t upordown,
                        const char*& info,
                        const MC2Point* dragToPoint = NULL,
                        bool* movement = NULL ); 

   /**
    *   Stops the current action.
    */
   void stop();

   /**
    *   Called by the periodic timer.
    */
   void timerExpired( uint32 );
   
   /**
    *   Get the position of the cursor relative to the map.
    */
   MC2Point getCursorPosInMap() const;

   /**
    *   Sets if the cursor should be visible.
    *   @param visible  If the cursor should be visible.
    */
   virtual void setCursorVisible( bool visible );
  
   /**
    *   Sets the cursor's position on the screen.
    */
   void setCursorPos( const MC2Point& pos );
   
   /**
    *   Sets the cursor position in the map.
    */
   void setCursorPosInMap( const MC2Point& pos );

   /**
    *   Set the box of the map area.
    */
   void setMapBox( const PixelBox& mapBox );
  
   /**
    *   Get the box of the map area.
    */
   PixelBox getMapBox() const;
 
   /**
    *    Get info about the feature near the specified position.
    *    @param   pos         The position on the map.
    *    @param   onlyBitmaps If only bitmap feature should be processed.
    *    @return  Info about the feature near the position.
    */
   const ClickInfo&
   getInfoForFeatureAt( const MC2Point& point,
                        bool onlyPois,
                        TileMapInfoCallback* infoCallback = NULL );

   virtual void setStaticCursor( bool staticCursor );

protected:
   /// Moves the map.
   virtual bool move( int32 moveDeltaX, int32 moveDeltaY );

   /// Zooms the map.
   virtual void zoom( double factor );

   /// Drags the map to the specified point.
   void dragTo( const MC2Point& pointOnScreen );
   
   /// Highlight next feature.
   virtual void highlightNextFeature();
public:
   /// Centers the cursor position.
   virtual void centerCursor();

   /// Center map at cursor.
   virtual void centerMapAtCursor();

   /// Resets the zoom coordinate.
   virtual void resetZoomCoord();

protected:
   /// Reset any highlighted cursor to be nonhighlighted.
   virtual void prepareMove();

   /**
    * Check if to highlight at the cursor.
    * @return A pointer to the name of the feature at the cursor position.
    *         NULL if there is no such feature with a name.
    */
   virtual const char* checkHighlight();
   
   /**
    *   Handles a key up event.
    *   @param info     [OUT] The pointer is set to point at the name
    *                   of the feature that the cursor is pointing at,
    *                   or NULL if there is no such feature (with a name).
    *   @param dragToPoint [Optional] Only necessary to supply if a
    *                      DRAG_TO key event is made. Should contain
    *                      the current position of the pointer device.
    */
   inline bool handleKeyUpEvent( key_t key, 
                                 bool& repaint, 
                                 const char*& info,
                                 const MC2Point* dragToPoint = NULL );

   /**
    *   Handles a key repeat event.
    *   @param dragToPoint [Optional] Only necessary to supply if a
    *                      DRAG_TO key event is made. Should contain
    *                      the current position of the pointer device.
    */
   inline bool handleKeyRepeatEvent( TileMapKeyHandler::key_t key,
                                     bool& repaint,
                                     const MC2Point* dragToPoint = NULL );
   
   /**
    *   Handles a key down event.
    *   @param dragToPoint [Optional] Only necessary to supply if a
    *                      DRAG_TO key event is made. Should contain
    *                      the current position of the pointer device.
    */ 
   inline bool handleKeyDownEvent( key_t key, bool& repaint,
                                   const MC2Point* dragToPoint = NULL );
   /// Resets the press-times for all keys
   inline void resetAllKeys();

   /* updates the deltas according to the 
      time consumed for previous transform */
   inline float32 getDelta( const float32* thresholds,
                            const float32* deltas,
                            int nbrThresholds,
                            const float32* maxValues,
                            uint32 firstKeyPress );
  
   void updateRotation(uint32 keyTime);

   void updateMove(uint32 keyTime);

   void updateZoom(uint32 keyTime);

   /* sets delta values to default */
   void deltaDefaults();

#ifdef AUTOMOVE_ON
   /// Handles the timer in the test state.
   inline void handleTestStateTimer();
#endif
   
   /// Handles the key timer. Returns true if repaint needed.
   bool handleKeyTimer();
   
   /// State of the map
   enum map_state_t {
      AUTO_ZOOMING,
      MOVING,
      ROTATING,
      STATIC,
      ZOOMING,
#ifdef AUTOMOVE_ON
      SLOW_TEST_RUNNING,
      FAST_TEST_RUNNING,
#endif
   } m_mapState;

   /// Update the map state.
   inline void setMapState( TileMapKeyHandler::map_state_t state );

   /// Update the map state if the bool is true. Return the bool
   inline bool setMapStateIf( bool condition,
                              TileMapKeyHandler::map_state_t state );

   /// Periodic timer for key-repeats
   TileMapPeriodicTimer* m_timer;
   /// TileMapHandler
   MapMovingInterface* m_tileMapHandler;
   /// TileMapHandler
   MapDrawingInterface* m_mapRedrawer;
   /// Toolkit
   TileMapToolkit* m_toolkit;
   /// Listener
   TileMapTimerListenerTemplate<TileMapKeyHandler>* m_timerListener;
   /// Times for events
   uint32 m_key[NO_KEY];
   /// Scale where autozoom started
   double m_autoZoomFinalScale;

   uint32 m_lastDrawTime;
   int32 iMovDelta;
   int32 iRotDelta;
   float32 iZoomInFactor;
   float32 iZoomOutFactor;

   TileMapKeyHandlerCallback* m_callBack;
   
#ifdef AUTOMOVE_ON
   // These two are for the time measurments.
   TMKDestination* m_origPlace;
   TMKAutoMoveList* m_autoMoveList;
#endif

   /// The cursor.
   Cursor* m_cursor;

   /// If the cursor is statically centered in the map.
   bool m_staticCursor;

   /// The drag coordinate, if dragging the map.
   MC2Coordinate m_dragCoord;

   /// The map box.
   PixelBox m_mapBox;

   /// The top left position of the map box on the screen.
   MC2Point m_topLeftMapPos;

   /// The first point in the screen tapped when dragging.
   MC2Point m_firstPointOnScreen;

   /// If the map has moved since started waiting for info.
   bool m_movedSinceWaitingForInfo;

   /// The current name of the feature that the cursor is pointing at.
   MC2SimpleString m_featureName;

   /// The info callback.
   TileMapInfoCallback* m_infoCallback;
};

class TileCursorKeyHandler : public TileMapKeyHandler
{
public:
   /**
    *   Creates a new TileCursorKeyHandler with the supplied
    *   tileMapHandler.
    *   @param tileMapHandler Needed to be able to set zoom etc.
    *   @param toolkit        Needed for the timers.
    *   @param cursor         The cursor. Will not be deleted by this 
    *                         object.
    *   @param callback       Will be called before each repaint if not NULL.
    *   @param infoCallback   Will be called when new strings that was needed
    *                         when showing info for a feature has arrived.    
    **/
   TileCursorKeyHandler( MapMovingInterface* mapMovingInterface,
                         MapDrawingInterface* mapDrawingInterface,
                         TileMapToolkit* toolkit,
                         Cursor* cursor,
                         TileMapKeyHandlerCallback* callback = NULL,
                         TileMapInfoCallback* infoCallback = NULL );

   /**
    *   Destructor.
    */
   ~TileCursorKeyHandler();

   /**
    *   Sets if the cursor should be visible.
    *   @param visible  If the cursor should be visible.
    */
   void setCursorVisible( bool visible );

protected:

   /// Moves the cursor or the map.
   bool move( int32 moveDeltaX, int32 moveDeltaY );
   
   /// Zooms the map at the point of the cursor.
   void zoom( double factor );

   /// Highlight next feature.
   void highlightNextFeature();

public:   
   /// Center map at cursor.
   void centerMapAtCursor();

   /// Resets the zoom coordinate.
   void resetZoomCoord();

protected:
   /// Reset any highlighted cursor to be nonhighlighted.
   void prepareMove();
   
   /**
    *    Check if to highlight at the point.
    *    @param   pos         The position to check for highlight.
    *    @param   onlyBitmaps True if only bitmap names are of interest.
    *                         False if also for instance street names
    *                         are of interest.
    *    @return The name of the highlighted feature, or NULL if
    *            not highlighted.
    */
   const char* checkHighlight( const MC2Point& pos, 
                               bool onlyBitmaps );
   
   /**
    *    Check if to highlight at the cursor position.
    *    @return The name of the feature at the cursor position, or NULL.
    */
   const char* checkHighlight();
   
   /// The zoom coordinate.
   MC2Coordinate m_zoomCoord;

   /// If the cursor should be shown or not.
   bool m_showCursor;

};

#endif
